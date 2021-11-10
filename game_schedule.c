/*
 * n01
 *
 * option_schedule.c
 *
 * Copyright (C) 1996-2006 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

/* Include Files */
#define _INC_OLE
#include <windows.h>
#undef  _INC_OLE
#include <commctrl.h>

#include "General.h"
#include "Memory.h"
#include "String.h"
#include "Message.h"
#include "option.h"
#include "game_option.h"
#include "font.h"
#include "ini.h"

#include "resource.h"

/* Define */

/* Global Variables */
extern HINSTANCE hInst;
extern OPTION_INFO op;
extern TCHAR ini_path[MAX_PATH];

/* Local Function Prototypes */
static void listview_set_text(const HWND hListView, const int i);
static void listview_set_game(const HWND hListView, GAME_INFO *gi, const BOOL copy);
static void listview_free_game(const HWND hListView);
static BOOL CALLBACK game_schedule_proc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

/*
 * listview_set_text - ListViewのテキストを設定
 */
static void listview_set_text(const HWND hListView, const int i)
{
	GAME_INFO *gi;
	TCHAR buf[BUF_SIZE];

	if ((gi = (GAME_INFO *)listview_get_lparam(hListView, i)) == NULL) {
		return;
	}
	// 開始スコア
	wsprintf(buf, TEXT("%d"), gi->start_score);
	ListView_SetItemText(hListView, i, 0, buf);

	// ラウンド
	if (gi->round_limit == 1) {
		wsprintf(buf, TEXT("%d"), gi->round);
		ListView_SetItemText(hListView, i, 1, buf);
	} else {
		ListView_SetItemText(hListView, i, 1, TEXT(""));
	}

	// レッグ
	if (gi->best_of == 1) {
		wsprintf(buf, message_get_res(IDS_STRING_OP_SCH_BEST_OF), gi->max_leg);
	} else {
		wsprintf(buf, TEXT("%d"), gi->max_leg);
	}
	ListView_SetItemText(hListView, i, 2, buf);

	// プレイヤー1
	if (gi->com[0] == TRUE) {
		wsprintf(buf, message_get_res(IDS_STRING_COM), gi->level[0] + 1);
		ListView_SetItemText(hListView, i, 3, buf);
	} else {
		ListView_SetItemText(hListView, i, 3, gi->player_name[0]);
	}

	// プレイヤー2
	if (gi->com[1] == TRUE) {
		wsprintf(buf, message_get_res(IDS_STRING_COM), gi->level[1] + 1);
		ListView_SetItemText(hListView, i, 4, buf);
	} else {
		ListView_SetItemText(hListView, i, 4, gi->player_name[1]);
	}
}

/*
 * listview_set_game - ListViewにゲーム情報を追加する
 */
static void listview_set_game(const HWND hListView, GAME_INFO *gi, const BOOL copy)
{
	LV_ITEM lvi;
	GAME_INFO *new_gi;
	int i;

	if (copy == TRUE) {
		if ((new_gi = mem_calloc(sizeof(GAME_INFO))) == NULL) {
			return;
		}
		CopyMemory(new_gi, gi, sizeof(GAME_INFO));
	} else {
		new_gi = gi;
	}

	lvi.mask = LVIF_TEXT | LVIF_PARAM;
	lvi.iItem = ListView_GetItemCount(hListView);
	lvi.iSubItem = 0;
	lvi.pszText = TEXT("");
	lvi.cchTextMax = BUF_SIZE - 1;
	lvi.iImage = 0;
	lvi.lParam = (LPARAM)new_gi;
	i = ListView_InsertItem(hListView, &lvi);

	listview_set_text(hListView, i);
}

/*
 * listview_free_game - ゲーム情報の解放
 */
static void listview_free_game(const HWND hListView)
{
	GAME_INFO *gi;
	int i;

	for (i = 0; i < ListView_GetItemCount(hListView); i++) {
		if ((gi = (GAME_INFO *)listview_get_lparam(hListView, i)) != NULL) {
			mem_free(&gi);
		}
		listview_set_lparam(hListView, i, 0);
	}
}

/*
 * game_schedule_proc - ウィンドウプロシージャ
 */
