//
// jot - Text Editor for Windows Mobile
//   Copyright (C) 2007-2008, Aquamarine Networks.  <http://pandora.sblo.jp/>
//
//    This program is EveryOne's ware; you can redistribute it and/or modify it 
//    under the terms of the NYSL version 0.9982 or later. <http://www.kmonos.net/nysl/>
//

// MainFrm.cpp : CMainFrame �N���X�̎���
//

#include "stdafx.h"
#include "jot.h"

#include "MainFrm.h"
#include "tpcshell.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//#ifdef WIN32_PLATFORM_PSPC
#define TOOLBAR_HEIGHT 24
//#endif // WIN32_PLATFORM_PSPC

const DWORD dwAdornmentFlags = 0; // [�I��] �{�^��

// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_WM_CREATE()
	ON_WM_SETFOCUS()
	ON_COMMAND(ID_EXIT, &CMainFrame::OnExit)
	ON_WM_CLOSE()
	ON_MESSAGE(WM_JOT_SHIFTLOCK, OnShiftLock)
	ON_MESSAGE(WM_JOT_RESIZEMENU, OnResizeMenu)
	ON_MESSAGE(WM_JOT_BOTTOMBAR, OnWrapMenu)

	ON_WM_ACTIVATE()
	ON_COMMAND(ID_SHIFTUNLOCK, &CMainFrame::OnShiftunlock)
	ON_WM_SIZE()

//#ifdef WIN32_PLATFORM_PSPC			// es/ades�p�̃e���L�[�ł̃��j���[�A�N�Z�X�@�\
	ON_MESSAGE(WM_ENTERMENULOOP, OnEnterMenuLoop)
	ON_MESSAGE(WM_EXITMENULOOP, OnExitMenuLoop)
//#endif

END_MESSAGE_MAP()

// CMainFrame �R���X�g���N�V����/�f�X�g���N�V����

CMainFrame::CMainFrame()
{
	m_pTempMenu = NULL;
	m_pShiftlockCmdbar = NULL;
}

CMainFrame::~CMainFrame()
{
	DEL(m_pShiftlockCmdbar);
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	CCountTime::printTime(_T("start mainframe oncreate") );

	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	// �t���[���̃N���C�A���g�̈�S�̂��߂�r���[���쐬���܂��B
	if (!m_wndView.Create(NULL, NULL, AFX_WS_DEFAULT_VIEW,
		CRect(0, 0, 0, 0), this, AFX_IDW_PANE_FIRST, NULL))
	{
		TRACE0("�r���[ �E�B���h�E���쐬�ł��܂���ł����B\n");
		return -1;
	}


	if (!m_wndCommandBar.Create(this) ||
	    !m_wndCommandBar.InsertMenuBar(IDR_MAINFRAME) ||
	    !m_wndCommandBar.AddAdornments(dwAdornmentFlags))
	{
		TRACE0("CommandBar �̍쐬�Ɏ��s���܂���\n");
		return -1;      // �쐬�ł��܂���ł����B
	}
	CCountTime::printTime(_T("mainframe create commandbar") );

	m_wndCommandBar.SetBarStyle(m_wndCommandBar.GetBarStyle() | CBRS_SIZE_FIXED);

//#ifdef WIN32_PLATFORM_WFSP
	CWnd* pWnd = CWnd::FromHandlePermanent(m_wndCommandBar.m_hWnd);

	RECT rect, rectDesktop;
	pWnd->GetWindowRect(&rect);
	pWnd->GetDesktopWindow()->GetWindowRect(&rectDesktop);

	int cx = rectDesktop.right - rectDesktop.left;
	int cy = (rectDesktop.bottom - rectDesktop.top) - (rect.bottom - rect.top);

	CCountTime::printTime(_T("mainframe calc windowsize") );

	SetWindowPos(NULL, 0, 0, cx, cy,  SWP_NOMOVE|SWP_NOZORDER);
	SetForegroundWindow();

	CCountTime::printTime(_T("mainframe set window pos") );

	// �^�X�N�o�[��\���̃`�F�b�N
	BOOL bFullSc = theApp.m_inifile.readInt( INI_SEC , INI_KEY_FULLSC , INI_DEF_FULLSC );
	if (bFullSc!=FALSE){
		SHFullScreen(m_hWnd, SHFS_HIDETASKBAR | SHFS_HIDESTARTICON | SHFS_HIDESIPBUTTON); 

		SHACTIVATEINFO sai;
		memset(&sai, 0, sizeof(SHACTIVATEINFO));
		SHHandleWMSettingChange(m_hWnd, -1, 0 , &sai);

		if (AfxGetAygshellUIModel() == Smartphone){
			RECT rectDesktop;
			pWnd->GetDesktopWindow()->GetWindowRect(&rectDesktop);

			int cx = rectDesktop.right - rectDesktop.left;
			int cy = (rectDesktop.bottom - rectDesktop.top);
			SetWindowPos(NULL, 0, 0, cx, cy,  SWP_NOZORDER);
		}
	}
	CCountTime::printTime(_T("mainframe check taskbar") );
	
//#endif // WIN32_PLATFORM_WFSP

	//// ���j���[����������B
	//for( int i=0;i<2;i++ ){
	//	TBBUTTONINFO tbbi = {0};
	//	tbbi.cbSize       = sizeof(tbbi);
	//	tbbi.dwMask       = TBIF_LPARAM | TBIF_BYINDEX;
	//	m_wndCommandBar.SendMessage(TB_GETBUTTONINFO,i, (LPARAM)&tbbi);
	//	HMENU	hMenu     = (HMENU)tbbi.lParam;

	//	CMenu *pMenu = CMenu::FromHandle(hMenu);
	//	if ( pMenu != NULL ){
	//		AddShortCut( pMenu );
	//	}
	//}
	CCountTime::printTime(_T("end mainframe oncreate") );
	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!CFrameWnd::PreCreateWindow(cs))
		return FALSE;

	cs.lpszClass = AfxRegisterWndClass(0);

	// MFC���o�^�����N���X���Ɠ������e�̃N���X��Ǝ����œo�^
	WNDCLASS c_wndclass ;
	GetClassInfo( AfxGetInstanceHandle(), cs.lpszClass, &c_wndclass ) ;
    c_wndclass.lpszMenuName = NULL ;
    c_wndclass.lpszClassName =  CLASS_NAME;
    c_wndclass.hIcon = AfxGetApp()->LoadIcon( IDR_MAINFRAME ) ;
    RegisterClass( &c_wndclass ) ;
    cs.lpszClass = CLASS_NAME ;

	return TRUE;
}



