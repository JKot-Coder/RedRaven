#include "ImguiPlatformImplementation.hpp"
#include "imgui.h"

#include "platform/Input.hpp"
#include "platform/Toolkit.hpp"

// GLFW
#include <GLFW/glfw3.h>
#ifdef _WIN32
#undef APIENTRY
#include "windows.h" // for HWND
#endif

#ifdef GLFW_RESIZE_NESW_CURSOR // Let's be nice to people who pulled GLFW between 2019-04-16 (3.4 define) and 2019-11-29 (cursors defines) // FIXME: Remove when GLFW 3.4 is released?
#define GLFW_HAS_NEW_CURSORS (GLFW_VERSION_MAJOR * 1000 + GLFW_VERSION_MINOR * 100 >= 3400) // 3.4+ GLFW_RESIZE_ALL_CURSOR, GLFW_RESIZE_NESW_CURSOR, GLFW_RESIZE_NWSE_CURSOR, GLFW_NOT_ALLOWED_CURSOR
#else
#define GLFW_HAS_NEW_CURSORS (0)
#endif
#define GLFW_HAS_GAMEPAD_API (GLFW_VERSION_MAJOR * 1000 + GLFW_VERSION_MINOR * 100 >= 3300) // 3.3+ glfwGetGamepadState() new api
#define GLFW_HAS_GET_KEY_NAME (GLFW_VERSION_MAJOR * 1000 + GLFW_VERSION_MINOR * 100 >= 3200) // 3.2+ glfwGetKeyName()

namespace RR
{
    using namespace Platform;

