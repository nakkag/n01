/*
 * n01
 *
 * score_left.c
 *
 * Copyright (C) 1996-2008 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

/* Include Files */
#define _INC_OLE
#include <windows.h>
#undef	_INC_OLE
#include <tchar.h>

#include "general.h"
#include "Memory.h"
#include "String.h"
#include "Message.h"
#include "font.h"
#include "score_left.h"

#include "resource.h"

/* Define */
#define WINDOW_CLASS				TEXT("score_left_wnd")

#define FONT_SIZE					26
#define BORDER_SIZE					18

/* Global Variables */
extern HINSTANCE hInst;

extern OPTION_INFO op;

typedef struct _DRAW_BUFFER {
	HDC draw_dc;
	HBITMAP draw_bmp;
	HBITMAP draw_ret_bmp;

	HFONT score_font;
	HFONT ret_font;
	int font_size;

	HRGN hrgn;

	HBRUSH back_brush;
	HBRUSH active_border_brush;

	BOOL current;

	PLAYER_INFO *pi;
} DRAW_BUFFER;

/* Local Function Prototypes */
static BOOL draw_init(const HWND hWnd, DRAW_BUFFER *bf);
static BOOL draw_free(const HWND hWnd, DRAW_BUFFER *bf);
static BOOL draw_score(const HWND hWnd, const DRAW_BUFFER *bf);
static LRESULT CALLBACK score_left_proc(const HWND hWnd, const UINT msg, WPARAM wParam, LPARAM lParam);

/*
 * draw_init - 描画情報の初期化
 */
static BOOL draw_init(const HWND hWnd, DRAW_BUFFER *bf)
{
	HDC hdc;
	HRGN hrgn[2];
	RECT rect, del_rect;
	int width;

	GetClientRect(hWnd, &rect);

	hdc = GetDC(hWnd);
	bf->draw_bmp = CreateCompatibleBitmap(hdc, rect.right, rect.bottom);
	bf->draw_ret_bmp = SelectObject(bf->draw_dc, bf->draw_bmp);
	ReleaseDC(hWnd, hdc);

	bf->font_size = rect.bottom;
	bf->font_size -= (rect.bottom / FONT_SIZE) * 10;
	width = rect.right - (rect.bottom / FONT_SIZE) * 2;
	while (1) {
		SIZE sz;
		HFONT hfont, ret_font;

		hfont = font_create(op.font_name, bf->font_size, FW_BOLD, FALSE, FALSE);
		ret_font = SelectObject(bf->draw_dc, hfont);
		GetTextExtentPoint32(bf->draw_dc, TEXT("8888"), 4, &sz);
		if (bf->font_size <= 0 || width <= 0 || sz.cx <= width) {
			SelectObject(bf->draw_dc, ret_font);
			DeleteObject(hfont);
			break;
		}
		SelectObject(bf->draw_dc, ret_font);
		DeleteObject(hfont);
		bf->font_size--;
	}

	bf->score_font = font_create(op.font_name, bf->font_size, FW_BOLD, FALSE, FALSE);
	bf->ret_font = SelectObject(bf->draw_dc, bf->score_font);

	SetTextColor(bf->draw_dc, op.ci.left_text);
	SetBkColor(bf->draw_dc, op.ci.left_background);

	// リージョンの作成
	hrgn[0] = CreateRectRgnIndirect(&rect);
	// 除去するリージョンの作成
	SetRect(&del_rect, rect.bottom / BORDER_SIZE, rect.bottom / BORDER_SIZE,
		rect.right - rect.bottom / BORDER_SIZE, rect.bottom - rect.bottom / BORDER_SIZE);
	hrgn[1] = CreateRectRgnIndirect(&del_rect);
	// リージョンの結合
	bf->hrgn = CreateRectRgnIndirect(&rect);
	CombineRgn(bf->hrgn, hrgn[0], hrgn[1], RGN_DIFF);
	DeleteObject(hrgn[0]);
	DeleteObject(hrgn[1]);
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
	if (bf->score_font != NULL) {
		DeleteObject(bf->score_font);
		bf->score_font = NULL;
	}
	DeleteObject(bf->hrgn);
	return TRUE;
}

/*
 * draw_score - 残りスコアの描画
 */
