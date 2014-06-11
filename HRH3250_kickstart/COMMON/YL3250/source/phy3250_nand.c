/***********************************************************************
 * $Id:: phy3250_nand.c 971 2008-07-28 21:03:08Z wellsk                $
 *
 * Project: Phytec LPC3250 NAND device definitions
 *
 * Description:
 *     This file contains board specific NAND functions.
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

#include "lpc_nandflash_params.h"
#include "phy3250_board.h"
#include "phy3250_nand.h"
#include "lpc32xx_mlcnand.h"
#include "lpc32xx_clkpwr_driver.h"
#include "lpc32xx_gpio_driver.h"

/***********************************************************************
 * Driver data
 **********************************************************************/

/* Driver data structure */
typedef struct
{
  NAND_GEOM_T geom;
} NAND_DRVDATA_T;
static NAND_DRVDATA_T nandrcdat;
BOOL_8 islargeblock = FALSE;
	
/***********************************************************************
 * Private functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: mlc_cmd
 *
 * Purpose: Issue a command to the MLC NAND device
 *
 * Processing:
 *     Write the command to the MLC command register.
 *
 * Parameters:
 *     cmd : Command to issue
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void mlc_cmd(UNS_8 cmd)
{
  MLCNAND->mlc_cmd = (UNS_32) cmd;
}

/***********************************************************************
 *
 * Function: mlc_addr
 *
 * Purpose: Issue a address to the MLC NAND device
 *
 * Processing:
 *     Write the address to the MLC address register.
 *
 * Parameters:
 *     addr : Address to issue
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void mlc_addr(UNS_8 addr)
{
  MLCNAND->mlc_addr = (UNS_32) addr;
}

/***********************************************************************
 *
 * Function: mlc_wait_ready
 *
 * Purpose: Wait for device to go to the ready state
 *
 * Processing:
 *     Loop until the ready status is detected.
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
void mlc_wait_ready(void)
{
  while ((MLCNAND->mlc_isr & MLC_DEV_RDY_STS) == 0);
}

/***********************************************************************
 *
 * Function: mlc_get_status
 *
 * Purpose: Return the current NAND status
 *
 * Processing:
 *     Issue the read status command to the device. Wait until the
 *     device is ready. Read the current status and return it to the
 *     caller.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: The status read from the NAND device
 *
 * Notes: None
 *
 **********************************************************************/
UNS_8 mlc_get_status(void)
{
  mlc_cmd(LPCNAND_CMD_STATUS);
  mlc_wait_ready();

  return (UNS_8) MLCNAND->mlc_data [0];
}

/***********************************************************************
 *
 * Function: mlc_write_addr
 *
 * Purpose: Converts the passed page and block to address write data
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     block : Block for address
 *     page  : Page for address
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void mlc_write_addr(INT_32 block,
                    INT_32 page)
{
  UNS_32 nandaddr;

  /* Block  Page  Index */
  /* 31..13 12..8 7..0  */

  if( TRUE == islargeblock )
  {
		mlc_addr((UNS_8) ((0 >> 0) & 0x00FF));
		mlc_addr((UNS_8) ((0 >> 8) & 0xFF00));
		mlc_addr((UNS_8) (((page >> 0) & 0x003F)) |
		    ((block << 6) & 0x00C0));
		mlc_addr((UNS_8) ((block >> 2) & 0x00FF));
		mlc_addr((UNS_8) ((block >> 10) & 0x0001));
  }
  else
  {
	  nandaddr = (page & 0x1F) << 8;
	  nandaddr = nandaddr | ((block & 0xFFF) << 13);

	  /* Write block and page address */
	  mlc_addr((UNS_8)(nandaddr >> 0)  & 0xFF);
	  mlc_addr((UNS_8)(nandaddr >> 8)  & 0xFF);
	  mlc_addr((UNS_8)(nandaddr >> 16) & 0xFF);
	  if (nandrcdat.geom.addrcycles == 4)
	  {
	    mlc_addr((UNS_8)(nandaddr >> 24) & 0xFF);
	  }
  }
}

