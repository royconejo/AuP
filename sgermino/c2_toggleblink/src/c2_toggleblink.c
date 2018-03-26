#include "board.h"
#include <string.h>


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


#define FIXED_U1616_TO_U8(n) ((n >> 16) & 0xFF)
#define DOUBLE_TO_FIXED_1616(n) (n * (1 << 16))

#define animationRepeatForever     0xFFFFFFFF


enum animationStatus_t
{
    animationStatus_t_STOP = 0,
    animationStatus_t_PLAYING
};

enum transitionType_t
{
    transitionType_IN,
    transitionType_OUT,
    transitionType_HOLD
};


typedef uint32_t fixed_u1616_t;


struct transitionData_t
{
    enum transitionType_t
                    type;
    fixed_u1616_t   min;
    fixed_u1616_t   max;
    fixed_u1616_t   step;
    fixed_u1616_t   current;
    uint32_t        duration;
    uint32_t        elapsed;
};


struct animationData_t
{
    enum animationStatus_t
                status;
    uint8_t     intensity;
    struct
    {
        uint32_t                current;
        struct transitionData_t data[4];
    }
    transition;
};


void animationReset (struct animationData_t *ad)
{
    if (!ad)
    {
        return;
    }

    memset (ad, 0, sizeof(struct animationData_t));
}


static void updateTransitionDuration (struct transitionData_t* td,
                                      uint32_t duration)
{
    //  TODO: assert (td->current > td->min && td->current < td->max)

    if (!duration)
    {
        td->step        = 0;
        td->duration    = 0;
        td->elapsed     = 0;
        return;
    }

    // Los calculos para determinar el step se realizan sobre el duration
    // proporcional segun el duration y elapsed anterior al cambio
    // (si la animacion estaba a mitad de esta transicion, va a seguir en la
    // mitad pero con nueva duracion y stepping)
    const double    ElapsedPercent = td->elapsed / (double)td->duration;

    td->duration    = duration;
    td->elapsed     = td->duration * ElapsedPercent;

    // La intensidad del LED se actualiza cada 1 periodo. La diferencia entre
    // la intensidad MAX y MIN devuelve la cantidad de periodos (transiciones)
    // entre ambos estados. La cantidad de periodos en un segundo es constante
    // (PWM_PERIOD_HZ) y se interpola a la duracion especificada en
    // milisegundos para obtener la magnitud del "step" por cada periodo.
    const double    DurationSecs = ((double)td->duration - td->elapsed) * 0.001;
    const double    PeriodsInDuration = PWM_PERIOD_HZ * DurationSecs;

    switch (td->type)
    {
        case transitionType_IN:
        case transitionType_OUT:
        {
            const uint8_t ProportionalPeriods = FIXED_U1616_TO_U8(td->max) -
                                                FIXED_U1616_TO_U8(td->current);
            td->step = DOUBLE_TO_FIXED_1616(ProportionalPeriods /
                                            PeriodsInDuration);
            break;
        }
        case transitionType_HOLD:
            td->step = DOUBLE_TO_FIXED_1616(PeriodsInDuration);
            break;
    }
}


void animationSetTransition (struct animationData_t *ad, uint32_t number,
                             enum transitionType_t type, uint8_t min,
                             uint8_t max, uint32_t duration)
{
    if (!ad || number > 3)
    {
        return;
    }

    struct transitionData_t* td = &ad->transition.data[number];
    td->type        = type;
    td->min         = ((uint32_t)min << 16);
    td->max         = ((uint32_t)max << 16);
    td->step        = 0;
    td->duration    = duration;
    td->elapsed     = 0;

    updateTransitionDuration (td, duration);
}


void animationDisableTransition (struct animationData_t *ad, uint32_t number)
{
    animationSetTransition (ad, number, transitionType_IN, 0, 0, 0);
}


static void rewindTransition (struct transitionData_t* td)
{
    switch (td->type)
    {
        case transitionType_IN:
            td->current = td->min;
            break;

        case transitionType_OUT:
            td->current = td->max;
            break;

        case transitionType_HOLD:
            td->current = td->min;
            break;
    }

    td->elapsed = 0;
}


static void animationSetNextTransition (struct animationData_t *ad)
{
    struct transitionData_t* td = &ad->transition.data[ad->transition.current];

    uint32_t testTransitions = 4;
    while (testTransitions --)
    {
        ad->transition.current = (ad->transition.current + 1) & 0x3;
        td = &ad->transition.data[ad->transition.current];

        if (td->duration)
        {
            rewindTransition (td);
            return;
        }
    }

    ad->status == animationStatus_t_STOP;
}


void animationUpdate (struct animationData_t *ad)
{
    if (!ad || ad->status == animationStatus_t_STOP)
    {
        return;
    }

    struct transitionData_t* td = &ad->transition.data[ad->transition.current];

    switch (td->type)
    {
        case transitionType_IN:
            td->current += td->step;
            if (td->current > td->max)
            {
                ad->intensity = FIXED_U1616_TO_U8(td->max);
                animationSetNextTransition (ad);
                return;
            }
            break;

        case transitionType_OUT:
            td->current -= td->step;
            if (td->current < td->step || td->current < td->min)
            {
                ad->intensity = FIXED_U1616_TO_U8(td->min);
                animationSetNextTransition (ad);
                return;
            }
            break;

        case transitionType_HOLD:
            td->max += td->step;
            if (td->max >= PWM_PERIOD_RES)
            {
                animationSetNextTransition (ad);
            }
            break;
    }

    ad->intensity = FIXED_U1616_TO_U8(td->current);
}


void animationStart (struct animationData_t *ad)
{
    if (!ad)
    {
        return;
    }

    struct transitionData_t* td;

    for (ad->transition.current = 0; ad->transition.current < 4;
         ++ ad->transition.current)
    {
        td = &ad->transition.data[ad->transition.current];
        if (td->duration)
        {
            rewindTransition (td);
            ad->intensity   = td->current;
            ad->status      = animationStatus_t_PLAYING;
            return;
        }
    }
}


void pwmUpdate (uint8_t port, uint8_t intensity)
{
    Board_LED_Set (port, intensity <= g_pwmPeriod);
}


void animationUpdatePWM (uint8_t port, struct animationData_t *ad)
{
    pwmUpdate (port, ad->intensity);
}


int main (void)
{
    SystemCoreClockUpdate    ();
    Board_Init               ();
    SysTick_Config           (SystemCoreClock / PWM_TICKS_HZ);

    struct animationData_t ledAnimation;

    animationReset          (&ledAnimation);
    animationSetTransition  (&ledAnimation, 0, transitionType_IN, 0, 255, 1500);
    animationSetTransition  (&ledAnimation, 1, transitionType_OUT, 0, 255, 500);
    animationStart          (&ledAnimation);

    while (1)
    {
        animationUpdatePWM (LED_RED, &ledAnimation);

        if (g_newPeriod)
        {
            g_newPeriod = false;
            animationUpdate (&ledAnimation);
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
