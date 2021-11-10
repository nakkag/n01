/*
 * n01
 *
 * score_name.h
 *
 * Copyright (C) 1996-2014 by Ohno Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

#ifndef _INC_SCORE_NAME_H
#define _INC_SCORE_NAME_H

/* Include Files */
#include "general.h"

/* Define */
#define WM_NAME_REDRAW				(WM_APP + 350)
#define WM_NAME_DRAW_INIT			(WM_APP + 351)
#define WM_NAME_GET_HEIGHT			(WM_APP + 352)
#define WM_NAME_SET_MODE			(WM_APP + 353)

/* Struct */

/* Function Prototypes */
BOOL score_name_regist(const HINSTANCE hInstance);
HWND score_name_create(const HINSTANCE hInstance, const HWND pWnd, int id, SCORE_INFO *si);

#endif
/* End of source */
