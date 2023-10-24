@echo off
rem /**********************************************************************************************
rem   Filename:       znp.bat
rem   Revised:        $Date: 2014-04-10 06:42:21 -0700 (Thu, 10 Apr 2014) $
rem   Revision:       $Revision: 38121 $
rem 
rem   Description:    This file is a launcher for the znp.js.
rem 
rem 
rem   Copyright 2010-2014 Texas Instruments Incorporated. All rights reserved.
rem 
rem   IMPORTANT: Your use of this Software is limited to those specific rights
rem   granted under the terms of a software license agreement between the user
rem   who downloaded the software, his/her employer (which must be your employer)
rem   and Texas Instruments Incorporated (the "License").  You may not use this
rem   Software unless you agree to abide by the terms of the License. The License
rem   limits your use, and you acknowledge, that the Software may not be modified,
rem   copied or distributed unless embedded on a Texas Instruments microcontroller
rem   or used solely and exclusively in conjunction with a Texas Instruments radio
rem   frequency transceiver, which is integrated into your product.  Other than for
rem   the foregoing purpose, you may not use, reproduce, copy, prepare derivative
rem   works of, modify, distribute, perform, display or sell this Software and/or
rem   its documentation for any purpose.
rem 
rem   YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
rem   PROVIDED “AS IS” WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, 
rem   INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE, 
rem   NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
rem   TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
rem   NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
rem   LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
rem   INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
rem   OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
rem   OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
rem   (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
rem 
rem   Should you have any questions regarding your right to use this Software,
rem   contact Texas Instruments Incorporated at www.TI.com.
rem **********************************************************************************************/

if "%2"=="" goto Usage

:Launch
chdir %1
start znp.js %2
exit /b

:Usage
echo.-------------------------------------------------------------------------------
echo.
echo. This batch file launches the ZNP build post-processing script called "znp.js"
echo. and is normally called during the IAR build process to create ZNP hex files.
echo.
echo. You are reading this message because one or both of the expected command-line
echo. parameters was not provided when calling "znp.bat".
echo.
echo. Proper invocation of this batch file is:
echo.    znp.bat "ZNP Tools Folder" "ZNP Build Config"
echo.
echo. For example, open the "znp.eww" IAR workspace and select the CC2530-ProdHex
echo. configuration. Select the Project-^>Options-^>BuildActions and have a look at
echo. the entry in the "Post-build command line:" text box.
echo.
echo.-------------------------------------------------------------------------------
pause
