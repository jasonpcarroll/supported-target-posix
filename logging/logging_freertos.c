/* Standard includes. */
#include <stdio.h>
#include <stdarg.h>

/* FreeRTOS-Kernel includes. */
#include "FreeRTOS.h"

/* Logging includes. */
#include "logging.h"

void vLoggingPrintf( const char * pcFormat,
                     ... )
{
    va_list arg;

    va_start( arg, pcFormat );

    taskENTER_CRITICAL();
    {
        vprintf( pcFormat, arg );

        fflush( stdout );
    }
    taskEXIT_CRITICAL();

    va_end( arg );
}
