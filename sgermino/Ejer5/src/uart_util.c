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
#include "uart_util.h"
#include "text.h"


bool UART_PutMessageArgs (struct UART_Context *ctx, const char *msg,
                          struct VARIANT argValues[], uint32_t argCount)
{
    if (!ctx || !msg || !argValues || !argCount)
    {
        return false;
    }

    for (uint32_t i = 0; msg[i]; ++i)
    {
        const char Next = msg[i + 1];
        if (Next)
        {
            if (msg[i] == '%')
            {
                UART_PutBinary (ctx, (uint8_t *)msg, i);
                msg = &msg[i + 2];              // %.[0]
                i   = 0;
                if (Next >= 49 && Next <= 57)   // [1-9]{1}
                {
                    const uint32_t arg = Next - 49;
                    if (arg < argCount)
                    {
                        UART_PutMessage (ctx,
                                         VARIANT_ToString(&argValues[arg]));
                    }
                }
            }
        }
        else
        {
            UART_PutBinary (ctx, (uint8_t *)msg, i);
        }
    }
    return true;
}


void UART_PutStatusMessage (struct UART_Context *ctx)
{
    struct VARIANT args[4];
    VARIANT_SetUint32 (&args[0], ctx->sendWrites);
    VARIANT_SetUint32 (&args[1], ctx->sendOverflow);
    VARIANT_SetUint32 (&args[2], ctx->recvWrites);
    VARIANT_SetUint32 (&args[3], ctx->recvOverflow);

    UART_PutMessage     (ctx, TEXT_UART_STATSBEGIN);
    UART_PutMessageArgs (ctx, TEXT_UART_STATS, args, 4);
    UART_PutMessage     (ctx, TEXT_UART_STATSEND);
}
