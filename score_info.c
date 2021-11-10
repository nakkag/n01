/*
 * n01
 *
 * score_info.c
 *
 * Copyright (C) 1996-2008 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

/* Include Files */
#define _INC_OLE
#include <windows.h>
#undef  _INC_OLE

#include "general.h"
#include "Memory.h"
#include "String.h"
#include "Message.h"

#include "resource.h"

/* Define */

/* Global Variables */
extern OPTION_INFO op;

/* Local Function Prototypes */

/*
 * score_info_free - スコア情報の解放
 */
BOOL score_info_free(SCORE_INFO *si)
{
	int i;

	if (si->leg != NULL) {
		for (i = 0; i <= si->current_leg; i++) {
			mem_free(&si->leg[i].score[0]);
			mem_free(&si->leg[i].score[1]);
		}
		mem_free(&si->leg);
	}
	mem_free(&si->tmp_check_out[0]);
	mem_free(&si->tmp_check_out[1]);
	si->current_leg = 0;
	return TRUE;
}

/*
 * score_info_init - スコア情報の初期化
 */
BOOL score_info_init(const HWND hWnd, SCORE_INFO *si, GAME_INFO *gi, const BOOL p_init)
{
	TCHAR err_str[BUF_SIZE];
	int first = 0;
	int i;

	si->history = FALSE;
	// オプションの設定
	si->start_score = gi->start_score;
	si->round_limit = gi->round_limit;
	si->round = gi->round;
	si->leg_limit = gi->leg_limit;
	si->max_leg = gi->max_leg;
	si->best_of = gi->best_of;
	// プレイヤー情報の初期化
	for (i = 0; i < 2; i++) {
		if (p_init == TRUE) {
			ZeroMemory(&si->player[i], sizeof(PLAYER_INFO));
		} else {
			// 統計のコピー
			si->player[i].set_stat.ton_count += si->player[i].stat.ton_count;
			si->player[i].set_stat.ton00_count += si->player[i].stat.ton00_count;
			si->player[i].set_stat.ton40_count += si->player[i].stat.ton40_count;
			si->player[i].set_stat.ton80_count += si->player[i].stat.ton80_count;
			si->player[i].set_stat.high_off = (TYPE_SCORE)((si->player[i].stat.high_off > si->player[i].set_stat.high_off) ?
				si->player[i].stat.high_off : si->player[i].set_stat.high_off);
			if (si->set_mode == TRUE && op.gi_list[si->current_set].start_score != op.gi_list[si->current_set - 1].start_score) {
				si->player[i].set_stat.short_game = 0;
			} else if (si->player[i].stat.short_game > 0 &&
				(si->player[i].set_stat.short_game == 0 || si->player[i].stat.short_game < si->player[i].set_stat.short_game)) {
				si->player[i].set_stat.short_game = si->player[i].stat.short_game;
			}
			if (si->set_mode == TRUE && op.gi_list[si->current_set].start_score != op.gi_list[si->current_set - 1].start_score) {
				si->player[i].set_stat.long_game = 0;
			} else if (si->player[i].stat.long_game > si->player[i].set_stat.long_game) {
				si->player[i].set_stat.long_game = si->player[i].stat.long_game;
			}

			// スコア平均
			si->player[i].set_stat.all_score += si->player[i].stat.all_score;
			si->player[i].set_stat.all_darts += si->player[i].stat.all_darts;

			// ダーツ数平均
			if (si->set_mode == TRUE && op.gi_list[si->current_set].start_score != op.gi_list[si->current_set - 1].start_score) {
				si->player[i].set_stat.win_darts = 0;
				si->player[i].set_stat.win_count = 0;
			} else {
				si->player[i].set_stat.win_darts += si->player[i].stat.win_darts;
				si->player[i].set_stat.win_count += si->player[i].stat.win_count;
			}

			// First 9 darts平均
			si->player[i].set_stat.first9_score += si->player[i].stat.first9_score;
			si->player[i].set_stat.first9_darts += si->player[i].stat.first9_darts;

			// チェックアウト率
			si->player[i].set_stat.check_out_aim += si->player[i].stat.check_out_aim;
			si->player[i].set_stat.check_out += si->player[i].stat.check_out;

			// キープ率
			si->player[i].set_stat.all_keep_legs += si->player[i].stat.all_keep_legs;
			si->player[i].set_stat.win_keep_legs += si->player[i].stat.win_keep_legs;
			// ブレイク率
			si->player[i].set_stat.all_break_legs += si->player[i].stat.all_break_legs;
			si->player[i].set_stat.win_break_legs += si->player[i].stat.win_break_legs;

			// Finish stats
			si->player[i].set_stat.success_2_80 += si->player[i].stat.success_2_80;
			si->player[i].set_stat.failure_2_80 += si->player[i].stat.failure_2_80;
			si->player[i].set_stat.success_81_130 += si->player[i].stat.success_81_130;
			si->player[i].set_stat.failure_81_130 += si->player[i].stat.failure_81_130;
			si->player[i].set_stat.success_131 += si->player[i].stat.success_131;
			si->player[i].set_stat.failure_131 += si->player[i].stat.failure_131;
		}
		// 統計情報の初期化
		ZeroMemory(&si->player[i].stat, sizeof(STATISTICS_INFO));
		// プレイヤー情報の設定
		if (p_init == TRUE || *gi->player_name[i] != TEXT('\0') || gi->com[i] == TRUE) {
			lstrcpy(si->player[i].name, gi->player_name[i]);
			si->player[i].lock = gi->com[i];
			si->player[i].check_out_mode = (gi->com[i] == TRUE) ? FALSE : ((op.check_out_mode == 0) ? FALSE : TRUE);
			si->player[i].com = gi->com[i];
			si->player[i].level = gi->level[i];
		}
		si->player[i].start_score = gi->player_start_score[i];
		if (si->player[i].start_score <= 0) {
			si->player[i].start_score = gi->start_score;
		}
		si->player[i].left = si->player[i].start_score;
		si->player[i].legs = 0;
	}
	if (*si->player[0].name == TEXT('\0')) {
		message_copy_res(IDS_STRING_PLAYER1, si->player[0].name);
	}
	if (*si->player[1].name == TEXT('\0')) {
		message_copy_res(IDS_STRING_PLAYER2, si->player[1].name);
	}
	if (si->leg != NULL) {
		// 前セットの先攻を退避
		first = si->leg[0].first;
	}

	// 開始時間の取得
	GetLocalTime(&si->start_time);
	// レッグ情報の初期化
	score_info_free(si);
	si->leg = (LEG_INFO *)mem_calloc(sizeof(LEG_INFO));
	if (si->leg == NULL) {
		message_get_error(GetLastError(), err_str);
		MessageBox(hWnd, err_str, APP_NAME, MB_ICONERROR);
		return FALSE;
	}
	if (gi->change_first == TRUE && si->current_set != 0) {
		// 先攻の入れ替え
		si->leg[0].first = !first;
	}
	si->leg[0].alloc_round = (si->round_limit == 0) ? ALLOC_ROUND : si->round;
	si->leg[0].max_round = (si->round_limit == 0) ? 1 : si->round;
	si->leg[0].score[0] = (TYPE_SCORE *)mem_calloc(sizeof(TYPE_SCORE) * (si->leg[0].alloc_round + 1));
	if (si->leg[0].score[0] == NULL) {
		message_get_error(GetLastError(), err_str);
		MessageBox(hWnd, err_str, APP_NAME, MB_ICONERROR);
		return FALSE;
	}
	si->leg[0].score[1] = (TYPE_SCORE *)mem_calloc(sizeof(TYPE_SCORE) * (si->leg[0].alloc_round + 1));
	if (si->leg[0].score[1] == NULL) {
		message_get_error(GetLastError(), err_str);
		MessageBox(hWnd, err_str, APP_NAME, MB_ICONERROR);
		return FALSE;
	}
	si->tmp_check_out[0] = (TYPE_CHECK_OUT *)mem_calloc(sizeof(TYPE_CHECK_OUT) * (si->leg[0].alloc_round + 1));
	if (si->tmp_check_out[0] == NULL) {
		message_get_error(GetLastError(), err_str);
		MessageBox(hWnd, err_str, APP_NAME, MB_ICONERROR);
		return FALSE;
	}
	si->tmp_check_out[1] = (TYPE_CHECK_OUT *)mem_calloc(sizeof(TYPE_CHECK_OUT) * (si->leg[0].alloc_round + 1));
	if (si->tmp_check_out[1] == NULL) {
		message_get_error(GetLastError(), err_str);
		MessageBox(hWnd, err_str, APP_NAME, MB_ICONERROR);
		return FALSE;
	}
	return TRUE;
}

