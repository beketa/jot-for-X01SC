//
// jot - Text Editor for Windows Mobile
//   Copyright (C) 2007-2008, Aquamarine Networks.  <http://pandora.sblo.jp/>
//
//    This program is EveryOne's ware; you can redistribute it and/or modify it 
//    under the terms of the NYSL version 0.9982 or later. <http://www.kmonos.net/nysl/>
//

// ScrollDialog.cpp : �����t�@�C��
//

#include "stdafx.h"
#include "jot.h"
#include "ScrollDialog.h"
#include "tpcshell.h"

bool	CScrollDialog::m_FullSc=false;

// �q�E�C���h�E��WM_SETFOCUS��������A�e�ɒʒm��������
LRESULT CALLBACK SubProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	HWND pWnd;
    switch (msg) {
        case WM_SETFOCUS:
			pWnd = GetParent( hWnd );
			PostMessage( pWnd , WM_CHILD_SETFOCUS , wp , (LPARAM)hWnd );
			break;
        default:
            break;
    }
    //�����ŏ������Ȃ����̂͌��̃v���V�[�W���ɂ���Ă��炤
	WNDPROC	wndproc = (WNDPROC)::GetWindowLong( hWnd , GWL_USERDATA );
    return (CallWindowProc(wndproc, hWnd, msg, wp, lp));
}


// CScrollDialog �_�C�A���O

IMPLEMENT_DYNAMIC(CScrollDialog, CDialog)

CScrollDialog::CScrollDialog(UINT DlgTemplate , CWnd* pParent /*=NULL*/)
	: CDialog(DlgTemplate, pParent)
{

}

CScrollDialog::~CScrollDialog()
{
}

void CScrollDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CScrollDialog, CDialog)
	ON_WM_LBUTTONDOWN()
	ON_WM_SIZE()
	ON_WM_VSCROLL()
END_MESSAGE_MAP()


// CScrollDialog ���b�Z�[�W �n���h��

BOOL CScrollDialog::OnInitDialog()
{
	__super::OnInitDialog();

	if (AfxGetAygshellUIModel() == PocketPC){
		// �^�X�N�o�[�������E�C���h�E�ʒu�𒲐�����
		if ( m_FullSc ){
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
		//// PocketPC�̎��A�_�C�A���O�����X�N���[��������
		m_nDialogHeight = 0;
		// �_�C�A���O�̎q�R���g���[�����T�u�N���X��
		CWnd	*dlgitm=GetWindow(GW_CHILD);
		while( dlgitm != NULL ){
			WNDPROC	wndproc = (WNDPROC)GetWindowLong( dlgitm->m_hWnd , GWL_WNDPROC );
			SetWindowLong( dlgitm->m_hWnd , GWL_WNDPROC ,(LONG)SubProc );
			SetWindowLong( dlgitm->m_hWnd , GWL_USERDATA ,(LONG)wndproc );
			
			CRect	rect;
			dlgitm->GetWindowRect( &rect );
			if ( rect.bottom > m_nDialogHeight ){
				m_nDialogHeight = rect.bottom;
			}

			dlgitm = dlgitm->GetWindow(GW_HWNDNEXT);
		}
		m_nDialogHeight += 16;
		m_nScrollPos = 0;
	}
			
	return TRUE;  // return TRUE unless you set the focus to a control
	// ��O : OCX �v���p�e�B �y�[�W�͕K�� FALSE ��Ԃ��܂��B
}

BOOL CScrollDialog::OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	if ( message == WM_CHILD_SETFOCUS ){		// �q�E�C���h�E�ւ�WM_SETFOCUS

		HWND hwnd = (HWND)lParam;
		CWnd *pItm= FromHandle(hwnd);

		// �_�C�A���O�̃N���C�A���g�̈�����߂�
		CRect	dlgrect ;
		GetClientRect( &dlgrect );

		// �q�E�C���h�E�̕\���ʒu�����߂�
		CRect	itemrect;
		pItm->GetWindowRect( &itemrect );
		ScreenToClient( &itemrect );

		// �N���C�A���g�̈���ɃA�C�e���������Ă��邩�H
		CRect intersect = dlgrect & itemrect;
		if ( intersect != itemrect ){
			// �����Ă��Ȃ��Ƃ�
			int pos=0;
			// ���݂̃X�N���[���o�[�̈ʒu�����߂�
			SCROLLINFO	sif;
			GetScrollInfo( SB_VERT , &sif ,SIF_ALL );

			// �A�C�e�����N���C�A���g�ʒu�������ɂ����
			if ( itemrect.bottom > dlgrect.bottom ){
				//pos = sif.nPos + itemrect.bottom - sif.nPage +30;
				pos = sif.nPos + itemrect.bottom - dlgrect.Height() +30;
			}
			// �A�C�e�����N���C�A���g�ʒu������ɂ����
			if ( itemrect.top < dlgrect.top ){
				pos = sif.nPos + itemrect.top -30;
			}
			// ��������ő�����
			pos = (pos<0)?0:pos;
			pos = (pos>(int)(sif.nMax-sif.nPage))?(sif.nMax-sif.nPage):pos;
			// �ʒu�ݒ肵����
			PostMessage( WM_VSCROLL , MAKELONG( SB_THUMBPOSITION , pos ) , (LPARAM)m_hWnd );
		}
	}
	return CDialog::OnWndMsg(message, wParam, lParam, pResult);
}

