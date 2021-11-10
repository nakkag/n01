/*
 * n01
 *
 * score_guide.h
 *
 * Copyright (C) 1996-2004 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

#ifndef _INC_SCORE_GUIDE_H
#define _INC_SCORE_GUIDE_H

/* Include Files */
#include "general.h"

/* Define */
#define WM_GUIDE_REDRAW				(WM_APP + 400)
#define WM_GUIDE_GET_HEIGHT			(WM_APP + 401)
#define WM_GUIDE_DRAW_INIT			(WM_APP + 402)

/* Struct */

/* Function Prototypes */
BOOL score_guide_regist(const HINSTANCE hInstance);
HWND score_guide_create(const HINSTANCE hInstance, const HWND pWnd, int id);

#endif
/* End of source */
