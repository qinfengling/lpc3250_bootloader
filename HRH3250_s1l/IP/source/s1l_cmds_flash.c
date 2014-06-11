/***********************************************************************
 * $Id:: s1l_cmds_flash.c 978 2008-07-29 19:14:57Z wellsk              $
 *
 * Project: NAND group commands
 *
 * Description:
 *     NAND group commands
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
#include "s1l_cmds.h"
#include "s1l_line_input.h"
#include "s1l_sys_inf.h"
#include "s1l_sys.h"

/* erase command */
BOOL_32 cmd_erase(void);
static UNS_32 cmd_erase_plist[] =
{
	(PARSE_TYPE_STR), /* The "erase" command */
	(PARSE_TYPE_DEC),
	(PARSE_TYPE_DEC),
	(PARSE_TYPE_DEC | PARSE_TYPE_END)
};
static CMD_ROUTE_T nand_erase_cmd =
{
	"erase",
	cmd_erase,
	"Erases a range of FLASH blocks",
	"erase [starting block][number of blocks][0, 1(erase bad blocks)]",
	cmd_erase_plist,
	NULL
};

/* write command */
BOOL_32 cmd_write(void);
static UNS_32 cmd_write_plist[] =
{
	(PARSE_TYPE_STR), /* The "write" command */
	(PARSE_TYPE_HEX),
	(PARSE_TYPE_DEC),
	(PARSE_TYPE_DEC),
	(PARSE_TYPE_DEC | PARSE_TYPE_END)
};
static CMD_ROUTE_T nand_write_cmd =
{
	"write",
	cmd_write,
	"Writes raw data from memory to FLASH",
	"write [hex address][first sector][sectors to write][0, 1(write bad blocks)]",
	cmd_write_plist,
	NULL
};

/* read command */
BOOL_32 cmd_read(void);
static UNS_32 cmd_read_plist[] =
{
	(PARSE_TYPE_STR), /* The "read" command */
	(PARSE_TYPE_HEX),
	(PARSE_TYPE_DEC),
	(PARSE_TYPE_DEC),
	(PARSE_TYPE_DEC | PARSE_TYPE_END)
};
static CMD_ROUTE_T nand_read_cmd =
{
	"read",
	cmd_read,
	"Reads raw data from FLASH to memory",
	"read [hex address][first sector][Sectors to read][0, 1(read bad blocks)]",
	cmd_read_plist,
	NULL
};

/* nandbb command */
BOOL_32 cmd_nandbb(void);
static UNS_32 cmd_nandbb_plist[] =
{
	(PARSE_TYPE_STR | PARSE_TYPE_END) /* The "nandbb" command */
};
static CMD_ROUTE_T nand_nandbb_cmd =
{
	"nandbb",
	cmd_nandbb,
	"Displays a list of bad NAND blocks",
	"nandbb",
	cmd_nandbb_plist,
	NULL
};

/* nsave command */
BOOL_32 cmd_nsave(void);
static UNS_32 cmd_nsave_plist[] =
{
	(PARSE_TYPE_STR | PARSE_TYPE_END) /* The "nsave" command */
};
static CMD_ROUTE_T nand_nsave_cmd =
{
	"nsave",
	cmd_nsave,
	"Saves the current binary image loaded in memory into FLASH",
	"nsave",
	cmd_nsave_plist,
	NULL
};

/* nload command */
BOOL_32 cmd_nload(void);
static UNS_32 cmd_nload_plist[] =
{
	(PARSE_TYPE_STR | PARSE_TYPE_END) /* The "nload" command */
};
static CMD_ROUTE_T nand_nload_cmd =
{
	"nload",
	cmd_nload,
	"Loads the binary image stored in FLASH into memory",
	"nload",
	cmd_nload_plist,
	NULL
};

/* nburn command */
BOOL_32 cmd_nburn(void);
static UNS_32 cmd_nburn_plist[] =
{
	(PARSE_TYPE_STR), /* The "nburn" command */
	(PARSE_TYPE_DEC),
	(PARSE_TYPE_DEC | PARSE_TYPE_END)
};
static CMD_ROUTE_T nand_nburn_cmd =
{
	"nburn",
	cmd_nburn,
	"Stored the current binary image in memory to FLASH",
	"nburn [starting block][0, 1(write bad blocks)]",
	cmd_nburn_plist,
	NULL
};

/* NAND group */
GROUP_LIST_T nand_group =
{
	"nand", /* NAND group */
	"NAND command group",
	NULL,
	NULL
};

