#include "imgui_impl_win_rr.h"
#include "imgui.h"

#include "windowing/WindowSystem.hpp"

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

// GLFW data
enum GlfwClientApi
{
    GlfwClientApi_Unknown,
    GlfwClientApi_OpenGL,
    GlfwClientApi_Vulkan
};

// Helper structure we store in the void* RenderUserData field of each ImGuiViewport to easily retrieve our backend data.
struct ImGui_ImplGlfw_ViewportData
{
    GLFWwindow* GLFWWindow;
    std::shared_ptr<RR::Windowing::Window> Window;
    bool WindowOwned;
    int IgnoreWindowPosEventFrame;
    int IgnoreWindowSizeEventFrame;

    ImGui_ImplGlfw_ViewportData()
    {
        GLFWWindow = nullptr;
        Window = nullptr;
        WindowOwned = false;
        IgnoreWindowSizeEventFrame = IgnoreWindowPosEventFrame = -1;
    }

    ~ImGui_ImplGlfw_ViewportData()
    {
        ASSERT(Window == nullptr);
        ASSERT(GLFWWindow == nullptr);
    }
};

struct ImGui_ImplGlfw_Data
{
    GLFWwindow* GlfwWindow;
    std::shared_ptr<RR::Windowing::Window> Window;
    GlfwClientApi ClientApi;
    double Time;
    GLFWwindow* MouseWindow;
    GLFWcursor* MouseCursors[ImGuiMouseCursor_COUNT];
    ImVec2 LastValidMousePos;
    GLFWwindow* KeyOwnerWindows[GLFW_KEY_LAST];
    bool InstalledCallbacks;
    bool WantUpdateMonitors;

    // Chain GLFW callbacks: our callbacks will call the user's previously installed callbacks, if any.
    GLFWwindowfocusfun PrevUserCallbackWindowFocus;
    GLFWcursorposfun PrevUserCallbackCursorPos;
    GLFWcursorenterfun PrevUserCallbackCursorEnter;
    GLFWmousebuttonfun PrevUserCallbackMousebutton;
    GLFWscrollfun PrevUserCallbackScroll;
    GLFWkeyfun PrevUserCallbackKey;
    GLFWcharfun PrevUserCallbackChar;
    GLFWmonitorfun PrevUserCallbackMonitor;

    ImGui_ImplGlfw_Data() { memset((void*)this, 0, sizeof(*this)); }
};

