//
// jot - Text Editor for Windows Mobile
//   Copyright (C) 2007-2008, Aquamarine Networks.  <http://pandora.sblo.jp/>
//
//    This program is EveryOne's ware; you can redistribute it and/or modify it 
//    under the terms of the NYSL version 0.9982 or later. <http://www.kmonos.net/nysl/>
//

// jot.cpp : �A�v���P�[�V�����̃N���X������`���܂��B
//

#include "stdafx.h"
#include "jot.h"
#include "MainFrm.h"

#include "ini.h"
#include "afxwin.h"

#include "nEdit.h"
#include "cmdarg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CjotApp

BEGIN_MESSAGE_MAP(CjotApp, CWinApp)
//#ifndef WIN32_PLATFORM_WFSP
	ON_COMMAND(ID_APP_ABOUT, &CjotApp::OnAppAbout)
//#endif // !WIN32_PLATFORM_WFSP
ON_COMMAND(ID_TOOL_ASSOC, &CjotApp::OnToolAssoc)
END_MESSAGE_MAP()



// CjotApp �R���X�g���N�V����
CjotApp::CjotApp()
	: CWinApp()
{
	// TODO: ���̈ʒu�ɍ\�z�p�R�[�h��ǉ����Ă��������B
	// ������ InitInstance ���̏d�v�ȏ��������������ׂċL�q���Ă��������B
}


// �B��� CjotApp �I�u�W�F�N�g�ł��B
CjotApp theApp;

// CjotApp ������

BOOL CjotApp::InitInstance()
{
	CCountTime::printTime(_T("jot start") );
	// �R�}���h���C�������̕ۑ�
	TCHAR *p=m_lpCmdLine;
	Argv::setCmdLine( p );

	CCountTime::printTime(_T("setCmdLine") );
//#if defined(WIN32_PLATFORM_PSPC) || defined(WIN32_PLATFORM_WFSP)
    // CAPEDIT ����� SIPPREF �̂悤�� Pocket PC �̓��ʂȃR���g���[��������������ɂ́A�A�v���P�[�V������
    // ���������� SHInitExtraControls ����x�Ăяo���K�v������܂��B
//    SHInitExtraControls();
//#endif // WIN32_PLATFORM_PSPC || WIN32_PLATFORM_WFSP

	// �W��������
	// �����̋@�\���g�킸�ɁA�ŏI�I�Ȏ��s�\�t�@�C���̃T�C�Y���k���������ꍇ�́A
	// �ȉ�����A�s�v�ȏ��������[�`����
	// �폜���Ă��������B

	// EDIT�o�^
	if (nedit_regist(theApp.m_hInstance) == FALSE) {
		return 0;
	}

	// ���s�t�@�C��������f�B���N�g���� ini ������B
	TCHAR temp[MAX_PATH*2]={0};
	GetModuleFileName(NULL, temp, MAX_PATH*2-1);
	CString s = temp;

	size_t pos = s.ReverseFind(_T('\\'));
	CString exepath = s.Left(pos+1) ;

	CString	CodePage;
	CodePage.LoadString( IDS_CODEPAGE );

	CCountTime::printTime(_T("start check template") );
	// �ݒ�t�@�C���e���v���[�g�R�s�[
	//if ( GetFileAttributes( exepath+MISCFILE )==0xFFFFFFFF  ) {
		// �t�@�C�����Ȃ���΃e���v���[�g����R�s�[
		CopyFile(  exepath+MISCFILETEMP ,exepath+MISCFILE , TRUE );
	//}else{
	//	//// �e���v���[�g���폜
	//	//DeleteFile( exepath+MISCFILETEMP );
	//}

	CString	TemplateTemp;
	TemplateTemp.Format( TEMPLATEFILETEMP , CodePage );
	//if ( GetFileAttributes( exepath+TEMPLATEFILE )==0xFFFFFFFF  ) {
		// �t�@�C�����Ȃ���΃e���v���[�g����R�s�[
		CopyFile(  exepath+TemplateTemp ,exepath+TEMPLATEFILE , TRUE );
	//}else{
	//	//// �e���v���[�g���폜
	//	//DeleteFile( exepath+TemplateTemp );
	//}
	CCountTime::printTime(_T("end check template") );

	// Ini�t�@�C���I�[�v��
	m_inifile.open(INIFILE);
	CCountTime::printTime(_T("open inifile") );
	m_miscfile.open(MISCFILE);
	CCountTime::printTime(_T("open miscfile") );

	// ���C�� �E�B���h�E���쐬����Ƃ��A���̃R�[�h�͐V�����t���[�� �E�B���h�E �I�u�W�F�N�g���쐬���A
	// ������A�v���P�[�V�����̃��C�� �E�B���h�E�ɃZ�b�g���܂�
	CMainFrame* pFrame = new CMainFrame;
	if (!pFrame)
		return FALSE;
	m_pMainWnd = pFrame;
	_dellog(m_pMainWnd );		// MFC���폜����̂Ń������ǐՂ���͏����Ă���
	CCountTime::printTime(_T("construct mainwindow") );

	// �t���[�������\�[�X���烍�[�h���č쐬���܂�
	pFrame->LoadFrame(IDR_MAINFRAME,
		WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, NULL, NULL);

	CCountTime::printTime(_T("load mainframe resource") );

	// ���C�� �E�B���h�E�����������ꂽ�̂ŁA�\���ƍX�V���s���܂�
	pFrame->ShowWindow(SW_SHOW);
	pFrame->UpdateWindow();
	return TRUE;
}

