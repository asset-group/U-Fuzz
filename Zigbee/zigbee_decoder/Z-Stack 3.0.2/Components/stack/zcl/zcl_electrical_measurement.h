/**************************************************************************************************
  Filename:       zcl_electrical_measurement.h
  Revised:        $Date: 2013-10-16 16:38:58 -0700 (Wed, 16 Oct 2013) $
  Revision:       $Revision: 35701 $

  Description:    This file contains the ZCL Electrical Measurement definitions.


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
  PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
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

#ifndef ZCL_ELECTRICAL_MEASUREMENT_H
#define ZCL_ELECTRICAL_MEASUREMENT_H

#ifdef ZCL_ELECTRICAL_MEASUREMENT

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************************************
 * INCLUDES
 */
#include "zcl.h"


/******************************************************************************
 * CONSTANTS
 */

/****************************************************/
/***  Electrical Measurements Cluster Attributes ***/
/***************************************************/

// Server Attributes
#define ATTRID_ELECTRICAL_MEASUREMENT_MEASUREMENT_TYPE                            0x0000  // M, R, BITMAP 32
#define ATTRID_ELECTRICAL_MEASUREMENT_DC_VOLTAGE                                  0x0100  // O, R, INT16
#define ATTRID_ELECTRICAL_MEASUREMENT_DC_VOLTAGE_MIN                              0x0101  // O, R, INT16
#define ATTRID_ELECTRICAL_MEASUREMENT_DC_VOLTAGE_MAX                              0x0102  // O, R, INT16
#define ATTRID_ELECTRICAL_MEASUREMENT_DC_CURRENT                                  0x0103  // O, R, INT16
#define ATTRID_ELECTRICAL_MEASUREMENT_DC_CURRENT_MIN                              0x0104  // O, R, INT16
#define ATTRID_ELECTRICAL_MEASUREMENT_DC_CURRENT_MAX                              0x0105  // O, R, INT16
#define ATTRID_ELECTRICAL_MEASUREMENT_DC_POWER                                    0x0106  // O, R, INT16
#define ATTRID_ELECTRICAL_MEASUREMENT_DC_POWER_MIN                                0x0107  // O, R, INT16
#define ATTRID_ELECTRICAL_MEASUREMENT_DC_POWER_MAX                                0x0108  // O, R, INT16
#define ATTRID_ELECTRICAL_MEASUREMENT_DC_VOLTAGE_MULTIPLIER                       0x0200  // O, R, UINT16
#define ATTRID_ELECTRICAL_MEASUREMENT_DC_VOLTAGE_DIVISOR                          0x0201  // O, R, UINT16
#define ATTRID_ELECTRICAL_MEASUREMENT_DC_CURRENT_MULTIPLIER                       0x0202  // O, R, UINT16
#define ATTRID_ELECTRICAL_MEASUREMENT_DC_CURRENT_DIVISOR                          0x0203  // O, R, UINT16
#define ATTRID_ELECTRICAL_MEASUREMENT_DC_POWER_MULTIPLIER                         0x0204  // O, R, UINT16
#define ATTRID_ELECTRICAL_MEASUREMENT_DC_POWER_DIVISOR                            0x0205  // O, R, UINT16
#define ATTRID_ELECTRICAL_MEASUREMENT_AC_FREQUENCY                                0x0300  // O, R, UINT16
#define ATTRID_ELECTRICAL_MEASUREMENT_AC_FREQUENCY_MIN                            0x0301  // O, R, UINT16
#define ATTRID_ELECTRICAL_MEASUREMENT_AC_FREQUENCY_MAX                            0x0302  // O, R, UINT16
#define ATTRID_ELECTRICAL_MEASUREMENT_NEUTRAL_CURRENT                             0x0303  // O, R, UINT16
#define ATTRID_ELECTRICAL_MEASUREMENT_TOTAL_ACTIVE_POWER                          0x0304  // O, R, INT32
#define ATTRID_ELECTRICAL_MEASUREMENT_TOTAL_REACTIVE_POWER                        0x0305  // O, R, INT32
#define ATTRID_ELECTRICAL_MEASUREMENT_TOTAL_APPARENT_POWER                        0x0306  // O, R, UINT32
#define ATTRID_ELECTRICAL_MEASUREMENT_MEASURED_1ST_HARMONIC_CURRENT               0x0307  // O, R, INT16
#define ATTRID_ELECTRICAL_MEASUREMENT_MEASURED_3RD_HARMONIC_CURRENT               0x0308  // O, R, INT16
#define ATTRID_ELECTRICAL_MEASUREMENT_MEASURED_5TH_HARMONIC_CURRENT               0x0309  // O, R, INT16
#define ATTRID_ELECTRICAL_MEASUREMENT_MEASURED_7TH_HARMONIC_CURRENT               0x030A  // O, R, INT16
#define ATTRID_ELECTRICAL_MEASUREMENT_MEASURED_9TH_HARMONIC_CURRENT               0x030B  // O, R, INT16
#define ATTRID_ELECTRICAL_MEASUREMENT_MEASURED_11TH_HARMONIC_CURRENT              0x030C  // O, R, INT16
#define ATTRID_ELECTRICAL_MEASUREMENT_MEASURED_PHASE_1ST_HARMONIC_CURRENT         0x030D  // O, R, INT16
#define ATTRID_ELECTRICAL_MEASUREMENT_MEASURED_PHASE_3RD_HARMONIC_CURRENT         0x030E  // O, R, INT16
#define ATTRID_ELECTRICAL_MEASUREMENT_MEASURED_PHASE_5TH_HARMONIC_CURRENT         0x030F  // O, R, INT16
#define ATTRID_ELECTRICAL_MEASUREMENT_MEASURED_PHASE_7TH_HARMONIC_CURRENT         0x0310  // O, R, INT16
#define ATTRID_ELECTRICAL_MEASUREMENT_MEASURED_PHASE_9TH_HARMONIC_CURRENT         0x0311  // O, R, INT16
#define ATTRID_ELECTRICAL_MEASUREMENT_MEASURED_PHASE_11TH_HARMONIC_CURRENT        0x0312  // O, R, INT16
#define ATTRID_ELECTRICAL_MEASUREMENT_AC_FREQUENCY_MULTIPLIER                     0x0400  // O, R, UINT16
#define ATTRID_ELECTRICAL_MEASUREMENT_AC_FREQUENCY_DIVISOR                        0x0401  // O, R, UINT16
#define ATTRID_ELECTRICAL_MEASUREMENT_POWER_MULTIPLIER                            0x0402  // O, R, UINT32
#define ATTRID_ELECTRICAL_MEASUREMENT_POWER_DIVISOR                               0x0403  // O, R, UINT32
#define ATTRID_ELECTRICAL_MEASUREMENT_HARMONIC_CURRENT_MULTIPLIER                 0x0404  // O, R, INT8
#define ATTRID_ELECTRICAL_MEASUREMENT_PHASE_HARMONIC_CURRENT_MULTIPLIER           0x0405  // O, R, INT8
#define ATTRID_ELECTRICAL_MEASUREMENT_INSTANTANEOUS_VOLTAGE                       0x0500  // O, R, INT16
#define ATTRID_ELECTRICAL_MEASUREMENT_INSTANTANEOUS_LINE_CURRENT                  0x0501  // O, R, UINT16
#define ATTRID_ELECTRICAL_MEASUREMENT_INSTANTANEOUS_ACTIVE_CURRENT                0x0502  // O, R, INT16
#define ATTRID_ELECTRICAL_MEASUREMENT_INSTANTANEOUS_REACTIVE_CURRENT              0x0503  // O, R, INT16
#define ATTRID_ELECTRICAL_MEASUREMENT_INSTANTANEOUS_POWER                         0x0504  // O, R, INT16
#define ATTRID_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE                                 0x0505  // O, R, UINT16
#define ATTRID_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_MIN                             0x0506  // O, R, UINT16
#define ATTRID_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_MAX                             0x0507  // O, R, UINT16
#define ATTRID_ELECTRICAL_MEASUREMENT_RMS_CURRENT                                 0x0508  // O, R, UINT16
#define ATTRID_ELECTRICAL_MEASUREMENT_RMS_CURRENT_MIN                             0x0509  // O, R, UINT16
#define ATTRID_ELECTRICAL_MEASUREMENT_RMS_CURRENT_MAX                             0x050A  // O, R, UINT16
#define ATTRID_ELECTRICAL_MEASUREMENT_ACTIVE_POWER                                0x050B  // O, R, INT16
#define ATTRID_ELECTRICAL_MEASUREMENT_ACTIVE_POWER_MIN                            0x050C  // O, R, INT16
#define ATTRID_ELECTRICAL_MEASUREMENT_ACTIVE_POWER_MAX                            0x050D  // O, R, INT16
#define ATTRID_ELECTRICAL_MEASUREMENT_REACTIVE_POWER                              0x050E  // O, R, INT16
#define ATTRID_ELECTRICAL_MEASUREMENT_APPARENT_POWER                              0x050F  // O, R, UINT16
#define ATTRID_ELECTRICAL_MEASUREMENT_POWER_FACTOR                                0x0510  // O, R, INT8
#define ATTRID_ELECTRICAL_MEASUREMENT_AVERAGE_RMS_VOLTAGE_MEASUREMENT_PERIOD      0x0511  // O, R/W, UINT16
#define ATTRID_ELECTRICAL_MEASUREMENT_AVERAGE_RMS_OVER_VOLTAGE_COUNTER            0x0512  // O, R/W, UINT16
#define ATTRID_ELECTRICAL_MEASUREMENT_AVERAGE_RMS_UNDER_VOLTAGE_COUNTER           0x0513  // O, R/W, UINT16
#define ATTRID_ELECTRICAL_MEASUREMENT_RMS_EXTREME_OVER_VOLTAGE_PERIOD             0x0514  // O, R/W, UINT16
#define ATTRID_ELECTRICAL_MEASUREMENT_RMS_EXTREME_UNDER_VOLTAGE_PERIOD            0x0515  // O, R/W, UINT16
#define ATTRID_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_SAG_PERIOD                      0x0516  // O, R/W, UINT16
#define ATTRID_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_SWELL_PERIOD                    0x0517  // O, R/W, UINT16
#define ATTRID_ELECTRICAL_MEASUREMENT_AC_VOLTAGE_MULTIPLIER                       0x0600  // O, R, UINT16
#define ATTRID_ELECTRICAL_MEASUREMENT_AC_VOLTAGE_DIVISOR                          0x0601  // O, R, UINT16
#define ATTRID_ELECTRICAL_MEASUREMENT_AC_CURRENT_MULTIPLIER                       0x0602  // O, R, UINT16
#define ATTRID_ELECTRICAL_MEASUREMENT_AC_CURRENT_DIVISOR                          0x0603  // O, R, UINT16
#define ATTRID_ELECTRICAL_MEASUREMENT_AC_POWER_MULTIPLIER                         0x0604  // O, R, UINT16
#define ATTRID_ELECTRICAL_MEASUREMENT_AC_POWER_DIVISOR                            0x0605  // O, R, UINT16
#define ATTRID_ELECTRICAL_MEASUREMENT_DC_OVERLOAD_ALARMS_MASK                     0x0700  // O, R/W, BITMAP8
#define ATTRID_ELECTRICAL_MEASUREMENT_DC_VOLTAGE_OVERLOAD                         0x0701  // O, R, INT16
#define ATTRID_ELECTRICAL_MEASUREMENT_DC_CURRENT_OVERLOAD                         0x0702  // O, R, INT16
#define ATTRID_ELECTRICAL_MEASUREMENT_AC_ALARMS_MASK                              0x0800  // O, R/W, BITMAP16
#define ATTRID_ELECTRICAL_MEASUREMENT_AC_VOLTAGE_OVERLOAD                         0x0801  // O, R, INT16
#define ATTRID_ELECTRICAL_MEASUREMENT_AC_CURRENT_OVERLOAD                         0x0802  // O, R, INT16
#define ATTRID_ELECTRICAL_MEASUREMENT_AC_ACTIVE_POWER_OVERLOAD                    0x0803  // O, R, INT16
#define ATTRID_ELECTRICAL_MEASUREMENT_AC_REACTIVE_POWER_OVERLOAD                  0x0804  // O, R, INT16
#define ATTRID_ELECTRICAL_MEASUREMENT_AVERAGE_RMS_OVER_VOLTAGE                    0x0805  // O, R, INT16
#define ATTRID_ELECTRICAL_MEASUREMENT_AVERAGE_RMS_UNDER_VOLTAGE                   0x0806  // O, R, INT16
#define ATTRID_ELECTRICAL_MEASUREMENT_RMS_EXTREME_OVER_VOLTAGE                    0x0807  // O, R/W, INT16
#define ATTRID_ELECTRICAL_MEASUREMENT_RMS_EXTREME_UNDER_VOLTAGE                   0x0808  // O, R/W, INT16
#define ATTRID_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_SAG                             0x0809  // O, R/W, INT16
#define ATTRID_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_SWELL                           0x080A  // O, R/W, INT16
#define ATTRID_ELECTRICAL_MEASUREMENT_LINE_CURRENT_PH_B                           0x0901  // O, R, UINT16
#define ATTRID_ELECTRICAL_MEASUREMENT_ACTIVE_CURRENT_PH_B                         0x0902  // O, R, INT16
#define ATTRID_ELECTRICAL_MEASUREMENT_REACTIVE_CURRENT_PH_B                       0x0903  // O, R, INT16
#define ATTRID_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_PH_B                            0x0905  // O, R, UINT16
#define ATTRID_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_MIN_PH_B                        0x0906  // O, R, UINT16
#define ATTRID_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_MAX_PH_B                        0x0907  // O, R, UINT16
#define ATTRID_ELECTRICAL_MEASUREMENT_RMS_CURRENT_PH_B                            0x0908  // O, R, UINT16
#define ATTRID_ELECTRICAL_MEASUREMENT_RMS_CURRENT_MIN_PH_B                        0x0909  // O, R, UINT16
#define ATTRID_ELECTRICAL_MEASUREMENT_RMS_CURRENT_MAX_PH_B                        0x090A  // O, R, UINT16
#define ATTRID_ELECTRICAL_MEASUREMENT_ACTIVE_POWER_PH_B                           0x090B  // O, R, INT16
#define ATTRID_ELECTRICAL_MEASUREMENT_ACTIVE_POWER_MIN_PH_B                       0x090C  // O, R, INT16
#define ATTRID_ELECTRICAL_MEASUREMENT_ACTIVE_POWER_MAX_PH_B                       0x090D  // O, R, INT16
#define ATTRID_ELECTRICAL_MEASUREMENT_REACTIVE_POWER_PH_B                         0x090E  // O, R, INT16
#define ATTRID_ELECTRICAL_MEASUREMENT_APPARENT_POWER_PH_B                         0x090F  // O, R, UINT16
#define ATTRID_ELECTRICAL_MEASUREMENT_POWER_FACTOR_PH_B                           0x0910  // O, R, INT8
#define ATTRID_ELECTRICAL_MEASUREMENT_AVERAGE_RMS_VOLTAGE_MEASURE_PERIOD_PH_B     0x0911  // O, R/W, UINT16
#define ATTRID_ELECTRICAL_MEASUREMENT_AVERAGE_RMS_OVER_VOLTAGE_COUNTER_PH_B       0x0912  // O, R/W, UINT16
#define ATTRID_ELECTRICAL_MEASUREMENT_AVERAGE_UNDER_VOLTAGE_COUNTER_PH_B          0x0913  // O, R/W, UINT16
#define ATTRID_ELECTRICAL_MEASUREMENT_RMS_EXTREME_OVER_VOLTAGE_PERIOD_PH_B        0x0914  // O, R/W, UINT16
#define ATTRID_ELECTRICAL_MEASUREMENT_RMS_EXTREME_UNDER_VOLTAGE_PERIOD_PH_B       0x0915  // O, R/W, UINT16
#define ATTRID_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_SAG_PERIOD_PH_B                 0x0916  // O, R/W, UINT16
#define ATTRID_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_SWELL_PERIOD_PH_B               0x0917  // O, R/W, UINT16
#define ATTRID_ELECTRICAL_MEASUREMENT_LINE_CURRENT_PH_C                           0x0A01  // O, R, UINT16
#define ATTRID_ELECTRICAL_MEASUREMENT_ACTIVE_CURRENT_PH_C                         0x0A02  // O, R, INT16
#define ATTRID_ELECTRICAL_MEASUREMENT_REACTIVE_CURRENT_PH_C                       0x0A03  // O, R, INT16
#define ATTRID_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_PH_C                            0x0A05  // O, R, UINT16
#define ATTRID_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_MIN_PH_C                        0x0A06  // O, R, UINT16
#define ATTRID_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_MAX_PH_C                        0x0A07  // O, R, UINT16
#define ATTRID_ELECTRICAL_MEASUREMENT_RMS_CURRENT_PH_C                            0x0A08  // O, R, UINT16
#define ATTRID_ELECTRICAL_MEASUREMENT_RMS_CURRENT_MIN_PH_C                        0x0A09  // O, R, UINT16
#define ATTRID_ELECTRICAL_MEASUREMENT_RMS_CURRENT_MAX_PH_C                        0x0A0A  // O, R, UINT16
#define ATTRID_ELECTRICAL_MEASUREMENT_ACTIVE_POWER_PH_C                           0x0A0B  // O, R, INT16
#define ATTRID_ELECTRICAL_MEASUREMENT_ACTIVE_POWER_MIN_PH_C                       0x0A0C  // O, R, INT16
#define ATTRID_ELECTRICAL_MEASUREMENT_ACTIVE_POWER_MAX_PH_C                       0x0A0D  // O, R, INT16
#define ATTRID_ELECTRICAL_MEASUREMENT_REACTIVE_POWER_PH_C                         0x0A0E  // O, R, INT16
#define ATTRID_ELECTRICAL_MEASUREMENT_APPARENT_POWER_PH_C                         0x0A0F  // O, R, UINT16
#define ATTRID_ELECTRICAL_MEASUREMENT_POWER_FACTOR_PH_C                           0x0A10  // O, R, INT8
#define ATTRID_ELECTRICAL_MEASUREMENT_AVERAGE_RMS_VOLTAGE_MEASURE_PERIOD_PH_C     0x0A11  // O, R/W, UINT16
#define ATTRID_ELECTRICAL_MEASUREMENT_AVERAGE_RMS_OVER_VOLTAGE_COUNTER_PH_C       0x0A12  // O, R/W, UINT16
#define ATTRID_ELECTRICAL_MEASUREMENT_AVERAGE_UNDER_VOLTAGE_COUNTER_PH_C          0x0A13  // O, R/W, UINT16
#define ATTRID_ELECTRICAL_MEASUREMENT_RMS_EXTREME_OVER_VOLTAGE_PERIOD_PH_C        0x0A14  // O, R/W, UINT16
#define ATTRID_ELECTRICAL_MEASUREMENT_RMS_EXTREME_UNDER_VOLTAGE_PERIOD_PH_C       0x0A15  // O, R/W, UINT16
#define ATTRID_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_SAG_PERIOD_PH_C                 0x0A16  // O, R/W, UINT16
#define ATTRID_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_SWELL_PERIOD_PH_C               0x0A17  // O, R/W, UINT16