    // GLFW data
    enum GlfwClientApi
    {
        GlfwClientApi_Unknown,
        GlfwClientApi_OpenGL,
        GlfwClientApi_Vulkan
    };

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
        std::shared_ptr<Window> Window;
        GlfwClientApi ClientApi;
        double Time;
        GLFWcursor* MouseCursors[ImGuiMouseCursor_COUNT];
        ImVec2 LastValidMousePos;
        std::array<Platform::Window*, size_t(Input::Key::Count)> KeyOwnerWindows;
        bool InstalledCallbacks;
        bool WantUpdateMonitors;
        ImGuiData() { memset((void*)this, 0, sizeof(*this)); }
    };

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
    }

    // Backend data stored in io.BackendPlatformUserData to allow support for multiple Dear ImGui contexts
    // It is STRONGLY preferred that you use docking branch with multi-viewports (== single Dear ImGui context + multiple windows) instead of multiple Dear ImGui contexts.
    // FIXME: multi-context support is not well tested and probably dysfunctional in this backend.
    // - Because glfwPollEvents() process all windows and some events may be called outside of it, you will need to register your own callbacks
    //   (passing install_callbacks=false in ImGui_ImplGlfw_InitXXX functions), set the current dear imgui context and then call our callbacks.
    // - Otherwise we may need to store a GLFWWindow* -> ImGuiContext* map and handle this in the backend, adding a little bit of extra complexity to it.
    // FIXME: some shared resources (mouse cursor shape, gamepad) are mishandled when using multi-context.
    static ImGuiData* ImGuiGetBackendData()
    {
        return ImGui::GetCurrentContext() ? (ImGuiData*)ImGui::GetIO().BackendPlatformUserData : nullptr;
    }

    // Forward Declarations
    static void UpdateMonitors();
    static void ImGuiInitPlatformInterface();
    static void ImGuiShutdownPlatformInterface();

    // Functions
    static const char* Platform_GetClipboardText(void* user_data)
    {
        std::ignore = user_data;

        ImGuiData* bd = ImGuiGetBackendData();
        ASSERT(bd);

        return bd->Window->GetClipboardText().c_str();
    }

    static void Platform_SetClipboardText(void* user_data, const char* text)
    {
        std::ignore = user_data;

        ImGuiData* bd = ImGuiGetBackendData();
        ASSERT(bd);

        return bd->Window->SetClipboardText(text);
    }

    static ImGuiKey KeyToImGuiKey(Input::Key key)
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
    /*
    static void ImGui_ImplGlfw_UpdateKeyModifiers(int mods)
    {
        ImGuiIO& io = ImGui::GetIO();
        io.AddKeyEvent(ImGuiKey_ModCtrl, (mods & GLFW_MOD_CONTROL) != 0);
        io.AddKeyEvent(ImGuiKey_ModShift, (mods & GLFW_MOD_SHIFT) != 0);
        io.AddKeyEvent(ImGuiKey_ModAlt, (mods & GLFW_MOD_ALT) != 0);
        io.AddKeyEvent(ImGuiKey_ModSuper, (mods & GLFW_MOD_SUPER) != 0);
    }*/

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
        std::ignore = modFlags;

        // TODO
        // ImGui_ImplGlfw_UpdateKeyModifiers(mods);

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

    void KeyCallback(const Window& window, Input::Key keycode, int32_t scancode, Input::KeyAction action, Input::ModifierFlag modifier)
    {
        ImGuiData* bd = ImGuiGetBackendData();

        if (action != Input::KeyAction::Press &&
            action != Input::KeyAction::Release)
            return;

        std::ignore = bd;
        std::ignore = window;
        std::ignore = modifier;

        //ImGui_ImplGlfw_UpdateKeyModifiers(mods);

        //        if (keycode >= 0 && keycode < IM_ARRAYSIZE(bd->KeyOwnerWindows))
        //          bd->KeyOwnerWindows[keycode] = (action == Input::KeyAction::Press) ? window : nullptr;

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

    void ContentScaleChange(const Window& window, const Vector2& scale)
    {
        std::ignore = window;
        std::ignore = scale;

        ImGuiData* bd = ImGuiGetBackendData();
        bd->WantUpdateMonitors = true;
    }

    void MouseMoveCallback(const Window& window, const Vector2i& position)
    {
        ImGuiData* bd = ImGuiGetBackendData();
        ImGuiIO& io = ImGui::GetIO();

        Vector2i mousePosition = position;

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
            mousePosition += window.GetPosition();

        const ImVec2& fmousePosition = Vector2iToImVec(mousePosition);

        io.AddMousePosEvent(fmousePosition.x, fmousePosition.y);
        bd->LastValidMousePos = fmousePosition;
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

        ImGuiData* bd = ImGuiGetBackendData();
        bd->WantUpdateMonitors = true;
    }

    void ImGui_ImplGlfw_InstallCallbacks(const std::shared_ptr<Window>& window)
    {
        ImGuiData* bd = ImGuiGetBackendData();
        ASSERT(bd->InstalledCallbacks == false && "Callbacks already installed!");
        ASSERT(bd->Window == window);

        window->OnContentScaleChange.Subscribe<ContentScaleChange>();
        window->OnFocus.Subscribe<WindowFocusCallback>();
        window->OnKey.Subscribe<KeyCallback>();
        window->OnMouseButton.Subscribe<MouseButtonCallback>();
        window->OnMouseMove.Subscribe<MouseMoveCallback>();
        window->OnScroll.Subscribe<ScrollCallback>();
        window->OnChar.Subscribe<CharCallback>();
        Toolkit::Instance().OnMonitorConfigChanged.Subscribe<MonitorConfigChangedCallback>();

        bd->InstalledCallbacks = true;
    }

    void ImGui_ImplGlfw_RestoreCallbacks(const std::shared_ptr<Window>& window)
    {
        ImGuiData* bd = ImGuiGetBackendData();
        ASSERT(bd->InstalledCallbacks == true && "Callbacks not installed!");
        ASSERT(bd->Window == window);

        window->OnContentScaleChange.Unsubscribe<ContentScaleChange>();
        window->OnFocus.Unsubscribe<WindowFocusCallback>();
        window->OnKey.Unsubscribe<KeyCallback>();
        window->OnMouseButton.Unsubscribe<MouseButtonCallback>();
        window->OnMouseMove.Unsubscribe<MouseMoveCallback>();
        window->OnScroll.Unsubscribe<ScrollCallback>();
        window->OnChar.Unsubscribe<CharCallback>();
        Toolkit::Instance().OnMonitorConfigChanged.Unsubscribe<MonitorConfigChangedCallback>();
        bd->InstalledCallbacks = false;
    }

    static bool ImGui_ImplRR_Init(const std::shared_ptr<Window>& window, bool install_callbacks, GlfwClientApi client_api)
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

        bd->Window = window;
        bd->Time = 0.0;
        bd->WantUpdateMonitors = true;

        io.SetClipboardTextFn = Platform_SetClipboardText;
        io.GetClipboardTextFn = Platform_GetClipboardText;
        io.ClipboardUserData = nullptr;

        // Create mouse cursors
        // (By design, on X11 cursors are user configurable and some cursors may be missing. When a cursor doesn't exist,
        // GLFW will emit an error which will often be printed by the app, so we temporarily disable error reporting.
        // Missing cursors will return nullptr and our _UpdateMouseCursor() function will use the Arrow cursor instead.)
        GLFWerrorfun prev_error_callback = glfwSetErrorCallback(nullptr);
        bd->MouseCursors[ImGuiMouseCursor_Arrow] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
        bd->MouseCursors[ImGuiMouseCursor_TextInput] = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
        bd->MouseCursors[ImGuiMouseCursor_ResizeNS] = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
        bd->MouseCursors[ImGuiMouseCursor_ResizeEW] = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
        bd->MouseCursors[ImGuiMouseCursor_Hand] = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
#if GLFW_HAS_NEW_CURSORS
        bd->MouseCursors[ImGuiMouseCursor_ResizeAll] = glfwCreateStandardCursor(GLFW_RESIZE_ALL_CURSOR);
        bd->MouseCursors[ImGuiMouseCursor_ResizeNESW] = glfwCreateStandardCursor(GLFW_RESIZE_NESW_CURSOR);
        bd->MouseCursors[ImGuiMouseCursor_ResizeNWSE] = glfwCreateStandardCursor(GLFW_RESIZE_NWSE_CURSOR);
        bd->MouseCursors[ImGuiMouseCursor_NotAllowed] = glfwCreateStandardCursor(GLFW_NOT_ALLOWED_CURSOR);
#else
        bd->MouseCursors[ImGuiMouseCursor_ResizeAll] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
        bd->MouseCursors[ImGuiMouseCursor_ResizeNESW] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
        bd->MouseCursors[ImGuiMouseCursor_ResizeNWSE] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
        bd->MouseCursors[ImGuiMouseCursor_NotAllowed] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
#endif
        glfwSetErrorCallback(prev_error_callback);

        // Chain GLFW callbacks: our callbacks will call the user's previously installed callbacks, if any.
        if (install_callbacks)
            ImGui_ImplGlfw_InstallCallbacks(window);

        // Update monitors the first time (note: monitor callback are broken in GLFW 3.2 and earlier, see github.com/glfw/glfw/issues/784)
        UpdateMonitors();

        // Our mouse update function expect PlatformHandle to be filled for the main viewport
        ImGuiViewport* main_viewport = ImGui::GetMainViewport();
        main_viewport->PlatformHandle = (void*)bd->Window.get();
#ifdef _WIN32
        main_viewport->PlatformHandleRaw = std::any_cast<HWND>(bd->Window->GetNativeHandleRaw());
#endif
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
            ImGuiInitPlatformInterface();

        bd->ClientApi = client_api;
        return true;
    }

    bool ImGui_ImplRR_InitForOpenGL(const std::shared_ptr<Window>& window, bool install_callbacks)
    {
        return ImGui_ImplRR_Init(window, install_callbacks, GlfwClientApi_OpenGL);
    }

    bool ImGui_ImplRR_InitForVulkan(const std::shared_ptr<Window>& window, bool install_callbacks)
    {
        return ImGui_ImplRR_Init(window, install_callbacks, GlfwClientApi_Vulkan);
    }

    bool ImGui_ImplRR_InitForOther(const std::shared_ptr<Window>& window, bool install_callbacks)
    {
        return ImGui_ImplRR_Init(window, install_callbacks, GlfwClientApi_Unknown);
    }

    void ImGui_ImplGlfw_Shutdown()
    {
        ImGuiData* bd = ImGuiGetBackendData();
        ASSERT(bd != nullptr && "No platform backend to shutdown, or already shutdown?");
        ImGuiIO& io = ImGui::GetIO();

        ImGuiShutdownPlatformInterface();

        if (bd->InstalledCallbacks)
            ImGui_ImplGlfw_RestoreCallbacks(bd->Window);

        for (ImGuiMouseCursor cursor_n = 0; cursor_n < ImGuiMouseCursor_COUNT; cursor_n++)
            glfwDestroyCursor(bd->MouseCursors[cursor_n]);

        io.BackendPlatformName = nullptr;
        io.BackendPlatformUserData = nullptr;
        IM_DELETE(bd);
    }

    static void UpdateMouseData()
    {
        // ImGui_ImplGlfw_Data* bd = ImGui_ImplGlfw_GetBackendData();
        ImGuiIO& io = ImGui::GetIO();
        ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();

        ImGuiID mouseViewportId = 0;
        const ImVec2& mousePos = io.MousePos;
        for (int index = 0; index < platform_io.Viewports.Size; index++)
        {
            ImGuiViewport* viewport = platform_io.Viewports[index];
            ImGuiViewportData* vd = (ImGuiViewportData*)viewport->PlatformUserData;
            auto window = vd->Window;
            // auto glfwWindow = vd->GLFWWindow;

            const bool isFocused = window->GetWindowAttribute(Window::Attribute::Focused) != 0;
            if (isFocused)
            {
                // (Optional) Set OS mouse position from Dear ImGui if requested (rarely used, only when ImGuiConfigFlags_NavEnableSetMousePos is enabled by user)
                // When multi-viewports are enabled, all Dear ImGui positions are same as OS positions.
                if (io.WantSetMousePos)
                    window->SetMousePosition(Vector2(mousePos.x - viewport->Pos.x,
                                                     mousePos.y - viewport->Pos.y)
                                                 .Cast<int32_t>());
                /*
                // (Optional) Fallback to provide mouse position when focused (ImGui_ImplGlfw_CursorPosCallback already provides this when hovered or captured)
                if (bd->MouseWindow == NULL)
                {
                    double mouse_x, mouse_y;
                    glfwGetCursorPos(glfwWindow, &mouse_x, &mouse_y);
                    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
                    {
                        // Single viewport mode: mouse position in client window coordinates (io.MousePos is (0,0) when the mouse is on the upper-left corner of the app window)
                        // Multi-viewport mode: mouse position in OS absolute coordinates (io.MousePos is (0,0) when the mouse is on the upper-left of the primary monitor)
                        int window_x, window_y;
                        glfwGetWindowPos(glfwWindow, &window_x, &window_y);
                        mouse_x += window_x;
                        mouse_y += window_y;
                    }
                    bd->LastValidMousePos = ImVec2((float)mouse_x, (float)mouse_y);
                    io.AddMousePosEvent((float)mouse_x, (float)mouse_y);
                }*/
            }

            // (Optional) When using multiple viewports: call io.AddMouseViewportEvent() with the viewport the OS mouse cursor is hovering.
            const bool windowNoInput = (viewport->Flags & ImGuiViewportFlags_NoInputs) != 0;

            vd->Window->SetWindowAttribute(Window::Attribute::MousePassthrough, windowNoInput);
            if (vd->Window->GetWindowAttribute(Window::Attribute::Hovered) && !windowNoInput)
                mouseViewportId = viewport->ID;
        }

        if (io.BackendFlags & ImGuiBackendFlags_HasMouseHoveredViewport)
            io.AddMouseViewportEvent(mouseViewportId);
    }

    static void ImGui_ImplGlfw_UpdateMouseCursor()
    {
        ImGuiIO& io = ImGui::GetIO();
        ImGuiData* bd = ImGuiGetBackendData();
        if ((io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange) ||
            (static_cast<Window::Cursor>(bd->Window->GetWindowAttribute(Window::Attribute::Cursor)) == Window::Cursor::Disabled))
            return;

        ImGuiMouseCursor imGuiCursor = ImGui::GetMouseCursor();
        ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
        for (int index = 0; index < platform_io.Viewports.Size; index++)
        {
            ImGuiViewportData* vd = (ImGuiViewportData*)platform_io.Viewports[index]->PlatformUserData;
            auto window = vd->Window;
            const auto glfwWindow = std::any_cast<GLFWwindow*>(window->GetNativeHandle());

            if (imGuiCursor == ImGuiMouseCursor_None || io.MouseDrawCursor)
            {
                // Hide OS mouse cursor if imgui is drawing it or if it wants no cursor
                window->SetWindowAttribute(Window::Attribute::Cursor, static_cast<int32_t>(Window::Cursor::Hidden));
            }
            else
            {
                glfwSetCursor(glfwWindow, bd->MouseCursors[imGuiCursor] ? bd->MouseCursors[imGuiCursor] : bd->MouseCursors[ImGuiMouseCursor_Arrow]);
                window->SetWindowAttribute(Window::Attribute::Cursor, static_cast<int32_t>(Window::Cursor::Normal));
            }
        }
    }

    // Update gamepad inputs
    static inline float Saturate(float v) { return v < 0.0f ? 0.0f : v > 1.0f ? 1.0f
                                                                              : v; }
    static void ImGui_ImplGlfw_UpdateGamepads()
    {
        ImGuiIO& io = ImGui::GetIO();
        if ((io.ConfigFlags & ImGuiConfigFlags_NavEnableGamepad) == 0)
            return;

        io.BackendFlags &= ~ImGuiBackendFlags_HasGamepad;
#if GLFW_HAS_GAMEPAD_API
        GLFWgamepadstate gamepad;
        if (!glfwGetGamepadState(GLFW_JOYSTICK_1, &gamepad))
            return;
#define MAP_BUTTON(KEY_NO, BUTTON_NO, _UNUSED)                   \
    do                                                           \
    {                                                            \
        io.AddKeyEvent(KEY_NO, gamepad.buttons[BUTTON_NO] != 0); \
    } while (0)
#define MAP_ANALOG(KEY_NO, AXIS_NO, _UNUSED, V0, V1)          \
    do                                                        \
    {                                                         \
        float v = gamepad.axes[AXIS_NO];                      \
        v = (v - V0) / (V1 - V0);                             \
        io.AddKeyAnalogEvent(KEY_NO, v > 0.10f, Saturate(v)); \
    } while (0)
#else
        int axes_count = 0, buttons_count = 0;
        const float* axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &axes_count);
        const unsigned char* buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &buttons_count);
        if (axes_count == 0 || buttons_count == 0)
            return;
