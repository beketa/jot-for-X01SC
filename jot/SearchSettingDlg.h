//
// jot - Text Editor for Windows Mobile
//   Copyright (C) 2007-2008, Aquamarine Networks.  <http://pandora.sblo.jp/>
//
//    This program is EveryOne's ware; you can redistribute it and/or modify it 
//    under the terms of the NYSL version 0.9982 or later. <http://www.kmonos.net/nysl/>
//

#pragma once

#include "scrolldialog.h"

// CSearchSettingDlg �_�C�A���O

class CSearchSettingDlg : public CScrollDialog
{
	DECLARE_DYNAMIC(CSearchSettingDlg)

public:
	CSearchSettingDlg(CWnd* pParent = NULL);   // �W���R���X�g���N�^
	virtual ~CSearchSettingDlg();

// �_�C�A���O �f�[�^
	enum { IDD = IDD_SEARCHSETTING };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �T�|�[�g

	DECLARE_MESSAGE_MAP()
public:
	BOOL m_bRegex;
	BOOL m_bIgnoreCase;
	virtual BOOL OnInitDialog();
};
