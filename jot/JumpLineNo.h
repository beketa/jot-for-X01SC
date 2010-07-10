//
// jot - Text Editor for Windows Mobile
//   Copyright (C) 2007-2008, Aquamarine Networks.  <http://pandora.sblo.jp/>
//
//    This program is EveryOne's ware; you can redistribute it and/or modify it 
//    under the terms of the NYSL version 0.9982 or later. <http://www.kmonos.net/nysl/>
//

#pragma once

#include	"BottomBar.h"
#include	"jot.h"
#include	"editor.h"
// CJumpLineNo

#define	ID_JLN_EDIT	40000


class CJumpLineNo : public CBottomBar
{
	DECLARE_DYNAMIC(CJumpLineNo)

	CStatic	m_label;
	CEditor	m_edit;
	CString	m_title;
public:
	CJumpLineNo();
	virtual ~CJumpLineNo();

	virtual	void	CreateContents(){
		m_title.LoadString( IDS_JUMPLINENO );
		m_label.Create( m_title , WS_VISIBLE|WS_CHILD  , CRect(0,0,0,0) ,this );
		m_edit.Create( WS_VISIBLE|WS_CHILD|WS_BORDER|ES_NUMBER
					, CRect(0,0,0,0) , this , ID_JLN_EDIT );
	
		MakeMenu( IDR_JUMPLINENO );
	}
	virtual	int		Layout(){
		CRect rect;
		GetClientRect(&rect);

		CDC	*pDc = GetDC();
		CSize sz = pDc->GetTextExtent( m_title );
		ReleaseDC(pDc);
		
		// ラベル
		CRect	lrect(0,4,sz.cx,sz.cy+4);
		m_label.MoveWindow( lrect );

		// エディットコントロール
		int	eh = sz.cy *15 /10;
		CRect	erect(lrect.right +1 , 1, rect.right - 1 , eh+1 );
		m_edit.MoveWindow( erect );
		
		return eh+7;
	}
	virtual	bool	EnterKey()
	{
		PostMessage( WM_COMMAND , ID_JUMP , 0 );
		return TRUE;
	}
	virtual	void	SetFocusItem()
	{
		m_edit.SetFocus();
	}

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnJump();
	afx_msg void OnUpdateJump(CCmdUI *pCmdUI);
};


