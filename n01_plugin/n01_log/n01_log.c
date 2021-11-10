/*
 * n01_log
 *
 * n01_log.c
 *
 * Copyright (C) 1996-2005 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

/* Include Files */
#define _INC_OLE
#include <windows.h>
#undef  _INC_OLE

#include "..\n01_plugin.h"

/* Define */
#define LOG_FILE					TEXT("n01.log")
#define CSV_FILE					TEXT("n01.csv")

/* Global Variables */
HINSTANCE hInst;
TCHAR log_path[MAX_PATH];
TCHAR csv_path[MAX_PATH];

/* Local Function Prototypes */

/*
 * DllMain - メイン
 */
int WINAPI DllMain(HINSTANCE hInstance, DWORD fdwReason, PVOID pvReserved)
{
	TCHAR work_path[MAX_PATH];
	TCHAR *p, *r;

	switch (fdwReason) {
	case DLL_PROCESS_ATTACH:
		hInst = hInstance;
		// アプリケーションのパスを取得
		GetModuleFileName(hInst, work_path, MAX_PATH - 1);
		for (p = r = work_path; *p != TEXT('\0'); p++) {
#ifndef UNICODE
			if (IsDBCSLeadByte((BYTE)*p) == TRUE) {
				p++;
				continue;
			}
#endif	// UNICODE
			if (*p == TEXT('\\') || *p == TEXT('/')) {
				r = p;
			}
		}
		*r = TEXT('\0');
		// iniパスを作成
		wsprintf(log_path, TEXT("%s\\%s"), work_path, LOG_FILE);
		wsprintf(csv_path, TEXT("%s\\%s"), work_path, CSV_FILE);
		break;

	case DLL_PROCESS_DETACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	}
	return TRUE;
}

/*
 * get_plugin_info - プラグイン情報取得
 */
__declspec(dllexport) BOOL CALLBACK get_plugin_info(const HWND hWnd, const int index, PLUGIN_GET_INFO *pgi)
{
	switch (index) {
	case 0:
		lstrcpy(pgi->title, TEXT("&Log"));
		lstrcpy(pgi->func_name, TEXT("n01_log"));
		lstrcpy(pgi->cmd_line, TEXT(""));
		return TRUE;

	case 1:
		lstrcpy(pgi->title, TEXT("&CSV"));
		lstrcpy(pgi->func_name, TEXT("n01_csv"));
		lstrcpy(pgi->cmd_line, TEXT(""));
		return TRUE;
	}
	return FALSE;
}

/*
 * save_log - ログの保存
 */
static BOOL save_log(TCHAR *buf)
{
	HANDLE hFile;
	DWORD ret;

	//保存するファイルを開く
	hFile = CreateFile(log_path, GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == NULL || hFile == (HANDLE)-1){
		return FALSE;
	}
	SetFilePointer(hFile, 0, NULL, FILE_END);
	WriteFile(hFile, buf, sizeof(TCHAR) * lstrlen(buf), &ret, NULL);
	WriteFile(hFile, TEXT("\r\n"), sizeof(TCHAR) * lstrlen(TEXT("\r\n")), &ret, NULL);
	CloseHandle(hFile);
	return TRUE;
}

/*
 * n01_log_initialize - プラグインの初期化
 */
__declspec(dllexport) BOOL CALLBACK n01_log_initialize(const HWND hWnd, PLUGIN_EXEC_INFO *pei)
{
	if (pei->lParam1 == 0) {
		pei->lParam1 = 1;
		// 古いログの削除
		DeleteFile(log_path);
	}
	save_log(TEXT("n01_log_initialize"));
	return (CALLTYPE_START | CALLTYPE_END | CALLTYPE_MENU | CALLTYPE_SET_START | CALLTYPE_SET_END | CALLTYPE_LEG_START | CALLTYPE_LEG_END | CALLTYPE_INPUT_START | CALLTYPE_INPUT_END | CALLTYPE_FINISH | CALLTYPE_PREV_LEG);
}

/*
 * n01_log_property - プラグインのプロパティ
 */
__declspec(dllexport) BOOL CALLBACK n01_log_property(const HWND hWnd, PLUGIN_EXEC_INFO *pei)
{
	save_log(TEXT("n01_log_property"));
	return PLUGIN_ERROR;
}

/*
 * n01_log
 */
