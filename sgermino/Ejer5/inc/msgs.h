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
#include "term.h"


#define MSGS_NL             "\r\n"
#define MSGS_PREFIX_INFO    "░░ "
#define MSGS_PREFIX_WARNING "▓▓ "
#define MSGS_PREFIX_ERROR   "▓▓ "
#define MSGS_PREFIX_GROUP   " ░ "

#define MSGS_INFO_BEGIN     MSGS_PREFIX_INFO
#define MSGS_INFO_END       MSGS_NL
#define MSGS_INFO(s)        MSGS_INFO_BEGIN s MSGS_INFO_END

#define MSGS_COLOR_BEGIN(c) TERM_COLOR_##c \
                            MSGS_PREFIX_ERROR
#define MSGS_COLOR_END      TERM_NO_COLOR \
                            MSGS_NL
#define MSGS_COLOR(c,s)     MSGS_COLOR_BEGIN(c) s MSGS_COLOR_END

#define MSGS_WARNING(s)     MSGS_COLOR(BROWN,s)
#define MSGS_ERROR(s)       MSGS_COLOR(RED,s)
#define MSGS_OK(s)          MSGS_COLOR(BOLD_GREEN,s)
#define MSGS_CRITICAL(s)    MSGS_COLOR(BOLD_RED,s)
