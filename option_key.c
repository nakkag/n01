/*
 * n01
 *
 * option_key.c
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

#include "general.h"
#include "Memory.h"
#include "String.h"
#include "Message.h"
#include "ini.h"
#include "option.h"
#include "option_key.h"

#include "resource.h"

/* Define */
#define STR_SEPARATE		TEXT("----------------------------------------")

/* Global Variables */
extern HINSTANCE hInst;
extern HACCEL hKeyAccel;
extern int prop_ret;

extern OPTION_INFO op;

/* Local Function Prototypes */
static void listview_set_text(const HWND hListView, const int i);
static int listview_set_key(const HWND hListView, KEY_INFO *ki, const BOOL copy);
static void listview_free_key(const HWND hListView);
static BOOL CALLBACK set_key_item_proc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

/*
 * create_accelerator - アクセラレータの作成
 */
void create_accelerator(void)
{
	ACCEL *ac;
	int cnt = 0;
	int i;

	free_accelerator();

	ac = (ACCEL *)mem_alloc(sizeof(ACCEL) * op.key_info_count);
	if (ac == NULL) {
		return;
	}
	for (i = 0; i < op.key_info_count; i++) {
		if (op.key_info[i].ctrl == 0 && op.key_info[i].key == 0) {
			continue;
		}
		ac[cnt].fVirt = (BYTE)(op.key_info[i].ctrl | FVIRTKEY);
		ac[cnt].key = (WORD)op.key_info[i].key;
		ac[cnt].cmd = (WORD)op.key_info[i].action;
		cnt++;
	}
	if (cnt != 0) {
		hKeyAccel = CreateAcceleratorTable(ac, cnt);
	}
	mem_free(&ac);
}

/*
 * free_accelerator - アクセラレータの解放
 */
void free_accelerator(void)
{
	if (hKeyAccel != NULL) {
		DestroyAcceleratorTable(hKeyAccel);
		hKeyAccel = NULL;
	}
}

/*
 * get_keyname - キー名を取得
 */
BOOL get_keyname(const UINT modifiers, const UINT virtkey, TCHAR *ret)
{
	UINT scan_code;
	int ext_flag = 0;

	if (virtkey == 0 || (scan_code = MapVirtualKey(virtkey, 0)) <= 0) {
		// なし
		return FALSE;
	}
	*ret = TEXT('\0');
	if (modifiers & FCONTROL) {
		lstrcat(ret, TEXT("Ctrl+"));
	}
	if (modifiers & FSHIFT) {
		lstrcat(ret, TEXT("Shift+"));
	}
	if (modifiers & FALT) {
		lstrcat(ret, TEXT("Alt+"));
	}
	if (virtkey == VK_APPS ||
		virtkey == VK_PRIOR ||
		virtkey == VK_NEXT ||
		virtkey == VK_END ||
		virtkey == VK_HOME ||
		virtkey == VK_LEFT ||
		virtkey == VK_UP ||
		virtkey == VK_RIGHT ||
		virtkey == VK_DOWN ||
		virtkey == VK_INSERT ||
		virtkey == VK_DELETE ||
		virtkey == VK_NUMLOCK) {
		ext_flag = 1 << 24;
	}
	GetKeyNameText((scan_code << 16) | ext_flag, ret + lstrlen(ret), BUF_SIZE - lstrlen(ret) - 1);
	return TRUE;
}

/*
 * listview_set_text - ListViewのテキストを設定
 */
static void listview_set_text(const HWND hListView, const int i)
{
	KEY_INFO *ki;
	TCHAR buf[BUF_SIZE];

	if ((ki = (KEY_INFO *)listview_get_lparam(hListView, i)) == NULL) {
		return;
	}
	if (ki->action >= ID_ACCEL_INPUT_SCORE && ki->action <= ID_ACCEL_INPUT_SCORE + 180) {
		wsprintf(buf, TEXT("%s - %d"), message_get_res(IDS_STRING_OP_KEY_NUMBER), ki->action - ID_ACCEL_INPUT_SCORE);
	} else {
		str_noprefix_cpy(buf, message_get_res(ki->action));
	}
	// タイトル
	ListView_SetItemText(hListView, i, 0, buf);

	// キー
	*buf = TEXT('\0');
	get_keyname(ki->ctrl, ki->key, buf);
	if (*buf == TEXT('\0')) {
		message_copy_res(IDS_STRING_OP_KEY_NOTHING, buf);
	}
	ListView_SetItemText(hListView, i, 1, buf);
}

