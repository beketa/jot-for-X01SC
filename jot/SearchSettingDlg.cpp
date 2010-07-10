//
// jot - Text Editor for Windows Mobile
//   Copyright (C) 2007-2008, Aquamarine Networks.  <http://pandora.sblo.jp/>
//
//    This program is EveryOne's ware; you can redistribute it and/or modify it 
//    under the terms of the NYSL version 0.9982 or later. <http://www.kmonos.net/nysl/>
//

// SearchSetting.cpp : 実装ファイル
//

#include "stdafx.h"
#include "jot.h"
#include "SearchSettingDlg.h"


// CSearchSettingDlg ダイアログ

IMPLEMENT_DYNAMIC(CSearchSettingDlg, CScrollDialog)

CSearchSettingDlg::CSearchSettingDlg(CWnd* pParent /*=NULL*/)
	: CScrollDialog(CSearchSettingDlg::IDD, pParent)
	, m_bRegex(FALSE)
	, m_bIgnoreCase(FALSE)
{

}

CSearchSettingDlg::~CSearchSettingDlg()
{
}

void CSearchSettingDlg::DoDataExchange(CDataExchange* pDX)
{
	CScrollDialog::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK_REGEX, m_bRegex);
	DDX_Check(pDX, IDC_CHECK_IGNORECASE, m_bIgnoreCase);
}


BEGIN_MESSAGE_MAP(CSearchSettingDlg, CScrollDialog)
END_MESSAGE_MAP()


// CSearchSettingDlg メッセージ ハンドラ

BOOL CSearchSettingDlg::OnInitDialog()
{
	__super::OnInitDialog();

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
