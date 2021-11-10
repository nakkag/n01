/*
 * n01
 *
 * nEdit.c
 *
 * Copyright (C) 1996-2008 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

/* Include Files */
#include <windows.h>
#include <tchar.h>

#include "general.h"
#include "memory.h"
#include "string.h"
#include "nEdit.h"
#include "score_list.h"

/* Define */
#define RESERVE_BUF						1024
#define RESERVE_INPUT					256
#define RESERVE_UNDO					256

#define DRAW_LEN						256
#define CARET_WIDTH						2

#define SWAP(a, b)						{a = b - a; b -= a; a += b;}

// ホイールメッセージ
#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL					0x020A
#endif

/* Global Variables */
extern OPTION_INFO op;

/* Local Function Prototypes */
static BOOL string_to_clipboard(const HWND hWnd, const TCHAR *st, const TCHAR *en);

static TCHAR *char_next(const BUFFER *bf, TCHAR *p);
static TCHAR *char_prev(const BUFFER *bf, TCHAR *p);
static TCHAR *index_to_char(const BUFFER *bf, const DWORD index);
static DWORD char_to_index(const BUFFER *bf, const TCHAR *p);

static BOOL undo_alloc(BUFFER *bf);
static void undo_free(BUFFER *bf, const int index);
static BOOL undo_set(BUFFER *bf, const int type, const DWORD st, const DWORD len);
static BOOL undo_exec(const HWND hWnd, BUFFER *bf);
static BOOL redo_exec(const HWND hWnd, BUFFER *bf);

static BOOL string_init(BUFFER *bf);
static BOOL string_set(const HWND hWnd, BUFFER *bf, const TCHAR *str, const DWORD len);
static BOOL string_insert(const HWND hWnd, BUFFER *bf, TCHAR *str, const BOOL insert_mode);
static void string_delete(const HWND hWnd, BUFFER *bf, DWORD st, DWORD en);
static void string_delete_char(const HWND hWnd, BUFFER *bf, DWORD st);
static BOOL string_flush(BUFFER *bf, const BOOL undo_flag);

static int draw_string(const HDC mdc, const BUFFER *bf, const int left, const int top, const TCHAR *str, const int len);
static void draw_line(const HDC mdc, BUFFER *bf, const int width, const int height);

static void caret_set_size(const HWND hWnd, BUFFER *bf);
static int caret_char_to_caret(const HDC mdc, BUFFER *bf, const DWORD cp, const int width);
static DWORD caret_point_to_caret(BUFFER *bf, const int x, const int width);
static void caret_move(const HWND hWnd, BUFFER *bf, const int key);

static LRESULT CALLBACK nedit_proc(const HWND hWnd, const UINT msg, const WPARAM wParam, const LPARAM lParam);

/*
 * string_to_clipboard - 文字列をクリップボードに設定
 */
static BOOL string_to_clipboard(const HWND hWnd, const TCHAR *st, const TCHAR *en)
{
	HANDLE hMem;
	TCHAR *buf;

	if (OpenClipboard(hWnd) == FALSE) {
		return FALSE;
	}
	if (EmptyClipboard() == FALSE) {
		CloseClipboard();
		return FALSE;
	}
	if ((hMem = GlobalAlloc(GHND, sizeof(TCHAR) * (en - st + 1))) == NULL) {
		CloseClipboard();
		return FALSE;
	}
	if ((buf = GlobalLock(hMem)) == NULL) {
		GlobalFree(hMem);
		CloseClipboard();
		return FALSE;
	}
	lstrcpyn(buf, st, en - st + 1);
	GlobalUnlock(hMem);
#ifdef UNICODE
	SetClipboardData(CF_UNICODETEXT, hMem);
#else
	SetClipboardData(CF_TEXT, hMem);
#endif
	CloseClipboard();
	return TRUE;
}

/*
 * char_next - 次の文字を取得
 */
static TCHAR *char_next(const BUFFER *bf, TCHAR *p)
{
	if (bf->dp != NULL && p == bf->dp - 1) {
		return (bf->dp + bf->del_len);
	}
	if (bf->ip != NULL) {
		if (p == bf->ip - 1) {
			return bf->input_buf;
		} else if (p == bf->input_buf + bf->input_len - 1) {
			return (bf->ip + bf->ip_len);
		}
	}
	return (p + 1);
}

/*
 * char_prev - 前の文字を取得
 */
static TCHAR *char_prev(const BUFFER *bf, TCHAR *p)
{
	if (bf->dp != NULL && p == bf->dp + bf->del_len) {
		return (bf->dp - 1);
	}
	if (bf->ip != NULL) {
		if (p == (bf->ip + bf->ip_len)) {
			return (bf->input_buf + bf->input_len - 1);
		} else if (p == bf->input_buf) {
			return (bf->ip - 1);
		}
	}
	return (p - 1);
}

/*
 * index_to_char - インデックスから文字位置を取得
 */
static TCHAR *index_to_char(const BUFFER *bf, const DWORD index)
{
	TCHAR *p;

	if (bf->dp != NULL && index >= (DWORD)(bf->dp - bf->buf)) {
		p = bf->buf + index + bf->del_len;
	} else if (bf->ip != NULL && index >= (DWORD)(bf->ip - bf->buf)) {
		if (index < (DWORD)(bf->ip - bf->buf + bf->input_len)) {
			p = bf->input_buf + (index - (bf->ip - bf->buf));
		} else {
			p = bf->buf + index - bf->input_len + bf->ip_len;
		}
	} else {
		p = bf->buf + index;
	}
	return p;
}

/*
 * char_to_index - 文字位置からインデックスを取得
 */
static DWORD char_to_index(const BUFFER *bf, const TCHAR *p)
{
	DWORD i;

	if (bf->dp != NULL && p >= bf->dp) {
		i = p - bf->buf - bf->del_len;
	} else if (bf->ip != NULL && !(p >= bf->buf && p < bf->ip)) {
		if (p >= bf->input_buf && p <= bf->input_buf + bf->input_len) {
			i = (bf->ip - bf->buf) + (p - bf->input_buf);
		} else {
			i = (p - bf->buf) + bf->input_len - bf->ip_len;
		}
	} else {
		i = p - bf->buf;
	}
	return i;
}

/*
 * undo_alloc - UNDOの確保
 */
static BOOL undo_alloc(BUFFER *bf)
{
	UNDO *ud;

	bf->undo_size += RESERVE_UNDO;
	if ((ud = mem_calloc(sizeof(UNDO) * bf->undo_size)) == NULL) {
		return FALSE;
	}
	if (bf->undo != NULL) {
		CopyMemory(ud, bf->undo, sizeof(UNDO) * bf->undo_len);
		mem_free(&bf->undo);
	}
	bf->undo = ud;
	return TRUE;
}

