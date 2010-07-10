/*
 * nEdit
 *
 * nEdit.h
 *
 * Copyright (C) 1996-2005 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

#ifndef _INC_NEDIT_H
#define _INC_NEDIT_H

#ifdef __cplusplus
extern "C" {
#endif


/* Include Files */
#include <windows.h>
#include <tchar.h>
#ifdef OP_XP_STYLE
#include <uxtheme.h>
#include <tmschema.h>
#endif	// OP_XP_STYLE

/* Define */
#define NEDIT_WND_CLASS					TEXT("nEdit")

#define WM_GETBUFFERINFO				(WM_APP + 1)
#define WM_REFLECT						(WM_APP + 2)
#define WM_GETWORDWRAP					(WM_APP + 3)
#define WM_SETWORDWRAP					(WM_APP + 4)
#define WM_GETMEMSIZE					(WM_APP + 5)
#define WM_GETMEM						(WM_APP + 6)
#define WM_SETMEM						(WM_APP + 7)
#define WM_SETPLATFORM					(WM_APP + 8)
#define WM_DRAWSYMBOL					(WM_APP + 9)
#define WM_SETCOLOR						(WM_APP + 10)
#define WM_SETSPACING					(WM_APP + 11)
#define WM_SETFLAGS						(WM_APP + 12)
#define WM_GETWORDWRAPPOS				(WM_APP + 13)
#define WM_SETWORDWRAPPOS				(WM_APP + 14)

#define EM_GETREADONLY					(WM_APP + 100)
#define WM_GETLOCALMEM					(WM_APP + 101)

#ifndef EM_REDO
#define EM_REDO							(WM_USER + 84)
#endif
#ifndef EM_CANREDO
#define EM_CANREDO						(WM_USER + 85)
#endif

#define UNDO_TYPE_INPUT					1
#define UNDO_TYPE_DELETE				2

#define	DS_LINEBREAK		(0x00000001)
#define DS_TAB				(0x00000002)
#define	DS_SPACE			(0x00000004)
#define DS_WIDESPACE		(0x00000008)

#define	NEDIT_FLAGS_UNDERLINE	(0x00000001)
#define	NEDIT_FLAGS_NOTFOCUSIME	(0x00000002)
#define	NEDIT_FLAGS_BSWMCHAR	(0x00000004)
#define	NEDIT_FLAGS_LINENUMBER	(0x00000008)
#define	NEDIT_FLAGS_RULER		(0x00000010)
#define	NEDIT_FLAGS_ALL			(0xFFFFFFFF)

/* Function Prototypes */
BOOL nedit_regist(const HINSTANCE hInstance);

enum	NEditColor {
	NEDIT_COLOR_WINDOW	=		(0),
	NEDIT_COLOR_WINDOWTEXT,
	NEDIT_COLOR_HIGHLIGHT,
	NEDIT_COLOR_HIGHLIGHTTEXT,
	NEDIT_COLOR_UNDERLINE,

	NEDIT_COLOR_MAX,
};

#ifdef __cplusplus
}
#endif

#endif

/* End of source */
