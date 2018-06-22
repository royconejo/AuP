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
#include "fsm.h"
#include "fsm_util.h"
#include "btn.h"
#include "copos.h"
#include "systick.h"
#include "text.h"
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


enum FSM_StateReturn FEMState_Desarmada (struct FEM *f,
                                        enum FSM_Stage stage, uint32_t ticks);
enum FSM_StateReturn FEMState_DiegoArmando (struct FEM *f,
                                       enum FSM_Stage stage, uint32_t ticks);
enum FSM_StateReturn FEMState_Armada (struct FEM *f,
                                      enum FSM_Stage stage, uint32_t ticks);
enum FSM_StateReturn FEMState_Intruso (struct FEM *f,
                                       enum FSM_Stage stage, uint32_t ticks);
enum FSM_StateReturn FEMState_Desarmando (struct FEM *f,
                                          enum FSM_Stage stage, uint32_t ticks);


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


struct APP
{
    struct UART         uart;
    struct INDATA       indata;
    struct FEM          mainFem;
    struct ARRAY        password;
    uint8_t             passwordData    [INDATA_BUFFER_SIZE];
    bool                passwordInRequest;
    bool                cancelRequest;
    uint32_t            disarmRetries;
    uint32_t            sensorStatus;
    uint32_t            ledStatus;
    uint32_t            tecDebounce     [4];
    struct VARIANT      vtmp;
};


bool APP_Init (struct APP *a)
{
    if (!a)
    {
        return false;
    }

    memset (a, 0, sizeof(struct APP));

    UART_Init       (&a->uart, DEBUG_UART, 9600);
    INDATA_Init     (&a->indata, &a->uart);
    ARRAY_Init      (&a->password, a->passwordData,
                     sizeof(a->passwordData));
    FSM_Init        (&a->mainFem, a);
    FSM_ChangeState (&a->mainFem, FEMState_Desarmada);

    // Password inicial
    ARRAY_AppendString (&a->password, "1234");
    return true;
}


void APP_ClearRequests (struct APP *a)
{
    a->passwordInRequest  = false;
    a->cancelRequest      = false;

    // Cancela cualquier pedido de input en progreso
    if (INDATA_Status(&a->indata) == INDATA_StatusPrompt)
    {
        UART_PutMessage (&a->uart, TEXT_NEWLINE);
    }
    INDATA_End (&a->indata);
}


bool APP_UpdateLed (struct APP *a, uint8_t led, bool on,
                    bool toggling)
{
    if (!a || led > 5)
    {
        return false;
    }

    led <<= 1;
    a->ledStatus = (a->ledStatus & ~(0b11 << led)) |
                        (on << led) | (toggling << (led + 1));
    return true;
}


enum APP_PasswordLogicAction APP_PasswordLogic (struct APP *a)
{
    struct UART   *uart   = (struct UART *)   &a->uart;
    struct INDATA *indata = (struct INDATA *) &a->indata;

    switch (INDATA_Status(indata))
    {
        // No entrando datos
        case INDATA_StatusDisabled:
        default:
            if (a->passwordInRequest)
            {
                // Se pidio ingresar password
                APP_ClearRequests   (a);
                UART_PutMessage     (uart, TEXT_PASSWORDINPUT);
                //  9)  Cuando se esté esperando que el usuario ingrese una
                //      contraseña se debe encender el led RGB azul.
                APP_UpdateLed       (a, LED_BLUE, true, false);
                INDATA_Begin        (indata, INDATA_TypeAlphanum);
                return APP_PasswordLogicAction_Asking;
            }
            return APP_PasswordLogicAction_None;

        // Aun se esta ingresando
        case INDATA_StatusPrompt:
            return APP_PasswordLogicAction_None;

        // Datos disponibles
        case INDATA_StatusReady:
            UART_PutMessage (uart, TEXT_CHECKINGPASSWORD);
            if (ARRAY_CheckEqualContents (&a->password, INDATA_Data(indata)))
            {
                UART_PutMessage (uart, TEXT_PASSWORDMATCH);
                INDATA_End      (indata);
                APP_UpdateLed   (a, LED_BLUE, false, false);
                return APP_PasswordLogicAction_Match;
            }
            UART_PutMessage (uart, TEXT_WRONGPASSWORD);
            break;

        // Datos invalidos
        case INDATA_StatusInvalid:
            UART_PutMessage (uart, TEXT_INVALIDPASSWORD);
            break;
    }

