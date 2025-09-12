# Delta Standard Library
## Modules
### Standard Graphics
```
int stdOpenWindow(char *title, int width, int height);

bool stdIsWindowOpen(int window);
void stdKeepWindowOpen(int window);
void stdUpdateWindow(int window);

void stdSetWindowTitle(char *title);
void stdSetWindowSize(int width, int height);
void stdGetWindowSize(int* width, int* height);

void stdDestroyWindow(int window);
void stdCloseWindow(int window);
void stdMaximizeWindow(int window);
void stdMinimizeWindow(int window);

bool stdIsKeyPressed(int window, char ascii_code);
void stdClearWindow(int window, int r, int g, int b);
```

### Standard Time
```
void stdSleep(int milliseconds);
```