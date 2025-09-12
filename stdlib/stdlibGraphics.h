#ifndef STD_GRAPHICS
#define STD_GRAPHICS

#include <stdbool.h>

int stdOpenWindow(char *title, int width, int height);
bool stdIsWindowOpen(int window);
void stdKeepWindowOpen(int window);
void stdUpdateWindow(int window);
void stdDestroyWindow(int window);

#endif