void	CjotApp::SendShiftKey( bool down )
{

	INPUT inputKey;

	inputKey.type = INPUT_KEYBOARD;
	inputKey.ki.wVk = VK_SHIFT;
	inputKey.ki.wScan = MapVirtualKey(VK_SHIFT, 0);
	inputKey.ki.dwFlags = down?0:KEYEVENTF_KEYUP;
	inputKey.ki.dwExtraInfo = 0;
	inputKey.ki.time = 0;

	SendInput(1, &inputKey, sizeof(INPUT));
}

void	CjotApp::ShowShiftUnlockCommandBar( BOOL shiftlock )
{
	if ( m_pMainWnd != NULL ){
		m_pMainWnd->PostMessage( WM_JOT_SHIFTLOCK , shiftlock , 0 );
	}
}

void	CjotApp::PostMainWnd( UINT msg , WPARAM wp , LPARAM lp )
{
	if ( m_pMainWnd != NULL ){
		m_pMainWnd->PostMessage( msg , wp , lp );
	}
}


// CjotApp ���b�Z�[�W �n���h��


//#ifndef WIN32_PLATFORM_WFSP
// �A�v���P�[�V�����̃o�[�W�������Ɏg���� CAboutDlg �_�C�A���O
class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// �_�C�A���O �f�[�^
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �T�|�[�g

// ����
protected:
#ifdef _DEVICE_RESOLUTION_AWARE
	afx_msg void OnSize(UINT /*nType*/, int /*cx*/, int /*cy*/);
#endif
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()

	CCommandBar m_wndCommandBar;

public:
	CStatic m_version;
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_VERSION, m_version);
}

BOOL CAboutDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	if (AfxGetAygshellUIModel() == PocketPC){
		// �^�X�N�o�[�������E�C���h�E�ʒu�𒲐�����
		BOOL bFullSc = theApp.m_inifile.readInt( INI_SEC , INI_KEY_FULLSC , INI_DEF_FULLSC );
		if (bFullSc!=FALSE){
			SHFullScreen(m_hWnd, SHFS_HIDETASKBAR | SHFS_HIDESTARTICON | SHFS_HIDESIPBUTTON); 

			SHACTIVATEINFO sai;
			memset(&sai, 0, sizeof(SHACTIVATEINFO));
			SHHandleWMSettingChange(m_hWnd, -1, 0 , &sai);

			RECT rectDesktop;
			GetDesktopWindow()->GetWindowRect(&rectDesktop);

			int cx = rectDesktop.right - rectDesktop.left;
			int cy = (rectDesktop.bottom - rectDesktop.top);
			SetWindowPos(NULL, 0, 0, cx, cy,  SWP_NOZORDER);
		}
	}

	m_wndCommandBar.Create(this);
	m_wndCommandBar.InsertMenuBar(IDR_OK);

	CString ver;
	CString edition;
//#ifdef WIN32_PLATFORM_WFSP
//	edition.LoadString( IDS_EDITION_SP );
//#endif
//#ifdef WIN32_PLATFORM_PSPC
//	edition.LoadString( IDS_EDITION_PPC );
//#endif

//	ver.Format(_T("%s version %s\n for %s"),APPTITLE,VERSION_STRINGS,edition );
	ver.Format(_T("%s version %s"),APPTITLE,VERSION_STRINGS );

	m_version.SetWindowTextW( ver );

	return TRUE;	// �t�H�[�J�X���R���g���[���ɐݒ肵���ꍇ�������ATRUE ��Ԃ��܂��B
			// ��O: OCX �v���p�e�B �y�[�W�� FALSE ��Ԃ��Ȃ���΂Ȃ�܂���
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
#ifdef _DEVICE_RESOLUTION_AWARE
	ON_WM_SIZE()
