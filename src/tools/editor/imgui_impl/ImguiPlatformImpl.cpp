#include "ImguiPlatformImpl.hpp"
#include "imgui.h"

#include "platform/Input.hpp"
#include "platform/Toolkit.hpp"

#ifdef _WIN32
#undef APIENTRY
#include "windows.h" // for HWND
#endif

namespace RR
{
    using namespace Platform;

    namespace
    {
        ImVec2 Vector2ToImVec(const Vector2& vector)
        {
            return *reinterpret_cast<const ImVec2*>(&vector);
        }

        ImVec2 Vector2iToImVec(const Vector2i& vector)
        {
            auto fVector = vector.Cast<float>();
            return *reinterpret_cast<ImVec2*>(&fVector);
        }

        Vector2i ImVecToVector2i(const ImVec2& vector)
        {
            auto fVector = reinterpret_cast<const Vector2*>(&vector);
            return fVector->Cast<int32_t>();
        }

        ImGuiKey KeyToImGuiKey(Input::Key key)
        {
            switch (key)
            {
                case Input::Key::Tab: return ImGuiKey_Tab;
                case Input::Key::Left: return ImGuiKey_LeftArrow;
                case Input::Key::Right: return ImGuiKey_RightArrow;
                case Input::Key::Up: return ImGuiKey_UpArrow;
                case Input::Key::Down: return ImGuiKey_DownArrow;
                case Input::Key::PageUp: return ImGuiKey_PageUp;
                case Input::Key::PageDown: return ImGuiKey_PageDown;
                case Input::Key::Home: return ImGuiKey_Home;
                case Input::Key::End: return ImGuiKey_End;
                case Input::Key::Insert: return ImGuiKey_Insert;
                case Input::Key::Delete: return ImGuiKey_Delete;
                case Input::Key::Backspace: return ImGuiKey_Backspace;
                case Input::Key::Space: return ImGuiKey_Space;
                case Input::Key::Enter: return ImGuiKey_Enter;
                case Input::Key::Escape: return ImGuiKey_Escape;
                case Input::Key::Apostrophe: return ImGuiKey_Apostrophe;
                case Input::Key::Comma: return ImGuiKey_Comma;
                case Input::Key::Minus: return ImGuiKey_Minus;
                case Input::Key::Period: return ImGuiKey_Period;
                case Input::Key::Slash: return ImGuiKey_Slash;
                case Input::Key::Semicolon: return ImGuiKey_Semicolon;
                case Input::Key::Equal: return ImGuiKey_Equal;
                case Input::Key::LeftBracket: return ImGuiKey_LeftBracket;
                case Input::Key::Backslash: return ImGuiKey_Backslash;
                case Input::Key::RightBracket: return ImGuiKey_RightBracket;
                case Input::Key::GraveAccent: return ImGuiKey_GraveAccent;
                case Input::Key::CapsLock: return ImGuiKey_CapsLock;
                case Input::Key::ScrollLock: return ImGuiKey_ScrollLock;
                case Input::Key::NumLock: return ImGuiKey_NumLock;
                case Input::Key::PrintScreen: return ImGuiKey_PrintScreen;
                case Input::Key::Pause: return ImGuiKey_Pause;
                case Input::Key::Keypad0: return ImGuiKey_Keypad0;
                case Input::Key::Keypad1: return ImGuiKey_Keypad1;
                case Input::Key::Keypad2: return ImGuiKey_Keypad2;
                case Input::Key::Keypad3: return ImGuiKey_Keypad3;
                case Input::Key::Keypad4: return ImGuiKey_Keypad4;
                case Input::Key::Keypad5: return ImGuiKey_Keypad5;
                case Input::Key::Keypad6: return ImGuiKey_Keypad6;
                case Input::Key::Keypad7: return ImGuiKey_Keypad7;
                case Input::Key::Keypad8: return ImGuiKey_Keypad8;
                case Input::Key::Keypad9: return ImGuiKey_Keypad9;
                case Input::Key::KeypadDecimal: return ImGuiKey_KeypadDecimal;
                case Input::Key::KeypadDivide: return ImGuiKey_KeypadDivide;
                case Input::Key::KeypadMultiply: return ImGuiKey_KeypadMultiply;
                case Input::Key::KeypadSubtract: return ImGuiKey_KeypadSubtract;
                case Input::Key::KeypadAdd: return ImGuiKey_KeypadAdd;
                case Input::Key::KeypadEnter: return ImGuiKey_KeypadEnter;
                case Input::Key::KeypadEqual: return ImGuiKey_KeypadEqual;
                case Input::Key::LeftShift: return ImGuiKey_LeftShift;
                case Input::Key::LeftControl: return ImGuiKey_LeftCtrl;
                case Input::Key::LeftAlt: return ImGuiKey_LeftAlt;
                case Input::Key::LeftSuper: return ImGuiKey_LeftSuper;
                case Input::Key::RightShift: return ImGuiKey_RightShift;
                case Input::Key::RightControl: return ImGuiKey_RightCtrl;
                case Input::Key::RightAlt: return ImGuiKey_RightAlt;
                case Input::Key::RightSuper: return ImGuiKey_RightSuper;
                case Input::Key::Menu: return ImGuiKey_Menu;
                case Input::Key::Key0: return ImGuiKey_0;
                case Input::Key::Key1: return ImGuiKey_1;
                case Input::Key::Key2: return ImGuiKey_2;
                case Input::Key::Key3: return ImGuiKey_3;
                case Input::Key::Key4: return ImGuiKey_4;
                case Input::Key::Key5: return ImGuiKey_5;
                case Input::Key::Key6: return ImGuiKey_6;
                case Input::Key::Key7: return ImGuiKey_7;
                case Input::Key::Key8: return ImGuiKey_8;
                case Input::Key::Key9: return ImGuiKey_9;
                case Input::Key::A: return ImGuiKey_A;
                case Input::Key::B: return ImGuiKey_B;
                case Input::Key::C: return ImGuiKey_C;
                case Input::Key::D: return ImGuiKey_D;
                case Input::Key::E: return ImGuiKey_E;
                case Input::Key::F: return ImGuiKey_F;
                case Input::Key::G: return ImGuiKey_G;
                case Input::Key::H: return ImGuiKey_H;
                case Input::Key::I: return ImGuiKey_I;
                case Input::Key::J: return ImGuiKey_J;
                case Input::Key::K: return ImGuiKey_K;
                case Input::Key::L: return ImGuiKey_L;
                case Input::Key::M: return ImGuiKey_M;
                case Input::Key::N: return ImGuiKey_N;
                case Input::Key::O: return ImGuiKey_O;
                case Input::Key::P: return ImGuiKey_P;
                case Input::Key::Q: return ImGuiKey_Q;
                case Input::Key::R: return ImGuiKey_R;
                case Input::Key::S: return ImGuiKey_S;
                case Input::Key::T: return ImGuiKey_T;
                case Input::Key::U: return ImGuiKey_U;
                case Input::Key::V: return ImGuiKey_V;
                case Input::Key::W: return ImGuiKey_W;
                case Input::Key::X: return ImGuiKey_X;
                case Input::Key::Y: return ImGuiKey_Y;
                case Input::Key::Z: return ImGuiKey_Z;
                case Input::Key::F1: return ImGuiKey_F1;
                case Input::Key::F2: return ImGuiKey_F2;
                case Input::Key::F3: return ImGuiKey_F3;
                case Input::Key::F4: return ImGuiKey_F4;
                case Input::Key::F5: return ImGuiKey_F5;
                case Input::Key::F6: return ImGuiKey_F6;
                case Input::Key::F7: return ImGuiKey_F7;
                case Input::Key::F8: return ImGuiKey_F8;
                case Input::Key::F9: return ImGuiKey_F9;
                case Input::Key::F10: return ImGuiKey_F10;
                case Input::Key::F11: return ImGuiKey_F11;
                case Input::Key::F12: return ImGuiKey_F12;
                default: return ImGuiKey_None;
            }
        }
    }

