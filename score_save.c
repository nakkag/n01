/*
 * n01
 *
 * score_save.c
 *
 * Copyright (C) 1996-2006 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

/* Include Files */
#define _INC_OLE
#include <windows.h>
#undef  _INC_OLE
#include <tchar.h>
#include <stdio.h>

#include "general.h"
#include "Memory.h"
#include "String.h"
#include "Message.h"
#include "file.h"

#include "resource.h"

/* Define */
#define	STR_SKIP(p)			for (; *p != TEXT('\0'); p++)

/* Global Variables */
extern OPTION_INFO op;
extern TCHAR work_path[MAX_PATH];

/* Local Function Prototypes */
static TCHAR *conv_str(const TCHAR *buf);
static int create_line_length(const TCHAR *title, const int *num1, const int *num2);
static TCHAR *create_line(TCHAR *ret, const TCHAR *title, const int *num1, const int *num2);

/*
 * conv_str - CSV出力用文字列の作成
 */
static TCHAR *conv_str(const TCHAR *buf)
{
	static TCHAR ret[BUF_SIZE];
	TCHAR *r;
	int i;

	for (i = 0; *(buf + i) != TEXT('\0') && *(buf + i) != TEXT(',') && *(buf + i) != TEXT('"'); i++);
	if (*(buf + i) == TEXT('\0')) {
		return (TCHAR *)buf;
	}
	r = ret;
	*(r++) = TEXT('"');
	for (i = 0; *(buf + i) != TEXT('\0'); i++) {
		if (*(buf + i) == TEXT('"')) {
			*(r++) = TEXT('"');
		}
		*(r++) = *(buf + i);
	}
	*(r++) = TEXT('"');
	*r = TEXT('\0');
	return ret;
}

/*
 * create_line_length - CSV1行の長さ取得
 */
static int create_line_length(const TCHAR *title, const int *num1, const int *num2)
{
	TCHAR buf[BUF_SIZE];
	int ret = 0;
	int title_len;

	title_len = lstrlen(title);

	ret++;		//,
	// タイトル1
	ret += title_len;
	ret++;		//,
	if (num1 != NULL) {
		// 数値1
		_itot(*num1, buf, 10);
		ret += lstrlen(buf);
	}
	ret++;		//,
	// タイトル2
	ret += title_len;
	ret++;		//,
	if (num2 != NULL) {
		// 数値2
		_itot(*num2, buf, 10);
		ret += lstrlen(buf);
	}
	ret += 2;	//CRLF
	return ret;
}

/*
 * create_line - CSV1行の作成
 */
static TCHAR *create_line(TCHAR *ret, const TCHAR *title, const int *num1, const int *num2)
{
	TCHAR *p = ret;
	int title_len;

	title_len = lstrlen(title);

	*(p++) = TEXT(',');
	// タイトル1
	lstrcpy(p, title);
	p += title_len;
	*(p++) = TEXT(',');
	if (num1 != NULL) {
		// 数値1
		_itot(*num1, p, 10);
		STR_SKIP(p);
	}
	*(p++) = TEXT(',');
	// タイトル2
	lstrcpy(p, title);
	p += title_len;
	*(p++) = TEXT(',');
	if (num2 != NULL) {
		// 数値2
		_itot(*num2, p, 10);
		STR_SKIP(p);
	}
	*(p++) = TEXT('\r');
	*(p++) = TEXT('\n');
	*p = TEXT('\0');
	return p;
}

/*
 * get_score_string_length - スコア文字列の長さ取得
 */
