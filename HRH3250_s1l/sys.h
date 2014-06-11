/***********************************************************************
 * $Id:: sys.h 875 2008-07-08 17:27:04Z wellsk                         $
 *
 * Project: System structures and functions
 *
 * Description:
 *     Various system specific items
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

#ifndef SYS_H
#define SYS_H

/**********************************************************************/
/* Default S1L prompt for this system */
#define DEFPROMPT "PHY3250>"

/* System S1L startup message */
#define SYSHEADER "HRH LPC3250 board"

/* Number of blocks dedicated to S1L bootloader and kickstart */
#define BL_NUM_BLOCKS 25

/* Maximum amount of blocks available for saved FLASH applications
   stored at block (BL_FIRST_BLOCK + BL_NUM_BLOCKS), use 0xFFFFFFFF
   for maximum blocks */
#define FLASHAPP_MAX_BLOCKS 100

/**********************************************************************/
/* Offset into serial EEPROM where stage 1 boot loader data is
   saved */
#define PHY3250_SSEPROM_SINDEX (PHY3250_SEEPROM_SIZE - 0x800)

/* FLASH reserved marker offset (used with WinCE) */
#define NAND_LPCRESERVED_OFFS (512 + 4)

/* Clock configuration structure */
typedef struct
{
	UNS_32 armclk;       /* ARM clock frequency (Hz) */
	UNS_32 hclkdiv;      /* HCLK divider from armclk */
	UNS_32 pclkdiv;      /* PCLK divider from armclk */
} SYS_CLOCK_T;

/* Saved system specific data structure */
typedef struct
{
	SYS_CLOCK_T clksetup;
	UNS_32 verikey;
} SYS_SAVED_DATA_T;
extern SYS_SAVED_DATA_T sys_saved_data;

/* Get saved system information from storage */
BOOL_32 sys_get(SYS_SAVED_DATA_T *pASysData);

/* Saved system information to storage */
BOOL_32 sys_save(SYS_SAVED_DATA_T *pASysData);

/* SDMMC init */
BOOL_32 phy3250_sdmmc_init(void);

/* SDMMC sector read */
BOOL_32 phy3250_sdmmc_read(void *buff, UNS_32 sector);

/* SDMMC deinit */
BOOL_32 phy3250_sdmmc_deinit(void);

/* Initialize SDMMC and FAT if needed */
void sdcard_init(void);

/* Safely adjust or readjust system clocks */
UNS_32 clock_adjust(void);

/* Purpose: Verifies savedinfo structure is good */
BOOL_32 svinfockgood(UNS_32 *buff,
							int bytes,
							UNS_32 verikey);

/* Purpose: Generate verification key for savedinfo structure */
void svinfockgen(UNS_32 *buff,
						int bytes,
						UNS_32 *verikey);

/* Places a new bootloader into the first block(s) */
BOOL_32 nand_bootloader_update(void);

/* Places a new kickstart loader into the first block */
BOOL_32 nand_kickstart_update(void);

/* 10mS timer tick used for some timeout checks */
extern volatile UNS_32 tick_10ms;

/* Initialize MMU command group */
void mmu_cmd_group_init(void);

/* Initialize hardware command group */
void hw_cmd_group_init(void);

void icache_invalid(void);

/* Flush only or flush and invalidate data cache */
void dcache_flush(BOOL_32 flush_invalid);

/* Marks a block as reserved, support function */
BOOL_32 flash_reserve_block(UNS_32 block);

/* Returns TRUE if the current block is a reserved block */
BOOL_32 flash_is_rsv_block(UNS_32 block);

#endif /* SYS_H */
