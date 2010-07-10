//
// jot - Text Editor for Windows Mobile
//   Copyright (C) 2007-2008, Aquamarine Networks.  <http://pandora.sblo.jp/>
//
//    This program is EveryOne's ware; you can redistribute it and/or modify it 
//    under the terms of the NYSL version 0.9982 or later. <http://www.kmonos.net/nysl/>
//

// MyComboBox.cpp : 実装ファイル
//

#include "stdafx.h"
#include "jot.h"
#include "MyComboBox.h"


// CMyComboBox

IMPLEMENT_DYNAMIC(CMyComboBox, CComboBox)

CMyComboBox::CMyComboBox()
{

}

CMyComboBox::~CMyComboBox()
{
}


BEGIN_MESSAGE_MAP(CMyComboBox, CComboBox)
	ON_WM_KEYDOWN()
	ON_CONTROL_REFLECT(CBN_EDITCHANGE, &CMyComboBox::OnCbnEditchange)
END_MESSAGE_MAP()



// CMyComboBox メッセージ ハンドラ



BOOL CMyComboBox::PreTranslateMessage(MSG* pMsg)
{
	bool	ctrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
	if ( pMsg->message == WM_KEYDOWN){
		switch( pMsg->wParam ){
		case 'C':
			if ( ctrl ){
				Copy();
				return TRUE;
			}
			break;
		case 'X':
			if ( ctrl ) {
				Cut();
				return TRUE;
			}
			break;
		case 'V':
			if ( ctrl ) {
				Paste();
				return TRUE;
			}
			break;
		case VK_DOWN:
			if ( GetDroppedState() == FALSE ){
				ShowDropDown( TRUE );
			}
			break;
		}
	}

	return CComboBox::PreTranslateMessage(pMsg);
}

void CMyComboBox::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{

	CComboBox::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CMyComboBox::OnCbnEditchange()
{
	// ユーザがテキストを変更したらコンボを閉じる
	if ( GetDroppedState() != FALSE ){
		ShowDropDown( FALSE );
	}
}
