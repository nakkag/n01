/*
 * n01
 *
 * recovery.h
 *
 * Copyright (C) 1996-2004 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

#ifndef _INC_RECOVERY_H
#define _INC_RECOVERY_H

/* Include Files */
#include "general.h"

/* Define */

/* Struct */

/* Function Prototypes */
BOOL recovery_save(const SCORE_INFO *si, const int add_mode, const int player, const int round);
SCORE_INFO *recovery_load(const HWND hWnd);
void recovery_delete(void);

#endif
/* End of source */
