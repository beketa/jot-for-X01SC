//
// jot - Text Editor for Windows Mobile
//   Copyright (C) 2007-2008, Aquamarine Networks.  <http://pandora.sblo.jp/>
//
//    This program is EveryOne's ware; you can redistribute it and/or modify it 
//    under the terms of the NYSL version 0.9982 or later. <http://www.kmonos.net/nysl/>
//

// CompatiDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "jot.h"
#include "CompatiDlg.h"


// CCompatiDlg ダイアログ

IMPLEMENT_DYNAMIC(CCompatiDlg, CScrollDialog)

CCompatiDlg::CCompatiDlg(CWnd* pParent /*=NULL*/)
	: CScrollDialog(CCompatiDlg::IDD, pParent)
	, m_bNotFocusIme(FALSE)
	, m_bBsWmChar(FALSE)
	, m_bBackAsBs(FALSE)
{

}

CCompatiDlg::~CCompatiDlg()
{
}

void CCompatiDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK_NOTFOCUSIME, m_bNotFocusIme);
	DDX_Check(pDX, IDC_CHECK_BSWMCHAR, m_bBsWmChar);
	DDX_Check(pDX, IDC_CHECK_BACKASBS, m_bBackAsBs);
}


BEGIN_MESSAGE_MAP(CCompatiDlg, CScrollDialog)
END_MESSAGE_MAP()


// CCompatiDlg メッセージ ハンドラ

BOOL CCompatiDlg::OnInitDialog()
{
	CScrollDialog::OnInitDialog();

	// OK/CANCEL のコマンドバーをつける
	if (!m_wndCommandBar.Create(this) ||
	    !m_wndCommandBar.InsertMenuBar(IDR_OKCANCEL,0) )
	{
		TRACE0("CommandBar の作成に失敗しました\n");
		return -1;      // 作成できませんでした。
	}
	m_wndCommandBar.SetBarStyle(m_wndCommandBar.GetBarStyle() | CBRS_SIZE_FIXED);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 例外 : OCX プロパティ ページは必ず FALSE を返します。
}
