/*
 * n01
 *
 * score_player.c
 *
 * Copyright (C) 1996-2017 by Ohno Tomoaki. All rights reserved.
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
#include "score_player.h"

#include "resource.h"

/* Define */
#define WINDOW_CLASS					TEXT("score_player_wnd")

#define CHAR_COUNT						10
#define LARGE_COUNT						5
#define HISTORY_COUNT					14

#define CHAR_MIN_SIZE					9
#define SMALL_MIN_SIZE					9
#define LARGE_MIN_SIZE					32

#define LEFT_MARGIN						(rect.right / 6)
#define SCROLL_HEIGHT					bf->scroll_height

#define ID_TIMER_BUTTON_DRAW			1
#define ID_TIMER_BUTTON_CLICK			2
#define TIMER_INTERVAL_BUTTON_DRAW		1
#define TIMER_INTERVAL_BUTTON_CLICK_1	500
#define TIMER_INTERVAL_BUTTON_CLICK_2	50

/* Global Variables */
extern HINSTANCE hInst;
extern OPTION_INFO op;

typedef struct _DRAW_BUFFER {
	int top;
	int height;

	HDC draw_dc;
	HBITMAP draw_bmp;
	HBITMAP draw_ret_bmp;
	int bmp_height;

	HDC name_dc;
	HBITMAP name_bmp;
	HBITMAP name_ret_bmp;
	int name_height;

	HFONT name_font;
	HFONT info_font;
	int font_width;
	int font_height;
	HFONT small_font;
	int small_font_height;
	HFONT large_font;
	int large_font_height;
	int large_font_flag;

	int height_margin;

	HBRUSH back_brush;
	HBRUSH name_back_brush;

	BOOL first;
	BOOL set_mode;
	BOOL lock;
	BOOL show_all;
	BOOL history;
	BOOL option;

	int scroll_height;
	BOOL top_button;
	BOOL bottom_button;

	PLAYER_INFO *pi;
	STATISTICS_INFO *set_stat;
} DRAW_BUFFER;

/* Local Function Prototypes */
static BOOL draw_init(const HWND hWnd, DRAW_BUFFER *bf);
static BOOL draw_free(const HWND hWnd, DRAW_BUFFER *bf);
static BOOL draw_name(const HWND hWnd, DRAW_BUFFER *bf);
static int get_draw_height(const HWND hWnd, DRAW_BUFFER *bf, const BOOL arrange_flag);
static void draw_player_title(const DRAW_BUFFER *bf, const TCHAR *title, const int left, const int top, const int right);
static int draw_player_content(const DRAW_BUFFER *bf, const TCHAR *buf, const int top, const int right, const int win_right);
static int draw_player_count(const DRAW_BUFFER *bf, const TCHAR *buf, const int top, const int right, const int win_right);
static BOOL draw_player(const HWND hWnd, DRAW_BUFFER *bf);
static LRESULT CALLBACK score_player_proc(const HWND hWnd, const UINT msg, WPARAM wParam, LPARAM lParam);

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
	int small_font_size;
	int large_font_size;
	int char_count;

	GetClientRect(hWnd, &rect);
	hdc = GetDC(hWnd);

	if (bf->history == TRUE) {
		char_count = HISTORY_COUNT;
	} else {
		char_count = CHAR_COUNT;
	}
	if (bf->option == TRUE || bf->show_all == TRUE) {
		bf->large_font_flag = 0;
	} else {
		bf->large_font_flag = 1;
	}
	if (bf->large_font_flag == 1) {
		bf->height_margin = -1;
		font_size = rect.right / CHAR_COUNT;
		small_font_size = font_size / 5 * 4;
		large_font_size = rect.right / LARGE_COUNT;
		while (1) {
			// 項目用フォント
			bf->info_font = font_create(op.font_name, font_size, 0, FALSE, FALSE);
			ret_font = SelectObject(hdc, bf->info_font);
			GetTextMetrics(hdc, &tm);
			bf->font_width = tm.tmAveCharWidth;
			bf->font_height = tm.tmHeight;
			if (bf->height_margin < 0) {
				bf->height_margin = bf->font_height / 2;
			}
			SelectObject(hdc, ret_font);

			// 小さいサイズのフォント
			bf->small_font = font_create(op.font_name, small_font_size, 0, FALSE, FALSE);
			ret_font = SelectObject(hdc, bf->small_font);
			GetTextMetrics(hdc, &tm);
			bf->small_font_height = tm.tmHeight;
			SelectObject(hdc, ret_font);

			// 大きいサイズのフォント
			bf->large_font = font_create(op.font_name, large_font_size, 0, FALSE, FALSE);
			ret_font = SelectObject(hdc, bf->large_font);
			GetTextMetrics(hdc, &tm);
			bf->large_font_height = tm.tmHeight;
			SelectObject(hdc, ret_font);

			// 描画高さの取得
			bf->bmp_height = get_draw_height(hWnd, bf, FALSE);
			if (bf->bmp_height <= rect.bottom) {
				break;
			}
			// 高さマージンの再調整
			while (bf->height_margin >= 0) {
				bf->height_margin--;
				bf->bmp_height = get_draw_height(hWnd, bf, FALSE);
				if (bf->bmp_height <= rect.bottom) {
					break;
				}
			}
			if (bf->height_margin >= 0) {
				break;
			}
			// フォントサイズの再調整
			if (large_font_size <= rect.right / 6) {
				bf->large_font_flag = 0;
				break;
			}
			font_size--;
			small_font_size--;
			large_font_size--;
			if (bf->info_font != NULL) {
				DeleteObject(bf->info_font);
				bf->info_font = NULL;
			}
			if (bf->small_font != NULL) {
				DeleteObject(bf->small_font);
				bf->small_font = NULL;
			}
			if (bf->large_font != NULL) {
				DeleteObject(bf->large_font);
				bf->large_font = NULL;
			}
		}
	}
	if (bf->large_font_flag == 0) {
		bf->height_margin = -1;
		font_size = (rect.right / char_count < CHAR_MIN_SIZE) ? CHAR_MIN_SIZE : rect.right / char_count;
		small_font_size = (font_size / 5 * 4 < SMALL_MIN_SIZE) ? SMALL_MIN_SIZE : font_size / 5 * 4;
		while (1) {
			// 項目用フォント
			bf->info_font = font_create(op.font_name, font_size, 0, FALSE, FALSE);
			ret_font = SelectObject(hdc, bf->info_font);
			GetTextMetrics(hdc, &tm);
			bf->font_width = tm.tmAveCharWidth;
			bf->font_height = tm.tmHeight;
			if (bf->height_margin < 0) {
				bf->height_margin = bf->font_height / 2;
			}
			SelectObject(hdc, ret_font);
			
			// 小さいサイズのフォント
			bf->small_font = font_create(op.font_name, small_font_size, 0, FALSE, FALSE);
			ret_font = SelectObject(hdc, bf->small_font);
			GetTextMetrics(hdc, &tm);
			bf->small_font_height = tm.tmHeight;
			SelectObject(hdc, ret_font);

			if (bf->option == TRUE) {
				break;
			}
			// 描画高さの取得
			bf->bmp_height = get_draw_height(hWnd, bf, FALSE);
			if (bf->bmp_height <= rect.bottom) {
				break;
			}
			// 高さマージンの再調整
			while (bf->height_margin >= 0) {
				bf->height_margin--;
				bf->bmp_height = get_draw_height(hWnd, bf, FALSE);
				if (bf->bmp_height <= rect.bottom) {
					break;
				}
			}
			if (bf->height_margin >= 0) {
				break;
			}
			// フォントサイズの再調整
			if (font_size <= CHAR_MIN_SIZE) {
				break;
			}
			font_size--;
			if (font_size < CHAR_MIN_SIZE) {
				font_size = CHAR_MIN_SIZE;
			}
			small_font_size--;
			if (small_font_size < SMALL_MIN_SIZE) {
				small_font_size = SMALL_MIN_SIZE;
			}
			if (bf->info_font != NULL) {
				DeleteObject(bf->info_font);
				bf->info_font = NULL;
			}
			if (bf->small_font != NULL) {
				DeleteObject(bf->small_font);
				bf->small_font = NULL;
			}
		}
	}

	if ((op.opi.name != 0 && op.view_name == 0) || bf->show_all == TRUE) {
		// 名前用フォント
		bf->name_font = font_create_menu(font_size, FW_BOLD, FALSE);
		ret_font = SelectObject(hdc, bf->name_font);
		GetTextMetrics(hdc, &tm);
		bf->name_height = tm.tmHeight + tm.tmHeight / 8;
		SelectObject(hdc, ret_font);

		bf->name_bmp = CreateCompatibleBitmap(hdc, rect.right, bf->name_height);
		bf->name_ret_bmp = SelectObject(bf->name_dc, bf->name_bmp);
		// 名前の描画
		draw_name(hWnd, bf);
	} else {
		bf->name_font = NULL;
		bf->name_height = 0;
	}


	// 描画用ビットマップの作成
	bf->bmp_height = get_draw_height(hWnd, bf, TRUE);
	if (bf->bmp_height < rect.bottom) {
		bf->bmp_height = rect.bottom;
	}
	bf->draw_bmp = CreateCompatibleBitmap(hdc, rect.right, bf->bmp_height);
	bf->draw_ret_bmp = SelectObject(bf->draw_dc, bf->draw_bmp);

	ReleaseDC(hWnd, hdc);

	bf->top = 0;
	bf->top_button = FALSE;
	bf->bottom_button = FALSE;
	bf->scroll_height = bf->font_height * 2;
	return TRUE;
}

