/*
    CESE Actividad 5: ALARMA
    Copyright 2018 Santiago Germino (royconejo@gmail.com)

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    1.  Redistributions of source code must retain the above copyright notice,
        this list of conditions and the following disclaimer.

    2.  Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

    3.  Neither the name of the copyright holder nor the names of its
        contributors may be used to endorse or promote products derived from
        this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.
*/
#include "board_fixed.h"
#include "uart.h"
#include "uart_util.h"
#include "term.h"
#include "array.h"
#include "indata.h"
#include "fem.h"
#include "fem_util.h"
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


enum FEM_StateReturn FEMState_Desarmada (struct FEM_Context *ctx,
                                        enum FEM_Stage stage, uint32_t ticks);
enum FEM_StateReturn FEMState_DiegoArmando (struct FEM_Context *ctx,
                                       enum FEM_Stage stage, uint32_t ticks);
enum FEM_StateReturn FEMState_Armada (struct FEM_Context *ctx,
                                      enum FEM_Stage stage, uint32_t ticks);
enum FEM_StateReturn FEMState_Intruso (struct FEM_Context *ctx,
                                       enum FEM_Stage stage, uint32_t ticks);
enum FEM_StateReturn FEMState_Desarmando (struct FEM_Context *ctx,
                                          enum FEM_Stage stage, uint32_t ticks);


#define     ARMING_PERIOD_MSEC      6000
#define     DOOR_DISARMING_MSEC     6000
#define     DOOR_DISARM_MAX_RETRIES 3

#define     SENSOR_DOOR             0b0001
#define     SENSOR_WINDOW_1         0b0010
#define     SENSOR_WINDOW_2         0b0100
#define     SENSOR_WINDOW_3         0b1000


enum APP_PasswordLogicAction
{
    APP_PasswordLogicAction_None,
    APP_PasswordLogicAction_Asking,
    APP_PasswordLogicAction_AskAgain,
    APP_PasswordLogicAction_Match,
};


struct APP_Context
{
    struct UART_Context     uartCtx;
    struct INDATA_Context   indataCtx;
    struct FEM_Context      mainFemCtx;
    struct ARRAY            password;
    uint8_t                 passwordData    [INDATA_BUFFER_SIZE];
    bool                    passwordInRequest;
    bool                    cancelRequest;
    uint32_t                disarmRetries;
    uint32_t                sensorStatus;
    uint32_t                ledStatus;
    uint32_t                tecDebounce     [4];
    struct VARIANT          vtmp;
};


bool APP_Init (struct APP_Context *ctx)
{
    if (!ctx)
    {
        return false;
    }

    memset (ctx, 0, sizeof(struct APP_Context));

    UART_Init       (&ctx->uartCtx, DEBUG_UART, 9600);
    INDATA_Init     (&ctx->indataCtx, &ctx->uartCtx);
    ARRAY_Init      (&ctx->password, ctx->passwordData,
                     sizeof(ctx->passwordData));
    FEM_Init        (&ctx->mainFemCtx, ctx);
    FEM_ChangeState (&ctx->mainFemCtx, FEMState_Desarmada);

    // Password inicial
    ARRAY_AppendString (&ctx->password, "1234");
    return true;
}


void APP_ClearRequests (struct APP_Context *ctx)
{
    ctx->passwordInRequest  = false;
    ctx->cancelRequest      = false;
}


bool APP_UpdateLed (struct APP_Context *ctx, uint8_t led, bool on,
                    bool toggling)
{
    if (!ctx || led > 5)
    {
        return false;
    }

    led <<= 1;
    ctx->ledStatus = (ctx->ledStatus & ~(0b11 << led)) |
                        (on << led) | (toggling << (led + 1));
    return true;
}


