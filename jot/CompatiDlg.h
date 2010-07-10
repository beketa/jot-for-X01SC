//
// jot - Text Editor for Windows Mobile
//   Copyright (C) 2007-2008, Aquamarine Networks.  <http://pandora.sblo.jp/>
//
//    This program is EveryOne's ware; you can redistribute it and/or modify it 
//    under the terms of the NYSL version 0.9982 or later. <http://www.kmonos.net/nysl/>
//

#pragma once

#include "scrolldialog.h"

// CCompatiDlg ダイアログ

class CCompatiDlg : public CScrollDialog
{
	DECLARE_DYNAMIC(CCompatiDlg)

public:
	CCompatiDlg(CWnd* pParent = NULL);   // 標準コンストラクタ
	virtual ~CCompatiDlg();

// ダイアログ データ
	enum { IDD = IDD_COMP_SETTING };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	BOOL m_bNotFocusIme;
	BOOL m_bBsWmChar;
	BOOL m_bBackAsBs;
};
