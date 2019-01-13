#extension GL_ARB_explicit_attrib_location : require

#define PI 3.1415926535897932384626433832795

uniform vec4 Material;
uniform vec4 CameraPosition;
uniform vec4 LightDirection;

float saturate(float x)
{
    return max(0.0, min(1.0, x));
}

// Pow5 uses the same amount of instructions as generic pow(), but has 2 advantages:
// 1) better instruction pipelining
// 2) no need to worry about NaNs
float Pow5(float x)
{
    return x*x * x*x * x;
}

vec2 Pow5(vec2 x)
{
    return x*x * x*x * x;
}

vec3 Pow5(vec3 x)
{
    return x*x * x*x * x;
}

vec4 Pow5(vec4 x)
{
    return x*x * x*x * x;
}

vec3 F_Schlick(vec3 f0, float f90, float u)
{
    return f0 + (vec3(f90) - f0) * Pow5(1.0 - u);
}

float V_SmithGGXCorrelated(float NdotL, float NdotV, float alphaG)
{
    // Original formulation of G_SmithGGX Correlated
    // lambda_v = ( -1 + sqrt ( alphaG2 * (1 - NdotL2 ) / NdotL2 + 1)) * 0.5 f;
    // lambda_l = ( -1 + sqrt ( alphaG2 * (1 - NdotV2 ) / NdotV2 + 1)) * 0.5 f;
    // G_SmithGGXCorrelated = 1 / (1 + lambda_v + lambda_l );
    // V_SmithGGXCorrelated = G_SmithGGXCorrelated / (4.0 f * NdotL * NdotV );

    // This is the optimize version
    float alphaG2 = alphaG * alphaG;
    // Caution : the " NdotL *" and " NdotV *" are explicitely inversed , this is not a mistake .
    float Lambda_GGXV = NdotL * sqrt (( - NdotV * alphaG2 + NdotV ) * NdotV + alphaG2 );
    float Lambda_GGXL = NdotV * sqrt (( - NdotL * alphaG2 + NdotL ) * NdotL + alphaG2 );

    return 0.5 / (Lambda_GGXV + Lambda_GGXL);
}

float D_GGX(float NdotH , float m)
{
    // Divide by PI is apply later
    float m2 = m * m;
    float f = (NdotH * m2 - NdotH) * NdotH + 1;
    return m2 / (f * f);
}

float Lambert(float NdotV, float NdotL, float LdotH, float linearRoughness)
{
    return 1.0f;//NdotL;
}

float DisneyDiffuse(float NdotV, float NdotL, float LdotH, float linearRoughness)
{
    float fd90           = 0.5 + 2 * LdotH * LdotH * linearRoughness;
    vec3  f0             = vec3(1.0, 1.0, 1.0);
    // Two schlick fresnel term
    float lightScatter   = F_Schlick(f0, fd90, NdotL).r;
    float viewScatter    = F_Schlick(f0, fd90, NdotV).r;

    return lightScatter * viewScatter;
}

//Renormalized energy version
float DisneyDiffuseRenorm(float NdotV, float NdotL, float LdotH, float linearRoughness)
{
    float energyBias    = mix(0, 0.5, linearRoughness);
    float energyFactor  = mix(1.0, 1.0 / 1.51, linearRoughness);
    float fd90          = energyBias + 2.0 * LdotH * LdotH * linearRoughness;
    vec3  f0            = vec3(1.0, 1.0, 1.0);
     // Two schlick fresnel term
    float lightScatter   = F_Schlick(f0, fd90, NdotL).r;
    float viewScatter    = F_Schlick(f0, fd90, NdotV).r;

    return lightScatter * viewScatter * energyFactor;
}

vec3 tonemapReinhard(vec3 x){
    float exposure = 0.125;
    float lum = dot(x, vec3(0.2126f, 0.7152f, 0.0722f));
    float L = exposure*lum;//(scale / averageLum) * lum;
    //float Ld = (L * (1.0 + L / lumwhite2)) / (1.0 + L);
    float Ld = (L * (1.0 + L)) / (1.0 + L);
    return (x / lum) * Ld;
}

