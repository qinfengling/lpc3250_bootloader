/***********************************************************************
 * $Id:: sys_hw.c 977 2008-07-29 18:30:15Z wellsk                      $
 *
 * Project: NXP PHY3250 startup code for stage 1
 *
 * Description:
 *     This file provides initialization code for the S1L.
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

#include "lpc_types.h"
#include "phy3250_board.h"
#include "phy3250_nand.h"
#include "lpc_arm922t_cp15_driver.h"
#include "lpc32xx_clkpwr_driver.h"
#include "s1l_sys_inf.h"
#include "lpc_string.h"
#include "s1l_sys.h"
#include "sys.h"
#include "phy3250_startup.h"

SYS_SAVED_DATA_T sys_saved_data;
volatile UNS_32 tick_10ms;

/* Invalidate instruction cache */
void icache_invalid(void)
{
	cp15_invalidate_cache();
}

/***********************************************************************
 *
 * Function: clock_adjust
 *
 * Purpose: Safely adjust or readjust system clocks from saved values
 *
 * Processing:
 *     Safely adjust the system clocks by placing DRAM in sleep mode,
 *     adjusting clocks, and restoring DRAM state.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: New clock frequency in Hz
 *
 * Notes: None
 *
 **********************************************************************/
UNS_32 clock_adjust(void)
{
	CLKPWR_HCLK_PLL_SETUP_T pllcfg;
	UNS_32 freqr;
	INT_32 idx;

	/* Place DRAM into self refresh mode */
	CLKPWR->clkpwr_pwr_ctrl |= CLKPWR_SDRAM_SELF_RFSH;
	CLKPWR->clkpwr_pwr_ctrl |= CLKPWR_UPD_SDRAM_SELF_RFSH;
	CLKPWR->clkpwr_pwr_ctrl &= ~CLKPWR_UPD_SDRAM_SELF_RFSH;
	CLKPWR->clkpwr_pwr_ctrl &= ~CLKPWR_AUTO_SDRAM_SELF_RFSH;
	while ((EMC->emcstatus & EMC_DYN_SELFRESH_MODE_BIT) == 0);

	/* Enter direct-run mode */
	clkpwr_set_hclk_divs(CLKPWR_HCLKDIV_DDRCLK_NORM, 1, 2);
	clkpwr_set_mode(CLKPWR_MD_DIRECTRUN);

	/* Find and set new PLL frequency */
	clkpwr_pll_dis_en(CLKPWR_HCLK_PLL, 0);
	clkpwr_find_pll_cfg(MAIN_OSC_FREQ, sys_saved_data.clksetup.armclk,
		10, &pllcfg);
    freqr = clkpwr_hclkpll_setup(&pllcfg);
	if (freqr != 0) 
	{
		/* Wait for PLL to lock before switching back into RUN mode */
		while (clkpwr_is_pll_locked(CLKPWR_HCLK_PLL) == 0);

		/* Switch out of direct-run mode and set new dividers */
		clkpwr_set_mode(CLKPWR_MD_RUN);
		clkpwr_set_hclk_divs(CLKPWR_HCLKDIV_DDRCLK_NORM,
			sys_saved_data.clksetup.pclkdiv,
			sys_saved_data.clksetup.hclkdiv);

		/* Take DRAM out of self-refresh */
		CLKPWR->clkpwr_pwr_ctrl &= ~CLKPWR_SDRAM_SELF_RFSH;
		CLKPWR->clkpwr_pwr_ctrl |= CLKPWR_UPD_SDRAM_SELF_RFSH;
		CLKPWR->clkpwr_pwr_ctrl &= ~CLKPWR_UPD_SDRAM_SELF_RFSH;
		while ((EMC->emcstatus & EMC_DYN_SELFRESH_MODE_BIT) != 0);

		/* Optimize timings */
		sram_adjust_timing();
		idx = (phyhwdesc.dramcfg & PHYHW_DRAM_SIZE_MASK) >> 2;
		sdram_adjust_timing(&dram_cfg[idx]);
		term_setbaud(syscfg.baudrate);
	}

	return freqr;
}

