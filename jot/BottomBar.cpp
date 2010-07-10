//
// jot - Text Editor for Windows Mobile
//   Copyright (C) 2007-2008, Aquamarine Networks.  <http://pandora.sblo.jp/>
//
//    This program is EveryOne's ware; you can redistribute it and/or modify it 
//    under the terms of the NYSL version 0.9982 or later. <http://www.kmonos.net/nysl/>
//


// BottomBar.cpp : 実装ファイル
//

#include "stdafx.h"
#include "jot.h"
#include "BottomBar.h"



// CBottomBar

IMPLEMENT_DYNAMIC(CBottomBar, CStatic)

CBottomBar::CBottomBar()
{
	m_init=false;
}

CBottomBar::~CBottomBar()
{
	// メニュー閉じる
	MakeMenu( 0, false );
}


BEGIN_MESSAGE_MAP(CBottomBar, CStatic)
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_COMMAND(ID_BOTTOMBAR_CANCEL, &CBottomBar::OnBottombarCancel)
END_MESSAGE_MAP()



// CBottomBar メッセージ ハンドラ



void CBottomBar::OnSize(UINT nType, int cx, int cy)
{
	CStatic::OnSize(nType, cx, cy);

	if ( m_init ){
		Layout();
	}
}


BOOL CBottomBar::PreTranslateMessage(MSG* pMsg)
{
	if ( pMsg->message == WM_KEYDOWN){
		switch( pMsg->wParam ){
		case VK_ESCAPE:
			PostMessage( WM_COMMAND , ID_BOTTOMBAR_CANCEL , 0 );
			return TRUE;
		case VK_RETURN:
			return EnterKey();
		case VK_UP:
			return UpKey();
		case VK_DOWN:
			return DownKey();
		case VK_TAB:
			return TabKey();
		}
	}

	return CStatic::PreTranslateMessage(pMsg);
}

void CBottomBar::OnSetFocus(CWnd* pOldWnd)
{
	CStatic::OnSetFocus(pOldWnd);

	SetFocusItem();
}

void CBottomBar::OnBottombarCancel()
{
	m_parentBottomBar->NotifyBottomBar( ID_BOTTOMBAR_CANCEL );
}


BOOL CBottomBar::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// フォーカスのあるところが自分子供だったらコマンドを送る
	CWnd *pWnd = GetFocus();
	if ( pWnd!=NULL && pWnd->GetParent() == this ){
		if (pWnd->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
			return TRUE;
	}

	return CStatic::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}
