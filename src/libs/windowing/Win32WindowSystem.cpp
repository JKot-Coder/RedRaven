#include "WindowSystem.hpp"

#include "common/Exception.hpp"
#include "common/Math.hpp"

#include "Windowing/Window.hpp"

#include <Windows.h>

namespace OpenDemo
{
    namespace
    {

        static LRESULT CALLBACK windowProc(HWND hWnd, UINT uMsg,
                                           WPARAM wParam, LPARAM lParam)
        {
           // _GLFWwindow* window = GetPropW(hWnd, L"GLFW");
        /*    if (!window)
            {
                // This is the message handling for the hidden helper window
                // and for a regular window during its initial creation

                switch (uMsg)
                {
                case WM_NCCREATE:
                {
                    if (_glfwIsWindows10AnniversaryUpdateOrGreaterWin32())
                    {
                        const CREATESTRUCTW* cs = (const CREATESTRUCTW*)lParam;
                        const _GLFWwndconfig* wndconfig = cs->lpCreateParams;

                        // On per-monitor DPI aware V1 systems, only enable
                        // non-client scaling for windows that scale the client area
                        // We need WM_GETDPISCALEDSIZE from V2 to keep the client
                        // area static when the non-client area is scaled
                        if (wndconfig && wndconfig->scaleToMonitor)
                            EnableNonClientDpiScaling(hWnd);
                    }

                    break;
                }

                case WM_DISPLAYCHANGE:
                    _glfwPollMonitorsWin32();
                    break;

                case WM_DEVICECHANGE:
                {
                    if (!_glfw.joysticksInitialized)
                        break;

                    if (wParam == DBT_DEVICEARRIVAL)
                    {
                        DEV_BROADCAST_HDR* dbh = (DEV_BROADCAST_HDR*)lParam;
                        if (dbh && dbh->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE)
                            _glfwDetectJoystickConnectionWin32();
                    }
                    else if (wParam == DBT_DEVICEREMOVECOMPLETE)
                    {
                        DEV_BROADCAST_HDR* dbh = (DEV_BROADCAST_HDR*)lParam;
                        if (dbh && dbh->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE)
                            _glfwDetectJoystickDisconnectionWin32();
                    }

                    break;
                }
                }

                return DefWindowProcW(hWnd, uMsg, wParam, lParam);
            }

            switch (uMsg)
            {
            case WM_MOUSEACTIVATE:
            {
                // HACK: Postpone cursor disabling when the window was activated by
                //       clicking a caption button
                if (HIWORD(lParam) == WM_LBUTTONDOWN)
                {
                    if (LOWORD(lParam) != HTCLIENT)
                        window->win32.frameAction = GLFW_TRUE;
                }

                break;
            }

            case WM_CAPTURECHANGED:
            {
                // HACK: Disable the cursor once the caption button action has been
                //       completed or cancelled
                if (lParam == 0 && window->win32.frameAction)
                {
                    if (window->cursorMode == GLFW_CURSOR_DISABLED)
                        disableCursor(window);

                    window->win32.frameAction = GLFW_FALSE;
                }

                break;
            }

            case WM_SETFOCUS:
            {
                _glfwInputWindowFocus(window, GLFW_TRUE);

                // HACK: Do not disable cursor while the user is interacting with
                //       a caption button
                if (window->win32.frameAction)
                    break;

                if (window->cursorMode == GLFW_CURSOR_DISABLED)
                    disableCursor(window);

                return 0;
            }

            case WM_KILLFOCUS:
            {
                if (window->cursorMode == GLFW_CURSOR_DISABLED)
                    enableCursor(window);

                if (window->monitor && window->autoIconify)
                    _glfwPlatformIconifyWindow(window);

                _glfwInputWindowFocus(window, GLFW_FALSE);
                return 0;
            }

            case WM_SYSCOMMAND:
            {
                switch (wParam & 0xfff0)
                {
                case SC_SCREENSAVE:
                case SC_MONITORPOWER:
                {
                    if (window->monitor)
                    {
                        // We are running in full screen mode, so disallow
                        // screen saver and screen blanking
                        return 0;
                    }
                    else
                        break;
                }

                // User trying to access application menu using ALT?
                case SC_KEYMENU:
                {
                    if (!window->win32.keymenu)
                        return 0;

                    break;
                }
                }
                break;
            }

            case WM_CLOSE:
            {
                _glfwInputWindowCloseRequest(window);
                return 0;
            }

            case WM_INPUTLANGCHANGE:
            {
                _glfwUpdateKeyNamesWin32();
                break;
            }

            case WM_CHAR:
            case WM_SYSCHAR:
            {
                if (wParam >= 0xd800 && wParam <= 0xdbff)
                    window->win32.highSurrogate = (WCHAR)wParam;
                else
                {
                    unsigned int codepoint = 0;

                    if (wParam >= 0xdc00 && wParam <= 0xdfff)
                    {
                        if (window->win32.highSurrogate)
                        {
                            codepoint += (window->win32.highSurrogate - 0xd800) << 10;
                            codepoint += (WCHAR)wParam - 0xdc00;
                            codepoint += 0x10000;
                        }
                    }
                    else
                        codepoint = (WCHAR)wParam;

                    window->win32.highSurrogate = 0;
                    _glfwInputChar(window, codepoint, getKeyMods(), uMsg != WM_SYSCHAR);
                }

                if (uMsg == WM_SYSCHAR && window->win32.keymenu)
                    break;

                return 0;
            }

            case WM_EXITSIZEMOVE:
            case WM_EXITMENULOOP:
            {
                if (window->win32.frameAction)
                    break;

                // HACK: Disable the cursor once the user is done moving or
                //       resizing the window or using the menu
                if (window->cursorMode == GLFW_CURSOR_DISABLED)
                    disableCursor(window);

                break;
            }

            case WM_SIZE:
            {
                const int width = LOWORD(lParam);
                const int height = HIWORD(lParam);
                const GLFWbool iconified = wParam == SIZE_MINIMIZED;
                const GLFWbool maximized = wParam == SIZE_MAXIMIZED ||
                                           (window->win32.maximized &&
                                            wParam != SIZE_RESTORED);

                if (_glfw.win32.disabledCursorWindow == window)
                    updateClipRect(window);

                if (window->win32.iconified != iconified)
                    _glfwInputWindowIconify(window, iconified);

                if (window->win32.maximized != maximized)
                    _glfwInputWindowMaximize(window, maximized);

                if (width != window->win32.width || height != window->win32.height)
                {
                    window->win32.width = width;
                    window->win32.height = height;

                    _glfwInputFramebufferSize(window, width, height);
                    _glfwInputWindowSize(window, width, height);
                }

                if (window->monitor && window->win32.iconified != iconified)
                {
                    if (iconified)
                        releaseMonitor(window);
                    else
                    {
                        acquireMonitor(window);
                        fitToMonitor(window);
                    }
                }

                window->win32.iconified = iconified;
                window->win32.maximized = maximized;
                return 0;
            }

            case WM_MOVE:
            {
                if (_glfw.win32.disabledCursorWindow == window)
                    updateClipRect(window);

                // NOTE: This cannot use LOWORD/HIWORD recommended by MSDN, as
                // those macros do not handle negative window positions correctly
                _glfwInputWindowPos(window,
                                    GET_X_LPARAM(lParam),
                                    GET_Y_LPARAM(lParam));
                return 0;
            }

            case WM_SIZING:
            {
                if (window->numer == GLFW_DONT_CARE ||
                    window->denom == GLFW_DONT_CARE)
                {
                    break;
                }

                applyAspectRatio(window, (int)wParam, (RECT*)lParam);
                return TRUE;
            }

            case WM_GETMINMAXINFO:
            {
                int xoff, yoff;
                UINT dpi = USER_DEFAULT_SCREEN_DPI;
                MINMAXINFO* mmi = (MINMAXINFO*)lParam;

                if (window->monitor)
                    break;

                if (_glfwIsWindows10AnniversaryUpdateOrGreaterWin32())
                    dpi = GetDpiForWindow(window->win32.handle);

                getFullWindowSize(getWindowStyle(window), getWindowExStyle(window),
                                  0, 0, &xoff, &yoff, dpi);

                if (window->minwidth != GLFW_DONT_CARE &&
                    window->minheight != GLFW_DONT_CARE)
                {
                    mmi->ptMinTrackSize.x = window->minwidth + xoff;
                    mmi->ptMinTrackSize.y = window->minheight + yoff;
                }

                if (window->maxwidth != GLFW_DONT_CARE &&
                    window->maxheight != GLFW_DONT_CARE)
                {
                    mmi->ptMaxTrackSize.x = window->maxwidth + xoff;
                    mmi->ptMaxTrackSize.y = window->maxheight + yoff;
                }

                if (!window->decorated)
                {
                    MONITORINFO mi;
                    const HMONITOR mh = MonitorFromWindow(window->win32.handle,
                                                          MONITOR_DEFAULTTONEAREST);

                    ZeroMemory(&mi, sizeof(mi));
                    mi.cbSize = sizeof(mi);
                    GetMonitorInfo(mh, &mi);

                    mmi->ptMaxPosition.x = mi.rcWork.left - mi.rcMonitor.left;
                    mmi->ptMaxPosition.y = mi.rcWork.top - mi.rcMonitor.top;
                    mmi->ptMaxSize.x = mi.rcWork.right - mi.rcWork.left;
                    mmi->ptMaxSize.y = mi.rcWork.bottom - mi.rcWork.top;
                }

                return 0;
            }

            case WM_PAINT:
            {
                _glfwInputWindowDamage(window);
                break;
            }

            case WM_ERASEBKGND:
            {
                return TRUE;
            }

            case WM_NCACTIVATE:
            case WM_NCPAINT:
            {
                // Prevent title bar from being drawn after restoring a minimized
                // undecorated window
                if (!window->decorated)
                    return TRUE;

                break;
            }

            case WM_DWMCOMPOSITIONCHANGED:
            case WM_DWMCOLORIZATIONCOLORCHANGED:
            {
                if (window->win32.transparent)
                    updateFramebufferTransparency(window);
                return 0;
            }

            case WM_GETDPISCALEDSIZE:
            {
                if (window->win32.scaleToMonitor)
                    break;

                // Adjust the window size to keep the content area size constant
                if (_glfwIsWindows10CreatorsUpdateOrGreaterWin32())
                {
                    RECT source = { 0 }, target = { 0 };
                    SIZE* size = (SIZE*)lParam;

                    AdjustWindowRectExForDpi(&source, getWindowStyle(window),
                                             FALSE, getWindowExStyle(window),
                                             GetDpiForWindow(window->win32.handle));
                    AdjustWindowRectExForDpi(&target, getWindowStyle(window),
                                             FALSE, getWindowExStyle(window),
                                             LOWORD(wParam));

                    size->cx += (target.right - target.left) -
                                (source.right - source.left);
                    size->cy += (target.bottom - target.top) -
                                (source.bottom - source.top);
                    return TRUE;
                }

                break;
            }

            case WM_DPICHANGED:
            {
                const float xscale = HIWORD(wParam) / (float)USER_DEFAULT_SCREEN_DPI;
                const float yscale = LOWORD(wParam) / (float)USER_DEFAULT_SCREEN_DPI;

                // Resize windowed mode windows that either permit rescaling or that
                // need it to compensate for non-client area scaling
                if (!window->monitor &&
                    (window->win32.scaleToMonitor ||
                     _glfwIsWindows10CreatorsUpdateOrGreaterWin32()))
                {
                    RECT* suggested = (RECT*)lParam;
                    SetWindowPos(window->win32.handle, HWND_TOP,
                                 suggested->left,
                                 suggested->top,
                                 suggested->right - suggested->left,
                                 suggested->bottom - suggested->top,
                                 SWP_NOACTIVATE | SWP_NOZORDER);
                }

                _glfwInputWindowContentScale(window, xscale, yscale);
                break;
            }

            case WM_SETCURSOR:
            {
                if (LOWORD(lParam) == HTCLIENT)
                {
                    updateCursorImage(window);
                    return TRUE;
                }

                break;
            }

            case WM_DROPFILES:
            {
                HDROP drop = (HDROP)wParam;
                POINT pt;
                int i;

                const int count = DragQueryFileW(drop, 0xffffffff, NULL, 0);
                char** paths = calloc(count, sizeof(char*));

                // Move the mouse to the position of the drop
                DragQueryPoint(drop, &pt);
                _glfwInputCursorPos(window, pt.x, pt.y);

                for (i = 0; i < count; i++)
                {
                    const UINT length = DragQueryFileW(drop, i, NULL, 0);
                    WCHAR* buffer = calloc((size_t)length + 1, sizeof(WCHAR));

                    DragQueryFileW(drop, i, buffer, length + 1);
                    paths[i] = _glfwCreateUTF8FromWideStringWin32(buffer);

                    free(buffer);
                }

                _glfwInputDrop(window, count, (const char**)paths);

                for (i = 0; i < count; i++)
                    free(paths[i]);
                free(paths);

                DragFinish(drop);
                return 0;
            }
            }

            return DefWindowProcW(hWnd, uMsg, wParam, lParam);
        }*/
    }

