// Spdx-License-Identifier: MIT

#include "core.h"

int main(int argc, char *argv[])
{
    int     status = 0;
    core_t *core   = NULL;

    if (CORE_ERROR == init_core("demo", &core))
    {
        status = -1;
        goto quit;
    }

    load_map("E:\\demo.tmx", core);

    while(CORE_OK == update_core(core));

    quit:
    unload_map(core);
    free_core(core);
    return status;
}
