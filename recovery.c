/*
 * n01
 *
 * recovery.c
 *
 * Copyright (C) 1996-2006 by Nakashima Tomoaki. All rights reserved.
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
#include "file.h"
#include "recovery.h"

#include "resource.h"

/* Define */
#define RECOVERY_FILE				TEXT("\\n01.tmp")

/* Global Variables */
extern OPTION_INFO op;
extern TCHAR work_path[MAX_PATH];

/* Local Function Prototypes */

/*
 * recovery_save - リカバリ情報の保存
 */
BOOL recovery_save(const SCORE_INFO *si, const int add_mode, const int player, const int round)
{
	static long file_pointer = 0xFFFFFFFF;
	static int alloc_round;
	HANDLE hFile;
	DWORD ret;
	TCHAR path[MAX_PATH];
	int i;
	BYTE ver;

	if (op.recovery == 0) {
		return TRUE;
	}

	lstrcpy(path, work_path);
	lstrcpy(path + lstrlen(work_path), RECOVERY_FILE);

	if (add_mode == 1 && file_pointer != 0xFFFFFFFF) {
		// スコアのみ出力
		// ファイルを開く
		hFile = CreateFile(path, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == NULL || hFile == (HANDLE)-1) {
			return recovery_save(si, 0, 0, 0);
		}
		// ファイルポインタを追加位置に移動
		if (SetFilePointer(hFile, file_pointer, NULL, FILE_BEGIN) == 0xFFFFFFFF) {
			CloseHandle(hFile);
			return recovery_save(si, 0, 0, 0);
		}
		// レッグ情報の書き込み
		i = (alloc_round != si->leg[si->current_leg].alloc_round) ? 5 : 4;
		if (WriteFile(hFile, &si->leg[si->current_leg], sizeof(int) * i, &ret, NULL) == FALSE) {
			CloseHandle(hFile);
			return FALSE;
		}
		if (SetFilePointer(hFile, sizeof(LEG_INFO) - sizeof(int) * i, NULL, FILE_CURRENT) == 0xFFFFFFFF) {
			CloseHandle(hFile);
			return FALSE;
		}
	} else if (add_mode == 2 && file_pointer != 0xFFFFFFFF) {
		// レッグの変わり目
		alloc_round = 0;

		// ファイルを開く
		hFile = CreateFile(path, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == NULL || hFile == (HANDLE)-1) {
			return recovery_save(si, 0, 0, 0);
		}
		if (SetFilePointer(hFile, sizeof(BYTE) + sizeof(GAME_INFO), NULL, FILE_BEGIN) == 0xFFFFFFFF) {
			CloseHandle(hFile);
			return recovery_save(si, 0, 0, 0);
		}
		// スコア情報の書き込み
		if (WriteFile(hFile, si, sizeof(SCORE_INFO), &ret, NULL) == FALSE) {
			CloseHandle(hFile);
			return FALSE;
		}

		// ファイルポインタを追加位置に移動
		if (SetFilePointer(hFile, file_pointer, NULL, FILE_BEGIN) == 0xFFFFFFFF) {
			CloseHandle(hFile);
			return recovery_save(si, 0, 0, 0);
		}
		// 過去レッグ情報の書き込み
		if (WriteFile(hFile, &si->leg[si->current_leg - 1], sizeof(LEG_INFO), &ret, NULL) == FALSE) {
			CloseHandle(hFile);
			return FALSE;
		}
		// 過去スコアの書き込み
		if (WriteFile(hFile, si->leg[si->current_leg - 1].score[0], sizeof(TYPE_SCORE) * (si->leg[si->current_leg - 1].current_round + 1), &ret, NULL) == FALSE) {
			CloseHandle(hFile);
			return FALSE;
		}
		if (WriteFile(hFile, si->leg[si->current_leg - 1].score[1], sizeof(TYPE_SCORE) * (si->leg[si->current_leg - 1].current_round + 1), &ret, NULL) == FALSE) {
			CloseHandle(hFile);
			return FALSE;
		}
		// 追加位置の取得
		file_pointer = SetFilePointer(hFile, 0, NULL, FILE_CURRENT);

		// レッグ情報の書き込み
		if (WriteFile(hFile, &si->leg[si->current_leg], sizeof(LEG_INFO), &ret, NULL) == FALSE) {
			CloseHandle(hFile);
			return FALSE;
		}
	} else {
		// 全て出力
		file_pointer = 0xFFFFFFFF;
		alloc_round = 0;

		// ファイルを開く
		hFile = CreateFile(path, GENERIC_READ | GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == NULL || hFile == (HANDLE)-1) {
			return FALSE;
		}

		// バージョンの書き込み
		ver = (BYTE)APP_VERSION;
		if (WriteFile(hFile, &ver, sizeof(BYTE), &ret, NULL) == FALSE) {
			CloseHandle(hFile);
			return FALSE;
		}

		// ゲーム情報の書き込み
		if (WriteFile(hFile, &op.gi, sizeof(GAME_INFO), &ret, NULL) == FALSE) {
			CloseHandle(hFile);
			return FALSE;
		}

		// スコア情報の書き込み
		if (WriteFile(hFile, si, sizeof(SCORE_INFO), &ret, NULL) == FALSE) {
			CloseHandle(hFile);
			return FALSE;
		}

		// スケジュール情報の書き込み
		if (si->set_mode == TRUE) {
			if (WriteFile(hFile, &op.gi_list_count, sizeof(int), &ret, NULL) == FALSE) {
				CloseHandle(hFile);
				return FALSE;
			}
			for (i = 0; i < op.gi_list_count; i++) {
				if (WriteFile(hFile, &op.gi_list[i], sizeof(GAME_INFO), &ret, NULL) == FALSE) {
					CloseHandle(hFile);
					return FALSE;
				}
			}
		}

		for (i = 0; i < si->current_leg; i++) {
			// 過去レッグ情報の書き込み
			if (WriteFile(hFile, &si->leg[i], sizeof(LEG_INFO), &ret, NULL) == FALSE) {
				CloseHandle(hFile);
				return FALSE;
			}
			// 過去スコアの書き込み
			if (WriteFile(hFile, si->leg[i].score[0], sizeof(TYPE_SCORE) * (si->leg[i].current_round + 1), &ret, NULL) == FALSE) {
				CloseHandle(hFile);
				return FALSE;
			}
			if (WriteFile(hFile, si->leg[i].score[1], sizeof(TYPE_SCORE) * (si->leg[i].current_round + 1), &ret, NULL) == FALSE) {
				CloseHandle(hFile);
				return FALSE;
			}
		}
		// 追加位置の取得
		file_pointer = SetFilePointer(hFile, 0, NULL, FILE_CURRENT);

		// レッグ情報の書き込み
		if (WriteFile(hFile, &si->leg[si->current_leg], sizeof(LEG_INFO), &ret, NULL) == FALSE) {
			CloseHandle(hFile);
			return FALSE;
		}
	}

	if (alloc_round != si->leg[si->current_leg].alloc_round || add_mode == FALSE) {
		alloc_round = si->leg[si->current_leg].alloc_round;
		for (i = 0; i < 2; i++) {
			// スコアの書き込み
			if (WriteFile(hFile, si->leg[si->current_leg].score[i], sizeof(TYPE_SCORE) * (si->leg[si->current_leg].alloc_round + 1), &ret, NULL) == FALSE) {
				CloseHandle(hFile);
				return FALSE;
			}
			// チェックアウト本数の書き込み
			if (WriteFile(hFile, si->tmp_check_out[i], sizeof(TYPE_CHECK_OUT) * (si->leg[si->current_leg].alloc_round + 1), &ret, NULL) == FALSE) {
				CloseHandle(hFile);
				return FALSE;
			}
		}
	} else {
		// スコアの書き込み
		if (SetFilePointer(hFile,
			player * sizeof(TYPE_SCORE) * (si->leg[si->current_leg].alloc_round + 1) +
			player * sizeof(TYPE_CHECK_OUT) * (si->leg[si->current_leg].alloc_round + 1) + 
			sizeof(TYPE_SCORE) * round,
			NULL, FILE_CURRENT) == 0xFFFFFFFF) {
			CloseHandle(hFile);
			return FALSE;
		}
		if (WriteFile(hFile, &si->leg[si->current_leg].score[player][round], sizeof(TYPE_SCORE), &ret, NULL) == FALSE) {
			CloseHandle(hFile);
			return FALSE;
		}
		// チェックアウト本数の書き込み
		if (SetFilePointer(hFile,
			sizeof(TYPE_SCORE) * ((si->leg[si->current_leg].alloc_round + 1) - (round + 1)) +
			sizeof(TYPE_CHECK_OUT) * round,
			NULL, FILE_CURRENT) == 0xFFFFFFFF) {
			CloseHandle(hFile);
			return FALSE;
		}
		if (WriteFile(hFile, &si->tmp_check_out[player][round], sizeof(TYPE_CHECK_OUT), &ret, NULL) == FALSE) {
			CloseHandle(hFile);
			return FALSE;
		}
	}
	CloseHandle(hFile);
	return TRUE;
}

