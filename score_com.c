/*
 * n01
 *
 * score_com.c
 *
 * Copyright (C) 1996-2009 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

/* Include Files */
#define _INC_OLE
#include <windows.h>
#undef  _INC_OLE
#include <tchar.h>

#include "General.h"
#include "font.h"
#include "Message.h"

#include "resource.h"

/* Define */
#define MAX_LEVEL					13
#define MAX_LEFT_INDEX				1000

#define SEGMENT_SINGLE				1
#define SEGMENT_DOUBLE				2
#define SEGMENT_TRIPLE				3

#define ID_TIMER_COM				1

/* Global Variables */
extern OPTION_INFO op;
extern SCORE_INFO si;

typedef struct _COM_SCORE_INFO {
	int number;
	int segment;
} COM_SCORE_INFO;


/* Local Function Prototypes */
static int com_get_throw(int lv, const COM_SCORE_INFO *csi, COM_SCORE_INFO *ret);
static void string_to_score_info(TCHAR *buf, COM_SCORE_INFO *ret);
static void score_info_to_string(const COM_SCORE_INFO *csi, TCHAR *ret);
static int com_get_score(const int player, const int throw_index, COM_SCORE_INFO *csi);
static BOOL CALLBACK score_com_proc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

/*
 * com_get_throw - レベルに応じた得点を取得
 *
 * Level- 設定darts数 (Darts, Score, First9, Check-Out)
 * Lv1  - 60 darts (65.66, 7.66,  13.13, 9.78)
 * Lv2  - 40 darts (42.19, 11.88, 16.33, 15.02)
 * Lv3  - 30 darts (32.99, 15.20, 19.52, 19.69)
 * Lv4  - 28 darts (28.86, 17.36, 21.37, 22.40)
 * Lv5  - 26 darts (26.50, 18.92, 22.18, 26.89)
 * Lv6  - 24 darts (24.66, 20.31, 23.58, 28.49)
 * Lv7  - 22 darts (22.53, 22.23, 25.82, 30.62)
 * Lv8  - 20 darts (20.42, 24.53, 28.48, 34.17)
 * Lv9  - 18 darts (18.44, 27.17, 30.91, 40.74)
 * Lv10 - 16 darts (16.26, 30.81, 34.80, 47.06)
 * Lv11 - 14 darts (14.43, 34.72, 38.75, 54.95)
 * Lv12 - 12 darts (12.12, 41.33, 46.35, 67.01)
 */
static int com_get_throw(int lv, const COM_SCORE_INFO *csi, COM_SCORE_INFO *ret)
{
	int board[21][2] = {
		{ 0,  0},	//0
		{20, 18},	//1
		{15, 17},	//2
		{17, 19},	//3
		{18, 13},	//4
		{12, 20},	//5
		{13, 10},	//6
		{19, 16},	//7
		{16, 11},	//8
		{14, 12},	//9
		{ 6, 15},	//10
		{ 8, 14},	//11
		{ 9,  5},	//12
		{ 4,  6},	//13
		{11,  9},	//14
		{10,  2},	//15
		{ 7,  8},	//16
		{ 2,  3},	//17
		{ 1,  4},	//18
		{ 3,  7},	//19
		{ 5,  1},	//20
	};
	// Level                      1   2   3   4   5   6   7   8   9   10  11  12
	int rate_triple[MAX_LEVEL] = {1,  5 , 10, 13, 15, 17, 20, 25, 30, 40, 50, 70, 100};
	int rate_double[MAX_LEVEL] = {10, 15, 20, 23, 25, 27, 30, 35, 40, 50, 60, 70, 100};
	int rate_single[MAX_LEVEL] = {20, 30, 40, 55, 60, 70, 80, 85, 90, 90, 90, 95, 100};
	int i, j;

	if (lv >= MAX_LEVEL) {
		lv = MAX_LEVEL - 1;
	}
	ret->number = 0;
	ret->segment = SEGMENT_SINGLE;

	// 番号決定
	i = csi->number;
	if (i == 25) {
		// bull
		if ((rand() % 100) + 1 > rate_double[lv]) {
			i = rand() % 21;
		}
		if (i == 25 || i == 0) {
			ret->number = 25;
			switch (csi->segment) {
			case SEGMENT_SINGLE:
				if ((rand() % 100) + 1 <= rate_double[lv]) {
					ret->segment = SEGMENT_SINGLE;
				} else {
					ret->segment = SEGMENT_DOUBLE;
				}
				break;

			case SEGMENT_DOUBLE:
				if ((rand() % 100) + 1 <= rate_triple[lv]) {
					ret->segment = SEGMENT_DOUBLE;
				} else {
					ret->segment = SEGMENT_SINGLE;
				}
				break;
			}
		} else {
			ret->number = i;
			ret->segment = SEGMENT_SINGLE;
		}
		return i;
	}

	// 領域決定
	j = rand() % 2;
	do {
		switch (csi->segment) {
		case SEGMENT_SINGLE:
			if ((rand() % 100) + 1 <= rate_single[lv]) {
				// シングル
				ret->number = i;
				ret->segment = SEGMENT_SINGLE;
			} else if ((rand() % 100) + 1 > rate_single[lv]) {
				if ((rand() % 100) + 1 <= rate_triple[lv]) {
					// トリプル
					ret->number = i;
					ret->segment = SEGMENT_TRIPLE;
				} else if ((rand() % 100) + 1 <= rate_double[lv]) {
					// ダブル
					ret->number = i;
					ret->segment = SEGMENT_DOUBLE;
				} else {
					i = board[i][j];
				}
			} else {
				i = board[i][j];
			}
			break;

		case SEGMENT_DOUBLE:
			if ((rand() % 100) + 1 <= rate_double[lv]) {
				// ダブル
				ret->number = i;
				ret->segment = SEGMENT_DOUBLE;
			} else if ((rand() % 100) + 1 <= rate_single[lv]) {
				if ((rand() % 100) + 1 <= 50) {
					// シングル
					ret->number = i;
					ret->segment = SEGMENT_SINGLE;
				} else {
					// アウトボード
					ret->number = 0;
					ret->segment = SEGMENT_SINGLE;
					return 0;
				}
			} else {
				i = board[i][j];
			}
			break;

		case SEGMENT_TRIPLE:
			if ((rand() % 100) + 1 <= rate_triple[lv]) {
				// トリプル
				ret->number = i;
				ret->segment = SEGMENT_TRIPLE;
			} else if ((rand() % 100) + 1 <= rate_single[lv]) {
				// シングル
				ret->number = i;
				ret->segment = SEGMENT_SINGLE;
			} else {
				i = board[i][j];
			}
			break;
		}
	} while (ret->number == 0);
	return i;
}

