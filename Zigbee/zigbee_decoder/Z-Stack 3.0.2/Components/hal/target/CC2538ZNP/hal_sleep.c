/**************************************************************************************************
  Filename:       _hal_sleep.c
  Revised:        $Date: 2014-12-05 13:07:19 -0800 (Fri, 05 Dec 2014) $
  Revision:       $Revision: 41365 $

  Description:    This module contains the HAL power management procedures for the CC2538.


  Copyright 2011-2014 Texas Instruments Incorporated. All rights reserved.

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
**************************************************************************************************/

/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */

#include "hal_mcu.h"
#include "hal_board.h"
#include "hal_sleep.h"
#include "hal_led.h"
#include "hal_key.h"
#include "mac_api.h"
#include "comdef.h"
#include "OSAL.h"
#include "OSAL_Clock.h"
#include "OSAL_PwrMgr.h"
#include "OnBoard.h"
#include "hal_drivers.h"
#include "hal_assert.h"
#include "mac_mcu.h"

#ifndef ZG_BUILD_ENDDEVICE_TYPE
# define ZG_BUILD_ENDDEVICE_TYPE FALSE
#endif

#if ZG_BUILD_ENDDEVICE_TYPE && defined (NWK_AUTO_POLL)
#include "nwk_globals.h"
#include "ZGlobals.h"
#endif

/* ------------------------------------------------------------------------------------------------
 *                                           Macros
 * ------------------------------------------------------------------------------------------------
 */

/* POWER CONSERVATION DEFINITIONS
 * Sleep mode H/W definitions (enabled with POWER_SAVING compile option)
 */
#define CC2538_PM0            0  /* PM0, Clock oscillators on, voltage regulator on */
#define CC2538_PM1            1  /* PM1, 32.768 kHz oscillators on, voltage regulator on */
#define CC2538_PM2            2  /* PM2, 32.768 kHz oscillators on, voltage regulator off */
#define CC2538_PM3            3  /* PM3, All clock oscillators off, voltage regulator off */

/* HAL power management mode is set according to the power management state. The default
 * setting is HAL_SLEEP_OFF. The actual value is tailored to different HW platform. Both
 * HAL_SLEEP_TIMER and HAL_SLEEP_DEEP selections will:
 *   1. turn off the system clock, and
 *   2. halt the MCU.
 * HAL_SLEEP_TIMER can be woken up by sleep timer interrupt, I/O interrupt and reset.
 * HAL_SLEEP_DEEP can be woken up by I/O interrupt and reset.
 */
#define HAL_SLEEP_OFF         CC2538_PM0
#define HAL_SLEEP_CHECK       CC2538_PM1
#define HAL_SLEEP_TIMER       CC2538_PM2
#define HAL_SLEEP_DEEP        CC2538_PM3

#define SLEEP_IE_FLAG    BV(INT_SMTIM - 48)  /* Sleep Interrupt Enable bit */
#define GPIOA_IE_FLAG    BV(INT_GPIOA - 16)  /* GPIOA Interrupt Enable bit */
#define GPIOC_IE_FLAG    BV(INT_GPIOC - 16)  /* GPIOC Interrupt Enable bit */
#define GPIOAC_IE_FLAG   (GPIOC_IE_FLAG | GPIOA_IE_FLAG)
#define DISABLE_IE_FLAG   0UL                /* GPIOC Interrupt Enable bit */

/* MAX_SLEEP_TIME calculation:
 *   Sleep timer maximum duration = 0xFFFF7F / 32768 Hz = 511.996 seconds
 *   Round it to 510 seconds or 510000 ms
 */
#define MAX_SLEEP_TIME                   510000             /* maximum time to sleep allowed by ST */

/* 
 * Choosing value to be lower than MAC_BACKOFF_TIMER_DEFAULT_NONBEACON_ROLLOVER
 *  The unit is in ms. The back off timer rollover should be greater 
 * than the value below
 */ 
#define MAX_SLEEP_LOOP_TIME              0x510000           /* ~84 minutes */         

/* minimum time to sleep, this macro is to:
 * 1. avoid thrashing in-and-out of sleep with short OSAL timer (~2ms)
 * 2. define minimum safe sleep period
 */
/* default to minimum safe sleep time minimum CAP */
#if !defined (PM_MIN_SLEEP_TIME)
#define PM_MIN_SLEEP_TIME                2  
#endif

#define SLEEP_TIMER_ROLLOVER                  0xFFFFFFFF

