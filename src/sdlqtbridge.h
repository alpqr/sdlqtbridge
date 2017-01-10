#ifndef SDLQTBRIDGE_H
#define SDLQTBRIDGE_H

#include <SDL.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _MSC_VER
#ifdef SDLQT_BUILD_DLL
#define SDLQT_EXPORT __declspec(dllexport)
#else
#define SDLQT_EXPORT __declspec(dllimport)
#endif
#else
#define SDLQT_EXPORT
#endif

typedef void *sdlqt_renderer;

// main thread functions
SDLQT_EXPORT int sdlqt_init(int argc, char **argv);
SDLQT_EXPORT void sdlqt_quit(void);
SDLQT_EXPORT sdlqt_renderer sdlqt_create_renderer(void);
SDLQT_EXPORT void sdlqt_delete_renderer(sdlqt_renderer renderer);
SDLQT_EXPORT SDL_GLContext sdlqt_get_context(sdlqt_renderer renderer);
SDLQT_EXPORT unsigned int sdlqt_get_texture(sdlqt_renderer renderer);
typedef void (*sdlqt_render_func)(unsigned int fbo);
SDLQT_EXPORT void sdlqt_render_offscreen(sdlqt_renderer renderer, int width, int height, sdlqt_render_func func);

#ifdef __cplusplus
}
#endif

#endif
