/*
 * n01
 *
 * Font.c
 *
 * Copyright (C) 1996-2008 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

/* Include Files */
#define _INC_OLE
#include <windows.h>
#undef  _INC_OLE

/* Define */

/* Global Variables */

/* Local Function Prototypes */

/*
 * font_create - フォントを作成する
 */
HFONT font_create(const TCHAR *FontName, const int FontSize, const int weight, const BOOL under_line, const BOOL fixed)
{
	LOGFONT lf;
	HDC hdc;

	ZeroMemory(&lf, sizeof(LOGFONT));

	hdc = GetDC(NULL);
	lf.lfHeight = -(int)((FontSize * GetDeviceCaps(hdc, LOGPIXELSY)) / 72);
	ReleaseDC(NULL, hdc);

	lf.lfWidth = 0;
	lf.lfEscapement = 0;
	lf.lfOrientation = 0;
	lf.lfWeight = weight;
	lf.lfItalic = FALSE;
	lf.lfUnderline = (BYTE)under_line;
	lf.lfStrikeOut = FALSE;
//	lf.lfCharSet = GetTextCharset(hdc);
	lf.lfCharSet = DEFAULT_CHARSET;
	lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
	lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	lf.lfQuality = DEFAULT_QUALITY;
	lf.lfPitchAndFamily = (BYTE)(((fixed == TRUE) ? FIXED_PITCH : DEFAULT_PITCH) | FF_DONTCARE);
	lstrcpy(lf.lfFaceName, FontName);
	return CreateFontIndirect((CONST LOGFONT *)&lf);
}

/*
 * font_create_menu - メニューフォントを作成する
 */
HFONT font_create_menu(const int FontSize, const int weight, const BOOL under_line)
{
	NONCLIENTMETRICS ncMetrics;
	HDC hdc;

	// システムのメニュー用フォントの取得
	ZeroMemory(&ncMetrics, sizeof(NONCLIENTMETRICS));
	ncMetrics.cbSize = sizeof(NONCLIENTMETRICS);
	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncMetrics, 0);
	if (FontSize > 0) {
		// サイズを設定
		hdc = GetDC(NULL);
		ncMetrics.lfMenuFont.lfHeight = -(int)((FontSize * GetDeviceCaps(hdc, LOGPIXELSY)) / 72);
		ReleaseDC(NULL, hdc);
	}
	if (weight > 0) {
		// 太さを設定
		ncMetrics.lfMenuFont.lfWeight = weight;
	}
	ncMetrics.lfMenuFont.lfUnderline = (BYTE)under_line;
	// フォントの作成
	return CreateFontIndirect(&ncMetrics.lfMenuFont);
}
/* End of source */