int get_score_string_length(const SCORE_INFO *si)
{
	TCHAR buf[BUF_SIZE];
	int left_over[2];
	int i, j, t;
	int ret = 0;
	float f1, f2;

	// 日付
	GetDateFormat(0, 0, &si->start_time, NULL, buf, BUF_SIZE - 1);
	ret += lstrlen(conv_str(buf));
	ret++;		// 
	GetTimeFormat(0, 0, &si->start_time, NULL, buf, BUF_SIZE - 1);
	ret += lstrlen(conv_str(buf));
	ret++;		//,

	// 名前
	if (si->player[0].com == TRUE) {
		wsprintf(buf, message_get_res(IDS_STRING_COM), si->player[0].level + 1);
		ret += lstrlen(buf);
	} else {
		ret += lstrlen(conv_str(si->player[0].name));
	}
	ret++;		//,
	ret++;		//,
	if (si->player[1].com == TRUE) {
		wsprintf(buf, message_get_res(IDS_STRING_COM), si->player[1].level + 1);
		ret += lstrlen(buf);
	} else {
		ret += lstrlen(conv_str(si->player[1].name));
	}
	ret++;		//,
	ret += 2;	//CRLF

	// スタッツ
	// Totals
	ret += lstrlen(message_get_res(IDS_STRING_P_TOTAL));
	i = (si->player[0].legs > si->player[1].legs) ? 1 : 0;
	j = (si->player[0].legs < si->player[1].legs) ? 1 : 0;
	ret += create_line_length(message_get_res(IDS_STRING_P_SETS), &i, &j);
	ret += create_line_length(message_get_res(IDS_STRING_P_LEGS), &si->player[0].legs, &si->player[1].legs);
	ret += create_line_length(message_get_res(IDS_STRING_P_TONS), &si->player[0].stat.ton_count, &si->player[1].stat.ton_count);
	ret += create_line_length(message_get_res(IDS_STRING_P_100), &si->player[0].stat.ton00_count, &si->player[1].stat.ton00_count);
	ret += create_line_length(message_get_res(IDS_STRING_P_140), &si->player[0].stat.ton40_count, &si->player[1].stat.ton40_count);
	ret += create_line_length(message_get_res(IDS_STRING_P_180S), &si->player[0].stat.ton80_count, &si->player[1].stat.ton80_count);
	ret += create_line_length(message_get_res(IDS_STRING_P_HIGHOFF),
		(si->player[0].stat.high_off > 0) ? (int *)&si->player[0].stat.high_off : NULL,
		(si->player[1].stat.high_off > 0) ? (int *)&si->player[1].stat.high_off : NULL);
	ret += create_line_length(message_get_res(IDS_STRING_P_SHORT),
		(si->player[0].stat.short_game > 0) ? &si->player[0].stat.short_game : NULL,
		(si->player[1].stat.short_game > 0) ? &si->player[1].stat.short_game : NULL);
	ret += create_line_length(message_get_res(IDS_STRING_P_LONG),
		(si->player[0].stat.long_game > 0) ? &si->player[0].stat.long_game : NULL,
		(si->player[1].stat.long_game > 0) ? &si->player[1].stat.long_game : NULL);

	// Averages
	ret += lstrlen(message_get_res(IDS_STRING_P_AVERAGE));
	f1 = (si->player[0].stat.win_count != 0) ? (float)(si->player[0].stat.win_darts) / (float)(si->player[0].stat.win_count) : 0;
	f2 = (si->player[1].stat.win_count != 0) ? (float)(si->player[1].stat.win_darts) / (float)(si->player[1].stat.win_count) : 0;
	_stprintf(buf, TEXT(",%s,%.2f,%s,%.2f\r\n"), message_get_res(IDS_STRING_P_AVG_DARTS), f1, message_get_res(IDS_STRING_P_AVG_DARTS), f2);
	ret += lstrlen(buf);

	f1 = (si->player[0].stat.all_darts != 0) ? (float)(si->player[0].stat.all_score) / (float)(si->player[0].stat.all_darts) : 0;
	f2 = (si->player[1].stat.all_darts != 0) ? (float)(si->player[1].stat.all_score) / (float)(si->player[1].stat.all_darts) : 0;
	_stprintf(buf, TEXT(",%s,%.2f,%s,%.2f\r\n"), message_get_res(IDS_STRING_P_AVG_SCORE), f1, message_get_res(IDS_STRING_P_AVG_SCORE), f2);
	ret += lstrlen(buf);

	f1 = (si->player[0].stat.first9_darts != 0) ? (float)(si->player[0].stat.first9_score) / (float)(si->player[0].stat.first9_darts) : 0;
	f2 = (si->player[1].stat.first9_darts != 0) ? (float)(si->player[1].stat.first9_score) / (float)(si->player[1].stat.first9_darts) : 0;
	_stprintf(buf, TEXT(",%s,%.2f,%s,%.2f\r\n"), message_get_res(IDS_STRING_P_AVG_FIRST9), f1, message_get_res(IDS_STRING_P_AVG_FIRST9), f2);
	ret += lstrlen(buf);

	f1 = (si->player[0].stat.check_out_aim != 0) ? ((float)(si->player[0].stat.check_out) / (float)(si->player[0].stat.check_out_aim)) * 100 : 0;
	f2 = (si->player[1].stat.check_out_aim != 0) ? ((float)(si->player[1].stat.check_out) / (float)(si->player[1].stat.check_out_aim)) * 100 : 0;
	_stprintf(buf, TEXT(",%s,%.2f,%s,%.2f\r\n"), message_get_res(IDS_STRING_P_AVG_CHECK_OUT_LARGE), f1, message_get_res(IDS_STRING_P_AVG_CHECK_OUT_LARGE), f2);
	ret += lstrlen(buf);

	f1 = (si->player[0].stat.all_keep_legs != 0) ? ((float)(si->player[0].stat.win_keep_legs) / (float)(si->player[0].stat.all_keep_legs)) * 100 : 0;
	f2 = (si->player[1].stat.all_keep_legs != 0) ? ((float)(si->player[1].stat.win_keep_legs) / (float)(si->player[1].stat.all_keep_legs)) * 100 : 0;
	_stprintf(buf, TEXT(",%s,%.2f,%s,%.2f\r\n"), message_get_res(IDS_STRING_P_AVG_KEEP), f1, message_get_res(IDS_STRING_P_AVG_KEEP), f2);
	ret += lstrlen(buf);

	f1 = (si->player[0].stat.all_break_legs != 0) ? ((float)(si->player[0].stat.win_break_legs) / (float)(si->player[0].stat.all_break_legs)) * 100 : 0;
	f2 = (si->player[1].stat.all_break_legs != 0) ? ((float)(si->player[1].stat.win_break_legs) / (float)(si->player[1].stat.all_break_legs)) * 100 : 0;
	_stprintf(buf, TEXT(",%s,%.2f,%s,%.2f\r\n"), message_get_res(IDS_STRING_P_AVG_BREAK), f1, message_get_res(IDS_STRING_P_AVG_BREAK), f2);
	ret += lstrlen(buf);

	// Finish stats
	ret += lstrlen(message_get_res(IDS_STRING_P_FINISH_STATS));
	ret += create_line_length(message_get_res(IDS_STRING_P_2_80_SUCCESS), &si->player[0].stat.success_2_80, &si->player[1].stat.success_2_80);
	ret += create_line_length(message_get_res(IDS_STRING_P_2_80_FAILURE), &si->player[0].stat.failure_2_80, &si->player[1].stat.failure_2_80);
	ret += create_line_length(message_get_res(IDS_STRING_P_81_130_SUCCESS), &si->player[0].stat.success_81_130, &si->player[1].stat.success_81_130);
	ret += create_line_length(message_get_res(IDS_STRING_P_81_130_FAILURE), &si->player[0].stat.failure_81_130, &si->player[1].stat.failure_81_130);
	ret += create_line_length(message_get_res(IDS_STRING_P_131_SUCCESS), &si->player[0].stat.success_131, &si->player[1].stat.success_131);
	ret += create_line_length(message_get_res(IDS_STRING_P_131_FAILURE), &si->player[0].stat.failure_131, &si->player[1].stat.failure_131);

	// Counts
	ret += lstrlen(message_get_res(IDS_STRING_P_COUNTS));
	ret += create_line_length(message_get_res(IDS_STRING_P_LEGS), &si->current_leg, &si->current_leg);
	ret += create_line_length(message_get_res(IDS_STRING_P_WIN_DARTS), &si->player[0].stat.win_darts, &si->player[1].stat.win_darts);
	ret += create_line_length(message_get_res(IDS_STRING_P_AVG_SCORE), &si->player[0].stat.all_score, &si->player[1].stat.all_score);
	ret += create_line_length(message_get_res(IDS_STRING_P_AVG_DARTS), &si->player[0].stat.all_darts, &si->player[1].stat.all_darts);
	ret += create_line_length(message_get_res(IDS_STRING_P_FIRST9_SCORE), &si->player[0].stat.first9_score, &si->player[1].stat.first9_score);
	ret += create_line_length(message_get_res(IDS_STRING_P_FIRST9_DARTS), &si->player[0].stat.first9_darts, &si->player[1].stat.first9_darts);
	ret += create_line_length(message_get_res(IDS_STRING_P_AVG_CHECK_OUT_LARGE), &si->player[0].stat.check_out, &si->player[1].stat.check_out);
	ret += create_line_length(message_get_res(IDS_STRING_P_DOUBLE), &si->player[0].stat.check_out_aim, &si->player[1].stat.check_out_aim);
	ret += create_line_length(message_get_res(IDS_STRING_P_AVG_KEEP_WON), &si->player[0].stat.win_keep_legs, &si->player[1].stat.win_keep_legs);
	ret += create_line_length(message_get_res(IDS_STRING_P_AVG_KEEP_ALL), &si->player[0].stat.all_keep_legs, &si->player[1].stat.all_keep_legs);
	ret += create_line_length(message_get_res(IDS_STRING_P_AVG_BREAK_WON), &si->player[0].stat.win_break_legs, &si->player[1].stat.win_break_legs);
	ret += create_line_length(message_get_res(IDS_STRING_P_AVG_BREAK_ALL), &si->player[0].stat.all_break_legs, &si->player[1].stat.all_break_legs);

	// Leg
	for (i = 0; i < si->current_leg; i++) {
		ret += 2;	//CRLF

		// ヘッダ Round,Scored,To Go,Scored,To Go
		ret += lstrlen(message_get_res(IDS_STRING_ROUND));
		ret++;		//,
		ret += lstrlen(message_get_res(IDS_STRING_SCORED));
		ret++;		//,
		ret += lstrlen(message_get_res(IDS_STRING_TO_GO));
		ret++;		//,
		ret += lstrlen(message_get_res(IDS_STRING_SCORED));
		ret++;		//,
		ret += lstrlen(message_get_res(IDS_STRING_TO_GO));
		ret += 2;	//CRLF

		// 開始スコア
		ret++;		//,
		if (si->leg[i].first == 0) {
			ret += lstrlen(message_get_res(IDS_STRING_FIRST_MARK));
		}
		ret++;		//,
		_itot(si->player[0].start_score, buf, 10);
		ret += lstrlen(buf);
		ret++;		//,
		if (si->leg[i].first == 1) {
			ret += lstrlen(message_get_res(IDS_STRING_FIRST_MARK));
		}
		ret++;		//,
		_itot(si->player[1].start_score, buf, 10);
		ret += lstrlen(buf);
		ret += 2;	//CRLF

		// スコア
		left_over[0] = si->player[0].start_score;
		left_over[1] = si->player[1].start_score;
		for (j = 0; j < si->leg[i].max_round; j++) {
			_itot(j + 1, buf, 10);
			ret += lstrlen(buf);
			for (t = 0; t < 2; t++) {
				if (j < si->leg[i].current_round || (j == si->leg[i].current_round && si->leg[i].first == t && si->leg[i].current_player != t)) {
					ret++;		//,
					_itot(si->leg[i].score[t][j], buf, 10);
					ret += lstrlen(buf);
					ret++;		//,
					if (si->leg[i].score[t][j] >= 0) {
						left_over[t] -= si->leg[i].score[t][j];
						_itot(left_over[t], buf, 10);
						ret += lstrlen(buf);
					}
				} else {
					ret++;		//,
					ret++;		//,
				}
			}
			ret += 2;	//CRLF
		}
		// Darts数
		ret += lstrlen(message_get_res(IDS_STRING_DARTS));
		ret++;		//,
		if (si->leg[i].winner == 1) {
			ret++;		//,
			ret++;		//,
		}
		_itot(si->leg[i].darts, buf, 10);
		ret += lstrlen(buf);
		if (si->leg[i].winner == 0) {
			ret++;		//,
			ret++;		//,
		}
		ret++;		//,
		ret += 2;	//CRLF
	}
	return ret;
}

