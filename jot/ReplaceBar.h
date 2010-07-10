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

// CReplaceBar
#define	ID_RB_FIND		40000
#define	ID_RB_REPLACE	40001


class CReplaceBar : public CBottomBar
{
	DECLARE_DYNAMIC(CReplaceBar)

	CStatic	m_label0;
	CStatic	m_label1;
	CMyComboBox	m_find;
	CMyComboBox	m_replace;
	CString	m_title0;
	CString	m_title1;
	CMruFind	*m_pMruFind;
	CMruFind	*m_pMruRepl;
	CString		m_inistr;

public:
	CReplaceBar(CString inistr , CMruFind *mru,CMruFind *rep);
	virtual ~CReplaceBar();

	virtual	void	CreateContents(){
		m_title0.LoadString( IDS_FIND );
		m_title1.LoadString( IDS_REPLACE );
		m_label0.Create( m_title0 , WS_VISIBLE|WS_CHILD  , CRect(0,0,0,0) ,this );
		m_label1.Create( m_title1 , WS_VISIBLE|WS_CHILD  , CRect(0,0,0,0) ,this );
		m_find.Create( WS_VISIBLE|WS_CHILD|WS_BORDER, CRect(0,0,0,0) , this , ID_RB_FIND );
		m_replace.Create( WS_VISIBLE|WS_CHILD|WS_BORDER, CRect(0,0,0,0) , this , ID_RB_REPLACE );

		SetMruList(&m_find,m_pMruFind,_T(""));
		SetMruList(&m_replace,m_pMruRepl,_T(""));

		m_find.SetWindowText( m_inistr );

		MakeMenu( IDR_REPLACE );
	}
	virtual	int		Layout(){
		CRect rect;
		GetClientRect(&rect);

		CDC	*pDc = GetDC();
		CSize sz = pDc->GetTextExtent( m_title0 );
		ReleaseDC(pDc);
		
		// ラベル
		CRect	lrect0(0,4,sz.cx,sz.cy+4);
		m_label0.MoveWindow( lrect0 );

		// エディットコントロール
		int	eh = sz.cy *15 /10;
		CRect	erect0(lrect0.right +1 , 1, rect.right - 1 , (eh+1)*5 );
		m_find.MoveWindow( erect0 );
		
		// 2段目
		int	h2 = eh + 1;

		// ラベル
		CRect	lrect1(0,h2+4,sz.cx,h2+sz.cy+4);
		m_label1.MoveWindow( lrect1 );

		// エディットコントロール
		CRect	erect1(lrect0.right +1 , h2+1, rect.right - 1 , h2+(eh+1)*5 );
		m_replace.MoveWindow( erect1 );

		return (eh+1)*2+6;
	}
	virtual	bool	EnterKey()
	{
		CWnd *pwnd = GetFocus();
		if ( pwnd !=NULL )	pwnd = pwnd->GetParent();
		if ( pwnd == &m_find ){
			if ( m_find.GetDroppedState() == FALSE ){
				m_replace.SetFocus();
				return TRUE;
			}
		}else if ( pwnd == &m_replace ){
			if ( m_replace.GetDroppedState() == FALSE ){
				PostMessage( WM_COMMAND , ID_REPLACE_DOWN , 0 );
				return TRUE;
			}
		}
		return FALSE;
	}
	virtual	void	SetFocusItem()
	{
		m_find.SetFocus();
	}
	virtual	bool	UpKey()
	{
		CWnd *pwnd = GetFocus();
		if ( pwnd !=NULL )	pwnd = pwnd->GetParent();
		if ( pwnd == &m_find ){
			if ( m_find.GetDroppedState() == FALSE ){
				PostMessage( WM_COMMAND , ID_BOTTOMBAR_CANCEL , 0 );
				return TRUE;
			}
		}else if ( pwnd == &m_replace ){
			if ( m_replace.GetDroppedState() == FALSE ){
				m_find.SetFocus();
				return TRUE;
			}
		}
		return FALSE;
	}
	virtual	bool	TabKey() 
	{
		CWnd *pwnd = GetFocus();
		if ( pwnd !=NULL )	pwnd = pwnd->GetParent();
		if ( pwnd == &m_find ){
			if ( m_find.GetDroppedState() == FALSE ){
				m_replace.SetFocus();
				return TRUE;
			}
		}else if ( pwnd == &m_replace ){
			if ( m_replace.GetDroppedState() == FALSE ){
				m_find.SetFocus();
				return TRUE;
			}
		}
		return FALSE;
	}

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnReplaceAll();
	afx_msg void OnUpdateReplaceAll(CCmdUI *pCmdUI);
	afx_msg void OnReplaceDown();
	afx_msg void OnUpdateReplaceDown(CCmdUI *pCmdUI);
	afx_msg void OnReplaceUp();
	afx_msg void OnUpdateReplaceUp(CCmdUI *pCmdUI);
	afx_msg void OnReplace();
	afx_msg void OnUpdateReplace(CCmdUI *pCmdUI);

	void	SetMruList(CComboBox *pcmb ,CMruFind	*mru, CString s){
		bool	left=false;
		// 追加文字列と同じものを残して残りを削除
		for (int i = pcmb->GetCount()-1; i >= 0; i--){
			CString	sel;
			pcmb->GetLBText( i , sel );
			if ( !s.IsEmpty() && sel.Compare( s ) != 0 ){
			   pcmb->DeleteString( i );
			}else{
				left = true;
			}
		}
		int	start = 0;
		if ( left ){
			start = 1;
		}
		// 上で同じものがあれば二個目から処理
		for( int i=start ; i<mru->GetCount() ; i++ ){
			pcmb->AddString( mru->Getstr(i) );
		}
	}
	CString	GetFindStr(){
		CString	s;
		m_find.GetWindowTextW( s );
		m_pMruFind->Add( s );
		SetMruList( &m_find , m_pMruFind , s );
		return s;
	}
	CString	GetReplaceStr(){
		CString	r;
		m_replace.GetWindowTextW( r );
		m_pMruRepl->Add( r );
		SetMruList( &m_replace , m_pMruRepl , r );
		return r;
	}


};


