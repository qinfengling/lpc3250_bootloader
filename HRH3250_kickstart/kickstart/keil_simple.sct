 ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; 
 ; $Id:: ldscript_rom_rvw.ld 620 2008-04-18 19:32:45Z wellsk           $
 ; 
 ; Project: Phytec LPC3250 startup code linker file for ARM Realview
 ; 
 ; Description: 
 ;     This file implements linker script for the Phytec LPC3250 board
 ;     when used with the Phytec LPC3250 startup code. The image will
 ;     be linked to execute out of address 0x00000000.
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
 ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; 

; Kickstart links code at the last 32K of IRAM. Although the code starts
; execution at address 0x00000000, it is immediately relocated to
; address 0x00038000 and then execution continues.
FLASH 0x00038000
{
	; The executable region starts at the load region
    	ER_RO +0
        {
                phy3250_kickstart.o (STARTUP, +FIRST) 
                * (+RO) 
        }
        ER_RW +0
        {
                * (+RW)
        }
        ER_ZI +0        
        {
                * (+ZI)
        }
}
