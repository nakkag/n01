/*
 * n01
 *
 * n01_plugin.h Ver 0.2.7
 *
 * Copyright (C) 1996-2017 by Ohno Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

#ifndef _INC_N01_PLUGIN_H
#define _INC_N01_PLUGIN_H

/* Include Files */
#define _INC_OLE
#include <windows.h>
#undef  _INC_OLE

/* Define */
#ifndef BUF_SIZE
#define BUF_SIZE						256
#endif

#define NAME_SIZE						64
#define MAX_ROUND						60
#define ALLOC_ROUND						10
#define NAME_LIST_COUNT					64
#define THROW_LIST_SIZE					10

// typedef
typedef short TYPE_SCORE;
typedef char TYPE_CHECK_OUT;

// general
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

// �v���O�C�������s����^�C�~���O
#define CALLTYPE_START					1				// �v���O�����J�n
#define CALLTYPE_END					2				// �v���O�����I��
#define CALLTYPE_MENU					4				// ���j���[
#define CALLTYPE_SET_START				8				// �Z�b�g�J�n
#define CALLTYPE_SET_END				16				// �Z�b�g�I��
#define CALLTYPE_LEG_START				32				// ���b�O�J�n
#define CALLTYPE_LEG_END				64				// ���b�O�I��
#define CALLTYPE_INPUT_START			128				// �X�R�A���͊J�n
#define CALLTYPE_INPUT_END				256				// �X�R�A���͊m��
#define CALLTYPE_FINISH					512				// �t�B�j�b�V��
#define CALLTYPE_PREV_LEG				1024			// �O�̃��b�O�ɖ߂�

// �v���O�C���߂�l
#define PLUGIN_ERROR					0				// �v���O�C���̃G���[
#define PLUGIN_SUCCEED					1				// �v���O�C���̐���I��
#define PLUGIN_CANCEL					2				// �ȍ~�̏������L�����Z�� (CALLTYPE_END, CALLTYPE_INPUT_END, CALLTYPE_FINISH)
#define PLUGIN_DATA_MODIFIED			4				// �f�[�^�ύX���� (CALLTYPE_INPUT_END, CALLTYPE_FINISH)

/* Struct */
// �v���O�C���擾���
typedef struct _PLUGIN_GET_INFO {
	DWORD struct_size;					// �\���̂̃T�C�Y

	TCHAR title[BUF_SIZE];
	TCHAR func_name[BUF_SIZE];
	TCHAR cmd_line[BUF_SIZE];
} PLUGIN_GET_INFO;

// �v���O�C�����s���
typedef struct _PLUGIN_EXEC_INFO {
	DWORD struct_size;					// �\���̂̃T�C�Y

	TCHAR cmd_line[BUF_SIZE];			// �v���O�C���ݒ�Ŏw�肵���R�}���h���C�� (�I�����ɕۑ������)
	LPARAM lParam1;						// �v���O�C���ɑΉ�����long�l
	LPARAM lParam2;						// �v���O�C���ɑΉ�����long�l

	struct _SCORE_INFO *si;
} PLUGIN_EXEC_INFO;

// �E�B���h�E��� (WM_N01_GET_WINDOW)
typedef struct _WINDOW_INFO {
	HWND hWnd;

	HWND score_list_wnd;
	HWND score_left_wnd[2];
	HWND score_player_wnd[2];
	HWND score_guide_wnd;
	// �ǉ�����
	HWND score_name_wnd;
} WINDOW_INFO;

// �Q�[����� (WM_N01_GET_GAME_OPTION)
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

// �A�����W���
typedef struct _ARRANGE_INFO {
	int left;
	TCHAR throw_list[5][THROW_LIST_SIZE + 1];
} ARRANGE_INFO;

// �F���
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

	// Ver 0.2.2
	COLORREF view_separate;

	// Ver 0.2.4
	COLORREF guide_background;
	COLORREF guide_text;
	COLORREF guide_select_background;
	COLORREF guide_select_text;
	COLORREF guide_box_background;
	COLORREF guide_box_text;
	COLORREF guide_box_frame;
	COLORREF guide_box_disable;
} COLOR_INFO;

// �L�[���
typedef struct _KEY_INFO {
	int action;
	int ctrl;
	int key;
} KEY_INFO;

// �v���C���[�I�v�V�������
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

// �v���O�C�����
typedef struct _PLUGIN_INFO {
	TCHAR title[BUF_SIZE];
	TCHAR lib_file_path[BUF_SIZE];
	TCHAR func_name[BUF_SIZE];
	int call_type;						// CALLTYPE_

	HANDLE lib;
	FARPROC func;

	PLUGIN_EXEC_INFO pei;

	int index;

	int ctrl;
	int key;
} PLUGIN_INFO;

// �I�v�V������� (WM_N01_GET_OPTION)
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

	// �ǉ�����
	int recovery_delete;
	int left_font_size;
	int com_timer;
	int view_name;
	int view_name_size;
	BOOL full_screen;
	int full_screen_menu_interval;
	int view_separate_width;
} OPTION_INFO;

// ���v���
typedef struct _STATISTICS_INFO {
	int ton_count;
	int ton00_count;
	int ton20_count;
	int ton40_count;
	int ton80_count;
	TYPE_SCORE high_off;
	int short_game;
	int long_game;

	// �_�[�c1�{������̕���
	int all_score;
	int all_darts;
	// �_�[�c���̕���
	int win_darts;
	int win_count;
	// 3���E���h�̕���
	int first9_score;
	int first9_darts;
	// �`�F�b�N�A�E�g��
	int check_out_aim;
	int check_out;
	// ��U�ŏ��������b�O��
	int all_keep_legs;
	int win_keep_legs;
	// ��U�ŏ��������b�O��
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

// �v���C���[���
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

// ���b�O���
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

// �X�R�A��� (WM_N01_GET_SCORE_INFO)
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

// �X�R�A���� (WM_N01_GET_SCORE_HISTORY)
typedef struct _SCORE_HISTORY {
	SCORE_INFO *list;
	int list_count;

	SCORE_INFO *si;
} SCORE_HISTORY;

#endif
/* End of source */
