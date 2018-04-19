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
#include "indata.h"
#include "text.h"
#include <string.h> // memset
#include <stdlib.h> // atoi


bool INDATA_Init (struct INDATA_Context *ctx, struct UART_Context *uartCtx)
{
    if (!ctx || !uartCtx)
    {
        return false;
    }

    memset (ctx, 0, sizeof(struct INDATA_Context));
    ctx->uart = uartCtx;
    return true;
}


bool INDATA_Begin (struct INDATA_Context *ctx, enum INDATA_Type type)
{
    if (!ctx)
    {
        return false;
    }

    ctx->status = INDATA_StatusPrompt;
    ctx->type   = type;
    ctx->index  = 0;
    return true;
}


enum INDATA_Status INDATA_Status (struct INDATA_Context *ctx)
{
    if (!ctx)
    {
        return INDATA_StatusDisabled;
    }

    return ctx->status;
}


static bool checkEndOfIndex (struct INDATA_Context *ctx)
{
    if (ctx->index == INDATA_BUFFER_SIZE - 1)
    {
        UART_PutMessage (ctx->uart, TEXT_INDATA_TOOLONG);
        ctx->status = INDATA_StatusInvalid;
        return false;
    }
    return true;
}


static bool checkTypeDecimal (struct INDATA_Context *ctx)
{
    for (uint32_t i = 0; ctx->data[i]; ++ i)
    {
        if (ctx->data[i] < '0' || ctx->data[i] > '9')
        {
            return false;
        }
    }
    return true;
}


static void validate (struct INDATA_Context *ctx)
{
    UART_PutMessage (ctx->uart, TEXT_INDATA_VALIDATING);
    ctx->status = INDATA_StatusInvalid;

    switch (ctx->type)
    {
        case INDATA_TypeDecimal:
            if (checkTypeDecimal (ctx))
            {
                ctx->status = INDATA_StatusReady;
                break;
            }
            UART_PutMessage (ctx->uart, TEXT_INDATA_WRONGTYPEINT);
            break;

        default:
            UART_PutMessage (ctx->uart, TEXT_INDATA_NOTYPEVAL);
            break;
    }
}


static bool checkInteraction (struct INDATA_Context *ctx, uint8_t val)
{
    switch (val)
    {
        case 0x0D:  // CR (Enter)
            ctx->data[ctx->index] = '\0';
            validate (ctx);
            return false;

        case 0x7F:  // DEL (Backspace)
            if (!ctx->index)
            {
                break;
            }

            do
            {
                // NOTA: Asume entrada en ASCII o UTF-8
                const uint8_t Byte = ctx->data[-- ctx->index];
                // Caracter unico (ASCII) o comienzo de caracter multibyte
                if (Byte <= 127 || Byte >= 192)
                {
                    break;
                }
            }
            while (ctx->index);

            UART_PutMessage (ctx->uart, TERM_CURSOR_LEFT(1) " "
                             TERM_CURSOR_LEFT(1));
            break;

        default:
        {
            ctx->data[ctx->index ++] = val;
            char c[2] = { val, '\0' };
            UART_PutMessage (ctx->uart, c);
            break;
        }
    }
    return true;
}


bool INDATA_Prompt (struct INDATA_Context *ctx)
{
    if (!ctx || ctx->status != INDATA_StatusPrompt)
    {
        return false;
    }

    const uint32_t Pending = UART_RecvPendingCount (ctx->uart);
    for (uint32_t i = 0; i < Pending; ++i)
    {
        if (!checkEndOfIndex    (ctx) ||
            !checkInteraction   (ctx, UART_RecvPeek (ctx->uart, i)))
        {
            break;
        }
    }
    return true;
}


int32_t INDATA_Decimal (struct INDATA_Context *ctx)
{
    if (!ctx || ctx->status != INDATA_StatusReady)
    {
        return 0;
    }

    if (ctx->type != INDATA_TypeDecimal)
    {
        return 0;
    }

    return atoi ((char *)ctx->data);
}


bool INDATA_End (struct INDATA_Context *ctx)
{
    if (!ctx)
    {
        return false;
    }

    ctx->status = INDATA_StatusDisabled;
    return true;
}

