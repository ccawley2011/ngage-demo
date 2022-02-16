// Spdx-License-Identifier: MIT

#include <stdlib.h>
#include <SDL.h>
#include <dbgprint.h>

int main(int argc, char *argv[])
{
    int          status     = EXIT_SUCCESS;
    SDL_bool     is_running = SDL_TRUE;
    SDL_Surface *splash;
    SDL_Surface *screen;
    SDL_Window  *window;

    (void)argc;
    (void)argv;

    SDL_SetMainReady();

    if (0 != SDL_Init(SDL_INIT_VIDEO))
    {
        dbgprint("Unable to initialise SDL: %s", SDL_GetError());
        status = EXIT_FAILURE;
        goto error;
    }

    window = SDL_CreateWindow(
        "demo",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        176, 208,
        SDL_WINDOW_FULLSCREEN);

    if (NULL == window)
    {
        dbgprint("Could not create window: %s", SDL_GetError());
        status = EXIT_FAILURE;
        goto error;
    }

    screen = SDL_GetWindowSurface(window);
    if (NULL == screen)
    {
        dbgprint("Could not get window surface: %s", SDL_GetError());
        status = EXIT_FAILURE;
        goto error;
    }

    splash = SDL_LoadBMP("E:\\splash.bmp");
    if (NULL == splash)
    {
        dbgprint("Failed to load image: %s", SDL_GetError());
        status = EXIT_FAILURE;
        goto error;
    }

    while(is_running)
    {
        SDL_Event event;

        if (SDL_PollEvent(&event))
        {
            switch (event.key.keysym.sym)
            {
                case SDLK_ESCAPE:
                    is_running = SDL_FALSE;
                    break;
                default:
                    break;
            }
        }

        SDL_BlitSurface(splash, NULL, screen, NULL);
        SDL_UpdateWindowSurface(window);
    }

error:
    SDL_FreeSurface(splash);
    SDL_FreeSurface(screen);

    if (window)
    {
        SDL_DestroyWindow(window);
    }

    SDL_Quit();
    return status;
}
