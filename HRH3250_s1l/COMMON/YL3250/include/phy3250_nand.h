/***********************************************************************
 * $Id:: phy3250_nand.h 970 2008-07-28 21:01:39Z wellsk                $
 *
 * Project: Phytec LPC3250 NAND device definitions
 *
 * Description:
 *     This file contains board specific information such as the
 *     NAND device commands, geometry structures, etc.
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

#ifndef PHY3250_NAND_H
#define PHY3250_NAND_H

#include "lpc_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

/***********************************************************************
 * NAND part number : ST Micro NAND128-A/NAND256-A/NAND512-A/NAND01G-A/NAND01G-B
 * Size             : 128Mbits/256Mbits/512Mbits/1Gbits
 * Ids              : 0x73/(0x35,0x75)/(0x36,0x76)/(0x39,0x79)
 * Number of blocks : 1024/2048/4096/8192
 * Pages per block  : 32
 * Bytes per page   : 528 (512 + 16 spare)/2112 (2048 + 64 spare)
 * Bad block marker : (Offset 512+6)/(Offset 2048+6)
 * Bus size         : 8 bits
 * Other info       : None
 **********************************************************************/

/* Structure that stores the device geometry */
typedef struct
{
  INT_32 num_blocks;
  INT_32 pages_per_block;
  INT_32 bytes_per_page;
  INT_32 extra_per_page;
  UNS_32 addrcycles;
} NAND_GEOM_T;

/* The bad block marker provided by the vendor is in the spare data
   area at byte 6 (offset 5) , ref flash datasheet*/
#define NAND_BADBLOCK_SPAREOFFS     (5)

/* LPC3250 MLC controller index implementation of NAND bad block - to
   prevent erasures of data blocks that use the vendor implemented
   bad block index, the software checks BOTH the vendor and LPC3250
   bad block indices for markers. */
#define NAND_LPCBADBLOCK_SPAREOFFS  NAND_BADBLOCK_SPAREOFFS

/* LPC bad block marker flag */
#define NAND_BAD_BLOCK_MARKER 0x77

/* Good block marker flag */
#define NAND_GOOD_BLOCK_MARKER 0xFF

/* ECC storage area offset into the extra data area */
#define NAND_ECC_OFFS          8

/***********************************************************************
 * NAND support functions
 **********************************************************************/

/* Setup NAND timing */
void nand_clock_setup(void);

/* Initialize NAND and get NAND gemoetry */
INT_32 nand_init(NAND_GEOM_T *geom);

/* Erase a NAND block */
INT_32 nand_erase_block(INT_32 block);

/* Read a NAND sector */
INT_32 nand_read_sector(INT_32 sector,
                        UNS_8 *readbuff,
                        UNS_8 *extrabuff);

/* Write a NAND sector */
INT_32 nand_write_sector(INT_32 sector,
                         UNS_8 *writebuff,
                         UNS_8 *extrabuff);

/* Read a series of NAND sectors, optionally skip bad blocks */
INT_32 nand_read_sectors(INT_32 sector_start,
                         INT_32 num_sectors,
                         UNS_8 *readbuff,
                         BOOL_32 skipbad);

/* Write a series of NAND sectors, optionally skip bad blocks */
INT_32 nand_write_sectors(INT_32 sector_start,
                          INT_32 num_sectors,
                          UNS_8 *writebuff,
                          BOOL_32 skipbad);

/* Translate a block and page address to a sector */
INT_32 nand_to_sector(INT_32 block,
                      INT_32 page);

/* Translate a sector address to a block and page address */
void sector_to_nand(INT_32 sector,
                    INT_32 *block,
                    INT_32 *page);

/* Enable or disable NAND write protect */
void phy3250_nand_wp(BOOL_32 enable);

#ifdef __cplusplus
}
#endif

#endif /* PHY3250_NAND_H */
