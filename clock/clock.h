/**
 * @file clock.h
 * @brief Time-related functions for getting time and sleeping.
 */

#ifndef CLOCK_H_
#define CLOCK_H_

/* Standard includes. */
#include <stdint.h>

/* *INDENT-OFF* */
#ifdef __cplusplus
    extern "C" {
#endif
/* *INDENT-ON* */

/**
 * @brief The timer query function.
 *
 * This function returns the elapsed time.
 *
 * @return Time in milliseconds.
 */
uint32_t Clock_GetTimeMs( void );

/**
 * @brief Millisecond sleep function.
 *
 * @param[in] sleepTimeMs milliseconds to sleep.
 */
void Clock_SleepMs( uint32_t sleepTimeMs );

/* *INDENT-OFF* */
#ifdef __cplusplus
    }
#endif
/* *INDENT-ON* */

#endif /* ifndef CLOCK_H_ */
