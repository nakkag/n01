/*
 * n01
 *
 * score_list.h
 *
 * Copyright (C) 1996-2005 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

#ifndef _INC_SCORE_LIST_H
#define _INC_SCORE_LIST_H

/* Include Files */

/* Define */
#define WM_SCORE_INIT_LEG			(WM_APP + 100)
#define WM_SCORE_NEXT_LEG			(WM_APP + 101)
#define WM_SCORE_PREV_LEG			(WM_APP + 102)
#define WM_SCORE_SHOW_LEG			(WM_APP + 103)
#define WM_SCORE_RESTART			(WM_APP + 104)
#define WM_SCORE_REDRAW				(WM_APP + 105)
#define WM_SCORE_REFRESH			(WM_APP + 106)
#define WM_SCORE_INPUT				(WM_APP + 107)
#define WM_SCORE_MOVE_LEFT			(WM_APP + 108)
#define WM_SCORE_MOVE_RIGHT			(WM_APP + 109)
#define WM_SCORE_SET_HALF_HEIGHT	(WM_APP + 110)
#define WM_SCORE_DRAW_INIT			(WM_APP + 111)
#define WM_SCORE_SET_LOCK			(WM_APP + 112)
#define WM_SCORE_SET_INFO			(WM_APP + 113)
#define WM_SCORE_GET_BUFFER			(WM_APP + 114)

/* Struct */

/* Function Prototypes */
BOOL score_list_regist(const HINSTANCE hInstance);
HWND score_list_create(const HINSTANCE hInstance, const HWND pWnd, int id, SCORE_INFO *si);

#endif
/* End of source */
