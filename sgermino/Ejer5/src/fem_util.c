#include "fem_util.h"
#include "uart_util.h"
#include "text.h"
#include "variant.h"


void FEM_PutStatusMessage (struct FEM *f, struct UART *uart)
{
    struct VARIANT args[9];
    VARIANT_SetPointer  (&args[0], f->state);
    VARIANT_SetString   (&args[1], f->info);
    VARIANT_SetUint32   (&args[2], f->stateCalls);
    VARIANT_SetUint32   (&args[3], f->stateStartTicks);
    VARIANT_SetUint32   (&args[4], f->stateCountdownTicks);
    VARIANT_SetPointer  (&args[5], f->app);
    VARIANT_SetUint32   (&args[6], f->stage);
    VARIANT_SetUint32   (&args[7], f->stateCalls);
    VARIANT_SetUint32   (&args[8], f->stateStartTicks);

    UART_PutMessage     (uart, TEXT_FEM_STATSBEGIN);
    UART_PutMessageArgs (uart, TEXT_FEM_STATS1, args, 9);

    VARIANT_SetPointer  (&args[0], f->invalidStage);
    VARIANT_SetPointer  (&args[1], f->maxRecCalls);

    UART_PutMessageArgs (uart, TEXT_FEM_STATS2, args, 2);
    UART_PutMessage     (uart, TEXT_FEM_STATSEND);
}
