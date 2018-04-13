#include "board_fixed.h"
#include "cooperativeOs_isr.h"
#include "cooperativeOs_scheduler.h"

/*
Actividad

    Re-implementar el ejercicio entregable 3, incorporando una interfaz UART
    bidireccional en base a los ejemplos vistos en la clase 5, que tenga
    desacoplada la lógica de funcionamiento según el modelo productor-
    consumidor:

    Incorporar mejores prácticas en base a la puesta en común hecha en clase.
    La interfaz UART debe utilizar 2 buffers, uno para tx y otro rx, de forma
    similar al ejemplo 2 de la presentación, que permita:

        Mostrar un menú en la terminal serie con la configuración del sistema.

        Realizar las mismas funciones que las teclas TEC1 y TEC2.

        [Opcional] Permita ingresar un tiempo de encendido entero hasta 1000ms
*/


volatile uint32_t g_ticks       = 0;
void (* volatile g_tickHook)()  = NULL;
/*
    LPC4337 dispone de "16 B Receive and Transmit FIFOs."
    con una velocidad de 115200 bps configurado como 8N1 (9 bits), es necesario
    que la tarea que levanta los datos de la FIFO de entrada se llame con una
    frecuencia minima de 115200 / 9 = 12800 bytes por segundo / 16 = 800
    FIFOs llenas por segundo, 1/800: 1.25 milisegundo. Como no existen valores
    intermedios, la tarea se debe ejecutar en cada milisegundo.

    A una velocidad de 9600 bps, la frecuencia de la tarea puede ser 12
    veces mas lenta: 15 milisegundos.

    En cuanto al buffer de entrada, uno de 160 bytes debe consumirse cada 12.5 o
    150 ms segun sea 115200 o 9600 bps.

    En la salida y para que la FIFO no overflowee, hay que enviar hasta 16 bytes
    de datos por cada entrada a la tarea en no menos que 2.50 o 300 ms segun sea
    115000 o 9600 (3 y 350 ms para estar seguros).
*/

#define UART_RECV_BUFFER_MAX    256
#define UART_SEND_BUFFER_MAX    1024


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

uint32_t uartRecvPutIndex   = 0;
uint32_t uartRecvGetIndex   = 0;
uint32_t uartSendPutIndex   = 0;
uint32_t uartSendGetIndex   = 0;

static uint8_t uartRecvBuffer [UART_RECV_BUFFER_MAX];
static uint8_t uartSendBuffer [UART_SEND_BUFFER_MAX];


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


static void sendMessage (const char* msg)
{
    while (*msg != '\0')
    {
        uartSendBuffer[uartSendPutIndex] = *msg ++;
        uartSendPutIndex = (uartSendPutIndex + 1) & (UART_SEND_BUFFER_MAX - 1);
    }
}


static void blinkLedTask ()
{
    Board_LED_Toggle (Led[ledCurrentIndex]);
}


static void nextLed ()
{
    Board_LED_Set (Led[ledCurrentIndex], true);
    if (Led[++ ledCurrentIndex] == INVALID_LED)
    {
        ledCurrentIndex = 0;
    }
}


static void nextPeriod ()
{
    if (HalfPeriod[++ halfPeriodIndex] == INVALID_HALF_PERIOD)
    {
        halfPeriodIndex = 0;
    }

    schedulerDeleteTask (ledTaskIndex);
    ledTaskIndex = schedulerAddTask (blinkLedTask, 0,
                                     HalfPeriod[halfPeriodIndex]);
}


static void debounceTec1Task ()
{
    if (buttonPressed (Board_BTN_TEC_1, &tec1Pressed))
    {
        nextPeriod ();
    }
}


static void debounceTec2Task ()
{
    if (buttonPressed (Board_BTN_TEC_2, &tec2Pressed))
    {
        nextLed ();
    }
}


static void uartRecvTask ()
{
    int byte;
    while ((byte = Board_UARTGetChar()) != EOF)
    {
        uartRecvBuffer[uartRecvPutIndex] = (uint8_t) byte;
        uartRecvPutIndex = (uartRecvPutIndex + 1) & (UART_RECV_BUFFER_MAX - 1);
    }
}


static void uartSendTask ()
{
    uint32_t sendBytes = (uartSendPutIndex - uartSendGetIndex) &
                            (UART_SEND_BUFFER_MAX - 1);
    if (!sendBytes)
    {
        return;
    }

    if (sendBytes > 15)
    {
        sendBytes = 15;
    }

    while (sendBytes --)
    {
        Board_UARTPutChar (uartSendBuffer[uartSendGetIndex]);
        uartSendGetIndex = (uartSendGetIndex + 1) & (UART_SEND_BUFFER_MAX - 1);
    }
}


static void uartProcessTask ()
{
    uint32_t recvBytes = (uartRecvPutIndex - uartRecvGetIndex) &
                          (UART_RECV_BUFFER_MAX - 1);
    if (!recvBytes)
    {
        return;
    }

    char        c[5];
    uint32_t    i = 0;

    // De lo que reciba solo proceso 4 bytes, el resto lo tiro
    while (i < recvBytes && i < 4)
    {
        c[i] = uartRecvBuffer[uartRecvGetIndex];
        uartRecvGetIndex = (uartRecvGetIndex + 1) & (UART_RECV_BUFFER_MAX - 1);
        ++ i;
    }
    c[i] = 0;
    uartRecvGetIndex = (uartRecvGetIndex + recvBytes - i) & (UART_RECV_BUFFER_MAX - 1);

    if (i == 1)
    {
        switch (c[0])
        {
            case 'i':
                sendMessage ("Bienvenido a \"Actividad 4\".\n"
                             "Comandos:\n"
                             "  'i': Este mensaje.\n"
                             "  'l': Siguiente led.\n"
                             "  'p': Siguiente periodo.\n");
                break;

            case 'l':
                sendMessage ("Siguiente Led.\n");
                nextLed ();
                break;

            case 'p':
                sendMessage ("Siguiente Periodo.\n");
                nextPeriod ();
                break;

            default:
                sendMessage ("Comando no reconocido: '");
                sendMessage (c);
                sendMessage ("'.\n");
                break;
        }
    }
}


int main (void)
{
    SystemCoreClockUpdate   ();
    Board_Init_Fixed        ();

    schedulerInit       ();
    ledTaskIndex = schedulerAddTask
                        (blinkLedTask       ,0  ,HalfPeriod[halfPeriodIndex]);
    schedulerAddTask    (debounceTec1Task   ,1  ,20);
    schedulerAddTask    (debounceTec2Task   ,2  ,20);
    schedulerAddTask    (uartRecvTask       ,0  ,1);
    schedulerAddTask    (uartSendTask       ,0  ,3);
    schedulerAddTask    (uartProcessTask    ,0  ,12);

    schedulerStart      (1);

    uartRecvBuffer[0]   = 'i';
    uartRecvPutIndex    = 1;

    while (1)
    {
        schedulerDispatchTasks ();
    }

    return 0;
}
