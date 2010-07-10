//
// jot - Text Editor for Windows Mobile
//   Copyright (C) 2007-2008, Aquamarine Networks.  <http://pandora.sblo.jp/>
//
//    This program is EveryOne's ware; you can redistribute it and/or modify it 
//    under the terms of the NYSL version 0.9982 or later. <http://www.kmonos.net/nysl/>
//

#pragma once


// CBottomBar

class IUseBottomBar
{
public:
	virtual	void NotifyBottomBar(UINT endcode ,CString *p1=NULL , CString *p2=NULL )=0;
};


class CBottomBar : public CStatic
{
	DECLARE_DYNAMIC(CBottomBar)

	int	m_height;
	bool	m_init;
	IUseBottomBar *m_parentBottomBar;
public:
	CBottomBar();
	virtual ~CBottomBar();
	void	Create(CWnd *parentWnd,IUseBottomBar *parentBottomBar){
		m_parentBottomBar = parentBottomBar;

		CStatic::Create( _T("") ,WS_CHILD|WS_VISIBLE|WS_BORDER  ,CRect( 0,0,100,100 ) ,parentWnd );
		CreateContents();
		m_height = Layout();
		m_init = true;
	}
	int	height(){return m_height;}

	void	MakeMenu(UINT id,bool make=true){
		theApp.PostMainWnd( WM_JOT_BOTTOMBAR , id , make?1:0 );
	}

	virtual	void	CreateContents()=0;
	virtual	int		Layout()=0;
	virtual	bool	EnterKey(){	return false;	}
	virtual	bool	UpKey()	  {	return false;	}
	virtual	bool	DownKey() {	return false;	}
	virtual	bool	TabKey() {	return false;	}
	virtual	void	SetFocusItem()=0;
	

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnBottombarCancel();
public:
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
};