#define MAP_BUTTON(KEY_NO, _UNUSED, BUTTON_NO)                                                   \
    do                                                                                           \
    {                                                                                            \
        io.AddKeyEvent(KEY_NO, (buttons_count > BUTTON_NO && buttons[BUTTON_NO] == GLFW_PRESS)); \
    } while (0)
#define MAP_ANALOG(KEY_NO, _UNUSED, AXIS_NO, V0, V1)           \
    do                                                         \
    {                                                          \
        float v = (axes_count > AXIS_NO) ? axes[AXIS_NO] : V0; \
        v = (v - V0) / (V1 - V0);                              \
        io.AddKeyAnalogEvent(KEY_NO, v > 0.10f, Saturate(v));  \
    } while (0)
#endif
        io.BackendFlags |= ImGuiBackendFlags_HasGamepad;
        MAP_BUTTON(ImGuiKey_GamepadStart, GLFW_GAMEPAD_BUTTON_START, 7);
        MAP_BUTTON(ImGuiKey_GamepadBack, GLFW_GAMEPAD_BUTTON_BACK, 6);
        MAP_BUTTON(ImGuiKey_GamepadFaceDown, GLFW_GAMEPAD_BUTTON_A, 0); // Xbox A, PS Cross
        MAP_BUTTON(ImGuiKey_GamepadFaceRight, GLFW_GAMEPAD_BUTTON_B, 1); // Xbox B, PS Circle
        MAP_BUTTON(ImGuiKey_GamepadFaceLeft, GLFW_GAMEPAD_BUTTON_X, 2); // Xbox X, PS Square
        MAP_BUTTON(ImGuiKey_GamepadFaceUp, GLFW_GAMEPAD_BUTTON_Y, 3); // Xbox Y, PS Triangle
        MAP_BUTTON(ImGuiKey_GamepadDpadLeft, GLFW_GAMEPAD_BUTTON_DPAD_LEFT, 13);
        MAP_BUTTON(ImGuiKey_GamepadDpadRight, GLFW_GAMEPAD_BUTTON_DPAD_RIGHT, 11);
        MAP_BUTTON(ImGuiKey_GamepadDpadUp, GLFW_GAMEPAD_BUTTON_DPAD_UP, 10);
        MAP_BUTTON(ImGuiKey_GamepadDpadDown, GLFW_GAMEPAD_BUTTON_DPAD_DOWN, 12);
        MAP_BUTTON(ImGuiKey_GamepadL1, GLFW_GAMEPAD_BUTTON_LEFT_BUMPER, 4);
        MAP_BUTTON(ImGuiKey_GamepadR1, GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER, 5);
        MAP_ANALOG(ImGuiKey_GamepadL2, GLFW_GAMEPAD_AXIS_LEFT_TRIGGER, 4, -0.75f, +1.0f);
        MAP_ANALOG(ImGuiKey_GamepadR2, GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER, 5, -0.75f, +1.0f);
        MAP_BUTTON(ImGuiKey_GamepadL3, GLFW_GAMEPAD_BUTTON_LEFT_THUMB, 8);
        MAP_BUTTON(ImGuiKey_GamepadR3, GLFW_GAMEPAD_BUTTON_RIGHT_THUMB, 9);
        MAP_ANALOG(ImGuiKey_GamepadLStickLeft, GLFW_GAMEPAD_AXIS_LEFT_X, 0, -0.25f, -1.0f);
        MAP_ANALOG(ImGuiKey_GamepadLStickRight, GLFW_GAMEPAD_AXIS_LEFT_X, 0, +0.25f, +1.0f);
        MAP_ANALOG(ImGuiKey_GamepadLStickUp, GLFW_GAMEPAD_AXIS_LEFT_Y, 1, -0.25f, -1.0f);
        MAP_ANALOG(ImGuiKey_GamepadLStickDown, GLFW_GAMEPAD_AXIS_LEFT_Y, 1, +0.25f, +1.0f);
        MAP_ANALOG(ImGuiKey_GamepadRStickLeft, GLFW_GAMEPAD_AXIS_RIGHT_X, 2, -0.25f, -1.0f);
        MAP_ANALOG(ImGuiKey_GamepadRStickRight, GLFW_GAMEPAD_AXIS_RIGHT_X, 2, +0.25f, +1.0f);
        MAP_ANALOG(ImGuiKey_GamepadRStickUp, GLFW_GAMEPAD_AXIS_RIGHT_Y, 3, -0.25f, -1.0f);
        MAP_ANALOG(ImGuiKey_GamepadRStickDown, GLFW_GAMEPAD_AXIS_RIGHT_Y, 3, +0.25f, +1.0f);
