# Delta Standard Library
## Modules
### Standard Graphics
```
int stdOpenWindow(char *title, int width, int height); // Opens a window and returns the index for it
bool stdIsWindowOpen(int window); // Checks if window is open
void stdKeepWindowOpen(int window); // Keeps the window open in a while loop
void stdUpdateWindow(int window); // Update the window and poll events
void stdDestroyWindow(int window); // Destroys the window
```