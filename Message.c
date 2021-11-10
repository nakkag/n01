/*
 * n01
 *
 * Message.c
 *
 * Copyright (C) 1996-2006 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

/* Include Files */

#define _INC_OLE
#include <windows.h>
#undef  _INC_OLE

/* Define */
#define BUF_SIZE			256

/* Global Variables */
extern HINSTANCE hInst;

/* Local Function Prototypes */

/*
 * message_get_error - �G���[�l���烁�b�Z�[�W���擾
 */
BOOL message_get_error(const int err_code, TCHAR *err_str)
{
	if (err_str == NULL) {
		return FALSE;
	}
	*err_str = TEXT('\0');
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, err_code, 0, err_str, BUF_SIZE - 1, NULL);
	return TRUE;
}

/*
 * message_get_res - ���\�[�X���烁�b�Z�[�W���擾
 */
TCHAR *message_get_res(const UINT id)
{
	static TCHAR buf[BUF_SIZE];

	*buf = TEXT('\0');
	LoadString(hInst, id, buf, BUF_SIZE - 1);
	return buf;
}

/*
 * message_copy_res - ���\�[�X���烁�b�Z�[�W���R�s�[
 */
TCHAR *message_copy_res(const UINT id, TCHAR *buf)
{
	*buf = TEXT('\0');
	LoadString(hInst, id, buf, BUF_SIZE - 1);
	return buf;
}
/* End of source */