#endif
END_MESSAGE_MAP()

#ifdef _DEVICE_RESOLUTION_AWARE
void CAboutDlg::OnSize(UINT /*nType*/, int /*cx*/, int /*cy*/)
{
	//if (AfxIsDRAEnabled())
 //   	{
	//	DRA::RelayoutDialog(
	//		AfxGetResourceHandle(), 
	//		this->m_hWnd, 
	//		DRA::GetDisplayMode() != DRA::Portrait ? MAKEINTRESOURCE(IDD_ABOUTBOX_WIDE) : MAKEINTRESOURCE(IDD_ABOUTBOX));
	//}
}
#endif

// �_�C�A���O�����s���邽�߂̃A�v���P�[�V���� �R�}���h
void CjotApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}
//#endif // !WIN32_PLATFORM_WFSP


void	CjotApp::ClickKey( UINT vchar )
{
	INPUT inputKey[2];
	for( int i=0;i<2;i++){
		inputKey[i].type = INPUT_KEYBOARD;
		inputKey[i].ki.wVk = vchar;
		inputKey[i].ki.wScan = MapVirtualKey(vchar, 0);
		inputKey[i].ki.dwFlags = 0;
		inputKey[i].ki.dwExtraInfo = i==0?0:KEYEVENTF_KEYUP;
		inputKey[i].ki.time = 0;
	}
	SendInput(2, inputKey, sizeof(INPUT));
}


void	CjotApp::SendKeys( INPUT &inputKey , UINT vchar , bool bDown )
{
	inputKey.type = INPUT_KEYBOARD;
	inputKey.ki.wVk = vchar;
	inputKey.ki.wScan = MapVirtualKey(vchar, 0);
	inputKey.ki.dwFlags = 0;
	inputKey.ki.dwExtraInfo = bDown?0:KEYEVENTF_KEYUP;
	inputKey.ki.time = 0;
}


void	CjotApp::GetMenu( CMenu *pMenu , UINT nID )
{
	pMenu->LoadMenu(nID);

	((CMainFrame*)m_pMainWnd)->AddShortCut( pMenu );

}

//#ifdef WIN32_PLATFORM_PSPC			// es/ades�p�̃e���L�[�ł̃��j���[�A�N�Z�X�@�\

void CjotApp::ChangeInputMode(int Mode)
{
	HKEY hKeyPs;
	DWORD dwValue;

	if (RegOpenKeyEx(HKEY_CURRENT_USER, REG_KEY_PHONESTAT, 0, KEY_ALL_ACCESS, &hKeyPs) == ERROR_SUCCESS)
	{
		dwValue = Mode;
		RegSetValueEx(hKeyPs, _T("Status22"), 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(DWORD));
		RegCloseKey(hKeyPs);
	}
}

void CjotApp::SaveInputMode()
{
	HKEY hKeyPs;
	DWORD dwSize, dwValue;

	if (RegOpenKeyEx(HKEY_CURRENT_USER, REG_KEY_PHONESTAT, 0, KEY_ALL_ACCESS, &hKeyPs) == ERROR_SUCCESS)
	{
		dwSize = sizeof(DWORD);
		dwValue = 0;
		if (RegQueryValueEx(hKeyPs, _T("Status22"), NULL, NULL, (LPBYTE)&dwValue, &dwSize) == ERROR_SUCCESS)
			m_lastInputMode = dwValue;
		RegCloseKey(hKeyPs);
	}
}

void CjotApp::RestoreInputMode()
{
	HKEY hKeyPs;
	DWORD dwValue;

	if (RegOpenKeyEx(HKEY_CURRENT_USER, REG_KEY_PHONESTAT, 0, KEY_ALL_ACCESS, &hKeyPs) == ERROR_SUCCESS)
	{
		dwValue = m_lastInputMode;
		RegSetValueEx(hKeyPs, _T("Status22"), 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(DWORD));
		RegCloseKey(hKeyPs);
	}
}

void CjotApp::MenuLoop(bool enter)
{
	if ( ((CMainFrame*)m_pMainWnd)->IsMenuTenkey() == TRUE ){
		if ( enter ){
			SaveInputMode();
			ChangeInputMode(INPUTMODE_HAN_NUMERIC);
		}else{
			RestoreInputMode();
		}
	}
}

//#endif