/***********************************************************************
 *
 * Function: svinfockgood
 *
 * Purpose: Verifies savedinfo structure is good
 *
 * Processing:
 *     See function.
 *
 * Parameters: TBD
 *
 * Outputs: None
 *
 * Returns: TRUE if the structure is good, otherwise FALSE
 *
 * Notes: None
 *
 **********************************************************************/
BOOL_32 svinfockgood(UNS_32 *buff,
							int bytes,
							UNS_32 verikey) 
{
	BOOL_32 goodinfo = FALSE;
	UNS_32 genck = 0;
	int sz = bytes / sizeof(UNS_32);
	while (sz > 0) 
	{
		genck = genck + *buff;
		buff++;
		sz--;
	}

	if ((genck ^ verikey) == 0xFFFFFFFF) 
	{
		goodinfo = TRUE;
	}

	return goodinfo;
}

/***********************************************************************
 *
 * Function: svinfockgen
 *
 * Purpose: Generate verification key for savedinfo structure
 *
 * Processing:
 *     See function.
 *
 * Parameters: TBD
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void svinfockgen(UNS_32 *buff,
						int bytes,
						UNS_32 *verikey) 
{
	UNS_32 genck = 0;
	int sz = bytes / sizeof(UNS_32);
	while (sz > 0) 
	{
		genck = genck + *buff;
		buff++;
		sz--;
	}

	*verikey = ~genck;
}

/***********************************************************************
 *
 * Function: sys_default
 *
 * Purpose: Sets default system configuration
 *
 * Processing:
 *     Sets the system configuration to the defaults.
 *
 * Parameters: TBD
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
static void sys_default(SYS_SAVED_DATA_T *pASysData)
{
	pASysData->clksetup.armclk = 208000000;
	pASysData->clksetup.hclkdiv = 2;
	pASysData->clksetup.pclkdiv = 16;
}

/***********************************************************************
 *
 * Function: nand_bootloader_update
 *
 * Purpose:
 *     Places a new bootloader into the first block(s).
 *
 * Processing:
 *     See function.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: TRUE if the NAND was updated, otherwise FALSE
 *
 * Notes: Will overwrite the bootloader. Be careful.
 *
 **********************************************************************/
