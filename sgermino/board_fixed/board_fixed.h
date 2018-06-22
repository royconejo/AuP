#pragma once

#include "board.h"


#define BOARD_TEC_1         0
#define BOARD_TEC_2         1
#define BOARD_TEC_3         2
#define BOARD_TEC_4         3


void Board_Init_Fixed       ();
bool Board_TEC_GetStatus    (uint8_t button);

#if 0
5 V tolerant pad providing digital I/O functions (with TTL levels and hysteresis) and analog input or output (5 V tolerant if VDDIO present;
if VDDIO not present, do not exceed 3.6 V). When configured as a ADC input or DAC output, the pin is not 5 V tolerant and the digital
section of the pad must be disabled by setting the pin to an input function and disabling the pull-up resistor through the pin’s SFSP
register.
#endif
