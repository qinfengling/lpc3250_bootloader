/*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; $Id:: phy3250_startup_entry.asm 1093 2008-08-18 22:18:52Z wellsk     $
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
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;*/

    .global arm926ejs_reset
    .global dcache_flush

    .global phy3250_init
    .global mmu_base_aadr
    .global __gnu_bssstart

    .global __gnu_bssend

    /*; This is the user application that is called by the startup code
    ; once board initialization is complete*/
    .global c_entry

/*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Private defines and data
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;*/

.EQU MODE_USR,    0x010
.EQU MODE_FIQ,    0x011
.EQU MODE_IRQ,    0x012
.EQU MODE_SVC,    0x013
.EQU MODE_SVC_NI, 0x0D3
.EQU MODE_ABORT,  0x017
.EQU MODE_UNDEF,  0x01b
.EQU MODE_SYSTEM, 0x01f
.EQU MODE_BITS,   0x01f
.EQU I_MASK,      0x080
.EQU F_MASK,      0x040
.EQU IF_MASK,     0x0C0

/*; End of internal RAM*/
.EQU END_OF_IRAM, 0x08040000

/*; Masks used to disable and enable the MMU and caches*/
.EQU MMU_DISABLE_MASK,   0xFFFFEFFA
.EQU MMU_ENABLE_MASK,    0x00001005

/*; Default stack sizes*/
.EQU FIQ_STACK_SIZE,     512
.EQU IRQ_STACK_SIZE,     1024
.EQU ABORT_STACK_SIZE,   512
.EQU UNDEF_STACK_SIZE,   512
.EQU SYSTEM_STACK_SIZE,  512
.EQU SVC_STACK_SIZE,     4096

	.text
	.code 32   /*; Startup code*/	
	.align 2

/*; This initial handler is only for reset, a real application will
; replace this exception handler in memory with a more capable one*/
arm926ejs_reset:
    B     arm926ejs_reset_handler /*; Reset*/
arm926ejs_undef:
    B     .
arm926ejs_swi:
    B     .
arm926ejs_prefetch:
    B     .
arm926ejs_abort:
    B     .
arm926ejs_irq:
    B     .
arm926ejs_fiq:
    B     .

    /*(it was) SPACE   0x40 ; Extra space for when vector table is moved*/
    .word 0, 0, 0, 0, 0, 0, 0, 0

/*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
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
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;*/
arm926ejs_reset_handler:
    /*; Put the processor is in system mode with interrupts disabled*/
    MOV   r0, #MODE_SVC_NI
    MSR   cpsr_cxsf, r0

    /*; Ensure the MMU is disabled*/
    MRC   p15, 0, r1, c1, c0, 0
    LDR   r2,=MMU_DISABLE_MASK
    AND   r1, r1, r2
    MCR   p15, 0, r1, c1, c0, 0

    /*; Invalidate TLBs*/
    MOV   r1,#0
    MCR   p15, 0, r1, c8, c7, 0

    /*; Invalidate both caches*/
    MCR   p15, 0, r1, c7, c7, 0

    /*; Enable instruction cache (for now)*/
    MRC   p15, 0, r1, c1, c0, 0
    LDR   r2,=0x1000
    ORR   r1, r1, r2
    MCR   p15, 0, r1, c1, c0, 0

    /*; Get end of internal memory and set aside 16K for the page table*/
    LDR   r11, =END_OF_IRAM
    SUB   r11, r11, #(16*1024)
    LDR   r0, =mmu_base_aadr
    STR   r11, [r0] /*Must be aligned on a 16K boundary*/

    /*; Setup current stack pointer to end of internal memory*/
    SUB   r11, r11, #4
    MOV   sp, r11

    /*; Clear ZI segment*/
    LDR   r0, =__gnu_bssstart /*Replace ZIBEGIN*/
    LDR   r1, =__gnu_bssend   /*Replace ZIEND*/
    MOV   r2, #0

clearzi:
    CMP   r1, #0
    BEQ   clearzi_exit
    STR   r2, [r0]
    ADD   r0, r0, #4
    B clearzi

clearzi_exit:
    /*; Initial Phytec board*/
    BL    phy3250_init

    /*; Setup the Domain Access Control as all Manager
    ; Make all domains open, user can impose restrictions*/
    MVN   r1, #0
    MCR   p15, 0, r1, c3, c0, 0
    
    /*; Setup Translation Table Base*/
    LDR   r0, =mmu_base_aadr
    LDR   r3, [r0]
    MCR   p15, 0, r3, c2, c0, 0

    /*; Setup jump to run out of virtual memory at location inVirtMem*/
    LDR   r5, =inVirtMem

    /*; Enable the MMU with instruction and data caches enabled*/
    MRC   p15, 0, r1, c1, c0, 0
    LDR   r2,=MMU_ENABLE_MASK
    ORR   r1, r1, r2
    MCR   p15, 0, r1, c1, c0, 0

    /*; Jump to the virtual address*/
    MOV   pc, r5

    /*; The following NOPs are to clear the pipeline after the MMU virtual
    ; address jump*/
    NOP
    NOP
    NOP

inVirtMem:
    /*; The code is operating out of virtual memory now - register R3
    ; contains the virtual address for the top of stack space
    ; Initialize stacks for all modes
    ; All interrupts disabled at core for all modes*/
    MOV   r1, #IF_MASK  /*; No Interrupts*/

    /*; Adjust stack top*/
    SUB   r3, r3, #4

    /*; Enter FIQ mode and setup the FIQ stack pointer*/
    ORR   r0, r1, #MODE_FIQ
    MSR   cpsr_cxsf, r0
    MOV   sp, r3
    SUB   r3, r3, #FIQ_STACK_SIZE

    /*; Enter IRQ mode and setup the IRQ stack pointer*/
    ORR   r0, r1, #MODE_IRQ
    MSR   cpsr_cxsf, r0
    MOV   sp, r3
    SUB   r3, r3, #IRQ_STACK_SIZE

    /*; Enter Abort mode and setup the Abort stack pointer*/
    ORR   r0, r1, #MODE_ABORT
    MSR   cpsr_cxsf, r0
    MOV   sp, r3
    SUB   r3, r3, #ABORT_STACK_SIZE

    /*; Enter Undefined mode and setup the Undefined stack pointer*/
    ORR   r0, r1, #MODE_UNDEF
    MSR   cpsr_cxsf, r0
    MOV   sp, r3
    SUB   r3, r3, #UNDEF_STACK_SIZE

    /*; Enter System mode and setup the User/System stack pointer*/
    ORR   r0, r1, #MODE_SYSTEM
    MSR   cpsr_cxsf, r0
    MOV   sp, r3
    SUB   r3, r3, #SYSTEM_STACK_SIZE

    /*; Enter SVC mode and setup the SVC stack pointer.
    ; This is the mode for runtime initialization.*/
    ORR   r0, r1, #MODE_SVC
    MSR   cpsr_cxsf, r0
    MOV   sp, r3
    SUB   r3, r3, #SVC_STACK_SIZE

    /*; Get address of application to execute*/
    LDR   pc, =c_entry
    NOP

dcache_flush:
	CMP   r0, #0
	BNE   inval

flushonly:
	MRC  p15, 0, r15, c7, c10, 3
	BNE  flushonly
	MOV  pc, lr

inval:
	MRC  p15, 0, r15, c7, c14, 3
	BNE  inval
	MOV  pc, lr

     /*;END */
     .END