/*
 * string_to_score_info - スコア文字列をスコア情報に変換
 */
static void string_to_score_info(TCHAR *buf, COM_SCORE_INFO *ret)
{
	TCHAR *p;

	for (p = buf; *p != TEXT('\0') && (*p < TEXT('0') || *p > TEXT('9')); p++);
	switch (*buf) {
	case TEXT('T'):
		ret->number = _ttoi(p);
		ret->segment = SEGMENT_TRIPLE;
		break;
	case TEXT('D'):
		ret->number = _ttoi(p);
		ret->segment = SEGMENT_DOUBLE;
		break;
	case TEXT('S'):
	default:
		ret->number = _ttoi(p);
		ret->segment = SEGMENT_SINGLE;
		break;
	}
}

/*
 * score_info_to_string - スコア情報をスコア文字列に変換
 */
static void score_info_to_string(const COM_SCORE_INFO *csi, TCHAR *ret)
{
	if (csi->number == 0) {
		lstrcpy(ret, TEXT("0"));
		return;
	}
	switch (csi->segment)
	{
	case SEGMENT_TRIPLE:
		wsprintf(ret, TEXT("T%d"), csi->number);
		break;
	case SEGMENT_DOUBLE:
		wsprintf(ret, TEXT("D%d"), csi->number);
		break;
	case SEGMENT_SINGLE:
	default:
		wsprintf(ret, TEXT("S%d"), csi->number);
		break;
	}
}

/*
 * choice_arrange - アレンジの選択
 */
static int choice_arrange(const int player, const int throw_index, COM_SCORE_INFO *csi)
{
	int left_index[MAX_LEFT_INDEX];
	int left_index_count;
	int left;
	int left_flag;
	int i, j;

	// 相手の残り点数フラグ
	left_flag = (si.player[!player].left > 170) ? 2 : 1;

	// アレンジ抽出
	left = si.player[player].left;
	for (i = 0; i < throw_index; i++) {
		left -= (csi[i].number * csi[i].segment);
	}
	left_index_count = 0;
	for (i = 0; i < op.com_arrange_info_count && left_index_count < MAX_LEFT_INDEX; i++) {
		if (op.com_arrange_info[i].left == left &&
			(_ttoi(op.com_arrange_info[i].throw_list[4]) == 0 || _ttoi(op.com_arrange_info[i].throw_list[4]) == left_flag)) {
			if (_ttoi(op.com_arrange_info[i].throw_list[3]) <= 1) {
				left_index[left_index_count] = i;
				left_index_count++;
			} else {
				// 確立変動のための繰り返し
				for (j = 0; j < _ttoi(op.com_arrange_info[i].throw_list[3]) && left_index_count < MAX_LEFT_INDEX; j++) {
					left_index[left_index_count] = i;
					left_index_count++;
				}
			}
		}
	}
	if (left_index_count <= 0) {
		// アレンジ無し
		return -1;
	}
	return left_index[(rand() % left_index_count)];
}

/*
 * com_get_score - COMのスコアを取得
 */
