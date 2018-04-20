#include "fem_util.h"
#include "uart_util.h"
#include "text.h"
#include "variant.h"


void FEM_PutStatusMessage (struct FEM_Context *ctx,
                           struct UART_Context *uartCtx)
{
    struct VARIANT args[9];
    VARIANT_SetPointer  (&args[0], ctx->state);
    VARIANT_SetString   (&args[1], ctx->info);
    VARIANT_SetUint32   (&args[2], ctx->stateCalls);
    VARIANT_SetUint32   (&args[3], ctx->stateStartTicks);
    VARIANT_SetUint32   (&args[4], ctx->stateCountdownTicks);
    VARIANT_SetPointer  (&args[5], ctx->appCtx);
    VARIANT_SetUint32   (&args[6], ctx->stage);
    VARIANT_SetUint32   (&args[7], ctx->stateCalls);
    VARIANT_SetUint32   (&args[8], ctx->stateStartTicks);

    UART_PutMessage     (uartCtx, TEXT_FEM_STATSBEGIN);
    UART_PutMessageArgs (uartCtx, TEXT_FEM_STATS1, args, 9);

    VARIANT_SetPointer  (&args[0], ctx->invalidStage);
    VARIANT_SetPointer  (&args[1], ctx->maxRecCalls);

    UART_PutMessageArgs (uartCtx, TEXT_FEM_STATS2, args, 2);
    UART_PutMessage     (uartCtx, TEXT_FEM_STATSEND);
}
