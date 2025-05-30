#ifndef ENGINE_API
#define ENGINE_API

struct HWND__;
typedef struct HWND__* HWND;

HWND createEngine(int width, int height);

bool runFrame();

void destroyEngine();

#endif