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
#include "uart.h"
#include "uart_io.h"
#include <string.h>


bool UART_Init (struct UART_Context *ctx, void *handler, uint32_t baudRate)
{
    if (!ctx)
    {
        return false;
    }

    memset (ctx, 0, sizeof(struct UART_Context));
    ctx->handler = handler;
    return UART_Config (ctx, baudRate);
}


uint32_t UART_SendPendingCount (struct UART_Context *ctx)
{
    if (!ctx)
    {
        return 0;
    }

    return (ctx->sendPutIndex - ctx->sendGetIndex) & UART_SEND_BUFFER_MASK;
}


uint32_t UART_RecvPendingCount (struct UART_Context *ctx)
{
    if (!ctx)
    {
        return 0;
    }

    return (ctx->recvPutIndex - ctx->recvGetIndex) & UART_RECV_BUFFER_MASK;
}


bool UART_PutBinary (struct UART_Context *ctx, const uint8_t *data,
                     uint32_t size)
{
    if (!ctx)
    {
        return false;
    }

    const uint32_t  Pending = UART_SendPendingCount (ctx);
    uint32_t        count   = 0;

    while (count < size)
    {
        ctx->send[ctx->sendPutIndex] = data[count];
        ctx->sendPutIndex = (ctx->sendPutIndex + 1) & UART_SEND_BUFFER_MASK;
        ++ count;
    }

    ctx->sendWrites += count;

    if (count > UART_SEND_BUFFER_SIZE - Pending)
    {
        ctx->sendOverflow += count - (UART_SEND_BUFFER_SIZE - Pending);
    }

    return true;
}


bool UART_PutMessage (struct UART_Context *ctx, const char *msg)
{
    return UART_PutBinary (ctx, (uint8_t *)msg, strlen(msg));
}


uint32_t UART_Send (struct UART_Context *ctx)
{
    if (!ctx)
    {
        return 0;
    }

    uint32_t pending = UART_SendPendingCount (ctx);
    if (!pending)
    {
        return 0;
    }

    if (pending > UART_HW_FIFO_SIZE)
    {
        pending = UART_HW_FIFO_SIZE;
    }

    const uint32_t BytesSent = pending;

    do
    {
        UART_PutByte (ctx, ctx->send[ctx->sendGetIndex]);
        ctx->sendGetIndex = (ctx->sendGetIndex + 1) & UART_SEND_BUFFER_MASK;
    }
    while (-- pending);

    return BytesSent;
}


uint32_t UART_Recv (struct UART_Context *ctx)
{
    if (!ctx)
    {
        return 0;
    }

    const uint32_t  Pending = UART_SendPendingCount (ctx);
    uint32_t        count   = 0;
    int             byte;

    while ((byte = UART_GetByte (ctx)) != UART_EOF)
    {
        ctx->recv[ctx->recvPutIndex] = (uint8_t) byte;
        ctx->recvPutIndex = (ctx->recvPutIndex + 1) & UART_RECV_BUFFER_MASK;
        ++ count;
    }

    ctx->recvWrites += count;

    if (count > UART_RECV_BUFFER_SIZE - Pending)
    {
        ctx->recvOverflow += count - (UART_RECV_BUFFER_SIZE - Pending);
    }

    return count;
}


bool UART_RecvInjectByte (struct UART_Context *ctx, uint8_t byte)
{
    if (!ctx)
    {
        return 0;
    }

    ctx->recv[ctx->recvPutIndex] = byte;
    ctx->recvPutIndex = (ctx->recvPutIndex + 1) & UART_RECV_BUFFER_MASK;
    ctx->recvWrites += 1;

    const uint32_t Pending = UART_SendPendingCount (ctx);
    if (1 > UART_RECV_BUFFER_SIZE - Pending)
    {
        ctx->recvOverflow += 1 - (UART_RECV_BUFFER_SIZE - Pending);
    }
    return true;
}


uint8_t UART_RecvPeek (struct UART_Context *ctx, uint32_t offset)
{
    if (!ctx)
    {
        return 0;
    }

    return ctx->recv[ctx->recvGetIndex + offset];
}


bool UART_RecvConsumePending (struct UART_Context *ctx)
{
    if (!ctx)
    {
        return false;
    }

    const uint32_t Pending = UART_RecvPendingCount (ctx);
    ctx->recvGetIndex = (ctx->recvGetIndex + Pending) & UART_RECV_BUFFER_MASK;
    return true;
}

