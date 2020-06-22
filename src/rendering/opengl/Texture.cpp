#include "Texture.hpp"

#include "glad/glad.h"

namespace OpenDemo
{
    namespace Rendering
    {
        namespace OpenGL
        {

            Texture2D::Texture2D()
                : _width(0)
                , _height(0)
            {
                glGenTextures(1, &_id);
            }

            Texture2D::~Texture2D()
            {
                glDeleteTextures(1, &_id);
            }

            Texture2D::OpenGlPixelFormatDescription Texture2D::GetOpenGlPixelFormatDescription(PixelFormat pixelFormat) const
            {

                static const OpenGlPixelFormatDescription formats[PIXEL_FORMAT_MAX] = {
                    { GL_R8, GL_RED, GL_UNSIGNED_BYTE }, // R8
                    { GL_RG8, GL_RG, GL_UNSIGNED_BYTE }, // RG8
                    { GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE }, // RGB8
                    { GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE }, // RGBA8
                    { GL_RGB565, GL_RGB, GL_UNSIGNED_SHORT_5_6_5 }, // R5G6B5
                    { GL_RGB5_A1, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1 }, // R5G5B5A1
                    { GL_RGBA32F, GL_RGBA, GL_FLOAT }, // R32G32B32A32_FLOAT
                    { GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT }, // R16G16B16A16_HALF
                    { GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT }, // D16
                };

                return formats[pixelFormat];
            }

            void Texture2D::Init(const Texture2D::Description& description, void* data)
            {
                _width = description.width;
                _height = description.height;
                _pixelFormatDescription = GetOpenGlPixelFormatDescription(description.pixelFormat);

                Bind(0);
                glTexImage2D(GL_TEXTURE_2D, 0, _pixelFormatDescription.internalFormat, _width, _height, 0, _pixelFormatDescription.format, _pixelFormatDescription.type, data);

                //TODO: normal sampler setup
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            }

            void Texture2D::Bind(int sampler)
            {
                glActiveTexture(GL_TEXTURE0 + sampler);
                glBindTexture(GL_TEXTURE_2D, _id);
            }

            void Texture2D::Resize(int width_, int height_)
            {
                //TODO asserts and check for update
                _width = width_;
                _height = height_;

                Bind(0);
                glTexImage2D(GL_TEXTURE_2D, 0, _pixelFormatDescription.internalFormat, _width, _height, 0, _pixelFormatDescription.format, _pixelFormatDescription.type, nullptr);
            };

        }
    }
}