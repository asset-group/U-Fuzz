/**************************************************************************************************
  Filename:       hal_lcd.c
  Revised:        $Date: 2013-10-21 17:25:23 -0700 (Mon, 21 Oct 2013) $
  Revision:       $Revision: 35745 $

  Description:    This file contains the interface to the HAL LCD Service.


  Copyright 2011-2013 Texas Instruments Incorporated. All rights reserved.

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

/**************************************************************************************************
 *                                           INCLUDES
 **************************************************************************************************/
#include <string.h>
#include "hal_types.h"
#include "hal_lcd.h"
#include "OnBoard.h"
#include "OSAL.h"
#include "bsp.h"
/*
 * "hal_mac_cfg.h" is included here because the OLED and CC2520 radio are
 * sharing the same SSI port. CC2520 cSn needs to be disabled while writing
 * to OLED.
 */
#include "hal_mac_cfg.h"
#include "lcd_dogm128_6.h"

#if defined (ZTOOL_P1) || defined (ZTOOL_P2)
  #include "DebugTrace.h"
#endif

/**************************************************************************************************
 *                                          CONSTANTS
 **************************************************************************************************/

#if defined LCD_NO_DEFAULT_BUFFER
#define HAL_LCD_DEF_BUF     ((const char *)0x20007000)
#else
#define HAL_LCD_DEF_BUF     ((const char *)NULL)
#endif

/**************************************************************************************************
 *                                           MACROS
 **************************************************************************************************/

#define SPI_CLOCK 2000000UL


/**************************************************************************************************
 *                                       LOCAL FUNCTIONS
 **************************************************************************************************/

#if (HAL_LCD == TRUE)
void HalLcd_HW_WriteLine(char str[], uint8 line);
#endif

/**************************************************************************************************
 * @fn      HalLcdInit
 *
 * @brief   Initilize LCD Service
 *
 * @param   init - pointer to void that contains the initialized value
 *
 * @return  None
 **************************************************************************************************/
void HalLcdInit(void)
{
#if (HAL_LCD == TRUE)
  bspSpiInit(SPI_CLOCK);
  lcdInit();
#if defined LCD_NO_DEFAULT_BUFFER && (defined HAL_IMG_AREA && (HAL_IMG_AREA == 2))
  (void)memset(HAL_LCD_DEF_BUF, 0, LCD_BYTES);
#endif
#endif
}

/*************************************************************************************************
 *                    LCD EMULATION FUNCTIONS
 *
 * Some evaluation boards are equipped with Liquid Crystal Displays
 * (LCD) which may be used to display diagnostic information. These
 * functions provide LCD emulation, sending the diagnostic strings
 * to Z-Tool via the RS232 serial port. These functions are enabled
 * when the "LCD_SUPPORTED" compiler flag is placed in the makefile.
 *
 * Most applications update both lines (1 and 2) of the LCD whenever
 * text is posted to the device. This emulator assumes that line 1 is
 * updated first (saved locally) and the formatting and send operation
 * is triggered by receipt of line 2. Nothing will be transmitted if
 * only line 1 is updated.
 *
 *************************************************************************************************/



/**************************************************************************************************
 * @fn      HalLcdWriteString
 *
 * @brief   Write a string to the LCD
 *
 * @param   str    - pointer to the string that will be displayed
 *          line   - line number to display
 *
 * @return  None
 **************************************************************************************************/
void HalLcdWriteString ( char *str, uint8 line)
{
#if (HAL_LCD == TRUE)
#if (defined HAL_IMG_AREA && (HAL_IMG_AREA == 3))
  line += 4;
#endif
#if defined (ZTOOL_P1) || defined (ZTOOL_P2)
#if defined (SERIAL_DEBUG_SUPPORTED)
  debug_str( (uint8*)str );
#endif
#endif
  HalLcd_HW_WriteLine (str, line - 1);
#endif
}

/**************************************************************************************************
 * @fn      HalLcdWriteValue
 *
 * @brief   Write a value to the LCD
 *
 * @param   value  - value that will be displayed
 *          radix  - 8, 10, 16
 *          option - display options
 *
 * @return  None
 **************************************************************************************************/
void HalLcdWriteValue ( uint32 value, const uint8 radix, uint8 option)
{
#if (HAL_LCD == TRUE)
  uint8 buf[HAL_LCD_MAX_CHARS];

  _ltoa( value, &buf[0], radix );
  HalLcdWriteString( (char*)buf, option );
#endif /* HAL_LCD */
}

/**************************************************************************************************
 * @fn      HalLcdWriteScreen
 *
 * @brief   Write a value to the LCD
 *
 * @param   line1  - string that will be displayed on line 1
 *          line2  - string that will be displayed on line 2
 *
 * @return  None
 **************************************************************************************************/
