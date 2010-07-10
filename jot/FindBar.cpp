//
// jot - Text Editor for Windows Mobile
//   Copyright (C) 2007-2008, Aquamarine Networks.  <http://pandora.sblo.jp/>
//
//    This program is EveryOne's ware; you can redistribute it and/or modify it 
//    under the terms of the NYSL version 0.9982 or later. <http://www.kmonos.net/nysl/>
//

// FindBar.cpp : 実装ファイル
//

#include "stdafx.h"
#include "jot.h"
#include "FindBar.h"


// CFindBar

IMPLEMENT_DYNAMIC(CFindBar, CBottomBar)

CFindBar::CFindBar(CString inistr , CMruFind *mru)
{
	m_inistr = inistr;
	m_pMruFind = mru;
}

CFindBar::~CFindBar()
{
}


BEGIN_MESSAGE_MAP(CFindBar, CBottomBar)
	ON_COMMAND(ID_FIND, &CFindBar::OnFind)
	ON_UPDATE_COMMAND_UI(ID_FIND, &CFindBar::OnUpdateFind)
	ON_COMMAND(ID_FIND_UP, &CFindBar::OnFindUp)
	ON_UPDATE_COMMAND_UI(ID_FIND_UP, &CFindBar::OnUpdateFindUp)
END_MESSAGE_MAP()



// CFindBar メッセージ ハンドラ



void CFindBar::OnFind()
{
	CString	s;
	m_combo.GetWindowTextW( s );
	m_pMruFind->Add( s );
	SetMruList( s );
	m_parentBottomBar->NotifyBottomBar( ID_FIND ,&s );

}

void CFindBar::OnUpdateFind(CCmdUI *pCmdUI)
{
	CString	s;
	m_combo.GetWindowTextW( s );

	pCmdUI->Enable( ( s.GetLength() > 0 )?TRUE:FALSE );
}

void CFindBar::OnFindUp()
{
	CString	s;
	m_combo.GetWindowTextW( s );
	m_pMruFind->Add( s );
	SetMruList( s );
	m_parentBottomBar->NotifyBottomBar( ID_FIND_UP ,&s );
}

void CFindBar::OnUpdateFindUp(CCmdUI *pCmdUI)
{
	CString	s;
	m_combo.GetWindowTextW( s );

	pCmdUI->Enable( ( s.GetLength() > 0 )?TRUE:FALSE );
}

