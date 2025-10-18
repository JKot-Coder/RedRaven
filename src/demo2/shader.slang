struct FrameParams
{
    float4x4 g_WorldViewProj;
};
ParameterBlock<FrameParams> frame;

//struct ObjectParams
//{
    uniform float offset;
    uniform float offset2;
//};
//ParameterBlock<ObjectParams> params;


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
    //float4x4 g_WorldViewProj = float4x4(0.5, 0.0, 0.0, 0.0, 0.0, 0.5, 0.0, 0.0, 0.0, 0.0, 0.01, -0.001, 0.0, 0.0, 0.0, 1.0);
    PSIn.Pos   = mul(float4(VSIn.Pos + float3(0,0,offset + offset2), 1.0), frame.g_WorldViewProj);
    PSIn.Color = VSIn.Color;
}

#define CONVERT_PS_OUTPUT_TO_GAMMA 0

[shader("fragment")]
float4 mainPS(in PSInput  PSIn): SV_TARGET
{
    float4 color = PSIn.Color;
#if CONVERT_PS_OUTPUT_TO_GAMMA
    // Use fast approximation for gamma correction.
    color.rgb = pow(color.rgb, float3(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2));
#endif
    return color;
}