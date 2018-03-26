#include "board.h"

#define PWM_PERIOD_RES  256
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


#define transitionRepeatForever     0xFFFFFFFF

enum transitionStatus_t
{
    transitionStatus_END = 0,
    transitionStatus_GOING,
};


struct transitionData_t
{
    uint32_t        startIntensity;   // FIXME: no se usa
    uint32_t        repeat;
    void const *    s;
};

struct animationData_t;

struct transition_t
{
    struct transitionData_t data;
    void (*init) (struct animationData_t *, const struct transitionData_t *);
    enum transitionStatus_t (*update) (struct animationData_t *,
                                       const struct transitionData_t *);
};


struct IN_OUT_transitionData_t
{
    uint32_t    minIntensity;
    uint32_t    maxIntensity;
    uint32_t    intensityStep;
};


struct HOLD_transitionData_t
{
    uint32_t    holdStep;
};


enum animationStatus_t
{
    animationStatus_END = 0,
    animationStatus_ERROR,
    animationStatus_TRANSITION_INIT,
    animationStatus_TRANSITION_UPDATE
};


struct animationData_t
{
    uint32_t    curIntensity;
    uint32_t    curTransitionRepeat;
    uint32_t    speedMultiplier;
    uint32_t    aux;
};


struct animation_t
{
    enum animationStatus_t  status;
    struct animationData_t  data;
    uint32_t                curAnimationRepeat;
    uint32_t                curTransition;
    struct transition_t     const *transitions[];
};


void HOLD_transitionInit (struct animationData_t *ad,
                          const struct transitionData_t *td)
{
    struct HOLD_transitionData_t *ts =
            (struct HOLD_transitionData_t *) td->s;

    ad->aux = ts->holdStep * ad->speedMultiplier >> 16;
}


enum transitionStatus_t IN_transitionUpdate (
        struct animationData_t *ad,
        const struct transitionData_t *td)
{
    struct IN_OUT_transitionData_t *ts =
            (struct IN_OUT_transitionData_t *) td->s;

    const uint32_t Step = ts->intensityStep * ad->speedMultiplier >> 16;
    ad->curIntensity += Step;
    if (ad->curIntensity > ts->maxIntensity)
    {
        ad->curIntensity = ad->curTransitionRepeat? ts->minIntensity :
                                                    ts->maxIntensity;
        return transitionStatus_END;
    }
    return transitionStatus_GOING;
}


enum transitionStatus_t HOLD_transitionUpdate (
        struct animationData_t *ad,
        const struct transitionData_t *td)
{
    if (!ad->aux)
    {
        return transitionStatus_END;
    }

    -- ad->aux;
    return transitionStatus_GOING;
}


enum transitionStatus_t OUT_transitionUpdate (
        struct animationData_t *ad,
        const struct transitionData_t *td)
{
    struct IN_OUT_transitionData_t *ts =
            (struct IN_OUT_transitionData_t *) td->s;

    const uint32_t Step = ts->intensityStep * ad->speedMultiplier >> 16;
    ad->curIntensity -= Step;
    if (ad->curIntensity < Step ||
        ad->curIntensity < ts->minIntensity)
    {
        ad->curIntensity = ad->curTransitionRepeat? ts->maxIntensity :
                                                    ts->minIntensity;
        return transitionStatus_END;
    }
    return transitionStatus_GOING;
}


#define U8_TO_FIXED_16_16(a) (((a) & 0xFF) << 16)
#define DOUBLE_TO_FIXED_16_16(a) ((a) * (1 << 16))


void animationInit (struct animation_t *a, uint32_t repeat)
{
    a->status                   = animationStatus_TRANSITION_INIT;
    a->curAnimationRepeat       = repeat;
    a->curTransition            = 0;
    a->data.curIntensity        = 0;
    a->data.curTransitionRepeat = 0;
    // iniciar setMultiplier sacar animationInit de update
}


void animationSetMultiplier (struct animation_t *a, double multi)
{
    a->data.speedMultiplier = DOUBLE_TO_FIXED_16_16 (multi);
}