namespace
{
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
static ImGui_ImplGlfw_Data* ImGui_ImplGlfw_GetBackendData()
{
    return ImGui::GetCurrentContext() ? (ImGui_ImplGlfw_Data*)ImGui::GetIO().BackendPlatformUserData : nullptr;
}

// Forward Declarations
static void ImGui_ImplGlfw_UpdateMonitors();
static void ImGui_ImplGlfw_InitPlatformInterface();
static void ImGui_ImplGlfw_ShutdownPlatformInterface();

// Functions
static const char* ImGui_ImplGlfw_GetClipboardText(void* user_data)
{
    return glfwGetClipboardString((GLFWwindow*)user_data);
}

static void ImGui_ImplGlfw_SetClipboardText(void* user_data, const char* text)
{
    glfwSetClipboardString((GLFWwindow*)user_data, text);
}

static ImGuiKey ImGui_ImplGlfw_KeyToImGuiKey(int key)
{
    switch (key)
    {
        case GLFW_KEY_TAB: return ImGuiKey_Tab;
        case GLFW_KEY_LEFT: return ImGuiKey_LeftArrow;
        case GLFW_KEY_RIGHT: return ImGuiKey_RightArrow;
        case GLFW_KEY_UP: return ImGuiKey_UpArrow;
        case GLFW_KEY_DOWN: return ImGuiKey_DownArrow;
        case GLFW_KEY_PAGE_UP: return ImGuiKey_PageUp;
        case GLFW_KEY_PAGE_DOWN: return ImGuiKey_PageDown;
        case GLFW_KEY_HOME: return ImGuiKey_Home;
        case GLFW_KEY_END: return ImGuiKey_End;
        case GLFW_KEY_INSERT: return ImGuiKey_Insert;
        case GLFW_KEY_DELETE: return ImGuiKey_Delete;
        case GLFW_KEY_BACKSPACE: return ImGuiKey_Backspace;
        case GLFW_KEY_SPACE: return ImGuiKey_Space;
        case GLFW_KEY_ENTER: return ImGuiKey_Enter;
        case GLFW_KEY_ESCAPE: return ImGuiKey_Escape;
        case GLFW_KEY_APOSTROPHE: return ImGuiKey_Apostrophe;
        case GLFW_KEY_COMMA: return ImGuiKey_Comma;
        case GLFW_KEY_MINUS: return ImGuiKey_Minus;
        case GLFW_KEY_PERIOD: return ImGuiKey_Period;
        case GLFW_KEY_SLASH: return ImGuiKey_Slash;
        case GLFW_KEY_SEMICOLON: return ImGuiKey_Semicolon;
        case GLFW_KEY_EQUAL: return ImGuiKey_Equal;
        case GLFW_KEY_LEFT_BRACKET: return ImGuiKey_LeftBracket;
        case GLFW_KEY_BACKSLASH: return ImGuiKey_Backslash;
        case GLFW_KEY_RIGHT_BRACKET: return ImGuiKey_RightBracket;
        case GLFW_KEY_GRAVE_ACCENT: return ImGuiKey_GraveAccent;
        case GLFW_KEY_CAPS_LOCK: return ImGuiKey_CapsLock;
        case GLFW_KEY_SCROLL_LOCK: return ImGuiKey_ScrollLock;
        case GLFW_KEY_NUM_LOCK: return ImGuiKey_NumLock;
        case GLFW_KEY_PRINT_SCREEN: return ImGuiKey_PrintScreen;
        case GLFW_KEY_PAUSE: return ImGuiKey_Pause;
        case GLFW_KEY_KP_0: return ImGuiKey_Keypad0;
        case GLFW_KEY_KP_1: return ImGuiKey_Keypad1;
        case GLFW_KEY_KP_2: return ImGuiKey_Keypad2;
        case GLFW_KEY_KP_3: return ImGuiKey_Keypad3;
        case GLFW_KEY_KP_4: return ImGuiKey_Keypad4;
        case GLFW_KEY_KP_5: return ImGuiKey_Keypad5;
        case GLFW_KEY_KP_6: return ImGuiKey_Keypad6;
        case GLFW_KEY_KP_7: return ImGuiKey_Keypad7;
        case GLFW_KEY_KP_8: return ImGuiKey_Keypad8;
        case GLFW_KEY_KP_9: return ImGuiKey_Keypad9;
        case GLFW_KEY_KP_DECIMAL: return ImGuiKey_KeypadDecimal;
        case GLFW_KEY_KP_DIVIDE: return ImGuiKey_KeypadDivide;
        case GLFW_KEY_KP_MULTIPLY: return ImGuiKey_KeypadMultiply;
        case GLFW_KEY_KP_SUBTRACT: return ImGuiKey_KeypadSubtract;
        case GLFW_KEY_KP_ADD: return ImGuiKey_KeypadAdd;
        case GLFW_KEY_KP_ENTER: return ImGuiKey_KeypadEnter;
        case GLFW_KEY_KP_EQUAL: return ImGuiKey_KeypadEqual;
        case GLFW_KEY_LEFT_SHIFT: return ImGuiKey_LeftShift;
        case GLFW_KEY_LEFT_CONTROL: return ImGuiKey_LeftCtrl;
        case GLFW_KEY_LEFT_ALT: return ImGuiKey_LeftAlt;
        case GLFW_KEY_LEFT_SUPER: return ImGuiKey_LeftSuper;
        case GLFW_KEY_RIGHT_SHIFT: return ImGuiKey_RightShift;
        case GLFW_KEY_RIGHT_CONTROL: return ImGuiKey_RightCtrl;
        case GLFW_KEY_RIGHT_ALT: return ImGuiKey_RightAlt;
        case GLFW_KEY_RIGHT_SUPER: return ImGuiKey_RightSuper;
        case GLFW_KEY_MENU: return ImGuiKey_Menu;
        case GLFW_KEY_0: return ImGuiKey_0;
        case GLFW_KEY_1: return ImGuiKey_1;
        case GLFW_KEY_2: return ImGuiKey_2;
        case GLFW_KEY_3: return ImGuiKey_3;
        case GLFW_KEY_4: return ImGuiKey_4;
        case GLFW_KEY_5: return ImGuiKey_5;
        case GLFW_KEY_6: return ImGuiKey_6;
        case GLFW_KEY_7: return ImGuiKey_7;
        case GLFW_KEY_8: return ImGuiKey_8;
        case GLFW_KEY_9: return ImGuiKey_9;
        case GLFW_KEY_A: return ImGuiKey_A;
        case GLFW_KEY_B: return ImGuiKey_B;
        case GLFW_KEY_C: return ImGuiKey_C;
        case GLFW_KEY_D: return ImGuiKey_D;
        case GLFW_KEY_E: return ImGuiKey_E;
        case GLFW_KEY_F: return ImGuiKey_F;
        case GLFW_KEY_G: return ImGuiKey_G;
        case GLFW_KEY_H: return ImGuiKey_H;
        case GLFW_KEY_I: return ImGuiKey_I;
        case GLFW_KEY_J: return ImGuiKey_J;
        case GLFW_KEY_K: return ImGuiKey_K;
        case GLFW_KEY_L: return ImGuiKey_L;
        case GLFW_KEY_M: return ImGuiKey_M;
        case GLFW_KEY_N: return ImGuiKey_N;
        case GLFW_KEY_O: return ImGuiKey_O;
        case GLFW_KEY_P: return ImGuiKey_P;
        case GLFW_KEY_Q: return ImGuiKey_Q;
        case GLFW_KEY_R: return ImGuiKey_R;
        case GLFW_KEY_S: return ImGuiKey_S;
        case GLFW_KEY_T: return ImGuiKey_T;
        case GLFW_KEY_U: return ImGuiKey_U;
        case GLFW_KEY_V: return ImGuiKey_V;
        case GLFW_KEY_W: return ImGuiKey_W;
        case GLFW_KEY_X: return ImGuiKey_X;
        case GLFW_KEY_Y: return ImGuiKey_Y;
        case GLFW_KEY_Z: return ImGuiKey_Z;
        case GLFW_KEY_F1: return ImGuiKey_F1;
        case GLFW_KEY_F2: return ImGuiKey_F2;
        case GLFW_KEY_F3: return ImGuiKey_F3;
        case GLFW_KEY_F4: return ImGuiKey_F4;
        case GLFW_KEY_F5: return ImGuiKey_F5;
        case GLFW_KEY_F6: return ImGuiKey_F6;
        case GLFW_KEY_F7: return ImGuiKey_F7;
        case GLFW_KEY_F8: return ImGuiKey_F8;
        case GLFW_KEY_F9: return ImGuiKey_F9;
        case GLFW_KEY_F10: return ImGuiKey_F10;
        case GLFW_KEY_F11: return ImGuiKey_F11;
        case GLFW_KEY_F12: return ImGuiKey_F12;
        default: return ImGuiKey_None;
    }
}

static int ImGui_ImplGlfw_KeyToModifier(int key)
{
    if (key == GLFW_KEY_LEFT_CONTROL || key == GLFW_KEY_RIGHT_CONTROL)
        return GLFW_MOD_CONTROL;
    if (key == GLFW_KEY_LEFT_SHIFT || key == GLFW_KEY_RIGHT_SHIFT)
        return GLFW_MOD_SHIFT;
    if (key == GLFW_KEY_LEFT_ALT || key == GLFW_KEY_RIGHT_ALT)
        return GLFW_MOD_ALT;
    if (key == GLFW_KEY_LEFT_SUPER || key == GLFW_KEY_RIGHT_SUPER)
        return GLFW_MOD_SUPER;
    return 0;
}

static void ImGui_ImplGlfw_UpdateKeyModifiers(int mods)
{
    ImGuiIO& io = ImGui::GetIO();
    io.AddKeyEvent(ImGuiKey_ModCtrl, (mods & GLFW_MOD_CONTROL) != 0);
    io.AddKeyEvent(ImGuiKey_ModShift, (mods & GLFW_MOD_SHIFT) != 0);
    io.AddKeyEvent(ImGuiKey_ModAlt, (mods & GLFW_MOD_ALT) != 0);
    io.AddKeyEvent(ImGuiKey_ModSuper, (mods & GLFW_MOD_SUPER) != 0);
}

void ImGui_ImplGlfw_MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    ImGui_ImplGlfw_Data* bd = ImGui_ImplGlfw_GetBackendData();
    if (bd->PrevUserCallbackMousebutton != nullptr && window == bd->GlfwWindow)
        bd->PrevUserCallbackMousebutton(window, button, action, mods);

