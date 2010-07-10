//
// jot - Text Editor for Windows Mobile
//   Copyright (C) 2007-2008, Aquamarine Networks.  <http://pandora.sblo.jp/>
//
//    This program is EveryOne's ware; you can redistribute it and/or modify it 
//    under the terms of the NYSL version 0.9982 or later. <http://www.kmonos.net/nysl/>
//

// jot.h : jot アプリケーションのメイン ヘッダー ファイル
//
#pragma once

#ifndef __AFXWIN_H__
	#error "PCH のこのファイルをインクルードする前に、'stdafx.h' をインクルードします"
#endif

//#ifdef POCKETPC2003_UI_MODEL
//#include "resourceppc.h"
//#endif 
//#ifdef SMARTPHONE2003_UI_MODEL
//#include "resourcesp.h"
//#endif
#include "resourceppc.h"

// CjotApp:
// このクラスの実装については、jot.cpp を参照してください。
//
#include "ini.h"

class CjotApp : public CWinApp
{
public:
	CjotApp();
	virtual	~CjotApp(){
		_OutputMemoryLeak();
	}
// オーバーライド
public:
	virtual BOOL InitInstance();

// 実装
public:
//#ifndef WIN32_PLATFORM_WFSP
	afx_msg void OnAppAbout();
//#endif // !WIN32_PLATFORM_WFSP

	DECLARE_MESSAGE_MAP()

protected:
	DWORD	m_lastInputMode;
	bool	m_bBackAsBs;

public:
	Ini	m_inifile;
	Ini	m_miscfile;

	void	SendShiftKey( bool down );
	void	ClickKey( UINT vchar );
	void	SendKeys( INPUT &inputKey , UINT vchar , bool bDown );
	void	ShowShiftUnlockCommandBar( BOOL shiftlock );
	void	PostMainWnd( UINT msg , WPARAM wp , LPARAM lp );
	void	GetMenu( CMenu *pMenu , UINT nID );

	//入力モード切替
	void	ChangeInputMode(int Mode);
	void	SaveInputMode();
	void	RestoreInputMode();

	void	MenuLoop( bool enter );

	bool	IsAssoc( TCHAR* key , TCHAR* val );
	void	SetAssoc( TCHAR* key , TCHAR* val );
	void	DelAssoc( TCHAR* key  );
	void 	SetExtAssoc( int mode=-1 );

	afx_msg void OnToolAssoc();

	void	SetBackAsBs( bool val ){	m_bBackAsBs = val; }
	bool	IsBackAsBs(){	return m_bBackAsBs;	}
};


extern CjotApp theApp;

BOOL GetOpenFileNameEx(OPENFILENAME* pofn);
BOOL GetSaveFileNameEx(OPENFILENAME* pofn);

#define WM_JOT_TEXTCHANGED	(WM_APP+100)		// 変更があった場合に親ウインドウにポストする
#define WM_JOT_SHIFTLOCK	(WM_APP+101)		// シフトロック状態が変更になった場合にポストする
#define WM_JOT_RESIZEMENU	(WM_APP+102)		// メニュー変更を行う場合にポストする
#define	WM_JOT_BOTTOMBAR	(WM_APP+103)		// 上にかぶせるメニューの指示
#define	WM_JOT_NOTIFYFOCUS	(WM_APP+104)		// フォーカスが来たことを親に知らせる


// 関連づけのレジストリ
#define REG_KEY_INI		_T(".ini")
#define REG_KEY_TXT		_T(".txt")
#define REG_VAL_INI		_T("jotini")
#define REG_VAL_TXT		_T("jottxt")
#define REG_DEF_TXT		_T("txtfile")


#define APPTITLE	_T("jot")
#define	CLASS_NAME	_T("JOT")

#define	INIFILE		_T("jot.ini")			// Iniファイル関連

#define	INI_SEC				_T("init")