/*
 * undo_free - UNDOの解放
 */
static void undo_free(BUFFER *bf, const int index)
{
	int i;

	for (i = index; i < bf->undo_size; i++) {
		(bf->undo + i)->type = 0;
		mem_free(&(bf->undo + i)->buf);
	}
}

/*
 * undo_set - UNDOのセット
 */
static BOOL undo_set(BUFFER *bf, const int type, const DWORD st, const DWORD len)
{
	undo_free(bf, bf->undo_len);
	if (bf->undo_len + 1 >= bf->undo_size && undo_alloc(bf) == FALSE) {
		return FALSE;
	}
	(bf->undo + bf->undo_len)->type = type;
	(bf->undo + bf->undo_len)->st = st;
	(bf->undo + bf->undo_len)->len = len;
	switch (type) {
	case UNDO_TYPE_INPUT:
		// 入力
		break;

	case UNDO_TYPE_DELETE:
		// 削除
		if (((bf->undo + bf->undo_len)->buf = mem_alloc(sizeof(TCHAR) * (len + 1))) == NULL) {
			return FALSE;
		}
		lstrcpyn((bf->undo + bf->undo_len)->buf, bf->buf + st, len + 1);
		break;
	}
	bf->undo_len++;
	return TRUE;
}

/*
 * undo_exec - UNDOの実行
 */
static BOOL undo_exec(const HWND hWnd, BUFFER *bf)
{
	int i;

	string_flush(bf, TRUE);

	i = bf->undo_len - 1;
	if (i < 0) {
		return TRUE;
	}
	bf->sp = bf->cp = (bf->undo + i)->st;
	switch ((bf->undo + i)->type) {
	case UNDO_TYPE_INPUT:
		if ((bf->undo + i)->buf == NULL) {
			if (((bf->undo + i)->buf = mem_alloc(sizeof(TCHAR) * ((bf->undo + i)->len + 1))) == NULL) {
				return FALSE;
			}
			lstrcpyn((bf->undo + i)->buf, bf->buf + (bf->undo + i)->st, (bf->undo + i)->len + 1);
		}
		string_delete(hWnd, bf, (bf->undo + i)->st, (bf->undo + i)->st + (bf->undo + i)->len);
		break;

	case UNDO_TYPE_DELETE:
		string_insert(hWnd, bf, (bf->undo + i)->buf, TRUE);
		break;
	}
	string_flush(bf, FALSE);
	bf->undo_len--;

	SendMessage(hWnd, EM_SETMODIFY, (bf->undo_len == bf->undo_pos) ? FALSE : TRUE, 0);
	SendMessage(hWnd, WM_REFRESH, 0, 0);
	return TRUE;
}

/*
 * redo_exec - REDOの実行
 */
static BOOL redo_exec(const HWND hWnd, BUFFER *bf)
{
	int i;

	string_flush(bf, TRUE);

	i = bf->undo_len;
	if ((bf->undo + i)->type == 0) {
		return TRUE;
	}
	bf->sp = bf->cp = (bf->undo + i)->st;
	switch ((bf->undo + i)->type) {
	case UNDO_TYPE_INPUT:
		string_insert(hWnd, bf, (bf->undo + i)->buf, TRUE);
		break;

	case UNDO_TYPE_DELETE:
		string_delete(hWnd, bf, (bf->undo + i)->st, (bf->undo + i)->st + (bf->undo + i)->len);
		break;
	}
	string_flush(bf, FALSE);
	bf->undo_len++;

	SendMessage(hWnd, EM_SETMODIFY, (bf->undo_len == bf->undo_pos) ? FALSE : TRUE, 0);
	SendMessage(hWnd, WM_REFRESH, 0, 0);
	return TRUE;
}

/*
 * string_init - 初期化
 */
static BOOL string_init(BUFFER *bf)
{
	// free
	mem_free(&bf->input_buf);
	undo_free(bf, 0);
	mem_free(&bf->undo);

	// input init
	bf->input_size = RESERVE_INPUT;
	if ((bf->input_buf = mem_alloc(sizeof(TCHAR) * bf->input_size)) == NULL) {
		return FALSE;
	}
	*bf->input_buf = TEXT('\0');
	bf->input_len = 0;
	bf->ip = NULL;
	bf->ip_len = 0;

	// undo init
	bf->undo_size = RESERVE_UNDO;
	if ((bf->undo = mem_calloc(sizeof(UNDO) * bf->undo_size)) == NULL) {
		return FALSE;
	}
	bf->undo_len = 0;

	bf->sp = bf->cp = 0;
	bf->dp = NULL;
	bf->del_len = 0;
	bf->modified = FALSE;
	return TRUE;
}

/*
 * string_set - 文字列の設定
 */
static BOOL string_set(const HWND hWnd, BUFFER *bf, const TCHAR *str, const DWORD len)
{
	if (string_init(bf) == FALSE) {
		return FALSE;
	}

	// 文字列設定
	mem_free(&bf->buf);
	bf->buf_len = bf->view_len = len;
	bf->buf_size = bf->buf_len + 1 + RESERVE_BUF;
	if ((bf->buf = mem_alloc(sizeof(TCHAR) * bf->buf_size)) == NULL) {
		return FALSE;
	}
	CopyMemory(bf->buf, str, sizeof(TCHAR) * bf->buf_len + 1);
	SendMessage(hWnd, WM_REFRESH, 0, 0);
	return TRUE;
}

/*
 * string_insert - 文字列の追加
 */
