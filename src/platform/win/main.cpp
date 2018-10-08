#include <windows.h>
#include "core.h"

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include "crtdbg.h"
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif

HWND hWnd;

#ifdef RENDER_GL
    HDC   hDC;
    HGLRC hRC;

    void ContextCreate() {
        hDC = GetDC(hWnd);

        PIXELFORMATDESCRIPTOR pfd;
        memset(&pfd, 0, sizeof(pfd));
        pfd.nSize        = sizeof(pfd);
        pfd.nVersion     = 1;
        pfd.dwFlags      = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
        pfd.cColorBits   = 32;
        pfd.cRedBits     = 8;
        pfd.cGreenBits   = 8;
        pfd.cBlueBits    = 8;
        pfd.cAlphaBits   = 8;
        pfd.cDepthBits   = 24;
        pfd.cStencilBits = 8;

        int format = ChoosePixelFormat(hDC, &pfd);
        SetPixelFormat(hDC, format, &pfd);
        hRC = wglCreateContext(hDC);
        wglMakeCurrent(hDC, hRC);
    }

    void ContextDelete() {
        wglMakeCurrent(0, 0);
        wglDeleteContext(hRC);
        ReleaseDC(hWnd, hDC);
    }

    void ContextResize() {}

    void ContextSwap() {
        SwapBuffers(hDC);
    }
#endif

// Input
Input::InputKey keyToInputKey(int code) {
    int codes[] = {
            113, 114, 111, 116, 65, 23, 36, 9, 50, 37, 64,
            19, 10, 11, 12, 13, 14, 15, 16, 17, 18,
            38, 56, 54, 40, 26, 41, 42, 43, 31, 44, 45, 46, 58,
            57, 32, 33, 24, 27, 39, 28, 30, 55, 25, 53, 29, 52,
    };

    for (int i = 0; i < sizeof(codes) / sizeof(codes[0]); i++)
        if (codes[i] == code)
            return (Input::InputKey)(Input::ikLeft + i);
    return Input::ikNone;
}

Input::InputKey mouseToInputKey(int btn) {
    switch (btn) {
        case 1 : return Input::ikMouseL;
        case 2 : return Input::ikMouseM;
        case 3 : return Input::ikMouseR;
    }
    return Input::ikNone;
}

static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        // window
        case WM_ACTIVATE :
            Input::reset();
            break;
        case WM_SIZE:
            Core::width  = LOWORD(lParam);
            Core::height = HIWORD(lParam);
            ContextResize();
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
            // keyboard
        case WM_CHAR       :
        case WM_SYSCHAR    :
            break;
        case WM_KEYDOWN    :
        case WM_KEYUP      :
        case WM_SYSKEYDOWN :
        case WM_SYSKEYUP   :
            if (msg == WM_SYSKEYDOWN && wParam == VK_RETURN) { // switch to fullscreen or window
                static WINDOWPLACEMENT pLast;
                DWORD style = GetWindowLong(hWnd, GWL_STYLE);
                if (style & WS_OVERLAPPEDWINDOW) {
                    MONITORINFO mInfo = { sizeof(mInfo) };
                    if (GetWindowPlacement(hWnd, &pLast) && GetMonitorInfo(MonitorFromWindow(hWnd, MONITOR_DEFAULTTOPRIMARY), &mInfo)) {
                        RECT &r = mInfo.rcMonitor;
                        SetWindowLong(hWnd, GWL_STYLE, style & ~WS_OVERLAPPEDWINDOW);
                        MoveWindow(hWnd, r.left, r.top, r.right - r.left, r.bottom - r.top, FALSE);
                    }
                } else {
                    SetWindowLong(hWnd, GWL_STYLE, style | WS_OVERLAPPEDWINDOW);
                    SetWindowPlacement(hWnd, &pLast);
                }
                break;
            }
            Input::setDown(keyToInputKey(wParam), msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN);
            break;
            // mouse
        case WM_LBUTTONDOWN   :
        case WM_LBUTTONUP     :
        case WM_LBUTTONDBLCLK :
        case WM_RBUTTONDOWN   :
        case WM_RBUTTONUP     :
        case WM_RBUTTONDBLCLK :
        case WM_MBUTTONDOWN   :
        case WM_MBUTTONUP     :
        case WM_MBUTTONDBLCLK : {
            if ((GetMessageExtraInfo() & 0xFFFFFF00) == 0xFF515700) break;
            Input::InputKey key = mouseToInputKey(msg);
            Input::setPos(key, vec2((float)(short)LOWORD(lParam), (float)(short)HIWORD(lParam)));
            bool down = msg != WM_LBUTTONUP && msg != WM_RBUTTONUP && msg != WM_MBUTTONUP;
            Input::setDown(key, down);
            if (down)
                SetCapture(hWnd);
            else
                ReleaseCapture();
            break;
        }
        case WM_MOUSEMOVE :
            if ((GetMessageExtraInfo() & 0xFFFFFF00) == 0xFF515700) break;
            Input::setPos(Input::ikMouseL, vec2((float)(short)LOWORD(lParam), (float)(short)HIWORD(lParam)));
            break;
        default :
            return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return 0;
}

int main(int argc, char** argv) {

#ifdef _DEBUG
    _CrtMemState _msBegin, _msEnd, _msDiff;
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
    _CrtMemCheckpoint(&_msBegin);
#endif

    RECT r = { 0, 0, 1280, 720 };
    AdjustWindowRect(&r, WS_OVERLAPPEDWINDOW, false);

    hWnd = CreateWindow("static", "OpenDemo", WS_OVERLAPPEDWINDOW, 0, 0, r.right - r.left, r.bottom - r.top, 0, 0, 0, 0);

    ContextCreate();

    SetWindowLong(hWnd, GWL_WNDPROC, (LONG)&WndProc);
    ShowWindow(hWnd, SW_SHOWDEFAULT);

    MSG msg;
    while (!Core::isQuit) {
        if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                Core::quit();
        }
        //if (Game::update()) {
         //   Game::render();
          //  Core::waitVBlank();
            ContextSwap();
        //}
    }

    //Game::deinit();

    ContextDelete();

    DestroyWindow(hWnd);

#ifdef _DEBUG
    _CrtMemCheckpoint(&_msEnd);

    if (_CrtMemDifference(&_msDiff, &_msBegin, &_msEnd) > 0) {
        _CrtDumpMemoryLeaks();
        system("pause");
    }
#endif
}