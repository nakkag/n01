/*
 * n01
 *
 * String.h
 *
 * Copyright (C) 1996-2006 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

#ifndef _INC_STRING_H
#define _INC_STRING_H

/* Include Files */

/* Define */
#ifdef UNICODE
#define tchar_to_char_size(wbuf)		(WideCharToMultiByte(CP_ACP, 0, wbuf, -1, NULL, 0, NULL, NULL) - 1)
#else
#define tchar_to_char_size(wbuf)		(lstrlen(wbuf))
#endif

#ifdef UNICODE
#define tchar_to_char(wbuf, ret, len)	(WideCharToMultiByte(CP_ACP, 0, wbuf, -1, ret, len + 1, NULL, NULL))
#else
#define tchar_to_char(wbuf, ret, len)	(lstrcpyn(ret, wbuf, len + 1))
#endif

#ifdef UNICODE
#define char_to_tchar_size(buf)			(MultiByteToWideChar(CP_ACP, 0, buf, -1, NULL, 0) - 1)
#else
#define char_to_tchar_size(buf)			(lstrlen(buf))
#endif

#ifdef UNICODE
#define char_to_tchar(buf, wret, len)	(MultiByteToWideChar(CP_ACP, 0, buf, -1, wret, len + 1))
#else
#define char_to_tchar(buf, wret, len)	(lstrcpyn(wret, buf, len + 1))
#endif


/* Struct */

/* Function Prototypes */
int a2i(const char *str);
int x2i(const char *str);
#ifdef UNICODE
int tx2i(const TCHAR *str);
#else
#define tx2i	x2i
#endif
UINT str2hash(const TCHAR *str);
BOOL str_match(const TCHAR *ptn, const TCHAR *str);
BOOL trim(TCHAR *buf);
TCHAR *str_noprefix_cpy(TCHAR *ret, TCHAR *buf);

#endif
/* End of source */
