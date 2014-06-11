/***********************************************************************
 * $Id:: sysapi_blkdev.c 875 2008-07-08 17:27:04Z wellsk               $
 *
 * Project: System functions (various)
 *
 * Description:
 *     Processes system functions
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

#include "phy3250_board.h"
#include "sys.h"
#include "lpc_string.h"
#include "lpc32xx_timer_driver.h"
#include "lpc32xx_gpio_driver.h"
#include "lpc32xx_sdcard_driver.h"
#include "lpc32xx_clkpwr_driver.h"
#include "lpc_sdmmc.h"

/***********************************************************************
 * SD/MMC controller and loader support below
 **********************************************************************/

/* Operating condition register value */
#define OCRVAL 0x001C0000 /* around 3.1v */

/* SDMMC Block (sector) size in bytes */
#define SDMMC_BLK_SIZE    512

/* SDMMC OCR power-up complete mask (used against word 0) */
#define SDMMC_OCR_MASK    0x80000000

/* Number of blocks to read for test */
#define BLKSTOREAD 0x0FFF

/* SDMMC maximum number of OCR request retries before considering a
   card dead */
#define SDMMC_MAX_OCR_RET 512

/* SDMMC OCR sequence clock speed - also the default clock speed of
   the bus whenever a new card is detected and configured */
#define SDMMC_OCR_CLOCK   390000

/* Normal clock speeds - the FIFOs may not be able to be maintained
   without using DMA at these clock speeds. */
#define SD_NORM_CLOCK  5000000
#define MMC_NORM_CLOCK 5000000

/* Each command enumeration has a SDMMC command number (used by the
   card) and a SDMMC command/control word (used by the controller).
   This structure defines this word pair. */
typedef struct
{
    UNS_32 cmd;            /* Mapped SDMMC (spec) command */
    SDMMC_RESPONSE_T resp; /* Expected response type */
} SDMMC_CMD_CTRL_T;

/* This array maps the SDMMC command enumeration to the hardware SDMMC
   command index, the controller setup value, and the expected response
   type */
static SDMMC_CMD_CTRL_T sdmmc_cmds[SDMMC_INVALID_CMD] =
{
    /* SDMMC_IDLE */
    {0,  SDMMC_RESPONSE_NONE},
    /* MMC_SENDOP_COND */
    {1,  SDMMC_RESPONSE_R3},
    /* SDMMC_ALL_SEND_CID */
    {2,  SDMMC_RESPONSE_R2},
    /* SDMMC_SRA */
    {3,  SDMMC_RESPONSE_R1},
    /* MMC_PROGRAM_DSR */
    {4,  SDMMC_RESPONSE_NONE},
    /* SDMMC_SELECT_CARD */
    {7,  SDMMC_RESPONSE_R1},
    /* SDMMC_SEND_CSD */
    {9,  SDMMC_RESPONSE_R2},
    /* SDMMC_SEND_CID */
    {10, SDMMC_RESPONSE_R2},
    /* SDMMC_READ_UNTIL_STOP */
    {11, SDMMC_RESPONSE_R1},
    /* SDMMC_STOP_XFER */
    {12, SDMMC_RESPONSE_R1},
    /* SDMMC_SSTAT */
    {13, SDMMC_RESPONSE_R1},
    /* SDMMC_INACTIVE */
    {15, SDMMC_RESPONSE_NONE},
    /* SDMMC_SET_BLEN */
    {16, SDMMC_RESPONSE_R1},
    /* SDMMC_READ_SINGLE */
    {17, SDMMC_RESPONSE_R1},
    /* SDMMC_READ_MULTIPLE */
    {18, SDMMC_RESPONSE_R1},
    /* SDMMC_WRITE_UNTIL_STOP */
    {20, SDMMC_RESPONSE_R1},
    /* SDMMC_SET_BLOCK_COUNT */
    {23, SDMMC_RESPONSE_R1},
    /* SDMMC_WRITE_SINGLE */
    {24, SDMMC_RESPONSE_R1},
    /* SDMMC_WRITE_MULTIPLE */
    {25, SDMMC_RESPONSE_R1},
    /* MMC_PROGRAM_CID */
    {26, SDMMC_RESPONSE_R1},
    /* SDMMC_PROGRAM_CSD */
    {27, SDMMC_RESPONSE_R1},
    /* SDMMC_SET_WR_PROT */
    {28, SDMMC_RESPONSE_R1B},
    /* SDMMC_CLEAR_WR_PROT */
    {29, SDMMC_RESPONSE_R1B},
    /* SDMMC_SEND_WR_PROT */
    {30, SDMMC_RESPONSE_R1},
    /* SD_ERASE_BLOCK_START */
    {32, SDMMC_RESPONSE_R1},
    /* SD_ERASE_BLOCK_END */
    {33, SDMMC_RESPONSE_R1},
    /* MMC_ERASE_BLOCK_START */
    {35, SDMMC_RESPONSE_R1},
    /* MMC_ERASE_BLOCK_END */
    {36, SDMMC_RESPONSE_R1},
    /* MMC_ERASE_BLOCKS */
    {38, SDMMC_RESPONSE_R1B},
    /* MMC_FAST_IO */
    {39, SDMMC_RESPONSE_R4},
    /* MMC_GO_IRQ_STATE */
    {40, SDMMC_RESPONSE_R5},
    /* MMC_LOCK_UNLOCK */
    {42, SDMMC_RESPONSE_R1B},
    /* SDMMC_APP_CMD */
    {55, SDMMC_RESPONSE_R1},
    /* SDMMC_GEN_CMD */
    {56, SDMMC_RESPONSE_R1B}
};

