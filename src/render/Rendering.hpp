#pragma once

#include <memory>

namespace Windowing {
    class Window;
}

namespace Render {

    class Shader;

    enum Attributes{
        POSITION,
        NORMAL,
        TEXCOORD,
        COLOR,

        MAX_ATTRIBUTES
    };

    class Rendering {
    public:
        inline static const std::unique_ptr<Rendering>& Instance() {
            return instance;
        }

        virtual ~Rendering() {}

        virtual void Init(const std::shared_ptr<Windowing::Window>& window) = 0;
        virtual void Terminate() = 0;

        virtual std::shared_ptr<Shader> CreateShader() = 0;

    private:
        static std::unique_ptr<Rendering> instance;
    };

    inline static const std::unique_ptr<Rendering>& Instance() {
        return Rendering::Instance();
    }

}