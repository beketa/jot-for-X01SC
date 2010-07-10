//
// jot - Text Editor for Windows Mobile
//   Copyright (C) 2007-2008, Aquamarine Networks.  <http://pandora.sblo.jp/>
//
//    This program is EveryOne's ware; you can redistribute it and/or modify it 
//    under the terms of the NYSL version 0.9982 or later. <http://www.kmonos.net/nysl/>
//

#pragma once

#include "scrolldialog.h"

// CSearchSettingDlg ダイアログ

class CSearchSettingDlg : public CScrollDialog
{
	DECLARE_DYNAMIC(CSearchSettingDlg)

public:
	CSearchSettingDlg(CWnd* pParent = NULL);   // 標準コンストラクタ
	virtual ~CSearchSettingDlg();

// ダイアログ データ
	enum { IDD = IDD_SEARCHSETTING };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
public:
	BOOL m_bRegex;
	BOOL m_bIgnoreCase;
	virtual BOOL OnInitDialog();
};
