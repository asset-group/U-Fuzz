/**************************************************************************************************
  Filename:       hal_timer.c
  Revised:        $Date: 2014-12-05 13:20:43 -0800 (Fri, 05 Dec 2014) $
  Revision:       $Revision: 41366 $

  Description:   This file contains the interface to the Timer Service.


  Copyright 2014 Texas Instruments Incorporated. All rights reserved.

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
/*********************************************************************
 NOTE: The following mapping is done between the logical timer
       names defined in HAL_TIMER.H and the physical HW timer.

       HAL_TIMER_0 --> HW Timer 3  (8-bits)
       HAL_TIMER_2 --> HW Timer 4  (8-bits)
       HAL_TIMER_3 --> HW Timer 1  (16-bits)

 NOTE: The timer code assumes only one channel, CHANNEL 0, is used
       for each timer.  There is currently no support for other
       channels.

 NOTE: Only Output Compare Mode is supported.  There is no provision
       to support Input Capture Mode.

 NOTE: There is no support to map the output of the timers to a
       physical I/O pin

*********************************************************************/
/*********************************************************************
 * INCLUDES
 */
#include  "hal_mcu.h"
#include  "hal_defs.h"
#include  "hal_types.h"
#include  "hal_timer.h"

#include "hw_ints.h"
#include "hw_memmap.h"
#include "gpio.h"
#include "interrupt.h"
#include "ioc.h"
#include "hw_ioc.h"
#include "sys_ctrl.h"
#include "gptimer.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */
#define PWM_PERIOD 1000

/*********************************************************************
 * FUNCTIONS - External
 */

/*********************************************************************
 * FUNCTIONS - Local
 */
uint8 halTimer1SetPeriod (uint32 timePerTick);
void halTimer1SetChannelCount (uint8 channel, uint16 count);
uint8 halTimerSetCount (uint8 hwtimerid, uint32 timePerTick);

/*********************************************************************
 * FUNCTIONS - API
 */

/*********************************************************************
 * @fn      HalTimer1Init
 *
 * @brief   Initialize Timer Service
 *
 * @param   None
 *
 * @return  None
 */
void HalTimer1Init (halTimerCBack_t cBack)
{
  //not used for now
  (void) cBack;

  //
  // Enable GPT0
  //  
  SysCtrlPeripheralEnable(SYS_CTRL_PERIPH_GPT0);
    
  //
  // Configure Timer0A for PWM use
  //
  TimerConfigure(GPTIMER0_BASE, GPTIMER_CFG_SPLIT_PAIR |
                 GPTIMER_CFG_A_PWM);

  //
  // Set Duty cycle and enable
  //
  TimerLoadSet(GPTIMER0_BASE, GPTIMER_A, PWM_PERIOD);
  TimerMatchSet(GPTIMER0_BASE, GPTIMER_A, PWM_PERIOD);
  TimerEnable(GPTIMER0_BASE, GPTIMER_A);

  //
  // Set duty cycle to 0
  //
  TimerMatchSet(GPTIMER0_BASE, GPTIMER_A, 0); 
}

/***************************************************************************************************
 * @fn      halTimer1SetPeriod
 *
 * @brief   Set period for Timer1 PWM
 *
 * @param   timerPerTick - Number micro sec per ticks
 *
 * @return  Status - OK or Not OK
 ***************************************************************************************************/
uint8 halTimer1SetPeriod (uint32 timePerTick)
{
  //Stub for now, only PWM implemented, with hard coded PWM period
  return 0;
}

/***************************************************************************************************
 * @fn      halTimer1SetChannelDuty
 *
 * @brief   Set duty for Timer1 PWM
 *
 * @param   channel - timer channel to control
 *          percent - dutycycle in promill
 *
 * @return  None
 ***************************************************************************************************/
