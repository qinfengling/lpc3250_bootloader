/***********************************************************************
* $Id:: lpc32xx_gpio.h 1019 2008-08-06 18:36:40Z wellsk               $
*
* Project: LPC32XX chip family GPIO and MUX definitions
*
* Description:
*     This file contains the structure definitions and manifest
*     constants for the LPC32XX chip family components:
*         General Purpose Input/Output controller
*         Signal muxing
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

#ifndef LPC32XX_GPIO_H
#define LPC32XX_GPIO_H

#include "lpc_types.h"
#include "lpc32xx_chip.h"

/***********************************************************************
* GPIO Module Register Structure
**********************************************************************/

/* GPIO Module Register Structure */
typedef struct
{
  volatile UNS_32 p3_inp_state;   /* Input pin state register */
  volatile UNS_32 p3_outp_set;    /* Output pin set register */
  volatile UNS_32 p3_outp_clr;    /* Output pin clear register */
  volatile UNS_32 p3_outp_state;  /* Output pin state register */
  volatile UNS_32 p2_dir_set;     /* GPIO direction set register */
  volatile UNS_32 p2_dir_clr;     /* GPIO direction clear register */
  volatile UNS_32 p2_dir_state;   /* GPIO direction state register */
  volatile UNS_32 p2_inp_state; /* SDRAM-Input pin state register*/
  volatile UNS_32 p2_outp_set;  /* SDRAM-Output pin set register */
  volatile UNS_32 p2_outp_clr;  /* SDRAM-Output pin clear register*/
  volatile UNS_32 p2_mux_set;     /* PIO mux control set register*/
  volatile UNS_32 p2_mux_clr;     /* PIO mux control clear register*/
  volatile UNS_32 p2_mux_state;   /* PIO mux state register */
  volatile UNS_32 reserved1 [3];
  volatile UNS_32 p0_inp_state;    /* P0 GPIOs pin read register */
  volatile UNS_32 p0_outp_set;    /* P0 GPIOs output set register */
  volatile UNS_32 p0_outp_clr;    /* P0 GPIOs output clear register */
  volatile UNS_32 p0_outp_state;  /* P0 GPIOs output state register */
  volatile UNS_32 p0_dir_set;     /* P0 GPIOs direction set reg */
  volatile UNS_32 p0_dir_clr;     /* P0 GPIOs direction clear reg */
  volatile UNS_32 p0_dir_state;   /* P0 GPIOs direction state reg */
  volatile UNS_32 reserved2;
  volatile UNS_32 p1_inp_state;    /* P1 GPIOs pin read register */
  volatile UNS_32 p1_outp_set;    /* P1 GPIOs output set register */
  volatile UNS_32 p1_outp_clr;    /* P1 GPIOs output clear register */
  volatile UNS_32 p1_outp_state;  /* P1 GPIOs output state register */
  volatile UNS_32 p1_dir_set;     /* P1 GPIOs direction set reg */
  volatile UNS_32 p1_dir_clr;     /* P1 GPIOs direction clear reg */
  volatile UNS_32 p1_dir_state;   /* P1 GPIOs direction state reg */
  volatile UNS_32 reserved3;
  volatile UNS_32 reserved4 [32];
  volatile UNS_32 p_mux_set;     /* PIO mux2 control set register*/
  volatile UNS_32 p_mux_clr;     /* PIO mux2 control clear register*/
  volatile UNS_32 p_mux_state;   /* PIO mux2 state register */
  volatile UNS_32 reserved5;
  volatile UNS_32 p3_mux_set;     /* PIO mux3 control set register*/
  volatile UNS_32 p3_mux_clr;     /* PIO mux3 control clear register*/
  volatile UNS_32 p3_mux_state;   /* PIO mux3 state register */
  volatile UNS_32 reserved6;
  volatile UNS_32 p0_mux_set;       /* P0 mux control set register*/
  volatile UNS_32 p0_mux_clr;       /* P0 mux control clear register*/
  volatile UNS_32 p0_mux_state;     /* P0 mux state register */
  volatile UNS_32 reserved7;
  volatile UNS_32 p1_mux_set;       /* P1 mux control set register*/
  volatile UNS_32 p1_mux_clr;       /* P1 mux control clear register*/
  volatile UNS_32 p1_mux_state;     /* P1 mux state register */
} GPIO_REGS_T;

