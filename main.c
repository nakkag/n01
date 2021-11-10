/*
 * n01
 *
 * main.c
 *
 * Copyright (C) 1996-2017 by Ohno Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

/* Include Files */
#define _INC_OLE
#include <windows.h>
#undef  _INC_OLE
#include <shlobj.h>
#include <commctrl.h>

#include "general.h"
#include "Memory.h"
#include "String.h"
#include "Message.h"
#include "ini.h"
#include "nEdit.h"
#include "shortcut.h"
#include "score_info.h"
#include "score_list.h"
#include "score_left.h"
#include "score_player.h"
#include "score_guide.h"
#include "score_name.h"
#include "score_save.h"
#include "game_option.h"
#include "game_schedule.h"
#include "game_history.h"
#include "arrange.h"
#include "option.h"
#include "option_key.h"
#include "option_view.h"
#include "recovery.h"

#include "resource.h"

/* Define */
#define MAIN_WND_CLASS				TEXT("n01_wnd")
#define WINDOW_TITLE				TEXT("n01")

#define INI_FILE					TEXT("n01.ini")
#define LNK_FILE					TEXT("n01.lnk")

#define ID_INIT_MENU				(WM_APP + 10)

#define ID_TIMER_INIT				1
#define TIMER_INTERVAL_INIT			1

#define ID_TIMER_MAXIMIZE			2
#define TIMER_INTERVAL_MAXIMIZE		100

#define ID_TIMER_MENU				3

#define WINDOW_MENU_COUNT			4
#define WINDOW_MENU_TOOL			3

/* Global Variables */
HINSTANCE hInst;
HACCEL hKeyAccel;
HACCEL hPluginAccel;
HMENU hMenu;

HBRUSH back_brush;

OPTION_INFO op;
SCORE_HISTORY sh;
SCORE_INFO si;

TCHAR work_path[MAX_PATH];
TCHAR ini_path[MAX_PATH];

typedef struct _WINDOW_INFO {
	HWND hWnd;

	HWND score_list_wnd;
	HWND score_left_wnd[2];
	HWND score_player_wnd[2];
	HWND score_guide_wnd;
	HWND score_name_wnd;
} WINDOW_INFO;

/* Local Function Prototypes */
static void set_menu_key(const HMENU menu, const int cmd);
static void set_menu_score(const HMENU menu);
static void set_menu_plugin(const HWND hWnd, const HMENU menu);
static LRESULT CALLBACK main_proc(const HWND hWnd, const UINT msg, WPARAM wParam, LPARAM lParam);
static void get_path(const HINSTANCE hInstance);
static BOOL init_application(const HINSTANCE hInstance);
static HWND init_instance(const HINSTANCE hInstance, const int CmdShow);

/*
 * set_menu_key - メニューにアクセラレーターを追加
 */
static void set_menu_key(const HMENU menu, const int cmd)
{
	MENUITEMINFO mii;
	TCHAR buf[BUF_SIZE];
	TCHAR menu_name[BUF_SIZE];
	int i;

	message_copy_res(cmd, menu_name);

	for (i = 0; i < op.key_info_count; i++) {
		if (cmd != op.key_info[i].action) {
			continue;
		}
		if (get_keyname(op.key_info[i].ctrl, op.key_info[i].key, buf) == FALSE) {
			continue;
		}
		wsprintf(menu_name, TEXT("%s\t%s"), message_get_res(cmd), buf);
		break;
	}
	ZeroMemory(&mii, sizeof(mii));
	mii.cbSize = sizeof(mii);
	mii.fMask = MIIM_TYPE;
	mii.fType = MFT_STRING;
	GetMenuItemInfo(menu, cmd, FALSE, &mii);
	mii.dwTypeData = menu_name;
	SetMenuItemInfo(menu, cmd, FALSE, &mii);
}

/*
 * set_menu_score - メニューに定型入力値を追加
 */
static void set_menu_score(const HMENU menu)
{
	TCHAR buf[BUF_SIZE];
	TCHAR menu_name[BUF_SIZE];
	int i, j;

	while(DeleteMenu(menu, 0, MF_BYPOSITION) == TRUE);

	for (i = 0; i < op.key_info_count; i++) {
		if (op.key_info[i].action < ID_ACCEL_INPUT_SCORE || op.key_info[i].action > ID_ACCEL_INPUT_SCORE + 180) {
			continue;
		}
		if (get_keyname(op.key_info[i].ctrl, op.key_info[i].key, buf) == FALSE) {
			continue;
		}
		for (j = 0; j < i; j++) {
			if (op.key_info[i].action == op.key_info[j].action) {
				break;
			}
		}
		if (j >= i) {
			wsprintf(menu_name, TEXT("%d\t%s"), op.key_info[i].action - ID_ACCEL_INPUT_SCORE, buf);
			AppendMenu(menu, MF_STRING, op.key_info[i].action, menu_name);
		}
	}
	if (GetMenuItemCount(menu) == 0) {
		AppendMenu(menu, MF_STRING | MF_GRAYED, ID_MENUITEM_NOTHING, message_get_res(IDS_STRING_MENU_NOTHING));
	}
}

/*
 * set_menu_plugin - ツールメニューにプラグインを追加
 */
static void set_menu_plugin(const HWND hWnd, const HMENU menu)
{
	TCHAR buf[BUF_SIZE];
	TCHAR menu_name[BUF_SIZE];
	int i;

	// ツールメニューの削除
	if (GetMenuItemCount(menu) == WINDOW_MENU_COUNT + 1) {
		DeleteMenu(menu, WINDOW_MENU_TOOL, MF_BYPOSITION);
	}
	// ツールメニューにプラグインを設定
	for (i = 0; i < op.plugin_info_count; i++) {
		if (!(op.plugin_info[i].call_type & CALLTYPE_MENU)) {
			continue;
		}
		if (GetMenuItemCount(menu) == WINDOW_MENU_COUNT) {
			InsertMenu(menu, WINDOW_MENU_TOOL, MF_BYPOSITION | MF_POPUP, (UINT)CreatePopupMenu(),
				message_get_res(IDS_STRING_MENU_TOOL));
		}
		if (get_keyname(op.plugin_info[i].ctrl, op.plugin_info[i].key, buf) == FALSE) {
			AppendMenu(GetSubMenu(menu, WINDOW_MENU_TOOL), MF_STRING, ID_MENUITEM_TOOL + i,
				op.plugin_info[i].title);
		} else {
			wsprintf(menu_name, TEXT("%s\t%s"), op.plugin_info[i].title, buf);
			AppendMenu(GetSubMenu(menu, WINDOW_MENU_TOOL), MF_STRING, ID_MENUITEM_TOOL + i, menu_name);
		}
	}
	DrawMenuBar(hWnd);
}

/*
 * main_proc - メインウィンドウプロシージャ
 */
