/***********************************************************************
 * $Id:: phy3250_board.c 1016 2008-08-06 18:35:51Z wellsk              $
 *
 * Project: Phytec LPC3250 board functions
 *
 * Description:
 *     This file contains driver support for various Phytect LPC3250
 *     board functions.
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

#include "phy3250_board.h"
#include "lpc32xx_gpio_driver.h"
#include "lpc32xx_clkpwr_driver.h"
#include "lpc32xx_ssp_driver.h"

/* SDRASM configurations */
PHY_DRAM_CFG_T dram_cfg [4] =
{
  /* MT48H4M16LFB4-8 IT (x2 chips -> 16MB) */
  {SDRAM_ADDRESS_MAP_16MB, DRAM_MODE_VAL_16MB,
    DRAM_EMODE_VAL_16MB, 6, 2,
    64000,    /* tdynref = 16uS between refreshes (64mS/4096)*/
    41666666, /* trp = 24nS */
    20833333, /* tras = 48nS */
    12500000, /* tsrex = 80nS */
    66666666, /* twr = 15nS */
    13888888, /* trc = 72nS */
    12500000, /* trfc = 80nS */
    12500000, /* txsr = 80nS */
    62500000, /* trrd = 16nS */
    2,        /* tmrd = 2 clock periods */
    1,        /* tcdlr = 1 clock period */},

  /* MT48H8M16LFB4-8 IT (x2 chips -> 32MB) */
  {SDRAM_ADDRESS_MAP_32MB, DRAM_MODE_VAL_32MB,
   DRAM_EMODE_VAL_32MB, 6, 2,
   64000,    /* tdynref = 16uS between refreshes (64mS/4096)*/
   41666666, /* trp = 24nS */
   20833333, /* tras = 48nS */
   12500000, /* tsrex = 80nS */
   66666666, /* twr = 15nS */
   13888888, /* trc = 72nS */
   12500000, /* trfc = 80nS */
   12500000, /* txsr = 80nS */
   62500000, /* trrd = 16nS */
   2,        /* tmrd = 2 clock periods */
   1,        /* tcdlr = 1 clock period */},

  /*  MT48H16M16LFBF-8 IT (x2 chips -> 64MB) */
  {SDRAM_ADDRESS_MAP_64MB, DRAM_MODE_VAL_64MB,
   DRAM_EMODE_VAL_64MB, 6, 2,
   128000,   /* tdynref = 16uS between refreshes (64mS/8192)*/
   52631578, /* trp = 19nS */
   20833333, /* tras = 48nS */
   12500000, /* tsrex = 80nS */
   66666666, /* twr = 15nS */
   13888888, /* trc = 72nS */
   10256410, /* trfc = 97.5nS */
   12500000, /* txsr = 80nS */
   2,        /* trrd = 2 clock periods (vs time) */
   2,        /* tmrd = 2 clock periods */
   1,        /* tcdlr = 1 clock period */},

  /* MT48H32M16LFBF-8 IT (x2 chips -> 128MB) */
  {SDRAM_ADDRESS_MAP_128MB, DRAM_MODE_VAL_128MB,
   DRAM_EMODE_VAL_128MB, 6, 2,
   128000,   /* tdynref = 16uS between refreshes (64mS/8192)*/
   52631578, /* trp = 19nS */
   22222222, /* tras = 45nS */
   8333333,  /* tsrex = 120nS */
   66666666, /* twr = 15nS */
   14814814, /* trc = 67.5nS */
   10256410, /* trfc = 97.5nS */
   8333333,  /* txsr = 120nS */
   2,        /* trrd = 2 clock periods (vs time) */
   2,        /* tmrd = 2 clock periods */
   1,        /* tcdlr = 1 clock period */}
};

/* Phytec hardware descriptor */
PHY_HW_T phyhwdesc;

/* SSP driver flag */
static INT_32 sspid;