static BOOL CALLBACK game_schedule_proc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HFONT hfont;
	static int current_set;
	LV_COLUMN lvc;
	RECT parent_rect, rect;
	GAME_INFO *gi;
	TCHAR buf[BUF_SIZE];
	int left, top;
	int i;
	BOOL enable;
	GAME_INFO *tmp_list;
	int tmp_count;

	switch (msg) {
	case WM_INITDIALOG:
		current_set = lParam;
		if (current_set >= 0) {
			current_set--;
			SetWindowText(hDlg, message_get_res(IDS_STRING_OP_SCH_TITLE));
			SetWindowText(GetDlgItem(hDlg, IDOK), message_get_res(IDS_STRING_OP_SCH_OK));
		}

		// センタリング
		GetWindowRect(GetParent(hDlg), &parent_rect);
		GetWindowRect(hDlg, &rect);
		left = parent_rect.left + ((parent_rect.right - parent_rect.left) - (rect.right - rect.left)) / 2;
		if (left < 0) left = 0;
		top = parent_rect.top + ((parent_rect.bottom - parent_rect.top) - (rect.bottom - rect.top)) / 2;
		if (top < 0) top = 0;
		SetWindowPos(hDlg, 0, left, top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

		hfont = font_create_menu(12, 0, FALSE);
		SendDlgItemMessage(hDlg, IDC_LIST_GAME, WM_SETFONT, (WPARAM)hfont, 0);

		i = 0;
		lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;

		lvc.fmt = LVCFMT_LEFT;
		lvc.cx = 85;
		lvc.pszText = message_get_res(IDS_STRING_OP_SCH_START_SCORE);
		lvc.iSubItem = i++;
		ListView_InsertColumn(GetDlgItem(hDlg, IDC_LIST_GAME), lvc.iSubItem, &lvc);

		lvc.fmt = LVCFMT_RIGHT;
		lvc.cx = 70;
		lvc.pszText = message_get_res(IDS_STRING_OP_SCH_ROUND);
		lvc.iSubItem = i++;
		ListView_InsertColumn(GetDlgItem(hDlg, IDC_LIST_GAME), lvc.iSubItem, &lvc);

		lvc.fmt = LVCFMT_RIGHT;
		lvc.cx = 90;
		lvc.pszText = message_get_res(IDS_STRING_OP_SCH_LEG);
		lvc.iSubItem = i++;
		ListView_InsertColumn(GetDlgItem(hDlg, IDC_LIST_GAME), lvc.iSubItem, &lvc);

		lvc.fmt = LVCFMT_LEFT;
		lvc.cx = 120;
		lvc.pszText = message_get_res(IDS_STRING_OP_SCH_P1);
		lvc.iSubItem = i++;
		ListView_InsertColumn(GetDlgItem(hDlg, IDC_LIST_GAME), lvc.iSubItem, &lvc);

		lvc.fmt = LVCFMT_LEFT;
		lvc.cx = 120;
		lvc.pszText = message_get_res(IDS_STRING_OP_SCH_P2);
		lvc.iSubItem = i++;
		ListView_InsertColumn(GetDlgItem(hDlg, IDC_LIST_GAME), lvc.iSubItem, &lvc);

		SendDlgItemMessage(hDlg, IDC_LIST_GAME, LVM_SETEXTENDEDLISTVIEWSTYLE, 0,
			LVS_EX_ONECLICKACTIVATE | LVS_EX_FULLROWSELECT |
			SendDlgItemMessage(hDlg, IDC_LIST_GAME, LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0));

		for (i = 0; i < op.gi_list_count; i++) {
			listview_set_game(GetDlgItem(hDlg, IDC_LIST_GAME), &op.gi_list[i], TRUE);
		}
		if (ListView_GetItemCount(GetDlgItem(hDlg, IDC_LIST_GAME)) <= 0) {
			EnableWindow(GetDlgItem(hDlg, IDOK), FALSE);
		}
		ListView_SetItemState(GetDlgItem(hDlg, IDC_LIST_GAME), 0, LVIS_FOCUSED, LVIS_FOCUSED);
		SendMessage(hDlg, WM_LV_EVENT, LVN_ITEMCHANGED, 0);
		break;

	case WM_CLOSE:
		listview_free_game(GetDlgItem(hDlg, IDC_LIST_GAME));
		DeleteObject(hfont);
		EndDialog(hDlg, FALSE);
		break;

	case WM_DRAWITEM:
		switch ((UINT)wParam) {
		case IDC_BUTTON_UP:
			i = DFCS_SCROLLUP;
			break;

		case IDC_BUTTON_DOWN:
			i = DFCS_SCROLLDOWN;
			break;

		default:
			return FALSE;
		}
		draw_scroll_control((LPDRAWITEMSTRUCT)lParam, i);
		break;

	case WM_NOTIFY:
		listview_notify_proc(hDlg, lParam, GetDlgItem(hDlg, IDC_LIST_GAME));
		break;

	case WM_LV_EVENT:
		switch (wParam) {
		case LVN_ITEMCHANGED:
			enable = (ListView_GetSelectedCount(GetDlgItem(hDlg, IDC_LIST_GAME)) <= 0) ? FALSE : TRUE;
			if (current_set >= 0 && enable == TRUE &&
				current_set >= ListView_GetNextItem(GetDlgItem(hDlg, IDC_LIST_GAME), -1, LVNI_SELECTED)) {
				enable = FALSE;
			}
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_UP), enable);
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_DOWN), enable);
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_EDIT), enable);
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_DELETE), enable);
			break;

		case NM_CUSTOMDRAW:
			// カスタムドロー
			switch (((LPNMLVCUSTOMDRAW)lParam)->nmcd.dwDrawStage) {
			case CDDS_PREPAINT:
				SetWindowLong(hDlg, DWL_MSGRESULT, CDRF_NOTIFYITEMDRAW);
				return TRUE;

			case CDDS_ITEMPREPAINT:
				if (current_set >= 0 && current_set >= (int)((LPNMLVCUSTOMDRAW)lParam)->nmcd.dwItemSpec &&
					((LPNMLVCUSTOMDRAW)lParam)->nmcd.uItemState != (CDIS_FOCUS | CDIS_SELECTED)) {
					((LPNMLVCUSTOMDRAW)lParam)->clrText = GetSysColor(COLOR_GRAYTEXT);
				}
				SetWindowLong(hDlg, DWL_MSGRESULT, CDRF_DODEFAULT);
				return TRUE;
			}
			break;
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_BUTTON_UP:
			if ((i = ListView_GetNextItem(GetDlgItem(hDlg, IDC_LIST_GAME), -1, LVNI_SELECTED)) == -1) {
				break;
			}
			if (i == 0) {
				break;
			}
			if (current_set >= 0 && i == current_set + 1) {
				break;
			}
			listview_move_item(GetDlgItem(hDlg, IDC_LIST_GAME), i, -1);
			SetFocus(GetDlgItem(hDlg, LOWORD(wParam)));
			break;

		case IDC_BUTTON_DOWN:
			if ((i = ListView_GetNextItem(GetDlgItem(hDlg, IDC_LIST_GAME), -1, LVNI_SELECTED)) == -1) {
				break;
			}
			if (i == ListView_GetItemCount(GetDlgItem(hDlg, IDC_LIST_GAME)) - 1) {
				break;
			}
			listview_move_item(GetDlgItem(hDlg, IDC_LIST_GAME), i, 1);
			SetFocus(GetDlgItem(hDlg, LOWORD(wParam)));
			break;

		case IDC_BUTTON_ADD:
			gi = (GAME_INFO *)mem_alloc(sizeof(GAME_INFO));
			if (gi == NULL) {
				message_get_error(GetLastError(), buf);
				MessageBox(hDlg, buf, APP_NAME, MB_ICONERROR);
				break;
			}
			CopyMemory(gi, &op.gi, sizeof(GAME_INFO));
			gi->schedule_flag = TRUE;
			gi->leg_limit = 1;
			if (show_game_option(hInst, hDlg, gi) == FALSE) {
				mem_free(&gi);
			} else {
				listview_set_game(GetDlgItem(hDlg, IDC_LIST_GAME), gi, FALSE);
			}
			if (ListView_GetItemCount(GetDlgItem(hDlg, IDC_LIST_GAME)) > 0) {
				EnableWindow(GetDlgItem(hDlg, IDOK), TRUE);
			}
			break;

		case IDC_BUTTON_EDIT:
			if ((i = ListView_GetNextItem(GetDlgItem(hDlg, IDC_LIST_GAME), -1, LVNI_SELECTED)) == -1) {
				break;
			}
			if (current_set >= 0 && i <= current_set) {
				break;
			}
			if ((gi = (GAME_INFO *)listview_get_lparam(GetDlgItem(hDlg, IDC_LIST_GAME), i)) != NULL) {
				gi->schedule_flag = TRUE;
				show_game_option(hInst, hDlg, gi);
			}
			listview_set_text(GetDlgItem(hDlg, IDC_LIST_GAME), i);
			break;

		case IDC_BUTTON_DELETE:
			if ((i = ListView_GetNextItem(GetDlgItem(hDlg, IDC_LIST_GAME), -1, LVNI_SELECTED)) == -1) {
				break;
			}
			if (current_set >= 0 && i <= current_set) {
				break;
			}
			if (MessageBox(hDlg, message_get_res(IDS_STRING_OP_DELETE), APP_NAME, MB_ICONQUESTION | MB_YESNO) == IDNO) {
				break;
			}
			if ((gi = (GAME_INFO *)listview_get_lparam(GetDlgItem(hDlg, IDC_LIST_GAME), i)) != NULL) {
				mem_free(&gi);
			}
			ListView_DeleteItem(GetDlgItem(hDlg, IDC_LIST_GAME), i);
			ListView_SetItemState(GetDlgItem(hDlg, IDC_LIST_GAME), i,
				LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
			if (ListView_GetItemCount(GetDlgItem(hDlg, IDC_LIST_GAME)) <= 0) {
				EnableWindow(GetDlgItem(hDlg, IDOK), FALSE);
			}
			break;

		case IDC_BUTTON_SAVE:
			// 退避
			tmp_count = op.gi_list_count;
			tmp_list = op.gi_list;
			op.gi_list_count = ListView_GetItemCount(GetDlgItem(hDlg, IDC_LIST_GAME));
			op.gi_list = (GAME_INFO *)mem_alloc(sizeof(GAME_INFO) * op.gi_list_count);
			if (op.gi_list == NULL) {
				op.gi_list_count = tmp_count;
				op.gi_list = tmp_list;
				break;
			}
			for (i = 0; i < op.gi_list_count; i++) {
				if ((gi = (GAME_INFO *)listview_get_lparam(GetDlgItem(hDlg, IDC_LIST_GAME), i)) != NULL) {
					CopyMemory(&op.gi_list[i], gi, sizeof(GAME_INFO));
				}
			}
			ini_put_game_schedule(ini_path);
			mem_free(&op.gi_list);
			op.gi_list_count = tmp_count;
			op.gi_list = tmp_list;
			MessageBox(hDlg, message_get_res(IDS_STRING_OP_GAME_SAVE), APP_NAME, MB_ICONINFORMATION);
			break;

		case IDOK:
			i = ListView_GetItemCount(GetDlgItem(hDlg, IDC_LIST_GAME));
			gi = (GAME_INFO *)mem_alloc(sizeof(GAME_INFO) * i);
			if (gi == NULL) {
				break;
			}
			mem_free(&op.gi_list);
			op.gi_list = gi;
			op.gi_list_count = i;
			for (i = 0; i < op.gi_list_count; i++) {
				if ((gi = (GAME_INFO *)listview_get_lparam(GetDlgItem(hDlg, IDC_LIST_GAME), i)) != NULL) {
					CopyMemory(&op.gi_list[i], gi, sizeof(GAME_INFO));
				}
			}
			listview_free_game(GetDlgItem(hDlg, IDC_LIST_GAME));
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
 * show_game_schedule - スケジュール設定
 */
BOOL show_game_schedule(const HINSTANCE hInst, const HWND hWnd, int current_set)
{
	return DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_GAME_SCHEDULE), hWnd, game_schedule_proc, current_set);
}
/* End of source */
