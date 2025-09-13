#ifdef _WIN32
#pragma message("Compiling stdGraphics.c for Windows")
#include <windows.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <GL/gl.h>
#include "stdGraphics.h"

#define MAX_WINDOWS 16

typedef struct {
    HWND hwnd;
    HINSTANCE hInstance;
    HDC hdc;
    HGLRC hglrc;
    bool open;
    bool has_context;
} StdWindow;

static StdWindow g_windows[MAX_WINDOWS] = {0};

static LRESULT CALLBACK StdWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CLOSE:
        DestroyWindow(hwnd);
        return 0;
    case WM_DESTROY:
        // Clean up OpenGL context and mark closed
        for (int i = 0; i < MAX_WINDOWS; i++) {
            if (g_windows[i].hwnd == hwnd) {
                if (g_windows[i].has_context) {
                    wglMakeCurrent(NULL, NULL);
                    wglDeleteContext(g_windows[i].hglrc);
                    ReleaseDC(hwnd, g_windows[i].hdc);
                }
                g_windows[i].open = false;
                g_windows[i].has_context = false;
                g_windows[i].hwnd = NULL;
                g_windows[i].hdc = NULL;
                g_windows[i].hglrc = NULL;
                break;
            }
        }
        return 0;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}

int stdOpenWindow(char *title, int width, int height) {
    HINSTANCE hInstance = GetModuleHandle(NULL);

    WNDCLASS wc = {0};
    wc.lpfnWndProc = StdWndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "StdGraphicsWindowClass";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    static bool class_registered = false;
    if (!class_registered) {
        if (!RegisterClass(&wc)) {
            MessageBox(NULL, "Failed to register window class!", "Error", MB_ICONERROR);
            return -1;
        }
        class_registered = true;
    }

    HWND hwnd = CreateWindow(
        wc.lpszClassName,
        title,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        width, height,
        NULL, NULL, hInstance, NULL);

    if (!hwnd) {
        MessageBox(NULL, "Failed to create window!", "Error", MB_ICONERROR);
        return -1;
    }

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    // Store in first free slot
    for (int i = 0; i < MAX_WINDOWS; i++) {
        if (!g_windows[i].open) {
            g_windows[i].hwnd = hwnd;
            g_windows[i].hInstance = hInstance;
            g_windows[i].open = true;
            g_windows[i].has_context = false;
            g_windows[i].hdc = NULL;
            g_windows[i].hglrc = NULL;
            return i;
        }
    }

    DestroyWindow(hwnd);
    return -1;
}

bool stdCreateOpenGLContext(int window) {
    if (window < 0 || window >= MAX_WINDOWS) return false;
    if (!g_windows[window].open) return false;
    if (g_windows[window].has_context) return true; // Already has context

    HWND hwnd = g_windows[window].hwnd;
    HDC hdc = GetDC(hwnd);
    
    if (!hdc) {
        return false;
    }

    PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA,
        32,                // Color depth
        0, 0, 0, 0, 0, 0,  // Color bits ignored
        0,                 // No alpha buffer
        0,                 // Shift bit ignored
        0,                 // No accumulation buffer
        0, 0, 0, 0,        // Accumulation bits ignored
        24,                // 24-bit z-buffer
        8,                 // 8-bit stencil buffer
        0,                 // No auxiliary buffer
        PFD_MAIN_PLANE,    // Main layer
        0,                 // Reserved
        0, 0, 0            // Layer masks ignored
    };

    int pixelFormat = ChoosePixelFormat(hdc, &pfd);
    if (!pixelFormat) {
        ReleaseDC(hwnd, hdc);
        return false;
    }

    if (!SetPixelFormat(hdc, pixelFormat, &pfd)) {
        ReleaseDC(hwnd, hdc);
        return false;
    }

    HGLRC hglrc = wglCreateContext(hdc);
    if (!hglrc) {
        ReleaseDC(hwnd, hdc);
        return false;
    }

    if (!wglMakeCurrent(hdc, hglrc)) {
        wglDeleteContext(hglrc);
        ReleaseDC(hwnd, hdc);
        return false;
    }

    g_windows[window].hdc = hdc;
    g_windows[window].hglrc = hglrc;
    g_windows[window].has_context = true;

    return true;
}

