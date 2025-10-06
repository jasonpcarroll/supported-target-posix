/* Standard includes. */
#include <stdio.h>

/* FreeRTOS-Kernel includes. */
#include "FreeRTOS.h"
#include "semphr.h"

/* Logging includes. */
#include "logging.h"

#if ( configUSE_MUTEXES == 0 )
    #error Logging relies on mutexes being available for thread-safety.
#endif

#if ( configSUPPORT_STATIC_ALLOCATION == 1 )

    static SemaphoreHandle_t xStdioMutex;
    static StaticSemaphore_t xStdioMutexBuffer;

#endif

void vLoggingInit( void )
{
    #if ( configSUPPORT_STATIC_ALLOCATION == 1 )
    {
        xStdioMutex = xSemaphoreCreateMutexStatic( &xStdioMutexBuffer );
    }
    #else /* if( configSUPPORT_STATIC_ALLOCATION == 1 ) */
    {
        xStdioMutex = xSemaphoreCreateMutex();
    }
    #endif /* if( configSUPPORT_STATIC_ALLOCATION == 1 ) */
}

void vLoggingPrintf( const char * pcFormat,
                     ... )
{
    va_list arg;

    va_start( arg, pcFormat );

    xSemaphoreTake( xStdioMutex, portMAX_DELAY );

    vprintf( pcFormat, arg );

    xSemaphoreTake( xStdioMutex, portMAX_DELAY );

    va_end( arg );
}
