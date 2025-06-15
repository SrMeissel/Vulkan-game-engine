#ifndef ENGINE_API
#define ENGINE_API

struct HWND__;
typedef struct HWND__* HWND;

void createEngine(int width, int height, HWND handle);

bool runFrame();

void resize(int width, int height);

void destroyEngine();

#endif