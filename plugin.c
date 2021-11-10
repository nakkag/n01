/*
 * n01
 *
 * plugin.c
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
#include "plugin.h"

/* Define */

/* Global Variables */
extern HINSTANCE hInst;
extern OPTION_INFO op;
extern HACCEL hPluginAccel;

/* Local Function Prototypes */

/*
 * plugin_create_accelerator - アクセラレータの作成
 */
void plugin_create_accelerator(void)
{
	ACCEL *ac;
	int cnt = 0;
	int i;

	plugin_free_accelerator();

	ac = (ACCEL *)mem_alloc(sizeof(ACCEL) * op.plugin_info_count);
	if (ac == NULL) {
		return;
	}
	for (i = 0; i < op.plugin_info_count; i++) {
		if (!(op.plugin_info[i].call_type & CALLTYPE_MENU)) {
			continue;
		}
		if (op.plugin_info[i].ctrl == 0 && op.plugin_info[i].key == 0) {
			continue;
		}
		ac[cnt].fVirt = (BYTE)(op.plugin_info[i].ctrl | FVIRTKEY);
		ac[cnt].key = (WORD)op.plugin_info[i].key;
		ac[cnt].cmd = (WORD)(ID_MENUITEM_TOOL + i);
		cnt++;
	}
	if (cnt != 0) {
		hPluginAccel = CreateAcceleratorTable(ac, cnt);
	}
	mem_free(&ac);
}

/*
 * plugin_free_accelerator - アクセラレータの解放
 */
void plugin_free_accelerator(void)
{
	if (hPluginAccel != NULL) {
		DestroyAcceleratorTable(hPluginAccel);
		hPluginAccel = NULL;
	}
}

/*
 * plugin_initialize - プラグイン情報の初期化
 */
BOOL plugin_initialize(const HWND hWnd, struct _SCORE_INFO *si)
{
	HANDLE lib;
	FARPROC func_initialize;
	TCHAR buf[BUF_SIZE];
	TCHAR err_str[BUF_SIZE];
	int i;
#ifdef UNICODE
	char cbuf[BUF_SIZE];
#endif

	for (i = 0; i < op.plugin_info_count; i++) {
		if (op.plugin_info[i].lib != NULL ||
			op.plugin_info[i].lib_file_path == NULL ||
			*op.plugin_info[i].lib_file_path == TEXT('\0')) {
			continue;
		}
		// モジュールハンドル取得
		lib = op.plugin_info[i].lib = LoadLibrary(op.plugin_info[i].lib_file_path);
		if (lib == NULL) {
			message_get_error(GetLastError(), buf);
			wsprintf(err_str, TEXT("%s\r\n%s"), buf, op.plugin_info[i].lib_file_path);
			MessageBox(hWnd, err_str, APP_NAME, MB_ICONERROR);
			return FALSE;
		}
		// 関数アドレス取得
#ifdef UNICODE
		tchar_to_char(op.plugin_info[i].func_name, cbuf, BUF_SIZE - 1);
		op.plugin_info[i].func = GetProcAddress(lib, cbuf);
#else
		op.plugin_info[i].func = GetProcAddress(lib, op.plugin_info[i].func_name);
#endif
		if (op.plugin_info[i].func == NULL) {
			message_get_error(GetLastError(), buf);
			wsprintf(err_str, TEXT("%s\r\n%s\r\n%s"), buf,
				op.plugin_info[i].lib_file_path, op.plugin_info[i].func_name);
			MessageBox(hWnd, err_str, APP_NAME, MB_ICONERROR);
			return FALSE;
		}

		// プラグインの初期化
		op.plugin_info[i].call_type = 0;
		wsprintf(buf, TEXT("%s_initialize"), op.plugin_info[i].func_name);
#ifdef UNICODE
		tchar_to_char(buf, cbuf, BUF_SIZE - 1);
		func_initialize = GetProcAddress(lib, cbuf);
#else
		func_initialize = GetProcAddress(lib, buf);
#endif
		if (func_initialize != NULL) {
			// 実行情報の設定
			op.plugin_info[i].pei.struct_size = sizeof(PLUGIN_EXEC_INFO);
			op.plugin_info[i].pei.si = si;
			op.plugin_info[i].call_type = func_initialize(hWnd, &op.plugin_info[i].pei);
		}
	}
	return TRUE;
}

/*
 * plugin_free - プラグイン情報の解放
 */
void plugin_free(void)
{
	int i;

	for (i = 0; i < op.plugin_info_count; i++) {
		if (op.plugin_info[i].lib != NULL) {
			FreeLibrary(op.plugin_info[i].lib);
			op.plugin_info[i].lib = NULL;
			op.plugin_info[i].func = NULL;
		}
	}
}

/*
 * plugin_execute - プラグインの呼び出し
 */
int plugin_execute(const HWND hWnd, PLUGIN_INFO *pi, const int call_type, struct _SCORE_INFO *si, int param)
{
	int ret = PLUGIN_ERROR;

	if (pi->lib != NULL && pi->func != NULL) {
		// プラグインの呼び出し
		pi->pei.struct_size = sizeof(PLUGIN_EXEC_INFO);
		pi->pei.si = si;
		ret = pi->func(hWnd, call_type, &pi->pei, param);
	}
	return ret;
}

/*
 * plugin_execute_all - 呼び出し方法にマッチするプラグインの実行
 */
int plugin_execute_all(const HWND hWnd, const int call_type, struct _SCORE_INFO *si, int param)
{
	int ret = PLUGIN_ERROR;
	int i;

	for (i = 0; i < op.plugin_info_count; i++) {
		if (op.plugin_info[i].call_type & call_type) {
			ret |= plugin_execute(hWnd, &op.plugin_info[i], call_type, si, param);
			if (ret & PLUGIN_CANCEL) {
				return ret;
			}
		}
	}
	return ret;
}
/* End of source */