enum APP_PasswordLogicAction
APP_PasswordLogic (struct APP_Context *ctx)
{
    struct UART_Context *uartCtx = (struct UART_Context *)   &ctx->uartCtx;
    struct INDATA_Context *inCtx = (struct INDATA_Context *) &ctx->indataCtx;

    switch (INDATA_Status(inCtx))
    {
        // No entrando datos
        case INDATA_StatusDisabled:
        default:
            if (ctx->passwordInRequest)
            {
                // Se pidio ingresar password
                APP_ClearRequests   (ctx);
                UART_PutMessage     (uartCtx, TEXT_PASSWORDINPUT);
                //  9)  Cuando se esté esperando que el usuario ingrese una
                //      contraseña se debe encender el led RGB azul.
                APP_UpdateLed       (ctx, LED_BLUE, true, false);
                INDATA_Begin        (inCtx, INDATA_TypeAlphanum);
                return APP_PasswordLogicAction_Asking;
            }
            return APP_PasswordLogicAction_None;

        // Aun se esta ingresando
        case INDATA_StatusPrompt:
            return APP_PasswordLogicAction_None;

        // Datos disponibles
        case INDATA_StatusReady:
            UART_PutMessage (uartCtx, TEXT_CHECKINGPASSWORD);
            if (ARRAY_CheckEqualContents (&ctx->password, INDATA_Data(inCtx)))
            {
                UART_PutMessage (uartCtx, TEXT_PASSWORDMATCH);
                INDATA_End      (inCtx);
                APP_UpdateLed   (ctx, LED_BLUE, false, false);
                return APP_PasswordLogicAction_Match;
            }
            UART_PutMessage (uartCtx, TEXT_WRONGPASSWORD);
            break;

        // Datos invalidos
        case INDATA_StatusInvalid:
            UART_PutMessage (uartCtx, TEXT_INVALIDPASSWORD);
            break;
    }

    INDATA_End      (inCtx);
    APP_UpdateLed   (ctx, LED_BLUE, false, false);
    return APP_PasswordLogicAction_AskAgain;
}


enum FEM_StateReturn FEMState_Desarmada (struct FEM_Context *ctx,
                                        enum FEM_Stage stage, uint32_t ticks)
{
    FEM_SetStateInfo (ctx, __FUNCTION__);

    struct APP_Context   *appCtx = (struct APP_Context *)    ctx->appCtx;
    struct UART_Context *uartCtx = (struct UART_Context *)   &appCtx->uartCtx;
    struct INDATA_Context *inCtx = (struct INDATA_Context *) &appCtx->indataCtx;

    switch (stage)
    {
        case FEM_StageBegin:
            APP_ClearRequests   (appCtx);
            //  6)  Cuando la alarma esté desarmada se debe encender el led RGB
            //      verde fijo.
            APP_UpdateLed       (appCtx, LED_GREEN, true, false);
            UART_PutMessage     (uartCtx, TEXT_PASSWORDTOARM);
            FEM_GotoStage       (ctx, FEM_StageMain);
            break;

        case FEM_StageMain:
        {
            const enum APP_PasswordLogicAction Pl = APP_PasswordLogic (appCtx);
            if (Pl == APP_PasswordLogicAction_Asking)
            {
                APP_UpdateLed       (appCtx, LED_GREEN, false, false);
            }
            else if (Pl == APP_PasswordLogicAction_AskAgain)
            {
                FEM_GotoStage       (ctx, FEM_StageBegin);
            }
            else if (Pl == APP_PasswordLogicAction_Match)
            {
                FEM_ChangeState     (ctx, FEMState_DiegoArmando);
            }
            break;
        }

        case FEM_StageEnd:
            break;
    }

    return FEM_StateReturnYield;
}


enum FEM_StateReturn FEMState_DiegoArmando (struct FEM_Context *ctx,
                                       enum FEM_Stage stage, uint32_t ticks)
{
    FEM_SetStateInfo (ctx, __FUNCTION__);

    struct APP_Context   *appCtx = (struct APP_Context *)    ctx->appCtx;
    struct UART_Context *uartCtx = (struct UART_Context *)   &appCtx->uartCtx;

    switch (stage)
    {
        case FEM_StageBegin:
            APP_ClearRequests   (appCtx);
            VARIANT_SetUint32   (&appCtx->vtmp, ARMING_PERIOD_MSEC / 1000);
            UART_PutMessageArgs (uartCtx, TEXT_ALARMARMINGBEGIN,
                                 &appCtx->vtmp, 1);
            APP_UpdateLed       (appCtx, LED_GREEN, true, true);
            FEM_GotoStage       (ctx, FEM_StageMain);
            break;

        case FEM_StageMain:
            if (appCtx->cancelRequest)
            {
                APP_ClearRequests   (appCtx);
                UART_PutMessage     (uartCtx, TEXT_ALARMARMINGCANCELLED);
                FEM_ChangeState     (ctx, FEMState_Desarmada);
            }
            else if (FEM_StageTimeout (ctx, ARMING_PERIOD_MSEC))
            {
                FEM_GotoStage (ctx, FEM_StageEnd);
            }
            break;

        case FEM_StageEnd:
            UART_PutMessage     (uartCtx, TEXT_ALARMARMINGEND);
            APP_UpdateLed       (appCtx, LED_GREEN, false, false);
            FEM_ChangeState     (ctx, FEMState_Armada);
            break;
    }

    return FEM_StateReturnYield;
}


enum FEM_StateReturn FEMState_Armada (struct FEM_Context *ctx,
                                      enum FEM_Stage stage, uint32_t ticks)
{
    FEM_SetStateInfo (ctx, __FUNCTION__);

