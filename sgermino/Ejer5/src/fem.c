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
#include "fem.h"
#include "systick.h"
#include <string.h>


static enum FEM_StateReturn FEMState_DummyError (struct FEM_Context *ctx,
                                        enum FEM_Stage stage, uint32_t ticks)
{
    return FEM_StateReturnYield;
}


bool FEM_Init (struct FEM_Context *ctx, void *appCtx)
{
    if (!ctx)
    {
        return false;
    }

    memset (ctx, 0, sizeof(struct FEM_Context));
    ctx->appCtx         = appCtx;
    ctx->invalidStage   = FEMState_DummyError;
    ctx->maxRecCalls    = FEMState_DummyError;
    return true;
}


bool FEM_SetErrorStates (struct FEM_Context *ctx, FEM_StateFunc invalidStage,
                         FEM_StateFunc maxRecurringCalls)
{
    if (!ctx)
    {
        return false;
    }

    if (invalidStage)
    {
        ctx->invalidStage = invalidStage;
    }

    if (maxRecurringCalls)
    {
        ctx->maxRecCalls = maxRecurringCalls;
    }

    return true;
}


bool FEM_SetStateInfo (struct FEM_Context *ctx, const char* info)
{
    if (!ctx)
    {
        return false;
    }

    ctx->info = info;
    return true;
}


bool FEM_GotoStage (struct FEM_Context *ctx, enum FEM_Stage newStage)
{
    if (newStage >= FEM_StageInvalid)
    {
        return false;
    }

    ctx->stage = newStage;
    return true;
}


bool FEM_ChangeState (struct FEM_Context *ctx, FEM_StateFunc newState)
{
    if (!ctx || !newState)
    {
        return false;
    }

    ctx->state  = newState;
    ctx->stage  = FEM_StageBegin;
    ctx->info   = NULL;
    ctx->calls  = 0;
    return true;
}


bool FEM_Process (struct FEM_Context *ctx, uint32_t curTicks,
                  uint32_t timeoutTicks)
{
    if (!ctx)
    {
        return false;
    }

    if (timeoutTicks)
    {
        timeoutTicks = curTicks + timeoutTicks;
    }

    const uint32_t StartCalls = ctx->calls;
    enum FEM_StateReturn ret;
    do
    {
        if (!ctx->state)
        {
            return false;
        }

        if (timeoutTicks && SYSTICK_Now() >= timeoutTicks)
        {
            break;
        }

        if (ctx->stage >= FEM_StageInvalid)
        {
            FEM_ChangeState     (ctx, ctx->invalidStage);
            FEM_SetStateInfo    (ctx, "ERROR: Invalid stage.");
        }
        else if (FEM_MAX_RECURRING_CALLS && StartCalls - ctx->calls >=
                 FEM_MAX_RECURRING_CALLS)
        {
            FEM_ChangeState     (ctx, ctx->maxRecCalls);
            FEM_SetStateInfo    (ctx, "ERROR: Max recurring calls reached.");
        }

        ret = ctx->state (ctx, ctx->stage, SYSTICK_Now());
        ++ ctx->calls;
    }
    while (ret == FEM_StateReturnAgain);
    return true;
}
