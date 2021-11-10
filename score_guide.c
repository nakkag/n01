/*
 * n01
 *
 * score_guide.c
 *
 * Copyright (C) 1996-2016 by Ohno Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

/* Include Files */
#define _INC_OLE
#include <windows.h>
#undef	_INC_OLE
#include <commctrl.h>
#include <tchar.h>

#include "general.h"
#include "Memory.h"
#include "String.h"
#include "Message.h"
#include "font.h"
#include "score_guide.h"

#include "resource.h"

/* Define */
#define WINDOW_CLASS				TEXT("score_guide_wnd")

#define BOX_COUNT					6
#define CHAR_COUNT					(7 * BOX_COUNT)

#define ID_TIMER_KEY				1
#define TIMER_INTERVAL_KEY			100

/* Global Variables */
extern HINSTANCE hInst;
extern OPTION_INFO op;

typedef struct _DRAW_BUFFER {
	HDC draw_dc;
	HBITMAP draw_bmp;
	HBITMAP draw_ret_bmp;

	HFONT guide_font;
	HFONT ret_font;

	HBRUSH back_brush;
	HBRUSH box_back_brush;
	HBRUSH box_select_brush;

	HPEN box_frame_pen;
	HPEN box_dis_frame_pen;

	BOOL mousedown;
	int sel_index;
	int mouse_index;

	HWND hToolTip;
	int tip_index;

	int key;
} DRAW_BUFFER;

/* Local Function Prototypes */
static BOOL draw_init(const HWND hWnd, DRAW_BUFFER *bf);
static BOOL draw_free(const HWND hWnd, DRAW_BUFFER *bf);
static BOOL draw_guide(const HWND hWnd, const DRAW_BUFFER *bf);
static HWND create_tooltip(const HWND hWnd);
static LRESULT CALLBACK score_guide_proc(const HWND hWnd, const UINT msg, WPARAM wParam, LPARAM lParam);

/*
 * draw_init - 描画情報の初期化
 */
static BOOL draw_init(const HWND hWnd, DRAW_BUFFER *bf)
{
	HDC hdc;
	RECT rect;
	int size;
	int height;

	GetClientRect(hWnd, &rect);

	hdc = GetDC(hWnd);
	bf->draw_bmp = CreateCompatibleBitmap(hdc, rect.right, rect.bottom);
	bf->draw_ret_bmp = SelectObject(bf->draw_dc, bf->draw_bmp);
	ReleaseDC(hWnd, hdc);

	// フォントの作成
	size = rect.bottom / 2;
	height = rect.bottom / 2 - 6;
	while (1) {
		TEXTMETRIC tm;
		HFONT hfont, ret_font;

		hfont = font_create_menu(size, 0, FALSE);
		ret_font = SelectObject(bf->draw_dc, hfont);
		GetTextMetrics(bf->draw_dc, &tm);
		if (size <= 0 || height <= 0 || tm.tmHeight <= height) {
			SelectObject(bf->draw_dc, ret_font);
			DeleteObject(hfont);
			break;
		}
		SelectObject(bf->draw_dc, ret_font);
		DeleteObject(hfont);
		size--;
	}

	bf->guide_font = font_create_menu(size, 0, FALSE);
	bf->ret_font = SelectObject(bf->draw_dc, bf->guide_font);
	return TRUE;
}

/*
 * draw_free - 描画情報の解放
 */
static BOOL draw_free(const HWND hWnd, DRAW_BUFFER *bf)
{
	if (bf->draw_dc != NULL) {
		SelectObject(bf->draw_dc, bf->ret_font);
		SelectObject(bf->draw_dc, bf->draw_ret_bmp);
		DeleteObject(bf->draw_bmp);
		bf->draw_bmp = NULL;
	}
	if (bf->guide_font != NULL) {
		DeleteObject(bf->guide_font);
		bf->guide_font = NULL;
	}
	return TRUE;
}

/*
 * draw_guide - ガイドの描画
 */