    struct APP_Context   *appCtx = (struct APP_Context *)    ctx->appCtx;
    struct UART_Context *uartCtx = (struct UART_Context *)   &appCtx->uartCtx;

    switch (stage)
    {
        case FEM_StageBegin:
            APP_ClearRequests   (appCtx);
            UART_PutMessage     (uartCtx, TEXT_ARMEDPASSWORDTODISARM);
            //  7)  Cuando la alarma esté armada se debe encender el led RGB
            //      rojo fijo
            APP_UpdateLed       (appCtx, LED_RED, true, false);
            FEM_GotoStage       (ctx, FEM_StageMain);
            break;

        case FEM_StageMain:
        {
            // Se activo un sensor?
            if (appCtx->sensorStatus)
            {
                FEM_GotoStage   (ctx, FEM_StageEnd);
                return FEM_StateReturnAgain;
            }

            const enum APP_PasswordLogicAction Pl = APP_PasswordLogic (appCtx);
            if (Pl == APP_PasswordLogicAction_Asking)
            {
                APP_UpdateLed   (appCtx, LED_RED, false, false);
            }
            else if (Pl == APP_PasswordLogicAction_AskAgain)
            {
                FEM_GotoStage   (ctx, FEM_StageBegin);
            }
            else if (Pl == APP_PasswordLogicAction_Match)
            {
                FEM_ChangeState (ctx, FEMState_Desarmada);
            }
            break;
        }

        case FEM_StageEnd:
            // Discrimina cual sensor se activo
            if (appCtx->sensorStatus & ~SENSOR_DOOR)
            {
               UART_PutMessage  (uartCtx, TEXT_SENSORWINDOW);
               FEM_ChangeState  (ctx, FEMState_Intruso);
            }
            else if (appCtx->sensorStatus & SENSOR_DOOR)
            {
               UART_PutMessage  (uartCtx, TEXT_SENSORDOOR);
               FEM_ChangeState  (ctx, FEMState_Desarmando);
            }

            APP_UpdateLed (appCtx, LED_RED, false, false);
            break;
    }

    return FEM_StateReturnYield;
}


enum FEM_StateReturn FEMState_Intruso (struct FEM_Context *ctx,
                                        enum FEM_Stage stage, uint32_t ticks)
{
    FEM_SetStateInfo (ctx, __FUNCTION__);

    struct APP_Context   *appCtx = (struct APP_Context *)    ctx->appCtx;
    struct UART_Context *uartCtx = (struct UART_Context *)   &appCtx->uartCtx;

    switch (stage)
    {
        case FEM_StageBegin:
            APP_ClearRequests   (appCtx);
            UART_PutMessage     (uartCtx, TEXT_INTRUDERPASSWORDTODISARM);
            //  8)  Cuando la alarma esté disparada (intruso) se debe parpadear
            //      el led RGB rojo.
            APP_UpdateLed       (appCtx, LED_RED, true, true);
            FEM_GotoStage       (ctx, FEM_StageMain);
            break;

        case FEM_StageMain:
        {
            const enum APP_PasswordLogicAction Pl = APP_PasswordLogic (appCtx);
            if (Pl == APP_PasswordLogicAction_Asking)
            {
                APP_UpdateLed   (appCtx, LED_RED, false, false);
            }
            else if (Pl == APP_PasswordLogicAction_AskAgain)
            {
                FEM_GotoStage   (ctx, FEM_StageBegin);
            }
            else if (Pl == APP_PasswordLogicAction_Match)
            {
                FEM_ChangeState (ctx, FEMState_Desarmada);
            }
            break;
        }

        case FEM_StageEnd:
            break;
    }

    return FEM_StateReturnYield;
}


enum FEM_StateReturn FEMState_Desarmando (struct FEM_Context *ctx,
                                          enum FEM_Stage stage, uint32_t ticks)
{
    FEM_SetStateInfo (ctx, __FUNCTION__);

    struct APP_Context   *appCtx = (struct APP_Context *)    ctx->appCtx;
    struct UART_Context *uartCtx = (struct UART_Context *)   &appCtx->uartCtx;