void halTimer1SetChannelDuty (uint8 channel, uint16 promill)
{
  if(channel == 0)
  {
    uint32 timerAMatch = (PWM_PERIOD * (100-promill)) / 100;
    
    if(timerAMatch == PWM_PERIOD)
    {
      timerAMatch--;
    }
    
    //
    // The PWM counter counts down thus calculate actual match count based
    // on duty cycle as: period*(100-DucyCycle)/100
    //
    TimerMatchSet(GPTIMER0_BASE, GPTIMER_A, timerAMatch); 
  }
}

/***************************************************************************************************
 * @fn      halTimer1SetChannelCount
 *
 * @brief   Stop the Timer Service
 *
 * @param   hwtimerid - ID of the timer
 *          timerPerTick - Number micro sec per ticks
 *
 * @return  Status - OK or Not OK
 ***************************************************************************************************/
void halTimer1SetChannelCount (uint8 channel, uint16 count)
{
  //Stub for now, only PWM implemented
}

/***************************************************************************************************
 * @fn      HalTimerInit
 *
 * @brief   Initialize Timer Service
 *
 * @param   None
 *
 * @return  None
 ***************************************************************************************************/
void HalTimerInit (void)
{
  //Stub for now, only PWM implemented
  return;
}

/***************************************************************************************************
 * @fn      HalTimerConfig
 *
 * @brief   Configure the Timer Serivce
 *
 * @param   timerId - Id of the timer
 *          opMode  - Operation mode
 *          channel - Channel where the counter operates on
 *          channelMode - Mode of that channel
 *          prescale - Prescale of the clock
 *          cBack - Pointer to the callback function
 *
 * @return  Status of the configuration
 ***************************************************************************************************/
uint8 HalTimerConfig (uint8 timerId, uint8 opMode, uint8 channel, uint8 channelMode,
                      bool intEnable, halTimerCBack_t cBack)
{
  //Stub for now, only PWM implemented
  return 0;
}

/***************************************************************************************************
 * @fn      HalTimerStart
 *
 * @brief   Start the Timer Service
 *
 * @param   timerId      - ID of the timer
 *          timerPerTick - number of micro sec per tick, (ticks x prescale) / clock = usec/tick
 *
 * @return  Status - OK or Not OK
 ***************************************************************************************************/
uint8 HalTimerStart (uint8 timerId, uint32 timePerTick)
{
  //Stub for now, only PWM implemented
  return 0;
}

/***************************************************************************************************
 * @fn      HalTimerTick
 *
 * @brief   Check the counter for expired counter.
 *
 * @param   None
 *
 * @return  None
 ***************************************************************************************************/
void HalTimerTick (void)
{
  //Stub for now, only PWM implemented
}

/***************************************************************************************************
 * @fn      HalTimerStop
 *
 * @brief   Stop the Timer Service
 *
 * @param   timerId - ID of the timer
 *
 * @return  Status - OK or Not OK
 ***************************************************************************************************/
uint8 HalTimerStop (uint8 timerId)
{
  //Stub for now, only PWM implemented
  return 0;
}

/***************************************************************************************************
 * @fn      halTimerSetCount
 *
 * @brief   Stop the Timer Service
 *
 * @param   hwtimerid - ID of the timer
 *          timerPerTick - Number micro sec per ticks
 *
 * @return  Status - OK or Not OK
 ***************************************************************************************************/
uint8 halTimerSetCount (uint8 hwtimerid, uint32 timePerTick)
{
  //Stub for now, only PWM implemented
  return 0;
}

/***************************************************************************************************
 * @fn      HalTimerInterruptEnable
 *
 * @brief   Setup operate modes
 *
 * @param   hwtimerid - ID of the timer
 *          channelMode - channel mode
 *          enable - TRUE or FALSE
 *
 * @return  Status - OK or Not OK
 ***************************************************************************************************/
uint8 HalTimerInterruptEnable (uint8 hwtimerid, uint8 channelMode, bool enable)
{
  //Stub for now, only PWM implemented
  return 0;
}

/***************************************************************************************************
***************************************************************************************************/