/*
 * listview_set_key - ListViewにキー設定を追加する
 */
static int listview_set_key(const HWND hListView, KEY_INFO *ki, const BOOL copy)
{
	LV_ITEM lvi;
	KEY_INFO *new_ki;
	int i;

	if ((ki->action < ID_ACCEL_INPUT_SCORE || ki->action > ID_ACCEL_INPUT_SCORE + 180) &&
		*message_get_res(ki->action) == TEXT('\0')){
		return -1;
	}

	if (copy == TRUE) {
		if ((new_ki = mem_calloc(sizeof(KEY_INFO))) == NULL) {
			return -1;
		}
		CopyMemory(new_ki, ki, sizeof(KEY_INFO));
	} else {
		new_ki = ki;
	}

	lvi.mask = LVIF_TEXT | LVIF_PARAM;
	lvi.iItem = ListView_GetItemCount(hListView);
	lvi.iSubItem = 0;
	lvi.pszText = TEXT("");
	lvi.cchTextMax = BUF_SIZE - 1;
	lvi.iImage = 0;
	lvi.lParam = (LPARAM)new_ki;
	i = ListView_InsertItem(hListView, &lvi);

	listview_set_text(hListView, i);
	return i;
}

/*
 * listview_free_key - キー設定の解放
 */
static void listview_free_key(const HWND hListView)
{
	KEY_INFO *ki;
	int i;

	for (i = 0; i < ListView_GetItemCount(hListView); i++) {
		if ((ki = (KEY_INFO *)listview_get_lparam(hListView, i)) != NULL) {
			mem_free(&ki);
		}
		listview_set_lparam(hListView, i, 0);
	}
}

/*
 * set_key_item_proc - キー設定の項目を設定
 */
