/***********************************************************************
 * $Id:: phy3250_startup.h 964 2008-07-28 17:54:10Z wellsk             $
 *
 * Project: Phytec LPC3250 board startup code
 *
 * Description:
 *     This package contains the startup functions that initialize the
 *     Phytec LPC3250 board into a known state.
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

#ifndef PHY3250_STARTUP_H
#define PHY3250_STARTUP_H

#ifdef __cplusplus
extern "C"
{
#endif

/***********************************************************************
 * Startup code support functions
 **********************************************************************/

/* Main startup code entry point, called from reset entry code */
void phy3250_init(void);

/* Flush data cache (and invalidate) */
void dcache_flush(BOOL_32 inval);

#ifdef __cplusplus
}
#endif

#endif /* PHY3250_STARTUP_H */
