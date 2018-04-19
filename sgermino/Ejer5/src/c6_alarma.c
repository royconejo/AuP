#include "board_fixed.h"
#include "uart.h"
#include "uart_util.h"
#include "indata.h"
#include "fem.h"
#include "btn.h"
#include "copos.h"
#include "systick.h"
#include "text_app.h"
#include <string.h>

/*
Actividad

    Implementar un sistema de alarma domiciliaria como el visto en el ejemplo de
    la presentación de la clase 6, con las siguientes consideraciones:

    1)  El sistema debe implementarse sobre el S.O. cooperativo visto en clase.
    2)  Para la lógica de control de la alarma debe utilizarse una tarea
        multiestado dentro de una MEF que se actualice cada 50ms.
    3)  Utilizar la interfaz UART para indicar el estado de la alarma, los
        eventos de puerta y ventanas
    4)  La contraseña para armar o desarmar la alarma se debe ingresar por la
        UART.
    5)  Utilizar el modelo productor consumidor con 2 buffers para la interfaz
        UART.
    6)  Cuando la alarma esté desarmada se debe encender el led RGB verde fijo.
    7)  Cuando la alarma esté armada se debe encender el led RGB rojo fijo
    8)  Cuando la alarma esté disparada (intruso) se debe parpadear el led RGB
        rojo.
    9)  Cuando se esté esperando que el usuario ingrese una contraseña se debe
        encender el led RGB azul.
    10) La TEC1 simula el sensor de puerta.
    11) TEC2, TEC3 y TEC4 simulan sensores de ventana.
    12) Todas las teclas deben tener antirebote.
    13) [opcional] Indicar mediante los leds 1-3 qué sensor de ventana fue
        activado (desde que se dispara la alarma hasta que esta se desarma).
    14) [opcional] Separar en distintos archivos fuentes las funciones como
        bibliotecas
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


#define     SENSOR_DOOR         0x00000001;
#define     SENSOR_WINDOW_1     0x00000002;
#define     SENSOR_WINDOW_2     0x00000004;
#define     SENSOR_WINDOW_3     0x00000008;


struct APP_Context
{
    struct UART_Context     uartCtx;
    struct INDATA_Context   indataCtx;
    struct FEM_Context      mainFemCtx;

    uint32_t    sensorStatus;
    uint32_t    tecDebounce[4];
};


enum FEM_StateReturn FEMState_Desarmada (struct FEM_Context *ctx,
                                        enum FEM_Stage stage, uint32_t ticks)
{
    FEM_SetStateInfo (ctx, __FUNCTION__);
    struct APP_Context *appCtx = (struct APP_Context *) ctx->appCtx;

    switch (stage)
    {
        case FEM_StageBegin:
            //  6)  Cuando la alarma esté desarmada se debe encender el led RGB
            //      verde fijo.
            Board_LED_Set   (LED_GREEN, false);
            UART_PutMessage (&appCtx->uartCtx, TEXT_PASSWORDTOARM);
            FEM_GotoStage   (ctx, FEM_StageMain);
            break;

        case FEM_StageMain:
            if (INDATA_Status(&appCtx->indataCtx) == INDATA_StatusPrompt)
            {
                break;
            }

            if (INDATA_Status(&appCtx->indataCtx) == INDATA_StatusReady)
            {
                UART_PutMessage (&appCtx->uartCtx, "agarro password y comparo...");
                UART_PutMessage (&appCtx->uartCtx, "NO ES IGUAL! vuelvo a Begin...");
                INDATA_End (&appCtx->indataCtx);
                FEM_GotoStage (ctx, FEM_StageBegin);
            }
            else if (INDATA_Status(&appCtx->indataCtx) == INDATA_StatusInvalid)
            {
                UART_PutMessage (&appCtx->uartCtx, TEXT_INVALIDPASSWORD);
                INDATA_End (&appCtx->indataCtx);
                FEM_GotoStage (ctx, FEM_StageBegin);
            }
            break;

        case FEM_StageEnd:

            break;
    }

    return FEM_StateReturnYield;
}


enum FEM_StateReturn FEMState_DiegoArmando (struct FEM_Context *ctx,
                                        enum FEM_Stage stage, uint32_t ticks)
{
    FEM_SetStateInfo (ctx, __FUNCTION__);

    return FEM_StateReturnYield;
}


enum FEM_StateReturn FEMState_Intruso (struct FEM_Context *ctx,
                                        enum FEM_Stage stage, uint32_t ticks)
{
    FEM_SetStateInfo (ctx, __FUNCTION__);

    return FEM_StateReturnYield;
}


/*
void blinkLedTask (void *ctx, uint32_t ticks)
{
    struct APP_Context *appCtx = (struct APP_Context *) ctx;
    Board_LED_Toggle (Led[appCtx->ledCurrentIndex]);
}
*/


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
            UART_PutMessage (uartCtx, TEXT_PASSWORDINPUT);
            INDATA_Begin (indaCtx, INDATA_TypeAlphanum);
            break;

        case 's':
            UART_PutStatusMessage (uartCtx);
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
    struct APP_Context *appCtx = (struct APP_Context *) ctx;

    if (INDATA_Status(&appCtx->indataCtx) == INDATA_StatusPrompt)
    {
        INDATA_Prompt (&appCtx->indataCtx);
    }
    else
    {
        processCommand (appCtx);
    }

    // uartProcessTask siempre debe consumir todos los datos disponibles
    UART_RecvConsumePending (&appCtx->uartCtx);
}


void debounceTecTask (void *ctx, uint32_t ticks)
{
    struct APP_Context *appCtx = (struct APP_Context *) ctx;

    for (uint32_t i = BOARD_TEC_1; i < BOARD_TEC_4; ++i)
    {
        appCtx->sensorStatus = BTN_DebouncePressed (i, ticks, 0,
                                    &appCtx->tecDebounce[i])?
                                    appCtx->sensorStatus | (1 << i) :
                                    appCtx->sensorStatus & ~(1 << i);
    }
}


void alarmFEMTask (void *ctx, uint32_t ticks)
{
    struct APP_Context *appCtx = (struct APP_Context *) ctx;
    FEM_Process (&appCtx->mainFemCtx, ticks, 1);
}


bool APP_Init (struct APP_Context *ctx)
{
    if (!ctx)
    {
        return false;
    }

    memset (ctx, 0, sizeof(struct APP_Context));

    UART_Init       (&ctx->uartCtx, DEBUG_UART, 9600);
    INDATA_Init     (&ctx->indataCtx, &ctx->uartCtx);
    FEM_Init        (&ctx->mainFemCtx, ctx);
    FEM_ChangeState (&ctx->mainFemCtx, FEMState_Desarmada);
    return true;
}


int main (void)
{
    Board_Init_Fixed ();

    struct APP_Context  appCtx;
    APP_Init (&appCtx);

    // Inserta comando para mostrar menu al principio del programa
    UART_RecvInjectByte (&appCtx.uartCtx, 'i');

    schedulerInit       ();
    schedulerAddTask    (uartRecvTask       ,&appCtx.uartCtx    ,0  ,14);
    schedulerAddTask    (uartSendTask       ,&appCtx.uartCtx    ,0  ,16);
    schedulerAddTask    (uartProcessTask    ,&appCtx            ,0  ,140);
    schedulerAddTask    (debounceTecTask    ,&appCtx            ,1  ,20);
    schedulerAddTask    (alarmFEMTask       ,&appCtx            ,0  ,50);
    schedulerStart      (1);

    while (1)
    {
        schedulerDispatchTasks (SYSTICK_Now ());
    }

    return 0;
}
