/*
 * n01
 *
 * option_plugin.c
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

#include "general.h"
#include "Memory.h"
#include "String.h"
#include "Message.h"
#include "option.h"
#include "option_key.h"

#include "resource.h"

/* Define */

/* Global Variables */
extern HINSTANCE hInst;
extern int prop_ret;

extern OPTION_INFO op;

/* Local Function Prototypes */
static int file_select(const HWND hDlg, const TCHAR *title, const TCHAR *filter, const int index, TCHAR *ret);
static BOOL dll_to_list(const HWND hDlg, const TCHAR *lib_path);
static BOOL list_to_item(const HWND hDlg, const TCHAR *lib_path);
static void show_property(const HWND hDlg, PLUGIN_INFO *pi);
static void listview_set_text(const HWND hListView, const int i);
static int listview_set_plugin(const HWND hListView, PLUGIN_INFO *pi, const BOOL copy);
static void listview_free_plugin(const HWND hListView);
static BOOL CALLBACK set_plugin_item_proc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

/*
 * file_select - ファイル選択ダイアログの表示
 */
static int file_select(const HWND hDlg, const TCHAR *title, const TCHAR *filter, const int index, TCHAR *ret)
{
	OPENFILENAME of;
	TCHAR filename[MAX_PATH];

	*filename = TEXT('\0');

	ZeroMemory(&of, sizeof(OPENFILENAME));
	of.lStructSize = sizeof(OPENFILENAME);
	of.hwndOwner = hDlg;
	of.lpstrFilter = filter;
	of.lpstrTitle = title;
	of.nMaxCustFilter = 40;
	of.nFilterIndex = index;
	of.lpstrFile = filename;
	of.nMaxFile = BUF_SIZE - 1;
	of.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

	if (GetOpenFileName((LPOPENFILENAME)&of) == TRUE) {
		lstrcpy(ret, of.lpstrFile);
		return of.nFilterIndex;
	}
	return -1;
}

/*
 * dll_to_list - DLL内の関数をリストボックスに表示
 */
static BOOL dll_to_list(const HWND hDlg, const TCHAR *lib_path)
{
	HANDLE lib;
	FARPROC func_get_tool_info;
	PLUGIN_GET_INFO pgi;
	TCHAR err_str[BUF_SIZE];
	int i;

	// DLLロード
	if ((lib = LoadLibrary(lib_path)) == NULL) {
		message_get_error(GetLastError(), err_str);
		if (*err_str != TEXT('\0')) {
			MessageBox(hDlg, err_str, lib_path, MB_ICONERROR);
		}
		return FALSE;
	}
	// 関数アドレス取得
	if ((func_get_tool_info = GetProcAddress(lib, "get_plugin_info")) == NULL) {
		message_get_error(GetLastError(), err_str);
		if (*err_str != TEXT('\0')) {
			MessageBox(hDlg, err_str, lib_path, MB_ICONERROR);
		}
		FreeLibrary(lib);
		return FALSE;
	}

	// アイテムの追加
	i = 0;
	while (1) {
		ZeroMemory(&pgi, sizeof(PLUGIN_GET_INFO));
		pgi.struct_size = sizeof(PLUGIN_GET_INFO);
		if (func_get_tool_info(hDlg, i++, &pgi) == FALSE) {
			break;
		}
		SendDlgItemMessage(hDlg, IDC_LIST, LB_ADDSTRING, 0, (LPARAM)pgi.title);
	}
	FreeLibrary(lib);
	return TRUE;
}

/*
 * list_to_item - DLL内の関数をリストビューに表示
 */