__declspec(dllexport) int CALLBACK n01_log(const HWND hWnd, const int call_type, PLUGIN_EXEC_INFO *pei, int param)
{
	TCHAR buf[BUF_SIZE];
	int x, y;

	switch (call_type) {
	case CALLTYPE_MENU:
		wsprintf(buf, TEXT("CALLTYPE_MENU: param=%d"), param);
		save_log(buf);
		// ログを開く
		ShellExecute(hWnd, (LPCTSTR)NULL, (LPCTSTR)log_path, NULL, NULL, SW_SHOWNORMAL);
		break;

	case CALLTYPE_START:
		wsprintf(buf, TEXT("CALLTYPE_START: param=%d"), param);
		save_log(buf);
		break;
	case CALLTYPE_END:
		wsprintf(buf, TEXT("CALLTYPE_END: param=%d"), param);
		save_log(buf);
		break;
	case CALLTYPE_SET_START:
		wsprintf(buf, TEXT("CALLTYPE_SET_START: param=%d, sets=%d, leg_limit=%d, max_leg=%d, best_of=%d"),
			param, pei->si->current_set, pei->si->leg_limit, pei->si->max_leg, pei->si->best_of);
		save_log(buf);
		break;
	case CALLTYPE_SET_END:
		wsprintf(buf, TEXT("CALLTYPE_SET_END: param=%d"), param);
		save_log(buf);
		break;
	case CALLTYPE_LEG_START:
		wsprintf(buf, TEXT("CALLTYPE_LEG_START: param=%d, sets=%d, legs=%d, start_score=%d, round_limit=%d, round=%d"),
			param, pei->si->current_set, pei->si->current_leg, pei->si->start_score, pei->si->round_limit, pei->si->round);
		save_log(buf);
		break;
	case CALLTYPE_LEG_END:
		wsprintf(buf, TEXT("CALLTYPE_LEG_END: param=%d, winner=%d, darts=%d"),
			param, pei->si->leg[param].winner, pei->si->leg[param].darts);
		save_log(buf);
		break;
	case CALLTYPE_INPUT_START:
		SendMessage(hWnd, WM_N01_GET_INPUT_POS, (WPARAM)&x, (LPARAM)&y);
		wsprintf(buf, TEXT("CALLTYPE_INPUT_START: param=%d, x= %d, y=%d"), param, x, y);
		save_log(buf);
		break;
	case CALLTYPE_INPUT_END:
		SendMessage(hWnd, WM_N01_GET_INPUT_POS, (WPARAM)&x, (LPARAM)&y);
		wsprintf(buf, TEXT("CALLTYPE_INPUT_END: param=%d, x=%d, y=%d"), param, x, y);
		save_log(buf);
		break;
	case CALLTYPE_FINISH:
		SendMessage(hWnd, WM_N01_GET_INPUT_POS, (WPARAM)&x, (LPARAM)&y);
		wsprintf(buf, TEXT("CALLTYPE_FINISH: param=%d, x=%d, y=%d"), param, x, y);
		save_log(buf);
		break;
	case CALLTYPE_PREV_LEG:
		wsprintf(buf, TEXT("CALLTYPE_PREV_LEG: param=%d"), param);
		save_log(buf);
		break;
	}
	return PLUGIN_SUCCEED;
}

/*
 * n01_csv_initialize - プラグインの初期化
 */
__declspec(dllexport) BOOL CALLBACK n01_csv_initialize(const HWND hWnd, PLUGIN_EXEC_INFO *pei)
{
	return CALLTYPE_MENU;
}

/*
 * n01_csv
 */
__declspec(dllexport) int CALLBACK n01_csv(const HWND hWnd, const int call_type, PLUGIN_EXEC_INFO *pei, int param)
{
	HANDLE hFile;
	DWORD ret;
	TCHAR *p;

	// CSV取得
	p = (TCHAR *)SendMessage(hWnd, WM_N01_GET_CSV, 0, (LPARAM)pei->si);
	if (p == NULL) {
		return PLUGIN_ERROR;
	}

	//CSV保存
	hFile = CreateFile(csv_path, GENERIC_READ | GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == NULL || hFile == (HANDLE)-1){
		SendMessage(hWnd, WM_N01_FREE_CSV, 0, (LPARAM)p);
		return PLUGIN_ERROR;
	}
	WriteFile(hFile, p, sizeof(TCHAR) * lstrlen(p), &ret, NULL);
	CloseHandle(hFile);

	// CSV解放
	SendMessage(hWnd, WM_N01_FREE_CSV, 0, (LPARAM)p);

	// 開く
	ShellExecute(hWnd, (LPCTSTR)NULL, (LPCTSTR)csv_path, NULL, NULL, SW_SHOWNORMAL);
	return PLUGIN_SUCCEED;
}
/* End of source */
