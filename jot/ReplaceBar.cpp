//
// jot - Text Editor for Windows Mobile
//   Copyright (C) 2007-2008, Aquamarine Networks.  <http://pandora.sblo.jp/>
//
//    This program is EveryOne's ware; you can redistribute it and/or modify it 
//    under the terms of the NYSL version 0.9982 or later. <http://www.kmonos.net/nysl/>
//

// ReplaceBar.cpp : 実装ファイル
//

#include "stdafx.h"
#include "jot.h"
#include "ReplaceBar.h"


// CReplaceBar

IMPLEMENT_DYNAMIC(CReplaceBar, CBottomBar)

CReplaceBar::CReplaceBar(CString inistr , CMruFind *mru,CMruFind *rep)
{
	m_inistr = inistr;
	m_pMruFind = mru;
	m_pMruRepl = rep;

}

CReplaceBar::~CReplaceBar()
{
}


BEGIN_MESSAGE_MAP(CReplaceBar, CBottomBar)
	ON_COMMAND(ID_REPLACE_ALL, &CReplaceBar::OnReplaceAll)
	ON_COMMAND(ID_REPLACE_DOWN, &CReplaceBar::OnReplaceDown)
	ON_COMMAND(ID_REPLACE_UP, &CReplaceBar::OnReplaceUp)
	ON_COMMAND(ID_REPLACE, &CReplaceBar::OnReplace)

	ON_UPDATE_COMMAND_UI(ID_REPLACE_ALL, &CReplaceBar::OnUpdateReplace)
	ON_UPDATE_COMMAND_UI(ID_REPLACE_DOWN, &CReplaceBar::OnUpdateReplace)
	ON_UPDATE_COMMAND_UI(ID_REPLACE_UP, &CReplaceBar::OnUpdateReplace)
	ON_UPDATE_COMMAND_UI(ID_REPLACE, &CReplaceBar::OnUpdateReplace)
END_MESSAGE_MAP()



// CReplaceBar メッセージ ハンドラ



void CReplaceBar::OnReplaceAll()
{
	CString	s = GetFindStr();
	CString	r = GetReplaceStr();
	m_parentBottomBar->NotifyBottomBar( ID_REPLACE_ALL ,&s ,&r);
}




void CReplaceBar::OnReplaceDown()
{
	CString	s = GetFindStr();
	m_parentBottomBar->NotifyBottomBar( ID_FIND ,&s );
}

void CReplaceBar::OnReplaceUp()
{
	CString	s = GetFindStr();
	m_parentBottomBar->NotifyBottomBar( ID_FIND_UP ,&s );
}


void CReplaceBar::OnReplace()
{
	CString	s = GetFindStr();
	CString	r = GetReplaceStr();
	m_parentBottomBar->NotifyBottomBar( ID_REPLACE ,&s ,&r);
}

void CReplaceBar::OnUpdateReplace(CCmdUI *pCmdUI)
{
	CString	s;
	m_find.GetWindowTextW( s );

	pCmdUI->Enable( ( s.GetLength() > 0 )?TRUE:FALSE );
}
