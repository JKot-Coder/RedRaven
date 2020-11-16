#pragma once

namespace OpenDemo
{
    namespace Render
    {
        class Object;
        class Fence;
        class Resource;

        class ResourceView;
        struct ResourceViewDescription;
        class RenderTargetView;

        class Texture;
        struct TextureDescription;
        enum class TextureDimension : uint32_t;

        class Buffer;
        class Submission;
        class CommandContext;

        struct Result;
    }
}