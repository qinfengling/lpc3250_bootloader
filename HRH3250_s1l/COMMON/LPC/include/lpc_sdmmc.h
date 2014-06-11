/***********************************************************************
 * $Id:: lpc_sdmmc.h 745 2008-05-13 19:59:29Z pdurgesh                 $
 *
 * Project: SD/MMC definitions
 *
 * Description:
 *     This file contains the structure definitions and manifest
 *     constants for the following component:
 *         SD/MMC storage cards
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

#ifndef LPC_SDMMC_H
#define LPC_SDMMC_H

#include "lpc_types.h"

#if defined (__cplusplus)
extern "C"
{
#endif

/* SD/MMC commands - this matrix shows the command, response types, and
   supported card type for that command.
   Command                 Number Resp  SD  MMC
   ----------------------- ------ ----- --- ---
   Reset (go idle)         CMD0   NA    x   x
   Send op condition       CMD1   R3        x
   All send CID            CMD2   R2    x   x
   Send relative address   CMD3   R1        x
   Send relative address   CMD3   R6    x
   Program DSR             CMD4   NA        x
   Select/deselect card    CMD7   R1b       x
   Select/deselect card    CMD7   R1    x
   Send CSD                CMD9   R2    x   x
   Send CID                CMD10  R2    x   x
   Read data until stop    CMD11  R1    x   x
   Stop transmission       CMD12  R1/b  x   x
   Send status             CMD13  R1    x   x
   Go inactive state       CMD15  NA    x   x
   Set block length        CMD16  R1    x   x
   Read single block       CMD17  R1    x   x
   Read multiple blocks    CMD18  R1    x   x
   Write data until stop   CMD20  R1        x
   Setblock count          CMD23  R1        x
   Write single block      CMD24  R1    x   x
   Write multiple blocks   CMD25  R1    x   x
   Program CID             CMD26  R1        x
   Program CSD             CMD27  R1    x   x
   Set write protection    CMD28  R1b   x   x
   Clear write protection  CMD29  R1b   x   x
   Send write protection   CMD30  R1    x   x
   Erase block start       CMD32  R1    x
   Erase block end         CMD33  R1    x
   Erase block start       CMD35  R1        x
   Erase block end         CMD36  R1        x
   Erase blocks            CMD38  R1b       x
   Fast IO                 CMD39  R4        x
   Go IRQ state            CMD40  R5        x
   Lock/unlock             CMD42  R1b       x
   Application command     CMD55  R1        x
   General command         CMD56  R1b       x

   *** SD card application commands - these must be preceded with ***
   *** MMC CMD55 application specific command first               ***
   Set bus width           ACMD6  R1    x
   Send SD status          ACMD13 R1    x
   Send number WR blocks   ACMD22 R1    x
   Set WR block erase cnt  ACMD23 R1    x
   Send op condition       ACMD41 R3    x
   Set clear card detect   ACMD42 R1    x
   Send CSR                ACMD51 R1    x */
typedef enum
{
  SDMMC_IDLE,              /* Put card in idle mode */
  MMC_SENDOP_COND,         /* Send operating condition */
  SDMMC_ALL_SEND_CID,      /* All cards send CID */
  SDMMC_SRA,               /* Set relative address */
  MMC_PROGRAM_DSR,         /* Program DSR */
  SDMMC_SELECT_CARD,       /* Select card */
  SDMMC_SEND_CSD,          /* Send CSD data */
  SDMMC_SEND_CID,          /* Send CID register data (with rel.
                                addr) */
  SDMMC_READ_UNTIL_STOP,   /* Read data until stop */
  SDMMC_STOP_XFER,         /* Stop current transmission */
  SDMMC_SSTAT,             /* Send status */
  SDMMC_INACTIVE,          /* Put card in inactive state */
  SDMMC_SET_BLEN,          /* Set block transfer length */
  SDMMC_READ_SINGLE,       /* Read single block */
  SDMMC_READ_MULTIPLE,     /* Read multiple blocks */
  SDMMC_WRITE_UNTIL_STOP,  /* Write data until stop */
  SDMMC_SET_BLOCK_COUNT,   /* Set block count */
  SDMMC_WRITE_SINGLE,      /* Write single block */
  SDMMC_WRITE_MULTIPLE,    /* Write multiple blocks */
  MMC_PROGRAM_CID,         /* Program CID */
  SDMMC_PROGRAM_CSD,       /* Program CSD */
  SDMMC_SET_WR_PROT,       /* Set write protection */
  SDMMC_CLEAR_WR_PROT,     /* Clear write protection */
  SDMMC_SEND_WR_PROT,      /* Send write protection */
  SD_ERASE_BLOCK_START,    /* Set starting erase block */
  SD_ERASE_BLOCK_END,      /* Set ending erase block */
  MMC_ERASE_BLOCK_START,   /* Set starting erase block */
  MMC_ERASE_BLOCK_END,     /* Set ending erase block */
  MMC_ERASE_BLOCKS,        /* Erase blocks */
  MMC_FAST_IO,             /* Fast IO */
  MMC_GO_IRQ_STATE,        /* Go into IRQ state */
  MMC_LOCK_UNLOCK,         /* Lock/unlock */
  SDMMC_APP_CMD,           /* Application specific command */
  SDMMC_GEN_CMD,           /* General purpose command */
  SDMMC_INVALID_CMD        /* Invalid SDMMC command */
} SDMMC_COMMAND_T;

/* SDMMC application specific commands for SD cards only - these must
   be preceded by the SDMMC CMD55 to work correctly */
typedef enum
{
  SD_SET_BUS_WIDTH,        /* Set the SD bus width */
  SD_SEND_STATUS,          /* Send the SD card status */
  SD_SEND_WR_BLOCKS,       /* Send the number of written clocks */
  SD_SET_ERASE_COUNT,      /* Set the number of blocks to pre-erase */
  SD_SENDOP_COND,          /* Send the OCR register (init) */
  SD_CLEAR_CARD_DET,       /* Set or clear the 50K detect pullup */
  SD_SEND_SCR,             /* Send the SD configuration register */
  SD_INVALID_APP_CMD       /* Invalid SD application command */
} SD_APP_CMD_T;

/* Possible SDMMC response types */
typedef enum
{
  SDMMC_RESPONSE_R1,       /* Typical status */
  SDMMC_RESPONSE_R1B,      /* Typical status with busy */
  SDMMC_RESPONSE_R2,       /* CID/CSD registers (CMD2 and CMD10) */
  SDMMC_RESPONSE_R3,       /* OCR register (CMD1, ACMD41) */
  SDMMC_RESPONSE_R4,       /* Fast IO response word */
  SDMMC_RESPONSE_R5,       /* Go IRQ state response word */
  SDMMC_RESPONSE_R6,       /* Published RCA response */
  SDMMC_RESPONSE_NONE      /* No response expected */
} SDMMC_RESPONSE_T;

#if defined (__cplusplus)
}
#endif /*__cplusplus */

#endif /* LPC_SDMMC_H */
