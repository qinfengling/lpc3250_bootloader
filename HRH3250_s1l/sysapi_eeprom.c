/***********************************************************************
 * $Id:: sysapi_eeprom.c 875 2008-07-08 17:27:04Z wellsk               $
 *
 * Project: System configuration save and restore functions
 *
 * Description:
 *     Implements the following functions required for the S1L API
 *         cfg_save
 *         cfg_load
 *         cfg_override
 *
 ***********************************************************************
 * Software that is described herein is for illustrative purposes only  
 * which provides customers with programming information regarding the  
 * products. This software is supplied "AS IS" without any warranties.  
 * NXP Semiconductors assumes no responsibility or liability for the 
 * use of the software, conveys no license or title under any patent, 
 * copyright, or mask work right to the product. NXP Semiconductors 
 * reserves the right to make changes in the software without 
 * notification. NXP Semiconductors also make no representation or 
 * warranty that such application will be suitable for the specified 
 * use without further testing or modification. 
 **********************************************************************/

#include "lpc_string.h"
#include "sys.h"
#include "s1l_sys.h"
#include "s1l_sys_inf.h"
#include "phy3250_board.h"

/***********************************************************************
 *
 * Function: cfg_save
 *
 * Purpose: Save a S1L configuration to non-volatile memory
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     pCfg : Pointer to config structure to save
 *
 * Outputs: None
 *
 * Returns: Always returns TRUE
 *
 * Notes: None
 *
 **********************************************************************/
BOOL_32 cfg_save(S1L_CFG_T *pCfg)
{
	int idx;
	UNS_8 *psspcfg = (UNS_8 *) pCfg;

	/* Update verification key first */
	svinfockgen((UNS_32 *) pCfg, (sizeof(S1L_CFG_T) - sizeof(UNS_32)),
		&pCfg->verikey);

	/* Write configuration structure */
	for (idx = 0; idx < sizeof(S1L_CFG_T); idx++) 
	{
		phy3250_sspwrite(*psspcfg, (PHY3250_SSEPROM_SINDEX + idx));
		psspcfg++;
	}

	return TRUE;
}

/***********************************************************************
 *
 * Function: cfg_load
 *
 * Purpose: Load an S1L conmfiguariton from non-volatile memory
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     pCfg : Pointer to config structure to populate
 *
 * Outputs: None
 *
 * Returns: FALSE if the structure wasn't loaded, otherwise TRUE
 *
 * Notes: None
 *
 **********************************************************************/
BOOL_32 cfg_load(S1L_CFG_T *pCfg)
{
	UNS_8 *psspcfg = (UNS_8 *) pCfg;
	BOOL_32 usestored;
	int idx;

	/* Read configuration structure */
	for (idx = 0; idx < sizeof(S1L_CFG_T); idx++) 
	{
		*psspcfg = phy3250_sspread(PHY3250_SSEPROM_SINDEX + idx);
		psspcfg++;
	}

	/* Verify checksum */
	usestored = svinfockgood((UNS_32 *) pCfg,
		(sizeof(S1L_CFG_T) - sizeof(UNS_32)), pCfg->verikey);

	return usestored;
}

/***********************************************************************
 *
 * Function: cfg_load
 *
 * Purpose:
 *     If this is returned as TRUE (when S1L is booted), the S1L
 *     configuration will return to the default configuration, can be
 *     tied to a pushbutton state.
 *
 * Processing:
 *     See function.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: TRUE if the config reset button is pressed, otherwise FALSE
 *
 * Notes: None
 *
 **********************************************************************/
BOOL_32 cfg_override(void)
{
	BOOL_32 pressed = FALSE;

	if (phy3250_button_state(PHY3250_BTN1) != 0)
	{
		pressed = TRUE;
	}

	return pressed;
}

/***********************************************************************
 *
 * Function: cfg_user_reset
 *
 * Purpose: Reset user configuration data
 *
 * Processing:
 *     See function.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
**********************************************************************/
void cfg_user_reset(void)
{
	sys_saved_data.clksetup.armclk = 208000000;
	sys_saved_data.clksetup.hclkdiv = 2;
	sys_saved_data.clksetup.pclkdiv = 16;

	/* Update saved system configuration */
	sys_save(&sys_saved_data);

	/* Update system clock settings */
	clock_adjust();
}