BOOL_32 nand_bootloader_update(void) 
{
	UNS_32 nblks, fblk;
	int sector, towrite, idx, numsecs;
	UNS_32 sizes[2], block, page;
	NAND_GEOM_T lgeom;
	UNS_8 *p8, tmp[6*4];

	/* Initialize the MLC NAND controller */
	if (nand_init(&lgeom) == 0)
	{
		/* Cannot initialize MLC controller */
		return FALSE;
	}

	/* Erase the first blocks in the system, skipping bad blocks */
	fblk = 1;
	nblks = BL_NUM_BLOCKS - 1;
	while (nblks > 0) 
	{
		/* Read block first to get bad block marker */
		sector = nand_to_sector(fblk, 0);
		nand_read_sector(sector, secdat, &secdat[lgeom.bytes_per_page]);
		if (secdat [lgeom.bytes_per_page + NAND_LPCBADBLOCK_SPAREOFFS] ==
			NAND_GOOD_BLOCK_MARKER) 
		{
			nand_erase_block(fblk);
		}

		fblk++;
		nblks--;
	}

	/* The first sector contains the size of the image to load and
	   it's inverse */
	sizes[0] = sysinfo.lfile.num_bytes;
	sizes[1] = 0xFFFFFFFF - sysinfo.lfile.num_bytes;
	memcpy(secdat, sizes, sizeof(sizes));

	for (idx = 0; idx < 6; idx++)
	{
		tmp[idx] = 0xFF;
		tmp[6+idx] = 0xFF;
		tmp[(2*6)+idx] = 0xFF;
		tmp[(3*6)+idx] = 0xFF;
	}
	tmp[4] = 0xFE; /* Mark as reserved */
	tmp[1*6+4] = 0xFE; /* Mark as reserved */
	tmp[2*6+4] = 0xFE; /* Mark as reserved */
	tmp[3*6+4] = 0xFE; /* Mark as reserved */

	/* Write first sector with reserved flag */
	block = 1;
	page = 0;
	sector = nand_to_sector(block, page);
	nand_write_sector(sector, (UNS_8 *) secdat, tmp);

	/* Setup for the rest of the right */
	sector++;
	towrite = 1 + (sysinfo.lfile.num_bytes /
		sysinfo.nandgeom.data_bytes_per_sector);

	/* Setup write for empty pages */
	for (idx = 0; idx < sysinfo.nandgeom.data_bytes_per_sector; idx++)
	{
		secdat[idx] = 0xFF;
	}

	/* Write loop */
	p8 = (UNS_8 *) sysinfo.lfile.loadaddr;
	nblks = BL_NUM_BLOCKS - 1;

	/* Write the rest of the block 1 */
	numsecs = towrite;
	if (numsecs > (sysinfo.nandgeom.sectors_per_block - 1))
	{
		numsecs = sysinfo.nandgeom.sectors_per_block - 1;
	}
	if (towrite > 0)
	{
		nand_write_sectors(sector, numsecs, p8, FALSE);
		p8 += (numsecs * sysinfo.nandgeom.data_bytes_per_sector);
		towrite -= numsecs;
		sector += numsecs;
	}
	nblks--;
	block++;

	/* Do the rest of the blocks */
	while (towrite > 0)
	{
		sector = nand_to_sector(block, 0);

		/* Do block first so it can be reserved */
		nand_write_sector(sector, p8, tmp);
		p8 += sysinfo.nandgeom.data_bytes_per_sector;
		towrite--;
		sector++;

		/* Write the rest of the sectors for this block */
		numsecs = towrite;
		if (numsecs > (sysinfo.nandgeom.sectors_per_block - 1))
		{
			numsecs = sysinfo.nandgeom.sectors_per_block - 1;
		}
		nand_write_sectors(sector, numsecs, p8, FALSE);
		p8 += (numsecs * sysinfo.nandgeom.data_bytes_per_sector);
		towrite -= numsecs;
		nblks--;
		block++;
	}

	/* Mark remaining blocks */
	while (nblks > 0)
	{
		sector = nand_to_sector(block, 0);
		block++;
		nblks--;
		nand_write_sector(sector, (UNS_8 *) secdat, tmp);
	}

	return TRUE;
}

/***********************************************************************
 *
 * Function: nand_kickstart_update
 *
 * Purpose:
 *     Places a new kickstart loader into the first block.
 *
 * Processing:
 *     See function.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: TRUE if the NAND was updated, otherwise FALSE
 *
 * Notes: WIll overwrite the kickstart code in block 0. Be careful.
 *
 **********************************************************************/
BOOL_32 nand_kickstart_update(void) 
{
	UNS_16 tmp16, tmp16i, idcnt = 4, f16page [256*4];
	UNS_8 *f8page = (UNS_8 *) f16page, tmp[6*4];
	int idx, ptw, sector;
	NAND_GEOM_T lgeom;

	/* Initialize the MLC NAND controller */
	if (nand_init(&lgeom) == 0)
	{
		/* Cannot initialize MLC controller */
		return FALSE;
	}

	/* Note no bad block markers used in boot code */
	nand_erase_block(0);

	if(lgeom.bytes_per_page == 512)
	   idcnt = 1;

	/* Generate first page data for LPC3250 loader */
	for (idx = 0; idx < (256 * idcnt); idx++) 
	{
		f16page [idx] = 0x0000;
	}

    /* Setup FLASH config for small block, 3 or 4 address */
	if (sysinfo.nandgeom.addrcycles == 3) 
	{
		/* 3 address cycles per sector, small block */
	    tmp16 = 0xF0;
	}
	else
	{
		if( lgeom.bytes_per_page == 512 )
		{
			/* 4 address cycles per sector, small block */
	    tmp16 = 0xD2;
		}
		else
		{
			/* 4 address cycles per sector, large block */
			tmp16 = 0xB4;
		}
	}
    tmp16i = 0xFF - tmp16;
    f16page[0] = tmp16;
    f16page[2] = tmp16i;
    f16page[4] = tmp16;
    f16page[6] = tmp16i;

    /* Determine number of blocks to write */
    ptw = ((INT_32) sysinfo.lfile.num_bytes) / sysinfo.nandgeom.data_bytes_per_sector;
    if ((ptw * sysinfo.nandgeom.data_bytes_per_sector) < sysinfo.lfile.num_bytes)
    {
    	ptw++;
    }
   	ptw++; /* Include non-used sector */
    tmp16 = (UNS_16) ptw;
    tmp16i = 0x00FF - tmp16;
    f16page[8] = tmp16;
    f16page[10] = tmp16i;
    f16page[12] = tmp16;
    f16page[14] = tmp16i;
    f16page[16] = tmp16;
    f16page[18] = tmp16i;
    f16page[20] = tmp16;
    f16page[22] = tmp16i;
    f16page[24] = 0x00AA; /* Good block marker for page #0 ICR only */

   /* Get location where to write ICR */
    sector = 0;

	for (idx = 0; idx < 6; idx++)
	{
		tmp[idx] = 0xFF;
		tmp[6+idx] = 0xFF;
		tmp[(2*6)+idx] = 0xFF;
		tmp[(3*6)+idx] = 0xFF;
	}
	tmp[4] = 0xFE; /* Mark as reserved */
	tmp[1*6+4] = 0xFE; /* Mark as reserved */
	tmp[2*6+4] = 0xFE; /* Mark as reserved */
	tmp[3*6+4] = 0xFE; /* Mark as reserved */
	
	/* Write ICR data to first usable block/page #0 */
    nand_write_sector(sector, f8page, tmp);
    sector++;

	/* Write the rest of the data */
	nand_write_sectors(sector, ptw, (UNS_8 *) sysinfo.lfile.loadaddr, FALSE);

	return TRUE;
}

