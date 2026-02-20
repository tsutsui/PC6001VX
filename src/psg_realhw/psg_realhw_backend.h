#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void psg_realhw_write_reg(uint8_t reg, uint8_t val);

#ifdef __cplusplus
}
#endif
