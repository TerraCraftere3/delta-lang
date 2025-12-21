// Wrapper functions for standard C library functions
// These allow Delta's namespaced functions to call the actual C runtime functions

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>


// math wrappers
double std_sin(double x) {
    return sin(x);
}

double std_cos(double x) {
    return cos(x);
}

double std_tan(double x) {
    return tan(x);
}

double std_sqrt(double x) {
    return sqrt(x);
}

double std_pow(double x, double y) {
    return pow(x, y);
}

double std_fabs(double x) {
    return fabs(x);
}

// string wrappers
long long std_strlen(const char* s) {
    return (long long)strlen(s);
}

char* std_strcpy(char* dest, const char* src) {
    return strcpy(dest, src);
}

char* std_strncpy(char* dest, const char* src, size_t n) {
    return strncpy(dest, src, n);
}

char* std_strcat(char* dest, const char* src) {
    return strcat(dest, src);
}

int std_strcmp(const char* s1, const char* s2) {
    return strcmp(s1, s2);
}

int std_strncmp(const char* s1, const char* s2, size_t n) {
    return strncmp(s1, s2, n);
}

char* std_strchr(const char* s, int c) {
    return strchr(s, c);
}

char* std_strstr(const char* haystack, const char* needle) {
    return strstr(haystack, needle);
}

// memory wrappers
void* std_malloc(long long size) {
    return malloc((size_t)size);
}

void* std_calloc(long long nmemb, long long size) {
    return calloc((size_t)nmemb, (size_t)size);
}

void* std_realloc(void* ptr, long long size) {
    return realloc(ptr, (size_t)size);
}

void std_free(void* ptr) {
    free(ptr);
}

// system wrappers
void std_exit(int status) {
    exit(status);
}

int std_system(const char* command) {
    return system(command);
}

char* std_getenv(const char* name) {
    return getenv(name);
}

int std_putenv(char* string) {
    return putenv(string);
}