// CMainFrame �f�f

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}
#endif //_DEBUG

// CMainFrame ���b�Z�[�W �n���h��

void CMainFrame::OnSetFocus(CWnd* /*pOldWnd*/)
{
	// �r���[ �E�B���h�E�Ƀt�H�[�J�X��^���܂��B
	m_wndView.SetFocus();
}

BOOL CMainFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// �r���[�ɍŏ��ɃR�}���h����������@���^���܂��B
	if (m_wndView.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	// ����ȊO�̏ꍇ�́A����̏������s���܂��B
	return CFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}



void CMainFrame::OnExit()
{
	PostMessageW(WM_CLOSE);
}

void CMainFrame::OnClose()
{
	// �ۑ��m�F�_�C�A���O���o���ăL�����Z������Ȃ���ΏI��
	if ( m_wndView.ConfirmSave() ){
		CFrameWnd::OnClose();
	}
	theApp.SendShiftKey(FALSE);		// �V�t�g�L�[����

}

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg)
{
//#ifdef WIN32_PLATFORM_WFSP
//	if (AfxGetAygshellUIModel() == Smartphone){
	// SP��ESC/BACK�L�[�őO�̃A�v���ɖ߂��Ă��܂��̂�h��
	if ( pMsg->message == WM_HOTKEY ){
#if	0	// KOTETU����̃A�h�o�C�X�ɏ]���C��
		if ( HIWORD(pMsg->lParam )== VK_ESCAPE ){
			// HOTKEY-VK_ESCAPE���E���� 
			if ( theApp.IsBackAsBs() ){
				if ((LOWORD(pMsg->lParam)&0x1000) == 0 ){	
					// 1��ڂ���BACK�L�[����������
					INPUT inputKey[2];
					
					for( int i=0;i<2;i++ ){
						inputKey[i].type = INPUT_KEYBOARD;
						inputKey[i].ki.wVk = VK_BACK;
						inputKey[i].ki.wScan = MapVirtualKey(VK_BACK, 0);
						inputKey[i].ki.dwFlags = (i==0)?0:KEYEVENTF_KEYUP;
						inputKey[i].ki.dwExtraInfo = 0;
						inputKey[i].ki.time = 0;
					}
					SendInput(2, inputKey, sizeof(INPUT));
				}
			}
#else
		if ( HIWORD(pMsg->lParam )== VK_TBACK ){
			SHSendBackToFocusWindow(WM_HOTKEY, pMsg->wParam, pMsg->lParam); 
#endif
			return TRUE;
		}
	}
//#endif
	// �V�t�g�L�[�A�b�v���R�}���h�o�[��߂�
	if ( pMsg->message == WM_KEYUP ){
		switch ( pMsg->wParam ){
		case VK_SHIFT:
			// ���C���t���[���ɒʒm
			theApp.ShowShiftUnlockCommandBar( false );
			break;
		}
	}

	return CFrameWnd::PreTranslateMessage(pMsg);
}

LRESULT CMainFrame::OnShiftLock(WPARAM wp, LPARAM lp)
{
	BOOL shiftlock = (BOOL)wp;
	
	if ( shiftlock ){
		if ( m_pShiftlockCmdbar == NULL ){
			// �V�t�g���b�N���ꂽ��R�}���h�o�[���㏑��
			m_pShiftlockCmdbar = new CCommandBar();
			m_pShiftlockCmdbar->Create(this);
			m_pShiftlockCmdbar->InsertMenuBar(IDR_SUMENU);
		}
	}else{
		// �������ꂽ��߂�
		if ( m_pShiftlockCmdbar != NULL ){
			m_pShiftlockCmdbar->DestroyWindow();
			DEL(m_pShiftlockCmdbar);
		}
	}
	return 0;
}

void CMainFrame::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	CFrameWnd::OnActivate(nState, pWndOther, bMinimized);

	if ( nState == WA_INACTIVE  ){
		// �C���A�N�e�B�u�ɂȂ�����V�t�g�L�[����
		theApp.SendShiftKey( FALSE );
		theApp.ShowShiftUnlockCommandBar( FALSE );

		// �����ۑ�
		m_wndView.AutoSave(true);
	}else{
		// �A�N�e�B�u�ɂȂ����烁�j���[�č\�z
		ResizeMenu();

		// �����ۑ�����
		m_wndView.AutoSave(false);
	}

}