/*
 * get_score_string - スコア文字列の作成
 */
BOOL get_score_string(const SCORE_INFO *si, TCHAR *ret)
{
	TCHAR *p = ret;
	TCHAR buf[BUF_SIZE];
	int left_over[2];
	int i, j, t;
	float f1, f2;

	// 日付
	GetDateFormat(0, 0, &si->start_time, NULL, buf, BUF_SIZE - 1);
	lstrcpy(p, conv_str(buf));
	STR_SKIP(p);
	*(p++) = TEXT(' ');
	GetTimeFormat(0, 0, &si->start_time, NULL, buf, BUF_SIZE - 1);
	lstrcpy(p, conv_str(buf));
	STR_SKIP(p);
	*(p++) = TEXT(',');

	// 名前
	if (si->player[0].com == TRUE) {
		wsprintf(p, message_get_res(IDS_STRING_COM), si->player[0].level + 1);
	} else {
		lstrcpy(p, conv_str(si->player[0].name));
	}
	STR_SKIP(p);
	*(p++) = TEXT(',');
	*(p++) = TEXT(',');
	if (si->player[1].com == TRUE) {
		wsprintf(p, message_get_res(IDS_STRING_COM), si->player[1].level + 1);
	} else {
		lstrcpy(p, conv_str(si->player[1].name));
	}
	STR_SKIP(p);
	*(p++) = TEXT(',');
	*(p++) = TEXT('\r');
	*(p++) = TEXT('\n');

	// スタッツ
	// Totals
	message_copy_res(IDS_STRING_P_TOTAL, p);
	STR_SKIP(p);
	i = (si->player[0].legs > si->player[1].legs) ? 1 : 0;
	j = (si->player[0].legs < si->player[1].legs) ? 1 : 0;
	p = create_line(p, message_get_res(IDS_STRING_P_SETS), &i, &j);
	p = create_line(p, message_get_res(IDS_STRING_P_LEGS), &si->player[0].legs, &si->player[1].legs);
	p = create_line(p, message_get_res(IDS_STRING_P_TONS), &si->player[0].stat.ton_count, &si->player[1].stat.ton_count);
	p = create_line(p, message_get_res(IDS_STRING_P_100), &si->player[0].stat.ton00_count, &si->player[1].stat.ton00_count);
	p = create_line(p, message_get_res(IDS_STRING_P_140), &si->player[0].stat.ton40_count, &si->player[1].stat.ton40_count);
	p = create_line(p, message_get_res(IDS_STRING_P_180S), &si->player[0].stat.ton80_count, &si->player[1].stat.ton80_count);
	p = create_line(p, message_get_res(IDS_STRING_P_HIGHOFF),
		(si->player[0].stat.high_off > 0) ? (int *)&si->player[0].stat.high_off : NULL,
		(si->player[1].stat.high_off > 0) ? (int *)&si->player[1].stat.high_off : NULL);
	p = create_line(p, message_get_res(IDS_STRING_P_SHORT),
		(si->player[0].stat.short_game > 0) ? &si->player[0].stat.short_game : NULL,
		(si->player[1].stat.short_game > 0) ? &si->player[1].stat.short_game : NULL);
	p = create_line(p, message_get_res(IDS_STRING_P_LONG),
		(si->player[0].stat.long_game > 0) ? &si->player[0].stat.long_game : NULL,
		(si->player[1].stat.long_game > 0) ? &si->player[1].stat.long_game : NULL);

	// Averages
	message_copy_res(IDS_STRING_P_AVERAGE, p);
	STR_SKIP(p);
	f1 = (si->player[0].stat.win_count != 0) ? (float)(si->player[0].stat.win_darts) / (float)(si->player[0].stat.win_count) : 0;
	f2 = (si->player[1].stat.win_count != 0) ? (float)(si->player[1].stat.win_darts) / (float)(si->player[1].stat.win_count) : 0;
	_stprintf(p, TEXT(",%s,%.2f,%s,%.2f\r\n"), message_get_res(IDS_STRING_P_AVG_DARTS), f1, message_get_res(IDS_STRING_P_AVG_DARTS), f2);
	STR_SKIP(p);

	f1 = (si->player[0].stat.all_darts != 0) ? (float)(si->player[0].stat.all_score) / (float)(si->player[0].stat.all_darts) : 0;
	f2 = (si->player[1].stat.all_darts != 0) ? (float)(si->player[1].stat.all_score) / (float)(si->player[1].stat.all_darts) : 0;
	_stprintf(p, TEXT(",%s,%.2f,%s,%.2f\r\n"), message_get_res(IDS_STRING_P_AVG_SCORE), f1, message_get_res(IDS_STRING_P_AVG_SCORE), f2);
	STR_SKIP(p);

	f1 = (si->player[0].stat.first9_darts != 0) ? (float)(si->player[0].stat.first9_score) / (float)(si->player[0].stat.first9_darts) : 0;
	f2 = (si->player[1].stat.first9_darts != 0) ? (float)(si->player[1].stat.first9_score) / (float)(si->player[1].stat.first9_darts) : 0;
	_stprintf(p, TEXT(",%s,%.2f,%s,%.2f\r\n"), message_get_res(IDS_STRING_P_AVG_FIRST9), f1, message_get_res(IDS_STRING_P_AVG_FIRST9), f2);
	STR_SKIP(p);

	f1 = (si->player[0].stat.check_out_aim != 0) ? ((float)(si->player[0].stat.check_out) / (float)(si->player[0].stat.check_out_aim)) * 100 : 0;
	f2 = (si->player[1].stat.check_out_aim != 0) ? ((float)(si->player[1].stat.check_out) / (float)(si->player[1].stat.check_out_aim)) * 100 : 0;
	_stprintf(p, TEXT(",%s,%.2f,%s,%.2f\r\n"), message_get_res(IDS_STRING_P_AVG_CHECK_OUT_LARGE), f1, message_get_res(IDS_STRING_P_AVG_CHECK_OUT_LARGE), f2);
	STR_SKIP(p);

	f1 = (si->player[0].stat.all_keep_legs != 0) ? ((float)(si->player[0].stat.win_keep_legs) / (float)(si->player[0].stat.all_keep_legs)) * 100 : 0;
	f2 = (si->player[1].stat.all_keep_legs != 0) ? ((float)(si->player[1].stat.win_keep_legs) / (float)(si->player[1].stat.all_keep_legs)) * 100 : 0;
	_stprintf(p, TEXT(",%s,%.2f,%s,%.2f\r\n"), message_get_res(IDS_STRING_P_AVG_KEEP), f1, message_get_res(IDS_STRING_P_AVG_KEEP), f2);
	STR_SKIP(p);

	f1 = (si->player[0].stat.all_break_legs != 0) ? ((float)(si->player[0].stat.win_break_legs) / (float)(si->player[0].stat.all_break_legs)) * 100 : 0;
	f2 = (si->player[1].stat.all_break_legs != 0) ? ((float)(si->player[1].stat.win_break_legs) / (float)(si->player[1].stat.all_break_legs)) * 100 : 0;
	_stprintf(p, TEXT(",%s,%.2f,%s,%.2f\r\n"), message_get_res(IDS_STRING_P_AVG_BREAK), f1, message_get_res(IDS_STRING_P_AVG_BREAK), f2);
	STR_SKIP(p);

	// Finish stats
	message_copy_res(IDS_STRING_P_FINISH_STATS, p);
	STR_SKIP(p);
	p = create_line(p, message_get_res(IDS_STRING_P_2_80_SUCCESS), &si->player[0].stat.success_2_80, &si->player[1].stat.success_2_80);
	p = create_line(p, message_get_res(IDS_STRING_P_2_80_FAILURE), &si->player[0].stat.failure_2_80, &si->player[1].stat.failure_2_80);
	p = create_line(p, message_get_res(IDS_STRING_P_81_130_SUCCESS), &si->player[0].stat.success_81_130, &si->player[1].stat.success_81_130);
	p = create_line(p, message_get_res(IDS_STRING_P_81_130_FAILURE), &si->player[0].stat.failure_81_130, &si->player[1].stat.failure_81_130);
	p = create_line(p, message_get_res(IDS_STRING_P_131_SUCCESS), &si->player[0].stat.success_131, &si->player[1].stat.success_131);
	p = create_line(p, message_get_res(IDS_STRING_P_131_FAILURE), &si->player[0].stat.failure_131, &si->player[1].stat.failure_131);

	// Counts
	message_copy_res(IDS_STRING_P_COUNTS, p);
	STR_SKIP(p);
	p = create_line(p, message_get_res(IDS_STRING_P_LEGS), &si->current_leg, &si->current_leg);
	p = create_line(p, message_get_res(IDS_STRING_P_WIN_DARTS), &si->player[0].stat.win_darts, &si->player[1].stat.win_darts);
	p = create_line(p, message_get_res(IDS_STRING_P_AVG_SCORE), &si->player[0].stat.all_score, &si->player[1].stat.all_score);
	p = create_line(p, message_get_res(IDS_STRING_P_AVG_DARTS), &si->player[0].stat.all_darts, &si->player[1].stat.all_darts);
	p = create_line(p, message_get_res(IDS_STRING_P_FIRST9_SCORE), &si->player[0].stat.first9_score, &si->player[1].stat.first9_score);
	p = create_line(p, message_get_res(IDS_STRING_P_FIRST9_DARTS), &si->player[0].stat.first9_darts, &si->player[1].stat.first9_darts);
	p = create_line(p, message_get_res(IDS_STRING_P_AVG_CHECK_OUT_LARGE), &si->player[0].stat.check_out, &si->player[1].stat.check_out);
	p = create_line(p, message_get_res(IDS_STRING_P_DOUBLE), &si->player[0].stat.check_out_aim, &si->player[1].stat.check_out_aim);
	p = create_line(p, message_get_res(IDS_STRING_P_AVG_KEEP_WON), &si->player[0].stat.win_keep_legs, &si->player[1].stat.win_keep_legs);
	p = create_line(p, message_get_res(IDS_STRING_P_AVG_KEEP_ALL), &si->player[0].stat.all_keep_legs, &si->player[1].stat.all_keep_legs);
	p = create_line(p, message_get_res(IDS_STRING_P_AVG_BREAK_WON), &si->player[0].stat.win_break_legs, &si->player[1].stat.win_break_legs);
	p = create_line(p, message_get_res(IDS_STRING_P_AVG_BREAK_ALL), &si->player[0].stat.all_break_legs, &si->player[1].stat.all_break_legs);

	// Leg
	for (i = 0; i < si->current_leg; i++) {
		*(p++) = TEXT('\r');
		*(p++) = TEXT('\n');

		// ヘッダ Round,Scored,To Go,Scored,To Go
		message_copy_res(IDS_STRING_ROUND, p);
		STR_SKIP(p);
		*(p++) = TEXT(',');
		message_copy_res(IDS_STRING_SCORED, p);
		STR_SKIP(p);
		*(p++) = TEXT(',');
		message_copy_res(IDS_STRING_TO_GO, p);
		STR_SKIP(p);
		*(p++) = TEXT(',');
		message_copy_res(IDS_STRING_SCORED, p);
		STR_SKIP(p);
		*(p++) = TEXT(',');
		message_copy_res(IDS_STRING_TO_GO, p);
		STR_SKIP(p);
		*(p++) = TEXT('\r');
		*(p++) = TEXT('\n');

		// 開始スコア
		*(p++) = TEXT(',');
		if (si->leg[i].first == 0) {
			message_copy_res(IDS_STRING_FIRST_MARK, p);
			STR_SKIP(p);
		}
		*(p++) = TEXT(',');
		_itot(si->player[0].start_score, p, 10);
		STR_SKIP(p);
		*(p++) = TEXT(',');
		if (si->leg[i].first == 1) {
			message_copy_res(IDS_STRING_FIRST_MARK, p);
			STR_SKIP(p);
		}
		*(p++) = TEXT(',');
		_itot(si->player[1].start_score, p, 10);
		STR_SKIP(p);
		*(p++) = TEXT('\r');
		*(p++) = TEXT('\n');

		// スコア
		left_over[0] = si->player[0].start_score;
		left_over[1] = si->player[1].start_score;
		for (j = 0; j < si->leg[i].max_round; j++) {
			_itot(j + 1, p, 10);
			STR_SKIP(p);
			for (t = 0; t < 2; t++) {
				if (j < si->leg[i].current_round || (j == si->leg[i].current_round && si->leg[i].first == t && si->leg[i].current_player != t)) {
					*(p++) = TEXT(',');
					_itot(si->leg[i].score[t][j], p, 10);
					STR_SKIP(p);
					*(p++) = TEXT(',');
					if (si->leg[i].score[t][j] >= 0) {
						left_over[t] -= si->leg[i].score[t][j];
						_itot(left_over[t], p, 10);
						STR_SKIP(p);
					}
				} else {
					*(p++) = TEXT(',');
					*(p++) = TEXT(',');
				}
			}
			*(p++) = TEXT('\r');
			*(p++) = TEXT('\n');
		}
		// Darts数
		message_copy_res(IDS_STRING_DARTS, p);
		STR_SKIP(p);
		*(p++) = TEXT(',');
		if (si->leg[i].winner == 1) {
			*(p++) = TEXT(',');
			*(p++) = TEXT(',');
		}
		_itot(si->leg[i].darts, p, 10);
		STR_SKIP(p);
		if (si->leg[i].winner == 0) {
			*(p++) = TEXT(',');
			*(p++) = TEXT(',');
		}
		*(p++) = TEXT(',');
		*(p++) = TEXT('\r');
		*(p++) = TEXT('\n');
	}
	*p = TEXT('\0');
	return TRUE;
}

