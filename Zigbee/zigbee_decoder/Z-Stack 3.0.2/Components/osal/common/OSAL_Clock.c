/******************************************************************************
  Filename:       OSAL_Clock.c
  Revised:        $Date: 2014-06-30 16:38:56 -0700 (Mon, 30 Jun 2014) $
  Revision:       $Revision: 39297 $

  Description:    OSAL Clock definition and manipulation functions.

  Copyright 2008-2014 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License"). You may not use this
  Software unless you agree to abide by the terms of the License. The License
  limits your use, and you acknowledge, that the Software may not be modified,
  copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio
  frequency transceiver, which is integrated into your product. Other than for
  the foregoing purpose, you may not use, reproduce, copy, prepare derivative
  works of, modify, distribute, perform, display or sell this Software and/or
  its documentation for any purpose.

  YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
  PROVIDED “AS IS” WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
  INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE,
  NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
  TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
  NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
  LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
  INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
  OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
  OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
  (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

  Should you have any questions regarding your right to use this Software,
  contact Texas Instruments Incorporated at www.TI.com.
******************************************************************************/

/*********************************************************************
 * INCLUDES
 */

#include "comdef.h"
#include "hal_board.h"
#include "OnBoard.h"
#include "OSAL.h"
#include "OSAL_Clock.h"

/*********************************************************************
 * MACROS
 */

#define	YearLength(yr)	((uint16)(IsLeapYear(yr) ? 366 : 365))

/*********************************************************************
 * CONSTANTS
 */

#define	BEGYEAR  2000     //  UTC started at 00:00:00 January 1, 2000

#define	DAY      86400UL  // 24 hours * 60 minutes * 60 seconds

/* Check Below for an explanation */
#define COUNTER_TICK320US 204775UL 

/* converted COUNTER_TICK320US from 320us ticks to ms */
#define COUNTER_ELAPSEDMS 65528UL   

/* 
 * Each tick is 320us so a value greater than 3 implies 
 * that atleast one millisecond has elapsed 320us*4 > 1 ms 
 */
#define TIMER_CLOCK_UPDATE 4 
                                  
/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */
extern uint32 macMcuPrecisionCount(void);

#if (defined HAL_MCU_CC2430) || (defined HAL_MCU_CC2530) || (defined HAL_MCU_CC2533)

  /*  This function is used to divide a 31 bit dividend by a 16 bit
   *  divisor and return a packed 16 bit quotient and 16 bit
   *  remainder.
   *
   *  Note: This routine takes ~25.6us @32MHz. With C overhead, the
   *        time is ~32us.
   *
   *  dividend - 31 bit dividend.
   *  divisor - 16 bit divisor.
   *
   *  return - MSW divisor; LSW quotient
   */
  extern __near_func uint32 osalMcuDivide31By16To16( uint32 dividend, uint16 divisor );

  #define CONVERT_320US_TO_MS_ELAPSED_REMAINDER( x, y, z ) st( \
                                                               \
    /* The 16 bit quotient is in MSW and */                    \
    /* the 16 bit remainder is in LSW. */                      \
    x = osalMcuDivide31By16To16( x, 25 );                      \
                                                               \
    /* Add quotient to y */                                    \
    y += (x >> 16);                                            \
                                                               \
    /* Copy remainder to z */                                  \
    z = (uint16)(x & 0x0FFFF);                                 \
  )
  
  #define CONVERT_MS_TO_S_ELAPSED_REMAINDER( x, y, z ) st(     \
                                                               \
    /* The 16 bit quotient is in MSW and */                    \
    /* the 16 bit remainder is in LSW. */                      \
    x = osalMcuDivide31By16To16( x, 1000 );                    \
                                                               \
    /* Add quotient to y */                                    \
    y += (x >> 16);                                            \
                                                               \
    /* Copy remainder to z */                                  \
    z = (uint16)(x & 0x0FFFF);                                 \
  )

#else /* (defined HAL_MCU_CC2430) || (defined HAL_MCU_CC2530) || (defined HAL_MCU_CC2533) */

  #define CONVERT_320US_TO_MS_ELAPSED_REMAINDER( x, y, z ) st( \
    y += x / 25;                                               \
    z = x % 25;                                                \
  )
  
  #define CONVERT_MS_TO_S_ELAPSED_REMAINDER( x, y, z ) st(     \
    y += x / 1000;                                             \
    z = x % 1000;                                              \
  )
