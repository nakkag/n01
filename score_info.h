/*
 * n01
 *
 * score_info.h
 *
 * Copyright (C) 1996-2005 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

#ifndef _INC_SCORE_INFO_H
#define _INC_SCORE_INFO_H

/* Include Files */

/* Define */

/* Struct */

/* Function Prototypes */
BOOL score_info_free(SCORE_INFO *si);
BOOL score_info_init(const HWND hWnd, SCORE_INFO *si, GAME_INFO *gi, const BOOL p_init);
BOOL score_info_copy(SCORE_INFO *to_si, const SCORE_INFO *from_si);
BOOL score_history_set(const HWND hWnd, SCORE_HISTORY *sh, SCORE_INFO *si);
BOOL score_history_free(SCORE_HISTORY *sh);

#endif
/* End of source */