/***********************************************************************
 * Private functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: phy3250_sspconfig
 *
 * Purpose: Config SSP if it hasn't already been configured
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
static void phy3250_sspconfig(void)
{
  SSP_CONFIG_T sspcfg;

  if (sspid == 0)
  {
    /* Try to open SSP driver */
    sspcfg.databits           = 8;
    sspcfg.mode               = SSP_CR0_FRF_SPI;
    sspcfg.highclk_spi_frames = FALSE;
    sspcfg.usesecond_clk_spi  = FALSE;
    sspcfg.ssp_clk            = 5000000;
    sspcfg.master_mode        = TRUE;
    sspid = ssp_open(SSP0, (INT_32) & sspcfg);
    ssp_ioctl(sspid, SSP_ENABLE, 1);
  }
}

/***********************************************************************
 *
 * Function: phy3250_sspxfer
 *
 * Purpose: Transfer data to/from the serial EEPROM
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     out   : Output data
 *     in    : Input data
 *     bytes : Number of bytes to send/receive
 *
 * Outputs: None
 *
 * Returns: TRUE if the byte was transferred
 *
 * Notes: Do not use this function to transfer more than 8 bytes!
 *
 **********************************************************************/
static BOOL_32 phy3250_sspxfer(UNS_8 *out,
                               UNS_8 *in,
                               int bytes)
{
  INT_32 rbytes = 0;
  BOOL_32 xfrd = FALSE;

  phy3250_sspconfig();
  if (sspid != 0)
  {
    /* Asset chip select */
    GPIO->p3_outp_clr = OUTP_STATE_GPIO(5);
    ssp_write(sspid, out, bytes);
    while (rbytes < bytes)
    {
      rbytes += ssp_read(sspid, &in [rbytes], 1);
    }
    GPIO->p3_outp_set = OUTP_STATE_GPIO(5);
    xfrd = TRUE;
  }

  return xfrd;
}

/***********************************************************************
 * Public functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: phy3250_board_init
 *
 * Purpose: Initializes basic board functions
 *
 * Processing:
 *     Does nothing
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
void phy3250_board_init(void)
{
  ;
}

/***********************************************************************
 *
 * Function: sram_adjust_timing
 *
 * Purpose: Adjust system board timings for SRAM
 *
 * Processing:
 *     Based on the current HCLK, reset timings to optimal.
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
void sram_adjust_timing(void)
{
  UNS_32 clk, syscfg;

  /* Get clock rate (HCLK) for computing times */
  clk = clkpwr_get_base_clock_rate(CLKPWR_HCLK);

  /* TBD for now, need to read system configuartion word */
  syscfg = PHYHW_SDIO_POP;

  /* If SDIO is populated, adjust timings for it */
  if ((syscfg & PHYHW_SDIO_POP) != 0)
  {
    EMC->emcstatic_regs[1].emcstaticconfig = PHY_SDIO_MEM_CFG;
    /* Clocks between nCS and start of nWE (1) */
    EMC->emcstatic_regs[0].emcstaticwaitwen =
      (clk / PHY_NOR_CS_TO_WE);
    /* Clocks between nCS and start of nOE (1) */
    EMC->emcstatic_regs[0].emcstaticwait0en =
      (clk / PHY_NOR_CS_TO_OE);
    /* First read delay for non-page and page mode */
    EMC->emcstatic_regs[0].emcstaticwaitrd =
      (clk / PHY_NOR_CS_TO_OUT);
    /* Page mode burst read timing (not used) is configured for
       standard read timing, used in page mode for reads 2, 3,
       and 4 */
    EMC->emcstatic_regs[0].emcstaticpage =
      (clk / PHY_NOR_OE_BURST);
    /* First write delay for non-page and page mode */
    EMC->emcstatic_regs[0].emcstaticwr =
      (clk / PHY_NOR_WE_HOLD_TIME);
    /* Bus turnaround delay is time from release of nOE to
       high-z */
    EMC->emcstatic_regs[0].emcstaticturn =
      (clk / PHY_NOR_OE_TO_HIZ);
  }

  /* NOR FLASH on chip select 0 */
  EMC->emcstatic_regs[0].emcstaticconfig = PHY_NOR_MEM_CFG;
  /* Clocks between nCS and start of nWE (1) */
  EMC->emcstatic_regs[0].emcstaticwaitwen = (clk / PHY_NOR_CS_TO_WE);
  /* Clocks between nCS and start of nOE (1) */
  EMC->emcstatic_regs[0].emcstaticwait0en = (clk / PHY_NOR_CS_TO_OE);
  /* First read delay for non-page and page mode */
  EMC->emcstatic_regs[0].emcstaticwaitrd = (clk / PHY_NOR_CS_TO_OUT);
  /* Page mode burst read timing (not used) is configured for standard
     read timing, used in page mode for reads 2, 3, and 4 */
  EMC->emcstatic_regs[0].emcstaticpage = (clk / PHY_NOR_OE_BURST);
  /* First write delay for non-page and page mode */
  EMC->emcstatic_regs[0].emcstaticwr = (clk / PHY_NOR_WE_HOLD_TIME);
  /* Bus turnaround delay is time from release of nOE to high-z */
  EMC->emcstatic_regs[0].emcstaticturn = (clk / PHY_NOR_OE_TO_HIZ);
}

