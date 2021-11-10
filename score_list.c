/*
 * n01
 *
 * score_list.c
 *
 * Copyright (C) 1996-2008 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

/* Include Files */
#define _INC_OLE
#include <windows.h>
#undef	_INC_OLE

#include "general.h"
#include "Memory.h"
#include "String.h"
#include "Message.h"
#include "font.h"
#include "nEdit.h"
#include "score_info.h"
#include "score_list.h"
#include "score_com.h"
#include "score_save.h"
#include "finish.h"
#include "middle.h"
#include "arrange.h"
#include "check_out.h"
#include "recovery.h"

#include "resource.h"

/* Define */
#define WINDOW_CLASS				TEXT("score_list_wnd")

#define CHAR_COUNT					17
#define HEADER_LENGTH				6
#define SCORE_LENGTH				4
#define INPUT_LIMIT					3

#define ID_TIMER_COM				1
#define TIMER_INTERVAL_COM			1

// ホイールメッセージ
#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL				0x020A
#endif
#define WHEEL_COUNT					3

#define SWAP(a, b)					{a = b - a; b -= a; a += b;}

/* Global Variables */
extern HINSTANCE hInst;
extern OPTION_INFO op;
extern SCORE_INFO si;

typedef struct _PREV_INFO {
	BOOL prev_flag;

	SCORE_INFO si;
	int view_leg;
	int input_x;
	int input_y;
} PREV_INFO;

typedef struct _DRAW_BUFFER {
	HWND hedit;				// 入力EDITのハンドル
	int input_x;			// 入力横位置
	int input_y;			// 入力縦位置

	int view_leg;			// 表示レッグ
	BOOL half;				// ハーフモード
	BOOL lock;				// ロックモード

	int max_y;				// スクロール最大値
	int pos_y;				// スクロールボックスの位置
	int page_y;				// ページサイズ

	int input_left[2];		// 入力スコアの左座標
	int score_left[2];		// 残りスコアの左座標
	int score_right[2];		// 残りスコアの右座標

	// 描画情報
	HDC draw_dc;
	HBITMAP draw_bmp;
	HBITMAP draw_ret_bmp;

	// 背景描画情報
	HDC back_dc;
	HBITMAP back_bmp;
	HBITMAP back_ret_bmp;
	int back_width;
	int back_height;
	BOOL back_redraw;
	int back_first;

	// 円描画情報
	HDC ellipse_dc;
	HBITMAP ellipse_bmp;
	HBITMAP ellipse_ret_bmp;

	// フォント
	HFONT header_font;
	int header_height;
	HFONT score_font;
	int score_height;
	HFONT back_ret_font;
	HFONT draw_ret_font;

	// ブラシ
	HBRUSH back_brush;
	HBRUSH odd_back_brush;
	HBRUSH header_back_brush;

	// ペン
	HPEN line_pen;
	HPEN separate_pen;
	HPEN separate_bold_pen;
	HPEN finish_pen;
	HPEN ton_circle_pen;
	int pen_size;

	// スコア情報
	SCORE_INFO *si;

	// 戻り情報
	PREV_INFO prev_info;
	PREV_INFO next_info;
} DRAW_BUFFER;

/* Local Function Prototypes */
static void set_scrollbar(const HWND hWnd, DRAW_BUFFER *bf, const SCORE_INFO *si);
static BOOL draw_init(const HWND hWnd, DRAW_BUFFER *bf);
static BOOL draw_free(const HWND hWnd, DRAW_BUFFER *bf);
static void draw_text(const HDC hdc, const TCHAR *str, const int len, const RECT *rect);
static BOOL draw_background(const DRAW_BUFFER *bf, const int first);
static BOOL draw_line(DRAW_BUFFER *bf, const SCORE_INFO *si, const RECT *rect, const int round, int *left_score);
static void set_player_info(const DRAW_BUFFER *bf, SCORE_INFO *si, const int player, const int round);
static TYPE_CHECK_OUT get_auto_check_out_count(const int left, const int old_left, const int finish);
static TYPE_CHECK_OUT get_check_out_count(const HWND hWnd, const int left, const int old_left, const int finish, const int default_count);
static int get_left(const DRAW_BUFFER *bf, const SCORE_INFO *si, const int round, const int player);
static BOOL set_score(const HWND hWnd, DRAW_BUFFER *bf, SCORE_INFO *si, const TYPE_SCORE score);
static BOOL set_finish(const HWND hWnd, DRAW_BUFFER *bf, SCORE_INFO *si, int count, const BOOL lock);
static BOOL set_middle(const HWND hWnd, DRAW_BUFFER *bf, SCORE_INFO *si);
static BOOL point_to_score(const HWND hWnd, const DRAW_BUFFER *bf, const SCORE_INFO *si, const int x, const int y, int *ret_x, int *ret_y);
static BOOL move_input(const HWND hWnd, DRAW_BUFFER *bf, const SCORE_INFO *si, int x, int y);
static BOOL show_edit(const HWND hWnd, DRAW_BUFFER *bf, SCORE_INFO *si, const BOOL score_set, const BOOL ensure_flag);
static LRESULT CALLBACK score_list_proc(const HWND hWnd, const UINT msg, WPARAM wParam, LPARAM lParam);

/*
 * set_scrollbar - スクロールバーの設定
 */
static void set_scrollbar(const HWND hWnd, DRAW_BUFFER *bf, const SCORE_INFO *si)
{
	SCROLLINFO sci;
	RECT rect;

	GetClientRect(hWnd, &rect);
	bf->page_y = ((rect.bottom - bf->header_height) + 1) / bf->score_height;

	// 縦スクロールバー
	if (bf->page_y < si->leg[bf->view_leg].max_round + 1) {
		EnableScrollBar(hWnd, SB_VERT, ESB_ENABLE_BOTH);

		bf->max_y = si->leg[bf->view_leg].max_round - (bf->page_y - 1);
		bf->pos_y = (bf->pos_y < bf->max_y) ? bf->pos_y : bf->max_y;

		ZeroMemory(&sci, sizeof(SCROLLINFO));
		sci.cbSize = sizeof(SCROLLINFO);
		sci.fMask = SIF_POS | SIF_RANGE | SIF_PAGE | ((op.view_scroll_bar == 0) ? SIF_DISABLENOSCROLL : 0);
		sci.nPage = bf->page_y;
		sci.nMax = si->leg[bf->view_leg].max_round;
		sci.nPos = bf->pos_y;
		SetScrollInfo(hWnd, SB_VERT, &sci, TRUE);
	} else {
		EnableScrollBar(hWnd, SB_VERT, ESB_DISABLE_BOTH);

		bf->max_y = bf->pos_y = 0;

		ZeroMemory(&sci, sizeof(SCROLLINFO));
		sci.cbSize = sizeof(SCROLLINFO);
		sci.fMask = SIF_POS | SIF_PAGE | SIF_RANGE | ((op.view_scroll_bar == 0) ? SIF_DISABLENOSCROLL : 0);
		sci.nMax = 1;
		SetScrollInfo(hWnd, SB_VERT, &sci, TRUE);
	}
}

/*
 * draw_init - 描画情報の初期化
 */
static BOOL draw_init(const HWND hWnd, DRAW_BUFFER *bf)
{
	HDC hdc;
	HFONT ret_font;
	TEXTMETRIC tm;
	RECT rect;
	int i;

	GetClientRect(hWnd, &rect);
	hdc = GetDC(hWnd);

	// 前景
	if (bf->half == TRUE) {
		bf->score_font = font_create(op.font_name, rect.right / (CHAR_COUNT + 5), 0, FALSE, FALSE);
	} else {
		bf->score_font = font_create(op.font_name, rect.right / CHAR_COUNT, 0, FALSE, FALSE);
	}
	ret_font = SelectObject(hdc, bf->score_font);
	GetTextMetrics(hdc, &tm);
	SelectObject(hdc, ret_font);
	bf->score_height = tm.tmHeight + tm.tmHeight / 6;
	bf->draw_bmp = CreateCompatibleBitmap(hdc, rect.right, bf->score_height);
	bf->draw_ret_bmp = SelectObject(bf->draw_dc, bf->draw_bmp);
	bf->draw_ret_font = SelectObject(bf->draw_dc, bf->score_font);

	// 背景
	bf->header_font = font_create(op.font_name, rect.right / CHAR_COUNT / 2, 0, FALSE, FALSE);
	ret_font = SelectObject(hdc, bf->header_font);
	GetTextMetrics(hdc, &tm);
	SelectObject(hdc, ret_font);
	bf->header_height = tm.tmHeight + tm.tmHeight / 6;
	bf->back_width = rect.right;
	bf->back_height = bf->header_height + bf->score_height * 2;
	bf->back_redraw = TRUE;
	bf->back_bmp = CreateCompatibleBitmap(hdc, rect.right, bf->back_height);
	bf->back_ret_bmp = SelectObject(bf->back_dc, bf->back_bmp);
	bf->back_ret_font = SelectObject(bf->back_dc, bf->header_font);

	// 位置の設定
	i = (rect.right * 100) / CHAR_COUNT;
	bf->input_left[0] = 0;
	bf->score_left[0] = i * 3 / 100;
	bf->score_right[0] = i * 7 / 100;
	bf->input_left[1] = i * 10 / 100;
	bf->score_left[1] = i * 13 / 100;
	bf->score_right[1] = rect.right;

	// 円
	bf->ellipse_bmp = CreateCompatibleBitmap(hdc, (bf->score_left[1] - bf->input_left[1]) * 2, bf->score_height * 2);
	bf->ellipse_ret_bmp = SelectObject(bf->ellipse_dc, bf->ellipse_bmp);

	ReleaseDC(hWnd, hdc);
	return TRUE;
}

/*
 * draw_free - 描画情報の解放
 */
static BOOL draw_free(const HWND hWnd, DRAW_BUFFER *bf)
{
	if (bf->draw_dc != NULL) {
		SelectObject(bf->draw_dc, bf->draw_ret_bmp);
		SelectObject(bf->draw_dc, bf->draw_ret_font);
	}
	if (bf->back_dc != NULL) {
		SelectObject(bf->back_dc, bf->back_ret_bmp);
		SelectObject(bf->back_dc, bf->back_ret_font);
	}
	if (bf->ellipse_dc != NULL) {
		SelectObject(bf->ellipse_dc, bf->ellipse_ret_bmp);
	}
	if (bf->draw_bmp != NULL) {
		DeleteObject(bf->draw_bmp);
		bf->draw_bmp = NULL;
	}
	if (bf->back_bmp != NULL) {
		DeleteObject(bf->back_bmp);
		bf->back_bmp = NULL;
	}
	if (bf->ellipse_bmp != NULL) {
		DeleteObject(bf->ellipse_bmp);
		bf->ellipse_bmp = NULL;
	}
	if (bf->score_font != NULL) {
		DeleteObject(bf->score_font);
		bf->score_font = NULL;
	}
	if (bf->header_font != NULL) {
		DeleteObject(bf->header_font);
		bf->header_font = NULL;
	}
	return TRUE;
}

/*
 * draw_text - テキストの描画
 */
static void draw_text(const HDC hdc, const TCHAR *str, const int len, const RECT *rect)
{
	SIZE sz;
	int left, top;

	GetTextExtentPoint32(hdc, str, len, &sz);
	left = rect->left + (rect->right - rect->left - sz.cx) / 2;
	top = rect->top + (rect->bottom - rect->top - sz.cy) / 2;
	ExtTextOut(hdc, left, top, ETO_CLIPPED, rect, str, len, NULL);
}

/*
 * draw_background - 背景の描画
 */
static BOOL draw_background(const DRAW_BUFFER *bf, const int first)
{
	RECT draw_rect;
	HPEN ret_pen;
	RECT rect;
	TCHAR buf[BUF_SIZE];
	int height;
	int j;

	// 背景の塗りつぶし
	SetRect(&rect, 0, 0, bf->back_width, bf->back_height);
	FillRect(bf->back_dc, &rect, bf->back_brush);

	if (op.ci.background != op.ci.odd_background) {
		// 奇数行の塗りつぶし
		height = bf->header_height + bf->score_height;
		SetRect(&draw_rect, 0, height, bf->back_width, height + bf->score_height);
		FillRect(bf->back_dc, &draw_rect, bf->odd_back_brush);
	}

	// ヘッダの塗りつぶし
	SetRect(&draw_rect, 0, 0, bf->back_width, bf->header_height);
	FillRect(bf->back_dc, &draw_rect, bf->header_back_brush);
	SetRect(&draw_rect, bf->score_right[0], 0, bf->input_left[1], bf->back_height);
	FillRect(bf->back_dc, &draw_rect, bf->header_back_brush);

	// ヘッダの描画
	SetTextColor(bf->back_dc, op.ci.header_text);
	SetBkColor(bf->back_dc, op.ci.header_background);
	for (j = 0; j < 2; j++) {
		message_copy_res(IDS_STRING_SCORED, buf);
		SetRect(&draw_rect, bf->input_left[j], 0, bf->score_left[j], bf->header_height - 1);
		draw_text(bf->back_dc, buf, lstrlen(buf), &draw_rect);
		if (first == j && bf->half == TRUE) {
			message_copy_res(IDS_STRING_FIRST_MARK, buf);
			lstrcat(buf, TEXT(" "));
			message_copy_res(IDS_STRING_TO_GO, buf + lstrlen(buf));
		} else {
			message_copy_res(IDS_STRING_TO_GO, buf);
		}
		SetRect(&draw_rect, bf->score_left[j] + 1, 0, bf->score_right[j], bf->header_height - 1);
		draw_text(bf->back_dc, buf, lstrlen(buf), &draw_rect);
	}

	// 線の描画
	ret_pen = SelectObject(bf->back_dc, bf->line_pen);
	// 縦線
	for (j = 0; j < 2; j++) {
		MoveToEx(bf->back_dc, bf->score_left[j], 0, NULL);
		LineTo(bf->back_dc, bf->score_left[j], bf->back_height);
	}
	// 横線
	for (height = bf->header_height + bf->score_height - 1; height < bf->back_height; height = height + bf->score_height) {
		MoveToEx(bf->back_dc, 0, height, NULL);
		LineTo(bf->back_dc, bf->back_width, height);
	}
	SelectObject(bf->back_dc, ret_pen);

	// 太線の描画
	ret_pen = SelectObject(bf->back_dc, bf->separate_bold_pen);
	// ヘッダ線
	MoveToEx(bf->back_dc, -1, bf->header_height - 1, NULL);
	LineTo(bf->back_dc, bf->back_width, bf->header_height - 1);
	// 中央線
	MoveToEx(bf->back_dc, bf->score_right[0] + 1, 0, NULL);
	LineTo(bf->back_dc, bf->score_right[0] + 1, bf->back_height);
	MoveToEx(bf->back_dc, bf->input_left[1] - 1, 0, NULL);
	LineTo(bf->back_dc, bf->input_left[1] - 1, bf->back_height);
	SelectObject(bf->back_dc, ret_pen);
	return TRUE;
}

