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


static enum FEM_StateReturn FEMState_DummyError (struct FEM *f,
                                        enum FEM_Stage stage, uint32_t ticks)
{
    return FEM_StateReturnYield;
}


bool FEM_Init (struct FEM *f, void *app)
{
    if (!f)
    {
        return false;
    }

    memset (f, 0, sizeof(struct FEM));
    f->app            = app;
    f->invalidStage   = FEMState_DummyError;
    f->maxRecCalls    = FEMState_DummyError;
    return true;
}


bool FEM_SetErrorStates (struct FEM *f, FEM_StateFunc invalidStage,
                         FEM_StateFunc maxRecurringCalls)
{
    if (!f)
    {
        return false;
    }

    if (invalidStage)
    {
        f->invalidStage = invalidStage;
    }

    if (maxRecurringCalls)
    {
        f->maxRecCalls = maxRecurringCalls;
    }

    return true;
}


bool FEM_SetStateInfo (struct FEM *f, const char* info)
{
    if (!f)
    {
        return false;
    }

    f->info = info;
    return true;
}


bool FEM_StateTimeout (struct FEM *f, uint32_t timeoutTicks)
{
    return (f && f->stateStartTicks + timeoutTicks >= SYSTICK_Now());
}


bool FEM_StageTimeout (struct FEM *f, uint32_t timeoutTicks)
{
    return (f && f->stageStartTicks + timeoutTicks <= SYSTICK_Now());
}


bool FEM_StateCountdown (struct FEM *f, uint32_t timeoutTicks)
{
    if (!f)
    {
        return false;
    }

    const uint32_t NextTimeout = SYSTICK_Now() + timeoutTicks;

    if (!f->stateCountdownTicks)
    {
        f->stateCountdownTicks = NextTimeout;
    }
    else if (SYSTICK_Now() >= f->stateCountdownTicks)
    {
        f->stateCountdownTicks = timeoutTicks? NextTimeout : 0;
        return true;
    }

    return false;
}


uint32_t FEM_StateCountdownSeconds (struct FEM *f)
{
    if (!f || !f->stateCountdownTicks)
    {
        return 0;
    }

    const uint32_t Now = SYSTICK_Now ();

    if (Now >= f->stateCountdownTicks)
    {
        return 0;
    }

    return (f->stateCountdownTicks - Now) / SYSTICK_GetTickRateMicroseconds();
}


bool FEM_GotoStage (struct FEM *f, enum FEM_Stage newStage)
{
    if (newStage > FEM_Stage_LAST_)
    {
        return false;
    }

    f->stage              = newStage;
    f->stageCalls         = 0;
    f->stageStartTicks    = SYSTICK_Now ();
    return true;
}


bool FEM_ChangeState (struct FEM *f, FEM_StateFunc newState)
{
    if (!f || !newState)
    {
        return false;
    }

    f->state                  = newState;
    f->info                   = NULL;
    f->stateCalls             = 0;
    f->stateStartTicks        = SYSTICK_Now ();
    f->stateCountdownTicks    = 0;

    FEM_GotoStage (f, FEM_StageBegin);
    return true;
}


bool FEM_Process (struct FEM *f, uint32_t curTicks,
                  uint32_t timeoutTicks)
{
    if (!f)
    {
        return false;
    }

    if (timeoutTicks)
    {
        timeoutTicks = curTicks + timeoutTicks;
    }

    uint32_t recurringCalls = 0;
    enum FEM_StateReturn ret;
    do
    {
        if (!f->state)
        {
            return false;
        }

        if (timeoutTicks && SYSTICK_Now() >= timeoutTicks)
        {
            break;
        }

        if (f->stage > FEM_Stage_LAST_)
        {
            FEM_ChangeState     (f, f->invalidStage);
            FEM_SetStateInfo    (f, "ERROR: Invalid stage.");
        }
        else if (FEM_MAX_RECURRING_CALLS && recurringCalls >=
                 FEM_MAX_RECURRING_CALLS)
        {
            FEM_ChangeState     (f, f->maxRecCalls);
            FEM_SetStateInfo    (f, "ERROR: Max recurring calls reached.");
        }

        ret = f->state (f, f->stage, SYSTICK_Now());

        ++ f->stateCalls;
        ++ f->stageCalls;
        ++ recurringCalls;
    }
    while (ret == FEM_StateReturnAgain);
    return true;
}
