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
#include "msgs.h"


const char *TEXT_UART_STATSBEGIN = {
    MSGS_INFO_BEGIN "Estado de los buffers de I/O." MSGS_NL
};

const char *TEXT_UART_STATS = {
    MSGS_PREFIX_GROUP "Send writes      : %1 bytes." MSGS_NL
    MSGS_PREFIX_GROUP "Send overflow    : %2 bytes." MSGS_NL
    MSGS_PREFIX_GROUP "Receive writes   : %3 bytes." MSGS_NL
    MSGS_PREFIX_GROUP "Receive overflow : %4 bytes." MSGS_NL
};

const char *TEXT_UART_STATSEND = {
    MSGS_INFO_END
};



const char *TEXT_INDATA_TOOLONG = {
    MSGS_NL MSGS_ERROR("Dato demasiado largo.")
};

const char *TEXT_INDATA_VALIDATING = {
    MSGS_NL MSGS_PREFIX_GROUP "Validando dato ingresado..." MSGS_NL
};

const char *TEXT_INDATA_WRONGTYPEINT = {
    MSGS_ERROR("El dato ingresado debe ser un numero entero.")
};

const char *TEXT_INDATA_NOTYPEVAL = {
    MSGS_ERROR("Validacion no implementada para este tipo de dato.")
};