/*
 * draw_line - スコアシートの描画
 */
static BOOL draw_line(DRAW_BUFFER *bf, const SCORE_INFO *si, const RECT *rect, const int round, int *left_score)
{
	RECT draw_rect;
	HPEN ret_pen;
	HBRUSH ret_brush;
	HBRUSH ret_ellipse_brush;
	TCHAR buf[BUF_SIZE];
	int i, j;

	// 背景のコピー
	BitBlt(bf->draw_dc, rect->left, 0, rect->right, bf->score_height, bf->back_dc, rect->left,
		(round / 2 == (round + 1) / 2) ? bf->header_height : (bf->header_height + bf->score_height),
		SRCCOPY);

	// 横区切り線
	if (op.view_separate == 1) {
		ret_pen = SelectObject(bf->draw_dc, bf->separate_pen);
		if (round % 5 == 0) {
			MoveToEx(bf->draw_dc, rect->left, bf->score_height - 1, NULL);
			LineTo(bf->draw_dc, rect->right, bf->score_height - 1);
		}
		SelectObject(bf->draw_dc, ret_pen);
	}

	// To Go
	if (round == 0) {
		SetTextColor(bf->draw_dc, op.ci.togo_text);
		SetBkColor(bf->draw_dc, op.ci.background);
		for (j = 0; j < 2; j++) {
			SetRect(&draw_rect, bf->input_left[j], 0, bf->score_left[j], bf->score_height - 1);
			FillRect(bf->draw_dc, &draw_rect, bf->header_back_brush);

			_itot(si->player[j].start_score, buf, 10);
			SetRect(&draw_rect, bf->score_left[j] + 1, 0, bf->score_right[j], bf->score_height);
			draw_text(bf->draw_dc, buf, lstrlen(buf), &draw_rect);
		}
		return TRUE;
	}

	// ナンバーの描画
	if (si->round_limit != 0 && round > si->leg[bf->view_leg].max_round) {
		return TRUE;
	}
	if (si->round_limit != 0 && round > si->round - 3) {
		SetTextColor(bf->draw_dc, op.ci.last3number_text);
	} else {
		SetTextColor(bf->draw_dc, op.ci.header_text);
	}
	SetBkColor(bf->draw_dc, op.ci.header_background);
	if (op.view_throw_count == 0) {
		_itot(round, buf, 10);
	} else {
		_itot(round * 3, buf, 10);
	}
	SetRect(&draw_rect, bf->score_right[0] + 2, 0, bf->input_left[1] - 2, bf->score_height - 1);
	draw_text(bf->draw_dc, buf, lstrlen(buf), &draw_rect);

	// スコアの描画
	i = round - 1;
	if (i > si->leg[bf->view_leg].current_round) {
		return TRUE;
	}
	if (round / 2 == (round + 1) / 2) {
		SetBkColor(bf->draw_dc, op.ci.background);
		ret_brush = SelectObject(bf->draw_dc, bf->back_brush);
	} else {
		SetBkColor(bf->draw_dc, op.ci.odd_background);
		ret_brush = SelectObject(bf->draw_dc, bf->odd_back_brush);
	}
	for (j = 0; j < 2; j++) {
		if (i == si->leg[bf->view_leg].current_round &&
			(si->leg[bf->view_leg].first != j || si->leg[bf->view_leg].current_player == j)) {
			continue;
		}
		if (si->leg[bf->view_leg].score[j][i] < 0) {
			// フィニッシュ本数
			if (bf->pen_size == 4) {
				SetRect(&draw_rect, 0, 0, (bf->score_left[j] - bf->input_left[j]) * 2, (bf->score_height - 1) * 2);
				FillRect(bf->ellipse_dc, &draw_rect, GetCurrentObject(bf->draw_dc, OBJ_BRUSH));
				ret_ellipse_brush = SelectObject(bf->ellipse_dc, GetCurrentObject(bf->draw_dc, OBJ_BRUSH));
				ret_pen = SelectObject(bf->ellipse_dc, bf->finish_pen);
				Ellipse(bf->ellipse_dc,
					(bf->score_left[j] - bf->input_left[j] - (bf->score_height - 1)) * 2 / 2 + 2,
					2,
					(bf->score_left[j] - bf->input_left[j]) * 2 - (bf->score_left[j] - bf->input_left[j] - (bf->score_height - 1)) * 2 / 2 - 2,
					(bf->score_height - 1) * 2 - 2);
				SelectObject(bf->ellipse_dc, ret_pen);
				SelectObject(bf->ellipse_dc, ret_ellipse_brush);
				StretchBlt(bf->draw_dc, bf->input_left[j], 0, bf->score_left[j] - bf->input_left[j], bf->score_height - 1,
					bf->ellipse_dc, 0, 0, (bf->score_left[j] - bf->input_left[j]) * 2, (bf->score_height - 1) * 2, SRCCOPY);
			} else {
				ret_pen = SelectObject(bf->draw_dc, bf->finish_pen);
				Ellipse(bf->draw_dc,
					bf->input_left[j] + (bf->score_left[j] - bf->input_left[j] - (bf->score_height - 1)) / 2 + 1,
					1,
					bf->score_left[j] - (bf->score_left[j] - bf->input_left[j] - (bf->score_height - 1)) / 2 - 1,
					(bf->score_height - 1) - 1);
				SelectObject(bf->draw_dc, ret_pen);
			}

			SetTextColor(bf->draw_dc, op.ci.scored_text);
			_itot(si->leg[bf->view_leg].score[j][i] * -1, buf, 10);
			SetRect(&draw_rect, bf->input_left[j], 0, bf->score_left[j], bf->score_height - 1);
			draw_text(bf->draw_dc, buf, lstrlen(buf), &draw_rect);
		} else {
			// スコア
			if (si->leg[bf->view_leg].score[j][i] >= 100 && op.view_ton_circle == 1) {
				// TON
				if (bf->pen_size == 4) {
					SetRect(&draw_rect, 0, 0, (bf->score_left[j] - bf->input_left[j]) * 2, (bf->score_height - 1) * 2);
					FillRect(bf->ellipse_dc, &draw_rect, GetCurrentObject(bf->draw_dc, OBJ_BRUSH));
					ret_ellipse_brush = SelectObject(bf->ellipse_dc, GetCurrentObject(bf->draw_dc, OBJ_BRUSH));
					ret_pen = SelectObject(bf->ellipse_dc, bf->ton_circle_pen);
					Ellipse(bf->ellipse_dc, 2, 2,
						(bf->score_left[j] - bf->input_left[j]) * 2 - 2, (bf->score_height - 1) * 2 - 2);
					SelectObject(bf->ellipse_dc, ret_pen);
					SelectObject(bf->ellipse_dc, ret_ellipse_brush);
					StretchBlt(bf->draw_dc, bf->input_left[j], 0, bf->score_left[j] - bf->input_left[j], bf->score_height - 1,
						bf->ellipse_dc, 0, 0, (bf->score_left[j] - bf->input_left[j]) * 2, (bf->score_height - 1) * 2, SRCCOPY);
				} else {
					ret_pen = SelectObject(bf->draw_dc, bf->ton_circle_pen);
					Ellipse(bf->draw_dc,
						bf->input_left[j] + 1, 1,
						bf->score_left[j] - 1, (bf->score_height - 1) - 1);
					SelectObject(bf->draw_dc, ret_pen);
				}
			}
			SetTextColor(bf->draw_dc, op.ci.scored_text);
			_itot(si->leg[bf->view_leg].score[j][i], buf, 10);
			SetRect(&draw_rect, bf->input_left[j], 0, bf->score_left[j], bf->score_height - 1);
			draw_text(bf->draw_dc, buf, lstrlen(buf), &draw_rect);

			// 残りスコア
			left_score[j] -= si->leg[bf->view_leg].score[j][i];
			SetTextColor(bf->draw_dc, op.ci.togo_text);
			_itot(left_score[j], buf, 10);
			SetRect(&draw_rect, bf->score_left[j] + 1, 0, bf->score_right[j], bf->score_height - 1);
			draw_text(bf->draw_dc, buf, lstrlen(buf), &draw_rect);
		}
	}
	SelectObject(bf->draw_dc, ret_brush);
	return TRUE;
}

/*
 * set_player_info - プレイヤー情報設定
 */
static void set_player_info(const DRAW_BUFFER *bf, SCORE_INFO *si, const int player, const int round)
{
	int left;
	int i, j;

	// 算出用ラウンド数取得
	if (round >= 0) {
		j = round;
	} else if ((si->leg[bf->view_leg].first != si->leg[bf->view_leg].current_player &&
		si->leg[bf->view_leg].current_player == player) ||
		si->leg[bf->view_leg].first == si->leg[bf->view_leg].current_player) {
		j = si->leg[bf->view_leg].current_round;
	} else {
		j = si->leg[bf->view_leg].current_round + 1;
	}

	// 初期化
	si->leg[bf->view_leg].ton_count[player] = 0;
	si->leg[bf->view_leg].ton00_count[player] = 0;
	si->leg[bf->view_leg].ton40_count[player] = 0;
	si->leg[bf->view_leg].ton80_count[player] = 0;
	si->leg[bf->view_leg].first9_score[player] = 0;
	si->leg[bf->view_leg].first9_darts[player] = 0;
	si->leg[bf->view_leg].check_out_aim[player] = 0;
	si->leg[bf->view_leg].failure_2_80[player] = 0;
	si->leg[bf->view_leg].failure_81_130[player] = 0;
	si->leg[bf->view_leg].failure_131[player] = 0;

	// カレントレッグの統計
	left = si->player[player].start_score;
	for (i = 0; i < j; i++) {
		// TON数設定
		if (si->leg[bf->view_leg].score[player][i] >= 100) {
			si->leg[bf->view_leg].ton_count[player]++;
		}
		// 100数設定
		if (si->leg[bf->view_leg].score[player][i] >= 100 && si->leg[bf->view_leg].score[player][i] < 140) {
			si->leg[bf->view_leg].ton00_count[player]++;
		}
		// 140数設定
		if (si->leg[bf->view_leg].score[player][i] >= 140 && si->leg[bf->view_leg].score[player][i] < 180) {
			si->leg[bf->view_leg].ton40_count[player]++;
		}
		// 180数設定
		if (si->leg[bf->view_leg].score[player][i] == 180) {
			si->leg[bf->view_leg].ton80_count[player]++;
		}
		// 3ラウンドスコア平均設定 (501の場合のみ)
		if (i < 3 && si->player[player].start_score == 501) {
			si->leg[bf->view_leg].first9_score[player] += si->leg[bf->view_leg].score[player][i];
			si->leg[bf->view_leg].first9_darts[player] += 3;
		}
		// チェックアウト本数設定
		si->leg[bf->view_leg].check_out_aim[player] += si->tmp_check_out[player][i];

		// Finish stats
		if (si->tmp_check_out[player][i] > 0 && si->leg[bf->view_leg].score[player][i] >= 0) {
			if (left >= 2 && left <= 80) {
				si->leg[bf->view_leg].failure_2_80[player]++;
			} else if (left >= 81 && left <= 130) {
				si->leg[bf->view_leg].failure_81_130[player]++;
			} else if (left >= 131) {
				si->leg[bf->view_leg].failure_131[player]++;
			}
		}
		left -= si->leg[bf->view_leg].score[player][i];
	}

	// 初期化
	si->player[player].stat.ton_count = 0;
	si->player[player].stat.ton00_count = 0;
	si->player[player].stat.ton40_count = 0;
	si->player[player].stat.ton80_count = 0;
	si->player[player].stat.all_score = 0;
	si->player[player].stat.all_darts = 0;
	si->player[player].stat.win_darts = 0;
	si->player[player].stat.win_count = 0;
	si->player[player].stat.first9_score = 0;
	si->player[player].stat.first9_darts = 0;
	si->player[player].stat.check_out_aim = 0;
	si->player[player].stat.failure_2_80 = 0;
	si->player[player].stat.failure_81_130 = 0;
	si->player[player].stat.failure_131 = 0;

	// カレントセットの統計
	for (i = 0; i <= bf->view_leg; i++) {
		// TON数設定
		si->player[player].stat.ton_count += si->leg[i].ton_count[player];
		if (si->leg[i].end_flag == TRUE && si->leg[i].winner == player && si->leg[i].out_left >= 100) {
			si->player[player].stat.ton_count++;
		}
		// 100数設定
		si->player[player].stat.ton00_count += si->leg[i].ton00_count[player];
		if (si->leg[i].end_flag == TRUE && si->leg[i].winner == player && si->leg[i].out_left >= 100 && si->leg[i].out_left < 140) {
			si->player[player].stat.ton00_count++;
		}
		// 140数設定
		si->player[player].stat.ton40_count += si->leg[i].ton40_count[player];
		if (si->leg[i].end_flag == TRUE && si->leg[i].winner == player && si->leg[i].out_left >= 140 && si->leg[i].out_left < 180) {
			si->player[player].stat.ton40_count++;
		}
		// 180数設定
		si->player[player].stat.ton80_count += si->leg[i].ton80_count[player];
		if (si->leg[i].end_flag == TRUE && si->leg[i].winner == player && si->leg[i].out_left == 180) {
			si->player[player].stat.ton80_count++;
		}
		// ダーツ1本あたりのスコア平均設定
		si->player[player].stat.all_score += si->player[player].start_score - si->leg[i].all_score[player];
		si->player[player].stat.all_darts += si->leg[i].all_darts[player];
		// ダーツ数平均設定
		if (si->leg[i].end_flag == TRUE && si->leg[i].winner == player && si->leg[i].darts != 0) {
			si->player[player].stat.win_darts += si->leg[i].darts;
			si->player[player].stat.win_count++;
		}
		// 3ラウンドスコア平均設定
		si->player[player].stat.first9_score += si->leg[i].first9_score[player];
		si->player[player].stat.first9_darts += si->leg[i].first9_darts[player];
		// チェックアウト本数設定
		si->player[player].stat.check_out_aim += si->leg[i].check_out_aim[player];

		// Finish stats
		si->player[player].stat.failure_2_80 += si->leg[i].failure_2_80[player];
		si->player[player].stat.failure_81_130 += si->leg[i].failure_81_130[player];
		si->player[player].stat.failure_131 += si->leg[i].failure_131[player];
	}
}