    namespace Windowing
    {
        std::vector<IListener*> WindowSystem::WindowSystem::_listeners = std::vector<IListener*>();

        WindowSystem::WindowSystem()
        {
            WNDCLASSEXW wc;

            ZeroMemory(&wc, sizeof(wc));
            wc.cbSize = sizeof(wc);
            wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
            wc.lpfnWndProc = (WNDPROC)windowProc;
            wc.hInstance = GetModuleHandleW(NULL);
            wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
            wc.lpszClassName = WINDOW_CLASS_NAME;

            if (!RegisterClassExW(&wc))
            {
                //_glfwInputErrorWin32(GLFW_PLATFORM_ERROR,
                //                     "Win32: Failed to register window class");
                //  return GLFW_FALSE;
            }
        }

        WindowSystem::~WindowSystem()
        {
        }

        std::shared_ptr<Window> WindowSystem::Create(const WindowDescription& description)
        {
            const auto& window = std::shared_ptr<Window>(new Window());

            if (!window->Init(description))
                return nullptr;

            return window;
        }

        void WindowSystem::PoolEvents()
        {
            MSG msg;

            while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
            {
                if (msg.message == WM_QUIT)
                {
                    // NOTE: While GLFW does not itself post WM_QUIT, other processes
                    //       may post it to this one, for example Task Manager
                    // HACK: Treat WM_QUIT as a close on all windows

                    /*   window = _glfw.windowListHead;
                    while (window)
                    {
                        _glfwInputWindowCloseRequest(window);
                        window = window->next;
                    }*/
                }
                else
                {
                    TranslateMessage(&msg);
                    DispatchMessageW(&msg);
                }
            }

            /* glfwPollEvents();


           SDL_Event e;

            while (SDL_PollEvent(&e))
            {
                for (auto& listener : _listeners)
                {
                    switch (e.type)
                    {
                    case SDL_QUIT:
                        listener->OnQuit();
                        break;
                    case SDL_KEYDOWN:
                    case SDL_KEYUP:
                    {
                        const auto& keyboardEvent = e.key;
                        auto* sdlWindow = SDL_GetWindowFromID(keyboardEvent.windowID);
                        const auto* window = static_cast<Window*>(SDL_GetWindowData(sdlWindow, "WindowObject"));

                        switch (keyboardEvent.state)
                        {
                        case SDL_PRESSED:
                            listener->OnKeyDown(*window, keyboardEvent.keysym);
                            break;
                        case SDL_RELEASED:
                            listener->OnKeyUp(*window, keyboardEvent.keysym);
                            break;
                        }

                        break;
                    }
                    case SDL_MOUSEMOTION:
                    {
                        const auto& motionEvent = e.motion;
                        auto* sdlWindow = SDL_GetWindowFromID(motionEvent.windowID);
                        const auto* window = static_cast<Window*>(SDL_GetWindowData(sdlWindow, "WindowObject"));

                        listener->OnMouseMotion(*window,
                                                Vector2i(motionEvent.x, motionEvent.y),
                                                Vector2i(motionEvent.xrel, motionEvent.yrel));
                    }
                    case SDL_MOUSEBUTTONDOWN:
                    case SDL_MOUSEBUTTONUP:
                    {
                        const auto& buttonEvent = e.button;
                        auto* sdlWindow = SDL_GetWindowFromID(buttonEvent.windowID);
                        const auto* window = static_cast<Window*>(SDL_GetWindowData(sdlWindow, "WindowObject"));

                        switch (buttonEvent.state)
                        {
                        case SDL_PRESSED:
                            listener->OnMouseButtonDown(*window, buttonEvent.button);
                            break;
                        case SDL_RELEASED:
                            listener->OnMouseButtonUp(*window, buttonEvent.button);
                            break;
                        }
                    }
                    case SDL_WINDOWEVENT:
                    {
                        auto* sdlWindow = SDL_GetWindowFromID(e.window.windowID);
                        const auto* window = static_cast<Window*>(SDL_GetWindowData(sdlWindow, "WindowObject"));

                        switch (e.window.event)
                        {
                        case SDL_WINDOWEVENT_RESIZED:
                            listener->OnWindowResize(*window);
                            break;
                        case SDL_WINDOWEVENT_SHOWN:
                            listener->OnWindowShown(*window);
                            break;
                        case SDL_WINDOWEVENT_HIDDEN:
                            listener->OnWindowHidden(*window);
                            break;
                        case SDL_WINDOWEVENT_FOCUS_GAINED:
                            listener->OnWindowFocusGained(*window);
                            break;
                        case SDL_WINDOWEVENT_FOCUS_LOST:
                            listener->OnWindowFocusLost(*window);
                            break;
                        }
                        break;
                    }
                    }
                }
            }*/
        }

        void WindowSystem::Subscribe(IListener* listener)
        {
            for (auto& item : _listeners)
            {
                if (listener == item)
                    throw Common::Exception("Error subscribe listener, listener already subscribed");
            }

            _listeners.push_back(listener);
        }

        void WindowSystem::UnSubscribe(const IListener* listener)
        {
            for (auto it = _listeners.begin(); it != _listeners.end();)
            {
                if (listener == *it)
                {
                    it = _listeners.erase(it);
                    return;
                }
                ++it;
            }
        }
    }
}