struct VSOutput
{
    float3 color : COLOR;
    float4 pos   : SV_Position;
};

struct PSInput
{
    float3 color : COLOR;
};

[shader("vertex")]
VSOutput vertexMain(in uint VertId : SV_VulkanVertexID)
{
    float4 Pos[3];
    Pos[0] = float4(-0.5, -0.5, 0.0, 1.0);
    Pos[1] = float4( 0.0, +0.5, 0.0, 1.0);
    Pos[2] = float4(+0.5, -0.5, 0.0, 1.0);

    float3 Col[3];
    Col[0] = float3(1.0, 0.0, 0.0); // red
    Col[1] = float3(0.0, 1.0, 0.0); // green
    Col[2] = float3(0.0, 0.0, 1.0); // blue

    VSOutput o;
    o.pos   = Pos[VertId];
    o.color = Col[VertId];
    return o;
}

// Fragment Shader
[shader("fragment")]
float4 fragmentMain(PSInput psIn) : SV_Target
{
    return float4(psIn.color.rgb, 1.0);
}