/*
 * get_auto_check_out_count - チェックアウトを狙った本数の自動算出
 */
static TYPE_CHECK_OUT get_auto_check_out_count(const int left, const int old_left, const int finish)
{
	if (left == 0) {
		// finish
		if (old_left > 50) {
			if (old_left < 99 || old_left == 100 || old_left == 101 || old_left == 104 || old_left == 107 || old_left == 110) {
				// 前回2本で上がれる
				return (TYPE_CHECK_OUT)((finish == 1) ? 1 : finish - 1);
			} else {
				// 前回3本で上がれる
				return 1;
			}
		} else if (old_left == 50 || (old_left <= 40 && old_left == (old_left / 2) * 2)) {
			// 前回1本で上がれる
			return (TYPE_CHECK_OUT)finish;
		} else {
			// 前回2本で上がれる(前回残り50点以下で奇数)
			return (TYPE_CHECK_OUT)((finish == 1) ? 1 : finish - 1);
		}
	}
	if (left == old_left || left <= 50) {
		// bust or 今回残り50点以下(ダブルを狙った可能性あり)
		if (old_left > 50) {
			if (old_left < 99 || old_left == 100 || old_left == 101 || old_left == 104 || old_left == 107 || old_left == 110) {
				// 前回2本で上がれる
				if (left == old_left || left <= 40) {
					// ダブルを狙った可能性あり
					return 2;
				} else {
					// BULLを狙った可能性あり
					return 1;
				}
			} else if (old_left <= 158 || old_left == 160 || old_left == 161 || old_left == 164 || old_left == 167 || old_left == 170) {
				// 前回3本で上がれる
				return 1;
			} else {
				// 前回上がり目が出ていない
				return 0;
			}
		} else if (old_left == 50 || (old_left <= 40 && old_left == (old_left / 2) * 2)) {
			// 前回1本で上がれる
			if (left == old_left || left <= 40) {
				// ダブルを狙った可能性あり
				return 3;
			} else {
				// BULLを狙った可能性あり
				return 1;
			}
		} else {
			// 前回2本で上がれる(前回残り50点以下で奇数)
			if (left == old_left || left <= 40) {
				// ダブルを狙った可能性あり
				return 2;
			} else {
				// 上がり目が出ていない(前回残り41～49点で今回残り41点以上)
				return 0;
			}
		}
	}
	return 0;
}

/*
 * get_check_out_count - チェックアウトを狙った本数取得
 */
static TYPE_CHECK_OUT get_check_out_count(const HWND hWnd, const int left, const int old_left, const int finish, const int default_count)
{
	TYPE_CHECK_OUT ret;

	ret = get_auto_check_out_count(left, old_left, finish);
	if (ret == 0) {
		return 0;
	}
	if (left == 0 && ret == 1) {
		return ret;
	}
	if (default_count != -1) {
		ret = (TYPE_CHECK_OUT)default_count;
	}
	if (finish != 0) {
		ret *= -1;
	}
	if (show_check_out(hInst, hWnd, &ret) == FALSE) {
		return -1;
	}
	return ret;
}

/*
 * get_left - 残り点数取得
 */
static int get_left(const DRAW_BUFFER *bf, const SCORE_INFO *si, const int round, const int player)
{
	int ret;
	int i, j;

	// 算出用ラウンド数取得
	if (round >= 0) {
		j = round;
	} else if ((si->leg[bf->view_leg].first != si->leg[bf->view_leg].current_player &&
		si->leg[bf->view_leg].current_player == player) ||
		si->leg[bf->view_leg].first == si->leg[bf->view_leg].current_player) {
		j = si->leg[bf->view_leg].current_round;
	} else {
		j = si->leg[bf->view_leg].current_round + 1;
	}

	// 残りスコア取得
	ret = si->player[player].start_score;
	for (i = 0; i < j; i++) {
		ret -= si->leg[bf->view_leg].score[player][i];
	}
	return ret;
}

/*
 * set_score - スコア設定
 */
static BOOL set_score(const HWND hWnd, DRAW_BUFFER *bf, SCORE_INFO *si, const TYPE_SCORE score)
{
	TCHAR err_str[BUF_SIZE];
	TYPE_SCORE *alloc_score;
	TYPE_CHECK_OUT *alloc_check_out;
	TYPE_CHECK_OUT check_out;
	TYPE_SCORE tmp_score;
	TYPE_CHECK_OUT tmp_check_out;
	int tmp_left;
	int left;
	int ret;
	int i;

	// 入力エラーのチェック
	if (score > 180 ||
		score == 163 || score == 166 || score == 169 ||
		score == 172 || score == 173 || score == 175 || score == 176 || score == 178 || score == 179) {
		MessageBeep(0xFFFFFFFF);
		return FALSE;
	}
	if (get_left(bf, si, bf->input_y, bf->input_x) - score == 0) {
		// フィニッシュ
		i = show_finish(hInst, hWnd, score);
		if (i == 0) {
			show_edit(hWnd, bf, si, TRUE, FALSE);
			return FALSE;
		}
		set_finish(hWnd, bf, si, i, FALSE);
		return FALSE;
	}
	if (get_left(bf, si, bf->input_y, bf->input_x) - score < 0) {
		if (score == si->leg[bf->view_leg].score[bf->input_x][bf->input_y]) {
			return TRUE;
		}
		// 残りスコアが 0 点以下
		MessageBeep(0xFFFFFFFF);
		return FALSE;
	}

	tmp_score = si->leg[bf->view_leg].score[bf->input_x][bf->input_y];
	tmp_left = si->player[bf->input_x].left;
	tmp_check_out = si->tmp_check_out[bf->input_x][bf->input_y];

	// チェックアウト本数取得
	if (si->player[bf->input_x].check_out_mode == TRUE) {
		left = get_left(bf, si, bf->input_y, bf->input_x) - score;
		if (si->leg[bf->view_leg].score[bf->input_x][bf->input_y] == score &&
			(bf->input_y != si->leg[bf->view_leg].current_round ||
			(bf->input_y == si->leg[bf->view_leg].current_round &&
			si->leg[bf->view_leg].current_player != si->leg[bf->view_leg].first &&
			si->leg[bf->view_leg].first == bf->input_x))) {
			i = si->tmp_check_out[bf->input_x][bf->input_y];
		} else {
			i = -1;
		}
		if ((check_out = get_check_out_count(hWnd, left, left + score, 0, i)) == -1) {
			return FALSE;
		}
		si->tmp_check_out[bf->input_x][bf->input_y] = check_out;
	}

	// スコアの設定
	si->leg[bf->view_leg].score[bf->input_x][bf->input_y] = score;
	si->player[bf->input_x].left = get_left(bf, si,
		((bf->input_y == si->leg[bf->view_leg].current_round) ? si->leg[bf->view_leg].current_round + 1 : -1),
		bf->input_x);
	// プラグイン処理
	ret = plugin_execute_all(GetParent(hWnd), CALLTYPE_INPUT_END, si, score);
	if (ret & PLUGIN_CANCEL) {
		si->leg[bf->view_leg].score[bf->input_x][bf->input_y] = tmp_score;
		si->player[bf->input_x].left = tmp_left;
		si->tmp_check_out[bf->input_x][bf->input_y] = tmp_check_out;
		return FALSE;
	} else if (ret & PLUGIN_DATA_MODIFIED) {
		if (si->leg[bf->view_leg].score[bf->input_x][bf->input_y] < 0) {
			set_finish(hWnd, bf, si, si->leg[bf->view_leg].score[bf->input_x][bf->input_y] * -1, si->player[bf->input_x].lock);
			return FALSE;
		}
		si->player[bf->input_x].left = get_left(bf, si,
			((bf->input_y == si->leg[bf->view_leg].current_round) ? si->leg[bf->view_leg].current_round + 1 : -1),
			bf->input_x);
	}
	si->leg[bf->view_leg].all_score[bf->input_x] = si->player[bf->input_x].left;
	if ((bf->input_y == si->leg[bf->view_leg].current_round) ||
		(bf->input_y != si->leg[bf->view_leg].current_round &&
		si->leg[bf->view_leg].current_player != si->leg[bf->view_leg].first &&
		si->leg[bf->view_leg].current_player != bf->input_x)) {
		si->leg[bf->view_leg].all_darts[bf->input_x] = (si->leg[bf->view_leg].current_round + 1) * 3;
	} else {
		si->leg[bf->view_leg].all_darts[bf->input_x] = si->leg[bf->view_leg].current_round * 3;
	}
	set_player_info(bf, si, bf->input_x,
		((bf->input_y == si->leg[bf->view_leg].current_round) ? si->leg[bf->view_leg].current_round + 1 : -1));
	SendMessage(GetParent(hWnd), WM_WINDOW_REDRAW, bf->input_x, FALSE);

	if (bf->input_y == si->leg[bf->view_leg].current_round) {
		if (bf->input_x != si->leg[bf->view_leg].current_player && bf->input_x != si->leg[bf->view_leg].first) {
			// 先攻設定
			si->leg[bf->view_leg].first = bf->input_x;
			SendMessage(hWnd, WM_SCORE_REDRAW, 0, 0);
			SendMessage(GetParent(hWnd), WM_WINDOW_SET_FIRST, TRUE, 0);
		}
		if (bf->input_x == si->leg[bf->view_leg].first) {
			// プレイヤー移動
			si->leg[bf->view_leg].current_player = !bf->input_x;
		} else {
			// ラウンド移動
			si->leg[bf->view_leg].current_player = si->leg[bf->view_leg].first;
			si->leg[bf->view_leg].current_round++;
			if (si->leg[bf->view_leg].current_round >= si->leg[bf->view_leg].max_round) {
				si->leg[bf->view_leg].max_round++;
				if (si->leg[bf->view_leg].max_round > si->leg[bf->view_leg].alloc_round) {
					// ラウンド情報の再確保
					si->leg[bf->view_leg].alloc_round += ALLOC_ROUND;
					for (i = 0; i < 2; i++) {
						alloc_score = (TYPE_SCORE *)mem_calloc(sizeof(TYPE_SCORE) * (si->leg[bf->view_leg].alloc_round + 1));
						if (alloc_score == NULL) {
							message_get_error(GetLastError(), err_str);
							MessageBox(hWnd, err_str, APP_NAME, MB_ICONERROR);
							break;
						}
						CopyMemory(alloc_score, si->leg[bf->view_leg].score[i], sizeof(TYPE_SCORE) * si->leg[bf->view_leg].max_round);
						mem_free(&si->leg[bf->view_leg].score[i]);
						si->leg[bf->view_leg].score[i] = alloc_score;

						alloc_check_out = (TYPE_CHECK_OUT *)mem_calloc(sizeof(TYPE_CHECK_OUT) * (si->leg[bf->view_leg].alloc_round + 1));
						if (alloc_check_out == NULL) {
							message_get_error(GetLastError(), err_str);
							MessageBox(hWnd, err_str, APP_NAME, MB_ICONERROR);
							break;
						}
						CopyMemory(alloc_check_out, si->tmp_check_out[i], sizeof(TYPE_CHECK_OUT) * si->leg[bf->view_leg].max_round);
						mem_free(&si->tmp_check_out[i]);
						si->tmp_check_out[i] = alloc_check_out;
					}
				}
				if (si->round_limit == 1) {
					// ラウンド制限有り
					if (si->player[0].com == TRUE || si->player[1].com == TRUE) {
						// COMの場合は引き分けにする
						si->leg[bf->view_leg].end_flag = TRUE;
						si->leg[bf->view_leg].max_round--;
						show_edit(hWnd, bf, si, TRUE, FALSE);
						SendMessage(hWnd, WM_SCORE_REDRAW, 0, 0);
						MessageBox(hWnd, message_get_res(IDS_STRING_TIE), APP_NAME, MB_ICONINFORMATION);
						// 戻り情報の初期化
						bf->prev_info.prev_flag = FALSE;
						SendMessage(GetParent(hWnd), WM_WINDOW_SET_PREV_MENU, FALSE, 0);
						// LEGの移動
						if (SendMessage(hWnd, WM_SCORE_NEXT_LEG, 0, 0) == FALSE) {
							return FALSE;
						}
						SendMessage(hWnd, WM_SCORE_INIT_LEG, FALSE, FALSE);
						return FALSE;
					} else if (si->leg[bf->view_leg].current_round == si->round) {
						// Middle for diddle
						SendMessage(hWnd, WM_SCORE_REDRAW, (bf->input_y + 1) - bf->pos_y, 0);
						if (set_middle(hWnd, bf, si) == TRUE) {
							return FALSE;
						}
					}
				}
				set_scrollbar(hWnd, bf, si);
			}
		}
		if (si->player[si->leg[bf->view_leg].current_player].com == TRUE) {
			// COMの番
			EnableWindow(GetParent(hWnd), FALSE);
			SetTimer(hWnd, ID_TIMER_COM, TIMER_INTERVAL_COM, NULL);
		}
	}
	SendMessage(bf->hedit, EM_SETMODIFY, FALSE, 0);
	SendMessage(GetParent(hWnd), WM_WINDOW_SET_CURRENT, si->leg[bf->view_leg].current_player, 0);
	SendMessage(hWnd, WM_SCORE_REDRAW, (bf->input_y + 1) - bf->pos_y, 0);
	recovery_save(si, 1, bf->input_x, bf->input_y);
	return TRUE;
}

/*
 * set_finish - フィニッシュ設定
 */
