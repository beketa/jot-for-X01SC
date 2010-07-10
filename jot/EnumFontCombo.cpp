//
// jot - Text Editor for Windows Mobile
//   Copyright (C) 2007-2008, Aquamarine Networks.  <http://pandora.sblo.jp/>
//
//    This program is EveryOne's ware; you can redistribute it and/or modify it 
//    under the terms of the NYSL version 0.9982 or later. <http://www.kmonos.net/nysl/>
//

// EnumFontCombo.cpp : 実装ファイル
//

#include "stdafx.h"
#include "EnumFontCombo.h"



IMPLEMENT_DYNAMIC(CEnumFontCombo, CComboBox)


//ソースファイル
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
    //第２引数が TRUE なら、前回までに列挙したフォントを削除する
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
    ////トゥルータイプフォントだけを列挙する
    //if(FontType != TRUETYPE_FONTTYPE)
    //    return 1;

    //ドロップダウンリストにフェイス名を追加する
    CString facename = lpelfe->elfLogFont.lfFaceName;
    CEnumFontCombo* pCombo = (CEnumFontCombo*)lParam;	
    pCombo->AddString(facename);

    // LOGFONT オブジェクトを生成し、フォントデータをコピーしてからそのポインタを配列に追加する
    LOGFONT* pLogFont = new LOGFONT;
    ::CopyMemory(pLogFont, &(lpelfe->elfLogFont), sizeof(LOGFONT));
    pCombo->m_pLogFontArray.Add(pLogFont);

    return 1;
}

BOOL CEnumFontCombo::GetLogFont(LPLOGFONT lpLogFont)
{
    //選択されているフェース名を取得する
    CString selFaceName;
    GetWindowText(selFaceName);

    //フェース名に該当するフォントデータ（LOGFONT 構造体）を引数にコピーする
    CString tmpFaceName;
    int i, size = m_pLogFontArray.GetSize();
    for(i=0; i<size; i++){
        tmpFaceName = m_pLogFontArray[i]->lfFaceName;
        if(tmpFaceName == selFaceName){
            ::CopyMemory(lpLogFont, m_pLogFontArray[i], sizeof(LOGFONT));
            return TRUE;
        }
    }

    //フォントデータが見つからなかったら FALSE を返す               
    return FALSE;
}

void CEnumFontCombo::OnDestroy() 
{
    CComboBox::OnDestroy();

    // LOGFONT オブジェクトを全て破棄する
    int i, size = m_pLogFontArray.GetSize();
    for(i=0; i<size; i++)
        DEL( m_pLogFontArray[i] );

    //配列の要素もすべて削除する（念のため、、、）
    m_pLogFontArray.RemoveAll();
}


void CEnumFontCombo::OnCbnSelchange()
{
	int a=0;
}
