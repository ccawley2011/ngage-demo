// SPDX-License-Identifier: MIT

#include <stb_sprintf.h> // libxml2

#include <SDL.h>
#include <tmx.h>
#include "core.h"

#define H_is_in_foreground 0xdba8c1d8d43eb8f1

static void tmxlib_store_property(tmx_property* property, void* core);

Sint32 get_first_gid(tmx_map* tiled_map)
{
    return (Sint32)tiled_map->ts_head->firstgid;
}

tmx_layer* get_head_layer(tmx_map* tiled_map)
{
    return tiled_map->ly_head;
}

SDL_bool is_tiled_layer_of_type(const enum tmx_layer_type tiled_type, tmx_layer* tiled_layer)
{
    if (tiled_type == tiled_layer->type)
    {
        return SDL_TRUE;
    }

    return SDL_FALSE;
}

tmx_object* get_head_object(tmx_layer* tiled_layer, core_t* core)
{
    if (is_tiled_layer_of_type(L_OBJGR, tiled_layer))
    {
        return tiled_layer->content.objgr->head;
    }

    return NULL;
}

tmx_tileset* get_head_tileset(tmx_map* tiled_map)
{
    Sint32 first_gid = get_first_gid(tiled_map);
    return tiled_map->tiles[first_gid]->tileset;
}

Sint32* get_layer_content(tmx_layer* tiled_layer)
{
    return (Sint32*)tiled_layer->content.gids;
}

const char* get_layer_name(tmx_layer* tiled_layer)
{
    return tiled_layer->name;
}

Sint32 get_layer_property_count(tmx_layer* tiled_layer)
{
    (void)tiled_layer;
    return 0;
}

Sint32 get_local_id(Sint32 gid, tmx_map* tiled_map)
{
    (void)tiled_map;
    return gid;
}

Sint32 get_map_property_count(tmx_map* tiled_map)
{
    (void)tiled_map;
    return 0;
}

Sint32 get_next_animated_tile_id(Sint32 gid, Sint32 current_frame, tmx_map* tiled_map)
{
    return (Sint32)tiled_map->tiles[gid]->animation[current_frame].tile_id;

    return 0;
}

const char* get_object_name(tmx_object* tiled_object)
{
    return tiled_object->name;
}

Sint32 get_object_property_count(tmx_object* tiled_object)
{
    (void)tiled_object;
    return 0;
}

const char* get_object_type_name(tmx_object* tiled_object)
{
    return tiled_object->type;
}

Sint32 get_tile_height(tmx_map* tiled_map)
{
    Sint32 first_gid = get_first_gid(tiled_map);
    return (Sint32)tiled_map->tiles[first_gid]->tileset->tile_height;
}

void get_tile_position(Sint32 gid, Sint32* pos_x, Sint32* pos_y, tmx_map* tiled_map)
{
    *pos_x = (Sint32)tiled_map->tiles[gid]->ul_x;
    *pos_y = (Sint32)tiled_map->tiles[gid]->ul_y;
}

Sint32 get_tile_property_count(tmx_tile* tiled_tile)
{
    (void)tiled_tile;
    return 0;
}

Sint32 get_tile_width(tmx_map* tiled_map)
{
    Sint32 first_gid = get_first_gid(tiled_map);
    return (Sint32)tiled_map->tiles[first_gid]->tileset->tile_width;
}

void set_tileset_path(char* path_name, Sint32 path_length, core_t* core)
{
    Sint32 first_gid      = get_first_gid(core->map->handle);
    char*  ts_path        = core->map->handle->ts_head->source;
    size_t ts_path_length = strlen(ts_path);

    /* The tileset image source is stored relatively to the tileset
     * file but because we only know the location of the tileset
     * file relatively to the map file, we need to adjust the path
     * accordingly.  It's a hack, but it works.
     */

    SDL_strlcpy(ts_path, core->map->handle->ts_head->source, ts_path_length + 1);
    stbsp_snprintf(path_name, (Sint32)path_length, "%s%s%s",
                   core->map->path,
                   ts_path,
                   core->map->handle->tiles[first_gid]->tileset->image->source);
}

Sint32 get_tileset_path_length(core_t* core)
{
    Sint32 path_length    = 0;
    Sint32 first_gid      = get_first_gid(core->map->handle);
    size_t ts_path_length = strlen(core->map->handle->ts_head->source);

    path_length += (Sint32)SDL_strlen(core->map->path);
    path_length += strlen(core->map->handle->tiles[first_gid]->tileset->image->source);
    path_length += (Sint32)ts_path_length + 1;

    return path_length;
}