// Server Attribute Defaults
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_MEASUREMENT_TYPE                          0
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_DC_VOLTAGE                                0x8000
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_DC_VOLTAGE_MIN                            0x8000
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_DC_VOLTAGE_MAX                            0x8000
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_DC_CURRENT                                0x8000
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_DC_CURRENT_MIN                            0x8000
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_DC_CURRENT_MAX                            0x8000
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_DC_POWER                                  0x8000
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_DC_POWER_MIN                              0x8000
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_DC_POWER_MAX                              0x8000
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_DC_VOLTAGE_MULTIPLIER                     0x0001
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_DC_VOLTAGE_DIVISOR                        0x0001
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_DC_CURRENT_MULTIPLIER                     0x0001
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_DC_CURRENT_DIVISOR                        0x0001
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_DC_POWER_MULTIPLIER                       0x0001
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_DC_POWER_DIVISOR                          0x0001
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_AC_FREQUENCY                              0xFFFF
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_AC_FREQUENCY_MIN                          0xFFFF
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_AC_FREQUENCY_MAX                          0xFFFF
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_NEUTRAL_CURRENT                           0xFFFF
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_TOTAL_ACTIVE_POWER                        0
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_TOTAL_REACTIVE_POWER                      0
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_TOTAL_APPARENT_POWER                      0x000001
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_MEASURED_1ST_HARMONIC_CURRENT             0x8000
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_MEASURED_3RD_HARMONIC_CURRENT             0x8000
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_MEASURED_5TH_HARMONIC_CURRENT             0x8000
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_MEASURED_7TH_HARMONIC_CURRENT             0x8000
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_MEASURED_9TH_HARMONIC_CURRENT             0x8000
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_MEASURED_11TH_HARMONIC_CURRENT            0x8000
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_MEASURED_PHASE_1ST_HARMONIC_CURRENT       0x8000
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_MEASURED_PHASE_3RD_HARMONIC_CURRENT       0x8000
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_MEASURED_PHASE_5TH_HARMONIC_CURRENT       0x8000
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_MEASURED_PHASE_7TH_HARMONIC_CURRENT       0x8000
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_MEASURED_PHASE_9TH_HARMONIC_CURRENT       0x8000
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_MEASURED_PHASE_11TH_HARMONIC_CURRENT      0x8000
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_AC_FREQUENCY_MULTIPLIER                   0x0001
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_AC_FREQUENCY_DIVISOR                      0x0001
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_POWER_MULTIPLIER                          0x00000001
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_POWER_DIVISOR                             0x00000001
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_HARMONIC_CURRENT_MULTIPLIER               0x00
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_PHASE_HARMONIC_CURRENT_MULTIPLIER         0x00
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_INSTANTANEOUS_VOLTAGE                     0xFFFF
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_INSTANTANEOUS_LINE_CURRENT                0xFFFF
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_INSTANTANEOUS_ACTIVE_CURRENT              0xFFFF
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_INSTANTANEOUS_REACTIVE_CURRENT            0xFFFF
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_INSTANTANEOUS_POWER                       0xFFFF
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE                               0xFFFF
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_MIN                           0x8000
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_MAX                           0x8000
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_RMS_CURRENT                               0xFFFF
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_RMS_CURRENT_MIN                           0xFFFF
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_RMS_CURRENT_MAX                           0xFFFF
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_ACTIVE_POWER                              0xFFFF
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_ACTIVE_POWER_MIN                          0xFFFF
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_ACTIVE_POWER_MAX                          0xFFFF
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_REACTIVE_POWER                            0xFFFF
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_APPARENT_POWER                            0xFFFF
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_POWER_FACTOR                              0x00
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_AVERAGE_RMS_VOLTAGE_MEASUREMENT_PERIOD    0x0000
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_AVERAGE_RMS_OVER_VOLTAGE_COUNTER          0x0000
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_AVERAGE_RMS_UNDER_VOLTAGE_COUNTER         0x0000
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_RMS_EXTREME_OVER_VOLTAGE_PERIOD           0x0000
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_RMS_EXTREME_UNDER_VOLTAGE_PERIOD          0x0000
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_SAG_PERIOD                    0x0000
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_SWELL_PERIOD                  0x0000
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_AC_VOLTAGE_MULTIPLIER                     0x0001
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_AC_VOLTAGE_DIVISOR                        0x0001
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_AC_CURRENT_MULTIPLIER                     0x0001
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_AC_CURRENT_DIVISOR                        0x0001
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_AC_POWER_MULTIPLIER                       0x0001
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_AC_POWER_DIVISOR                          0x0001
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_DC_OVERLOAD_ALARMS_MASK                   0
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_DC_VOLTAGE_OVERLOAD                       0xFFFF
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_DC_CURRENT_OVERLOAD                       0xFFFF
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_AC_ALARMS_MASK                            0
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_AC_VOLTAGE_OVERLOAD                       0xFFFF
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_AC_CURRENT_OVERLOAD                       0xFFFF
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_AC_ACTIVE_POWER_OVERLOAD                  0xFFFF
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_AC_REACTIVE_POWER_OVERLOAD                0xFFFF
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_AVERAGE_RMS_OVER_VOLTAGE                  0
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_AVERAGE_RMS_UNDER_VOLTAGE                 0
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_RMS_EXTREME_OVER_VOLTAGE                  0
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_RMS_EXTREME_UNDER_VOLTAGE                 0
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_SAG                           0
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_SWELL                         0
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_LINE_CURRENT_PH_B                         0xFFFF
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_ACTIVE_CURRENT_PH_B                       0xFFFF
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_REACTIVE_CURRENT_PH_B                     0xFFFF
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_PH_B                          0xFFFF
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_MIN_PH_B                      0x8000
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_MAX_PH_B                      0x8000
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_RMS_CURRENT_PH_B                          0xFFFF
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_RMS_CURRENT_MIN_PH_B                      0xFFFF
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_RMS_CURRENT_MAX_PH_B                      0xFFFF
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_ACTIVE_POWER_PH_B                         0xFFFF
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_ACTIVE_POWER_MIN_PH_B                     0xFFFF
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_ACTIVE_POWER_MAX_PH_B                     0xFFFF
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_REACTIVE_POWER_PH_B                       0xFFFF
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_APPARENT_POWER_PH_B                       0xFFFF
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_POWER_FACTOR_PH_B                         0
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_AVERAGE_RMS_VOLTAGE_MEASURE_PERIOD_PH_B   0
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_AVERAGE_RMS_OVER_VOLTAGE_COUNTER_PH_B     0
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_AVERAGE_UNDER_VOLTAGE_COUNTER_PH_B        0
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_RMS_EXTREME_OVER_VOLTAGE_PERIOD_PH_B      0
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_RMS_EXTREME_UNDER_VOLTAGE_PERIOD_PH_B     0
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_SAG_PERIOD_PH_B               0
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_SWELL_PERIOD_PH_B             0
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_LINE_CURRENT_PH_C                         0xFFFF
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_ACTIVE_CURRENT_PH_C                       0xFFFF
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_REACTIVE_CURRENT_PH_C                     0xFFFF
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_PH_C                          0xFFFF
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_MIN_PH_C                      0x8000
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_MAX_PH_C                      0x8000
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_RMS_CURRENT_PH_C                          0xFFFF
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_RMS_CURRENT_MIN_PH_C                      0xFFFF
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_RMS_CURRENT_MAX_PH_C                      0xFFFF
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_ACTIVE_POWER_PH_C                         0xFFFF
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_ACTIVE_POWER_MIN_PH_C                     0xFFFF
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_ACTIVE_POWER_MAX_PH_C                     0xFFFF
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_REACTIVE_POWER_PH_C                       0xFFFF
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_APPARENT_POWER_PH_C                       0xFFFF
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_POWER_FACTOR_PH_C                         0
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_AVERAGE_RMS_VOLTAGE_MEASURE_PERIOD_PH_C   0
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_AVERAGE_RMS_OVER_VOLTAGE_COUNTER_PH_C     0
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_AVERAGE_UNDER_VOLTAGE_COUNTER_PH_C        0
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_RMS_EXTREME_OVER_VOLTAGE_PERIOD_PH_C      0
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_RMS_EXTREME_UNDER_VOLTAGE_PERIOD_PH_C     0
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_SAG_PERIOD_PH_C               0
#define ATTR_DEFAULT_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_SWELL_PERIOD_PH_C             0

