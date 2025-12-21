// Wrapper functions for standard C library functions
// These allow Delta's namespaced functions to call the actual C runtime functions

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>

// stdio wrappers
int std_io_printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    int result = vprintf(format, args);
    va_end(args);
    return result;
}

int std_io_fprintf(FILE* stream, const char* format, ...) {
    va_list args;
    va_start(args, format);
    int result = vfprintf(stream, format, args);
    va_end(args);
    return result;
}

int std_io_sprintf(char* str, const char* format, ...) {
    va_list args;
    va_start(args, format);
    int result = vsprintf(str, format, args);
    va_end(args);
    return result;
}

int std_io_scanf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    int result = vscanf(format, args);
    va_end(args);
    return result;
}

void* std_io_fopen(const char* filename, const char* mode) {
    return fopen(filename, mode);
}

int std_io_fclose(void* stream) {
    return fclose((FILE*)stream);
}

size_t std_io_fread(void* ptr, size_t size, size_t nmemb, void* stream) {
    return fread(ptr, size, nmemb, (FILE*)stream);
}

size_t std_io_fwrite(const void* ptr, size_t size, size_t nmemb, void* stream) {
    return fwrite(ptr, size, nmemb, (FILE*)stream);
}

int std_io_fseek(void* stream, size_t offset, int whence) {
    return fseek((FILE*)stream, (long)offset, whence);
}

long long std_io_ftell(void* stream) {
    return (long long)ftell((FILE*)stream);
}

int std_io_execute(char* cmd) {
    return system(cmd);
}