SDL_bool is_gid_valid(Sint32 gid, tmx_map* tiled_map)
{
    if (tiled_map->tiles[gid])
    {
        return SDL_TRUE;
    }

    return SDL_FALSE;
}

SDL_bool is_tile_animated(Sint32 gid, Sint32* animation_length, Sint32* id, tmx_map* tiled_map)
{
    Sint32 local_id = get_local_id(gid, tiled_map);

    if (tiled_map->tiles[local_id])
    {
        if (tiled_map->tiles[local_id]->animation)
        {
            if (animation_length)
            {
                *animation_length = (Sint32)tiled_map->tiles[local_id]->animation_len;
            }
            if (id)
            {
                *id = (Sint32)tiled_map->tiles[local_id]->animation[0].tile_id;
            }
            return SDL_TRUE;
        }
    }

    return SDL_FALSE;
}

/* djb2 by Dan Bernstein
 * http://www.cse.yorku.ca/~oz/hash.html
 */
Uint64 generate_hash(const unsigned char* name)
{
    Uint64 hash = 5381;
    Uint32 c;

    while ((c = *name++))
    {
        hash = ((hash << 5) + hash) + c;
    }

    return hash;
}

void load_property(const Uint64 name_hash, tmx_property* properties, Sint32 property_count, core_t* core)
{
    (void)property_count;
    core->map->hash_query = name_hash;
    tmx_property_foreach(properties, tmxlib_store_property, (void*)core);
}

status_t load_tiled_map(const char* map_file_name, core_t* core)
{
    FILE* fp = fopen(map_file_name, "r");

    if (fp)
    {
        fclose(fp);
    }
    else
    {
        dbgprint("%s: %s not found.", FUNCTION_NAME, map_file_name);
        return CORE_WARNING;
    }

    core->map->handle = (tmx_map*)tmx_load(map_file_name);
    if (! core->map->handle)
    {
        dbgprint("%s: %s.", FUNCTION_NAME, tmx_strerr());
        return CORE_WARNING;
    }

    return CORE_OK;
}

Sint32 remove_gid_flip_bits(Sint32 gid)
{
    return gid & TMX_FLIP_BITS_REMOVAL;
}

SDL_bool tile_has_properties(Sint32 gid, tmx_tile** tile, tmx_map* tiled_map)
{
    Sint32 local_id;
    return SDL_TRUE;
}

void unload_tiled_map(core_t* core)
{
    if (core->map->handle)
    {
        tmx_map_free(core->map->handle);
    }
}

SDL_bool is_map_loaded(core_t* core)
{
    if (core->is_map_loaded)
    {
        return SDL_TRUE;
    }

    return SDL_FALSE;
}

SDL_bool get_boolean_map_property(const Uint64 name_hash, core_t* core)
{
    Sint32 prop_cnt;

    if (! is_map_loaded(core))
    {
        return SDL_FALSE;
    }

    prop_cnt                    = get_map_property_count(core->map->handle);
    core->map->boolean_property = SDL_FALSE;
    load_property(name_hash, core->map->handle->properties, prop_cnt, core);
    return core->map->boolean_property;
}

double get_decimal_map_property(const Uint64 name_hash, core_t* core)
{
    Sint32 prop_cnt;

    if (! is_map_loaded(core))
    {
        return 0.0;
    }

    prop_cnt                    = get_map_property_count(core->map->handle);
    core->map->decimal_property = 0.0;
    load_property(name_hash, core->map->handle->properties, prop_cnt, core);
    return core->map->decimal_property;
}

Sint32 get_integer_map_property(const Uint64 name_hash, core_t* core)
{
    Sint32 prop_cnt;

    if (! is_map_loaded(core))
    {
        return 0;
    }

    prop_cnt                    = get_map_property_count(core->map->handle);
    core->map->integer_property = 0;
    load_property(name_hash, core->map->handle->properties, prop_cnt, core);
    return core->map->integer_property;
}

const char* get_string_map_property(const Uint64 name_hash, core_t* core)
{
    Sint32 prop_cnt;

    if (! is_map_loaded(core))
    {
        return NULL;
    }

    prop_cnt                   = get_map_property_count(core->map->handle);
    core->map->string_property = NULL;
    load_property(name_hash, core->map->handle->properties, prop_cnt, core);
    return core->map->string_property;
}

