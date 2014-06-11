/***********************************************************************
 * $Id:: phy3250_kickstrt.c 1015 2008-08-06 18:23:17Z wellsk           $
 *
 * Project: Phytec LPC3250 kickstart code
 *
 * Description:
 *     This file contains kickstart code used with the Phytec LPC3250
 *     board.
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
#include "phy3250_nand.h"

/***********************************************************************
 * Startup code private data
 **********************************************************************/

/***********************************************************************
 * Private functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: phy3250_gpio_setup
 *
 * Purpose: Setup GPIO and MUX states
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
static void phy3250_gpio_setup(void)
{
  /* Serial EEPROM SSEL signal - a GPIO is used instead of the SSP
     controlled SSEL signal */
  GPIO->p2_dir_set = OUTP_STATE_GPIO(5);
  GPIO->p3_outp_set = OUTP_STATE_GPIO(5);

  /* Sets the following signals muxes :
     GPIO_02 / KEY_ROW6 | (ENET_MDC)      ->KEY_ROW6 | (ENET_MDC)
     GPIO_03 / KEY_ROW7 | (ENET_MDIO)     ->KEY_ROW7 | (ENET_MDIO)
     GPO_21 / U4_TX | (LCDVD[3])          ->U4_TX | (LCDVD[3])
     EMC_D_SEL                            ->(D16 ..D31 used)
     GPIO_04 / SSEL1 | (LCDVD[22])        ->SSEL1 | (LCDVD[22])
     GPIO_05 / SSEL0                      ->SSEL0
    */
  GPIO->p2_mux_clr = (P2_SDRAMD19D31_GPIO | P2_GPIO05_SSEL0);
  GPIO->p2_mux_set = (P2_GPIO03_KEYROW7 | P2_GPIO02_KEYROW6 |
                       P2_GPO21_U4TX | P2_GPIO04_SSEL1);

  /* Sets the following signal muxes:
     I2S1TX_SDA / MAT3.1                  ->I2S1TX_SDA
     I2S1TX_CLK / MAT3.0                  ->I2S1TX_CLK
     I2S1TX_WS / CAP3.0                   ->I2S1TX_WS
     SPI2_DATIO / MOSI1 | (LCDVD[20])     ->MOSI1 | (LCDVD[20])
     SPI2_DATIN / MISO1 | (LCDVD[21])     ->MISO1 | (LCDVD[21])
     SPI2_CLK / SCK1 | (LCDVD[23])        ->SCK1 | (LCDVD[23])
     SPI1_DATIO / MOSI0                   ->MOSI0
     SPI1_DATIN / MISO0                   ->MISO0
     SPI1_CLK / SCK0                      ->SCK0
     (MS_BS) | MAT2.1 / PWM3.6            ->(MS_BS)
     (MS_SCLK) | MAT2.0 / PWM3.5          ->(MS_SCLK)
     U7_TX / MAT1.1 | (LCDVD[11])         ->MAT1.1 | (LCDVD[11])
     (MS_DIO3) | MAT0.3 / PWM3.4          ->(MS_DIO3)
     (MS_DIO2) | MAT0.2 / PWM3.3          ->(MS_DIO2)
     (MS_DIO1) | MAT0.1 / PWM3.2          ->(MS_DIO1)
     (MS_DIO0) | MAT0.0 / PWM3.1          ->(MS_DIO0)
  */
  GPIO->p_mux_set = (P_SPI2DATAIO_MOSI1 |
    P_SPI2DATAIN_MISO1 | P_SPI2CLK_SCK1 |
    P_SPI1DATAIO_SSP0_MOSI | P_SPI1DATAIN_SSP0_MISO |
    P_SPI1CLK_SCK0 | P_U7TX_MAT11);
  GPIO->p_mux_clr = (P_I2STXSDA1_MAT31 | P_I2STXCLK1_MAT30 |
    P_I2STXWS1_CAP30 | P_MAT21_PWM36 |
    P_MAT20_PWM35 | P_MAT03_PWM34 | P_MAT02_PWM33 |
    P_MAT01_PWM32 | P_MAT00_PWM31);

  /* Sets the following signal muxes:
     GPO_02 / MAT1.0 | (LCDVD[0])         ->GPO_02
     GPO_06 / PWM4.3 | (LCDVD[18])        ->PWM4.3 | (LCDVD[18])
     GPO_08 / PWM4.2 | (LCDVD[8])         ->PWM4.2 | (LCDVD[8])
     GPO_09 / PWM4.1 | (LCDVD[9])         ->PWM4.1 | (LCDVD[9])
     GPO_10 / PWM3.6 | (LCDPWR)           ->PWM3.6 | (LCDPWR)
     GPO_12 / PWM3.5 | (LCDLE)            ->PWM3.5 | (LCDLE)
     GPO_13 / PWM3.4 | (LCDDCLK)          ->PWM3.4 | (LCDDCLK)
     GPO_15 / PWM3.3 | (LCDFP)            ->PWM3.3 | (LCDFP)
     GPO_16 / PWM3.2 | (LCDENAB/LCDM)     ->PWM3.2 | (LCDENAB/LCDM)
     GPO_18 / PWM3.1 | (LCDLP)            ->PWM3.1 | (LCDLP)
  */
  GPIO->p3_mux_set = (P3_GPO6_PWM43 | P3_GPO8_PWM42 |
    P3_GPO9_PWM41 | P3_GPO10_PWM36 | P3_GPO12_PWM35 |
    P3_GPO13_PWM34 | P3_GPO15_PWM33 | P3_GPO16_PWM32 |
    P3_GPO18_PWM31);
  GPIO->p3_mux_clr = P3_GPO2_MAT10;

  /* Sets the following signal muxes:
     P0.0 / I2S1RX_CLK                    ->I2S1RX_CLK
      P0.1 / I2S1RX_WS                     ->I2S1RX_WS
     P0.2 / I2S0RX_SDA | (LCDVD[4])       ->I2S0RX_SDA | (LCDVD[4])
     P0.3 / I2S0RX_CLK | (LCDVD[5])       ->I2S0RX_CLK | (LCDVD[5])
     P0.4 / I2S0RX_WS | (LCDVD[6])        ->I2S0RX_WS | (LCDVD[6])
     P0.5 / I2S0TX_SDA | (LCDVD[7])       ->I2S0TX_SDA | (LCDVD[7])
     P0.6 / I2S0TX_CLK | (LCDVD[12])      ->I2S0TX_CLK | (LCDVD[12])
     P0.7 / I2S0TX_WS | (LCDVD[13])       ->I2S0TX_WS | (LCDVD[13])
  */
  GPIO->p0_mux_set = P0_ALL;

  /* Default mux configuation for P1 as follows:
     All signals  -> mapped to address lines (Clear) */
  GPIO->p1_mux_clr = P1_ALL;

  /* Some GPO and GPIO states and directions needs to be setup here:
     GPO_20                      -> Output (watchdog enable) low
     GPO_19                      -> Output (NAND write protect) high
     GPO_17                      -> Output (deep sleep set) low
     GPO_11                      -> Output (deep sleep exit) low
     GPO_05                      -> Output (SDMMC power control) low
     GPO_04                      -> Output (unused) low
     GPO_02                      -> Output (audio reset) low
     GPO_01                      -> Output (LED1) low
     GPIO_1                      -> Input (MMC write protect)
     GPIO_0                      -> Input (MMC detect)
  */
  gpio_set_gpo_state(OUTP_STATE_GPO(19),
    (OUTP_STATE_GPO(20) | OUTP_STATE_GPO(17) | OUTP_STATE_GPO(11) |
    OUTP_STATE_GPO(5) | OUTP_STATE_GPO(4) | OUTP_STATE_GPO(2) |
    OUTP_STATE_GPO(1)));
}