    // Helper structure we store in the void* RenderUserData field of each ImGuiViewport to easily retrieve our backend data.
    struct ImGuiViewportData
    {
        std::shared_ptr<Window> Window;
        bool WindowOwned;
        int IgnoreWindowPosEventFrame;
        int IgnoreWindowSizeEventFrame;

        ImGuiViewportData()
        {
            Window = nullptr;
            WindowOwned = false;
            IgnoreWindowSizeEventFrame = IgnoreWindowPosEventFrame = -1;
        }

        ~ImGuiViewportData()
        {
            ASSERT(Window == nullptr);
        }
    };

    struct ImGuiData
    {
        std::shared_ptr<Window> MainWindow;
        std::array<std::shared_ptr<Cursor>, size_t(ImGuiMouseCursor_COUNT)> MouseCursors;
        std::array<const Platform::Window*, size_t(Input::Key::Count)> KeyOwnerWindows;
        bool InstalledCallbacks;
        bool WantUpdateMonitors;
        ImGuiData() { memset((void*)this, 0, sizeof(*this)); }
    };

    // Backend data stored in io.BackendPlatformUserData to allow support for multiple Dear ImGui contexts
    // It is STRONGLY preferred that you use docking branch with multi-viewports (== single Dear ImGui context + multiple windows) instead of multiple Dear ImGui contexts.
    // FIXME: multi-context support is not well tested and probably dysfunctional in this backend.
    // FIXME: some shared resources (mouse cursor shape, gamepad) are mishandled when using multi-context.
    ImGuiData* GetBackendDataPtr()
    {
        return ImGui::GetCurrentContext() ? (ImGuiData*)ImGui::GetIO().BackendPlatformUserData : nullptr;
    }

