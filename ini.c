/*
 * n01
 *
 * Ini.c
 *
 * Copyright (C) 1996-2016 by Ohno Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

/* Include Files */
#define _INC_OLE
#include <windows.h>
#undef  _INC_OLE
#include <tchar.h>

#include "General.h"
#include "Memory.h"
#include "String.h"
#include "Message.h"
#include "Profile.h"

#include "resource.h"

/* Define */

/* Global Variables */
extern OPTION_INFO op;

/* Local Function Prototypes */
static COLORREF get_color(const TCHAR *buf, const COLORREF def_color);
static void put_color(const TCHAR *ini_path, const TCHAR *key, const COLORREF color, const COLORREF def_color);

/*
 * get_color - 色情報を取得
 */
static COLORREF get_color(const TCHAR *buf, const COLORREF def_color)
{
	if (*buf == TEXT('\0')) {
		return def_color;
	}
	return _tcstol(buf, NULL, 0);
}

/*
 * ini_get_option - オプションを取得
 */
BOOL ini_get_option(const TCHAR *ini_path)
{
	RECT rect;
	TCHAR buf[BUF_SIZE];
	WORD lang;
	int i, j;

	profile_initialize(ini_path, TRUE);

	SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0);
	lang = (WORD)PRIMARYLANGID(LANGIDFROMLCID(GetThreadLocale()));

	op.window_rect.left = profile_get_int(TEXT("window"), TEXT("left"), 0, ini_path);
	op.window_rect.top = profile_get_int(TEXT("window"), TEXT("top"), 0, ini_path);
	op.window_rect.right = profile_get_int(TEXT("window"), TEXT("right"), 0, ini_path);
	op.window_rect.bottom = profile_get_int(TEXT("window"), TEXT("bottom"), 0, ini_path);
	if (op.window_rect.left > rect.right || op.window_rect.top > rect.bottom) {
		op.window_rect.left = 0;
		op.window_rect.top = 0;
	}
	op.window_state = profile_get_int(TEXT("window"), TEXT("state"), SW_SHOWDEFAULT, ini_path);
	op.full_screen = profile_get_int(TEXT("window"), TEXT("full_screen"), 0, ini_path);
	op.full_screen_menu_interval = profile_get_int(TEXT("window"), TEXT("full_screen_menu_interval"), 3000, ini_path);

	op.view_name = profile_get_int(TEXT("view"), TEXT("name"), 1, ini_path);
	op.view_name_size = profile_get_int(TEXT("view"), TEXT("name_size"), 11, ini_path);
	op.view_player = profile_get_int(TEXT("view"), TEXT("player"), 1, ini_path);
	op.view_left = profile_get_int(TEXT("view"), TEXT("left"), 1, ini_path);
	op.view_guide = profile_get_int(TEXT("view"), TEXT("guide"), 1, ini_path);
	op.view_throw_count = profile_get_int(TEXT("view"), TEXT("throw_count"), 1, ini_path);
	op.view_ton_circle = profile_get_int(TEXT("view"), TEXT("ton_circle"), 1, ini_path);
	op.view_separate = profile_get_int(TEXT("view"), TEXT("separate"), 1, ini_path);
	op.view_scroll_bar = profile_get_int(TEXT("view"), TEXT("scroll_bar"), 1, ini_path);
	op.view_separate_width = profile_get_int(TEXT("view"), TEXT("view_separate_width"), 2, ini_path);

	// game option
	op.gi.start_score = profile_get_int(TEXT("game"), TEXT("start_score"), 501, ini_path);
	op.gi.round_limit = profile_get_int(TEXT("game"), TEXT("round_limit"), ((lang != LANG_JAPANESE) ? 0 : 1), ini_path);
	op.gi.round = profile_get_int(TEXT("game"), TEXT("round"), 15, ini_path);

	op.gi.leg_limit = profile_get_int(TEXT("game"), TEXT("leg_limit"), 0, ini_path);
	op.gi.max_leg = profile_get_int(TEXT("game"), TEXT("max_leg"), 3, ini_path);
	op.gi.best_of = profile_get_int(TEXT("game"), TEXT("best_of"), 1, ini_path);
	op.gi.change_first = profile_get_int(TEXT("game"), TEXT("change_first"), 0, ini_path);

	profile_get_string(TEXT("game"), TEXT("p1_name"), message_get_res(IDS_STRING_PLAYER1), op.gi.player_name[0], NAME_SIZE - 1, ini_path);
	op.gi.player_start_score[0] = profile_get_int(TEXT("game"), TEXT("p1_start_score"), 0, ini_path);
	op.gi.com[0] = profile_get_int(TEXT("game"), TEXT("p1_com"), FALSE, ini_path);
	op.gi.level[0] = profile_get_int(TEXT("game"), TEXT("p1_com_level"), 0, ini_path);

	profile_get_string(TEXT("game"), TEXT("p2_name"), message_get_res(IDS_STRING_PLAYER2), op.gi.player_name[1], NAME_SIZE - 1, ini_path);
	op.gi.player_start_score[1] = profile_get_int(TEXT("game"), TEXT("p2_start_score"), 0, ini_path);
	op.gi.com[1] = profile_get_int(TEXT("game"), TEXT("p2_com"), FALSE, ini_path);
	op.gi.level[1] = profile_get_int(TEXT("game"), TEXT("p2_com_level"), 0, ini_path);

	// 自動保存
	op.auto_save = profile_get_int(TEXT("game"), TEXT("auto_save"), 0, ini_path);
	profile_get_string(TEXT("game"), TEXT("auto_save_path"), TEXT(""), op.auto_save_path, BUF_SIZE - 1, ini_path);

	// チェックアウトモード
	op.check_out_mode = profile_get_int(TEXT("game"), TEXT("check_out_mode"), 0, ini_path);

	// 名前リスト
	op.name_list_count = profile_get_int(TEXT("game"), TEXT("name_list_count"), 0, ini_path);
	if (op.name_list_count > NAME_LIST_COUNT) {
		op.name_list_count = NAME_LIST_COUNT;
	}
	for (i = 0; i < op.name_list_count; i++) {
		wsprintf(buf, TEXT("name_list_%d"), i);
		profile_get_string(TEXT("game"), buf, TEXT(""), op.name_list[i], NAME_SIZE - 1, ini_path);
	}

	// チェックアウトモード
	op.check_out_mode = profile_get_int(TEXT("game"), TEXT("check_out_mode"), 0, ini_path);
	
	// リカバリモード
	op.recovery = profile_get_int(TEXT("game"), TEXT("recovery"), 1, ini_path);
	op.recovery_delete = profile_get_int(TEXT("game"), TEXT("recovery_delete"), 0, ini_path);

	// com
	op.com_default_number = profile_get_int(TEXT("com"), TEXT("default_number"), 20, ini_path);
	op.com_default_segment = profile_get_int(TEXT("com"), TEXT("default_segment"), 3, ini_path);
	op.com_timer = profile_get_int(TEXT("com"), TEXT("com_timer"), 1000, ini_path);

	// スケジュール
	op.gi_list_count = profile_get_int(TEXT("schedule"), TEXT("count"), -1, ini_path);
	if (op.gi_list_count == -1) {
		op.gi_list = (GAME_INFO *)mem_calloc(sizeof(GAME_INFO) * 10);
		if (op.gi_list == NULL) {
			return FALSE;
		}
		op.gi_list_count = 0;
		op.gi_list[op.gi_list_count].start_score = 1001;
		op.gi_list[op.gi_list_count].round_limit = ((lang != LANG_JAPANESE) ? 0 : 1);
		op.gi_list[op.gi_list_count].round = 30;
		op.gi_list[op.gi_list_count].leg_limit = 1;
		op.gi_list[op.gi_list_count].max_leg = 1;
		op.gi_list[op.gi_list_count].best_of = 1;
		op.gi_list[op.gi_list_count].change_first = 0;
		message_copy_res(IDS_STRING_PLAYER1, op.gi_list[op.gi_list_count].player_name[0]);
		message_copy_res(IDS_STRING_PLAYER2, op.gi_list[op.gi_list_count].player_name[1]);
		op.gi_list_count++;
		for (j = 0; j < 6; j++) {
			op.gi_list[op.gi_list_count].start_score = 501;
			op.gi_list[op.gi_list_count].round_limit = ((lang != LANG_JAPANESE) ? 0 : 1);
			op.gi_list[op.gi_list_count].round = 15;
			op.gi_list[op.gi_list_count].leg_limit = 1;
			op.gi_list[op.gi_list_count].max_leg = 3;
			op.gi_list[op.gi_list_count].best_of = 1;
			op.gi_list[op.gi_list_count].change_first = 0;
			op.gi_list_count++;
		}
	} else {
		op.gi_list = (GAME_INFO *)mem_calloc(sizeof(GAME_INFO) * op.gi_list_count);
		if (op.gi_list == NULL) {
			return FALSE;
		}
		for (i = 0; i < op.gi_list_count; i++) {
			wsprintf(buf, TEXT("start_score_%d"), i);
			op.gi_list[i].start_score = profile_get_int(TEXT("schedule"), buf, 501, ini_path);

			wsprintf(buf, TEXT("round_limit_%d"), i);
			op.gi_list[i].round_limit = profile_get_int(TEXT("schedule"), buf, ((lang != LANG_JAPANESE) ? 0 : 1), ini_path);
			wsprintf(buf, TEXT("round_%d"), i);
			op.gi_list[i].round = profile_get_int(TEXT("schedule"), buf, 15, ini_path);

			op.gi_list[i].leg_limit = 1;
			wsprintf(buf, TEXT("max_leg_%d"), i);
			op.gi_list[i].max_leg = profile_get_int(TEXT("schedule"), buf, 3, ini_path);
			wsprintf(buf, TEXT("best_of_%d"), i);
			op.gi_list[i].best_of = profile_get_int(TEXT("schedule"), buf, 1, ini_path);
			wsprintf(buf, TEXT("change_first_%d"), i);
			op.gi_list[i].change_first = profile_get_int(TEXT("schedule"), buf, 0, ini_path);

			wsprintf(buf, TEXT("p1_name_%d"), i);
			profile_get_string(TEXT("schedule"), buf, TEXT(""), op.gi_list[i].player_name[0], NAME_SIZE - 1, ini_path);
			wsprintf(buf, TEXT("p1_start_score_%d"), i);
			op.gi_list[i].player_start_score[0] = profile_get_int(TEXT("schedule"), buf, 501, ini_path);
			wsprintf(buf, TEXT("p1_com_%d"), i);
			op.gi_list[i].com[0] = profile_get_int(TEXT("schedule"), buf, FALSE, ini_path);
			wsprintf(buf, TEXT("p1_com_level_%d"), i);
			op.gi_list[i].level[0] = profile_get_int(TEXT("schedule"), buf, 0, ini_path);

			wsprintf(buf, TEXT("p2_name_%d"), i);
			profile_get_string(TEXT("schedule"), buf, TEXT(""), op.gi_list[i].player_name[1], NAME_SIZE - 1, ini_path);
			wsprintf(buf, TEXT("p2_start_score_%d"), i);
			op.gi_list[i].player_start_score[1] = profile_get_int(TEXT("schedule"), buf, 0, ini_path);
			wsprintf(buf, TEXT("p2_com_%d"), i);
			op.gi_list[i].com[1] = profile_get_int(TEXT("schedule"), buf, FALSE, ini_path);
			wsprintf(buf, TEXT("p2_com_level_%d"), i);
			op.gi_list[i].level[1] = profile_get_int(TEXT("schedule"), buf, 0, ini_path);
		}
	}

	// ゲーム履歴
	op.history_rect.left = profile_get_int(TEXT("history"), TEXT("left"), 0, ini_path);
	op.history_rect.top = profile_get_int(TEXT("history"), TEXT("top"), 0, ini_path);
	op.history_rect.right = profile_get_int(TEXT("history"), TEXT("right"), 0, ini_path);
	op.history_rect.bottom = profile_get_int(TEXT("history"), TEXT("bottom"), 0, ini_path);
	if (op.history_rect.left > rect.right || op.history_rect.top > rect.bottom) {
		op.history_rect.left = 0;
		op.history_rect.top = 0;
	}
	// スコア一覧
	op.score_list_rect.left = profile_get_int(TEXT("score_list"), TEXT("left"), 0, ini_path);
	op.score_list_rect.top = profile_get_int(TEXT("score_list"), TEXT("top"), 0, ini_path);
	op.score_list_rect.right = profile_get_int(TEXT("score_list"), TEXT("right"), 0, ini_path);
	op.score_list_rect.bottom = profile_get_int(TEXT("score_list"), TEXT("bottom"), 0, ini_path);
	if (op.score_list_rect.left > rect.right || op.score_list_rect.top > rect.bottom) {
		op.score_list_rect.left = 0;
		op.score_list_rect.top = 0;
	}

	// アレンジ
	op.arrange_rect.left = profile_get_int(TEXT("arrange"), TEXT("left"), 0, ini_path);
	op.arrange_rect.top = profile_get_int(TEXT("arrange"), TEXT("top"), 0, ini_path);
	op.arrange_rect.right = profile_get_int(TEXT("arrange"), TEXT("right"), 0, ini_path);
	op.arrange_rect.bottom = profile_get_int(TEXT("arrange"), TEXT("bottom"), 0, ini_path);
	if (op.arrange_rect.left > rect.right || op.arrange_rect.top > rect.bottom) {
		op.arrange_rect.left = 0;
		op.arrange_rect.top = 0;
	}

	// 色情報
	profile_get_string(TEXT("color"), TEXT("view_separate"), TEXT(""), buf, BUF_SIZE - 1, ini_path);
	op.ci.view_separate = get_color(buf, D_COLOR_VIEW_SEPARATE);

	profile_get_string(TEXT("color"), TEXT("background"), TEXT(""), buf, BUF_SIZE - 1, ini_path);
	op.ci.background = get_color(buf, D_COLOR_BACKGROUND);
	profile_get_string(TEXT("color"), TEXT("odd_background"), TEXT(""), buf, BUF_SIZE - 1, ini_path);
	op.ci.odd_background = get_color(buf, D_COLOR_ODD_BACKGROUND);
	profile_get_string(TEXT("color"), TEXT("scored_text"), TEXT(""), buf, BUF_SIZE - 1, ini_path);
	op.ci.scored_text = get_color(buf, D_COLOR_TEXT);
	profile_get_string(TEXT("color"), TEXT("text"), TEXT(""), buf, BUF_SIZE - 1, ini_path);
	op.ci.togo_text = get_color(buf, D_COLOR_TEXT);
	profile_get_string(TEXT("color"), TEXT("header_background"), TEXT(""), buf, BUF_SIZE - 1, ini_path);
	op.ci.header_background = get_color(buf, D_COLOR_HEADER_BACKGROUND);
	profile_get_string(TEXT("color"), TEXT("header_text"), TEXT(""), buf, BUF_SIZE - 1, ini_path);
	op.ci.header_text = get_color(buf, D_COLOR_HEADER_TEXT);
	profile_get_string(TEXT("color"), TEXT("last3number_text"), TEXT(""), buf, BUF_SIZE - 1, ini_path);
	op.ci.last3number_text = get_color(buf, D_COLOR_LAST3NUMBER_TEXT);
	profile_get_string(TEXT("color"), TEXT("line"), TEXT(""), buf, BUF_SIZE - 1, ini_path);
	op.ci.line = get_color(buf, D_COLOR_LINE);
	profile_get_string(TEXT("color"), TEXT("separate"), TEXT(""), buf, BUF_SIZE - 1, ini_path);
	op.ci.separate = get_color(buf, D_COLOR_SEPARATE);
	profile_get_string(TEXT("color"), TEXT("ton_circle"), TEXT(""), buf, BUF_SIZE - 1, ini_path);
	op.ci.ton_circle = get_color(buf, D_COLOR_TON_CIRCLE);

	profile_get_string(TEXT("color"), TEXT("input_background"), TEXT(""), buf, BUF_SIZE - 1, ini_path);
	op.ci.input_background = get_color(buf, D_COLOR_INPUT_BACKGROUND);
	profile_get_string(TEXT("color"), TEXT("input_text"), TEXT(""), buf, BUF_SIZE - 1, ini_path);
	op.ci.input_text = get_color(buf, D_COLOR_INPUT_TEXT);
	profile_get_string(TEXT("color"), TEXT("input_select_background"), TEXT(""), buf, BUF_SIZE - 1, ini_path);
	op.ci.input_select_background = get_color(buf, D_COLOR_INPUT_SELECT_BACKGROUND);
	profile_get_string(TEXT("color"), TEXT("input_select_text"), TEXT(""), buf, BUF_SIZE - 1, ini_path);
	op.ci.input_select_text = get_color(buf, D_COLOR_INPUT_SELECT_TEXT);

	profile_get_string(TEXT("color"), TEXT("left_background"), TEXT(""), buf, BUF_SIZE - 1, ini_path);
	op.ci.left_background = get_color(buf, D_COLOR_LEFT_BACKGROUND);
	profile_get_string(TEXT("color"), TEXT("left_text"), TEXT(""), buf, BUF_SIZE - 1, ini_path);
	op.ci.left_text = get_color(buf, D_COLOR_LEFT_TEXT);
	profile_get_string(TEXT("color"), TEXT("left_active_border"), TEXT(""), buf, BUF_SIZE - 1, ini_path);
	op.ci.left_active_border = get_color(buf, D_COLOR_LEFT_ACTIVE_BORDER);

	profile_get_string(TEXT("color"), TEXT("player_name_background"), TEXT(""), buf, BUF_SIZE - 1, ini_path);
	op.ci.player_name_background = get_color(buf, D_COLOR_PLAYER_NAME_BACKGROUND);
	profile_get_string(TEXT("color"), TEXT("player_name_text"), TEXT(""), buf, BUF_SIZE - 1, ini_path);
	op.ci.player_name_text = get_color(buf, D_COLOR_PLAYER_NAME_TEXT);
	profile_get_string(TEXT("color"), TEXT("player_background"), TEXT(""), buf, BUF_SIZE - 1, ini_path);
	op.ci.player_background = get_color(buf, D_COLOR_PLAYER_BACKGROUND);
	profile_get_string(TEXT("color"), TEXT("player_text"), TEXT(""), buf, BUF_SIZE - 1, ini_path);
	op.ci.player_text = get_color(buf, D_COLOR_PLAYER_TEXT);
	profile_get_string(TEXT("color"), TEXT("player_info_title"), TEXT(""), buf, BUF_SIZE - 1, ini_path);
	op.ci.player_info_title = get_color(buf, D_COLOR_PLAYER_INFO_TITLE);

	profile_get_string(TEXT("color"), TEXT("guide_background"), TEXT(""), buf, BUF_SIZE - 1, ini_path);
	op.ci.guide_background = get_color(buf, D_COLOR_GUIDE_BACKGROUND);
	profile_get_string(TEXT("color"), TEXT("guide_text"), TEXT(""), buf, BUF_SIZE - 1, ini_path);
	op.ci.guide_text = get_color(buf, D_COLOR_GUIDE_TEXT);
	profile_get_string(TEXT("color"), TEXT("guide_select_background"), TEXT(""), buf, BUF_SIZE - 1, ini_path);
	op.ci.guide_select_background = get_color(buf, D_COLOR_GUIDE_SELECT_BACKGROUND);
	profile_get_string(TEXT("color"), TEXT("guide_select_text"), TEXT(""), buf, BUF_SIZE - 1, ini_path);
	op.ci.guide_select_text = get_color(buf, D_COLOR_GUIDE_SELECT_TEXT);
	profile_get_string(TEXT("color"), TEXT("guide_box_background"), TEXT(""), buf, BUF_SIZE - 1, ini_path);
	op.ci.guide_box_background = get_color(buf, D_COLOR_GUIDE_BOX_BACKGROUND);
	profile_get_string(TEXT("color"), TEXT("guide_box_text"), TEXT(""), buf, BUF_SIZE - 1, ini_path);
	op.ci.guide_box_text = get_color(buf, D_COLOR_GUIDE_BOX_TEXT);
	profile_get_string(TEXT("color"), TEXT("guide_box_frame"), TEXT(""), buf, BUF_SIZE - 1, ini_path);
	op.ci.guide_box_frame = get_color(buf, D_COLOR_GUIDE_BOX_FRAME);
	profile_get_string(TEXT("color"), TEXT("guide_box_disable"), TEXT(""), buf, BUF_SIZE - 1, ini_path);
	op.ci.guide_box_disable = get_color(buf, D_COLOR_GUIDE_BOX_DISABLE);

	// フォント情報
	profile_get_string(TEXT("font"), TEXT("name"), message_get_res(IDS_STRING_DEFAULT_FONT), op.font_name, BUF_SIZE - 1, ini_path);
	op.left_font_size = profile_get_int(TEXT("font"), TEXT("left_font_size"), FONT_SIZE_M, ini_path);

	// キー情報
	op.key_info_count = profile_get_int(TEXT("key"), TEXT("count"), -1, ini_path);
	if (op.key_info_count == -1) {
		// Default
		op.key_save = FALSE;
		op.key_info = (KEY_INFO *)mem_calloc(sizeof(KEY_INFO) * 30);
		if (op.key_info == NULL) {
			return FALSE;
		}
		op.key_info_count = 0;

		op.key_info[op.key_info_count].action = ID_ACCEL_INPUT_SCORE;
		op.key_info[op.key_info_count].ctrl = 0;
		op.key_info[op.key_info_count].key = VK_F1;
		op.key_info_count++;
		op.key_info[op.key_info_count].action = ID_ACCEL_INPUT_SCORE + 26;
		op.key_info[op.key_info_count].ctrl = 0;
		op.key_info[op.key_info_count].key = VK_F2;
		op.key_info_count++;
		op.key_info[op.key_info_count].action = ID_ACCEL_INPUT_SCORE + 41;
		op.key_info[op.key_info_count].ctrl = 0;
		op.key_info[op.key_info_count].key = VK_F3;
		op.key_info_count++;
		op.key_info[op.key_info_count].action = ID_ACCEL_INPUT_SCORE + 45;
		op.key_info[op.key_info_count].ctrl = 0;
		op.key_info[op.key_info_count].key = VK_F4;
		op.key_info_count++;
		op.key_info[op.key_info_count].action = ID_ACCEL_INPUT_SCORE + 60;
		op.key_info[op.key_info_count].ctrl = 0;
		op.key_info[op.key_info_count].key = VK_F5;
		op.key_info_count++;
		op.key_info[op.key_info_count].action = ID_ACCEL_INPUT_SCORE + 81;
		op.key_info[op.key_info_count].ctrl = 0;
		op.key_info[op.key_info_count].key = VK_F6;
		op.key_info_count++;
		op.key_info[op.key_info_count].action = ID_ACCEL_INPUT_SCORE + 85;
		op.key_info[op.key_info_count].ctrl = 0;
		op.key_info[op.key_info_count].key = VK_F7;
		op.key_info_count++;
		op.key_info[op.key_info_count].action = ID_ACCEL_INPUT_SCORE + 100;
		op.key_info[op.key_info_count].ctrl = 0;
		op.key_info[op.key_info_count].key = VK_F8;
		op.key_info_count++;

		op.key_info[op.key_info_count].action = ID_ACCEL_INPUT_SCORE + 43;
		op.key_info[op.key_info_count].ctrl = FSHIFT;
		op.key_info[op.key_info_count].key = VK_F1;
		op.key_info_count++;
		op.key_info[op.key_info_count].action = ID_ACCEL_INPUT_SCORE + 55;
		op.key_info[op.key_info_count].ctrl = FSHIFT;
		op.key_info[op.key_info_count].key = VK_F2;
		op.key_info_count++;
		op.key_info[op.key_info_count].action = ID_ACCEL_INPUT_SCORE + 83;
		op.key_info[op.key_info_count].ctrl = FSHIFT;
		op.key_info[op.key_info_count].key = VK_F3;
		op.key_info_count++;
		op.key_info[op.key_info_count].action = ID_ACCEL_INPUT_SCORE + 95;
		op.key_info[op.key_info_count].ctrl = FSHIFT;
		op.key_info[op.key_info_count].key = VK_F4;
		op.key_info_count++;
		op.key_info[op.key_info_count].action = ID_ACCEL_INPUT_SCORE + 121;
		op.key_info[op.key_info_count].ctrl = FSHIFT;
		op.key_info[op.key_info_count].key = VK_F5;
		op.key_info_count++;
		op.key_info[op.key_info_count].action = ID_ACCEL_INPUT_SCORE + 125;
		op.key_info[op.key_info_count].ctrl = FSHIFT;
		op.key_info[op.key_info_count].key = VK_F6;
		op.key_info_count++;
		op.key_info[op.key_info_count].action = ID_ACCEL_INPUT_SCORE + 140;
		op.key_info[op.key_info_count].ctrl = FSHIFT;
		op.key_info[op.key_info_count].key = VK_F7;
		op.key_info_count++;
		op.key_info[op.key_info_count].action = ID_ACCEL_INPUT_SCORE + 180;
		op.key_info[op.key_info_count].ctrl = FSHIFT;
		op.key_info[op.key_info_count].key = VK_F8;
		op.key_info_count++;

		op.key_info[op.key_info_count].action = ID_MENUITEM_FINISH_ONE;
		op.key_info[op.key_info_count].ctrl = FCONTROL;
		op.key_info[op.key_info_count].key = '1';
		op.key_info_count++;
		op.key_info[op.key_info_count].action = ID_MENUITEM_FINISH_TWO;
		op.key_info[op.key_info_count].ctrl = FCONTROL;
		op.key_info[op.key_info_count].key = '2';
		op.key_info_count++;
		op.key_info[op.key_info_count].action = ID_MENUITEM_FINISH_THREE;
		op.key_info[op.key_info_count].ctrl = FCONTROL;
		op.key_info[op.key_info_count].key = '3';
		op.key_info_count++;
		op.key_info[op.key_info_count].action = ID_MENUITEM_MIDDLE;
		op.key_info[op.key_info_count].ctrl = FCONTROL;
		op.key_info[op.key_info_count].key = 'M';
		op.key_info_count++;
		op.key_info[op.key_info_count].action = ID_MENUITEM_SCORE_LEFT;
		op.key_info[op.key_info_count].ctrl = FCONTROL;
		op.key_info[op.key_info_count].key = 'T';
		op.key_info_count++;

		op.key_info[op.key_info_count].action = ID_MENUITEM_NEW_GAME;
		op.key_info[op.key_info_count].ctrl = FCONTROL;
		op.key_info[op.key_info_count].key = 'N';
		op.key_info_count++;
		op.key_info[op.key_info_count].action = ID_MENUITEM_GAME_HISTORY;
		op.key_info[op.key_info_count].ctrl = FCONTROL;
		op.key_info[op.key_info_count].key = 'H';
		op.key_info_count++;
		op.key_info[op.key_info_count].action = ID_MENUITEM_ARRANGE;
		op.key_info[op.key_info_count].ctrl = FCONTROL;
		op.key_info[op.key_info_count].key = 'A';
		op.key_info_count++;

		op.key_info[op.key_info_count].action = ID_MENUITEM_SCORE_LEFT;
		op.key_info[op.key_info_count].ctrl = 0;
		op.key_info[op.key_info_count].key = VK_F9;
		op.key_info_count++;
		op.key_info[op.key_info_count].action = ID_MENUITEM_MIDDLE;
		op.key_info[op.key_info_count].ctrl = FSHIFT;
		op.key_info[op.key_info_count].key = VK_F9;
		op.key_info_count++;
		op.key_info[op.key_info_count].action = ID_MENUITEM_FINISH_ONE;
		op.key_info[op.key_info_count].ctrl = 0;
		op.key_info[op.key_info_count].key = VK_F10;
		op.key_info_count++;
		op.key_info[op.key_info_count].action = ID_MENUITEM_FINISH_TWO;
		op.key_info[op.key_info_count].ctrl = 0;
		op.key_info[op.key_info_count].key = VK_F11;
		op.key_info_count++;
		op.key_info[op.key_info_count].action = ID_MENUITEM_FINISH_THREE;
		op.key_info[op.key_info_count].ctrl = 0;
		op.key_info[op.key_info_count].key = VK_F12;
		op.key_info_count++;
	} else {
		op.key_save = TRUE;
		op.key_info = (KEY_INFO *)mem_calloc(sizeof(KEY_INFO) * op.key_info_count);
		if (op.key_info == NULL) {
			return FALSE;
		}
		for (i = 0; i < op.key_info_count; i++) {
			wsprintf(buf, TEXT("action_%d"), i);
			op.key_info[i].action = profile_get_int(TEXT("key"), buf, 0, ini_path);
			wsprintf(buf, TEXT("ctrl_%d"), i);
			op.key_info[i].ctrl = profile_get_int(TEXT("key"), buf, 0, ini_path);
			wsprintf(buf, TEXT("key_%d"), i);
			op.key_info[i].key = profile_get_int(TEXT("key"), buf, 0, ini_path);
		}
	}

	// プレイヤー情報
	op.opi.large_font = profile_get_int(TEXT("player"), TEXT("large_font"), 1, ini_path);
	op.opi.scroll = profile_get_int(TEXT("player"), TEXT("scroll"), 1, ini_path);
	op.opi.avg_per_round = profile_get_int(TEXT("player"), TEXT("avg_per_round"), 1, ini_path);

	op.opi.name = profile_get_int(TEXT("player"), TEXT("name"), 1, ini_path);
	op.opi.first = profile_get_int(TEXT("player"), TEXT("first"), 1, ini_path);

	op.opi.total_sets = profile_get_int(TEXT("player"), TEXT("total_sets"), 1, ini_path);
	op.opi.total_legs = profile_get_int(TEXT("player"), TEXT("total_legs"), 1, ini_path);
	op.opi.total_tons = profile_get_int(TEXT("player"), TEXT("total_tons"), 0, ini_path);
	op.opi.total_100 = profile_get_int(TEXT("player"), TEXT("total_100"), 0, ini_path);
	op.opi.total_140 = profile_get_int(TEXT("player"), TEXT("total_140"), 0, ini_path);
	op.opi.total_180s = profile_get_int(TEXT("player"), TEXT("total_180s"), 0, ini_path);
	op.opi.total_high_off = profile_get_int(TEXT("player"), TEXT("total_high_off"), 0, ini_path);
	op.opi.total_short = profile_get_int(TEXT("player"), TEXT("total_short"), 0, ini_path);
	op.opi.total_long = profile_get_int(TEXT("player"), TEXT("total_long"), 0, ini_path);

	op.opi.avg_score = profile_get_int(TEXT("player"), TEXT("avg_score"), 1, ini_path);
	op.opi.avg_darts = profile_get_int(TEXT("player"), TEXT("avg_darts"), 0, ini_path);
	op.opi.avg_first9 = profile_get_int(TEXT("player"), TEXT("avg_first9"), 0, ini_path);
	op.opi.avg_check_out = profile_get_int(TEXT("player"), TEXT("avg_check_out"), 0, ini_path);
	op.opi.avg_check_out_count = profile_get_int(TEXT("player"), TEXT("avg_check_out_count"), 1, ini_path);
	op.opi.avg_keep = profile_get_int(TEXT("player"), TEXT("avg_keep"), 0, ini_path);
	op.opi.avg_keep_count = profile_get_int(TEXT("player"), TEXT("avg_keep_count"), 1, ini_path);
	op.opi.avg_break = profile_get_int(TEXT("player"), TEXT("avg_break"), 0, ini_path);
	op.opi.avg_break_count = profile_get_int(TEXT("player"), TEXT("avg_break_count"), 1, ini_path);

	op.opi.arrange = profile_get_int(TEXT("player"), TEXT("arrange"), 0, ini_path);

	// プラグイン情報
	op.plugin_info_count = profile_get_int(TEXT("plugin"), TEXT("count"), 0, ini_path);
	op.plugin_info = (PLUGIN_INFO *)mem_calloc(sizeof(PLUGIN_INFO) * op.plugin_info_count);
	if (op.plugin_info == NULL) {
		return FALSE;
	}
	for (i = 0; i < op.plugin_info_count; i++) {
		wsprintf(buf, TEXT("title_%d"), i);
		profile_get_string(TEXT("plugin"), buf, TEXT(""), op.plugin_info[i].title, BUF_SIZE - 1, ini_path);
		wsprintf(buf, TEXT("lib_file_path_%d"), i);
		profile_get_string(TEXT("plugin"), buf, TEXT(""), op.plugin_info[i].lib_file_path, BUF_SIZE - 1, ini_path);
		wsprintf(buf, TEXT("func_name_%d"), i);
		profile_get_string(TEXT("plugin"), buf, TEXT(""), op.plugin_info[i].func_name, BUF_SIZE - 1, ini_path);
		wsprintf(buf, TEXT("cmd_line_%d"), i);
		profile_get_string(TEXT("plugin"), buf, TEXT(""), op.plugin_info[i].pei.cmd_line, BUF_SIZE - 1, ini_path);

		wsprintf(buf, TEXT("ctrl_%d"), i);
		op.plugin_info[i].ctrl = profile_get_int(TEXT("plugin"), buf, 0, ini_path);
		wsprintf(buf, TEXT("key_%d"), i);
		op.plugin_info[i].key = profile_get_int(TEXT("plugin"), buf, 0, ini_path);
	}
	profile_free();
	return TRUE;
}