BOOL GetOpenFileNameEx(OPENFILENAME* pofn)
{
	HINSTANCE hInst = LoadLibrary(_T("gsgetfile.dll"));
	if (hInst) {
		BOOL (*gsGetOpenFileName)(OPENFILENAME* pofn);
		(FARPROC&)gsGetOpenFileName = GetProcAddress(hInst, _T("gsGetOpenFileName"));
		if (gsGetOpenFileName) {
			BOOL ret = gsGetOpenFileName(pofn);
			FreeLibrary(hInst);
			return ret;
		}
		FreeLibrary(hInst);
	}
	return GetOpenFileName(pofn);
}

BOOL GetSaveFileNameEx(OPENFILENAME* pofn)
{
	HINSTANCE hInst = LoadLibrary(_T("gsgetfile.dll"));
	if (hInst) {
		BOOL (*gsGetSaveFileName)(OPENFILENAME* pofn);
		(FARPROC&)gsGetSaveFileName = GetProcAddress(hInst, _T("gsGetSaveFileName"));
		if (gsGetSaveFileName) {
			BOOL ret = gsGetSaveFileName(pofn);
			FreeLibrary(hInst);
			return ret;
		}
		FreeLibrary(hInst);
	}
	return GetSaveFileName(pofn);
}

bool	CjotApp::IsAssoc( TCHAR* key , TCHAR* val )
{
	HKEY hKeyPs;
	DWORD dwSize;
	bool ret = false;
	if (RegOpenKeyEx(HKEY_CLASSES_ROOT, key, 0, KEY_ALL_ACCESS, &hKeyPs) == ERROR_SUCCESS)
	{
		TCHAR	buff[256];
		dwSize = sizeof(buff);
		if (RegQueryValueEx(hKeyPs, NULL, NULL, NULL, (LPBYTE)buff, &dwSize) == ERROR_SUCCESS){
			if ( _tcscmp( buff , val )== 0 ){
				ret = true;
			}
		}
		RegCloseKey(hKeyPs);
	}
	return ret;
}
void	CjotApp::SetAssoc( TCHAR* key , TCHAR* val )
{
	HKEY hKeyPs;
	DWORD dwSize;
	DWORD result;
	if (RegCreateKeyEx(HKEY_CLASSES_ROOT, key, 0, 0, REG_OPTION_NON_VOLATILE,
						KEY_ALL_ACCESS,NULL,&hKeyPs,&result ) == ERROR_SUCCESS)
	{
		dwSize = _tcslen(val)*sizeof(TCHAR);
		RegSetValueEx(hKeyPs, NULL , NULL, REG_SZ, (LPBYTE)val, dwSize );
		RegCloseKey(hKeyPs);
	}
}

void	CjotApp::DelAssoc( TCHAR* key  )
{
	RegDeleteKey(HKEY_CLASSES_ROOT, key );
}

void 	CjotApp::SetExtAssoc( int mode )
{
	// mode:-1	toggle
	// mode:0	del
	// mode:1	set

	// �֘A�Â�����Ă��邩�m�F
	bool	isAssoc = true;

	if ( !IsAssoc( REG_KEY_INI ,REG_VAL_INI ) ){
		isAssoc = false;
	}
	if ( !IsAssoc( REG_KEY_TXT ,REG_VAL_TXT ) ){
		isAssoc = false;
	}

	CString caption;
	caption.LoadString(IDS_CONFIRM);

	CString msg;

	if ( !isAssoc && mode!=0 ){
		// �֘A�Â����Ȃ��ꍇ
		msg.LoadString(IDS_CONFIRM_SETASSOC);
		// �֘A�t�����܂����H
		int res = MessageBox( NULL , msg,caption , MB_YESNO );
		if ( res == IDYES ){
			// �֘A�t����ݒ�
			SetAssoc( REG_KEY_INI , REG_VAL_INI );
			SetAssoc( REG_KEY_TXT , REG_VAL_TXT );
		}
	}else if ( isAssoc && mode!=1 ){
		// �֘A�t������Ă���ꍇ
		msg.LoadString(IDS_CONFIRM_DELASSOC);
		// �֘A�t�����폜���܂����H
		int res = MessageBox( NULL , msg,caption , MB_YESNO );
		if ( res == IDYES ){
			// �֘A�t�����폜
			DelAssoc( REG_KEY_INI );
			SetAssoc( REG_KEY_TXT , REG_DEF_TXT );
		}
	}
}


void CjotApp::OnToolAssoc()
{
	SetExtAssoc(  );		// �g���q�֘A�t���i�g�O���j
}