/***********************************************************************
 *
 * Function: mlc_write_page
 *
 * Purpose: Write some data to the NAND device
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     block : Block to write
 *     page  : Page to write
 *     buff  : Pointer to the write data buffer
 *     extrabuff : Pointer to the spare data area buffer (NULL ok)
 *
 * Outputs: None
 *
 * Returns: Number of bytes written
 *
 * Notes: This should never write a non-bad block or page!
 *
 **********************************************************************/
INT_32 mlc_write_page(INT_32 block,
                      INT_32 page,
                      UNS_8 *buff,
                      UNS_8 *extrabuff)
{
  INT_32 bytes = 0, idx, idcnt = 4, towrite;
  UNS_8 status;
  volatile UNS_32 tmp;

  /*
	small block 
	The sequence must be modified follows:
	1. Write Serial Input command (0x80) to Command register.
	2. Write page address data to Address register.
	3. Write Start Encode register.
	4. Write 518 bytes of NAND data.
	5. Write MLC NAND Write Parity register.
	6. Read Status register.18
	7. Wait controller Ready status bit set.
	8. Write Auto Program command to Command register.

	largeblock 
	The sequence must be modified follows:
	1. Write Serial Input command (0x80) to Command register.
	2. Write page address data to Address register.
	3. Write Start Encode register.
	4. Write 518 bytes of NAND data.
	5. Write MLC NAND Write Parity register.
	6. Read Status register.18
	7. Wait controller Ready status bit set.
	8. Repeat 3 to 7 for 2nd quarter page.
	9. Repeat 3 to 7 for 3rd quarter page.
	10. 10) Repeat 3 to 7 for 4th quarter page.
	11. Write Auto Program command to Command register.
  */

  /* Force nCE for the entire cycle */
  MLCNAND->mlc_ceh = MLC_NORMAL_NCE;

   /* Issue page write1 command */
    if( FALSE == islargeblock )
    {
        mlc_cmd(LPCNAND_CMD_PAGE_READ1);
        idcnt = 1;
    }

    mlc_cmd(LPCNAND_CMD_PAGE_WRITE1);

    /* Write block and page address */
	mlc_write_addr(block, page);

    /* idcnt sub-pages of 518 bytes each = 2072 bytes */
    for (idx = 0; idx < idcnt; idx++) 
    {
        /* Start encode */
        MLCNAND->mlc_enc_ecc = 1;

        /* Write 512 bytes of data */
	    towrite = 512;
	    while (towrite > 0)
    	{
			tmp = *buff;
			buff++;
			tmp |= (*buff << 8);
			buff++;
			tmp |= (*buff << 16);
			buff++;
			tmp |= (*buff << 24);
			buff++;
    		MLCNAND->mlc_data [0] = tmp;
    		towrite = towrite - 4;
	    }
   		bytes = bytes + 512;

	    /* Write 6 dummy bytes */
	if (extrabuff == NULL)
  {
    MLCNAND->mlc_data [0] = 0xFFFFFFFF;
    * (volatile UNS_16 *) &MLCNAND->mlc_data [0] = 0xFFFF;
  }
  else
  {
    tmp = (*extrabuff | (*(extrabuff + 1) << 8) |
           (*(extrabuff + 2) << 16) | (*(extrabuff + 3) << 24));
    MLCNAND->mlc_data [0] = tmp;
	    extrabuff += 4;
	    tmp = (*extrabuff | (*(extrabuff + 1) << 8));
	    * (volatile UNS_16 *) &MLCNAND->mlc_data [0] = (UNS_16) tmp;
		extrabuff += 2;
	  }

	  /* Write NAND parity */
	  MLCNAND->mlc_wpr = 1;

		/* Wait for controller ready */
		while ((MLCNAND->mlc_isr & MLC_CNTRLLR_RDY_STS) == 0);
    }

    /* Issue page write2 command */
    mlc_cmd(LPCNAND_CMD_PAGE_WRITE2);

    /* Deassert nCE */
    MLCNAND->mlc_ceh = 0;

	/* Wait for device ready */
	mlc_wait_ready();

    /* Read status */
    status = mlc_get_status();
    if ((status & 0x1) != 0)
    {
    	/* Program was not good */
    	bytes = 0;
    }

  return bytes;
}

