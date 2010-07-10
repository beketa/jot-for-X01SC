//
// jot - Text Editor for Windows Mobile
//   Copyright (C) 2007-2008, Aquamarine Networks.  <http://pandora.sblo.jp/>
//
//    This program is EveryOne's ware; you can redistribute it and/or modify it 
//    under the terms of the NYSL version 0.9982 or later. <http://www.kmonos.net/nysl/>
//

// SearchSetting.cpp : �����t�@�C��
//

#include "stdafx.h"
#include "jot.h"
#include "SearchSettingDlg.h"


// CSearchSettingDlg �_�C�A���O

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


// CSearchSettingDlg ���b�Z�[�W �n���h��

BOOL CSearchSettingDlg::OnInitDialog()
{
	__super::OnInitDialog();

	// OK/CANCEL �̃R�}���h�o�[������
	if (!m_wndCommandBar.Create(this) ||
	    !m_wndCommandBar.InsertMenuBar(IDR_OKCANCEL,0) )
	{
		TRACE0("CommandBar �̍쐬�Ɏ��s���܂���\n");
		return -1;      // �쐬�ł��܂���ł����B
	}
	m_wndCommandBar.SetBarStyle(m_wndCommandBar.GetBarStyle() | CBRS_SIZE_FIXED);

	return TRUE;  // return TRUE unless you set the focus to a control
	// ��O : OCX �v���p�e�B �y�[�W�͕K�� FALSE ��Ԃ��܂��B
}
