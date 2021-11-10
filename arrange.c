/*
 * n01
 *
 * arrange.c
 *
 * Copyright (C) 1996-2008 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

/* Include Files */
#define _INC_OLE
#include <windows.h>
#undef  _INC_OLE
#include <tchar.h>
#include <commctrl.h>
#include <imm.h>

#include "General.h"
#include "Memory.h"
#include "String.h"
#include "Message.h"
#include "file.h"
#include "font.h"
#include "arrange.h"

#include "resource.h"

/* Define */

/* Global Variables */
extern OPTION_INFO op;

static HTREEITEM root_item;

/* Local Function Prototypes */
static ARRANGE_INFO *string_to_arrange(TCHAR *buf, int *ret_count);
static int string_to_score(TCHAR *buf);
static HTREEITEM treeview_set_item(const HWND hTreeView, TCHAR *buf, const HTREEITEM hParent, const HTREEITEM After, LPARAM lParam);
static LPARAM treeview_get_lparam(const HWND hTreeView, const HTREEITEM hItem);
static HTREEITEM treeview_lparam_to_item(const HWND hTreeView, const HTREEITEM hParent, const LPARAM lParam);
static void set_tree_arrange(const HWND hTreeView, const HTREEITEM hParent, const int left, const ignore);
static void get_control_pos(const HWND hDlg, RECT *c_rect);
static BOOL CALLBACK arrange_proc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

/*
 * arrange_check - アレンジのチェック
 */
#ifdef _DEBUG
void arrange_check(void)
{
	TCHAR buf[BUF_SIZE];
	TCHAR *p;
	int score;
	int i, j;

	for (i = 0; i < op.arrange_info_count; i++) {
		score = op.arrange_info[i].left;
		for (j = 0; j < 3; j++) {
			for (p = op.arrange_info[i].throw_list[j]; *p != TEXT('\0') && (*p < TEXT('0') || *p > TEXT('9')); p++);
			switch (*op.arrange_info[i].throw_list[j]) {
			case TEXT('T'):
				score -= _ttoi(p) * 3;
				break;
			case TEXT('D'):
				score -= _ttoi(p) * 2;
				break;
			case TEXT('S'):
				score -= _ttoi(p);
				break;
			default:
				score -= _ttoi(p);
				break;
			}
		}
		if (score != 0) {
			wsprintf(buf, TEXT("(%d) %s-%s-%s = %d"),
				op.arrange_info[i].left,
				op.arrange_info[i].throw_list[0],
				op.arrange_info[i].throw_list[1],
				op.arrange_info[i].throw_list[2],
				score);
			if (MessageBox(NULL, buf, APP_NAME, MB_ICONEXCLAMATION | MB_OKCANCEL) == IDCANCEL) {
				break;
			}
		}
	}
}
#endif

/*
 * free_arrange - アレンジの解放
 */
void free_arrange(ARRANGE_INFO **arrange_info, int *arrange_info_count)
{
	if (*arrange_info != NULL) {
		mem_free(arrange_info);
	}
	*arrange_info_count = 0;
}

/*
 * string_to_arrange - 文字列からアレンジを取得
 */
