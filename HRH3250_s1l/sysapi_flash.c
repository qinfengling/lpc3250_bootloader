/***********************************************************************
 * $Id:: sysapi_flash.c 901 2008-07-17 21:00:10Z wellsk                $
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
#include "phy3250_nand.h"
#include "lpc32xx_slcnand.h"
#include "lpc32xx_clkpwr.h"
#include "lpc32xx_clkpwr_driver.h"
#include "lpc_nandflash_params.h"
#include "phy3250_board.h"

/* NAND256R3A2CZA6 device timing values */
#define PHY_NAND_WDR         3
#define PHY_NAND_WWIDTH      28571428  // 35nS
#define PHY_NAND_WHOLD       100000000 // 10nS
#define PHY_NAND_WSETUP      66666666  // 15nS
#define PHY_NAND_RDR         3
#define PHY_NAND_RWIDTH      28571428  // 35nS
#define PHY_NAND_RHOLD       100000000 // 10nS
#define PHY_NAND_RSETUP      66666666  // 15nS

/* Alternate NAND command defines */
#define LPCNAND_CMD_PAGE_WRITE1   0x80
#define LPCNAND_CMD_PAGE_WRITE2   0x10
#define LPCNAND_CMD_PAGE_READA    0x00
#define LPCNAND_CMD_PAGE_READB    0x01
#define LPCNAND_CMD_PAGE_READC    0x50
#define LPCNAND_CMD_STATUS        0x70
#define LPCNAND_CMD_READ_ID       0x90
#define LPCNAND_CMD_ERASE1        0x60
#define LPCNAND_CMD_ERASE2        0xD0
#define LPCNAND_CMD_RESET         0xFF

static FLASH_GEOM_T nandgeom;
static UNS_32 addressCycles;
extern BOOL_8 islargeblock;

/* Lock or unlock NAND chip select signal */
static void nandCSLock(BOOL_32 lock)
{
	if (lock != FALSE)
	{
		SLCNAND->slc_cfg |= SLCCFG_CE_LOW;
	}
	else
	{
		SLCNAND->slc_cfg &= ~SLCCFG_CE_LOW;
	}
}

/* Check NAND ready */
static BOOL_32 nandCheckReady(void)
{
	BOOL_32 rdy = FALSE;

	if ((SLCNAND->slc_stat & SLCSTAT_NAND_READY) != 0)
	{
		rdy = TRUE;
	}

	return rdy;
}

/* Write FLASH address */
static void nandWriteAddress (UNS_8 *addr,
							  int bytes)
{
    int idx = 0;
    
    for (idx = 0; idx < bytes; idx++)
	{
		SLCNAND->slc_addr = (UNS_32) addr[idx];
    }
}

/* Write FLASH command */
static void nandWriteCommand (UNS_8 cmd)
{
    SLCNAND->slc_cmd = (UNS_32) cmd;
}

/* Write to FLASH data */
static void nandWriteData (void *data,
						   int bytes)
{
    int idx;
    UNS_8 *datab = (UNS_8 *) data;

	for (idx = 0; idx < bytes; idx++)
	{
        SLCNAND->slc_data = (UNS_32) datab[idx];
    }
}

/* Read from FLASH data */
static void nandReadData (void *addr,
						  int bytes)
{
    int idx;
    UNS_8 *datab = (UNS_8 *) addr;

	for (idx = 0; idx < bytes; idx++)
	{
        datab[idx] = (UNS_8) SLCNAND->slc_data;
    }
}

/* Wait for data ready from device */
static BOOL_32 nandWaitReady(UNS_32 to)
{
	BOOL_32 rdy = FALSE;

	UNS_32 ct = tick_10ms + to;

	while ((rdy == FALSE) && (ct > tick_10ms))
	{
		rdy = nandCheckReady();
	}

	return rdy;
}

