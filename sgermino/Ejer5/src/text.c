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
#include "text.h"
#include "texstyle.h"

// Neutral language text or text closely related to variable names in the code

const char *TEXT_NEWLINE = {
    TEXSTYLE_NL
};

const char *TEXT_REPLACEMENTCHAR = {
    "�"
};

const char *TEXT_UART_STATS1 = {
    TEXSTYLE_PREFIX_GROUP "Send" TEXSTYLE_NL
    TEXSTYLE_PREFIX_GROUP "  size      : %1" TEXSTYLE_NL
    TEXSTYLE_PREFIX_GROUP "  reads     : %2" TEXSTYLE_NL
    TEXSTYLE_PREFIX_GROUP "  writes    : %3" TEXSTYLE_NL
    TEXSTYLE_PREFIX_GROUP "  overflows : %4" TEXSTYLE_NL
    TEXSTYLE_PREFIX_GROUP "  peeks     : %5" TEXSTYLE_NL
    TEXSTYLE_PREFIX_GROUP "  discards  : %6" TEXSTYLE_NL
};

const char *TEXT_UART_STATS2 = {
    TEXSTYLE_PREFIX_GROUP "Receive" TEXSTYLE_NL
    TEXSTYLE_PREFIX_GROUP "  size      : %1" TEXSTYLE_NL
    TEXSTYLE_PREFIX_GROUP "  reads     : %2" TEXSTYLE_NL
    TEXSTYLE_PREFIX_GROUP "  writes    : %3" TEXSTYLE_NL
    TEXSTYLE_PREFIX_GROUP "  overflows : %4" TEXSTYLE_NL
    TEXSTYLE_PREFIX_GROUP "  peeks     : %5" TEXSTYLE_NL
    TEXSTYLE_PREFIX_GROUP "  discards  : %6"
};

const char *TEXT_FSM_STATS1 = {
    TEXSTYLE_PREFIX_GROUP "State             : %1" TEXSTYLE_NL
    TEXSTYLE_PREFIX_GROUP "  info            : \"%2\"" TEXSTYLE_NL
    TEXSTYLE_PREFIX_GROUP "  calls           : %3" TEXSTYLE_NL
    TEXSTYLE_PREFIX_GROUP "  start ticks     : %4" TEXSTYLE_NL
    TEXSTYLE_PREFIX_GROUP "  countdown ticks : %5" TEXSTYLE_NL
    TEXSTYLE_PREFIX_GROUP "  app context     : %6" TEXSTYLE_NL
    TEXSTYLE_PREFIX_GROUP "  Stage           : %7" TEXSTYLE_NL
    TEXSTYLE_PREFIX_GROUP "    calls         : %8" TEXSTYLE_NL
    TEXSTYLE_PREFIX_GROUP "    start ticks   : %9" TEXSTYLE_NL
};

const char *TEXT_FSM_STATS2 = {
    TEXSTYLE_PREFIX_GROUP "Error states" TEXSTYLE_NL
    TEXSTYLE_PREFIX_GROUP "  invalid stage   : %1" TEXSTYLE_NL
    TEXSTYLE_PREFIX_GROUP "  max rec. calls  : %2"
};
