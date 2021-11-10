/*
 * n01
 *
 * nEdit.h
 *
 * Copyright (C) 1996-2006 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

#ifndef _INC_NEDIT_H
#define _INC_NEDIT_H

/* Include Files */
#include <windows.h>
#include <tchar.h>

/* Define */
#define NEDIT_WND_CLASS					TEXT("n01nEdit")

#define WM_GETBUFFERINFO				(WM_APP + 1)
#define WM_REFLECT						(WM_APP + 2)
#define WM_GETLOCK						(WM_APP + 3)
#define WM_SETLOCK						(WM_APP + 4)
#define WM_EDIT_INIT_COLOR				(WM_APP + 5)
#define WM_REFRESH						(WM_APP + 6)
#define WM_SET_REFRESH_MODE				(WM_APP + 7)

#ifndef EM_REDO
#define EM_REDO							(WM_USER + 84)
#endif
#ifndef EM_CANREDO
#define EM_CANREDO						(WM_USER + 85)
#endif

#define UNDO_TYPE_INPUT					1
#define UNDO_TYPE_DELETE				2

/* Struct */
typedef struct _UNDO {
	int type;

	DWORD st;
	DWORD len;
	TCHAR *buf;
} UNDO;

typedef struct _BUFFER {
	// �ێ����Ă�����e
	TCHAR *buf;
	DWORD buf_size;
	DWORD buf_len;
	DWORD view_len;
	DWORD limit_len;

	// ���̓o�b�t�@
	TCHAR *input_buf;
	DWORD input_size;
	DWORD input_len;

	UNDO *undo;
	int undo_size;
	int undo_len;
	int undo_pos;

	// ���͊J�n�ʒu
	TCHAR *ip;
	DWORD ip_len;
	// �폜�J�n�ʒu
	TCHAR *dp;
	DWORD del_len;

	// �L�����b�g�̈ʒu
	DWORD cp;
	// �I���ʒu
	DWORD sp;

	// �`��p���
	HDC mdc;
	HBITMAP ret_bmp;
	HFONT hfont;
	HFONT ret_font;

	// �t�H���g
	int font_height;
	int char_width;
	BOOL fixed;
	BOOL bold;

	HBRUSH back_brush;

	HDC ellipse_dc;
	HBITMAP ellipse_ret_bmp;
	HPEN ton_circle_effect_pen;
	HPEN ton_circle_pen;
	int pen_size;

	// �C���t���O
	BOOL modified;
	// ���b�N�t���O
	BOOL lock;
	// ���̓��[�h
	BOOL insert_mode;
	// �}�E�X���t���O
	BOOL mousedown;
	// �X�V���[�h
	BOOL refresh_mode;
} BUFFER;

/* Function Prototypes */
BOOL nedit_regist(const HINSTANCE hInstance);
HWND nedit_create(const HINSTANCE hInstance, const HWND pWnd, int id);

#endif
/* End of source */
