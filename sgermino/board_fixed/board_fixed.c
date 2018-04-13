#include "board_fixed.h"

// Este archivo es necesario dada la imposibilidad de patchear las libreras
// en el repositorio usado en el curso.


#define BOARD_GPIO_BUTTONS_SIZE (sizeof(gpioButtons) / sizeof(struct io_port_t))


struct io_port_t
{
   uint8_t port;
   uint8_t pin;
};


static const struct io_port_t gpioButtons[] = {
    {0, 4}, {0, 8}, {0, 9}, {1, 9}
};


static void Board_UART_Init_Fixed (LPC_USART_T *pUART)
{
    Chip_SCU_PinMuxSet (7, 1, SCU_MODE_INACT | SCU_MODE_FUNC6);
    Chip_SCU_PinMuxSet (7, 2, SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_FUNC6);
}


static void Board_Debug_Init_Fixed ()
{
#ifdef DEBUG_UART
    Chip_UART_TXDisable     (DEBUG_UART);
    Board_UART_Init_Fixed   (DEBUG_UART);
    Chip_UART_Init          (DEBUG_UART);
    Chip_UART_SetBaudFDR    (DEBUG_UART, 115200);
    Chip_UART_ConfigData    (DEBUG_UART, UART_LCR_WLEN8 | UART_LCR_PARITY_DIS |
                             UART_LCR_SBS_1BIT);
    Chip_UART_TXEnable      (DEBUG_UART);
#endif
}


static void Board_BTN_Init ()
{
    for (uint32_t i = 0; i < BOARD_GPIO_BUTTONS_SIZE; ++i)
    {
        Chip_GPIO_SetPinDIRInput (LPC_GPIO_PORT, gpioButtons[i].port,
                                  gpioButtons[i].pin);
    }
}


bool Board_BTN_State (enum Board_BTN button)
{
   if (button < 0 || button >= BOARD_GPIO_BUTTONS_SIZE)
   {
       return false;
   }

   return Chip_GPIO_GetPinState (LPC_GPIO_PORT, gpioButtons[button].port,
                                 gpioButtons[button].pin);
}


void Board_Init_Fixed ()
{
    // Inicia el codigo para board Edu-CIAA en LPCOpen
    Board_Init ();

#ifdef DEBUG_ENABLE
    // Arregla la configuracion de USART2 (debug en FT2232)
    Board_Debug_Init_Fixed ();
#endif

    // Arregla el LED_GREEN: pin 81, P2_1, selecciona funcion GPIO5[1].
    // Esta misma configuracion ya se esta seteando en Board_SystemInit()
    // (board_sysinit.c en LPCOpen) llamada antes de main.c.
    // Pero luego en main.c, al llamar a Board_Init() en board.c,
    // Board_UART_Init() reconfigura el P2_1 (LEDG) como U0_RXD y P6_4 (GPIO1)
    // como U0_TXD!!!
    Chip_SCU_PinMuxSet (2, 1, SCU_MODE_INBUFF_EN | SCU_MODE_PULLUP |
                        SCU_MODE_FUNC4);

    // Configura los pulsadores TEC_1-4.
    // En board_sysinit.c se configura el SCU para input, pero en board.c
    // no se configura la direccion del GPIO! (si se hace con los LEDs)
    Board_BTN_Init ();
}
