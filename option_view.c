/*
 * n01
 *
 * option_view.c
 *
 * Copyright (C) 1996-2016 by Ohno Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

/* Include Files */
#define _INC_OLE
#include <windows.h>
#undef  _INC_OLE
#include <tchar.h>
#include <commctrl.h>

#include "General.h"
#include "Memory.h"
#include "Message.h"
#include "ini.h"
#include "score_list.h"
#include "score_left.h"
#include "score_player.h"
#include "option.h"

#include "resource.h"

/* Define */
#define WM_SET_COLOR				(WM_APP + 1)

typedef enum _POSINFO {
	POS_BACKGROUND = 0,
	POS_ODD_BACKGROUND,
	POS_SCORED_TEXT,
	POS_TOGO_TEXT,
	POS_HEADER_BACKGROUND,
	POS_HEADER_TEXT,
	POS_LAST3NUMBER_TEXT,
	POS_LINE,
	POS_SEPARATE,
	POS_TON_CIRCLE,

	POS_INPUT_BACKGROUND,
	POS_INPUT_TEXT,
	POS_INPUT_SELECT_BACKGROUND,
	POS_INPUT_SELECT_TEXT,

	POS_LEFT_BACKGROUND,
	POS_LEFT_TEXT,
	POS_LEFT_ACTIVE_BORDER,

	POS_PLAYER_NAME_BACKGROUND,
	POS_PLAYER_NAME_TEXT,
	POS_PLAYER_BACKGROUND,
	POS_PLAYER_TEXT,
	POS_PLAYER_INFO_TITLE,

	POS_GUIDE_BACKGROUND,
	POS_GUIDE_TEXT,
	POS_GUIDE_SELECT_BACKGROUND,
	POS_GUIDE_SELECT_TEXT,
	POS_GUIDE_BOX_BACKGROUND,
	POS_GUIDE_BOX_TEXT,
	POS_GUIDE_BOX_FRAME,
	POS_GUIDE_BOX_DISABLE,

	POS_VIEW_SEPARATE,
} POSINFO;

/* Global Variables */
extern HINSTANCE hInst;
extern OPTION_INFO op;
extern int prop_ret;

HBRUSH op_back_brush;

/* Local Function Prototypes */
static long get_color(const HWND hWnd, long color);

/*
 * get_color - 色の選択
 */
static long get_color(const HWND hWnd, long color)
{
	CHOOSECOLOR ch;
	static DWORD ColorTbl[16];

	ZeroMemory(&ch, sizeof(CHOOSECOLOR));
	ch.lStructSize = sizeof(CHOOSECOLOR);
	ch.hwndOwner = hWnd;
	ch.rgbResult = color;
	ch.lpCustColors = ColorTbl;
	ch.Flags = CC_RGBINIT | CC_FULLOPEN;
	// 色の選択
	if(ChooseColor(&ch) == TRUE){
		return ch.rgbResult;
	}
	return color;
}

/*
 * enum_faces_proc - フォントの列挙
 */
static int CALLBACK enum_faces_proc(LPLOGFONT lplf, LPTEXTMETRIC lptm, DWORD dwType, LPVOID lpData)
{
	int i;

	i = SendMessage((HWND)lpData, CB_ADDSTRING, 0, (LPARAM)lplf->lfFaceName);
	if (lstrcmp(lplf->lfFaceName, op.font_name) == 0) {
		SendMessage((HWND)lpData, CB_SETCURSEL, i, 0);
	}
	return 1;
}

/*
 * option_view_proc - ウィンドウプロシージャ
 */
