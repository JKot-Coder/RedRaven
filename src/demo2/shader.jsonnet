local RenderStates = import "renderStates.jsonnet";
local ColorMask = import "colorMask.jsonnet";

{
    vertexLayouts: {
        cube:
        {
            POSITION: {
                index: 0,
                format: "FLOAT3"
                # slot: 0
                # perInstance: false
                # InstanceDataStepRate
            },
            COLOR: {
                index: 0,
                format: "FLOAT4"
                # slot: 0
                # perInstance: false
                # InstanceDataStepRate
            },
        },
    },
    effects:
    {
        cube:
        {
            mainPass:{
                renderState: RenderStates.default,
                modules: ["shader.slang"],
                vertexShader: "mainVS",
                pixelShader: "mainPS",
              //  computeShader: "main",
                inputLayout: "cube"
            }
        }
    }
}