/*
 * score_info_copy - スコア情報のコピー
 */
BOOL score_info_copy(SCORE_INFO *to_si, const SCORE_INFO *from_si)
{
	int i, j;

	// スコア情報のコピー
	*to_si = *from_si;

	// レッグ情報のコピー
	to_si->leg = (LEG_INFO *)mem_alloc(sizeof(LEG_INFO) * (from_si->current_leg + 1));
	if (to_si->leg == NULL) {
		return FALSE;
	}
	CopyMemory(to_si->leg, from_si->leg, sizeof(LEG_INFO) * (from_si->current_leg + 1));
	// スコアのコピー
	for (i = 0; i <= from_si->current_leg; i++) {
		for (j = 0; j < 2; j++) {
			to_si->leg[i].score[j] = (TYPE_SCORE *)mem_alloc(sizeof(TYPE_SCORE) * (from_si->leg[i].alloc_round + 1));
			if (to_si->leg[i].score[j] == NULL) {
				return FALSE;
			}
			CopyMemory(to_si->leg[i].score[j], from_si->leg[i].score[j], sizeof(TYPE_SCORE) * (from_si->leg[i].alloc_round + 1));
		}
	}
	// チェックアウト本数のコピー
	for (j = 0; j < 2; j++) {
		to_si->tmp_check_out[j] = (TYPE_CHECK_OUT *)mem_alloc(sizeof(TYPE_CHECK_OUT) * (from_si->leg[from_si->current_leg].alloc_round + 1));
		if (to_si->tmp_check_out[j] == NULL) {
			return FALSE;
		}
		CopyMemory(to_si->tmp_check_out[j], from_si->tmp_check_out[j], sizeof(TYPE_CHECK_OUT) * (from_si->leg[from_si->current_leg].alloc_round + 1));
	}
	return TRUE;
}