static LRESULT CALLBACK main_proc(const HWND hWnd, const UINT msg, WPARAM wParam, LPARAM lParam)
{
	static WINDOW_INFO wi;
	SCORE_INFO *tmp_si;
	RECT rect;
	TCHAR buf[BUF_SIZE];
	int left, right;
	int left_height, guide_height, name_height;
	int i;

	switch (msg) {
	case WM_CREATE:
		// リカバリ情報の読み込み
		if ((tmp_si = recovery_load(hWnd)) != NULL) {
			si = *tmp_si;
		}

		// スコア情報の初期化
		sh.si = &si;
		if (tmp_si == NULL && score_info_init(hWnd, &si, &op.gi, TRUE) == FALSE) {
			return -1;
		}

		// ウィンドウの作成
		wi.hWnd = hWnd;
		wi.score_name_wnd = score_name_create(hInst, hWnd, 0, &si);
		wi.score_list_wnd = score_list_create(hInst, hWnd, 0, &si);
		wi.score_left_wnd[0] = score_left_create(hInst, hWnd, 0, &si.player[0]);
		wi.score_left_wnd[1] = score_left_create(hInst, hWnd, 0, &si.player[1]);
		wi.score_player_wnd[0] = score_player_create(hInst, hWnd, 0, &si.player[0]);
		wi.score_player_wnd[1] = score_player_create(hInst, hWnd, 0, &si.player[1]);
		wi.score_guide_wnd = score_guide_create(hInst, hWnd, 0);

		// ウィンドウ表示設定
		if (op.check_out_mode == 1) {
			CheckMenuItem(GetSubMenu(GetMenu(hWnd), 0), ID_MENUITEM_CHECK_OUT_MODE, MF_CHECKED);
		}
		if (op.view_name == 1) {
			CheckMenuItem(GetSubMenu(GetMenu(hWnd), 1), ID_MENUITEM_SHOW_NAME, MF_CHECKED);
			ShowWindow(wi.score_name_wnd, SW_SHOW);
		}
		if (op.view_player == 1) {
			CheckMenuItem(GetSubMenu(GetMenu(hWnd), 1), ID_MENUITEM_SHOW_PLAYER, MF_CHECKED);
			ShowWindow(wi.score_player_wnd[0], SW_SHOW);
			ShowWindow(wi.score_player_wnd[1], SW_SHOW);
		}
		if (op.view_left == 1) {
			CheckMenuItem(GetSubMenu(GetMenu(hWnd), 1), ID_MENUITEM_SHOW_LEFT, MF_CHECKED);
			ShowWindow(wi.score_left_wnd[0], SW_SHOW);
			ShowWindow(wi.score_left_wnd[1], SW_SHOW);
		}
		if (op.view_guide == 1) {
			CheckMenuItem(GetSubMenu(GetMenu(hWnd), 1), ID_MENUITEM_SHOW_GUIDE, MF_CHECKED);
			ShowWindow(wi.score_guide_wnd, SW_SHOW);
		}

		// メニューの設定
		SendMessage(hWnd, WM_COMMAND, ID_INIT_MENU, 0);
		CheckMenuItem(GetSubMenu(GetMenu(hWnd), 0), ID_MENUITEM_AUTO_SAVE, (op.auto_save == 1) ? MF_CHECKED : MF_UNCHECKED);
		CheckMenuItem(GetSubMenu(GetMenu(hWnd), 1), ID_MENUITEM_SHOW_DARTSCOUNT, (op.view_throw_count == 1) ? MF_CHECKED : MF_UNCHECKED);
		EnableMenuItem(GetSubMenu(GetMenu(hWnd), 0), ID_MENUITEM_SET_SCHEDULE, MF_GRAYED);
		switch (op.left_font_size) {
		case FONT_SIZE_L:
			CheckMenuRadioItem(GetSubMenu(GetMenu(hWnd), 1), ID_MENUITEM_FONT_L, ID_MENUITEM_FONT_S, ID_MENUITEM_FONT_L, MF_BYCOMMAND);
			break;
		case FONT_SIZE_M:
		default:
			CheckMenuRadioItem(GetSubMenu(GetMenu(hWnd), 1), ID_MENUITEM_FONT_L, ID_MENUITEM_FONT_S, ID_MENUITEM_FONT_M, MF_BYCOMMAND);
			break;
		case FONT_SIZE_S:
			CheckMenuRadioItem(GetSubMenu(GetMenu(hWnd), 1), ID_MENUITEM_FONT_L, ID_MENUITEM_FONT_S, ID_MENUITEM_FONT_S, MF_BYCOMMAND);
			break;
		}
		CheckMenuItem(GetSubMenu(GetMenu(hWnd), 1), ID_MENUITEM_FULL_SCREEN, (op.full_screen == TRUE) ? MF_CHECKED : MF_UNCHECKED);

		// プラグインの初期化
		plugin_initialize(hWnd, &si);
		set_menu_plugin(hWnd, GetMenu(hWnd));
		plugin_execute_all(hWnd, CALLTYPE_START, &si, 0);

		// ゲームの初期化
		SendMessage(wi.score_name_wnd, WM_NAME_SET_MODE, (si.leg_limit == 1) ? TRUE : FALSE, 0);
		SendMessage(wi.score_player_wnd[0], WM_PLAYER_SET_MODE, (si.leg_limit == 1) ? TRUE : FALSE, 0);
		SendMessage(wi.score_player_wnd[1], WM_PLAYER_SET_MODE, (si.leg_limit == 1) ? TRUE : FALSE, 0);
		if (tmp_si == NULL) {
			SendMessage(wi.score_list_wnd, WM_SCORE_INIT_LEG, TRUE, TRUE);
			// 初期化用タイマー
			SetTimer(hWnd, ID_TIMER_INIT, TIMER_INTERVAL_INIT, NULL);
		} else {
			// ゲームの再開 (リカバリ)
			SendMessage(wi.score_list_wnd, WM_SCORE_RESTART, si.current_leg, 0);
			if (si.set_mode == TRUE) {
				EnableMenuItem(GetSubMenu(GetMenu(hWnd), 0), ID_MENUITEM_SET_SCHEDULE, MF_ENABLED);
			}
		}
		break;

	case WM_SIZE:
		// ウィンドウサイズ変更
		if (GetWindowLong(hWnd, GWL_STYLE) & WS_OVERLAPPEDWINDOW) {
			op.window_state = (IsZoomed(hWnd) == 0) ? SW_SHOWDEFAULT : SW_MAXIMIZE;
		}

		GetClientRect(hWnd, &rect);
		left = op.view_separate_width;
		right = rect.right - (op.view_separate_width * 2);
		left_height = 0;
		guide_height = op.view_separate_width;
		name_height = op.view_separate_width;
		if (op.view_name == 1) {
			name_height = SendMessage(wi.score_name_wnd, WM_NAME_GET_HEIGHT, (rect.right < rect.bottom) ? rect.right : rect.bottom, 0);
			MoveWindow(wi.score_name_wnd, op.view_separate_width, op.view_separate_width,
				rect.right - (op.view_separate_width * 2), name_height, TRUE);
			name_height += (op.view_separate_width * 2);
			InvalidateRect(wi.score_name_wnd, NULL, FALSE);
			UpdateWindow(wi.score_name_wnd);
		}
		if (op.view_guide == 1) {
			guide_height = SendMessage(wi.score_guide_wnd, WM_GUIDE_GET_HEIGHT, (rect.right < rect.bottom) ? rect.right : rect.bottom, 0);
			guide_height += op.view_separate_width;
		}
		if (op.view_left == 1) {
			left_height = SendMessage(wi.score_left_wnd[0], WM_LEFT_GET_HEIGHT, ((rect.right < rect.bottom) ? rect.right : rect.bottom), 0);
			MoveWindow(wi.score_left_wnd[0], op.view_separate_width, rect.bottom - left_height - guide_height,
				rect.right / 2 - (op.view_separate_width + op.view_separate_width / 2), left_height, TRUE);
			MoveWindow(wi.score_left_wnd[1], rect.right / 2 + (op.view_separate_width / 2), rect.bottom - left_height - guide_height,
				rect.right - (rect.right / 2 + (op.view_separate_width / 2)) - op.view_separate_width, left_height, TRUE);
			left_height += op.view_separate_width;

			InvalidateRect(wi.score_left_wnd[0], NULL, FALSE);
			UpdateWindow(wi.score_left_wnd[0]);
			InvalidateRect(wi.score_left_wnd[1], NULL, FALSE);
			UpdateWindow(wi.score_left_wnd[1]);
		}
		if (op.view_player == 1) {
			i = (rect.right * 100) / 5;
			MoveWindow(wi.score_player_wnd[0],
				op.view_separate_width,
				name_height,
				i / 100 - op.view_separate_width,
				rect.bottom - left_height - guide_height - name_height, TRUE);
			MoveWindow(wi.score_player_wnd[1],
				rect.right - (i / 100),
				name_height,
				i / 100 - op.view_separate_width,
				rect.bottom - left_height - guide_height - name_height, TRUE);
			left = i / 100 + op.view_separate_width;
			right = rect.right - (i / 100 + op.view_separate_width) - left;

			InvalidateRect(wi.score_player_wnd[0], NULL, FALSE);
			UpdateWindow(wi.score_player_wnd[0]);
			InvalidateRect(wi.score_player_wnd[1], NULL, FALSE);
			UpdateWindow(wi.score_player_wnd[1]);
		}
		MoveWindow(wi.score_list_wnd, left, name_height, right, rect.bottom - left_height - guide_height - name_height, TRUE);

		if (op.view_guide == 1) {
			guide_height -= op.view_separate_width;
			MoveWindow(wi.score_guide_wnd, 0, rect.bottom - guide_height, rect.right, guide_height, TRUE);
			InvalidateRect(wi.score_guide_wnd, NULL, FALSE);
			UpdateWindow(wi.score_guide_wnd);
		}
		break;

	case WM_EXITSIZEMOVE:
		// サイズ変更完了
		if (GetWindowLong(hWnd, GWL_STYLE) & WS_OVERLAPPEDWINDOW &&
			IsWindowVisible(hWnd) != 0 && IsIconic(hWnd) == 0 && IsZoomed(hWnd) == 0) {
			GetWindowRect(hWnd, (LPRECT)&op.window_rect);
			op.window_rect.right -= op.window_rect.left;
			op.window_rect.bottom -= op.window_rect.top;
		}
		break;

	case WM_QUERYENDSESSION:
		// OSの終了
		if (plugin_execute_all(hWnd, CALLTYPE_END, &si, WM_QUERYENDSESSION) == PLUGIN_CANCEL) {
			return FALSE;
		}
		return TRUE;

	case WM_ENDSESSION:
		// OSの終了
		DestroyWindow(hWnd);
		return 0;

	case WM_CLOSE:
		// ウィンドウを閉じる
		if (MessageBox(hWnd, message_get_res(IDS_STRING_EXIT), APP_NAME, MB_ICONQUESTION | MB_YESNO) == IDNO) {
			break;
		}
		if (plugin_execute_all(hWnd, CALLTYPE_END, &si, WM_CLOSE) == PLUGIN_CANCEL) {
			break;
		}
		DestroyWindow(hWnd);
		break;

	case WM_DESTROY:
		DeleteObject(back_brush);
		back_brush = NULL;
		// ウィンドウの破棄
		// リカバリ情報削除
		recovery_delete();
		// 設定保存
		ini_put_option(ini_path);
		// スコア情報解放
		score_info_free(&si);
		score_history_free(&sh);
		// プラグイン情報解放
		plugin_free();
		PostQuitMessage(0);
		break;

	case WM_PAINT:
		if (back_brush == NULL) {
			back_brush = CreateSolidBrush(op.ci.view_separate);
		}
		{
			HDC hdc;
			PAINTSTRUCT ps;

			hdc = BeginPaint(hWnd , &ps);
			GetClientRect(hWnd, &rect);
			FillRect(hdc, &rect, back_brush);
			EndPaint(hWnd , &ps);
		}
		break;

	case WM_SETFOCUS:
		// フォーカスの設定
		SetFocus(wi.score_list_wnd);
		break;

	case WM_SYSCOMMAND:
		if (op.full_screen && op.full_screen_menu_interval >= 0 &&
			wParam == SC_KEYMENU && GetMenu(hWnd) == NULL && GetForegroundWindow() == hWnd) {
			SetMenu(hWnd, hMenu);
			KillTimer(hWnd, ID_TIMER_MENU);
			SetTimer(hWnd, ID_TIMER_MENU, op.full_screen_menu_interval, NULL);
		}
		return DefWindowProc(hWnd, msg, wParam, lParam);

	case WM_SETCURSOR:
		if (op.full_screen && op.full_screen_menu_interval >= 0 &&
			HIWORD(lParam) == WM_MOUSEMOVE && GetMenu(hWnd) == NULL && GetForegroundWindow() == hWnd) {
			// フルスクリーン時に画面上部にマウスを持っていくとメニューを表示
			POINT pt;
			GetCursorPos(&pt);
			if (pt.y <= GetSystemMetrics(SM_CYMENU)) {
				SetMenu(hWnd, hMenu);
				KillTimer(hWnd, ID_TIMER_MENU);
				SetTimer(hWnd, ID_TIMER_MENU, op.full_screen_menu_interval, NULL);
			}
		}
		return DefWindowProc(hWnd, msg, wParam, lParam);

	case WM_MENUSELECT:
		if (op.full_screen && op.full_screen_menu_interval >= 0) {
			if ((UINT)HIWORD(wParam) == (UINT)0xFFFF) {
				KillTimer(hWnd, ID_TIMER_MENU);
				SetTimer(hWnd, ID_TIMER_MENU, op.full_screen_menu_interval, NULL);
			} else {
				// メニュー選択時
				RECT rect;

				if ((HMENU)lParam != NULL && GetMenu(hWnd) == NULL) {
					SetMenu(hWnd, hMenu);
				}
				// メニューによる表示乱れを再描画
				GetClientRect(hWnd, &rect);
				rect.bottom = GetSystemMetrics(SM_CYMENU) + op.view_separate_width;
				InvalidateRect(hWnd, &rect, FALSE);
				UpdateWindow(hWnd);

				KillTimer(hWnd, ID_TIMER_MENU);
			}
		}
		break;

	case WM_TIMER:
		// タイマー
		switch (wParam) {
		case ID_TIMER_INIT:
			KillTimer(hWnd, ID_TIMER_INIT);
			// ゲーム設定
			SendMessage(hWnd, WM_COMMAND, ID_MENUITEM_NEW_GAME, 0);
			break;
		case ID_TIMER_MAXIMIZE:
			KillTimer(hWnd, ID_TIMER_MAXIMIZE);
			ShowWindow(hWnd, SW_MAXIMIZE);
			break;
		case ID_TIMER_MENU:
			KillTimer(hWnd, ID_TIMER_MENU);
			if (op.full_screen && op.full_screen_menu_interval >= 0 && GetMenu(hWnd) != NULL) {
				POINT pt;
				GetCursorPos(&pt);
				if (pt.y > GetSystemMetrics(SM_CYMENU)) {
					SetMenu(hWnd, NULL);
				} else {
					SetTimer(hWnd, ID_TIMER_MENU, op.full_screen_menu_interval, NULL);
				}
			}
			break;
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case ID_ACCEL_TAB:
		case ID_ACCEL_STAB:
		case ID_ACCEL_ESC:
		case ID_ACCEL_NONE:
		case ID_ACCEL_UP:
		case ID_ACCEL_DOWN:
		case ID_ACCEL_PRIOR:
		case ID_ACCEL_NEXT:
		case ID_ACCEL_HOME:
		case ID_ACCEL_END:
		case ID_MENUITEM_FINISH_ONE:
		case ID_MENUITEM_FINISH_TWO:
		case ID_MENUITEM_FINISH_THREE:
		case ID_MENUITEM_MIDDLE:
		case ID_MENUITEM_SCORE_LEFT:
		case ID_MENUITEM_SCORE_SWAP:
		case ID_MENUITEM_SCORE_CLEAR:
		case ID_MENUITEM_ARRANGE:
			// スコアウィンドウに送信
			SendMessage(wi.score_list_wnd, WM_COMMAND, wParam, lParam);
			break;

		case IDS_STRING_LEFT:
			// ←
			keybd_event((BYTE)VK_LEFT, 0, 0, 0);
			keybd_event((BYTE)VK_LEFT, 0, KEYEVENTF_KEYUP, 0);
			break;

		case IDS_STRING_RIGHT:
			// →
			keybd_event((BYTE)VK_RIGHT, 0, 0, 0);
			keybd_event((BYTE)VK_RIGHT, 0, KEYEVENTF_KEYUP, 0);
			break;

		case ID_MENUITEM_NEW_GAME:
			// 新規ゲーム
			if (show_game_option(hInst, hWnd, &op.gi) == FALSE) {
				break;
			}
			SendMessage(hWnd, WM_N01_GAME_ON, FALSE, 0);
			break;

		case ID_MENUITEM_NEW_SCHEDULE:
			// 新規スケジュール
			if (show_game_schedule(hInst, hWnd, -1) == FALSE) {
				break;
			}
			SendMessage(hWnd, WM_N01_GAME_ON, TRUE, 0);
			break;

		case ID_MENUITEM_SET_SCHEDULE:
			// スケジュール変更
			i = si.current_set;
			if (si.current_leg == 0 && si.leg[0].current_round == 0 && si.leg[0].current_player == 0 && si.leg[0].first == 0) {
				i--;
			}
			if (show_game_schedule(hInst, hWnd, i + 1) == FALSE) {
				break;
			}
			if (i != si.current_set) {
				if (score_info_init(hWnd, &si, &op.gi_list[si.current_set], FALSE) == FALSE) {
					break;
				}
				SendMessage(wi.score_player_wnd[0], WM_PLAYER_DRAW_INIT, 0, 0);
				SendMessage(wi.score_player_wnd[1], WM_PLAYER_DRAW_INIT, 0, 0);
				SendMessage(wi.score_list_wnd, WM_SCORE_INIT_LEG, TRUE, TRUE);
			}
			break;

		case ID_MENUITEM_GAME_HISTORY:
			// ゲーム履歴
			show_game_history(hInst, hWnd, &sh);
			break;

		case ID_MENUITEM_PREV_LEG:
			// 前のレッグに戻る
			SendMessage(wi.score_list_wnd, WM_SCORE_PREV_LEG, 0, 0);
			break;

		case ID_MENUITEM_OPTION:
			// オプション
			if (show_option(hWnd) == 0) {
				break;
			}
			ini_put_option(ini_path);
			DeleteObject(back_brush);
			back_brush = NULL;
			InvalidateRect(hWnd, NULL, FALSE);
			UpdateWindow(hWnd);

			SendMessage(wi.score_list_wnd, WM_SCORE_DRAW_INIT, 0, 0);
			SendMessage(wi.score_name_wnd, WM_NAME_DRAW_INIT, 0, 0);
			SendMessage(wi.score_player_wnd[0], WM_PLAYER_DRAW_INIT, 0, 0);
			SendMessage(wi.score_player_wnd[1], WM_PLAYER_DRAW_INIT, 0, 0);
			SendMessage(wi.score_left_wnd[0], WM_LEFT_DRAW_INIT, 0, 0);
			SendMessage(wi.score_left_wnd[1], WM_LEFT_DRAW_INIT, 0, 0);
			SendMessage(wi.score_guide_wnd, WM_GUIDE_DRAW_INIT, 0, 0);

			SendMessage(hWnd, WM_SIZE, 0, 0);
			SendMessage(wi.score_left_wnd[0], WM_SIZE, 0, 0);
			SendMessage(wi.score_left_wnd[1], WM_SIZE, 0, 0);

			SendMessage(wi.score_guide_wnd, WM_GUIDE_REDRAW, 0, 0);
			create_accelerator();
			op.check_out_font_size = 0;
			// プラグインの初期化
			plugin_initialize(hWnd, &si);
			plugin_create_accelerator();
			set_menu_plugin(hWnd, hMenu);
		case ID_INIT_MENU:
			// メニューの設定
			{
				HMENU hInitMenu = hMenu;
				if (hInitMenu == NULL) {
					hInitMenu = GetMenu(hWnd);
				}
				set_menu_score(GetSubMenu(GetSubMenu(hInitMenu, 2), 6));

				set_menu_key(GetSubMenu(hInitMenu, 0), ID_MENUITEM_NEW_GAME);
				set_menu_key(GetSubMenu(hInitMenu, 0), ID_MENUITEM_NEW_SCHEDULE);
				set_menu_key(GetSubMenu(hInitMenu, 0), ID_MENUITEM_SET_SCHEDULE);
				set_menu_key(GetSubMenu(hInitMenu, 0), ID_MENUITEM_GAME_HISTORY);
				set_menu_key(GetSubMenu(hInitMenu, 0), ID_MENUITEM_OPTION);
				set_menu_key(GetSubMenu(hInitMenu, 0), ID_MENUITEM_AUTO_SAVE);
				set_menu_key(GetSubMenu(hInitMenu, 0), ID_MENUITEM_CHECK_OUT_MODE);
				set_menu_key(GetSubMenu(hInitMenu, 0), ID_MENUITEM_EXIT);

				set_menu_key(GetSubMenu(hInitMenu, 1), ID_MENUITEM_SHOW_NAME);
				set_menu_key(GetSubMenu(hInitMenu, 1), ID_MENUITEM_SHOW_PLAYER);
				set_menu_key(GetSubMenu(hInitMenu, 1), ID_MENUITEM_SHOW_LEFT);
				set_menu_key(GetSubMenu(hInitMenu, 1), ID_MENUITEM_SHOW_GUIDE);
				set_menu_key(GetSubMenu(GetSubMenu(hInitMenu, 1), 4), ID_MENUITEM_FONT_L);
				set_menu_key(GetSubMenu(GetSubMenu(hInitMenu, 1), 4), ID_MENUITEM_FONT_M);
				set_menu_key(GetSubMenu(GetSubMenu(hInitMenu, 1), 4), ID_MENUITEM_FONT_S);
				set_menu_key(GetSubMenu(hInitMenu, 1), ID_MENUITEM_FULL_SCREEN);
				set_menu_key(GetSubMenu(hInitMenu, 1), ID_MENUITEM_SHOW_DARTSCOUNT);

				set_menu_key(GetSubMenu(hInitMenu, 2), ID_MENUITEM_FINISH_ONE);
				set_menu_key(GetSubMenu(hInitMenu, 2), ID_MENUITEM_FINISH_TWO);
				set_menu_key(GetSubMenu(hInitMenu, 2), ID_MENUITEM_FINISH_THREE);
				set_menu_key(GetSubMenu(hInitMenu, 2), ID_MENUITEM_MIDDLE);
				set_menu_key(GetSubMenu(hInitMenu, 2), ID_MENUITEM_SCORE_LEFT);
				set_menu_key(GetSubMenu(hInitMenu, 2), ID_MENUITEM_PREV_LEG);
				set_menu_key(GetSubMenu(hInitMenu, 2), ID_MENUITEM_SCORE_SWAP);
				set_menu_key(GetSubMenu(hInitMenu, 2), ID_MENUITEM_SCORE_CLEAR);
				set_menu_key(GetSubMenu(hInitMenu, 2), ID_MENUITEM_ARRANGE);
			}
			break;

		case ID_MENUITEM_AUTO_SAVE:
			// スコアの自動保存
			op.auto_save = !op.auto_save;
			CheckMenuItem(GetSubMenu(hMenu, 0), ID_MENUITEM_AUTO_SAVE, (op.auto_save == 1) ? MF_CHECKED : MF_UNCHECKED);
			if (op.auto_save == 1) {
				TCHAR path[BUF_SIZE];
				if (*op.auto_save_path == TEXT('\0')) {
					wsprintf(path, TEXT("%s\\data"), work_path);
				} else {
					lstrcpy(path, op.auto_save_path);
				}
				wsprintf(buf, message_get_res(IDS_STRING_AUTO_SAVE_MODE), path);
				MessageBox(hWnd, buf, APP_NAME, MB_ICONINFORMATION);
			}
			break;

		case ID_MENUITEM_CHECK_OUT_MODE:
			// チェックアウトモード
			op.check_out_mode = !op.check_out_mode;
			CheckMenuItem(GetSubMenu(hMenu, 0), ID_MENUITEM_CHECK_OUT_MODE, (op.check_out_mode == 1) ? MF_CHECKED : MF_UNCHECKED);
			if (op.check_out_mode != 0) {
				MessageBox(hWnd, message_get_res(IDS_STRING_CHECK_OUT_MODE), APP_NAME, MB_ICONINFORMATION);
			}
			si.player[0].check_out_mode = (si.player[0].com == TRUE) ? FALSE : ((op.check_out_mode == 0) ? FALSE : TRUE);
			si.player[1].check_out_mode = (si.player[1].com == TRUE) ? FALSE : ((op.check_out_mode == 0) ? FALSE : TRUE);
			break;

		case ID_MENUITEM_ONLINE:
			// n01 Online
			ShellExecute(NULL, "open", "http://nakka.com/n01/online/", NULL, NULL, SW_SHOWNORMAL);
			break;

		case ID_MENUITEM_EXIT:
			// プログラムの終了
			SendMessage(hWnd, WM_CLOSE, 0, 0);
			break;

		case ID_MENUITEM_SHOW_NAME:
			// 名前
			op.view_name = !op.view_name;
			ShowWindow(wi.score_name_wnd, (op.view_name == 1) ? SW_SHOW : SW_HIDE);
			CheckMenuItem(GetSubMenu(hMenu, 1), ID_MENUITEM_SHOW_NAME, (op.view_name == 1) ? MF_CHECKED : MF_UNCHECKED);
			SendMessage(hWnd, WM_SIZE, 0, 0);
			break;

		case ID_MENUITEM_SHOW_PLAYER:
			// プレイヤー情報
			op.view_player = !op.view_player;
			ShowWindow(wi.score_player_wnd[0], (op.view_player == 1) ? SW_SHOW : SW_HIDE);
			ShowWindow(wi.score_player_wnd[1], (op.view_player == 1) ? SW_SHOW : SW_HIDE);
			CheckMenuItem(GetSubMenu(hMenu, 1), ID_MENUITEM_SHOW_PLAYER, (op.view_player == 1) ? MF_CHECKED : MF_UNCHECKED);
			SendMessage(hWnd, WM_SIZE, 0, 0);
			SendMessage(wi.score_name_wnd, WM_NAME_REDRAW, 0, 0);
			break;

		case ID_MENUITEM_SHOW_LEFT:
			// 残りスコア
			op.view_left = !op.view_left;
			ShowWindow(wi.score_left_wnd[0], (op.view_left == 1) ? SW_SHOW : SW_HIDE);
			ShowWindow(wi.score_left_wnd[1], (op.view_left == 1) ? SW_SHOW : SW_HIDE);
			CheckMenuItem(GetSubMenu(hMenu, 1), ID_MENUITEM_SHOW_LEFT, (op.view_left == 1) ? MF_CHECKED : MF_UNCHECKED);
			SendMessage(hWnd, WM_SIZE, 0, 0);
			break;

		case ID_MENUITEM_SHOW_GUIDE:
			// ファンクションキー
			op.view_guide = !op.view_guide;
			ShowWindow(wi.score_guide_wnd, (op.view_guide == 1) ? SW_SHOW : SW_HIDE);
			CheckMenuItem(GetSubMenu(hMenu, 1), ID_MENUITEM_SHOW_GUIDE, (op.view_guide == 1) ? MF_CHECKED : MF_UNCHECKED);
			SendMessage(hWnd, WM_SIZE, 0, 0);
			break;

		case ID_MENUITEM_FONT_L:
			// フォント大
			op.left_font_size = FONT_SIZE_L;
			SendMessage(hWnd, WM_SIZE, 0, 0);
			CheckMenuRadioItem(GetSubMenu(hMenu, 1), ID_MENUITEM_FONT_L, ID_MENUITEM_FONT_S, ID_MENUITEM_FONT_L, MF_BYCOMMAND);
			break;

		case ID_MENUITEM_FONT_M:
			// フォント中
			op.left_font_size = FONT_SIZE_M;
			SendMessage(hWnd, WM_SIZE, 0, 0);
			CheckMenuRadioItem(GetSubMenu(hMenu, 1), ID_MENUITEM_FONT_L, ID_MENUITEM_FONT_S, ID_MENUITEM_FONT_M, MF_BYCOMMAND);
			break;

		case ID_MENUITEM_FONT_S:
			// フォント小
			op.left_font_size = FONT_SIZE_S;
			SendMessage(hWnd, WM_SIZE, 0, 0);
			CheckMenuRadioItem(GetSubMenu(hMenu, 1), ID_MENUITEM_FONT_L, ID_MENUITEM_FONT_S, ID_MENUITEM_FONT_S, MF_BYCOMMAND);
			break;

		case ID_MENUITEM_FULL_SCREEN:
			// フルスクリーン
			if (!op.full_screen) {
				op.full_screen = TRUE;
				op.window_state = (IsZoomed(hWnd) == 0) ? SW_SHOWDEFAULT : SW_MAXIMIZE;
				SetWindowLong(hWnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);
				ShowWindow(hWnd, SW_MAXIMIZE);
				if (op.full_screen_menu_interval >= 0) {
					SetMenu(hWnd, NULL);
				}
			} else {
				op.full_screen = FALSE;
				SetWindowLong(hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW | WS_VISIBLE);
				if (op.window_state == SW_MAXIMIZE) {
					SetTimer(hWnd, ID_TIMER_MAXIMIZE, TIMER_INTERVAL_MAXIMIZE, NULL);
				}
				if (op.window_rect.left == 0 && op.window_rect.top == 0 &&
					op.window_rect.right == 0 && op.window_rect.bottom == 0) {
					RECT rect;
					SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0);
					rect.right -= rect.left;
					rect.bottom -= rect.top;
					SetWindowPos(hWnd, HWND_TOP, rect.left, rect.top, rect.right, rect.bottom,	SWP_FRAMECHANGED);
				} else {
					SetWindowPos(hWnd, HWND_TOP, op.window_rect.left, op.window_rect.top, op.window_rect.right, op.window_rect.bottom,	SWP_FRAMECHANGED);
				}
				ShowWindow(hWnd, SW_SHOWNORMAL);
				SetMenu(hWnd, hMenu);
			}
			CheckMenuItem(GetSubMenu(hMenu, 1), ID_MENUITEM_FULL_SCREEN, (op.full_screen == TRUE) ? MF_CHECKED : MF_UNCHECKED);
			break;

		case ID_MENUITEM_SHOW_DARTSCOUNT:
			// インデックスをダーツ数で表示
			op.view_throw_count = !op.view_throw_count;
			CheckMenuItem(GetSubMenu(hMenu, 1), ID_MENUITEM_SHOW_DARTSCOUNT, (op.view_throw_count == 1) ? MF_CHECKED : MF_UNCHECKED);
			SendMessage(wi.score_list_wnd, WM_SCORE_REDRAW, 0, 0);
			break;

		case ID_MENUITEM_ABOUT:
			// バージョン情報
			MessageBox(hWnd,
				APP_NAME
				TEXT("\nCopyright (C) 1996-2017 by Ohno Tomoaki. All rights reserved.\n\n")
				TEXT("WEB SITE: http://www.nakka.com/\nE-MAIL: nakka@nakka.com"),
				TEXT("About"), MB_OK | MB_ICONINFORMATION);
			break;

		default:
			if (LOWORD(wParam) >= ID_ACCEL_INPUT_SCORE && LOWORD(wParam) <= ID_ACCEL_INPUT_SCORE + 180) {
				// 定型入力
				SendMessage(wi.score_list_wnd, WM_SCORE_INPUT, LOWORD(wParam) - ID_ACCEL_INPUT_SCORE, 0);
				break;
			}
			if (LOWORD(wParam) >= ID_MENUITEM_TOOL && LOWORD(wParam) < ID_MENUITEM_TOOL + 1000) {
				// ツールメニュー (プラグイン)
				plugin_execute(hWnd, op.plugin_info + LOWORD(wParam) - ID_MENUITEM_TOOL, CALLTYPE_MENU, &si, 0);
				break;
			}
			break;
		}
		break;

	case WM_WINDOW_REDRAW:
		// ウィンドウの再描画
		SendMessage(wi.score_name_wnd, WM_NAME_REDRAW, 0, 0);
		SendMessage(wi.score_left_wnd[wParam], WM_LEFT_REDRAW, 0, 0);
		SendMessage(wi.score_player_wnd[wParam], WM_PLAYER_REDRAW, 0, 0);

		if (lParam == TRUE) {
			if (si.leg_limit == 1 && si.best_of == 1) {
				wsprintf(buf, message_get_res(IDS_STRING_TITLE_LEG_BEST_OF), WINDOW_TITLE, si.current_set + 1, si.current_leg + 1, si.max_leg);
			} else if (si.leg_limit == 1) {
				wsprintf(buf, message_get_res(IDS_STRING_TITLE_LEG), WINDOW_TITLE, si.current_set + 1, si.current_leg + 1, si.max_leg);
			} else {
				wsprintf(buf, message_get_res(IDS_STRING_TITLE), WINDOW_TITLE, si.current_leg + 1);
			}
			if (si.set_mode == TRUE) {
				message_copy_res(IDS_STRING_SCHEDULE_MODE, buf + lstrlen(buf));
			}
			SetWindowText(hWnd, buf);
		}
		break;

	case WM_WINDOW_SET_CURRENT:
		// カレントプレイヤーの設定
		SendMessage(wi.score_left_wnd[wParam], WM_LEFT_SET_CURRENT, TRUE, 0);
		SendMessage(wi.score_left_wnd[!wParam], WM_LEFT_SET_CURRENT, FALSE, 0);
		break;

	case WM_WINDOW_SHOW_MENU:
		{
			POINT apos;

			// メニューの表示
			GetCursorPos((LPPOINT)&apos);
			TrackPopupMenu(GetSubMenu(hMenu, 2), TPM_TOPALIGN, apos.x, apos.y, 0, hWnd, NULL);
		}
		break;

	case WM_WINDOW_SET_FIRST:
		// 先攻の設定
		SendMessage(wi.score_name_wnd, WM_WINDOW_SET_FIRST, si.leg[si.current_leg].first, 0);
		SendMessage(wi.score_player_wnd[0], WM_WINDOW_SET_FIRST, (si.leg[si.current_leg].first == 0), 0);
		SendMessage(wi.score_player_wnd[1], WM_WINDOW_SET_FIRST, (si.leg[si.current_leg].first == 1), 0);
		if (wParam == TRUE) {
			SendMessage(wi.score_name_wnd, WM_NAME_REDRAW, 0, 0);
			SendMessage(wi.score_player_wnd[0], WM_PLAYER_REDRAW, 0, 0);
			SendMessage(wi.score_player_wnd[1], WM_PLAYER_REDRAW, 0, 0);
		}
		break;

	case WM_WINDOW_SET_KEY:
		// キー状態変更を通知
		SendMessage(wi.score_guide_wnd, WM_WINDOW_SET_KEY, 0, 0);
		break;

	case WM_WINDOW_NEXT_SET:
		// セットの移動
		if (si.set_mode == TRUE) {
			if (si.current_set + 1 >= op.gi_list_count) {
				TCHAR name[NAME_SIZE];

				SendMessage(wi.score_player_wnd[0], WM_PLAYER_REDRAW, 0, 0);
				SendMessage(wi.score_player_wnd[1], WM_PLAYER_REDRAW, 0, 0);

				if (score_history_set(hWnd, &sh, &si) == FALSE) {
					return FALSE;
				}
				if (si.player[0].sets == si.player[1].sets) {
					// 引き分け
					message_copy_res(IDS_STRING_TIE, buf);
				} else if (si.player[0].sets > si.player[1].sets) {
					// Player 1の勝利
					wsprintf(buf, message_get_res(IDS_STRING_SET_END),
						si.player[0].sets, si.player[1].sets, message_copy_res(IDS_STRING_OP_SCH_P1, name));
				} else {
					// Player 2の勝利
					wsprintf(buf, message_get_res(IDS_STRING_SET_END),
						si.player[0].sets, si.player[1].sets, message_copy_res(IDS_STRING_OP_SCH_P2, name));
				}
				MessageBox(hWnd, buf, APP_NAME, MB_ICONINFORMATION);
				EnableMenuItem(GetSubMenu(hMenu, 0), ID_MENUITEM_SET_SCHEDULE, MF_GRAYED);
				return FALSE;
			}
			if (score_history_set(hWnd, &sh, &si) == FALSE) {
				return FALSE;
			}
			si.current_set++;
			if (score_info_init(hWnd, &si, &op.gi_list[si.current_set], FALSE) == FALSE) {
				return FALSE;
			}
			SendMessage(wi.score_player_wnd[0], WM_PLAYER_DRAW_INIT, 0, 0);
			SendMessage(wi.score_player_wnd[1], WM_PLAYER_DRAW_INIT, 0, 0);
		} else {
			if (score_history_set(hWnd, &sh, &si) == FALSE) {
				return FALSE;
			}
			si.current_set++;
			if (score_info_init(hWnd, &si, &op.gi, FALSE) == FALSE) {
				return FALSE;
			}
		}
		SendMessage(wi.score_list_wnd, WM_SCORE_INIT_LEG, FALSE, FALSE);
		return TRUE;

	case WM_WINDOW_CLEAR_LIST:
		// スコア履歴をクリア
		score_history_free(&sh);
		break;

	case WM_WINDOW_DELETE_LIST:
		// 最新のスコア履歴を削除
		sh.list_count--;
		if (sh.list_count >= 0) {
			score_info_free(&sh.list[sh.list_count]);
		}
		break;

	case WM_WINDOW_SET_PREV_MENU:
		// 前のレッグに戻るメニューの設定
		EnableMenuItem(GetSubMenu(hMenu, 2), ID_MENUITEM_PREV_LEG, (wParam == TRUE) ? MF_ENABLED : MF_GRAYED);
		break;

	case WM_N01_GET_VERSION:
		// バージョン取得
		return APP_VERSION;

	case WM_N01_GET_OPTION:
		// オプション取得
		return (LRESULT)&op;

	case WM_N01_GET_GAME_OPTION:
		// ゲーム設定取得
		if (si.set_mode == TRUE) {
			return (LRESULT)&op.gi_list[si.current_set];
		}
		return (LRESULT)&op.gi;

	case WM_N01_GET_SCORE_INFO:
		// スコア情報取得
		return (LRESULT)&si;

	case WM_N01_GET_SCORE_HISTORY:
		// スコア履歴取得
		return (LRESULT)&sh;

	case WM_N01_GET_WINDOW:
		return (LRESULT)&wi;

	case WM_N01_GET_CSV:
		// CSV取得
		if (lParam != 0) {
			TCHAR *p;

			i = get_score_string_length((SCORE_INFO *)lParam);
			p = mem_alloc(sizeof(TCHAR) * (i + 1));
			if (p == NULL) {
				break;
			}
			get_score_string((SCORE_INFO *)lParam, p);
			return (LRESULT)p;
		}
		return (LRESULT)NULL;

	case WM_N01_FREE_CSV:
		// CSV解放
		if (lParam != 0) {
			mem_free(&((TCHAR *)lParam));
		}
		break;

	case WM_N01_REFRESH:
		// スコア更新
		SendMessage(wi.score_list_wnd, WM_SCORE_REFRESH, 0, 0);
		SendMessage(hWnd, WM_WINDOW_REDRAW, 0, TRUE);
		SendMessage(hWnd, WM_WINDOW_REDRAW, 1, FALSE);
		break;

	case WM_N01_GAME_ON:
		// ゲーム開始
		if (score_history_set(hWnd, &sh, &si) == FALSE) {
			return FALSE;
		}
		if (wParam == TRUE && op.gi_list_count <= 0) {
			return FALSE;
		}
		si.set_mode = wParam;
		si.current_set = 0;
		if (score_info_init(hWnd, &si, (wParam == TRUE) ? &op.gi_list[si.current_set] : &op.gi, TRUE) == FALSE) {
			return FALSE;
		}
		SendMessage(wi.score_name_wnd, WM_NAME_SET_MODE, (si.leg_limit == 1) ? TRUE : FALSE, 0);
		SendMessage(wi.score_player_wnd[0], WM_PLAYER_SET_MODE, (si.leg_limit == 1) ? TRUE : FALSE, 0);
		SendMessage(wi.score_player_wnd[1], WM_PLAYER_SET_MODE, (si.leg_limit == 1) ? TRUE : FALSE, 0);
		SendMessage(wi.score_list_wnd, WM_SCORE_INIT_LEG, TRUE, TRUE);
		EnableMenuItem(GetSubMenu(hMenu, 0), ID_MENUITEM_SET_SCHEDULE, (wParam == TRUE) ? MF_ENABLED : MF_GRAYED);
		return TRUE;

	case WM_N01_GET_SCORE_BUFFER:
		return SendMessage(wi.score_list_wnd, WM_SCORE_GET_BUFFER, wParam, lParam);

	case WM_N01_GET_INPUT_POS:
	case WM_N01_SET_INPUT_POS:
	case WM_N01_SCORE_INPUT:
		return SendMessage(wi.score_list_wnd, msg, wParam, lParam);

	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

/*
 * get_path - パスの作成
 */
static void get_path(const HINSTANCE hInstance)
{
	TCHAR *p, *r;

	// アプリケーションのパスを取得
	GetModuleFileName(hInstance, work_path, MAX_PATH - 1);
	for (p = r = work_path; *p != TEXT('\0'); p++) {
#ifndef UNICODE
		if (IsDBCSLeadByte((BYTE)*p) == TRUE) {
			p++;
			continue;
		}
#endif	// UNICODE
		if (*p == TEXT('\\') || *p == TEXT('/')) {
			r = p;
		}
	}
	*r = TEXT('\0');
	// iniパスを作成
	wsprintf(ini_path, TEXT("%s\\%s"), work_path, INI_FILE);
}

/*
 * init_application - ウィンドウクラスの登録
 */
static BOOL init_application(const HINSTANCE hInstance)
{
	WNDCLASS wc;

	wc.style = 0;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.lpszMenuName = MAKEINTRESOURCE(IDR_MENU_MAIN);
	wc.lpfnWndProc = (WNDPROC)main_proc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON_MAIN));
	wc.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);
	wc.lpszClassName = MAIN_WND_CLASS;
	// ウィンドウクラスの登録
	return RegisterClass(&wc);
}

