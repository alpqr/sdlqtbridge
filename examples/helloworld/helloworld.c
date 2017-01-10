#include <stdio.h>
#include <SDL.h>
#include <windows.h>
#include <GL/gl.h>
#include "../../src/sdlqtbridge.h"

void render(void)
{
    static GLfloat f = 0;
    glClearColor(1, f, 0, 1);
    f += 0.01f;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void render2(unsigned int fbo)
{
    printf("%d\n", fbo);
    glClearColor(0, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

int main(int argc, char **argv)
{
    SDL_Window *wnd = NULL;
    SDL_GLContext ctx = NULL;
    const char *version;
    sdlqt_renderer *qtr = NULL;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        goto teardown;
    }

    if (sdlqt_init(argc, argv) < 0) {
        fprintf(stderr, "sdlqt_init failed\n");
        goto teardown;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

    wnd = SDL_CreateWindow("test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                           1024, 768, SDL_WINDOW_OPENGL);
    if (!wnd) {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        goto teardown;
    }

    qtr = sdlqt_create_renderer();
    if (!qtr) {
        fprintf(stderr, "sdlqt_create_renderer failed\n");
        goto teardown;
    }

    ctx = sdlqt_get_context(qtr);
    if (!ctx) {
        fprintf(stderr, "sdlqt_get_context failed\n");
        goto teardown;
    }

    if (SDL_GL_MakeCurrent(wnd, ctx) < 0) {
        fprintf(stderr, "SDL_GL_MakeCurrent failed: %s\n", SDL_GetError());
        goto teardown;
    }

    version = glGetString(GL_VERSION);
    printf("GL_VERSION: %s\n", version);

    SDL_GL_SetSwapInterval(1);

    for ( ; ; ) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                goto teardown;
            default:
                break;
            }
        }
        sdlqt_render_offscreen(qtr, 1024, 768, render2);
        SDL_GL_MakeCurrent(wnd, ctx);
        render();
        SDL_GL_SwapWindow(wnd);
    }

teardown:
    if (qtr)
        sdlqt_delete_renderer(qtr);
    sdlqt_quit();
    if (ctx)
        SDL_GL_DeleteContext(ctx);
    if (wnd)
        SDL_DestroyWindow(wnd);
    SDL_Quit();

    return 0;
}