/*
 * put_color - 色情報を書きこむ
 */
static void put_color(const TCHAR *ini_path, const TCHAR *key, const COLORREF color, const COLORREF def_color)
{
	TCHAR buf[BUF_SIZE];

	if (def_color == color) {
		profile_write_string(TEXT("color"), key, TEXT(""), ini_path);
	} else {
		wsprintf(buf, TEXT("0x%06lX"), color);
		profile_write_string(TEXT("color"), key, buf, ini_path);
	}
}

/*
 * ini_put_option - オプションを書きこむ
 */
BOOL ini_put_option(const TCHAR *ini_path)
{
	TCHAR buf[BUF_SIZE];
	int i;

	profile_initialize(ini_path, TRUE);

	profile_write_int(TEXT("window"), TEXT("left"), op.window_rect.left, ini_path);
	profile_write_int(TEXT("window"), TEXT("top"), op.window_rect.top, ini_path);
	profile_write_int(TEXT("window"), TEXT("right"), op.window_rect.right, ini_path);
	profile_write_int(TEXT("window"), TEXT("bottom"), op.window_rect.bottom, ini_path);
	profile_write_int(TEXT("window"), TEXT("state"), op.window_state, ini_path);
	profile_write_int(TEXT("window"), TEXT("full_screen"), op.full_screen, ini_path);
	profile_write_int(TEXT("window"), TEXT("full_screen_menu_interval"), op.full_screen_menu_interval, ini_path);

	profile_write_int(TEXT("view"), TEXT("name"), op.view_name, ini_path);
	profile_write_int(TEXT("view"), TEXT("name_size"), op.view_name_size, ini_path);
	profile_write_int(TEXT("view"), TEXT("player"), op.view_player, ini_path);
	profile_write_int(TEXT("view"), TEXT("left"), op.view_left, ini_path);
	profile_write_int(TEXT("view"), TEXT("guide"), op.view_guide, ini_path);
	profile_write_int(TEXT("view"), TEXT("throw_count"), op.view_throw_count, ini_path);
	profile_write_int(TEXT("view"), TEXT("ton_circle"), op.view_ton_circle, ini_path);
	profile_write_int(TEXT("view"), TEXT("separate"), op.view_separate, ini_path);
	profile_write_int(TEXT("view"), TEXT("scroll_bar"), op.view_scroll_bar, ini_path);
	profile_write_int(TEXT("view"), TEXT("view_separate_width"), op.view_separate_width, ini_path);

	// 自動保存
	profile_write_int(TEXT("game"), TEXT("auto_save"), op.auto_save, ini_path);
	profile_write_string(TEXT("game"), TEXT("auto_save_path"), op.auto_save_path, ini_path);

	// チェックアウトモード
	profile_write_int(TEXT("game"), TEXT("check_out_mode"), op.check_out_mode, ini_path);

	// リカバリモード
	profile_write_int(TEXT("game"), TEXT("recovery"), op.recovery, ini_path);
	profile_write_int(TEXT("game"), TEXT("recovery_delete"), op.recovery_delete, ini_path);

	// 名前リスト
	profile_write_int(TEXT("game"), TEXT("name_list_count"), op.name_list_count, ini_path);
	for (i = 0; i < op.name_list_count; i++) {
		wsprintf(buf, TEXT("name_list_%d"), i);
		profile_write_string(TEXT("game"), buf, op.name_list[i], ini_path);
	}

	// com
	profile_write_int(TEXT("com"), TEXT("default_number"), op.com_default_number, ini_path);
	profile_write_int(TEXT("com"), TEXT("default_segment"), op.com_default_segment, ini_path);
	profile_write_int(TEXT("com"), TEXT("com_timer"), op.com_timer, ini_path);

	// ゲーム履歴
	profile_write_int(TEXT("history"), TEXT("left"), op.history_rect.left, ini_path);
	profile_write_int(TEXT("history"), TEXT("top"), op.history_rect.top, ini_path);
	profile_write_int(TEXT("history"), TEXT("right"), op.history_rect.right, ini_path);
	profile_write_int(TEXT("history"), TEXT("bottom"), op.history_rect.bottom, ini_path);
	// スコア一覧
	profile_write_int(TEXT("score_list"), TEXT("left"), op.score_list_rect.left, ini_path);
	profile_write_int(TEXT("score_list"), TEXT("top"), op.score_list_rect.top, ini_path);
	profile_write_int(TEXT("score_list"), TEXT("right"), op.score_list_rect.right, ini_path);
	profile_write_int(TEXT("score_list"), TEXT("bottom"), op.score_list_rect.bottom, ini_path);

	// アレンジ
	profile_write_int(TEXT("arrange"), TEXT("left"), op.arrange_rect.left, ini_path);
	profile_write_int(TEXT("arrange"), TEXT("top"), op.arrange_rect.top, ini_path);
	profile_write_int(TEXT("arrange"), TEXT("right"), op.arrange_rect.right, ini_path);
	profile_write_int(TEXT("arrange"), TEXT("bottom"), op.arrange_rect.bottom, ini_path);

	// 色情報
	put_color(ini_path, TEXT("view_separate"), op.ci.view_separate, D_COLOR_VIEW_SEPARATE);

	put_color(ini_path, TEXT("background"), op.ci.background, D_COLOR_BACKGROUND);
	put_color(ini_path, TEXT("odd_background"), op.ci.odd_background, D_COLOR_ODD_BACKGROUND);
	put_color(ini_path, TEXT("scored_text"), op.ci.scored_text, D_COLOR_TEXT);
	put_color(ini_path, TEXT("text"), op.ci.togo_text, D_COLOR_TEXT);
	put_color(ini_path, TEXT("header_background"), op.ci.header_background, D_COLOR_HEADER_BACKGROUND);
	put_color(ini_path, TEXT("header_text"), op.ci.header_text, D_COLOR_HEADER_TEXT);
	put_color(ini_path, TEXT("last3number_text"), op.ci.last3number_text, D_COLOR_LAST3NUMBER_TEXT);
	put_color(ini_path, TEXT("line"), op.ci.line, D_COLOR_LINE);
	put_color(ini_path, TEXT("separate"), op.ci.separate, D_COLOR_SEPARATE);
	put_color(ini_path, TEXT("ton_circle"), op.ci.ton_circle, D_COLOR_TON_CIRCLE);

	put_color(ini_path, TEXT("input_background"), op.ci.input_background, D_COLOR_INPUT_BACKGROUND);
	put_color(ini_path, TEXT("input_text"), op.ci.input_text, D_COLOR_INPUT_TEXT);
	put_color(ini_path, TEXT("input_select_background"), op.ci.input_select_background, D_COLOR_INPUT_SELECT_BACKGROUND);
	put_color(ini_path, TEXT("input_select_text"), op.ci.input_select_text, D_COLOR_INPUT_SELECT_TEXT);

	put_color(ini_path, TEXT("left_background"), op.ci.left_background, D_COLOR_LEFT_BACKGROUND);
	put_color(ini_path, TEXT("left_text"), op.ci.left_text, D_COLOR_LEFT_TEXT);
	put_color(ini_path, TEXT("left_active_border"), op.ci.left_active_border, D_COLOR_LEFT_ACTIVE_BORDER);

	put_color(ini_path, TEXT("player_name_background"), op.ci.player_name_background, D_COLOR_PLAYER_NAME_BACKGROUND);
	put_color(ini_path, TEXT("player_name_text"), op.ci.player_name_text, D_COLOR_PLAYER_NAME_TEXT);
	put_color(ini_path, TEXT("player_background"), op.ci.player_background, D_COLOR_PLAYER_BACKGROUND);
	put_color(ini_path, TEXT("player_text"), op.ci.player_text, D_COLOR_PLAYER_TEXT);
	put_color(ini_path, TEXT("player_info_title"), op.ci.player_info_title, D_COLOR_PLAYER_INFO_TITLE);

	put_color(ini_path, TEXT("guide_background"), op.ci.guide_background, D_COLOR_GUIDE_BACKGROUND);
	put_color(ini_path, TEXT("guide_text"), op.ci.guide_text, D_COLOR_GUIDE_TEXT);
	put_color(ini_path, TEXT("guide_select_background"), op.ci.guide_select_background, D_COLOR_GUIDE_SELECT_BACKGROUND);
	put_color(ini_path, TEXT("guide_select_text"), op.ci.guide_select_text, D_COLOR_GUIDE_SELECT_TEXT);
	put_color(ini_path, TEXT("guide_box_background"), op.ci.guide_box_background, D_COLOR_GUIDE_BOX_BACKGROUND);
	put_color(ini_path, TEXT("guide_box_text"), op.ci.guide_box_text, D_COLOR_GUIDE_BOX_TEXT);
	put_color(ini_path, TEXT("guide_box_frame"), op.ci.guide_box_frame, D_COLOR_GUIDE_BOX_FRAME);
	put_color(ini_path, TEXT("guide_box_disable"), op.ci.guide_box_disable, D_COLOR_GUIDE_BOX_DISABLE);

	// フォント情報
	profile_write_string(TEXT("font"), TEXT("name"), op.font_name, ini_path);
	profile_write_int(TEXT("font"), TEXT("left_font_size"), op.left_font_size, ini_path);

	// キー情報
	if (op.key_save == TRUE) {
		profile_write_int(TEXT("key"), TEXT("count"), op.key_info_count, ini_path);
		for (i = 0; i < op.key_info_count; i++) {
			wsprintf(buf, TEXT("action_%d"), i);
			profile_write_int(TEXT("key"), buf, op.key_info[i].action, ini_path);
			wsprintf(buf, TEXT("ctrl_%d"), i);
			profile_write_int(TEXT("key"), buf, op.key_info[i].ctrl, ini_path);
			wsprintf(buf, TEXT("key_%d"), i);
			profile_write_int(TEXT("key"), buf, op.key_info[i].key, ini_path);
		}
	}

	// プレイヤー情報
	profile_write_int(TEXT("player"), TEXT("large_font"), op.opi.large_font, ini_path);
	profile_write_int(TEXT("player"), TEXT("scroll"), op.opi.scroll, ini_path);
	profile_write_int(TEXT("player"), TEXT("avg_per_round"), op.opi.avg_per_round, ini_path);

	profile_write_int(TEXT("player"), TEXT("name"), op.opi.name, ini_path);
	profile_write_int(TEXT("player"), TEXT("first"), op.opi.first, ini_path);

	profile_write_int(TEXT("player"), TEXT("total_sets"), op.opi.total_sets, ini_path);
	profile_write_int(TEXT("player"), TEXT("total_legs"), op.opi.total_legs, ini_path);
	profile_write_int(TEXT("player"), TEXT("total_tons"), op.opi.total_tons, ini_path);
	profile_write_int(TEXT("player"), TEXT("total_100"), op.opi.total_100, ini_path);
	profile_write_int(TEXT("player"), TEXT("total_140"), op.opi.total_140, ini_path);
	profile_write_int(TEXT("player"), TEXT("total_180s"), op.opi.total_180s, ini_path);
	profile_write_int(TEXT("player"), TEXT("total_high_off"), op.opi.total_high_off, ini_path);
	profile_write_int(TEXT("player"), TEXT("total_short"), op.opi.total_short, ini_path);
	profile_write_int(TEXT("player"), TEXT("total_long"), op.opi.total_long, ini_path);

	profile_write_int(TEXT("player"), TEXT("avg_score"), op.opi.avg_score, ini_path);
	profile_write_int(TEXT("player"), TEXT("avg_darts"), op.opi.avg_darts, ini_path);
	profile_write_int(TEXT("player"), TEXT("avg_first9"), op.opi.avg_first9, ini_path);
	profile_write_int(TEXT("player"), TEXT("avg_check_out"), op.opi.avg_check_out, ini_path);
	profile_write_int(TEXT("player"), TEXT("avg_check_out_count"), op.opi.avg_check_out_count, ini_path);
	profile_write_int(TEXT("player"), TEXT("avg_keep"), op.opi.avg_keep, ini_path);
	profile_write_int(TEXT("player"), TEXT("avg_keep_count"), op.opi.avg_keep_count, ini_path);
	profile_write_int(TEXT("player"), TEXT("avg_break"), op.opi.avg_break, ini_path);
	profile_write_int(TEXT("player"), TEXT("avg_break_count"), op.opi.avg_break_count, ini_path);

	profile_write_int(TEXT("player"), TEXT("arrange"), op.opi.arrange, ini_path);

	// プラグイン情報
	profile_write_int(TEXT("plugin"), TEXT("count"), op.plugin_info_count, ini_path);
	for (i = 0; i < op.plugin_info_count; i++) {
		wsprintf(buf, TEXT("title_%d"), i);
		profile_write_string(TEXT("plugin"), buf, op.plugin_info[i].title, ini_path);
		wsprintf(buf, TEXT("lib_file_path_%d"), i);
		profile_write_string(TEXT("plugin"), buf, op.plugin_info[i].lib_file_path, ini_path);
		wsprintf(buf, TEXT("func_name_%d"), i);
		profile_write_string(TEXT("plugin"), buf, op.plugin_info[i].func_name, ini_path);
		wsprintf(buf, TEXT("cmd_line_%d"), i);
		profile_write_string(TEXT("plugin"), buf, op.plugin_info[i].pei.cmd_line, ini_path);

		wsprintf(buf, TEXT("ctrl_%d"), i);
		profile_write_int(TEXT("plugin"), buf, op.plugin_info[i].ctrl, ini_path);
		wsprintf(buf, TEXT("key_%d"), i);
		profile_write_int(TEXT("plugin"), buf, op.plugin_info[i].key, ini_path);
	}
	profile_flush(ini_path);
	profile_free();
	return TRUE;
}