status_t load_map_path(const char* file_name, core_t* core)
{
    core->map->path = (char*)calloc(1, (size_t)(strlen(file_name) + 1));
    if (! core->map->path)
    {
        dbgprint("%s: error allocating memory.", FUNCTION_NAME);
        return CORE_ERROR;
    }

    core->map->path_length = strlen(file_name);
    SDL_strlcpy(core->map->path, file_name, core->map->path_length + 1);

    return CORE_OK;
}

status_t load_texture_from_file(const char* file_name, SDL_Texture** texture, core_t* core)
{
    SDL_Surface* surface;

    if (! file_name)
    {
        return CORE_WARNING;
    }

    surface = SDL_LoadBMP(file_name);
    if (NULL == surface)
    {
        dbgprint("Failed to load image: %s", SDL_GetError());
        return CORE_ERROR;
    }

    *texture = SDL_CreateTextureFromSurface(core->renderer, surface);
    if (NULL == *texture)
    {
        dbgprint("Could not create texture from surface: %s", SDL_GetError());
        SDL_FreeSurface(surface);
        return CORE_ERROR;
    }
    SDL_FreeSurface(surface);

    dbgprint("Loading image from file: %s.", file_name);

    return CORE_OK;
}

status_t load_tileset(core_t* core)
{
    status_t   status      = CORE_OK;
    char*      image_path  = NULL;
    Sint32     path_length = get_tileset_path_length(core);

    image_path = (char*)calloc(1, path_length);
    if (! image_path)
    {
        dbgprint("%s: error allocating memory.", FUNCTION_NAME);
        return CORE_ERROR;
    }

    set_tileset_path(image_path, path_length, core);

    if (CORE_OK != load_texture_from_file(image_path, &core->map->tileset_texture, core))
    {
        dbgprint("%s: Error loading image '%s'.", FUNCTION_NAME, image_path);
        status = CORE_ERROR;
    }
    free(image_path);

    return status;
}

status_t load_animated_tiles(core_t* core)
{
    tmx_layer* layer               = get_head_layer(core->map->handle);
    Sint32     animated_tile_count = 0;
    Sint32     index_height        = 0;
    Sint32     index_width         = 0;

    while (layer)
    {
        if (is_tiled_layer_of_type(L_LAYER, layer) && layer->visible)
        {
            for (index_height = 0; index_height < (Sint32)core->map->handle->height; index_height += 1)
            {
                for (index_width = 0; index_width < (Sint32)core->map->handle->width; index_width += 1)
                {
                    Sint32* layer_content = get_layer_content(layer);
                    Sint32  gid           = remove_gid_flip_bits((Sint32)layer_content[(index_height * (Sint32)core->map->handle->width) + index_width]);

                    if (is_tile_animated(gid, NULL, NULL, core->map->handle))
                    {
                        animated_tile_count += 1;
                    }
                }
            }
        }
        layer = layer->next;
    }

    if (0 >= animated_tile_count)
    {
        return CORE_OK;
    }
    else
    {
        core->map->animated_tile = (animated_tile_t*)calloc((size_t)animated_tile_count, sizeof(struct animated_tile));
        if (!core->map->animated_tile)
        {
            dbgprint("%s: error allocating memory.", FUNCTION_NAME);
            return CORE_ERROR;
        }
    }

    dbgprint("Load %u animated tile(s).", animated_tile_count);

    return CORE_OK;
}

status_t create_and_set_render_target(SDL_Texture** target, core_t* core)
{
    if (! (*target))
    {
        (*target) = SDL_CreateTexture(
            core->renderer,
            SDL_PIXELFORMAT_RGB444,
            SDL_TEXTUREACCESS_TARGET,
            176,
            208);
    }

    if (! (*target))
    {
        dbgprint("%s: %s.", FUNCTION_NAME, SDL_GetError());
        return CORE_ERROR;
    }
    else
    {
        if (0 > SDL_SetTextureBlendMode((*target), SDL_BLENDMODE_BLEND))
        {
            dbgprint("%s: %s.", FUNCTION_NAME, SDL_GetError());
            SDL_DestroyTexture((*target));
            return CORE_ERROR;
        }
    }

    if (0 > SDL_SetRenderTarget(core->renderer, (*target)))
    {
        dbgprint("%s: %s.", FUNCTION_NAME, SDL_GetError());
        SDL_DestroyTexture((*target));
        return CORE_ERROR;
    }

    SDL_RenderClear(core->renderer);

    return CORE_OK;
}

