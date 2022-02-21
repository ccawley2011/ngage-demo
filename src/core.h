// Spdx-License-Identifier: MIT

#ifndef CORE_H
#define CORE_H

#include <SDL.h>
#include <tmx.h>

typedef struct map
{
    tmx_map*    handle;
    Uint64      hash_query;
    size_t      path_length;
    char*       path;

    Sint32      width;
    Sint32      height;

    SDL_bool    boolean_property;
    double      decimal_property;
    Sint32      integer_property;
    const char* string_property;
    Uint32*     tile_properties;

} map_t;

typedef struct core
{
    SDL_Renderer* renderer;
    SDL_Window*   window;
    map_t*        map;
    SDL_bool      is_active;
    SDL_bool      is_map_loaded;

} core_t;

typedef enum status
{
    CORE_OK = 0,
    CORE_WARNING,
    CORE_ERROR,
    CORE_EXIT

} status_t;

status_t init_core(const char* title, core_t** core);
status_t update_core(core_t* core);
void     free_core(core_t *core);
status_t load_map(const char* file_name, core_t* core);
void     unload_map(core_t* core);

#endif /* CORE_H */
