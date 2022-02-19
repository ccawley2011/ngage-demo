// Spdx-License-Identifier: MIT

#include <SDL.h>
#include "core.h"

status_t init_core(const char* title, core_t** core)
{
    status_t status = CORE_OK;

    *core = (core_t*)SDL_calloc(1, sizeof(struct core));
    if (NULL == *core)
    {
        SDL_Log("%s: error allocating memory.", __FUNCTION__);
        return CORE_ERROR;
    }

    SDL_SetMainReady();

    if (0 != SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_Log("Unable to initialise SDL: %s", SDL_GetError());
        return CORE_ERROR;
    }

    (*core)->window = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        176, 208,
        SDL_WINDOW_FULLSCREEN);
    if (NULL == (*core)->window)
    {
        SDL_Log("Could not create window: %s", SDL_GetError());
        return CORE_ERROR;
    }

    (*core)->renderer = SDL_CreateRenderer((*core)->window, 0, SDL_RENDERER_SOFTWARE);
    if (NULL == (*core)->renderer)
    {
        SDL_Log("Could not create renderer: %s", SDL_GetError());
        SDL_DestroyWindow((*core)->window);
        return CORE_ERROR;
    }
    if (0 != SDL_RenderSetIntegerScale((*core)->renderer, SDL_TRUE))
    {
        SDL_Log("Could not enable integer scale: %s", SDL_GetError());
        status = CORE_WARNING;
    }

    (*core)->is_active = SDL_TRUE;

    return status;
}

status_t update_core(core_t* core)
{
    SDL_Event event;

    (void)core;

    if (SDL_PollEvent(&event))
    {
        switch (event.key.keysym.sym)
        {
            case SDLK_BACKSPACE:
                return CORE_EXIT;
            default:
                break;
        }
    }

    return CORE_OK;
}

void free_core(core_t *core)
{
    if (core->window)
    {
        SDL_DestroyWindow(core->window);
    }

    if (core->renderer)
    {
        SDL_DestroyRenderer(core->renderer);
    }

    if (core)
    {
        SDL_free(core);
        core = NULL;
    }

    SDL_Quit();
}