    ImGuiData& GetBackendData()
    {
        auto dataPtr = GetBackendDataPtr();
        ASSERT_MSG(dataPtr, "Did you call Init()?");

        return *dataPtr;
    }

    void UpdateKeyModifiers(Input::ModifierFlag modFlags)
    {
        ImGuiIO& io = ImGui::GetIO();
        io.AddKeyEvent(ImGuiKey_ModCtrl, IsSet(modFlags, Input::ModifierFlag::Control));
        io.AddKeyEvent(ImGuiKey_ModShift, IsSet(modFlags, Input::ModifierFlag::Shift));
        io.AddKeyEvent(ImGuiKey_ModAlt, IsSet(modFlags, Input::ModifierFlag::Alt));
        io.AddKeyEvent(ImGuiKey_ModSuper, IsSet(modFlags, Input::ModifierFlag::Super));
    }

    void WindowCloseCallback(const Window& window)
    {
        if (ImGuiViewport* viewport = ImGui::FindViewportByPlatformHandle((void*)&window))
            viewport->PlatformRequestClose = true;
    }

    // GLFW may dispatch glfwWindow pos/size events after calling glfwSetWindowPos()/glfwSetWindowSize().
    // However: depending on the platform the callback may be invoked at different time:
    // - on Windows it appears to be called within the glfwSetWindowPos()/glfwSetWindowSize() call
    // - on Linux it is queued and invoked during glfwPollEvents()
    // Because the event doesn't always fire on glfwSetWindowXXX() we use a frame counter tag to only
    // ignore recent glfwSetWindowXXX() calls.
    void WindowPosCallback(const Window& window, const Vector2i& pos)
    {
        std::ignore = pos;

        if (ImGuiViewport* viewport = ImGui::FindViewportByPlatformHandle((void*)&window))
        {
            if (ImGuiViewportData* vd = (ImGuiViewportData*)viewport->PlatformUserData)
            {
                bool ignoreEvent = (ImGui::GetFrameCount() <= vd->IgnoreWindowPosEventFrame + 1);
                //data->IgnoreWindowPosEventFrame = -1;
                if (ignoreEvent)
                    return;
            }
            viewport->PlatformRequestMove = true;
        }
    }

    void WindowSizeCallback(const Window& window, const Vector2i& size)
    {
        std::ignore = size;
        if (ImGuiViewport* viewport = ImGui::FindViewportByPlatformHandle((void*)&window))
        {
            if (ImGuiViewportData* vd = (ImGuiViewportData*)viewport->PlatformUserData)
            {
                bool ignore_event = (ImGui::GetFrameCount() <= vd->IgnoreWindowSizeEventFrame + 1);
                //data->IgnoreWindowSizeEventFrame = -1;
                if (ignore_event)
                    return;
            }
            viewport->PlatformRequestResize = true;
        }
    }

    void MouseCrossCallback(const Window& window, bool entered)
    {
        std::ignore = window;
        ImGuiIO& io = ImGui::GetIO();

        if (!entered)
            io.AddMousePosEvent(-FLT_MAX, -FLT_MAX);
    }

    void MouseButtonCallback(const Window& window, Input::MouseButton button, Input::KeyAction action, Input::ModifierFlag modFlags)
    {
        std::ignore = window;
        UpdateKeyModifiers(modFlags);

        if (action != Input::KeyAction::Press &&
            action != Input::KeyAction::Release)
            return;

        ImGuiIO& io = ImGui::GetIO();
        switch (button)
        {
            case Input::MouseButton::Left: io.AddMouseButtonEvent(ImGuiMouseButton_Left, action == Input::KeyAction::Press); break;
            case Input::MouseButton::Right: io.AddMouseButtonEvent(ImGuiMouseButton_Right, action == Input::KeyAction::Press); break;
            case Input::MouseButton::Middle: io.AddMouseButtonEvent(ImGuiMouseButton_Middle, action == Input::KeyAction::Press); break;
            default: return;
        }
    }

