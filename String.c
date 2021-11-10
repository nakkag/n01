/*
 * n01
 *
 * String.c
 *
 * Copyright (C) 1996-2006 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

/* Include Files */
#include <windows.h>
#include <tchar.h>

#include "Memory.h"

/* Define */
#define to_lower(c)		((c >= TEXT('A') && c <= TEXT('Z')) ? (c - TEXT('A') + TEXT('a')) : c)

/* Global Variables */

/* Local Function Prototypes */

/*
 * a2i - �����̕�����𐔒l(int)�ɕϊ�����
 */
int a2i(const char *str)
{
	int num = 0;
	int m = 1;

	if (*str == '-') {
		m = -1;
		str++;
	} else if (*str == '+') {
		str++;
	}

	for (; *str >= '0' && *str <= '9'; str++) {
		num = 10 * num + (*str - '0');
	}
	return (num * m);
}

/*
 * x2i - 16�i������𐔒l(int)�ɕϊ�����
 */
int x2i(const char *str)
{
	int ret = 0;
	int num;
	int i;

	if (str == NULL) {
		return 0;
	}

	if (*str == '0' && (*(str + 1) == 'x' || *(str + 1) == 'X')) {
		str += 2;
	} else if (*str == 'x' || *str == 'X') {
		str++;
	}

	for (i = 0; i < 8 && *str != '\0'; i++, str++) {
		if (*str >= '0' && *str <= '9') {
			num = *str - '0';
		} else if (*str >= 'a' && *str <= 'f') {
			num = *str - 'a' + 10;
		} else if (*str >= 'A' && *str <= 'F') {
			num = *str - 'A' + 10;
		} else {
			break;
		}
		ret = ret * 16 + num;
	}
	return ret;
}

/*
 * tx2i - 16�i������(UNICODE)�𐔒l(int)�ɕϊ�����
 */
#ifdef UNICODE
int tx2i(const TCHAR *str)
{
	char *c_str;
	int ret = 0;

	if ((c_str = alloc_tchar_to_char(str)) != NULL) {
		ret = x2i(c_str);
		mem_free(&c_str);
	}
	return ret;
}
#endif

/*
 * str2hash - ������̃n�b�V���l���擾
 */
UINT str2hash(const TCHAR *str)
{
	UINT hash = 0;

	for (; *str != TEXT('\0'); str++) {
		if (*str != TEXT(' ')) {
			hash ^= ((hash << 4) + to_lower(*str));
		}
	}
	return hash;
}

/*
 * str_match - 2�̕���������C���h�J�[�h(*)���g���Ĕ�r���s��
 */
BOOL str_match(const TCHAR *ptn, const TCHAR *str)
{
	switch (*ptn) {
	case TEXT('\0'):
		return (*str == TEXT('\0'));
	case TEXT('*'):
		if (*(ptn + 1) == TEXT('\0')) {
			return TRUE;
		}
		if (str_match(ptn + 1, str) == TRUE) {
			return TRUE;
		}
		while (*str != TEXT('\0')) {
			str++;
			if (str_match(ptn + 1, str) == TRUE) {
				return TRUE;
			}
		}
		return FALSE;
	case TEXT('?'):
		return (*str != TEXT('\0')) && str_match(ptn + 1, str + 1);
	default:
		while (to_lower(*ptn) == to_lower(*str)) {
			if (*ptn == TEXT('\0')) {
				return TRUE;
			}
			ptn++;
			str++;
			if (*ptn == TEXT('*') || *ptn == TEXT('?')) {
				return str_match(ptn, str);
			}
		}
		return FALSE;
	}
}

/*
 * trim - ������̑O��̋�, Tab����������
 */
BOOL trim(TCHAR *buf)
{
	TCHAR *p, *r;

	// �O��̋󔒂��������|�C���^���擾
	for (p = buf; (*p == TEXT(' ') || *p == TEXT('\t')) && *p != TEXT('\0'); p++)
		;
	for (r = buf + lstrlen(buf) - 1; r > p && (*r == TEXT(' ') || *r == TEXT('\t')); r--)
		;
	*(r + 1) = TEXT('\0');

	// ���̕�����ɃR�s�[
	lstrcpy(buf, p);
	return TRUE;
}

/*
 * str_noprefix_cpy - prefix���������ĕ�����̃R�s�[
 */
TCHAR *str_noprefix_cpy(TCHAR *ret, TCHAR *buf)
{
	TCHAR *p, *r;

	for (p = buf, r = ret; *p != TEXT('\0'); p++) {
#ifndef UNICODE
		if (IsDBCSLeadByte((BYTE)*p) == TRUE) {
			*(r++) = *(p++);
			*(r++) = *p;
			continue;
		}
#endif	// UNICODE
		if (*p == TEXT('&')) {
			if (*(p + 1) == TEXT('&')) {
				*(r++) = *(p++);
			}
		} else if (*p == TEXT('(')) {
			for (; *p != TEXT('\0') && *p != TEXT(')'); p++);
			if (*p == TEXT('\0')) {
				break;
			}
		} else if (*p == TEXT('.')) {
		} else {
			*(r++) = *p;
		}
	}
	*r = TEXT('\0');
	return ret;
}
/* End of source */