// server commands
#define COMMAND_ELECTRICAL_MEASUREMENT_GET_PROFILE_INFO_RSP             0x00  // O, zclElectricalMeasurementGetProfileInfoRsp_t
#define COMMAND_ELECTRICAL_MEASUREMENT_GET_MEASUREMENT_PROFILE_RSP      0x01  // O, zclElectricalMeasurementGetMeasurementProfileRsp_t

// client commands
#define COMMAND_ELECTRICAL_MEASUREMENT_GET_PROFILE_INFO                 0x00  // O, no payload
#define COMMAND_ELECTRICAL_MEASUREMENT_GET_MEASUREMENT_PROFILE          0x01  // O, zclElectricalMeasurementGetMeasurementProfile_t

// Profile Interval Period, enumerated type
#define PROFILE_INTERVAL_PERIOD_DAILY       0
#define PROFILE_INTERVAL_PERIOD_60MINUTES   1
#define PROFILE_INTERVAL_PERIOD_30MINUTES   2
#define PROFILE_INTERVAL_PERIOD_15MINUTES   3
#define PROFILE_INTERVAL_PERIOD_10MINUTES   4
#define PROFILE_INTERVAL_PERIOD_7_5MINUTES  5
#define PROFILE_INTERVAL_PERIOD_5MINUTES    6
#define PROFILE_INTERVAL_PERIOD_2_5MINUTES  7