#undef MAP_BUTTON
#undef MAP_ANALOG
    }

    static void UpdateMonitors()
    {
        ImGuiData* bd = ImGuiGetBackendData();
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
        bd->WantUpdateMonitors = false;
    }

    void ImGui_ImplGlfw_NewFrame()
    {
        ImGuiIO& io = ImGui::GetIO();
        ImGuiData* bd = ImGuiGetBackendData();
        ASSERT_MSG(bd != nullptr, "Did you call ImGui_ImplGlfw_InitForXXX()?");

        // Setup display size (every frame to accommodate for window resizing)
        auto windowSize = bd->Window->GetSize();
        auto framebuffer = bd->Window->GetFramebufferSize();
        io.DisplaySize = Vector2iToImVec(windowSize);

        if (windowSize.x > 0 && windowSize.y > 0)
            io.DisplayFramebufferScale = ImVec2((float)framebuffer.x / (float)windowSize.x,
                                                (float)framebuffer.y / (float)windowSize.y);
        if (bd->WantUpdateMonitors)
            UpdateMonitors();

        // Setup time step
        double current_time = glfwGetTime();
        io.DeltaTime = bd->Time > 0.0 ? (float)(current_time - bd->Time) : (float)(1.0f / 60.0f);
        bd->Time = current_time;

        UpdateMouseData();
        ImGui_ImplGlfw_UpdateMouseCursor();

        // Update game controllers (if enabled and available)
        ImGui_ImplGlfw_UpdateGamepads();

        ImGui::NewFrame();
    }

    //--------------------------------------------------------------------------------------------------------
    // MULTI-VIEWPORT / PLATFORM INTERFACE SUPPORT
    // This is an _advanced_ and _optional_ feature, allowing the backend to create and handle multiple viewports simultaneously.
    // If you are new to dear imgui or creating a new binding for dear imgui, it is recommended that you completely ignore this section first..
    //--------------------------------------------------------------------------------------------------------

    static void WindowCloseCallback(const Window& window)
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
    static void WindowPosCallback(const Window& window, const Vector2i& pos)
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

    static void WindowSizeCallback(const Window& window, const Vector2i& size)
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

    static void Platform_CreateWindow(ImGuiViewport* viewport)
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
        vd->Window->OnContentScaleChange.Subscribe<ContentScaleChange>();
        vd->Window->OnFocus.Subscribe<WindowFocusCallback>();
        vd->Window->OnKey.Subscribe<KeyCallback>();
        vd->Window->OnMouseButton.Subscribe<MouseButtonCallback>();
        vd->Window->OnMouseCross.Subscribe<MouseCrossCallback>();
        vd->Window->OnMouseMove.Subscribe<MouseMoveCallback>();
        vd->Window->OnMove.Subscribe<WindowPosCallback>();
        vd->Window->OnResize.Subscribe<WindowSizeCallback>();
        vd->Window->OnScroll.Subscribe<ScrollCallback>();
    }

    static void Platform_DestroyWindow(ImGuiViewport* viewport)
    {
        ImGuiData* bd = ImGuiGetBackendData();

        if (ImGuiViewportData* vd = (ImGuiViewportData*)viewport->PlatformUserData)
        {
            ASSERT(vd->Window);
            const auto* window = vd->Window.get();

            if (vd->WindowOwned)
            {
                // Release any keys that were pressed in the window being destroyed and are still held down,
                // because we will not receive any release events after window is destroyed.
                for (size_t keyIndex = 0; keyIndex < bd->KeyOwnerWindows.size(); keyIndex++)
                    if (bd->KeyOwnerWindows[keyIndex] == window)
                        // Later params are only used for main viewport, on which this function is never called.
                        KeyCallback(*window, Input::Key(keyIndex), 0, Input::KeyAction::Release, Input::ModifierFlag::None); 
            }
            vd->Window = nullptr;
            IM_DELETE(vd);
        }
        viewport->PlatformUserData = viewport->PlatformHandle = nullptr;
    }

    static void Platform_ShowWindow(ImGuiViewport* viewport)
    {
        ASSERT(viewport->PlatformUserData);

        ImGuiViewportData* vd = (ImGuiViewportData*)viewport->PlatformUserData;

        vd->Window->SetWindowAttribute(Window::Attribute::TaskbarIcon, !(viewport->Flags & ImGuiViewportFlags_NoTaskBarIcon));
        vd->Window->SetWindowAttribute(Window::Attribute::MousePassthrough, true);
        vd->Window->Show();
    }

    static ImVec2 Platform_GetWindowPos(ImGuiViewport* viewport)
    {
        ASSERT(viewport->PlatformUserData);

        ImGuiViewportData* vd = (ImGuiViewportData*)viewport->PlatformUserData;
        const auto& position = vd->Window->GetPosition();

        return ImVec2(static_cast<float>(position.x),
                      static_cast<float>(position.y));
    }

    static void Platform_SetWindowPos(ImGuiViewport* viewport, ImVec2 pos)
    {
        ASSERT(viewport->PlatformUserData);

        ImGuiViewportData* vd = (ImGuiViewportData*)viewport->PlatformUserData;
        vd->IgnoreWindowPosEventFrame = ImGui::GetFrameCount();
        vd->Window->SetPosition(Vector2i(static_cast<int32_t>(pos.x),
                                         static_cast<int32_t>(pos.y)));
    }

    static ImVec2 Platform_GetWindowSize(ImGuiViewport* viewport)
    {
        ASSERT(viewport->PlatformUserData);

        ImGuiViewportData* vd = (ImGuiViewportData*)viewport->PlatformUserData;
        return Vector2iToImVec(vd->Window->GetSize());
    }

    static void Platform_SetWindowSize(ImGuiViewport* viewport, ImVec2 size)
    {
        ASSERT(viewport->PlatformUserData);

        ImGuiViewportData* vd = (ImGuiViewportData*)viewport->PlatformUserData;
        vd->Window->SetSize(ImVecToVector2i(size));
        vd->IgnoreWindowSizeEventFrame = ImGui::GetFrameCount();
    }

    static void Platform_SetWindowTitle(ImGuiViewport* viewport, const char* title)
    {
        ASSERT(viewport->PlatformUserData);

        ImGuiViewportData* vd = (ImGuiViewportData*)viewport->PlatformUserData;
        vd->Window->SetTitle(title);
    }

    static void Platform_SetWindowFocus(ImGuiViewport* viewport)
    {
        ASSERT(viewport->PlatformUserData);

        ImGuiViewportData* vd = (ImGuiViewportData*)viewport->PlatformUserData;
        vd->Window->Focus();
    }

    static bool Platform_GetWindowFocus(ImGuiViewport* viewport)
    {
        ASSERT(viewport->PlatformUserData);

        ImGuiViewportData* vd = (ImGuiViewportData*)viewport->PlatformUserData;
        return vd->Window->GetWindowAttribute(Window::Attribute::Focused) != 0;
    }

    static bool Platform_GetWindowMinimized(ImGuiViewport* viewport)
    {
        ASSERT(viewport->PlatformUserData);

        ImGuiViewportData* vd = (ImGuiViewportData*)viewport->PlatformUserData;
        return vd->Window->GetWindowAttribute(Window::Attribute::Minimized) != 0;
    }

    static void Platform_SetWindowAlpha(ImGuiViewport* viewport, float alpha)
    {
        ASSERT(viewport->PlatformUserData);

        ImGuiViewportData* vd = (ImGuiViewportData*)viewport->PlatformUserData;
        vd->Window->SetWindowAlpha(alpha);
    }

    static void ImGuiInitPlatformInterface()
    {
        // Register platform interface (will be coupled with a renderer interface)
        ImGuiData* bd = ImGuiGetBackendData();
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
        vd->Window = bd->Window;
        vd->WindowOwned = false;
        mainViewport->PlatformUserData = vd;
        mainViewport->PlatformHandle = (void*)bd->Window.get();
    }

    static void ImGuiShutdownPlatformInterface()
    {
        ImGui::DestroyPlatformWindows();
    }
}