/***********************************************************************
 *
 * Function: mlc_read_page
 *
 * Purpose: Read some data from the NAND device
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     block : Block to read
 *     page  : Page to read
 *     buff  : Pointer to buffer to read data into
 *     extrabuff: Pointer to where to place extra data >= 8 bytes
 *
 * Outputs: None
 *
 * Returns: Number of bytes read
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 mlc_read_page(INT_32 block,
                     INT_32 page,
                     UNS_8 *buff,
                     UNS_8 *extrabuff)
{
  INT_32 bytes = 0, idx, idcnt = 4, toread;
  volatile UNS_32 tmp;
  volatile UNS_16 tmp16;

  /*
	small block 
	1. Write Read Mode (1) command (0x00) to Command register.
	2. Write Read Start command (0x30) to Command register.
	3. Write address data to Address register.
	4. Read controller’s Status register.
	5. Wait until NAND Ready status bit set.
	6. Write Start Decode register.
	7. Read 518 NAND data bytes.
	8. Write Read Parity register.
	9. Read Status register.10
	10. Wait until ECC Ready status bit set.
	11. Check error detection/correction status.11
	12. If error was detected, read 518/528 bytes from serial Data Buffer.

	large block
	1. Write Read Mode (1) command (0x00) to Command register.
	2. Write Read Start command (0x30) to Command register.
	3. Write address data to Address register.
	4. Read controller’s Status register.
	5. Wait until NAND Ready status bit set.
	6. Write Start Decode register.
	7. Read 518 NAND data bytes.
	8. Write Read Parity register.
	9. Read Status register.10
	10. Wait until ECC Ready status bit set.
	11. Check error detection/correction status.11
	12. If error was detected, read 518/528 bytes from serial Data Buffer.
	13. Repeat steps 6 to 12 for 2nd quarter page.
	14. Repeat steps 6 to 12 for 3rd quarter page.
	15. Repeat steps 6 to 12 for 4th quarter page.
  */

  /* Force nCE for the entire cycle */
  MLCNAND->mlc_ceh = MLC_NORMAL_NCE;

    /* Issue page read1 command */
    mlc_cmd(LPCNAND_CMD_PAGE_READ1);

    /* Write block and page address */
	mlc_write_addr(block, page);
    if( TRUE == islargeblock )
    {
        mlc_addr((UNS_8) ((0 >> 0) & 0x00FF));

        /* Issue page read2 command */
        mlc_cmd(LPCNAND_CMD_PAGE_READ2);
    }
    else
    {
        idcnt = 1;
    }
	/* Wait for ready */
	mlc_wait_ready();

    /* idcnt sub-blocks of 512 bytes each */
    for (idx = 0; idx < idcnt; idx++) 
    {
	    /* Start decode */
	    MLCNAND->mlc_dec_ecc = 1;

	    /* Read data */
    	toread = 512;
	    while (toread > 0)
    	{
			tmp = MLCNAND->mlc_data [0];
	    	*buff = (UNS_8) ((tmp >> 0) & 0xFF);
    		buff++;
    		*buff = (UNS_8) ((tmp >> 8) & 0xFF);
	    	buff++;
    		*buff = (UNS_8) ((tmp >> 16) & 0xFF);
    		buff++;
	    	*buff = (UNS_8) ((tmp >> 24) & 0xFF);
    		buff++;
    		toread = toread - 4;
	    }
	    bytes = bytes + 512;

  /* Read 6 dummy bytes */
  if (extrabuff == NULL)
  {
    tmp = MLCNAND->mlc_data [0];
    tmp16 = * (volatile UNS_16 *) & MLCNAND->mlc_data [0];
  }
  else
  {
    tmp = MLCNAND->mlc_data [0];
    *extrabuff = (UNS_8)((tmp >> 0) & 0xFF);
    extrabuff++;
    *extrabuff = (UNS_8)((tmp >> 8) & 0xFF);
    extrabuff++;
    *extrabuff = (UNS_8)((tmp >> 16) & 0xFF);
    extrabuff++;
    *extrabuff = (UNS_8)((tmp >> 24) & 0xFF);
    extrabuff++;
    tmp16 = * (volatile UNS_16 *) & MLCNAND->mlc_data [0];
    *extrabuff = (UNS_8)((tmp16 >> 0) & 0xFF);
    extrabuff++;
    *extrabuff = (UNS_8)((tmp16 >> 8) & 0xFF);
	extrabuff++;
  }
        /* Write read parity register */
	    MLCNAND->mlc_rpr = 1;

		/* Wait for ECC ready */
		while ((MLCNAND->mlc_isr & MLC_ECC_RDY_STS) == 0);
    }

    /* Deassert nCE */
    MLCNAND->mlc_ceh = 0;

	/* Wait for device ready */
	mlc_wait_ready();

  return bytes;
}

