#include "board_fixed.h"
#include "uart.h"
#include "indata.h"
#include "btn.h"
#include "text.h"
#include "msgs.h"
#include "copos.h"
#include <string.h>

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


/*
    con cutecom la interaccion con el usuario se dificulta ya que no es un
    emulador de terminal ni trabaja con mismo la terminal del sistema.

    En particular, no es posible generar un prompt para que el usuario pueda
    ingresar y editar un valor. Otra desventaja importante es que solo
    representa ASCII basico (no acentos ni enie).

    iniciar picocom:

    picocom /dev/ttyUSB1 --baud=9600
    salir con C-a C-q
*/

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

    En la salida y para que la FIFO no overflowee por tirarle mas datos de los
    que puede enviar, hay que enviar hasta 16 bytes de datos por cada entrada a
    la tarea en no menos de 2 a 15 ms segun sea 115000 o 9600.
*/


volatile uint32_t g_ticks                       = 0;
void (* volatile g_tickHook)(uint32_t ticks)    = NULL;


void SysTick_Handler (void)
{
    ++ g_ticks;

    if (g_tickHook)
    {
        g_tickHook (g_ticks);
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


const char *LedName[] =
{
    "rgb rojo",
    "rgb verde",
    "rgb azul",
    "amarillo",
    "rojo",
    "verde"
};


const uint32_t HalfPeriod[] =
{
    50,
    100,
    1000,
    INVALID_HALF_PERIOD
};


enum ProcessTaskActivity
{
    ProcessTaskActivity_GetCommand = 0,
    ProcessTaskActivity_GetPeriodInput
};


struct APP_Context
{
    struct UART_Context         uartCtx;
    struct INDATA_Context       indataCtx;
    enum ProcessTaskActivity    processTaskActivity;

    uint32_t ledCurrentIndex;
    uint32_t halfPeriodIndex;
    uint32_t tec1Pressed;
    uint32_t tec2Pressed;
    uint32_t ledTaskIndex;
};


static void currentLedMessage (struct UART_Context *uartCtx,
                               const char *ledName)
{
    UART_PutMessage (uartCtx, MSGS_INFO_BEGIN "Led ");
    UART_PutMessage (uartCtx, ledName);
    UART_PutMessage (uartCtx, "." MSGS_INFO_END);
}


static void currentPeriodMessage (struct UART_Context *uartCtx, uint32_t period)
{
    char strnum[12];
    snprintf (strnum, sizeof(strnum), "%lu", period);

    UART_PutMessage (uartCtx, MSGS_INFO_BEGIN "Periodo ");
    UART_PutMessage (uartCtx, strnum);
    UART_PutMessage (uartCtx, " ms." MSGS_INFO_END);
}


static void bufferStatsMessage (struct UART_Context *ctx)
{
    char strnum[12];
    UART_PutMessage (ctx, MSGS_INFO_BEGIN
                     "Estado de los buffers de I/O." MSGS_NL);

    UART_PutMessage (ctx, MSGS_PREFIX_GROUP "Send writes      : ");
    snprintf (strnum, sizeof(strnum), "%lu", ctx->sendWrites);
    UART_PutMessage (ctx, strnum);
    UART_PutMessage (ctx, " bytes." MSGS_NL);

    UART_PutMessage (ctx, MSGS_PREFIX_GROUP "Send overflow    : ");
    snprintf (strnum, sizeof(strnum), "%lu", ctx->sendOverflow);
    UART_PutMessage (ctx, strnum);
    UART_PutMessage (ctx, " bytes." MSGS_NL);

    UART_PutMessage (ctx, MSGS_PREFIX_GROUP "Receive writes   : ");
    snprintf (strnum, sizeof(strnum), "%lu", ctx->recvWrites);
    UART_PutMessage (ctx, strnum);
    UART_PutMessage (ctx, " bytes." MSGS_NL);

    UART_PutMessage (ctx, MSGS_PREFIX_GROUP "Receive overflow : ");
    snprintf (strnum, sizeof(strnum), "%lu", ctx->recvOverflow);
    UART_PutMessage (ctx, strnum);
    UART_PutMessage (ctx, " bytes.");

    UART_PutMessage (ctx, MSGS_INFO_END);
}


static void nextLed (struct APP_Context *ctx)
{
    Board_LED_Set (Led[ctx->ledCurrentIndex], true);
    if (Led[++ ctx->ledCurrentIndex] == INVALID_LED)
    {
        ctx->ledCurrentIndex = 0;
    }
    currentLedMessage (&ctx->uartCtx, LedName[ctx->ledCurrentIndex]);
}


static void nextPeriod (struct APP_Context *ctx)
{
    if (HalfPeriod[++ ctx->halfPeriodIndex] == INVALID_HALF_PERIOD)
    {
        ctx->halfPeriodIndex = 0;
    }

    schedulerModifyTaskPeriod (ctx->ledTaskIndex,
                               HalfPeriod[ctx->halfPeriodIndex]);

    currentPeriodMessage (&ctx->uartCtx, HalfPeriod[ctx->halfPeriodIndex] << 1);
}


void blinkLedTask (void *ctx, uint32_t ticks)
{
    struct APP_Context *appCtx = (struct APP_Context *) ctx;
    Board_LED_Toggle (Led[appCtx->ledCurrentIndex]);
}


void debounceTec1Task (void *ctx, uint32_t ticks)
{
    struct APP_Context *appCtx = (struct APP_Context *) ctx;
    if (BTN_DebouncePressed (BOARD_TEC_1, ticks, 0, &appCtx->tec1Pressed))
    {
        nextPeriod (appCtx);
    }
}


void debounceTec2Task (void *ctx, uint32_t ticks)
{
    struct APP_Context *appCtx = (struct APP_Context *) ctx;
    if (BTN_DebouncePressed (BOARD_TEC_2, ticks, 0, &appCtx->tec2Pressed))
    {
        nextLed (appCtx);
    }
}


void uartRecvTask (void *ctx, uint32_t ticks)
{
    struct UART_Context *uartCtx = (struct UART_Context *) ctx;
    UART_Recv (uartCtx);
}


void uartSendTask (void *ctx, uint32_t ticks)
{
    struct UART_Context *uartCtx = (struct UART_Context *) ctx;
    UART_Send (uartCtx);
}


static void processCommand (struct APP_Context *ctx)
{
    struct UART_Context   *uartCtx = (struct UART_Context *)   &ctx->uartCtx;
    struct INDATA_Context *indaCtx = (struct INDATA_Context *) &ctx->indataCtx;

    const uint32_t Pending = UART_RecvPendingCount (uartCtx);
    if (!Pending)
    {
        // Nada que procesar
        return;
    }

    if (Pending != 1)
    {
        UART_PutMessage (uartCtx, TEXT_WRONGCOMMANDSIZE);
        return;
    }

    const uint8_t Command = UART_RecvPeek (uartCtx, 0);
    switch (Command)
    {
        case 'i':
            UART_PutMessage (uartCtx, TEXT_WELCOME);
            break;

        case 'p':
            nextPeriod (ctx);
            break;

        case 'l':
            nextLed (ctx);
            break;

        case 'n':
            UART_PutMessage (uartCtx, TEXT_PERIODINPUT);

            ctx->processTaskActivity = ProcessTaskActivity_GetPeriodInput;
            INDATA_Begin (indaCtx, INDATA_TypeDecimal);
            break;

        case 's':
            bufferStatsMessage (uartCtx);
            break;

        case 'c':
            UART_PutMessage (uartCtx, TERM_CLEAR_SCREEN);
            break;

        default:
            UART_PutMessage (uartCtx, TEXT_WRONGCOMMAND);
            break;
    }
}


void uartProcessTask (void *ctx, uint32_t ticks)
{
    struct APP_Context      *appCtx = (struct APP_Context *) ctx;
    struct UART_Context     *uartCtx;
    struct INDATA_Context   *indaCtx;

    uartCtx = (struct UART_Context *)   &appCtx->uartCtx;
    indaCtx = (struct INDATA_Context *) &appCtx->indataCtx;

    switch (appCtx->processTaskActivity)
    {
        case ProcessTaskActivity_GetCommand:
            processCommand (appCtx);
            break;

        case ProcessTaskActivity_GetPeriodInput:
        {
            const enum INDATA_Status InSt = INDATA_Status (indaCtx);
            if (InSt == INDATA_StatusPrompt)
            {
                INDATA_Process (indaCtx);
                break;
            }
            else if (InSt == INDATA_StatusReady)
            {
                const int HalfPeriod = INDATA_Decimal (indaCtx) >> 1;
                schedulerModifyTaskPeriod (appCtx->ledTaskIndex, HalfPeriod);
                currentPeriodMessage (uartCtx, HalfPeriod << 1);
            }

            INDATA_End (indaCtx);
            appCtx->processTaskActivity = ProcessTaskActivity_GetCommand;
            break;
        }

        default:
            UART_PutMessage (uartCtx, TEXT_INVALIDTASKSTATUS);
            INDATA_End (indaCtx);
            appCtx->processTaskActivity = ProcessTaskActivity_GetCommand;
            break;
    }

    // uartProcessTask siempre debe consumir todos los datos disponibles
    UART_RecvConsumePending (uartCtx);
}


bool APP_Init (struct APP_Context *ctx)
{
    if (!ctx)
    {
        return false;
    }

    memset (ctx, 0, sizeof(struct APP_Context));

    UART_Init   (&ctx->uartCtx, DEBUG_UART, 9600);
    INDATA_Init (&ctx->indataCtx, &ctx->uartCtx);
    return true;
}


int main (void)
{
    SystemCoreClockUpdate   ();
    Board_Init_Fixed        ();

    struct APP_Context  appCtx;
    APP_Init (&appCtx);

    // Inserta comando para mostrar menu al principio del programa
    UART_RecvInjectByte (&appCtx.uartCtx, 'i');

    const uint32_t Hp = HalfPeriod[appCtx.halfPeriodIndex];

    schedulerInit       ();
    appCtx.ledTaskIndex = schedulerAddTask
                        (blinkLedTask       ,&appCtx            ,0  ,Hp);
    schedulerAddTask    (debounceTec1Task   ,&appCtx            ,1  ,20);
    schedulerAddTask    (debounceTec2Task   ,&appCtx            ,2  ,20);
    schedulerAddTask    (uartRecvTask       ,&appCtx.uartCtx    ,0  ,14);
    schedulerAddTask    (uartSendTask       ,&appCtx.uartCtx    ,0  ,16);
    schedulerAddTask    (uartProcessTask    ,&appCtx            ,0  ,140);
    schedulerStart      (1);

    while (1)
    {
        schedulerDispatchTasks (g_ticks);
    }

    return 0;
}