    void ScrollCallback(const Window& window, const Vector2& wheel)
    {
        std::ignore = window;

        ImGuiIO& io = ImGui::GetIO();
        io.AddMouseWheelEvent(wheel.x, wheel.y);
    }

    void KeyCallback(const Window& window, Input::Key keycode, int32_t scancode, Input::KeyAction action, Input::ModifierFlag modFlags)
    {
        auto& bd = GetBackendData();

        if (action != Input::KeyAction::Press &&
            action != Input::KeyAction::Release)
            return;

        UpdateKeyModifiers(modFlags);
        bd.KeyOwnerWindows[size_t(keycode)] = (action == Input::KeyAction::Press) ? &window : nullptr;

        ImGuiIO& io = ImGui::GetIO();
        ImGuiKey imgui_key = KeyToImGuiKey(keycode);
        io.AddKeyEvent(imgui_key, action == Input::KeyAction::Press);
        io.SetKeyEventNativeData(imgui_key, static_cast<int>(keycode), scancode); // To support legacy indexing (<1.87 user code)
    }

    void WindowFocusCallback(const Window& window, bool focused)
    {
        std::ignore = window;

        ImGuiIO& io = ImGui::GetIO();
        io.AddFocusEvent(focused);
    }

    void ContentScaleChangeCallback(const Window& window, const Vector2& scale)
    {
        std::ignore = window;
        std::ignore = scale;

        auto& bd = GetBackendData();
        bd.WantUpdateMonitors = true;
    }

    void MouseMoveCallback(const Window& window, const Vector2i& position)
    {
        ImGuiIO& io = ImGui::GetIO();

        Vector2i mousePosition = position;

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
            mousePosition += window.GetPosition();

        const ImVec2& fmousePosition = Vector2iToImVec(mousePosition);

        io.AddMousePosEvent(fmousePosition.x, fmousePosition.y);
    }

    void CharCallback(const Window& window, U8Glyph ch)
    {
        std::ignore = window;

        ImGuiIO& io = ImGui::GetIO();
        io.AddInputCharacter(ch);
    }

    void MonitorConfigChangedCallback(const Monitor& monitor, Monitor::Event event)
    {
        std::ignore = monitor;
        std::ignore = event;

        auto& bd = GetBackendData();
        bd.WantUpdateMonitors = true;
    }

    void InstallCallbacks(const std::shared_ptr<Window>& window)
    {
        auto& bd = GetBackendData();
        ASSERT(bd.InstalledCallbacks == false && "Callbacks already installed!");
        ASSERT(bd.MainWindow == window);

        window->OnContentScaleChange.Subscribe<ContentScaleChangeCallback>();
        window->OnFocus.Subscribe<WindowFocusCallback>();
        window->OnKey.Subscribe<KeyCallback>();
        window->OnMouseButton.Subscribe<MouseButtonCallback>();
        window->OnMouseMove.Subscribe<MouseMoveCallback>();
        window->OnScroll.Subscribe<ScrollCallback>();
        window->OnChar.Subscribe<CharCallback>();
        Toolkit::Instance().OnMonitorConfigChanged.Subscribe<MonitorConfigChangedCallback>();

        bd.InstalledCallbacks = true;
    }

    void RestoreCallbacks(const std::shared_ptr<Window>& window)
    {
        auto& bd = GetBackendData();
        ASSERT(bd.InstalledCallbacks == true && "Callbacks not installed!");
        ASSERT(bd.MainWindow == window);

        window->OnContentScaleChange.Unsubscribe<ContentScaleChangeCallback>();
        window->OnFocus.Unsubscribe<WindowFocusCallback>();
        window->OnKey.Unsubscribe<KeyCallback>();
        window->OnMouseButton.Unsubscribe<MouseButtonCallback>();
        window->OnMouseMove.Unsubscribe<MouseMoveCallback>();
        window->OnScroll.Unsubscribe<ScrollCallback>();
        window->OnChar.Unsubscribe<CharCallback>();
        Toolkit::Instance().OnMonitorConfigChanged.Unsubscribe<MonitorConfigChangedCallback>();
        bd.InstalledCallbacks = false;
    }

