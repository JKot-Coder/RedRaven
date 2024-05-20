#pragma once

namespace OpenDemo
{
    namespace Rendering
    {
        enum class BlendingParameters
        {
            ZERO,
            ONE,
            SRC_COLOR,
            INV_SRC_COLOR,
            SRC_ALPHA,
            INV_SRC_ALPHA,
            DEST_ALPHA,
            INV_DEST_ALPHA,
            DEST_COLOR,
            INV_DEST_COLOR,
            SRC_ALPHA_SAT,
            BLEND_FACTOR,
            INV_BLEND_FACTOR,
            SRC1_COLOR,
            INV_SRC1_COLOR,
            SRC1_ALPHA,
            INV_SRC1_ALPHA,
            BLENDING_MAX
        };

        enum class BlendingOperation
        {
            ADD,
            BLENDING_OPERATION_MAX
        };

        enum class BlendingMode
        {
            ADDITIVE
        };

        struct BlendingDescription
        {
        public:
            inline BlendingDescription()
            {
                SetBlendingMode(BlendingMode::ADDITIVE);
            }

            inline BlendingDescription(BlendingMode blendingMode)
            {
                SetBlendingMode(blendingMode);
            }

            inline void SetBlendingMode(BlendingMode blendingMode)
            {
                switch (blendingMode)
                {
                    case BlendingMode::ADDITIVE:
                        colorSrc = BlendingParameters::ONE;
                        colorDest = BlendingParameters::ONE;
                        colorOperation = BlendingOperation::ADD;

                        alphaSrc = BlendingParameters::ZERO;
                        alphaDest = BlendingParameters::ONE;
                        alphaOperation = BlendingOperation::ADD;
                        break;
                }
            }

            BlendingParameters colorSrc, colorDest;
            BlendingParameters alphaSrc, alphaDest;
            BlendingOperation colorOperation, alphaOperation;
        };
    }
}