    INDATA_End      (indata);
    APP_UpdateLed   (a, LED_BLUE, false, false);
    return APP_PasswordLogicAction_AskAgain;
}


enum FSM_StateReturn FEMState_Desarmada (struct FEM *f,
                                        enum FSM_Stage stage, uint32_t ticks)
{
    FSM_SetStateInfo (f, __FUNCTION__);

    struct APP  *app  = (struct APP *)  f->app;
    struct UART *uart = (struct UART *) &app->uart;

    switch (stage)
    {
        case FSM_StageBegin:
            APP_ClearRequests   (app);
            //  6)  Cuando la alarma esté desarmada se debe encender el led RGB
            //      verde fijo.
            APP_UpdateLed       (app, LED_GREEN, true, false);
            UART_PutMessage     (uart, TEXT_PASSWORDTOARM);
            FSM_GotoStage       (f, FSM_StageMain);
            break;

        case FSM_StageMain:
        {
            const enum APP_PasswordLogicAction Pl = APP_PasswordLogic (app);
            if (Pl == APP_PasswordLogicAction_Asking)
            {
                APP_UpdateLed       (app, LED_GREEN, false, false);
            }
            else if (Pl == APP_PasswordLogicAction_AskAgain)
            {
                FSM_GotoStage       (f, FSM_StageBegin);
            }
            else if (Pl == APP_PasswordLogicAction_Match)
            {
                FSM_ChangeState     (f, FEMState_DiegoArmando);
            }
            break;
        }

        case FSM_StageEnd:
            break;
    }

    return FSM_StateReturnYield;
}


enum FSM_StateReturn FEMState_DiegoArmando (struct FEM *f,
                                       enum FSM_Stage stage, uint32_t ticks)
{
    FSM_SetStateInfo (f, __FUNCTION__);

    struct APP  *app  = (struct APP *)  f->app;
    struct UART *uart = (struct UART *) &app->uart;

    switch (stage)
    {
        case FSM_StageBegin:
            APP_ClearRequests   (app);
            VARIANT_SetUint32   (&app->vtmp, ARMING_PERIOD_MSEC / 1000);
            UART_PutMessageArgs (uart, TEXT_ALARMARMINGBEGIN,
                                 &app->vtmp, 1);
            APP_UpdateLed       (app, LED_GREEN, true, true);
            FSM_GotoStage       (f, FSM_StageMain);
            break;

        case FSM_StageMain:
            if (app->cancelRequest)
            {
                APP_ClearRequests   (app);
                UART_PutMessage     (uart, TEXT_ALARMARMINGCANCELLED);
                FSM_ChangeState     (f, FEMState_Desarmada);
            }
            else if (FSM_StageTimeout (f, ARMING_PERIOD_MSEC))
            {
                FSM_GotoStage (f, FSM_StageEnd);
            }
            break;

        case FSM_StageEnd:
            UART_PutMessage     (uart, TEXT_ALARMARMINGEND);
            APP_UpdateLed       (app, LED_GREEN, false, false);
            FSM_ChangeState     (f, FEMState_Armada);
            break;
    }

    return FSM_StateReturnYield;
}


enum FSM_StateReturn FEMState_Armada (struct FEM *f,
                                      enum FSM_Stage stage, uint32_t ticks)
{
    FSM_SetStateInfo (f, __FUNCTION__);

    struct APP  *app  = (struct APP *)  f->app;
    struct UART *uart = (struct UART *) &app->uart;

    switch (stage)
    {
        case FSM_StageBegin:
            APP_ClearRequests   (app);
            UART_PutMessage     (uart, TEXT_ARMEDPASSWORDTODISARM);
            //  7)  Cuando la alarma esté armada se debe encender el led RGB
            //      rojo fijo
            APP_UpdateLed       (app, LED_RED, true, false);
            FSM_GotoStage       (f, FSM_StageMain);
            break;

        case FSM_StageMain:
        {
            // Se activo un sensor?
            if (app->sensorStatus)
            {
                FSM_GotoStage   (f, FSM_StageEnd);
                return FSM_StateReturnAgain;
            }

            const enum APP_PasswordLogicAction Pl = APP_PasswordLogic (app);
            if (Pl == APP_PasswordLogicAction_Asking)
            {
                APP_UpdateLed   (app, LED_RED, false, false);
            }
            else if (Pl == APP_PasswordLogicAction_AskAgain)
            {
                FSM_GotoStage   (f, FSM_StageBegin);
            }
            else if (Pl == APP_PasswordLogicAction_Match)
            {
                FSM_ChangeState (f, FEMState_Desarmada);
            }
            break;
        }

        case FSM_StageEnd:
            // Discrimina cual sensor se activo
            if (app->sensorStatus & ~SENSOR_DOOR)
            {
               UART_PutMessage  (uart, TEXT_SENSORWINDOW);
               FSM_ChangeState  (f, FEMState_Intruso);
            }
            else if (app->sensorStatus & SENSOR_DOOR)
            {
               UART_PutMessage  (uart, TEXT_SENSORDOOR);
               FSM_ChangeState  (f, FEMState_Desarmando);
            }

            APP_UpdateLed (app, LED_RED, false, false);
            break;
    }

