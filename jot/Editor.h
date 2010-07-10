//
// jot - Text Editor for Windows Mobile
//   Copyright (C) 2007-2008, Aquamarine Networks.  <http://pandora.sblo.jp/>
//
//    This program is EveryOne's ware; you can redistribute it and/or modify it 
//    under the terms of the NYSL version 0.9982 or later. <http://www.kmonos.net/nysl/>
//

#pragma once

// CEditor

#define	EDITOR_INISTR_MAX	64
#define	EDITOR_SELSTR_MAX	1024*1024

class CEditor : public CEdit
{
	DECLARE_DYNAMIC(CEditor)

public:
	CEditor();
	virtual ~CEditor();

protected:
	DECLARE_MESSAGE_MAP()

	BOOL	m_lastmodify;
	bool	m_bXscrawl;
	TCHAR	m_inistr[EDITOR_INISTR_MAX+1];
	bool	m_bRegex;
	bool	m_bNoCase;

	UINT	m_modeXscrawl;
	void	DoXscrawl( bool up );

	unsigned int	m_wordwrap_pos_p;
	unsigned int	m_wordwrap_pos_l;

	TCHAR * m_buffer;
	int		m_bufferLen;

	TCHAR * GetBuffer();
	BOOL	SetBuffer( const TCHAR * buff, int size );

protected:
	afx_msg LRESULT OnEnterMenuLoop(WPARAM wp, LPARAM lp);
	afx_msg LRESULT OnExitMenuLoop(WPARAM wp, LPARAM lp);
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
	afx_msg void OnEditCopy();
	afx_msg void OnEditCut();
	afx_msg void OnEditPaste();
	afx_msg void OnEditUndo();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnEnChange();
	afx_msg void OnShiftLock();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnSelectall();
	afx_msg void OnJumpEnd();
	afx_msg void OnJumpHome();
	afx_msg void OnJumpNext();
	afx_msg void OnJumpPrev();
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnSetFocus(CWnd* pOldWnd);

public:
	BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);

	void	SetXscrawl( UINT mode )
	{
		m_modeXscrawl = mode;
	}

	void	SetModify(BOOL bModified = 1){
		m_lastmodify = bModified;
		__super::SetModify( bModified  );
	}

	void	SetWordwrap( BOOL wrap );
	void	SetWordwrapPos( unsigned int portrait ,unsigned int landscape );
	BOOL	SetMemory( const TCHAR *buff , int size );
	const	TCHAR *GetMemory( );
	void	JumpLineNo(int lineno);
	void	FindString( CString str , bool down , bool first );
	void	Replace( CString str ,CString rep , bool down );
	void	ReplaceAll( CString str ,CString rep );
	void	SetDrawSymbol( DWORD flags );
	const TCHAR *	GetFindInistr();
	CString	GetSelectedStr();

	void	SetRegex( bool b ){	m_bRegex = b; }
	void	SetNoCase( bool b ){m_bNoCase = b; }

	void	SetColor( int code , DWORD color );
	void	SetSpacing(int	spacing);
	void	SetUnderline( BOOL value );
	void	SetNotFocusIme( BOOL value );
	void	SetBsWmChar( BOOL value );
	void	SetDrawLineNumber( BOOL value );
	void	Relocate();
};

enum	XscrawlMode {
	None = 0,
	line05,
	line10,
	line15,
	line20,
	page,
	line01,
	line02,
};

#ifndef	DS_LINEBREAK		
#define	DS_LINEBREAK		(0x00000001)
#define DS_TAB				(0x00000002)
#define	DS_SPACE			(0x00000004)
#define DS_WIDESPACE		(0x00000008)
#endif
