/*
 * n01
 *
 * score_left.h
 *
 * Copyright (C) 1996-2004 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

#ifndef _INC_SCORE_LEFT_H
#define _INC_SCORE_LEFT_H

/* Include Files */
#include "general.h"

/* Define */
#define WM_LEFT_REDRAW				(WM_APP + 200)
#define WM_LEFT_GET_HEIGHT			(WM_APP + 201)
#define WM_LEFT_DRAW_INIT			(WM_APP + 202)
#define WM_LEFT_SET_CURRENT			(WM_APP + 203)

/* Struct */

/* Function Prototypes */
BOOL score_left_regist(const HINSTANCE hInstance);
HWND score_left_create(const HINSTANCE hInstance, const HWND pWnd, int id, PLAYER_INFO *pi);

#endif
/* End of source */
