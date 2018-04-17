#pragma once

#include "board.h"


#define BOARD_TEC_1         0
#define BOARD_TEC_2         1
#define BOARD_TEC_3         2
#define BOARD_TEC_4         3


void Board_Init_Fixed       ();
bool Board_TEC_GetStatus    (uint8_t button);
