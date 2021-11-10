/*
 * n01
 *
 * score_save.h
 *
 * Copyright (C) 1996-2005 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

#ifndef _INC_SCORE_SAVE_H
#define _INC_SCORE_SAVE_H

/* Include Files */
#include "general.h"

/* Define */

/* Struct */

/* Function Prototypes */
int get_score_string_length(const SCORE_INFO *si);
BOOL get_score_string(const SCORE_INFO *si, TCHAR *ret);
BOOL score_save(const HWND hWnd, const SCORE_INFO *si);
BOOL score_auto_save(const HWND hWnd, const SCORE_INFO *si);

#endif
/* End of source */