void CMainFrame::OnShiftunlock()
{
	// �V�t�g���b�N�����������ꂽ��V�t�g�L�[����
	theApp.SendShiftKey( FALSE );
	theApp.ShowShiftUnlockCommandBar( FALSE );
}



// ���j���[���ڂ̌��ɃV���[�g�J�b�g������
void	CMainFrame::AddShortCut( CMenu *pMenu )
{
	UINT	len = pMenu->GetMenuItemCount();

	UINT	no=1;
	for( UINT i=0;i<len;i++ ){
		TCHAR	buf[1024];
		MENUITEMINFO mif;
		memset((char *)&mif, 0, sizeof(mif));
		mif.cbSize = sizeof (MENUITEMINFO); // must fill up this field
		mif.fMask = MIIM_TYPE;             // get the state of the menu item
		mif.fType = MFT_STRING;
		mif.dwTypeData = buf;
		mif.cch = 1023;

		if ( pMenu->GetMenuItemInfo( i , &mif ,TRUE ) ){
			if ( ( mif.fType & MFT_SEPARATOR ) == 0 ){
				CString old = mif.dwTypeData;

				// _�Ŏn�܂郁�j���[���ڂ͉��������Ȃ̂ō폜����B
				if ( *old == _T('_') ){
					pMenu->DeleteMenu( i , MF_BYPOSITION );
					len --;
					i--;
					continue;
				}

//#ifdef WIN32_PLATFORM_PSPC
				if (AfxGetAygshellUIModel() == PocketPC){
					// ���j���[�̕t���ւ�
					if ( m_wndView.m_menutenkey == TRUE && DRA::GetDisplayMode() == DRA::Portrait ){
						// PsPC�ŏc���[�h�̎��A���j���[���ڂ̌���(&1)������
						
						// ���̃��j���[�̃V���[�g�J�b�g����������
						_tcscpy( buf , old );
						TCHAR *p;
						for( p=buf ; *p!=_T('\0') ;p++ ){
							if ( *p == _T('\t') ){
								*p = _T('\0');
								break;
							}
						}
						// �����V���[�g�J�b�g������
						_stprintf( p , _T("\t(&%d)") , no++ );
						pMenu->SetMenuItemInfo( i , &mif , TRUE );
					}
				}else{
//#endif
//#ifdef WIN32_PLATFORM_WFSP
					// ���̃��j���[�̃V���[�g�J�b�g����������
					_tcscpy( buf , old );
					TCHAR *p;
					for( p=buf ; *p!=_T('\0') ;p++ ){
						if ( *p == _T('\t') ){
							*p = _T('\0');
							break;
						}
					}
					pMenu->SetMenuItemInfo( i , &mif , TRUE );
				}
//#endif
			}
		}

		CMenu *p = pMenu->GetSubMenu(i);
		if ( p!=NULL ){
			AddShortCut( p );
		}
	}

}

