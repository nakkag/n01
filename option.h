/*
 * n01
 *
 * option.h
 *
 * Copyright (C) 1996-2005 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

#ifndef _INC_OPTION_H
#define _INC_OPTION_H

/* Include Files */
#include "general.h"

/* Define */
#define WM_LV_EVENT			(WM_APP + 1)

/* Struct */

/* Function Prototypes */
void draw_scroll_control(LPDRAWITEMSTRUCT lpDrawItem, UINT i);
LRESULT listview_notify_proc(const HWND hWnd, const LPARAM lParam, const HWND hListView);
BOOL listview_set_lparam(const HWND hListView, const int i, const LPARAM lParam);
LPARAM listview_get_lparam(const HWND hListView, const int i);
void listview_move_item(const HWND hListView, int index, const int Move);
LRESULT option_notify_proc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
int show_option(const HWND hWnd);

#endif
/* End of source */
