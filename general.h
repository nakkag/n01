/*
 * n01
 *
 * general.h
 *
 * Copyright (C) 1996-2017 by Ohno Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

#ifndef _INC_N01_GENERAL_H
#define _INC_N01_GENERAL_H

/* Include Files */
#include "arrange.h"
#include "plugin.h"

/* Define */
#define BUF_SIZE						256
#define NAME_SIZE						64

#define APP_NAME						TEXT("n01 Ver 0.2.7")
#define APP_VERSION						27

#define WM_WINDOW_REDRAW				(WM_APP + 1)
#define WM_WINDOW_SHOW_MENU				(WM_APP + 2)
#define WM_WINDOW_SET_FIRST				(WM_APP + 3)
#define WM_WINDOW_SET_KEY				(WM_APP + 4)
#define WM_WINDOW_NEXT_SET				(WM_APP + 5)
#define WM_WINDOW_CLEAR_LIST			(WM_APP + 6)
#define WM_WINDOW_DELETE_LIST			(WM_APP + 7)
#define WM_WINDOW_SET_CURRENT			(WM_APP + 8)
#define WM_WINDOW_SET_PREV_MENU			(WM_APP + 9)

#define WM_N01_GET_VERSION				(WM_APP + 500)
#define WM_N01_GET_OPTION				(WM_APP + 510)
#define WM_N01_GET_GAME_OPTION			(WM_APP + 511)
#define WM_N01_GET_SCORE_INFO			(WM_APP + 512)
#define WM_N01_GET_SCORE_HISTORY		(WM_APP + 513)
#define WM_N01_GET_SCORE_BUFFER			(WM_APP + 514)
#define WM_N01_GET_INPUT_POS			(WM_APP + 515)
#define WM_N01_GET_WINDOW				(WM_APP + 516)
#define WM_N01_GET_CSV					(WM_APP + 520)
#define WM_N01_FREE_CSV					(WM_APP + 521)
#define WM_N01_REFRESH					(WM_APP + 530)
#define WM_N01_GAME_ON					(WM_APP + 531)
#define WM_N01_SCORE_INPUT				(WM_APP + 532)
#define WM_N01_SET_INPUT_POS			(WM_APP + 540)

#define IDPCANCEL						(WM_APP + 1000)

#define MAX_ROUND						60
#define ALLOC_ROUND						10
#define NAME_LIST_COUNT					64

#define ID_ACCEL_INPUT_SCORE			51000
#define ID_MENUITEM_TOOL				(WM_APP + 1000)

// Color
#define D_COLOR_VIEW_SEPARATE			RGB(0, 0, 0)

#define D_COLOR_BACKGROUND				RGB(255, 255, 255)
#define D_COLOR_ODD_BACKGROUND			RGB(255, 255, 255)
#define D_COLOR_TEXT					RGB(0, 0, 0)
#define D_COLOR_HEADER_BACKGROUND		RGB(201, 201, 201)
#define D_COLOR_HEADER_TEXT				RGB(0, 0, 0)
#define D_COLOR_LAST3NUMBER_TEXT		RGB(128, 0, 0)
#define D_COLOR_LINE					RGB(85, 85, 85)
#define D_COLOR_SEPARATE				RGB(0, 0, 0)
#define D_COLOR_TON_CIRCLE				RGB(160, 160, 160)

#define D_COLOR_INPUT_BACKGROUND		RGB(255, 255, 128)
#define D_COLOR_INPUT_TEXT				RGB(0, 0, 0)
#define D_COLOR_INPUT_SELECT_BACKGROUND	RGB(10, 36, 106)
#define D_COLOR_INPUT_SELECT_TEXT		RGB(255, 255, 255)

#define D_COLOR_LEFT_BACKGROUND			RGB(255, 255, 255)
#define D_COLOR_LEFT_TEXT				RGB(0, 0, 0)
#define D_COLOR_LEFT_ACTIVE_BORDER		RGB(10, 36, 106)

#define D_COLOR_PLAYER_NAME_BACKGROUND	RGB(10, 36, 106)
#define D_COLOR_PLAYER_NAME_TEXT		RGB(255, 255, 255)
#define D_COLOR_PLAYER_BACKGROUND		RGB(201, 201, 201)
#define D_COLOR_PLAYER_TEXT				RGB(0, 0, 0)
#define D_COLOR_PLAYER_INFO_TITLE		RGB(10, 36, 106)

#define D_COLOR_GUIDE_BACKGROUND		RGB(201, 201, 201)
#define D_COLOR_GUIDE_TEXT				RGB(0, 0, 0)
#define D_COLOR_GUIDE_SELECT_BACKGROUND	RGB(10, 36, 106)
#define D_COLOR_GUIDE_SELECT_TEXT		RGB(255, 255, 255)
#define D_COLOR_GUIDE_BOX_BACKGROUND	RGB(255, 255, 255)
#define D_COLOR_GUIDE_BOX_TEXT			RGB(0, 0, 0)
#define D_COLOR_GUIDE_BOX_FRAME			RGB(0, 0, 0)
#define D_COLOR_GUIDE_BOX_DISABLE		RGB(128, 128, 128)

// Font
#define FONT_SIZE_L						20
#define FONT_SIZE_M						40
#define FONT_SIZE_S						60

// typedef
typedef short TYPE_SCORE;
typedef char TYPE_CHECK_OUT;

/* Struct */
// オプション
typedef struct _GAME_INFO {
	int start_score;

	int round_limit;
	int round;

	int leg_limit;
	int max_leg;
	int best_of;

	TCHAR player_name[2][NAME_SIZE];
	int player_start_score[2];
	BOOL com[2];
	int level[2];

	BOOL schedule_flag;
	BOOL change_first;
} GAME_INFO;

