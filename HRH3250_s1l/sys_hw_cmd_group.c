/***********************************************************************
 * $Id:: sys_hw_cmd_group.c 902 2008-07-17 21:01:15Z wellsk            $
 *
 * Project: Command processor
 *
 * Description:
 *     Processes commands from the command prompt
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
#include "lpc32xx_clkpwr_driver.h"
#include "lpc32xx_emc.h"
#include "phy3250_board.h"
#include "s1l_cmds.h"
#include "s1l_line_input.h"
#include "s1l_sys_inf.h"

/* clock command */
BOOL_32 cmd_clock(void);
static UNS_32 cmd_clock_plist[] =
{
	(PARSE_TYPE_STR), /* The "clock" command */
	(PARSE_TYPE_DEC),
	(PARSE_TYPE_DEC),
	(PARSE_TYPE_DEC | PARSE_TYPE_END)
};
static CMD_ROUTE_T hw_clock_cmd =
{
	"clock",
	cmd_clock,
	"Sets the system clock frequencies",
	"clock [ARM clock (MHz)][HCLK divider(1, 2, or 4)][PCLK divider(1-32)]",
	cmd_clock_plist,
	NULL
};

/* maddr command */
BOOL_32 cmd_maddr(void);
static UNS_32 cmd_maddr_plist[] =
{
	(PARSE_TYPE_STR), /* The "maddr" command */
	(PARSE_TYPE_STR | PARSE_TYPE_END)
};
static CMD_ROUTE_T hw_maddr_cmd =
{
	"maddr",
	cmd_maddr,
	"Sets the MAC address of the system",
	"maddr [ha:ha:ha:ha:ha:ha]]",
	cmd_maddr_plist,
	NULL
};

/* update command */
BOOL_32 cmd_update(void);
static UNS_32 cmd_update_plist[] =
{
	(PARSE_TYPE_STR | PARSE_TYPE_OPT), /* The "update" command */
	(PARSE_TYPE_STR | PARSE_TYPE_END)
};
static CMD_ROUTE_T hw_update_cmd =
{
	"update",
	cmd_update,
	"Updates the kickstart or the bootloader application",
	"update <kick>",
	cmd_update_plist,
	NULL
};

/* hw group */
static GROUP_LIST_T hw_group =
{
	"hw", /* hardware group */
	"Hardware command group",
	NULL,
	NULL
};

/* nrsv command */
BOOL_32 cmd_nrsv(void);
static UNS_32 cmd_nrsv_plist[] =
{
	(PARSE_TYPE_STR), /* The "nrsv" command */
	(PARSE_TYPE_DEC),
	(PARSE_TYPE_DEC | PARSE_TYPE_END)
};
static CMD_ROUTE_T hw_nrsv_cmd =
{
	"nrsv",
	cmd_nrsv,
	"Reserve a range of blocks for WinCE",
	"nrsv [first block] [number of blocks]",
	cmd_nrsv_plist,
	NULL
};

/* nandrs command */
BOOL_32 cmd_nandrs(void);
static UNS_32 cmd_nandrs_plist[] =
{
	(PARSE_TYPE_STR | PARSE_TYPE_END) /* The "nandrs" command */
};
static CMD_ROUTE_T hw_nandrs_cmd =
{
	"nandrs",
	cmd_nandrs,
	"Displays a list of reserved NAND blocks",
	"nandrs",
	cmd_nandrs_plist,
	NULL
};

static UNS_8 clockerr_msg[] = "Error: ARM clock rate must be between "
	"33MHz and 550MHz";
static UNS_8 clock_msg[] = "New programmed ARM clock frequency is ";
static UNS_8 clockbad_msg[] =
	"Error couldn't find a close speed for that rate";
static UNS_8 macafail_msg[] = "Error setting ethernet MAC address";
static UNS_8 smac_msg[] = "Are you sure you want to use this MAC address?";
static UNS_8 updnotc_msg[] = "Image in memory is not contiguous";
static UNS_8 updnoim_msg[] = "No image in memory to update NAND";
static UNS_8 updnotraw_msg[] = "Only raw images can be updated in NAND";
static UNS_8 updgt31_msg[] =
    "Image is too big for kickstart, must be less than 15872 bytes";
static UNS_8 updgt256_msg[] =
    "Image is too big for stage 1, must be less than 256Kbytes";
static UNS_8 updfail_msg[] = "NAND bootloader update failed";
static UNS_8 updgood_msg[] = "Update completed";
static UNS_8 blkmkerr_msg[] = "Error marking block(s)";
static UNS_8 rsvlist_msg[] = "Displaying NAND reserved block list";
extern UNS_8 noflash_msg[];