static BOOL list_to_item(const HWND hDlg, const TCHAR *lib_path)
{
	HANDLE lib;
	FARPROC func_get_tool_info;
	PLUGIN_GET_INFO pgi;
	PLUGIN_INFO *pi;
	HWND pWnd;
	TCHAR err_str[BUF_SIZE];
	int count;
	int i;

	// DLLロード
	if ((lib = LoadLibrary(lib_path)) == NULL) {
		message_get_error(GetLastError(), err_str);
		if (*err_str != TEXT('\0')) {
			MessageBox(hDlg, err_str, lib_path, MB_ICONERROR);
		}
		return FALSE;
	}
	// 関数アドレス取得
	if ((func_get_tool_info = GetProcAddress(lib, "get_plugin_info")) == NULL) {
		message_get_error(GetLastError(), err_str);
		if (*err_str != TEXT('\0')) {
			MessageBox(hDlg, err_str, lib_path, MB_ICONERROR);
		}
		FreeLibrary(lib);
		return FALSE;
	}

	pWnd = PropSheet_GetCurrentPageHwnd(GetParent(hDlg));
	count = SendDlgItemMessage(hDlg, IDC_LIST, LB_GETCOUNT, 0, 0);
	for (i = 0; i < count; i++) {
		if (SendDlgItemMessage(hDlg, IDC_LIST, LB_GETSEL, i, 0) == 0) {
			continue;
		}
		ZeroMemory(&pgi, sizeof(PLUGIN_GET_INFO));
		pgi.struct_size = sizeof(PLUGIN_GET_INFO);
		func_get_tool_info(hDlg, i, &pgi);

		pi = mem_calloc(sizeof(PLUGIN_INFO));
		if (pi == NULL) {
			message_get_error(GetLastError(), err_str);
			if (*err_str != TEXT('\0')) {
				MessageBox(hDlg, err_str, lib_path, MB_ICONERROR);
			}
			FreeLibrary(lib);
			return FALSE;
		}
		lstrcpy(pi->title, pgi.title);
		lstrcpy(pi->lib_file_path, lib_path);
		lstrcpy(pi->func_name, pgi.func_name);
		lstrcpy(pi->pei.cmd_line, pgi.cmd_line);
		pi->index = -1;
		listview_set_plugin(GetDlgItem(pWnd, IDC_LIST_PLUGIN), pi, FALSE);
	}
	FreeLibrary(lib);
	return TRUE;
}

/*
 * show_property - プロパティ表示
 */
static void show_property(const HWND hDlg, PLUGIN_INFO *pi)
{
	HANDLE lib;
	FARPROC func_property;
	TCHAR err_str[BUF_SIZE];
	TCHAR buf[BUF_SIZE];
	int ret = PLUGIN_ERROR;
#ifdef UNICODE
	char cbuf[BUF_SIZE];
#endif

	lib = pi->lib;
	// DLLロード
	if (lib == NULL && (lib = LoadLibrary(pi->lib_file_path)) == NULL) {
		message_get_error(GetLastError(), err_str);
		if (*err_str != TEXT('\0')) {
			MessageBox(hDlg, err_str, pi->lib_file_path, MB_ICONERROR);
		}
		return;
	}
	// 関数アドレス取得
	wsprintf(buf, TEXT("%s_property"), pi->func_name);
#ifdef UNICODE
	tchar_to_char(buf, cbuf, BUF_SIZE - 1);
	func_property = GetProcAddress(lib, cbuf);
#else
	func_property = GetProcAddress(lib, buf);
#endif
	if (func_property != NULL) {
		// 実行情報の設定
		pi->pei.struct_size = sizeof(PLUGIN_EXEC_INFO);
		pi->pei.si = NULL;
		// プロパティ表示
		ret = func_property(hDlg, &pi->pei);
	}
	if (pi->lib == NULL) {
		FreeLibrary(lib);
	}
	if (ret != PLUGIN_SUCCEED) {
		MessageBox(hDlg, message_get_res(IDS_STRING_OP_PLUGIN_NO_PROP), APP_NAME, MB_OK | MB_ICONEXCLAMATION);
	}
}

/*
 * listview_set_text - ListViewのテキストを設定
 */
static void listview_set_text(const HWND hListView, const int i)
{
	PLUGIN_INFO *pi;
	TCHAR buf[BUF_SIZE];

	if ((pi = (PLUGIN_INFO *)listview_get_lparam(hListView, i)) == NULL) {
		return;
	}
	// タイトル
	ListView_SetItemText(hListView, i, 0, pi->title);
	// パス
	ListView_SetItemText(hListView, i, 1, pi->lib_file_path);
	// 関数名
	ListView_SetItemText(hListView, i, 2, pi->func_name);
	// キー
	*buf = TEXT('\0');
	get_keyname(pi->ctrl, pi->key, buf);
	if (*buf == TEXT('\0')) {
		message_copy_res(IDS_STRING_OP_KEY_NOTHING, buf);
	}
	ListView_SetItemText(hListView, i, 3, buf);
}

/*
 * listview_set_plugin - ListViewにプラグイン設定を追加する
 */