static BOOL set_finish(const HWND hWnd, DRAW_BUFFER *bf, SCORE_INFO *si, int count, const BOOL lock)
{
	SCORE_INFO tmp_si;
	LEG_INFO tmp_leg;
	PREV_INFO tmp_prev_info;
	TCHAR buf[BUF_SIZE];
	TCHAR name[NAME_SIZE];
	TYPE_SCORE tmp_score;
	TYPE_CHECK_OUT check_out;
	TYPE_CHECK_OUT tmp_check_out;
	int left;
	int mb_flag;
	int ret;

	score_info_copy(&tmp_prev_info.si, si);
	tmp_prev_info.view_leg = bf->view_leg;
	tmp_prev_info.input_x = bf->input_x;
	tmp_prev_info.input_y = bf->input_y;
	tmp_prev_info.prev_flag = TRUE;

	tmp_si = *si;
	tmp_leg = si->leg[bf->view_leg];
	left = get_left(bf, si, bf->input_y, bf->input_x);

	if (left > 180 ||
		left == 163 || left == 166 || left == 169 ||
		left == 172 || left == 173 || left == 175 || left == 176 || left == 178 || left == 179) {
		MessageBeep(0xFFFFFFFF);
		score_info_free(&tmp_prev_info.si);
		return FALSE;
	}

	// チェックアウト本数取得
	tmp_check_out = si->tmp_check_out[bf->input_x][bf->input_y];
	if (si->player[bf->input_x].check_out_mode == TRUE) {
		if ((check_out = get_check_out_count(hWnd, 0, left, count, -1)) == -1) {
			score_info_free(&tmp_prev_info.si);
			return FALSE;
		}
		si->tmp_check_out[bf->input_x][bf->input_y] = check_out;
	}

	if (bf->input_y == si->leg[bf->view_leg].current_round &&
		bf->input_x != si->leg[bf->view_leg].current_player &&
		bf->input_x != si->leg[bf->view_leg].first) {
		// 先攻設定
		si->leg[bf->view_leg].first = bf->input_x;
		SendMessage(GetParent(hWnd), WM_WINDOW_SET_FIRST, TRUE, 0);
	}
	tmp_score = si->leg[bf->view_leg].score[bf->input_x][bf->input_y];
	si->leg[bf->view_leg].score[bf->input_x][bf->input_y] = (TYPE_SCORE)(count * -1);
	si->player[bf->input_x].left = 0;
	if (bf->input_x == si->leg[bf->view_leg].first) {
		// プレイヤー移動
		si->leg[bf->view_leg].current_player = !bf->input_x;
		si->leg[bf->view_leg].current_round = bf->input_y;
	} else {
		// ラウンド移動
		si->leg[bf->view_leg].current_player = si->leg[bf->view_leg].first;
		si->leg[bf->view_leg].current_round = bf->input_y + 1;
	}
	// 最大ラウンド数設定
	if (si->round_limit == 0 || (si->leg[bf->view_leg].max_round > si->round && bf->input_y >= si->round)) {
		si->leg[bf->view_leg].max_round = bf->input_y + 1;
	} else if (si->round_limit == 1 && si->leg[bf->view_leg].max_round > si->round && bf->input_y < si->round) {
		si->leg[bf->view_leg].max_round = si->round;
	}
	// プラグイン処理
	ret = plugin_execute_all(GetParent(hWnd), CALLTYPE_FINISH, si, count * -1);
	if (ret & PLUGIN_CANCEL) {
		*si = tmp_si;
		si->leg[bf->view_leg] = tmp_leg;
		si->leg[bf->view_leg].score[bf->input_x][bf->input_y] = tmp_score;
		si->tmp_check_out[bf->input_x][bf->input_y] = tmp_check_out;
		score_info_free(&tmp_prev_info.si);
		return FALSE;
	} else if (ret & PLUGIN_DATA_MODIFIED) {
		count = si->leg[bf->view_leg].score[bf->input_x][bf->input_y] * -1;
	}
	// 終了フラグ設定
	si->leg[bf->view_leg].end_flag = TRUE;
	show_edit(hWnd, bf, si, TRUE, FALSE);
	SendMessage(hWnd, WM_SCORE_REDRAW, 0, 0);

	// クリア設定
	si->player[bf->input_x].legs++;
	if (si->player[bf->input_x].stat.high_off < left) {
		si->player[bf->input_x].stat.high_off = (TYPE_SCORE)left;
	}
	if (si->player[bf->input_x].stat.short_game == 0 ||
		si->player[bf->input_x].stat.short_game > bf->input_y * 3 + count) {
		si->player[bf->input_x].stat.short_game = bf->input_y * 3 + count;
	}
	if (si->player[bf->input_x].stat.long_game < bf->input_y * 3 + count) {
		si->player[bf->input_x].stat.long_game = bf->input_y * 3 + count;
	}
	si->leg[bf->view_leg].winner = bf->input_x;
	si->leg[bf->view_leg].darts = bf->input_y * 3 + count;
	si->leg[bf->view_leg].out_left = (TYPE_SCORE)left;
	si->leg[bf->view_leg].all_score[bf->input_x] = 0;
	si->leg[bf->view_leg].all_darts[bf->input_x] = si->leg[bf->view_leg].darts;
	// チェックアウト設定
	if (si->player[bf->input_x].check_out_mode == TRUE || si->player[bf->input_x].com == TRUE) {
		si->player[bf->input_x].stat.check_out++;
		// Finish stats
		if (left >= 2 && left <= 80) {
			si->player[bf->input_x].stat.success_2_80++;
		} else if (left >= 81 && left <= 130) {
			si->player[bf->input_x].stat.success_81_130++;
		} else if (left >= 131) {
			si->player[bf->input_x].stat.success_131++;
		}
	}
	// レッグ統計設定
	if (bf->input_x == si->leg[bf->view_leg].first) {
		si->player[bf->input_x].stat.all_keep_legs++;
		si->player[bf->input_x].stat.win_keep_legs++;
		si->player[!bf->input_x].stat.all_break_legs++;
	} else {
		si->player[bf->input_x].stat.all_break_legs++;
		si->player[bf->input_x].stat.win_break_legs++;
		si->player[!bf->input_x].stat.all_keep_legs++;
	}

	si->player[!bf->input_x].left = get_left(bf, si, -1, !bf->input_x);
	si->leg[bf->view_leg].all_score[!bf->input_x] = si->player[!bf->input_x].left;
	si->leg[bf->view_leg].all_darts[!bf->input_x] = (si->leg[bf->view_leg].current_round) * 3;

	set_player_info(bf, si, 0, -1);
	set_player_info(bf, si, 1, -1);
	SendMessage(GetParent(hWnd), WM_WINDOW_REDRAW, 0, FALSE);
	SendMessage(GetParent(hWnd), WM_WINDOW_REDRAW, 1, FALSE);

	// 確認メッセージ
	if (lock == TRUE) {
		mb_flag = MB_ICONINFORMATION;
		message_copy_res(IDS_STRING_COM_NAME, name);
	} else {
		mb_flag = MB_ICONINFORMATION | MB_OKCANCEL;
		lstrcpy(name, si->player[bf->input_x].name);
	}
	wsprintf(buf, message_get_res(IDS_STRING_FINISH), count, si->leg[bf->view_leg].darts);
#ifndef _DEBUG_NO_MSG
	if (MessageBox(hWnd, buf, name, mb_flag) == IDCANCEL) {
		*si = tmp_si;
		si->leg[bf->view_leg] = tmp_leg;
		si->leg[bf->view_leg].score[bf->input_x][bf->input_y] = tmp_score;
		si->tmp_check_out[bf->input_x][bf->input_y] = tmp_check_out;
		show_edit(hWnd, bf, si, TRUE, FALSE);
		SendMessage(hWnd, WM_SCORE_REDRAW, 0, 0);
		SendMessage(GetParent(hWnd), WM_WINDOW_SET_FIRST, FALSE, 0);
		set_player_info(bf, si, 0, -1);
		set_player_info(bf, si, 1, -1);
		SendMessage(GetParent(hWnd), WM_WINDOW_REDRAW, 0, FALSE);
		SendMessage(GetParent(hWnd), WM_WINDOW_REDRAW, 1, FALSE);
		score_info_free(&tmp_prev_info.si);
		return FALSE;
	}
#endif	//_DEBUG_NO_MSG
	score_info_free(&bf->prev_info.si);
	bf->prev_info = tmp_prev_info;
	SendMessage(GetParent(hWnd), WM_WINDOW_SET_PREV_MENU,
		bf->prev_info.prev_flag && (!bf->si->player[0].com) && (!bf->si->player[1].com), 0);

	// LEGの移動
	if (SendMessage(hWnd, WM_SCORE_NEXT_LEG, 0, 0) == FALSE) {
		return FALSE;
	}
	SendMessage(hWnd, WM_SCORE_INIT_LEG, FALSE, FALSE);
	return TRUE;
}

/*
 * set_middle - ミドルフォーディドル
 */
static BOOL set_middle(const HWND hWnd, DRAW_BUFFER *bf, SCORE_INFO *si)
{
	SCORE_INFO tmp_si;
	LEG_INFO tmp_leg;
	PREV_INFO tmp_prev_info;
	TCHAR buf[BUF_SIZE];
	TCHAR name[NAME_SIZE];
	int i;

	score_info_copy(&tmp_prev_info.si, si);
	tmp_prev_info.view_leg = bf->view_leg;
	tmp_prev_info.input_x = bf->input_x;
	tmp_prev_info.input_y = bf->input_y;
	tmp_prev_info.prev_flag = TRUE;

	tmp_si = *si;
	tmp_leg = si->leg[bf->view_leg];

	si->leg[bf->view_leg].end_flag = TRUE;
	show_edit(hWnd, bf, si, TRUE, FALSE);

	// 勝者選択
	if (show_middle(hInst, hWnd, si) == FALSE) {
		*si = tmp_si;
		si->leg[bf->view_leg] = tmp_leg;
		show_edit(hWnd, bf, si, TRUE, FALSE);
		score_info_free(&tmp_prev_info.si);
		return FALSE;
	}

	// プラグイン処理
	if (plugin_execute_all(GetParent(hWnd), CALLTYPE_FINISH, si, 0) & PLUGIN_CANCEL) {
		*si = tmp_si;
		si->leg[bf->view_leg] = tmp_leg;
		show_edit(hWnd, bf, si, TRUE, FALSE);
		score_info_free(&tmp_prev_info.si);
		return FALSE;
	}

	// プレイヤー情報設定
	if (si->round_limit == 1 && si->leg[bf->view_leg].current_round <= si->round) {
		i = si->round * 3 + 1;
	} else {
		i = si->leg[bf->view_leg].current_round * 3 + 1;
	}
	if ((si->round_limit == 1 && si->leg[bf->view_leg].max_round > si->round &&
		bf->si->leg[bf->view_leg].first == bf->si->leg[bf->view_leg].current_player) ||
		(si->round_limit == 0 && bf->si->leg[bf->view_leg].first == bf->si->leg[bf->view_leg].current_player)) {
		si->leg[bf->view_leg].max_round--;
		SendMessage(hWnd, WM_SCORE_REDRAW, 0, 0);
	}
	si->player[si->leg[bf->view_leg].winner].legs++;
	if (si->player[si->leg[bf->view_leg].winner].stat.short_game == 0 ||
		si->player[si->leg[bf->view_leg].winner].stat.short_game > i) {
		si->player[si->leg[bf->view_leg].winner].stat.short_game = i;
	}
	if (si->player[si->leg[bf->view_leg].winner].stat.long_game < i) {
		si->player[si->leg[bf->view_leg].winner].stat.long_game = i;
	}
	si->leg[bf->view_leg].darts = i;
	// レッグ統計設定
	if (si->leg[bf->view_leg].winner == si->leg[bf->view_leg].first) {
		si->player[si->leg[bf->view_leg].winner].stat.all_keep_legs++;
		si->player[si->leg[bf->view_leg].winner].stat.win_keep_legs++;
		si->player[!si->leg[bf->view_leg].winner].stat.all_break_legs++;
	} else {
		si->player[si->leg[bf->view_leg].winner].stat.all_break_legs++;
		si->player[si->leg[bf->view_leg].winner].stat.win_break_legs++;
		si->player[!si->leg[bf->view_leg].winner].stat.all_keep_legs++;
	}
	set_player_info(bf, si, si->leg[bf->view_leg].winner, -1);
	SendMessage(GetParent(hWnd), WM_WINDOW_REDRAW, si->leg[bf->view_leg].winner, FALSE);

	// 確認メッセージ
	wsprintf(buf, message_get_res(IDS_STRING_MIDDLE), i);
	if (si->player[si->leg[bf->view_leg].winner].com == TRUE) {
		message_copy_res(IDS_STRING_COM_NAME, name);
	} else {
		lstrcpy(name, si->player[si->leg[bf->view_leg].winner].name);
	}
	if (MessageBox(hWnd, buf, name, MB_ICONINFORMATION | MB_OKCANCEL) == IDCANCEL) {
		*si = tmp_si;
		si->leg[bf->view_leg] = tmp_leg;
		si->leg[bf->view_leg].end_flag = FALSE;
		show_edit(hWnd, bf, si, TRUE, FALSE);
		SendMessage(hWnd, WM_SCORE_REDRAW, 0, 0);
		set_player_info(bf, si, 0, -1);
		set_player_info(bf, si, 1, -1);
		SendMessage(GetParent(hWnd), WM_WINDOW_REDRAW, 0, FALSE);
		SendMessage(GetParent(hWnd), WM_WINDOW_REDRAW, 1, FALSE);
		score_info_free(&tmp_prev_info.si);
		return FALSE;
	}
	score_info_free(&bf->prev_info.si);
	bf->prev_info = tmp_prev_info;
	SendMessage(GetParent(hWnd), WM_WINDOW_SET_PREV_MENU,
		bf->prev_info.prev_flag && (!bf->si->player[0].com) && (!bf->si->player[1].com), 0);

	// LEGの移動
	if (SendMessage(hWnd, WM_SCORE_NEXT_LEG, 0, 0) == FALSE) {
		return TRUE;
	}
	SendMessage(hWnd, WM_SCORE_INIT_LEG, FALSE, FALSE);
	return TRUE;
}

/*
 * point_to_score - 座標からスコア位置を取得
 */
static BOOL point_to_score(const HWND hWnd, const DRAW_BUFFER *bf, const SCORE_INFO *si, const int x, const int y, int *ret_x, int *ret_y)
{
	RECT rect;
	int xx = 0, yy = 0;
	BOOL set_x = FALSE, set_y = FALSE;

	GetClientRect(hWnd, &rect);

	if (x >= 0 && x < bf->score_right[0]) {
		xx = 0;
		set_x = TRUE;
	} else if (x >= bf->input_left[1] && x < rect.right) {
		xx = 1;
		set_x = TRUE;
	}
	if (y >= bf->header_height) {
		yy = bf->pos_y + (y - bf->header_height) / bf->score_height - 1;
		if (yy >= si->leg[bf->view_leg].current_round) {
			yy = si->leg[bf->view_leg].current_round;
		}
		if (yy < si->leg[bf->view_leg].max_round) {
			set_y = TRUE;
		}
	}
	if (set_x == TRUE && set_y == TRUE) {
		*ret_x = xx;
		*ret_y = yy;
		return TRUE;
	}
	return FALSE;
}

/*
 * move_input - 入力域の移動
 */
static BOOL move_input(const HWND hWnd, DRAW_BUFFER *bf, const SCORE_INFO *si, int x, int y)
{
	if (x < 0 || x > 1) {
		return FALSE;
	}
	if (y < -1 || y >= si->leg[bf->view_leg].max_round || y > si->leg[bf->view_leg].current_round) {
		return FALSE;
	}
	if (bf->input_x == x && bf->input_y == y) {
		return FALSE;
	}
	bf->input_x = x;
	bf->input_y = y;
	return TRUE;
}

