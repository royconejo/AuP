#include "board.h"

#define PWM_PERIOD_RES  256
#define PWM_PERIOD_HZ   100
#define PWM_TICKS_HZ    (PWM_PERIOD_HZ * PWM_PERIOD_RES)
// SystemCoreClock = 204000000

volatile uint8_t     g_pwmPeriod   = 0;
volatile bool        g_newPeriod   = false;
volatile uint32_t    g_ticksSecond = PWM_TICKS_HZ;
volatile bool        g_newSecond   = false;


void SysTick_Handler (void)
{
    if (!(++ g_pwmPeriod))
    {
        g_newPeriod = true;
        if (!(g_ticksSecond -= PWM_PERIOD_RES))
        {
            g_ticksSecond   = PWM_TICKS_HZ;
            g_newSecond     = true;
        }
    }
}


#define ledPwmRepeatForever     0xFFFFFFFF


enum ledPwmTransition
{
    ledPwmTransition_IN,
    ledPwmTransition_OUT,
    ledPwmTransition_PING_PONG
};


struct ledPwm_t
{
    uint32_t    minIntensity;
    uint32_t    maxIntensity;
    uint32_t    curIntensity;
    uint32_t    intensityStep;
    enum ledPwmTransition
                transition;
    uint32_t    holdPeriods;
    uint32_t    repeat;
    uint32_t    initHoldPeriods;
    bool        forward;
    uint8_t     pin;
    uint8_t     initIntensity;
};


void ledPwmUpdate (struct ledPwm_t *lp)
{
    if (!lp || !lp->repeat)
    {
        return;
    }

    if (lp->holdPeriods)
    {
        if (!(-- lp->holdPeriods) && lp->repeat != ledPwmRepeatForever)
        {
            -- lp->repeat;
        }
        return;
    }

    do
    {
        if (lp->transition == ledPwmTransition_IN
                || (lp->transition == ledPwmTransition_PING_PONG
                    && lp->forward))
        {
            lp->curIntensity += lp->intensityStep;
            if (lp->curIntensity > lp->maxIntensity)
            {
                break;
            }
        }
        else if (lp->transition == ledPwmTransition_OUT
                || (lp->transition == ledPwmTransition_PING_PONG
                    && !lp->forward))
        {
            lp->curIntensity -= lp->intensityStep;
            if (lp->curIntensity < lp->intensityStep ||
                lp->curIntensity < lp->minIntensity)
            {
                break;
            }
        }
        return;
    }
    while (0);

    do
    {
        if (lp->transition == ledPwmTransition_IN)
        {
            lp->curIntensity    = lp->repeat? lp->minIntensity : lp->maxIntensity;
        }
        else if (lp->transition == ledPwmTransition_OUT)
        {
            lp->curIntensity    = lp->repeat? lp->maxIntensity : lp->minIntensity;
        }
        else if (lp->transition == ledPwmTransition_PING_PONG)
        {
            lp->curIntensity    = lp->forward? lp->maxIntensity : lp->minIntensity;
            lp->forward         = !lp->forward;

            if (lp->forward)
            {
                break;
            }
        }

        if (lp->initHoldPeriods)
        {
            lp->holdPeriods = lp->initHoldPeriods;
        }
        else if (lp->repeat != ledPwmRepeatForever)
        {
            -- lp->repeat;
        }
    }
    while (0);
}


void ledPwmShow (struct ledPwm_t *lp)
{
    if (!lp)
    {
        return;
    }

    Board_LED_Set (lp->pin, (lp->curIntensity >> 16) <= g_pwmPeriod);
}


void ledPwmInit (struct ledPwm_t *lp,
                 uint8_t pin,
                 uint8_t minIntensity,
                 uint8_t maxIntensity,
                 uint8_t curIntensity,
                 uint32_t transitionMsecs,
                 uint32_t holdMsecs,
                 enum ledPwmTransition transition,
                 uint32_t repeat)
{
    if (!lp)
    {
        return;
    }

    curIntensity = (curIntensity < minIntensity)? minIntensity :
                        (curIntensity > maxIntensity)? maxIntensity :
                            curIntensity;
    /*
        La intensidad del LED se actualiza cada 1 periodo. La diferencia entre
        la intensidad MAX y MIN devuelve la cantidad de periodos (transiciones)
        entre ambos estados. La cantidad de periodos en un segundo es constante
        (PWM_PERIOD_HZ) y se interpola a la duracion especificada en
        milisegundos para obtener la magnitud del "step" por cada periodo. La
        animacion puede durar menos si curIntensity es mayor a minIntensity y/o
        menor a maxIntensity (esto segun en que direccion vaya la animacion).
    */
    const uint32_t  Periods             = maxIntensity -  minIntensity;
    const double    DurationSecs        = transitionMsecs * 0.001;
    const double    PeriodsInDuration   = PWM_PERIOD_HZ * DurationSecs;
    const double    IntensityStep       = Periods / PeriodsInDuration;
    const double    HoldPeriods         = holdMsecs * 0.001 * PWM_PERIOD_HZ;

    lp->pin             = pin;
    lp->initIntensity   = curIntensity;
    lp->minIntensity    = ((uint32_t)minIntensity) << 16;
    lp->maxIntensity    = ((uint32_t)maxIntensity) << 16;
    lp->curIntensity    = ((uint32_t)curIntensity) << 16;
    lp->forward         = true;
    lp->intensityStep   = IntensityStep * (1 << 16);
    lp->transition      = transition;
    lp->initHoldPeriods = HoldPeriods;
    lp->holdPeriods     = 0;
    lp->repeat          = repeat;

    if (lp->transition == ledPwmTransition_PING_PONG)
    {
        lp->intensityStep <<= 1;
    }
}


int main (void)
{
    SystemCoreClockUpdate    ();
    Board_Init               ();
    SysTick_Config           (SystemCoreClock / PWM_TICKS_HZ);

    struct ledPwm_t led1, led2, led3;

    ledPwmInit (&led1, LED_1, 0, 255, 0, 300, 200,
                ledPwmTransition_PING_PONG, ledPwmRepeatForever);

    ledPwmInit (&led2, LED_2, 0, 255, 85, 3000, 1000,
                ledPwmTransition_PING_PONG, ledPwmRepeatForever);

    ledPwmInit (&led3, LED_3, 0, 255, 170, 5000, 0,
                ledPwmTransition_PING_PONG, ledPwmRepeatForever);

    while (1)
    {
        ledPwmShow (&led1);
        ledPwmShow (&led2);
        ledPwmShow (&led3);

        if (g_newPeriod)
        {
            g_newPeriod = false;
            ledPwmUpdate (&led1);
            ledPwmUpdate (&led2);
            ledPwmUpdate (&led3);
        }

        if (g_newSecond)
        {
            g_newSecond = false;
            Board_LED_Toggle (LED_RED);
        }

        __WFI();
    }

    return 0;
}
