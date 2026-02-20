/*
 * psg_realhw_rpi_gpio.cpp
 *
 * Minimal glue: PC6001VX (C++) -> psg_backend_rpi_gpio (C)
 *
 * - lazy init on first write
 * - uses existing backend ops (init/enable/reset/write_reg)
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <mutex>

extern "C" {
#include "ym2149f.h"
#include "psg_realhw_backend.h"

/* backend interface headers */
#include "psg_backend.h"
#include "psg_backend_rpi_gpio.h"
}

namespace {

static std::once_flag g_once;
static int g_ready = 0;

static psg_backend_t g_be;
static psg_backend_ops_t g_ops;

static void
realhw_fini(void)
{
    if (g_ready != 0) {
        if (g_ops.disable != NULL) {
            g_ops.disable(&g_be);
        }
        if (g_ops.fini != NULL) {
            g_ops.fini(&g_be);
        }
        g_ready = 0;
    }
}

static void
realhw_init_once(void)
{
    memset(&g_be, 0, sizeof(g_be));
    memset(&g_ops, 0, sizeof(g_ops));

    psg_backend_rpi_gpio_bind(&g_ops);

    if (g_ops.init == NULL || g_ops.enable == NULL || g_ops.reset == NULL ||
        g_ops.write_reg == NULL || g_ops.fini == NULL) {
        return;
    }

    if (g_ops.init(&g_be) == 0) {
        return;
    }
    if (g_ops.enable(&g_be) == 0) {
        g_ops.fini(&g_be);
        return;
    }
    if (g_ops.reset(&g_be) == 0) {
        g_ops.disable(&g_be);
        g_ops.fini(&g_be);
        return;
    }

    g_ready = 1;
    atexit(realhw_fini);
}

} // namespace

extern "C" void
psg_realhw_write_reg(uint8_t reg, uint8_t val)
{
    /*
     * For first validation:
     * - accept 0..13 (AY_ESHAPE) only
     * - ignore others (i.e. I/O ports) silently
     */
    if (reg > AY_ESHAPE) {
        return;
    }

    std::call_once(g_once, realhw_init_once);

    if (g_ready == 0) {
        return;
    }

    (void)g_ops.write_reg(&g_be, reg, val);
}