#endif /* (defined HAL_MCU_CC2430) || (defined HAL_MCU_CC2530) || (defined HAL_MCU_CC2533) */

/*********************************************************************
 * LOCAL VARIABLES
 */
#ifndef USE_ICALL
  static uint32 previousMacTimerTick = 0;
  static uint16 remUsTicks = 0;
#endif /* !USE_ICALL */
  
static uint32 timeMSec = 0;

// number of seconds since 0 hrs, 0 minutes, 0 seconds, on the
// 1st of January 2000 UTC
UTCTime OSAL_timeSeconds = 0;

/*********************************************************************
 * LOCAL FUNCTION PROTOTYPES
 */
static uint8 monthLength( uint8 lpyr, uint8 mon );

static void osalClockUpdate( uint32 elapsedMSec );

/*********************************************************************
 * FUNCTIONS
 *********************************************************************/

/*********************************************************************
 * @fn      osalTimeUpdate
 *
 * @brief   Uses the free running rollover count of the MAC backoff timer;
 *          this timer runs freely with a constant 320 usec interval.  The
 *          count of 320-usec ticks is converted to msecs and used to update
 *          the OSAL clock and Timers by invoking osalClockUpdate() and
 *          osalTimerUpdate().  This function is intended to be invoked
 *          from the background, not interrupt level.
 *
 * @param   None.
 *
 * @return  None.
 */
void osalTimeUpdate( void )
{
#ifndef USE_ICALL
  /* Note that when ICall is in use the OSAL tick is not updated
   * in this fashion but rather through real OS timer tick. */
  halIntState_t intState;
  uint32 tmp;
  uint32 ticks320us;
  uint32 elapsedMSec = 0;

  HAL_ENTER_CRITICAL_SECTION(intState);
  // Get the free-running count of 320us timer ticks
  tmp = macMcuPrecisionCount();
  HAL_EXIT_CRITICAL_SECTION(intState);
  
  if ( tmp != previousMacTimerTick )
  {
    // Calculate the elapsed ticks of the free-running timer.
    ticks320us = (tmp - previousMacTimerTick) & 0xffffffffu;

    if (ticks320us >= TIMER_CLOCK_UPDATE )
    {
      // Store the MAC Timer tick count for the next time through this function.
      previousMacTimerTick = tmp;
      
      /*
       * remUsTicks can have a maximum value of 24 (Since remusTicks got by mod 
       * of 25). The value of COUNTER_TICK320US is a multiple of 25 and the 
       * quotient of  CONVERT_320US_TO_MS_ELAPSED_REMAINDER() does not exceed 
       * 0xFFFF or 16 bit.
       */
      while(ticks320us >= COUNTER_TICK320US)
      {
        ticks320us  -= COUNTER_TICK320US;
        elapsedMSec += COUNTER_ELAPSEDMS;
      }
    
      // update converted number with remaining ticks from loop and the
      // accumulated remainder from loop
      tmp = (ticks320us * 8) + remUsTicks;

      // Convert the 320 us ticks into milliseconds and a remainder
      CONVERT_320US_TO_MS_ELAPSED_REMAINDER( tmp, elapsedMSec, remUsTicks );
      
      // Update OSAL Clock and Timers
      osalClockUpdate( elapsedMSec );
      osalTimerUpdate( elapsedMSec );
    }
  }
#endif /* USE_ICALL */
}

/*********************************************************************
 * @fn      osalClockUpdate
 *
 * @brief   Updates the OSAL Clock time with elapsed milliseconds.
 *
 * @param   elapsedMSec - elapsed milliseconds
 *
 * @return  none
 */
static void osalClockUpdate( uint32 elapsedMSec )
{
  uint32 tmp;
  halIntState_t intState;
  
  HAL_ENTER_CRITICAL_SECTION(intState);
  // Add elapsed milliseconds to the saved millisecond portion of time
  timeMSec += elapsedMSec;

  // Roll up milliseconds to the number of seconds
  if ( timeMSec >= 1000 )
  {
    tmp = timeMSec;
    CONVERT_MS_TO_S_ELAPSED_REMAINDER(tmp, OSAL_timeSeconds, timeMSec);
  }
  HAL_EXIT_CRITICAL_SECTION(intState);
}

#if defined HAL_BOARD_CC2538 || defined USE_ICALL
/*********************************************************************
 * @fn      osalAdjustTimer
 *
 * @brief   Updates the OSAL Clock and Timer with elapsed milliseconds.
 *
 * @param   MSec - elapsed milliseconds
 *
 * @return  none
 */