/*
 * ini_put_game_option - オプションを書きこむ
 */
BOOL ini_put_game_option(const TCHAR *ini_path)
{
	profile_initialize(ini_path, TRUE);

	profile_write_int(TEXT("game"), TEXT("start_score"), op.gi.start_score, ini_path);
	profile_write_int(TEXT("game"), TEXT("round_limit"), op.gi.round_limit, ini_path);
	profile_write_int(TEXT("game"), TEXT("round"), op.gi.round, ini_path);

	profile_write_int(TEXT("game"), TEXT("leg_limit"), op.gi.leg_limit, ini_path);
	profile_write_int(TEXT("game"), TEXT("max_leg"), op.gi.max_leg, ini_path);
	profile_write_int(TEXT("game"), TEXT("best_of"), op.gi.best_of, ini_path);
	profile_write_int(TEXT("game"), TEXT("change_first"), op.gi.change_first, ini_path);

	profile_write_string(TEXT("game"), TEXT("p1_name"), op.gi.player_name[0], ini_path);
	profile_write_int(TEXT("game"), TEXT("p1_start_score"), op.gi.player_start_score[0], ini_path);
	profile_write_int(TEXT("game"), TEXT("p1_com"), op.gi.com[0], ini_path);
	profile_write_int(TEXT("game"), TEXT("p1_com_level"), op.gi.level[0], ini_path);

	profile_write_string(TEXT("game"), TEXT("p2_name"), op.gi.player_name[1], ini_path);
	profile_write_int(TEXT("game"), TEXT("p2_start_score"), op.gi.player_start_score[1], ini_path);
	profile_write_int(TEXT("game"), TEXT("p2_com"), op.gi.com[1], ini_path);
	profile_write_int(TEXT("game"), TEXT("p2_com_level"), op.gi.level[1], ini_path);

	profile_flush(ini_path);
	profile_free();
	return TRUE;
}