bool stdMakeContextCurrent(int window) {
    if (window < 0 || window >= MAX_WINDOWS) return false;
    if (!g_windows[window].open) return false;
    if (!g_windows[window].has_context) return false;

    return wglMakeCurrent(g_windows[window].hdc, g_windows[window].hglrc);
}

void stdSwapBuffers(int window) {
    if (window < 0 || window >= MAX_WINDOWS) return;
    if (!g_windows[window].open) return;
    if (!g_windows[window].has_context) return;

    SwapBuffers(g_windows[window].hdc);
}

bool stdIsWindowOpen(int window) {
    if (window < 0 || window >= MAX_WINDOWS) return false;
    return g_windows[window].open;
}

void stdKeepWindowOpen(int window) {
    while(stdIsWindowOpen(window)){
        stdUpdateWindow(window);
    }
}

void stdUpdateWindow(int window) {
    if (window < 0 || window >= MAX_WINDOWS) return;
    if (!g_windows[window].open) return;

    MSG msg;
    while (PeekMessage(&msg, g_windows[window].hwnd, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void stdSetWindowTitle(int window, char *title) {
    if (window < 0 || window >= MAX_WINDOWS) return;
    if (!g_windows[window].open) return;

    SetWindowText(g_windows[window].hwnd, title);
}

void stdSetWindowSize(int window, int width, int height) {
    if (window < 0 || window >= MAX_WINDOWS) return;
    if (!g_windows[window].open) return;

    RECT rect;
    HWND hwnd = g_windows[window].hwnd;

    DWORD style = GetWindowLong(hwnd, GWL_STYLE);
    RECT wr = {0, 0, width, height};
    AdjustWindowRect(&wr, style, FALSE);

    SetWindowPos(hwnd, NULL, 0, 0, wr.right - wr.left, wr.bottom - wr.top, SWP_NOMOVE | SWP_NOZORDER);
}

void stdGetWindowSize(int window, int* width, int* height) {
    if (window < 0 || window >= MAX_WINDOWS) return;
    if (!g_windows[window].open) {
        *width = 0;
        *height = 0;
        return;
    }

    RECT rect;
    HWND hwnd = g_windows[window].hwnd;
    GetClientRect(hwnd, &rect);

    *width = rect.right - rect.left;
    *height = rect.bottom - rect.top;
}

void stdDestroyWindow(int window) {
    if (window < 0 || window >= MAX_WINDOWS) return;
    if (!g_windows[window].open) return;

    if (g_windows[window].has_context) {
        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(g_windows[window].hglrc);
        ReleaseDC(g_windows[window].hwnd, g_windows[window].hdc);
        g_windows[window].has_context = false;
    }

    DestroyWindow(g_windows[window].hwnd);
    g_windows[window].hwnd = NULL;
    g_windows[window].open = false;
}

void stdCloseWindow(int window) {
    if (window < 0 || window >= MAX_WINDOWS) return;
    if (!g_windows[window].open) return;

    PostMessage(g_windows[window].hwnd, WM_CLOSE, 0, 0);
}

void stdMaximizeWindow(int window) {
    if (window < 0 || window >= MAX_WINDOWS) return;
    if (!g_windows[window].open) return;

    ShowWindow(g_windows[window].hwnd, SW_MAXIMIZE);
}

void stdMinimizeWindow(int window) {
    if (window < 0 || window >= MAX_WINDOWS) return;
    if (!g_windows[window].open) return;

    ShowWindow(g_windows[window].hwnd, SW_MINIMIZE);
}

bool stdIsKeyPressed(int window, char ascii_code) {
    if (window < 0 || window >= MAX_WINDOWS) return false;
    if (!g_windows[window].open) return false;

    SHORT state = GetAsyncKeyState((int)ascii_code);
    return (state & 0x8000) != 0;
}

#endif