    ImGui_ImplGlfw_UpdateKeyModifiers(mods);

    ImGuiIO& io = ImGui::GetIO();
    if (button >= 0 && button < ImGuiMouseButton_COUNT)
        io.AddMouseButtonEvent(button, action == GLFW_PRESS);
}

void ImGui_ImplGlfw_ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    ImGui_ImplGlfw_Data* bd = ImGui_ImplGlfw_GetBackendData();
    if (bd->PrevUserCallbackScroll != nullptr && window == bd->GlfwWindow)
        bd->PrevUserCallbackScroll(window, xoffset, yoffset);

    ImGuiIO& io = ImGui::GetIO();
    io.AddMouseWheelEvent((float)xoffset, (float)yoffset);
}

static int ImGui_ImplGlfw_TranslateUntranslatedKey(int key, int scancode)
{
#if GLFW_HAS_GET_KEY_NAME && !defined(__EMSCRIPTEN__)
    // GLFW 3.1+ attempts to "untranslate" keys, which goes the opposite of what every other framework does, making using lettered shortcuts difficult.
    // (It had reasons to do so: namely GLFW is/was more likely to be used for WASD-type game controls rather than lettered shortcuts, but IHMO the 3.1 change could have been done differently)
    // See https://github.com/glfw/glfw/issues/1502 for details.
    // Adding a workaround to undo this (so our keys are translated->untranslated->translated, likely a lossy process).
    // This won't cover edge cases but this is at least going to cover common cases.
    if (key >= GLFW_KEY_KP_0 && key <= GLFW_KEY_KP_EQUAL)
        return key;
    const char* key_name = glfwGetKeyName(key, scancode);
    if (key_name && key_name[0] != 0 && key_name[1] == 0)
    {
        const char char_names[] = "`-=[]\\,;\'./";
        const int char_keys[] = { GLFW_KEY_GRAVE_ACCENT, GLFW_KEY_MINUS, GLFW_KEY_EQUAL, GLFW_KEY_LEFT_BRACKET, GLFW_KEY_RIGHT_BRACKET, GLFW_KEY_BACKSLASH, GLFW_KEY_COMMA, GLFW_KEY_SEMICOLON, GLFW_KEY_APOSTROPHE, GLFW_KEY_PERIOD, GLFW_KEY_SLASH, 0 };
        // TODO ASSERT
        IM_ASSERT(IM_ARRAYSIZE(char_names) == IM_ARRAYSIZE(char_keys));
        if (key_name[0] >= '0' && key_name[0] <= '9')
        {
            key = GLFW_KEY_0 + (key_name[0] - '0');
        }
        else if (key_name[0] >= 'A' && key_name[0] <= 'Z')
        {
            key = GLFW_KEY_A + (key_name[0] - 'A');
        }
        else if (key_name[0] >= 'a' && key_name[0] <= 'z')
        {
            key = GLFW_KEY_A + (key_name[0] - 'a');
        }
        else if (const char* p = strchr(char_names, key_name[0]))
        {
            key = char_keys[p - char_names];
        }
    }
    // if (action == GLFW_PRESS) printf("key %d scancode %d name '%s'\n", key, scancode, key_name);
#else
    IM_UNUSED(scancode);
#endif
    return key;
}

void ImGui_ImplGlfw_KeyCallback(GLFWwindow* window, int keycode, int scancode, int action, int mods)
{
    ImGui_ImplGlfw_Data* bd = ImGui_ImplGlfw_GetBackendData();
    if (bd->PrevUserCallbackKey != nullptr && window == bd->GlfwWindow)
        bd->PrevUserCallbackKey(window, keycode, scancode, action, mods);

    if (action != GLFW_PRESS && action != GLFW_RELEASE)
        return;

    // Workaround: X11 does not include current pressed/released modifier key in 'mods' flags. https://github.com/glfw/glfw/issues/1630
    if (int keycode_to_mod = ImGui_ImplGlfw_KeyToModifier(keycode))
        mods = (action == GLFW_PRESS) ? (mods | keycode_to_mod) : (mods & ~keycode_to_mod);
    ImGui_ImplGlfw_UpdateKeyModifiers(mods);

    if (keycode >= 0 && keycode < IM_ARRAYSIZE(bd->KeyOwnerWindows))
        bd->KeyOwnerWindows[keycode] = (action == GLFW_PRESS) ? window : nullptr;

    keycode = ImGui_ImplGlfw_TranslateUntranslatedKey(keycode, scancode);

    ImGuiIO& io = ImGui::GetIO();
    ImGuiKey imgui_key = ImGui_ImplGlfw_KeyToImGuiKey(keycode);
    io.AddKeyEvent(imgui_key, (action == GLFW_PRESS));
    io.SetKeyEventNativeData(imgui_key, keycode, scancode); // To support legacy indexing (<1.87 user code)
}

void ImGui_ImplGlfw_WindowFocusCallback(GLFWwindow* window, int focused)
{
    ImGui_ImplGlfw_Data* bd = ImGui_ImplGlfw_GetBackendData();
    if (bd->PrevUserCallbackWindowFocus != nullptr && window == bd->GlfwWindow)
        bd->PrevUserCallbackWindowFocus(window, focused);

    ImGuiIO& io = ImGui::GetIO();
    io.AddFocusEvent(focused != 0);
}

void ImGui_ImplGlfw_CursorPosCallback(GLFWwindow* window, double x, double y)
{
    ImGui_ImplGlfw_Data* bd = ImGui_ImplGlfw_GetBackendData();
    if (bd->PrevUserCallbackCursorPos != nullptr && window == bd->GlfwWindow)
        bd->PrevUserCallbackCursorPos(window, x, y);

    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        int window_x, window_y;
        glfwGetWindowPos(window, &window_x, &window_y);
        x += window_x;
        y += window_y;
    }
    io.AddMousePosEvent((float)x, (float)y);
    bd->LastValidMousePos = ImVec2((float)x, (float)y);
}