// Status, enumerated type
#define STATUS_SUCCESS                      0x00
#define STATUS_ATTR_NOT_SUPPORTED           0x01  // Attribute Profile not supported
#define STATUS_INVALID_START_TIME           0x02
#define STATUS_TOO_MANY_INTERVALS           0x03  // More intervals requested than can be returned
#define STATUS_NO_AVAILABLE_INTERVALS       0x04  // No intervals available for the requested time

/*******************************************************************************
 * TYPEDEFS
 */

/*** Server Commands Generated ***/

/*** ZCL Electrical Measurement Cluster: Get Profile Info Response payload ***/
typedef struct
{
  uint8 profileCount;
  uint8 profileIntervalPeriod;    // e.g. PROFILE_INTERVAL_PERIOD_DAILY
  uint8 maxNumberOfIntervals;
  uint8 numberOfAttributes;     // determines number of attrs for pListOfAttributes
  uint16 *pListOfAttributes;     // variable length
} zclElectricalMeasurementGetProfileInfoRsp_t;

/*** ZCL Electrical Measurement Cluster: Get Measurement Profile Response payload ***/
typedef struct
{
  uint32 startTime;
  uint8 status;                       // e.g. STATUS_SUCCESS
  uint8 profileIntervalPeriod;        // e.g. PROFILE_INTERVAL_PERIOD_DAILY
  uint8 numberOfIntervalsDelivered;
  uint16 attributeID;
  uint8 *pIntervals;    // variable length array based on numberOfIntervalsDelivered, type based on attributeID
} zclElectricalMeasurementGetMeasurementProfileRsp_t;