    void UpdateMouseData()
    {
        ImGuiIO& io = ImGui::GetIO();
        ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();

        ImGuiID mouseViewportId = 0;
        const ImVec2& mousePos = io.MousePos;
        for (int index = 0; index < platform_io.Viewports.Size; index++)
        {
            ImGuiViewport* viewport = platform_io.Viewports[index];
            auto window = (Platform::Window*)viewport->PlatformHandle;

            const bool isFocused = window->GetWindowAttribute(Window::Attribute::Focused) != 0;
            if (isFocused)
            {
                // (Optional) Set OS mouse position from Dear ImGui if requested (rarely used, only when ImGuiConfigFlags_NavEnableSetMousePos is enabled by user)
                // When multi-viewports are enabled, all Dear ImGui positions are same as OS positions.
                if (io.WantSetMousePos)
                    window->SetMousePosition(Vector2(mousePos.x - viewport->Pos.x,
                                                     mousePos.y - viewport->Pos.y)
                                                 .Cast<int32_t>());
            }

            // (Optional) When using multiple viewports: call io.AddMouseViewportEvent() with the viewport the OS mouse cursor is hovering.
            const bool windowNoInput = (viewport->Flags & ImGuiViewportFlags_NoInputs) != 0;

            window->SetWindowAttribute(Window::Attribute::MousePassthrough, windowNoInput);
            if (window->GetWindowAttribute(Window::Attribute::Hovered) && !windowNoInput)
                mouseViewportId = viewport->ID;
        }

        if (io.BackendFlags & ImGuiBackendFlags_HasMouseHoveredViewport)
            io.AddMouseViewportEvent(mouseViewportId);
    }

    void UpdateMouseCursor()
    {
        ImGuiIO& io = ImGui::GetIO();
        const auto& bd = GetBackendData();

        if ((io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange) ||
            (static_cast<Window::CursorState>(bd.MainWindow->GetWindowAttribute(Window::Attribute::Cursor)) == Window::CursorState::Disabled))
            return;

        ImGuiMouseCursor imGuiCursor = ImGui::GetMouseCursor();
        ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
        for (int index = 0; index < platform_io.Viewports.Size; index++)
        {
            auto window = (Platform::Window*)platform_io.Viewports[index]->PlatformHandle;

            if (imGuiCursor == ImGuiMouseCursor_None || io.MouseDrawCursor)
            {
                // Hide OS mouse cursor if imgui is drawing it or if it wants no cursor
                window->SetWindowAttribute(Window::Attribute::Cursor, static_cast<int32_t>(Window::CursorState::Hidden));
            }
            else
            {
                window->SetCursor(bd.MouseCursors[imGuiCursor] ? bd.MouseCursors[imGuiCursor] : bd.MouseCursors[ImGuiMouseCursor_Arrow]);
                window->SetWindowAttribute(Window::Attribute::Cursor, static_cast<int32_t>(Window::CursorState::Normal));
            }
        }
    }

    void UpdateMonitors()
    {
        auto& bd = GetBackendData();
        ImGuiPlatformIO& platformIo = ImGui::GetPlatformIO();

        const auto& toolkit = Toolkit::Instance();
        const auto& monitors = toolkit.GetMonitors();
        for (const auto& monitor : monitors)
        {
            ImGuiPlatformMonitor imGuiMonitor;

            imGuiMonitor.MainPos = Vector2iToImVec(monitor.position);
            imGuiMonitor.MainSize = Vector2iToImVec(monitor.videoMode.resolution);

            imGuiMonitor.WorkPos = Vector2iToImVec(monitor.workArea.GetPosition());
            imGuiMonitor.WorkSize = Vector2iToImVec(monitor.workArea.GetSize());

            // Warning: the validity of monitor DPI information on Windows depends on the application DPI awareness settings, which generally needs to be set in the manifest or at runtime.
            imGuiMonitor.DpiScale = monitor.dpiScale.x;

            platformIo.Monitors.push_back(imGuiMonitor);
        }
        bd.WantUpdateMonitors = false;
    }

    //--------------------------------------------------------------------------------------------------------
    // MULTI-VIEWPORT / PLATFORM INTERFACE SUPPORT
    // This is an _advanced_ and _optional_ feature, allowing the backend to create and handle multiple viewports simultaneously.
    // If you are new to dear imgui or creating a new binding for dear imgui, it is recommended that you completely ignore this section first..
    //--------------------------------------------------------------------------------------------------------

    const char* Platform_GetClipboardText(void* user_data)
    {
        std::ignore = user_data;

        const auto& bd = GetBackendData();
        return bd.MainWindow->GetClipboardText();
    }

    void Platform_SetClipboardText(void* user_data, const char* text)
    {
        std::ignore = user_data;

        const auto& bd = GetBackendData();
        return bd.MainWindow->SetClipboardText(text);
    }