/* This value is used to adjust the sleep timer compare value such that the sleep timer
 * compare takes into account the amount of processing time spent in function halSleep().
 * The first value is determined by measuring the number of sleep timer ticks it from
 * the beginning of the function to entering sleep mode or more precisely, when
 * MAC_PwrNextTimeout() is called.  The second value is determined by measuring the number
 * of sleep timer ticks from exit of sleep mode to the call to MAC_PwrOnReq() where the
 * MAC timer is restarted.
 */
#define HAL_SLEEP_ADJ_TICKS   (11 + 12)

#ifndef HAL_SLEEP_DEBUG_POWER_MODE
/* set CC2538 power mode; always use PM2 */
#define HAL_SLEEP_PREP_POWER_MODE(mode)  st( PMCTL &= ~SYS_CTRL_PMCTL_PM_M; /* clear mode bits */ \
                                                PMCTL |= mode;   /* set mode bits   */            \
                                                while (!(STLOAD & LDRDY));                        \
                                                halSleepPconValue = PCON_IDLE;                    \
                                              )
#define HAL_SLEEP_SET_POWER_MODE()          halSetSleepMode()
#else
/* Debug: don't set power mode, just block until sleep timer interrupt */
#define HAL_SLEEP_PREP_POWER_MODE(mode)     /* nothing */
#define HAL_SLEEP_SET_POWER_MODE()          st( while(halSleepInt == FALSE); \
                                                halSleepInt = FALSE;         \
                                                HAL_DISABLE_INTERRUPTS();    \
                                              )
#endif

/* sleep timer interrupt control */
#define HAL_SLEEP_TIMER_ENABLE_INT()        st(IntEnable(INT_SMTIM);)    /* enable sleep timer interrupt */
#define HAL_SLEEP_TIMER_DISABLE_INT()       st(IntDisable(INT_SMTIM);)   /* disable sleep timer interrupt */
#define HAL_SLEEP_TIMER_CLEAR_INT()         st(IntPendClear(INT_SMTIM);) /* clear sleep interrupt flag */

/* backup interrupt enable registers before sleep */
#define HAL_SLEEP_IE_BACKUP_AND_DISABLE(ien0, ien1, ien2, ien3, ien4)                                  \
                                              st(ien0  = (HWREG(NVIC_EN0)); /* backup IEN0 register */ \
                                                ien1  = (HWREG(NVIC_EN1)); /* backup IEN1 register */  \
                                                ien2  = (HWREG(NVIC_EN2)); /* backup IEN2 register */  \
                                                ien3  = (HWREG(NVIC_EN3)); /* backup IEN3 register */  \
                                                ien4  = (HWREG(NVIC_EN4)); /* backup IEN4 register */  \
                                                 /* disable interrupts in EN0 except GPIO A & C*/      \
                                                (HWREG(NVIC_EN0)) &= GPIOAC_IE_FLAG;                   \
                                                 /* disable interrupts in EN1 except Sleep */          \
                                                (HWREG(NVIC_EN1)) &= SLEEP_IE_FLAG;                    \
                                                (HWREG(NVIC_EN2)) &= DISABLE_IE_FLAG;                  \
                                                (HWREG(NVIC_EN3)) &= DISABLE_IE_FLAG;                  \
                                                (HWREG(NVIC_EN4)) &= DISABLE_IE_FLAG;)

/* disable IEN2 except Port A and C*/
/* restore interrupt enable registers before sleep */          
#define HAL_SLEEP_IE_RESTORE(ien0, ien1, ien2, ien3, ien4)                                      \
                                     st((HWREG(NVIC_EN0)) = ien0;   /* restore IEN0 register */ \
                                     (HWREG(NVIC_EN1)) = ien1;   /* restore IEN1 register */    \
                                     (HWREG(NVIC_EN2)) = ien2;   /* restore IEN2 register */    \
                                     (HWREG(NVIC_EN3)) = ien3;   /* restore IEN3 register */    \
                                     (HWREG(NVIC_EN4)) = ien4;)  /* restore IEN4 register */                                                                

/* convert msec to 320 usec units with round */
#define HAL_SLEEP_MS_TO_320US(ms)           (((((uint32) (ms)) * 100) + 31) / 32)

/* for optimized indexing of uint32's */
#if HAL_MCU_LITTLE_ENDIAN()
#define UINT32_NDX0   0
#define UINT32_NDX1   1
#define UINT32_NDX2   2
#define UINT32_NDX3   3
#else
#define UINT32_NDX0   3
#define UINT32_NDX1   2
#define UINT32_NDX2   1
#define UINT32_NDX3   0
#endif

