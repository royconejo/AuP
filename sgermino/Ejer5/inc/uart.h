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


#ifndef UART_HW_FIFO_SIZE
    #define UART_HW_FIFO_SIZE       16
#endif

// IMPORTANTE: Los valores deben ser 2^X!
#ifndef UART_RECV_BUFFER_SIZE
    #define UART_RECV_BUFFER_SIZE   256
#endif

#ifndef UART_SEND_BUFFER_SIZE
    #define UART_SEND_BUFFER_SIZE   2048
#endif

#define UART_RECV_BUFFER_MASK   (UART_RECV_BUFFER_SIZE - 1)
#define UART_SEND_BUFFER_MASK   (UART_SEND_BUFFER_SIZE - 1)


struct UART_Context
{
    // Put: productor ingresa datos, Get: consumidor procesa datos
    uint32_t    recvPutIndex;
    uint32_t    recvGetIndex;
    uint32_t    sendPutIndex;
    uint32_t    sendGetIndex;

    // Estadisticas de uso de los buffers
    uint32_t    recvWrites;
    uint32_t    recvOverflow;
    uint32_t    sendWrites;
    uint32_t    sendOverflow;

    // Platform dependant UART handler
    void        *handler;

    uint8_t     recv [UART_RECV_BUFFER_SIZE];
    uint8_t     send [UART_SEND_BUFFER_SIZE];
};


bool        UART_Init               (struct UART_Context *ctx, void *handler,
                                     uint32_t baudRate);
uint32_t    UART_SendPendingCount   (struct UART_Context *ctx);
uint32_t    UART_RecvPendingCount   (struct UART_Context *ctx);
bool        UART_PutBinary          (struct UART_Context *ctx,
                                     const uint8_t *data, uint32_t size);
bool        UART_PutMessage         (struct UART_Context *ctx, const char *msg);
uint32_t    UART_Send               (struct UART_Context *ctx);
uint32_t    UART_Recv               (struct UART_Context *ctx);
bool        UART_RecvInjectByte     (struct UART_Context *ctx, uint8_t byte);
uint8_t     UART_RecvPeek           (struct UART_Context *ctx, uint32_t offset);
bool        UART_RecvConsumePending (struct UART_Context *ctx);
