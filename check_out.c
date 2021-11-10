/*
 * n01
 *
 * check_out.c
 *
 * Copyright (C) 1996-2008 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

/* Include Files */
#define _INC_OLE
#include <windows.h>
#undef  _INC_OLE
#include <commctrl.h>
#include <imm.h>

#include "General.h"
#include "font.h"

#include "resource.h"

/* Define */
#define ABS(n)					((n < 0) ? (n * -1) : n)

/* Global Variables */
extern OPTION_INFO op;

/* Local Function Prototypes */
static BOOL CALLBACK check_out_proc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

/*
 * check_out_proc - ウィンドウプロシージャ
 */
static BOOL CALLBACK check_out_proc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HFONT hfont;
	HDC hdc;
	HFONT ret_font;
	TEXTMETRIC tm;
	RECT parent_rect, rect;
	int left, top;
	int i;
	TYPE_CHECK_OUT *cnt;

	switch (msg) {
	case WM_INITDIALOG:
		if ((TYPE_CHECK_OUT *)lParam == NULL) {
			EndDialog(hDlg, FALSE);
			break;
		}
		SetWindowLong(hDlg, GWL_USERDATA, lParam);
		i = *((TYPE_CHECK_OUT *)lParam);

		if (op.check_out_font_size == 0) {
			// フォントサイズ取得
			hdc = GetDC(NULL);
			GetClientRect(GetDlgItem(hDlg, IDC_EDIT_COUNT), &rect);
			op.check_out_font_size = rect.bottom;
			do {
				hfont = font_create(op.font_name, op.check_out_font_size, 0, FALSE, FALSE);
				ret_font = SelectObject(hdc, hfont);
				GetTextMetrics(hdc, &tm);
				SelectObject(hdc, ret_font);
				if (tm.tmHeight < rect.bottom) {
					break;
				}
				DeleteObject(hfont);
				op.check_out_font_size--;
			} while (1);
			ReleaseDC(NULL, hdc);
		} else {
			hfont = font_create(op.font_name, op.check_out_font_size, 0, FALSE, FALSE);
		}
		SendDlgItemMessage(hDlg, IDC_EDIT_COUNT, WM_SETFONT, (WPARAM)hfont, 0);

		SendDlgItemMessage(hDlg, IDC_SPIN_COUNT, UDM_SETRANGE, 0, (LPARAM)MAKELONG(3, (i < 0) ? 1 : 0));
		ImmAssociateContext(GetDlgItem(hDlg, IDC_EDIT_COUNT), (HIMC)NULL);
		SendDlgItemMessage(hDlg, IDC_EDIT_COUNT, EM_LIMITTEXT, 1, 0);

		SetDlgItemInt(hDlg, IDC_EDIT_COUNT, ABS(i), FALSE);

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
		DeleteObject(hfont);
		EndDialog(hDlg, FALSE);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			cnt = (TYPE_CHECK_OUT *)GetWindowLong(hDlg, GWL_USERDATA);
			if (cnt == NULL) {
				SendMessage(hDlg, WM_CLOSE, 0, 0);
				break;
			}
			*cnt = (TYPE_CHECK_OUT)GetDlgItemInt(hDlg, IDC_EDIT_COUNT, NULL, FALSE);

			i = SendDlgItemMessage(hDlg, IDC_SPIN_COUNT, UDM_GETRANGE, 0, 0);
			if (*cnt > LOWORD(i) || *cnt < HIWORD(i)) {
				break;
			}
			DeleteObject(hfont);
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
 * show_check_out - ダブルを狙った本数入力
 */
BOOL show_check_out(const HINSTANCE hInst, const HWND hWnd, TYPE_CHECK_OUT *cnt)
{
	return DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_CHECK_OUT), hWnd, check_out_proc, (LPARAM)cnt);
}
/* End of source */
