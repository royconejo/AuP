/*
    PWM Library
    Copyright (C) 2018 Santiago Germino

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

    You can contact the author by e-mail: royconejo at gmail dot com
*/

#include "pwm.h"
#include "cmsis.h"


volatile uint8_t    g_pwmPeriod     = 0;
volatile bool       g_pwmNewPeriod  = false;
volatile uint32_t   g_pwmTicksToMs  = PWM_TICKS_MS;
volatile uint32_t   g_time          = 0;


void SysTick_Handler (void)
{
    if (!(++ g_pwmPeriod))
    {
        g_pwmNewPeriod = true;
    }

    if (!(-- g_pwmTicksToMs))
    {
        g_pwmTicksToMs  = PWM_TICKS_MS;
        ++ g_time;
    }
}


void PWM_ResetAnimation (struct PWM_AnimationContext *ctx, uint32_t outputData,
                         void(* outputUpdate)(uint32_t outputData, bool state))
{
    if (!ctx)
    {
        return;
    }

    memset (ctx, 0, sizeof(struct PWM_AnimationContext));

    ctx->outputData      = outputData;
    ctx->outputUpdate    = outputUpdate;
}


static void updateTransitionDuration (struct PWM_AnimTransData* td,
                                      uint32_t duration)
{
    //  TODO: assert (td->current > td->min && td->current < td->max)

    if (!duration)
    {
        td->step        = 0;
        td->duration    = 0;
        td->elapsed     = 0;
        return;
    }

    // Los calculos para determinar el step se realizan sobre el duration
    // proporcional segun el duration y elapsed anterior al cambio
    // (si la animacion estaba a mitad de esta transicion, va a seguir en la
    // mitad pero con nueva duracion y stepping)
    const double    ElapsedPercent = td->elapsed / (double)td->duration;

    td->duration    = duration;
    td->elapsed     = td->duration * ElapsedPercent;

    // La intensidad del LED se actualiza cada 1 periodo. La diferencia entre
    // la intensidad MAX y MIN devuelve la cantidad de periodos (transiciones)
    // entre ambos estados. La cantidad de periodos en un segundo es constante
    // (PWM_PERIOD_HZ) y se interpola a la duracion especificada en
    // milisegundos para obtener la magnitud del "step" por cada periodo.
    const double    DurationSecs = ((double)td->duration - td->elapsed) * 0.001;
    const double    PeriodsInDuration = PWM_PERIOD_HZ * DurationSecs;

    switch (td->type)
    {
        case PWM_AnimTransType_IN:
        case PWM_AnimTransType_OUT:
        {
            const uint8_t ProportionalPeriods = FIXED_U1616_TO_U8(td->max) -
                                                FIXED_U1616_TO_U8(td->current);
            td->step = DOUBLE_TO_FIXED_1616(ProportionalPeriods /
                                            PeriodsInDuration);
            break;
        }
        case PWM_AnimTransType_HOLD:
            td->step = DOUBLE_TO_FIXED_1616(PeriodsInDuration);
            break;
    }
}


void PWM_SetAnimationTransition (struct PWM_AnimationContext *ctx,
                                 uint32_t number, enum PWM_AnimTransType type,
                                 uint8_t min, uint8_t max, uint32_t duration)
{
    if (!ctx || number > 3)
    {
        return;
    }

    struct PWM_AnimTransData* td = &ctx->transition.data[number];
    td->type        = type;
    td->min         = ((uint32_t)min << 16);
    td->max         = ((uint32_t)max << 16);
    td->step        = 0;
    td->duration    = duration;
    td->elapsed     = 0;

    updateTransitionDuration (td, duration);
}


void PWM_ClearAnimationTransition (struct PWM_AnimationContext *ctx, uint32_t number)
{
    PWM_SetAnimationTransition (ctx, number, PWM_AnimTransType_IN, 0, 0, 0);
}


static void rewindTransition (struct PWM_AnimTransData* td)
{
    switch (td->type)
    {
        case PWM_AnimTransType_IN:
            td->current = td->min;
            break;

        case PWM_AnimTransType_OUT:
            td->current = td->max;
            break;

        case PWM_AnimTransType_HOLD:
            td->current = td->min;
            td->max     = 0;
            break;
    }

    td->elapsed = 0;
}


static void animationSetNextTransition (struct PWM_AnimationContext *ctx)
{
    struct PWM_AnimTransData* td = &ctx->transition.data[ctx->transition.current];

    uint32_t testTransitions = 4;
    while (testTransitions --)
    {
        ctx->transition.current = (ctx->transition.current + 1) & 0x3;
        td = &ctx->transition.data[ctx->transition.current];

        if (td->duration)
        {
            rewindTransition (td);
            return;
        }
    }

    ctx->status == PWM_AnimStatus_STOP;
}


static void updateAnimation (struct PWM_AnimationContext *ctx)
{
    if (!ctx || ctx->status == PWM_AnimStatus_STOP)
    {
        return;
    }

    struct PWM_AnimTransData* td = &ctx->transition.data[ctx->transition.current];

    switch (td->type)
    {
        case PWM_AnimTransType_IN:
            td->current += td->step;
            if (td->current > td->max)
            {
                ctx->intensity = FIXED_U1616_TO_U8(td->max);
                animationSetNextTransition (ctx);
                return;
            }
            break;

        case PWM_AnimTransType_OUT:
            td->current -= td->step;
            if (td->current < td->step || td->current < td->min)
            {
                ctx->intensity = FIXED_U1616_TO_U8(td->min);
                animationSetNextTransition (ctx);
                return;
            }
            break;

        case PWM_AnimTransType_HOLD:
            td->max += td->step;
            if (td->max >= PWM_PERIOD_RES)
            {
                animationSetNextTransition (ctx);
            }
            break;
    }

    ctx->intensity = FIXED_U1616_TO_U8(td->current);
}


void PWM_StartAnimation (struct PWM_AnimationContext *ctx)
{
    if (!ctx)
    {
        return;
    }

    struct PWM_AnimTransData* td;

    for (ctx->transition.current = 0; ctx->transition.current < 4;
         ++ ctx->transition.current)
    {
        td = &ctx->transition.data[ctx->transition.current];
        if (td->duration)
        {
            rewindTransition (td);
            ctx->intensity   = td->current;
            ctx->status      = PWM_AnimStatus_PLAYING;
            return;
        }
    }
}


void PWM_UpdateAnimation (struct PWM_AnimationContext *ctx)
{
    if (!ctx)
    {
        return;
    }

    const bool NewOutputState = ctx->intensity <= g_pwmPeriod;
    if (ctx->outputUpdate && NewOutputState != ctx->outputLastState)
    {
        ctx->outputUpdate (ctx->outputData, NewOutputState);
        ctx->outputLastState = NewOutputState;
    }

    if (g_pwmNewPeriod)
    {
        updateAnimation (ctx);
    }
}


void PWM_WaitNextCycle ()
{
    g_pwmNewPeriod = false;
    __WFI();
}