/***********************************************************************
 *
 * Function: sdram_adjust_timing
 *
 * Purpose: Adjust system board timings for DRAM
 *
 * Processing:
 *     Does nothing
 *
 * Parameters:
 *     psdrcfg : Pointer to DRAM timing data
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes:
 *     These timings should only be adjusted when SDRAM is idle!
 *
 **********************************************************************/
void sdram_adjust_timing(PHY_DRAM_CFG_T *psdrcfg)
{
  UNS_32 clk;

  /* Get SDRAM clock rate (HCLK) */
  clk = clkpwr_get_clock_rate(CLKPWR_SDRAMDDR_CLK);

  /* Setup precharge command delay */
  EMC->emcdynamictrp =
    EMC_DYN_PRE_CMD_PER(clk / psdrcfg->trp);

  /* Setup Dynamic Memory Active to Precharge Command period */
  EMC->emcdynamictras = EMC_DYN_ACTPRE_CMD_PER(clk / psdrcfg->tras);

  /* Dynamic Memory Self-refresh Exit Time */
  EMC->emcdynamictsrex = EMC_DYN_SELF_RFSH_EXIT(clk / psdrcfg->tsrex);

  /* Dynamic Memory write recovery Time */
  EMC->emcdynamictwr = EMC_DYN_WR_RECOVERT_TIME(clk / psdrcfg->twr);

  /* Dynamic Memory Active To Active Command Period */
  EMC->emcdynamictrc = EMC_DYN_ACT2CMD_PER(clk / psdrcfg->trc);

  /* Dynamic Memory Auto-refresh Period */
  EMC->emcdynamictrfc = EMC_DYN_AUTOREFRESH_PER(clk / psdrcfg->trfc);

  /* Dynamic Memory Active To Active Command Period */
  EMC->emcdynamictxsr = EMC_DYN_EXIT_SRFSH_TIME(clk / psdrcfg->txsr);

  /* Dynamic Memory Active Bank A to Active Bank B Time */
  if (psdrcfg->trrd < 16)
  {
    EMC->emcdynamictrrd =
      EMC_DYN_BANKA2BANKB_LAT(psdrcfg->trrd - 1);
  }
  else
  {
    EMC->emcdynamictrrd =
      EMC_DYN_BANKA2BANKB_LAT(clk / psdrcfg->trrd);
  }

  /* Dynamic Memory Load Mode Register To Active Command Time */
  EMC->emcdynamictmrd = EMC_DYN_LM2ACT_CMD_TIME(psdrcfg->tmrd - 1);

  /* Dynamic Memory Last Data In to Read Command Time */
  EMC->emcdynamictcdlr = EMC_DYN_LASTDIN_CMD_TIME(psdrcfg->tcdlr - 1);

  /* Dynamic refresh */
  EMC->emcdynamicrefresh =
    EMC_DYN_REFRESH_IVAL(clk / psdrcfg->tdynref);
}

