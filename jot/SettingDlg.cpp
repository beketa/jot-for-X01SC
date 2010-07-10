//
// jot - Text Editor for Windows Mobile
//   Copyright (C) 2007-2008, Aquamarine Networks.  <http://pandora.sblo.jp/>
//
//    This program is EveryOne's ware; you can redistribute it and/or modify it 
//    under the terms of the NYSL version 0.9982 or later. <http://www.kmonos.net/nysl/>
//

// SettingDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "jot.h"
#include "SettingDlg.h"


// CSettingDlg ダイアログ

IMPLEMENT_DYNAMIC(CSettingDlg, CScrollDialog)

CSettingDlg::CSettingDlg(CWnd* pParent /*=NULL*/)
	: CScrollDialog(CSettingDlg::IDD, pParent)
	, m_bMenuTenkey(FALSE)
//	, m_bAllFile(FALSE)
	, m_xscrawl(0)
	, m_bHScroll(FALSE)
	, m_bDrawLinebreak(FALSE)
	, m_bDrawTab(FALSE)
	, m_bDrawWideSpace(FALSE)
	, m_bDrawSpace(FALSE)
	, m_mrunum(0)
	, m_bFullSc(FALSE)
	, m_bBoldFont(FALSE)
	, m_bDisAutoDetect(FALSE)
	, m_bUnderline(FALSE)
	, m_linespacing(0)
	, m_NewFileCharCode(0)
	, m_NewFileLineBreak(0)
	, m_wordwrap_pos_p(0)
	, m_wordwrap_pos_l(0)
	, m_bDrawLineNumber(FALSE)
{

}

CSettingDlg::~CSettingDlg()
{
}

void CSettingDlg::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_FONT, m_combofontname);
	DDX_Control(pDX, IDC_COMBO_FONTSIZE, m_combofontsize);
	DDX_Control(pDX, IDC_EDIT_TABSIZE, m_edittabsize);
	DDX_Control(pDX, IDC_CHECK_MENUTENKEY, m_chkMenuTenkey);
	DDX_Check(pDX, IDC_CHECK_MENUTENKEY, m_bMenuTenkey);
	DDX_Control(pDX, IDC_STATIC_XSCRAWL, m_staXscrawl);
	DDX_Control(pDX, IDC_COMBO_XSCRAWL, m_cmbXscrawl);
	DDX_CBIndex(pDX, IDC_COMBO_XSCRAWL, m_xscrawl);
	DDX_Check(pDX, IDC_CHECK_HSCROLL, m_bHScroll);
	DDX_Check(pDX, IDC_CHECK_DRAWLINEBREAK, m_bDrawLinebreak);
	DDX_Check(pDX, IDC_CHECK_DRAWTAB, m_bDrawTab);
	DDX_Check(pDX, IDC_CHECK_DRAWWIDESPACE, m_bDrawWideSpace);
	DDX_Check(pDX, IDC_CHECK_DRAWSPACE, m_bDrawSpace);
	DDX_Text(pDX, IDC_EDIT_MRUNUM, m_mrunum);
	DDV_MinMaxInt(pDX, m_mrunum, 1, 9);
	DDX_Check(pDX, IDC_CHECK_FULLSC, m_bFullSc);
	DDX_Control(pDX, IDC_CHECK_BOLD, m_CheckBold);
	DDX_Check(pDX, IDC_CHECK_BOLD, m_bBoldFont);
	DDX_Check(pDX, IDC_CHECK_DISAUTODETECT, m_bDisAutoDetect);
	DDX_Check(pDX, IDC_CHECK_UNDERLINE, m_bUnderline);
	DDX_Text(pDX, IDC_EDIT_SPACING, m_linespacing);
	DDV_MinMaxInt(pDX, m_linespacing, 0, 99);
	DDX_CBIndex(pDX, IDC_COMBO_NEFILECHARCODE, m_NewFileCharCode);
	DDX_CBIndex(pDX, IDC_COMBO_NEWFILELINEBREAK, m_NewFileLineBreak);
	DDX_Text(pDX, IDC_EDIT_WORDWRAP_POS_P, m_wordwrap_pos_p);
	DDV_MinMaxInt(pDX, m_wordwrap_pos_p, 0, 255);
	DDX_Text(pDX, IDC_EDIT_WORDWRAP_POS_L, m_wordwrap_pos_l);
	DDV_MinMaxInt(pDX, m_wordwrap_pos_l, 0, 255);
	DDX_Check(pDX, IDC_CHECK_DRAWLINENUMBER, m_bDrawLineNumber);
}


