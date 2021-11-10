/*
 * n01
 *
 * arrange.h
 *
 * Copyright (C) 1996-2005 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

#ifndef _INC_ARRANGE_H
#define _INC_ARRANGE_H

/* Include Files */

/* Define */
#define THROW_LIST_SIZE					10

/* Struct */
typedef struct _ARRANGE_INFO {
	int left;
	TCHAR throw_list[5][THROW_LIST_SIZE + 1];
} ARRANGE_INFO;

/* Function Prototypes */
#ifdef _DEBUG
void arrange_check(void);
#endif
void free_arrange(ARRANGE_INFO **arrange_info, int *arrange_info_count);
ARRANGE_INFO *file_read_arrange(const TCHAR *path, int *arrange_info_count);
ARRANGE_INFO *res_to_arrange(const HINSTANCE hInst, const UINT res_id, int *arrange_info_count);
BOOL show_arrange(const HINSTANCE hInst, const HWND hWnd, int left);

#endif
/* End of source */
