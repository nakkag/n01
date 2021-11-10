/*
 * n01
 *
 * n01_plugin_template.c
 *
 * Copyright (C) 1996-2005 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

/* Include Files */
#define _INC_OLE
#include <windows.h>
#undef  _INC_OLE

#include "n01_plugin.h"

/* Define */

/* Global Variables */

/* Local Function Prototypes */

/*
 * get_plugin_info - �v���O�C�����擾
 *
 *	����:
 *		hWnd - �Ăяo�����E�B���h�E
 *		index - �擾�̃C���f�b�N�X (0�`)
 *		pgi - �v���O�C���擾���
 *
 *	�߂�l:
 *		TRUE - ���Ɏ擾����v���O�C������
 *		FALSE - �擾�̏I��
 */
__declspec(dllexport) BOOL CALLBACK get_plugin_info(const HWND hWnd, const int index, PLUGIN_GET_INFO *pgi)
{
	switch (index) {
	case 0:
		lstrcpy(pgi->title, TEXT("�^�C�g��"));
		lstrcpy(pgi->func_name, TEXT("func_plugin"));
		lstrcpy(pgi->cmd_line, TEXT(""));
		return TRUE;

	case 1:
		return FALSE;
	}
	return FALSE;
}

/*
 * func_plugin_initialize - �v���O�C���̏�����
 *
 *	����:
 *		hWnd - �Ăяo�����E�B���h�E
 *		pei - �v���O�C�����s���
 *
 *	�߂�l:
 *		�v���O�C���̌Ăяo�����@
 *
 *	���l:
 *		�֐����� �v���O�C����_initialize
 */
__declspec(dllexport) BOOL CALLBACK func_plugin_initialize(const HWND hWnd, PLUGIN_EXEC_INFO *pei)
{
	// �߂�l�ɌĂяo�����@�̑g�ݍ��킹��ݒ肷��
	return (CALLTYPE_START | CALLTYPE_END | CALLTYPE_MENU | CALLTYPE_SET_START | CALLTYPE_SET_END | CALLTYPE_LEG_START | CALLTYPE_LEG_END | CALLTYPE_INPUT_START | CALLTYPE_INPUT_END | CALLTYPE_FINISH | CALLTYPE_PREV_LEG);
}

/*
 * func_plugin_property - �v���O�C���̃v���p�e�B
 *
 *	����:
 *		hWnd - �Ăяo�����E�B���h�E
 *		pei - �v���O�C�����s���
 *
 *	�߂�l:
 *		PLUGIN_
 *
 *	���l:
 *		�֐����� �v���O�C����_property
 *		�ݒ肷��v���p�e�B�������ꍇ�A���̊֐��͕K�v�����B
 */
__declspec(dllexport) BOOL CALLBACK func_plugin_property(const HWND hWnd, PLUGIN_EXEC_INFO *pei)
{
	// �v���p�e�B��ݒ肵���ꍇ�� PLUGIN_SUCCEED ��Ԃ�
	// �ݒ肷��v���p�e�B�������ꍇ�� PLUGIN_ERROR ��Ԃ�
	return PLUGIN_ERROR;
}

/*
 * func_plugin - �v���O�C������
 *
 *	����:
 *		hWnd - �Ăяo�����E�B���h�E
 *		call_type - �Ăяo�����@
 *		pei - �v���O�C�����s���
 *		param - �Ăяo�������
 *
 *	�߂�l:
 *		PLUGIN_
 */
__declspec(dllexport) int CALLBACK func_plugin(const HWND hWnd, const int call_type, PLUGIN_EXEC_INFO *pei, int param)
{
	switch (call_type) {
	case CALLTYPE_START:
		// �v���O�����J�n
		break;

	case CALLTYPE_END:
		// �v���O�����I��
		// param - �I�����@ (WM_CLOSE or WM_QUERYENDSESSION)
		// �v���O�������I�����Ȃ��ꍇ�� PLUGIN_CANCEL ��Ԃ�
		break;

	case CALLTYPE_MENU:
		// ���j���[
		break;

	case CALLTYPE_SET_START:
		// �Z�b�g�J�n
		// param - ���݂̃Z�b�g��
		break;

	case CALLTYPE_SET_END:
		// �Z�b�g�I��
		// param - ���݂̃Z�b�g��
		break;

	case CALLTYPE_LEG_START:
		// ���b�O�J�n
		// param - ���݂̃��b�O��
		break;

	case CALLTYPE_LEG_END:
		// ���b�O�I��
		// param - ���݂̃��b�O��
		break;

	case CALLTYPE_INPUT_START:
		// �X�R�A���͊J�n
		break;

	case CALLTYPE_INPUT_END:
		// �X�R�A���͊m��
		// param - ���͂����X�R�A�̒l
		// �߂�l�� PLUGIN_CANCEL ��Ԃ��ƃX�R�A���͂��m�肵�Ȃ�
		// �X�R�A(pei->si->leg[pei->si->current_leg].score)��ύX�����ꍇ�� PLUGIN_DATA_MODIFIED ��Ԃ�
		break;

	case CALLTYPE_FINISH:
		// �t�B�j�b�V��
		// param - �t�B�j�b�V���{�� (-1, -2, -3)
		// param �� 0 �̏ꍇ�̓~�h���t�H�[�f�B�h���ł̃t�B�j�b�V��
		// �߂�l�� PLUGIN_CANCEL ��Ԃ��ƃt�B�j�b�V�����L�����Z������
		// �X�R�A(pei->si->leg[pei->si->current_leg].score)��ύX�����ꍇ�� PLUGIN_DATA_MODIFIED ��Ԃ�
		break;

	case CALLTYPE_PREV_LEG:
		// �O�̃��b�O�ɖ߂�
		// �߂�l�� PLUGIN_CANCEL ��Ԃ��ƑO�̃��b�O�ɖ߂�@�\�𖳌��ɂ��܂��B
		break;
	}
	return PLUGIN_SUCCEED;
}
/* End of source */
