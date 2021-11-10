/*
 * n01
 *
 * file.h
 *
 * Copyright (C) 1996-2004 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

#ifndef _INC_FILE_H
#define _INC_FILE_H

/* Include Files */

/* Define */

/* Struct */

/* Function Prototypes */
BOOL file_check_file(const TCHAR *path);
BOOL file_check_directory(const TCHAR *path);
void create_tree_directory(const TCHAR *p);
BOOL get_save_path(const HWND hWnd, TCHAR *title, TCHAR *filter, TCHAR *def, TCHAR *path);
BYTE *file_read_buf(const TCHAR *path, DWORD *ret_size, TCHAR *err_str);
BOOL file_write_buf(const TCHAR *path, const BYTE *data, const DWORD size, TCHAR *err_str);

#endif
/* End of source */
