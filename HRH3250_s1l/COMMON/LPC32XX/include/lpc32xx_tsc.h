/***********************************************************************
* $Id:: lpc32xx_tsc.h 699 2008-05-07 20:00:53Z kendwyer               $
*
* Project: LPC32xx touchscreen controller definitions
*
* Description:
*     This file contains the structure definitions and manifest
*     constants for the LPC32XX chip family component:
*         Touchscreen controller
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

#ifndef LPC32xx_TSC_H
#define LPC32xx_TSC_H

#include "lpc_types.h"
#include "lpc32xx_chip.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**********************************************************************
* Touchscreen controller register structures
**********************************************************************/

/* Touchscreen controller module register structures */
typedef struct
{
  volatile UNS_32 tsc_stat;      /* Touchscreen controller status */
  volatile UNS_32 adc_sel;       /* ADC control state register */
  volatile UNS_32 adc_con;       /* Touchscreen control register */
  volatile UNS_32 tsc_fifo;      /* Same FIFO */
  volatile UNS_32 tsc_dtr;       /* Delay time register */
  volatile UNS_32 tsc_rtr;       /* Rise time register */
  volatile UNS_32 tsc_utr;       /* Update time register */
  volatile UNS_32 tsc_ttr;       /* Touch time register */
  volatile UNS_32 tsc_dxp;       /* Drain X plate time register */
  volatile UNS_32 tsc_min_x;     /* Minimum X value */
  volatile UNS_32 tsc_max_x;     /* Maximum X value */
  volatile UNS_32 tsc_min_y;     /* Minimum Y value */
  volatile UNS_32 tsc_max_y;     /* Maximum Y value */
  volatile UNS_32 tsc_aux_utr;   /* Aux update time register */
  volatile UNS_32 tsc_aux_min;   /* Aux minimum value register */
  volatile UNS_32 tsc_aux_max;   /* Aux maximum value register */
  volatile UNS_32 reserved;
  volatile UNS_32 tsc_aux_value; /* Aux value register */
  volatile UNS_32 adc_dat;       /* ADC data register */
} LPC3250_ADCTSC_REGS_T;

/**********************************************************************
* tsc_stat register definitions
**********************************************************************/
#define TSC_STAT_FIFO_OVRRN     _BIT(8) /* FIFO overrun status */
#define TSC_STAT_FIFO_EMPTY     _BIT(7) /* FIFO empty status */
#define TSC_STAT_ADC_STAT_MASK  0x00000070 /* FIFO ADC status mask */
#define TSC_STAT_TSC_STAT_MASK  0x0000000F /* FIFO TSC status mask */

/* The register masked with TSC_STAT_ADC_STAT_MASK will give these
   status values. These statuses are only valid when AUTO_EN in the
   touch control register is enabled. */
#define TSC_STAT_ADC_SCAN_STOP_STS      0x00000000
#define TSC_STAT_ADC_WAIT_RISE_STS      0x00000010
#define TSC_STAT_ADC_WAIT_SAMPLE_STS    0x00000020
#define TSC_STAT_ADC_WAIT_ACCURACY_STS  0x00000030
#define TSC_STAT_ADC_NOT_READY_STS      0x00000040

/* The register masked with TSC_STAT_TSC_STAT_MASK will give these
   status values. These statuses are only valid when AUTO_EN in the
   touch control register is not enabled. */
#define TSC_STAT_ADC_TSC_INACTIVE_STS      0x00000000
#define TSC_STAT_ADC_TSC_AUTO_STS          0x00000001
#define TSC_STAT_ADC_TSC_AUX_DETECT_STS    0x00000002
#define TSC_STAT_ADC_TSC_WAIT_DELAY_STS    0x00000003
#define TSC_STAT_ADC_TSC_MEASURE_X_STS     0x00000004
#define TSC_STAT_ADC_TSC_MEASURE_Y_STS     0x00000005
#define TSC_STAT_ADC_TSC_MEASURE_AUX_STS   0x00000006
#define TSC_STAT_ADC_TSC_DRAIN_X_STS       0x00000007
#define TSC_STAT_ADC_TSC_TOUCH_DETECT_STS  0x00000008
#define TSC_STAT_ADC_TSC_CHK_FIFO_MT_STS   0x00000009
#define TSC_STAT_ADC_TSC_WAIT_UPD_TMR_STS  0x0000000A
#define TSC_STAT_ADC_TSC_UPD_TIMER_AUC_STS 0x0000000C