typedef struct _COLOR_INFO {
	COLORREF background;
	COLORREF odd_background;
	COLORREF scored_text;
	COLORREF togo_text;
	COLORREF header_background;
	COLORREF header_text;
	COLORREF last3number_text;
	COLORREF line;
	COLORREF separate;
	COLORREF ton_circle;

	COLORREF input_background;
	COLORREF input_text;
	COLORREF input_select_background;
	COLORREF input_select_text;

	COLORREF left_background;
	COLORREF left_text;
	COLORREF left_active_border;

	COLORREF player_name_background;
	COLORREF player_name_text;
	COLORREF player_background;
	COLORREF player_text;
	COLORREF player_info_title;

	COLORREF view_separate;

	COLORREF guide_background;
	COLORREF guide_text;
	COLORREF guide_select_background;
	COLORREF guide_select_text;
	COLORREF guide_box_background;
	COLORREF guide_box_text;
	COLORREF guide_box_frame;
	COLORREF guide_box_disable;
} COLOR_INFO;

typedef struct _KEY_INFO {
	int action;
	int ctrl;
	int key;
} KEY_INFO;

typedef struct _OP_PLAYER_INFO {
	int name;
	int first;

	int total_sets;
	int total_legs;
	int total_tons;
	int total_100;
	int total_140;
	int total_180s;
	int total_high_off;
	int total_short;
	int total_long;

	int avg_score;
	int avg_darts;
	int avg_first9;
	int avg_check_out;
	int avg_check_out_count;
	int avg_keep;
	int avg_keep_count;
	int avg_break;
	int avg_break_count;

	int arrange;

	int large_font;
	int scroll;
	int avg_per_round;
} OP_PLAYER_INFO;

typedef struct _OPTION_INFO {
	RECT window_rect;
	int window_state;

	int view_player;
	int view_left;
	int view_guide;
	int view_throw_count;
	int view_ton_circle;
	int view_separate;
	int view_scroll_bar;

	int auto_save;
	TCHAR auto_save_path[BUF_SIZE];
	int check_out_mode;
	int recovery;

	int check_out_font_size;

	// com
	int com_default_number;
	int com_default_segment;

	// Game option
	GAME_INFO gi;
	GAME_INFO *gi_list;
	int gi_list_count;
	TCHAR name_list[NAME_LIST_COUNT][NAME_SIZE];
	int name_list_count;

	// Game Histroy
	RECT history_rect;
	RECT score_list_rect;

	// Arrange
	RECT arrange_rect;
	ARRANGE_INFO *arrange_info;
	int arrange_info_count;
	ARRANGE_INFO *com_arrange_info;
	int com_arrange_info_count;

	// Option view
	COLOR_INFO ci;
	TCHAR font_name[BUF_SIZE];

	// Option key
	KEY_INFO *key_info;
	int key_info_count;
	BOOL key_save;

	// Option player
	OP_PLAYER_INFO opi;

	// Plug-in
	PLUGIN_INFO *plugin_info;
	int plugin_info_count;

	// 追加項目
	int recovery_delete;
	int left_font_size;
	int com_timer;
	int view_name;
	int view_name_size;
	BOOL full_screen;
	int full_screen_menu_interval;
	int view_separate_width;
} OPTION_INFO;

// スコア情報
typedef struct _STATISTICS_INFO {
	int ton_count;
	int ton00_count;
	int ton20_count;
	int ton40_count;
	int ton80_count;
	TYPE_SCORE high_off;
	int short_game;
	int long_game;

	// ダーツ1本あたりの平均
	int all_score;
	int all_darts;
	// ダーツ数の平均
	int win_darts;
	int win_count;
	// 3ラウンドの平均
	int first9_score;
	int first9_darts;
	// チェックアウト率
	int check_out_aim;
	int check_out;
	// 先攻で勝ったレッグ数
	int all_keep_legs;
	int win_keep_legs;
	// 後攻で勝ったレッグ数
	int all_break_legs;
	int win_break_legs;

	// Finish stats
	int success_2_80;
	int failure_2_80;
	int success_81_130;
	int failure_81_130;
	int success_131;
	int failure_131;
} STATISTICS_INFO;

typedef struct _PLAYER_INFO {
	TCHAR name[NAME_SIZE];

	int start_score;
	int left;

	int sets;
	int legs;

	STATISTICS_INFO stat;
	STATISTICS_INFO set_stat;

	BOOL lock;
	BOOL check_out_mode;

	BOOL com;
	int level;
	TYPE_SCORE com_score;
} PLAYER_INFO;

typedef struct _LEG_INFO {
	int first;

	int current_round;
	int current_player;

	int max_round;
	int alloc_round;

	TYPE_SCORE *score[2];

	BOOL end_flag;
	int winner;
	int darts;
	TYPE_SCORE out_left;

	int ton_count[2];
	int ton00_count[2];
	int ton20_count[2];
	int ton40_count[2];
	int ton80_count[2];
	int all_score[2];
	int all_darts[2];
	int first9_score[2];
	int first9_darts[2];
	int check_out_aim[2];
	int failure_2_80[2];
	int failure_81_130[2];
	int failure_131[2];
} LEG_INFO;

typedef struct _SCORE_INFO {
	BOOL set_mode;
	BOOL history;

	int start_score;

	int current_set;
	int current_leg;

	int leg_limit;
	int max_leg;
	int best_of;

	int round_limit;
	int round;

	SYSTEMTIME start_time;

	PLAYER_INFO player[2];
	LEG_INFO *leg;
	TYPE_CHECK_OUT *tmp_check_out[2];
} SCORE_INFO;

typedef struct _SCORE_HISTORY {
	SCORE_INFO *list;
	int list_count;

	SCORE_INFO *si;
} SCORE_HISTORY;

#endif
/* End of source */