static BOOL string_insert(const HWND hWnd, BUFFER *bf, TCHAR *str, const BOOL insert_mode)
{
	TCHAR *p;
	DWORD len, ip_len;
	BOOL sel = FALSE;

	// 入力制限
	for (p = str; *p >= TEXT('0') && *p <= TEXT('9'); p++);
	if (*p != TEXT('\0')) {
		MessageBeep(0xFFFFFFFF);
		return FALSE;
	}
	if (insert_mode == TRUE && bf->limit_len != 0 && 
		(bf->view_len - ((bf->cp > bf->sp) ? bf->cp - bf->sp : bf->sp - bf->cp)) + lstrlen(str) > bf->limit_len) {
		MessageBeep(0xFFFFFFFF);
		return FALSE;
	}

	if (bf->cp != bf->sp) {
		// 選択文字の削除
		string_delete(hWnd, bf, bf->cp, bf->sp);
		if (bf->cp > bf->sp) {
			SWAP(bf->cp, bf->sp);
		}
		bf->sp = bf->cp;
		sel = TRUE;
	}
	if (bf->dp != NULL) {
		string_flush(bf, TRUE);
	}

	// 入力制限
	if ((insert_mode == FALSE && bf->limit_len != 0 &&
		(bf->cp != bf->view_len && bf->view_len - lstrlen(str) >= bf->limit_len)) ||
		(bf->cp == bf->view_len && bf->view_len >= bf->limit_len)) {
		MessageBeep(0xFFFFFFFF);
		return FALSE;
	}

	// 入力バッファの設定
	len = lstrlen(str);
	if (bf->input_len + len + 1 > bf->input_size) {
		bf->input_size = bf->input_len + len + 1 + RESERVE_INPUT;
		if ((p = mem_alloc(sizeof(TCHAR) * bf->input_size)) == NULL) {
			return FALSE;
		}
		lstrcpy(p, bf->input_buf);
		mem_free(&bf->input_buf);
		bf->input_buf = p;
	}
	lstrcpy(bf->input_buf + bf->input_len, str);
	bf->input_len += len;
	if (bf->ip == NULL) {
		bf->ip = index_to_char(bf, bf->cp);
	}
	bf->cp += len;
	bf->sp = bf->cp;

	// 上書きモード
	for (p = str, ip_len = 0; sel == FALSE && insert_mode == FALSE && *p != TEXT('\0'); p++, ip_len++) {
		if (*(bf->ip + bf->ip_len + ip_len) == TEXT('\0')) {
			break;
		}
	}
	bf->ip_len += ip_len;
	bf->view_len = bf->buf_len + bf->input_len - bf->del_len - bf->ip_len;
	SendMessage(hWnd, WM_REFRESH, 0, 0);

	if (bf->modified == FALSE) {
		bf->undo_pos = bf->undo_len;
	}
	bf->modified = TRUE;
	return TRUE;
}

/*
 * string_delete - 文字列の削除
 */
static void string_delete(const HWND hWnd, BUFFER *bf, DWORD st, DWORD en)
{
	if (st > en) {
		SWAP(st, en);
	}
	if (bf->ip != NULL || (bf->dp != NULL && bf->dp != bf->buf + st && bf->dp != bf->buf + en)) {
		string_flush(bf, TRUE);
	}
	if (en - st > bf->buf_len - bf->del_len || st >= bf->buf_len - bf->del_len) {
		return;
	}

	// 削除位置設定
	bf->dp = bf->buf + st;
	bf->del_len += en - st;
	bf->view_len = bf->buf_len + bf->input_len - bf->del_len - bf->ip_len;

	InvalidateRect(hWnd, NULL, FALSE);
	if (bf->modified == FALSE) {
		bf->undo_pos = bf->undo_len;
	}
	bf->modified = TRUE;
}

/*
 * string_delete_char - 文字の削除
 */
static void string_delete_char(const HWND hWnd, BUFFER *bf, DWORD st)
{
	int i = 1;

	if (bf->cp != bf->sp) {
		// 選択文字の削除
		string_delete(hWnd, bf, bf->cp, bf->sp);
		if (bf->cp > bf->sp) {
			SWAP(bf->cp, bf->sp);
		}
		bf->sp = bf->cp;
	} else {
		// 一文字削除
		string_delete(hWnd, bf, st, st + i);
	}
}

/*
 * string_flush - 削除と入力バッファの反映
 */
static BOOL string_flush(BUFFER *bf, const BOOL undo_flag)
{
	TCHAR *p;

	if (bf->dp != NULL) {
		if (undo_flag == TRUE) {
			// undoに追加
			undo_set(bf, UNDO_TYPE_DELETE, bf->dp - bf->buf, bf->del_len);
		}
		// 削除文字列の反映
		MoveMemory(bf->dp, bf->dp + bf->del_len, sizeof(TCHAR) * (bf->buf_len - (bf->dp - bf->buf + bf->del_len) + 1));
		bf->buf_len -= bf->del_len;
		bf->dp = NULL;
		bf->del_len = 0;

		if (bf->buf_len + 1 < bf->buf_size - RESERVE_BUF) {
			bf->buf_size = bf->buf_len + 1 + RESERVE_BUF;
			p = mem_alloc(sizeof(TCHAR) * bf->buf_size);
			if (p == NULL) {
				return FALSE;
			}
			CopyMemory(p, bf->buf, sizeof(TCHAR) * (bf->buf_len + 1));
			mem_free(&bf->buf);
			bf->buf = p;
		}
		return TRUE;
	}
	if (bf->ip == NULL) {
		return TRUE;
	}

	if (undo_flag == TRUE) {
		// undoに追加
		if (bf->ip_len > 0) {
			undo_set(bf, UNDO_TYPE_DELETE, bf->ip - bf->buf, bf->ip_len);
		}
		undo_set(bf, UNDO_TYPE_INPUT, bf->ip - bf->buf, bf->input_len);
	}

	// 入力バッファの反映
	if (bf->buf_len + bf->input_len - bf->ip_len + 1 > bf->buf_size) {
		bf->buf_size = bf->buf_len + bf->input_len - bf->ip_len + 1 + RESERVE_BUF;
		if ((p = mem_alloc(sizeof(TCHAR) * bf->buf_size)) == NULL) {
			return FALSE;
		}
		if (bf->ip != bf->buf) {
			CopyMemory(p, bf->buf, sizeof(TCHAR) * (bf->ip - bf->buf));
		}
		CopyMemory(p + (bf->ip - bf->buf), bf->input_buf, sizeof(TCHAR) * bf->input_len);
		CopyMemory(p + (bf->ip - bf->buf) + bf->input_len, bf->ip + bf->ip_len, sizeof(TCHAR) * (bf->buf_len - (bf->ip + bf->ip_len - bf->buf) + 1));
		mem_free(&bf->buf);
		bf->buf = p;
	} else {
		MoveMemory(bf->ip + bf->input_len, bf->ip + bf->ip_len, sizeof(TCHAR) * (bf->buf_len - (bf->ip + bf->ip_len - bf->buf) + 1));
		CopyMemory(bf->ip, bf->input_buf, sizeof(TCHAR) * bf->input_len);
	}
	bf->buf_len += bf->input_len - bf->ip_len;

	// 入力バッファの解放
	if (bf->input_size > RESERVE_INPUT) {
		mem_free(&bf->input_buf);
		bf->input_size = RESERVE_INPUT;
		if ((bf->input_buf = mem_alloc(sizeof(TCHAR) * bf->input_size)) == NULL) {
			return FALSE;
		}
	}
	*bf->input_buf = TEXT('\0');
	bf->input_len = 0;
	bf->ip = NULL;
	bf->ip_len = 0;
	return TRUE;
}