/*
 * score_save - スコアの保存
 */
BOOL score_save(const HWND hWnd, const SCORE_INFO *si)
{
	HCURSOR old_cursor;
	TCHAR path[MAX_PATH];
	TCHAR *buf;
	TCHAR date[BUF_SIZE];
	TCHAR time[BUF_SIZE];
	TCHAR err_str[BUF_SIZE];
	DWORD size;
#ifdef UNICODE
	char *cbuf;
#endif

	// ファイル名の生成
	GetDateFormat(0, 0, &si->start_time, TEXT("yyyyMMdd"), date, BUF_SIZE - 1);
	GetTimeFormat(0, 0, &si->start_time, TEXT("HHmmss"), time, BUF_SIZE - 1);
	wsprintf(path, TEXT("%d_%s_%s.csv"), si->start_score, date, time);

	// ファイルの選択
	if (get_save_path(hWnd, message_get_res(IDS_STRING_SAVE_TITLE),
		TEXT("CSV (*.csv)\0*.csv\0All files (*.*)\0*.*\0\0"), TEXT("csv"), path) == FALSE) {
		return FALSE;
	}

	// スコア文字列の作成
	old_cursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
	size = get_score_string_length(si);
	buf = mem_alloc(sizeof(TCHAR) * (size + 1));
	if (buf == NULL) {
		message_get_error(GetLastError(), err_str);
		SetCursor(old_cursor);
		MessageBox(hWnd, err_str, APP_NAME, MB_ICONERROR);
		return FALSE;
	}
	get_score_string(si, buf);
#ifdef UNICODE
	cbuf = mem_alloc(sizeof(char) * (size + 1));
	if (cbuf == NULL) {
		message_get_error(GetLastError(), err_str);
		SetCursor(old_cursor);
		MessageBox(hWnd, err_str, APP_NAME, MB_ICONERROR);
		return FALSE;
	}
	tchar_to_char(buf, cbuf, size);
	mem_free(&buf);
	if (file_write_buf(path, (BYTE *)cbuf, sizeof(char) * size, err_str) == FALSE) {
		mem_free(&cbuf);
		SetCursor(old_cursor);
		MessageBox(hWnd, err_str, APP_NAME, MB_ICONERROR);
		return FALSE;
	}
	mem_free(&cbuf);
#else
	if (file_write_buf(path, (BYTE *)buf, sizeof(TCHAR) * size, err_str) == FALSE) {
		mem_free(&buf);
		SetCursor(old_cursor);
		MessageBox(hWnd, err_str, APP_NAME, MB_ICONERROR);
		return FALSE;
	}
	mem_free(&buf);
#endif
	SetCursor(old_cursor);
	return TRUE;
}

