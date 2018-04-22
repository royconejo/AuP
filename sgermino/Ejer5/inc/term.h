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


// ANSI escape codes
// https://en.wikipedia.org/wiki/ANSI_escape_code
#define TERM_CSI                    "\033["
#define TERM_CLEAR_SCREEN           TERM_CSI "2J"
#define TERM_CURSOR_POS(row,col)    TERM_CSI #row ";" #col "H"
#define TERM_CURSOR_UP(n)           TERM_CSI #n "A"
#define TERM_CURSOR_DOWN(n)         TERM_CSI #n "B"
#define TERM_CURSOR_RIGHT(n)        TERM_CSI #n "C"
#define TERM_CURSOR_LEFT(n)         TERM_CSI #n "D"
#define TERM_CURSOR_SAVE            TERM_CSI "s"
#define TERM_CURSOR_RESTORE         TERM_CSI "u"
#define TERM_FG_COLOR_RGB(r,g,b)    TERM_CSI "38;2;" #r ";" #g ";" #b "m"
#define TERM_BG_COLOR_RGB(r,g,b)    TERM_CSI "48;2;" #r ";" #g ";" #b "m"
#define TERM_COLOR(b,c)             TERM_CSI #b ";" #c "m"
#define TERM_NO_COLOR               TERM_CSI "0m"
// Foreground colors (codes 30 - 37)
#define TERM_FG_COLOR_BLACK         TERM_COLOR(0,30)
#define TERM_FG_COLOR_RED           TERM_COLOR(0,31)
#define TERM_FG_COLOR_GREEN         TERM_COLOR(0,32)
#define TERM_FG_COLOR_BROWN         TERM_COLOR(0,33)
#define TERM_FG_COLOR_BLUE          TERM_COLOR(0,34)
#define TERM_FG_COLOR_PURPLE        TERM_COLOR(0,35)
#define TERM_FG_COLOR_CYAN          TERM_COLOR(0,36)
#define TERM_FG_COLOR_GRAY          TERM_COLOR(0,37)
#define TERM_FG_COLOR_BOLD_GRAY     TERM_COLOR(1,30)
#define TERM_FG_COLOR_BOLD_RED      TERM_COLOR(1,31)
#define TERM_FG_COLOR_BOLD_GREEN    TERM_COLOR(1,32)
#define TERM_FG_COLOR_BOLD_YELLOW   TERM_COLOR(1,33)
#define TERM_FG_COLOR_BOLD_BLUE     TERM_COLOR(1,34)
#define TERM_FG_COLOR_BOLD_PURPLE   TERM_COLOR(1,35)
#define TERM_FG_COLOR_BOLD_CYAN     TERM_COLOR(1,36)
#define TERM_FG_COLOR_WHITE         TERM_COLOR(1,37)