/*
 * recovery_load - リカバリ情報の読み込み
 */
SCORE_INFO *recovery_load(const HWND hWnd)
{
	static SCORE_INFO si;
	OPTION_INFO tmp_op;
	BYTE *data, *p;
	DWORD size;
	TCHAR path[MAX_PATH];
	TCHAR err_str[BUF_SIZE];
	int i, j;
	BYTE ver;

	if (op.recovery == 0) {
		return NULL;
	}
	tmp_op = op;

	// ファイルの存在チェック
	lstrcpy(path, work_path);
	lstrcpy(path + lstrlen(work_path), RECOVERY_FILE);
	if (file_check_file(path) == FALSE) {
		return NULL;
	}

	// ファイルを読み込む
	p = data = file_read_buf(path, &size, err_str);
	if (data == NULL) {
		MessageBox(hWnd, err_str, APP_NAME, MB_ICONERROR);
		return NULL;
	}

	// バージョンを読みこむ
	if (size < sizeof(BYTE)) {
		mem_free(&data);
		return NULL;
	}
	CopyMemory(&ver, p, sizeof(BYTE));
	if (ver != (BYTE)APP_VERSION) {
		mem_free(&data);
		return NULL;
	}
	p += sizeof(BYTE);
	size -= sizeof(BYTE);

	// ゲーム情報を読みこむ
	if (size < sizeof(GAME_INFO)) {
		mem_free(&data);
		return NULL;
	}
	CopyMemory(&op.gi, p, sizeof(GAME_INFO));
	p += sizeof(GAME_INFO);
	size -= sizeof(GAME_INFO);

	// スコア情報を読みこむ
	if (size < sizeof(SCORE_INFO)) {
		mem_free(&data);
		op = tmp_op;
		return NULL;
	}
	CopyMemory(&si, p, sizeof(SCORE_INFO));
	p += sizeof(SCORE_INFO);
	size -= sizeof(SCORE_INFO);

	// スケジュール情報を読みこむ
	if (si.set_mode == TRUE) {
		if (size < sizeof(int)) {
			mem_free(&data);
			op = tmp_op;
			return NULL;
		}
		CopyMemory(&op.gi_list_count, p, sizeof(int));
		p += sizeof(int);
		size -= sizeof(int);

		if (size < sizeof(GAME_INFO) * op.gi_list_count) {
			mem_free(&data);
			op = tmp_op;
			return NULL;
		}
		op.gi_list = (GAME_INFO *)mem_alloc(sizeof(GAME_INFO) * op.gi_list_count);
		if (op.gi_list == NULL) {
			message_get_error(GetLastError(), err_str);
			MessageBox(hWnd, err_str, APP_NAME, MB_ICONERROR);
			mem_free(&data);
			op = tmp_op;
			return NULL;
		}
		for (i = 0; i < op.gi_list_count; i++) {
			CopyMemory(&op.gi_list[i], p, sizeof(GAME_INFO));
			p += sizeof(GAME_INFO);
			size -= sizeof(GAME_INFO);
		}
	}

	// レッグ情報の確保
	si.leg = mem_calloc(sizeof(LEG_INFO) * (si.current_leg + 1));
	if (si.leg == NULL) {
		message_get_error(GetLastError(), err_str);
		MessageBox(hWnd, err_str, APP_NAME, MB_ICONERROR);
		mem_free(&data);
		if (si.set_mode == TRUE) mem_free(&op.gi_list);
		op = tmp_op;
		return NULL;
	}
	for (i = 0; i < si.current_leg; i++) {
		// 過去のレッグ情報を読みこむ
		if (size < sizeof(LEG_INFO)) {
			for (j = 0; j < i; j++) {
				mem_free(&si.leg[j].score[0]);
				mem_free(&si.leg[j].score[1]);
			}
			mem_free(&si.leg);
			mem_free(&data);
			if (si.set_mode == TRUE) mem_free(&op.gi_list);
			op = tmp_op;
			return NULL;
		}
		CopyMemory(&si.leg[i], p, sizeof(LEG_INFO));
		p += sizeof(LEG_INFO);
		size -= sizeof(LEG_INFO);

		// 過去のスコアを読みこむ
		if (si.leg[i].current_round > si.leg[i].alloc_round) {
			si.leg[i].max_round = si.leg[i].alloc_round = si.leg[i].current_round + 1;
		}
		if (size < (sizeof(TYPE_SCORE) * (si.leg[i].current_round + 1)) * 2) {
			for (j = 0; j < i; j++) {
				mem_free(&si.leg[j].score[0]);
				mem_free(&si.leg[j].score[1]);
			}
			mem_free(&si.leg);
			mem_free(&data);
			if (si.set_mode == TRUE) mem_free(&op.gi_list);
			op = tmp_op;
			return NULL;
		}
		for (j = 0; j < 2; j++) {
			si.leg[i].score[j] = mem_calloc(sizeof(TYPE_SCORE) * (si.leg[i].alloc_round + 1));
			if (si.leg[i].score[j] == NULL) {
				message_get_error(GetLastError(), err_str);
				MessageBox(hWnd, err_str, APP_NAME, MB_ICONERROR);
				mem_free(&si.leg);
				mem_free(&data);
				if (si.set_mode == TRUE) mem_free(&op.gi_list);
				op = tmp_op;
				return NULL;
			}
			CopyMemory(si.leg[i].score[j], p, sizeof(TYPE_SCORE) * (si.leg[i].current_round + 1));
			p += sizeof(TYPE_SCORE) * (si.leg[i].current_round + 1);
			size -= sizeof(TYPE_SCORE) * (si.leg[i].current_round + 1);
		}
	}

	// レッグ情報を読みこむ
	if (size < sizeof(LEG_INFO)) {
		for (j = 0; j < si.current_leg; j++) {
			mem_free(&si.leg[j].score[0]);
			mem_free(&si.leg[j].score[1]);
		}
		mem_free(&si.leg);
		mem_free(&data);
		if (si.set_mode == TRUE) mem_free(&op.gi_list);
		op = tmp_op;
		return NULL;
	}
	CopyMemory(&si.leg[si.current_leg], p, sizeof(LEG_INFO));
	p += sizeof(LEG_INFO);
	size -= sizeof(LEG_INFO);

	// スコア情報を読みこむ
	if (size < (sizeof(TYPE_SCORE) * (si.leg[si.current_leg].alloc_round + 1)) * 2 + (sizeof(TYPE_CHECK_OUT) * (si.leg[si.current_leg].alloc_round + 1)) * 2 ||
		si.leg[si.current_leg].current_round > si.leg[si.current_leg].alloc_round) {
		for (j = 0; j < si.current_leg; j++) {
			mem_free(&si.leg[j].score[0]);
			mem_free(&si.leg[j].score[1]);
		}
		mem_free(&si.leg);
		mem_free(&data);
		if (si.set_mode == TRUE) mem_free(&op.gi_list);
		op = tmp_op;
		return NULL;
	}
	for (i = 0; i < 2; i++) {
		// スコアを読みこむ
		si.leg[si.current_leg].score[i] = mem_calloc(sizeof(TYPE_SCORE) * (si.leg[si.current_leg].alloc_round + 1));
		if (si.leg[si.current_leg].score[i] == NULL) {
			message_get_error(GetLastError(), err_str);
			MessageBox(hWnd, err_str, APP_NAME, MB_ICONERROR);
			mem_free(&si.leg);
			mem_free(&data);
			if (si.set_mode == TRUE) mem_free(&op.gi_list);
			op = tmp_op;
			return NULL;
		}
		CopyMemory(si.leg[si.current_leg].score[i], p, sizeof(TYPE_SCORE) * (si.leg[si.current_leg].alloc_round + 1));
		p += sizeof(TYPE_SCORE) * (si.leg[si.current_leg].alloc_round + 1);
		size -= sizeof(TYPE_SCORE) * (si.leg[si.current_leg].alloc_round + 1);

		// チェックアウト本数を読み込む
		si.tmp_check_out[i] = mem_calloc(sizeof(TYPE_CHECK_OUT) * (si.leg[si.current_leg].alloc_round + 1));
		if (si.tmp_check_out[i] == NULL) {
			message_get_error(GetLastError(), err_str);
			MessageBox(hWnd, err_str, APP_NAME, MB_ICONERROR);
			mem_free(&si.leg);
			mem_free(&data);
			if (si.set_mode == TRUE) mem_free(&op.gi_list);
			op = tmp_op;
			return NULL;
		}
		CopyMemory(si.tmp_check_out[i], p, sizeof(TYPE_CHECK_OUT) * (si.leg[si.current_leg].alloc_round + 1));
		p += sizeof(TYPE_CHECK_OUT) * (si.leg[si.current_leg].alloc_round + 1);
		size -= sizeof(TYPE_CHECK_OUT) * (si.leg[si.current_leg].alloc_round + 1);
	}
	if (si.set_mode == TRUE) {
		mem_free(&tmp_op.gi_list);
	}
	mem_free(&data);
	return &si;
}

/*
 * recovery_delete - リカバリ情報の削除
 */
void recovery_delete(void)
{
	TCHAR path[MAX_PATH];

	if (op.recovery_delete == 0) {
		return;
	}
	lstrcpy(path, work_path);
	lstrcpy(path + lstrlen(work_path), RECOVERY_FILE);
	DeleteFile(path);
}
/* End of source */
