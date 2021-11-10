/*
 * n01
 *
 * plugin.h
 *
 * Copyright (C) 1996-2006 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

#ifndef _INC_PLUGIN_H
#define _INC_PLUGIN_H

/* Include Files */

/* Define */
#define BUF_SIZE						256

#define CALLTYPE_START					1				// プログラム開始
#define CALLTYPE_END					2				// プログラム終了
#define CALLTYPE_MENU					4				// メニュー
#define CALLTYPE_SET_START				8				// セット開始
#define CALLTYPE_SET_END				16				// セット終了
#define CALLTYPE_LEG_START				32				// レッグ開始
#define CALLTYPE_LEG_END				64				// レッグ終了
#define CALLTYPE_INPUT_START			128				// スコア入力開始
#define CALLTYPE_INPUT_END				256				// スコア入力確定
#define CALLTYPE_FINISH					512				// フィニッシュ
#define CALLTYPE_PREV_LEG				1024			// 前のレッグに戻る

// プラグイン戻り値
#define PLUGIN_ERROR					0				// プラグインのエラー
#define PLUGIN_SUCCEED					1				// プラグインの正常終了
#define PLUGIN_CANCEL					2				// 以降の処理をキャンセル (CALLTYPE_END, CALLTYPE_INPUT_END, CALLTYPE_FINISH)
#define PLUGIN_DATA_MODIFIED			4				// データ変更あり (CALLTYPE_INPUT_END, CALLTYPE_FINISH)

/* Struct */
// プラグイン取得情報
typedef struct _PLUGIN_GET_INFO {
	DWORD struct_size;					// 構造体のサイズ

	TCHAR title[BUF_SIZE];
	TCHAR func_name[BUF_SIZE];
	TCHAR cmd_line[BUF_SIZE];
} PLUGIN_GET_INFO;

// プラグイン実行情報
typedef struct _PLUGIN_EXEC_INFO {
	DWORD struct_size;					// 構造体のサイズ

	TCHAR cmd_line[BUF_SIZE];			// プラグイン設定で指定したコマンドライン
	LPARAM lParam1;						// プラグインに対応するlong値
	LPARAM lParam2;						// プラグインに対応するlong値

	struct _SCORE_INFO *si;
} PLUGIN_EXEC_INFO;

// プラグイン情報
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

/* Function Prototypes */
void plugin_create_accelerator(void);
void plugin_free_accelerator(void);
BOOL plugin_initialize(const HWND hWnd, struct _SCORE_INFO *si);
void plugin_free(void);
int plugin_execute(const HWND hWnd, PLUGIN_INFO *pi, const int call_type, struct _SCORE_INFO *si, int param);
int plugin_execute_all(const HWND hWnd, const int call_type, struct _SCORE_INFO *si, int param);

#endif
/* End of source */