/***********************************************************************
 *
 * Function: mlc_nand_detect
 *
 * Purpose: Detect and initialize MLC NAND device
 *
 * Processing:
 *     Does nothing
 *
 * Parameters:
 *     geom : Pointer to geometry structure to fill
 *
 * Outputs: None
 *
 * Returns: '1' if a device was found, otherwise '0'
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 mlc_nand_detect(NAND_GEOM_T *geom)
{
  UNS_8 id[4];
  volatile UNS_32 tmp;
  INT_32 init = 0;

  /* Reset NAND device and wait for ready */
  mlc_cmd(LPCNAND_CMD_RESET);
  mlc_wait_ready();

  /* Reset buffer pointer */
  MLCNAND->mlc_rubp = 0x1;

  /* Read the device ID */
  mlc_cmd(LPCNAND_CMD_READ_ID);
  mlc_addr(0);
  tmp = MLCNAND->mlc_data [0];
  id [0] = (UNS_8)((tmp >> 0) & 0xFF);
  id [1] = (UNS_8)((tmp >> 8) & 0xFF);
  id [2] = (UNS_8)((tmp >> 16) & 0xFF);
  id [3] = (UNS_8)((tmp >> 24) & 0xFF);

  /* Verify ID */
  if (id [0] == LPCNAND_VENDOR_STMICRO)
  {
    nandrcdat.geom.pages_per_block = 32;
	nandrcdat.geom.bytes_per_page  = 512;
    nandrcdat.geom.extra_per_page  = 16;
	islargeblock = FALSE;
    switch (id [1])
    {
      case 0x73:
        /* NAND128-A */
        init = 1;
        nandrcdat.geom.num_blocks = 1024;
        nandrcdat.geom.addrcycles = 3;
        break;

      case 0x35:
      case 0x75:
        /* NAND256-A */
        init = 1;
        nandrcdat.geom.num_blocks = 2048;
        nandrcdat.geom.addrcycles = 3;
        break;

      case 0x36:
      case 0x76:
        /* NAND512-A */
        init = 1;
        nandrcdat.geom.num_blocks = 4096;
        nandrcdat.geom.addrcycles = 4;
        break;

      case 0x39:
      case 0x79:
        /* NAND01G-A */
        init = 1;
        nandrcdat.geom.num_blocks = 8192;
        nandrcdat.geom.addrcycles = 4;
        break;

	case 0xF1:
		/* 1024MBit large block*/
		init = 1;
		nandrcdat.geom.num_blocks = 1024;
		nandrcdat.geom.addrcycles = 4;

		nandrcdat.geom.pages_per_block = 64;
		nandrcdat.geom.bytes_per_page  = 2048;
		nandrcdat.geom.extra_per_page  = 64;
		islargeblock = TRUE;
		break;

      default:
        break;
    }

    *geom = nandrcdat.geom;
  }

  return init;
}