static BOOL draw_score(const HWND hWnd, const DRAW_BUFFER *bf)
{
	RECT rect;
	SIZE sz;
	TCHAR buf[BUF_SIZE];
	int len;

	GetClientRect(hWnd, &rect);
	FillRect(bf->draw_dc, &rect, bf->back_brush);

	// 残りスコアの描画
	_itot(bf->pi->left, buf, 10);
	len = lstrlen(buf);
	GetTextExtentPoint32(bf->draw_dc, buf, len, &sz);
	TextOut(bf->draw_dc,
		rect.left + (rect.right - rect.left - sz.cx) / 2,
		rect.top + (rect.bottom - rect.top - sz.cy) / 2,
		buf, len);

	// 境界線の塗りつぶし
	if (bf->current == TRUE) {
		FillRgn(bf->draw_dc, bf->hrgn, bf->active_border_brush);
	}
	return TRUE;
}

/*
 * score_left_proc - スコア表示ウィンドウプロシージャ
 */
static LRESULT CALLBACK score_left_proc(const HWND hWnd, const UINT msg, WPARAM wParam, LPARAM lParam)
{
	DRAW_BUFFER *bf;
	PLAYER_INFO *pi;
	HDC hdc;
	PAINTSTRUCT ps;
	int size;

	switch (msg) {
	case WM_CREATE:
		pi = (PLAYER_INFO *)((CREATESTRUCT *)lParam)->lpCreateParams;
		if (pi == NULL) {
			return -1;
		}
		bf = (DRAW_BUFFER *)mem_calloc(sizeof(DRAW_BUFFER));
		if (bf == NULL) {
			return -1;
		}
		bf->pi = pi;

		// 描画用情報
		hdc = GetDC(hWnd);
		bf->draw_dc = CreateCompatibleDC(hdc);
		ReleaseDC(hWnd, hdc);
		draw_init(hWnd, bf);
		SetBkMode(bf->draw_dc, TRANSPARENT);
		bf->back_brush = CreateSolidBrush(op.ci.left_background);
		bf->active_border_brush = CreateSolidBrush(op.ci.left_active_border);
		draw_score(hWnd, bf);

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
			DeleteObject(bf->active_border_brush);
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
		draw_score(hWnd, bf);
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

	case WM_LEFT_REDRAW:
		bf = (DRAW_BUFFER *)GetWindowLong(hWnd, GWL_USERDATA);
		if (bf == NULL) {
			break;
		}
		draw_score(hWnd, bf);
		InvalidateRect(hWnd, NULL, FALSE);
		UpdateWindow(hWnd);
		break;

	case WM_LEFT_GET_HEIGHT:
		switch (op.left_font_size) {
		case FONT_SIZE_L:
			size = 15;
			break;
		case FONT_SIZE_M:
			size = 20;
			break;
		case FONT_SIZE_S:
			size = 30;
			break;
		}
		return wParam / size * 5;

	case WM_LEFT_DRAW_INIT:
		bf = (DRAW_BUFFER *)GetWindowLong(hWnd, GWL_USERDATA);
		if (bf == NULL) {
			break;
		}
		draw_free(hWnd, bf);
		draw_init(hWnd, bf);
		DeleteObject(bf->back_brush);
		DeleteObject(bf->active_border_brush);
		bf->back_brush = CreateSolidBrush(op.ci.left_background);
		bf->active_border_brush = CreateSolidBrush(op.ci.left_active_border);
		SendMessage(hWnd, WM_LEFT_REDRAW, 0, 0);
		break;

	case WM_LEFT_SET_CURRENT:
		bf = (DRAW_BUFFER *)GetWindowLong(hWnd, GWL_USERDATA);
		if (bf == NULL) {
			break;
		}
		bf->current = wParam;
		// 境界線の塗りつぶし
		if (bf->current == TRUE) {
			FillRgn(bf->draw_dc, bf->hrgn, bf->active_border_brush);
		} else {
			FillRgn(bf->draw_dc, bf->hrgn, bf->back_brush);
		}
		InvalidateRgn(hWnd, bf->hrgn, FALSE);
		UpdateWindow(hWnd);
		break;

	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

/*
 * score_left_regist - ウィンドウクラスの登録
 */
BOOL score_left_regist(const HINSTANCE hInstance)
{
	WNDCLASS wc;

	wc.style = 0;
	wc.lpfnWndProc = (WNDPROC)score_left_proc;
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
 * score_left_create - ウィンドウの作成
 */
HWND score_left_create(const HINSTANCE hInstance, const HWND pWnd, int id, PLAYER_INFO *pi)
{
	HWND hWnd;

	// ウィンドウの作成
	hWnd = CreateWindow(WINDOW_CLASS,
		TEXT(""),
		WS_CHILD,
		0, 0, 0, 0, pWnd, (HMENU)id, hInstance, pi);
	return hWnd;
}
/* End of source */
