 /*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
 ; $Id:: ldscript_rom_gnu.ld 991 2008-08-05 21:05:10Z wellsk           $
 ; 
 ; Project:PHYL3250 startup code linker file for GNU toolset
 ; 
 ; Description: 
 ;     This file implements linker script for the PHY3250 board when
 ;     used with the PHY3250 startup code. The image will be linked to
 ;     execute out of address 0x00000000.
 ;
 ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;  
 ; Software that is described herein is for illustrative purposes only  
 ; which provides customers with programming information regarding the  
 ; products. This software is supplied "AS IS" without any warranties.  
 ; NXP Semiconductors assumes no responsibility or liability for the 
 ; use of the software, conveys no license or title under any patent, 
 ; copyright, or mask work right to the product. NXP Semiconductors 
 ; reserves the right to make changes in the software without 
 ; notification. NXP Semiconductors also make no representation or 
 ; warranty that such application will be suitable for the specified 
 ; use without further testing or modification. 
 ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;*/

/* Define the supported output formats - elf32-littlearm is the
   default */
OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")

/* Define the target architecture */
OUTPUT_ARCH(arm)

/* Define the system entry point */
ENTRY(arm926ejs_reset)

/* Define the memory layout for the board */
SECTIONS
{
    /* Set up the location counter */
    . = 0x00000000;

    /* Define a execution region that resides at ram base */
    .text :
    {
        /* Start of .text segment */
        __exidx_start = .;
        PROVIDE (__gnu_textstart = .);

        phy3250_kickstart.o *(.text) 		
    }

    /* Define a readonly data section in flash that resides at the
       end of the .rom execution region */
    .rodata : 
    { 
        /* Place all .rodata regions in this section. */
        *(.rodata); 
    }

    /* (Real) End of .text segment */
    __exidx_end = .;
    PROVIDE (__gnu_textend = .);

    /* Set up the location counter for the data area */
    . = .;
    .data :
/*                AT ( ADDR(.text) + SIZEOF(.text) + SIZEOF(.rodata) ) */
    {
        /* Start of .data segment */
        PROVIDE (__gnu_datastart = .);

        *(.data); 

        /* End of .data segment */
        PROVIDE (__gnu_dataend = .);
    }

    /* Define a ZI section at the end of the .data section */
    .bss :
    {
        /* Start of .bss segment */
        PROVIDE (__gnu_bssstart = .);

        *(.bss) *(COMMON); 

        /* End of .bss segment */
        PROVIDE (__gnu_bssend = .);
    }

    /* Define linker variable to reference end address of this
       section - required by gcc */
    PROVIDE (end = .);
    PROVIDE (__end = .);

    /* DWARF debug sections
       Symbols in the DWARF debugging sections are relative 
       to the beginning of the section so we begin them at 0 */
  	
    /* DWARF 1 */
    .debug          0 	: { *(.debug) }
    .line           0 	: { *(.line) }
  	
    /* GNU DWARF 1 extensions */
    .debug_srcinfo  0 	: { *(.debug_srcinfo) }
    .debug_sfnames  0 	: { *(.debug_sfnames) }
  	
    /* DWARF 1.1 and DWARF 2 */
    .debug_aranges  0 	: { *(.debug_aranges) }
    .debug_pubnames 0 	: { *(.debug_pubnames) }
  	
    /* DWARF 2 */
    .debug_info     0 	: { *(.debug_info) }
    .debug_abbrev   0 	: { *(.debug_abbrev) }
    .debug_line     0 	: { *(.debug_line) }
    .debug_frame    0 	: { *(.debug_frame) }
    .debug_str      0 	: { *(.debug_str) }
    .debug_loc      0 	: { *(.debug_loc) }
    .debug_macinfo  0 	: { *(.debug_macinfo) }
}

