/*
    RETRO-CIAA™ Library
    Copyright 2018 Santiago Germino (royconejo@gmail.com)

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    1.  Redistributions of source code must retain the above copyright notice,
        this list of conditions and the following disclaimer.

    2.  Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

    3.  Neither the name of the copyright holder nor the names of its
        contributors may be used to endorse or promote products derived from
        this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.
*/
#pragma once

// BASH
#define TERM_CLEAR_SCREEN           "\033[2J"
#define TERM_CURSOR_UP(n)           "\033[" #n "A"
#define TERM_CURSOR_DOWN(n)         "\033[" #n "B"
#define TERM_CURSOR_RIGHT(n)        "\033[" #n "C"
#define TERM_CURSOR_LEFT(n)         "\033[" #n "D"
#define TERM_COLOR(i,c)             "\033[" #i ";" #c "m"
#define TERM_NO_COLOR               "\033[0m"
#define TERM_COLOR_BLACK            TERM_COLOR(0,30)
#define TERM_COLOR_RED              TERM_COLOR(0,31)
#define TERM_COLOR_GREEN            TERM_COLOR(0,32)
#define TERM_COLOR_BROWN            TERM_COLOR(0,33)
#define TERM_COLOR_BLUE             TERM_COLOR(0,34)
#define TERM_COLOR_PURPLE           TERM_COLOR(0,35)
#define TERM_COLOR_CYAN             TERM_COLOR(0,36)
#define TERM_COLOR_GRAY             TERM_COLOR(0,37)
#define TERM_COLOR_BOLD_GRAY        TERM_COLOR(1,30)
#define TERM_COLOR_BOLD_RED         TERM_COLOR(1,31)
#define TERM_COLOR_BOLD_GREEN       TERM_COLOR(1,32)
#define TERM_COLOR_BOLD_YELLOW      TERM_COLOR(1,33)
#define TERM_COLOR_BOLD_BLUE        TERM_COLOR(1,34)
#define TERM_COLOR_BOLD_PURPLE      TERM_COLOR(1,35)
#define TERM_COLOR_BOLD_CYAN        TERM_COLOR(1,36)
#define TERM_COLOR_WHITE            TERM_COLOR(1,37)
