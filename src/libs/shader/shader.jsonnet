local Effects = import "../../shaders/effects.jsonnet";
local ColorMask = import "../../shaders/colorMask.jsonnet";

{
    basicEffect: Effects.default + {
        cullMode: 'Back',
        colorWriteMasks: [ColorMask.RED | ColorMask.GREEN | ColorMask.BLUE],
    }
}