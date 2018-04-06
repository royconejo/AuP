/*
    PWM Library.
    Copyright (C) 2018 Santiago Germino.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    ===============
    You can contact the author by e-mail:
    royconejo (at) gmail (dot) com
*/

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <string.h>


// Para SystemCoreClock = 204000000
// PERIOD_RES * PERIOD_HZ = 32000 / 1000 = 32
// 32 ciclos por cada millisegundo.
// El valor de PERIOD_HZ se eligio para evitar flicker en la animacion y porque
// multiplicado por PERIOD_HZ, da un numero redondo divisible por 1000!
#define PWM_PERIOD_RES      256
#define PWM_PERIOD_HZ       125
#define PWM_TICKS_HZ        (PWM_PERIOD_HZ * PWM_PERIOD_RES)
#define PWM_TICKS_MS        (PWM_TICKS_HZ / 1000)

#define PWM_AnimationRepeatForever     0xFFFFFFFF

// FIXME: Buscar un lugar donde meter esto de fixed point!
#define FIXED_U1616_TO_U8(n) ((n >> 16) & 0xFF)
#define DOUBLE_TO_FIXED_1616(n) (n * (1 << 16))


typedef uint32_t fixed_u1616;


enum PWM_AnimStatus
{
    PWM_AnimStatus_STOP = 0,
    PWM_AnimStatus_PLAYING
};


enum PWM_AnimTransType
{
    PWM_AnimTransType_IN,
    PWM_AnimTransType_OUT,
    PWM_AnimTransType_HOLD
};


struct PWM_AnimTransData
{
    enum PWM_AnimTransType  type;
    fixed_u1616             min;
    fixed_u1616             max;
    fixed_u1616             step;
    fixed_u1616             current;
    uint32_t                duration;
    uint32_t                elapsed;
};


struct PWM_AnimationContext
{
    struct
    {
        uint32_t                    current;
        struct PWM_AnimTransData    data[4];
    }                       transition;

    enum PWM_AnimStatus     status;
    uint32_t                outputData;
    void                    (* outputUpdate)(uint32_t outputData, bool state);
    bool                    outputLastState;
    uint8_t                 intensity;
};


void PWM_ResetAnimation             (struct PWM_AnimationContext *ctx, uint32_t outputData,
                                     void(* outputUpdate)(uint32_t outputData, bool state));
void PWM_SetAnimationTransition     (struct PWM_AnimationContext *ctx,
                                     uint32_t number, enum PWM_AnimTransType type,
                                     uint8_t min, uint8_t max, uint32_t duration);
void PWM_ClearAnimationTransition   (struct PWM_AnimationContext *ctx, uint32_t number);
void PWM_StartAnimation             (struct PWM_AnimationContext *ctx);
void PWM_UpdateAnimation            (struct PWM_AnimationContext *ctx);
void PWM_WaitNextCycle              ();
