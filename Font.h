/*
 * n01
 *
 * Font.h
 *
 * Copyright (C) 1996-2004 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

#ifndef _INC_FONT_H
#define _INC_FONT_H

/* Include Files */

/* Define */

/* Struct */

/* Function Prototypes */
HFONT font_create(const TCHAR *FontName, const int FontSize, const int weight, const BOOL under_line, const BOOL fixed);
HFONT font_create_menu(const int FontSize, const int weight, const BOOL under_line);

#endif
/* End of source */