static BOOL CALLBACK set_key_item_proc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	KEY_INFO *ki;
	TCHAR buf[BUF_SIZE];
	int i, j;

	switch (uMsg) {
	case WM_INITDIALOG:
		i = SendDlgItemMessage(hDlg, IDC_COMBO_ACTION, CB_ADDSTRING, 0, (LPARAM)message_get_res(IDS_STRING_OP_KEY_NUMBER));
		SendDlgItemMessage(hDlg, IDC_COMBO_ACTION, CB_SETITEMDATA, i, ID_ACCEL_INPUT_SCORE);

#define SET_COMBO_ITEM(id)	SendDlgItemMessage(hDlg, IDC_COMBO_ACTION, CB_SETITEMDATA, \
							SendDlgItemMessage(hDlg, IDC_COMBO_ACTION, CB_ADDSTRING, 0, (LPARAM)str_noprefix_cpy(buf, message_get_res(id))), id)

		SendDlgItemMessage(hDlg, IDC_COMBO_ACTION, CB_ADDSTRING, 0, (LPARAM)STR_SEPARATE);
		SET_COMBO_ITEM(ID_MENUITEM_NEW_GAME);
		SET_COMBO_ITEM(ID_MENUITEM_NEW_SCHEDULE);
		SET_COMBO_ITEM(ID_MENUITEM_SET_SCHEDULE);
		SET_COMBO_ITEM(ID_MENUITEM_GAME_HISTORY);
		SET_COMBO_ITEM(ID_MENUITEM_OPTION);
		SET_COMBO_ITEM(ID_MENUITEM_AUTO_SAVE);
		SET_COMBO_ITEM(ID_MENUITEM_CHECK_OUT_MODE);
		SET_COMBO_ITEM(ID_MENUITEM_EXIT);
		SendDlgItemMessage(hDlg, IDC_COMBO_ACTION, CB_ADDSTRING, 0, (LPARAM)STR_SEPARATE);
		SET_COMBO_ITEM(ID_MENUITEM_SHOW_NAME);
		SET_COMBO_ITEM(ID_MENUITEM_SHOW_PLAYER);
		SET_COMBO_ITEM(ID_MENUITEM_SHOW_LEFT);
		SET_COMBO_ITEM(ID_MENUITEM_SHOW_GUIDE);
		SET_COMBO_ITEM(ID_MENUITEM_FONT_L);
		SET_COMBO_ITEM(ID_MENUITEM_FONT_M);
		SET_COMBO_ITEM(ID_MENUITEM_FONT_S);
		SET_COMBO_ITEM(ID_MENUITEM_FULL_SCREEN);
		SET_COMBO_ITEM(ID_MENUITEM_SHOW_DARTSCOUNT);
		SendDlgItemMessage(hDlg, IDC_COMBO_ACTION, CB_ADDSTRING, 0, (LPARAM)STR_SEPARATE);
		SET_COMBO_ITEM(ID_MENUITEM_FINISH_ONE);
		SET_COMBO_ITEM(ID_MENUITEM_FINISH_TWO);
		SET_COMBO_ITEM(ID_MENUITEM_FINISH_THREE);
		SET_COMBO_ITEM(ID_MENUITEM_MIDDLE);
		SET_COMBO_ITEM(ID_MENUITEM_SCORE_LEFT);
		SET_COMBO_ITEM(ID_MENUITEM_PREV_LEG);
		SET_COMBO_ITEM(ID_MENUITEM_SCORE_SWAP);
		SET_COMBO_ITEM(ID_MENUITEM_SCORE_CLEAR);
		SET_COMBO_ITEM(ID_MENUITEM_ARRANGE);
		SendDlgItemMessage(hDlg, IDC_COMBO_ACTION, CB_ADDSTRING, 0, (LPARAM)STR_SEPARATE);
		SET_COMBO_ITEM(ID_ACCEL_TAB);
		SET_COMBO_ITEM(ID_ACCEL_STAB);
		SET_COMBO_ITEM(ID_ACCEL_ESC);
		SET_COMBO_ITEM(ID_ACCEL_NONE);
		SendDlgItemMessage(hDlg, IDC_COMBO_ACTION, CB_ADDSTRING, 0, (LPARAM)STR_SEPARATE);
		SET_COMBO_ITEM(IDS_STRING_LEFT);
		SET_COMBO_ITEM(IDS_STRING_RIGHT);
		SET_COMBO_ITEM(ID_ACCEL_UP);
		SET_COMBO_ITEM(ID_ACCEL_DOWN);

		SendDlgItemMessage(hDlg, IDC_SPIN_INPUT, UDM_SETRANGE, 0, (LPARAM)MAKELONG(180, 0));

		if (lParam == 0) {
			// 新規追加
			EnableWindow(GetDlgItem(hDlg, IDC_EDIT_INPUT), FALSE);
			SetWindowLong(hDlg, GWL_USERDATA, 0);
			break;
		}
		ki = (KEY_INFO *)lParam;
		j = ki->action;
		if (ki->action >= ID_ACCEL_INPUT_SCORE && ki->action <= ID_ACCEL_INPUT_SCORE + 180) {
			j = ID_ACCEL_INPUT_SCORE;
			SetDlgItemInt(hDlg, IDC_EDIT_INPUT, ki->action - ID_ACCEL_INPUT_SCORE, FALSE);
		} else {
			EnableWindow(GetDlgItem(hDlg, IDC_EDIT_INPUT), FALSE);
		}
		for (i = 0; i < SendDlgItemMessage(hDlg, IDC_COMBO_ACTION, CB_GETCOUNT, 0, 0); i++) {
			if (j == SendDlgItemMessage(hDlg, IDC_COMBO_ACTION, CB_GETITEMDATA, i, 0)) {
				SendDlgItemMessage(hDlg, IDC_COMBO_ACTION, CB_SETCURSEL, i, 0);
			}
		}

		// キー
		i = 0;
		if (ki->ctrl & FSHIFT) {
			i |= HOTKEYF_SHIFT;
		}
		if (ki->ctrl & FCONTROL) {
			i |= HOTKEYF_CONTROL;
		}
		if (ki->ctrl & FALT) {
			i |= HOTKEYF_ALT;
		}
		if (ki->key != 0) {
			SendDlgItemMessage(hDlg, IDC_HOTKEY_KEY, HKM_SETHOTKEY, (WPARAM)MAKEWORD(ki->key, i), 0);
		}
		SetWindowLong(hDlg, GWL_USERDATA, lParam);
		break;

	case WM_CLOSE:
		EndDialog(hDlg, FALSE);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_COMBO_ACTION:
			if (HIWORD(wParam) == CBN_SELENDOK) {
				i = SendDlgItemMessage(hDlg, IDC_COMBO_ACTION, CB_GETCURSEL, 0, 0);
				if (SendDlgItemMessage(hDlg, IDC_COMBO_ACTION, CB_GETITEMDATA, i, 0) == ID_ACCEL_INPUT_SCORE) {
					EnableWindow(GetDlgItem(hDlg, IDC_EDIT_INPUT), TRUE);
				} else {
					EnableWindow(GetDlgItem(hDlg, IDC_EDIT_INPUT), FALSE);
				}
			}
			break;

		case IDOK:
			if (IsWindowEnabled(GetDlgItem(hDlg, IDC_EDIT_INPUT)) == TRUE) {
				j = GetDlgItemInt(hDlg, IDC_EDIT_INPUT, NULL, FALSE);
				if (j < 0 || j > 180) {
					MessageBox(hDlg, message_get_res(IDS_STRING_OP_KEY_NUMBER_ERROR), APP_NAME, MB_ICONEXCLAMATION);
					SetFocus(GetDlgItem(hDlg, IDC_EDIT_INPUT));
					break;
				}
			}
			i = SendDlgItemMessage(hDlg, IDC_COMBO_ACTION, CB_GETCURSEL, 0, 0);
			if (i == -1) {
				SetFocus(GetDlgItem(hDlg, IDC_COMBO_ACTION));
				break;
			}
			if (SendDlgItemMessage(hDlg, IDC_COMBO_ACTION, CB_GETITEMDATA, i, 0) == 0) {
				SetFocus(GetDlgItem(hDlg, IDC_COMBO_ACTION));
				break;
			}
			if ((ki = (KEY_INFO *)GetWindowLong(hDlg, GWL_USERDATA)) == NULL) {
				ki = mem_calloc(sizeof(KEY_INFO));
			}
			if (ki != NULL) {
				// 設定取得
				i = SendDlgItemMessage(hDlg, IDC_COMBO_ACTION, CB_GETCURSEL, 0, 0);
				ki->action = SendDlgItemMessage(hDlg, IDC_COMBO_ACTION, CB_GETITEMDATA, i, 0);
				if (ki->action == ID_ACCEL_INPUT_SCORE) {
					ki->action += GetDlgItemInt(hDlg, IDC_EDIT_INPUT, NULL, FALSE);
				}

				// キー
				i = SendDlgItemMessage(hDlg,IDC_HOTKEY_KEY, HKM_GETHOTKEY, 0, 0);
				ki->key = LOBYTE(i);
				i = HIBYTE(i);
				ki->ctrl = ((i & HOTKEYF_SHIFT) ? FSHIFT : 0) |
					((i & HOTKEYF_CONTROL) ? FCONTROL : 0) |
					((i & HOTKEYF_ALT) ? FALT : 0);
			}

			if (GetWindowLong(hDlg, GWL_USERDATA) == 0) {
				// 新規
				HWND pWnd = PropSheet_GetCurrentPageHwnd(GetParent(hDlg));
				i = listview_set_key(GetDlgItem(pWnd, IDC_LIST_KEY), ki, FALSE);
				ListView_SetItemState(GetDlgItem(pWnd, IDC_LIST_KEY), i,
					LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
				ListView_EnsureVisible(GetDlgItem(pWnd, IDC_LIST_KEY), i, TRUE);
			}
			EndDialog(hDlg, TRUE);
			break;

		case IDCANCEL:
			EndDialog(hDlg, FALSE);
			break;
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

/*
 * option_key_proc - ウィンドウプロシージャ
 */
BOOL CALLBACK option_key_proc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LV_COLUMN lvc;
	KEY_INFO *ki;
	int i;
	BOOL enable;

	switch (msg) {
	case WM_INITDIALOG:
		i = 0;
		lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;

		lvc.fmt = LVCFMT_LEFT;
		lvc.cx = 200;
		lvc.pszText = message_get_res(IDS_STRING_OP_KEY_ACTION);
		lvc.iSubItem = i++;
		ListView_InsertColumn(GetDlgItem(hDlg, IDC_LIST_KEY), lvc.iSubItem, &lvc);

		lvc.fmt = LVCFMT_LEFT;
		lvc.cx = 100;
		lvc.pszText = message_get_res(IDS_STRING_OP_KEY_KEY);
		lvc.iSubItem = i++;
		ListView_InsertColumn(GetDlgItem(hDlg, IDC_LIST_KEY), lvc.iSubItem, &lvc);

		SendDlgItemMessage(hDlg, IDC_LIST_KEY, LVM_SETEXTENDEDLISTVIEWSTYLE, 0,
			LVS_EX_ONECLICKACTIVATE | LVS_EX_FULLROWSELECT |
			SendDlgItemMessage(hDlg, IDC_LIST_KEY, LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0));

		for (i = 0; i < op.key_info_count; i++) {
			listview_set_key(GetDlgItem(hDlg, IDC_LIST_KEY), &op.key_info[i], TRUE);
		}
		ListView_SetItemState(GetDlgItem(hDlg, IDC_LIST_KEY), 0, LVIS_FOCUSED, LVIS_FOCUSED);
		SendMessage(hDlg, WM_LV_EVENT, LVN_ITEMCHANGED, 0);
		break;

	case WM_DESTROY:
		listview_free_key(GetDlgItem(hDlg, IDC_LIST_KEY));
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
		if (listview_notify_proc(hDlg, lParam, GetDlgItem(hDlg, IDC_LIST_KEY)) == 0) {
			return option_notify_proc(hDlg, msg, wParam, lParam);
		}
		break;

	case WM_LV_EVENT:
		switch (wParam) {
		case LVN_ITEMCHANGED:
			enable = (ListView_GetSelectedCount(GetDlgItem(hDlg, IDC_LIST_KEY)) <= 0) ? FALSE : TRUE;
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_UP), enable);
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_DOWN), enable);
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_EDIT), enable);
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_DELETE), enable);
			break;
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_BUTTON_UP:
			if ((i = ListView_GetNextItem(GetDlgItem(hDlg, IDC_LIST_KEY), -1, LVNI_SELECTED)) == -1) {
				break;
			}
			if (i == 0) {
				break;
			}
			listview_move_item(GetDlgItem(hDlg, IDC_LIST_KEY), i, -1);
			op.key_save = TRUE;
			SetFocus(GetDlgItem(hDlg, LOWORD(wParam)));
			break;

		case IDC_BUTTON_DOWN:
			if ((i = ListView_GetNextItem(GetDlgItem(hDlg, IDC_LIST_KEY), -1, LVNI_SELECTED)) == -1) {
				break;
			}
			if (i == ListView_GetItemCount(GetDlgItem(hDlg, IDC_LIST_KEY)) - 1) {
				break;
			}
			listview_move_item(GetDlgItem(hDlg, IDC_LIST_KEY), i, 1);
			op.key_save = TRUE;
			SetFocus(GetDlgItem(hDlg, LOWORD(wParam)));
			break;

		case IDC_BUTTON_ADD:
			if (DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_SET_KEY), hDlg, set_key_item_proc, 0) == TRUE) {
				op.key_save = TRUE;
			}
			break;

		case IDC_BUTTON_EDIT:
			if ((i = ListView_GetNextItem(GetDlgItem(hDlg, IDC_LIST_KEY), -1, LVNI_SELECTED)) == -1) {
				break;
			}
			if (DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_SET_KEY), hDlg, set_key_item_proc,
				listview_get_lparam(GetDlgItem(hDlg, IDC_LIST_KEY), i)) == TRUE) {
				op.key_save = TRUE;
				listview_set_text(GetDlgItem(hDlg, IDC_LIST_KEY), i);
			}
			break;

		case IDC_BUTTON_DELETE:
			if ((i = ListView_GetNextItem(GetDlgItem(hDlg, IDC_LIST_KEY), -1, LVNI_SELECTED)) == -1) {
				break;
			}
			if (MessageBox(hDlg, message_get_res(IDS_STRING_OP_DELETE), APP_NAME, MB_ICONQUESTION | MB_YESNO) == IDNO) {
				break;
			}
			op.key_save = TRUE;
			if ((ki = (KEY_INFO *)listview_get_lparam(GetDlgItem(hDlg, IDC_LIST_KEY), i)) != NULL) {
				mem_free(&ki);
			}
			ListView_DeleteItem(GetDlgItem(hDlg, IDC_LIST_KEY), i);
			ListView_SetItemState(GetDlgItem(hDlg, IDC_LIST_KEY), i,
				LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
			break;

		case IDOK:
			i = ListView_GetItemCount(GetDlgItem(hDlg, IDC_LIST_KEY));
			ki = (KEY_INFO *)mem_alloc(sizeof(KEY_INFO) * i);
			if (ki == NULL) {
				break;
			}
			mem_free(&op.key_info);
			op.key_info = ki;
			op.key_info_count = i;
			for (i = 0; i < op.key_info_count; i++) {
				if ((ki = (KEY_INFO *)listview_get_lparam(GetDlgItem(hDlg, IDC_LIST_KEY), i)) != NULL) {
					CopyMemory(&op.key_info[i], ki, sizeof(KEY_INFO));
				}
			}
			listview_free_key(GetDlgItem(hDlg, IDC_LIST_KEY));
			prop_ret = 1;
			break;

		case IDPCANCEL:
			break;
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;
}
/* End of source */