BEGIN_MESSAGE_MAP(CSettingDlg, CScrollDialog)
END_MESSAGE_MAP()


// CSettingDlg メッセージ ハンドラ

BOOL CSettingDlg::OnInitDialog()
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

	// フォントコンボボックス初期化
	CString	str;
	str.LoadString( IDS_FONT_NONE );
	m_combofontname.Init(str , m_fontname);

	// フォントサイズコンボボックス初期化
	int len = m_combofontsize.GetCount();
	for(int i=0;i<len;i++ ){
		CString item;
		m_combofontsize.GetLBText(i,item);
		int	itemsize = _ttoi( item );
		if ( itemsize == m_fontsize ){
			m_combofontsize.SetCurSel( i );
		}
	}

	// タブサイズ初期化
	CString tabsizetmp;
	tabsizetmp.Format( _T("%d" ),m_tabsize );
	m_edittabsize.SetWindowText( tabsizetmp );

	// フォント未設定時はフォントサイズは無効
	m_combofontsize.EnableWindow(( m_combofontname.GetCurSel() != 0 ));
	m_CheckBold.EnableWindow(( m_combofontname.GetCurSel() != 0 ));

	// SP用項目とPsPC用項目の表示を分ける
	//#ifdef WIN32_PLATFORM_WFSP
	////CRect	rm,ra;
	////m_chkMenuTenkey.GetWindowRect(&rm);
	////m_chkAllFile.GetWindowRect(&ra);
	////m_chkAllFile.MoveWindow(rm.left,rm.top-rm.Height(),ra.Width(),ra.Height());
	////m_chkAllFile.ShowWindow(TRUE);		// ファイルフィルタを*.*にするかどうか
	//#endif
	//#ifdef WIN32_PLATFORM_PSPC	
	if (AfxGetAygshellUIModel() == PocketPC){
		m_chkMenuTenkey.ShowWindow(TRUE);	// 縦画面時、テンキーでメニューアクセスするか
		m_staXscrawl.ShowWindow(TRUE);
		m_cmbXscrawl.ShowWindow(TRUE);
	}
//	#endif

	return TRUE;  // return TRUE unless you set the focus to a control
	// 例外 : OCX プロパティ ページは必ず FALSE を返します。
}

void CSettingDlg::OnOK()
{
	// フォント名取得
	int fn = m_combofontname.GetCurSel();
	if ( fn == 0 ){
		m_fontname = _T("");
	}else{
		m_combofontname.GetLBText( fn , m_fontname );
	}
	// フォントサイズ取得
	CString tmp;
	m_combofontsize.GetWindowText(tmp);
	m_fontsize = _ttoi( tmp );

	// タブサイズ取得
	m_edittabsize.GetWindowText( tmp );
	m_tabsize = _ttoi( tmp );

	__super::OnOK();
}



// コンボボックスが変更されたら状態を更新させる
BOOL CSettingDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
	UINT ID = LOWORD(wParam);
	UINT noti = HIWORD(wParam);
	NMHDR *p = (NMHDR*)lParam;
	if ( ID==IDC_COMBO_FONT ){
		if ( noti == CBN_SELCHANGE ){
			m_combofontsize.EnableWindow(( m_combofontname.GetCurSel() != 0 ));
			m_CheckBold.EnableWindow(( m_combofontname.GetCurSel() != 0 ));
		}
	}
	return __super::OnCommand(wParam, lParam);
}
