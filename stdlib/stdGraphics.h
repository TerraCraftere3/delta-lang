#ifndef STD_GRAPHICS
#define STD_GRAPHICS

#include <stdbool.h>

int stdOpenWindow(char *title, int width, int height);
bool stdIsWindowOpen(int window);
void stdKeepWindowOpen(int window);
void stdUpdateWindow(int window);
void stdSetWindowTitle(int window, char *title);
void stdSetWindowSize(int window, int width, int height);
void stdGetWindowSize(int window, int* width, int* height);
void stdDestroyWindow(int window);
void stdCloseWindow(int window);
void stdMaximizeWindow(int window);
void stdMinimizeWindow(int window);

// OpenGL context management
bool stdCreateOpenGLContext(int window);
bool stdMakeContextCurrent(int window);
void stdSwapBuffers(int window);

// Input handling
bool stdIsKeyPressed(int window, char ascii_code);
#endif