/*
 * init_instance - ウィンドウの作成
 */
static HWND init_instance(const HINSTANCE hInstance, const int CmdShow)
{
	HWND hWnd = NULL;
	RECT rect;
	DWORD dwStyle;

	// ウィンドウサイズの設定
	if (op.window_rect.left == 0 && op.window_rect.top == 0 &&
		op.window_rect.right == 0 && op.window_rect.bottom == 0) {
		SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0);
		rect.right -= rect.left;
		rect.bottom -= rect.top;
	} else {
		SetRect(&rect, op.window_rect.left, op.window_rect.top, 
			op.window_rect.right, op.window_rect.bottom);
	}

	if (op.full_screen == TRUE) {
		dwStyle = WS_POPUP;
	} else {
		dwStyle = WS_OVERLAPPEDWINDOW;
	}
	// ウィンドウの作成
	hWnd = CreateWindowEx(0,
		MAIN_WND_CLASS,
		WINDOW_TITLE,
		dwStyle,
		rect.left,
		rect.top,
		rect.right,
		rect.bottom,
		NULL, NULL, hInstance, NULL);
	if (hWnd == NULL) {
		return NULL;
	}
	hMenu = GetMenu(hWnd);
	if (op.full_screen && op.full_screen_menu_interval >= 0) {
		SetMenu(hWnd, NULL);
	}
	// ウィンドウの表示
	ShowWindow(hWnd, (op.full_screen == TRUE) ? SW_MAXIMIZE : CmdShow);
	UpdateWindow(hWnd);
	return hWnd;
}