/* For direction registers, a '1' is an output */
#define GPIO_DIR_OUT          0x1

/***********************************************************************
* Input Pin State Register defines
**********************************************************************/
/* Input state of GPI_pin. Where pin = 0-9 */
#define INP_STATE_GPI_00	  _BIT(0)
#define INP_STATE_GPI_01	  _BIT(1)
#define INP_STATE_GPI_02	  _BIT(2)
#define INP_STATE_GPI_03	  _BIT(3)
#define INP_STATE_GPI_04	  _BIT(4)
#define INP_STATE_GPI_05	  _BIT(5)
#define INP_STATE_GPI_06	  _BIT(6)
#define INP_STATE_GPI_07	  _BIT(7)
#define INP_STATE_GPI_08	  _BIT(8)
#define INP_STATE_GPI_09	  _BIT(9)
#define INP_STATE_GPIO_00	  _BIT(10)
#define INP_STATE_GPIO_01	  _BIT(11)
#define INP_STATE_GPIO_02	  _BIT(12)
#define INP_STATE_GPIO_03	  _BIT(13)
#define INP_STATE_GPIO_04	  _BIT(14)
#define INP_STATE_U1_RX		  _BIT(15)
#define INP_STATE_U2_HCTS	  _BIT(16)
#define INP_STATE_U2_RX		  _BIT(17)
#define INP_STATE_U3_RX		  _BIT(18)
#define INP_STATE_GPI_19_U4RX _BIT(19)
#define INP_STATE_U5_RX		  _BIT(20)
#define INP_STATE_U6_IRRX	  _BIT(21)
#define INP_STATE_U7_HCTS	  _BIT(22)
#define INP_STATE_U7_RX		  _BIT(23)
#define INP_STATE_GPIO_05	  _BIT(24)
#define INP_STATE_SPI1_DATIN  _BIT(25)
#define INP_STATE_SPI2_DATIN  _BIT(27)
#define INP_STATE_GPI_28_U3RI _BIT(28)

/***********************************************************************
* p3_outp_set, p3_outp_clr, and p3_outp_state register defines
**********************************************************************/
/* Following macro is used to determine bit position for GPO pin in
*  P3_OUTP_SET, P3_OUTP_CLR & P3_OUTP_STATE registers.
*  Where pin = {0-23}
*/
#define OUTP_STATE_GPO(pin)	  _BIT((pin))

/* Following macro is used to determine bit position for GPIO pin in
*  PIO_OUTP_SET, P3_OUTP_CLR & P3_OUTP_STATE registers.
*  Where pin = {0-5}
*/
#define OUTP_STATE_GPIO(pin)  _BIT(((pin) + 25))

/***********************************************************************
* GPIO Direction Register defines
**********************************************************************/
/* Following macro is used to determine bit position for GPIO pin in
*  P2_DIR_SET, P2_DIR_STATE & P2_DIR_CLR registers.
*  Where pin = {0-5}
*/
#define PIO_DIR_GPIO(pin)	 _BIT(((pin) + 25))

/* Following macro is used to determine bit position for RAM_D pin in
*  P2_DIR_SET, P2_DIR_CLR, P2_DIR_STATE, P2_INP_STATE,
*  P2_OUTP_SET, & P2_OUTP_CLR.
*  Where pin = {19-31}
*/
#define PIO_SDRAM_DIR_PIN(pin) _BIT(((pin) - 19))

/* Macro for GPIO direction muxed with the high 16 bits of the SDRAM
   data related bit locations (when configured as a GPIO) */
#define PIO_SDRAM_PIN_ALL    0x00001FFF

