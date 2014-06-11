;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; $Id:: phy3250_kickstart.s 460 2008-03-25 19:05:47Z wellsk            $
; 
; Project: Phytec 3250 board startup code
;
; Description:
;     Basic startup code for the Phytec 3250 board
;
; Notes:
;     This version of the file is for the Realview 3.x toolset.
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
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

    export arm926ejs_reset

    ; This is the user application that is called by the startup code
    ; once board initialization is complete
    extern c_entry

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Private defines and data
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

MODE_SVC    EQU 0x013
I_MASK      EQU 0x080
F_MASK      EQU 0x040

; End of internal RAM
END_OF_IRAM EQU 0x08040000

; Masks used to disable and enable the MMU and caches
MMU_DISABLE_MASK   EQU    0xFFFFEFFA

	PRESERVE8
	AREA STARTUP, CODE   ; Startup code	
	ENTRY

; This initial handler is only for reset, a real application will
; replace this exception handler in memory with a more capable one
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
; Function: arm926ejs_reset_handler
;
; Purpose: Reset vector entry point
;
; Description:
;     Place ARM core in supervisor mode and disable interrupts. Disable
;     the MMU and caches. Setup a basic stack pointer and call the
;     system init function to set up the memory interfaces, MMU
;     table, initialize code and data regions, and any other board
;     specific initialization. Setup the base address for the MMU
;     translation table and enable the MMU and caches. Setup stacks for
;     all ARM core modes and jump to the c_entry() function.
;
; Parameters: NA
;
; Outputs; NA
;
; Returns: NA
;
; Notes: NA
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
arm926ejs_reset
    ; Put the processor is in system mode with interrupts disabled
    MOV   r0, #MODE_SVC:OR:I_MASK:OR:F_MASK
    MSR   cpsr_cxsf, r0

    ; Ensure the MMU is disabled
    MRC   p15, 0, r1, c1, c0, 0
    LDR   r2,=MMU_DISABLE_MASK
    AND   r1, r1, r2
    MCR   p15, 0, r1, c1, c0, 0

clearzi_exit
    ; Enter SVC mode and setup the SVC stack pointer.
    ; This is the mode for runtime initialization.
    MOV   r1, #I_MASK:OR:F_MASK ; No Interrupts
    ORR   r0, r1, #MODE_SVC
    MSR   cpsr_cxsf, r0
    LDR   r11, =END_OF_IRAM
    MOV   sp, r11

	; Relocate 15.5K of image
	LDR   r0, =END_OF_IRAM
	LDR   r1, =0x8000
	SUB   r0, r0, r1
	MOV   r8, r0
	MOV   r2, #0
	LDR   r1, =0x4000

loopmove
	LDR   r3, [r2], #4
	STR   r3, [r0], #4
	SUB   r1, r1, #4
	CMP   r1, #0
	BNE   loopmove

    ; Get address of application to execute
    LDR   r0, =c_entry
    BX    r0

    END