static BOOL draw_guide(const HWND hWnd, const DRAW_BUFFER *bf)
{
	RECT draw_rect, rect;
	HPEN ret_pen;
	HBRUSH ret_brush;
	SIZE sz;
	TCHAR buf[BUF_SIZE];
	int left, right;
	int i, j;
	int height;
	int key_width;

	GetClientRect(hWnd, &rect);
	FillRect(bf->draw_dc, &rect, bf->back_brush);

	GetTextExtentPoint32(bf->draw_dc, TEXT("F188"), lstrlen(TEXT("F188")), &sz);
	key_width = sz.cx;

	left = 0;
	height = rect.bottom / 2 - 2;
	rect.top += 2;
	for (j = 0; j < 12; j++) {
		if (j == BOX_COUNT) {
			rect.top += height + 1;
			left = 0;
		}
		for (i = 0; i < op.key_info_count; i++) {
			if (op.key_info[i].key == VK_F1 + j && op.key_info[i].ctrl == bf->key) {
				break;
			}
		}
		// 選択
		if (bf->sel_index - 1 == j && i < op.key_info_count) {
			SetRect(&draw_rect, left, rect.top, left + rect.right / BOX_COUNT, rect.top + height);
			FillRect(bf->draw_dc, &draw_rect, bf->box_select_brush);
		}
		// キー
		if (i < op.key_info_count) {
			ret_pen = SelectObject(bf->draw_dc, bf->box_frame_pen);
			ret_brush = SelectObject(bf->draw_dc, bf->box_back_brush);
			SetTextColor(bf->draw_dc, op.ci.guide_box_text);
			SetBkColor(bf->draw_dc, op.ci.guide_box_background);
		} else {
			ret_pen = SelectObject(bf->draw_dc, bf->box_dis_frame_pen);
			ret_brush = SelectObject(bf->draw_dc, bf->back_brush);
			SetTextColor(bf->draw_dc, op.ci.guide_box_disable);
			SetBkColor(bf->draw_dc, op.ci.guide_background);
		}
		SetRect(&draw_rect, left + 1, rect.top + 1, left + key_width, rect.top + height - 1);
		// キー枠の描画
		RoundRect(bf->draw_dc, draw_rect.left, draw_rect.top, draw_rect.right, draw_rect.bottom, key_width / 6, key_width / 6);
		SelectObject(bf->draw_dc, ret_pen);
		SelectObject(bf->draw_dc, ret_brush);
		// キー名の描画
		wsprintf(buf, TEXT("F%d"), j + 1);
		DrawText(bf->draw_dc, buf, lstrlen(buf), &draw_rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

		// アクション
		right = left + rect.right / BOX_COUNT;
		left += key_width + 2;
		for (i = 0; i < op.key_info_count; i++) {
			if (op.key_info[i].key != VK_F1 + j || op.key_info[i].ctrl != bf->key) {
				continue;
			}
			if (bf->sel_index - 1 == j) {
				SetTextColor(bf->draw_dc, op.ci.guide_select_text);
				SetBkColor(bf->draw_dc, op.ci.guide_select_background);
			} else {
				SetTextColor(bf->draw_dc, op.ci.guide_text);
				SetBkColor(bf->draw_dc, op.ci.guide_background);
			}
			if (op.key_info[i].action >= ID_ACCEL_INPUT_SCORE && op.key_info[i].action <= ID_ACCEL_INPUT_SCORE + 180) {
				_itot(op.key_info[i].action - ID_ACCEL_INPUT_SCORE, buf, 10);
			} else {
				str_noprefix_cpy(buf, message_get_res(op.key_info[i].action));
			}
			SetRect(&draw_rect, left, rect.top, right, rect.top + height);
			DrawText(bf->draw_dc, buf, lstrlen(buf), &draw_rect, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
			break;
		}
		left = right;
	}
	return TRUE;
}

/*
 * create_tooltip - ツールチップの作成
 */
static HWND create_tooltip(const HWND hWnd)
{
	TOOLINFO ti;
	HWND hToolTip;

	hToolTip = CreateWindowEx(0, TOOLTIPS_CLASS, NULL, TTS_ALWAYSTIP,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		hWnd, NULL, hInst, NULL);

	ZeroMemory(&ti, sizeof(TOOLINFO));
	ti.cbSize = sizeof(TOOLINFO);
	ti.hwnd = hWnd;
	ti.hinst = NULL;
	ti.uFlags = TTF_IDISHWND;
	ti.uId = (UINT)hWnd;
	ti.lpszText = LPSTR_TEXTCALLBACK;
	SendMessage(hToolTip, TTM_ADDTOOL, 0, (LPARAM)&ti);
	SendMessage(hToolTip, TTM_SETDELAYTIME, TTDT_INITIAL, MAKELONG(2000, 0));
	return hToolTip;
}

/*
 * score_guide_proc - ガイド表示ウィンドウプロシージャ
 */
static LRESULT CALLBACK score_guide_proc(const HWND hWnd, const UINT msg, WPARAM wParam, LPARAM lParam)
{
	DRAW_BUFFER *bf;
	HDC hdc;
	PAINTSTRUCT ps;
	RECT rect;
	MSG _msg;
	int i;

	switch (msg) {
	case WM_CREATE:
		bf = (DRAW_BUFFER *)mem_calloc(sizeof(DRAW_BUFFER));
		if (bf == NULL) {
			return -1;
		}
		bf->hToolTip = create_tooltip(hWnd);

		// 描画用情報
		hdc = GetDC(hWnd);
		bf->draw_dc = CreateCompatibleDC(hdc);
		ReleaseDC(hWnd, hdc);
		draw_init(hWnd, bf);
		bf->back_brush = CreateSolidBrush(op.ci.guide_background);
		bf->box_back_brush = CreateSolidBrush(op.ci.guide_box_background);
		bf->box_select_brush = CreateSolidBrush(op.ci.guide_select_background);
		bf->box_frame_pen = CreatePen(PS_SOLID, 1, op.ci.guide_box_frame);
		bf->box_dis_frame_pen = CreatePen(PS_SOLID, 1, op.ci.guide_box_disable);
		draw_guide(hWnd, bf);

		SetWindowLong(hWnd, GWL_USERDATA, (LONG)bf);
		break;

	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;

	case WM_DESTROY:
		bf = (DRAW_BUFFER *)GetWindowLong(hWnd, GWL_USERDATA);
		if (bf != NULL) {
			draw_free(hWnd, bf);
			if (bf->draw_dc != NULL) {
				DeleteDC(bf->draw_dc);
				bf->draw_dc = NULL;
			}
			DeleteObject(bf->back_brush);
			DeleteObject(bf->box_back_brush);
			DeleteObject(bf->box_select_brush);
			DeleteObject(bf->box_frame_pen);
			DeleteObject(bf->box_dis_frame_pen);
			mem_free(&bf);
		}
		return DefWindowProc(hWnd, msg, wParam, lParam);

	case WM_SIZE:
		bf = (DRAW_BUFFER *)GetWindowLong(hWnd, GWL_USERDATA);
		if (bf == NULL) {
			break;
		}
		draw_free(hWnd, bf);
		draw_init(hWnd, bf);
		draw_guide(hWnd, bf);
		InvalidateRect(hWnd, NULL, FALSE);
		UpdateWindow(hWnd);
		break;

	case WM_PAINT:
		bf = (DRAW_BUFFER *)GetWindowLong(hWnd, GWL_USERDATA);
		if (bf == NULL) {
			break;
		}
		hdc = BeginPaint(hWnd, &ps);

		BitBlt(hdc, ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right, ps.rcPaint.bottom,
			bf->draw_dc, ps.rcPaint.left, ps.rcPaint.top, SRCCOPY);

		EndPaint(hWnd, &ps);
		break;

	case WM_ERASEBKGND:
		return TRUE;

	case WM_LBUTTONDOWN:
		bf = (DRAW_BUFFER *)GetWindowLong(hWnd, GWL_USERDATA);
		if (bf == NULL) {
			break;
		}
		SetCapture(hWnd);
		bf->mousedown = TRUE;

		GetClientRect(hWnd, &rect);
		bf->mouse_index = bf->sel_index = LOWORD(lParam) / (rect.right / BOX_COUNT) + (HIWORD(lParam) / (rect.bottom / 2)) * BOX_COUNT + 1;
		SendMessage(hWnd, WM_GUIDE_REDRAW, 0, 0);
		break;

	case WM_MOUSEMOVE:
		bf = (DRAW_BUFFER *)GetWindowLong(hWnd, GWL_USERDATA);
		if (bf == NULL) {
			break;
		}
		GetClientRect(hWnd, &rect);
		i = LOWORD(lParam) / (rect.right / BOX_COUNT) + (HIWORD(lParam) / (rect.bottom / 2)) * BOX_COUNT + 1;
		if (bf->tip_index != i) {
			bf->tip_index = i;
			// ツールチップを非表示
			SendMessage(bf->hToolTip, TTM_POP, 0, 0);
			SendMessage(bf->hToolTip, TTM_ACTIVATE, FALSE, 0);
		}
		// ツールチップを表示
		_msg.hwnd = hWnd;
		_msg.message = msg;
		_msg.wParam = wParam;
		_msg.lParam = lParam;
		SendMessage(bf->hToolTip, TTM_RELAYEVENT, 0, (LPARAM)&_msg);
		SendMessage(bf->hToolTip, TTM_ACTIVATE, TRUE, 0);

		if (bf->mousedown == FALSE) {
			break;
		}
		bf->sel_index = i;
		if (bf->sel_index != bf->mouse_index) {
			bf->sel_index = 0;
		}
		SendMessage(hWnd, WM_GUIDE_REDRAW, 0, 0);
		break;

	case WM_LBUTTONUP:
		bf = (DRAW_BUFFER *)GetWindowLong(hWnd, GWL_USERDATA);
		if (bf == NULL) {
			break;
		}
		if (bf->mousedown == FALSE) {
			break;
		}
		ReleaseCapture();
		bf->mousedown = FALSE;

		GetClientRect(hWnd, &rect);
		bf->sel_index = LOWORD(lParam) / (rect.right / BOX_COUNT) + (HIWORD(lParam) / (rect.bottom / 2)) * BOX_COUNT + 1;
		if (bf->sel_index == bf->mouse_index) {
			for (i = 0; i < op.key_info_count; i++) {
				if (op.key_info[i].key == VK_F1 + bf->sel_index - 1 && op.key_info[i].ctrl == bf->key) {
					SendMessage(GetParent(hWnd), WM_COMMAND, op.key_info[i].action, 0);
					break;
				}
			}
		}
		bf->mouse_index = bf->sel_index = 0;
		SendMessage(hWnd, WM_GUIDE_REDRAW, 0, 0);
		break;

	case WM_NOTIFY:
		if (((NMHDR *)lParam)->code == TTN_NEEDTEXT) {
			TOOLTIPTEXT *ti = (TOOLTIPTEXT *)lParam;
			POINT pt;
			int j;

			bf = (DRAW_BUFFER *)GetWindowLong(hWnd, GWL_USERDATA);
			if (bf == NULL) {
				break;
			}
			// ツールチップに表示する文字列の設定
			GetClientRect(hWnd, &rect);
			GetCursorPos(&pt);
			ScreenToClient(hWnd, &pt);
			j = pt.x / (rect.right / BOX_COUNT) + (pt.y / (rect.bottom / 2)) * BOX_COUNT;
			for (i = 0; i < op.key_info_count; i++) {
				if (op.key_info[i].key == VK_F1 + j && op.key_info[i].ctrl == bf->key) {
					if (op.key_info[i].action >= ID_ACCEL_INPUT_SCORE && op.key_info[i].action <= ID_ACCEL_INPUT_SCORE + 180) {
						_itot(op.key_info[i].action - ID_ACCEL_INPUT_SCORE, ti->szText, 10);
					} else {
						str_noprefix_cpy(ti->szText, message_get_res(op.key_info[i].action));
					}
					break;
				}
			}
		}
		break;

	case WM_TIMER:
		switch (wParam) {
		case ID_TIMER_KEY:
			bf = (DRAW_BUFFER *)GetWindowLong(hWnd, GWL_USERDATA);
			if (bf == NULL) {
				break;
			}
			i = ((GetKeyState(VK_SHIFT) < 0) ? FSHIFT : 0) |
				((GetKeyState(VK_CONTROL) < 0) ? FCONTROL : 0) |
				((GetKeyState(VK_MENU) < 0) ? FALT : 0);
			if (bf->key != i) {
				bf->key = i;
				bf->mouse_index = bf->sel_index = 0;
				SendMessage(hWnd, WM_GUIDE_REDRAW, 0, 0);
			}
			if (i == 0) {
				KillTimer(hWnd, ID_TIMER_KEY);
			}
			break;
		}
		break;

	case WM_WINDOW_SET_KEY:
		SetTimer(hWnd, ID_TIMER_KEY, TIMER_INTERVAL_KEY, NULL);
		SendMessage(hWnd, WM_TIMER, ID_TIMER_KEY, 0);
		break;

	case WM_GUIDE_REDRAW:
		bf = (DRAW_BUFFER *)GetWindowLong(hWnd, GWL_USERDATA);
		if (bf == NULL) {
			break;
		}
		draw_guide(hWnd, bf);
		InvalidateRect(hWnd, NULL, FALSE);
		UpdateWindow(hWnd);
		break;

	case WM_GUIDE_GET_HEIGHT:
		return wParam / 10;

	case WM_GUIDE_DRAW_INIT:
		bf = (DRAW_BUFFER *)GetWindowLong(hWnd, GWL_USERDATA);
		if (bf == NULL) {
			break;
		}
		draw_free(hWnd, bf);
		DeleteObject(bf->back_brush);
		DeleteObject(bf->box_back_brush);
		DeleteObject(bf->box_select_brush);
		DeleteObject(bf->box_frame_pen);
		DeleteObject(bf->box_dis_frame_pen);

		draw_init(hWnd, bf);
		bf->back_brush = CreateSolidBrush(op.ci.guide_background);
		bf->box_back_brush = CreateSolidBrush(op.ci.guide_box_background);
		bf->box_select_brush = CreateSolidBrush(op.ci.guide_select_background);
		bf->box_frame_pen = CreatePen(PS_SOLID, 1, op.ci.guide_box_frame);
		bf->box_dis_frame_pen = CreatePen(PS_SOLID, 1, op.ci.guide_box_disable);
		SendMessage(hWnd, WM_GUIDE_REDRAW, 0, 0);
		break;

	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

/*
 * score_guide_regist - ウィンドウクラスの登録
 */
BOOL score_guide_regist(const HINSTANCE hInstance)
{
	WNDCLASS wc;

	wc.style = CS_DBLCLKS;
	wc.lpfnWndProc = (WNDPROC)score_guide_proc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = WINDOW_CLASS;
	// ウィンドウクラスの登録
	return RegisterClass(&wc);
}

/*
 * score_guide_create - ウィンドウの作成
 */
HWND score_guide_create(const HINSTANCE hInstance, const HWND pWnd, int id)
{
	HWND hWnd;

	// ウィンドウの作成
	hWnd = CreateWindow(WINDOW_CLASS,
		TEXT(""),
		WS_CHILD,
		0, 0, 0, 0, pWnd, (HMENU)id, hInstance, 0);
	return hWnd;
}
/* End of source */