void HalLcdWriteScreen( char *line1, char *line2 )
{
#if (HAL_LCD == TRUE)
  HalLcdWriteString( line1, 1 );
  HalLcdWriteString( line2, 2 );
#endif /* HAL_LCD */
}

/**************************************************************************************************
 * @fn      HalLcdWriteStringValue
 *
 * @brief   Write a string followed by a value to the LCD
 *
 * @param   title  - Title that will be displayed before the value
 *          value  - value
 *          format - redix
 *          line   - line number
 *
 * @return  None
 **************************************************************************************************/
void HalLcdWriteStringValue( char *title, uint16 value, uint8 format, uint8 line )
{
#if (HAL_LCD == TRUE)
  uint8 tmpLen;
  uint8 buf[HAL_LCD_MAX_CHARS];
  uint32 err;

  tmpLen = (uint8)osal_strlen( (char*)title );
  osal_memcpy( buf, title, tmpLen );
  buf[tmpLen] = ' ';
  err = (uint32)(value);
  _ltoa( err, &buf[tmpLen+1], format );
  HalLcdWriteString( (char*)buf, line );		
#endif /* HAL_LCD */
}

/**************************************************************************************************
 * @fn      HalLcdWriteStringValue
 *
 * @brief   Write a string followed by a value to the LCD
 *
 * @param   title   - Title that will be displayed before the value
 *          value1  - value #1
 *          format1 - redix of value #1
 *          value2  - value #2
 *          format2 - redix of value #2
 *          line    - line number
 *
 * @return  None
 **************************************************************************************************/
void HalLcdWriteStringValueValue( char *title, uint16 value1, uint8 format1,
                                  uint16 value2, uint8 format2, uint8 line )
{
#if (HAL_LCD == TRUE)
  uint8 tmpLen;
  uint8 buf[HAL_LCD_MAX_CHARS];
  uint32 err;

  tmpLen = (uint8)osal_strlen( (char*)title );
  if ( tmpLen )
  {
    osal_memcpy( buf, title, tmpLen );
    buf[tmpLen++] = ' ';
  }

  err = (uint32)(value1);
  _ltoa( err, &buf[tmpLen], format1 );
  tmpLen = (uint8)osal_strlen( (char*)buf );

  buf[tmpLen++] = ',';
  buf[tmpLen++] = ' ';
  err = (uint32)(value2);
  _ltoa( err, &buf[tmpLen], format2 );

  HalLcdWriteString( (char *)buf, line );		
#endif /* HAL_LCD */
}

/**************************************************************************************************
 * @fn      HalLcdDisplayPercentBar
 *
 * @brief   Display percentage bar on the LCD
 *
 * @param   title   -
 *          value   -
 *
 * @return  None
 **************************************************************************************************/
void HalLcdDisplayPercentBar( char *title, uint8 value )
{
#if (HAL_LCD == TRUE)
  uint8 percent;
  uint8 leftOver;
  uint8 buf[17];
  uint32 err;
  uint8 x;

  /* Write the title: */
  HalLcdWriteString( title, HAL_LCD_LINE_1 );

  if ( value > 100 )
    value = 100;

  /* convert to blocks */
  percent = (uint8)(value / 10);
  leftOver = (uint8)(value % 10);

  /* Make window */
  osal_memcpy( buf, "[          ]  ", 15 );

  for ( x = 0; x < percent; x ++ )
  {
    buf[1+x] = '>';
  }

  if ( leftOver >= 5 )
    buf[1+x] = '+';

  err = (uint32)value;
  _ltoa( err, (uint8*)&buf[13], 10 );

  HalLcdWriteString( (char*)buf, HAL_LCD_LINE_2 );
#endif /* HAL_LCD */
}

#if (HAL_LCD == TRUE)
/**************************************************************************************************
 * @fn          halLcdWriteLine
 *
 * @brief       Write one line on display. It is required to place a 
 *              NULL terminator \0 at the end of the string
 *
 * @param       uint8 line - display line
 *              char *pText - text buffer to write
 *
 * @return      none
 **************************************************************************************************/
void HalLcd_HW_WriteLine(char str[], uint8 line)
{
  /* Initialize with spaces to clear the line on the LCD. */
  uint8 buf[] = "                     ";  
  uint32 len = ((strlen(str) > HAL_LCD_MAX_CHARS) ? HAL_LCD_MAX_CHARS : strlen(str));
  
  /* Copy string characters but not the NULL to keep spaces */
  (void)memcpy(buf, str, len);    

  /* Copy into the default buffer */
  lcdBufferPrintString((char *)HAL_LCD_DEF_BUF, (char const *)buf, 0, line); 
  
  /* Flush the default buffer to the LCD */
  lcdSendBuffer(HAL_LCD_DEF_BUF);                       
}

#endif

/**************************************************************************************************
**************************************************************************************************/