/* This array maps the SDMMC application specific command enumeration to
   the hardware SDMMC command index, the controller setup value, and the
   expected response type */
static SDMMC_CMD_CTRL_T sdmmc_app_cmds[SD_INVALID_APP_CMD] =
{
    /* SD_SET_BUS_WIDTH */
    {6,  SDMMC_RESPONSE_R1},
    /* SD_SEND_STATUS */
    {13, SDMMC_RESPONSE_R1},
    /* SD_SEND_WR_BLOCKS */
    {22, SDMMC_RESPONSE_R1},
    /* SD_SET_ERASE_COUNT */
    {23, SDMMC_RESPONSE_R1},
    /* SD_SENDOP_COND */
    {41, SDMMC_RESPONSE_R3},
    /* SD_CLEAR_CARD_DET */
    {42, SDMMC_RESPONSE_R1},
    /* SD_SEND_SCR */
    {51, SDMMC_RESPONSE_R1}
};

static volatile INT_32 cmdresp, datadone;
static INT_32 sddev = 0;
static UNS_32 rca;

/***********************************************************************
 *
 * Function: wait4cmddone
 *
 * Purpose: Sets command completion flag (callback)
 *
 * Processing:
 *     Sets the cmdresp flag to 1.
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
 static void wait4cmddone(void)
 {
 	cmdresp = 1;
 }

/***********************************************************************
 *
 * Function: wait4datadone
 *
 * Purpose: Sets data completion flag (callback)
 *
 * Processing:
 *     Sets the datadone flag to 1.
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
 static void wait4datadone(void)
 {
 	datadone = 1;
 }

/***********************************************************************
 *
 * Function: sdmmc_cmd_setup
 *
 * Purpose: Setup and SD/MMC command structure
 *
 * Processing:
 *     From the passed arguments, sets up the command structure for the
 *     SD_ISSUE_CMD IOCTL call to the SD card driver.
 *
 * Parameters:
 *     pcmdsetup : Pointer to command structure to fill
 *     cmd       : Command to send
 *     arg       : Argument to send
 *     resp_type : Expected response type for this command
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
static void sdmmc_cmd_setup(SD_CMD_T *pcmdsetup,
                            UNS_32 cmd,
                            UNS_32 arg,
                            SDMMC_RESPONSE_T resp_type)
{
	INT_32 tmp = 0;

	/* Determine response size */
	switch (resp_type)
	{
		case SDMMC_RESPONSE_NONE:
			break;

		case SDMMC_RESPONSE_R1:
		case SDMMC_RESPONSE_R1B:
		case SDMMC_RESPONSE_R3:
			tmp = 48;
			break;

		case SDMMC_RESPONSE_R2:
			tmp = 136;
			break;
	}

	/* Setup SD command structure */
	pcmdsetup->cmd = cmd;
	pcmdsetup->arg = arg;
	pcmdsetup->cmd_resp_size = tmp;
}

/***********************************************************************
 *
 * Function: sdmmc_cmd_send
 *
 * Purpose: Process a SDMMC command and response (without data transfer)
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     cmd       : Command to send
 *     arg       : Argument to send
 *     resp_type : Expected response type for this command
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
static void sdmmc_cmd_send(SDMMC_COMMAND_T cmd,
                           UNS_32 arg,
                           SD_CMDRESP_T *resp)
{
    SD_CMDDATA_T sdcmd;

	/* Perform command setup from standard MMC command table */
	sdmmc_cmd_setup(&sdcmd.cmd, sdmmc_cmds[cmd].cmd, arg,
		sdmmc_cmds[cmd].resp);

	/* No data for this setup */
	sdcmd.data.dataop = SD_DATAOP_NONE;

	/* Issue command and wait for it to complete */
	cmdresp = 0;
	sdcard_ioctl(sddev, SD_ISSUE_CMD, (INT_32) &sdcmd);
	while (cmdresp == 0);

    sdcard_ioctl(sddev, SD_GET_CMD_RESP, (INT_32) resp);
}