    void Platform_CreateWindow(ImGuiViewport* viewport)
    {
        ImGuiViewportData* vd = IM_NEW(ImGuiViewportData)();
        viewport->PlatformUserData = vd;

        auto& toolkit = Toolkit::Instance();

        Window::Description windowDesc;
        windowDesc.size = ImVecToVector2i(viewport->Size);
        windowDesc.title = "No Title Yet";
        windowDesc.visible = false;
        windowDesc.focused = false;
        windowDesc.focusOnShow = false;
        windowDesc.decorated = (viewport->Flags & ImGuiViewportFlags_NoDecoration) ? false : true;
        windowDesc.floating = (viewport->Flags & ImGuiViewportFlags_TopMost) ? true : false;

        vd->Window = toolkit.CreatePlatformWindow(windowDesc);
        ASSERT(vd->Window);

        vd->WindowOwned = true;
        viewport->PlatformHandle = (void*)vd->Window.get();
#ifdef _WIN32
        viewport->PlatformHandleRaw = std::any_cast<HWND>(vd->Window->GetNativeHandleRaw());
#endif
        vd->Window->SetPosition(ImVecToVector2i(viewport->Pos));

        // Install callbacks for secondary viewports
        vd->Window->OnChar.Subscribe<CharCallback>();
        vd->Window->OnClose.Subscribe<WindowCloseCallback>();
        vd->Window->OnContentScaleChange.Subscribe<ContentScaleChangeCallback>();
        vd->Window->OnFocus.Subscribe<WindowFocusCallback>();
        vd->Window->OnKey.Subscribe<KeyCallback>();
        vd->Window->OnMouseButton.Subscribe<MouseButtonCallback>();
        vd->Window->OnMouseCross.Subscribe<MouseCrossCallback>();
        vd->Window->OnMouseMove.Subscribe<MouseMoveCallback>();
        vd->Window->OnMove.Subscribe<WindowPosCallback>();
        vd->Window->OnResize.Subscribe<WindowSizeCallback>();
        vd->Window->OnScroll.Subscribe<ScrollCallback>();
    }

    void Platform_DestroyWindow(ImGuiViewport* viewport)
    {
        const auto& bd = GetBackendData();

        if (ImGuiViewportData* vd = (ImGuiViewportData*)viewport->PlatformUserData)
        {
            ASSERT(vd->Window);
            const auto* window = vd->Window.get();

            if (vd->WindowOwned)
            {
                // Release any keys that were pressed in the window being destroyed and are still held down,
                // because we will not receive any release events after window is destroyed.
                for (size_t keyIndex = 0; keyIndex < bd.KeyOwnerWindows.size(); keyIndex++)
                    if (bd.KeyOwnerWindows[keyIndex] == window)
                        // Later params are only used for main viewport, on which this function is never called.
                        KeyCallback(*window, Input::Key(keyIndex), 0, Input::KeyAction::Release, Input::ModifierFlag::None);
            }
            vd->Window = nullptr;
            IM_DELETE(vd);
        }
        viewport->PlatformUserData = viewport->PlatformHandle = nullptr;
    }

    void Platform_ShowWindow(ImGuiViewport* viewport)
    {
        ASSERT(viewport->PlatformUserData);

        ImGuiViewportData* vd = (ImGuiViewportData*)viewport->PlatformUserData;

        vd->Window->SetWindowAttribute(Window::Attribute::TaskbarIcon, !(viewport->Flags & ImGuiViewportFlags_NoTaskBarIcon));
        vd->Window->SetWindowAttribute(Window::Attribute::MousePassthrough, true);
        vd->Window->Show();
    }

    ImVec2 Platform_GetWindowPos(ImGuiViewport* viewport)
    {
        ASSERT(viewport->PlatformUserData);

        ImGuiViewportData* vd = (ImGuiViewportData*)viewport->PlatformUserData;
        const auto& position = vd->Window->GetPosition();

        return ImVec2(static_cast<float>(position.x),
                      static_cast<float>(position.y));
    }

    void Platform_SetWindowPos(ImGuiViewport* viewport, ImVec2 pos)
    {
        ASSERT(viewport->PlatformUserData);

        ImGuiViewportData* vd = (ImGuiViewportData*)viewport->PlatformUserData;
        vd->IgnoreWindowPosEventFrame = ImGui::GetFrameCount();
        vd->Window->SetPosition(Vector2i(static_cast<int32_t>(pos.x),
                                         static_cast<int32_t>(pos.y)));
    }

    ImVec2 Platform_GetWindowSize(ImGuiViewport* viewport)
    {
        ASSERT(viewport->PlatformUserData);

        ImGuiViewportData* vd = (ImGuiViewportData*)viewport->PlatformUserData;
        return Vector2iToImVec(vd->Window->GetSize());
    }

