/*
 * n01
 *
 * score_name.c
 *
 * Copyright (C) 1996-2015 by Ohno Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

/* Include Files */
#define _INC_OLE
#include <windows.h>
#undef	_INC_OLE
#include <tchar.h>
#include <stdio.h>

#include "general.h"
#include "Memory.h"
#include "String.h"
#include "Message.h"
#include "font.h"
#include "score_name.h"

#include "resource.h"

/* Define */
#define WINDOW_CLASS					TEXT("score_name_wnd")

/* Global Variables */
extern HINSTANCE hInst;
extern OPTION_INFO op;

typedef struct _DRAW_BUFFER {
	HDC name_dc;
	HBITMAP name_bmp;
	HBITMAP name_ret_bmp;

	HFONT name_font;
	HFONT info_font;
	int font_width;

	HBRUSH back_brush;
	HBRUSH name_back_brush;

	PLAYER_INFO *p1;
	PLAYER_INFO *p2;

	int first;
	BOOL set_mode;
} DRAW_BUFFER;

/* Local Function Prototypes */
static BOOL draw_init(const HWND hWnd, DRAW_BUFFER *bf);
static BOOL draw_free(const HWND hWnd, DRAW_BUFFER *bf);
static BOOL draw_name(const HWND hWnd, DRAW_BUFFER *bf);
static LRESULT CALLBACK score_name_proc(const HWND hWnd, const UINT msg, WPARAM wParam, LPARAM lParam);

/*
 * draw_init - 描画情報の初期化
 */
static BOOL draw_init(const HWND hWnd, DRAW_BUFFER *bf)
{
	HDC hdc;
	RECT rect;
	HFONT ret_font;
	TEXTMETRIC tm;
	int font_size;
	int height;

	GetClientRect(hWnd, &rect);
	hdc = GetDC(hWnd);

	// 名前用フォント
	font_size = rect.bottom;
	height = rect.bottom - 2;
	while (1) {
		TEXTMETRIC tm;
		HFONT hfont, ret_font;

		hfont = font_create_menu(font_size, FW_BOLD, FALSE);
		ret_font = SelectObject(bf->name_dc, hfont);
		GetTextMetrics(bf->name_dc, &tm);
		if (font_size <= 0 || height <= 0 || tm.tmHeight <= height) {
			SelectObject(bf->name_dc, ret_font);
			DeleteObject(hfont);
			break;
		}
		SelectObject(bf->name_dc, ret_font);
		DeleteObject(hfont);
		font_size--;
	}

	bf->name_font = font_create_menu(font_size, FW_BOLD, FALSE);
	ret_font = SelectObject(hdc, bf->name_font);
	GetTextMetrics(hdc, &tm);
	bf->font_width = tm.tmAveCharWidth;
	SelectObject(hdc, ret_font);

	bf->info_font = font_create_menu(font_size, FW_BOLD, FALSE);

	bf->name_bmp = CreateCompatibleBitmap(hdc, rect.right, rect.bottom);
	bf->name_ret_bmp = SelectObject(bf->name_dc, bf->name_bmp);
	// 名前の描画
	draw_name(hWnd, bf);

	return TRUE;
}

/*
 * draw_free - 描画情報の解放
 */
static BOOL draw_free(const HWND hWnd, DRAW_BUFFER *bf)
{
	if (bf->name_dc != NULL) {
		SelectObject(bf->name_dc, bf->name_ret_bmp);
		DeleteObject(bf->name_bmp);
		bf->name_bmp = NULL;
	}
	if (bf->name_font != NULL) {
		DeleteObject(bf->name_font);
		bf->name_font = NULL;
	}
	if (bf->info_font != NULL) {
		DeleteObject(bf->info_font);
		bf->info_font = NULL;
	}
	return TRUE;
}

/*
 * draw_name - プレイヤー名の描画
 */