static int listview_set_plugin(const HWND hListView, PLUGIN_INFO *pi, const BOOL copy)
{
	LV_ITEM lvi;
	PLUGIN_INFO *new_pi;
	int i;

	if (*pi->title == TEXT('\0') && *pi->lib_file_path == TEXT('\0') && *pi->func_name == TEXT('\0')) {
		return -1;
	}
	if (copy == TRUE) {
		if ((new_pi = mem_calloc(sizeof(PLUGIN_INFO))) == NULL) {
			return -1;
		}
		CopyMemory(new_pi, pi, sizeof(PLUGIN_INFO));
	} else {
		new_pi = pi;
	}
	lvi.mask = LVIF_TEXT | LVIF_PARAM;
	lvi.iItem = ListView_GetItemCount(hListView);
	lvi.iSubItem = 0;
	lvi.pszText = TEXT("");
	lvi.cchTextMax = BUF_SIZE - 1;
	lvi.iImage = 0;
	lvi.lParam = (LPARAM)new_pi;
	i = ListView_InsertItem(hListView, &lvi);
	listview_set_text(hListView, i);
	return i;
}

/*
 * listview_free_plugin - プラグイン設定の解放
 */
static void listview_free_plugin(const HWND hListView)
{
	PLUGIN_INFO *pi;
	int i;

	for (i = 0; i < ListView_GetItemCount(hListView); i++) {
		if ((pi = (PLUGIN_INFO *)listview_get_lparam(hListView, i)) != NULL) {
			mem_free(&pi);
		}
		listview_set_lparam(hListView, i, 0);
	}
}

/*
 * set_plugin_item_proc - プラグイン設定の追加
 */