/*
 * show_edit - EDITの表示設定
 */
static BOOL show_edit(const HWND hWnd, DRAW_BUFFER *bf, SCORE_INFO *si, const BOOL score_set, const BOOL ensure_flag)
{
	RECT rect, edit_rect;
	POINT pt;
	TCHAR buf[BUF_SIZE];
	int i;

	GetClientRect(hWnd, &rect);

	GetWindowRect(bf->hedit, &edit_rect);
	pt.x = edit_rect.left; pt.y = edit_rect.top;
	ScreenToClient(hWnd, &pt);
	edit_rect.left = pt.x; edit_rect.top = pt.y;
	pt.x = edit_rect.right; pt.y = edit_rect.bottom;
	ScreenToClient(hWnd, &pt);
	edit_rect.right = pt.x; edit_rect.bottom = pt.y;

	if (ensure_flag == TRUE && si->leg[bf->view_leg].end_flag == FALSE) {
		// 入力位置にスクロール
		rect.top += bf->header_height;

		i = bf->pos_y;
		if (bf->input_y + 1 < bf->pos_y) {
			bf->pos_y = bf->input_y + 1;
		}
		if (rect.bottom > 0 && bf->input_y + 1 > bf->pos_y + (bf->page_y - 1)) {
			bf->pos_y = bf->input_y + 1 - (bf->page_y - 1);
			if (bf->pos_y > bf->max_y) {
				bf->pos_y = bf->max_y;
			}
		}
		if (i != bf->pos_y) {
			SetScrollPos(hWnd, SB_VERT, bf->pos_y, TRUE);
			ScrollWindowEx(hWnd, 0, (i - bf->pos_y) * bf->score_height, NULL, &rect, NULL, NULL, SW_INVALIDATE | SW_ERASE);
			edit_rect.top += (i - bf->pos_y) * bf->score_height;
			edit_rect.bottom += (i - bf->pos_y) * bf->score_height;
		}
	}
	if (bf->input_y < 0) {
		bf->input_y = 0;
	}

	if (score_set == TRUE) {
		// スコアの設定
		SendMessage(bf->hedit, WM_SET_REFRESH_MODE, FALSE, 0);
		if ((bf->input_y == si->leg[bf->view_leg].current_round && bf->input_x == si->leg[bf->view_leg].current_player) ||
			(bf->input_y == si->leg[bf->view_leg].current_round &&
			bf->input_x != si->leg[bf->view_leg].current_player && bf->input_x != si->leg[bf->view_leg].first)) {
			SendMessage(bf->hedit, WM_SETTEXT, 0, (LPARAM)TEXT(""));
		} else {
			_itot(si->leg[bf->view_leg].score[bf->input_x][bf->input_y], buf, 10);
			SendMessage(bf->hedit, WM_SETTEXT, 0, (LPARAM)buf);
			SendMessage(bf->hedit, EM_SETSEL, 0, -1);
		}
		SendMessage(bf->hedit, WM_SET_REFRESH_MODE, TRUE, 0);
	}

	if (bf->input_y + 1 - bf->pos_y < 0 ||
		bf->header_height + (bf->input_y + 1 - bf->pos_y) * bf->score_height > rect.bottom ||
		si->leg[bf->view_leg].end_flag == TRUE) {
		// EDIT非表示
		if (IsWindowVisible(bf->hedit) == TRUE) {
			ShowWindow(bf->hedit, SW_HIDE);
			InvalidateRect(hWnd, &edit_rect, FALSE);
			UpdateWindow(hWnd);
		} else {
			ShowWindow(bf->hedit, SW_HIDE);
		}
		if (si->leg[bf->view_leg].end_flag == TRUE) {
			SendMessage(bf->hedit, WM_SETTEXT, 0, (LPARAM)TEXT(""));
		}
		return FALSE;
	}
	SendMessage(bf->hedit, WM_SETLOCK, si->player[bf->input_x].lock, 0);

	// ウィンドウの移動
	MoveWindow(bf->hedit,
		bf->input_left[bf->input_x],
		bf->header_height + (bf->input_y - bf->pos_y + 1) * bf->score_height,
		bf->score_left[bf->input_x] - bf->input_left[bf->input_x],
		bf->score_height - 1, FALSE);
	InvalidateRect(hWnd, &edit_rect, FALSE);
	UpdateWindow(hWnd);

	// ウィンドウの表示
	ShowWindow(bf->hedit, SW_SHOW);
	SetFocus(bf->hedit);
	SendMessage(bf->hedit, WM_REFRESH, 0, 0);

	if (score_set == TRUE) {
		// プラグイン処理
		plugin_execute_all(GetParent(hWnd), CALLTYPE_INPUT_START, si, 0);
	}
	return TRUE;
}

/*
 * score_list_proc - スコア一覧ウィンドウプロシージャ
 */
