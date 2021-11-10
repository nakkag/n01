/*
 * n01
 *
 * game_history.c
 *
 * Copyright (C) 1996-2005 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

/* Include Files */
#define _INC_OLE
#include <windows.h>
#undef  _INC_OLE
#include <tchar.h>

#include "General.h"
#include "Memory.h"
#include "font.h"
#include "Message.h"
#include "score_list.h"
#include "score_player.h"
#include "score_save.h"

#include "resource.h"

/* Define */
#define TYPE_HEADER				0
#define TYPE_LEG				1

/* Global Variables */
extern HINSTANCE hInst;
extern OPTION_INFO op;
extern SCORE_INFO si;

typedef struct _VIEW_INFO {
	int type;

	SCORE_INFO *si;
	int leg;
} VIEW_INFO;

/* Local Function Prototypes */
static void get_control_pos(const HWND hDlg, RECT *c_rect);
static BOOL CALLBACK score_list_proc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL add_score_info(const HWND hlist, SCORE_INFO *si);
static BOOL add_score_list(const HWND hlist, SCORE_HISTORY *sh);
static void drow_list_item(const DRAWITEMSTRUCT *di);
static void set_enabled(const HWND hDlg, const SCORE_HISTORY *sh);
static BOOL CALLBACK option_game_history_proc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

/*
 * get_control_pos - コントロールのクライアント座標を取得
 */
static void get_control_pos(const HWND hDlg, RECT *c_rect)
{
	RECT rect;
	POINT pt;

	GetClientRect(hDlg, &rect);

	c_rect->right = c_rect->right - c_rect->left;
	c_rect->bottom = c_rect->bottom - c_rect->top;
	pt.x = c_rect->left;
	pt.y = c_rect->top;
	ScreenToClient(hDlg, &pt);
	c_rect->left = rect.right - pt.x;
	c_rect->top = rect.bottom - pt.y;
}

/*
 * score_list_proc - ウィンドウプロシージャ
 */