/***********************************************************************
 * Public functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: nand_clock_setup
 *
 * Purpose: Setup NAND timing
 *
 * Processing:
 *     Sets up NAND controller and device timing.
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
void nand_clock_setup(void)
{
  UNS_32 clk;

  /* Setup MLC timing - timing will not vary based on current NAND
     controller clock and are based on a 104MHz NAND clock */
  MLCNAND->mlc_lock_pr = MLC_UNLOCK_REG_VALUE;

	if( TRUE == islargeblock )
	{
    MLCNAND->mlc_time = (MLC_LOAD_TCEA(0x03) | MLC_LOAD_TWBTRB(0x0F) |
        MLC_LOAD_TRHZ(0x03) | MLC_LOAD_TREH(0x07) |
        MLC_LOAD_TRP(0x07) | MLC_LOAD_TWH(0x07) | MLC_LOAD_TWP(0x07));
	}
	else
	{
		/* Get NAND controller clock */
		clk = clkpwr_get_clock_rate(CLKPWR_NAND_MLC_CLK);
		
	  MLCNAND->mlc_time = (MLC_LOAD_TCEA(1 + (clk / PHY_NAND_TCEA_DELAY)) |
	    MLC_LOAD_TWBTRB(1 + (clk / PHY_NAND_BUSY_DELAY)) |
	    MLC_LOAD_TRHZ(1 + (clk / PHY_NAND_NAND_TA)) |
	    MLC_LOAD_TREH(1 + (clk / PHY_NAND_RD_HIGH)) |
	    MLC_LOAD_TRP(1 + (clk / PHY_NAND_RD_LOW)) |
	    MLC_LOAD_TWH(1 + (clk / PHY_NAND_WR_HIGH)) |
	    MLC_LOAD_TWP(1 + (clk / PHY_NAND_WR_LOW)));
	}
}

/***********************************************************************
 *
 * Function: nand_init
 *
 * Purpose: Initialize NAND and get NAND gemoetry on the MLC interface
 *
 * Processing:
 *     Does nothing
 *
 * Parameters:
 *     geom     : Pointer to geometry structure to fill
 *
 * Outputs: None
 *
 * Returns: '1' if a device was found, otherwise '0'
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 nand_init(NAND_GEOM_T *geom)
{
  /* Enable MLC controller clock and setup MLC mode */
  clkpwr_setup_nand_ctrlr(0, 0, 0);
  clkpwr_clk_en_dis(CLKPWR_NAND_MLC_CLK, 1);

  /* Configure controller for no software write protection, x8 bus
     width, large block device, and 4 address words */
  MLCNAND->mlc_lock_pr = MLC_UNLOCK_REG_VALUE;
  MLCNAND->mlc_icr = (MLC_LARGE_BLK_ENABLE | MLC_ADDR4_ENABLE);

  /* Make sure MLC interrupts are disabled */
  MLCNAND->mlc_irq_mr = 0;

  /* Normal chip enable operation */
  MLCNAND->mlc_ceh = MLC_NORMAL_NCE;

  /* Setup NAND timing */
  nand_clock_setup();

  /* Detect device */
  return mlc_nand_detect(geom);
}

