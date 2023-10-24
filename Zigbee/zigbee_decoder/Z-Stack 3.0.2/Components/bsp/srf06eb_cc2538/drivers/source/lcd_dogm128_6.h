//*****************************************************************************
//! @file       lcd_dogm128_6.h
//! @brief      Device driver header file for DOGM128-6 LCD display.
//!
//!             The DOGM128-6 LCD display is a 128x64 dot matrix and is divided
//!             into 8 pages (\c LCD_PAGE_0 through \c LCD_PAGE_7),
//!             each 8 px high.
//!
//! Datasheet   http://www.lcd-module.com/eng/pdf/grafik/dogm128e.pdf
//!
//! Revised     $Date: 2013-04-11 10:57:23 -0700 (Thu, 11 Apr 2013) $
//! Revision    $Revision: 9711 $
//
//  Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
//
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions
//  are met:
//
//    Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//
//    Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//
//    Neither the name of Texas Instruments Incorporated nor the names of
//    its contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
//  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
//  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
//  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
//  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
//  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//****************************************************************************/
#ifndef __LCD_DOGM128_6_H__
#define __LCD_DOGM128_6_H__


/******************************************************************************
* If building with a C++ compiler, make all of the definitions in this header
* have a C binding.
******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif


/******************************************************************************
* INCLUDES
*/
#include <stdint.h>


/******************************************************************************
* DEFINES
*/
#define LCD_PIXELS              8192    // Number of pixels in LCD display
#define LCD_BYTES               1024    // Number of bytes needed in LCD buffer
#define LCD_COLS                128     // Number of pixel columns
#define LCD_ROWS                64      // Number of pixel rows
#define LCD_PAGES               8       // Number of pages
#define LCD_PAGE_ROWS           8       // Number of pixel rows per LCD page

//
// The difference between LCD_CHAR_WIDTH and LCD_FONT_WIDTH
// equals the character spacing on the LCD display.
//
#define LCD_CHAR_WIDTH          6       // Space used for each character
#define LCD_FONT_WIDTH          5       // The actual font character width

//
// LCD animation enum. Used by lcdSendBufferAnimated
//
typedef enum
{
    eLcdNoMotion,
    eLcdSlideRight,
    eLcdSlideLeft
}
tLcdMotion;

//
// LCD alignment enum. Used by lcdPrintxxxAligned functions
//
typedef enum
{
    eLcdAlignLeft,
    eLcdAlignCenter,
    eLcdAlignRight
}
tLcdAlign;

//
// LCD page enum. Used for page argument in lcdBufferxxx function
//
typedef enum
{
    eLcdPage0 = 0,
    eLcdPage1 = 1,
    eLcdPage2 = 2,
    eLcdPage3 = 3,
    eLcdPage4 = 4,
    eLcdPage5 = 5,
    eLcdPage6 = 6,
    eLcdPage7 = 7
}
tLcdPage;

//
// LCD x-axis enum. Used for x argument in lcdBufferxxx functions
//
typedef enum
{
    eLcdXFirst = 0,
    eLcdXLast = (LCD_COLS-1)
}
tLcdXLimit;

//
// LCD y-axis enum. Used for y argument in lcdBufferxxx functions
//
typedef enum
{
    eLcdYFirst = 0,
    eLcdYLast = (LCD_ROWS-1)
}
tLcdYLimit;

/******************************************************************************
* EXTERNAL VARIABLES
*/

extern const uint8_t lcd_alphabet[];

#ifndef LCD_NO_DEFAULT_BUFFER
extern char lcdDefaultBuffer[LCD_BYTES];
#endif


/******************************************************************************
* FUNCTION PROTOTYPES
*/
//
// Functions accessing LCD
//
extern void lcdInit(void);
extern void lcdClear(void);
extern void lcdSpiInit(void);
extern void lcdSendCommand(const char *pcCmd, uint8_t ui8Len);
extern void lcdSendData(const char *pcData, uint16_t ui1Len);
extern void lcdSendBufferAnimated(const char *pcToBuffer,
                                  const char *pcFromBuffer, tLcdMotion iMotion);

