/****************************************************************************
  Filename:       saddr.c
  Revised:        $Date: 2014-11-06 11:03:55 -0800 (Thu, 06 Nov 2014) $
  Revision:       $Revision: 41021 $

  Description:    Zigbee and 802.15.4 device address utility functions.


  Copyright 2005-2014 Texas Instruments Incorporated. All rights reserved.

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
****************************************************************************/

/****************************************************************************
 * INCLUDES
 */
#include "hal_types.h"
#include "OSAL.h"
#include "saddr.h"

#include "R2R_FlashJT.h"
#if defined (CC26XX)
#include "R2F_FlashJT.h"
#endif /* CC26XX */

/****************************************************************************
 * @fn          sAddrCmp
 *
 * @brief       Compare two device addresses.
 *
 * input parameters
 *
 * @param       pAddr1        - Pointer to first address.
 * @param       pAddr2        - Pointer to second address.
 *
 * output parameters
 *
 * @return      TRUE if addresses are equal, FALSE otherwise
 */
bool sAddrCmp(const sAddr_t *pAddr1, const sAddr_t *pAddr2)
{
  if (pAddr1->addrMode != pAddr2->addrMode)
  {
    return FALSE;
  }
  else if (pAddr1->addrMode == SADDR_MODE_NONE)
  {
    return FALSE;
  }
  else if (pAddr1->addrMode == SADDR_MODE_SHORT)
  {
    return (bool) (pAddr1->addr.shortAddr == pAddr2->addr.shortAddr);
  }
  else if (pAddr1->addrMode == SADDR_MODE_EXT)
  {
    return (MAP_sAddrExtCmp(pAddr1->addr.extAddr, pAddr2->addr.extAddr));
  }
  else
  {
    return FALSE;
  }
}

/****************************************************************************
 * @fn          sAddrIden
 *
 * @brief       Check if two device addresses are identical.
 *
 *              This routine is virtually the same as sAddrCmp, which is used
 *              to determine if two different addresses are the same. However,
 *              this routine can be used to determine if an address is the
 *              same as a previously stored address. The key difference is in
 *              the former case, if the address mode is "none", then the
 *              assumption is that the two addresses can not be the same. But
 *              in the latter case, the address mode itself is being compared.
 *              So two addresses can be identical even if the address mode is
 *              "none", as long as the address mode of both addresses being
 *              compared is "none".
 *
 * input parameters
 *
 * @param       pAddr1        - Pointer to first address.
 * @param       pAddr2        - Pointer to second address.
 *
 * output parameters
 *
 * @return      TRUE if addresses are identical, FALSE otherwise
 */
bool sAddrIden(const sAddr_t *pAddr1, const sAddr_t *pAddr2)
{
  // first check if the address modes are the same
  if (pAddr1->addrMode != pAddr2->addrMode)
  {
    // no, so no point in comparing any further
    return FALSE;
  }
  // the address modes are the same; check if there is no address
  else if (pAddr1->addrMode == SADDR_MODE_NONE)
  {
    // no address, so no need to compare any further as both addresses have the
    // same address mode but no address, so they are identical
    return TRUE;
  }
  // there's an address; check if it is short
  else if (pAddr1->addrMode == SADDR_MODE_SHORT)
  {
    // compare short addresses
    return (bool) (pAddr1->addr.shortAddr == pAddr2->addr.shortAddr);
  }
  // there's an address; check if it is extended
  else if (pAddr1->addrMode == SADDR_MODE_EXT)
  {
    // compare extended addresses
    return (MAP_sAddrExtCmp(pAddr1->addr.extAddr, pAddr2->addr.extAddr));
  }
  else // unknown error
  {
    return FALSE;
  }
}

/****************************************************************************
 * @fn          sAddrCpy
 *
 * @brief       Copy a device address.
 *
 * input parameters
 *
 * @param       pSrc         - Pointer to address to copy.
 *
 * output parameters
 *
 * @param       pDest        - Pointer to address of copy.
 *
 * @return      None.
 */
void sAddrCpy(sAddr_t *pDest, const sAddr_t *pSrc)
{
  pDest->addrMode = pSrc->addrMode;

  if (pDest->addrMode == SADDR_MODE_EXT)
  {
    MAP_sAddrExtCpy(pDest->addr.extAddr, pSrc->addr.extAddr);
  }
  else
  {
    pDest->addr.shortAddr = pSrc->addr.shortAddr;
  }
}

/****************************************************************************
 * @fn          sAddrExtCmp
 *
 * @brief       Compare two extended addresses.
 *
 * input parameters
 *
 * @param       pAddr1        - Pointer to first address.
 * @param       pAddr2        - Pointer to second address.
 *
 * output parameters
 *
 * @return      TRUE if addresses are equal, FALSE otherwise
 */
bool sAddrExtCmp(const uint8 * pAddr1, const uint8 * pAddr2)
{
  uint8 i;

  for (i = SADDR_EXT_LEN; i != 0; i--)
  {
    if (*pAddr1++ != *pAddr2++)
    {
      return FALSE;
    }
  }
  return TRUE;
}

/****************************************************************************
 * @fn          sAddrExtCpy
 *
 * @brief       Copy an extended address.
 *
 * input parameters
 *
 * @param       pSrc         - Pointer to address to copy.
 *
 * output parameters
 *
 * @param       pDest        - Pointer to address of copy.
 *
 * @return      pDest + SADDR_EXT_LEN.
 */
void *sAddrExtCpy(uint8 * pDest, const uint8 * pSrc)
{
  return osal_memcpy(pDest, pSrc, SADDR_EXT_LEN);
}