/*
 * ini_put_game_schedule - オプションを書きこむ
 */
BOOL ini_put_game_schedule(const TCHAR *ini_path)
{
	TCHAR buf[BUF_SIZE];
	int i;

	profile_initialize(ini_path, TRUE);

	profile_write_int(TEXT("schedule"), TEXT("count"), op.gi_list_count, ini_path);
	for (i = 0; i < op.gi_list_count; i++) {
		wsprintf(buf, TEXT("start_score_%d"), i);
		profile_write_int(TEXT("schedule"), buf, op.gi_list[i].start_score, ini_path);

		wsprintf(buf, TEXT("round_limit_%d"), i);
		profile_write_int(TEXT("schedule"), buf, op.gi_list[i].round_limit, ini_path);
		wsprintf(buf, TEXT("round_%d"), i);
		profile_write_int(TEXT("schedule"), buf, op.gi_list[i].round, ini_path);

		wsprintf(buf, TEXT("max_leg_%d"), i);
		profile_write_int(TEXT("schedule"), buf, op.gi_list[i].max_leg, ini_path);
		wsprintf(buf, TEXT("best_of_%d"), i);
		profile_write_int(TEXT("schedule"), buf, op.gi_list[i].best_of, ini_path);
		wsprintf(buf, TEXT("change_first_%d"), i);
		profile_write_int(TEXT("schedule"), buf, op.gi_list[i].change_first, ini_path);

		wsprintf(buf, TEXT("p1_name_%d"), i);
		profile_write_string(TEXT("schedule"), buf, op.gi_list[i].player_name[0], ini_path);
		wsprintf(buf, TEXT("p1_start_score_%d"), i);
		profile_write_int(TEXT("schedule"), buf, op.gi_list[i].player_start_score[0], ini_path);
		wsprintf(buf, TEXT("p1_com_%d"), i);
		profile_write_int(TEXT("schedule"), buf, op.gi_list[i].com[0], ini_path);
		wsprintf(buf, TEXT("p1_com_level_%d"), i);
		profile_write_int(TEXT("schedule"), buf, op.gi_list[i].level[0], ini_path);

		wsprintf(buf, TEXT("p2_name_%d"), i);
		profile_write_string(TEXT("schedule"), buf, op.gi_list[i].player_name[1], ini_path);
		wsprintf(buf, TEXT("p2_start_score_%d"), i);
		profile_write_int(TEXT("schedule"), buf, op.gi_list[i].player_start_score[1], ini_path);
		wsprintf(buf, TEXT("p2_com_%d"), i);
		profile_write_int(TEXT("schedule"), buf, op.gi_list[i].com[1], ini_path);
		wsprintf(buf, TEXT("p2_com_level_%d"), i);
		profile_write_int(TEXT("schedule"), buf, op.gi_list[i].level[1], ini_path);
	}

	profile_flush(ini_path);
	profile_free();
	return TRUE;
}
/* End of source */
