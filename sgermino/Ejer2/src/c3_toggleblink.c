#include "board_fixed.h"

/*
Actividad

    Reimplementar el ejercicio entregable 1 con las siguientes modificaciones:

    1.  Incorporar mejores prácticas de programación al código en base a la
        puesta en común hecha en clase.

    2.  Utilizar delays no bloqueantes para el parpadeo de los leds.
        [OPCIONAL] utilizar una MEF para la lógica de parpadeo.

    3.  Utilizar antirebote por software mediante MEFs para TEC1 y TEC2.

    4.  Incorporar 1 nuevo parpadeo con tiempo de encendido 1000.
        Utilizar todos los leds: {LEDR, LEDG, LEDB, LED1, LED2, LED3}.
*/

volatile uint32_t g_ticks = 0;


void SysTick_Handler (void)
{
    ++ g_ticks;
}


#define INVALID_LED             0xFF
#define INVALID_HALF_PERIOD     0xFFFFFFFF


const uint8_t Led[] =
{
    LED_RED,
    LED_GREEN,
    LED_BLUE,
    LED_1,
    LED_2,
    LED_3,
    INVALID_LED
};

const uint32_t HalfPeriod[] =
{
    50,
    100,
    1000,
    INVALID_HALF_PERIOD
};


static bool buttonPressed (enum Board_BTN button, uint32_t *pressed)
{
    if (!Board_BTN_State (button))
    {
        if (! *pressed)
        {
            *pressed = g_ticks + 200;
            return true;
        }
    }
    else if (*pressed <= g_ticks)
    {
        *pressed = 0;
    }
    return false;
}


int main (void)
{
    SystemCoreClockUpdate   ();
    Board_Init_Fixed        ();
    SysTick_Config          (SystemCoreClock / 1000);

    uint32_t ledCurrentIndex    = 0;
    uint32_t halfPeriodIndex    = 0;
    uint32_t ledHalfPeriod      = HalfPeriod[halfPeriodIndex];
    uint32_t ledCurrentCount    = 0;
    uint32_t tec1Pressed        = 0;
    uint32_t tec2Pressed        = 0;

    while (1)
    {
        if (g_ticks >= ledCurrentCount + ledHalfPeriod)
        {
            ledCurrentCount = g_ticks + ledHalfPeriod;
        }

        Board_LED_Set (Led[ledCurrentIndex], ledCurrentCount > g_ticks);

        // TEC_1
        if (buttonPressed (Board_BTN_TEC_1, &tec1Pressed))
        {
            if ((ledHalfPeriod = HalfPeriod[++ halfPeriodIndex]) == INVALID_HALF_PERIOD)
            {
                ledHalfPeriod = HalfPeriod[halfPeriodIndex = 0];
            }

        }

        // TEC_2
        if (buttonPressed (Board_BTN_TEC_2, &tec2Pressed))
        {
            Board_LED_Set (Led[ledCurrentIndex], true);
            if (Led[++ ledCurrentIndex] == INVALID_LED)
            {
                ledCurrentIndex = 0;
            }
        }

        __WFI ();
    }

    return 0;
}