/*
 * WinMain - メイン
 */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	MSG msg;
	HWND hWnd;
	HANDLE hAccel;
	TCHAR path[MAX_PATH];
	TCHAR err_str[BUF_SIZE];

	hInst = hInstance;

	if (stricmp(lpCmdLine, "/shortcut") == 0) {
		LPMALLOC pMalloc;
		LPITEMIDLIST pItemIDList;
		TCHAR app_path[MAX_PATH];
		TCHAR sp_path[MAX_PATH];
		TCHAR link_path[MAX_PATH];

		//OLE初期化
		CoInitialize(NULL);
		// アプリケーションのパスを取得
		GetModuleFileName(hInstance, app_path, MAX_PATH - 1);
		// デスクトップのパスを取得
		*sp_path = TEXT('\0');
		if (SHGetMalloc(&pMalloc) == NOERROR) {
			if (SHGetSpecialFolderLocation(NULL, CSIDL_COMMON_DESKTOPDIRECTORY, &pItemIDList) == NOERROR) {
				SHGetPathFromIDList(pItemIDList, sp_path);
				pMalloc->lpVtbl->Free(pMalloc, pItemIDList);
			}
			pMalloc->lpVtbl->Release(pMalloc);
		}
		if (*sp_path == TEXT('\0')) {
			if (SHGetMalloc(&pMalloc) == NOERROR) {
				if (SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOP, &pItemIDList) == NOERROR) {
					SHGetPathFromIDList(pItemIDList, sp_path);
					pMalloc->lpVtbl->Free(pMalloc, pItemIDList);
				}
				pMalloc->lpVtbl->Release(pMalloc);
			}
		}
		if (*sp_path != TEXT('\0')) {
			if (*(sp_path + lstrlen(sp_path) - 1) == TEXT('\\')) {
				wsprintf(link_path, TEXT("%s%s"), sp_path, LNK_FILE);
			} else {
				wsprintf(link_path, TEXT("%s\\%s"), sp_path, LNK_FILE);
			}
			// ショートカットファイルの作成
			create_shortcut(app_path, link_path);
		}
		CoUninitialize();
		return 0;
	}

	// CommonControlの初期化
	InitCommonControls();

	// 設定取得
	get_path(hInst);
	if (ini_get_option(ini_path) == FALSE) {
		message_get_error(GetLastError(), err_str);
		MessageBox(NULL, err_str, APP_NAME, MB_ICONERROR);
		return 0;
	}

	// アレンジの取得
	wsprintf(path, TEXT("%s\\arrange.txt"), work_path);
	op.arrange_info = file_read_arrange(path, &op.arrange_info_count);
	if (op.arrange_info == NULL) {
		op.arrange_info = res_to_arrange(hInst, IDR_TEXT_ARRANGE, &op.arrange_info_count);
	}