/***********************************************************************
 *
 * Function: app_cmd_send
 *
 * Purpose: Process a SD APP command and response
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     cmd  : App command to send
 *     arg  : Argument for command
 *     resp : Pointer to response buffer
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
static void app_cmd_send(SD_APP_CMD_T cmd,
                         UNS_32 arg,
                         SD_CMDRESP_T *resp)
{
    SD_CMDDATA_T sdcmd;

	/* Perform command setup from SD APP command table */
	sdmmc_cmd_setup(&sdcmd.cmd, sdmmc_app_cmds[cmd].cmd, arg,
		sdmmc_app_cmds[cmd].resp);

	/* No data for this setup */
	sdcmd.data.dataop = SD_DATAOP_NONE;

	/* Issue command and wait for it to complete */
	cmdresp = 0;
	sdcard_ioctl(sddev, SD_ISSUE_CMD, (INT_32) &sdcmd);
	while (cmdresp == 0);

    sdcard_ioctl(sddev, SD_GET_CMD_RESP, (INT_32) resp);
}

/***********************************************************************
 *
 * Function: sdmmc_cmd_start_data
 *
 * Purpose: Read SD/MMC data blocks
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     cmd  : Pointer to command structure
 *     resp : Pointer to response buffer
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
static INT_32 sdmmc_cmd_start_data(SD_CMDDATA_T *cmd,
                                   SD_CMDRESP_T *resp)
{
	INT_32 status = 0;

	/* Issue command and wait for it to complete */
	datadone = 0;
	sdcard_ioctl(sddev, SD_ISSUE_CMD, (INT_32) cmd);
	while (datadone == 0);

	/* Get the data transfer state */
	sdcard_ioctl(sddev, SD_GET_CMD_RESP, (INT_32) resp);
	if ((resp->data_status & SD_DATABLK_END) == 0)
	{
		status = -1;
	}

	return status;
}

/***********************************************************************
 *
 * Function: sdmmc_read_block
 *
 * Purpose: Read SD/MMC data blocks
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     buff    : Pointer to data buffer
 *     numblks : Number of blocks to read
 *     index   : Block read index
 *     resp    :  Pointer to response buffers
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
static INT_32 sdmmc_read_block(UNS_32 *buff,
                               INT_32 numblks, /* Must be 1 */
                               UNS_32 index,
                               SD_CMDRESP_T *resp)
{
    SD_CMDDATA_T sdcmd;

	/* Setup read data */
	sdcmd.data.dataop = SD_DATAOP_READ;
	sdcmd.data.blocks = numblks;
	sdcmd.data.buff = buff;
	sdcmd.data.usependcmd = FALSE;
	sdcmd.data.stream = FALSE;

	/* Setup block count to data size */
	sdmmc_cmd_send(SDMMC_SET_BLEN, SDMMC_BLK_SIZE, resp);

	/* Perform command setup from standard MMC command table */
	sdmmc_cmd_setup(&sdcmd.cmd, sdmmc_cmds[SDMMC_READ_SINGLE].cmd,
		index, sdmmc_cmds[SDMMC_READ_SINGLE].resp);

	/* Read data from the SD card */
	return sdmmc_cmd_start_data(&sdcmd, resp);
}

/***********************************************************************
 *
 * Function: blkdev_deinit
 *
 * Purpose: SDMMC deinit
 *
 * Processing:
 *     De-initialize the SDMMC card
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: Always returns TRUE
 *
 * Notes: None
 *
 **********************************************************************/
BOOL_32 blkdev_deinit(void) 
{
	if (sddev != 0) 
	{
		sdcard_close(sddev);
		sddev = 0;
	}

	return TRUE;
}

/***********************************************************************
 *
 * Function: blkdev_init
 *
 * Purpose: SDMMC init
 *
 * Processing:
 *     Initialize the SDMMC card
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: Always returns TRUE
 *
 * Notes: None
 *
 **********************************************************************/
