#ifndef ENGINE_API
#define ENGINE_API

#ifdef __cplusplus
extern "C" {
#endif

// typedef struct Window Window;
// typedef struct Renderer Renderer;
// typedef struct AssetSystem AssetSystem;
// typedef struct Engine Engine;

typedef void* WindowHandle;

WindowHandle createEngine();


#ifdef __cplusplus
}
#endif

#endif