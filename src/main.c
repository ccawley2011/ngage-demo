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

    while(CORE_OK == update_core(core));

    quit:
    free_core(core);
    return status;
}