/*
 * get_draw_string_length - 描画する文字の幅取得
 */
static int get_draw_string_length(const HDC mdc, const BUFFER *bf, const TCHAR *str, const int len)
{
	SIZE sz;
	int csize;

	if (str == NULL) {
		return 0;
	}
	if (bf->fixed == FALSE || bf->bold == TRUE) {
		GetTextExtentPoint32(mdc, str, len, &sz);
		csize = sz.cx;
	} else {
		csize = len * bf->char_width;
	}
	return csize;
}

/*
 * get_draw_line_length - 描画する文字列の幅取得
 */
static int get_draw_line_length(const HDC mdc, const BUFFER *bf)
{
	TCHAR *p, *r, *s;
	DWORD j;
	int offset;

	offset = 0;

	for (j = 0, s = p = index_to_char(bf, 0); j < bf->view_len; j++, p = char_next(bf, p)) {
		if (s != p) {
			if (bf->ip != NULL && (p == bf->ip + bf->ip_len || p == bf->input_buf)) {
				// 入力バッファの出力
				r = (p == bf->input_buf) ? bf->ip : (bf->input_buf + bf->input_len);
				offset += get_draw_string_length(mdc, bf, s, r - s);
				s = p;
			} else if (bf->dp != NULL && p == bf->dp + bf->del_len) {
				// 削除文字列
				offset += get_draw_string_length(mdc, bf, s, bf->dp - s);
				s = p;
			} else if ((j % DRAW_LEN) == 0) {
				offset += get_draw_string_length(mdc, bf, s, p - s);
				s = p;
			}
		}
	}
	if (s != p && bf->ip != NULL && (p == bf->ip + bf->ip_len || p == bf->input_buf)) {
		r = (p == bf->input_buf) ? bf->ip : (bf->input_buf + bf->input_len);
	} else if (s != p && bf->dp != NULL && p == bf->dp + bf->del_len) {
		r = bf->dp;
	} else {
		r = p;
	}
	return (get_draw_string_length(mdc, bf, s, r - s) + offset);
}

/*
 * draw_string - 文字列描画
 */
static int draw_string(const HDC mdc, const BUFFER *bf, const int left, const int top, const TCHAR *str, const int len)
{
	SIZE sz;
	int csize;

	if (str == NULL) {
		return 0;
	}
	if (bf->fixed == FALSE || bf->bold == TRUE) {
		GetTextExtentPoint32(mdc, str, len, &sz);
		csize = sz.cx;
	} else {
		csize = len * bf->char_width;
	}
	TextOut(mdc, left, top, str, len);
	return csize;
}

/*
 * draw_line - 1行描画
 */
static void draw_line(const HDC mdc, BUFFER *bf, const int width, const int height)
{
	TCHAR *p, *r, *s;
	DWORD j;
	int offset;
	int top;
	BOOL sel = FALSE;

	if (bf->view_len >= 3 && op.view_ton_circle == 1 && *index_to_char(bf, 0) != TEXT('0')) {
		// TON
		HPEN ret_pen;
		HBRUSH ret_brush;
		RECT draw_rect;

		if (bf->pen_size == 4) {
			ret_brush = SelectObject(bf->ellipse_dc, bf->back_brush);
			ret_pen = SelectObject(bf->ellipse_dc, bf->ton_circle_pen);
			SetRect(&draw_rect, 0, 0, width * 2, height * 2);
			FillRect(bf->ellipse_dc, &draw_rect, bf->back_brush);
			Ellipse(bf->ellipse_dc, 2, 2, width * 2 - 2, height * 2 - 2);
			SelectObject(bf->ellipse_dc, ret_pen);
			SelectObject(bf->ellipse_dc, ret_brush);

			StretchBlt(mdc, 0, 0, width, height,
				bf->ellipse_dc, 0, 0, width * 2, height * 2, SRCCOPY);
		} else {
			ret_brush = SelectObject(mdc, bf->back_brush);
			ret_pen = SelectObject(mdc, bf->ton_circle_pen);
			Ellipse(mdc, 1, 1, width - 1, height - 1);
			SelectObject(mdc, ret_pen);
			SelectObject(mdc, ret_brush);
		}
	}
	SetTextColor(mdc, op.ci.input_text);
	SetBkColor(bf->mdc, op.ci.input_background);
	SetBkMode(mdc, TRANSPARENT);

	offset = (width - get_draw_line_length(mdc, bf)) / 2;
	top = (height - bf->font_height) / 2;
	for (j = 0, s = p = index_to_char(bf, 0); j < bf->view_len; j++, p = char_next(bf, p)) {
		if (s != p) {
			if (bf->ip != NULL && (p == bf->ip + bf->ip_len || p == bf->input_buf)) {
				// 入力バッファの出力
				r = (p == bf->input_buf) ? bf->ip : (bf->input_buf + bf->input_len);
				offset += draw_string(mdc, bf, offset, top, s, r - s);
				s = p;
			} else if (bf->dp != NULL && p == bf->dp + bf->del_len) {
				// 削除文字列
				offset += draw_string(mdc, bf, offset, top, s, bf->dp - s);
				s = p;
			} else if ((j % DRAW_LEN) == 0) {
				offset += draw_string(mdc, bf, offset, top, s, p - s);
				s = p;
			}
		}
		if ((j >= bf->sp && j < bf->cp) || (j >= bf->cp && j < bf->sp)) {
			if (sel == FALSE) {
				// 選択開始
				offset += draw_string(mdc, bf, offset, top, s, p - s);
				s = p;

				SetTextColor(mdc, op.ci.input_select_text);
				SetBkColor(mdc, op.ci.input_select_background);
				SetBkMode(mdc, OPAQUE);
				sel = TRUE;
			}
		} else if (sel == TRUE) {
			// 選択終了
			offset += draw_string(mdc, bf, offset, top, s, p - s);
			s = p;

			SetTextColor(mdc, op.ci.input_text);
			SetBkColor(mdc, op.ci.input_background);
			SetBkMode(mdc, TRANSPARENT);
			sel = FALSE;
		}
	}
	if (s != p && bf->ip != NULL && (p == bf->ip + bf->ip_len || p == bf->input_buf)) {
		r = (p == bf->input_buf) ? bf->ip : (bf->input_buf + bf->input_len);
	} else if (s != p && bf->dp != NULL && p == bf->dp + bf->del_len) {
		r = bf->dp;
	} else {
		r = p;
	}
	draw_string(mdc, bf, offset, top, s, r - s);
}