/***********************************************************************
 *
 * Function: cmd_clock
 *
 * Purpose: Clock command
 *
 * Processing:
 *     Parse the string elements for the clock command and sets the
 *     new clock value.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: TRUE if the command was processed, otherwise FALSE
 *
 * Notes: May not work in DDR systems.
 *
 **********************************************************************/
BOOL_32 cmd_clock(void) 
{
	UNS_32 freqr, armclk, hclkdiv, pclkdiv;
	UNS_8 dclk [32];
	BOOL_32 processed = TRUE;

	/* Get arguments */
	armclk = cmd_get_field_val(1);
	hclkdiv = cmd_get_field_val(2);
	pclkdiv = cmd_get_field_val(3);

	if (!((hclkdiv == 1) || (hclkdiv == 2) || (hclkdiv == 4))) 
	{
		processed = FALSE;
	}
	if ((pclkdiv < 1) || (pclkdiv > 32)) 
	{
		processed = FALSE;
	}
	if ((armclk < 33) || (armclk > 550))
	{
		term_dat_out_crlf(clockerr_msg);
		processed = FALSE;
	}
	else 
	{
		armclk = armclk * 1000 * 1000;
	}

	if (processed == TRUE) 
	{
		sys_saved_data.clksetup.armclk = armclk;
		sys_saved_data.clksetup.hclkdiv = hclkdiv;
		sys_saved_data.clksetup.pclkdiv = pclkdiv;
		freqr = clock_adjust();
		if (freqr == 0)
		{
			term_dat_out_crlf(clockbad_msg);
		}
		else
		{
			term_dat_out(clock_msg);
			str_makedec(dclk, freqr);
			term_dat_out_crlf(dclk);
			sys_save(&sys_saved_data);
		}
	}

	return TRUE;
}

/***********************************************************************
 *
 * Function: cmd_maddr
 *
 * Purpose: Sets MAC address of ethernet device
 *
 * Processing:
 *     See function.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: TRUE if the command was good, otherwise FALSE
 *
 * Notes: None
 *
 **********************************************************************/
BOOL_32 cmd_maddr(void) {
	int idx, offs;
	UNS_8 *p8, *curp, mac[8], str[16];
	UNS_32 ea;
	BOOL_32 goodmac = FALSE;

	str [0] = '0';
	str [1] = 'x';
	str [4] = '\0';
	mac [6] = mac [7] = 0;

	/* An address was passed, use it */
	if (parse_get_entry_count() == 2) {
		curp = get_parsed_entry(1);
		goodmac = TRUE;
		offs = 0;
		for (idx = 0; idx < 6; idx++) {
			str [2] = curp[offs];
			str [3] = curp[offs + 1];
			goodmac &= str_hex_to_val(str, &ea);
			mac [idx] = (UNS_8) ea;
			offs += 3;
		}

		if (goodmac == TRUE) 
		{
			/* Yes NO VERIFY? */
			term_dat_out(smac_msg);
            if (prompt_yesno() != FALSE)
			{
				/* Save structure */
				for (idx = 0; idx < 8; idx++) {
					phyhwdesc.mac [idx] = mac [idx];
				}
				p8 = (UNS_8 *) &phyhwdesc;
				for (idx = 0; idx < sizeof(phyhwdesc); idx++) 
				{
					phy3250_sspwrite(*p8, (PHY3250_SEEPROM_CFGOFS + idx));
					p8++;
				}
			}
		}
		else
		{
			term_dat_out_crlf(macafail_msg);
		}
	}

	return goodmac;
}

/***********************************************************************
 *
 * Function: cmd_update
 *
 * Purpose: Updates the stage 1 bootloader
 *
 * Processing:
 *     See function.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: Always returns TRUE.
 *
 * Notes: None
 *
 **********************************************************************/
