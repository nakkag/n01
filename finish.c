/*
 * n01
 *
 * finish.c
 *
 * Copyright (C) 1996-2004 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

/* Include Files */
#define _INC_OLE
#include <windows.h>
#undef  _INC_OLE

#include "General.h"

#include "resource.h"

/* Define */

/* Global Variables */
static int ret_value;

/* Local Function Prototypes */
static BOOL CALLBACK finish_proc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

/*
 * finish_proc - ウィンドウプロシージャ
 */
static BOOL CALLBACK finish_proc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	RECT parent_rect, rect;
	int left, top;

	switch (msg) {
	case WM_INITDIALOG:
		if (lParam > 60) {
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_FINISH_1), FALSE);
		}
		if (lParam > 120) {
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_FINISH_2), FALSE);
		}

		// センタリング
		GetWindowRect(GetParent(hDlg), &parent_rect);
		GetWindowRect(hDlg, &rect);
		left = parent_rect.left + ((parent_rect.right - parent_rect.left) - (rect.right - rect.left)) / 2;
		if (left < 0) left = 0;
		top = parent_rect.top + ((parent_rect.bottom - parent_rect.top) - (rect.bottom - rect.top)) / 2;
		if (top < 0) top = 0;
		SetWindowPos(hDlg, 0, left, top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
		break;

	case WM_CLOSE:
		EndDialog(hDlg, FALSE);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_BUTTON_FINISH_1:
			ret_value = 1;
			EndDialog(hDlg, TRUE);
			break;

		case IDC_BUTTON_FINISH_2:
			ret_value = 2;
			EndDialog(hDlg, TRUE);
			break;

		case IDC_BUTTON_FINISH_3:
			ret_value = 3;
			EndDialog(hDlg, TRUE);
			break;

		case IDCANCEL:
			SendMessage(hDlg, WM_CLOSE, 0, 0);
			break;
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

/*
 * show_finish - フィニッシュ本数選択
 */
int show_finish(const HINSTANCE hInst, const HWND hWnd, const int left_score)
{
	ret_value = 0;
	if (DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_FINISH), hWnd, finish_proc, left_score) == TRUE) {
		return ret_value;
	}
	return 0;
}
/* End of source */