static BOOL CALLBACK set_plugin_item_proc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	TCHAR *lib_path;

	switch (uMsg) {
	case WM_INITDIALOG:
		lib_path = (TCHAR *)lParam;
		if (lib_path == NULL || *lib_path == TEXT('\0')) {
			EndDialog(hDlg, FALSE);
			break;
		}
		if (dll_to_list(hDlg, lib_path) == FALSE) {
			EndDialog(hDlg, FALSE);
			break;
		}
		SendDlgItemMessage(hDlg, IDC_LIST, LB_SETSEL, TRUE, -1);

		SetWindowLong(hDlg, GWL_USERDATA, lParam);
		break;

	case WM_CLOSE:
		EndDialog(hDlg, FALSE);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			lib_path = (TCHAR *)GetWindowLong(hDlg, GWL_USERDATA);
			if (list_to_item(hDlg, lib_path) == FALSE) {
				break;
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
 * set_plugin_key_proc - キー設定
 */
static BOOL CALLBACK set_plugin_key_proc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PLUGIN_INFO *pi;
	int i;

	switch (uMsg) {
	case WM_INITDIALOG:
		pi = (PLUGIN_INFO *)lParam;
		if (pi == NULL) {
			EndDialog(hDlg, FALSE);
			break;
		}
		SetWindowLong(hDlg, GWL_USERDATA, lParam);

		i = 0;
		if (pi->ctrl & FSHIFT) {
			i |= HOTKEYF_SHIFT;
		}
		if (pi->ctrl & FCONTROL) {
			i |= HOTKEYF_CONTROL;
		}
		if (pi->ctrl & FALT) {
			i |= HOTKEYF_ALT;
		}
		if (pi->key != 0) {
			SendDlgItemMessage(hDlg, IDC_HOTKEY_KEY, HKM_SETHOTKEY, (WPARAM)MAKEWORD(pi->key, i), 0);
		}
		break;

	case WM_CLOSE:
		EndDialog(hDlg, FALSE);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			pi = (PLUGIN_INFO *)GetWindowLong(hDlg, GWL_USERDATA);
			i = SendDlgItemMessage(hDlg,IDC_HOTKEY_KEY, HKM_GETHOTKEY, 0, 0);
			pi->key = LOBYTE(i);
			i = HIBYTE(i);
			pi->ctrl = ((i & HOTKEYF_SHIFT) ? FSHIFT : 0) |
				((i & HOTKEYF_CONTROL) ? FCONTROL : 0) |
				((i & HOTKEYF_ALT) ? FALT : 0);
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
 * option_plugin_proc - ウィンドウプロシージャ
 */
BOOL CALLBACK option_plugin_proc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LV_COLUMN lvc;
	PLUGIN_INFO *pi;
	TCHAR buf[BUF_SIZE];
	int cnt;
	int i, j;
	BOOL enable;

	switch (msg) {
	case WM_INITDIALOG:
		// D&Dを受け付ける
		SetWindowLong(hDlg, GWL_EXSTYLE, GetWindowLong(hDlg, GWL_EXSTYLE) | WS_EX_ACCEPTFILES);

		i = 0;
		lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;

		lvc.fmt = LVCFMT_LEFT;
		lvc.cx = 120;
		lvc.pszText = message_get_res(IDS_STRING_OP_PLUGIN_TITLE);
		lvc.iSubItem = i++;
		ListView_InsertColumn(GetDlgItem(hDlg, IDC_LIST_PLUGIN), lvc.iSubItem, &lvc);

		lvc.fmt = LVCFMT_LEFT;
		lvc.cx = 70;
		lvc.pszText = message_get_res(IDS_STRING_OP_PLUGIN_DLL);
		lvc.iSubItem = i++;
		ListView_InsertColumn(GetDlgItem(hDlg, IDC_LIST_PLUGIN), lvc.iSubItem, &lvc);

		lvc.fmt = LVCFMT_LEFT;
		lvc.cx = 70;
		lvc.pszText = message_get_res(IDS_STRING_OP_PLUGIN_FUNCTION);
		lvc.iSubItem = i++;
		ListView_InsertColumn(GetDlgItem(hDlg, IDC_LIST_PLUGIN), lvc.iSubItem, &lvc);

		lvc.fmt = LVCFMT_LEFT;
		lvc.cx = 80;
		lvc.pszText = message_get_res(IDS_STRING_OP_KEY_KEY);
		lvc.iSubItem = i++;
		ListView_InsertColumn(GetDlgItem(hDlg, IDC_LIST_PLUGIN), lvc.iSubItem, &lvc);

		SendDlgItemMessage(hDlg, IDC_LIST_PLUGIN, LVM_SETEXTENDEDLISTVIEWSTYLE, 0,
			LVS_EX_ONECLICKACTIVATE | LVS_EX_FULLROWSELECT |
			SendDlgItemMessage(hDlg, IDC_LIST_PLUGIN, LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0));

		for (i = 0; i < op.plugin_info_count; i++) {
			op.plugin_info[i].index = i;
			listview_set_plugin(GetDlgItem(hDlg, IDC_LIST_PLUGIN), &op.plugin_info[i], TRUE);
		}
		ListView_SetItemState(GetDlgItem(hDlg, IDC_LIST_PLUGIN), 0, LVIS_FOCUSED, LVIS_FOCUSED);
		SendMessage(hDlg, WM_LV_EVENT, LVN_ITEMCHANGED, 0);
		break;

	case WM_DESTROY:
		listview_free_plugin(GetDlgItem(hDlg, IDC_LIST_PLUGIN));
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

	case WM_DROPFILES:
		cnt = DragQueryFile((HANDLE)wParam, 0xFFFFFFFF, NULL, 0);
		for (i = 0; i < cnt; i++) {
			DragQueryFile((HANDLE)wParam, i, buf, BUF_SIZE - 1);
			if (DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_SET_PLUGIN), hDlg, set_plugin_item_proc, (LPARAM)buf) == FALSE) {
				break;
			}
		}
		DragFinish((HANDLE)wParam);
		break;

	case WM_NOTIFY:
		if (listview_notify_proc(hDlg, lParam, GetDlgItem(hDlg, IDC_LIST_PLUGIN)) == 0) {
			return option_notify_proc(hDlg, msg, wParam, lParam);
		}
		break;

	case WM_LV_EVENT:
		switch (wParam) {
		case LVN_ITEMCHANGED:
			enable = (ListView_GetSelectedCount(GetDlgItem(hDlg, IDC_LIST_PLUGIN)) <= 0) ? FALSE : TRUE;
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_UP), enable);
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_DOWN), enable);
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_PROPERTY), enable);
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_DELETE), enable);
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_KEY), enable);
			break;
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_BUTTON_UP:
			if ((i = ListView_GetNextItem(GetDlgItem(hDlg, IDC_LIST_PLUGIN), -1, LVNI_SELECTED)) == -1) {
				break;
			}
			if (i == 0) {
				break;
			}
			listview_move_item(GetDlgItem(hDlg, IDC_LIST_PLUGIN), i, -1);
			SetFocus(GetDlgItem(hDlg, LOWORD(wParam)));
			break;

		case IDC_BUTTON_DOWN:
			if ((i = ListView_GetNextItem(GetDlgItem(hDlg, IDC_LIST_PLUGIN), -1, LVNI_SELECTED)) == -1) {
				break;
			}
			if (i == ListView_GetItemCount(GetDlgItem(hDlg, IDC_LIST_PLUGIN)) - 1) {
				break;
			}
			listview_move_item(GetDlgItem(hDlg, IDC_LIST_PLUGIN), i, 1);
			SetFocus(GetDlgItem(hDlg, LOWORD(wParam)));
			break;

		case IDC_BUTTON_ADD:
			// ファイル選択
			if (file_select(hDlg, message_get_res(IDS_STRING_OP_FILESELECT), TEXT("*.n01;*.dll\0*.n01;*.dll\0*.*\0*.*\0\0"), 1, buf) == -1) {
				break;
			}
			DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_SET_PLUGIN), hDlg, set_plugin_item_proc, (LPARAM)buf);
			break;

		case IDC_BUTTON_DELETE:
			if ((i = ListView_GetNextItem(GetDlgItem(hDlg, IDC_LIST_PLUGIN), -1, LVNI_SELECTED)) == -1) {
				break;
			}
			if (MessageBox(hDlg, message_get_res(IDS_STRING_OP_DELETE), APP_NAME, MB_ICONQUESTION | MB_YESNO) == IDNO) {
				break;
			}
			if ((pi = (PLUGIN_INFO *)listview_get_lparam(GetDlgItem(hDlg, IDC_LIST_PLUGIN), i)) != NULL) {
				mem_free(&pi);
			}
			ListView_DeleteItem(GetDlgItem(hDlg, IDC_LIST_PLUGIN), i);
			ListView_SetItemState(GetDlgItem(hDlg, IDC_LIST_PLUGIN), i,
				LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
			break;

		case IDC_BUTTON_PROPERTY:
		case IDC_BUTTON_EDIT:
			if ((i = ListView_GetNextItem(GetDlgItem(hDlg, IDC_LIST_PLUGIN), -1, LVNI_SELECTED)) == -1) {
				break;
			}
			show_property(hDlg, (PLUGIN_INFO *)listview_get_lparam(GetDlgItem(hDlg, IDC_LIST_PLUGIN), i));
			break;

		case IDC_BUTTON_KEY:
			if ((i = ListView_GetNextItem(GetDlgItem(hDlg, IDC_LIST_PLUGIN), -1, LVNI_SELECTED)) == -1) {
				break;
			}
			if ((pi = (PLUGIN_INFO *)listview_get_lparam(GetDlgItem(hDlg, IDC_LIST_PLUGIN), i)) == NULL) {
				break;
			}
			if (DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_SET_PLUGIN_KEY), hDlg, set_plugin_key_proc, (LPARAM)pi) == TRUE) {
				listview_set_text(GetDlgItem(hDlg, IDC_LIST_PLUGIN), i);
			}
			break;

		case IDOK:
			// プラグイン情報から削除されたライブラリの解放
			for (i = 0; i < op.plugin_info_count; i++) {
				for (j = 0; j < ListView_GetItemCount(GetDlgItem(hDlg, IDC_LIST_PLUGIN)); j++) {
					if ((pi = (PLUGIN_INFO *)listview_get_lparam(GetDlgItem(hDlg, IDC_LIST_PLUGIN), j)) == NULL) {
						continue;
					}
					if (pi->index == op.plugin_info[i].index) {
						break;
					}
				}
				if (j >= ListView_GetItemCount(GetDlgItem(hDlg, IDC_LIST_PLUGIN))) {
					FreeLibrary(op.plugin_info[i].lib);
					op.plugin_info[i].lib = NULL;
					op.plugin_info[i].func = NULL;
				}
			}
			// プラグイン情報のコピー
			i = ListView_GetItemCount(GetDlgItem(hDlg, IDC_LIST_PLUGIN));
			pi = (PLUGIN_INFO *)mem_alloc(sizeof(PLUGIN_INFO) * i);
			if (pi == NULL) {
				break;
			}
			mem_free(&op.plugin_info);
			op.plugin_info = pi;
			op.plugin_info_count = i;
			for (i = 0; i < op.plugin_info_count; i++) {
				if ((pi = (PLUGIN_INFO *)listview_get_lparam(GetDlgItem(hDlg, IDC_LIST_PLUGIN), i)) != NULL) {
					CopyMemory(&op.plugin_info[i], pi, sizeof(PLUGIN_INFO));
				}
			}
			listview_free_plugin(GetDlgItem(hDlg, IDC_LIST_PLUGIN));
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
