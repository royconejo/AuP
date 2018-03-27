#include "board.h"
#include "pwm.h"


void EduCIAA_UpdateOutput (uint32_t outputData, bool state)
{
    Board_LED_Set (outputData & 0x000000FF, state);
}


int main (void)
{
    SystemCoreClockUpdate    ();
    Board_Init               ();
    SysTick_Config           (SystemCoreClock / PWM_TICKS_HZ);

    struct PWM_AnimationContext ledAnimation;

    PWM_ResetAnimation          (&ledAnimation, LED_BLUE, EduCIAA_UpdateOutput);
    PWM_SetAnimationTransition  (&ledAnimation, 0, PWM_AnimTransType_IN, 0, 255, 300);
    PWM_SetAnimationTransition  (&ledAnimation, 1, PWM_AnimTransType_HOLD, 255, 0, 300);
    PWM_SetAnimationTransition  (&ledAnimation, 2, PWM_AnimTransType_OUT, 0, 255, 300);
    PWM_StartAnimation          (&ledAnimation);

    while (1)
    {
        PWM_UpdateAnimation (&ledAnimation);

        PWM_WaitNextCycle ();
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
