//
// jot - Text Editor for Windows Mobile
//   Copyright (C) 2007-2008, Aquamarine Networks.  <http://pandora.sblo.jp/>
//
//    This program is EveryOne's ware; you can redistribute it and/or modify it 
//    under the terms of the NYSL version 0.9982 or later. <http://www.kmonos.net/nysl/>
//

// EnumFontCombo.cpp : �����t�@�C��
//

#include "stdafx.h"
#include "EnumFontCombo.h"



IMPLEMENT_DYNAMIC(CEnumFontCombo, CComboBox)


//�\�[�X�t�@�C��
CEnumFontCombo::CEnumFontCombo()
{
}

CEnumFontCombo::~CEnumFontCombo()
{
}

BEGIN_MESSAGE_MAP(CEnumFontCombo, CComboBox)
    //{{AFX_MSG_MAP(CEnumFontCombo)
    ON_WM_DESTROY()
    //}}AFX_MSG_MAP
//	ON_WM_CREATE()
ON_CONTROL_REFLECT(CBN_SELCHANGE, &CEnumFontCombo::OnCbnSelchange)
END_MESSAGE_MAP()

void CEnumFontCombo::EnumFont(BYTE CharSet, BOOL bDelPrevFonts)
{
    //��Q������ TRUE �Ȃ�A�O��܂łɗ񋓂����t�H���g���폜����
    if(bDelPrevFonts){
        int i, size = m_pLogFontArray.GetSize();
        for(i=0; i<size; i++)
            DEL( m_pLogFontArray[i] );
        m_pLogFontArray.RemoveAll();

        while(DeleteString(0) > 0)
            ;
    }

    HDC hDC = ::GetDC(m_hWnd);

    LOGFONT logFont;
    _tcscpy(logFont.lfFaceName, _T(""));
    logFont.lfPitchAndFamily = 0;
    logFont.lfCharSet = CharSet;
    ::EnumFontFamiliesEx(hDC, &logFont, (FONTENUMPROC)EnumFontFamExProc, (LPARAM)this, 0);

    ::ReleaseDC(m_hWnd, hDC);
}

int CALLBACK CEnumFontCombo::EnumFontFamExProc(ENUMLOGFONTEX *lpelfe, NEWTEXTMETRICEX *lpntme, int FontType, LPARAM lParam)
{
    ////�g�D���[�^�C�v�t�H���g������񋓂���
    //if(FontType != TRUETYPE_FONTTYPE)
    //    return 1;

    //�h���b�v�_�E�����X�g�Ƀt�F�C�X����ǉ�����
    CString facename = lpelfe->elfLogFont.lfFaceName;
    CEnumFontCombo* pCombo = (CEnumFontCombo*)lParam;	
    pCombo->AddString(facename);

    // LOGFONT �I�u�W�F�N�g�𐶐����A�t�H���g�f�[�^���R�s�[���Ă��炻�̃|�C���^��z��ɒǉ�����
    LOGFONT* pLogFont = new LOGFONT;
    ::CopyMemory(pLogFont, &(lpelfe->elfLogFont), sizeof(LOGFONT));
    pCombo->m_pLogFontArray.Add(pLogFont);

    return 1;
}

BOOL CEnumFontCombo::GetLogFont(LPLOGFONT lpLogFont)
{
    //�I������Ă���t�F�[�X�����擾����
    CString selFaceName;
    GetWindowText(selFaceName);

    //�t�F�[�X���ɊY������t�H���g�f�[�^�iLOGFONT �\���́j�������ɃR�s�[����
    CString tmpFaceName;
    int i, size = m_pLogFontArray.GetSize();
    for(i=0; i<size; i++){
        tmpFaceName = m_pLogFontArray[i]->lfFaceName;
        if(tmpFaceName == selFaceName){
            ::CopyMemory(lpLogFont, m_pLogFontArray[i], sizeof(LOGFONT));
            return TRUE;
        }
    }

    //�t�H���g�f�[�^��������Ȃ������� FALSE ��Ԃ�               
    return FALSE;
}

void CEnumFontCombo::OnDestroy() 
{
    CComboBox::OnDestroy();

    // LOGFONT �I�u�W�F�N�g��S�Ĕj������
    int i, size = m_pLogFontArray.GetSize();
    for(i=0; i<size; i++)
        DEL( m_pLogFontArray[i] );

    //�z��̗v�f�����ׂč폜����i�O�̂��߁A�A�A�j
    m_pLogFontArray.RemoveAll();
}


void CEnumFontCombo::OnCbnSelchange()
{
	int a=0;
}
