/*
 * n01
 *
 * option_player.c
 *
 * Copyright (C) 1996-2008 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

/* Include Files */
#define _INC_OLE
#include <windows.h>
#undef  _INC_OLE

#include "General.h"
#include "option.h"

#include "resource.h"

/* Define */

/* Global Variables */
extern OPTION_INFO op;
extern int prop_ret;

/* Local Function Prototypes */

/*
 * option_player_proc - ウィンドウプロシージャ
 */
BOOL CALLBACK option_player_proc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		CheckDlgButton(hDlg, IDC_CHECK_AVG_PPR, op.opi.avg_per_round);

		CheckDlgButton(hDlg, IDC_CHECK_NAME, op.opi.name);
		CheckDlgButton(hDlg, IDC_CHECK_FIRST, op.opi.first);

		CheckDlgButton(hDlg, IDC_CHECK_SETS, op.opi.total_sets);
		CheckDlgButton(hDlg, IDC_CHECK_LEGS, op.opi.total_legs);
		CheckDlgButton(hDlg, IDC_CHECK_TONS, op.opi.total_tons);
		CheckDlgButton(hDlg, IDC_CHECK_100, op.opi.total_100);
		CheckDlgButton(hDlg, IDC_CHECK_140, op.opi.total_140);
		CheckDlgButton(hDlg, IDC_CHECK_180S, op.opi.total_180s);
		CheckDlgButton(hDlg, IDC_CHECK_HIGH_OFF, op.opi.total_high_off);
		CheckDlgButton(hDlg, IDC_CHECK_SHORT, op.opi.total_short);
		CheckDlgButton(hDlg, IDC_CHECK_LONG, op.opi.total_long);

		CheckDlgButton(hDlg, IDC_CHECK_AVG_SCORE, op.opi.avg_score);
		CheckDlgButton(hDlg, IDC_CHECK_AVG_DARTS, op.opi.avg_darts);
		CheckDlgButton(hDlg, IDC_CHECK_AVG_FIRST9, op.opi.avg_first9);
		CheckDlgButton(hDlg, IDC_CHECK_AVG_CHECK_OUT, op.opi.avg_check_out);
		CheckDlgButton(hDlg, IDC_CHECK_AVG_CHECK_OUT_COUNT, op.opi.avg_check_out_count);
		CheckDlgButton(hDlg, IDC_CHECK_AVG_KEEP, op.opi.avg_keep);
		CheckDlgButton(hDlg, IDC_CHECK_AVG_KEEP_COUNT, op.opi.avg_keep_count);
		CheckDlgButton(hDlg, IDC_CHECK_AVG_BREAK, op.opi.avg_break);
		CheckDlgButton(hDlg, IDC_CHECK_AVG_BREAK_COUNT, op.opi.avg_break_count);

		CheckDlgButton(hDlg, IDC_CHECK_ARRANGE, op.opi.arrange);

		SendMessage(hDlg, WM_COMMAND, IDC_CHECK_AVG_CHECK_OUT, 0);
		SendMessage(hDlg, WM_COMMAND, IDC_CHECK_AVG_KEEP, 0);
		SendMessage(hDlg, WM_COMMAND, IDC_CHECK_AVG_BREAK, 0);
		break;

	case WM_DESTROY:
		break;

	case WM_NOTIFY:
		return option_notify_proc(hDlg, msg, wParam, lParam);

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_CHECK_AVG_CHECK_OUT:
			if (IsDlgButtonChecked(hDlg, IDC_CHECK_AVG_CHECK_OUT) == BST_CHECKED) {
				EnableWindow(GetDlgItem(hDlg, IDC_CHECK_AVG_CHECK_OUT_COUNT), TRUE);
			} else {
				EnableWindow(GetDlgItem(hDlg, IDC_CHECK_AVG_CHECK_OUT_COUNT), FALSE);
			}
			break;

		case IDC_CHECK_AVG_KEEP:
			if (IsDlgButtonChecked(hDlg, IDC_CHECK_AVG_KEEP) == BST_CHECKED) {
				EnableWindow(GetDlgItem(hDlg, IDC_CHECK_AVG_KEEP_COUNT), TRUE);
			} else {
				EnableWindow(GetDlgItem(hDlg, IDC_CHECK_AVG_KEEP_COUNT), FALSE);
			}
			break;

		case IDC_CHECK_AVG_BREAK:
			if (IsDlgButtonChecked(hDlg, IDC_CHECK_AVG_BREAK) == BST_CHECKED) {
				EnableWindow(GetDlgItem(hDlg, IDC_CHECK_AVG_BREAK_COUNT), TRUE);
			} else {
				EnableWindow(GetDlgItem(hDlg, IDC_CHECK_AVG_BREAK_COUNT), FALSE);
			}
			break;

		case IDOK:
			op.opi.avg_per_round = IsDlgButtonChecked(hDlg, IDC_CHECK_AVG_PPR);

			op.opi.name = IsDlgButtonChecked(hDlg, IDC_CHECK_NAME);
			op.opi.first = IsDlgButtonChecked(hDlg, IDC_CHECK_FIRST);

			op.opi.total_sets = IsDlgButtonChecked(hDlg, IDC_CHECK_SETS);
			op.opi.total_legs = IsDlgButtonChecked(hDlg, IDC_CHECK_LEGS);
			op.opi.total_tons = IsDlgButtonChecked(hDlg, IDC_CHECK_TONS);
			op.opi.total_100 = IsDlgButtonChecked(hDlg, IDC_CHECK_100);
			op.opi.total_140 = IsDlgButtonChecked(hDlg, IDC_CHECK_140);
			op.opi.total_180s = IsDlgButtonChecked(hDlg, IDC_CHECK_180S);
			op.opi.total_high_off = IsDlgButtonChecked(hDlg, IDC_CHECK_HIGH_OFF);
			op.opi.total_short = IsDlgButtonChecked(hDlg, IDC_CHECK_SHORT);
			op.opi.total_long = IsDlgButtonChecked(hDlg, IDC_CHECK_LONG);

			op.opi.avg_score = IsDlgButtonChecked(hDlg, IDC_CHECK_AVG_SCORE);
			op.opi.avg_darts = IsDlgButtonChecked(hDlg, IDC_CHECK_AVG_DARTS);
			op.opi.avg_first9 = IsDlgButtonChecked(hDlg, IDC_CHECK_AVG_FIRST9);
			op.opi.avg_check_out = IsDlgButtonChecked(hDlg, IDC_CHECK_AVG_CHECK_OUT);
			op.opi.avg_check_out_count = IsDlgButtonChecked(hDlg, IDC_CHECK_AVG_CHECK_OUT_COUNT);
			op.opi.avg_keep = IsDlgButtonChecked(hDlg, IDC_CHECK_AVG_KEEP);
			op.opi.avg_keep_count = IsDlgButtonChecked(hDlg, IDC_CHECK_AVG_KEEP_COUNT);
			op.opi.avg_break = IsDlgButtonChecked(hDlg, IDC_CHECK_AVG_BREAK);
			op.opi.avg_break_count = IsDlgButtonChecked(hDlg, IDC_CHECK_AVG_BREAK_COUNT);

			op.opi.arrange = IsDlgButtonChecked(hDlg, IDC_CHECK_ARRANGE);

			prop_ret = 1;
			break;

		case IDPCANCEL:
			break;
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;
}
/* End of source */
