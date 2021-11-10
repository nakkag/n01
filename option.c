/*
 * n01
 *
 * option.c
 *
 * Copyright (C) 1996-2005 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

/* Include Files */
#define _INC_OLE
#include <windows.h>
#undef  _INC_OLE
#include <commctrl.h>

#include "General.h"
#include "String.h"
#include "Message.h"
#include "option.h"
#include "option_view.h"
#include "option_key.h"
#include "option_player.h"
#include "option_plugin.h"

#include "resource.h"

/* Define */

/* Global Variables */
extern HINSTANCE hInst;

int prop_ret;

/* Local Function Prototypes */

/*
 * draw_scroll_control - スクロールバーのボタンの描画
 */
void draw_scroll_control(LPDRAWITEMSTRUCT lpDrawItem, UINT i)
{
#define FOCUSRECT_SIZE			3
	if (lpDrawItem->itemState & ODS_DISABLED) {
		// 使用不能
		i |= DFCS_INACTIVE;
	}
	if (lpDrawItem->itemState & ODS_SELECTED) {
		// 選択
		i |= DFCS_PUSHED;
	}

	// フレームコントロールの描画
	DrawFrameControl(lpDrawItem->hDC, &(lpDrawItem->rcItem), DFC_SCROLL, i);

	// フォーカス
	if (lpDrawItem->itemState & ODS_FOCUS) {
		lpDrawItem->rcItem.left += FOCUSRECT_SIZE;
		lpDrawItem->rcItem.top += FOCUSRECT_SIZE;
		lpDrawItem->rcItem.right -= FOCUSRECT_SIZE;
		lpDrawItem->rcItem.bottom -= FOCUSRECT_SIZE;
		DrawFocusRect(lpDrawItem->hDC, &(lpDrawItem->rcItem));
	}
}

/*
 * listview_notify_proc - リストビューメッセージ
 */
LRESULT listview_notify_proc(const HWND hWnd, const LPARAM lParam, const HWND hListView)
{
	NMHDR *CForm = (NMHDR *)lParam;
	LV_KEYDOWN *LKey = (LV_KEYDOWN *)lParam;
	LV_DISPINFO *plv = (LV_DISPINFO *)lParam;

	if (CForm->hwndFrom != hListView) {
		return 0;
	}

	switch (plv->hdr.code) {
	case LVN_ITEMCHANGED:		// アイテムの選択状態の変更
	case NM_CUSTOMDRAW:
		return SendMessage(hWnd, WM_LV_EVENT, plv->hdr.code, lParam);
	}

	switch (CForm->code) {
	case NM_DBLCLK:				// ダブルクリック
		SendMessage(hWnd, WM_COMMAND, IDC_BUTTON_EDIT, 0);
		return 1;
	}

	switch (LKey->hdr.code) {
	case LVN_KEYDOWN:			// キーダウン
		if (LKey->wVKey == VK_DELETE) {
			SendMessage(hWnd, WM_COMMAND, IDC_BUTTON_DELETE, 0);
			return 1;
		}
	}
	return 0;
}

/*
 * listview_set_lparam - アイテムのLPARAMを設定
 */
BOOL listview_set_lparam(const HWND hListView, const int i, const LPARAM lParam)
{
	LV_ITEM lvi;

	ZeroMemory(&lvi, sizeof(LVITEM));
	lvi.mask = LVIF_PARAM;
	lvi.iItem = i;
	lvi.lParam = lParam;
	return ListView_SetItem(hListView, &lvi);
}

/*
 * listview_get_lparam - アイテムのLPARAMを取得
 */
LPARAM listview_get_lparam(const HWND hListView, const int i)
{
	LV_ITEM lvi;

	ZeroMemory(&lvi, sizeof(LVITEM));
	lvi.mask = LVIF_PARAM;
	lvi.iItem = i;
	ListView_GetItem(hListView, &lvi);
	return lvi.lParam;
}

/*
 * listview_move_item - リストビューのアイテムを移動
 */
