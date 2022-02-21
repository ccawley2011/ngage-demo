// Spdx-License-Identifier: MIT

#include <SDL.h>
#include "core.h"

status_t init_core(const char* title, core_t** core)
{
    status_t status = CORE_OK;

    *core = (core_t*)calloc(1, sizeof(struct core));
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

    SDL_RenderPresent(core->renderer);
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
        free(core);
        core = NULL;
    }

    SDL_Quit();
}

status_t load_map(const char* file_name, core_t* core)
{
    char* tileset_image_source = NULL;

    if (is_map_loaded(core))
    {
        SDL_Log("A map has already been loaded: unload map first.");
        return CORE_WARNING;
    }

    // Load map file and allocate required memory.

    // [1] Map.
    core->map = (map_t*)calloc(1, sizeof(struct map));
    if (! core->map)
    {
        SDL_Log("%s: error allocating memory.", __FUNCTION__);
        return CORE_WARNING;
    }

    // [2] Tiled map.
    if (CORE_OK != load_tiled_map(file_name, core))
    {
        goto warning;
    }
    core->is_map_loaded = SDL_TRUE;

    // [3] Paths.
    if (CORE_OK != load_map_path(file_name, core))
    {
        goto warning;
    }

    core->map->height = (Sint32)((Sint32)core->map->handle->height * get_tile_height(core->map->handle));
    core->map->width  = (Sint32)((Sint32)core->map->handle->width  * get_tile_width(core->map->handle));

    return CORE_OK;
warning:
    unload_map(core);
    return CORE_WARNING;
}

void unload_map(core_t* core)
{
    if (! is_map_loaded(core))
    {
        SDL_Log("No map has been loaded.");
        return;
    }
    core->is_map_loaded = SDL_FALSE;

    // Free up allocated memory in reverse order.

    // [3] Paths and file locations.
    free(core->map->path);

    // [2] Tiled map.
    unload_tiled_map(core);

    // [1] Map.
    free(core->map);
}