static BOOL CALLBACK score_list_proc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HFONT hfont;
	VIEW_INFO *vi;
	RECT rect;
	POINT pt;
	TCHAR buf[BUF_SIZE];
	int i;
	static RECT score_rect, p1_rect, p2_rect, cancel_rect;

	switch (msg) {
	case WM_INITDIALOG:
		if ((vi = (VIEW_INFO *)lParam) == NULL) {
			EndDialog(hDlg, FALSE);
			break;
		}
		SetWindowLong(hDlg, GWL_USERDATA, lParam);

		// コントロールの初期座標を取得
		GetClientRect(hDlg, &rect);
		GetWindowRect(GetDlgItem(hDlg, IDC_VIEW_SCORE), &score_rect);
		pt.x = 0;
		pt.y = score_rect.top;
		ScreenToClient(hDlg, &pt);
		score_rect.top = pt.y;
		pt.x = 0;
		pt.y = score_rect.bottom;
		ScreenToClient(hDlg, &pt);
		score_rect.bottom = rect.bottom - pt.y + score_rect.top;

		GetWindowRect(GetDlgItem(hDlg, IDC_STATIC_P1), &p1_rect);
		get_control_pos(hDlg, &p1_rect);
		GetWindowRect(GetDlgItem(hDlg, IDC_STATIC_P2), &p2_rect);
		get_control_pos(hDlg, &p2_rect);
		GetWindowRect(GetDlgItem(hDlg, IDCANCEL), &cancel_rect);
		get_control_pos(hDlg, &cancel_rect);

		// ウィンドウの位置、サイズ設定
		if (op.score_list_rect.right > 0 && op.score_list_rect.bottom > 0) {
			SetWindowPos(hDlg, 0,
				op.score_list_rect.left, op.score_list_rect.top,
				op.score_list_rect.right, op.score_list_rect.bottom, SWP_NOZORDER);
		}
		SendMessage(hDlg, WM_SIZE, 0, 0);

		SendDlgItemMessage(hDlg, IDC_VIEW_SCORE, WM_SCORE_SET_INFO, 0, (LPARAM)vi->si);
		SendDlgItemMessage(hDlg, IDC_VIEW_SCORE, WM_SCORE_SET_HALF_HEIGHT, TRUE, 0);
		SendDlgItemMessage(hDlg, IDC_VIEW_P1, WM_PLAYER_SET_INFO, 1, (LPARAM)&vi->si->player[0]);
		SendDlgItemMessage(hDlg, IDC_VIEW_P1, WM_PLAYER_MODE_HISTORY, TRUE, 0);
		SendDlgItemMessage(hDlg, IDC_VIEW_P2, WM_PLAYER_SET_INFO, 1, (LPARAM)&vi->si->player[1]);
		SendDlgItemMessage(hDlg, IDC_VIEW_P2, WM_PLAYER_MODE_HISTORY, TRUE, 0);

		hfont = font_create(op.font_name, 12, 0, FALSE, FALSE);
		SendDlgItemMessage(hDlg, IDC_STATIC_P1, WM_SETFONT, (WPARAM)hfont, 0);
		SendDlgItemMessage(hDlg, IDC_STATIC_P2, WM_SETFONT, (WPARAM)hfont, 0);

		SendDlgItemMessage(hDlg, IDC_COMBO_LEG, CB_ADDSTRING, 0, (LPARAM)message_get_res(IDS_STRING_OP_LIST_PLAYER_INFO));
		for (i = 0; i < vi->si->current_leg; i++) {
			wsprintf(buf, message_get_res(IDS_STRING_OP_LIST_LEG), i + 1);
			SendDlgItemMessage(hDlg, IDC_COMBO_LEG, CB_ADDSTRING, 0, (LPARAM)buf);
		}
		SendDlgItemMessage(hDlg, IDC_COMBO_LEG, CB_SETCURSEL, vi->leg, 0);
		SendMessage(hDlg, WM_COMMAND, MAKELPARAM(IDC_COMBO_LEG, CBN_SELENDOK), 0);
		break;

	case WM_CLOSE:
		DeleteObject(hfont);
		EndDialog(hDlg, FALSE);
		break;

	case WM_SIZE:
		GetClientRect(hDlg, &rect);

		MoveWindow(GetDlgItem(hDlg, IDC_VIEW_SCORE), 0, score_rect.top, rect.right, rect.bottom - score_rect.bottom, TRUE);
		MoveWindow(GetDlgItem(hDlg, IDC_VIEW_P1), 0, score_rect.top, rect.right / 2 - 2, rect.bottom - score_rect.bottom + p1_rect.bottom, TRUE);
		MoveWindow(GetDlgItem(hDlg, IDC_VIEW_P2), rect.right / 2 + 2, score_rect.top, rect.right / 2 - 2, rect.bottom - score_rect.bottom + p1_rect.bottom, TRUE);
		MoveWindow(GetDlgItem(hDlg, IDC_STATIC_P1), 0, rect.bottom - p1_rect.top, p1_rect.right, p1_rect.bottom, TRUE);
		MoveWindow(GetDlgItem(hDlg, IDC_STATIC_P2), rect.right - p2_rect.left, rect.bottom - p2_rect.top, p2_rect.right, p2_rect.bottom, TRUE);
		MoveWindow(GetDlgItem(hDlg, IDCANCEL), rect.right - cancel_rect.left, rect.bottom - cancel_rect.top, cancel_rect.right, cancel_rect.bottom, TRUE);

		InvalidateRect(GetDlgItem(hDlg, IDCANCEL), NULL, FALSE);
		UpdateWindow(GetDlgItem(hDlg, IDCANCEL));
		break;

	case WM_EXITSIZEMOVE:
		// サイズ変更完了
		if (IsWindowVisible(hDlg) != 0 && IsIconic(hDlg) == 0 && IsZoomed(hDlg) == 0) {
			GetWindowRect(hDlg, (LPRECT)&op.score_list_rect);
			op.score_list_rect.right -= op.score_list_rect.left;
			op.score_list_rect.bottom -= op.score_list_rect.top;
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_COMBO_LEG:
			if (HIWORD(wParam) == CBN_SELENDOK) {
				vi = (VIEW_INFO *)GetWindowLong(hDlg, GWL_USERDATA);
				if (vi == NULL) {
					EndDialog(hDlg, FALSE);
					break;
				}
				i = SendDlgItemMessage(hDlg, IDC_COMBO_LEG, CB_GETCURSEL, 0, 0);
				if (i == 0) {
					ShowWindow(GetDlgItem(hDlg, IDC_VIEW_SCORE), SW_HIDE);
					ShowWindow(GetDlgItem(hDlg, IDC_VIEW_P1), SW_SHOW);
					ShowWindow(GetDlgItem(hDlg, IDC_VIEW_P2), SW_SHOW);
					ShowWindow(GetDlgItem(hDlg, IDC_STATIC_P1), SW_HIDE);
					ShowWindow(GetDlgItem(hDlg, IDC_STATIC_P2), SW_HIDE);
					break;
				}
				if (i < 0) {
					break;
				}
				ShowWindow(GetDlgItem(hDlg, IDC_VIEW_SCORE), SW_SHOW);
				ShowWindow(GetDlgItem(hDlg, IDC_VIEW_P1), SW_HIDE);
				ShowWindow(GetDlgItem(hDlg, IDC_VIEW_P2), SW_HIDE);
				ShowWindow(GetDlgItem(hDlg, IDC_STATIC_P1), SW_HIDE);
				ShowWindow(GetDlgItem(hDlg, IDC_STATIC_P2), SW_HIDE);
				if (vi->si->leg[i - 1].darts > 0) {
					wsprintf(buf, message_get_res(IDS_STRING_OP_LIST_DARTS), vi->si->leg[i - 1].darts);
					if (vi->si->leg[i - 1].winner == 0) {
						ShowWindow(GetDlgItem(hDlg, IDC_STATIC_P1), SW_SHOW);
						SetWindowText(GetDlgItem(hDlg, IDC_STATIC_P1), buf);
						SetWindowText(GetDlgItem(hDlg, IDC_STATIC_P2), TEXT(""));
					} else {
						ShowWindow(GetDlgItem(hDlg, IDC_STATIC_P2), SW_SHOW);
						SetWindowText(GetDlgItem(hDlg, IDC_STATIC_P1), TEXT(""));
						SetWindowText(GetDlgItem(hDlg, IDC_STATIC_P2), buf);
					}
				}
				SendDlgItemMessage(hDlg, IDC_VIEW_SCORE, WM_SCORE_SHOW_LEG, i - 1, 0);
			}
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
 * add_score_info - リストにLeg情報を追加
 */
static BOOL add_score_info(const HWND hlist, SCORE_INFO *si)
{
	VIEW_INFO *vi;
	TCHAR buf[BUF_SIZE];
	int i, j;

	for (i = 0; i < si->current_leg; i++) {
		wsprintf(buf, message_get_res(IDS_STRING_OP_LIST_LEG), i + 1);
		j = SendMessage(hlist, LB_ADDSTRING, 0, (LPARAM)buf);
		vi = (VIEW_INFO *)mem_calloc(sizeof(VIEW_INFO));
		vi->type = TYPE_LEG;
		vi->si = si;
		vi->leg = i + 1;
		SendMessage(hlist, LB_SETITEMDATA, j, (LPARAM)vi);
	}
	return TRUE;
}

/*
 * add_score_list - リストにアイテムを追加
 */
static BOOL add_score_list(const HWND hlist, SCORE_HISTORY *sh)
{
	VIEW_INFO *vi;
	TCHAR buf[BUF_SIZE];
	int i, j;

	for (i = 0; i < sh->list_count; i++) {
		_itot(sh->list[i].start_score, buf, 10);
		j = SendMessage(hlist, LB_ADDSTRING, 0, (LPARAM)buf);
		vi = (VIEW_INFO *)mem_calloc(sizeof(VIEW_INFO));
		vi->type = TYPE_HEADER;
		vi->si = &sh->list[i];
		vi->leg = 0;
		SendMessage(hlist, LB_SETITEMDATA, j, (LPARAM)vi);
		add_score_info(hlist, &sh->list[i]);
	}
	if (sh->si->history == FALSE && sh->si->current_leg > 0) {
		_itot(sh->si->start_score, buf, 10);
		j = SendMessage(hlist, LB_ADDSTRING, 0, (LPARAM)buf);
		vi = (VIEW_INFO *)mem_calloc(sizeof(VIEW_INFO));
		vi->type = TYPE_HEADER;
		vi->si = sh->si;
		vi->leg = 0;
		SendMessage(hlist, LB_SETITEMDATA, j, (LPARAM)vi);
		add_score_info(hlist, sh->si);
	}
	return TRUE;
}

/*
 * drow_list_item - リストアイテムの描画
 */
static void drow_list_item(const DRAWITEMSTRUCT *di)
{
	VIEW_INFO *vi;
	HANDLE hBrush;
	HFONT hfont, ret_font;
	HPEN pen, ret_pen;
	RECT rect;
	RECT header_rect, info_rect, p_rect[2];
	SIZE sz;
	COLORREF text_color;
	COLORREF back_color;
	TCHAR buf[BUF_SIZE];
	int i;

	if((int)di->itemData <= 0){
		hBrush = CreateSolidBrush(GetSysColor(COLOR_WINDOW));
		FillRect(di->hDC, &di->rcItem, hBrush);
		DeleteObject(hBrush);
		return;
	}
	vi = (VIEW_INFO *)di->itemData;

	rect = di->rcItem;
	i = (rect.right * 100) / 6;
	SetRect(&header_rect, rect.left + 4, rect.top, i * 1 / 100, rect.bottom);
	SetRect(&info_rect, i * 1 / 100, rect.top, i * 2 / 100, rect.bottom);
	SetRect(&p_rect[0], i * 2 / 100, rect.top, i * 4 / 100, rect.bottom);
	SetRect(&p_rect[1], i * 4 / 100, rect.top, rect.right, rect.bottom);

	if (di->itemState & ODS_SELECTED) {
		text_color = GetSysColor(COLOR_HIGHLIGHTTEXT);
		back_color = GetSysColor(COLOR_HIGHLIGHT);
	} else {
		text_color = GetSysColor(COLOR_WINDOWTEXT);
		back_color = (vi->type == TYPE_HEADER) ? GetSysColor(COLOR_3DFACE) : GetSysColor(COLOR_WINDOW);
	}
	hBrush = CreateSolidBrush(back_color);
	FillRect(di->hDC, &rect, hBrush);
	DeleteObject(hBrush);
	if (di->itemState & ODS_FOCUS) {
		DrawFocusRect(di->hDC, &rect);
	}

	SetTextColor(di->hDC, text_color);
	SetBkColor(di->hDC, back_color);

	switch (vi->type) {
	case TYPE_HEADER:
		hfont = font_create_menu(12, FW_BOLD, FALSE);
		ret_font = SelectObject(di->hDC, hfont);
		_itot(vi->si->start_score, buf, 10);
		DrawText(di->hDC, buf, lstrlen(buf), &header_rect, DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
		if (vi->si->leg_limit == 1) {
			wsprintf(buf, message_get_res(IDS_STRING_OP_LIST_SET), vi->si->current_set + 1);
			DrawText(di->hDC, buf, lstrlen(buf), &info_rect, DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
		}
		SelectObject(di->hDC, ret_font);
		DeleteObject(hfont);

		for (i = 0; i < 2; i++) {
			if (vi->si->player[i].com == TRUE) {
				wsprintf(buf, message_get_res(IDS_STRING_COM), vi->si->player[i].level + 1);
			} else {
				lstrcpy(buf, vi->si->player[i].name);
			}
			if (vi->si->player[i].legs > vi->si->player[!i].legs) {
				hfont = font_create_menu(12, FW_BOLD, TRUE);
				ret_font = SelectObject(di->hDC, hfont);
			} else {
				hfont = font_create_menu(12, FW_BOLD, FALSE);
				ret_font = SelectObject(di->hDC, hfont);
			}
			DrawText(di->hDC, buf, lstrlen(buf), &p_rect[i], DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
			SelectObject(di->hDC, ret_font);
			DeleteObject(hfont);
		}
		break;

	case TYPE_LEG:
		hfont = font_create_menu(12, 0, FALSE);
		ret_font = SelectObject(di->hDC, hfont);
		wsprintf(buf, message_get_res(IDS_STRING_OP_LIST_LEG), vi->leg);
		DrawText(di->hDC, buf, lstrlen(buf), &info_rect, DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

		GetTextExtentPoint32(di->hDC, TEXT("8888"), lstrlen(TEXT("8888")), &sz);
		for (i = 0; i < 2; i++) {
			p_rect[i].left += ((p_rect[i].right - p_rect[i].left) - sz.cx) / 2;
			p_rect[i].right = p_rect[i].left + sz.cx;
			p_rect[i].top++;
			p_rect[i].bottom--;
			if (vi->si->leg[vi->leg - 1].winner == i && vi->si->leg[vi->leg - 1].darts > 0) {
				SetTextColor(di->hDC, text_color);
				hBrush = CreateSolidBrush(text_color);
				_itot(vi->si->leg[vi->leg - 1].darts, buf, 10);
			} else {
				SetTextColor(di->hDC, GetSysColor(COLOR_3DSHADOW));
				hBrush = CreateSolidBrush(GetSysColor(COLOR_3DSHADOW));
				lstrcpy(buf, TEXT("-"));
			}
			DrawText(di->hDC, buf, lstrlen(buf), &p_rect[i], DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
			FrameRect(di->hDC, &p_rect[i], hBrush);
			DeleteObject(hBrush);
		}
		SelectObject(di->hDC, ret_font);
		DeleteObject(hfont);

		pen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DSHADOW));
		ret_pen = SelectObject(di->hDC, pen);
		MoveToEx(di->hDC, p_rect[0].right, rect.top + (rect.bottom - rect.top) / 2, NULL);
		LineTo(di->hDC, p_rect[1].left, rect.top + (rect.bottom - rect.top) / 2);
		SelectObject(di->hDC, ret_pen);
		DeleteObject(pen);
		break;
	}
}

/*
 * set_enabled - ボタンの状態設定
 */
static void set_enabled(const HWND hDlg, const SCORE_HISTORY *sh)
{
	if (SendDlgItemMessage(hDlg, IDC_LIST_SCORE_LIST, LB_GETCURSEL, 0, 0) == -1) {
		EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_VIEW), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_SAVE), FALSE);
	} else {
		EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_VIEW), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_SAVE), TRUE);
	}
	if (sh->list_count == 0) {
		EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_CLEAR), FALSE);
	} else {
		EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_CLEAR), TRUE);
	}
}

