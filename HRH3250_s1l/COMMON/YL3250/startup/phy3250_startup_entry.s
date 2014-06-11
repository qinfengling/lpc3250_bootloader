;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; $Id:: phy3250_startup_entry.s 661 2008-04-25 23:00:08Z wellsk        $
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
    export dcache_flush

    extern phy3250_init
    extern mmu_base_aadr
    extern |Image$$ER_ZI$$ZI$$Base|
    extern |Image$$ER_ZI$$ZI$$Length|

    ; This is the user application that is called by the startup code
    ; once board initialization is complete
    extern c_entry

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Private defines and data
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

MODE_USR    EQU 0x010
MODE_FIQ    EQU 0x011
MODE_IRQ    EQU 0x012
MODE_SVC    EQU 0x013
MODE_ABORT  EQU 0x017
MODE_UNDEF  EQU 0x01b
MODE_SYSTEM EQU 0x01f
MODE_BITS   EQU 0x01f
I_MASK      EQU 0x080
F_MASK      EQU 0x040

; End of internal RAM
END_OF_IRAM EQU 0x08040000

; Masks used to disable and enable the MMU and caches
MMU_DISABLE_MASK   EQU    0xFFFFEFFA
MMU_ENABLE_MASK    EQU    0x00001005

; Default stack sizes
FIQ_STACK_SIZE     EQU    512
IRQ_STACK_SIZE     EQU    1024
ABORT_STACK_SIZE   EQU    512
UNDEF_STACK_SIZE   EQU    512
SYSTEM_STACK_SIZE  EQU    512
SVC_STACK_SIZE     EQU    4096

	PRESERVE8
	AREA STARTUP, CODE   ; Startup code	
	ENTRY

; This initial handler is only for reset, a real application will
; replace this exception handler in memory with a more capable one
arm926ejs_reset
    B     arm926ejs_reset_handler ; Reset
arm926ejs_undef
    B     .
arm926ejs_swi
    B     .
arm926ejs_prefetch
    B     .
arm926ejs_abort
    B     .
arm926ejs_irq
    B     .
arm926ejs_fiq
    B     .

    SPACE   0x40 ; Extra space for when vector table is moved

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
arm926ejs_reset_handler
    ; Put the processor is in system mode with interrupts disabled
    MOV   r0, #MODE_SVC:OR:I_MASK:OR:F_MASK
    MSR   cpsr_cxsf, r0

    ; Ensure the MMU is disabled
    MRC   p15, 0, r1, c1, c0, 0
    LDR   r2,=MMU_DISABLE_MASK
    AND   r1, r1, r2
    MCR   p15, 0, r1, c1, c0, 0

    ; Invalidate TLBs
    MOV   r1,#0
    MCR   p15, 0, r1, c8, c7, 0

    ; Invalidate both caches
    MCR   p15, 0, r1, c7, c7, 0

    ; Enable instruction cache (for now)
    MRC   p15, 0, r1, c1, c0, 0
    LDR   r2,=0x1000
    ORR   r1, r1, r2
    MCR   p15, 0, r1, c1, c0, 0

    ; Get end of internal memory and set aside 16K for the page table
    LDR   r11, =END_OF_IRAM
    SUB   r11, r11, #(16*1024)
    LDR   r0, =mmu_base_aadr
    STR   r11, [r0] ; Must be aligned on a 16K boundary

    ; Setup current stack pointer to end of internal memory
    SUB   r11, r11, #4
    MOV   sp, r11

    ; Clear ZI segment
    LDR   r0, =|Image$$ER_ZI$$ZI$$Base|
    LDR   r1, =|Image$$ER_ZI$$ZI$$Length|
    MOV   r2, #0
clearzi
    CMP   r1, #0
    BEQ   clearzi_exit
    SUB   r1, r1, #4
    STR   r2, [r0]
    ADD   r0, r0, #4
    B clearzi

clearzi_exit
    ; Initial Phytec board
    BL    phy3250_init

    ; Setup the Domain Access Control as all Manager
    ; Make all domains open, user can impose restrictions
    MVN   r1, #0
    MCR   p15, 0, r1, c3, c0, 0
    
    ; Setup Translation Table Base
    LDR   r0, =mmu_base_aadr
    LDR   r3, [r0]
    MCR   p15, 0, r3, c2, c0, 0

    ; Setup jump to run out of virtual memory at location inVirtMem
    LDR   r5, =inVirtMem

    ; Enable the MMU with instruction and data caches enabled
    MRC   p15, 0, r1, c1, c0, 0
    LDR   r2,=MMU_ENABLE_MASK
    ORR   r1, r1, r2
    MCR   p15, 0, r1, c1, c0, 0

    ; Jump to the virtual address
    MOV   pc, r5

    ; The following NOPs are to clear the pipeline after the MMU virtual
    ; address jump
    NOP
    NOP
    NOP

inVirtMem
    ; The code is operating out of virtual memory now - register R3
    ; contains the virtual address for the top of stack space

    ; Initialize stacks for all modes
    ; All interrupts disabled at core for all modes
    MOV   r1, #I_MASK:OR:F_MASK ; No Interrupts

    ; Adjust stack top
    SUB   r3, r3, #4

    ; Enter FIQ mode and setup the FIQ stack pointer
    ORR   r0, r1, #MODE_FIQ
    MSR   cpsr_cxsf, r0
    MOV   sp, r3
    SUB   r3, r3, #FIQ_STACK_SIZE

    ; Enter IRQ mode and setup the IRQ stack pointer
    ORR   r0, r1, #MODE_IRQ
    MSR   cpsr_cxsf, r0
    MOV   sp, r3
    SUB   r3, r3, #IRQ_STACK_SIZE

    ; Enter Abort mode and setup the Abort stack pointer
    ORR   r0, r1, #MODE_ABORT
    MSR   cpsr_cxsf, r0
    MOV   sp, r3
    SUB   r3, r3, #ABORT_STACK_SIZE

    ; Enter Undefined mode and setup the Undefined stack pointer
    ORR   r0, r1, #MODE_UNDEF
    MSR   cpsr_cxsf, r0
    MOV   sp, r3
    SUB   r3, r3, #UNDEF_STACK_SIZE

    ; Enter System mode and setup the User/System stack pointer
    ORR   r0, r1, #MODE_SYSTEM
    MSR   cpsr_cxsf, r0
    MOV   sp, r3
    SUB   r3, r3, #SYSTEM_STACK_SIZE

    ; Enter SVC mode and setup the SVC stack pointer.
    ; This is the mode for runtime initialization.
    ORR   r0, r1, #MODE_SVC
    MSR   cpsr_cxsf, r0
    MOV   sp, r3
    SUB   r3, r3, #SVC_STACK_SIZE

    ; Get address of application to execute
    LDR   pc, =c_entry
    NOP

dcache_flush
	CMP   r0, #0
	BNE   inval

flushonly
	MRC  p15, 0, r15, c7, c10, 3
	BNE  flushonly
	MOV  pc, lr

inval
	MRC  p15, 0, r15, c7, c14, 3
	BNE  inval
	MOV  pc, lr

    END