/***********************************************************************
 *
 * Function: sys_get
 *
 * Purpose: Get saved system information from storage
 *
 * Processing:
 *     Gets system information from storage
 *
 * Parameters: TBD
 *
 * Outputs: None
 *
 * Returns: TBD
 *
 * Notes: None
 *
 **********************************************************************/
BOOL_32 sys_get(SYS_SAVED_DATA_T *pASysData) 
{
	UNS_8 *psspcfg = (UNS_8 *) pASysData;
	BOOL_32 usestored;
	int idx,ll;

	/* Read configuration structure */
	ll=sizeof(SYS_SAVED_DATA_T);
	for (idx = 0; idx < sizeof(SYS_SAVED_DATA_T); idx++) 
	{
		*psspcfg = phy3250_sspread(PHY3250_SSEPROM_SINDEX +
			sizeof(S1L_CFG_T) + idx);
		psspcfg++;
	}

	/* Verify checksum */
	usestored = svinfockgood((UNS_32 *) pASysData,
		(sizeof(SYS_SAVED_DATA_T) - sizeof(UNS_32)), pASysData->verikey);
	if (usestored == FALSE)
	{
		sys_default(pASysData);
	}

	return usestored;
}

/***********************************************************************
 *
 * Function: sys_save
 *
 * Purpose: Saved system information to storage
 *
 * Processing:
 *     Saves system information to storage
 *
 * Parameters: TBD
 *
 * Outputs: None
 *
 * Returns: TBD
 *
 * Notes: None
 *
 **********************************************************************/
BOOL_32 sys_save(SYS_SAVED_DATA_T *pASysData) 
{
	int idx;
	UNS_8 *psspcfg = (UNS_8 *) pASysData;

	/* Update verification key first */
	svinfockgen((UNS_32 *) pASysData, (sizeof(SYS_SAVED_DATA_T) -
		sizeof(UNS_32)), &pASysData->verikey);

	/* Write configuration structure */
	for (idx = 0; idx < sizeof(SYS_SAVED_DATA_T); idx++) 
	{
		phy3250_sspwrite(*psspcfg, (PHY3250_SSEPROM_SINDEX +
			sizeof(S1L_CFG_T) +  idx));
		psspcfg++;
	}

	return TRUE;
}

/***********************************************************************
 *
 * Function: c_entry
 *
 * Purpose: Application entry point from the startup code
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
void c_entry(void) {
	BOOL_32 goto_prompt = TRUE;

	while (1) 
	{
		sys_get(&sys_saved_data);

		/* Go to boot manager */
		boot_manager(goto_prompt);
		goto_prompt = FALSE;
	}
}
