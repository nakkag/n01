/*
 * n01
 *
 * shortcut.c
 *
 * Copyright (C) 1996-2008 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

/* Include Files */

#define _INC_OLE
#include <windows.h>
#undef  _INC_OLE
#include <shlobj.h>

#include "shortcut.h"

/* Define */
#define BUF_SIZE						256

/* Global Variables */

/* Local Function Prototypes */

/*
 * get_directory - ファイルパスからディレクトリパスの取得
 */
static void get_directory(TCHAR *buf)
{
	TCHAR *p, *r;

	for (r = p = buf; *p != TEXT('\0'); p++) {
#ifndef UNICODE
		if(IsDBCSLeadByte(*p) == TRUE){
			p++;
			continue;
		}
#endif	// UNICODE
		if(*p == TEXT('\\') || *p == TEXT('/')){
			r = p;
		}
	}
	*r = TEXT('\0');
}

/*
 * create_shortcut - ショートカットファイルの作成
 */
HRESULT create_shortcut(TCHAR *pszShortcutFile, TCHAR *pszLink)
{
	HRESULT ret;
	IShellLink *psl;
	IPersistFile *ppf;
	TCHAR wkDir[MAX_PATH];
#ifndef UNICODE
	WCHAR buf[MAX_PATH];
#endif

	ret = CoCreateInstance(CLSID_ShellLink, NULL,
		CLSCTX_INPROC_SERVER, IID_IShellLink, (void **)&psl);
	if (!SUCCEEDED(ret)) {
		return ret;
	}
	ret = psl->QueryInterface(IID_IPersistFile, (void **)&ppf);
	if (!SUCCEEDED(ret)) {
		psl->Release();
		return ret;
	}
	psl->SetPath(pszShortcutFile);
	lstrcpy(wkDir, pszShortcutFile);
	get_directory(wkDir);
	psl->SetWorkingDirectory(wkDir);
	psl->SetArguments(TEXT(""));
	psl->SetIconLocation(pszShortcutFile, 0);
#ifndef UNICODE
	MultiByteToWideChar(CP_ACP, 0, pszLink, -1, buf, BUF_SIZE);
	ret = ppf->Save(buf, TRUE);
#else
	ret = ppf->Save(pszLink, TRUE);
#endif
	ppf->Release();
	psl->Release();
	return ret;
}
/* End of source */