    return FSM_StateReturnYield;
}


enum FSM_StateReturn FEMState_Intruso (struct FEM *f,
                                        enum FSM_Stage stage, uint32_t ticks)
{
    FSM_SetStateInfo (f, __FUNCTION__);

    struct APP  *app  = (struct APP *)  f->app;
    struct UART *uart = (struct UART *) &app->uart;

    switch (stage)
    {
        case FSM_StageBegin:
            APP_ClearRequests   (app);
            UART_PutMessage     (uart, TEXT_INTRUDERPASSWORDTODISARM);
            //  8)  Cuando la alarma esté disparada (intruso) se debe parpadear
            //      el led RGB rojo.
            APP_UpdateLed       (app, LED_RED, true, true);
            FSM_GotoStage       (f, FSM_StageMain);
            break;

        case FSM_StageMain:
        {
            const enum APP_PasswordLogicAction Pl = APP_PasswordLogic (app);
            if (Pl == APP_PasswordLogicAction_Asking)
            {
                APP_UpdateLed   (app, LED_RED, false, false);
            }
            else if (Pl == APP_PasswordLogicAction_AskAgain)
            {
                FSM_GotoStage   (f, FSM_StageBegin);
            }
            else if (Pl == APP_PasswordLogicAction_Match)
            {
                FSM_ChangeState (f, FEMState_Desarmada);
            }
            break;
        }

        case FSM_StageEnd:
            break;
    }

    return FSM_StateReturnYield;
}


enum FSM_StateReturn FEMState_Desarmando (struct FEM *f, enum FSM_Stage stage,
                                          uint32_t ticks)
{
    FSM_SetStateInfo (f, __FUNCTION__);

    struct APP  *app  = (struct APP *)  f->app;
    struct UART *uart = (struct UART *) &app->uart;

    switch (stage)
    {
        case FSM_StageBegin:
            app->disarmRetries = 0;
            APP_ClearRequests   (app);
            FSM_StateCountdown  (f, DOOR_DISARMING_MSEC);
            VARIANT_SetUint32   (&app->vtmp, FSM_StateCountdownSeconds(f));
            UART_PutMessageArgs (uart, TEXT_DOORPASSWORDTODISARM,
                                 &app->vtmp, 1);
            APP_UpdateLed       (app, LED_RED, true, false);
            FSM_GotoStage       (f, FSM_StageMain);
            return FSM_StateReturnAgain;

        case FSM_StageMain:
            if (FSM_StateCountdown (f, 0))
            {
                FSM_ChangeState (f, FEMState_Intruso);
                return FSM_StateReturnYield;
            }

            const enum APP_PasswordLogicAction Pl = APP_PasswordLogic (app);
            if (Pl == APP_PasswordLogicAction_Asking)
            {
                APP_UpdateLed       (app, LED_RED, false, false);
            }
            else if (Pl == APP_PasswordLogicAction_AskAgain)
            {
                if (++ app->disarmRetries >= DOOR_DISARM_MAX_RETRIES)
                {
                    FSM_ChangeState (f, FEMState_Intruso);
                    return FSM_StateReturnYield;
                }
                // Reintenta
                APP_UpdateLed       (app, LED_RED, true, false);
                VARIANT_SetUint32   (&app->vtmp, DOOR_DISARM_MAX_RETRIES -
                                     app->disarmRetries);
                UART_PutMessageArgs (uart, TEXT_DOORPASSWORDTODISARMAGAIN,
                                     &app->vtmp, 1);
                VARIANT_SetUint32   (&app->vtmp,
                                     FSM_StateCountdownSeconds(f));
                UART_PutMessageArgs (uart, TEXT_DOORPASSWORDTODISARM,
                                     &app->vtmp, 1);
            }
            else if (Pl == APP_PasswordLogicAction_Match)
            {
                FSM_ChangeState     (f, FEMState_DiegoArmando);
            }
            break;

        case FSM_StageEnd:
            break;
    }

    return FSM_StateReturnYield;
}


