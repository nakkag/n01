/*
 * n01
 *
 * n01_plugin_template.c
 *
 * Copyright (C) 1996-2005 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

/* Include Files */
#define _INC_OLE
#include <windows.h>
#undef  _INC_OLE

#include "n01_plugin.h"

/* Define */

/* Global Variables */

/* Local Function Prototypes */

/*
 * get_plugin_info - プラグイン情報取得
 *
 *	引数:
 *		hWnd - 呼び出し元ウィンドウ
 *		index - 取得のインデックス (0～)
 *		pgi - プラグイン取得情報
 *
 *	戻り値:
 *		TRUE - 次に取得するプラグインあり
 *		FALSE - 取得の終了
 */
__declspec(dllexport) BOOL CALLBACK get_plugin_info(const HWND hWnd, const int index, PLUGIN_GET_INFO *pgi)
{
	switch (index) {
	case 0:
		lstrcpy(pgi->title, TEXT("タイトル"));
		lstrcpy(pgi->func_name, TEXT("func_plugin"));
		lstrcpy(pgi->cmd_line, TEXT(""));
		return TRUE;

	case 1:
		return FALSE;
	}
	return FALSE;
}

/*
 * func_plugin_initialize - プラグインの初期化
 *
 *	引数:
 *		hWnd - 呼び出し元ウィンドウ
 *		pei - プラグイン実行情報
 *
 *	戻り値:
 *		プラグインの呼び出し方法
 *
 *	備考:
 *		関数名は プラグイン名_initialize
 */
__declspec(dllexport) BOOL CALLBACK func_plugin_initialize(const HWND hWnd, PLUGIN_EXEC_INFO *pei)
{
	// 戻り値に呼び出し方法の組み合わせを設定する
	return (CALLTYPE_START | CALLTYPE_END | CALLTYPE_MENU | CALLTYPE_SET_START | CALLTYPE_SET_END | CALLTYPE_LEG_START | CALLTYPE_LEG_END | CALLTYPE_INPUT_START | CALLTYPE_INPUT_END | CALLTYPE_FINISH | CALLTYPE_PREV_LEG);
}

/*
 * func_plugin_property - プラグインのプロパティ
 *
 *	引数:
 *		hWnd - 呼び出し元ウィンドウ
 *		pei - プラグイン実行情報
 *
 *	戻り値:
 *		PLUGIN_
 *
 *	備考:
 *		関数名は プラグイン名_property
 *		設定するプロパティが無い場合、この関数は必要無し。
 */
__declspec(dllexport) BOOL CALLBACK func_plugin_property(const HWND hWnd, PLUGIN_EXEC_INFO *pei)
{
	// プロパティを設定した場合は PLUGIN_SUCCEED を返す
	// 設定するプロパティが無い場合は PLUGIN_ERROR を返す
	return PLUGIN_ERROR;
}

/*
 * func_plugin - プラグイン処理
 *
 *	引数:
 *		hWnd - 呼び出し元ウィンドウ
 *		call_type - 呼び出し方法
 *		pei - プラグイン実行情報
 *		param - 呼び出し元情報
 *
 *	戻り値:
 *		PLUGIN_
 */
__declspec(dllexport) int CALLBACK func_plugin(const HWND hWnd, const int call_type, PLUGIN_EXEC_INFO *pei, int param)
{
	switch (call_type) {
	case CALLTYPE_START:
		// プログラム開始
		break;

	case CALLTYPE_END:
		// プログラム終了
		// param - 終了方法 (WM_CLOSE or WM_QUERYENDSESSION)
		// プログラムを終了しない場合は PLUGIN_CANCEL を返す
		break;

	case CALLTYPE_MENU:
		// メニュー
		break;

	case CALLTYPE_SET_START:
		// セット開始
		// param - 現在のセット数
		break;

	case CALLTYPE_SET_END:
		// セット終了
		// param - 現在のセット数
		break;

	case CALLTYPE_LEG_START:
		// レッグ開始
		// param - 現在のレッグ数
		break;

	case CALLTYPE_LEG_END:
		// レッグ終了
		// param - 現在のレッグ数
		break;

	case CALLTYPE_INPUT_START:
		// スコア入力開始
		break;

	case CALLTYPE_INPUT_END:
		// スコア入力確定
		// param - 入力したスコアの値
		// 戻り値に PLUGIN_CANCEL を返すとスコア入力を確定しない
		// スコア(pei->si->leg[pei->si->current_leg].score)を変更した場合は PLUGIN_DATA_MODIFIED を返す
		break;

	case CALLTYPE_FINISH:
		// フィニッシュ
		// param - フィニッシュ本数 (-1, -2, -3)
		// param が 0 の場合はミドルフォーディドルでのフィニッシュ
		// 戻り値に PLUGIN_CANCEL を返すとフィニッシュをキャンセルする
		// スコア(pei->si->leg[pei->si->current_leg].score)を変更した場合は PLUGIN_DATA_MODIFIED を返す
		break;

	case CALLTYPE_PREV_LEG:
		// 前のレッグに戻る
		// 戻り値に PLUGIN_CANCEL を返すと前のレッグに戻る機能を無効にします。
		break;
	}
	return PLUGIN_SUCCEED;
}
/* End of source */