#define	INI_KEY_FONTNAME	_T("fontname")
#define	INI_KEY_FONTSIZE	_T("fontsize")
#define	INI_KEY_TABSIZE		_T("tabsize")
//#define	INI_KEY_ALLFILE		_T("allfile")
#define	INI_KEY_MENUTENKEY	_T("menutenkey")
#define	INI_KEY_XSCRAWL		_T("xscrawl")
#define	INI_DEF_FONTNAME	_T("")
#define	INI_DEF_FONTSIZE	0
#define	INI_DEF_TABSIZE		4
//#define	INI_DEF_ALLFILE		0
#define	INI_DEF_MENUTENKEY	0
#define	INI_DEF_XSCRAWL		0
#define	INI_KEY_HSCROLL		_T("nowordwrap")
#define	INI_DEF_HSCROLL		0
#define	INI_KEY_DRAWSYMBOL		_T("drawsymbol")
#define	INI_DEF_DRAWSYMBOL		0
#define	INI_KEY_MRUNUM		_T("mrunum")
#define	INI_DEF_MRUNUM		5

#define	INI_SEC_MRU		_T("mrulist")
#define	INI_KEY_FNAME	_T("filename")
#define	INI_KEY_POS		_T("pos")

#define	INI_SEC_MRUFIND		_T("mrufind")
#define	INI_KEY_MRUFIND		_T("find")

#define	INI_SEC_MRUREPL		_T("mrurepl")
#define	INI_KEY_MRUREPL		_T("replace")

#define	INI_KEY_REGEX		_T("regex")
#define	INI_DEF_REGEX		0
#define	INI_KEY_IGNORECASE	_T("ignorecase")
#define	INI_DEF_IGNORECASE	0

#define	INI_KEY_FULLSC		_T("fullsc")
#define	INI_DEF_FULLSC		0

#define	INI_KEY_BOLDFONT	_T("boldfont")
#define	INI_KEY_DISAUTODETECT	_T("disautodetect")
#define	INI_DEF_BOLDFONT	0
#define	INI_DEF_DISAUTODETECT	0

#define	INI_KEY_FIRST		_T("first")
#define	INI_DEF_FIRST		0

#define	INI_KEY_UNDERLINE		_T("underline")
#define	INI_DEF_UNDERLINE		0

#define	INI_KEY_LINESPACING		_T("linespacing")
#define	INI_DEF_LINESPACING		0

#define	INI_KEY_NOTFOCUSIME		_T("notfocusime")
#define	INI_DEF_NOTFOCUSIME		0

#define	INI_KEY_NEWFILECHARCODE		_T("newfilecharcode")
#define	INI_DEF_NEWFILECHARCODE		3				// UTF-8

#define	INI_KEY_NEWFILELINEBREAK	_T("newfilelinebreak")
#define	INI_DEF_NEWFILELINEBREAK	1				// CRLF

#define	INI_KEY_BSWMCHAR		_T("bswmchar")
#define	INI_DEF_BSWMCHAR_PPC	0
#define	INI_DEF_BSWMCHAR_SP		1

#define	INI_KEY_BACKASBS		_T("backasbs")
#define	INI_DEF_BACKASBS_PPC		0
#define	INI_DEF_BACKASBS_SP		1

#define	INI_KEY_WORDWRAP_POS_P		_T("wordwrap_pos_p")
#define	INI_DEF_WORDWRAP_POS_P		0
#define	INI_KEY_WORDWRAP_POS_L		_T("wordwrap_pos_l")
#define	INI_DEF_WORDWRAP_POS_L		0
#define	INI_KEY_DRAWLINENUMBER		_T("drawlinenumber")
#define	INI_DEF_DRAWLINENUMBER		0

#define	MISCFILE		_T("misc.ini")			// その他設定ファイル関連
#define	MISCFILETEMP	_T("misc_.ini")			// その他設定ファイル関連
#define	MISC_SEC		_T("misc")
#define	MISC_KEY_COLOR_HIGHLIGHT		_T("color_highlight")
#define	MISC_KEY_COLOR_HIGHLIGHTTEXT	_T("color_highlighttext")
#define	MISC_KEY_COLOR_WINDOW			_T("color_window")
#define	MISC_KEY_COLOR_WINDOWTEXT		_T("color_windowtext")
#define	MISC_KEY_COLOR_UNDERLINE		_T("color_underline")

#define	MRUFILE			_T("file.mru")
#define	MRUFIND			_T("find.mru")
#define	MRUREPLACE		_T("repl.mru")
#define	TEMPLATEFILE	_T("template.ini")
#define	TEMPLATEFILETEMP	_T("template_%s.ini")

#define	HOMEPAGE_URL	_T("http://pandora.sblo.jp/")

#define	VERSION_STRINGS		_T("0.6.02")
