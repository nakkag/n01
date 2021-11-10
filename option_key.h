/*
 * n01
 *
 * option_key.h
 *
 * Copyright (C) 1996-2006 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

#ifndef _INC_OPTION_KEY_H
#define _INC_OPTION_KEY_H

/* Include Files */

/* Define */

/* Struct */

/* Function Prototypes */
void create_accelerator(void);
void free_accelerator(void);
BOOL get_keyname(const UINT modifiers, const UINT virtkey, TCHAR *ret);
BOOL CALLBACK option_key_proc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

#endif
/* End of source */
