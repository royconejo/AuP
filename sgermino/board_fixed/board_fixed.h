#pragma once

#include "board.h"


enum Board_BTN
{
    Board_BTN_TEC_1 = 0,
    Board_BTN_TEC_2,
    Board_BTN_TEC_3,
    Board_BTN_TEC_4,
    Board_BTN_Invalid,
};


void Board_Init_Fixed       ();
bool Board_BTN_State        (enum Board_BTN button);
