//
// jot - Text Editor for Windows Mobile
//   Copyright (C) 2007-2008, Aquamarine Networks.  <http://pandora.sblo.jp/>
//
//    This program is EveryOne's ware; you can redistribute it and/or modify it 
//    under the terms of the NYSL version 0.9982 or later. <http://www.kmonos.net/nysl/>
//

#pragma once
#include "enumfontcombo.h"
#include "afxwin.h"
#include "scrolldialog.h"


// CSettingDlg �_�C�A���O

class CSettingDlg : public CScrollDialog
{
	DECLARE_DYNAMIC(CSettingDlg)

public:
	CSettingDlg(CWnd* pParent = NULL);   // �W���R���X�g���N�^
	virtual ~CSettingDlg();

// �_�C�A���O �f�[�^
	enum { IDD = IDD_SETTING };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �T�|�[�g
	DECLARE_MESSAGE_MAP()

	CCommandBar m_wndCommandBar;
	CEnumFontCombo m_combofontname;
	virtual BOOL OnInitDialog();

	CComboBox m_combofontsize;
	CEdit m_edittabsize;
	virtual void OnOK();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	CButton m_chkMenuTenkey;
	CComboBox m_cmbXscrawl;
	CStatic m_staXscrawl;
public:
	CString	m_fontname;
	UINT	m_fontsize;
	UINT	m_tabsize;
	BOOL	m_bMenuTenkey;
	int		m_xscrawl;
	BOOL	m_bHScroll;
	BOOL	m_bDrawLinebreak;
	BOOL	m_bDrawTab;
	BOOL	m_bDrawWideSpace;
	BOOL	m_bDrawSpace;
	int		m_mrunum;
	BOOL	m_bFullSc;
	CButton m_CheckBold;
	BOOL	m_bBoldFont;
	BOOL	m_bDisAutoDetect;
	BOOL	m_bUnderline;
	int		m_linespacing;
	int m_NewFileCharCode;
	int m_NewFileLineBreak;
	unsigned char m_wordwrap_pos_p;
	unsigned char m_wordwrap_pos_l;
	BOOL	m_bDrawLineNumber;
};
