/*
 * n01
 *
 * middle.c
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
#include "Message.h"

#include "resource.h"

/* Define */

/* Global Variables */

/* Local Function Prototypes */
static BOOL CALLBACK middle_proc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

/*
 * middle_proc - ウィンドウプロシージャ
 */
static BOOL CALLBACK middle_proc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	SCORE_INFO *si;
	RECT parent_rect, rect;
	TCHAR buf[BUF_SIZE];
	int left, top;

	switch (msg) {
	case WM_INITDIALOG:
		if ((si = (SCORE_INFO *)lParam) == NULL) {
			EndDialog(hDlg, FALSE);
			break;
		}
		SetWindowLong(hDlg, GWL_USERDATA, lParam);

		if (si->player[0].com == TRUE) {
			wsprintf(buf, TEXT("%s (&1)"), message_get_res(IDS_STRING_COM_NAME));
		} else {
			wsprintf(buf, TEXT("%s (&1)"), si->player[0].name);
		}
		SetWindowText(GetDlgItem(hDlg, IDC_BUTTON_P1), buf);
		if (si->player[1].com == TRUE) {
			wsprintf(buf, TEXT("%s (&2)"), message_get_res(IDS_STRING_COM_NAME));
		} else {
			wsprintf(buf, TEXT("%s (&2)"), si->player[1].name);
		}
		SetWindowText(GetDlgItem(hDlg, IDC_BUTTON_P2), buf);

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
		case IDC_BUTTON_P1:
		case IDC_BUTTON_P2:
			si = (SCORE_INFO *)GetWindowLong(hDlg, GWL_USERDATA);
			if (si == NULL) {
				EndDialog(hDlg, FALSE);
				break;
			}
			// 勝者設定
			si->leg[si->current_leg].winner = LOWORD(wParam) - IDC_BUTTON_P1;
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
 * show_middle - ミドルでの勝者選択
 */
BOOL show_middle(const HINSTANCE hInst, const HWND hWnd, SCORE_INFO *si)
{
	return DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_MIDDLE), hWnd, middle_proc, (LPARAM)si);
}
/* End of source */