/* Get an address from a sector number */
static void nandGetPageIndex(UNS_32 SectorAddr,
							 UNS_32 offset,
							 UNS_8 *addrbytes) {
    UNS_32 block, page, nandaddr;

	// Limit offset
    if (offset >= 256)
	{
        offset = 0;
    }

    // Determine block and page offsets from passed sector address
    block = SectorAddr / nandgeom.sectors_per_block;
    page = SectorAddr - (block * nandgeom.sectors_per_block);

	/*
	*	large block process,ignore offset param
	*/
	if( TRUE == islargeblock )
	{
		addrbytes[0] = (UNS_8)((0 >> 0) & 0x00FF);
		addrbytes[1] = (UNS_8) ((0 >> 8) & 0xFF00);
		addrbytes[2] = (UNS_8) (offset + ((page >> 0) & 0x003F)) |
	    ((block << 6) & 0x00C0);
		if (addressCycles == 4) 
		{
			addrbytes[3] = (UNS_8) ((block >> 2) & 0x00FF);
		}
		else
		if (addressCycles == 5) 
		{
			addrbytes[3] = (UNS_8) ((block >> 2) & 0x00FF);
			addrbytes[4] = (UNS_8) ((block >> 10) & 0x0001);
		}    
	}
	else
 	{
		// Block  Page  Index
		// 31..13 12..8 7..0
		nandaddr = offset + ((page & 0x1F) << 8);
		nandaddr = nandaddr | ((block & 0xFFF) << 13);
	
	    // Save block and page address
		addrbytes[0] = (UNS_8) ((nandaddr >> 0) & 0xFF);
		addrbytes[1] = (UNS_8) ((nandaddr >> 8) & 0xFF);
		addrbytes[2] = (UNS_8) ((nandaddr >> 16) & 0xFF);
		if (addressCycles == 4) 
		{
			addrbytes[3] = (UNS_8) ((nandaddr >> 24) & 0xFF);
		}
	}
}

/* Initializes/re-initializes FLASH */
static void flash_reinit(void)
{
	int idx;
	UNS_32 clk;

	/* Switch over to SLC if necessary */
	if ((CLKPWR->clkpwr_nand_clk_ctrl & CLKPWR_NANDCLK_SLCCLK_EN) != 0)
	{
		return;
	}

	/* Setup SLC mode and enable SLC clock */
	CLKPWR->clkpwr_nand_clk_ctrl = (CLKPWR_NANDCLK_SEL_SLC |
		CLKPWR_NANDCLK_SLCCLK_EN);

	// Reset SLC controller and setup for 8-bit mode, disable and clear interrupts
	SLCNAND->slc_ctrl = SLCCTRL_SW_RESET;
	for (idx = 0; idx < 200000; idx++);
	SLCNAND->slc_cfg = 0;
	SLCNAND->slc_ien = SLCSTAT_INT_RDY_EN;
	SLCNAND->slc_icr = (SLCSTAT_INT_TC | SLCSTAT_INT_RDY_EN);

	// Setup SLC timing based on current clock
	clk = clkpwr_get_base_clock_rate(CLKPWR_HCLK);
    SLCNAND->slc_tac = (
		SLCTAC_WDR(PHY_NAND_WDR) |
		SLCTAC_WWIDTH(clk / PHY_NAND_WWIDTH) |
		SLCTAC_WHOLD(clk / PHY_NAND_WHOLD) |
		SLCTAC_WSETUP(clk / PHY_NAND_WSETUP) |
		SLCTAC_RDR(PHY_NAND_RDR) |
		SLCTAC_RWIDTH(clk / PHY_NAND_RWIDTH) |
		SLCTAC_RHOLD(clk / PHY_NAND_RHOLD) |
		SLCTAC_RSETUP(clk / PHY_NAND_RSETUP));
}

/* Initialize NAND device and populate NAND structure */
BOOL_32 flash_init(FLASH_GEOM_T *pNand)
{
	NAND_GEOM_T	geom;

	if(nand_init(&geom))
	{
		if(pNand)
		{
			pNand->blocks = geom.num_blocks;
			pNand->sectors_per_block = geom.pages_per_block;
			pNand->data_bytes_per_sector = geom.bytes_per_page;
			pNand->addrcycles = geom.addrcycles;
		}
		return TRUE;
	}

	return FALSE;
}

/* De-initialize FLASH */
void flash_deinit(void)
{
	phy3250_nand_wp(FALSE);

	/* Disable clocks */
	CLKPWR->clkpwr_nand_clk_ctrl &= ~CLKPWR_NANDCLK_SLCCLK_EN;
}