struct VertexData {
    vec3 Normal;
    vec3 Tangent;
    vec3 Binormal;
    vec3 WorldPosition;
    vec2 UV;
};

#ifdef VERTEX

layout(location = 0) in vec3 Position;
layout(location = 1) in vec2 UV;
layout(location = 2) in vec3 Normal;
layout(location = 3) in vec3 Tangent;
layout(location = 4) in vec3 Binormal;
layout(location = 5) in vec4 Color;

uniform mat4 ViewProjection;
uniform mat4 Model;

out VertexData Vertex;

void main()
{
    vec4 WorldPosition = Model * vec4(Position, 1.0);

    Vertex.Normal = normalize(mat3(Model) * Normal);
    Vertex.Tangent = normalize(mat3(Model) * Tangent);
    Vertex.Binormal = normalize(mat3(Model) * Binormal);
	Vertex.WorldPosition = WorldPosition.xyz;
	Vertex.UV = UV;

	gl_Position = ViewProjection * WorldPosition;
}

#endif

#ifdef FRAGMENT

in VertexData Vertex;

out vec4 FragColor;

uniform sampler2D AlbedoMap;
uniform sampler2D NormalMap;
uniform sampler2D RoughnessMap;
uniform sampler2D MetallicMap;

void main()
{
    mat3 TBNBasis = mat3(normalize(Vertex.Tangent),
                         normalize(Vertex.Binormal),
                         normalize(Vertex.Normal));

    vec3 normal = texture(NormalMap, Vertex.UV).xyz;
    vec3 N = normalize(TBNBasis * (2.0 * normal - 1.0));
    vec3 V = normalize(CameraPosition.xyz - Vertex.WorldPosition);
    vec3 L = normalize(LightDirection.xyz);

   // V = vec3(0, 0, 1);
    L = dot(L, N) < 0 ? -L : L;
    L = normalize(vec3(-1, 1, 1));

    // This code is an example of call of previous functions
    float NdotV = abs(dot(N, V)) + 1e-5; // avoid artifact

    vec3 H = normalize(V + L);

    float LdotH = saturate(dot(L, H));
    float NdotH = saturate(dot(N, H));
    float NdotL = saturate(dot(N, L));

    float LdotV = saturate(dot(L, V));

    float linearRoughness = texture(RoughnessMap, Vertex.UV).r;
    float roughness = linearRoughness * linearRoughness;
    float metallic = texture(MetallicMap, Vertex.UV).r;

    float oneMinusReflectivity = 0.96f - metallic * 0.96f;
    vec4 albedo = texture(AlbedoMap, Vertex.UV);
    albedo.rgb = vec3( pow(albedo.r, 2.2), pow(albedo.g, 2.2), pow(albedo.b, 2.2));

    // Specular BRDF
    vec3  f0 = mix(vec3(0.04f, 0.04f, 0.04f), albedo.rgb, metallic);
    float f90 = 1.0;//- linearRoughness);// + (1-oneMinusReflectivity));
    vec3  F   = F_Schlick(f0, f90, NdotV);
    float Vis = V_SmithGGXCorrelated(NdotV, NdotL, roughness);
    float D   = D_GGX(NdotH, roughness);
    vec3  Fr  = D * F * Vis;
    albedo.rgb *= oneMinusReflectivity;

    // Diffuse BRDF
    //vec3 renormCoeff = vec3(1.0f - F);
    //vec3 renormCoeff = vec3(mix( 1.0, 1.0 / 1.51, linearRoughness ));
    //float Fd = Lambert(NdotV, NdotL, LdotH, linearRoughness);

    float Fd = DisneyDiffuseRenorm(NdotV, NdotL, LdotH, linearRoughness);
    vec3 diffuse = Fd * albedo.rgb;
	vec3 color = vec3(diffuse + Fr) * NdotL / PI;

    FragColor = vec4(color, albedo.a);
}

#endif