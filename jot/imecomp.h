//
// jot - Text Editor for Windows Mobile
//   Copyright (C) 2007-2008, Aquamarine Networks.  <http://pandora.sblo.jp/>
//
//    This program is EveryOne's ware; you can redistribute it and/or modify it 
//    under the terms of the NYSL version 0.9982 or later. <http://www.kmonos.net/nysl/>
//

#pragma once

#define IMECOMP_WND_CLASS				TEXT("nImeComp")

#define	WM_IMECOMP_COPMSTR		(WM_APP+1)
#define	WM_IMECOMP_ENDCOMP		(WM_APP+2)
#define	WM_IMECOMP_GETCANDPOS	(WM_APP+3)
#define	WM_IMECOMP_SETCOMPPOS	(WM_APP+4)
#define	WM_IMECOMP_SETCOLOR		(WM_APP+5)

#ifndef WM_SETPLATFORM
#define WM_SETPLATFORM			(WM_APP+8)			// nEdit‚Æ‚ ‚í‚¹‚é
#endif
#define WM_SETSPACING			(WM_APP + 11)

#ifdef __cplusplus
extern "C" {
#endif

/* Function Prototypes */
BOOL imecomp_regist(const HINSTANCE hInstance);

#ifdef __cplusplus
}
#endif

