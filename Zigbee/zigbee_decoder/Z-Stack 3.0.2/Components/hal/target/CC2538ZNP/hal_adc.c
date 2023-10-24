/**************************************************************************************************
  Filename:       hal_adc.c
  Revised:        $Date: 2013-05-20 10:14:45 -0700 (Mon, 20 May 2013) $
  Revision:       $Revision: 34373 $

  Description:    This file contains the interface to the HAL ADC.


  Copyright 2013 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License").  You may not use this
  Software unless you agree to abide by the terms of the License. The License
  limits your use, and you acknowledge, that the Software may not be modified,
  copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio
  frequency transceiver, which is integrated into your product.  Other than for
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

/**************************************************************************************************
 *                                           INCLUDES
 **************************************************************************************************/

#include  "hal_adc.h"
#include  "hal_defs.h"
#include  "hal_mcu.h"
#include  "hal_types.h"
#include <math.h>

/**************************************************************************************************
 *                                            CONSTANTS
 **************************************************************************************************/
#define HAL_ADC_EOC         0x80    /* End of Conversion bit */
#define HAL_ADC_START       0x40    /* Starts Conversion */

#define HAL_ADC_STSEL_EXT   0x00    /* External Trigger */
#define HAL_ADC_STSEL_FULL  0x10    /* Full Speed, No Trigger */
#define HAL_ADC_STSEL_T1C0  0x20    /* Timer1, Channel 0 Compare Event Trigger */
#define HAL_ADC_STSEL_ST    0x30    /* ADCCON1.ST =1 Trigger */

#define HAL_ADC_RAND_NORM   0x00    /* Normal Operation */
#define HAL_ADC_RAND_LFSR   0x04    /* Clock LFSR */
#define HAL_ADC_RAND_SEED   0x08    /* Seed Modulator */
#define HAL_ADC_RAND_STOP   0x0c    /* Stop Random Generator */
#define HAL_ADC_RAND_BITS   0x0c    /* Bits [3:2] */

#define HAL_ADC_DEC_064     0x00    /* Decimate by 64 : 8-bit resolution */
#define HAL_ADC_DEC_128     0x10    /* Decimate by 128 : 10-bit resolution */
#define HAL_ADC_DEC_256     0x20    /* Decimate by 256 : 12-bit resolution */
#define HAL_ADC_DEC_512     0x30    /* Decimate by 512 : 14-bit resolution */
#define HAL_ADC_DEC_BITS    0x30    /* Bits [5:4] */

#define HAL_ADC_STSEL       HAL_ADC_STSEL_ST
#define HAL_ADC_RAND_GEN    HAL_ADC_RAND_STOP
#define HAL_ADC_REF_VOLT    HAL_ADC_REF_AVDD
#define HAL_ADC_DEC_RATE    HAL_ADC_DEC_064
#define HAL_ADC_SCHN        HAL_ADC_CHN_VDD3
#define HAL_ADC_ECHN        HAL_ADC_CHN_GND

#ifdef ADC_CAL
#define INFO_CAL_H              (*((uint8 *)0x7826)) //calibration values stored here
#define INFO_CAL_L              (*((uint8 *)0x7827))
#endif //ADC_CAL

#define HAL_BITS_CHN_A0A1   0x03    /* Bit mask for AIN0,AIN1 */
#define HAL_BITS_CHN_A2A3   0x0c    /* Bit mask for AIN2,AIN3 */
#define HAL_BITS_CHN_A4A5   0x30    /* Bit mask for AIN4,AIN5 */
#define HAL_BITS_CHN_A6A7   0xc0    /* Bit mask for AIN6,AIN7 */

/* ------------------------------------------------------------------------------------------------
 *                                       Local Variables
 * ------------------------------------------------------------------------------------------------
 */

#if (HAL_ADC == TRUE)
static uint8 adcRef;
#endif

#ifdef ADC_CAL
uint32 IDEAL_VDD3_CODE = 1780; // this is ideal value for Vdd/3 at 3.0V input and 1.15V V_ref at 12 bit resolution
uint16 vdd3InfoPg;
int16 compensation;
#endif //ADC_CAL

/***************************************************************************************************
 *                                            LOCAL FUNCTION
 ***************************************************************************************************/
#ifdef ADC_CAL
static uint16 HalAdcCompensate ( uint16 rawAdcVal, uint8 resolution );
#endif //ADC_CAL

