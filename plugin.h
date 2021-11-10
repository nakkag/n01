/*
 * n01
 *
 * plugin.h
 *
 * Copyright (C) 1996-2006 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

#ifndef _INC_PLUGIN_H
#define _INC_PLUGIN_H

/* Include Files */

/* Define */
#define BUF_SIZE						256

#define CALLTYPE_START					1				// �v���O�����J�n
#define CALLTYPE_END					2				// �v���O�����I��
#define CALLTYPE_MENU					4				// ���j���[
#define CALLTYPE_SET_START				8				// �Z�b�g�J�n
#define CALLTYPE_SET_END				16				// �Z�b�g�I��
#define CALLTYPE_LEG_START				32				// ���b�O�J�n
#define CALLTYPE_LEG_END				64				// ���b�O�I��
#define CALLTYPE_INPUT_START			128				// �X�R�A���͊J�n
#define CALLTYPE_INPUT_END				256				// �X�R�A���͊m��
#define CALLTYPE_FINISH					512				// �t�B�j�b�V��
#define CALLTYPE_PREV_LEG				1024			// �O�̃��b�O�ɖ߂�

// �v���O�C���߂�l
#define PLUGIN_ERROR					0				// �v���O�C���̃G���[
#define PLUGIN_SUCCEED					1				// �v���O�C���̐���I��
#define PLUGIN_CANCEL					2				// �ȍ~�̏������L�����Z�� (CALLTYPE_END, CALLTYPE_INPUT_END, CALLTYPE_FINISH)
#define PLUGIN_DATA_MODIFIED			4				// �f�[�^�ύX���� (CALLTYPE_INPUT_END, CALLTYPE_FINISH)

/* Struct */
// �v���O�C���擾���
typedef struct _PLUGIN_GET_INFO {
	DWORD struct_size;					// �\���̂̃T�C�Y

	TCHAR title[BUF_SIZE];
	TCHAR func_name[BUF_SIZE];
	TCHAR cmd_line[BUF_SIZE];
} PLUGIN_GET_INFO;

// �v���O�C�����s���
typedef struct _PLUGIN_EXEC_INFO {
	DWORD struct_size;					// �\���̂̃T�C�Y

	TCHAR cmd_line[BUF_SIZE];			// �v���O�C���ݒ�Ŏw�肵���R�}���h���C��
	LPARAM lParam1;						// �v���O�C���ɑΉ�����long�l
	LPARAM lParam2;						// �v���O�C���ɑΉ�����long�l

	struct _SCORE_INFO *si;
} PLUGIN_EXEC_INFO;

// �v���O�C�����
typedef struct _PLUGIN_INFO {
	TCHAR title[BUF_SIZE];
	TCHAR lib_file_path[BUF_SIZE];
	TCHAR func_name[BUF_SIZE];
	int call_type;						// CALLTYPE_

	HANDLE lib;
	FARPROC func;

	PLUGIN_EXEC_INFO pei;

	int index;

	int ctrl;
	int key;
} PLUGIN_INFO;

/* Function Prototypes */
void plugin_create_accelerator(void);
void plugin_free_accelerator(void);
BOOL plugin_initialize(const HWND hWnd, struct _SCORE_INFO *si);
void plugin_free(void);
int plugin_execute(const HWND hWnd, PLUGIN_INFO *pi, const int call_type, struct _SCORE_INFO *si, int param);
int plugin_execute_all(const HWND hWnd, const int call_type, struct _SCORE_INFO *si, int param);

#endif
/* End of source */
