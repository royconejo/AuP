#include "board.h"

#define PWM_PERIOD_RES  255
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

int main (void)
{
    SystemCoreClockUpdate    ();
    Board_Init               ();
    SysTick_Config           (SystemCoreClock / PWM_TICKS_HZ);

    uint8_t pwmLed1 = 0;
    uint8_t pwmLed2 = 85;
    uint8_t pwmLed3 = 170;

    while (1)
    {
        Board_LED_Set       (LED_1, (pwmLed1 <= g_pwmPeriod));
        Board_LED_Set       (LED_2, (pwmLed2 <= g_pwmPeriod));
        Board_LED_Set       (LED_3, (pwmLed3 <= g_pwmPeriod));

        if (g_newPeriod)
        {
            g_newPeriod = false;
            ++ pwmLed1;
            ++ pwmLed2;
            ++ pwmLed3;
        }

        __WFI();
    }

    return 0;
}
