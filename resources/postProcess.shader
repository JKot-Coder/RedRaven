#extension GL_ARB_explicit_attrib_location : require

#define PI 3.1415926535897932384626433832795

struct VertexData {
    vec2 TextureCoord;
};

uniform sampler2D AlbedoTex;

#ifdef VERTEX

layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec2 TextureCoord;

out VertexData Vertex;

void main()
{
    Vertex.TextureCoord = TextureCoord;
	gl_Position = vec4(Position, 1.0);
}

#endif

#ifdef FRAGMENT

in VertexData Vertex;

out vec4 fragColor;

void main()
{
    vec4 color = texture(AlbedoTex, Vertex.TextureCoord);
    color.rgb = (color.rgb / color.a) * 2.0 * PI;

  if (max(max(color.r, color.g), color.b) > 1.2)
    {
        fragColor = vec4(0.0, 1.0, 0.0, 1.0);
        return;
    }

    if (max(max(color.r, color.g), color.b) > 1.0)
    {
        fragColor = vec4(1.0, 0.0, 0.0, 1.0);
        return;
    }

 //   fragColor = vec4( color.rgb, 1.0);
	fragColor = vec4( pow(color.r, 1/2.2), pow(color.g, 1/2.2), pow(color.b, 1/2.2), 1.0);
}

#endif