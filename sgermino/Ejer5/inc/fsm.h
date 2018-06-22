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


#ifndef FSM_MAX_RECURRING_CALLS
    #define FSM_MAX_RECURRING_CALLS     99
#endif


enum FSM_Stage
{
    FSM_StageBegin = 0,
    FSM_StageMain,
    FSM_StageEnd,
    FSM_Stage_LAST_ = FSM_StageEnd
};


enum FSM_StateReturn
{
    FSM_StateReturnYield = 0,
    FSM_StateReturnAgain
};


struct FEM;
typedef enum FSM_StateReturn (* FSM_StateFunc) (struct FEM *f,
                                         enum FSM_Stage stage, uint32_t ticks);


struct FEM
{
    FSM_StateFunc   state;
    enum FSM_Stage  stage;
    const char      *info;
    uint32_t        stateCalls;
    uint32_t        stageCalls;
    uint32_t        stateStartTicks;
    uint32_t        stageStartTicks;
    uint32_t        stateCountdownTicks;
    void            *app;
    FSM_StateFunc   invalidStage;
    FSM_StateFunc   maxRecCalls;
};


bool        FSM_Init           (struct FEM *f, void *app);
bool        FSM_SetErrorStates (struct FEM *f,
                                FSM_StateFunc invalidStage,
                                FSM_StateFunc maxRecurringCalls);
bool        FSM_SetStateInfo   (struct FEM *f, const char* info);
bool        FSM_StateTimeout   (struct FEM *f, uint32_t timeoutTicks);
bool        FSM_StageTimeout   (struct FEM *f, uint32_t timeoutTicks);
bool        FSM_StateCountdown (struct FEM *f, uint32_t timeoutTicks);
uint32_t    FSM_StateCountdownSeconds
                               (struct FEM *f);
bool        FSM_GotoStage      (struct FEM *f, enum FSM_Stage newStage);
bool        FSM_ChangeState    (struct FEM *f, FSM_StateFunc newState);
bool        FSM_Process        (struct FEM *f, uint32_t curTicks,
                                uint32_t timeoutTicks);