/* Read a NAND sector */
BOOL_32 flash_read_sector(UNS_32 sector,
						  void *buff)
{
	UNS_8 addrbytes[5];

	if( islargeblock == TRUE )
	{
		if( 0 == nand_read_sector(sector, buff, NULL ))
		{
			return FALSE;
		}
		return TRUE;
	}
	else
	{
		flash_reinit();

		/* Wait until device is ready */
		if (nandWaitReady(5) == FALSE)
		{
			return FALSE;
		}

		/* Lock chip select */
	    nandCSLock(TRUE);

	    nandGetPageIndex(sector, 0, addrbytes);
		nandWriteCommand(LPCNAND_CMD_PAGE_READA);
	    nandWriteAddress(addrbytes, addressCycles);

		if (nandWaitReady(5) == FALSE)
		{
			nandCSLock(FALSE);
			return FALSE;
		}

	    /* Copy buffer from NAND */
		nandReadData(buff, 512);		

	    /* Unlock access and chip select */
		nandCSLock(FALSE);
	}
	return TRUE;
}

/* Write a NAND sector */
BOOL_32 flash_write_sector(UNS_32 sector,
						   void *buff)
{
	UNS_8 addrbytes[5];

	if( islargeblock == TRUE )
	{
		if( 0 == nand_write_sector( sector, buff, NULL ))
		{
			return FALSE;
		}
		return TRUE;
	}
	else
	{
		flash_reinit();

		/* Lock access and chip select */
		nandCSLock(TRUE);

		/* Setup write */
		nandGetPageIndex(sector, 0, addrbytes);
		nandWriteCommand(LPCNAND_CMD_PAGE_READA);
		nandWriteCommand(LPCNAND_CMD_PAGE_WRITE1);
		nandWriteAddress(addrbytes, addressCycles);
		        
		/* Copy buffer to NAND */
		nandWriteData(buff, 512);
		            
		nandWriteCommand(LPCNAND_CMD_PAGE_WRITE2);
		if (nandWaitReady(5) == FALSE)
		{
			nandCSLock(FALSE);
			return FALSE;
		}

		/* Unlock access and chip select */
		nandCSLock(FALSE);
	}
	
	return TRUE;
}

/* Erase a block */
BOOL_32 flash_erase_block(UNS_32 block,
						  BOOL_32 override)
{
	UNS_8 status, addrbytes[3];
	if( islargeblock == TRUE )
	{
		return nand_erase_block( block );
	}

	flash_reinit();

	// Lock access and chip select
    nandCSLock(TRUE);
    
    // Issue erase command with block address and wait
    nandWriteCommand(LPCNAND_CMD_ERASE1);

	// Generate block address
	addrbytes[0] = (unsigned char) ((block << 5) & 0x00E0);
	addrbytes[1] = (unsigned char) ((block >> 3) & 0x00FF);
	if (addressCycles == 4)
	{
		addrbytes[2] = (unsigned char) ((block >> 11) & 0x0003);
	}

	nandWriteAddress(addrbytes, (addressCycles - 1));
    nandWriteCommand(LPCNAND_CMD_ERASE2);
	if (nandWaitReady(10) == FALSE)
	{
		nandCSLock(FALSE);
		return FALSE;
	}
    
    // Get NAND operation status
    nandWriteCommand(LPCNAND_CMD_STATUS);
    nandReadData(&status, 1);
    
    // Unlock access and chip select
    nandCSLock(FALSE);
    
    if ((status & 0x1) == 0)
	{
        // Passed
        return TRUE;
    }

	return FALSE;
}

/* Returns TRUE if the current block is a bad block */
BOOL_32 flash_is_bad_block(UNS_32 block)
{
	UNS_8 addrbytes[5];
	BOOL_32 blkbad = FALSE;


	if( islargeblock == TRUE )
	{
		INT_32 sector;
		UNS_8 extrabuff [2048+64];
		sector = nand_to_sector(block, 0);
		nand_read_sector( sector, extrabuff, &extrabuff[2048] );
		if (extrabuff [2048 + NAND_LPCBADBLOCK_SPAREOFFS] != NAND_GOOD_BLOCK_MARKER)
		  return TRUE;

		return FALSE;
	}
	else
	{
		UNS_8 tmp[2];
		
		flash_reinit();
		/* Wait until device is ready */
	    if (nandWaitReady(5) == FALSE)
		{
			return FALSE;
		}

		/* Lock chip select */
	    nandCSLock(TRUE);

		block = block * nandgeom.sectors_per_block;
	    nandGetPageIndex(block, 5, addrbytes);
		nandWriteCommand(LPCNAND_CMD_PAGE_READC);
	    nandWriteAddress(addrbytes, addressCycles);
	    if (nandWaitReady(5) == FALSE)
		{
			nandCSLock(FALSE);
			return FALSE;
		}

	    /* Copy buffer from NAND */
	    nandReadData(tmp, 1);

	    /* Unlock access and chip select */
		nandCSLock(FALSE);

		if (tmp[0] != 0xFF)
		{
			blkbad = TRUE;
		}
	}
	return blkbad;
}