/*** Client Commands Generated ***/

/*** ZCL Electrical Measurement Cluster: Get Measurement Profile payload ***/
typedef struct
{
  uint16 attributeID;
  uint32 startTime;
  uint8 numberOfIntervals;
} zclElectricalMeasurementGetMeasurementProfile_t;


// This callback is called to process a GetProfileInfo command on a client (ZR)
typedef ZStatus_t (*zclElectrical_Measurement_GetProfileInfo_t)( void );
typedef ZStatus_t (*zclElectrical_Measurement_GetProfileInfoRsp_t)( zclElectricalMeasurementGetProfileInfoRsp_t *pCmd );
typedef ZStatus_t (*zclElectrical_Measurement_GetMeasurementProfile_t)( zclElectricalMeasurementGetMeasurementProfile_t *pCmd );
typedef ZStatus_t (*zclElectrical_Measurement_GetMeasurementProfileRsp_t)( zclElectricalMeasurementGetMeasurementProfileRsp_t *pCmd );


// Register Callbacks table entry - enter function pointers for callbacks that
// the application would like to receive
typedef struct
{
  zclElectrical_Measurement_GetProfileInfo_t                    pfnElectricalMeasurement_GetProfileInfo;
  zclElectrical_Measurement_GetProfileInfoRsp_t                 pfnElectricalMeasurement_GetProfileInfoRsp;
  zclElectrical_Measurement_GetMeasurementProfile_t             pfnElectricalMeasurement_GetMeasurementProfile;
  zclElectrical_Measurement_GetMeasurementProfileRsp_t          pfnElectricalMeasurement_GetMeasurementProfileRsp;
} zclElectricalMeasurement_AppCallbacks_t;