/* ------------------------------------------------------------------------------------------------
 *                                        Global Variables
 * ------------------------------------------------------------------------------------------------
 */
/* PCON flag value to program when setting power mode */
#if defined (ewarm)
volatile __data uint8 halSleepPconValue = PCON_IDLE;
#else
volatile uint8 halSleepPconValue = PCON_IDLE;
#endif
/* ------------------------------------------------------------------------------------------------
 *                                        Local Variables
 * ------------------------------------------------------------------------------------------------
 */

/* HAL power management mode is set according to the power management state.
 */
static uint8 halPwrMgtMode = HAL_SLEEP_OFF;

#ifdef HAL_SLEEP_DEBUG_POWER_MODE
static bool halSleepInt = FALSE;
#endif

/* ------------------------------------------------------------------------------------------------
 *                                      Function Prototypes
 * ------------------------------------------------------------------------------------------------
 */

void halSetSleepMode(void);
void halSleepTimerIsr(void);
uint32 halSleepSetTimer(uint32 timeout);

/**************************************************************************************************
 * @fn          halSleep
 *
 * @brief       This function put the CC2538 to sleep.
 *
 * input parameters
 *
 * @param       None.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
void halSetSleepMode(void)
{ 
  /* Clear if any pending GPIO power interrupts */
  SysCtrlPowIntClear();
  /* Enable SysCtrl Power interrupts during sleep */
  SysCtrlPowIntEnableSetting();
  
  /* If sleep pcon flag is enabled then go to deep sleep */
  if(halSleepPconValue)
  {  
    SysCtrlDeepSleep();
  }
  
  /* Disable SysCtrl Power interrupts after wakeup from sleep */
  SysCtrlPowIntDisableSetting();
  
  HAL_DISABLE_INTERRUPTS();
}

/**************************************************************************************************
 * @fn          halSleep
 *
 * @brief       This function is called from the OSAL task loop using and existing OSAL
 *              interface.  It sets the low power mode of the MAC and the CC2538.
 *
 * input parameters
 *
 * @param       osal_timeout - Next OSAL timer timeout.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
void halSleep( uint32 osal_timeout )
{
  uint32        timeout;
  uint32        macTimeout = 0;
  uint32        sleepTimeStart = 0;
  uint32        sleepTimeStop = 0;
  uint32        sleepTime = 0;
  uint32        sleepTimeinms = 0;
  
  /* get next OSAL timer expiration converted to 320 usec units */
  timeout = HAL_SLEEP_MS_TO_320US(osal_timeout);
  
  if (timeout == 0)
  {
    timeout = MAC_PwrNextTimeout();
  }
  else
  {
    /* get next MAC timer expiration */
    macTimeout = MAC_PwrNextTimeout();

    /* get lesser of two timeouts */
    if ((macTimeout != 0) && (macTimeout < timeout))
    {
      timeout = macTimeout;
    }
  }
  
  /* HAL_SLEEP_PM2 is entered only if the timeout is zero and
   * the device is a stimulated device.
   */
  halPwrMgtMode = (timeout == 0) ? HAL_SLEEP_DEEP : HAL_SLEEP_TIMER;

  /* DEEP sleep can only be entered when zgPollRate == 0.
   * This is to eliminate any possibility of entering PM3 between
   * two network timers.
   */
#if ZG_BUILD_ENDDEVICE_TYPE && defined (NWK_AUTO_POLL)
  if ((timeout > HAL_SLEEP_MS_TO_320US(PM_MIN_SLEEP_TIME)) ||
      (timeout == 0 && zgPollRate == 0))
#else
  if ((timeout > HAL_SLEEP_MS_TO_320US(PM_MIN_SLEEP_TIME)) ||
      (timeout == 0))
