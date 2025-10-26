#ifndef ENGINE_API
#define ENGINE_API

#include <cstdint>
#include <string>
#include <vector>
#include <typeindex>

struct HWND__;
typedef struct HWND__* HWND;

void createEngine(int width, int height, HWND handle);
void destroyEngine();

bool runFrame();

void resize(int width, int height);

void loadCollection(std::string);
void unloadData(std::string);

std::vector<uint64_t> getAllEntities();

void* getComplement(uint64_t entity, uint8_t type);

uint8_t findComplement(std::type_index type);

#endif