/*
 * score_auto_save - スコアの自動保存
 */
BOOL score_auto_save(const HWND hWnd, const SCORE_INFO *si)
{
	HCURSOR old_cursor;
	TCHAR path[MAX_PATH];
	TCHAR *buf;
	TCHAR file_name[BUF_SIZE];
	TCHAR date[BUF_SIZE];
	TCHAR time[BUF_SIZE];
	TCHAR err_str[BUF_SIZE];
	DWORD size;
#ifdef UNICODE
	char *cbuf;
#endif

	// ファイル名の生成
	GetDateFormat(0, 0, &si->start_time, TEXT("yyyyMMdd"), date, BUF_SIZE - 1);
	GetTimeFormat(0, 0, &si->start_time, TEXT("HHmmss"), time, BUF_SIZE - 1);
	wsprintf(file_name, TEXT("%d_%s_%s.csv"), si->start_score, date, time);

	if (*op.auto_save_path == TEXT('\0')) {
		wsprintf(path, TEXT("%s\\data"), work_path);
		CreateDirectory(path, NULL);
		wsprintf(path, TEXT("%s\\data\\%s"), work_path, file_name);
	} else {
		create_tree_directory(op.auto_save_path);
		if(*(op.auto_save_path + lstrlen(op.auto_save_path)) == TEXT('\\') ||
			*(op.auto_save_path + lstrlen(op.auto_save_path)) == TEXT('/')){
			wsprintf(path, TEXT("%s%s"), op.auto_save_path, file_name);
		} else {
			wsprintf(path, TEXT("%s\\%s"), op.auto_save_path, file_name);
		}
	}

	// スコア文字列の作成
	old_cursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
	size = get_score_string_length(si);
	buf = mem_alloc(sizeof(TCHAR) * (size + 1));
	if (buf == NULL) {
		message_get_error(GetLastError(), err_str);
		SetCursor(old_cursor);
		MessageBox(hWnd, err_str, APP_NAME, MB_ICONERROR);
		return FALSE;
	}
	get_score_string(si, buf);

#ifdef UNICODE
	cbuf = mem_alloc(sizeof(char) * (size + 1));
	if (cbuf == NULL) {
		message_get_error(GetLastError(), err_str);
		SetCursor(old_cursor);
		MessageBox(hWnd, err_str, APP_NAME, MB_ICONERROR);
		return FALSE;
	}
	tchar_to_char(buf, cbuf, size);
	mem_free(&buf);
	if (file_write_buf(path, (BYTE *)cbuf, sizeof(char) * size, err_str) == FALSE) {
		mem_free(&cbuf);
		SetCursor(old_cursor);
		MessageBox(hWnd, err_str, APP_NAME, MB_ICONERROR);
		return FALSE;
	}
	mem_free(&cbuf);
#else
	if (file_write_buf(path, (BYTE *)buf, sizeof(TCHAR) * size, err_str) == FALSE) {
		mem_free(&buf);
		SetCursor(old_cursor);
		MessageBox(hWnd, err_str, APP_NAME, MB_ICONERROR);
		return FALSE;
	}
	mem_free(&buf);
#endif
	SetCursor(old_cursor);
	return TRUE;
}
/* End of source */
