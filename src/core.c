// Spdx-License-Identifier: MIT

#include <SDL.h>
#include "core.h"

status_t init_core(const char* title, core_t** core)
{
    status_t status = CORE_OK;

    *core = (core_t*)calloc(1, sizeof(struct core));
    if (NULL == *core)
    {
        dbgprint("%s: error allocating memory.", __FUNCTION__);
        return CORE_ERROR;
    }

    SDL_SetMainReady();

    if (0 != SDL_Init(SDL_INIT_VIDEO))
    {
        dbgprint("Unable to initialise SDL: %s", SDL_GetError());
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
        dbgprint("Could not create window: %s", SDL_GetError());
        return CORE_ERROR;
    }

    (*core)->renderer = SDL_CreateRenderer((*core)->window, 0, SDL_RENDERER_SOFTWARE);
    if (NULL == (*core)->renderer)
    {
        dbgprint("Could not create renderer: %s", SDL_GetError());
        SDL_DestroyWindow((*core)->window);
        return CORE_ERROR;
    }
    if (0 != SDL_RenderSetIntegerScale((*core)->renderer, SDL_TRUE))
    {
        dbgprint("Could not enable integer scale: %s", SDL_GetError());
        status = CORE_WARNING;
    }

    (*core)->is_active = SDL_TRUE;

    return status;
}

status_t update_core(core_t* core)
{
    status_t  status     = CORE_OK;
    SDL_Event event;
    Uint32    delta_time = 0;

    (void)core;

    if (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_QUIT:
                status = CORE_EXIT;
                goto exit;
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                switch (event.key.keysym.sym)
                {
                    case SDLK_ESCAPE:
                    case SDLK_BACKSPACE:
                        status = CORE_EXIT;
                        goto exit;
                    case SDLK_UP:
                        core->up = (event.key.state == SDL_PRESSED);
                        break;
                    case SDLK_DOWN:
                        core->down = (event.key.state == SDL_PRESSED);
                        break;
                    case SDLK_LEFT:
                        core->left = (event.key.state == SDL_PRESSED);
                        break;
                    case SDLK_RIGHT:
                        core->right = (event.key.state == SDL_PRESSED);
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
    }

    if (core->up)
    {
        core->camera.pos_y -= 10;
    }
    if (core->down)
    {
        core->camera.pos_y += 10;
    }
    if (core->left)
    {
        core->camera.pos_x -= 10;
    }
    if (core->right)
    {
        core->camera.pos_x += 10;
    }

    if (core->camera.pos_x <= 0)
    {
        core->camera.pos_x = 0;
    }
    if (core->camera.pos_x >= core->map->width - 176)
    {
        core->camera.pos_x = core->map->width - 176;
    }
    if (core->camera.pos_y <= 0)
    {
        core->camera.pos_y = 0;
    }
    if (core->camera.pos_y >= core->map->height - 208)
    {
        core->camera.pos_y = core->map->height - 208;
    }

    core->time_b = core->time_a;
    core->time_a = SDL_GetTicks();

    if (core->time_a > core->time_b)
    {
        core->time_a = core->time_b;
    }

    delta_time                   = (Uint32)(core->time_b - core->time_a) / 1000;
    core->time_since_last_frame  = 17 - delta_time;

    // Delay? Probably not.

    if (! is_map_loaded(core))
    {
        return;
    }

    status = render_scene(core);
    if (CORE_OK != status)
    {
        goto exit;
    }
    status = draw_scene(core);

exit:
    return status;
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
        dbgprint("A map has already been loaded: unload map first.");
        return CORE_WARNING;
    }

    // Load map file and allocate required memory.

    // [1] Map.
    core->map = (map_t*)calloc(1, sizeof(struct map));
    if (! core->map)
    {
        dbgprint("%s: error allocating memory.", __FUNCTION__);
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

    // [4] Tileset.
    if (CORE_OK != load_tileset(core))
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
        dbgprint("No map has been loaded.");
        return;
    }
    core->is_map_loaded = SDL_FALSE;

    // Free up allocated memory in reverse order.

    // [4] Tileset.
    if (core->map->tileset_texture)
    {
        SDL_DestroyTexture(core->map->tileset_texture);
        core->map->tileset_texture = NULL;
    }

    // [3] Paths and file locations.
    free(core->map->path);

    // [2] Tiled map.
    unload_tiled_map(core);

    // [1] Map.
    free(core->map);
}