BOOL_32 cmd_update(void) 
{
	BOOL_32 updated = FALSE;

	/* Is an image in memory? */
	if (sysinfo.lfile.flt == FLT_NONE) 
	{
		term_dat_out_crlf(updnoim_msg);
		return TRUE;
	}

	/* Is an image in memory a raw image? */
	if (sysinfo.lfile.flt != FLT_RAW) 
	{
		term_dat_out_crlf(updnotraw_msg);
		return TRUE;
	}

	/* Is the image in memory contiguous? */
	if (sysinfo.lfile.contiguous == FALSE) 
	{
		term_dat_out_crlf(updnotc_msg);
		return TRUE;
	}

	/* Is the image for stage 1 or the kickstart loader */
	if (parse_get_entry_count() == 2) 
	{
		/* Get passed command */
		if (str_cmp(get_parsed_entry(1), "kick") == 0) 
		{
			/* Updating block 0, does the image exceed 15.5K(small block)? 
			*  large block 54K
			*/
			if (sysinfo.lfile.num_bytes > (31 * 512)) 
			{
				term_dat_out_crlf(updgt31_msg);
				return TRUE;
			}
			else 
			{
				updated = nand_kickstart_update();
			}
		}
	}
	else 
	{
		/* Updating block 0, does the image exceed 256K? */
		if (sysinfo.lfile.num_bytes > (256 * 512)) 
		{
			term_dat_out_crlf(updgt256_msg);
			return TRUE;
		}
		else 
		{
			updated = nand_bootloader_update();
		}
	}

	/* Image in memory passes all checks, FLASH it at start of
	   block 0 */
	if (updated == FALSE) 
	{
		term_dat_out_crlf(updfail_msg);
	}
	else 
	{
		term_dat_out_crlf(updgood_msg);
	}

	return TRUE;
}

/***********************************************************************
 *
 * Function: cmd_nrsv
 *
 * Purpose: Reserve a range of blocks for WinCE
 *
 * Processing:
 *     See function.
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
BOOL_32 cmd_nrsv(void) 
{
	UNS_32 lblock, fblock, nblks;

	/* Get arguments */
	fblock = cmd_get_field_val(1);
	nblks = cmd_get_field_val(2);

	/* Limit range outside of boot blocks */
	if (fblock < BL_NUM_BLOCKS)
	{
		/* Boot blocks are already marked as reserved, so skip them */

		/* Get last block to mark */
		lblock = (fblock + nblks) - 1;
		if (lblock < BL_NUM_BLOCKS)
		{
			nblks = 0;
		}
		else
		{
			fblock = 25;
			nblks = (lblock - fblock) + 1;
		}
	}

	/* Mark all of the blocks, skipping bad blocks */
	while (nblks > 0)
	{
		if (flash_reserve_block(fblock) == FALSE)
		{
			term_dat_out_crlf(blkmkerr_msg);
			nblks = 0;
		}
		else
		{
			fblock++;
			nblks--;
		}
	}

	return TRUE;
}

/***********************************************************************
 *
 * Function: cmd_nandrs
 *
 * Purpose: Displays a list of reserved NAND blocks
 *
 * Processing:
 *     See function.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: Always returns TRUE.
 *
 * Notes: None
 *
 **********************************************************************/
BOOL_32 cmd_nandrs(void) 
{
	UNS_8 blk [16];
	UNS_32 idx = 0;
	UNS_32 fblock = 0, nblks = 0;
	BOOL_32 ct, newct;

	if (sysinfo.nandgood == FALSE)
	{
		term_dat_out(noflash_msg);
	}
	else 
	{
		term_dat_out_crlf(rsvlist_msg);
		ct = flash_is_rsv_block((UNS_32) idx);
		while (idx < sysinfo.nandgeom.blocks) 
		{
			/* Read "extra" data area */
			newct = flash_is_rsv_block((UNS_32) idx);
			if ((newct != ct) || (idx == (sysinfo.nandgeom.blocks - 1)))
			{
				if (ct == FALSE)
				{
					term_dat_out("Nonreserved blocks :");
				}
				else
				{
					term_dat_out("Reserved blocks    :");
				}

				if (idx == (sysinfo.nandgeom.blocks - 1))
				{
					nblks++;
				}

				str_makedec(blk, fblock);
				term_dat_out(blk);
				term_dat_out("(");
				str_makedec(blk, nblks);
				term_dat_out(blk);
				term_dat_out_crlf(" block(s))");

				ct = newct;
				nblks = 0;
				fblock = (UNS_32) idx;
			}

			nblks++;
			idx++;
		}
	}

	return TRUE;
}

/***********************************************************************
 *
 * Function: hw_cmd_group_init
 *
 * Purpose: Initialize hardware command group
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
void hw_cmd_group_init(void)
{
	/* Add hw group */
	cmd_add_group(&hw_group);

	/* Add commands to the hw group */
	cmd_add_new_command(&hw_group, &hw_clock_cmd);
	cmd_add_new_command(&hw_group, &hw_maddr_cmd);
	cmd_add_new_command(&hw_group, &hw_update_cmd);
	cmd_add_new_command(&nand_group, &hw_nrsv_cmd);
	cmd_add_new_command(&nand_group, &hw_nandrs_cmd);
}
