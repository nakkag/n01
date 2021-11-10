/*
 * n01
 *
 * file.c
 *
 * Copyright (C) 1996-2004 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

/* Include Files */
#define _INC_OLE
#include <windows.h>
#undef  _INC_OLE

#include "Memory.h"
#include "Message.h"

/* Define */
#define BUF_SIZE				256

/* Global Variables */

/* Local Function Prototypes */

/*
 * file_check_file - ファイルが存在するかチェックする
 */
BOOL file_check_file(const TCHAR *path)
{
	WIN32_FIND_DATA FindData;
	HANDLE hFindFile;

	if ((hFindFile = FindFirstFile(path, &FindData)) == INVALID_HANDLE_VALUE) {
		return FALSE;
	}
	FindClose(hFindFile);

	if ((FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
		// ファイルが存在した場合
		return TRUE;
	}
	return FALSE;
}

/*
 * file_check_directory - ディレクトリが存在するかチェックする
 */
BOOL file_check_directory(const TCHAR *path)
{
	WIN32_FIND_DATA FindData;
	HANDLE hFindFile;

	if ((hFindFile = FindFirstFile(path, &FindData)) == INVALID_HANDLE_VALUE) {
		return FALSE;
	}
	FindClose(hFindFile);

	if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
		//ディレクトリが存在した場合
		return TRUE;
	}
	return FALSE;
}

/*
 * create_tree_directory - ディレクトリ階層を作成
 */
void create_tree_directory(const TCHAR *p)
{
	TCHAR tmp[BUF_SIZE];
	TCHAR *r;

	r = tmp;
	while (*p != TEXT('\0')) {
#ifndef UNICODE
		if (IsDBCSLeadByte((BYTE)*p) == TRUE) {
			*(r++) = *(p++);
			*(r++) = *(p++);
		} else
#endif
		if (*p == TEXT('\\') || *p == TEXT('/')) {
			*r = TEXT('\0');
			//ディレクトリの作成
			if (file_check_directory(tmp) == FALSE) {
				CreateDirectory(tmp, NULL);
			}
			*(r++) = *(p++);
		} else {
			*(r++) = *(p++);
		}
	}
	*r = TEXT('\0');
	if (file_check_directory(tmp) == FALSE) {
		CreateDirectory(tmp, NULL);
	}
}

/*
 * get_save_path - 保存先のファイル名を取得
 */
BOOL get_save_path(const HWND hWnd, TCHAR *title, TCHAR *filter, TCHAR *def, TCHAR *path)
{
	OPENFILENAME of;

	// ファイルの選択
	ZeroMemory(&of, sizeof(OPENFILENAME));
	of.lStructSize = sizeof(OPENFILENAME);
	of.hwndOwner = hWnd;
	of.lpstrFilter = filter;
	of.lpstrTitle = title;
	of.lpstrFile = path;
	of.nMaxFile = MAX_PATH - 1;
	of.lpstrDefExt = def;
	of.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	if (GetSaveFileName((LPOPENFILENAME)&of) == FALSE) {
		return FALSE;
	}
	return TRUE;
}

/*
 * file_read_buf - ファイルを読み込む
 */
BYTE *file_read_buf(const TCHAR *path, DWORD *ret_size, TCHAR *err_str)
{
	HANDLE hFile;
	DWORD size;
	DWORD ret;
	BYTE *buf;

	// ファイルを開く
	hFile = CreateFile(path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == NULL || hFile == (HANDLE)-1) {
		message_get_error(GetLastError(), err_str);
		return NULL;
	}
	if ((size = GetFileSize(hFile, NULL)) == 0xFFFFFFFF) {
		message_get_error(GetLastError(), err_str);
		CloseHandle(hFile);
		return NULL;
	}

	if ((buf = (BYTE *)mem_alloc(size + 1)) == NULL) {
		message_get_error(GetLastError(), err_str);
		CloseHandle(hFile);
		return NULL;
	}
	// ファイルを読みこむ
	if (ReadFile(hFile, buf, size, &ret, NULL) == FALSE) {
		message_get_error(GetLastError(), err_str);
		mem_free(&buf);
		CloseHandle(hFile);
		return NULL;
	}
	CloseHandle(hFile);

	if (ret_size != NULL) {
		*ret_size = size;
	}
	return buf;
}

/*
 * file_write_buf - ファイルに書き込む
 */
BOOL file_write_buf(const TCHAR *path, const BYTE *data, const DWORD size, TCHAR *err_str)
{
	HANDLE hFile;
	DWORD ret;

	// ファイルを開く
	hFile = CreateFile(path, GENERIC_READ | GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == NULL || hFile == (HANDLE)-1) {
		message_get_error(GetLastError(), err_str);
		return FALSE;
	}
	// ファイルの書き込み
	if (WriteFile(hFile, data, size, &ret, NULL) == FALSE) {
		message_get_error(GetLastError(), err_str);
		CloseHandle(hFile);
		return FALSE;
	}
	FlushFileBuffers(hFile);
	CloseHandle(hFile);
	return TRUE;
}
/* End of source */
