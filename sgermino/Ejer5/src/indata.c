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
#include "term.h"
#include <string.h> // memset


bool INDATA_Init (struct INDATA *d, struct UART *uart)
{
    if (!d || !uart)
    {
        return false;
    }

    memset (d, 0, sizeof(struct INDATA));

    ARRAY_Init (&d->a, d->data, sizeof(d->data));
    d->uart = uart;
    return true;
}


bool INDATA_Begin (struct INDATA *d, enum INDATA_Type type)
{
    if (!d)
    {
        return false;
    }

    d->status = INDATA_StatusPrompt;
    d->type   = type;
    ARRAY_Reset (&d->a);
    return true;
}


enum INDATA_Status INDATA_Status (struct INDATA *d)
{
    if (!d)
    {
        return INDATA_StatusDisabled;
    }

    return d->status;
}


static bool checkEndOfIndex (struct INDATA *d)
{
    if (ARRAY_Full (&d->a))
    {
        UART_PutMessage (d->uart, TEXT_INDATA_TOOLONG);
        d->status = INDATA_StatusInvalid;
        return false;
    }
    return true;
}


static void validate (struct INDATA *d)
{
    UART_PutMessage (d->uart, TEXT_INDATA_VALIDATING);
    d->status = INDATA_StatusInvalid;

    switch (d->type)
    {
        case INDATA_TypeDecimal:
            if (ARRAY_CheckDecimalChars (&d->a))
            {
                d->status = INDATA_StatusReady;
                break;
            }
            UART_PutMessage (d->uart, TEXT_INDATA_WRONGTYPEINT);
            break;

        case INDATA_TypeAlphanum:
            if (ARRAY_CheckAlnumChars (&d->a))
            {
                d->status = INDATA_StatusReady;
                break;
            }
            UART_PutMessage (d->uart, TEXT_INDATA_WRONGTYPEALNUM);
            break;

        default:
            UART_PutMessage (d->uart, TEXT_INDATA_NOTYPEVAL);
            break;
    }
}


static bool checkInteraction (struct INDATA *d, uint8_t val)
{
    switch (val)
    {
        case 0x0D:  // CR (Enter)
            ARRAY_Terminate (&d->a);
            validate (d);
            return false;

        case 0x7F:  // DEL (Backspace)
            if (ARRAY_RemoveChars (&d->a, 1))
            {
                UART_PutMessage (d->uart, TERM_CURSOR_LEFT(1) " "
                                 TERM_CURSOR_LEFT(1));
            }
            break;

        default:
        {
            ARRAY_Append    (&d->a, val);
            UART_PutBinary  (d->uart, &val, 1);
            break;
        }
    }
    return true;
}


bool INDATA_Prompt (struct INDATA *d)
{
    if (!d || d->status != INDATA_StatusPrompt)
    {
        return false;
    }

    const uint32_t Pending = UART_RecvPendingCount (d->uart);
    for (uint32_t i = 0; i < Pending; ++i)
    {
        if (!checkEndOfIndex    (d) ||
            !checkInteraction   (d, UART_RecvPeek (d->uart, i)))
        {
            break;
        }
    }
    return true;
}


struct ARRAY * INDATA_Data (struct INDATA *d)
{
    if (!d || d->status != INDATA_StatusReady)
    {
        return NULL;
    }

    return &d->a;
}


bool INDATA_End (struct INDATA *d)
{
    if (!d)
    {
        return false;
    }

    d->status = INDATA_StatusDisabled;
    return true;
}