bool animationUpdate (struct animation_t *a)
{
    if (!a)
    {
        return false;
    }

    if (a->status == animationStatus_END)
    {
        return false;
    }

    const struct transition_t *t = a->transitions[a->curTransition];
    if (!t)
    {
        if (!a->curAnimationRepeat)
        {
            a->status = animationStatus_END;
            return false;
        }

        if (a->curAnimationRepeat != transitionRepeatForever)
        {
            -- a->curAnimationRepeat;
        }

        animationInit (a, a->curAnimationRepeat);

        t = a->transitions[a->curTransition];
        if (!t)
        {
            a->status = animationStatus_ERROR;
            return false;
        }
    }

    if (a->status == animationStatus_TRANSITION_INIT)
    {
        a->data.curTransitionRepeat = t->data.repeat;
        if (t->init)
        {
            t->init (&a->data, &t->data);
        }
        a->status = animationStatus_TRANSITION_UPDATE;
    }

    if (t->update (&a->data, &t->data) == transitionStatus_END)
    {
        if (!a->data.curTransitionRepeat)
        {
            ++ a->curTransition;
            a->status = animationStatus_TRANSITION_INIT;
        }
        else
        {
            if (a->data.curTransitionRepeat != transitionRepeatForever)
            {
                -- a->data.curTransitionRepeat;
            }
            a->status = animationStatus_TRANSITION_UPDATE;
        }
    }

    return true;
}


void PWM_update (uint8_t port, uint8_t intensity)
{
    Board_LED_Set (port, intensity <= g_pwmPeriod);
}



void PWM_updateWithAnimationData (uint8_t port, struct animationData_t *ad)
{
    PWM_update (port, ad->curIntensity >> 16);
}


#define DECLARE_IN_OUT_TRANSITION_DATA(n, minIntensity, maxIntensity, \
    transitionMsecs) \
    const struct IN_OUT_transitionData_t n = { \
        U8_TO_FIXED_16_16(minIntensity), \
        U8_TO_FIXED_16_16(maxIntensity), \
        DOUBLE_TO_FIXED_16_16((maxIntensity - minIntensity) / (PWM_PERIOD_HZ \
            * transitionMsecs * 0.001)) \
    };

#define DECLARE_HOLD_TRANSITION_DATA(n, holdMsecs) \
    const struct HOLD_transitionData_t n = { \
        (((double)holdMsecs * 0.001) * PWM_PERIOD_HZ) \
    };

#define DECLARE_TRANSITION(n, startIntensity, repeat,dataPtr, initProc, \
    updateProc) \
    const struct transition_t n = { \
        { U8_TO_FIXED_16_16(startIntensity), repeat, dataPtr }, initProc, \
            updateProc, \
    };

#define DECLARE_ANIMATION(n, ...) \
    struct animation_t n = { animationStatus_TRANSITION_INIT, { 0, 0 }, \
            0, 0, { __VA_ARGS__ } \
    };


DECLARE_IN_OUT_TRANSITION_DATA(
        IN_OUT_Transition_Data_1, 0, 255, 2000)

DECLARE_HOLD_TRANSITION_DATA(
        HOLD_Transition_Data_1, 1000)

DECLARE_HOLD_TRANSITION_DATA(
        HOLD_Transition_Data_2, 5000)

DECLARE_TRANSITION(
        IN_Transition_1,
        0,
        0,
        &IN_OUT_Transition_Data_1,
        NULL,
        IN_transitionUpdate)

DECLARE_TRANSITION(
        HOLD_Transition_1,
        0,
        0,
        &HOLD_Transition_Data_1,
        HOLD_transitionInit,
        HOLD_transitionUpdate)

DECLARE_TRANSITION(
        OUT_Transition_1,
        0,
        0,
        &IN_OUT_Transition_Data_1,
        NULL,
        OUT_transitionUpdate)

DECLARE_TRANSITION(
        HOLD_Transition_2,
        0,
        0,
        &HOLD_Transition_Data_2,
        HOLD_transitionInit,
        HOLD_transitionUpdate)

DECLARE_ANIMATION(
        ANIMATION_1,
        &IN_Transition_1,
        &HOLD_Transition_1,
        &OUT_Transition_1,
        &HOLD_Transition_2,
        NULL)