/*
 * caret_set_size - キャレットのサイズ設定
 */
static void caret_set_size(const HWND hWnd, BUFFER *bf)
{
	SIZE sz;
	TCHAR *p;
	int csize;
	int len;

	p = index_to_char(bf, bf->cp);
	len = 1;
	if (bf->fixed == FALSE || bf->bold == TRUE) {
		GetTextExtentPoint32(bf->mdc, p, len, &sz);
		csize = sz.cx;
		if (csize <= 0) {
			csize = bf->char_width;
		}
	} else {
		csize = len * bf->char_width;
	}
	DestroyCaret();
	CreateCaret(hWnd, NULL, csize, bf->font_height);
}

/*
 * caret_char_to_caret - 文字位置からキャレットの位置取得
 */
static int caret_char_to_caret(const HDC mdc, BUFFER *bf, const DWORD cp, const int width)
{
	SIZE sz;
	TCHAR *p, *r, *s;
	DWORD j;
	int offset;

	offset = (((width - get_draw_line_length(mdc, bf)) * 10) / 2 + 5) / 10 - 1;
	for (j = 0, s = p = index_to_char(bf, 0); j < bf->view_len; j++, p = char_next(bf, p)) {
		r = NULL;
		if (s != p && bf->ip != NULL && (p == bf->ip + bf->ip_len || p == bf->input_buf)) {
			// 入力バッファ
			r = (p == bf->input_buf) ? bf->ip : (bf->input_buf + bf->input_len);
		} else if (s != p && bf->dp != NULL && p == bf->dp + bf->del_len) {
			// 削除文字列
			r = bf->dp;
		}
		if (r != NULL) {
			if (bf->fixed == FALSE || bf->bold == TRUE) {
				GetTextExtentPoint32(mdc, s, r - s, &sz);
				offset += sz.cx;
			} else {
				offset += (r - s) * bf->char_width;
			}
			s = p;
		}
		if (j >= cp) {
			break;
		}
	}
	if (s != p && bf->ip != NULL && (p == bf->ip + bf->ip_len || p == bf->input_buf)) {
		r = (p == bf->input_buf) ? bf->ip : (bf->input_buf + bf->input_len);
	} else if (s != p && bf->dp != NULL && p == bf->dp + bf->del_len) {
		r = bf->dp;
	} else {
		r = p;
	}
	if (bf->fixed == FALSE || bf->bold == TRUE) {
		GetTextExtentPoint32(mdc, s, r - s, &sz);
		offset += sz.cx;
	} else {
		offset += (r - s) * bf->char_width;
	}
	return offset;
}

/*
 * caret_point_to_caret - 座標からキャレットの位置取得
 */
static DWORD caret_point_to_caret(BUFFER *bf, const int x, const int width)
{
	SIZE sz;
	TCHAR *p;
	DWORD j;
	int offset, old;

	old = offset = (((width - get_draw_line_length(bf->mdc, bf)) * 10) / 2 + 5) / 10 - 1;
	for (j = 0, p = index_to_char(bf, 0); j < bf->view_len; j++, p = char_next(bf, p)) {
		if (bf->fixed == FALSE || bf->bold == TRUE) {
			GetTextExtentPoint32(bf->mdc, p, 1, &sz);
			offset += sz.cx;
		} else {
			offset += bf->char_width;
		}
		if (offset > x) {
			if ((offset - old) / 2 < x - old) {
				j++;
			}
			break;
		}
		old = offset;
	}
	return j;
}

/*
 * caret_move - キャレットの移動
 */
static void caret_move(const HWND hWnd, BUFFER *bf, const int key)
{
	RECT rect;
	TCHAR *p;
	DWORD oldcp, oldsp;
	DWORD j;
	int t;

	GetClientRect(hWnd, &rect);
	oldcp = bf->cp;
	oldsp = bf->sp;

	switch (key) {
	case VK_HOME:
		// 全体の先頭
		bf->cp = 0;
		InvalidateRect(hWnd, NULL, FALSE);
		break;

	case VK_END:
		// 全体の末尾
		bf->cp = bf->view_len;
		InvalidateRect(hWnd, NULL, FALSE);
		break;

	case VK_LEFT:
		if (GetKeyState(VK_SHIFT) >= 0) {
			if (bf->cp == 0 && bf->sp == 0) {
				SendMessage(GetParent(hWnd), WM_COMMAND, WM_SCORE_MOVE_LEFT, 0);
				return;
			}
		}
		if (bf->cp != bf->sp && GetKeyState(VK_SHIFT) >= 0) {
			// 選択解除
			if (bf->cp > bf->sp) {
				SWAP(bf->cp, bf->sp);
			}
			bf->sp = bf->cp;
			break;
		}
		if (bf->cp == 0) {
			break;
		}
		for (j = 0, t = 0; j < bf->view_len && j != bf->cp; j++) {
			t = j;
			p = index_to_char(bf, j);
			if (*p == TEXT('\0')) {
				break;
			}
		}
		bf->cp = t;
		break;

	case VK_RIGHT:
		if (GetKeyState(VK_SHIFT) >= 0) {
			if (bf->cp == bf->view_len && bf->sp == bf->view_len) {
				SendMessage(GetParent(hWnd), WM_COMMAND, WM_SCORE_MOVE_RIGHT, 0);
				return;
			}
		}
		if (bf->cp != bf->sp && GetKeyState(VK_SHIFT) >= 0) {
			// 選択解除
			if (bf->cp < bf->sp) {
				SWAP(bf->cp, bf->sp);
			}
			bf->sp = bf->cp;
			break;
		}
		if (*(index_to_char(bf, bf->cp)) == TEXT('\0')) {
			break;
		}
		bf->cp++;
		break;
	}
	if (GetKeyState(VK_SHIFT) >= 0) {
		bf->sp = bf->cp;
	}
	SendMessage(hWnd, WM_REFRESH, 0, 0);
}

/*
 * nedit_proc - Editウィンドウプロシージャ
 */