BOOL CALLBACK option_view_proc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static COLOR_INFO re_ci;
	static TCHAR re_font_name[BUF_SIZE];
	static HWND list_wnd;
	static HWND player_wnd;
	static HWND left_wnd;
	static SCORE_INFO si;
	DRAWITEMSTRUCT *dis;
	HDC hdc;
	HBRUSH hbrush;
	RECT rect;
	HCURSOR old_cursor;
	TCHAR buf[BUF_SIZE];
	long color;
	int i;

	switch (msg) {
	case WM_INITDIALOG:
		// 色情報の初期化
		re_ci = op.ci;
		// フォント情報の初期化
		lstrcpy(re_font_name, op.font_name);

		// スコア情報の初期化
		ZeroMemory(&si, sizeof(SCORE_INFO));
		si.leg = (LEG_INFO *)mem_calloc(sizeof(LEG_INFO));
		si.player[0].left = si.player[0].start_score = 501;
		lstrcpy(si.player[0].name, message_get_res(IDS_STRING_PLAYER1));

		// ウィンドウの作成
		GetClientRect(GetDlgItem(hDlg, IDC_STATIC_FRAME), &rect);
		list_wnd = score_list_create(hInst, GetDlgItem(hDlg, IDC_STATIC_FRAME), 0, &si);
		player_wnd = score_player_create(hInst, GetDlgItem(hDlg, IDC_STATIC_FRAME), 0, &si.player[0]);
		left_wnd = score_left_create(hInst, GetDlgItem(hDlg, IDC_STATIC_FRAME), 0, &si.player[0]);
		ShowWindow(list_wnd, SW_SHOW);
		ShowWindow(player_wnd, SW_SHOW);
		ShowWindow(left_wnd, SW_SHOW);
		SendMessage(list_wnd, WM_SCORE_INPUT, 100, 1);
		SendMessage(list_wnd, WM_SCORE_SET_LOCK, TRUE, 0);
		SendMessage(player_wnd, WM_PLAYER_MODE_OPTION, TRUE, 0);
		SendMessage(player_wnd, WM_PLAYER_SET_LOCK, TRUE, 0);
		SendMessage(left_wnd, WM_LEFT_SET_CURRENT, TRUE, 0);
		// ウィンドウの位置設定
		MoveWindow(list_wnd, rect.right / 3 + 2, 0, rect.right, rect.bottom / 4 * 3, FALSE);
		MoveWindow(player_wnd, 0, 0, rect.right / 3, rect.bottom / 4 * 3, FALSE);
		MoveWindow(left_wnd, 0, rect.bottom / 4 * 3 + 2, rect.right, rect.bottom - rect.bottom / 4 * 3 - 2, FALSE);

		// 指定する部分の追加
#define SetCmbItem(ctl, name)		SendDlgItemMessage(hDlg, ctl, CB_ADDSTRING, 0, (LPARAM)name)
		SetCmbItem(IDC_COMBO_COLOR, message_get_res(IDS_STRING_POS_BACKGROUND));
		SetCmbItem(IDC_COMBO_COLOR, message_get_res(IDS_STRING_POS_ODD_BACKGROUND));
		SetCmbItem(IDC_COMBO_COLOR, message_get_res(IDS_STRING_POS_SCORED_TEXT));
		SetCmbItem(IDC_COMBO_COLOR, message_get_res(IDS_STRING_POS_TOGO_TEXT));
		SetCmbItem(IDC_COMBO_COLOR, message_get_res(IDS_STRING_POS_HEADER_BACKGROUND));
		SetCmbItem(IDC_COMBO_COLOR, message_get_res(IDS_STRING_POS_HEADER_TEXT));
		SetCmbItem(IDC_COMBO_COLOR, message_get_res(IDS_STRING_POS_LAST3NUMBER_TEXT));
		SetCmbItem(IDC_COMBO_COLOR, message_get_res(IDS_STRING_POS_LINE));
		SetCmbItem(IDC_COMBO_COLOR, message_get_res(IDS_STRING_POS_SEPARATE));
		SetCmbItem(IDC_COMBO_COLOR, message_get_res(IDS_STRING_POS_TON_CIRCLE));

		SetCmbItem(IDC_COMBO_COLOR, message_get_res(IDS_STRING_POS_INPUT_BACKGROUND));
		SetCmbItem(IDC_COMBO_COLOR, message_get_res(IDS_STRING_POS_INPUT_TEXT));
		SetCmbItem(IDC_COMBO_COLOR, message_get_res(IDS_STRING_POS_INPUT_SELECT_BACKGROUND));
		SetCmbItem(IDC_COMBO_COLOR, message_get_res(IDS_STRING_POS_INPUT_SELECT_TEXT));

		SetCmbItem(IDC_COMBO_COLOR, message_get_res(IDS_STRING_POS_LEFT_BACKGROUND));
		SetCmbItem(IDC_COMBO_COLOR, message_get_res(IDS_STRING_POS_LEFT_TEXT));
		SetCmbItem(IDC_COMBO_COLOR, message_get_res(IDS_STRING_POS_LEFT_ACTIVE_BORDER));

		SetCmbItem(IDC_COMBO_COLOR, message_get_res(IDS_STRING_POS_PLAYER_NAME_BACKGROUND));
		SetCmbItem(IDC_COMBO_COLOR, message_get_res(IDS_STRING_POS_PLAYER_NAME_TEXT));
		SetCmbItem(IDC_COMBO_COLOR, message_get_res(IDS_STRING_POS_PLAYER_BACKGROUND));
		SetCmbItem(IDC_COMBO_COLOR, message_get_res(IDS_STRING_POS_PLAYER_TEXT));
		SetCmbItem(IDC_COMBO_COLOR, message_get_res(IDS_STRING_POS_PLAYER_INFO_TITLE));

		SetCmbItem(IDC_COMBO_COLOR, message_get_res(IDS_STRING_POS_GUIDE_BACKGROUND));
		SetCmbItem(IDC_COMBO_COLOR, message_get_res(IDS_STRING_POS_GUIDE_TEXT));
		SetCmbItem(IDC_COMBO_COLOR, message_get_res(IDS_STRING_POS_GUIDE_SELECT_BACKGROUND));
		SetCmbItem(IDC_COMBO_COLOR, message_get_res(IDS_STRING_POS_GUIDE_SELECT_TEXT));
		SetCmbItem(IDC_COMBO_COLOR, message_get_res(IDS_STRING_POS_GUIDE_BOX_BACKGROUND));
		SetCmbItem(IDC_COMBO_COLOR, message_get_res(IDS_STRING_POS_GUIDE_BOX_TEXT));
		SetCmbItem(IDC_COMBO_COLOR, message_get_res(IDS_STRING_POS_GUIDE_BOX_FRAME));
		SetCmbItem(IDC_COMBO_COLOR, message_get_res(IDS_STRING_POS_GUIDE_BOX_DISABLE));

		SetCmbItem(IDC_COMBO_COLOR, message_get_res(IDS_STRING_POS_VIEW_SEPARATE));

		SendDlgItemMessage(hDlg, IDC_COMBO_COLOR, CB_SETCURSEL, 0, 0);
		SendMessage(hDlg, WM_COMMAND, (WPARAM)MAKELPARAM(IDC_COMBO_COLOR, CBN_SELENDOK), 0);

		hdc = GetDC(hDlg);
		EnumFonts(hdc, NULL, (FONTENUMPROC)enum_faces_proc, (LPARAM)GetDlgItem(hDlg, IDC_COMBO_FONT));
		ReleaseDC(hDlg, hdc);
		break;

	case WM_CTLCOLORSTATIC:
        if ((HWND)lParam == GetDlgItem(hDlg, IDC_STATIC_FRAME)) {
			if (op_back_brush == NULL) {
				op_back_brush = CreateSolidBrush(op.ci.view_separate);
			}
            return (BOOL)op_back_brush;
		}
        return FALSE;;

	case WM_DESTROY:
		if (si.leg != NULL) {
			mem_free(&si.leg);
		}
		DeleteObject(op_back_brush);
		op_back_brush = NULL;
		break;

	case WM_DRAWITEM:
		dis = (LPDRAWITEMSTRUCT)lParam;
		// 枠の描画
		DrawFrameControl(dis->hDC, &(dis->rcItem), DFC_BUTTON,
			DFCS_BUTTONPUSH | ((dis->itemState & ODS_SELECTED) ? DFCS_PUSHED : 0));

		if (dis->itemState & ODS_DISABLED) {
			break;
		}

		i = (dis->itemState & ODS_SELECTED) ? 1 : 0;
		dis->rcItem.left += 4 + i;
		dis->rcItem.top += 4 + i;
		dis->rcItem.right -= 5 - i;
		dis->rcItem.bottom -= 5 - i;

		// 色の描画
		GetWindowText(GetDlgItem(hDlg, wParam), buf, BUF_SIZE - 1);
		color = _ttoi(buf);

		hbrush = CreateSolidBrush(color);
		FillRect(dis->hDC, &(dis->rcItem), hbrush);
		DeleteObject(hbrush);
		FrameRect(dis->hDC, &(dis->rcItem), GetStockObject(BLACK_BRUSH));

		if (dis->itemState & ODS_FOCUS) {
			// フォーカス
			DrawFocusRect(dis->hDC, &(dis->rcItem));
		}
		break;

	case WM_SETCURSOR:
		if (HIWORD(lParam) == WM_LBUTTONDOWN) {
			POINT pt;
			HWND point_wnd;
			HDC hdc;
			COLORREF color;

			GetCursorPos(&pt);
			point_wnd = WindowFromPoint(pt);

			hdc = GetDC(NULL);
			color = GetPixel(hdc, pt.x, pt.y);
			ReleaseDC(NULL, hdc);

			if(GetKeyState(VK_SHIFT) & 0x8000){
				SendMessage(hDlg, WM_SET_COLOR, 0, color);
				break;
			}

			if (point_wnd == list_wnd) {
				if (color == op.ci.background) {
					SendDlgItemMessage(hDlg, IDC_COMBO_COLOR, CB_SETCURSEL, POS_BACKGROUND, 0);
				} else if (color == op.ci.odd_background) {
					SendDlgItemMessage(hDlg, IDC_COMBO_COLOR, CB_SETCURSEL, POS_ODD_BACKGROUND, 0);
				} else if (color == op.ci.header_background) {
					SendDlgItemMessage(hDlg, IDC_COMBO_COLOR, CB_SETCURSEL, POS_HEADER_BACKGROUND, 0);
				} else if (color == op.ci.input_background) {
					SendDlgItemMessage(hDlg, IDC_COMBO_COLOR, CB_SETCURSEL, POS_INPUT_BACKGROUND, 0);
				} else if (color == op.ci.scored_text) {
					SendDlgItemMessage(hDlg, IDC_COMBO_COLOR, CB_SETCURSEL, POS_SCORED_TEXT, 0);
				} else {
					SendDlgItemMessage(hDlg, IDC_COMBO_COLOR, CB_SETCURSEL, POS_TOGO_TEXT, 0);
				}
				SendMessage(hDlg, WM_COMMAND, (WPARAM)MAKELPARAM(IDC_COMBO_COLOR, CBN_SELENDOK), 0);
			} else if (point_wnd == player_wnd) {
				if (color == op.ci.player_background) {
					SendDlgItemMessage(hDlg, IDC_COMBO_COLOR, CB_SETCURSEL, POS_PLAYER_BACKGROUND, 0);
				} else if (color == op.ci.player_name_background) {
					SendDlgItemMessage(hDlg, IDC_COMBO_COLOR, CB_SETCURSEL, POS_PLAYER_NAME_BACKGROUND, 0);
				} else if (color == op.ci.player_name_text) {
					SendDlgItemMessage(hDlg, IDC_COMBO_COLOR, CB_SETCURSEL, POS_PLAYER_NAME_TEXT, 0);
				} else {
					SendDlgItemMessage(hDlg, IDC_COMBO_COLOR, CB_SETCURSEL, POS_PLAYER_TEXT, 0);
				}
				SendMessage(hDlg, WM_COMMAND, (WPARAM)MAKELPARAM(IDC_COMBO_COLOR, CBN_SELENDOK), 0);
			} else if (point_wnd == left_wnd) {
				if (color == op.ci.left_background) {
					SendDlgItemMessage(hDlg, IDC_COMBO_COLOR, CB_SETCURSEL, POS_LEFT_BACKGROUND, 0);
				} else if (color == op.ci.left_active_border) {
					SendDlgItemMessage(hDlg, IDC_COMBO_COLOR, CB_SETCURSEL, POS_LEFT_ACTIVE_BORDER, 0);
				} else {
					SendDlgItemMessage(hDlg, IDC_COMBO_COLOR, CB_SETCURSEL, POS_LEFT_TEXT, 0);
				}
				SendMessage(hDlg, WM_COMMAND, (WPARAM)MAKELPARAM(IDC_COMBO_COLOR, CBN_SELENDOK), 0);
			}
		}
		break;

	case WM_SET_COLOR:
		color = lParam;

		wsprintf(buf, TEXT("%ld"), color);
		SetWindowText(GetDlgItem(hDlg, IDC_BUTTON_COLOR), buf);
		InvalidateRect(GetDlgItem(hDlg, IDC_BUTTON_COLOR), NULL, FALSE);
		UpdateWindow(GetDlgItem(hDlg, IDC_BUTTON_COLOR));

		switch (SendDlgItemMessage(hDlg, IDC_COMBO_COLOR, CB_GETCURSEL, 0, 0)) {
		case POS_VIEW_SEPARATE:
			op.ci.view_separate = color;
			DeleteObject(op_back_brush);
			op_back_brush = NULL;
			InvalidateRect(GetDlgItem(hDlg, IDC_STATIC_FRAME), NULL, FALSE);
			UpdateWindow(GetDlgItem(hDlg, IDC_STATIC_FRAME));
			break;
		case POS_BACKGROUND:
			op.ci.background = color;
			break;
		case POS_ODD_BACKGROUND:
			op.ci.odd_background = color;
			break;
		case POS_SCORED_TEXT:
			op.ci.scored_text = color;
			break;
		case POS_TOGO_TEXT:
			op.ci.togo_text = color;
			break;
		case POS_HEADER_BACKGROUND:
			op.ci.header_background = color;
			break;
		case POS_HEADER_TEXT:
			op.ci.header_text = color;
			break;
		case POS_LAST3NUMBER_TEXT:
			op.ci.last3number_text = color;
			break;
		case POS_LINE:
			op.ci.line = color;
			break;
		case POS_SEPARATE:
			op.ci.separate = color;
			break;
		case POS_TON_CIRCLE:
			op.ci.ton_circle = color;
			break;

		case POS_INPUT_BACKGROUND:
			op.ci.input_background = color;
			break;
		case POS_INPUT_TEXT:
			op.ci.input_text = color;
			break;
		case POS_INPUT_SELECT_BACKGROUND:
			op.ci.input_select_background = color;
			break;
		case POS_INPUT_SELECT_TEXT:
			op.ci.input_select_text = color;
			break;

		case POS_LEFT_BACKGROUND:
			op.ci.left_background = color;
			break;
		case POS_LEFT_TEXT:
			op.ci.left_text = color;
			break;
		case POS_LEFT_ACTIVE_BORDER:
			op.ci.left_active_border = color;
			break;

		case POS_PLAYER_NAME_BACKGROUND:
			op.ci.player_name_background = color;
			break;
		case POS_PLAYER_NAME_TEXT:
			op.ci.player_name_text = color;
			break;
		case POS_PLAYER_BACKGROUND:
			op.ci.player_background = color;
			break;
		case POS_PLAYER_TEXT:
			op.ci.player_text = color;
			break;
		case POS_PLAYER_INFO_TITLE:
			op.ci.player_info_title = color;
			break;

		case POS_GUIDE_BACKGROUND:
			op.ci.guide_background = color;
			break;
		case POS_GUIDE_TEXT:
			op.ci.guide_text = color;
			break;
		case POS_GUIDE_SELECT_BACKGROUND:
			op.ci.guide_select_background = color;
			break;
		case POS_GUIDE_SELECT_TEXT:
			op.ci.guide_select_text = color;
			break;
		case POS_GUIDE_BOX_BACKGROUND:
			op.ci.guide_box_background = color;
			break;
		case POS_GUIDE_BOX_TEXT:
			op.ci.guide_box_text = color;
			break;
		case POS_GUIDE_BOX_FRAME:
			op.ci.guide_box_frame = color;
			break;
		case POS_GUIDE_BOX_DISABLE:
			op.ci.guide_box_disable = color;
			break;
		}
		old_cursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

		SendMessage(list_wnd, WM_SCORE_DRAW_INIT, 0, 0);
		SendMessage(player_wnd, WM_PLAYER_DRAW_INIT, 0, 0);
		SendMessage(left_wnd, WM_LEFT_DRAW_INIT, 0, 0);

		SetCursor(old_cursor);
		break;

	case WM_NOTIFY:
		return option_notify_proc(hDlg, msg, wParam, lParam);

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_COMBO_FONT:
			if (HIWORD(wParam) == CBN_SELENDOK) {
				old_cursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

				SendDlgItemMessage(hDlg, IDC_COMBO_FONT, WM_GETTEXT, BUF_SIZE - 1, (LPARAM)op.font_name);
				SendMessage(list_wnd, WM_SCORE_DRAW_INIT, 0, 0);
				SendMessage(player_wnd, WM_PLAYER_DRAW_INIT, 0, 0);
				SendMessage(left_wnd, WM_LEFT_DRAW_INIT, 0, 0);

				SetCursor(old_cursor);
			}
			break;

		case IDC_COMBO_COLOR:
			if (HIWORD(wParam) == CBN_SELENDOK) {
				color = -1;
				switch (SendDlgItemMessage(hDlg, IDC_COMBO_COLOR, CB_GETCURSEL, 0, 0)) {
				case POS_VIEW_SEPARATE:
					color = op.ci.view_separate;
					break;
				case POS_BACKGROUND:
					color = op.ci.background;
					break;
				case POS_ODD_BACKGROUND:
					color = op.ci.odd_background;
					break;
				case POS_SCORED_TEXT:
					color = op.ci.scored_text;
					break;
				case POS_TOGO_TEXT:
					color = op.ci.togo_text;
					break;
				case POS_HEADER_BACKGROUND:
					color = op.ci.header_background;
					break;
				case POS_HEADER_TEXT:
					color = op.ci.header_text;
					break;
				case POS_LAST3NUMBER_TEXT:
					color = op.ci.last3number_text;
					break;
				case POS_LINE:
					color = op.ci.line;
					break;
				case POS_SEPARATE:
					color = op.ci.separate;
					break;
				case POS_TON_CIRCLE:
					color = op.ci.ton_circle;
					break;

				case POS_INPUT_BACKGROUND:
					color = op.ci.input_background;
					break;
				case POS_INPUT_TEXT:
					color = op.ci.input_text;
					break;
				case POS_INPUT_SELECT_BACKGROUND:
					color = op.ci.input_select_background;
					break;
				case POS_INPUT_SELECT_TEXT:
					color = op.ci.input_select_text;
					break;

				case POS_LEFT_BACKGROUND:
					color = op.ci.left_background;
					break;
				case POS_LEFT_TEXT:
					color = op.ci.left_text;
					break;
				case POS_LEFT_ACTIVE_BORDER:
					color = op.ci.left_active_border;
					break;

				case POS_PLAYER_NAME_BACKGROUND:
					color = op.ci.player_name_background;
					break;
				case POS_PLAYER_NAME_TEXT:
					color = op.ci.player_name_text;
					break;
				case POS_PLAYER_BACKGROUND:
					color = op.ci.player_background;
					break;
				case POS_PLAYER_TEXT:
					color = op.ci.player_text;
					break;
				case POS_PLAYER_INFO_TITLE:
					color = op.ci.player_info_title;
					break;

				case POS_GUIDE_BACKGROUND:
					color = op.ci.guide_background;
					break;
				case POS_GUIDE_TEXT:
					color = op.ci.guide_text;
					break;
				case POS_GUIDE_SELECT_BACKGROUND:
					color = op.ci.guide_select_background;
					break;
				case POS_GUIDE_SELECT_TEXT:
					color = op.ci.guide_select_text;
					break;
				case POS_GUIDE_BOX_BACKGROUND:
					color = op.ci.guide_box_background;
					break;
				case POS_GUIDE_BOX_TEXT:
					color = op.ci.guide_box_text;
					break;
				case POS_GUIDE_BOX_FRAME:
					color = op.ci.guide_box_frame;
					break;
				case POS_GUIDE_BOX_DISABLE:
					color = op.ci.guide_box_disable;
					break;
				}
				EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_COLOR), (color != -1) ? TRUE : FALSE);
				wsprintf(buf, TEXT("%ld"), color);
				SetWindowText(GetDlgItem(hDlg, IDC_BUTTON_COLOR), buf);
				InvalidateRect(GetDlgItem(hDlg, IDC_BUTTON_COLOR), NULL, FALSE);
				UpdateWindow(GetDlgItem(hDlg, IDC_BUTTON_COLOR));
			}
			break;

		case IDC_BUTTON_COLOR:
			GetWindowText(GetDlgItem(hDlg, IDC_BUTTON_COLOR), buf, BUF_SIZE - 1);
			color = _ttoi(buf);

			// 色の選択
			color = get_color(hDlg, color);
			if (color == _ttoi(buf)) {
				break;
			}
			SendMessage(hDlg, WM_SET_COLOR, 0, color);
			break;

		case IDC_BUTTON_RESET:
			if (MessageBox(hDlg, message_get_res(IDS_STRING_RESET_COLOR), APP_NAME, MB_ICONQUESTION | MB_YESNO) == IDNO) {
				break;
			}
			op.ci.view_separate = D_COLOR_VIEW_SEPARATE;

			op.ci.background = D_COLOR_BACKGROUND;
			op.ci.odd_background = D_COLOR_ODD_BACKGROUND;
			op.ci.togo_text = D_COLOR_TEXT;
			op.ci.scored_text = D_COLOR_TEXT;
			op.ci.header_background = D_COLOR_HEADER_BACKGROUND;
			op.ci.header_text = D_COLOR_HEADER_TEXT;
			op.ci.last3number_text = D_COLOR_LAST3NUMBER_TEXT;
			op.ci.line = D_COLOR_LINE;
			op.ci.separate = D_COLOR_SEPARATE;
			op.ci.ton_circle = D_COLOR_TON_CIRCLE;

			op.ci.input_background = D_COLOR_INPUT_BACKGROUND;
			op.ci.input_text = D_COLOR_INPUT_TEXT;
			op.ci.input_select_background = D_COLOR_INPUT_SELECT_BACKGROUND;
			op.ci.input_select_text = D_COLOR_INPUT_SELECT_TEXT;

			op.ci.left_background = D_COLOR_LEFT_BACKGROUND;
			op.ci.left_text = D_COLOR_LEFT_TEXT;
			op.ci.left_active_border = D_COLOR_LEFT_ACTIVE_BORDER;

			op.ci.player_name_background = D_COLOR_PLAYER_NAME_BACKGROUND;
			op.ci.player_name_text = D_COLOR_PLAYER_NAME_TEXT;
			op.ci.player_background = D_COLOR_PLAYER_BACKGROUND;
			op.ci.player_text = D_COLOR_PLAYER_TEXT;
			op.ci.player_info_title = D_COLOR_PLAYER_INFO_TITLE;

			op.ci.guide_background = D_COLOR_GUIDE_BACKGROUND;
			op.ci.guide_text = D_COLOR_GUIDE_TEXT;
			op.ci.guide_select_background = D_COLOR_GUIDE_SELECT_BACKGROUND;
			op.ci.guide_select_text = D_COLOR_GUIDE_SELECT_TEXT;
			op.ci.guide_box_background = D_COLOR_GUIDE_BOX_BACKGROUND;
			op.ci.guide_box_text = D_COLOR_GUIDE_BOX_TEXT;
			op.ci.guide_box_frame = D_COLOR_GUIDE_BOX_FRAME;
			op.ci.guide_box_disable = D_COLOR_GUIDE_BOX_DISABLE;

			old_cursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

			DeleteObject(op_back_brush);
			op_back_brush = NULL;
			InvalidateRect(GetDlgItem(hDlg, IDC_STATIC_FRAME), NULL, FALSE);
			UpdateWindow(GetDlgItem(hDlg, IDC_STATIC_FRAME));

			SendMessage(list_wnd, WM_SCORE_DRAW_INIT, 0, 0);
			SendMessage(player_wnd, WM_PLAYER_DRAW_INIT, 0, 0);
			SendMessage(left_wnd, WM_LEFT_DRAW_INIT, 0, 0);
			SendMessage(hDlg, WM_COMMAND, (WPARAM)MAKELPARAM(IDC_COMBO_COLOR, CBN_SELENDOK), 0);

			SetCursor(old_cursor);
			break;

		case IDOK:
			if (si.leg != NULL) {
				mem_free(&si.leg);
			}
			prop_ret = 1;
			break;

		case IDPCANCEL:
			// 情報の復元
			op.ci = re_ci;
			lstrcpy(op.font_name, re_font_name);
			break;
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;
}
/* End of source */