// Workaround: X11 seems to send spurious Leave/Enter events which would make us lose our position,
// so we back it up and restore on Leave/Enter (see https://github.com/ocornut/imgui/issues/4984)
void ImGui_ImplGlfw_CursorEnterCallback(GLFWwindow* window, int entered)
{
    ImGui_ImplGlfw_Data* bd = ImGui_ImplGlfw_GetBackendData();
    if (bd->PrevUserCallbackCursorEnter != nullptr && window == bd->GlfwWindow)
        bd->PrevUserCallbackCursorEnter(window, entered);

    ImGuiIO& io = ImGui::GetIO();
    if (entered)
    {
        bd->MouseWindow = window;
        io.AddMousePosEvent(bd->LastValidMousePos.x, bd->LastValidMousePos.y);
    }
    else if (!entered && bd->MouseWindow == window)
    {
        bd->LastValidMousePos = io.MousePos;
        bd->MouseWindow = nullptr;
        io.AddMousePosEvent(-FLT_MAX, -FLT_MAX);
    }
}

void ImGui_ImplGlfw_CharCallback(GLFWwindow* window, unsigned int c)
{
    ImGui_ImplGlfw_Data* bd = ImGui_ImplGlfw_GetBackendData();
    if (bd->PrevUserCallbackChar != nullptr && window == bd->GlfwWindow)
        bd->PrevUserCallbackChar(window, c);

    ImGuiIO& io = ImGui::GetIO();
    io.AddInputCharacter(c);
}

void ImGui_ImplGlfw_MonitorCallback(GLFWmonitor*, int)
{
    ImGui_ImplGlfw_Data* bd = ImGui_ImplGlfw_GetBackendData();
    bd->WantUpdateMonitors = true;
}

void ImGui_ImplGlfw_InstallCallbacks(GLFWwindow* window)
{
    ImGui_ImplGlfw_Data* bd = ImGui_ImplGlfw_GetBackendData();
    ASSERT(bd->InstalledCallbacks == false && "Callbacks already installed!");
    ASSERT(bd->GlfwWindow == window);

    bd->PrevUserCallbackWindowFocus = glfwSetWindowFocusCallback(window, ImGui_ImplGlfw_WindowFocusCallback);
    bd->PrevUserCallbackCursorEnter = glfwSetCursorEnterCallback(window, ImGui_ImplGlfw_CursorEnterCallback);
    bd->PrevUserCallbackCursorPos = glfwSetCursorPosCallback(window, ImGui_ImplGlfw_CursorPosCallback);
    bd->PrevUserCallbackMousebutton = glfwSetMouseButtonCallback(window, ImGui_ImplGlfw_MouseButtonCallback);
    bd->PrevUserCallbackScroll = glfwSetScrollCallback(window, ImGui_ImplGlfw_ScrollCallback);
    bd->PrevUserCallbackKey = glfwSetKeyCallback(window, ImGui_ImplGlfw_KeyCallback);
    bd->PrevUserCallbackChar = glfwSetCharCallback(window, ImGui_ImplGlfw_CharCallback);
    bd->PrevUserCallbackMonitor = glfwSetMonitorCallback(ImGui_ImplGlfw_MonitorCallback);
    bd->InstalledCallbacks = true;
}

void ImGui_ImplGlfw_RestoreCallbacks(GLFWwindow* window)
{
    ImGui_ImplGlfw_Data* bd = ImGui_ImplGlfw_GetBackendData();
    ASSERT(bd->InstalledCallbacks == true && "Callbacks not installed!");
    ASSERT(bd->GlfwWindow == window);

    glfwSetWindowFocusCallback(window, bd->PrevUserCallbackWindowFocus);
    glfwSetCursorEnterCallback(window, bd->PrevUserCallbackCursorEnter);
    glfwSetCursorPosCallback(window, bd->PrevUserCallbackCursorPos);
    glfwSetMouseButtonCallback(window, bd->PrevUserCallbackMousebutton);
    glfwSetScrollCallback(window, bd->PrevUserCallbackScroll);
    glfwSetKeyCallback(window, bd->PrevUserCallbackKey);
    glfwSetCharCallback(window, bd->PrevUserCallbackChar);
    glfwSetMonitorCallback(bd->PrevUserCallbackMonitor);
    bd->InstalledCallbacks = false;
    bd->PrevUserCallbackWindowFocus = nullptr;
    bd->PrevUserCallbackCursorEnter = nullptr;
    bd->PrevUserCallbackCursorPos = nullptr;
    bd->PrevUserCallbackMousebutton = nullptr;
    bd->PrevUserCallbackScroll = nullptr;
    bd->PrevUserCallbackKey = nullptr;
    bd->PrevUserCallbackChar = nullptr;
    bd->PrevUserCallbackMonitor = nullptr;
}

static bool ImGui_ImplRR_Init(const std::shared_ptr<RR::Windowing::Window>& window, bool install_callbacks, GlfwClientApi client_api)
{
    ImGuiIO& io = ImGui::GetIO();
    ASSERT(io.BackendPlatformUserData == nullptr && "Already initialized a platform backend!");

    // Setup backend capabilities flags
    ImGui_ImplGlfw_Data* bd = IM_NEW(ImGui_ImplGlfw_Data)();
    io.BackendPlatformUserData = (void*)bd;
    io.BackendPlatformName = "imgui_impl_glfw";
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors; // We can honor GetMouseCursor() values (optional)
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos; // We can honor io.WantSetMousePos requests (optional, rarely used)
    io.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports; // We can create multi-viewports on the Platform side (optional)
    io.BackendFlags |= ImGuiBackendFlags_HasMouseHoveredViewport; // We can call io.AddMouseViewportEvent() with correct data (optional)

    bd->Window = window;
    bd->GlfwWindow = std::any_cast<GLFWwindow*>(window->GetNativeHandle());
    bd->Time = 0.0;
    bd->WantUpdateMonitors = true;

    io.SetClipboardTextFn = ImGui_ImplGlfw_SetClipboardText;
    io.GetClipboardTextFn = ImGui_ImplGlfw_GetClipboardText;
    io.ClipboardUserData = bd->GlfwWindow;

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
        ImGui_ImplGlfw_InstallCallbacks(std::any_cast<GLFWwindow*>(window->GetNativeHandle()));

    // Update monitors the first time (note: monitor callback are broken in GLFW 3.2 and earlier, see github.com/glfw/glfw/issues/784)
    ImGui_ImplGlfw_UpdateMonitors();
    glfwSetMonitorCallback(ImGui_ImplGlfw_MonitorCallback);

    // Our mouse update function expect PlatformHandle to be filled for the main viewport
    ImGuiViewport* main_viewport = ImGui::GetMainViewport();
    main_viewport->PlatformHandle = (void*)bd->GlfwWindow;
#ifdef _WIN32
    main_viewport->PlatformHandleRaw = std::any_cast<HWND>(bd->Window->GetNativeHandleRaw());
#endif
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        ImGui_ImplGlfw_InitPlatformInterface();

    bd->ClientApi = client_api;
    return true;
}

