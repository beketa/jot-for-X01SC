//
// jot - Text Editor for Windows Mobile
//   Copyright (C) 2007-2008, Aquamarine Networks.  <http://pandora.sblo.jp/>
//
//    This program is EveryOne's ware; you can redistribute it and/or modify it 
//    under the terms of the NYSL version 0.9982 or later. <http://www.kmonos.net/nysl/>
//

// ChildView.h : CChildView クラスのインターフェイス
//


#pragma once
#include "Editor.h"

#include "BottomBar.h"

#include "mrulist.h"
#include "mrufind.h"
#include "Template.h"


#define	IDM_MRU_START	50000
#define	IDM_MRU_END		50040

#define	IDM_TEMPLATE_START	51000
#define	IDM_TEMPLATE_END	51040


// CChildView ウィンドウ
class	CReadBuff {
private:
	TCHAR	*m_readbuff;
	int		m_buffptr;
	int		m_buffsize;
	bool	m_bufferr;
	HANDLE	m_hMmap;
#define	BLOCKSIZE	65536*4*4*32
private:
	void*	Alloc( int size , HANDLE *hMap ){
		void *buff = NULL;
		*hMap = CreateFileMapping((HANDLE)INVALID_HANDLE_VALUE,NULL,PAGE_READWRITE|SEC_COMMIT,0,size,NULL );
		if ( *hMap != NULL ){
			buff = MapViewOfFile( *hMap , FILE_MAP_WRITE , 0 , 0 , 0 );
		}
		return buff;
	}

	void	Free( void *ptr , HANDLE hMap ){
		if ( hMap != NULL ){
			if ( ptr !=NULL ){
				UnmapViewOfFile( ptr );
			}
			CloseHandle(hMap);
		}
	}


public:
	CReadBuff(){
		m_bufferr = true;
		m_buffsize = BLOCKSIZE;
		//m_readbuff = (TCHAR*)VirtualAlloc(NULL,m_buffsize*sizeof(TCHAR),MEM_COMMIT,PAGE_EXECUTE_READWRITE);
		// virtualalloc使っても32MBの壁を越えられなかったorz
		//m_readbuff = new TCHAR[m_buffsize];
		m_readbuff = (TCHAR*)Alloc( m_buffsize*sizeof(TCHAR) , &m_hMmap );

		if ( m_readbuff !=NULL ){
			m_buffptr = 0;
			m_readbuff[0] = 0;
			m_bufferr = false;
		}
	}

	virtual	~CReadBuff()
	{
		if ( m_readbuff  != NULL ){
			//VirtualFree(m_readbuff ,0,MEM_RELEASE);
			//DELA( m_readbuff );
			Free( m_readbuff , m_hMmap );
		}
	}

	void	AddBuff(const TCHAR *p){
		if ( p==NULL || *p == 0 ){
			return;
		}

		int len = _tcslen(p);
		int bs = m_buffsize;
		while ( m_buffptr + len >= bs ){
			bs += BLOCKSIZE;
		}
		if ( bs != m_buffsize ){
			//TCHAR *newbuf = (TCHAR*)VirtualAlloc(NULL,bs*sizeof(TCHAR),MEM_COMMIT,PAGE_EXECUTE_READWRITE);
			//TCHAR *newbuf = new TCHAR[bs];
			HANDLE hMap;
			TCHAR *newbuf  = (TCHAR*)Alloc( bs*sizeof(TCHAR) , &hMap );

			if ( newbuf == NULL ){
				m_bufferr = true;
				//DELA( m_readbuff );
				Free( m_readbuff , m_hMmap );
				return;
			}
			CopyMemory( newbuf , m_readbuff , m_buffptr*sizeof(TCHAR) );

			//VirtualFree(m_readbuff ,0,MEM_RELEASE);
			//DELA( m_readbuff );
			Free( m_readbuff , m_hMmap );

			m_readbuff = newbuf;
			m_buffsize = bs;
		}

		_tcscpy( m_readbuff + m_buffptr , p );
		m_buffptr += len;
	}
	TCHAR	*GetBuff()
	{
		return m_readbuff;
	}
	int		GetLength()
	{
		return m_buffptr;
	}

	bool	IsError()
	{
		return m_bufferr;
	}
};

class CChildView : public CWnd,IUseBottomBar
{
// コンストラクション
public:
	CChildView();
	virtual ~CChildView();

protected:
	// TEXT
	CEditor	m_Edit;
	CString	m_filename;		// ファイル名

	int m_charset;		// 文字コード
	int m_lbcode;		// 改行コード
	CString	m_fontname;
	UINT	m_fontsize;
	UINT	m_tabsize;
	CFont	*m_pfont;
	//BOOL	m_allfile;
	BOOL	m_hscroll;
	DWORD	m_drawsymbol;
	const TCHAR	*GetFilter();
	TCHAR	m_filbuf[MAX_PATH];
	UINT	m_xscrawl;