/***********************************************************************
 * Public functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: c_entry
 *
 * Purpose: Application entry point from the startup code
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
void c_entry(void)
{
  NAND_GEOM_T nanddat;
  UNS_8 *p8, tmpbuff [2048+64];
  BOOL_32 blockgood;
  INT_32 toread, idx, blk, page, sector;
  UNS_32 sizes[2];
  PFV execa = (PFV) 0x00000000;

  /* Setup GPIO and MUX states */
  phy3250_gpio_setup();

  /* Initialize NAND FLASH */
  if (nand_init(&nanddat) != 1)
  {
    while (1);
  }

  /* Read the first page in the first block to get the size
     of the image to load */
  blk = 1;
  page = 0;
  p8 = (UNS_8 *) 0x0;
  sector = nand_to_sector(blk, page);
  nand_read_sector(sector, tmpbuff, &tmpbuff [nanddat.bytes_per_page]);
  memcpy(sizes, tmpbuff, sizeof(sizes));

  /* Use 128K if the sizes don't match */
  if (sizes[0] == (0xFFFFFFFF - sizes[1]))
  {
    toread = sizes[0];
    page++;
  }
  else
  {
    toread = 128 * 1024;
  }

  /* Read into memory at address 0x0 */
  while (toread > 0)
  {
    sector = nand_to_sector(blk, page);
    nand_read_sector(sector, tmpbuff, &tmpbuff [nanddat.bytes_per_page]);
    blockgood = TRUE;
    if (page == 0)
    {
      if (tmpbuff [nanddat.bytes_per_page + NAND_LPCBADBLOCK_SPAREOFFS] !=
          NAND_GOOD_BLOCK_MARKER)
      {
        blk++;
        blockgood = FALSE;
      }
    }

    if (blockgood == TRUE)
    {
      for (idx = 0; idx < nanddat.bytes_per_page; idx++)
      {
        *p8 = tmpbuff [idx];
        p8++;
      }

      page++;
      if (page >= nanddat.pages_per_block)
      {
        blk++;
        page = 0;
      }

      toread = toread - nanddat.bytes_per_page;
    }
  }

  execa();
}