bool ImGui_ImplRR_InitForOpenGL(const std::shared_ptr<RR::Windowing::Window>& window, bool install_callbacks)
{
    return ImGui_ImplRR_Init(window, install_callbacks, GlfwClientApi_OpenGL);
}

bool ImGui_ImplRR_InitForVulkan(const std::shared_ptr<RR::Windowing::Window>& window, bool install_callbacks)
{
    return ImGui_ImplRR_Init(window, install_callbacks, GlfwClientApi_Vulkan);
}

bool ImGui_ImplRR_InitForOther(const std::shared_ptr<RR::Windowing::Window>& window, bool install_callbacks)
{
    return ImGui_ImplRR_Init(window, install_callbacks, GlfwClientApi_Unknown);
}

void ImGui_ImplGlfw_Shutdown()
{
    ImGui_ImplGlfw_Data* bd = ImGui_ImplGlfw_GetBackendData();
    ASSERT(bd != nullptr && "No platform backend to shutdown, or already shutdown?");
    ImGuiIO& io = ImGui::GetIO();

    ImGui_ImplGlfw_ShutdownPlatformInterface();

    if (bd->InstalledCallbacks)
        ImGui_ImplGlfw_RestoreCallbacks(bd->GlfwWindow);

    for (ImGuiMouseCursor cursor_n = 0; cursor_n < ImGuiMouseCursor_COUNT; cursor_n++)
        glfwDestroyCursor(bd->MouseCursors[cursor_n]);

    io.BackendPlatformName = nullptr;
    io.BackendPlatformUserData = nullptr;
    IM_DELETE(bd);
}

static void ImGui_ImplGlfw_UpdateMouseData()
{
    ImGui_ImplGlfw_Data* bd = ImGui_ImplGlfw_GetBackendData();
    ImGuiIO& io = ImGui::GetIO();
    ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();

    ImGuiID mouse_viewport_id = 0;
    const ImVec2 mouse_pos_prev = io.MousePos;
    for (int n = 0; n < platform_io.Viewports.Size; n++)
    {
        ImGuiViewport* viewport = platform_io.Viewports[n];
        GLFWwindow* window = (GLFWwindow*)viewport->PlatformHandle;

#ifdef __EMSCRIPTEN__
        const bool is_window_focused = true;
#else
        const bool is_window_focused = glfwGetWindowAttrib(window, GLFW_FOCUSED) != 0;
#endif
        if (is_window_focused)
        {
            // (Optional) Set OS mouse position from Dear ImGui if requested (rarely used, only when ImGuiConfigFlags_NavEnableSetMousePos is enabled by user)
            // When multi-viewports are enabled, all Dear ImGui positions are same as OS positions.
            if (io.WantSetMousePos)
                glfwSetCursorPos(window, (double)(mouse_pos_prev.x - viewport->Pos.x), (double)(mouse_pos_prev.y - viewport->Pos.y));

            // (Optional) Fallback to provide mouse position when focused (ImGui_ImplGlfw_CursorPosCallback already provides this when hovered or captured)
            if (bd->MouseWindow == nullptr)
            {
                double mouse_x, mouse_y;
                glfwGetCursorPos(window, &mouse_x, &mouse_y);
                if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
                {
                    // Single viewport mode: mouse position in client window coordinates (io.MousePos is (0,0) when the mouse is on the upper-left corner of the app window)
                    // Multi-viewport mode: mouse position in OS absolute coordinates (io.MousePos is (0,0) when the mouse is on the upper-left of the primary monitor)
                    int window_x, window_y;
                    glfwGetWindowPos(window, &window_x, &window_y);
                    mouse_x += window_x;
                    mouse_y += window_y;
                }
                bd->LastValidMousePos = ImVec2((float)mouse_x, (float)mouse_y);
                io.AddMousePosEvent((float)mouse_x, (float)mouse_y);
            }
        }

        // (Optional) When using multiple viewports: call io.AddMouseViewportEvent() with the viewport the OS mouse cursor is hovering.
        // If ImGuiBackendFlags_HasMouseHoveredViewport is not set by the backend, Dear imGui will ignore this field and infer the information using its flawed heuristic.
        // - [X] GLFW >= 3.3 backend ON WINDOWS ONLY does correctly ignore viewports with the _NoInputs flag.
        // - [!] GLFW <= 3.2 backend CANNOT correctly ignore viewports with the _NoInputs flag, and CANNOT reported Hovered Viewport because of mouse capture.
        //       Some backend are not able to handle that correctly. If a backend report an hovered viewport that has the _NoInputs flag (e.g. when dragging a window
        //       for docking, the viewport has the _NoInputs flag in order to allow us to find the viewport under), then Dear ImGui is forced to ignore the value reported
        //       by the backend, and use its flawed heuristic to guess the viewport behind.
        // - [X] GLFW backend correctly reports this regardless of another viewport behind focused and dragged from (we need this to find a useful drag and drop target).
        // FIXME: This is currently only correct on Win32. See what we do below with the WM_NCHITTEST, missing an equivalent for other systems.
        // See https://github.com/glfw/glfw/issues/1236 if you want to help in making this a GLFW feature.
        const bool windowNoInput = (viewport->Flags & ImGuiViewportFlags_NoInputs) != 0;

        ImGui_ImplGlfw_ViewportData* vd = (ImGui_ImplGlfw_ViewportData*)viewport->PlatformUserData;
        vd->Window->SetWindowAttribute(RR::Windowing::Window::Attribute::MousePassthrough, windowNoInput);

        if (glfwGetWindowAttrib(window, GLFW_HOVERED) && !windowNoInput)
            mouse_viewport_id = viewport->ID;
    }

    if (io.BackendFlags & ImGuiBackendFlags_HasMouseHoveredViewport)
        io.AddMouseViewportEvent(mouse_viewport_id);
}

