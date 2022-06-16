#include "platform/Toolkit.hpp"

struct GLFWcursor;

namespace RR
{
    namespace Platform
    {
        struct GlfwCursor : Cursor
        {
            GlfwCursor(GLFWcursor* cursor) : cursor(cursor) { ASSERT(cursor); }
            ~GlfwCursor();
            GLFWcursor* cursor;
        };

        class GlfwToolkit final : public IToolkit
        {
        public:
            GlfwToolkit() = default;
            ~GlfwToolkit();

            void Init();
            std::vector<Monitor> GetMonitors() const override;
            void PoolEvents() const override;
            double GetTime() const override;

            std::shared_ptr<Window> CreatePlatformWindow(const Window::Description& description) const override;
            std::shared_ptr<Cursor> CreateCursor(Cursor::Type type) const override;

        private:
            bool isInited_ = false;
            mutable std::array<std::weak_ptr<GlfwCursor>, size_t(Cursor::Type::Count)> cursors_;
        };
    }
}