#endif
  {
    uint32 ien0, ien1, ien2, ien3, ien4;

    HAL_ASSERT(HAL_INTERRUPTS_ARE_ENABLED());
    HAL_DISABLE_INTERRUPTS();
    
    /* always use "deep sleep" to turn off radio VREG on CC2538 */
    if (halSleepPconValue != 0 && MAC_PwrOffReq(MAC_PWR_SLEEP_DEEP) == MAC_SUCCESS)
    {
      /* The PCON value is not zero. There is no interrupt overriding the 
       * sleep decision. Also, the radio granted the sleep request.
       */

#if ((defined HAL_KEY) && (HAL_KEY == TRUE))
      /* get peripherals ready for sleep */
      HalKeyEnterSleep();
#endif

#ifdef HAL_SLEEP_DEBUG_LED
      HAL_TURN_OFF_LED3();
#else
      /* use this to turn LEDs off during sleep */
      HalLedEnterSleep();
#endif

      if(timeout > HAL_SLEEP_MS_TO_320US( MAX_SLEEP_LOOP_TIME))
      {
        timeout = HAL_SLEEP_MS_TO_320US(MAX_SLEEP_LOOP_TIME);
      }  
      
      do
       {
        /* enable sleep timer interrupt */
        if(timeout != 0)
        { 
          if (timeout > HAL_SLEEP_MS_TO_320US( MAX_SLEEP_TIME ))
          {
            timeout -= HAL_SLEEP_MS_TO_320US( MAX_SLEEP_TIME );
            sleepTimeStart = halSleepSetTimer(HAL_SLEEP_MS_TO_320US( MAX_SLEEP_TIME ));
          }
          else
          {
            /* set sleep timer */
            sleepTimeStart = halSleepSetTimer(timeout);
            timeout = 0;
          }
          
          /* set up sleep timer interrupt */
          HAL_SLEEP_TIMER_CLEAR_INT();
          HAL_SLEEP_TIMER_ENABLE_INT();
        }

#ifdef HAL_SLEEP_DEBUG_LED
        if (halPwrMgtMode == HAL_SLEEP_CHECK)
        {
          HAL_TURN_ON_LED1();
        }
        else
        {
          HAL_TURN_OFF_LED1();
        }
#endif
        /* Prep CC2538 power mode */
        HAL_SLEEP_PREP_POWER_MODE(halPwrMgtMode);
        /* save interrupt enable registers and disable all  
         * interrupts except for sleep timer and Key press 
         */
        HAL_SLEEP_IE_BACKUP_AND_DISABLE(ien0, ien1, ien2, ien3, ien4);
        /* Disable SysTick Interrupt */
        SysTickIntDisable();
        
        /* Master Interrupt Enable */
        HAL_ENABLE_INTERRUPTS();
        
        /* set CC2538 power mode, interrupt is disabled after this function
         * Note that an ISR (that could wake up from power mode) which runs
         * between the previous instruction enabling interrupts and before
         * power mode is set would switch the halSleepPconValue so that
         * power mode shall not be entered in such a case. 
         */
        HAL_SLEEP_SET_POWER_MODE(); 
        
        /* restore interrupt enable registers */
        HAL_SLEEP_IE_RESTORE(ien0, ien1, ien2, ien3, ien4);

        /* disable sleep timer interrupt */
        HAL_SLEEP_TIMER_DISABLE_INT();

        if(HAL_SLEEP_TIMER == halPwrMgtMode)
        {
          /* To Calculate duration of time in sleep, 
           * read the sleep timer; ST0 must be read first 
           */
          ((uint8 *) &sleepTimeStop)[UINT32_NDX0] = ST0;
          ((uint8 *) &sleepTimeStop)[UINT32_NDX1] = ST1;
          ((uint8 *) &sleepTimeStop)[UINT32_NDX2] = ST2;
          ((uint8 *) &sleepTimeStop)[UINT32_NDX3] = ST3;
          
          sleepTime += ((sleepTimeStop - sleepTimeStart) + HAL_SLEEP_ADJ_TICKS) 
            & 0xffffffffu;
         }

#ifdef HAL_SLEEP_DEBUG_LED
        HAL_TURN_ON_LED3();
#else
        /* use this to turn LEDs back on after sleep */
        HalLedExitSleep();
#endif

#if ((defined HAL_KEY) && (HAL_KEY == TRUE))
        /* handle peripherals */
        if(HalKeyExitSleep())
        {
          break; 
        }
#endif
      } while(timeout != 0);
      
      /* Convert 32Khz ticks to ms = sleeptime * 1000/ 32768 */
      sleepTimeinms = ((sleepTime * 125) / 4096);
      
      if(sleepTimeinms)
      {
        /* Update the osal timers and clock with the sleep time */
        osalAdjustTimer(sleepTimeinms);
      }
      
      /* Enable Systick interrupts*/
      SysTickIntEnable();
      /* power on the MAC; blocks until completion */
      MAC_PwrOnReq();
      

      /* Enable the interrupts */
      HAL_ENABLE_INTERRUPTS();

      /* For CC2530, T2 interrupt won’t be generated when the current count is greater than
       * the comparator. The interrupt is only generated when the current count is equal to
       * the comparator. When the CC2530 is waking up from sleep, there is a small window
       * that the count may be grater than the comparator, therefore, missing the interrupt.
       * This workaround will call the T2 ISR when the current T2 count is greater than the
       * comparator. The problem only occurs when POWER_SAVING is turned on, i.e. the 32KHz
       * drives the chip in sleep and SYNC start is used.
       */
      macMcuTimer2OverflowWorkaround();
    }
    else
    {
      /* An interrupt may have changed the sleep decision. Do not sleep at all. Turn on
       * the interrupt, exit normally, and the next sleep will be allowed.
       */
      HAL_ENABLE_INTERRUPTS();
    }
#if defined FEATURE_8MHZ_HYBRID_POWER_SAVING
    SysCtrlClockSet(OSC_32KHZ, 
                    false,  
                    SYS_CTRL_SYSDIV_8MHZ);
#endif
  }
}