static LRESULT CALLBACK nedit_proc(const HWND hWnd, const UINT msg, const WPARAM wParam, const LPARAM lParam)
{
	BUFFER *bf;
	HDC hdc;
	HBITMAP hBmp;
	RECT rect;
	TCHAR in[3];
	DWORD cp, sp;
	int len;

	switch (msg) {
	case WM_CREATE:
		if ((bf = mem_calloc(sizeof(BUFFER))) == NULL) {
			return -1;
		}
		bf->insert_mode = TRUE;
		bf->refresh_mode = TRUE;

		if (string_init(bf) == FALSE) {
			return -1;
		}
		// バッファの初期化
		bf->buf_size = RESERVE_BUF;
		if ((bf->buf = mem_alloc(sizeof(TCHAR) * bf->buf_size)) == NULL) {
			return -1;
		}
		*bf->buf = TEXT('\0');

		// 描画情報の初期化
		hdc = GetDC(hWnd);
		GetClientRect(hWnd, &rect);
		bf->mdc = CreateCompatibleDC(hdc);
		hBmp = CreateCompatibleBitmap(hdc, rect.right, rect.bottom);
		bf->ret_bmp = SelectObject(bf->mdc, hBmp);
		if (GetDeviceCaps(bf->mdc, BITSPIXEL) >= 16) {
			SetStretchBltMode(bf->mdc, HALFTONE);
			SetBrushOrgEx(bf->mdc, 0, 0, NULL);
			bf->pen_size = 4;
		} else {
			bf->pen_size = 2;
		}

		bf->ellipse_dc = CreateCompatibleDC(hdc);
		hBmp = CreateCompatibleBitmap(hdc, rect.right * 2, rect.bottom * 2);
		bf->ellipse_ret_bmp = SelectObject(bf->ellipse_dc, hBmp);

		ReleaseDC(hWnd, hdc);

		SetMapMode(bf->mdc, MM_TEXT);
		SetTextCharacterExtra(bf->mdc, 0);
		SetTextJustification(bf->mdc, 0, 0);
		SetTextAlign(bf->mdc, TA_TOP | TA_LEFT);
		SetBkMode(bf->mdc, TRANSPARENT);
		bf->back_brush = CreateSolidBrush(op.ci.input_background);
		bf->ton_circle_pen = CreatePen(PS_SOLID, bf->pen_size, op.ci.ton_circle);

		ImmAssociateContext(hWnd, (HIMC)NULL);

		// buffer info to window long
		SetWindowLong(hWnd, GWL_USERDATA, (LPARAM)bf);
		SendMessage(hWnd, WM_REFLECT, 0, 0);
		break;

	case WM_DESTROY:
		if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) != NULL) {
			SetWindowLong(hWnd, GWL_USERDATA, (LPARAM)0);

			hBmp = SelectObject(bf->mdc, bf->ret_bmp);
			DeleteObject(hBmp);
			hBmp = SelectObject(bf->ellipse_dc, bf->ellipse_ret_bmp);
			DeleteObject(hBmp);
			if (bf->hfont != NULL) {
				SelectObject(bf->mdc, bf->ret_font);
				DeleteObject(bf->hfont);
			}
			DeleteDC(bf->mdc);
			DeleteDC(bf->ellipse_dc);
			DeleteObject(bf->back_brush);
			DeleteObject(bf->ton_circle_pen);

			mem_free(&bf->buf);
			mem_free(&bf->input_buf);
			undo_free(bf, 0);
			mem_free(&bf->undo);
			mem_free(&bf);
		}
		if (GetFocus() == hWnd) {
			DestroyCaret();
		}
		return DefWindowProc(hWnd, msg, wParam, lParam);

	case WM_SETFOCUS:
		if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL) {
			break;
		}
		CreateCaret(hWnd, NULL, CARET_WIDTH, bf->font_height);
		InvalidateRect(hWnd, NULL, FALSE);
		ShowCaret(hWnd);
		break;

	case WM_KILLFOCUS:
		HideCaret(hWnd);
		DestroyCaret();
		break;

	case WM_SIZE:
		if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL) {
			break;
		}
		if (wParam == SIZE_MINIMIZED) {
			break;
		}
		GetClientRect(hWnd, &rect);

		hdc = GetDC(hWnd);
		hBmp = SelectObject(bf->mdc, bf->ret_bmp);
		DeleteObject(hBmp);
		hBmp = CreateCompatibleBitmap(hdc, rect.right, rect.bottom);
		bf->ret_bmp = SelectObject(bf->mdc, hBmp);

		hBmp = SelectObject(bf->ellipse_dc, bf->ellipse_ret_bmp);
		DeleteObject(hBmp);
		hBmp = CreateCompatibleBitmap(hdc, rect.right * 2, rect.bottom * 2);
		bf->ellipse_ret_bmp = SelectObject(bf->ellipse_dc, hBmp);
		ReleaseDC(hWnd, hdc);

		InvalidateRect(hWnd, NULL, FALSE);
		break;

	case WM_GETDLGCODE:
		return DLGC_WANTALLKEYS;

	case WM_SYSKEYDOWN:
		if (wParam == VK_MENU) {
			SendMessage(GetParent(hWnd), WM_WINDOW_SET_KEY, 0, 0);
		}
		return DefWindowProc(hWnd, msg, wParam, lParam);

	case WM_KEYDOWN:
		if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL) {
			break;
		}
		if (wParam == VK_SHIFT || wParam == VK_CONTROL || wParam == VK_MENU) {
			SendMessage(GetParent(hWnd), WM_WINDOW_SET_KEY, 0, 0);
		}
		switch (wParam) {
		case VK_INSERT:
			if (GetKeyState(VK_CONTROL) < 0) {
				// コピー
				SendMessage(hWnd, WM_COPY, 0, 0);
			} else if (GetKeyState(VK_SHIFT) < 0) {
				// 貼り付け
				SendMessage(hWnd, WM_PASTE, 0, 0);
			} else {
				// 入力モード切替
				bf->insert_mode = !bf->insert_mode;

				DestroyCaret();
				CreateCaret(hWnd, NULL, CARET_WIDTH, bf->font_height);
				ShowCaret(hWnd);
				SendMessage(hWnd, WM_REFRESH, 0, 0);
			}
			break;

		case VK_DELETE:
			if (bf->lock == TRUE) {
				MessageBeep(0xFFFFFFFF);
				break;
			}
			if (GetKeyState(VK_SHIFT) < 0) {
				// 切り取り
				SendMessage(hWnd, WM_CUT, 0, 0);
			} else {
				// 削除
				string_delete_char(hWnd, bf, bf->cp);
			}
			break;

		case VK_BACK:
			if (bf->lock == TRUE) {
				MessageBeep(0xFFFFFFFF);
				break;
			}
			if (bf->cp == bf->sp) {
				if (bf->cp <= 0) {
					break;
				}
				caret_move(hWnd, bf, VK_LEFT);
			}
			string_delete_char(hWnd, bf, bf->cp);
			break;

		case 'A':
			if (GetKeyState(VK_CONTROL) < 0) {
				// 全て選択
				string_flush(bf, TRUE);
				bf->sp = 0;
				bf->cp = bf->buf_len;
				SendMessage(hWnd, WM_REFRESH, 0, 0);
			}
			break;

		case 'C':
			if (GetKeyState(VK_CONTROL) < 0) {
				SendMessage(hWnd, WM_COPY, 0, 0);
			}
			break;

		case 'X':
			if (GetKeyState(VK_CONTROL) < 0) {
				SendMessage(hWnd, WM_CUT, 0, 0);
			}
			break;

		case 'V':
			if (GetKeyState(VK_CONTROL) < 0) {
				SendMessage(hWnd, WM_PASTE, 0, 0);
			}
			break;

		case 'Z':
			if (GetKeyState(VK_CONTROL) < 0 && GetKeyState(VK_SHIFT) < 0) {
				// やり直し
				SendMessage(hWnd, EM_REDO, 0, 0);
			} else if (GetKeyState(VK_CONTROL) < 0) {
				// 元に戻す
				SendMessage(hWnd, EM_UNDO, 0, 0);
			}
			break;

		case 'Y':
			if (GetKeyState(VK_CONTROL) < 0) {
				SendMessage(hWnd, EM_REDO, 0, 0);
			}
			break;

		case VK_HOME:
		case VK_END:
		case VK_LEFT:
		case VK_RIGHT:
			string_flush(bf, TRUE);
			caret_move(hWnd, bf, wParam);
			break;
		}
		break;

	case WM_CHAR:
		if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL || bf->lock == TRUE) {
			MessageBeep(0xFFFFFFFF);
			break;
		}
		if (GetKeyState(VK_CONTROL) < 0) {
			break;
		}
		switch (wParam) {
		case VK_BACK:
			break;

		case VK_ESCAPE:
			break;

		default:
			in[0] = (TCHAR)wParam;
			in[1] = TEXT('\0');
			string_insert(hWnd, bf, in, bf->insert_mode);
			break;
		}
		break;

	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
		if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL) {
			break;
		}
		cp = bf->cp;
		sp = bf->sp;
		SetCursor(LoadCursor(0, IDC_IBEAM));
		if (msg == WM_MOUSEMOVE) {
			if (!(wParam & MK_LBUTTON) || bf->mousedown == FALSE) {
				break;
			}
		} else if (msg == WM_LBUTTONDOWN) {
			string_flush(bf, TRUE);
			SetCapture(hWnd);
			bf->mousedown = TRUE;
		} else if (msg == WM_LBUTTONUP) {
			if (bf->mousedown == FALSE) {
				break;
			}
			ReleaseCapture();
			bf->mousedown = FALSE;
		}
		SetFocus(hWnd);

		GetClientRect(hWnd, &rect);
		bf->cp = caret_point_to_caret(bf, (short)LOWORD(lParam), rect.right);
		if (msg == WM_LBUTTONDOWN && GetKeyState(VK_SHIFT) >= 0) {
			bf->sp = bf->cp;
		}
		SendMessage(hWnd, WM_REFRESH, 0, 0);
		break;

	case WM_RBUTTONDOWN:
		if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL) {
			break;
		}
		string_flush(bf, TRUE);
		SetFocus(hWnd);
		GetClientRect(hWnd, &rect);
		cp = caret_point_to_caret(bf, (short)LOWORD(lParam), rect.right);
		if (!(bf->cp >= cp && bf->sp <= cp || bf->sp >= cp && bf->cp <= cp)) {
			bf->cp = bf->sp = cp;
			SendMessage(hWnd, WM_REFRESH, 0, 0);
		}
		break;

	case WM_RBUTTONUP:
		SendMessage(GetParent(hWnd), WM_CONTEXTMENU, (WPARAM)hWnd, lParam);
		break;

	case WM_LBUTTONDBLCLK:
		if (!(wParam & MK_LBUTTON)) {
			break;
		}
		if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL) {
			break;
		}
		string_flush(bf, TRUE);
		SetFocus(hWnd);
		bf->sp = 0;
		bf->cp = bf->buf_len;
		SendMessage(hWnd, WM_REFRESH, 0, 0);
		break;

	case WM_MOUSEWHEEL:
		SendMessage(GetParent(hWnd), WM_MOUSEWHEEL, wParam, lParam);
		return 0;

	case WM_PAINT:
		if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) != NULL) {
			PAINTSTRUCT ps;

			hdc = BeginPaint(hWnd, &ps);
			GetClientRect(hWnd, &rect);
			FillRect(bf->mdc, &rect, bf->back_brush);

			// 1行描画
			draw_line(bf->mdc, bf, rect.right, rect.bottom);
			if (GetFocus() == hWnd) {
				if (bf->insert_mode == FALSE) {
					caret_set_size(hWnd, bf);
				}
				len = caret_char_to_caret(bf->mdc, bf, bf->cp, rect.right);
				SetCaretPos(len, (rect.bottom - bf->font_height) / 2);
			}
			BitBlt(hdc, ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right, ps.rcPaint.bottom,
				bf->mdc, ps.rcPaint.left, ps.rcPaint.top, SRCCOPY);
			EndPaint(hWnd, &ps);
		}
		break;

	case WM_ERASEBKGND:
		return 1;

	case WM_SETTEXT:
		if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL || lParam == 0) {
			break;
		}
		return string_set(hWnd, bf, (TCHAR *)lParam, lstrlen((TCHAR *)lParam));

	case WM_GETTEXT:
		if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL || lParam == 0) {
			break;
		}
		string_flush(bf, TRUE);
		lstrcpyn((TCHAR *)lParam, bf->buf, wParam);
		return lstrlen((TCHAR *)lParam);

	case WM_GETTEXTLENGTH:
		if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL) {
			break;
		}
		return bf->view_len;

	case WM_SETFONT:
		{
			LOGFONT lf;

			if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL) {
				break;
			}
			if (bf->hfont != NULL) {
				SelectObject(bf->mdc, bf->ret_font);
				DeleteObject(bf->hfont);
			}
			if (GetObject((HGDIOBJ)wParam, sizeof(LOGFONT), &lf) == 0) {
				break;
			}
			bf->hfont = CreateFontIndirect((CONST LOGFONT *)&lf);
			bf->ret_font = SelectObject(bf->mdc, bf->hfont);

			SendMessage(hWnd, WM_REFLECT, 0, 0);
		}
		break;

	case WM_CLEAR:
		// 削除
		if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL || bf->lock == TRUE) {
			MessageBeep(0xFFFFFFFF);
			break;
		}
		if (bf->cp != bf->sp) {
			string_delete_char(hWnd, bf, bf->cp);
		}
		break;

	case WM_COPY:
		// コピー
		if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL) {
			break;
		}
		string_flush(bf, TRUE);
		if (bf->cp != bf->sp) {
			if (bf->sp > bf->cp) {
				string_to_clipboard(hWnd, index_to_char(bf, bf->cp), index_to_char(bf, bf->sp));
			} else {
				string_to_clipboard(hWnd, index_to_char(bf, bf->sp), index_to_char(bf, bf->cp));
			}
		}
		break;

	case WM_CUT:
		// 切り取り
		if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL || bf->lock == TRUE) {
			MessageBeep(0xFFFFFFFF);
			break;
		}
		SendMessage(hWnd, WM_COPY, 0, 0);
		SendMessage(hWnd, WM_CLEAR, 0, 0);
		break;

	case WM_PASTE:
		// 貼り付け
		if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL || bf->lock == TRUE) {
			MessageBeep(0xFFFFFFFF);
			break;
		}