/* Marks a block as reserved, support function */
BOOL_32 flash_reserve_block(UNS_32 block)
{
	UNS_8 addrbytes[5];
	UNS_8 tmp[6*4];
	INT_32 sector;
	int idx;

	if( islargeblock == TRUE )
	{
		UNS_8 readbuff[2048+64];
		UNS_8 *spare = &readbuff[2048];
		sector = nand_to_sector(block, 0);
		if( 0 == nand_read_sector( sector, readbuff, &readbuff[2048] ))
		{
			return FALSE;
		}

		for (idx = 0; idx < 6; idx++)
		{
			spare [idx] = 0xFF;
			spare[6+idx] = 0xFF;
			spare[(2*6)+idx] = 0xFF;
			spare[(3*6)+idx] = 0xFF;
		}
		spare [4] = 0xFE;
		spare[1*6+4] = 0xFE;
		spare[2*6+4] = 0xFE;
		spare[3*6+4] = 0xFE;
		if( 0 == nand_write_sector( block, readbuff, spare ))
		{
			return FALSE;
		}
		
		return TRUE;
	}
	else
	{
		flash_reinit();
		/* Wait until device is ready */
	    if (nandWaitReady(5) == FALSE)
		{
			return FALSE;
		}

		/* Lock chip select */
	    nandCSLock(TRUE);

		block = block * nandgeom.sectors_per_block;
	    nandGetPageIndex(block, 0, addrbytes);
		nandWriteCommand(LPCNAND_CMD_PAGE_READC);
		nandWriteCommand(LPCNAND_CMD_PAGE_WRITE1);
		nandWriteAddress(addrbytes, addressCycles);

		// Setup write
		for (idx = 0; idx < 6; idx++)
		{
			tmp [idx] = 0xFF;
		}
		tmp [4] = 0xFE;
		nandWriteData(&tmp, 6);
	    nandWriteCommand(LPCNAND_CMD_PAGE_WRITE2);
		if (nandWaitReady(5000) == FALSE)
		{
			nandCSLock(FALSE);
			return FALSE;
		}

	    /* Unlock access and chip select */
		nandCSLock(FALSE);
	}
	return TRUE;
}

/* Returns TRUE if the current block is a reserved block */
BOOL_32 flash_is_rsv_block(UNS_32 block)
{
	UNS_8 addrbytes[5];
	UNS_8 tmp[6];
	BOOL_32 rsvd = FALSE;

	if( islargeblock == TRUE )
	{
		UNS_8 readbuff[2048+64];
		UNS_8 *spare = &readbuff[2048];
		if( 0 == nand_read_sector( block, readbuff, &readbuff[2048] ))
		{
			return FALSE;
		}
		
		if ((spare[4] & 0x01) == 0)
		{
			rsvd = TRUE;
		}
		return rsvd;
	}
	else
	{
		flash_reinit();
		/* Wait until device is ready */
	    if (nandWaitReady(5) == FALSE)
		{
			return FALSE;
		}

		/* Lock chip select */
	    nandCSLock(TRUE);

		block = block * nandgeom.sectors_per_block;
	    nandGetPageIndex(block, 0, addrbytes);
		nandWriteCommand(LPCNAND_CMD_PAGE_READC);
	    nandWriteAddress(addrbytes, addressCycles);
	    if (nandWaitReady(5) == FALSE)
		{
			nandCSLock(FALSE);
			return FALSE;
		}

	    /* Copy buffer from NAND */
	    nandReadData(tmp, 6);

	    /* Unlock access and chip select */
		nandCSLock(FALSE);

		if ((tmp[4] & 0x01) == 0)
		{
			rsvd = TRUE;
		}
	}

	return rsvd;
}
