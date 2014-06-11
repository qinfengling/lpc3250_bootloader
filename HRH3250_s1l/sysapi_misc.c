/***********************************************************************
 * $Id:: sysapi_misc.c 875 2008-07-08 17:27:04Z wellsk                 $
 *
 * Project: Command processor
 *
 * Description:
 *     Implements the following functions required for the S1L API
 *         ucmd_init
 *         cmd_info_extend
 *         get_seconds
 *         sys_up
 *         sys_down
 *         get_rt_s1lsys_cfg
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
#include "lpc_arm922t_cp15_driver.h"
#include "lpc_irq_fiq.h"
#include "phy3250_board.h"
#include "lpc_string.h"
#include "lpc32xx_rtc.h"
#include "sys.h"
#include "s1l_sys_inf.h"
#include "lpc32xx_intc_driver.h"
#include "lpc32xx_mstimer_driver.h"
#include "lpc32xx_clkpwr_driver.h"

/* Prototype for external IRQ handler */
void lpc32xx_irq_handler(void);

/* Millisecond timer device handles */
static INT_32 mstimerdev;

/* 10mS timer tick used for some timeout checks */
//extern volatile UNS_32 tick_10ms;

static UNS_8 mmu1_msg[] = "MMU : Enabled";
static UNS_8 mmu0_msg[] = "MMU : Disabled";
static UNS_8 armclk_msg[]  = "ARM system clock (Hz) = ";
static UNS_8 hclkclk_msg[] = "HCLK (Hz)             = ";
static UNS_8 pclkclk_msg[] = "Peripheral clock (Hz) = ";
static UNS_8 maca_msg[]   = "Ethernet MAC address: ";

/***********************************************************************
 *
 * Function: mstimer_user_interrupt
 *
 * Purpose: MSTIMER interrupt handler
 *
 * Processing:
 *     Clears the MSTIEMR interrupt and toggles the LED state.
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
static void mstimer_user_interrupt(void)
{
	static UNS_32 onoff1 = 0;

    /* Clear latched mstimer interrupt */
    mstimer_ioctl(mstimerdev, MST_CLEAR_INT, 0);

	/* Toggle LED1 connected to GPO_01 */
	if (onoff1 != (tick_10ms & 0x20))
	{
		onoff1 = tick_10ms & 0x20;
		if (onoff1 == 0) {
			phy3250_toggle_led(TRUE);
		}
		else
		{
			phy3250_toggle_led(FALSE);
		}
	}

	tick_10ms++;
}

void ucmd_init(void)
{
	mmu_cmd_group_init();
	hw_cmd_group_init();
}

/***********************************************************************
 *
 * Function: get_seconds
 *
 * Purpose: Get current hardware count value (in seconds)
 *
 * Processing:
 *     See function.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: A second reference value
 *
 * Notes: None
 *
 **********************************************************************/
UNS_32 get_seconds(void)
{
	return RTC->ucount;
}

/* Populate runtime system configuration data */
void get_rt_s1lsys_cfg(S1L_SYS_RTCFG_T *psysrt)
{
	str_copy(psysrt->default_prompt, DEFPROMPT);
	str_copy(psysrt->system_name, SYSHEADER);
	psysrt->bl_num_blks = BL_NUM_BLOCKS;
	psysrt->app_num_blks = FLASHAPP_MAX_BLOCKS;
}

