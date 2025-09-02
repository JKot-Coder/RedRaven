local ColorMask = import "colorMask.jsonnet";
local ComparisonFunc = import "comparisonFunc.jsonnet";

{
    default: {
        cullMode: 'Back',
        fillMode: 'Solid',
        colorWriteMasks: [ColorMask.ALL],
        depthAccess: 'ReadWrite',
        depthFunc: ComparisonFunc.LessEqual,
        stencilEnabled: false
    },
}