/*
 * score_history_set - スコア履歴の設定
 */
BOOL score_history_set(const HWND hWnd, SCORE_HISTORY *sh, SCORE_INFO *si)
{
	SCORE_INFO *tmp_si;
	TCHAR err_str[BUF_SIZE];
	int i, j;

	if (si->history == FALSE && si->current_leg > 0) {
		sh->list_count++;
		tmp_si = (SCORE_INFO *)mem_calloc(sizeof(SCORE_INFO) * sh->list_count);
		if (tmp_si == NULL) {
			message_get_error(GetLastError(), err_str);
			MessageBox(hWnd, err_str, APP_NAME, MB_ICONERROR);
			return FALSE;
		}
		if (sh->list != NULL) {
			CopyMemory(tmp_si, sh->list, sizeof(SCORE_INFO) * (sh->list_count - 1));
			mem_free(&sh->list);
		}
		sh->list = tmp_si;
		// スコア情報のコピー
		sh->list[sh->list_count - 1] = *si;
		// レッグ情報のコピー
		sh->list[sh->list_count - 1].leg = (LEG_INFO *)mem_alloc(sizeof(LEG_INFO) * (si->current_leg + 1));
		if (sh->list[sh->list_count - 1].leg == NULL) {
			message_get_error(GetLastError(), err_str);
			MessageBox(hWnd, err_str, APP_NAME, MB_ICONERROR);
			return FALSE;
		}
		CopyMemory(sh->list[sh->list_count - 1].leg, si->leg, sizeof(LEG_INFO) * (si->current_leg + 1));
		// スコアのコピー
		for (i = 0; i <= si->current_leg; i++) {
			for (j = 0; j < 2; j++) {
				sh->list[sh->list_count - 1].leg[i].score[j] = (TYPE_SCORE *)mem_alloc(sizeof(TYPE_SCORE) * (si->leg[i].alloc_round + 1));
				if (sh->list[sh->list_count - 1].leg[i].score[j] == NULL) {
					message_get_error(GetLastError(), err_str);
					MessageBox(hWnd, err_str, APP_NAME, MB_ICONERROR);
					return FALSE;
				}
				CopyMemory(sh->list[sh->list_count - 1].leg[i].score[j], si->leg[i].score[j], sizeof(TYPE_SCORE) * (si->leg[i].alloc_round + 1));
			}
		}
		// 統計情報の初期化
		ZeroMemory(&sh->list[sh->list_count - 1].player[0].set_stat, sizeof(STATISTICS_INFO));
		ZeroMemory(&sh->list[sh->list_count - 1].player[1].set_stat, sizeof(STATISTICS_INFO));
		sh->list[sh->list_count - 1].tmp_check_out[0] = NULL;
		sh->list[sh->list_count - 1].tmp_check_out[1] = NULL;
		si->history = TRUE;
	}
	return TRUE;
}

/*
 * score_history_free - スコア履歴の解放
 */
BOOL score_history_free(SCORE_HISTORY *sh)
{
	int i;

	if (sh->list != NULL) {
		for (i = 0; i < sh->list_count; i++) {
			score_info_free(&sh->list[i]);
		}
		mem_free(&sh->list);
	}
	sh->list_count = 0;
	return TRUE;
}
/* End of source */