void	CMainFrame::ResizeMenu()
{
	// �A�N�e�B�u�łȂ���Ή������Ȃ�
	CWnd	*p=GetActiveWindow();
	if ( p!=this ){
		return;
	}

	// �ꎞ���j���[�̍�蒼��
	if ( m_pTempMenu!=NULL ){
		m_pTempMenu->Destroy();
		if (!m_pTempMenu->Create(this) ||
			!m_pTempMenu->InsertMenuBar(m_TempMenuID) )
		{
			TRACE0("CommandBar �̍쐬�Ɏ��s���܂���\n");
			return ;      // �쐬�ł��܂���ł����B
		}

		// ���j���[����������B
		for( int i=0;i<2;i++ ){
			TBBUTTONINFO tbbi = {0};
			tbbi.cbSize       = sizeof(tbbi);
			tbbi.dwMask       = TBIF_LPARAM | TBIF_BYINDEX;
			m_pTempMenu->SendMessage(TB_GETBUTTONINFO,i, (LPARAM)&tbbi);
			HMENU	hMenu     = (HMENU)tbbi.lParam;

			CMenu *pMenu = CMenu::FromHandle(hMenu);
			if ( pMenu != NULL ){
				AddShortCut( pMenu );
			}
		}
		return;
	}

	// ���j���[�o�[��蒼��
	m_wndCommandBar.Destroy();
	if (!m_wndCommandBar.Create(this) ||
	    !m_wndCommandBar.InsertMenuBar(IDR_MAINFRAME) )
	{
		TRACE0("CommandBar �̍쐬�Ɏ��s���܂���\n");
		return ;      // �쐬�ł��܂���ł����B
	}


	// ���j���[����������B
	for( int i=0;i<2;i++ ){
		TBBUTTONINFO tbbi = {0};
		tbbi.cbSize       = sizeof(tbbi);
		tbbi.dwMask       = TBIF_LPARAM | TBIF_BYINDEX;
		m_wndCommandBar.SendMessage(TB_GETBUTTONINFO,i, (LPARAM)&tbbi);
		HMENU	hMenu     = (HMENU)tbbi.lParam;

		CMenu *pMenu = CMenu::FromHandle(hMenu);
		m_wndView.SetMruMenu(i , hMenu);
		if ( pMenu != NULL ){
			AddShortCut( pMenu );
		}
	}
	if ( m_pShiftlockCmdbar!= NULL ){
		m_pShiftlockCmdbar->BringWindowToTop();
	}

}



void CMainFrame::OnSize(UINT nType, int cx, int cy)
{
	CFrameWnd::OnSize(nType, cx, cy);

	ResizeMenu();
}

LRESULT CMainFrame::OnResizeMenu(WPARAM wp, LPARAM lp)
{
	ResizeMenu();

	return 0;
}

LRESULT CMainFrame::OnWrapMenu(WPARAM wp, LPARAM lp)
{
	// wp = �ꎞ���j���[�̃��\�[�XID
	// lp = 0:���j���[�폜  1:���j���[����

	// ���j���[�폜
	if ( m_pTempMenu != NULL ){
		m_pTempMenu->Destroy();
		DEL(m_pTempMenu );
	}

	// ���j���[����
	if ( lp != 0 ){
		m_TempMenuID = wp;
		m_pTempMenu = new CCommandBar();
		ResizeMenu();
	}
	return 0;
}

//#ifdef WIN32_PLATFORM_PSPC			// es/ades�p�̃e���L�[�ł̃��j���[�A�N�Z�X�@�\

LRESULT CMainFrame::OnEnterMenuLoop(WPARAM wp, LPARAM lp)
{
	theApp.MenuLoop( true );
	return 0;
}


LRESULT CMainFrame::OnExitMenuLoop(WPARAM wp, LPARAM lp)
{
	theApp.MenuLoop( false );
	return 0;
}

//#endif
