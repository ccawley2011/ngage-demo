// Spdx-License-Identifier: MIT

#ifndef CORE_H
#define CORE_H

#include <SDL.h>

typedef struct core
{
    SDL_Renderer* renderer;
    SDL_Window*   window;
    SDL_bool      is_active;

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
void free_core(core_t *core);

#endif /* CORE_H */