    void Platform_SetWindowSize(ImGuiViewport* viewport, ImVec2 size)
    {
        ASSERT(viewport->PlatformUserData);

        ImGuiViewportData* vd = (ImGuiViewportData*)viewport->PlatformUserData;
        vd->Window->SetSize(ImVecToVector2i(size));
        vd->IgnoreWindowSizeEventFrame = ImGui::GetFrameCount();
    }

    void Platform_SetWindowTitle(ImGuiViewport* viewport, const char* title)
    {
        ASSERT(viewport->PlatformUserData);

        ImGuiViewportData* vd = (ImGuiViewportData*)viewport->PlatformUserData;
        vd->Window->SetTitle(title);
    }

    void Platform_SetWindowFocus(ImGuiViewport* viewport)
    {
        ASSERT(viewport->PlatformUserData);

        ImGuiViewportData* vd = (ImGuiViewportData*)viewport->PlatformUserData;
        vd->Window->Focus();
    }

    bool Platform_GetWindowFocus(ImGuiViewport* viewport)
    {
        ASSERT(viewport->PlatformUserData);

        ImGuiViewportData* vd = (ImGuiViewportData*)viewport->PlatformUserData;
        return vd->Window->GetWindowAttribute(Window::Attribute::Focused) != 0;
    }

    bool Platform_GetWindowMinimized(ImGuiViewport* viewport)
    {
        ASSERT(viewport->PlatformUserData);

        ImGuiViewportData* vd = (ImGuiViewportData*)viewport->PlatformUserData;
        return vd->Window->GetWindowAttribute(Window::Attribute::Minimized) != 0;
    }

    void Platform_SetWindowAlpha(ImGuiViewport* viewport, float alpha)
    {
        ASSERT(viewport->PlatformUserData);

        ImGuiViewportData* vd = (ImGuiViewportData*)viewport->PlatformUserData;
        vd->Window->SetWindowAlpha(alpha);
    }

    // Forward Declarations
    void ImGuiInitPlatformInterface();
    void ImGuiShutdownPlatformInterface();

    bool ImguiPlatfomImpl::Init(const std::shared_ptr<Window>& window, bool installCallbacks)
    {
        ImGuiIO& io = ImGui::GetIO();
        ASSERT(io.BackendPlatformUserData == nullptr && "Already initialized a platform backend!");

        // Setup backend capabilities flags
        ImGuiData* bd = IM_NEW(ImGuiData)();
        io.BackendPlatformUserData = (void*)bd;
        io.BackendPlatformName = "imgui_impl_redrosemary";
        io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors; // We can honor GetMouseCursor() values (optional)
        io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos; // We can honor io.WantSetMousePos requests (optional, rarely used)
        io.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports; // We can create multi-viewports on the Platform side (optional)
        io.BackendFlags |= ImGuiBackendFlags_HasMouseHoveredViewport; // We can call io.AddMouseViewportEvent() with correct data (optional)

        bd->MainWindow = window;
        bd->WantUpdateMonitors = true;

        io.SetClipboardTextFn = Platform_SetClipboardText;
        io.GetClipboardTextFn = Platform_GetClipboardText;
        io.ClipboardUserData = nullptr;

        const auto& toolkit = Toolkit::Instance();

        bd->MouseCursors[ImGuiMouseCursor_Arrow] = toolkit.CreateCursor(Cursor::Type::Arrow);
        bd->MouseCursors[ImGuiMouseCursor_TextInput] = toolkit.CreateCursor(Cursor::Type::IBeam);
        bd->MouseCursors[ImGuiMouseCursor_ResizeNS] = toolkit.CreateCursor(Cursor::Type::VResize);
        bd->MouseCursors[ImGuiMouseCursor_ResizeEW] = toolkit.CreateCursor(Cursor::Type::HResize);
        bd->MouseCursors[ImGuiMouseCursor_Hand] = toolkit.CreateCursor(Cursor::Type::Hand);
#if 0
        bd->MouseCursors[ImGuiMouseCursor_ResizeAll] = toolkit.CreateCursor(Cursor::Type::Count);
        bd->MouseCursors[ImGuiMouseCursor_ResizeNESW] = toolkit.CreateCursor(Cursor::Type::Count);
        bd->MouseCursors[ImGuiMouseCursor_ResizeNWSE] = toolkit.CreateCursor(Cursor::Type::Count);
        bd->MouseCursors[ImGuiMouseCursor_NotAllowed] = toolkit.CreateCursor(Cursor::Type::Count);
#else
        bd->MouseCursors[ImGuiMouseCursor_ResizeAll] = toolkit.CreateCursor(Cursor::Type::Arrow);
        bd->MouseCursors[ImGuiMouseCursor_ResizeNESW] = toolkit.CreateCursor(Cursor::Type::Arrow);
        bd->MouseCursors[ImGuiMouseCursor_ResizeNWSE] = toolkit.CreateCursor(Cursor::Type::Arrow);
        bd->MouseCursors[ImGuiMouseCursor_NotAllowed] = toolkit.CreateCursor(Cursor::Type::Arrow);
#endif

        if (installCallbacks)
            InstallCallbacks(window);

        // Update monitors the first time
        UpdateMonitors();

        // Our mouse update function expect PlatformHandle to be filled for the main viewport
        ImGuiViewport* main_viewport = ImGui::GetMainViewport();
        main_viewport->PlatformHandle = (void*)bd->MainWindow.get();
#ifdef _WIN32
        main_viewport->PlatformHandleRaw = std::any_cast<HWND>(bd->MainWindow->GetNativeHandleRaw());
#endif
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
            ImGuiInitPlatformInterface();

        return true;
    }

