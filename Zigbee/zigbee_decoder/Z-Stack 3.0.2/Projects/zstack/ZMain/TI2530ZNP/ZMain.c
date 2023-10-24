/**************************************************************************************************
  Filename:       ZMain.c
  Revised:        $Date: 2010-09-17 16:25:30 -0700 (Fri, 17 Sep 2010) $
  Revision:       $Revision: 23835 $

  Description:    Startup and shutdown code for ZStack
  Notes:          This version targets the Chipcon CC2530


  Copyright 2005-2010 Texas Instruments Incorporated. All rights reserved.

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

/*********************************************************************
 * INCLUDES
 */

#ifndef NONWK
#include "AF.h"
#endif
#include "hal_adc.h"
#include "hal_flash.h"
#include "hal_lcd.h"
#include "hal_led.h"
#include "hal_drivers.h"
#include "OnBoard.h"
#include "OSAL.h"
#include "OSAL_Nv.h"
#include "ZComDef.h"
#include "ZMAC.h"
#include "znp_app.h"

extern void osal_start_znp( void );

/*********************************************************************
 * LOCAL FUNCTIONS
 */

static void zmain_ext_addr( void );
#if defined ZCL_KEY_ESTABLISH
static void zmain_cert_init( void );
#endif
static void zmain_dev_info( void );
static void zmain_vdd_check( void );

#ifdef LCD_SUPPORTED
static void zmain_lcd_init( void );
#endif

/*********************************************************************
 * @fn      main
 * @brief   First function called after startup.
 * @return  don't care
 */
int main( void )
{
  // Turn off interrupts
  osal_int_disable( INTS_ALL );

  // Initialization for board related stuff such as LEDs
  HAL_BOARD_INIT();

  // Make sure supply voltage is high enough to run
  zmain_vdd_check();

  // Initialize board I/O
  InitBoard( OB_COLD );

  // Initialze HAL drivers
  HalDriverInit();

  // Initialize NV System
  osal_nv_init( NULL );

  // Initialize and check the ZNP RF Test Mode NV items.
  znpTestRF();

  // Initialize the MAC
  ZMacInit();

  // Determine the extended address
  zmain_ext_addr();

#if defined ZCL_KEY_ESTABLISH
  // Initialize the Certicom certificate information.
  zmain_cert_init();
#endif

  // Initialize basic NV items
  zgInit();

#ifndef NONWK
  // Since the AF isn't a task, call it's initialization routine
  afInit();
#endif

  // Initialize the operating system
  osal_init_system();

  // Allow interrupts
  osal_int_enable( INTS_ALL );

  // Final board initialization
  InitBoard( OB_READY );

  // Display information about this device
  zmain_dev_info();

  /* Display the device info on the LCD */
#ifdef LCD_SUPPORTED
  zmain_lcd_init();
#endif

#ifdef WDT_IN_PM1
  /* If WDT is used, this is a good place to enable it. */
  WatchDogEnable( WDTIMX );
#endif

  osal_start_znp(); // No Return from here

  return 0;  // Shouldn't get here.
} // main()

/*********************************************************************
 * @fn      zmain_vdd_check
 * @brief   Check if the Vdd is OK to run the processor.
 * @return  Return if Vdd is ok; otherwise, flash LED, then reset
 *********************************************************************/
static void zmain_vdd_check( void )
{
  uint8 cnt = 16;
  
  do {
    while (!HalAdcCheckVdd(VDD_MIN_RUN));
  } while (--cnt);
}