/***********************************************************************
 *
 * Function: phy3250_toggle_led
 *
 * Purpose: Toggles LED
 *
 * Processing:
 *     Toggles LED on the board based on the on value.
 *
 * Parameters:
 *     on : TRUE to enable LED, FALSE to disable
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void phy3250_toggle_led(BOOL_32 on)
{
  UNS_32 set, clr;

  if (on == FALSE)
  {
    set = 0;
    clr = OUTP_STATE_GPO(1);
  }
  else
  {
    set = OUTP_STATE_GPO(1);
    clr = 0;
  }

  /* Set LED2 on GPO_O1 */
  gpio_set_gpo_state(set, clr);
}

/***********************************************************************
 *
 * Function: phy3250_sspwrite
 *
 * Purpose: Write a byte to the serial EEPROM
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
void phy3250_sspwrite(UNS_8 byte,
                      int index)
{
  UNS_8 prog, datai [8], datao [8];

  /* Write enable */
  datao [0] = SEEPROM_WREN;
  datao [1] = 0xFF;
  phy3250_sspxfer(datao, datai, 2);

  /* Write byte */
  datao [0] = SEEPROM_WRITE;
  datao [1] = (UNS_8)((index >> 8) & 0xFF);
  datao [2] = (UNS_8)((index >> 0) & 0xFF);
  datao [3] = byte;
  phy3250_sspxfer(datao, datai, 4);

  /* Wait for device to finish programming */
  prog = 0xFF;
  while ((prog & 0x2) != 0)
  {
    /* Read status */
    datao [0] = SEEPROM_RDSR;
    datao [1] = 0xFF;
    phy3250_sspxfer(datao, datai, 2);
    prog = datai [1];
  }
}

/***********************************************************************
 *
 * Function: phy3250_sspread
 *
 * Purpose: Read a byte from the serial EEPROM
 *
 * Processing:
 *     See function.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: Byte read from passed index
 *
 * Notes: None
 *
 **********************************************************************/
UNS_8 phy3250_sspread(int index)
{
  UNS_8 datai [8], datao [8];
  UNS_8 byte = 0;

  phy3250_sspconfig();

  if (sspid != 0)
  {
    /* Read byte */
    datao [0] = SEEPROM_READ;
    datao [1] = (UNS_8)((index >> 8) & 0xFF);
    datao [2] = (UNS_8)((index >> 0) & 0xFF);
    datao [3] = 0xFF;
    phy3250_sspxfer(datao, datai, 4);
    byte = datai [3];
  }

  return byte;
}

/***********************************************************************
 *
 * Function: phy3250_button_state
 *
 * Purpose: Read a button state
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     button : PHY3250_BTN1 or PHY3250_BTN2
 *
 * Outputs: None
 *
 * Returns: 1 if the button is pressed, or 0 if depressed
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 phy3250_button_state(PHY_BUTTON_T button)
{
  UNS_32 mask = 0;
  INT_32 pressed = 0;

  if (button == PHY3250_BTN1)
  {
    mask = INP_STATE_GPI_03;
  }
  else if (button == PHY3250_BTN2)
  {
    mask = INP_STATE_GPI_02;
  }

  if ((gpio_get_inppin_states() & mask) != 0)
  {
    pressed = 1;
  }

  return pressed;
}

/***********************************************************************
 *
 * Function: phy3250_sdpower_enable
 *
 * Purpose: Enable or disable SDMMC power
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
void phy3250_sdpower_enable(BOOL_32 enable)
{
  UNS_32 set, clr;

  if (enable == FALSE)
  {
    set = 0;
    clr = OUTP_STATE_GPO(11);
  }
  else
  {
    set = OUTP_STATE_GPO(11);
    clr = 0;
  }

  gpio_set_gpo_state(set, clr);
}

/***********************************************************************
 *
 * Function: phy3250_sdmmc_card_inserted
 *
 * Purpose: Returns card inserted status
 *
 * Processing:
 *     See function.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: Returns TRUE if a card is inserted, otherwise FALSE.
 *
 * Notes: None
 *
 **********************************************************************/
BOOL_32 phy3250_sdmmc_card_inserted(void)
{
  BOOL_32 ins = FALSE;

  if ((gpio_get_inppin_states() & INP_STATE_GPIO_01) == 0)
  {
    ins = TRUE;
  }

  return ins;
}
