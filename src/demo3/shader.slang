
uniform float foo;
uniform float bar;


struct test2
{
    float zzz;
}
struct test
{
    float j;
}
ConstantBuffer<test> ignore;

struct FrameParams
{
    float4x4 g_WorldViewProj;
   // Texture2D<float4> tex2;

   // ConstantBuffer<test> c;
    test2 zzz;
    float f;
};
ParameterBlock<FrameParams> frame;

struct FrameParams2
{
    float j2;
  //  ConstantBuffer<test> c2;
 //    ConstantBuffer<test> c3;
};
ParameterBlock<FrameParams2> frame2;

//Texture2D<float4> tex;


struct VSInput
{
    float3 Pos   : POSITION;
    float4 Color : COLOR;
};

struct PSInput
{
    float4 Pos   : SV_POSITION;
    float4 Color : COLOR0;
};


[shader("vertex")]
void mainVS(in  VSInput VSIn, out PSInput PSIn)
{
    float4x4 g_WorldViewProj = float4x4(0.5, 0.0, 0.0, 0.0, 0.0, 0.5, 0.0, 0.0, 0.0, 0.0, 0.01, -0.001, 0.0, 0.0, 0.0, 1.0);
    PSIn.Pos= mul(float4(VSIn.Pos, 1.0), g_WorldViewProj);

   // PSIn.Pos   = mul(float4(VSIn.Pos + float3(0,0,foo +frame.zzz.zzz+ frame.f+ bar  /*+ frame.c.j + frame2.c2.j  + frame2.c3.j */+ frame2.j2 /* tex.Load(int3(int2(VSIn.Pos.xy),0)).r  +  frame.tex2.Load(int3(int2(VSIn.Pos.xy),0)).r), 1.0)*/, frame.g_WorldViewProj);
    PSIn.Color = VSIn.Color;
}

#define CONVERT_PS_OUTPUT_TO_GAMMA 0

[shader("fragment")]
float4 mainPS(uniform float czzzz, in PSInput  PSIn): SV_TARGET
{
    float4 color = PSIn.Color;///+ czzzz  + frame.c.j + frame.f;
#if CONVERT_PS_OUTPUT_TO_GAMMA
    // Use fast approximation for gamma correction.
    color.rgb = pow(color.rgb, float3(1.0 / 2.2 , 1.0 / 2.2, 1.0 / 2.2));
#endif
    return color;
}