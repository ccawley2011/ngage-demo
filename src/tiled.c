// SPDX-License-Identifier: MIT

#include <stb_sprintf.h> // libxml2

#include <SDL.h>
#include <tmx.h>
#include "core.h"

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
        SDL_Log("%s: %s not found.", __FUNCTION__, map_file_name);
        return CORE_WARNING;
    }

    core->map->handle = (tmx_map*)tmx_load(map_file_name);
    if (! core->map->handle)
    {
        SDL_Log("%s: %s.", __FUNCTION__, tmx_strerr());
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
        SDL_Log("%s: error allocating memory.", __FUNCTION__);
        return CORE_ERROR;
    }

    core->map->path_length = strlen(file_name);
    SDL_strlcpy(core->map->path, file_name, core->map->path_length + 1);

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
                SDL_Log("Loading boolean property '%s': %u", property->name, property->value.boolean);

                core_ptr->map->boolean_property = (SDL_bool)property->value.boolean;
                break;
            case PT_FILE:
                SDL_Log("Loading string property '%s': %s", property->name, property->value.file);

                core_ptr->map->string_property  = property->value.file;
                break;
            case PT_FLOAT:
                SDL_Log("Loading decimal property '%s': %f", property->name, (double)property->value.decimal);

                core_ptr->map->decimal_property = (double)property->value.decimal;
                break;
            case PT_INT:
                SDL_Log("Loading integer property '%s': %d", property->name, property->value.integer);

                core_ptr->map->integer_property = property->value.integer;
                break;
            case PT_STRING:
                SDL_Log("Loading string property '%s': %s", property->name, property->value.string);

                core_ptr->map->string_property  = property->value.string;
                break;
        }
    }
}