SDL_bool get_boolean_property(const Uint64 name_hash, tmx_property* properties, Sint32 property_count, core_t* core)
{
    core->map->boolean_property = SDL_FALSE;
    load_property(name_hash, properties, property_count, core);
    return core->map->boolean_property;
}

double get_decimal_property(const Uint64 name_hash, tmx_property* properties, Sint32 property_count, core_t* core)
{
    core->map->decimal_property = 0.0;
    load_property(name_hash, properties, property_count, core);
    return core->map->decimal_property;
}

int32_t get_integer_property(const Uint64 name_hash, tmx_property* properties, Sint32 property_count, core_t* core)
{
    core->map->integer_property = 0;
    load_property(name_hash, properties, property_count, core);
    return core->map->integer_property;
}

const char* get_string_property(const Uint64 name_hash, tmx_property* properties, Sint32 property_count, core_t* core)
{
    core->map->string_property = NULL;
    load_property(name_hash, properties, property_count, core);
    return core->map->string_property;
}

status_t render_map(Sint32 level, core_t* core)
{
    tmx_layer*   layer;
    SDL_bool     render_animated_tiles = SDL_FALSE;
    render_layer render_layer          = RENDER_MAP_FG;
    Sint32       index;

    if (! core->is_map_loaded)
    {
        return CORE_OK;
    }

    layer = get_head_layer(core->map->handle);

    if (level >= MAP_LAYER_MAX)
    {
        dbgprint("%s: invalid layer level selected.", FUNCTION_NAME);
        return CORE_ERROR;
    }

    if (MAP_LAYER_BG == level)
    {
        render_layer = RENDER_MAP_BG;

        if (0 < core->map->animated_tile_fps)
        {
            render_animated_tiles = SDL_TRUE;
        }
    }

    if (CORE_OK != create_and_set_render_target(&core->map->render_target[render_layer], core))
    {
        return CORE_ERROR;
    }

    // Update and render animated tiles.
    core->map->time_since_last_anim_frame += core->time_since_last_frame;

    if (0 < core->map->animated_tile_index &&
        core->map->time_since_last_anim_frame >= 1000 / (Sint32)(core->map->animated_tile_fps) && render_animated_tiles)
    {
        core->map->time_since_last_anim_frame = 0;

        /* Remark: animated tiles are always rendered in the background
         * layer.
         */
        if (! core->map->animated_tile_texture)
        {
            core->map->animated_tile_texture = SDL_CreateTexture(
                core->renderer,
                SDL_PIXELFORMAT_RGB444,
                SDL_TEXTUREACCESS_TARGET,
                (Sint32)(core->map->width),
                (Sint32)(core->map->height));
        }

        if (! core->map->animated_tile_texture)
        {
            dbgprint("%s: %s.", FUNCTION_NAME, SDL_GetError());
            return CORE_ERROR;
        }

        if (0 > SDL_SetRenderTarget(core->renderer, core->map->animated_tile_texture))
        {
            dbgprint("%s: %s.", FUNCTION_NAME, SDL_GetError());
            return CORE_ERROR;
        }
        SDL_RenderClear(core->renderer);

        for (index = 0; core->map->animated_tile_index > index; index += 1)
        {
            tmx_tileset* tileset;
            Sint32       gid          = core->map->animated_tile[index].gid;
            Sint32       next_tile_id = 0;
            Sint32       local_id;
            SDL_Rect     dst;
            SDL_Rect     src;

            local_id = core->map->animated_tile[index].id + 1;
            tileset  = get_head_tileset(core->map->handle);
            src.w    = dst.w = get_tile_width(core->map->handle);
            src.h    = dst.h = get_tile_height(core->map->handle);
            dst.x    = core->map->animated_tile[index].dst_x;
            dst.y    = core->map->animated_tile[index].dst_y;

            get_tile_position(local_id, (Uint32*)&src.x, (Uint32*)&src.y, core->map->handle);

            if (0 > SDL_RenderCopy(core->renderer, core->map->tileset_texture, &src, &dst))
            {
                dbgprint("%s: %s.", FUNCTION_NAME, SDL_GetError());
                return CORE_ERROR;
            }

            core->map->animated_tile[index].current_frame += 1;

            if (core->map->animated_tile[index].current_frame >= core->map->animated_tile[index].animation_length)
            {
                core->map->animated_tile[index].current_frame = 0;
            }

            next_tile_id = get_next_animated_tile_id(gid, core->map->animated_tile[index].current_frame, core->map->handle);

            core->map->animated_tile[index].id = next_tile_id;
        }

        if (0 > SDL_SetRenderTarget(core->renderer, core->map->render_target[render_layer]))
        {
            dbgprint("%s: %s.", FUNCTION_NAME, SDL_GetError());
            return CORE_ERROR;
        }
        SDL_RenderClear(core->renderer);

        if (0 > SDL_SetTextureBlendMode(core->map->animated_tile_texture, SDL_BLENDMODE_BLEND))
        {
            dbgprint("%s: %s.", FUNCTION_NAME, SDL_GetError());
            return CORE_ERROR;
        }
    }

    // Texture has already been rendered.
    if (core->map->layer_texture[level])
    {
        Sint32   render_pos_x = core->map->pos_x - core->camera.pos_x;
        Sint32   render_pos_y = core->map->pos_y - core->camera.pos_y;
        SDL_Rect dst          = {
            (Sint32)render_pos_x,
            (Sint32)render_pos_y,
            (Sint32)core->map->width,
            (Sint32)core->map->height
        };

        if (0 > SDL_RenderCopyEx(core->renderer, core->map->layer_texture[level], NULL, &dst, 0, NULL, SDL_FLIP_NONE))
        {
            dbgprint("%s: %s.", FUNCTION_NAME, SDL_GetError());
            return CORE_ERROR;
        }

        if (render_animated_tiles)
        {
            if (core->map->animated_tile_texture)
            {
                if (0 > SDL_RenderCopyEx(core->renderer, core->map->animated_tile_texture, NULL, &dst, 0, NULL, SDL_FLIP_NONE))
                {
                    dbgprint("%s: %s.", FUNCTION_NAME, SDL_GetError());
                    return CORE_ERROR;
                }
            }
        }

        return CORE_OK;
    }

    // Texture does not yet exist. Render it!
    core->map->layer_texture[level] = SDL_CreateTexture(
        core->renderer,
        SDL_PIXELFORMAT_RGB444,
        SDL_TEXTUREACCESS_TARGET,
        (Sint32)core->map->width,
        (Sint32)core->map->height);

    if (! core->map->layer_texture[level])
    {
        dbgprint("%s: %s.", FUNCTION_NAME, SDL_GetError());
        return CORE_ERROR;
    }

    if (0 > SDL_SetRenderTarget(core->renderer, core->map->layer_texture[level]))
    {
        dbgprint("%s: %s.", FUNCTION_NAME, SDL_GetError());
        return CORE_ERROR;
    }
    SDL_RenderClear(core->renderer);

    while (layer)
    {
        SDL_Rect dst;
        SDL_Rect src;

        if (is_tiled_layer_of_type(L_LAYER, layer))
        {
            SDL_bool is_in_foreground  = SDL_FALSE;
            SDL_bool is_layer_rendered = SDL_FALSE;
            Sint32   prop_cnt = get_layer_property_count(layer);

            is_in_foreground = get_boolean_property(H_is_in_foreground, layer->properties, prop_cnt, core);

            if (MAP_LAYER_BG == level && SDL_FALSE == is_in_foreground)
            {
                is_layer_rendered = SDL_TRUE;
            }
            else if (MAP_LAYER_FG == level && is_in_foreground)
            {
                is_layer_rendered = SDL_TRUE;
            }

            if (layer->visible && is_layer_rendered)
            {
                Sint32 index_height;
                Sint32 index_width;

                for (index_height = 0; index_height < (Sint32)core->map->handle->height; index_height += 1)
                {
                    for (index_width = 0; index_width < (Sint32)core->map->handle->width; index_width += 1)
                    {
                        Sint32* layer_content = get_layer_content(layer);
                        Sint32  gid           = remove_gid_flip_bits((Sint32)layer_content[(index_height * (Sint32)core->map->handle->width) + index_width]);
                        Sint32  local_id      = gid - get_first_gid(core->map->handle);

                        if (is_gid_valid(gid, core->map->handle))
                        {
                            tmx_tileset* tileset = get_head_tileset(core->map->handle);

                            src.w = dst.w = get_tile_width(core->map->handle);
                            src.h = dst.h = get_tile_height(core->map->handle);
                            dst.x = (Sint32)(index_width  * get_tile_width(core->map->handle));
                            dst.y = (Sint32)(index_height * get_tile_height(core->map->handle));

                            get_tile_position(gid, (Uint32*)&src.x, (Uint32*)&src.y, core->map->handle);
                            SDL_RenderCopy(core->renderer, core->map->tileset_texture, &src, &dst);

                            if (render_animated_tiles)
                            {
                                Sint32 animation_length = 0;
                                Sint32 id               = 0;

                                if (is_tile_animated(gid, &animation_length, &id, core->map->handle))
                                {
                                    core->map->animated_tile[core->map->animated_tile_index].gid              = get_local_id(gid, core->map->handle);
                                    core->map->animated_tile[core->map->animated_tile_index].id               = id;
                                    core->map->animated_tile[core->map->animated_tile_index].dst_x            = dst.x;
                                    core->map->animated_tile[core->map->animated_tile_index].dst_y            = dst.y;
                                    core->map->animated_tile[core->map->animated_tile_index].current_frame    = 0;
                                    core->map->animated_tile[core->map->animated_tile_index].animation_length = animation_length;

                                    core->map->animated_tile_index += 1;
                                }
                            }
                        }
                    }
                }

                {
                    const char* layer_name = get_layer_name(layer);
                    dbgprint("Render map layer: %s", layer_name);
                }
            }
        }
        layer = layer->next;
    }

    if (0 > SDL_SetRenderTarget(core->renderer, core->map->render_target[render_layer]))
    {
        dbgprint("%s: %s.", FUNCTION_NAME, SDL_GetError());
        return CORE_ERROR;
    }

    if (0 > SDL_SetTextureBlendMode(core->map->layer_texture[level], SDL_BLENDMODE_BLEND))
    {
        dbgprint("%s: %s.", FUNCTION_NAME, SDL_GetError());
        return CORE_ERROR;
    }

    return CORE_OK;
}

