// Spdx-License-Identifier: MIT

#include <stdlib.h>
#include <SDL.h>

int main(int argc, char *argv[])
{
    int           status     = EXIT_SUCCESS;
    SDL_bool      is_running = SDL_TRUE;
    SDL_Renderer *renderer;
    SDL_Surface  *tempsf;
    SDL_Surface  *screen;
    SDL_Texture  *splash;
    SDL_Window   *window;

    (void)argc;
    (void)argv;

    SDL_SetMainReady();

    if (0 != SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_Log("Unable to initialise SDL: %s", SDL_GetError());
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
        SDL_Log("Could not create window: %s", SDL_GetError());
        status = EXIT_FAILURE;
        goto error;
    }

    renderer = SDL_CreateRenderer(window, 0, SDL_RENDERER_SOFTWARE);
    if (NULL == renderer)
    {
        SDL_Log("Could not create: %s", SDL_GetError());
        status = EXIT_FAILURE;
        goto error;
    }
    if (0 != SDL_RenderSetIntegerScale(renderer, SDL_TRUE))
    {
        SDL_Log("Could not enable integer scale: %s", SDL_GetError());
    }

    tempsf = SDL_LoadBMP("E:\\splash.bmp");
    if (NULL == tempsf)
    {
        SDL_Log("Failed to load image: %s", SDL_GetError());
        status = EXIT_FAILURE;
        goto error;
    }

    splash = SDL_CreateTextureFromSurface(renderer, tempsf);
    if (NULL == splash)
    {
        SDL_Log("Failed to create texture from surface: %s", SDL_GetError());
        status = EXIT_FAILURE;
        goto error;
    }
    SDL_FreeSurface(tempsf);

    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
    SDL_RenderClear(renderer);

    screen = SDL_GetWindowSurface(window);
    if (NULL == screen)
    {
        SDL_Log("Could not get window surface: %s", SDL_GetError());
        status = EXIT_FAILURE;
        goto error;
    }

    while(is_running)
    {
        SDL_Event        event;
        SDL_RendererFlip flip;
        double           angle;

        if (SDL_PollEvent(&event))
        {
            switch (event.key.keysym.sym)
            {
                case SDLK_5:
                    flip = SDL_FLIP_NONE;
                    break;
                case SDLK_7:
                    flip = SDL_FLIP_HORIZONTAL;
                    break;
                case SDLK_UP:
                    angle = 0.f;
                    break;
                case SDLK_DOWN:
                    angle = 180.f;
                    break;
                case SDLK_LEFT:
                    angle = 270.f;
                    break;
                case SDLK_RIGHT:
                    angle = 90.f;
                    break;
                case SDLK_BACKSPACE:
                    is_running = SDL_FALSE;
                    break;
                default:
                    angle = 0.f;
                    break;
            }
        }

        if (0 != SDL_RenderCopyEx(renderer, splash, NULL, NULL, angle, NULL, flip))
        {
            SDL_Log("Could not render texture: %s", SDL_GetError());
            goto error;
        }
        SDL_RenderPresent(renderer);
    }

error:
    if (screen)
    {
        SDL_FreeSurface(screen);
    }

    if (splash)
    {
        SDL_DestroyTexture(splash);
    }

    if (renderer)
    {
        SDL_DestroyRenderer(renderer);
    }

    if (window)
    {
        SDL_DestroyWindow(window);
    }

    SDL_Quit();
    return status;
}
