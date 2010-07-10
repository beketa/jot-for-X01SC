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
#include	"mycombobox.h"
#include	"mrufind.h"

// CFindBar
#define	ID_FB_EDIT	40000

class CFindBar : public CBottomBar
{
	DECLARE_DYNAMIC(CFindBar)

	CStatic	m_label;
	//CEditor	m_edit;
	CString	m_title;
	CMyComboBox	m_combo;
	CMruFind	*m_pMruFind;
	CString		m_inistr;

public:
	CFindBar(CString ini ,CMruFind *mru);
	virtual ~CFindBar();

	virtual	void	CreateContents(){
		m_title.LoadString( IDS_FIND );
		m_label.Create( m_title , WS_VISIBLE|WS_CHILD  , CRect(0,0,0,0) ,this );
		//m_edit.Create( WS_VISIBLE|WS_CHILD|WS_BORDER
		//			, CRect(0,0,0,0) , this , ID_FB_EDIT );
		m_combo.Create( WS_VISIBLE|WS_CHILD|WS_BORDER|CBS_AUTOHSCROLL|CBS_DROPDOWN ,
					CRect(0,0,0,0) , this , ID_FB_EDIT );

		m_combo.SetWindowText( m_inistr );

		SetMruList(_T(""));
		MakeMenu( IDR_FIND );
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
		CRect	erect(lrect.right +1 , 1, rect.right - 1 , (eh+1)*5 );
		m_combo.MoveWindow( erect );
		
		return eh+7;
	}
	virtual	bool	EnterKey()
	{
		if ( m_combo.GetDroppedState() == FALSE ){
			PostMessage( WM_COMMAND , ID_FIND , 0 );
			return TRUE;
		}
		return FALSE;
	}
	virtual	void	SetFocusItem()
	{
		m_combo.SetFocus();
	}
	virtual	bool	UpKey()
	{
		if ( m_combo.GetDroppedState() == FALSE ){
			PostMessage( WM_COMMAND , ID_BOTTOMBAR_CANCEL , 0 );
			return TRUE;
		}
		return FALSE;
	}
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnFind();
	afx_msg void OnUpdateFind(CCmdUI *pCmdUI);
	afx_msg void OnFindUp();
	afx_msg void OnUpdateFindUp(CCmdUI *pCmdUI);

	void	SetMruList(CString s){
		bool	left=false;
		// 追加文字列と同じものを残して残りを削除
		for (int i = m_combo.GetCount()-1; i >= 0; i--){
			CString	sel;
			m_combo.GetLBText( i , sel );
			if ( !s.IsEmpty() && sel.Compare( s ) != 0 ){
			   m_combo.DeleteString( i );
			}else{
				left = true;
			}
		}
		int	start = 0;
		if ( left ){
			start = 1;
		}
		// 上で同じものがあれば二個目から処理
		for( int i=start ; i<m_pMruFind->GetCount() ; i++ ){
			m_combo.AddString( m_pMruFind->Getstr(i) );
		}
	}

};