BOOL CScrollDialog::PreTranslateMessage(MSG* pMsg)
{
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
	return __super::PreTranslateMessage(pMsg);
}

void CScrollDialog::OnLButtonDown(UINT nFlags, CPoint point)
{
	Default();			// �^�b�v&�z�[���h�̂��邭��̗}�~
//	CDialog::OnLButtonDown(nFlags, point);
}

void CScrollDialog::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	if (AfxGetAygshellUIModel() == PocketPC){
		m_nCurHeight = cy;
		int nScrollMax;
		if (cy < m_nDialogHeight ){
			 nScrollMax = m_nDialogHeight  - cy;
		}else{
			 nScrollMax = 0;
		}

		SCROLLINFO si;
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_ALL; // SIF_ALL = SIF_PAGE | SIF_RANGE | SIF_POS;
		si.nMin = 0;
		si.nMax = nScrollMax;
		si.nPage = si.nMax/10;
		si.nPos = 0;
		SetScrollInfo(SB_VERT, &si, TRUE); 
	}
}

void CScrollDialog::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (AfxGetAygshellUIModel() == PocketPC){
		int nDelta;
		int nMaxPos = m_nDialogHeight - m_nCurHeight;

		switch (nSBCode)
		{
		case SB_LINEDOWN:
			if (m_nScrollPos >= nMaxPos)
				return;
			nDelta = min(nMaxPos/10,nMaxPos-m_nScrollPos);
			break;

		case SB_LINEUP:
			if (m_nScrollPos <= 0)
				return;
			nDelta = -min(nMaxPos/10,m_nScrollPos);
			break;

		case SB_THUMBTRACK://   �X�N���[�� �{�b�N�X���w��ʒu�փh���b�O�B���݈ʒu�̓p�����[�^ nPos �Ŏw�肳��܂��B
		case SB_THUMBPOSITION:
			nDelta = (int)nPos - m_nScrollPos;
			break;

		case SB_PAGEUP:
			if (m_nScrollPos <= 0)
				return;
			nDelta = -min(m_nCurHeight-16,m_nScrollPos);
			break;
		case SB_PAGEDOWN:
			if (m_nScrollPos >= nMaxPos)
				return;
			nDelta = min(m_nCurHeight-16,nMaxPos-m_nScrollPos);
			break;
		
		default:
			return;
		}
		m_nScrollPos += nDelta;
		SetScrollPos(SB_VERT,m_nScrollPos,TRUE);
		ScrollWindowEx(0,-nDelta,NULL,NULL,NULL,NULL,SW_SCROLLCHILDREN);
	}
	CDialog::OnVScroll(nSBCode, nPos, pScrollBar);
}
