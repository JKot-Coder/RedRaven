local RenderStates = import "renderStates.jsonnet";
local ColorMask = import "colorMask.jsonnet";

{
    basicEffect:
    {
        mainPass:{
            renderState: RenderStates.default + {
                cullMode: 'Back',
                colorWriteMasks: [ColorMask.RED | ColorMask.GREEN | ColorMask.BLUE],
            },
            modules: ["shader.slang"],
            vertexShader: "vertexMain",
            pixelShader: "fragmentMain"
        }
    }
}