status_t render_scene(core_t* core)
{
    status_t status = CORE_OK;
    Sint32   index;

    for (index = 0; index < MAP_LAYER_MAX; index  += 1)
    {
        status = render_map(index, core);
        if (CORE_OK != status)
        {
            return status;
        }
    }

    return status;
}

status_t draw_scene(core_t* core)
{
    SDL_Rect dst;
    Sint32   index;

    if (0 > SDL_SetRenderTarget(core->renderer, NULL))
    {
        dbgprint("%s: %s.", FUNCTION_NAME, SDL_GetError());
    }

    if (! core->is_map_loaded)
    {
        SDL_SetRenderDrawColor(core->renderer, 0x00, 0x00, 0x00, 0x00);
        SDL_RenderPresent(core->renderer);
        SDL_RenderClear(core->renderer);

        return CORE_OK;
    }

    dst.x = 0;
    dst.y = 0;
    dst.w = 176;
    dst.h = 208;

    for (index = 0; index < RENDER_LAYER_MAX; index += 1)
    {
        if (0 > SDL_RenderCopy(core->renderer, core->map->render_target[index], NULL, &dst))
        {
            dbgprint("%s: %s.", FUNCTION_NAME, SDL_GetError());
            return CORE_ERROR;
        }
    }
    SDL_RenderPresent(core->renderer);
    SDL_RenderClear(core->renderer);

    return CORE_OK;
}

static void tmxlib_store_property(tmx_property* property, void* core)
{
    core_t* core_ptr = core;

    if (core_ptr->map->hash_query == generate_hash((const unsigned char*)property->name))
    {
        switch (property->type)
        {
            case PT_COLOR:
            case PT_NONE:
                break;
            case PT_BOOL:
                dbgprint("Loading boolean property '%s': %u", property->name, property->value.boolean);

                core_ptr->map->boolean_property = (SDL_bool)property->value.boolean;
                break;
            case PT_FILE:
                dbgprint("Loading string property '%s': %s", property->name, property->value.file);

                core_ptr->map->string_property  = property->value.file;
                break;
            case PT_FLOAT:
                dbgprint("Loading decimal property '%s': %f", property->name, (double)property->value.decimal);

                core_ptr->map->decimal_property = (double)property->value.decimal;
                break;
            case PT_INT:
                dbgprint("Loading integer property '%s': %d", property->name, property->value.integer);

                core_ptr->map->integer_property = property->value.integer;
                break;
            case PT_STRING:
                dbgprint("Loading string property '%s': %s", property->name, property->value.string);

                core_ptr->map->string_property  = property->value.string;
                break;
        }
    }
}