/***********************************************************************
 *
 * Function: nand_erase_block
 *
 * Purpose: Erase a NAND block
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     block : Block to erase
 *
 * Outputs: None
 *
 * Returns: '1' if the block was erased, otherwise '0'
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 nand_erase_block(INT_32 block)
{
  UNS_8 status;
  INT_32 erased = 0;

  /* Issue block erase1 command */
  mlc_cmd(LPCNAND_CMD_ERASE1);

	if( TRUE == islargeblock )
	{
		/* Write block and page address */
		mlc_addr((UNS_8) ((block << 6) & 0x00C0));
		mlc_addr((UNS_8) ((block >> 2) & 0x00FF));
		mlc_addr((UNS_8) ((block >> 10) & 0x0001));
	}
	else
	{
	  /* Write block and page address */
	  mlc_addr((UNS_8)((block << 5) & 0x00E0));
	  mlc_addr((UNS_8)((block >> 3) & 0x00FF));
	  if (nandrcdat.geom.addrcycles == 4)
	  {
	    mlc_addr((UNS_8)((block >> 11) & 0x0003));
	  }
	}
	
  /* Issue page erase2 command */
  mlc_cmd(LPCNAND_CMD_ERASE2);

  /* Wait for ready */
  mlc_wait_ready();

  /* Read status */
  status = mlc_get_status();
  if ((status & 0x1) == 0)
  {
    /* Erase was good */
    erased = 1;
  }

  return erased;
}

/***********************************************************************
 *
 * Function: nand_read_sector
 *
 * Purpose: Read a NAND sector
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     sector   : Sector to read
 *     readbuff : Pointer to read buffer >= 512 bytes
 *     extrabuff: Pointer to where to place extra data, 6 bytes
 *
 * Outputs: None
 *
 * Returns: Always returns 512
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 nand_read_sector(INT_32 sector,
                        UNS_8 *readbuff,
                        UNS_8 *extrabuff)
{
  INT_32 block, page;

  /* Translate to page/block address */
  sector_to_nand(sector, &block, &page);

  return mlc_read_page(block, page, readbuff, extrabuff);
}

/***********************************************************************
 *
 * Function: nand_write_sector
 *
 * Purpose: Write a NAND sector
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     sector   : Sector to write
 *     readbuff : Pointer to write buffer >= 512/2048 bytes
 *     extrabuff: Extra data to write to spare area, 6 bytes
 *
 * Outputs: None
 *
 * Returns: Returns 512/2048, or 0 if a write error occurs
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 nand_write_sector(INT_32 sector,
                         UNS_8 *writebuff,
                         UNS_8 *extrabuff)
{
  INT_32 block, page;

  /* Translate to page/block address */
  sector_to_nand(sector, &block, &page);

  return mlc_write_page(block, page, writebuff, extrabuff);
}

/***********************************************************************
 *
 * Function: nand_read_sectors
 *
 * Purpose: Read a series of NAND sectors, optionally skip bad blocks
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     sector_start : Starting sector to read
 *     num_sectors  : Number of sectors to read
 *     readbuff     : Data buffer for read data
 *     skipbad      : TRUE to skip bad sectors, FALSE reads all sectors
 *
 * Outputs: None
 *
 * Returns: Returns number of bytes read, or 0 if a write error occurs
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 nand_read_sectors(INT_32 sector_start,
                         INT_32 num_sectors,
                         UNS_8 *readbuff,
                         BOOL_32 skipbad)
{
  UNS_8 extrabuff [2048 + 64];
  BOOL_32 checkblk = skipbad;
  INT_32 block, page, bread = 0;

  /* Translate to page/block address */
  sector_to_nand(sector_start, &block, &page);

  /* Read all sectors */
  while (num_sectors > 0)
  {
    /* Is a block read needed? */
    while (checkblk == TRUE)
    {
      mlc_read_page(block, 0, extrabuff, &extrabuff [nandrcdat.geom.bytes_per_page]);
      if (extrabuff [nandrcdat.geom.bytes_per_page + NAND_LPCBADBLOCK_SPAREOFFS] !=
          NAND_GOOD_BLOCK_MARKER)
      {
        /* Block is bad, skip to next block */
        block++;
        page = 0;
        checkblk = TRUE;
      }
      else
      {
        checkblk = FALSE;
      }
    }

    /* Read sector */
    mlc_read_page(block, page, readbuff, NULL);
    readbuff += nandrcdat.geom.bytes_per_page;
    bread += nandrcdat.geom.bytes_per_page;
    page++;
    if (page >= nandrcdat.geom.pages_per_block)
    {
      page = 0;
      block++;
      checkblk = skipbad;
    }

    num_sectors--;
  }

  return bread;
}