/***********************************************************************
* p_mux_set, p_mux_clr, p_mux_state register defines
**********************************************************************/
/* Muxed PIO#0 pin state defines */
#define P_I2STXSDA1_MAT31     _BIT(2)
#define P_I2STXCLK1_MAT30     _BIT(3)
#define P_I2STXWS1_CAP30      _BIT(4)
#define P_SPI2DATAIO_MOSI1    _BIT(5)
#define P_SPI2DATAIN_MISO1    _BIT(6)
#define P_SPI2CLK_SCK1        _BIT(8)
#define P_SPI1DATAIO_SSP0_MOSI _BIT(9)
#define P_SPI1DATAIN_SSP0_MISO _BIT(10)
#define P_SPI1CLK_SCK0        _BIT(12)
#define P_MAT21_PWM36         _BIT(13)
#define P_MAT20_PWM35         _BIT(14)
#define P_U7TX_MAT11          _BIT(15)
#define P_MAT03_PWM34         _BIT(17)
#define P_MAT02_PWM33         _BIT(18)
#define P_MAT01_PWM32         _BIT(19)
#define P_MAT00_PWM31         _BIT(20)

/***********************************************************************
* p0_mux_set, p0_mux_clr, p0_mux_state register defines
**********************************************************************/

/* Following macro is used to determine bit position for a P0 GPIO pin
   used with the p0_xxx registers for pins P0_0 to P0_7*/
#define OUTP_STATE_GPIO_P0(pin)	  _BIT((pin))

/* P0 pin mux defines (0 = GPIO, 1 = alternate function) */
#define P0_GPOP0_I2SRXCLK1		  _BIT(0)
#define P0_GPOP1_I2SRXWS1	      _BIT(1)
#define P0_GPOP2_I2SRXSDA0	      _BIT(2)
#define P0_GPOP3_I2SRXCLK0	      _BIT(3)
#define P0_GPOP4_I2SRXWS0	      _BIT(4)
#define P0_GPOP5_I2STXSDA0	      _BIT(5)
#define P0_GPOP6_I2STXCLK0	      _BIT(6)
#define P0_GPOP7_I2STXWS0	      _BIT(7)
#define P0_ALL						0xFF

/***********************************************************************
* p1_mux_set, p1_mux_clr, p1_mux_state register defines
**********************************************************************/

/* Following macro is used to determine bit position for a P1 GPIO pin
   used with the p1_xxx registers for pins P1_0 to P1_23*/
#define OUTP_STATE_GPIO_P1(pin)	  _BIT((pin))

/* Mask for all GPIO P1 bits */
#define P1_ALL                    0x00FFFFFF

/* Macro pointing to GPIO registers */
#define GPIO  ((GPIO_REGS_T *)(GPIO_BASE))

/***********************************************************************
* p2_mux_set, p2_mux_clr, p2_mux_state register defines
**********************************************************************/
/* Muxed PIO#2 pin state defines */
#define P2_GPIO05_SSEL0			_BIT(5)
#define P2_GPIO04_SSEL1			_BIT(4)
#define P2_SDRAMD19D31_GPIO		_BIT(3)
#define P2_GPO21_U4TX	        _BIT(2)
#define P2_GPIO03_KEYROW7		_BIT(1)
#define P2_GPIO02_KEYROW6		_BIT(0)

/***********************************************************************
* p3_mux_set, p3_mux_clr, p3_mux_state register defines
**********************************************************************/
/* Muxed PIO#3 pin states, first column is '0' state, second is '1' */
#define P3_GPO2_MAT10          _BIT(2)
#define P3_GPO6_PWM43          _BIT(6)
#define P3_GPO8_PWM42          _BIT(8)
#define P3_GPO9_PWM41          _BIT(9)
#define P3_GPO10_PWM36         _BIT(10)
#define P3_GPO12_PWM35         _BIT(12)
#define P3_GPO13_PWM34         _BIT(13)
#define P3_GPO15_PWM33         _BIT(15)
#define P3_GPO16_PWM32         _BIT(16)
#define P3_GPO18_PWM31         _BIT(18)

#endif /* LPC32XX_GPIO_H */
