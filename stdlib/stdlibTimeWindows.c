#ifdef _WIN32
#include "stdlibTime.h"
#include <windows.h>

void stdSleep(int milliseconds) {
    Sleep(milliseconds);
}
#endif