/**************************************************************************************************
 * @fn          zmain_ext_addr
 *
 * @brief       Execute a prioritized search for a valid extended address and write the results
 *              into the OSAL NV system for use by the system. Temporary address not saved to NV.
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
static void zmain_ext_addr(void)
{
  uint8 nullAddr[Z_EXTADDR_LEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  uint8 writeNV = TRUE;

  // First check whether a non-erased extended address exists in the OSAL NV.
  if ((SUCCESS != osal_nv_item_init(ZCD_NV_EXTADDR, Z_EXTADDR_LEN, NULL))  ||
      (SUCCESS != osal_nv_read(ZCD_NV_EXTADDR, 0, Z_EXTADDR_LEN, aExtendedAddress)) ||
      (osal_memcmp(aExtendedAddress, nullAddr, Z_EXTADDR_LEN)))
  {
    // Attempt to read the extended address from the location on the lock bits page
    // where the programming tools know to reserve it.
    HalFlashRead(HAL_FLASH_IEEE_PAGE, HAL_FLASH_IEEE_OSET, aExtendedAddress, Z_EXTADDR_LEN);

    if (osal_memcmp(aExtendedAddress, nullAddr, Z_EXTADDR_LEN))
    {
      // Attempt to read the extended address from the designated location in the Info Page.
      if (!osal_memcmp((uint8 *)(P_INFOPAGE+HAL_INFOP_IEEE_OSET), nullAddr, Z_EXTADDR_LEN))
      {
        osal_memcpy(aExtendedAddress, (uint8 *)(P_INFOPAGE+HAL_INFOP_IEEE_OSET), Z_EXTADDR_LEN);
      }
      else  // No valid extended address was found.
      {
        uint8 idx;
        
#if !defined ( NV_RESTORE )
        writeNV = FALSE;  // Make this a temporary IEEE address
#endif

        /* Attempt to create a sufficiently random extended address for expediency.
         * Note: this is only valid/legal in a test environment and
         *       must never be used for a commercial product.
         */
        for (idx = 0; idx < (Z_EXTADDR_LEN - 2);)
        {
          uint16 randy = osal_rand();
          aExtendedAddress[idx++] = LO_UINT16(randy);
          aExtendedAddress[idx++] = HI_UINT16(randy);
        }
        // Next-to-MSB identifies ZigBee devicetype.
#if ZG_BUILD_COORDINATOR_TYPE && !ZG_BUILD_JOINING_TYPE
        aExtendedAddress[idx++] = 0x10;
#elif ZG_BUILD_RTRONLY_TYPE
        aExtendedAddress[idx++] = 0x20;
#else
        aExtendedAddress[idx++] = 0x30;
#endif
        // MSB has historical signficance.
        aExtendedAddress[idx] = 0xF8;
      }
    }

    if (writeNV)
    {
      (void)osal_nv_write(ZCD_NV_EXTADDR, 0, Z_EXTADDR_LEN, aExtendedAddress);
    }
  }

  // Set the MAC PIB extended address according to results from above.
  (void)ZMacSetReq(MAC_EXTENDED_ADDRESS, aExtendedAddress);
}

