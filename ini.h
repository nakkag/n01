/*
 * n01
 *
 * Ini.h
 *
 * Copyright (C) 1996-2004 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

#ifndef _INC_INI_H
#define _INC_INI_H

/* Include Files */

/* Define */

/* Struct */

/* Function Prototypes */
BOOL ini_get_option(const TCHAR *ini_path);
BOOL ini_put_option(const TCHAR *ini_path);
BOOL ini_put_game_option(const TCHAR *ini_path);
BOOL ini_put_game_schedule(const TCHAR *ini_path);

#endif
/* End of source */