static ARRANGE_INFO *string_to_arrange(TCHAR *buf, int *ret_count)
{
	ARRANGE_INFO *arrange_info;
	TCHAR tmp[BUF_SIZE];
	TCHAR *p = buf, *r;
	int i = 0, j;
	int cnt;

	//行数のカウント
	*ret_count = 0;
	for (p = buf; *p != TEXT('\0'); p++) {
		if (*p == TEXT('\n')) {
			(*ret_count)++;
		}
	}
	arrange_info = (ARRANGE_INFO *)mem_calloc(sizeof(ARRANGE_INFO) * *ret_count);

	p = buf;
	while (*p != TEXT('\0')) {
		for (; *p == TEXT('\r') || *p == TEXT('\n') || *p == TEXT(' ') || *p == TEXT('\t'); p++);
		if (*p == TEXT('\0')) {
			break;
		}
		if (*p == TEXT('#')) {
			// コメント
			for (; *p != TEXT('\r') && *p != TEXT('\n') && *p != TEXT('\0'); p++);
			continue;
		}
		// left
		for (r = tmp; *p >= TEXT('0') && *p <= TEXT('9'); p++) {
			*(r++) = *p;
		}
		*r = TEXT('\0');
		arrange_info[i].left = _ttoi(tmp);

		for (; *p != TEXT(',') && *p != TEXT('\r') && *p != TEXT('\n') && *p != TEXT('\0'); p++);
		// throw_list
		for (j = 0; j < 5 && *p == TEXT(','); j++) {
			for (p++; *p == TEXT(' ') || *p == TEXT('\t'); p++);
			for (r = arrange_info[i].throw_list[j], cnt = 0; cnt < THROW_LIST_SIZE && *p != TEXT(' ') && *p != TEXT('\t') && *p != TEXT(',') && *p != TEXT('\r') && *p != TEXT('\n') && *p != TEXT('\0'); p++) {
				*(r++) = *p;
				cnt++;
			}
			*r = TEXT('\0');
			for (; *p != TEXT(',') && *p != TEXT('\r') && *p != TEXT('\n') && *p != TEXT('\0'); p++);
			if (*p != TEXT(',')) {
				break;
			}
		}
		for (; *p != TEXT('\r') && *p != TEXT('\n') && *p != TEXT('\0'); p++);
		i++;
	}
	*ret_count = i;
	return arrange_info;
}

/*
 * file_read_arrange - ファイルからアレンジを取得
 */
ARRANGE_INFO *file_read_arrange(const TCHAR *path, int *arrange_info_count)
{
	ARRANGE_INFO *arrange_info;
	TCHAR err_str[BUF_SIZE];
	BYTE *buf;
	DWORD size;
#ifdef UNICODE
	TCHAR *wbuf;
#endif

	// ファイルの読み込み
	if ((buf = file_read_buf(path, &size, err_str)) == NULL) {
		return NULL;
	}
	*(buf + size) = '\0';
#ifdef UNICODE
	wbuf = mem_alloc(sizeof(TCHAR) * (size + 1));
	if (wbuf == NULL) {
		return NULL;
	}
	char_to_tchar((LPCSTR)buf, wbuf, size);
	mem_free(&buf);
	arrange_info = string_to_arrange(wbuf, arrange_info_count);
	mem_free(&wbuf);
#else
	arrange_info = string_to_arrange((TCHAR *)buf, arrange_info_count);
	mem_free(&buf);
#endif
	return arrange_info;
}

/*
 * res_to_arrange - リソースからアレンジを取得
 */
ARRANGE_INFO *res_to_arrange(const HINSTANCE hInst, const UINT res_id, int *arrange_info_count)
{
	ARRANGE_INFO *arrange_info;
	HRSRC hRsrc;
	HGLOBAL hMem;
	LPVOID data;
#ifdef UNICODE
	TCHAR *wbuf;
#endif

	hRsrc = FindResource(hInst, MAKEINTRESOURCE(res_id), TEXT("TEXT"));
	if(hRsrc == NULL){
		return NULL;
	}
	hMem = LoadResource(hInst, hRsrc);
	if(hMem == NULL){
		return NULL;
	}
	data = LockResource(hMem);
	if(data == NULL){
		return NULL;
	}
#ifdef UNICODE
	wbuf = mem_alloc(sizeof(TCHAR) * (char_to_tchar_size(data) + 1));
	if (wbuf == NULL) {
		return NULL;
	}
	char_to_tchar(data, wbuf, char_to_tchar_size(data));
	arrange_info = string_to_arrange(wbuf, arrange_info_count);
	mem_free(&wbuf);
	return arrange_info;
#else
	arrange_info = string_to_arrange(data, arrange_info_count);
	return arrange_info;
#endif
}

/*
 * string_to_score - スコア文字列をスコアに変換
 */