/***********************************************************************
 *
 * Function: sys_up
 *
 * Purpose: Sets up system
 *
 * Processing:
 *     Sets up MS timer to generate an interrupt at 2Hz.
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
void sys_up(void)
{
	MST_MATCH_SETUP_T mstp;

    /* Disable interrupts in ARM core */
    disable_irq_fiq();

	/* Setup miscellaneous board functions */
	phy3250_board_init();

    /* Set virtual address of MMU table in last 16K of IRAM */
    cp15_set_vmmu_addr((void *)
		(IRAM_BASE + (256 * 1024) - (16 * 1024)));

	/* Initialize interrupt system */
    int_initialize(0xFFFFFFFF);

    /* Install standard IRQ dispatcher at ARM IRQ vector */
    int_install_arm_vec_handler(IRQ_VEC, (PFV) lpc32xx_irq_handler);
    enable_irq_fiq();

	/* Install exception handler */
    int_install_arm_vec_handler(UNDEFINED_INST_VEC, (PFV) arm9_exchand);
    int_install_arm_vec_handler(PREFETCH_ABORT_VEC, (PFV) arm9_exchand);
    int_install_arm_vec_handler(DATA_ABORT_VEC, (PFV) arm9_exchand);

    /* Install mstimer interrupts handlers as a IRQ interrupts */
    int_install_irq_handler(IRQ_MSTIMER, (PFV) mstimer_user_interrupt);

    /* Setup MS timer to toggle LED */
    mstimerdev = mstimer_open(MSTIMER, 0);
	if (mstimerdev != 0)
	{
		/* Set a 10mS match rate */
		mstp.timer_num      = 0;
		mstp.use_int        = TRUE;
		mstp.stop_on_match  = FALSE;
		mstp.reset_on_match = TRUE;
		mstp.tick_val       = 0;
		mstp.ms_val         = 10;
		mstimer_ioctl(mstimerdev, MST_CLEAR_INT, 0);
		mstimer_ioctl(mstimerdev, MST_TMR_SETUP_MATCH, (INT_32) &mstp);

		/* Reset terminal count */
		mstimer_ioctl(mstimerdev, MST_TMR_RESET, 0);

		/* Enable mstimer (starts counting) */
	    mstimer_ioctl(mstimerdev, MST_TMR_ENABLE, 1);

		/* Enable mstimer interrupts in the interrupt controller */
		int_enable(IRQ_MSTIMER);
	}

	/* Initialize terminal I/O */
	term_init();

	/* Get saved system parameters - some commands use and sve their
	   own parameters outside the S1L core values */
//	if (hw_get() == FALSE) 
//	{
//		hw_save();
//	}

	/* Enable SDCard slot power */
	phy3250_sdpower_enable(TRUE);

	/* Setup system clocking */
	clock_adjust();
}

/***********************************************************************
 *
 * Function: sys_down
 *
 * Purpose: Closes down system
 *
 * Processing:
 *     Disable MS timer and timer interrupt.
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
void sys_down(void)
{
	/* Shut down terminal I/O */
	term_deinit();

	/* Disable mstimer interrupts in the interrupt controller */
	int_disable(IRQ_MSTIMER);

	/* Close mstimer */
    mstimer_close(mstimerdev);

	phy3250_sdpower_enable(FALSE);

    /* Disable interrupts in ARM core */
    disable_irq_fiq();
}

void cmd_info_extend(void)
{
	UNS_8 str [128];
	int idx;
	UNS_32 tmp;

	/* MMU enabled? */
	if (cp15_mmu_enabled() == FALSE) 
	{
		term_dat_out_crlf(mmu0_msg);
	}
	else 
	{
		term_dat_out_crlf(mmu1_msg);
	}

	/* System clock speeds */
	term_dat_out(armclk_msg);
	tmp = clkpwr_get_base_clock_rate(CLKPWR_ARM_CLK);
	str_makedec(str, tmp);
	term_dat_out_crlf(str);
	term_dat_out(hclkclk_msg);
	tmp = clkpwr_get_base_clock_rate(CLKPWR_HCLK);
	str_makedec(str, tmp);
	term_dat_out_crlf(str);
	term_dat_out(pclkclk_msg);
	tmp = clkpwr_get_base_clock_rate(CLKPWR_PERIPH_CLK);
	str_makedec(str, tmp);
	term_dat_out_crlf(str);

	/* MAC address */
	term_dat_out(maca_msg);
	for (idx = 0; idx < 6; idx++) {
		str_makehex(str, (UNS_32) phyhwdesc.mac [idx], 2);
		term_dat_out_len(&str[2], 2);
		if (idx < 5) {
			term_dat_out(":");
		}
	}
	term_dat_out_crlf("");
}

/* Jump to passed address (execute program), requires cache flush on
   some systems */
void jumptoprog(PFV progaddr)
{
	/* Execute new program */
	dcache_flush(FALSE);
	icache_invalid();
	progaddr();
}
