#ifdef _WIN32
#pragma message("Compiling stdlibGraphics.c for Windows")
#include <windows.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "stdlibGraphics.h"

#define MAX_WINDOWS 16

typedef struct {
    HWND hwnd;
    HINSTANCE hInstance;
    bool open;
} StdWindow;

static StdWindow g_windows[MAX_WINDOWS] = {0};

static LRESULT CALLBACK StdWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CLOSE:
        DestroyWindow(hwnd);
        return 0;
    case WM_DESTROY:
        // mark closed
        for (int i = 0; i < MAX_WINDOWS; i++) {
            if (g_windows[i].hwnd == hwnd) {
                g_windows[i].open = false;
                g_windows[i].hwnd = NULL;
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

    // store in first free slot
    for (int i = 0; i < MAX_WINDOWS; i++) {
        if (!g_windows[i].open) {
            g_windows[i].hwnd = hwnd;
            g_windows[i].hInstance = hInstance;
            g_windows[i].open = true;
            return i;
        }
    }

    DestroyWindow(hwnd);
    return -1;
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

void stdDestroyWindow(int window) {
    if (window < 0 || window >= MAX_WINDOWS) return;
    if (!g_windows[window].open) return;

    DestroyWindow(g_windows[window].hwnd);
    g_windows[window].hwnd = NULL;
    g_windows[window].open = false;
}
#endif