    switch (stage)
    {
        case FEM_StageBegin:
            appCtx->disarmRetries = 0;
            FEM_StateCountdown  (ctx, DOOR_DISARMING_MSEC);
            APP_ClearRequests   (appCtx);
            VARIANT_SetUint32   (&appCtx->vtmp, FEM_StateCountdownSeconds(ctx));
            UART_PutMessageArgs (uartCtx, TEXT_DOORPASSWORDTODISARM,
                                 &appCtx->vtmp, 1);
            APP_UpdateLed       (appCtx, LED_RED, true, false);
            FEM_GotoStage       (ctx, FEM_StageMain);
            return FEM_StateReturnAgain;

        case FEM_StageMain:
            if (FEM_StateCountdown (ctx, 0))
            {
                FEM_ChangeState (ctx, FEMState_Intruso);
                return FEM_StateReturnYield;
            }

            const enum APP_PasswordLogicAction Pl = APP_PasswordLogic (appCtx);
            if (Pl == APP_PasswordLogicAction_Asking)
            {
                APP_UpdateLed       (appCtx, LED_RED, false, false);
            }
            else if (Pl == APP_PasswordLogicAction_AskAgain)
            {
                if (++ appCtx->disarmRetries >= DOOR_DISARM_MAX_RETRIES)
                {
                    FEM_ChangeState (ctx, FEMState_Intruso);
                    return FEM_StateReturnYield;
                }
                // Reintenta
                APP_UpdateLed       (appCtx, LED_RED, true, false);
                VARIANT_SetUint32   (&appCtx->vtmp, DOOR_DISARM_MAX_RETRIES -
                                     appCtx->disarmRetries);
                UART_PutMessageArgs (uartCtx, TEXT_DOORPASSWORDTODISARMAGAIN,
                                     &appCtx->vtmp, 1);
                VARIANT_SetUint32   (&appCtx->vtmp,
                                     FEM_StateCountdownSeconds(ctx));
                UART_PutMessageArgs (uartCtx, TEXT_DOORPASSWORDTODISARM,
                                     &appCtx->vtmp, 1);
            }
            else if (Pl == APP_PasswordLogicAction_Match)
            {
                FEM_ChangeState     (ctx, FEMState_DiegoArmando);
            }
            break;

        case FEM_StageEnd:
            break;
    }

    return FEM_StateReturnYield;
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
    struct UART_Context *uartCtx = (struct UART_Context *) &ctx->uartCtx;

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
            ctx->passwordInRequest = true;
            break;

        case 'x':
            ctx->cancelRequest = true;
            break;

        case 's':
            UART_PutStatusMessage (uartCtx);
            break;

        case 'm':
            FEM_PutStatusMessage (&ctx->mainFemCtx, uartCtx);
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


void ledUpdateTask (void *ctx, uint32_t ticks)
{
    struct APP_Context *appCtx = (struct APP_Context *) ctx;

    for (uint32_t i = 0; i <= 5; ++i)
    {
        const bool On     = appCtx->ledStatus & (1 << (i << 1));
        const bool Toggle = appCtx->ledStatus & (1 << ((i << 1) + 1));

        if (On && Toggle)
        {
            Board_LED_Toggle (i);
        }
        else {
            Board_LED_Set (i, !On);
        }
    }
}


void debounceTecTask (void *ctx, uint32_t ticks)
{
    struct APP_Context *appCtx = (struct APP_Context *) ctx;

    for (uint32_t i = BOARD_TEC_1; i <= BOARD_TEC_4; ++i)
    {
        if (BTN_DebouncePressed (i, ticks, 0, &appCtx->tecDebounce[i]))
        {
            // Togglea sensor correspondiente a la tecla presionada
            // NOTA: efecto de toggle generado para facilitar demostracion
            if (appCtx->sensorStatus & (1 << i))
            {
                appCtx->sensorStatus &= ~(1 << i);
            }
            else
            {
                appCtx->sensorStatus |= (1 << i);
            }
        }
    }
}


void sensorOutputsTask (void *ctx, uint32_t ticks)
{
    struct APP_Context *appCtx = (struct APP_Context *) ctx;

    const uint32_t Sensors = appCtx->sensorStatus;
    APP_UpdateLed (appCtx, LED_1, Sensors & SENSOR_WINDOW_1, false);
    APP_UpdateLed (appCtx, LED_2, Sensors & SENSOR_WINDOW_2, false);
    APP_UpdateLed (appCtx, LED_3, Sensors & SENSOR_WINDOW_3, false);
}


void alarmFEMTask (void *ctx, uint32_t ticks)
{
    struct APP_Context *appCtx = (struct APP_Context *) ctx;
    FEM_Process (&appCtx->mainFemCtx, ticks, 1);
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
    schedulerAddTask    (ledUpdateTask      ,&appCtx            ,1  ,500);
    schedulerAddTask    (debounceTecTask    ,&appCtx            ,1  ,20);
    schedulerAddTask    (sensorOutputsTask  ,&appCtx            ,1  ,20);
    schedulerAddTask    (alarmFEMTask       ,&appCtx            ,0  ,50);
    schedulerStart      (1);

    while (1)
    {
        schedulerDispatchTasks (SYSTICK_Now ());
    }

    return 0;
}
