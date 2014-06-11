/***********************************************************************
 * $Id:: s1l_line_input.h 871 2008-07-07 22:36:49Z wellsk              $
 *
 * Project: Line input method and prompt controls
 *
 * Description:
 *     Used to input commands and arguments from an input method
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

#ifndef S1L_LINEINPUT_H
#define S1L_LINEINPUT_H

/* Enables history support */
//#define HISTORY_ENABLE

/* Sets up prompt */
void key_prompt_set(UNS_8 *prompt);

/* Initializes line input (key) methods */
void key_line_init(UNS_8 *prompt);

/* Gets a command from the command prompt */
void key_get_command(UNS_8 *str);

/* Displays a prompt with a yes/no input */
BOOL_32 prompt_yesno(void);

#endif /* S1L_LINEINPUT_H */
