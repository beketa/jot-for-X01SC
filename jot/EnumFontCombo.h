//
// jot - Text Editor for Windows Mobile
//   Copyright (C) 2007-2008, Aquamarine Networks.  <http://pandora.sblo.jp/>
//
//    This program is EveryOne's ware; you can redistribute it and/or modify it 
//    under the terms of the NYSL version 0.9982 or later. <http://www.kmonos.net/nysl/>
//

#pragma once

//ヘッダファイル
class CEnumFontCombo : public CComboBox
{
	DECLARE_DYNAMIC(CEnumFontCombo)
public:
    CEnumFontCombo();
    virtual ~CEnumFontCombo();

protected:
    void EnumFont(BYTE CharSet = ANSI_CHARSET, BOOL bDelPrevFonts = FALSE);
    //リストにフォントを列挙する
    //第１引数：対象とするキャラクターセットを指定
    //第２引数：前回までに列挙したフォントを削除するかどうか（削除する場合は TRUE 、しない場合は FALSE)を指定

    BOOL GetLogFont(LPLOGFONT lpLogFont);
    //選択されているフォントのデータ（LOGFONT 構造体）を取得する
    //成功すると lpLogFont が指す LOGFONT 構造体にフォントのデータがコピーされ、TRUEを返す
    //失敗すると FALSE を返す

    //コールバック関数
    static int CALLBACK EnumFontFamExProc(ENUMLOGFONTEX *lpelfe, NEWTEXTMETRICEX *lpntme, int FontType, LPARAM lParam);

protected:
    //列挙されたフォントデータ（LOGFONG 構造体）へのポインタの配列
    CArray<LOGFONT*, LOGFONT*> m_pLogFontArray;

    afx_msg void OnDestroy();
	afx_msg void OnCbnSelchange();
    DECLARE_MESSAGE_MAP()

public:
	void	Init( const TCHAR *nonefontname , const TCHAR *initfont )
	{
		// コンボボックスにフォント名を設定
		AddString( nonefontname );		// フォント未設定の項目
		SetCurSel( 0 );
		EnumFont(DEFAULT_CHARSET);

		// コンボボックスの現在設定されているフォントを選択
		if ( initfont != NULL ){
			int len = GetCount();
			for(int i=1;i<len;i++ ){
				CString item;
				GetLBText(i,item);
				if ( item == initfont ){
					SetCurSel( i );
				}
			}
		}
	}

};