static void ImGui_ImplGlfw_UpdateMouseCursor()
{
    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplGlfw_Data* bd = ImGui_ImplGlfw_GetBackendData();
    if ((io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange) || glfwGetInputMode(bd->GlfwWindow, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
        return;

    ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
    ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
    for (int n = 0; n < platform_io.Viewports.Size; n++)
    {
        GLFWwindow* window = (GLFWwindow*)platform_io.Viewports[n]->PlatformHandle;
        if (imgui_cursor == ImGuiMouseCursor_None || io.MouseDrawCursor)
        {
            // Hide OS mouse cursor if imgui is drawing it or if it wants no cursor
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
        }
        else
        {
            // Show OS mouse cursor
            // FIXME-PLATFORM: Unfocused windows seems to fail changing the mouse cursor with GLFW 3.2, but 3.3 works here.
            glfwSetCursor(window, bd->MouseCursors[imgui_cursor] ? bd->MouseCursors[imgui_cursor] : bd->MouseCursors[ImGuiMouseCursor_Arrow]);
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
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

static void ImGui_ImplGlfw_UpdateMonitors()
{
    ImGui_ImplGlfw_Data* bd = ImGui_ImplGlfw_GetBackendData();
    ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
    int monitors_count = 0;
    GLFWmonitor** glfw_monitors = glfwGetMonitors(&monitors_count);
    platform_io.Monitors.resize(0);
    for (int n = 0; n < monitors_count; n++)
    {
        ImGuiPlatformMonitor monitor;
        int x, y;
        glfwGetMonitorPos(glfw_monitors[n], &x, &y);
        const GLFWvidmode* vid_mode = glfwGetVideoMode(glfw_monitors[n]);
        monitor.MainPos = monitor.WorkPos = ImVec2((float)x, (float)y);
        monitor.MainSize = monitor.WorkSize = ImVec2((float)vid_mode->width, (float)vid_mode->height);

        int w, h;
        glfwGetMonitorWorkarea(glfw_monitors[n], &x, &y, &w, &h);
        if (w > 0 && h > 0) // Workaround a small GLFW issue reporting zero on monitor changes: https://github.com/glfw/glfw/pull/1761
        {
            monitor.WorkPos = ImVec2((float)x, (float)y);
            monitor.WorkSize = ImVec2((float)w, (float)h);
        }

        // Warning: the validity of monitor DPI information on Windows depends on the application DPI awareness settings, which generally needs to be set in the manifest or at runtime.
        float x_scale, y_scale;
        glfwGetMonitorContentScale(glfw_monitors[n], &x_scale, &y_scale);
        monitor.DpiScale = x_scale;

        platform_io.Monitors.push_back(monitor);
    }
    bd->WantUpdateMonitors = false;
}

void ImGui_ImplGlfw_NewFrame()
{
    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplGlfw_Data* bd = ImGui_ImplGlfw_GetBackendData();
    ASSERT(bd != nullptr && "Did you call ImGui_ImplGlfw_InitForXXX()?");

    // Setup display size (every frame to accommodate for window resizing)
    int w, h;
    int display_w, display_h;
    glfwGetWindowSize(bd->GlfwWindow, &w, &h);
    glfwGetFramebufferSize(bd->GlfwWindow, &display_w, &display_h);
    io.DisplaySize = ImVec2((float)w, (float)h);
    if (w > 0 && h > 0)
        io.DisplayFramebufferScale = ImVec2((float)display_w / (float)w, (float)display_h / (float)h);
    if (bd->WantUpdateMonitors)
        ImGui_ImplGlfw_UpdateMonitors();

    // Setup time step
    double current_time = glfwGetTime();
    io.DeltaTime = bd->Time > 0.0 ? (float)(current_time - bd->Time) : (float)(1.0f / 60.0f);
    bd->Time = current_time;

    ImGui_ImplGlfw_UpdateMouseData();
    ImGui_ImplGlfw_UpdateMouseCursor();

    // Update game controllers (if enabled and available)
    ImGui_ImplGlfw_UpdateGamepads();
}

//--------------------------------------------------------------------------------------------------------
// MULTI-VIEWPORT / PLATFORM INTERFACE SUPPORT
// This is an _advanced_ and _optional_ feature, allowing the backend to create and handle multiple viewports simultaneously.
// If you are new to dear imgui or creating a new binding for dear imgui, it is recommended that you completely ignore this section first..
//--------------------------------------------------------------------------------------------------------

static void ImGui_ImplGlfw_WindowCloseCallback(GLFWwindow* window)
{
    if (ImGuiViewport* viewport = ImGui::FindViewportByPlatformHandle(window))
        viewport->PlatformRequestClose = true;
}

// GLFW may dispatch window pos/size events after calling glfwSetWindowPos()/glfwSetWindowSize().
// However: depending on the platform the callback may be invoked at different time:
// - on Windows it appears to be called within the glfwSetWindowPos()/glfwSetWindowSize() call
// - on Linux it is queued and invoked during glfwPollEvents()
// Because the event doesn't always fire on glfwSetWindowXXX() we use a frame counter tag to only
// ignore recent glfwSetWindowXXX() calls.
static void ImGui_ImplGlfw_WindowPosCallback(GLFWwindow* window, int, int)
{
    if (ImGuiViewport* viewport = ImGui::FindViewportByPlatformHandle(window))
    {
        if (ImGui_ImplGlfw_ViewportData* vd = (ImGui_ImplGlfw_ViewportData*)viewport->PlatformUserData)
        {
            bool ignoreEvent = (ImGui::GetFrameCount() <= vd->IgnoreWindowPosEventFrame + 1);
            //data->IgnoreWindowPosEventFrame = -1;
            if (ignoreEvent)
                return;
        }
        viewport->PlatformRequestMove = true;
    }
}

static void ImGui_ImplGlfw_WindowSizeCallback(GLFWwindow* window, int, int)
{
    if (ImGuiViewport* viewport = ImGui::FindViewportByPlatformHandle(window))
    {
        if (ImGui_ImplGlfw_ViewportData* vd = (ImGui_ImplGlfw_ViewportData*)viewport->PlatformUserData)
        {
            bool ignore_event = (ImGui::GetFrameCount() <= vd->IgnoreWindowSizeEventFrame + 1);
            //data->IgnoreWindowSizeEventFrame = -1;
            if (ignore_event)
                return;
        }
        viewport->PlatformRequestResize = true;
    }
}

static void ImGui_ImplGlfw_CreateWindow(ImGuiViewport* viewport)
{
    ImGui_ImplGlfw_Data* bd = ImGui_ImplGlfw_GetBackendData();
    ImGui_ImplGlfw_ViewportData* vd = IM_NEW(ImGui_ImplGlfw_ViewportData)();
    viewport->PlatformUserData = vd;

    auto& windowSystem = RR::Windowing::WindowSystem::Instance();

    RR::Windowing::Window::Description windowDesc;
    windowDesc.size = ImVecToVector2i(viewport->Size);
    windowDesc.title = "No Title Yet";
    windowDesc.visible = false;
    windowDesc.focused = false;
    windowDesc.focusOnShow = false;
    windowDesc.decorated = (viewport->Flags & ImGuiViewportFlags_NoDecoration) ? false : true;
    windowDesc.floating = (viewport->Flags & ImGuiViewportFlags_TopMost) ? true : false;
    windowDesc.title = "No Title Yet";

    GLFWwindow* share_window = (bd->ClientApi == GlfwClientApi_OpenGL) ? bd->GlfwWindow : nullptr;

    std::ignore = share_window;
    // TODO ShareWindow for opengl
    // glfwCreateWindow((int)viewport->Size.x, (int)viewport->Size.y, "No Title Yet", nullptr, share_window);

    vd->Window = windowSystem.Create(nullptr, windowDesc); // TODO asserts
    vd->GLFWWindow = std::any_cast<GLFWwindow*>(vd->Window->GetNativeHandle());

    vd->WindowOwned = true;
    viewport->PlatformHandle = (void*)vd->GLFWWindow;
#ifdef _WIN32
    viewport->PlatformHandleRaw = std::any_cast<HWND>(vd->Window->GetNativeHandleRaw());
#endif
    vd->Window->SetPosition(ImVecToVector2i(viewport->Pos));

    // Install GLFW callbacks for secondary viewports
    glfwSetWindowFocusCallback(vd->GLFWWindow, ImGui_ImplGlfw_WindowFocusCallback);
    glfwSetCursorEnterCallback(vd->GLFWWindow, ImGui_ImplGlfw_CursorEnterCallback);
    glfwSetCursorPosCallback(vd->GLFWWindow, ImGui_ImplGlfw_CursorPosCallback);
    glfwSetMouseButtonCallback(vd->GLFWWindow, ImGui_ImplGlfw_MouseButtonCallback);
    glfwSetScrollCallback(vd->GLFWWindow, ImGui_ImplGlfw_ScrollCallback);
    glfwSetKeyCallback(vd->GLFWWindow, ImGui_ImplGlfw_KeyCallback);
    glfwSetCharCallback(vd->GLFWWindow, ImGui_ImplGlfw_CharCallback);
    glfwSetWindowCloseCallback(vd->GLFWWindow, ImGui_ImplGlfw_WindowCloseCallback);
    glfwSetWindowPosCallback(vd->GLFWWindow, ImGui_ImplGlfw_WindowPosCallback);
    glfwSetWindowSizeCallback(vd->GLFWWindow, ImGui_ImplGlfw_WindowSizeCallback);
    if (bd->ClientApi == GlfwClientApi_OpenGL)
    {
        glfwMakeContextCurrent(vd->GLFWWindow);
        glfwSwapInterval(0);
    }
}

static void ImGui_ImplGlfw_DestroyWindow(ImGuiViewport* viewport)
{
    ImGui_ImplGlfw_Data* bd = ImGui_ImplGlfw_GetBackendData();
    if (ImGui_ImplGlfw_ViewportData* vd = (ImGui_ImplGlfw_ViewportData*)viewport->PlatformUserData)
    {
        if (vd->WindowOwned)
        {
            // Release any keys that were pressed in the window being destroyed and are still held down,
            // because we will not receive any release events after window is destroyed.
            for (int i = 0; i < IM_ARRAYSIZE(bd->KeyOwnerWindows); i++)
                if (bd->KeyOwnerWindows[i] == vd->GLFWWindow)
                    ImGui_ImplGlfw_KeyCallback(vd->GLFWWindow, i, 0, GLFW_RELEASE, 0); // Later params are only used for main viewport, on which this function is never called.

            //    glfwDestroyWindow(vd->GLFWWindow);
        }
        vd->GLFWWindow = nullptr;
        vd->Window = nullptr;
        IM_DELETE(vd);
    }
    viewport->PlatformUserData = viewport->PlatformHandle = nullptr;
}

static void Platform_ShowWindow(ImGuiViewport* viewport)
{
    ASSERT(viewport->PlatformUserData);

    ImGui_ImplGlfw_ViewportData* vd = (ImGui_ImplGlfw_ViewportData*)viewport->PlatformUserData;

    vd->Window->SetWindowAttribute(RR::Windowing::Window::Attribute::TaskbarIcon, !(viewport->Flags & ImGuiViewportFlags_NoTaskBarIcon));
    vd->Window->SetWindowAttribute(RR::Windowing::Window::Attribute::MousePassthrough, true);
    vd->Window->Show();
}

static ImVec2 Platform_GetWindowPos(ImGuiViewport* viewport)
{
    ASSERT(viewport->PlatformUserData);

    ImGui_ImplGlfw_ViewportData* vd = (ImGui_ImplGlfw_ViewportData*)viewport->PlatformUserData;
    const auto& position = vd->Window->GetPosition();

    return ImVec2(static_cast<float>(position.x),
                  static_cast<float>(position.y));
}

static void Platform_SetWindowPos(ImGuiViewport* viewport, ImVec2 pos)
{
    ASSERT(viewport->PlatformUserData);

    ImGui_ImplGlfw_ViewportData* vd = (ImGui_ImplGlfw_ViewportData*)viewport->PlatformUserData;
    vd->IgnoreWindowPosEventFrame = ImGui::GetFrameCount();
    vd->Window->SetPosition(Vector2i(static_cast<int32_t>(pos.x),
                                     static_cast<int32_t>(pos.y)));
}

static ImVec2 Platform_GetWindowSize(ImGuiViewport* viewport)
{
    ASSERT(viewport->PlatformUserData);

    ImGui_ImplGlfw_ViewportData* vd = (ImGui_ImplGlfw_ViewportData*)viewport->PlatformUserData;
    return Vector2iToImVec(vd->Window->GetSize());
}

static void Platform_SetWindowSize(ImGuiViewport* viewport, ImVec2 size)
{
    ASSERT(viewport->PlatformUserData);

    ImGui_ImplGlfw_ViewportData* vd = (ImGui_ImplGlfw_ViewportData*)viewport->PlatformUserData;
    vd->Window->SetSize(ImVecToVector2i(size));
    vd->IgnoreWindowSizeEventFrame = ImGui::GetFrameCount();
}

static void Platform_SetWindowTitle(ImGuiViewport* viewport, const char* title)
{
    ASSERT(viewport->PlatformUserData);

    ImGui_ImplGlfw_ViewportData* vd = (ImGui_ImplGlfw_ViewportData*)viewport->PlatformUserData;
    vd->Window->SetTitle(title);
}

static void Platform_SetWindowFocus(ImGuiViewport* viewport)
{
    ASSERT(viewport->PlatformUserData);

    ImGui_ImplGlfw_ViewportData* vd = (ImGui_ImplGlfw_ViewportData*)viewport->PlatformUserData;
    vd->Window->Focus();
}

static bool Platform_GetWindowFocus(ImGuiViewport* viewport)
{
    ASSERT(viewport->PlatformUserData);

    ImGui_ImplGlfw_ViewportData* vd = (ImGui_ImplGlfw_ViewportData*)viewport->PlatformUserData;
    return vd->Window->GetWindowAttribute(RR::Windowing::Window::Attribute::Focused) != 0;
}

static bool Platform_GetWindowMinimized(ImGuiViewport* viewport)
{
    ASSERT(viewport->PlatformUserData);

    ImGui_ImplGlfw_ViewportData* vd = (ImGui_ImplGlfw_ViewportData*)viewport->PlatformUserData;
    return vd->Window->GetWindowAttribute(RR::Windowing::Window::Attribute::Minimized) != 0;
}

static void Platform_SetWindowAlpha(ImGuiViewport* viewport, float alpha)
{
    ASSERT(viewport->PlatformUserData);

    ImGui_ImplGlfw_ViewportData* vd = (ImGui_ImplGlfw_ViewportData*)viewport->PlatformUserData;
    vd->Window->SetWindowAlpha(alpha);
}

static void ImGui_ImplGlfw_RenderWindow(ImGuiViewport* viewport, void*)
{
    ImGui_ImplGlfw_Data* bd = ImGui_ImplGlfw_GetBackendData();
    ImGui_ImplGlfw_ViewportData* vd = (ImGui_ImplGlfw_ViewportData*)viewport->PlatformUserData;
    if (bd->ClientApi == GlfwClientApi_OpenGL)
        glfwMakeContextCurrent(vd->GLFWWindow);
}

static void ImGui_ImplGlfw_SwapBuffers(ImGuiViewport* viewport, void*)
{
    ImGui_ImplGlfw_Data* bd = ImGui_ImplGlfw_GetBackendData();
    ImGui_ImplGlfw_ViewportData* vd = (ImGui_ImplGlfw_ViewportData*)viewport->PlatformUserData;
    if (bd->ClientApi == GlfwClientApi_OpenGL)
    {
        glfwMakeContextCurrent(vd->GLFWWindow);
        glfwSwapBuffers(vd->GLFWWindow);
    }
}

//--------------------------------------------------------------------------------------------------------
// Vulkan support (the Vulkan renderer needs to call a platform-side support function to create the surface)
//--------------------------------------------------------------------------------------------------------

static int Platform_CreateVkSurface(ImGuiViewport* viewport, ImU64 vk_instance, const void* vk_allocator, ImU64* out_vk_surface)
{
    std::ignore = out_vk_surface;
    std::ignore = vk_allocator;
    std::ignore = vk_instance;
    std::ignore = viewport;

    ImGui_ImplGlfw_Data* bd = ImGui_ImplGlfw_GetBackendData();
    std::ignore = bd;

    ASSERT(bd->ClientApi == GlfwClientApi_Vulkan);
    ASSERT_MSG(false, "Not implemented");

    return 0;
}

static void ImGui_ImplGlfw_InitPlatformInterface()
{
    // Register platform interface (will be coupled with a renderer interface)
    ImGui_ImplGlfw_Data* bd = ImGui_ImplGlfw_GetBackendData();
    ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
    platform_io.Platform_CreateWindow = ImGui_ImplGlfw_CreateWindow;
    platform_io.Platform_DestroyWindow = ImGui_ImplGlfw_DestroyWindow;
    platform_io.Platform_ShowWindow = Platform_ShowWindow;
    platform_io.Platform_SetWindowPos = Platform_SetWindowPos;
    platform_io.Platform_GetWindowPos = Platform_GetWindowPos;
    platform_io.Platform_SetWindowSize = Platform_SetWindowSize;
    platform_io.Platform_GetWindowSize = Platform_GetWindowSize;
    platform_io.Platform_SetWindowFocus = Platform_SetWindowFocus;
    platform_io.Platform_GetWindowFocus = Platform_GetWindowFocus;
    platform_io.Platform_GetWindowMinimized = Platform_GetWindowMinimized;
    platform_io.Platform_SetWindowTitle = Platform_SetWindowTitle;
    platform_io.Platform_RenderWindow = ImGui_ImplGlfw_RenderWindow;
    platform_io.Platform_SwapBuffers = ImGui_ImplGlfw_SwapBuffers;
    platform_io.Platform_SetWindowAlpha = Platform_SetWindowAlpha;
    platform_io.Platform_CreateVkSurface = Platform_CreateVkSurface;

    // Register main window handle (which is owned by the main application, not by us)
    // This is mostly for simplicity and consistency, so that our code (e.g. mouse handling etc.) can use same logic for main and secondary viewports.
    ImGuiViewport* main_viewport = ImGui::GetMainViewport();
    ImGui_ImplGlfw_ViewportData* vd = IM_NEW(ImGui_ImplGlfw_ViewportData)();
    vd->Window = bd->Window;
    vd->GLFWWindow = bd->GlfwWindow;
    vd->WindowOwned = false;
    main_viewport->PlatformUserData = vd;
    main_viewport->PlatformHandle = (void*)bd->GlfwWindow;
}

static void ImGui_ImplGlfw_ShutdownPlatformInterface()
{
    ImGui::DestroyPlatformWindows();
}