static BOOL draw_name(const HWND hWnd, DRAW_BUFFER *bf)
{
	RECT draw_rect, rect;
	HFONT ret_font;
	TCHAR buf[BUF_SIZE], *p;
	SIZE sz;
	int csize = 0;

	if (bf->p1 == NULL || bf->p2 == NULL) {
		return FALSE;
	}
	GetClientRect(hWnd, &rect);
	FillRect(bf->name_dc, &rect, bf->name_back_brush);

	SetTextColor(bf->name_dc, op.ci.player_name_text);
	SetBkColor(bf->name_dc, op.ci.player_name_background);

	if (op.view_player == 0) {
		*buf = TEXT('\0');
		if (bf->set_mode == TRUE && op.opi.total_sets != 0) {
			if (op.opi.total_legs != 0) {
				wsprintf(buf, TEXT("(%d) %d - %d (%d)"), bf->p1->sets, bf->p1->legs, bf->p2->legs, bf->p2->sets);
			} else {
				wsprintf(buf, TEXT("(%d) - (%d)"), bf->p1->sets, bf->p2->sets);
			}
		} else if (op.opi.total_legs != 0) {
			wsprintf(buf, TEXT("%d - %d"), bf->p1->legs, bf->p2->legs);
		}
		if (*buf != TEXT('\0')) {
			ret_font = SelectObject(bf->name_dc, bf->info_font);
			GetTextExtentPoint32(bf->name_dc, buf, lstrlen(buf), &sz);
			csize = sz.cx + bf->font_width / 2;

			SetRect(&draw_rect, 0, 0, rect.right, rect.bottom);
			DrawText(bf->name_dc, buf, lstrlen(buf), &draw_rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			SelectObject(bf->name_dc, ret_font);
		}
	}

	ret_font = SelectObject(bf->name_dc, bf->name_font);
	if (op.opi.first != 0 && op.view_player == 0 && bf->first == 0) {
		message_copy_res(IDS_STRING_FIRST_MARK, buf);
		lstrcat(buf, TEXT(" "));
		p = buf + lstrlen(buf);
	} else {
		p = buf;
	}
	if (bf->p1->com == TRUE) {
		wsprintf(p, message_get_res(IDS_STRING_COM), bf->p1->level + 1);
	} else {
		lstrcpy(p, bf->p1->name);
	}
	SetRect(&draw_rect, bf->font_width / 2, 0, rect.right / 2 - bf->font_width / 2 - csize / 2, rect.bottom);
	DrawText(bf->name_dc, buf, lstrlen(buf), &draw_rect, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

	if (op.opi.first != 0 && op.view_player == 0 && bf->first == 1) {
		message_copy_res(IDS_STRING_FIRST_MARK, buf);
		lstrcat(buf, TEXT(" "));
		p = buf + lstrlen(buf);
	} else {
		p = buf;
	}
	if (bf->p2->com == TRUE) {
		wsprintf(p, message_get_res(IDS_STRING_COM), bf->p2->level + 1);
	} else {
		lstrcpy(p, bf->p2->name);
	}
	SetRect(&draw_rect, rect.right / 2 + csize / 2, 0, rect.right - bf->font_width / 2, rect.bottom);
	DrawText(bf->name_dc, buf, lstrlen(buf), &draw_rect, DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

	SelectObject(bf->name_dc, ret_font);
	return TRUE;
}

/*
 * score_name_proc - プレイヤー情報表示ウィンドウプロシージャ
 */
static LRESULT CALLBACK score_name_proc(const HWND hWnd, const UINT msg, WPARAM wParam, LPARAM lParam)
{
	static STATISTICS_INFO tmp_stat;
	DRAW_BUFFER *bf;
	SCORE_INFO *si;
	HDC hdc;
	PAINTSTRUCT ps;

	switch (msg) {
	case WM_CREATE:
		si = (SCORE_INFO *)((CREATESTRUCT *)lParam)->lpCreateParams;
		bf = (DRAW_BUFFER *)mem_calloc(sizeof(DRAW_BUFFER));
		if (bf == NULL) {
			return -1;
		}
		bf->p1 = &si->player[0];
		bf->p2 = &si->player[1];

		// 描画用情報
		hdc = GetDC(hWnd);
		bf->name_dc = CreateCompatibleDC(hdc);
		ReleaseDC(hWnd, hdc);
		bf->back_brush = CreateSolidBrush(op.ci.player_background);
		bf->name_back_brush = CreateSolidBrush(op.ci.player_name_background);
		draw_init(hWnd, bf);
		draw_name(hWnd, bf);

		SetWindowLong(hWnd, GWL_USERDATA, (LONG)bf);
		break;

	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;

	case WM_DESTROY:
		bf = (DRAW_BUFFER *)GetWindowLong(hWnd, GWL_USERDATA);
		if (bf != NULL) {
			draw_free(hWnd, bf);
			if (bf->name_dc != NULL) {
				DeleteDC(bf->name_dc);
				bf->name_dc = NULL;
			}
			DeleteObject(bf->back_brush);
			DeleteObject(bf->name_back_brush);
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
		draw_name(hWnd, bf);
		InvalidateRect(hWnd, NULL, FALSE);
		UpdateWindow(hWnd);
		break;

	case WM_PAINT:
		bf = (DRAW_BUFFER *)GetWindowLong(hWnd, GWL_USERDATA);
		if (bf == NULL) {
			break;
		}
		hdc = BeginPaint(hWnd, &ps);

		// プレイヤー名の描画
		BitBlt(hdc, ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right, ps.rcPaint.bottom,
			bf->name_dc, ps.rcPaint.left, ps.rcPaint.top, SRCCOPY);

		EndPaint(hWnd, &ps);
		break;

	case WM_ERASEBKGND:
		return TRUE;

	case WM_NAME_REDRAW:
		bf = (DRAW_BUFFER *)GetWindowLong(hWnd, GWL_USERDATA);
		if (bf == NULL) {
			break;
		}
		draw_name(hWnd, bf);
		InvalidateRect(hWnd, NULL, FALSE);
		UpdateWindow(hWnd);
		break;

	case WM_NAME_DRAW_INIT:
		bf = (DRAW_BUFFER *)GetWindowLong(hWnd, GWL_USERDATA);
		if (bf == NULL) {
			break;
		}
		DeleteObject(bf->back_brush);
		DeleteObject(bf->name_back_brush);
		bf->back_brush = CreateSolidBrush(op.ci.player_background);
		bf->name_back_brush = CreateSolidBrush(op.ci.player_name_background);
		draw_free(hWnd, bf);
		draw_init(hWnd, bf);
		SendMessage(hWnd, WM_NAME_REDRAW, 0, 0);
		break;

	case WM_NAME_GET_HEIGHT:
		return wParam / op.view_name_size;

	case WM_WINDOW_SET_FIRST:
		bf = (DRAW_BUFFER *)GetWindowLong(hWnd, GWL_USERDATA);
		if (bf == NULL) {
			break;
		}
		bf->first = wParam;
		break;

	case WM_NAME_SET_MODE:
		bf = (DRAW_BUFFER *)GetWindowLong(hWnd, GWL_USERDATA);
		if (bf == NULL) {
			break;
		}
		bf->set_mode = wParam;
		SendMessage(hWnd, WM_NAME_REDRAW, 0, 0);
		break;

	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

/*
 * score_name_regist - ウィンドウクラスの登録
 */
BOOL score_name_regist(const HINSTANCE hInstance)
{
	WNDCLASS wc;

	wc.style = 0;
	wc.lpfnWndProc = (WNDPROC)score_name_proc;
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
 * score_name_create - ウィンドウの作成
 */
HWND score_name_create(const HINSTANCE hInstance, const HWND pWnd, int id, SCORE_INFO *si)
{
	HWND hWnd;

	// ウィンドウの作成
	hWnd = CreateWindow(WINDOW_CLASS,
		TEXT(""),
		WS_CHILD,
		0, 0, 0, 0, pWnd, (HMENU)id, hInstance, si);
	return hWnd;
}
/* End of source */
