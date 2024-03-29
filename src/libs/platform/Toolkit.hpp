#pragma once

#include "platform/Window.hpp"

#include "common/Event.hpp"
#include "common/Singleton.hpp"

namespace RR
{
    namespace Platform
    {
        struct VideoMode
        {
            Vector2i resolution;
            uint32_t redBits;
            uint32_t greenBits;
            uint32_t blueBits;
            uint32_t refreshRate;
        };

        struct Monitor
        {
            enum class Event
            {
                Disconnected,
                Connected
            };

            Vector2i position;
            VideoMode videoMode;
            Rect2i workArea;
            Vector2 dpiScale;
        };

        struct Cursor
        {
            enum class Type
            {
                Arrow,
                IBeam,
                Crosshair,
                Hand,
                HResize,
                VResize,
                Count
            };
        };

        class IToolkit
        {
        public:
            virtual ~IToolkit() = default;

        public:
            virtual std::vector<Monitor> GetMonitors() const = 0;
            virtual void PoolEvents() const = 0;
            virtual double GetTime() const = 0;

            // CreatePlatformWindow to avoid name collision with CreateWindow define from Windows.h
            virtual std::shared_ptr<Window> CreatePlatformWindow(const Window::Description& description) const = 0;
            virtual std::shared_ptr<Cursor> CreateCursor(Cursor::Type type) const = 0;
        };

        class Toolkit final : public Singleton<Toolkit>
        {
        public:
            Toolkit() = default;
            ~Toolkit();

            void Init();

            std::vector<Monitor> GetMonitors() const { return getPrivateImpl()->GetMonitors(); }
            void PoolEvents() const { getPrivateImpl()->PoolEvents(); }
            double GetTime() const { return getPrivateImpl()->GetTime(); }

            // CreatePlatformWindow to avoid name collision with CreateWindow define from Windows.h
            std::shared_ptr<Window> CreatePlatformWindow(const Window::Description& description) const
            {
                return getPrivateImpl()->CreatePlatformWindow(description);
            }

            std::shared_ptr<Cursor> CreateCursor(Cursor::Type type) const
            {
                return getPrivateImpl()->CreateCursor(type);
            }

        public:
            Event<void(const Monitor& monitor, Monitor::Event event)> OnMonitorConfigChanged;

        private:
            inline IToolkit* getPrivateImpl() const
            {
                ASSERT(impl_);
                return impl_.get();
            }

            inline IToolkit* getPrivateImpl()
            {
                ASSERT(impl_);
                return impl_.get();
            }

        private:
            std::unique_ptr<IToolkit> impl_;
        };
    }
}