static UNS_8 blkrangeerr_msg[] = "Block range is invalid";
static UNS_8 blkov_msg[] = "Operation will overwrite bootloader - ok?";
static UNS_8 starter_msg[] = "Starting block erase";
static UNS_8 ddbblist_msg[] = "Displaying NAND bad block list";
static UNS_8 bbat_msg[] = "Bad block at index ";
static UNS_8 fnldd_msg[] = "No image loaded to burn into FLASH";
static UNS_8 updnotc_msg[] = "Image in memory is not contiguous";
static UNS_8 tobig_msg[] = "Loaded image will not fit in FLASH";
static UNS_8 wr1_msg[] = "Bytes written     :";
static UNS_8 wr2_msg[] = "Last block        :";
static UNS_8 wr3_msg[] = "First sector      :";
static UNS_8 wr4_msg[] = "Number of sectors :";
extern UNS_8 noflash_msg[];

/***********************************************************************
 *
 * Function: check_bl_range
 *
 * Purpose: Returns TRUE if the block erase can be done
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     block_st : Starting block for erase to check
 *
 * Outputs: None
 *
 * Returns: TRUE if the block can be erased
 *
 * Notes: None
 *
 **********************************************************************/
static BOOL_32 check_bl_range(UNS_32 block_st)
{
	BOOL_32 ov = TRUE;

	/* Determine if the bootloader going to be overwritten */
	if (block_st < sysinfo.sysrtcfg.bl_num_blks)
	{
		term_dat_out_len(blkov_msg, str_size(blkov_msg));
		ov = prompt_yesno();
		term_dat_out_crlf("");
	}

	return ov;
}

/***********************************************************************
 *
 * Function: cmd_erase
 *
 * Purpose: Erase a range of NAND blocks
 *
 * Processing:
 *     See function.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: Always return TRUE
 *
 * Notes:
 *     erase [starting block][number of blocks][0, 1(erase bad blocks)]
 *
 **********************************************************************/
static BOOL_32 cmd_erase(void) {
	UNS_32 first, numblks, forceerase;
	BOOL_32 erase;

	/* Get arguments */
	first = cmd_get_field_val(1);
	numblks = cmd_get_field_val(2);
	forceerase = cmd_get_field_val(3);

	if (sysinfo.nandgood == FALSE)
	{
		term_dat_out(noflash_msg);
	}
	else if ((first + numblks) > sysinfo.nandgeom.blocks)
	{
		term_dat_out(blkrangeerr_msg);
	}
	else
	{
		/* Verify boot loader won't be overwritten */
		erase = check_bl_range(first);
		if (erase == TRUE)
		{
			/* Erase */
			term_dat_out(starter_msg);
			while (numblks > 0)
			{
				flash_erase_block(first, (forceerase == 1));
				first++;
				numblks--;
			}
		}
	}

	return TRUE;
}

/***********************************************************************
 *
 * Function: cmd_read
 *
 * Purpose: Read data from NAND into memory
 *
 * Processing:
 *     Load data from FLASH into memory
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: TRUE if the FLASH was read, otherwise FALSE
 *
 * Notes:
 *     read [hex address][first sector][Sectors to read][0, 1(skip bad blocks)]
 *
 **********************************************************************/
BOOL_32 cmd_read(void) {
	UNS_32 first, hexaddr, sectors, forceread;

	/* Get arguments */
	hexaddr = cmd_get_field_val(1);
	first = cmd_get_field_val(2);
	sectors = cmd_get_field_val(3);
	forceread = cmd_get_field_val(4);

	nand_to_mem(first, (void *) hexaddr,
		(sectors * sysinfo.nandgeom.data_bytes_per_sector),
		(forceread == 1));

	return TRUE;
}

/***********************************************************************
 *
 * Function: cmd_write
 *
 * Purpose: Move data into NAND from memory
 *
 * Processing:
 *     Load data from memory into FLASH
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: TRUE
 *
 * Notes:
 *     write [hex address][first sector][sectors to write][0, 1(skip bad blocks)]"
 *
 **********************************************************************/
BOOL_32 cmd_write(void) {
	UNS_32 first, hexaddr, sectors, forcewrite;

	/* Get arguments */
	hexaddr = cmd_get_field_val(1);
	first = cmd_get_field_val(2);
	sectors = cmd_get_field_val(3);
	forcewrite = cmd_get_field_val(4);

	mem_to_nand(first, (void *) hexaddr,
		(sectors * sysinfo.nandgeom.data_bytes_per_sector),
		(forcewrite == 1));

	return TRUE;
}

/***********************************************************************
 *
 * Function: cmd_nandbb
 *
 * Purpose: Displays a list of bad blocks in NAND
 *
 * Processing:
 *     Read page 0 of each NAND block and check the bad block marker.
 *     Display a message if the block is bad.
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
BOOL_32 cmd_nandbb(void) 
{
	UNS_8 blk [16];
	UNS_32 idx = 0;

	if (sysinfo.nandgood == FALSE)
	{
		term_dat_out(noflash_msg);
	}
	else 
	{
		term_dat_out_crlf(ddbblist_msg);
		while (idx < sysinfo.nandgeom.blocks) 
		{
			if (flash_is_bad_block(idx) != FALSE)
			{
				term_dat_out(bbat_msg);
				str_makedec(blk, idx);
				term_dat_out_crlf(blk);
			}
		
			idx++;
		}
	}

	return TRUE;
}

/***********************************************************************
 *
 * Function: cmd_nsave
 *
 * Purpose: Saves image in memory to FLASH
 *
 * Processing:
 *     See function.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: TRUE if the command was processed NAND ok.
 *
 * Notes: None
 *
 **********************************************************************/