void osalAdjustTimer(uint32 Msec )
{
  /* Disable SysTick interrupts */ 
  SysTickIntDisable(); 
  
  osalClockUpdate(Msec);
  osalTimerUpdate(Msec);
  
  /* Enable SysTick interrupts */ 
  SysTickIntEnable(); 
}
#endif /* HAL_BOARD_CC2538 || USE_ICALL */

/*********************************************************************
 * @fn      osal_setClock
 *
 * @brief   Set the new time.  This will only set the seconds portion
 *          of time and doesn't change the factional second counter.
 *
 * @param   newTime - number of seconds since 0 hrs, 0 minutes,
 *                    0 seconds, on the 1st of January 2000 UTC
 *
 * @return  none
 */
void osal_setClock( UTCTime newTime )
{
  HAL_CRITICAL_STATEMENT(OSAL_timeSeconds = newTime);
}

/*********************************************************************
 * @fn      osal_getClock
 *
 * @brief   Gets the current time.  This will only return the seconds
 *          portion of time and doesn't include the factional second
 *          counter.
 *
 * @param   none
 *
 * @return  number of seconds since 0 hrs, 0 minutes, 0 seconds,
 *          on the 1st of January 2000 UTC
 */
UTCTime osal_getClock( void )
{
  return ( OSAL_timeSeconds );
}

/*********************************************************************
 * @fn      osal_ConvertUTCTime
 *
 * @brief   Converts UTCTime to UTCTimeStruct
 *
 * @param   tm - pointer to breakdown struct
 *
 * @param   secTime - number of seconds since 0 hrs, 0 minutes,
 *          0 seconds, on the 1st of January 2000 UTC
 *
 * @return  none
 */
void osal_ConvertUTCTime( UTCTimeStruct *tm, UTCTime secTime )
{
  // calculate the time less than a day - hours, minutes, seconds
  {
    uint32 day = secTime % DAY;
    tm->seconds = day % 60UL;
    tm->minutes = (day % 3600UL) / 60UL;
    tm->hour = day / 3600UL;
  }

  // Fill in the calendar - day, month, year
  {
    uint16 numDays = secTime / DAY;
    tm->year = BEGYEAR;
    while ( numDays >= YearLength( tm->year ) )
    {
      numDays -= YearLength( tm->year );
      tm->year++;
    }

    tm->month = 0;
    while ( numDays >= monthLength( IsLeapYear( tm->year ), tm->month ) )
    {
      numDays -= monthLength( IsLeapYear( tm->year ), tm->month );
      tm->month++;
    }

    tm->day = numDays;
  }
}

/*********************************************************************
 * @fn      monthLength
 *
 * @param   lpyr - 1 for leap year, 0 if not
 *
 * @param   mon - 0 - 11 (jan - dec)
 *
 * @return  number of days in specified month
 */
static uint8 monthLength( uint8 lpyr, uint8 mon )
{
  uint8 days = 31;

	if ( mon == 1 ) // feb
  {
		days = ( 28 + lpyr );
  }
  else
  {
    if ( mon > 6 ) // aug-dec
    {
      mon--;
    }

    if ( mon & 1 )
    {
      days = 30;
    }
  }

	return ( days );
}

/*********************************************************************
 * @fn      osal_ConvertUTCSecs
 *
 * @brief   Converts a UTCTimeStruct to UTCTime
 *
 * @param   tm - pointer to provided struct
 *
 * @return  number of seconds since 00:00:00 on 01/01/2000 (UTC)
 */
UTCTime osal_ConvertUTCSecs( UTCTimeStruct *tm )
{
  uint32 seconds;

  /* Seconds for the partial day */
  seconds = (((tm->hour * 60UL) + tm->minutes) * 60UL) + tm->seconds;

  /* Account for previous complete days */
  {
    /* Start with complete days in current month */
    uint16 days = tm->day;

    /* Next, complete months in current year */
    {
      int8 month = tm->month;
      while ( --month >= 0 )
      {
        days += monthLength( IsLeapYear( tm->year ), month );
      }
    }

    /* Next, complete years before current year */
    {
      uint16 year = tm->year;
      while ( --year >= BEGYEAR )
      {
        days += YearLength( year );
      }
    }

    /* Add total seconds before partial day */
    seconds += (days * DAY);
  }

  return ( seconds );
}