void uartRecvTask (void *ctx, uint32_t ticks)
{
    struct UART *uart = (struct UART *) ctx;
    UART_Recv (uart);
}


void uartSendTask (void *ctx, uint32_t ticks)
{
    struct UART *uart = (struct UART *) ctx;
    UART_Send (uart);
}


static void processCommand (struct APP *a)
{
    struct UART *uart = (struct UART *) &a->uart;

    const uint32_t Pending = UART_RecvPendingCount (uart);
    if (!Pending)
    {
        // Nada que procesar
        return;
    }

    if (Pending != 1)
    {
        UART_PutMessage (uart, TEXT_WRONGCOMMANDSIZE);
        return;
    }

    const uint8_t Command = UART_RecvPeek (uart, 0);
    switch (Command)
    {
        case 'i':
            UART_PutMessage (uart, TEXT_WELCOME);
            break;

        case 'p':
            a->passwordInRequest = true;
            break;

        case 'x':
            a->cancelRequest = true;
            break;

        case 's':
            UART_PutStatusMessage (uart);
            break;

        case 'm':
            FSM_PutStatusMessage (&a->mainFem, uart);
            break;

        case 'c':
            UART_PutMessage (uart, TERM_CLEAR_SCREEN);
            break;

        default:
            UART_PutMessage (uart, TEXT_WRONGCOMMAND);
            break;
    }
}


void uartProcessTask (void *ctx, uint32_t ticks)
{
    struct APP *app = (struct APP *) ctx;

    if (INDATA_Status(&app->indata) == INDATA_StatusPrompt)
    {
        INDATA_Prompt (&app->indata);
    }
    else
    {
        processCommand (app);
    }

    // uartProcessTask siempre debe consumir todos los datos disponibles
    UART_RecvDiscardPending (&app->uart);
}


void ledUpdateTask (void *ctx, uint32_t ticks)
{
    struct APP *app = (struct APP *) ctx;

    for (uint32_t i = 0; i <= 5; ++i)
    {
        const bool On     = app->ledStatus & (1 << (i << 1));
        const bool Toggle = app->ledStatus & (1 << ((i << 1) + 1));

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
    struct APP *app = (struct APP *) ctx;

    for (uint32_t i = BOARD_TEC_1; i <= BOARD_TEC_4; ++i)
    {
        if (BTN_DebouncePressed (i, ticks, 0, &app->tecDebounce[i]))
        {
            // Togglea sensor correspondiente a la tecla presionada
            // NOTA: efecto de toggle generado para facilitar demostracion
            if (app->sensorStatus & (1 << i))
            {
                app->sensorStatus &= ~(1 << i);
            }
            else
            {
                app->sensorStatus |= (1 << i);
            }
        }
    }
}


void sensorOutputsTask (void *ctx, uint32_t ticks)
{
    struct APP *app = (struct APP *) ctx;

    const uint32_t Sensors = app->sensorStatus;
    APP_UpdateLed (app, LED_1, Sensors & SENSOR_WINDOW_1, false);
    APP_UpdateLed (app, LED_2, Sensors & SENSOR_WINDOW_2, false);
    APP_UpdateLed (app, LED_3, Sensors & SENSOR_WINDOW_3, false);
}


void alarmFEMTask (void *ctx, uint32_t ticks)
{
    struct APP *app = (struct APP *) ctx;
    FSM_Process (&app->mainFem, ticks, 1);
}


int main (void)
{
    Board_Init_Fixed ();

    struct APP app;
    APP_Init (&app);

    // Inserta comando para mostrar menu al principio del programa
    UART_RecvInjectByte (&app.uart, 'i');

    schedulerInit       ();
    schedulerAddTask    (uartRecvTask       ,&app.uart  ,0  ,14);
    schedulerAddTask    (uartSendTask       ,&app.uart  ,0  ,16);
    schedulerAddTask    (uartProcessTask    ,&app       ,0  ,140);
    schedulerAddTask    (ledUpdateTask      ,&app       ,1  ,500);
    schedulerAddTask    (debounceTecTask    ,&app       ,1  ,20);
    schedulerAddTask    (sensorOutputsTask  ,&app       ,1  ,20);
    schedulerAddTask    (alarmFEMTask       ,&app       ,0  ,50);
    schedulerStart      (1);

    while (1)
    {
        schedulerDispatchTasks (SYSTICK_Now ());
    }

    return 0;
}