/***********************************************************************
 *
 * Function: nand_write_sectors
 *
 * Purpose: Write a series of NAND sectors, optionally skip bad blocks
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     sector_start : Starting sector to write
 *     num_sectors  : Number of sectors to write
 *     readbuff     : Data buffer for write data
 *     skipbad      : TRUE to skip bad sectors, FALSE write all sectors
 *
 * Outputs: None
 *
 * Returns: Returns number of bytes written, or 0 if a write error occurs
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 nand_write_sectors(INT_32 sector_start,
                          INT_32 num_sectors,
                          UNS_8 *writebuff,
                          BOOL_32 skipbad)
{
  UNS_8 extrabuff [2048 + 64];
  BOOL_32 checkblk = skipbad;
  INT_32 block, page, bwrite = 0;

  /* Translate to page/block address */
  sector_to_nand(sector_start, &block, &page);

  /* Write all sectors */
  while (num_sectors > 0)
  {
    /* Is a block read needed? */
    while (checkblk == TRUE)
    {
      mlc_read_page(block, 0, extrabuff, &extrabuff [nandrcdat.geom.bytes_per_page]);
      if (extrabuff [nandrcdat.geom.bytes_per_page + NAND_LPCBADBLOCK_SPAREOFFS] !=
          NAND_GOOD_BLOCK_MARKER)
      {
        /* Block is bad, skip to next block */
        block++;
        page = 0;
        checkblk = TRUE;
      }
      else
      {
        checkblk = FALSE;
      }
    }

    /* Write sector */
    mlc_write_page(block, page, writebuff, NULL);
    writebuff += nandrcdat.geom.bytes_per_page;
    bwrite += nandrcdat.geom.bytes_per_page;
    page++;
    if (page >= nandrcdat.geom.pages_per_block)
    {
      page = 0;
      block++;
      checkblk = skipbad;
    }

    num_sectors--;
  }

  return bwrite;
}

/***********************************************************************
 *
 * Function: nand_to_sector
 *
 * Purpose: Translate a block and page address to a sector
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     block : Block number
 *     page  : Page number
 *
 * Outputs: None
 *
 * Returns: The mapped sector number
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 nand_to_sector(INT_32 block,
                      INT_32 page)
{
  return (page + (block * nandrcdat.geom.pages_per_block));
}

/***********************************************************************
 *
 * Function: sector_to_nand
 *
 * Purpose: Translate a sector address to a block and page address
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     sector : Sector number
 *     block  : Pointer to block number to fill
 *     page   : Pointer to page number
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void sector_to_nand(INT_32 sector,
                    INT_32 *block,
                    INT_32 *page)
{
  *block = sector / nandrcdat.geom.pages_per_block;
  *page = sector - (*block * nandrcdat.geom.pages_per_block);
}

/***********************************************************************
 *
 * Function: phy3250_nand_wp
 *
 * Purpose: Enable or disable NAND write protect
 *
 * Processing:
 *     Enable or disable NAND write protect.
 *
 * Parameters:
 *     enable : TRUE to enable write protect, FALSE to disable
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void phy3250_nand_wp(BOOL_32 enable)
{
  if (enable == TRUE)
  {
    gpio_set_gpo_state(OUTP_STATE_GPO(19), 0);
  }
  else
  {
    gpio_set_gpo_state(0, OUTP_STATE_GPO(19));
  }
}
