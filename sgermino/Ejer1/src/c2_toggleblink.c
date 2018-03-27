#include "board.h"

/*
Actividad

    Basándose en el ejemplo blinky provisto, escriba un programa que:

    1.  Cambie la frecuencia de parpadeo del led RGB rojo de la EDU-CIAA-NXP
        cada vez que se presione la tecla TEC1, de modo que el tiempo de
        encendido (1/2 período) alterne entre 50 y 100 milisegundos.

    2.  Cambie el led que parpadea cada vez que se presione la tecla TEC2,
        siguiendo circularmente la secuencia {RGB rojo, RGB verde, RGB azul}.

    3.  Utilice razonablemente la herramientas del lenguaje C.
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
    INVALID_LED
};

const uint32_t HalfPeriod[] =
{
    50,
    100,
    INVALID_HALF_PERIOD
};


uint32_t ledHalfPeriod      = 0;
uint32_t ledCurrentCount    = 0;
uint32_t ledCurrentIndex    = 0;
uint32_t halfPeriodIndex    = 0;
uint32_t tec1Pressed        = 0;
uint32_t tec2Pressed        = 0;


bool keyPressed (uint8_t port, uint8_t pin, uint32_t *pressed)
{
    if (!Chip_GPIO_GetPinState (LPC_GPIO_PORT, port, pin))
    {
        if (! *pressed)
        {
            *pressed = g_ticks + 200;
            return true;
        }
    }
    else if (*pressed < g_ticks)
    {
        *pressed = 0;
    }
    return false;
}


int main (void)
{
    SystemCoreClockUpdate    ();
    Board_Init               ();
    SysTick_Config           (SystemCoreClock / 1000);

    Chip_GPIO_SetPinDIRInput (LPC_GPIO_PORT, 0, 4);
    Chip_GPIO_SetPinDIRInput (LPC_GPIO_PORT, 0, 8);

    ledHalfPeriod = HalfPeriod[halfPeriodIndex];

    while (1)
    {
        if (g_ticks > ledCurrentCount + ledHalfPeriod)
        {
            ledCurrentCount = g_ticks + ledHalfPeriod;
        }

        Board_LED_Set (Led[ledCurrentIndex], ledCurrentCount > g_ticks);

        // TEC_1
        if (keyPressed (0, 4, &tec1Pressed))
        {
            if ((ledHalfPeriod = HalfPeriod[++ halfPeriodIndex]) == INVALID_HALF_PERIOD)
            {
                ledHalfPeriod = HalfPeriod[halfPeriodIndex = 0];
            }

        }

        // TEC_2
        if (keyPressed (0, 8, &tec2Pressed))
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