int main (void)
{
    SystemCoreClockUpdate    ();
    Board_Init               ();
    SysTick_Config           (SystemCoreClock / PWM_TICKS_HZ);

    animationInit (&ANIMATION_1, transitionRepeatForever);
    animationSetMultiplier (&ANIMATION_1, 20);

    while (1)
    {
        PWM_updateWithAnimationData (LED_RED, &ANIMATION_1.data);

        if (g_newPeriod)
        {
            g_newPeriod = false;
            animationUpdate (&ANIMATION_1);
        }

        if (g_newSecond)
        {
            g_newSecond = false;
            Board_LED_Toggle (LED_3);
        }

        __WFI();
    }

#if 0
   // Inicializar y configurar la plataforma
   boardConfig();

   // Inicializar UART_USB como salida Serial de debug
   debugPrintConfigUart( UART_USB, 115200 );
   debugPrintlnString( "DEBUG: UART_USB configurada." );

   // Inicializar UART_232 como salida Serial de consola
   consolePrintConfigUart( UART_232, 115200 );
   consolePrintlnString( "UART_232 configurada." );

   // Crear varias variables del tipo booleano
   bool_t tec1Pressed = OFF;
   bool_t tec2Pressed = OFF;

   // Indica que led esta seleccionado
   gpioMap_t ledActualSel = LEDR;

   // Estado del Led seleccionado
   bool_t ledActualOn = OFF;

   // Tiempo actual aproximado
   tick_t tiempoActual;

   // ---------- REPETIR POR SIEMPRE --------------------------
   while( TRUE )
   {
      tiempo = tickRead ();

      tec = gpioRead (TEC1);
      if (tec1Pressed && )


      /* Si se presiona TEC1, enciende el LEDR */

      // Leer pin conectado a la tecla.
      tec1Value = !gpioRead( TEC1 );
      // Invertir el valor leido, pues lee un 0 (OFF) con tecla
      // presionada y 1 (ON) al liberarla.

      tec1Value = !tec1Value;
      // Escribir el valor leido en el LED correspondiente.
      gpioWrite( LEDR, tec1Value );


      /* Si se presiona TEC2, enciende el LED1 */

      // Leer pin conectado a la tecla.
      tec2Value = gpioRead( TEC2 );
      // Invertir el valor leido, pues lee un 0 (OFF) con tecla
      // presionada y 1 (ON) al liberarla.
      tec2Value = !tec2Value;
      // Escribir el valor leido en el LED correspondiente.
      gpioWrite( LED1, tec2Value );


      /* Si se presiona TEC3, enciende el LED2 */

      // Leer pin conectado a la tecla.
      tec3Value = gpioRead( TEC3 );
      // Invertir el valor leido, pues lee un 0 (OFF) con tecla
      // presionada y 1 (ON) al liberarla.
      tec3Value = !tec3Value;
      // Escribir el valor leido en el LED correspondiente.
      gpioWrite( LED2, tec3Value );


      /* Si se presiona TEC4, enciende el LED3 */

      // Leer pin conectado a la tecla.
      tec4Value = gpioRead( TEC4 );
      // Invertir el valor leido, pues lee un 0 (OFF) con tecla
      // presionada y 1 (ON) al liberarla.
      tec4Value = !tec4Value;
      // Escribir el valor leido en el LED correspondiente.
      gpioWrite( LED3, tec4Value );


      /* Intercambiar el valor del pin conectado a LEDB */

      gpioToggle( LEDB );


      /* Mostrar por UART_USB (8N1 115200) el estado del LEDB */

      // Leer el estado del pin conectado al led
      ledbValue = gpioRead( LEDB );
      // Chequear si el valor leido es encedido
      if( ledbValue == ON ){
         // Si esta encendido mostrar por UART_USB "LEDB encendido."
         debugPrintlnString( "DEBUG: LEDB encendido." );
         consolePrintlnString( "LEDB encendido." );
         consolePrintEnter();
      } else{
         // Si esta apagado mostrar por UART_USB "LEDB apagado."
         debugPrintlnString( "DEBUG: LEDB apagado." );
         consolePrintlnString( "LEDB apagado." );
         consolePrintEnter();
      }


      /* Retardo bloqueante durante 250ms */

      delay( 100 );
   }
#endif

   // NO DEBE LLEGAR NUNCA AQUI, debido a que a este programa se ejecuta
   // directamenteno sobre un microcontroladore y no es llamado por ningun
   // Sistema Operativo, como en el caso de un programa para PC.
   return 0;
}

/*==================[definiciones de funciones internas]=====================*/

/*==================[definiciones de funciones externas]=====================*/

/*==================[fin del archivo]========================================*/
