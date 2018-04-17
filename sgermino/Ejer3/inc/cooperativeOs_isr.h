#pragma once

#include <stdint.h>

void    schedulerInit       (void);
void    schedulerStart      (uint32_t);
void    schedulerUpdate     (uint32_t ticks);
