/*
 * n01
 *
 * score_player.h
 *
 * Copyright (C) 1996-2004 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

#ifndef _INC_SCORE_PLAYER_H
#define _INC_SCORE_PLAYER_H

/* Include Files */
#include "general.h"

/* Define */
#define WM_PLAYER_REDRAW			(WM_APP + 300)
#define WM_PLAYER_DRAW_INIT			(WM_APP + 301)
#define WM_PLAYER_SET_INFO			(WM_APP + 302)
#define WM_PLAYER_SET_MODE			(WM_APP + 303)
#define WM_PLAYER_SET_LOCK			(WM_APP + 304)
#define WM_PLAYER_MODE_HISTORY		(WM_APP + 305)
#define WM_PLAYER_MODE_OPTION		(WM_APP + 306)

/* Struct */

/* Function Prototypes */
BOOL score_player_regist(const HINSTANCE hInstance);
HWND score_player_create(const HINSTANCE hInstance, const HWND pWnd, int id, PLAYER_INFO *pi);

#endif
/* End of source */
