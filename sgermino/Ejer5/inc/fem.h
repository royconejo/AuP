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


#ifndef FEM_MAX_RECURRING_CALLS
    #define FEM_MAX_RECURRING_CALLS     99
#endif


enum FEM_Stage
{
    FEM_StageBegin = 0,
    FEM_StageMain,
    FEM_StageEnd,
    FEM_Stage_LAST_ = FEM_StageEnd
};


enum FEM_StateReturn
{
    FEM_StateReturnYield = 0,
    FEM_StateReturnAgain
};


struct FEM;
typedef enum FEM_StateReturn (* FEM_StateFunc) (struct FEM *f,
                                         enum FEM_Stage stage, uint32_t ticks);


struct FEM
{
    FEM_StateFunc   state;
    enum FEM_Stage  stage;
    const char      *info;
    uint32_t        stateCalls;
    uint32_t        stageCalls;
    uint32_t        stateStartTicks;
    uint32_t        stageStartTicks;
    uint32_t        stateCountdownTicks;
    void            *app;
    FEM_StateFunc   invalidStage;
    FEM_StateFunc   maxRecCalls;
};


bool        FEM_Init           (struct FEM *f, void *app);
bool        FEM_SetErrorStates (struct FEM *f,
                                FEM_StateFunc invalidStage,
                                FEM_StateFunc maxRecurringCalls);
bool        FEM_SetStateInfo   (struct FEM *f, const char* info);
bool        FEM_StateTimeout   (struct FEM *f, uint32_t timeoutTicks);
bool        FEM_StageTimeout   (struct FEM *f, uint32_t timeoutTicks);
bool        FEM_StateCountdown (struct FEM *f, uint32_t timeoutTicks);
uint32_t    FEM_StateCountdownSeconds
                               (struct FEM *f);
bool        FEM_GotoStage      (struct FEM *f, enum FEM_Stage newStage);
bool        FEM_ChangeState    (struct FEM *f, FEM_StateFunc newState);
bool        FEM_Process        (struct FEM *f, uint32_t curTicks,
                                uint32_t timeoutTicks);