/**************************************************************************************************
 * @fn      HalAdcInit
 *
 * @brief   Initialize ADC Service
 *
 * @param   None
 *
 * @return  None
 **************************************************************************************************/
void HalAdcInit (void)
{
#if (HAL_ADC == TRUE)
  adcRef = HAL_ADC_REF_VOLT;
#ifdef ADC_CAL
  {
    vdd3InfoPg = ((INFO_CAL_H << 8) | INFO_CAL_L) >> 4;
    compensation = IDEAL_VDD3_CODE - vdd3InfoPg;
   }
#endif //ADC_CAL
#endif
}

#ifdef ADC_CAL
/**************************************************************************************************
 * @fn      HalAdcRead
 *
 * @brief   Compensate a raw ADC value
 *
 * @param   rawAdcVal - channel where ADC will be read
 *
 * @param   voltValue - Pointer that will be set to the converted voltage value
*                      Note: Pass NULL if convertion to voltage is not needed
 *
 * @param   resolution - Resolution of the rawAdcVal, only needed if converting
 *                       to voltageValue
 *
 * @return  16 bit value of the Compensated ADC value.
 *
 *          
 **************************************************************************************************/
uint16 HalAdcCompensate ( uint16 rawAdcVal, uint8 resolution )
{
{
  uint16 compAdcVal;
  int16 comp ;
  
  switch (resolution)
  {
    case HAL_ADC_RESOLUTION_8:
      comp = compensation >> 4;
      break;
    case HAL_ADC_RESOLUTION_10:
      comp = compensation >> 2;
      break;
    case HAL_ADC_RESOLUTION_14:    
      comp = compensation << 2;
      break;
    case HAL_ADC_RESOLUTION_12:
    default:
      break;
  }
  compAdcVal = rawAdcVal + comp;

  return compAdcVal;
}
#endif //ADC_CAL

/**************************************************************************************************
 * @fn      HalAdcRead
 *
 * @brief   Read the ADC based on given channel and resolution
 *
 * @param   channel - channel where ADC will be read
 * @param   resolution - the resolution of the value
 *
 * @return  16 bit value of the ADC in offset binary format.
 *
 *          Note that the ADC is "bipolar", which means the GND (0V) level is mid-scale.
 *          Note2: This function assumes that ADCCON3 contains the voltage reference.
 **************************************************************************************************/
uint16 HalAdcRead (uint8 channel, uint8 resolution)
{
  int16  reading = 0;

#if (HAL_ADC == TRUE)
  uint8   i, resbits;
  uint8   adcChannel = 1;
  uint32  padConfig, dirConfig;
  halIntState_t s;
  
  /*
   * If Analog input channel is AIN0..AIN7, make sure corresponing PA pin is 
   * setup. Only port A can be used as input to the ADC. If any pin on port A 
   * is to be used as an ADC input, the appropriate register, IOC_PAx_OVER, 
   * must be set to analog (that is, bit 0 must be set to 1). 
   */
  
  /* Hold off interrupts */
  HAL_ENTER_CRITICAL_SECTION(s);
      
  switch (channel)
  {
  case HAL_ADC_CHN_AIN0:
  case HAL_ADC_CHN_AIN1:
  case HAL_ADC_CHN_AIN2:
  case HAL_ADC_CHN_AIN3:
  case HAL_ADC_CHN_AIN4:
  case HAL_ADC_CHN_AIN5:
  case HAL_ADC_CHN_AIN6:
  case HAL_ADC_CHN_AIN7:
    adcChannel <<= channel;
  break;
  case HAL_ADC_CHN_A0A1:
    adcChannel = HAL_BITS_CHN_A0A1;
    break;
  
  case HAL_ADC_CHN_A2A3:
    adcChannel = HAL_BITS_CHN_A2A3;
    break;
  case HAL_ADC_CHN_A4A5:
    adcChannel = HAL_BITS_CHN_A4A5;
    break;
  case HAL_ADC_CHN_A6A7:
    adcChannel = HAL_BITS_CHN_A6A7;
    break; 
  default:
    adcChannel = 0;
    break;
  } 
  
  /* save the current pad setting of the PortA pin */
  padConfig = IOCPadConfigGet(GPIO_A_BASE, adcChannel);
  
  /* save the current gpio setting of the PortA pin */
  dirConfig = GPIODirModeGet(GPIO_A_BASE, adcChannel);
  
  /* set the PortA pin to Analog */
  IOCPadConfigSet(GPIO_A_BASE, adcChannel, IOC_OVERRIDE_ANA);
  
  /* set the PortA pin direction to input */
  GPIODirModeSet(GPIO_A_BASE, adcChannel, GPIO_DIR_MODE_IN);

  /* Convert resolution to decimation rate */
  switch (resolution)
  {
    case HAL_ADC_RESOLUTION_8:
      resbits = HAL_ADC_DEC_064;
      break;
    case HAL_ADC_RESOLUTION_10:
      resbits = HAL_ADC_DEC_128;
      break;
    case HAL_ADC_RESOLUTION_12:
      resbits = HAL_ADC_DEC_256;
      break;
    case HAL_ADC_RESOLUTION_14:
    default:
      resbits = HAL_ADC_DEC_512;
      break;
  }

  /* writing to this register starts the extra conversion */
  ADCCON3 = channel | resbits | adcRef;

  /* Wait for the conversion to be done */
  while (!(ADCCON1 & HAL_ADC_EOC));
  
  /* Set the pad configuration to previous value*/
  IOCPadConfigSet(GPIO_A_BASE, adcChannel, padConfig);
 
  /* Set the GPIO direction to previous value*/
  GPIODirModeSet(GPIO_A_BASE, adcChannel, dirConfig);
  
  /* Read the result */
  reading = (int16) (ADCL);
  reading |= (int16) (ADCH << 8);
  
  /* Enable interrupts */
  HAL_EXIT_CRITICAL_SECTION(s);

  /* Treat small negative as 0 */
  if (reading < 0)
    reading = 0;

  switch (resolution)
  {
    case HAL_ADC_RESOLUTION_8:
      reading >>= 8;
      break;
    case HAL_ADC_RESOLUTION_10:
      reading >>= 6;
      break;
    case HAL_ADC_RESOLUTION_12:
      reading >>= 4;
      break;
    case HAL_ADC_RESOLUTION_14:
    default:
      reading >>= 2;
    break;
  }
#else
  /* unused arguments */
  (void) channel;
  (void) resolution;
#endif

  return ((uint16)reading);
}

/**************************************************************************************************
 * @fn      HalAdcSetReference
 *
 * @brief   Sets the reference voltage for the ADC and initializes the service
 *
 * @param   reference - the reference voltage to be used by the ADC
 *
 * @return  none
 *
 **************************************************************************************************/
void HalAdcSetReference ( uint8 reference )
{
#if (HAL_ADC == TRUE)
  adcRef = reference;
#endif
}

/*********************************************************************
 * @fn      HalAdcCheckVdd
 *
 * @brief   Check for minimum Vdd specified.
 *
 * @param   vdd - The board-specific Vdd reading to check for.
 *
 * @return  TRUE if the Vdd measured is greater than the 'vdd' minimum parameter;
 *          FALSE if not.
 *
 *********************************************************************/
bool HalAdcCheckVdd(uint8 vdd)
{
#ifdef ADC_CAL
  uint16 compAdcVal;  
#endif //ADC_CAL
  
  ADCCON3 = 0x0F;
  while (!(ADCCON1 & 0x80));
  
#ifdef ADC_CAL
  compAdcVal = HalAdcCompensate (ADCH, HAL_ADC_RESOLUTION_8);
  return (compAdcVal > vdd);
#else
  return (ADCH > vdd);
#endif //ADC_CAL
}

/*********************************************************************
 * @fn      HalAdcCheckVddRaw
 *
 * @brief   Check Vdd.
 *
 * @param   none.
 *
 * @return  value measured
 *
 *********************************************************************/
uint8 HalAdcCheckVddRaw( void )
{
#ifdef ADC_CAL
  uint16 compAdcVal;  
#endif //ADC_CAL
  
  ADCCON3 = 0x0F;
  while (!(ADCCON1 & 0x80));
  
#ifdef ADC_CAL
  compAdcVal = HalAdcCompensate (ADCH, HAL_ADC_RESOLUTION_8);
  return compAdcVal;
#else
  return ADCH;
#endif
}

/**************************************************************************************************
**************************************************************************************************/