#if defined ZCL_KEY_ESTABLISH
/**************************************************************************************************
 * @fn          zmain_cert_init
 *
 * @brief       Initialize the Certicom certificate information.
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
static void zmain_cert_init(void)
{
  uint8 certData[ZCL_KE_IMPLICIT_CERTIFICATE_LEN];
  uint8 nullData[ZCL_KE_IMPLICIT_CERTIFICATE_LEN] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
  };

  (void)osal_nv_item_init(ZCD_NV_IMPLICIT_CERTIFICATE, ZCL_KE_IMPLICIT_CERTIFICATE_LEN, NULL);
  (void)osal_nv_item_init(ZCD_NV_DEVICE_PRIVATE_KEY, ZCL_KE_DEVICE_PRIVATE_KEY_LEN, NULL);

  // First check whether non-null certificate data exists in the OSAL NV. To save on code space,
  // just use the ZCD_NV_CA_PUBLIC_KEY as the bellwether for all three.
  if ((SUCCESS != osal_nv_item_init(ZCD_NV_CA_PUBLIC_KEY, ZCL_KE_CA_PUBLIC_KEY_LEN, NULL))    ||
      (SUCCESS != osal_nv_read(ZCD_NV_CA_PUBLIC_KEY, 0, ZCL_KE_CA_PUBLIC_KEY_LEN, certData))  ||
      (osal_memcmp(certData, nullData, ZCL_KE_CA_PUBLIC_KEY_LEN)))
  {
    // Attempt to read the certificate data from its corresponding location on the lock bits page.
    HalFlashRead(HAL_FLASH_IEEE_PAGE, HAL_FLASH_CA_PUBLIC_KEY_OSET, certData,
                                         ZCL_KE_CA_PUBLIC_KEY_LEN);
    // If the certificate data is not NULL, use it to update the corresponding NV items.
    if (!osal_memcmp(certData, nullData, ZCL_KE_CA_PUBLIC_KEY_LEN))
    {
      (void)osal_nv_write(ZCD_NV_CA_PUBLIC_KEY, 0, ZCL_KE_CA_PUBLIC_KEY_LEN, certData);
      HalFlashRead(HAL_FLASH_IEEE_PAGE, HAL_FLASH_IMPLICIT_CERT_OSET, certData,
                                           ZCL_KE_IMPLICIT_CERTIFICATE_LEN);
      (void)osal_nv_write(ZCD_NV_IMPLICIT_CERTIFICATE, 0,
                          ZCL_KE_IMPLICIT_CERTIFICATE_LEN, certData);
      HalFlashRead(HAL_FLASH_IEEE_PAGE, HAL_FLASH_DEV_PRIVATE_KEY_OSET, certData,
                                        ZCL_KE_DEVICE_PRIVATE_KEY_LEN);
      (void)osal_nv_write(ZCD_NV_DEVICE_PRIVATE_KEY, 0, ZCL_KE_DEVICE_PRIVATE_KEY_LEN, certData);
    }
  }
}
#endif

/**************************************************************************************************
 * @fn          zmain_dev_info
 *
 * @brief       This displays the IEEE (MSB to LSB) on the LCD.
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
static void zmain_dev_info(void)
{
#ifdef LCD_SUPPORTED
  uint8 i;
  uint8 *xad;
  uint8 lcd_buf[Z_EXTADDR_LEN*2+1];

  // Display the extended address.
  xad = aExtendedAddress + Z_EXTADDR_LEN - 1;

  for (i = 0; i < Z_EXTADDR_LEN*2; xad--)
  {
    uint8 ch;
    ch = (*xad >> 4) & 0x0F;
    lcd_buf[i++] = ch + (( ch < 10 ) ? '0' : '7');
    ch = *xad & 0x0F;
    lcd_buf[i++] = ch + (( ch < 10 ) ? '0' : '7');
  }
  lcd_buf[Z_EXTADDR_LEN*2] = '\0';
  HalLcdWriteString( "IEEE: ", HAL_LCD_LINE_1 );
  HalLcdWriteString( (char*)lcd_buf, HAL_LCD_LINE_2 );
#endif
}

#ifdef LCD_SUPPORTED
/*********************************************************************
 * @fn      zmain_lcd_init
 * @brief   Initialize LCD at start up.
 * @return  none
 *********************************************************************/
static void zmain_lcd_init ( void )
{
#ifdef SERIAL_DEBUG_SUPPORTED
  {
    HalLcdWriteString( "TexasInstruments", HAL_LCD_LINE_1 );

#if defined( MT_MAC_FUNC )
#if defined( ZDO_COORDINATOR )
      HalLcdWriteString( "MAC-MT Coord", HAL_LCD_LINE_2 );
#else
      HalLcdWriteString( "MAC-MT Device", HAL_LCD_LINE_2 );
#endif // ZDO
#elif defined( MT_NWK_FUNC )
#if defined( ZDO_COORDINATOR )
      HalLcdWriteString( "NWK Coordinator", HAL_LCD_LINE_2 );
#else
      HalLcdWriteString( "NWK Device", HAL_LCD_LINE_2 );
#endif // ZDO
#endif // MT_FUNC
  }
#endif // SERIAL_DEBUG_SUPPORTED
}
#endif

/*********************************************************************
*********************************************************************/
