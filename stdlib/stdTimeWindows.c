#ifdef _WIN32
#pragma message("Compiling stdtime for Windows")
#include <windows.h>

void stdSleep(int milliseconds) {
    Sleep(milliseconds);
}

long long stdGetTimeMS(void) {
    LARGE_INTEGER frequency;
    LARGE_INTEGER counter;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&counter);
    return (counter.QuadPart * 1000LL) / frequency.QuadPart;
}
#endif