static LRESULT CALLBACK score_list_proc(const HWND hWnd, const UINT msg, WPARAM wParam, LPARAM lParam)
{
	DRAW_BUFFER *bf;
	HDC hdc;
	PAINTSTRUCT ps;
	RECT rect;
	TCHAR buf[BUF_SIZE];
	int left_score[2];
	int i, j;
	int x, y;

	switch (msg) {
	case WM_CREATE:
		// 初期化
		bf = (DRAW_BUFFER *)mem_calloc(sizeof(DRAW_BUFFER));
		if (bf == NULL) {
			message_get_error(GetLastError(), buf);
			MessageBox(hWnd, buf, APP_NAME, MB_ICONERROR);
			return -1;
		}
		bf->si = (SCORE_INFO *)((CREATESTRUCT *)lParam)->lpCreateParams;
		if (bf->si == NULL) {
			bf->si = &si;
		}
		SetWindowLong(hWnd, GWL_USERDATA, (LONG)bf);

		// 描画用情報の初期化
		hdc = GetDC(hWnd);
		bf->draw_dc = CreateCompatibleDC(hdc);
		bf->back_dc = CreateCompatibleDC(hdc);
		bf->ellipse_dc = CreateCompatibleDC(hdc);
		ReleaseDC(hWnd, hdc);
		if (GetDeviceCaps(bf->draw_dc, BITSPIXEL) >= 16) {
			SetStretchBltMode(bf->draw_dc, HALFTONE);
			SetBrushOrgEx(bf->draw_dc, 0, 0, NULL);
			bf->pen_size = 4;
		} else {
			bf->pen_size = 2;
		}
		draw_init(hWnd, bf);
		SetBkMode(bf->draw_dc, TRANSPARENT);
		bf->back_brush = CreateSolidBrush(op.ci.background);
		bf->odd_back_brush = CreateSolidBrush(op.ci.odd_background);
		bf->header_back_brush = CreateSolidBrush(op.ci.header_background);
		bf->line_pen = CreatePen(PS_SOLID, 1, op.ci.line);
		bf->separate_pen = CreatePen(PS_SOLID, 1, op.ci.separate);
		bf->separate_bold_pen = CreatePen(PS_SOLID, 2, op.ci.separate);
		bf->finish_pen = CreatePen(PS_SOLID, bf->pen_size, op.ci.scored_text);
		bf->ton_circle_pen = CreatePen(PS_SOLID, bf->pen_size, op.ci.ton_circle);

		// IMEをOFFに設定
		ImmAssociateContext(hWnd, (HIMC)NULL);
		// スクロールバーの設定
		set_scrollbar(hWnd, bf, bf->si);

		// EDITの作成
		bf->hedit = nedit_create(hInst, hWnd, 0);
		SendMessage(bf->hedit, EM_LIMITTEXT, INPUT_LIMIT, 0);
		SendMessage(bf->hedit, WM_SETFONT, (WPARAM)bf->score_font, MAKELPARAM(TRUE, 0));
		break;

	case WM_CLOSE:
		// ウィンドウを閉じる
		DestroyWindow(hWnd);
		break;

	case WM_DESTROY:
		// ウィンドウの破棄
		bf = (DRAW_BUFFER *)GetWindowLong(hWnd, GWL_USERDATA);
		if (bf != NULL) {
			draw_free(hWnd, bf);
			if (bf->draw_dc != NULL) {
				DeleteDC(bf->draw_dc);
				bf->draw_dc = NULL;
			}
			if (bf->back_dc != NULL) {
				DeleteDC(bf->back_dc);
				bf->back_dc = NULL;
			}
			if (bf->ellipse_dc != NULL) {
				DeleteDC(bf->ellipse_dc);
				bf->ellipse_dc = NULL;
			}
			DeleteObject(bf->back_brush);
			DeleteObject(bf->odd_back_brush);
			DeleteObject(bf->header_back_brush);
			DeleteObject(bf->line_pen);
			DeleteObject(bf->separate_pen);
			DeleteObject(bf->separate_bold_pen);
			DeleteObject(bf->finish_pen);
			DeleteObject(bf->ton_circle_pen);

			score_info_free(&bf->next_info.si);
			score_info_free(&bf->prev_info.si);
			mem_free(&bf);
		}
		return DefWindowProc(hWnd, msg, wParam, lParam);

	case WM_SETFOCUS:
		// フォーカス設定
		bf = (DRAW_BUFFER *)GetWindowLong(hWnd, GWL_USERDATA);
		if (bf == NULL || bf->lock == TRUE) {
			break;
		}
		show_edit(hWnd, bf, bf->si, FALSE, FALSE);
		break;

	case WM_SIZE:
		// ウィンドウサイズ変更
		bf = (DRAW_BUFFER *)GetWindowLong(hWnd, GWL_USERDATA);
		if (bf == NULL) {
			break;
		}
		draw_free(hWnd, bf);
		draw_init(hWnd, bf);
		if (op.view_scroll_bar == 0) {
			ShowScrollBar(hWnd, SB_VERT, FALSE);
		}
		set_scrollbar(hWnd, bf, bf->si);
		SendMessage(hWnd, WM_SCORE_REDRAW, 0, 0);
		SendMessage(bf->hedit, WM_SETFONT, (WPARAM)bf->score_font, MAKELPARAM(TRUE, 0));
		show_edit(hWnd, bf, bf->si, FALSE, TRUE);
		break;

	case WM_PAINT:
		// 描画
		bf = (DRAW_BUFFER *)GetWindowLong(hWnd, GWL_USERDATA);
		if (bf == NULL) {
			break;
		}
		hdc = BeginPaint(hWnd, &ps);
		if (bf->back_redraw == TRUE || bf->back_first != bf->si->leg[bf->view_leg].first) {
			bf->back_redraw = FALSE;
			bf->back_first = bf->si->leg[bf->view_leg].first;
			// 背景の描画
			draw_background(bf, bf->back_first);
		}
		if (ps.rcPaint.top < bf->header_height) {
			// ヘッダーのコピー
			BitBlt(hdc, ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right, bf->header_height,
				bf->back_dc, ps.rcPaint.left, ps.rcPaint.top, SRCCOPY);
			ps.rcPaint.top = bf->header_height;
		}
		// スコアシートの描画
		y = ((ps.rcPaint.top - bf->header_height) / bf->score_height) * bf->score_height + bf->header_height;
		j = bf->pos_y + (y - bf->header_height) / bf->score_height;
		left_score[0] = bf->si->player[0].start_score;
		left_score[1] = bf->si->player[1].start_score;
		for (i = 0; i < j - 1 && i < bf->si->leg[bf->view_leg].current_round; i++) {
			left_score[0] -= bf->si->leg[bf->view_leg].score[0][i];
			left_score[1] -= bf->si->leg[bf->view_leg].score[1][i];
		}
		for (i = j; y < ps.rcPaint.bottom; y += bf->score_height, i++) {
			if (bf->si->round_limit != 0 && i > bf->si->leg[bf->view_leg].max_round) {
				SetRect(&rect, ps.rcPaint.left, y, ps.rcPaint.right, ps.rcPaint.bottom);
				FillRect(hdc, &rect, bf->header_back_brush);
				break;
			}
			draw_line(bf, bf->si, &ps.rcPaint, i, left_score);
			BitBlt(hdc, ps.rcPaint.left, y, ps.rcPaint.right, bf->score_height,
				bf->draw_dc, ps.rcPaint.left, 0, SRCCOPY);
		}
		EndPaint(hWnd, &ps);
		break;

	case WM_ERASEBKGND:
		// 背景を消さない
		return TRUE;

	case WM_VSCROLL:
		// 縦スクロール
		bf = (DRAW_BUFFER *)GetWindowLong(hWnd, GWL_USERDATA);
		if (bf == NULL || bf->lock == TRUE) {
			break;
		}
		GetClientRect(hWnd, &rect);
		rect.top += bf->header_height;
		i = bf->pos_y;
		switch ((int)LOWORD(wParam)) {
		case SB_TOP:
			bf->pos_y = 0;
			break;

		case SB_BOTTOM:
			bf->pos_y = bf->max_y;
			break;

		case SB_LINEDOWN:
			bf->pos_y = (bf->pos_y < bf->max_y) ? bf->pos_y + 1 : bf->max_y;
			break;

		case SB_LINEUP:
			bf->pos_y = (bf->pos_y > 0) ? bf->pos_y - 1 : 0;
			break;

		case SB_PAGEDOWN:
			bf->pos_y = (bf->pos_y + (bf->page_y - 1) < bf->max_y) ?
				bf->pos_y + (bf->page_y - 1) : bf->max_y;
			break;

		case SB_PAGEUP:
			bf->pos_y = (bf->pos_y - (bf->page_y - 1) > 0) ?
				bf->pos_y - (bf->page_y - 1) : 0;
			break;

		case SB_THUMBPOSITION:
		case SB_THUMBTRACK:
			{
				SCROLLINFO sci;

				ZeroMemory(&sci, sizeof(SCROLLINFO));
				sci.cbSize = sizeof(SCROLLINFO);
				sci.fMask = SIF_ALL;
				GetScrollInfo(hWnd, SB_VERT, &sci);
				bf->pos_y = sci.nTrackPos;
			}
			break;
		}
		SetScrollPos(hWnd, SB_VERT, bf->pos_y, TRUE);
		ScrollWindowEx(hWnd, 0, (i - bf->pos_y) * bf->score_height, NULL, &rect, NULL, NULL, SW_INVALIDATE | SW_ERASE);

		show_edit(hWnd, bf, bf->si, FALSE, FALSE);
		break;

	case WM_MOUSEWHEEL:
		// マウスホイール
		for (i = 0; i < WHEEL_COUNT; i++) {
			SendMessage(hWnd, WM_VSCROLL, ((short)HIWORD(wParam) > 0) ? SB_LINEUP : SB_LINEDOWN, 0);
		}
		break;

	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
		// マウスボタン
		bf = (DRAW_BUFFER *)GetWindowLong(hWnd, GWL_USERDATA);
		if (bf == NULL || bf->lock == TRUE) {
			break;
		}
		if (bf->si->leg[bf->view_leg].end_flag == TRUE) {
			SetFocus(hWnd);
			break;
		}
		SendMessage(bf->hedit, WM_GETTEXT, BUF_SIZE - 1, (LPARAM)buf);
		if (*buf == TEXT('\0') && bf->input_y != bf->si->leg[bf->view_leg].current_round) {
			show_edit(hWnd, bf, bf->si, FALSE, TRUE);
			break;
		}
		if (*buf != TEXT('\0') && SendMessage(bf->hedit, EM_GETMODIFY, 0, 0) == TRUE) {
			if (set_score(hWnd, bf, bf->si, (TYPE_SCORE)_ttoi(buf)) == FALSE) {
				show_edit(hWnd, bf, bf->si, FALSE, TRUE);
				break;
			}
		}
		if (point_to_score(hWnd, bf, bf->si, (short)LOWORD(lParam), (short)HIWORD(lParam), &x, &y) == TRUE) {
			if (move_input(hWnd, bf, bf->si, x, y) == TRUE) {
				show_edit(hWnd, bf, bf->si, TRUE, TRUE);
			}
		}
		break;

	case WM_RBUTTONUP:
	case WM_CONTEXTMENU:
		// コンテキストメニュー
		SendMessage(GetParent(hWnd), WM_WINDOW_SHOW_MENU, 0, 0);
		break;

	case WM_SYSKEYDOWN:
		// ALTキー
		if (wParam == VK_MENU) {
			SendMessage(hWnd, WM_WINDOW_SET_KEY, 0, 0);
		}
		return DefWindowProc(hWnd, msg, wParam, lParam);

	case WM_KEYDOWN:
		// キーボード処理
		bf = (DRAW_BUFFER *)GetWindowLong(hWnd, GWL_USERDATA);
		if (bf == NULL || bf->lock == TRUE) {
			break;
		}
		if (bf->si->leg[bf->view_leg].end_flag == FALSE) {
			show_edit(hWnd, bf, bf->si, FALSE, TRUE);
			SendMessage(bf->hedit, WM_KEYDOWN, wParam, lParam);
			break;
		}
		if (wParam == VK_SHIFT || wParam == VK_CONTROL || wParam == VK_MENU) {
			SendMessage(hWnd, WM_WINDOW_SET_KEY, 0, 0);
		}
		switch (wParam) {
		case VK_HOME:
			SendMessage(hWnd, WM_VSCROLL, SB_TOP, 0);
			break;

		case VK_END:
			SendMessage(hWnd, WM_VSCROLL, SB_BOTTOM, 0);
			break;

		case VK_PRIOR:
			SendMessage(hWnd, WM_VSCROLL, SB_PAGEUP, 0);
			break;

		case VK_NEXT:
			SendMessage(hWnd, WM_VSCROLL, SB_PAGEDOWN, 0);
			break;

		case VK_UP:
			SendMessage(hWnd, WM_VSCROLL, SB_LINEUP, 0);
			break;

		case VK_DOWN:
			SendMessage(hWnd, WM_VSCROLL, SB_LINEDOWN, 0);
			break;
		}
		break;

	case WM_CHAR:
		// 入力処理
		bf = (DRAW_BUFFER *)GetWindowLong(hWnd, GWL_USERDATA);
		if (bf == NULL || bf->lock == TRUE) {
			break;
		}
		if (bf->si->leg[bf->view_leg].end_flag == FALSE) {
			show_edit(hWnd, bf, bf->si, FALSE, TRUE);
			SendMessage(bf->hedit, WM_CHAR, wParam, lParam);
		}
		break;

	case WM_GETDLGCODE:
		// カーソルキーイベントを取得
		return DLGC_WANTARROWS;

	case WM_TIMER:
		// タイマー
		switch (wParam) {
		case ID_TIMER_COM:
			KillTimer(hWnd, ID_TIMER_COM);
			EnableWindow(GetParent(hWnd), TRUE);
			bf = (DRAW_BUFFER *)GetWindowLong(hWnd, GWL_USERDATA);
			if (bf == NULL) {
				break;
			}
			SendMessage(hWnd, WM_COMMAND, ID_ACCEL_ESC, 0);

			// COMのスコアを取得
			show_score_com(hInst, hWnd, bf->si->leg[bf->view_leg].current_player);
			if (bf->si->player[bf->si->leg[bf->view_leg].current_player].com_score == -5) {
				bf->si->leg[bf->view_leg].end_flag = TRUE;
				show_edit(hWnd, bf, bf->si, FALSE, FALSE);
				break;
			}
			if (bf->si->player[bf->si->leg[bf->view_leg].current_player].com_score < 0) {
				// クリア
				set_finish(hWnd, bf, bf->si, bf->si->player[bf->si->leg[bf->view_leg].current_player].com_score * -1, TRUE);
			} else {
				// スコア設定
				_itot(bf->si->player[bf->si->leg[bf->view_leg].current_player].com_score, buf, 10);
				SendMessage(bf->hedit, WM_SETTEXT, 0, (LPARAM)buf);
				SendMessage(bf->hedit, EM_SETMODIFY, TRUE, 0);
				SendMessage(hWnd, WM_COMMAND, ID_ACCEL_TAB, 0);
				bf->si->player[bf->si->leg[bf->view_leg].current_player].com_score = 0;
			}
			break;
		}
		break;

	case WM_COMMAND:
		// コマンド処理
		bf = (DRAW_BUFFER *)GetWindowLong(hWnd, GWL_USERDATA);
		if (bf == NULL || bf->lock == TRUE) {
			break;
		}
		switch (LOWORD(wParam)) {
		case ID_ACCEL_TAB:
		case ID_ACCEL_STAB:
			// 進む、戻る
			if (bf->si->leg[bf->view_leg].end_flag == TRUE) {
				break;
			}
			SendMessage(bf->hedit, WM_GETTEXT, BUF_SIZE - 1, (LPARAM)buf);
			if (*buf == TEXT('\0') &&
				(bf->input_y != bf->si->leg[bf->view_leg].current_round || LOWORD(wParam) == ID_ACCEL_TAB)) {
				show_edit(hWnd, bf, bf->si, FALSE, TRUE);
				break;
			}
			if (*buf != TEXT('\0') && SendMessage(bf->hedit, EM_GETMODIFY, 0, 0) == TRUE) {
				if (set_score(hWnd, bf, bf->si, (TYPE_SCORE)_ttoi(buf)) == FALSE) {
					show_edit(hWnd, bf, bf->si, FALSE, TRUE);
					break;
				}
			}
			if (LOWORD(wParam) == ID_ACCEL_TAB) {
				i = (bf->si->leg[bf->view_leg].first != bf->input_x) ? bf->input_y + 1 : bf->input_y;
			} else {
				i = (bf->si->leg[bf->view_leg].first == bf->input_x) ? bf->input_y - 1 : bf->input_y;
				if (i <= -1 && bf->si->leg[bf->view_leg].first == bf->input_x) {
					break;
				}
			}
			if (move_input(hWnd, bf, bf->si, !bf->input_x, i) == TRUE) {
				show_edit(hWnd, bf, bf->si, TRUE, TRUE);
			}
			break;

		case ID_ACCEL_ESC:
			// キャンセル
			if (bf->si->leg[bf->view_leg].end_flag == TRUE) {
				break;
			}
			if (bf->si->leg[bf->view_leg].current_round < bf->si->leg[bf->view_leg].max_round) {
				bf->input_x = bf->si->leg[bf->view_leg].current_player;
				bf->input_y = bf->si->leg[bf->view_leg].current_round;
			}
			show_edit(hWnd, bf, bf->si, TRUE, TRUE);
			break;

		case ID_ACCEL_NONE:
			// 何もしない
			break;

		case ID_ACCEL_UP:
		case ID_ACCEL_DOWN:
			// カーソルキー上下
			if (bf->si->leg[bf->view_leg].end_flag == TRUE) {
				SendMessage(hWnd, WM_VSCROLL, (LOWORD(wParam) == ID_ACCEL_UP) ? SB_LINEUP : SB_LINEDOWN, 0);
				break;
			}
			show_edit(hWnd, bf, bf->si, FALSE, TRUE);
			SendMessage(bf->hedit, WM_GETTEXT, BUF_SIZE - 1, (LPARAM)buf);
			if (*buf == TEXT('\0') && bf->input_y != bf->si->leg[bf->view_leg].current_round) {
				break;
			}
			if (*buf != TEXT('\0') && SendMessage(bf->hedit, EM_GETMODIFY, 0, 0) == TRUE) {
				if (set_score(hWnd, bf, bf->si, (TYPE_SCORE)_ttoi(buf)) == FALSE) {
					break;
				}
			}
			if (LOWORD(wParam) == ID_ACCEL_UP) {
				y = bf->input_y - 1;
			} else {
				y = bf->input_y + 1;
			}
			if (move_input(hWnd, bf, bf->si, bf->input_x, y) == TRUE) {
				show_edit(hWnd, bf, bf->si, TRUE, TRUE);
			}
			break;

		case WM_SCORE_MOVE_LEFT:
		case WM_SCORE_MOVE_RIGHT:
			// カーソルキー左右
			if (bf->si->leg[bf->view_leg].end_flag == TRUE) {
				break;
			}
			if ((LOWORD(wParam) == WM_SCORE_MOVE_LEFT && bf->input_x == 0) ||
				(LOWORD(wParam) == WM_SCORE_MOVE_RIGHT && bf->input_x == 1)) {
				break;
			}
			SendMessage(bf->hedit, WM_GETTEXT, BUF_SIZE - 1, (LPARAM)buf);
			if (*buf == TEXT('\0') && bf->input_y != bf->si->leg[bf->view_leg].current_round) {
				break;
			}
			if (*buf != TEXT('\0') && SendMessage(bf->hedit, EM_GETMODIFY, 0, 0) == TRUE) {
				if (set_score(hWnd, bf, bf->si, (TYPE_SCORE)_ttoi(buf)) == FALSE) {
					break;
				}
			}
			if (move_input(hWnd, bf, bf->si, !bf->input_x, bf->input_y) == TRUE) {
				show_edit(hWnd, bf, bf->si, TRUE, TRUE);
			}
			break;

		case ID_ACCEL_PRIOR:
		case ID_ACCEL_NEXT:
			// ページアップ、ページダウン
			GetClientRect(hWnd, &rect);
			if (bf->si->leg[bf->view_leg].end_flag == TRUE ||
				(bf->si->leg[bf->view_leg].current_round + 1 < bf->pos_y &&
				bf->si->leg[bf->view_leg].current_round == bf->input_y)) {
				SendMessage(hWnd, WM_VSCROLL, (LOWORD(wParam) == ID_ACCEL_PRIOR) ? SB_PAGEUP : SB_PAGEDOWN, 0);
				break;
			}
			show_edit(hWnd, bf, bf->si, FALSE, TRUE);
			SendMessage(bf->hedit, WM_GETTEXT, BUF_SIZE - 1, (LPARAM)buf);
			if (*buf == TEXT('\0') && bf->input_y != bf->si->leg[bf->view_leg].current_round) {
				break;
			}
			if (*buf != TEXT('\0') && SendMessage(bf->hedit, EM_GETMODIFY, 0, 0) == TRUE) {
				if (set_score(hWnd, bf, bf->si, (TYPE_SCORE)_ttoi(buf)) == FALSE) {
					break;
				}
			}
			SendMessage(hWnd, WM_VSCROLL, (LOWORD(wParam) == ID_ACCEL_PRIOR) ? SB_PAGEUP : SB_PAGEDOWN, 0);
			if (LOWORD(wParam) == ID_ACCEL_PRIOR) {
				y = bf->input_y - (bf->page_y - 1);
				if (y < -1) {
					y = -1;
				}
			} else {
				y = bf->input_y + (bf->page_y - 1);
				if (y > bf->si->leg[bf->view_leg].current_round) {
					y = bf->si->leg[bf->view_leg].current_round;
				}
			}
			if (move_input(hWnd, bf, bf->si, bf->input_x, y) == TRUE) {
				show_edit(hWnd, bf, bf->si, TRUE, FALSE);
			}
			break;

		case ID_ACCEL_HOME:
		case ID_ACCEL_END:
			// Homeキー、Endキー
			if (bf->si->leg[bf->view_leg].end_flag == TRUE) {
				SendMessage(hWnd, WM_VSCROLL, (LOWORD(wParam) == ID_ACCEL_HOME) ? SB_TOP : SB_BOTTOM, 0);
				break;
			}
			SendMessage(bf->hedit, WM_GETTEXT, BUF_SIZE - 1, (LPARAM)buf);
			if (*buf == TEXT('\0') && bf->input_y != bf->si->leg[bf->view_leg].current_round) {
				show_edit(hWnd, bf, bf->si, FALSE, TRUE);
				break;
			}
			if (*buf != TEXT('\0') && SendMessage(bf->hedit, EM_GETMODIFY, 0, 0) == TRUE) {
				if (set_score(hWnd, bf, bf->si, (TYPE_SCORE)_ttoi(buf)) == FALSE) {
					show_edit(hWnd, bf, bf->si, FALSE, TRUE);
					break;
				}
			}
			if (LOWORD(wParam) == ID_ACCEL_HOME) {
				x = bf->si->leg[bf->view_leg].first;
				y = -1;
			} else {
				x = bf->si->leg[bf->view_leg].current_player;
				y = bf->si->leg[bf->view_leg].current_round;
			}
			if (move_input(hWnd, bf, bf->si, x, y) == TRUE) {
				show_edit(hWnd, bf, bf->si, TRUE, TRUE);
			}
			break;

		case ID_MENUITEM_FINISH_ONE:
		case ID_MENUITEM_FINISH_TWO:
		case ID_MENUITEM_FINISH_THREE:
			// フィニッシュ
			i = LOWORD(wParam) - ID_MENUITEM_FINISH_ONE + 1;
			if (bf->si->leg[bf->view_leg].end_flag == TRUE) {
				break;
			}
			if (bf->si->player[bf->input_x].lock == TRUE) {
				MessageBeep(0xFFFFFFFF);
				break;
			}
			show_edit(hWnd, bf, bf->si, FALSE, TRUE);
			if (get_left(bf, bf->si, bf->input_y, bf->input_x) <= i * 60) {
				set_finish(hWnd, bf, bf->si, i, FALSE);
			} else {
				MessageBeep(0xFFFFFFFF);
			}
			break;

		case ID_MENUITEM_MIDDLE:
			// ミドルフォーディドル
			if (bf->si->leg[bf->view_leg].end_flag == TRUE) {
				break;
			}
			show_edit(hWnd, bf, bf->si, FALSE, TRUE);
			SendMessage(bf->hedit, WM_GETTEXT, BUF_SIZE - 1, (LPARAM)buf);
			if (*buf != TEXT('\0') && SendMessage(bf->hedit, EM_GETMODIFY, 0, 0) == TRUE) {
				if (set_score(hWnd, bf, bf->si, (TYPE_SCORE)_ttoi(buf)) == FALSE) {
					break;
				}
			}
			set_middle(hWnd, bf, bf->si);
			break;

		case ID_MENUITEM_SCORE_LEFT:
			// 残りスコアでの入力
			if (bf->si->leg[bf->view_leg].end_flag == TRUE) {
				break;
			}
			if (bf->si->player[bf->input_x].lock == TRUE) {
				MessageBeep(0xFFFFFFFF);
				break;
			}
			show_edit(hWnd, bf, bf->si, FALSE, TRUE);
			SendMessage(bf->hedit, WM_GETTEXT, BUF_SIZE - 1, (LPARAM)buf);
			if (*buf == TEXT('\0')) {
				MessageBeep(0xFFFFFFFF);
				break;
			}
			i = get_left(bf, bf->si, bf->input_y, bf->input_x) - _ttoi(buf);
			if (i < 0 || i > 180) {
				MessageBeep(0xFFFFFFFF);
				break;
			}
			_itot(i, buf, 10);
			SendMessage(bf->hedit, WM_SETTEXT, 0, (LPARAM)buf);
			SendMessage(bf->hedit, EM_SETMODIFY, TRUE, 0);
			SendMessage(hWnd, WM_COMMAND, ID_ACCEL_TAB, 0);
			break;

		case ID_MENUITEM_SCORE_SWAP:
			// スコアの入れ替え
			if (bf->si->player[0].lock == TRUE || bf->si->player[1].lock == TRUE) {
				MessageBeep(0xFFFFFFFF);
				break;
			}
			if (bf->si->leg[bf->view_leg].end_flag == TRUE) {
				break;
			}
			if (MessageBox(hWnd, message_get_res(IDS_STRING_SCORE_SWAP), APP_NAME, MB_ICONQUESTION | MB_YESNO) != IDNO) {
				bf->si->leg[bf->view_leg].first = !bf->si->leg[bf->view_leg].first;
				bf->si->leg[bf->view_leg].current_player = !bf->si->leg[bf->view_leg].current_player;
				bf->input_x = !bf->input_x;

				SWAP((int)bf->si->leg[bf->view_leg].score[0], (int)bf->si->leg[bf->view_leg].score[1]);
				SWAP((int)bf->si->tmp_check_out[0], (int)bf->si->tmp_check_out[1]);
				SWAP(bf->si->player[0].left, bf->si->player[1].left);
				SWAP(bf->si->leg[bf->view_leg].all_score[0], bf->si->leg[bf->view_leg].all_score[1]);
				SWAP(bf->si->leg[bf->view_leg].all_darts[0], bf->si->leg[bf->view_leg].all_darts[1]);

				set_player_info(bf, bf->si, 0, -1);
				set_player_info(bf, bf->si, 1, -1);
				SendMessage(GetParent(hWnd), WM_WINDOW_SET_FIRST, FALSE, 0);
				SendMessage(GetParent(hWnd), WM_WINDOW_REDRAW, 0, FALSE);
				SendMessage(GetParent(hWnd), WM_WINDOW_REDRAW, 1, FALSE);
				SendMessage(GetParent(hWnd), WM_WINDOW_SET_CURRENT, bf->si->leg[bf->view_leg].current_player, 0);
				SendMessage(hWnd, WM_SCORE_REDRAW, 0, 0);
				show_edit(hWnd, bf, bf->si, TRUE, TRUE);
			}
			break;

		case ID_MENUITEM_SCORE_CLEAR:
			// スコアのクリア
			if (bf->si->player[0].lock == TRUE || bf->si->player[1].lock == TRUE) {
				MessageBeep(0xFFFFFFFF);
				break;
			}
			if (MessageBox(hWnd, message_get_res(IDS_STRING_SCORE_CLEAR), APP_NAME, MB_ICONQUESTION | MB_YESNO) == IDNO) {
				break;
			}
			SendMessage(hWnd, WM_SCORE_INIT_LEG, TRUE, FALSE);
			ZeroMemory(bf->si->leg[bf->view_leg].score[0], sizeof(TYPE_SCORE) * (bf->si->leg[bf->view_leg].alloc_round + 1));
			ZeroMemory(bf->si->leg[bf->view_leg].score[1], sizeof(TYPE_SCORE) * (bf->si->leg[bf->view_leg].alloc_round + 1));
			ZeroMemory(bf->si->tmp_check_out[0], sizeof(TYPE_CHECK_OUT) * (bf->si->leg[bf->view_leg].alloc_round + 1));
			ZeroMemory(bf->si->tmp_check_out[1], sizeof(TYPE_CHECK_OUT) * (bf->si->leg[bf->view_leg].alloc_round + 1));
			set_player_info(bf, bf->si, 0, -1);
			set_player_info(bf, bf->si, 1, -1);
			SendMessage(GetParent(hWnd), WM_WINDOW_REDRAW, 0, FALSE);
			SendMessage(GetParent(hWnd), WM_WINDOW_REDRAW, 1, FALSE);
			break;

		case ID_MENUITEM_ARRANGE:
			// アレンジの表示
			show_arrange(hInst, hWnd, get_left(bf, bf->si, bf->input_y, bf->input_x));
			break;
		}
		break;

	case WM_WINDOW_SET_KEY:
		// キー状態変更を通知
		SendMessage(GetParent(hWnd), WM_WINDOW_SET_KEY, 0, 0);
		break;

	case WM_SCORE_SET_INFO:
		// 表示するスコア情報設定
		bf = (DRAW_BUFFER *)GetWindowLong(hWnd, GWL_USERDATA);
		if (bf == NULL) {
			break;
		}
		bf->si = (SCORE_INFO *)lParam;
		break;

	case WM_SCORE_INIT_LEG:
		// レッグの初期化
		bf = (DRAW_BUFFER *)GetWindowLong(hWnd, GWL_USERDATA);
		if (bf == NULL) {
			break;
		}
		bf->view_leg = bf->si->current_leg;
		if (lParam == TRUE) {
			// 戻る情報初期化
			score_info_free(&bf->prev_info.si);
			score_info_free(&bf->next_info.si);
			bf->prev_info.prev_flag = FALSE;
			bf->next_info.prev_flag = FALSE;
			SendMessage(GetParent(hWnd), WM_WINDOW_SET_PREV_MENU, FALSE, 0);
		}
		if (bf->next_info.prev_flag == FALSE || wParam == TRUE) {
			// レッグの初期化
			LEG_INFO leg;

			leg = bf->si->leg[bf->view_leg];
			ZeroMemory(&bf->si->leg[bf->view_leg], sizeof(LEG_INFO));
			bf->si->leg[bf->view_leg].current_player = leg.first;
			bf->si->leg[bf->view_leg].current_round = 0;
			bf->si->leg[bf->view_leg].alloc_round = (bf->si->round_limit == 0) ? ALLOC_ROUND : bf->si->round;
			bf->si->leg[bf->view_leg].max_round = (bf->si->round_limit == 0) ? 1 : bf->si->round;
			bf->si->leg[bf->view_leg].first = leg.first;
			for (i = 0; i < 2; i++) {
				bf->si->leg[bf->view_leg].score[i] = leg.score[i];
				bf->si->leg[bf->view_leg].score[i][0] = 0;
				bf->si->leg[bf->view_leg].all_score[i] = bf->si->player[i].start_score;
				bf->si->player[i].left = bf->si->player[i].start_score;
			}
			bf->input_x = leg.first;
			bf->input_y = -1;
			if (bf->si->current_leg == 0) {
				recovery_save(bf->si, 0, 0, 0);
			} else {
				recovery_save(bf->si, 2, 0, 0);
			}
		} else {
			// 次のレッグ情報設定
			mem_free(&bf->si->leg[bf->view_leg].score[0]);
			mem_free(&bf->si->leg[bf->view_leg].score[1]);
			mem_free(&bf->si->tmp_check_out[0]);
			mem_free(&bf->si->tmp_check_out[1]);

			bf->si->leg[bf->view_leg] = bf->next_info.si.leg[bf->next_info.view_leg];
			bf->next_info.si.leg[bf->next_info.view_leg].score[0] = NULL;
			bf->next_info.si.leg[bf->next_info.view_leg].score[1] = NULL;
			bf->si->tmp_check_out[0] = bf->next_info.si.tmp_check_out[0];
			bf->si->tmp_check_out[1] = bf->next_info.si.tmp_check_out[1];
			bf->next_info.si.tmp_check_out[0] = NULL;
			bf->next_info.si.tmp_check_out[1] = NULL;

			bf->si->player[0].left = bf->next_info.si.player[0].left;
			bf->si->player[1].left = bf->next_info.si.player[1].left;
			bf->input_x = bf->next_info.input_x;
			bf->input_y = bf->next_info.input_y;

			score_info_free(&bf->next_info.si);
			bf->next_info.prev_flag = FALSE;

			recovery_save(bf->si, 0, 0, 0);

			set_player_info(bf, bf->si, 0, -1);
			set_player_info(bf, bf->si, 1, -1);
			bf->pos_y = 0;
		}
		set_scrollbar(hWnd, bf, bf->si);
		SendMessage(hWnd, WM_SCORE_REDRAW, 0, 0);

		SendMessage(GetParent(hWnd), WM_WINDOW_SET_FIRST, FALSE, 0);
		SendMessage(GetParent(hWnd), WM_WINDOW_REDRAW, 0, TRUE);
		SendMessage(GetParent(hWnd), WM_WINDOW_REDRAW, 1, FALSE);
		SendMessage(GetParent(hWnd), WM_WINDOW_SET_CURRENT, bf->si->leg[bf->view_leg].current_player, 0);

		if (bf->si->current_leg == 0) {
			plugin_execute_all(GetParent(hWnd), CALLTYPE_SET_START, bf->si, bf->si->current_set);
		}
		plugin_execute_all(GetParent(hWnd), CALLTYPE_LEG_START, bf->si, bf->si->current_leg);
		show_edit(hWnd, bf, bf->si, TRUE, TRUE);

		if (bf->si->player[bf->si->leg[bf->view_leg].current_player].com == TRUE) {
			SetTimer(hWnd, ID_TIMER_COM, TIMER_INTERVAL_COM, NULL);
		}
		break;

	case WM_SCORE_NEXT_LEG:
		// レッグの移動
		bf = (DRAW_BUFFER *)GetWindowLong(hWnd, GWL_USERDATA);
		if (bf == NULL) {
			break;
		}
		bf->si->current_leg++;
		if (op.auto_save == 1) {
			score_auto_save(hWnd, bf->si);
		}
		plugin_execute_all(GetParent(hWnd), CALLTYPE_LEG_END, bf->si, bf->si->current_leg - 1);

		{
			LEG_INFO *leg;

			leg = (LEG_INFO *)mem_calloc(sizeof(LEG_INFO) * (bf->si->current_leg + 1));
			if (leg == NULL) {
				message_get_error(GetLastError(), buf);
				MessageBox(hWnd, buf, APP_NAME, MB_ICONERROR);
				bf->si->current_leg--;
				return FALSE;
			}
			CopyMemory(leg, bf->si->leg, sizeof(LEG_INFO) * bf->si->current_leg);
			mem_free(&bf->si->leg);
			bf->si->leg = leg;
			bf->si->leg[bf->si->current_leg].alloc_round = (bf->si->round_limit == 0) ? ALLOC_ROUND : bf->si->round;
			bf->si->leg[bf->si->current_leg].max_round = (bf->si->round_limit == 0) ? 1 : bf->si->round;
			bf->si->leg[bf->si->current_leg].score[0] = (TYPE_SCORE *)mem_calloc(sizeof(TYPE_SCORE) * (bf->si->leg[bf->si->current_leg].alloc_round + 1));
			if (bf->si->leg[bf->si->current_leg].score[0] == NULL) {
				message_get_error(GetLastError(), buf);
				MessageBox(hWnd, buf, APP_NAME, MB_ICONERROR);
				bf->si->current_leg--;
				return FALSE;
			}
			bf->si->leg[bf->si->current_leg].score[1] = (TYPE_SCORE *)mem_calloc(sizeof(TYPE_SCORE) * (bf->si->leg[bf->si->current_leg].alloc_round + 1));
			if (bf->si->leg[bf->si->current_leg].score[1] == NULL) {
				message_get_error(GetLastError(), buf);
				MessageBox(hWnd, buf, APP_NAME, MB_ICONERROR);
				bf->si->current_leg--;
				return FALSE;
			}

			mem_free(&bf->si->tmp_check_out[0]);
			mem_free(&bf->si->tmp_check_out[1]);
			bf->si->tmp_check_out[0] = (TYPE_CHECK_OUT *)mem_calloc(sizeof(TYPE_CHECK_OUT) * (bf->si->leg[bf->si->current_leg].alloc_round + 1));
			if (bf->si->tmp_check_out[0] == NULL) {
				message_get_error(GetLastError(), buf);
				MessageBox(hWnd, buf, APP_NAME, MB_ICONERROR);
				bf->si->current_leg--;
				return FALSE;
			}
			bf->si->tmp_check_out[1] = (TYPE_CHECK_OUT *)mem_calloc(sizeof(TYPE_CHECK_OUT) * (bf->si->leg[bf->si->current_leg].alloc_round + 1));
			if (bf->si->tmp_check_out[1] == NULL) {
				message_get_error(GetLastError(), buf);
				MessageBox(hWnd, buf, APP_NAME, MB_ICONERROR);
				bf->si->current_leg--;
				return FALSE;
			}
		}
		bf->si->leg[bf->si->current_leg].first = !bf->si->leg[bf->si->current_leg - 1].first;

		if (lParam == 0 && bf->si->leg_limit == 1 && bf->si->max_leg > 0 &&
			(bf->si->current_leg >= bf->si->max_leg ||
			(bf->si->best_of == 1 &&
			((bf->si->player[0].legs >= bf->si->max_leg / 2 + 1) ||
			(bf->si->player[1].legs >= bf->si->max_leg / 2 + 1))))) {
			if (bf->si->player[0].legs != bf->si->player[1].legs) {
				if (bf->si->player[0].legs > bf->si->player[1].legs) {
					bf->si->player[0].sets++;
				} else {
					bf->si->player[1].sets++;
				}
			}
			plugin_execute_all(GetParent(hWnd), CALLTYPE_SET_END, bf->si, bf->si->current_set);
			SendMessage(GetParent(hWnd), WM_WINDOW_NEXT_SET, 0, 0);
			return FALSE;
		}
		return TRUE;

	case WM_SCORE_PREV_LEG:
		// 前のレッグに戻る
		bf = (DRAW_BUFFER *)GetWindowLong(hWnd, GWL_USERDATA);
		if (bf == NULL) {
			break;
		}
		if (bf->prev_info.prev_flag == FALSE || bf->prev_info.si.player[0].com == TRUE || bf->prev_info.si.player[1].com == TRUE ||
			(plugin_execute_all(GetParent(hWnd), CALLTYPE_PREV_LEG, bf->si, 0) & PLUGIN_CANCEL)) {
			MessageBeep(0xFFFFFFFF);
			break;
		}
		// 確認メッセージ
		if (MessageBox(hWnd, message_get_res(IDS_STRING_PREV_LEG), APP_NAME, MB_ICONQUESTION | MB_YESNO) == IDNO) {
			break;
		}

		// 現在のレッグ情報の退避
		score_info_free(&bf->next_info.si);
		score_info_copy(&bf->next_info.si, bf->si);
		bf->next_info.view_leg = bf->view_leg;
		bf->next_info.input_x = bf->input_x;
		bf->next_info.input_y = bf->input_y;
		bf->next_info.prev_flag = TRUE;

		if (bf->si->current_leg == 0) {
			// 最新の履歴を削除
			SendMessage(GetParent(hWnd), WM_WINDOW_DELETE_LIST, 0, 0);
		}
		// 直前のレッグ情報をコピー
		score_info_free(bf->si);
		score_info_copy(bf->si, &bf->prev_info.si);
		bf->view_leg = bf->prev_info.view_leg;
		bf->input_x = bf->prev_info.input_x;
		bf->input_y = bf->prev_info.input_y;
		score_info_free(&bf->prev_info.si);
		bf->prev_info.prev_flag = FALSE;
		SendMessage(GetParent(hWnd), WM_WINDOW_SET_PREV_MENU, FALSE, 0);

		// リカバリ情報の再設定
		recovery_save(bf->si, 0, 0, 0);

		bf->pos_y = 0;
		set_scrollbar(hWnd, bf, bf->si);
		show_edit(hWnd, bf, bf->si, TRUE, TRUE);
		SendMessage(hWnd, WM_SCORE_REDRAW, 0, 0);
		SendMessage(GetParent(hWnd), WM_WINDOW_SET_FIRST, FALSE, 0);
		set_player_info(bf, bf->si, 0, -1);
		set_player_info(bf, bf->si, 1, -1);
		SendMessage(GetParent(hWnd), WM_WINDOW_REDRAW, 0, TRUE);
		SendMessage(GetParent(hWnd), WM_WINDOW_REDRAW, 1, FALSE);
		break;

	case WM_SCORE_SHOW_LEG:
		// 表示レッグの設定
		bf = (DRAW_BUFFER *)GetWindowLong(hWnd, GWL_USERDATA);
		if (bf == NULL) {
			break;
		}
		if ((int)wParam < 0) {
			break;
		}
		bf->view_leg = wParam;
		bf->pos_y = 0;
		set_scrollbar(hWnd, bf, bf->si);
		SendMessage(hWnd, WM_SCORE_REDRAW, 0, 0);
		show_edit(hWnd, bf, bf->si, FALSE, TRUE);
		break;

	case WM_SCORE_RESTART:
		// 再スタート (リカバリ)
		bf = (DRAW_BUFFER *)GetWindowLong(hWnd, GWL_USERDATA);
		if (bf == NULL) {
			break;
		}
		if ((int)wParam < 0) {
			break;
		}
		bf->view_leg = wParam;
		bf->input_x = bf->si->leg[bf->view_leg].current_player;
		bf->input_y = bf->si->leg[bf->view_leg].current_round;
		bf->si->player[0].left = get_left(bf, bf->si, -1, 0);
		bf->si->player[1].left = get_left(bf, bf->si, -1, 1);
		bf->si->leg[bf->view_leg].all_score[0] = bf->si->player[0].left;
		bf->si->leg[bf->view_leg].all_score[1] = bf->si->player[1].left;
		bf->si->leg[bf->view_leg].all_darts[0] = bf->si->leg[bf->view_leg].current_round * 3;
		bf->si->leg[bf->view_leg].all_darts[1] = bf->si->leg[bf->view_leg].current_round * 3;
		if (bf->si->leg[bf->view_leg].current_player != bf->si->leg[bf->view_leg].first) {
			bf->si->leg[bf->view_leg].all_darts[bf->si->leg[bf->view_leg].first] += 3;
		}
		set_player_info(bf, bf->si, 0, -1);
		set_player_info(bf, bf->si, 1, -1);

		set_scrollbar(hWnd, bf, bf->si);
		SendMessage(hWnd, WM_SCORE_REDRAW, 0, 0);

		SendMessage(GetParent(hWnd), WM_WINDOW_SET_FIRST, FALSE, 0);
		SendMessage(GetParent(hWnd), WM_WINDOW_REDRAW, 0, TRUE);
		SendMessage(GetParent(hWnd), WM_WINDOW_REDRAW, 1, FALSE);
		SendMessage(GetParent(hWnd), WM_WINDOW_SET_CURRENT, bf->si->leg[bf->view_leg].current_player, 0);

		if (bf->si->current_leg == 0) {
			plugin_execute_all(GetParent(hWnd), CALLTYPE_SET_START, bf->si, bf->si->current_set);
		}
		plugin_execute_all(GetParent(hWnd), CALLTYPE_LEG_START, bf->si, bf->si->current_leg);
		show_edit(hWnd, bf, bf->si, TRUE, TRUE);

		if (bf->si->leg[bf->view_leg].end_flag == FALSE &&
			bf->si->player[bf->si->leg[bf->view_leg].current_player].com == TRUE) {
			SetTimer(hWnd, ID_TIMER_COM, TIMER_INTERVAL_COM, NULL);
		}
		break;

	case WM_SCORE_REDRAW:
		// スコア情報の再描画
		bf = (DRAW_BUFFER *)GetWindowLong(hWnd, GWL_USERDATA);
		if (bf == NULL) {
			break;
		}
		if (wParam == 0) {
			InvalidateRect(hWnd, NULL, FALSE);
		} else {
			GetClientRect(hWnd, &rect);
			i = bf->header_height +
				((bf->si->leg[bf->view_leg].current_round + 1) - bf->pos_y) * bf->score_height +
				bf->score_height;
			SetRect(&rect,
				0, bf->header_height + wParam * bf->score_height,
				rect.right, (i > rect.bottom) ? rect.bottom : i);
			InvalidateRect(hWnd, &rect, FALSE);
		}
		UpdateWindow(hWnd);
		break;

	case WM_SCORE_REFRESH:
		// スコア情報の更新
		bf = (DRAW_BUFFER *)GetWindowLong(hWnd, GWL_USERDATA);
		if (bf == NULL) {
			break;
		}
		set_player_info(bf, bf->si, 0, -1);
		set_player_info(bf, bf->si, 1, -1);
		SendMessage(hWnd, WM_SCORE_REDRAW, 0, 0);
		show_edit(hWnd, bf, bf->si, TRUE, TRUE);
		break;

	case WM_SCORE_INPUT:
		// スコアの入力 (定型入力)
		bf = (DRAW_BUFFER *)GetWindowLong(hWnd, GWL_USERDATA);
		if (bf == NULL) {
			break;
		}
		if (bf->si->player[bf->input_x].lock == TRUE) {
			MessageBeep(0xFFFFFFFF);
			break;
		}
		_itot(wParam, buf, 10);
		SendMessage(bf->hedit, WM_SETTEXT, 0, (LPARAM)buf);
		SendMessage(bf->hedit, EM_SETMODIFY, TRUE, 0);
		if (lParam == 0) {
			SendMessage(hWnd, WM_COMMAND, ID_ACCEL_TAB, 0);
		}
		break;

	case WM_SCORE_SET_HALF_HEIGHT:
		// スコアシートの高さを半分に設定 (スコア履歴)
		bf = (DRAW_BUFFER *)GetWindowLong(hWnd, GWL_USERDATA);
		if (bf == NULL) {
			break;
		}
		bf->half = wParam;
		draw_free(hWnd, bf);
		draw_init(hWnd, bf);
		set_scrollbar(hWnd, bf, bf->si);
		SendMessage(hWnd, WM_SCORE_REDRAW, 0, 0);
		SendMessage(bf->hedit, WM_SETFONT, (WPARAM)bf->score_font, MAKELPARAM(TRUE, 0));
		show_edit(hWnd, bf, bf->si, FALSE, TRUE);
		break;

	case WM_SCORE_DRAW_INIT:
		// 描画情報の再初期化 (オプション)
		bf = (DRAW_BUFFER *)GetWindowLong(hWnd, GWL_USERDATA);
		if (bf == NULL) {
			break;
		}
		draw_free(hWnd, bf);
		draw_init(hWnd, bf);
		set_scrollbar(hWnd, bf, bf->si);

		DeleteObject(bf->back_brush);
		DeleteObject(bf->odd_back_brush);
		DeleteObject(bf->header_back_brush);
		DeleteObject(bf->line_pen);
		DeleteObject(bf->separate_pen);
		DeleteObject(bf->separate_bold_pen);
		DeleteObject(bf->finish_pen);
		DeleteObject(bf->ton_circle_pen);

		bf->back_brush = CreateSolidBrush(op.ci.background);
		bf->odd_back_brush = CreateSolidBrush(op.ci.odd_background);
		bf->header_back_brush = CreateSolidBrush(op.ci.header_background);
		bf->line_pen = CreatePen(PS_SOLID, 1, op.ci.line);
		bf->separate_pen = CreatePen(PS_SOLID, 1, op.ci.separate);
		bf->separate_bold_pen = CreatePen(PS_SOLID, 2, op.ci.separate);
		bf->finish_pen = CreatePen(PS_SOLID, bf->pen_size, op.ci.scored_text);
		bf->ton_circle_pen = CreatePen(PS_SOLID, bf->pen_size, op.ci.ton_circle);

		SendMessage(bf->hedit, WM_EDIT_INIT_COLOR, 0, 0);
		SendMessage(hWnd, WM_SCORE_REDRAW, 0, 0);
		SendMessage(bf->hedit, WM_SETFONT, (WPARAM)bf->score_font, MAKELPARAM(TRUE, 0));
		show_edit(hWnd, bf, bf->si, FALSE, FALSE);
		break;

	case WM_SCORE_SET_LOCK:
		// ウィンドウのロック
		bf = (DRAW_BUFFER *)GetWindowLong(hWnd, GWL_USERDATA);
		if (bf == NULL) {
			break;
		}
		bf->lock = wParam;
		EnableWindow(bf->hedit, (bf->lock == TRUE) ? FALSE : TRUE);
		break;

	case WM_SCORE_GET_BUFFER:
		// 描画情報の取得
		bf = (DRAW_BUFFER *)GetWindowLong(hWnd, GWL_USERDATA);
		if (bf == NULL) {
			break;
		}
		return (LRESULT)bf;

	case WM_N01_GET_INPUT_POS:
		// 入力位置の取得
		bf = (DRAW_BUFFER *)GetWindowLong(hWnd, GWL_USERDATA);
		if (bf == NULL) {
			break;
		}
		if (wParam != 0) {
			*(int *)wParam = bf->input_x;
		}
		if (lParam != 0) {
			*(int *)lParam = bf->input_y;
		}
		return MAKELONG((WORD)bf->input_x, (WORD)bf->input_y);

	case WM_N01_SET_INPUT_POS:
		// 入力位置の設定
		bf = (DRAW_BUFFER *)GetWindowLong(hWnd, GWL_USERDATA);
		if (bf == NULL) {
			break;
		}
		if (wParam == TRUE) {
			// スコア設定
			show_edit(hWnd, bf, bf->si, FALSE, TRUE);
			SendMessage(bf->hedit, WM_GETTEXT, BUF_SIZE - 1, (LPARAM)buf);
			if (*buf == TEXT('\0') && bf->input_y != bf->si->leg[bf->view_leg].current_round) {
				break;
			}
			if (*buf != TEXT('\0') && SendMessage(bf->hedit, EM_GETMODIFY, 0, 0) == TRUE) {
				if (set_score(hWnd, bf, bf->si, (TYPE_SCORE)_ttoi(buf)) == FALSE) {
					break;
				}
			}
		}
		x = (LOWORD(lParam) != 0xFFFF) ? LOWORD(lParam) : bf->input_x;
		y = (HIWORD(lParam) != 0xFFFF) ? HIWORD(lParam) : bf->input_y;
		if (move_input(hWnd, bf, bf->si, x, y) == TRUE) {
			show_edit(hWnd, bf, bf->si, TRUE, TRUE);
		}
		break;

	case WM_N01_SCORE_INPUT:
		// スコアの入力 (プラグイン)
		bf = (DRAW_BUFFER *)GetWindowLong(hWnd, GWL_USERDATA);
		if (bf == NULL) {
			break;
		}
		if ((int)wParam < 0) {
			set_finish(hWnd, bf, bf->si, (int)wParam * -1, bf->si->player[bf->input_x].lock);
		} else {
			_itot(wParam, buf, 10);
			SendMessage(bf->hedit, WM_SETTEXT, 0, (LPARAM)buf);
			SendMessage(bf->hedit, EM_SETMODIFY, TRUE, 0);
			if (lParam == 0) {
				SendMessage(hWnd, WM_COMMAND, ID_ACCEL_TAB, 0);
			}
		}
		break;

	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

/*
 * score_list_regist - ウィンドウクラスの登録
 */
BOOL score_list_regist(const HINSTANCE hInstance)
{
	WNDCLASS wc;

	wc.style = 0;
	wc.lpfnWndProc = (WNDPROC)score_list_proc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = WINDOW_CLASS;
	// ウィンドウクラスの登録
	return RegisterClass(&wc);
}

/*
 * score_list_create - ウィンドウの作成
 */
HWND score_list_create(const HINSTANCE hInstance, const HWND pWnd, int id, SCORE_INFO *si)
{
	HWND hWnd;

	// ウィンドウの作成
	hWnd = CreateWindow(WINDOW_CLASS,
		TEXT(""),
		WS_CHILD | WS_VISIBLE | WS_TABSTOP | ((op.view_scroll_bar == 1) ? WS_VSCROLL : 0),
		0, 0, 100, 100, pWnd, (HMENU)id, hInstance, si);
	return hWnd;
}
/* End of source */
