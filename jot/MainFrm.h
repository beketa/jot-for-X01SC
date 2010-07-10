//
// jot - Text Editor for Windows Mobile
//   Copyright (C) 2007-2008, Aquamarine Networks.  <http://pandora.sblo.jp/>
//
//    This program is EveryOne's ware; you can redistribute it and/or modify it 
//    under the terms of the NYSL version 0.9982 or later. <http://www.kmonos.net/nysl/>
//

// MainFrm.h : CMainFrame クラスのインターフェイス
//


#pragma once

#include "ChildView.h"



class CMainFrame : public CFrameWnd
{
public:
	CMainFrame();
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
#endif
	DECLARE_DYNAMIC(CMainFrame)

public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

protected:  // コントロール バー用メンバ
	CCommandBar m_wndCommandBar;
	CCommandBar *m_pShiftlockCmdbar;
	CCommandBar	*m_pTempMenu;
	UINT	m_TempMenuID;
	CChildView    m_wndView;

// 生成された、メッセージ割り当て関数
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSetFocus(CWnd *pOldWnd);
	afx_msg LRESULT OnEnterMenuLoop(WPARAM wp, LPARAM lp);
	afx_msg LRESULT OnExitMenuLoop(WPARAM wp, LPARAM lp);
	afx_msg void OnExit();
	afx_msg void OnClose();

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg LRESULT OnShiftLock(WPARAM wp, LPARAM lp);
	afx_msg LRESULT OnResizeMenu(WPARAM wp, LPARAM lp);
	afx_msg LRESULT OnWrapMenu(WPARAM wp, LPARAM lp);
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg void OnShiftunlock();
	afx_msg void OnSize(UINT nType, int cx, int cy);

public:
	bool	IsMenuTenkey(){	return m_wndView.m_menutenkey!=FALSE;	}
	void	AddShortCut( CMenu *pMenu );	// メニュー項目の後ろにショートカットをつける
	void	ResizeMenu();

};


// 入力モードのレジストリ
#define REG_KEY_PHONESTAT		_T("Software\\Sharp\\PhoneStatus")

#define	INPUTMODE_ZEN_HIRAGANA	(0)
#define	INPUTMODE_ZEN_KATAKANA	(1)
#define	INPUTMODE_HAN_KATAKANA	(2)
#define	INPUTMODE_ZEN_ALPHA_L	(3)
#define	INPUTMODE_ZEN_ALPHA_S	(4)
#define	INPUTMODE_HAN_ALPHA_L	(5)
#define	INPUTMODE_HAN_ALPHA_S	(6)
#define	INPUTMODE_HAN_NUMERIC	(7)
