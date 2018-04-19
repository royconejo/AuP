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
#include "uart.h"


#ifndef INDATA_BUFFER_SIZE
    #define INDATA_BUFFER_SIZE      32
#endif


enum INDATA_Status
{
    INDATA_StatusDisabled = 0,
    INDATA_StatusPrompt,
    INDATA_StatusInvalid,
    INDATA_StatusReady
};


enum INDATA_Type
{
    INDATA_TypeDecimal = 0,
    INDATA_TypeAlphanum,
    INDATA_TypeBinary
};


struct INDATA_Context
{
    enum INDATA_Status  status;
    enum INDATA_Type    type;
    uint32_t            index;
    struct UART_Context *uart;
    uint8_t             data [INDATA_BUFFER_SIZE];
};


bool                INDATA_Init         (struct INDATA_Context *ctx,
                                         struct UART_Context *uartCtx);
bool                INDATA_Begin        (struct INDATA_Context *ctx,
                                         enum INDATA_Type type);
enum INDATA_Status  INDATA_Status       (struct INDATA_Context *ctx);
bool                INDATA_Prompt       (struct INDATA_Context *ctx);
int32_t             INDATA_Decimal      (struct INDATA_Context *ctx);
bool                INDATA_End          (struct INDATA_Context *ctx);
