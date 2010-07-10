//
// jot - Text Editor for Windows Mobile
//   Copyright (C) 2007-2008, Aquamarine Networks.  <http://pandora.sblo.jp/>
//
//    This program is EveryOne's ware; you can redistribute it and/or modify it 
//    under the terms of the NYSL version 0.9982 or later. <http://www.kmonos.net/nysl/>
//

#pragma once

// CScrollDialog ダイアログ

class CScrollDialog : public CDialog
{
	DECLARE_DYNAMIC(CScrollDialog)

	static	bool	m_FullSc;

public:
	CScrollDialog(UINT DlgTemplate , CWnd* pParent = NULL);   // 標準コンストラクタ
	virtual ~CScrollDialog();

	static	void	SetFullSc( bool b ){	m_FullSc = b; }
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
	virtual BOOL OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);

	int m_nCurHeight;
	int m_nScrollPos;
	int	m_nDialogHeight;
	CCommandBar m_wndCommandBar;

	inline	void	CreateMenuBar( UINT id ){
		m_wndCommandBar.Create(this);
		m_wndCommandBar.InsertMenuBar(id);
	}

};

#define	WM_CHILD_SETFOCUS	(WM_APP+100)