extern void lcdSendBuffer(const char *pcBuffer);
extern void lcdSendBufferPart(const char *pcBuffer, uint8_t ui8XFrom,
                              uint8_t ui8XTo, tLcdPage iPageFrom,
                              tLcdPage iPageTo);
extern void lcdGotoXY(uint8_t ui8X, uint8_t ui8Y);
extern void lcdSetContrast(uint8_t ui8Contrast);

//
// Buffer manipulation functions
//
extern void lcdBufferClear(char *pcBuffer);
extern void lcdBufferClearPage(char *pcBuffer, tLcdPage iPage);
extern void lcdBufferClearPart(char *pcBuffer, uint8_t ui8XFrom,
                               uint8_t ui8XTo, tLcdPage iPageFrom,
                               tLcdPage iPageTo);
extern void lcdBufferInvert(char *pcBuffer, uint8_t ui8XFrom, uint8_t ui8YFrom,
                            uint8_t ui8XTo, uint8_t ui8YTo);
extern void lcdBufferInvertPage(char *pcBuffer, uint8_t ui8XFrom,
                                uint8_t ui8XTo, tLcdPage iPage);
extern uint8_t lcdGetStringLength(const char *pcStr);
extern uint8_t lcdGetIntLength(int32_t i32Number);
extern uint8_t lcdGetFloatLength(float fNumber, uint8_t ui8Decimals);
extern void lcdBufferPrintString(char *pcBuffer, const char *pcStr,
                                 uint8_t ui8X, tLcdPage iPage);
extern void lcdBufferPrintStringAligned(char *pcBuffer, const char *pcStr,
                                 tLcdAlign iAlignment, tLcdPage iPage);
extern void lcdBufferPrintInt(char *pcBuffer, int32_t i32Number, uint8_t ui8X,
                              tLcdPage iPage);
extern void lcdBufferPrintIntAligned(char *pcBuffer, int32_t i32Number,
                                     tLcdAlign iAlignment, tLcdPage iPage);
extern void lcdBufferPrintFloat(char *pcBuffer, float fNumber,
                                uint8_t ui8Decimals, uint8_t ui8X,
                                tLcdPage iPage);
extern void lcdBufferPrintFloatAligned(char *pcBuffer, float fNumber,
                                uint8_t ui8Decimals, tLcdAlign iAlignment,
                                tLcdPage iPage);
extern void lcdBufferSetLine(char *pcBuffer, uint8_t ui8XFrom,
                             uint8_t ui8YFrom, uint8_t ui8XTo, uint8_t ui8YTo);
extern void lcdBufferClearLine(char *pcBuffer, uint8_t ui8XFrom,
                               uint8_t ui8YFrom, uint8_t ui8XTo,
                               uint8_t ui8YTo);
extern void lcdBufferSetHLine(char *pcBuffer, uint8_t ui8XFrom, uint8_t ui8XTo,
                              uint8_t ui8Y);
extern void lcdBufferClearHLine(char *pcBuffer, uint8_t ui8XFrom,
                                uint8_t ui8XTo, uint8_t ui8Y);
extern void lcdBufferSetVLine(char *pcBuffer, uint8_t ui8X, uint8_t ui8YFrom,
                              uint8_t ui8YTo);
extern void lcdBufferClearVLine(char *pcBuffer, uint8_t ui8X, uint8_t ui8YFrom,
                                uint8_t ui8YTo);
extern void lcdBufferHArrow(char *pcBuffer, uint8_t ui8XFrom, uint8_t ui8XTo,
                            uint8_t ui8Y);
extern void lcdBufferVArrow(char *pcBuffer, uint8_t ui8X, uint8_t ui8YFrom,
                            uint8_t ui8YTo);
extern void lcdBufferSetPx(char *pcBuffer, uint8_t ui8X, uint8_t ui8Y);
extern void lcdBufferClearPx(char *pcBuffer, uint8_t ui8X, uint8_t ui8Y);
extern void lcdBufferCopy(const char *pcFromBuffer, char *pcToBuffer);


/******************************************************************************
* Mark the end of the C bindings section for C++ compilers.
******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif /* #ifndef __LCD_DOGM128_6_H__ */