#ifdef UNICODE
		if (IsClipboardFormatAvailable(CF_UNICODETEXT) == 0) {
#else
		if (IsClipboardFormatAvailable(CF_TEXT) == 0) {
#endif
			break;
		}
		if (OpenClipboard(hWnd) != 0) {
			HANDLE hclip;
			TCHAR *p;

#ifdef UNICODE
			hclip = GetClipboardData(CF_UNICODETEXT);
#else
			hclip = GetClipboardData(CF_TEXT);
#endif
			if ((p = GlobalLock(hclip)) != NULL) {
				string_insert(hWnd, bf, p, TRUE);
				GlobalUnlock(hclip);
			}
			CloseClipboard();
		}
		break;

	case WM_UNDO:
	case EM_UNDO:
		// 元に戻す
		if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL || bf->lock == TRUE) {
			MessageBeep(0xFFFFFFFF);
			break;
		}
		undo_exec(hWnd, bf);
		break;

	case EM_REDO:
		// やり直し
		if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL || bf->lock == TRUE) {
			MessageBeep(0xFFFFFFFF);
			break;
		}
		redo_exec(hWnd, bf);
		break;

	case EM_CANUNDO:
		if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL) {
			break;
		}
		return ((bf->undo_len > 0 || bf->ip != NULL || bf->dp != NULL) ? TRUE : FALSE);

	case EM_CANREDO:
		if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL) {
			break;
		}
		return (((bf->undo + bf->undo_len)->type != 0) ? TRUE : FALSE);

	case EM_GETMODIFY:
		if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL) {
			break;
		}
		return bf->modified;

	case EM_SETMODIFY:
		if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL) {
			break;
		}
		bf->modified = wParam;
		break;

	case EM_GETSEL:
		if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL || wParam == 0 || lParam == 0) {
			break;
		}
		*((LPDWORD)wParam) = ((bf->sp < bf->cp) ? bf->sp : bf->cp);
		*((LPDWORD)lParam) = ((bf->sp < bf->cp) ? bf->cp : bf->sp);
		return MAKELPARAM(*((LPDWORD)wParam), *((LPDWORD)lParam));

	case EM_SETSEL:
		if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL) {
			break;
		}
		string_flush(bf, TRUE);
		sp = ((DWORD)wParam < (DWORD)lParam) ? wParam : lParam;
		cp = ((DWORD)wParam < (DWORD)lParam) ? lParam : wParam;
		if (sp < 0) {
			sp = 0;
		}
		if (sp > bf->buf_len) {
			sp = bf->buf_len;
		}
		if (cp < 0 || cp > bf->buf_len) {
			cp = bf->buf_len;
		}
		bf->sp = sp; bf->cp = cp;
		SendMessage(hWnd, WM_REFRESH, 0, 0);
		break;

	case EM_LIMITTEXT:
		if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL) {
			break;
		}
		bf->limit_len = wParam;
		break;

	case WM_GETBUFFERINFO:
		if (lParam == 0) {
			break;
		}
		*((BUFFER **)lParam) = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA);
		break;

	case WM_REFLECT:
		if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) != NULL) {
			TEXTMETRIC tm;

			// フォント情報の取得
			GetTextMetrics(bf->mdc, &tm);
			bf->font_height = tm.tmHeight;
			bf->char_width = tm.tmAveCharWidth;
			bf->fixed = (tm.tmPitchAndFamily & TMPF_FIXED_PITCH) ? FALSE : TRUE;
			bf->bold = (tm.tmWeight < 700) ? FALSE : TRUE;
			if (GetFocus() == hWnd) {
				DestroyCaret();
				CreateCaret(hWnd, NULL, CARET_WIDTH, bf->font_height);
				ShowCaret(hWnd);
			}
			InvalidateRect(hWnd, NULL, FALSE);
		}
		break;

	case WM_GETLOCK:
		if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL) {
			break;
		}
		return bf->lock;

	case WM_SETLOCK:
		if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL) {
			break;
		}
		bf->lock = wParam;
		break;

	case WM_EDIT_INIT_COLOR:
		if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL) {
			break;
		}
		DeleteObject(bf->back_brush);
		DeleteObject(bf->ton_circle_pen);
		bf->back_brush = CreateSolidBrush(op.ci.input_background);
		bf->ton_circle_pen = CreatePen(PS_SOLID, bf->pen_size, op.ci.ton_circle);
		SendMessage(hWnd, WM_REFRESH, 0, 0);
		break;

	case WM_REFRESH:
		if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL) {
			break;
		}
		if (bf->refresh_mode == FALSE) {
			break;
		}
		InvalidateRect(hWnd, NULL, FALSE);
		break;

	case WM_SET_REFRESH_MODE:
		if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL) {
			break;
		}
		bf->refresh_mode = wParam;
		break;

	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

/*
 * nedit_regist - クラスの登録
 */
BOOL nedit_regist(const HINSTANCE hInstance)
{
	WNDCLASS wc;

	wc.style = CS_DBLCLKS;
	wc.lpfnWndProc = (WNDPROC)nedit_proc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(0, IDC_IBEAM);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = 0;
	wc.lpszClassName = NEDIT_WND_CLASS;
	return RegisterClass(&wc);
}

/*
 * nedit_create - ウィンドウの作成
 */
HWND nedit_create(const HINSTANCE hInstance, const HWND pWnd, int id)
{
	HWND hWnd;

	// ウィンドウの作成
	hWnd = CreateWindowEx(0, NEDIT_WND_CLASS, NULL,
		WS_TABSTOP | WS_CHILD,
		0, 0, 0, 0, pWnd, (HMENU)id, hInstance, NULL);
	return hWnd;
}
/* End of source */