/******************************************************************************
 * FUNCTION MACROS
 */

/******************************************************************************
 * VARIABLES
 */

/******************************************************************************
 * FUNCTIONS
 */

/*
 * Register for callbacks from this cluster library
 */
extern ZStatus_t zclElectricalMeasurement_RegisterCmdCallbacks( uint8 endpoint, zclElectricalMeasurement_AppCallbacks_t *callbacks );


/*********************************************************************
 * @fn      zclElectricalMeasurement_Send_GetProfileInfo
 *
 * @brief   Call to send out Electrical Measurement Get Profile info command from ZED to ZR/ZC. The Rsp
 *          will indicate the parameters of the device's profile.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclElectricalMeasurement_Send_GetProfileInfo( uint8 srcEP, afAddrType_t *dstAddr,
                                                               uint8 disableDefaultRsp, uint8 seqNum );

/*********************************************************************
 * @fn      zclElectricalMeasurement_Send_GetProfileInfoRsp
 *
 * @brief   Call to send out Electrical Measurement Get Profile Info Response. This will tell
 *          the client the appropriate parameters of the measurement profile.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pPayload:
 *          profileCount - total number of supported profiles
 *          profileIntervalPeriod - indicates the time frame of capture for profiling purposes
 *          maxNumberOfIntervals - maximum number of intervals allowed for the response
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclElectricalMeasurement_Send_GetProfileInfoRsp( uint8 srcEP, afAddrType_t *dstAddr,
                                                                  zclElectricalMeasurementGetProfileInfoRsp_t *pPayload,
                                                                  uint8 disableDefaultRsp, uint8 seqNum );

/*********************************************************************
 * @fn      zclElectricalMeasurement_Send_GetMeasurementProfile
 *
 * @brief   Call to send out Electrical Measurement Get Measurement Profile. This will
 *          ask the server for the appropriate parameters of the measurement profile.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   attributeID - the electricity measurement attribute being profiled
 * @param   startTime - selects the interval block from available interval blocks
 * @param   numberOfIntervals - represents the number of intervals being requested
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclElectricalMeasurement_Send_GetMeasurementProfile( uint8 srcEP, afAddrType_t *dstAddr,
                                                                      uint16 attributeID, uint32 startTime,
                                                                      uint8 numberOfIntervals,
                                                                      uint8 disableDefaultRsp, uint8 seqNum );

/*********************************************************************
 * @fn      zclElectricalMeasurement_Send_GetMeasurementProfileRsp
 *
 * @brief   Call to send out Electrical Measurement Get Measurement Profile Response. This will
 *          tell the client the appropriate parameters of the measurement profile.
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pPayload
 *          startTime - represents the end time of the most chronologically recent interval being requested
 *          status - table status enumeration lists the valid values returned in status field
 *          profileIntervalPeriod - time frame used to capture parameter for profiling purposes
 *          numberOfIntervalsDelivered - number of intervals the device is returning
 *          attributeID - attribute that has been profiled by the application
 *          intervals   - array of intervals that depend on numberOfIntervalsDelivered, type based on attributeID
 * @param   disableDefaultRsp - whether to disable the Default Response command
 * @param   seqNum - sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclElectricalMeasurement_Send_GetMeasurementProfileRsp( uint8 srcEP, afAddrType_t *dstAddr,
                                                                         zclElectricalMeasurementGetMeasurementProfileRsp_t *pPayload,
                                                                         uint8 disableDefaultRsp, uint8 seqNum );

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif // ZCL_ELECTRICAL_MEASUREMENT
#endif /* ZCL_ELECTRICAL_MEASUREMENT_H */
