#ifndef ENGINE_API
#define ENGINE_API

#include <string>
#include <vector>

struct HWND__;
typedef struct HWND__* HWND;

void createEngine(int width, int height, HWND handle);
void destroyEngine();

bool runFrame();

void resize(int width, int height);

void loadCollection(std::string);
void unloadData(std::string);

std::vector<uint64_t> getAllEntities();

#endif