BOOL_32 cmd_nsave(void) 
{
	if (sysinfo.lfile.loaded == FALSE) 
	{
		term_dat_out_crlf(fnldd_msg);
		return TRUE;
	}

	if (sysinfo.lfile.contiguous == FALSE) 
	{
		term_dat_out_crlf(updnotc_msg);
		return TRUE;
	}

	flash_image_save();

	return TRUE;
}

/***********************************************************************
 *
 * Function: cmd_nload
 *
 * Purpose: Loads image from FLASH into memory
 *
 * Processing:
 *     See function.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: Returns TRUE if the image was loaded, otherwise FALSE
 *
 * Notes: None
 *
 **********************************************************************/
BOOL_32 cmd_nload(void) 
{
	/* Load image */
	flash_image_load();

	return TRUE;
}

/***********************************************************************
 *
 * Function: cmd_nburn
 *
 * Purpose: Burns a loaded image into NAND FLASH
 *
 * Processing:
 *     See function.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: Returns TRUE if the command was good, otherwise FALSE
 *
 * Notes: None
 *
 **********************************************************************/
BOOL_32 cmd_nburn(void)
{
	UNS_8 str[16];
	UNS_32 numsecs, forcewrite;
	UNS_32 block, fblock, fsector, sector, nblks;

	if (sysinfo.nandgood == FALSE) 
	{
		term_dat_out_crlf(noflash_msg);
	}
	else
	{
		block = cmd_get_field_val(1);
		forcewrite = cmd_get_field_val(2);

		if (sysinfo.lfile.loaded == FALSE) 
		{
			term_dat_out_crlf(fnldd_msg);
			return TRUE;
		}

		if (sysinfo.lfile.contiguous == FALSE) 
		{
			term_dat_out_crlf(updnotc_msg);
			return TRUE;
		}

		/* Get first sector */
		fblock = block;
		fsector = conv_to_sector(block, 0);

		/* Get number of sectors for image */
		numsecs = sysinfo.lfile.num_bytes /
			sysinfo.nandgeom.data_bytes_per_sector;
		if ((numsecs * sysinfo.nandgeom.data_bytes_per_sector) <
			sysinfo.lfile.num_bytes) 
		{
			numsecs++;
		}

		/* Will exceed end of FLASH? */
		if ((fsector + numsecs) >= (sysinfo.nandgeom.sectors_per_block *
			sysinfo.nandgeom.blocks))
		{
			term_dat_out_crlf(tobig_msg);
			return TRUE;
		}

		/* Erase necessary blocks first */
		nblks = numsecs / sysinfo.nandgeom.sectors_per_block;
		if ((nblks * sysinfo.nandgeom.sectors_per_block) < numsecs)
		{
			nblks++;
		}
		while (nblks > 0) 
		{
			if ((flash_is_bad_block(fblock) == FALSE) || (forcewrite == 1))
			{
				flash_erase_block(fblock, (forcewrite == 1));
				nblks--;
			}

			fblock++;
		}

		sector = conv_to_sector(block, 0);
		mem_to_nand(sector, (void *) sysinfo.lfile.loadaddr,
			sysinfo.lfile.num_bytes, (forcewrite == 1));

		/* Display statistics */
		term_dat_out(wr1_msg);
		str_makedec(str, sysinfo.lfile.num_bytes);
		term_dat_out_crlf(str);
		term_dat_out(wr2_msg);
		str_makedec(str, (fblock - 1));
		term_dat_out_crlf(str);
		term_dat_out(wr3_msg);
		str_makedec(str, fsector);
		term_dat_out_crlf(str);
		term_dat_out(wr4_msg);
		str_makedec(str, numsecs);
		term_dat_out_crlf(str);
	}

	return TRUE;
}

/***********************************************************************
 *
 * Function: cmd_nand_add_commands
 *
 * Purpose: Initialize this command block
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
void cmd_nand_add_commands(void)
{
	/* Add NAND group */
	cmd_add_group(&nand_group);

	/* Add commands to the NAND group */
	cmd_add_new_command(&nand_group, &nand_erase_cmd);
	cmd_add_new_command(&nand_group, &nand_nandbb_cmd);
	cmd_add_new_command(&nand_group, &nand_nburn_cmd);
	cmd_add_new_command(&nand_group, &nand_nload_cmd);
	cmd_add_new_command(&nand_group, &nand_nsave_cmd);
	cmd_add_new_command(&nand_group, &nand_read_cmd);
	cmd_add_new_command(&nand_group, &nand_write_cmd);
}