/**************************************************************************************************
 * @fn          halSleepSetTimer
 *
 * @brief       This function sets the CC2538 sleep timer compare value.  First it reads and
 *              stores the value of the sleep timer; this value is used later to update OSAL
 *              timers.  Then the timeout value is converted from 320 usec units to 32 kHz
 *              period units and the compare value is set to the timeout.
 *
 * input parameters
 *
 * @param       timeout - Timeout value in 320 usec units.  The sleep timer compare is set to
 *                        this value.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
uint32 halSleepSetTimer(uint32 timeout)
{
  uint32 ticks;
  uint32 sleeptime;

  /* read the sleep timer; ST0 must be read first */
  ((uint8 *) &ticks)[UINT32_NDX0] = ST0;
  ((uint8 *) &ticks)[UINT32_NDX1] = ST1;
  ((uint8 *) &ticks)[UINT32_NDX2] = ST2;
  ((uint8 *) &ticks)[UINT32_NDX3] = ST3;
  
  /* store ticks in sleeptime */
  sleeptime = ticks;
  
  /* Compute sleep timer compare value.  The ratio of 32 kHz ticks to 320 usec ticks
   * is 32768/3125 = 10.48576.  This is nearly 671/64 = 10.484375.
   */
  ticks += (timeout * 671) / 64;

  /* subtract the processing time spent in function halSleep() */
  if(ticks >= HAL_SLEEP_ADJ_TICKS)
  {  
    ticks -= HAL_SLEEP_ADJ_TICKS;
  }
  
  /* set sleep timer compare; ST0 must be written last */
  ST3 = ((uint8 *) &ticks)[UINT32_NDX3];
  ST2 = ((uint8 *) &ticks)[UINT32_NDX2];
  ST1 = ((uint8 *) &ticks)[UINT32_NDX1];
  ST0 = ((uint8 *) &ticks)[UINT32_NDX0];
  
  return sleeptime; 
}

/**************************************************************************************************
 * @fn          TimerElapsed
 *
 * @brief       Determine the number of OSAL timer ticks elapsed during sleep.
 *              Deprecated for CC2538 and CC2430 SoC.
 *
 * input parameters
 *
 * @param       None.
 *
 * output parameters
 *
 * None.
 *
 * @return      Number of timer ticks elapsed during sleep.
 **************************************************************************************************
 */
uint32 TimerElapsed( void )
{
  /* Stubs */
  return (0);
}

/**************************************************************************************************
 * @fn          halRestoreSleepLevel
 *
 * @brief       Restore the deepest timer sleep level.
 *
 * input parameters
 *
 * @param       None
 *
 * output parameters
 *
 *              None.
 *
 * @return      None.
 **************************************************************************************************
 */
void halRestoreSleepLevel( void )
{
  /* Stubs */
}

/**************************************************************************************************
 * @fn          halSleepTimerIsr
 *
 * @brief       Sleep timer ISR.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
void halSleepTimerIsr(void)
{
  /* Clear any pending sleep interrupts */
  HAL_SLEEP_TIMER_CLEAR_INT();
  
#ifdef HAL_SLEEP_DEBUG_POWER_MODE
  halSleepInt = TRUE;
#endif
  
  /* Disable sleep mode till the next Hal_ProcessPoll  */
  CLEAR_SLEEP_MODE();
}