#ifdef _DEBUG
	arrange_check();
#endif
	wsprintf(path, TEXT("%s\\arrange_com.txt"), work_path);
	op.com_arrange_info = file_read_arrange(path, &op.com_arrange_info_count);
	if (op.com_arrange_info == NULL) {
		op.com_arrange_info = res_to_arrange(hInst, IDR_TEXT_ARRANGE_COM, &op.com_arrange_info_count);
	}

	// ウィンドウクラスの登録
	if (nedit_regist(hInstance) == FALSE ||
		score_name_regist(hInstance) == FALSE ||
		score_list_regist(hInstance) == FALSE ||
		score_left_regist(hInstance) == FALSE ||
		score_player_regist(hInstance) == FALSE ||
		score_guide_regist(hInstance) == FALSE) {
		message_get_error(GetLastError(), err_str);
		MessageBox(NULL, err_str, APP_NAME, MB_ICONERROR);
		return 0;
	}

	// ウィンドウクラスの登録
	if (init_application(hInstance) == FALSE) {
		message_get_error(GetLastError(), err_str);
		MessageBox(NULL, err_str, APP_NAME, MB_ICONERROR);
		return 0;
	}
	// ウィンドウの作成
	if ((hWnd = init_instance(hInstance, op.window_state)) == NULL) {
		return 0;
	}
	create_accelerator();
	plugin_create_accelerator();
	hAccel = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR));
	// ウィンドウメッセージ処理
	while (GetMessage(&msg, NULL, 0, 0) == TRUE) {
		if (hKeyAccel != NULL && TranslateAccelerator(hWnd, hKeyAccel, &msg) == TRUE) {
			continue;
		}
		if (hPluginAccel != NULL && TranslateAccelerator(hWnd, hPluginAccel, &msg) == TRUE) {
			continue;
		}
		if (TranslateAccelerator(hWnd, hAccel, &msg) == TRUE) {
			continue;
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	free_arrange(&op.arrange_info, &op.arrange_info_count);
	free_arrange(&op.com_arrange_info, &op.com_arrange_info_count);
	free_accelerator();
	plugin_free_accelerator();
	mem_free(&op.gi_list);
	mem_free(&op.key_info);
	mem_free(&op.plugin_info);

#ifdef _DEBUG
	mem_debug();
#endif
	return msg.wParam;
}
/* End of source */