static int string_to_score(TCHAR *buf)
{
	TCHAR *p;
	int ret = 0;

	for (p = buf; *p != TEXT('\0') && (*p < TEXT('0') || *p > TEXT('9')); p++);
	switch (*buf) {
	case TEXT('T'):
		ret = _ttoi(p) * 3;
		break;
	case TEXT('D'):
		ret = _ttoi(p) * 2;
		break;
	case TEXT('S'):
	default:
		ret = _ttoi(p);
		break;
	}
	return ret;
}

/*
 * treeview_set_item - ツリービューアイテムの追加
 */
static HTREEITEM treeview_set_item(const HWND hTreeView, TCHAR *buf, const HTREEITEM hParent, const HTREEITEM After, LPARAM lParam)
{
	TV_INSERTSTRUCT tvitn;
	TV_ITEM tvit;

	ZeroMemory(&tvit, sizeof(TV_ITEM));
	tvit.mask = TVIF_TEXT | TVIF_PARAM;
	tvit.lParam = lParam;
	tvit.pszText = buf;
	tvit.cchTextMax = BUF_SIZE - 1;

	ZeroMemory(&tvitn, sizeof(TV_INSERTSTRUCT));
	tvitn.hInsertAfter = After;
	tvitn.hParent = hParent;
	tvitn.item = tvit;
	return TreeView_InsertItem(hTreeView, &tvitn);
}

/*
 * treeview_get_lparam - アイテムに関連付けられた情報の取得
 */
static LPARAM treeview_get_lparam(const HWND hTreeView, const HTREEITEM hItem)
{
	TV_ITEM tvit;

	if (hItem == NULL) {
		return 0;
	}
	ZeroMemory(&tvit, sizeof(TV_ITEM));
	tvit.mask = TVIF_PARAM;
	tvit.hItem = hItem;
	tvit.lParam = 0;
	TreeView_GetItem(hTreeView, &tvit);
	return tvit.lParam;
}

/*
 * treeview_lparam_to_item - LPARAMからアイテムを検索
 */
static HTREEITEM treeview_lparam_to_item(const HWND hTreeView, const HTREEITEM hParent, const LPARAM lParam)
{
	HTREEITEM hItem;

	if (hParent == NULL || root_item == NULL) {
		return NULL;
	}
	if (hParent == TVI_ROOT) {
		hItem = root_item;
	} else {
		hItem = TreeView_GetChild(hTreeView, hParent);
	}
	while (hItem != NULL) {
		if (string_to_score(op.arrange_info[treeview_get_lparam(hTreeView, hItem)].throw_list[0]) == lParam) {
			return hItem;
		}
		hItem = TreeView_GetNextSibling(hTreeView, hItem);
	}
	return NULL;
}

/*
 * set_tree_arrange - Treeにアレンジを追加
 */
static void set_tree_arrange(const HWND hTreeView, const HTREEITEM hParent, const int left, const ignore)
{
	HTREEITEM wk_item;
	TCHAR buf[BUF_SIZE];
	int i, j, t;
	int l;

	if (left == 0) {
		return;
	}
	for (i = 0; i < op.arrange_info_count; i++) {
		if (op.arrange_info[i].left == left) {
			if (ignore == string_to_score(op.arrange_info[i].throw_list[0])) {
				continue;
			}
			if (treeview_lparam_to_item(hTreeView, hParent, string_to_score(op.arrange_info[i].throw_list[0])) != NULL) {
				continue;
			}
			*buf = TEXT('\0');
			for (j = 0; j < 3; j++) {
				if (*op.arrange_info[i].throw_list[j] == TEXT('\0')) {
					continue;
				}
				if (*buf != TEXT('\0')) {
					lstrcat(buf, TEXT(" - "));
				}
				lstrcat(buf, op.arrange_info[i].throw_list[j]);
				for (t = 0; t < 3 - lstrlen(op.arrange_info[i].throw_list[j]); t++) {
					lstrcat(buf, TEXT(" "));
				}
			}
			wk_item = treeview_set_item(hTreeView, buf, hParent, TVI_LAST, i);
			if (root_item == NULL) {
				root_item = wk_item;
			}
			l = string_to_score(op.arrange_info[i].throw_list[0]);
			if (l != 0 && left - l > 0 && *op.arrange_info[i].throw_list[2] != TEXT('\0')) {
				set_tree_arrange(hTreeView, wk_item, left - l, string_to_score(op.arrange_info[i].throw_list[1]));
			}
			if (hParent == TVI_ROOT) {
				TreeView_Expand(hTreeView, wk_item, TVE_EXPAND);
			}
		}
	}
}

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
 * arrange_proc - ウィンドウプロシージャ
 */