static int com_get_score(const int player, const int throw_index, COM_SCORE_INFO *csi)
{
	COM_SCORE_INFO sel_csi;
	int left;
	int i;

	i = choice_arrange(player, throw_index, csi);
	if (i >= 0) {
		// アレンジ
		string_to_score_info(op.com_arrange_info[i].throw_list[throw_index], &sel_csi);
		if (sel_csi.segment == SEGMENT_DOUBLE) {
			left = si.player[player].left;
			for (i = 0; i < throw_index; i++) {
				left -= (csi[i].number * csi[i].segment);
			}
			if (left == 50 || (left <= 40 && left == (left / 2) * 2)) {
				// チェックアウト本数追加
				si.tmp_check_out[player][si.leg[si.current_leg].current_round]++;
			}
		}
		com_get_throw(si.player[player].level, &sel_csi, &csi[throw_index]);
	} else {
		// デフォルトの数字
		sel_csi.number = op.com_default_number;
		if ((sel_csi.number < 1 || sel_csi.number > 20) && sel_csi.number != 25) {
			sel_csi.number = 20;
		}
		// デフォルトのセグメント
		sel_csi.segment = op.com_default_segment;
		if (sel_csi.number == 25 && (sel_csi.segment < SEGMENT_SINGLE || sel_csi.segment > SEGMENT_DOUBLE)) {
			sel_csi.segment = SEGMENT_DOUBLE;
		} else if (sel_csi.segment < SEGMENT_SINGLE || sel_csi.segment > SEGMENT_TRIPLE) {
			sel_csi.segment = SEGMENT_TRIPLE;
		}
		com_get_throw(si.player[player].level, &sel_csi, &csi[throw_index]);
	}
	return (csi[throw_index].number * csi[throw_index].segment);
}

/*
 * score_com_proc - ウィンドウプロシージャ
 */
static BOOL CALLBACK score_com_proc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static COM_SCORE_INFO csi[3];
	static int player;
	static int throw_index;
	static HFONT hfont;
	RECT parent_rect, rect;
	TCHAR buf[BUF_SIZE];
	int score, i;

	switch (msg) {
	case WM_INITDIALOG:
		// 乱数の初期化
		srand((unsigned)GetTickCount());

		player = lParam;
		throw_index = 0;

		hfont = font_create(op.font_name, 22, 0, FALSE, FALSE);
		SendDlgItemMessage(hDlg, IDC_STATIC_THROW_1, WM_SETFONT, (WPARAM)hfont, 0);
		SendDlgItemMessage(hDlg, IDC_STATIC_THROW_2, WM_SETFONT, (WPARAM)hfont, 0);
		SendDlgItemMessage(hDlg, IDC_STATIC_THROW_3, WM_SETFONT, (WPARAM)hfont, 0);

		// センタリング
		GetWindowRect(GetParent(hDlg), &parent_rect);
		GetWindowRect(hDlg, &rect);
		SetWindowPos(hDlg, 0,
			parent_rect.left + ((parent_rect.right - parent_rect.left) - (rect.right - rect.left)) / 2,
			parent_rect.top + ((parent_rect.bottom - parent_rect.top) - (rect.bottom - rect.top)) / 2,
			rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER);

		SetTimer(hDlg, ID_TIMER_COM, op.com_timer, NULL);
		break;

	case WM_TIMER:
		switch (wParam) {
		case ID_TIMER_COM:
			KillTimer(hDlg, ID_TIMER_COM);

			score = 0;
			for (i = 0; i < throw_index; i++) {
				score += (csi[i].number * csi[i].segment);
			}
			if (si.player[player].left - score == 0 && throw_index > 0 && csi[throw_index - 1].segment == SEGMENT_DOUBLE) {
				// クリア
				si.player[player].com_score = (TYPE_SCORE)(throw_index * -1);
				SendMessage(hDlg, WM_CLOSE, 0, 0);
				break;
			}
			if (si.player[player].left - score <= 1) {
				// バースト
				si.player[player].com_score = 0;
				SendMessage(hDlg, WM_CLOSE, 0, 0);
				break;
			}
			if (throw_index >= 3) {
				// スコア設定
				si.player[player].com_score = (TYPE_SCORE)score;
				SendMessage(hDlg, WM_CLOSE, 0, 0);
				break;
			}
			com_get_score(player, throw_index, csi);
			score_info_to_string(&csi[throw_index], buf);
			SetWindowText(GetDlgItem(hDlg, IDC_STATIC_THROW_1 + throw_index), buf);
			throw_index++;

			SetTimer(hDlg, ID_TIMER_COM, op.com_timer, NULL);
			break;
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDCANCEL:
			if (MessageBox(hDlg, message_get_res(IDS_STRING_GAME_STOP), APP_NAME, MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2) == IDYES) {
				KillTimer(hDlg, ID_TIMER_COM);
				si.player[player].com_score = -5;
				SendMessage(hDlg, WM_CLOSE, 0, 0);
			}
			break;
		}
		break;

	case WM_CLOSE:
		DeleteObject(hfont);
		EndDialog(hDlg, FALSE);
		break;

	case WM_GETDLGCODE:
		return DLGC_WANTALLKEYS;

	default:
		return FALSE;
	}
	return TRUE;
}

/*
 * show_score_com - COM
 */
int show_score_com(const HINSTANCE hInst, const HWND hWnd, int player)
{
	return DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_COM), hWnd, score_com_proc, (LPARAM)player);
}
/* End of source */