BOOL_32 blkdev_init(void) 
{
    SD_CMDRESP_T resp;
    SDC_PRMS_T params;
    SDC_XFER_SETUP_T dataset;
    INT_32 ocrtries, sdcardtype, validocr;

	if (phy3250_sdmmc_card_inserted() == FALSE)
	{
		return FALSE;
	}

	/* Open SD card controller driver */
	sddev = sdcard_open(SDCARD, 0);
	if (sddev == 0)
	{
		return FALSE;
	}

	/* Setup controller parameters */
	params.opendrain = TRUE;
	params.powermode = SD_POWER_ON_MODE;
	params.pullup0 = 1;
	params.pullup1 = 1;
	params.pullup23 = 1;
	params.pwrsave = FALSE;
	params.sdclk_rate = SDMMC_OCR_CLOCK;
	params.use_wide = FALSE;
	if (sdcard_ioctl(sddev, SD_SETUP_PARAMS,
		(INT_32) &params) == _ERROR)
	{
		blkdev_deinit();
		return FALSE;
	}

	/* Setup data transfer paramaters */
	dataset.data_callback = (PFV) wait4datadone;
	dataset.cmd_callback = (PFV) wait4cmddone;
	dataset.blocksize = SDMMC_BLK_SIZE;
	dataset.data_to = 0x001FFFFF; /* Long timeout for slow MMC cards */
	dataset.use_dma = FALSE;
	sdcard_ioctl(sddev, SD_SETUP_DATA_XFER, (INT_32) &dataset);

	/* Issue IDLE command */
	sdmmc_cmd_send(SDMMC_IDLE, 0, &resp);

	/* After the IDLE command, a small wait is needed to allow the cards
	   to initialize */
    timer_wait_ms(TIMER_CNTR0, 100);

	/* Issue APP command, only SD cards will respond to this */
	sdcardtype = 0;
	sdmmc_cmd_send(SDMMC_APP_CMD, 0, &resp);
	if ((resp.cmd_status & SD_CMD_RESP_RECEIVED) != 0)
	{
		sdcardtype = 1;
	}

	ocrtries = SDMMC_MAX_OCR_RET;
	validocr = 0;

	/* If this is an SD card, use the SD sendop command */
	if (sdcardtype == 1)
	{
		resp.cmd_resp [1] = 0;
		while ((validocr == 0) && (ocrtries >= 0))
		{
			/* SD card init sequence */
			app_cmd_send(SD_SENDOP_COND, OCRVAL, &resp);
			if ((resp.cmd_resp [1] & SDMMC_OCR_MASK) == 0)
			{
				/* Response received and busy, so try again */
				sdmmc_cmd_send(SDMMC_APP_CMD, 0, &resp);
			}
			else
			{
				validocr = 1;
			}
			
			ocrtries--;
		}

		if (validocr == 0)
		{
			blkdev_deinit();
			return FALSE;
		}

		/* Enter push-pull mode and switch to fast clock */
		params.opendrain = FALSE;
		params.sdclk_rate = SD_NORM_CLOCK;
		sdcard_ioctl(sddev, SD_SETUP_PARAMS, (INT_32) &params);

		/* Get CID */
		sdmmc_cmd_send(SDMMC_ALL_SEND_CID, 0, &resp);

		/* Get relative card address */
		sdmmc_cmd_send(SDMMC_SRA, 0, &resp);
		rca = (resp.cmd_resp [1] >> 16) & 0xFFFF;

		/* Select card (required for bus width change) */
		sdmmc_cmd_send(SDMMC_SELECT_CARD, (rca << 16), &resp);

		/* Set bus width to 4 bits */
		sdmmc_cmd_send(SDMMC_APP_CMD, (rca << 16), &resp);
		app_cmd_send(SD_SET_BUS_WIDTH, 2, &resp);

		/* Switch controller to 4-bit data bus */
		params.use_wide = TRUE;
		sdcard_ioctl(sddev, SD_SETUP_PARAMS, (INT_32) &params);
	}
	else
	{
		resp.cmd_resp [1] = 0;
		while ((validocr == 0) && (ocrtries >= 0))
		{
			/* MMC card init sequence */
			sdmmc_cmd_send(MMC_SENDOP_COND, OCRVAL, &resp);
			if ((resp.cmd_resp [1] & SDMMC_OCR_MASK) != 0)
			{
				validocr = 1;
			}
			
			ocrtries--;
		}

		if (validocr == 0)
		{
			blkdev_deinit();
			return FALSE;
		}

		/* Enter push-pull mode and switch to fast clock */
		params.opendrain = FALSE;
		params.sdclk_rate = MMC_NORM_CLOCK;
		sdcard_ioctl(sddev, SD_SETUP_PARAMS, (INT_32) &params);

		/* Get CID */
		sdmmc_cmd_send(SDMMC_ALL_SEND_CID, 0, &resp);

		/* Get relative card address */
		rca = 0x1234;
		sdmmc_cmd_send(SDMMC_SRA, (rca << 16), &resp);
	}

	/* Deselect card */
	sdmmc_cmd_send(SDMMC_SELECT_CARD, 0, &resp);

	/* Status request */
	sdmmc_cmd_send(SDMMC_SSTAT, (rca << 16), &resp);

	/* Select card */
	sdmmc_cmd_send(SDMMC_SELECT_CARD, (rca << 16), &resp);

	return TRUE;
}

/* SDMMC sector read */
BOOL_32 blkdev_read(void *buff, UNS_32 sector) 
{
    SD_CMDRESP_T resp;

	if (sdmmc_read_block(buff, 1, (sector * SDMMC_BLK_SIZE),
		&resp) < 0)
	{
		return FALSE;
	}
	
	return TRUE;
}
