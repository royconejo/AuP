#include "board_fixed.h"
#include "cooperativeOs_isr.h"
#include "cooperativeOs_scheduler.h"

/*
Actividad

    Re-implementar el ejercicio entregable 2 con las siguientes modificaciones:

    Incorporar mejores prácticas de programación al código en base a la
    puesta en común hecha en clase.

    Definir 3 tareas en el S.O. de Pont visto en clase:
    blinkLedTask, con periodicidad de acuerdo al enunciado anterior y 0ms de
    retardo.
    debounceTec1Task, con periodicidad 20ms y un retardo de 1ms.
    debounceTec2Task, con periodicidad 20ms y un retardo de 2ms.
*/


volatile uint32_t g_ticks       = 0;
void (* volatile g_tickHook)()  = NULL;


void SysTick_Handler (void)
{
    ++ g_ticks;

    if (g_tickHook)
    {
        g_tickHook ();
    }
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


uint32_t ledCurrentIndex    = 0;
uint32_t halfPeriodIndex    = 0;
uint32_t tec1Pressed        = 0;
uint32_t tec2Pressed        = 0;
uint32_t ledTaskIndex       = 0;


static bool buttonPressed (enum Board_BTN button, uint32_t *pressed)
{
    if (!Board_BTN_State (button))
    {
        if (! *pressed)
        {
            *pressed = 1;
            return true;
        }
    }
    else if (*pressed)
    {
        *pressed = 0;
    }
    return false;
}


static void blinkLedTask ()
{
    Board_LED_Toggle (Led[ledCurrentIndex]);
}


static void debounceTec1Task ()
{
    if (buttonPressed (Board_BTN_TEC_1, &tec1Pressed))
    {
        if (HalfPeriod[++ halfPeriodIndex] == INVALID_HALF_PERIOD)
        {
            halfPeriodIndex = 0;
        }

        schedulerDeleteTask (ledTaskIndex);
        ledTaskIndex = schedulerAddTask (blinkLedTask, 0,
                                         HalfPeriod[halfPeriodIndex]);
    }
}


static void debounceTec2Task ()
{
    if (buttonPressed (Board_BTN_TEC_2, &tec2Pressed))
    {
        Board_LED_Set (Led[ledCurrentIndex], true);
        if (Led[++ ledCurrentIndex] == INVALID_LED)
        {
            ledCurrentIndex = 0;
        }
    }
}


int main (void)
{
    SystemCoreClockUpdate   ();
    Board_Init_Fixed        ();

    schedulerInit       ();
    ledTaskIndex = schedulerAddTask
                        (blinkLedTask      ,0  ,HalfPeriod[halfPeriodIndex]);
    schedulerAddTask    (debounceTec1Task  ,1  ,20);
    schedulerAddTask    (debounceTec2Task  ,2  ,20);
    schedulerStart      (1);

    while (1)
    {
        schedulerDispatchTasks ();
    }

    return 0;
}