	bool	m_findfirst;
	bool	m_charset_autodetect;				// 文字コード自動検出フラグ
	bool	m_validfilename;	// ファイル名有効フラグ
	CBottomBar	*m_toolbar;
	HANDLE	m_hLock;	// 排他制御ハンドル

	CMruList m_mrulist;
	int		m_mrunum;

	CMruFind	m_mrufind;
	int			m_mrufindnum;
	CMruFind	m_mrurepl;
	int			m_mrureplnum;

	BOOL	m_bRegex;
	BOOL	m_bIgnoreCase;
	BOOL	m_bFullSc;
	BOOL	m_bBoldFont;
	BOOL	m_bDisAutoDetect;

	DWORD	m_ColorHighlight;
	DWORD	m_ColorHighlightText;
	DWORD	m_ColorWindow;
	DWORD	m_ColorWindowText;
	DWORD	m_ColorUnderline;

	int		m_property[16];

	bool	m_firstboot;
	CTemplate	m_template;

	BOOL	m_bUnderline;
	int		m_linespacing;
	BOOL	m_bNotFocusIme;

	int		m_NewFileCharCode;
	int		m_NewFileLineBreak;
	BOOL	m_bBsWmChar;
	BOOL	m_bBackAsBs;

	unsigned char m_wordwrap_pos_p;
	unsigned char m_wordwrap_pos_l;
	BOOL	m_bDrawLineNumber;

protected:
	bool	OpenFile( const TCHAR *filename ,bool arg=false);	// ファイル読み出し
	void	Savefile( const TCHAR *filename );	// ファイル保存
	bool	SaveAs();							// ファイル保存ダイアログ
	void	NewFile();							// 新規ファイル作成
	bool	SaveOverwrite();					// 上書き保存
	void	SetTitle( BOOL modify );			// ウインドウタイトル書き換え


	void	SaveSettings();
	void	LoadSettings();

	void	ChangeFont();
	void	Layout();
	void	NotifyBottomBar(UINT endcode,CString *p1 , CString *p2);

	HWND	LockFile(HWND hWnd,CString fname);
	void	UnlockFile();
	
	//void	LoadMruList();
	//void	SaveMruList();

	DWORD	LoadColor( const TCHAR *key )
	{
		CString colstr = theApp.m_miscfile.readString( MISC_SEC , key );
		if ( colstr.GetLength() == 0 ){		// 未定義ならシステム設定値
			return 0xFFFFFFFF;
		}
		TCHAR	*p;
		DWORD color0 = _tcstol(colstr,&p,16);		// RRGGBBのフォーマット

		DWORD	R = (color0>>16) & 0xFF;
		DWORD	G = (color0>> 8) & 0xFF;
		DWORD	B = (color0    ) & 0xFF;

		DWORD color = (B<<16)|(G<<8)|(R);			// BBGGRRにして返す
		return color;
	}

	void	LoadProperty(const TCHAR *filename);

	void	SetNewFileCharCode();
// 生成された、メッセージ割り当て関数
protected:
	DECLARE_MESSAGE_MAP()

	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	afx_msg LRESULT OnTextChanged(WPARAM wp, LPARAM lp);
	afx_msg LRESULT OnNotifyFocus(WPARAM wp, LPARAM lp);
	afx_msg void OnFileSaveas();
	afx_msg void OnFileSave();
	afx_msg void OnFileOpen();
	afx_msg void OnFileNew();
	afx_msg void OnCharsetAutodetect();
	afx_msg void OnUpdateCharsetAutodetect(CCmdUI *pCmdUI);
	afx_msg BOOL OnCharset(UINT nID);
	afx_msg void OnUpdateCharset(CCmdUI *pCmdUI);
	afx_msg BOOL OnLb(UINT nID);
	afx_msg void OnUpdateLb(CCmdUI *pCmdUI);

	afx_msg void OnToolOption();
	afx_msg void OnJumpLineno();

	afx_msg void OnSearchSearch();
	afx_msg void OnSearchReplace();
	afx_msg void OnFileReopen();
	afx_msg void OnUpdateFileReopen(CCmdUI *pCmdUI);
	afx_msg void OnMruOpen(UINT nID);
	afx_msg void OnToolSearchoption();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnViewFontdown();
	afx_msg void OnViewFontup();
	afx_msg void OnViewWordwrap();
	afx_msg void OnUpdateViewWordwrap(CCmdUI *pCmdUI);
	afx_msg void OnPropAutosave();
	afx_msg void OnUpdatePropAutosave(CCmdUI *pCmdUI);
	afx_msg void OnTemplate(UINT nID);
	afx_msg void OnToolCompatibility();
	afx_msg void OnViewRelocate();
public:
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	BOOL	m_menutenkey;
	bool	ConfirmSave();						// 保存確認
	void	SetMruMenu(int menuno , HMENU hmenu);
	void	AutoSave(bool mode);

};