/*
 * option_game_history_proc - ウィンドウプロシージャ
 */
static BOOL CALLBACK option_game_history_proc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	SCORE_HISTORY *sh;
	VIEW_INFO *vi;
	RECT rect;
	POINT pt;
	int i;
	static RECT list_rect, view_rect, save_rect, clear_rect, cancel_rect;

	switch (msg) {
	case WM_INITDIALOG:
		if ((sh = (SCORE_HISTORY *)lParam) == NULL) {
			EndDialog(hDlg, FALSE);
			break;
		}
		SetWindowLong(hDlg, GWL_USERDATA, lParam);

		// コントロールの初期座標を取得
		GetClientRect(hDlg, &rect);
		GetWindowRect(GetDlgItem(hDlg, IDC_LIST_SCORE_LIST), &list_rect);
		pt.x = 0;
		pt.y = list_rect.bottom;
		ScreenToClient(hDlg, &pt);
		list_rect.bottom = rect.bottom - pt.y;

		GetWindowRect(GetDlgItem(hDlg, IDC_BUTTON_VIEW), &view_rect);
		get_control_pos(hDlg, &view_rect);
		GetWindowRect(GetDlgItem(hDlg, IDC_BUTTON_SAVE), &save_rect);
		get_control_pos(hDlg, &save_rect);
		GetWindowRect(GetDlgItem(hDlg, IDC_BUTTON_CLEAR), &clear_rect);
		get_control_pos(hDlg, &clear_rect);
		GetWindowRect(GetDlgItem(hDlg, IDCANCEL), &cancel_rect);
		get_control_pos(hDlg, &cancel_rect);

		// ウィンドウの位置、サイズ設定
		if (op.history_rect.right > 0 && op.history_rect.bottom > 0) {
			SetWindowPos(hDlg, 0,
				op.history_rect.left, op.history_rect.top,
				op.history_rect.right, op.history_rect.bottom, SWP_NOZORDER);
		}
		SendMessage(hDlg, WM_SIZE, 0, 0);

		add_score_list(GetDlgItem(hDlg, IDC_LIST_SCORE_LIST), sh);
		SendDlgItemMessage(hDlg, IDC_LIST_SCORE_LIST, LB_SETCURSEL, 0, 0);
		set_enabled(hDlg, sh);
		break;

	case WM_CLOSE:
		for (i = 0; i < SendDlgItemMessage(hDlg, IDC_LIST_SCORE_LIST, LB_GETCOUNT, 0, 0); i++) {
			vi = (VIEW_INFO *)SendDlgItemMessage(hDlg, IDC_LIST_SCORE_LIST, LB_GETITEMDATA, i, 0);
			if (vi != NULL) {
				mem_free(&vi);
				SendDlgItemMessage(hDlg, IDC_LIST_SCORE_LIST, LB_SETITEMDATA, i, 0);
			}
		}
		EndDialog(hDlg, FALSE);
		break;

	case WM_SIZE:
		GetClientRect(hDlg, &rect);

		MoveWindow(GetDlgItem(hDlg, IDC_LIST_SCORE_LIST), 0, 0, rect.right, rect.bottom - list_rect.bottom, TRUE);

		MoveWindow(GetDlgItem(hDlg, IDC_BUTTON_VIEW), rect.right - view_rect.left, rect.bottom - view_rect.top, view_rect.right, view_rect.bottom, TRUE);
		MoveWindow(GetDlgItem(hDlg, IDC_BUTTON_SAVE), rect.right - save_rect.left, rect.bottom - save_rect.top, save_rect.right, save_rect.bottom, TRUE);
		MoveWindow(GetDlgItem(hDlg, IDC_BUTTON_CLEAR), rect.right - clear_rect.left, rect.bottom - clear_rect.top, clear_rect.right, clear_rect.bottom, TRUE);

		MoveWindow(GetDlgItem(hDlg, IDCANCEL), rect.right - cancel_rect.left, rect.bottom - cancel_rect.top, cancel_rect.right, cancel_rect.bottom, TRUE);

		InvalidateRect(hDlg, NULL, FALSE);
		UpdateWindow(hDlg);
		break;

	case WM_EXITSIZEMOVE:
		// サイズ変更完了
		if (IsWindowVisible(hDlg) != 0 && IsIconic(hDlg) == 0 && IsZoomed(hDlg) == 0) {
			GetWindowRect(hDlg, (LPRECT)&op.history_rect);
			op.history_rect.right -= op.history_rect.left;
			op.history_rect.bottom -= op.history_rect.top;
		}
		break;

	case WM_MEASUREITEM:
		if((UINT)wParam == IDC_LIST_SCORE_LIST){
			((LPMEASUREITEMSTRUCT)lParam)->itemHeight = 24;
		}
		return TRUE;

	case WM_DRAWITEM:
		if((UINT)wParam == IDC_LIST_SCORE_LIST){
			drow_list_item((LPDRAWITEMSTRUCT)lParam);
		}
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_LIST_SCORE_LIST:
			if (HIWORD(wParam) == LBN_DBLCLK) {
				SendMessage(hDlg, WM_COMMAND, IDC_BUTTON_VIEW, 0);
			}
			break;

		case IDC_BUTTON_VIEW:
			if (SendDlgItemMessage(hDlg, IDC_LIST_SCORE_LIST, LB_GETCURSEL, 0, 0) == -1) {
				break;
			}
			vi = (VIEW_INFO *)SendDlgItemMessage(hDlg, IDC_LIST_SCORE_LIST, LB_GETITEMDATA, SendDlgItemMessage(hDlg, IDC_LIST_SCORE_LIST, LB_GETCURSEL, 0, 0), 0);
			if (vi != NULL) {
				DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_SCORE_LIST), hDlg, score_list_proc, (LPARAM)vi);
			}
			break;

		case IDC_BUTTON_SAVE:
			if (SendDlgItemMessage(hDlg, IDC_LIST_SCORE_LIST, LB_GETCURSEL, 0, 0) == -1) {
				break;
			}
			vi = (VIEW_INFO *)SendDlgItemMessage(hDlg, IDC_LIST_SCORE_LIST, LB_GETITEMDATA, SendDlgItemMessage(hDlg, IDC_LIST_SCORE_LIST, LB_GETCURSEL, 0, 0), 0);
			if (vi != NULL) {
				score_save(hDlg, vi->si);
			}
			break;

		case IDC_BUTTON_CLEAR:
			if (MessageBox(hDlg, message_get_res(IDS_STRING_OP_LIST_CLEAR), APP_NAME, MB_ICONQUESTION | MB_YESNO) == IDNO) {
				break;
			}
			for (i = 0; i < SendDlgItemMessage(hDlg, IDC_LIST_SCORE_LIST, LB_GETCOUNT, 0, 0); i++) {
				vi = (VIEW_INFO *)SendDlgItemMessage(hDlg, IDC_LIST_SCORE_LIST, LB_GETITEMDATA, i, 0);
				if (vi != NULL) {
					mem_free(&vi);
					SendDlgItemMessage(hDlg, IDC_LIST_SCORE_LIST, LB_SETITEMDATA, i, 0);
				}
			}
			SendDlgItemMessage(hDlg, IDC_LIST_SCORE_LIST, LB_RESETCONTENT, 0, 0);
			SendMessage(GetParent(hDlg), WM_WINDOW_CLEAR_LIST, 0, 0);

			sh = (SCORE_HISTORY *)GetWindowLong(hDlg, GWL_USERDATA);
			if (sh == NULL) {
				break;
			}
			add_score_list(GetDlgItem(hDlg, IDC_LIST_SCORE_LIST), sh);
			SendDlgItemMessage(hDlg, IDC_LIST_SCORE_LIST, LB_SETCURSEL, 0, 0);
			set_enabled(hDlg, sh);
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
 * show_game_history - ゲーム履歴
 */
BOOL show_game_history(const HINSTANCE hInst, const HWND hWnd, SCORE_HISTORY *sh)
{
	return DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_GAME_HISTORY), hWnd, option_game_history_proc, (LPARAM)sh);
}
/* End of source */
