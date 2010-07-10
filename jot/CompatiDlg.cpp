//
// jot - Text Editor for Windows Mobile
//   Copyright (C) 2007-2008, Aquamarine Networks.  <http://pandora.sblo.jp/>
//
//    This program is EveryOne's ware; you can redistribute it and/or modify it 
//    under the terms of the NYSL version 0.9982 or later. <http://www.kmonos.net/nysl/>
//

// CompatiDlg.cpp : �����t�@�C��
//

#include "stdafx.h"
#include "jot.h"
#include "CompatiDlg.h"


// CCompatiDlg �_�C�A���O

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


// CCompatiDlg ���b�Z�[�W �n���h��

BOOL CCompatiDlg::OnInitDialog()
{
	CScrollDialog::OnInitDialog();

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