/*
 * draw_free - 描画情報の解放
 */
static BOOL draw_free(const HWND hWnd, DRAW_BUFFER *bf)
{
	if (bf->draw_dc != NULL) {
		SelectObject(bf->draw_dc, bf->draw_ret_bmp);
		DeleteObject(bf->draw_bmp);
		bf->draw_bmp = NULL;
	}
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
	if (bf->small_font != NULL) {
		DeleteObject(bf->small_font);
		bf->small_font = NULL;
	}
	if (bf->large_font != NULL) {
		DeleteObject(bf->large_font);
		bf->large_font = NULL;
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
	TCHAR buf[BUF_SIZE];

	if (bf->pi == NULL) {
		return FALSE;
	}
	if (op.opi.name != 0 || bf->show_all == TRUE) {
		GetClientRect(hWnd, &rect);
		SetRect(&draw_rect, 0, 0, rect.right, bf->name_height);
		FillRect(bf->name_dc, &draw_rect, bf->name_back_brush);

		ret_font = SelectObject(bf->name_dc, bf->name_font);
		SetTextColor(bf->name_dc, op.ci.player_name_text);
		SetBkColor(bf->name_dc, op.ci.player_name_background);
		if (bf->pi->com == TRUE) {
			wsprintf(buf, message_get_res(IDS_STRING_COM), bf->pi->level + 1);
		} else {
			lstrcpy(buf, bf->pi->name);
		}
		DrawText(bf->name_dc, buf, lstrlen(buf), &draw_rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
		SelectObject(bf->name_dc, ret_font);
	}
	return TRUE;
}

/*
 * get_draw_height - プレイヤー情報の描画高さ取得
 */
static int get_draw_height(const HWND hWnd, DRAW_BUFFER *bf, const BOOL arrange_flag)
{
	int height = 0;
	int font_height;

	if (bf->large_font_flag == 1 && bf->show_all == FALSE) {
		// 大きいフォントの高さ
		font_height = bf->font_height + bf->large_font_height + bf->height_margin;
	} else {
		// 通常フォントの高さ
		font_height = bf->font_height + bf->height_margin;
	}

	if ((op.opi.name != 0 && op.view_name == 0) || bf->show_all == TRUE) {
		height += bf->font_height;
	}

	if (op.opi.first != 0 && bf->show_all == FALSE) {
		height += bf->font_height;
	}

	// Totals Title
	if (op.opi.total_sets != 0 || op.opi.total_legs != 0 || op.opi.total_tons != 0 ||
		op.opi.total_100 != 0 || op.opi.total_140 != 0 || op.opi.total_180s != 0 ||
		op.opi.total_high_off != 0 || op.opi.total_short != 0 || op.opi.total_long != 0 ||
		bf->show_all == TRUE) {
		height += bf->font_height;
	}

	if ((op.opi.total_sets != 0 || bf->show_all == TRUE) && bf->set_mode == TRUE) {
		// sets
		height += font_height;
	}
	if (op.opi.total_legs != 0 || bf->show_all == TRUE) {
		// Legs
		height += font_height;
	}
	if (op.opi.total_tons != 0 || bf->show_all == TRUE) {
		// Tons
		height += font_height;
	}
	if (op.opi.total_100 != 0 || bf->show_all == TRUE) {
		// 100+
		height += font_height;
	}
	if (op.opi.total_140 != 0 || bf->show_all == TRUE) {
		// 140+
		height += font_height;
	}
	if (op.opi.total_180s != 0 || bf->show_all == TRUE) {
		// 180's
		height += font_height;
	}
	if (op.opi.total_high_off != 0 || bf->show_all == TRUE) {
		// High off
		height += font_height;
	}
	if (op.opi.total_short != 0 || bf->show_all == TRUE) {
		// Short Game
		height += font_height;
	}
	if (op.opi.total_long != 0 || bf->show_all == TRUE) {
		// Long Game
		height += font_height;
	}

	// Averages Title
	if (op.opi.avg_score != 0 || op.opi.avg_darts != 0 || op.opi.avg_first9 != 0 ||
		op.opi.avg_check_out != 0 || op.opi.avg_keep != 0 || op.opi.avg_break != 0 || bf->show_all == TRUE) {
		height += bf->font_height;
	}

	if (op.opi.avg_darts != 0 || bf->show_all == TRUE) {
		// Darts Average
		height += font_height;
	}
	if (op.opi.avg_score != 0 || bf->show_all == TRUE) {
		// Score Average
		height += font_height;
	}
	if (op.opi.avg_first9 != 0 || bf->show_all == TRUE) {
		// First 9 Score Average
		height += font_height;
	}

	if (op.opi.avg_check_out != 0 || bf->show_all == TRUE) {
		// Check-out Average
		height += font_height;
		if (op.opi.avg_check_out_count != 0 || bf->show_all == TRUE) {
			if (bf->large_font_flag == 1 && bf->show_all == FALSE) {
				height += bf->font_height;
			} else {
				height += bf->small_font_height;
			}
		}
	}
	if (op.opi.avg_keep != 0 || bf->show_all == TRUE) {
		// Keep Average
		height += font_height;
		if (op.opi.avg_keep_count != 0 || bf->show_all == TRUE) {
			if (bf->large_font_flag == 1 && bf->show_all == FALSE) {
				height += bf->font_height;
			} else {
				height += bf->small_font_height;
			}
		}
	}
	if (op.opi.avg_break != 0 || bf->show_all == TRUE) {
		// Break Average
		height += font_height;
		if (op.opi.avg_break_count != 0 || bf->show_all == TRUE) {
			if (bf->large_font_flag == 1 && bf->show_all == FALSE) {
				height += bf->font_height;
			} else {
				height += bf->small_font_height;
			}
		}
	}

	if (op.opi.arrange != 0 && bf->show_all == FALSE) {
		// Arrange Title
		height += bf->font_height;
		if (arrange_flag == TRUE) {
			height += bf->font_height * 20;
		} else {
			height += bf->font_height;
		}
	} else if (height > 0) {
		height -= bf->height_margin;
	}
	return height;
}

/*
 * draw_text - テキストの描画
 */
static void draw_text(const HDC hdc, const TCHAR *str, const int len, const RECT *rect, const int format)
{
	SIZE sz;
	int left, top;

	GetTextExtentPoint32(hdc, str, len, &sz);
	switch (format) {
	case DT_LEFT:
	default:
		left = rect->left;
		break;
	case DT_RIGHT:
		left = rect->right - sz.cx;
		break;
	case DT_CENTER:
		left = rect->left + (rect->right - rect->left - sz.cx) / 2;
		break;
	}
	top = rect->top + (rect->bottom - rect->top - sz.cy) / 2;
	TextOut(hdc, left, top, str, len);
}

/*
 * draw_player_title - プレイヤー情報のタイトル描画
 */
static void draw_player_title(const DRAW_BUFFER *bf, const TCHAR *title, const int left, const int top, const int right)
{
	RECT draw_rect;
	TCHAR buf[BUF_SIZE];

	lstrcpy(buf, title);
	lstrcat(buf, message_get_res(IDS_STRING_P_SEPARATE));

	SetRect(&draw_rect, left, top, right, top + bf->font_height);
	draw_text(bf->draw_dc, buf, lstrlen(buf), &draw_rect, DT_LEFT);
}

/*
 * draw_player_content - プレイヤー情報の内容描画
 */
static int draw_player_content(const DRAW_BUFFER *bf, const TCHAR *buf, const int top, const int right, const int win_right)
{
	HFONT ret_font;
	RECT draw_rect;
	int height = top;

	if (bf->large_font_flag == 1 && bf->show_all == FALSE) {
		// 大きいフォントで描画
		height += bf->font_height;

		ret_font = SelectObject(bf->draw_dc, bf->large_font);
		SetRect(&draw_rect, 0, height, win_right, height + bf->large_font_height);
		draw_text(bf->draw_dc, buf, lstrlen(buf), &draw_rect, DT_CENTER);
		height += bf->large_font_height + bf->height_margin;

		SelectObject(bf->draw_dc, ret_font);
	} else {
		// 通常フォントで描画
		SetRect(&draw_rect, 0, height, right, height + bf->font_height);
		draw_text(bf->draw_dc, buf, lstrlen(buf), &draw_rect, DT_RIGHT);
		height += bf->font_height + bf->height_margin;
	}
	return height;
}

/*
 * draw_player_count - プレイヤー情報のカウント数描画
 */
static int draw_player_count(const DRAW_BUFFER *bf, const TCHAR *buf, const int top, const int right, const int win_right)
{
	HFONT ret_font;
	RECT draw_rect;
	int height = top;

	height -= bf->height_margin;

	if (bf->large_font_flag == 1 && bf->show_all == FALSE) {
		// 通常フォントで描画
		SetRect(&draw_rect, 0, height, win_right, height + bf->font_height);
		draw_text(bf->draw_dc, buf, lstrlen(buf), &draw_rect, DT_CENTER);
		height += bf->font_height + bf->height_margin;
	} else {
		// 小さいフォントで描画
		ret_font = SelectObject(bf->draw_dc, bf->small_font);

		SetRect(&draw_rect, 0, height, right, height + bf->small_font_height);
		draw_text(bf->draw_dc, buf, lstrlen(buf), &draw_rect, DT_RIGHT);
		height += bf->small_font_height + bf->height_margin;

		SelectObject(bf->draw_dc, ret_font);
	}
	return height;
}

/*
 * draw_player - プレイヤー情報の描画
 */
static BOOL draw_player(const HWND hWnd, DRAW_BUFFER *bf)
{
	RECT draw_rect, rect;
	HFONT ret_font;
	SIZE sz;
	TCHAR buf[BUF_SIZE];
	int left, right;
	int title_left;
	int height = 0;
	int i, j;

	GetClientRect(hWnd, &rect);
	rect.bottom = bf->bmp_height;
	FillRect(bf->draw_dc, &rect, bf->back_brush);

	if (bf->pi == NULL) {
		return FALSE;
	}

	// info
	SetTextColor(bf->draw_dc, op.ci.player_text);
	SetBkColor(bf->draw_dc, op.ci.player_background);
	ret_font = SelectObject(bf->draw_dc, bf->info_font);

	if (op.opi.first != 0 && bf->show_all == FALSE) {
		if (bf->first == 1) {
			// - Firet -
			SetRect(&draw_rect, 0, height, rect.right, height + bf->font_height);
			draw_text(bf->draw_dc, message_get_res(IDS_STRING_P_FIRST), lstrlen(message_get_res(IDS_STRING_P_FIRST)), &draw_rect, DT_CENTER);
		}
		height += bf->font_height;
	}

	// 配置の設定
	if (bf->history == TRUE) {
		message_copy_res(IDS_STRING_P_AVG_CHECK_OUT_LARGE, buf);
	} else {
		message_copy_res(IDS_STRING_P_AVG_CHECK_OUT, buf);
	}
	lstrcat(buf, message_get_res(IDS_STRING_P_SEPARATE));
	lstrcat(buf, TEXT("188.88"));
	GetTextExtentPoint32(bf->draw_dc, buf, lstrlen(buf), &sz);

	if (LEFT_MARGIN < (rect.right - sz.cx) / 2 && bf->large_font_flag == 0) {
		left = LEFT_MARGIN;
	} else {
		left = (rect.right - sz.cx) / 2;
		if (left < 0) {
			left = 0;
		}
	} 
	title_left = left - bf->font_width * 2;
	if (title_left < 0) {
		title_left = 0;
	}
	right = rect.right - left;

	// Totals Title
	if (op.opi.total_sets != 0 || op.opi.total_legs != 0 || op.opi.total_tons != 0 ||
		op.opi.total_100 != 0 || op.opi.total_140 != 0 || op.opi.total_180s != 0 ||
		op.opi.total_high_off != 0 || op.opi.total_short != 0 || op.opi.total_long != 0 ||
		bf->show_all == TRUE) {
		SetTextColor(bf->draw_dc, op.ci.player_info_title);
		SetRect(&draw_rect, title_left, height, rect.right, height + bf->font_height);
		draw_text(bf->draw_dc, message_get_res(IDS_STRING_P_TOTAL), lstrlen(message_get_res(IDS_STRING_P_TOTAL)), &draw_rect, DT_LEFT);
		height += bf->font_height;
	}
	SetTextColor(bf->draw_dc, op.ci.player_text);

	if ((op.opi.total_sets != 0 || bf->show_all == TRUE) && bf->set_mode == TRUE) {
		// sets
		draw_player_title(bf, message_get_res(IDS_STRING_P_SETS), left, height, rect.right);
		_itot(bf->pi->sets, buf, 10);
		height = draw_player_content(bf, buf, height, right, rect.right);
	}
	if (op.opi.total_legs != 0 || bf->show_all == TRUE) {
		// Legs
		draw_player_title(bf, message_get_res(IDS_STRING_P_LEGS), left, height, rect.right);
		_itot(bf->pi->legs, buf, 10);
		height = draw_player_content(bf, buf, height, right, rect.right);
	}
	if (op.opi.total_tons != 0 || bf->show_all == TRUE) {
		// Tons
		draw_player_title(bf, message_get_res(IDS_STRING_P_TONS), left, height, rect.right);
		_itot(bf->pi->stat.ton_count + bf->set_stat->ton_count, buf, 10);
		height = draw_player_content(bf, buf, height, right, rect.right);
	}
	if (op.opi.total_100 != 0 || bf->show_all == TRUE) {
		// 100+
		draw_player_title(bf, message_get_res(IDS_STRING_P_100), left, height, rect.right);
		_itot(bf->pi->stat.ton00_count + bf->set_stat->ton00_count, buf, 10);
		height = draw_player_content(bf, buf, height, right, rect.right);
	}
	if (op.opi.total_140 != 0 || bf->show_all == TRUE) {
		// 140+
		draw_player_title(bf, message_get_res(IDS_STRING_P_140), left, height, rect.right);
		_itot(bf->pi->stat.ton40_count + bf->set_stat->ton40_count, buf, 10);
		height = draw_player_content(bf, buf, height, right, rect.right);
	}
	if (op.opi.total_180s != 0 || bf->show_all == TRUE) {
		// 180's
		draw_player_title(bf, message_get_res(IDS_STRING_P_180S), left, height, rect.right);
		_itot(bf->pi->stat.ton80_count + bf->set_stat->ton80_count, buf, 10);
		height = draw_player_content(bf, buf, height, right, rect.right);
	}
	if (op.opi.total_high_off != 0 || bf->show_all == TRUE) {
		// High off
		draw_player_title(bf, message_get_res(IDS_STRING_P_HIGHOFF), left, height, rect.right);
		_itot((bf->pi->stat.high_off > bf->set_stat->high_off) ? bf->pi->stat.high_off : bf->set_stat->high_off, buf, 10);
		height = draw_player_content(bf, buf, height, right, rect.right);
	}
	if (op.opi.total_short != 0 || bf->show_all == TRUE) {
		// Short Game
		draw_player_title(bf, message_get_res(IDS_STRING_P_SHORT), left, height, rect.right);
		if (bf->pi->stat.short_game == 0 && bf->set_stat->short_game > 0) {
			i = bf->set_stat->short_game;
		} else if (bf->set_stat->short_game == 0 && bf->pi->stat.short_game > 0) {
			i = bf->pi->stat.short_game;
		} else {
			i = (bf->pi->stat.short_game < bf->set_stat->short_game) ? bf->pi->stat.short_game : bf->set_stat->short_game;
		}
		_itot(i, buf, 10);
		height = draw_player_content(bf, buf, height, right, rect.right);
	}
	if (op.opi.total_long != 0 || bf->show_all == TRUE) {
		// Long Game
		draw_player_title(bf, message_get_res(IDS_STRING_P_LONG), left, height, rect.right);
		_itot((bf->pi->stat.long_game > bf->set_stat->long_game) ? bf->pi->stat.long_game : bf->set_stat->long_game, buf, 10);
		height = draw_player_content(bf, buf, height, right, rect.right);
	}

	// Averages Title
	if (op.opi.avg_score != 0 || op.opi.avg_darts != 0 || op.opi.avg_first9 != 0 ||
		op.opi.avg_check_out != 0 || op.opi.avg_keep != 0 || op.opi.avg_break != 0 || bf->show_all == TRUE) {
		SetTextColor(bf->draw_dc, op.ci.player_info_title);
		SetRect(&draw_rect, title_left, height, rect.right, height + bf->font_height);
		draw_text(bf->draw_dc, message_get_res(IDS_STRING_P_AVERAGE), lstrlen(message_get_res(IDS_STRING_P_AVERAGE)), &draw_rect, DT_LEFT);
		height += bf->font_height;
	}
	SetTextColor(bf->draw_dc, op.ci.player_text);

	if (op.opi.avg_darts != 0 || bf->show_all == TRUE) {
		// Darts Average
		draw_player_title(bf, message_get_res(IDS_STRING_P_AVG_DARTS), left, height, rect.right);
		if (bf->pi->stat.win_count + bf->set_stat->win_count > 0) {
			_stprintf(buf, TEXT("%.2f"), (float)(bf->pi->stat.win_darts + bf->set_stat->win_darts) / (float)(bf->pi->stat.win_count + bf->set_stat->win_count));
		} else {
			lstrcpy(buf, TEXT("0.00"));
		}
		height = draw_player_content(bf, buf, height, right, rect.right);
	}
	if (op.opi.avg_score != 0 || bf->show_all == TRUE) {
		// Score Average
		draw_player_title(bf, message_get_res(IDS_STRING_P_AVG_SCORE), left, height, rect.right);
		if (bf->pi->stat.all_darts + bf->set_stat->all_darts > 0) {
			if (op.opi.avg_per_round == 1) {
				_stprintf(buf, TEXT("%.2f"), ((float)(bf->pi->stat.all_score + bf->set_stat->all_score) / (float)(bf->pi->stat.all_darts + bf->set_stat->all_darts)) * 3);
			} else {
				_stprintf(buf, TEXT("%.2f"), (float)(bf->pi->stat.all_score + bf->set_stat->all_score) / (float)(bf->pi->stat.all_darts + bf->set_stat->all_darts));
			}
		} else {
			lstrcpy(buf, TEXT("0.00"));
		}
		height = draw_player_content(bf, buf, height, right, rect.right);
	}
	if (op.opi.avg_first9 != 0 || bf->show_all == TRUE) {
		// First 9 Score Average
		draw_player_title(bf, message_get_res(IDS_STRING_P_AVG_FIRST9), left, height, rect.right);
		if (bf->pi->stat.first9_darts + bf->set_stat->first9_darts > 0) {
			if (op.opi.avg_per_round == 1) {
				_stprintf(buf, TEXT("%.2f"), ((float)(bf->pi->stat.first9_score + bf->set_stat->first9_score) / (float)(bf->pi->stat.first9_darts + bf->set_stat->first9_darts)) * 3);
			} else {
				_stprintf(buf, TEXT("%.2f"), (float)(bf->pi->stat.first9_score + bf->set_stat->first9_score) / (float)(bf->pi->stat.first9_darts + bf->set_stat->first9_darts));
			}
		} else {
			lstrcpy(buf, TEXT("0.00"));
		}
		height = draw_player_content(bf, buf, height, right, rect.right);
	}

	if (op.opi.avg_check_out != 0 || bf->show_all == TRUE) {
		// Check-out Average
		if (bf->large_font_flag == 1 || bf->history == TRUE) {
			draw_player_title(bf, message_get_res(IDS_STRING_P_AVG_CHECK_OUT_LARGE), left, height, rect.right);
		} else {
			draw_player_title(bf, message_get_res(IDS_STRING_P_AVG_CHECK_OUT), left, height, rect.right);
		}
		if (bf->pi->stat.check_out_aim + bf->set_stat->check_out_aim > 0) {
			_stprintf(buf, TEXT("%.2f"), ((float)(bf->pi->stat.check_out + bf->set_stat->check_out) / (float)(bf->pi->stat.check_out_aim + bf->set_stat->check_out_aim)) * 100);
		} else {
			lstrcpy(buf, TEXT("0.00"));
		}
		height = draw_player_content(bf, buf, height, right, rect.right);

		if (op.opi.avg_check_out_count != 0 || bf->show_all == TRUE) {
			wsprintf(buf, TEXT("(%d / %d)"), bf->pi->stat.check_out + bf->set_stat->check_out, bf->pi->stat.check_out_aim + bf->set_stat->check_out_aim);
			height = draw_player_count(bf, buf, height, right, rect.right);
		}
	}
	if (op.opi.avg_keep != 0 || bf->show_all == TRUE) {
		// Keep Average
		draw_player_title(bf, message_get_res(IDS_STRING_P_AVG_KEEP), left, height, rect.right);
		if (bf->pi->stat.all_keep_legs + bf->set_stat->all_keep_legs > 0) {
			_stprintf(buf, TEXT("%.2f"), ((float)(bf->pi->stat.win_keep_legs + bf->set_stat->win_keep_legs) / (float)(bf->pi->stat.all_keep_legs + bf->set_stat->all_keep_legs)) * 100);
		} else {
			lstrcpy(buf, TEXT("0.00"));
		}
		height = draw_player_content(bf, buf, height, right, rect.right);

		if (op.opi.avg_keep_count != 0 || bf->show_all == TRUE) {
			wsprintf(buf, TEXT("(%d / %d)"), bf->pi->stat.win_keep_legs + bf->set_stat->win_keep_legs, bf->pi->stat.all_keep_legs + bf->set_stat->all_keep_legs);
			height = draw_player_count(bf, buf, height, right, rect.right);
		}
	}
	if (op.opi.avg_break != 0 || bf->show_all == TRUE) {
		// Break Average
		draw_player_title(bf, message_get_res(IDS_STRING_P_AVG_BREAK), left, height, rect.right);
		if (bf->pi->stat.all_break_legs + bf->set_stat->all_break_legs > 0) {
			_stprintf(buf, TEXT("%.2f"), ((float)(bf->pi->stat.win_break_legs + bf->set_stat->win_break_legs) / (float)(bf->pi->stat.all_break_legs + bf->set_stat->all_break_legs)) * 100);
		} else {
			lstrcpy(buf, TEXT("0.00"));
		}
		height = draw_player_content(bf, buf, height, right, rect.right);

		if (op.opi.avg_break_count != 0 || bf->show_all == TRUE) {
			wsprintf(buf, TEXT("(%d / %d)"), bf->pi->stat.win_break_legs + bf->set_stat->win_break_legs, bf->pi->stat.all_break_legs + bf->set_stat->all_break_legs);
			height = draw_player_count(bf, buf, height, right, rect.right);
		}
	}

	// 配置の設定
	GetTextExtentPoint32(bf->draw_dc, TEXT("S88-S88-S88"), lstrlen(TEXT("S88-S88-S88")), &sz);
	if (LEFT_MARGIN < (rect.right - sz.cx) / 2 && bf->large_font_flag == 0) {
		left = LEFT_MARGIN;
	} else {
		left = (rect.right - sz.cx) / 2;
		if (left < 0) {
			left = 0;
		}
	}

	if (op.opi.arrange != 0 && bf->show_all == FALSE) {
		// Arrange Title
		SetTextColor(bf->draw_dc, op.ci.player_info_title);
		SetRect(&draw_rect, title_left, height, rect.right, height + bf->font_height);
		draw_text(bf->draw_dc, message_get_res(IDS_STRING_P_ARRANGE), lstrlen(message_get_res(IDS_STRING_P_ARRANGE)), &draw_rect, DT_LEFT);
		height += bf->font_height;

		SetTextColor(bf->draw_dc, op.ci.player_text);
		for (i = 0; i < op.arrange_info_count; i++) {
			if (op.arrange_info[i].left == bf->pi->left) {
				*buf = TEXT('\0');
				for (j = 0; j < 3; j++) {
					if (*op.arrange_info[i].throw_list[j] == TEXT('\0')) {
						continue;
					}
					if (*buf != TEXT('\0')) {
						lstrcat(buf, TEXT("-"));
					}
					lstrcat(buf, op.arrange_info[i].throw_list[j]);
				}
				SetRect(&draw_rect, left, height, rect.right, height + bf->font_height);
				draw_text(bf->draw_dc, buf, lstrlen(buf), &draw_rect, DT_LEFT);
				height += bf->font_height;
			}
		}
	} else if (height > 0) {
		height -= bf->height_margin;
	}
	SelectObject(bf->draw_dc, ret_font);

	bf->height = height;
	return TRUE;
}

/*
 * score_player_proc - プレイヤー情報表示ウィンドウプロシージャ
 */
static LRESULT CALLBACK score_player_proc(const HWND hWnd, const UINT msg, WPARAM wParam, LPARAM lParam)
{
	static STATISTICS_INFO tmp_stat;
	DRAW_BUFFER *bf;
	PLAYER_INFO *pi;
	HDC hdc;
	PAINTSTRUCT ps;
	RECT rect;
	POINT apos;

	switch (msg) {
	case WM_CREATE:
		pi = (PLAYER_INFO *)((CREATESTRUCT *)lParam)->lpCreateParams;
		bf = (DRAW_BUFFER *)mem_calloc(sizeof(DRAW_BUFFER));
		if (bf == NULL) {
			return -1;
		}
		bf->pi = pi;
		if (bf->pi != NULL) {
			bf->set_stat = &bf->pi->set_stat;
		} else {
			bf->set_stat = &tmp_stat;
		}
		if (op.opi.scroll == 0) {
			bf->lock = TRUE;
		}

		// 描画用情報
		hdc = GetDC(hWnd);
		bf->draw_dc = CreateCompatibleDC(hdc);
		bf->name_dc = CreateCompatibleDC(hdc);
		ReleaseDC(hWnd, hdc);
		bf->back_brush = CreateSolidBrush(op.ci.player_background);
		bf->name_back_brush = CreateSolidBrush(op.ci.player_name_background);
		draw_init(hWnd, bf);
		draw_player(hWnd, bf);

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
		draw_player(hWnd, bf);
		InvalidateRect(hWnd, NULL, FALSE);
		UpdateWindow(hWnd);
		break;

	case WM_LBUTTONDOWN:
		bf = (DRAW_BUFFER *)GetWindowLong(hWnd, GWL_USERDATA);
		if (bf == NULL || bf->lock == TRUE) {
			break;
		}
		GetClientRect(hWnd, &rect);
		if (rect.bottom - bf->name_height >= bf->height && bf->top == 0) {
			break;
		}
		if (HIWORD(lParam) < bf->name_height + SCROLL_HEIGHT && HIWORD(lParam) > bf->name_height) {
			// 上にスクロール
			if (bf->top == 0) {
				break;
			}
			bf->top = bf->top - bf->font_height;
			if (bf->top < 0) {
				bf->top = 0;
			}
			rect.top = bf->name_height;
			InvalidateRect(hWnd, &rect, FALSE);
			UpdateWindow(hWnd);
			SetTimer(hWnd, ID_TIMER_BUTTON_CLICK, TIMER_INTERVAL_BUTTON_CLICK_1, NULL);

		}else if (HIWORD(lParam) > rect.bottom - SCROLL_HEIGHT) {
			// 下にスクロール
			if (bf->top == bf->height - rect.bottom + bf->name_height) {
				break;
			}
			bf->top = bf->top + bf->font_height;
			if (bf->top > bf->height - rect.bottom + bf->name_height) {
				bf->top = bf->height - rect.bottom + bf->name_height;
				if (bf->top < 0) {
					bf->top = 0;
				}
			}
			rect.top = bf->name_height;
			InvalidateRect(hWnd, &rect, FALSE);
			UpdateWindow(hWnd);
			SetTimer(hWnd, ID_TIMER_BUTTON_CLICK, TIMER_INTERVAL_BUTTON_CLICK_1, NULL);
		}
		break;

	case WM_LBUTTONUP:
		KillTimer(hWnd, ID_TIMER_BUTTON_CLICK);

		bf = (DRAW_BUFFER *)GetWindowLong(hWnd, GWL_USERDATA);
		if (bf == NULL) {
			break;
		}
		if (bf->top_button == TRUE || bf->bottom_button == TRUE) {
			GetClientRect(hWnd, &rect);
			rect.top = bf->name_height;
			InvalidateRect(hWnd, &rect, FALSE);
			UpdateWindow(hWnd);
		}
		break;

	case WM_MOUSEMOVE:
		bf = (DRAW_BUFFER *)GetWindowLong(hWnd, GWL_USERDATA);
		if (bf == NULL || bf->lock == TRUE) {
			break;
		}
		GetClientRect(hWnd, &rect);
		if (rect.bottom - bf->name_height >= bf->height && bf->top == 0) {
			break;
		}
		// スクロールボタンの表示
		if (bf->top_button == FALSE && bf->top > 0 &&
			HIWORD(lParam) < bf->name_height + SCROLL_HEIGHT && HIWORD(lParam) > bf->name_height) {
			bf->top_button = TRUE;
			rect.top = bf->name_height;
			InvalidateRect(hWnd, &rect, FALSE);
			UpdateWindow(hWnd);
			SetTimer(hWnd, ID_TIMER_BUTTON_DRAW, TIMER_INTERVAL_BUTTON_DRAW, NULL);

		} else if (bf->bottom_button == FALSE &&
			bf->top < bf->height - rect.bottom + bf->name_height &&
			HIWORD(lParam) > rect.bottom - SCROLL_HEIGHT) {
			bf->bottom_button = TRUE;
			rect.top = bf->name_height;
			InvalidateRect(hWnd, &rect, FALSE);
			UpdateWindow(hWnd);
			SetTimer(hWnd, ID_TIMER_BUTTON_DRAW, TIMER_INTERVAL_BUTTON_DRAW, NULL);
		}
		break;

	case WM_TIMER:
		bf = (DRAW_BUFFER *)GetWindowLong(hWnd, GWL_USERDATA);
		if (bf == NULL) {
			break;
		}
		switch (wParam) {
		case ID_TIMER_BUTTON_DRAW:
			GetClientRect(hWnd, &rect);
			GetCursorPos((LPPOINT)&apos);
			ScreenToClient(hWnd, &apos);
			// スクロールボタンの表示解除判定
			if (bf->top_button == TRUE) {
				if (bf->top <= 0 ||
					apos.y >= bf->name_height + SCROLL_HEIGHT || apos.y <= bf->name_height ||
					apos.x < rect.left || apos.x > rect.right) {
					KillTimer(hWnd, ID_TIMER_BUTTON_DRAW);
					bf->top_button = FALSE;
					rect.top = bf->name_height;
					InvalidateRect(hWnd, &rect, FALSE);
					UpdateWindow(hWnd);
				}
			} else if (bf->bottom_button == TRUE) {
				if (bf->top >= bf->height - rect.bottom + bf->name_height ||
					apos.y <= rect.bottom - SCROLL_HEIGHT || apos.y > rect.bottom ||
					apos.x < rect.left || apos.x > rect.right) {
					KillTimer(hWnd, ID_TIMER_BUTTON_DRAW);
					bf->bottom_button = FALSE;
					rect.top = bf->name_height;
					InvalidateRect(hWnd, &rect, FALSE);
					UpdateWindow(hWnd);
				}
			} else {
				KillTimer(hWnd, ID_TIMER_BUTTON_DRAW);
				break;
			}
			break;

		case ID_TIMER_BUTTON_CLICK:
			if (GetAsyncKeyState(VK_LBUTTON) >= 0) {
				KillTimer(hWnd, ID_TIMER_BUTTON_CLICK);
				break;
			}
			GetClientRect(hWnd, &rect);
			GetCursorPos((LPPOINT)&apos);
			ScreenToClient(hWnd, &apos);
			if (apos.x < rect.left || apos.x > rect.right) {
				break;
			}
			if (apos.y < bf->name_height + SCROLL_HEIGHT && apos.y > bf->name_height) {
				// 上にスクロール
				if (bf->top == 0) {
					break;
				}
				bf->top = bf->top - bf->font_height;
				if (bf->top < 0) {
					bf->top = 0;
				}
				rect.top = bf->name_height;
				InvalidateRect(hWnd, &rect, FALSE);
				UpdateWindow(hWnd);

			} else if (apos.y > rect.bottom - SCROLL_HEIGHT && apos.y < rect.bottom) {
				// 下にスクロール
				if (bf->top == bf->height - rect.bottom + bf->name_height) {
					break;
				}
				bf->top = bf->top + bf->font_height;
				if (bf->top > bf->height - rect.bottom + bf->name_height) {
					bf->top = bf->height - rect.bottom + bf->name_height;
					if (bf->top < 0) {
						bf->top = 0;
					}
				}
				rect.top = bf->name_height;
				InvalidateRect(hWnd, &rect, FALSE);
				UpdateWindow(hWnd);
			} 
			SetTimer(hWnd, ID_TIMER_BUTTON_CLICK, TIMER_INTERVAL_BUTTON_CLICK_2, NULL);
			break;
		}
		break;

	case WM_PAINT:
		bf = (DRAW_BUFFER *)GetWindowLong(hWnd, GWL_USERDATA);
		if (bf == NULL) {
			break;
		}
		hdc = BeginPaint(hWnd, &ps);

		if (ps.rcPaint.top < bf->name_height) {
			// プレイヤー名の描画
			BitBlt(hdc, ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right, bf->name_height,
				bf->name_dc, ps.rcPaint.left, ps.rcPaint.top, SRCCOPY);
			ps.rcPaint.top = bf->name_height;
		}
		// プレイヤー情報の描画
		BitBlt(hdc, ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right, ps.rcPaint.bottom,
			bf->draw_dc, ps.rcPaint.left, ps.rcPaint.top + bf->top - bf->name_height, SRCCOPY);

		if (bf->top_button == TRUE) {
			// 上へのスクロールボタン描画
			GetClientRect(hWnd, &rect);
			rect.top = bf->name_height;
			rect.bottom = bf->name_height + SCROLL_HEIGHT;
			DrawFrameControl(hdc, &rect, DFC_SCROLL, DFCS_SCROLLUP | ((GetAsyncKeyState(VK_LBUTTON) < 0) ? DFCS_PUSHED : 0));
		}
		if (bf->bottom_button == TRUE) {
			// 下へのスクロールボタン描画
			GetClientRect(hWnd, &rect);
			rect.top = rect.bottom - SCROLL_HEIGHT;
			DrawFrameControl(hdc, &rect, DFC_SCROLL, DFCS_SCROLLDOWN | ((GetAsyncKeyState(VK_LBUTTON) < 0) ? DFCS_PUSHED : 0));
		}
		EndPaint(hWnd, &ps);
		break;

	case WM_ERASEBKGND:
		return TRUE;

	case WM_PLAYER_REDRAW:
		bf = (DRAW_BUFFER *)GetWindowLong(hWnd, GWL_USERDATA);
		if (bf == NULL) {
			break;
		}
		draw_player(hWnd, bf);
		InvalidateRect(hWnd, NULL, FALSE);
		UpdateWindow(hWnd);
		break;

	case WM_PLAYER_DRAW_INIT:
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
		SendMessage(hWnd, WM_PLAYER_REDRAW, 0, 0);
		break;

	case WM_WINDOW_SET_FIRST:
		bf = (DRAW_BUFFER *)GetWindowLong(hWnd, GWL_USERDATA);
		if (bf == NULL) {
			break;
		}
		bf->first = wParam;
		break;

	case WM_PLAYER_SET_INFO:
		bf = (DRAW_BUFFER *)GetWindowLong(hWnd, GWL_USERDATA);
		if (bf == NULL) {
			break;
		}
		bf->pi = (PLAYER_INFO *)lParam;
		if (bf->pi != NULL && wParam == 0) {
			bf->set_stat = &bf->pi->set_stat;
		} else {
			bf->set_stat = &tmp_stat;
		}
		draw_free(hWnd, bf);
		draw_init(hWnd, bf);
		SendMessage(hWnd, WM_PLAYER_REDRAW, 0, 0);
		break;

	case WM_PLAYER_SET_MODE:
		bf = (DRAW_BUFFER *)GetWindowLong(hWnd, GWL_USERDATA);
		if (bf == NULL) {
			break;
		}
		bf->set_mode = wParam;
		draw_free(hWnd, bf);
		draw_init(hWnd, bf);
		SendMessage(hWnd, WM_PLAYER_REDRAW, 0, 0);
		break;

	case WM_PLAYER_SET_LOCK:
		bf = (DRAW_BUFFER *)GetWindowLong(hWnd, GWL_USERDATA);
		if (bf == NULL) {
			break;
		}
		bf->lock = wParam;
		break;

	case WM_PLAYER_MODE_HISTORY:
		bf = (DRAW_BUFFER *)GetWindowLong(hWnd, GWL_USERDATA);
		if (bf == NULL) {
			break;
		}
		bf->history = wParam;
		bf->show_all = wParam;
		draw_free(hWnd, bf);
		draw_init(hWnd, bf);
		SendMessage(hWnd, WM_PLAYER_REDRAW, 0, 0);
		break;

	case WM_PLAYER_MODE_OPTION:
		bf = (DRAW_BUFFER *)GetWindowLong(hWnd, GWL_USERDATA);
		if (bf == NULL) {
			break;
		}
		bf->option = wParam;
		bf->show_all = wParam;
		draw_free(hWnd, bf);
		draw_init(hWnd, bf);
		SendMessage(hWnd, WM_PLAYER_REDRAW, 0, 0);
		break;

	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

/*
 * score_player_regist - ウィンドウクラスの登録
 */
BOOL score_player_regist(const HINSTANCE hInstance)
{
	WNDCLASS wc;

	wc.style = 0;
	wc.lpfnWndProc = (WNDPROC)score_player_proc;
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
 * score_player_create - ウィンドウの作成
 */
HWND score_player_create(const HINSTANCE hInstance, const HWND pWnd, int id, PLAYER_INFO *pi)
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
