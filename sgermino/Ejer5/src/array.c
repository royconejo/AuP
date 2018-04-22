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
#include "array.h"
#include "variant.h"
#include <string.h>


bool ARRAY_Init (struct ARRAY *a, uint8_t *data, uint32_t capacity)
{
    if (!a)
    {
        return false;
    }

    memset (a, 0, sizeof(struct ARRAY));

    if (!data || !capacity)
    {
        return false;
    }

    a->data     = data;
    a->capacity = capacity;
    return true;
}


void ARRAY_Reset (struct ARRAY *a)
{
    if (a)
    {
        a->index = 0;
    }
}


uint32_t ARRAY_Elements (struct ARRAY *a)
{
    return (a)? a->index : 0;
}


bool ARRAY_Full (struct ARRAY *a)
{
    return (a && a->index == a->capacity - 1);
}


bool ARRAY_Append (struct ARRAY *a, uint8_t element)
{
    if (ARRAY_Full(a))
    {
        return false;
    }

    a->data[a->index ++] = element;
    return true;
}


bool ARRAY_AppendString (struct ARRAY *a, const char *str)
{
    if (!a || !str)
    {
        return false;
    }

    while (*str)
    {
        if (!ARRAY_Append (a, (uint8_t)*str ++))
        {
            return false;
        }
    }
    return true;
}


bool ARRAY_AppendBinary (struct ARRAY *a, const uint8_t *data, uint32_t size)
{
    if (!a || !data || !size)
    {
        return false;
    }

    while (size --)
    {
        if (!ARRAY_Append (a, *data ++))
        {
            return false;
        }
    }
    return true;
}


uint32_t ARRAY_RemoveChars (struct ARRAY *a, uint32_t count)
{
    if (!a || !count || !a->index)
    {
        return 0;
    }

    uint32_t charsRemoved = 0;
    do
    {
        const uint8_t Byte = a->data[-- a->index];
        // Quito bytes hasta encontrar un caracter unico (US-ASCII) o comienzo
        // de un caracter multibyte
        if (Byte <= 127 || Byte >= 192)
        {
            ++ charsRemoved;
        }

        if (charsRemoved >= count)
        {
            break;
        }
    }
    while (a->index);

    return charsRemoved;
}


bool ARRAY_Terminate (struct ARRAY *a)
{
    if (!a)
    {
        return false;
    }

    a->data[a->index] = '\0';
    return true;
}


bool ARRAY_CheckAlnumChars (struct ARRAY *a)
{
    if (!a)
    {
        return false;
    }

    for (uint32_t i = 0; i < a->index; ++ i)
    {
        // Basic Latin
        if (a->data[i] < 128)
        {
            if ((a->data[i] >= '0' && a->data[i] <= '9') ||
                (a->data[i] >= 'A' && a->data[i] <= 'Z') ||
                (a->data[i] >= 'a' && a->data[i] <= 'z'))
            {
                continue;
            }
        }
        else if (i + 1 < a->index &&
                 ((a->data[i + 0] & 0b11100000) == 0b11000000) &&
                 ((a->data[i + 1] & 0b11000000) == 0b10000000))
        {
            const uint16_t Code = (a->data[i + 0] & 0b00011111) << 6 |
                                  (a->data[i + 1] & 0b00111111);
            // Latin-1 Supplement
            if ((Code >= 0x00C0 && Code <= 0x00D6) ||
                (Code >= 0x00D8 && Code <= 0x00F6) ||
                (Code >= 0x00F8 && Code <= 0x00FF) ||
                // Latin Extended-A
                (Code >= 0x0100 && Code <= 0x017F))
            {
                ++ i;
                continue;
            }
        }
        return false;
    }

    return true;
}


bool ARRAY_CheckDecimalChars (struct ARRAY *a)
{
    if (!a)
    {
        return false;
    }

    for (uint32_t i = 0; i < a->index; ++ i)
    {
        if (a->data[i] < '0' || a->data[i] > '9')
        {
            return false;
        }
    }

    return true;
}


bool ARRAY_CheckEqualContents (struct ARRAY *a, struct ARRAY *b)
{
    if (!a || !b || a->index != b->index)
    {
        return false;
    }

    return !(memcmp (a->data, b->data, a->index))? true : false;
}


bool ARRAY_Copy (struct ARRAY *a, struct ARRAY *b)
{
    if (!a || !b || !b->capacity || a->index >= b->capacity - 1)
    {
        return false;
    }

    memcpy (b->data, a->data, a->index);
    b->index = a->index;
    return true;
}


bool ARRAY_ToVariant (struct ARRAY *a, struct VARIANT *v)
{
    if (!a || !v)
    {
        return false;
    }

    ARRAY_Terminate     (a);
    VARIANT_SetString   (v, (char *)a->data);
    return true;
}