void listview_move_item(const HWND hListView, int index, const int Move)
{
	HWND header_wnd;
	LV_ITEM lvi;
	TCHAR buf[100][BUF_SIZE];
	int column_cnt;
	int i = 0;
	LPARAM lp;

	// LPARAMの取得
	lp = listview_get_lparam(hListView, index);
	// ヘッダの取得
	if ((header_wnd = ListView_GetHeader(hListView)) == NULL) {
		header_wnd = GetWindow(hListView, GW_CHILD);
	}
	// テキストの取得
	column_cnt = Header_GetItemCount(header_wnd);
	for (i = 0; i < column_cnt; i++) {
		*(*(buf + i)) = TEXT('\0');
		ListView_GetItemText(hListView, index, i, *(buf + i), BUF_SIZE - 1);
	}
	// アイテムの削除
	ListView_DeleteItem(hListView, index);

	index += Move;

	// 移動先にアイテムを追加
	lvi.mask = LVIF_TEXT | LVIF_PARAM;
	lvi.iItem = index;
	lvi.iSubItem = 0;
	lvi.pszText = *buf;
	lvi.cchTextMax = BUF_SIZE - 1;
	lvi.lParam = lp;
	i = ListView_InsertItem(hListView, &lvi);
	for (i = 1; i < column_cnt; i++) {
		ListView_SetItemText(hListView, index, i, *(buf + i));
	}
	ListView_SetItemState(hListView, index,
		LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
	ListView_EnsureVisible(hListView, index, TRUE);
}

/*
 * option_notify_proc - プロパティシートのイベントの通知
 */
LRESULT option_notify_proc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PSHNOTIFY *pshn = (PSHNOTIFY FAR *)lParam;
	NMHDR *lpnmhdr = (NMHDR FAR *)&pshn->hdr;

	switch (lpnmhdr->code) {
	case PSN_APPLY:				// OK
		SendMessage(hDlg, WM_COMMAND, IDOK, 0);
		break;

	case PSN_QUERYCANCEL:		// キャンセル
		SendMessage(hDlg, WM_COMMAND, IDPCANCEL, 0);
		break;

	default:
		return PSNRET_NOERROR;
	}
	return PSNRET_NOERROR;
}

/*
 * show_option - オプションの画面の表示
 */
int show_option(const HWND hWnd)
{
#define sizeof_PROPSHEETHEADER		40	// 古いコモンコントロール対策
#define PROP_CNT_OPTION				4
	PROPSHEETPAGE psp;
	PROPSHEETHEADER psh;
	HPROPSHEETPAGE hpsp[PROP_CNT_OPTION];
	TCHAR buf[BUF_SIZE];

	psp.dwSize = sizeof(PROPSHEETPAGE);
	psp.dwFlags = PSP_DEFAULT;
	psp.hInstance = hInst;

	// 表示
	psp.pszTemplate = MAKEINTRESOURCE(IDD_DIALOG_OPTION_VIEW);
	psp.pfnDlgProc = option_view_proc;
	hpsp[0] = CreatePropertySheetPage(&psp);

	// プレイヤー情報
	psp.pszTemplate = MAKEINTRESOURCE(IDD_DIALOG_OPTION_PLAYER);
	psp.pfnDlgProc = option_player_proc;
	hpsp[1] = CreatePropertySheetPage(&psp);

	// キー
	psp.pszTemplate = MAKEINTRESOURCE(IDD_DIALOG_OPTION_KEY);
	psp.pfnDlgProc = option_key_proc;
	hpsp[2] = CreatePropertySheetPage(&psp);

	// プラグイン
	psp.pszTemplate = MAKEINTRESOURCE(IDD_DIALOG_OPTION_PLUGIN);
	psp.pfnDlgProc = option_plugin_proc;
	hpsp[3] = CreatePropertySheetPage(&psp);

	ZeroMemory(&psh, sizeof(PROPSHEETHEADER));
	psh.dwSize = sizeof_PROPSHEETHEADER;
	psh.dwFlags = PSH_NOAPPLYNOW;
	psh.hInstance = hInst;
	psh.hwndParent = hWnd;
	psh.pszCaption = str_noprefix_cpy(buf, message_get_res(ID_MENUITEM_OPTION));
	psh.nPages = sizeof(hpsp) / sizeof(HPROPSHEETPAGE);
	psh.phpage = hpsp;

	psh.nStartPage = 0;

	prop_ret = 0;
	PropertySheet(&psh);
	return prop_ret;
}
/* End of source */