    void ImguiPlatfomImpl::NewFrame(float dt)
    {
        ImGuiIO& io = ImGui::GetIO();
        const auto& bd = GetBackendData();

        // Setup display size (every frame to accommodate for window resizing)
        auto windowSize = bd.MainWindow->GetSize();
        auto framebuffer = bd.MainWindow->GetFramebufferSize();
        io.DisplaySize = Vector2iToImVec(windowSize);

        if (windowSize.x > 0 && windowSize.y > 0)
            io.DisplayFramebufferScale = ImVec2((float)framebuffer.x / (float)windowSize.x,
                                                (float)framebuffer.y / (float)windowSize.y);
        if (bd.WantUpdateMonitors)
            UpdateMonitors();

        io.DeltaTime = dt;

        UpdateMouseData();
        UpdateMouseCursor();

        ImGui::NewFrame();
    }

    void ImguiPlatfomImpl::Shutdown()
    {
        auto bd = GetBackendDataPtr();
        ASSERT(bd != nullptr && "No platform backend to shutdown, or already shutdown?");
        ImGuiIO& io = ImGui::GetIO();

        ImGuiShutdownPlatformInterface();

        if (bd->InstalledCallbacks)
            RestoreCallbacks(bd->MainWindow);

        io.BackendPlatformName = nullptr;
        io.BackendPlatformUserData = nullptr;
        IM_DELETE(bd);
    }

    void ImGuiInitPlatformInterface()
    {
        // Register platform interface (will be coupled with a renderer interface)
        const auto& bd = GetBackendData();
        ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
        platform_io.Platform_CreateWindow = Platform_CreateWindow;
        platform_io.Platform_DestroyWindow = Platform_DestroyWindow;
        platform_io.Platform_ShowWindow = Platform_ShowWindow;
        platform_io.Platform_SetWindowPos = Platform_SetWindowPos;
        platform_io.Platform_GetWindowPos = Platform_GetWindowPos;
        platform_io.Platform_SetWindowSize = Platform_SetWindowSize;
        platform_io.Platform_GetWindowSize = Platform_GetWindowSize;
        platform_io.Platform_SetWindowFocus = Platform_SetWindowFocus;
        platform_io.Platform_GetWindowFocus = Platform_GetWindowFocus;
        platform_io.Platform_GetWindowMinimized = Platform_GetWindowMinimized;
        platform_io.Platform_SetWindowTitle = Platform_SetWindowTitle;
        platform_io.Platform_SetWindowAlpha = Platform_SetWindowAlpha;
        // platform_io.Platform_CreateVkSurface = Platform_CreateVkSurface; TODO: Vulkan support

        // Register main window handle (which is owned by the main application, not by us)
        // This is mostly for simplicity and consistency, so that our code (e.g. mouse handling etc.) can use same logic for main and secondary viewports.
        ImGuiViewport* mainViewport = ImGui::GetMainViewport();
        ImGuiViewportData* vd = IM_NEW(ImGuiViewportData)();
        vd->Window = bd.MainWindow;
        vd->WindowOwned = false;
        mainViewport->PlatformUserData = vd;
        mainViewport->PlatformHandle = (void*)bd.MainWindow.get();
    }

    void ImGuiShutdownPlatformInterface()
    {
        ImGui::DestroyPlatformWindows();
    }
}