static BOOL CALLBACK arrange_proc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HFONT hfont;
	static HWND hToolTip;
	HDC hdc;
	HFONT ret_font;
	TEXTMETRIC tm;
	RECT parent_rect, rect;
	POINT pt;
	int left, top;
	static RECT tree_rect, cancel_rect;

	switch (msg) {
	case WM_INITDIALOG:
		// コントロールの初期座標を取得
		GetClientRect(hDlg, &rect);
		GetWindowRect(GetDlgItem(hDlg, IDC_TREE_ARRANGE), &tree_rect);
		pt.x = 0;
		pt.y = tree_rect.top;
		ScreenToClient(hDlg, &pt);
		tree_rect.top = pt.y;
		pt.x = 0;
		pt.y = tree_rect.bottom;
		ScreenToClient(hDlg, &pt);
		tree_rect.bottom = rect.bottom - pt.y + tree_rect.top;

		GetWindowRect(GetDlgItem(hDlg, IDCANCEL), &cancel_rect);
		get_control_pos(hDlg, &cancel_rect);

		if (op.arrange_rect.left != 0 && op.arrange_rect.top &&
			op.arrange_rect.right != 0 && op.arrange_rect.bottom != 0) {
			// ウィンドウの位置、サイズ設定
			SetWindowPos(hDlg, 0,
				op.arrange_rect.left, op.arrange_rect.top,
				op.arrange_rect.right, op.arrange_rect.bottom, SWP_NOZORDER);
		} else {
			// センタリング
			GetWindowRect(GetParent(hDlg), &parent_rect);
			GetWindowRect(hDlg, &rect);
			left = parent_rect.left + ((parent_rect.right - parent_rect.left) - (rect.right - rect.left)) / 2;
			if (left < 0) left = 0;
			top = parent_rect.top + ((parent_rect.bottom - parent_rect.top) - (rect.bottom - rect.top)) / 2;
			if (top < 0) top = 0;
			SetWindowPos(hDlg, 0, left, top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
		}

		// スピンコントロールの設定
		SendDlgItemMessage(hDlg, IDC_SPIN_LEFT, UDM_SETRANGE, 0, (LPARAM)MAKELONG(180, 1));

		hfont = font_create(message_get_res(IDS_STRING_ARRANGE_FONT), 12, 0, FALSE, TRUE);
		SendDlgItemMessage(hDlg, IDC_EDIT_LEFT, WM_SETFONT, (WPARAM)hfont, 0);
		SendDlgItemMessage(hDlg, IDC_TREE_ARRANGE, WM_SETFONT, (WPARAM)hfont, 0);

		// Treeの初期化
		hdc = GetDC(hDlg);
		ret_font = SelectObject(hdc, hfont);
		GetTextMetrics(hdc, &tm);
		SelectObject(hdc, ret_font);
		ReleaseDC(hDlg, hdc);
		TreeView_SetIndent(GetDlgItem(hDlg, IDC_TREE_ARRANGE), tm.tmAveCharWidth * 6);
		root_item = NULL;
		set_tree_arrange(GetDlgItem(hDlg, IDC_TREE_ARRANGE), (HTREEITEM)TVI_ROOT, lParam, 0);

		// Editの初期化
		SendDlgItemMessage(hDlg, IDC_EDIT_LEFT, EM_LIMITTEXT, 3, 0);
		SetDlgItemInt(hDlg, IDC_EDIT_LEFT, lParam, FALSE);
		ImmAssociateContext(GetDlgItem(hDlg, IDC_EDIT_LEFT), (HIMC)NULL);
		break;

	case WM_CLOSE:
		DeleteObject(hfont);
		EndDialog(hDlg, FALSE);
		break;

	case WM_SIZE:
		GetClientRect(hDlg, &rect);

		MoveWindow(GetDlgItem(hDlg, IDC_TREE_ARRANGE), 0, tree_rect.top, rect.right, rect.bottom - tree_rect.bottom, TRUE);
		MoveWindow(GetDlgItem(hDlg, IDCANCEL), rect.right - cancel_rect.left, rect.bottom - cancel_rect.top, cancel_rect.right, cancel_rect.bottom, TRUE);

		InvalidateRect(GetDlgItem(hDlg, IDCANCEL), NULL, FALSE);
		UpdateWindow(GetDlgItem(hDlg, IDCANCEL));
		break;

	case WM_EXITSIZEMOVE:
		// サイズ変更完了
		if (IsWindowVisible(hDlg) != 0 && IsIconic(hDlg) == 0 && IsZoomed(hDlg) == 0) {
			GetWindowRect(hDlg, (LPRECT)&op.arrange_rect);
			op.arrange_rect.right -= op.arrange_rect.left;
			op.arrange_rect.bottom -= op.arrange_rect.top;
		}
		break;

	case WM_NOTIFY:
		if (((NMHDR *)lParam)->hwndFrom == GetDlgItem(hDlg, IDC_TREE_ARRANGE)) {
			NMHDR *nmhdr = (NMHDR *)lParam;
			switch (nmhdr->code) {
			case NM_RCLICK:
				{
					HTREEITEM sel_item;
					HMENU hMenu;
					POINT apos;
					TCHAR buf[BUF_SIZE];
					int i;

					if ((sel_item = (HTREEITEM)SendMessage(GetDlgItem(hDlg, IDC_TREE_ARRANGE), TVM_GETNEXTITEM, TVGN_DROPHILITE, 0)) == NULL) {
						sel_item = TreeView_GetSelection(GetDlgItem(hDlg, IDC_TREE_ARRANGE));
					}
					if (sel_item == NULL) {
						break;
					}
					i = treeview_get_lparam(GetDlgItem(hDlg, IDC_TREE_ARRANGE), sel_item);
					wsprintf(buf, TEXT("&%d"), op.arrange_info[i].left);

					// メニューの作成
					hMenu = CreatePopupMenu();
					AppendMenu(hMenu, MF_STRING, 1, buf);
					// メニューの表示
					GetCursorPos((LPPOINT)&apos);
					if (TrackPopupMenu(hMenu, TPM_TOPALIGN | TPM_RETURNCMD, apos.x, apos.y, 0, hDlg, NULL) == 1) {
						SetDlgItemInt(hDlg, IDC_EDIT_LEFT, op.arrange_info[i].left, FALSE);
					}
					DestroyMenu(hMenu);
				}
				break;
			}
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_EDIT_LEFT:
			if (HIWORD(wParam) == EN_CHANGE) {
				SendMessage(GetDlgItem(hDlg, IDC_TREE_ARRANGE), WM_SETREDRAW, (WPARAM)FALSE, 0);

				TreeView_DeleteAllItems(GetDlgItem(hDlg, IDC_TREE_ARRANGE));
				root_item = NULL;
				set_tree_arrange(GetDlgItem(hDlg, IDC_TREE_ARRANGE), (HTREEITEM)TVI_ROOT, GetDlgItemInt(hDlg, IDC_EDIT_LEFT, NULL, FALSE), 0);
				if (root_item != NULL) {
					TreeView_EnsureVisible(GetDlgItem(hDlg, IDC_TREE_ARRANGE), root_item);
				}
				SendMessage(GetDlgItem(hDlg, IDC_TREE_ARRANGE), WM_SETREDRAW, (WPARAM)TRUE, 0);
				UpdateWindow(GetDlgItem(hDlg, IDC_TREE_ARRANGE));
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
 * show_arrange - アレンジの表示
 */
BOOL show_arrange(const HINSTANCE hInst, const HWND hWnd, int left)
{
	return DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_ARRANGE), hWnd, arrange_proc, (LPARAM)left);
}
/* End of source */