/**********************************************************************
* adc_sel register definitions
**********************************************************************/
#define TSC_ADCSEL_TSREF_MINUS_TO_XM       _SBF(8, 0)
#define TSC_ADCSEL_TSREF_MINUS_TO_YM       _SBF(8, 1)
#define TSC_ADCSEL_TSREF_MINUS_TO_VSS      _SBF(8, 2)
#define TSC_ADCSEL_TSREF_PLUS_TO_XP        _SBF(6, 0)
#define TSC_ADCSEL_TSREF_PLUS_TO_YP        _SBF(6, 1)
#define TSC_ADCSEL_TSREF_PLUS_TO_VDDTS     _SBF(6, 2)
#define TSC_ADCSEL_TSIN_TO_YM              _SBF(4, 0)
#define TSC_ADCSEL_TSIN_TO_XM              _SBF(4, 1)
#define TSC_ADCSEL_TSIN_TO_AUX             _SBF(4, 2)
#define TSC_ADCSEL_YM_TO_GND               _BIT(3)
#define TSC_ADCSEL_YP_TO_VDDTS             _BIT(2)
#define TSC_ADCSEL_XM_TO_GND               _BIT(1)
#define TSC_ADCSEL_XP_TO_VDDTS             _BIT(0)

#define ADC_SEL_MASK            0x284
#define TSC_SEL_MASK            0x4
#define ADCSEL_CH_0 			0x0
#define ADCSEL_CH_1 			0x10
#define ADCSEL_CH_2 			0x20

/**********************************************************************
* adc_con register definitions
**********************************************************************/
#define TSC_ADCCON_IRQ_TO_FIFO_1           _SBF(11, 0)
#define TSC_ADCCON_IRQ_TO_FIFO_4           _SBF(11, 1)
#define TSC_ADCCON_IRQ_TO_FIFO_8           _SBF(11, 2)
#define TSC_ADCCON_IRQ_TO_FIFO_16          _SBF(11, 3)
#define TSC_ADCCON_TS_AUX_EN               _BIT(10)

#define TSC_FIFO_MSK					   0x1800

/* Allowable sample sizes are 3 to 10 bits */
#define TSC_ADCCON_X_SAMPLE_SIZE(s)        _SBF(7, (10 - s))
#define TSC_ADCCON_Y_SAMPLE_SIZE(s)        _SBF(4, (10 - s))

#define TSC_ADCCON_POS_DET                 _BIT(3)
#define TSC_ADCCON_POWER_UP                _BIT(2)
#define TSC_ADCCON_ADC_STROBE              _BIT(1)
#define TSC_ADCCON_AUTO_EN                 _BIT(0)

/**********************************************************************
* tsc_fifo register definitions
**********************************************************************/
#define TSC_FIFO_TS_P_LEVEL                _BIT(31)
#define TSC_FIFO_TS_FIFO_EMPTY             _BIT(30)
#define TSC_FIFO_TS_FIFO_OVERRUN           _BIT(29)
#define TSC_FIFO_NORMALIZE_X_VAL(x)        ((x & 0x03FF0000) >> 16)
#define TSC_FIFO_NORMALIZE_Y_VAL(y)        (y & 0x000003FF)

/**********************************************************************
* tsc_dtr register definitions
**********************************************************************/
#define TSC_DTR_DELAY_TIME(d)              (d)

/**********************************************************************
* tsc_rtr register definitions
**********************************************************************/
#define TSC_RTR_RISE_TIME(d)               (d)

/**********************************************************************
* tsc_utr register definitions
**********************************************************************/
#define TSC_UTR_UPDATE_TIME(d)             (d)

/**********************************************************************
* tsc_ttr register definitions
**********************************************************************/
#define TSC_TTR_TOUCH_TIME(d)              (d)

/**********************************************************************
* tsc_dxp register definitions
**********************************************************************/
#define TSC_DXP_DRAINX_TIME(d)             (d)

/**********************************************************************
* tsc_min_x, tsc_max_x, tsc_min_y, and tsc_max_y register definitions
**********************************************************************/
#define TSC_DTR_MINX_VALUE(d)              (d)
#define TSC_DTR_MAXX_VALUE(d)              (d)
#define TSC_DTR_MINY_VALUE(d)              (d)
#define TSC_DTR_MAXY_VALUE(d)              (d)

/**********************************************************************
* tsc_aux_utr register definitions
**********************************************************************/
#define TSC_AUX_UTR_UPDATE_TIME(d)         (d)

/**********************************************************************
* tsc_aux_min, tsc_aux_max, and tsc_aux_max register definitions
**********************************************************************/
#define TSC_AUX_MIN_VALUE(d)               (d)
#define TSC_AUX_MAX_VALUE(d)               (d)
#define TSC_AUX_VALUE(d)                   (d)

/**********************************************************************
* adc_dat register definitions
**********************************************************************/
#define TSC_ADCDAT_P_LEVEL                 _BIT(10)
#define TSC_ADCDAT_VALUE_MASK              0x000003FF

/* Macro pointing to touchscreen controller registers */
#define ADC  ((LPC3250_ADCTSC_REGS_T *)(ADC_BASE))
#define TSC  ((LPC3250_ADCTSC_REGS_T *)(ADC_BASE))

#ifdef __cplusplus
}
#endif

#endif /* LPC32xx_TSC_H */
