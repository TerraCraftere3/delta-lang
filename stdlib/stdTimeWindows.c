#ifdef _WIN32
#pragma message("Compiling stdTime.c for Windows")
#include "stdTime.h"
#include <windows.h>

void stdSleep(int milliseconds) {
    Sleep(milliseconds);
}
#endif