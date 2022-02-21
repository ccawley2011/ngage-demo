// Spdx-License-Identifier: MIT

#ifndef CORE_H
#define CORE_H

#include <SDL.h>
#include <tmx.h>

#ifndef FUNCTION_NAME
#  if defined(__NGAGE__)
#    define FUNCTION_NAME __FUNCTION__
#  else
#    define FUNCTION_NAME __func__
#  endif
#endif

typedef enum
{
    MAP_LAYER_BG = 0,
    MAP_LAYER_FG,
    MAP_LAYER_MAX

} map_layer_level;

typedef enum
{
    RENDER_MAP_BG = 0,
    RENDER_MAP_FG,
    RENDER_LAYER_MAX

} render_layer;

typedef struct animated_tile
{
    Sint32 dst_x;
    Sint32 dst_y;
    Sint32 animation_length;
    Sint32 current_frame;
    Sint32 gid;
    Sint32 id;

} animated_tile_t;

typedef struct camera
{
    Sint32  pos_x;
    Sint32  pos_y;
    Sint32  max_pos_x;
    Sint32  max_pos_y;

} camera_t;

typedef struct map
{
    tmx_map*         handle;
    Uint64           hash_query;
    size_t           path_length;
    char*            path;

    Sint32           width;
    Sint32           height;
    Sint32           pos_x;
    Sint32           pos_y;

    animated_tile_t* animated_tile;
    Sint32           animated_tile_fps;
    Sint32           animated_tile_index;
    Uint32           time_since_last_anim_frame;

    SDL_Texture*     animated_tile_texture;
    SDL_Texture*     layer_texture[MAP_LAYER_MAX];
    SDL_Texture*     render_target[RENDER_LAYER_MAX];
    SDL_Texture*     tileset_texture;

    SDL_bool         boolean_property;
    double           decimal_property;
    Sint32           integer_property;
    const char*      string_property;
    Uint32*          tile_properties;

} map_t;

typedef struct core
{
    SDL_Renderer* renderer;
    SDL_Window*   window;
    map_t*        map;
    struct camera camera;
    SDL_bool      is_active;
    SDL_bool      is_map_loaded;
    Uint32        time_since_last_frame;
    Uint32        time_a;
    Uint32        time_b;

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
