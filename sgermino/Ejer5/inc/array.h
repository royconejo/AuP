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
#include <stdint.h>
#include <stdbool.h>


struct ARRAY
{
    uint8_t     *data;
    uint32_t    index;
    uint32_t    capacity;
};


bool        ARRAY_Init                  (struct ARRAY *a, uint8_t *data,
                                         uint32_t capacity);
void        ARRAY_Reset                 (struct ARRAY *a);
uint32_t    ARRAY_Elements              (struct ARRAY *a);
bool        ARRAY_Full                  (struct ARRAY *a);
bool        ARRAY_Append                (struct ARRAY *a, uint8_t element);
bool        ARRAY_AppendString          (struct ARRAY *a, const char *str);
uint32_t    ARRAY_RemoveChars           (struct ARRAY *a, uint32_t count);
bool        ARRAY_Terminate             (struct ARRAY *a);
bool        ARRAY_CheckAlnumChars       (struct ARRAY *a);
bool        ARRAY_CheckDecimalChars     (struct ARRAY *a);
bool        ARRAY_CheckEqualContents    (struct ARRAY *a, struct ARRAY *b);
