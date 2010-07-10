//
// jot - Text Editor for Windows Mobile
//   Copyright (C) 2007-2008, Aquamarine Networks.  <http://pandora.sblo.jp/>
//
//    This program is EveryOne's ware; you can redistribute it and/or modify it 
//    under the terms of the NYSL version 0.9982 or later. <http://www.kmonos.net/nysl/>
//

// Editor.cpp : 実装ファイル
//

#include "stdafx.h"
#include "jot.h"
#include "Editor.h"
#include "nEdit.h"

#include "search.h"

// CEditor

IMPLEMENT_DYNAMIC(CEditor, CEdit)

CEditor::CEditor()
{
	m_lastmodify = false;
	m_bXscrawl = false;
	m_bRegex = false;
	m_bNoCase = false;
	m_wordwrap_pos_p =0;
	m_wordwrap_pos_l =0;

	m_buffer = NULL;
	m_bufferLen = -1;
}

CEditor::~CEditor()
{
	delete [] m_buffer;
}


BEGIN_MESSAGE_MAP(CEditor, CEdit)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_EDIT_COPY, &CEditor::OnEditCopy)
	ON_COMMAND(ID_EDIT_CUT, &CEditor::OnEditCut)
	ON_COMMAND(ID_EDIT_PASTE, &CEditor::OnEditPaste)
	ON_COMMAND(ID_EDIT_UNDO, &CEditor::OnEditUndo)
	ON_WM_SIZE()
	ON_CONTROL_REFLECT(EN_CHANGE, &CEditor::OnEnChange)
	ON_COMMAND(ID_SHIFT_LOCK, &CEditor::OnShiftLock)
	ON_COMMAND(ID_SELECTALL, &CEditor::OnSelectall)
//#ifdef WIN32_PLATFORM_PSPC			// es/ades用のテンキーでのメニューアクセス機能
	ON_MESSAGE(WM_ENTERMENULOOP, OnEnterMenuLoop)
	ON_MESSAGE(WM_EXITMENULOOP, OnExitMenuLoop)
//#endif

	ON_COMMAND(ID_JUMP_END, &CEditor::OnJumpEnd)
	ON_COMMAND(ID_JUMP_HOME, &CEditor::OnJumpHome)
	ON_COMMAND(ID_JUMP_NEXT, &CEditor::OnJumpNext)
	ON_COMMAND(ID_JUMP_PREV, &CEditor::OnJumpPrev)
	ON_WM_MOUSEWHEEL()
	ON_WM_SETFOCUS()
END_MESSAGE_MAP()



// CEditor メッセージ ハンドラ



void CEditor::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	// シフトキー離す
	theApp.SendShiftKey( false );
	// メインフレームに通知
	theApp.ShowShiftUnlockCommandBar( false );

	CMenu Menu;
	theApp.GetMenu( &Menu , IDR_CONTEXTMENU);

	CMenu* pMenu = Menu.GetSubMenu(0);
	//CMenu* editmenu = menu->GetSubMenu(3);

	pMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_NONOTIFY, point.x, point.y,this);
}

BOOL CEditor::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	CWnd* pWnd = this;
	BOOL ret = FALSE;
	if ( ( dwStyle & ES_MULTILINE )!=0 ){
		// Use standard edit control instead of nEdit for X01SC
		ret = CEdit::Create(dwStyle, rect, pParentWnd, nID);
	}else{
		// シングルラインの時は通常のエディットコントロール
		ret = CEdit::Create(dwStyle, rect, pParentWnd, nID);
	}

	if (AfxGetAygshellUIModel() == PocketPC){
		SendMessage( WM_SETPLATFORM , 0 );
	}else{
		SendMessage( WM_SETPLATFORM , 1 );
	}
	return ret;

}

void CEditor::OnEditCopy()
{
	Copy();
}

void CEditor::OnEditCut()
{
	Cut();
}

void CEditor::OnEditPaste()
{
	Paste();
}


void CEditor::OnEditUndo()
{
	Undo();
}


void CEditor::OnSize(UINT nType, int cx, int cy)
{
	unsigned int	pos = DRA::GetDisplayMode()==DRA::Portrait ? m_wordwrap_pos_p : m_wordwrap_pos_l;

	// ワードラップ位置の設定
	SendMessage( WM_SETWORDWRAPPOS ,pos , 0 );

	CEdit::OnSize(nType, cx, cy);

	// リサイズされたとき、キャレット位置（選択位置）が画面内に来るように調整処理を入れる
	SendMessage( EM_SCROLLCARET , 0);
}

void CEditor::OnEnChange()
{
	// エディタに変更があれば、エディットコントロールの変更状態をチェックし、
	// 変更があれば状態を親にポストする
	BOOL nowmodify = GetModify();
	if ( m_lastmodify != nowmodify ){
		m_lastmodify = nowmodify;
		GetParent()->PostMessageW( WM_JOT_TEXTCHANGED , nowmodify ,0 );
	}
}



void CEditor::OnShiftLock()
{
	theApp.SendShiftKey( true );

	// メインフレームに通知
	theApp.ShowShiftUnlockCommandBar( true );
}

BOOL CEditor::PreTranslateMessage(MSG* pMsg)
{
	if ( pMsg->message == WM_KEYDOWN ){
		switch ( pMsg->wParam ){
		case VK_F1:
		case VK_F2:
			theApp.SendShiftKey( false );
			break;
//#ifdef WIN32_PLATFORM_PSPC		// Xscrawl対応はPPCのみ
		case 0x83:
			if (AfxGetAygshellUIModel() == PocketPC){
				m_bXscrawl = true;
			}
			break;
//#endif
		}

	}
	if ( pMsg->message == WM_KEYUP ){
		if (AfxGetAygshellUIModel() == PocketPC){
//#ifdef WIN32_PLATFORM_PSPC		// Xscrawl対応はPPCのみ
			switch ( pMsg->wParam ){
			case VK_UP:
				if ( m_bXscrawl ){
					DoXscrawl( TRUE );
					return FALSE;
				}
				break;
			case VK_DOWN:
				if ( m_bXscrawl ){
					DoXscrawl( FALSE );
					return FALSE;
				}
				break;
			case 0x83:
				m_bXscrawl = false;
				break;
			}
		}
//#endif
	}
	return CEdit::PreTranslateMessage(pMsg);
}


void CEditor::OnSelectall()
{
	SetSel( 0 , -1 );
}




//#ifdef WIN32_PLATFORM_PSPC			// es/ades用のテンキーでのメニューアクセス機能
// エクスクでの動作
void	CEditor::DoXscrawl( bool up )
{
	if (AfxGetAygshellUIModel() == PocketPC){
		switch( m_modeXscrawl ){
		case None:			// エクスク無し
			break;
		case line01:		// 1行スクロール
			{
				int line = 1;
				if ( up )	line *= -1;
				LineScroll( line , 0);
			}
			break;

		case line05:		// ５～２０行スクロール
		case line10:
		case line15:
		case line20:
			{
				int line = m_modeXscrawl - line05 +1;
				line *= 5;
				if ( up )	line *= -1;
				LineScroll( line , 0);
			}
			break;
		case page:			// ページスクロール
			if ( up ){
				theApp.ClickKey( VK_PRIOR );
			}else{
				theApp.ClickKey( VK_NEXT );
			}
			break;

		}
	}
}

LRESULT CEditor::OnEnterMenuLoop(WPARAM wp, LPARAM lp)
{
	if (AfxGetAygshellUIModel() == PocketPC){
		theApp.MenuLoop( true );
	}
	return 0;
}


LRESULT CEditor::OnExitMenuLoop(WPARAM wp, LPARAM lp)
{
	if (AfxGetAygshellUIModel() == PocketPC){
		theApp.MenuLoop( false );
	}
	return 0;
}

//#endif
void CEditor::OnJumpEnd()
{
	// 最終行に移動
	int	max = GetLineCount();
	int idx = LineIndex(max-1);
	SetSel( idx,idx );

	//// 最初CTRL+ENDを送ろうとしたんだけれど、
	//// CTRL UPがうまく送れず、CTRL入りっぱなしになってしまったので断念
}

void CEditor::OnJumpHome()
{
	// 先頭行に移動
	SetSel( 0,0 );

}

void CEditor::OnJumpNext()
{
	// PAGE_DOWNを送る
	theApp.ClickKey( VK_NEXT );
}

void CEditor::OnJumpPrev()
{
	// PAGE_UPを送る
	theApp.ClickKey( VK_PRIOR );
}


// EMONEのスクロールホイール対応
BOOL CEditor::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	if ( zDelta<0 ){				// 下
		DoXscrawl( false );
	}else if ( zDelta>0 ){			// 上
		DoXscrawl( true );
	}
	return CEdit::OnMouseWheel(nFlags, zDelta, pt);
}

// 行番号指定ジャンプ
void	CEditor::JumpLineNo(int lineno)
{
	// 最終行に移動
	int	max = GetLineCount();
	if ( lineno > max ){
		lineno = max;
	}
	int idx = LineIndex(lineno-1);
	SetSel( idx,idx );
}


void	CEditor::FindString( CString str , bool down , bool first )
{
	// 全テキスト取得
	const TCHAR	*text = GetBuffer();

	// 選択位置取得
	int st,ed;
	GetSel(st,ed);

	int	fst;
	int fed;

	// 検索オブジェクト生成
	CTextSearch		textsearch;
	CRegexSearch	regexsearch;

	CSearch	*search;
	if ( m_bRegex ){
		search = &regexsearch;
	}else{
		search = &textsearch;
	}
	search->IgnoreCase( m_bNoCase );
research:

	// 検索開始位置決定
	if ( down == first ){		// 下方向かつ最初、または、上方向かつ2回目以降なら選択先頭位置から
		fst = st;
	}else{						// それ以外は選択終了位置から
		fst = ed;
	}
	// パターン設定
	if ( search->Pattern( str ) ){
		// 検索実行
		if ( search->Search( down ,text,fst,fed ) ){
			// 発見

			// 初回で検索結果が選択位置と変わらなければやり直し
			if ( first && fst==st && fed==ed ){
				first = false;
				goto research;
			}
			SetSel( fst,fed );
		}
	}
}

// 検索初期文字列取得
const TCHAR *	CEditor::GetFindInistr()
{
	CString	ret;
	// 全テキスト取得
	const TCHAR	*text = GetBuffer();

	// 選択位置取得
	int st,ed;
	GetSel(st,ed);

	int	len = ed - st;

	// EDITOR_INISTR_MAX文字以上選択されていたら打ち切る
	if ( len > EDITOR_INISTR_MAX ){
		return _T("");
	}

	_tcsncpy( m_inistr , text+st , len );
	m_inistr[len] = _T('\0');

	// 改行があれば打ち切り
	TCHAR *p = _tcschr( m_inistr , _T('\r') );
	if ( p!=NULL ){
		return _T("");
	}
	return m_inistr;
}


void	CEditor::SetWordwrap( BOOL wrap )
{
	SendMessage( WM_SETWORDWRAP ,wrap );
}

BOOL CEditor::SetMemory( const TCHAR *buff , int size )
{
	return SetBuffer( buff, size );
}

const TCHAR *CEditor::GetMemory(  )
{
	return GetBuffer();
}

void	CEditor::SetDrawSymbol( DWORD flags ){
	SendMessage( WM_DRAWSYMBOL , (WPARAM)flags );
}

void CEditor::OnSetFocus(CWnd* pOldWnd)
{
	CEdit::OnSetFocus(pOldWnd);

	GetParent()->PostMessage( WM_JOT_NOTIFYFOCUS ,(WPARAM)m_hWnd );
}


void	CEditor::Replace( CString str ,CString rep , bool down )
{
	// 全テキスト取得
	const TCHAR	*text = GetBuffer();

	// 選択位置取得
	int st,ed;
	GetSel(st,ed);

	int	fst;
	int fed;

	// 検索オブジェクト生成
	CTextSearch		textsearch;
	CRegexSearch	regexsearch;

	CSearch	*search;
	if ( m_bRegex ){
		search = &regexsearch;
	}else{
		search = &textsearch;
	}
	search->IgnoreCase( m_bNoCase );

	// 検索開始位置決定
	fst = st;

	// パターン設定
	if ( search->Pattern( str ) ){

		// 検索実行
		if ( search->Search( down ,text,fst,fed ) ){
			// 発見
			SetSel( fst,fed );		// そこを選択して

			if ( st==fst && ed == fed ){	// 直前に選択されていた位置と一致すれば
				ReplaceSel( rep );					// 置換して
				FindString( str , down , false );	// 次を検索
			}
		}
	}
}

void	CEditor::ReplaceAll( CString str ,CString rep )
{
	int	fst=0;
	int fed;

	// 検索オブジェクト生成
	CTextSearch		textsearch;
	CRegexSearch	regexsearch;

	CSearch	*search;
	if ( m_bRegex ){
		search = &regexsearch;
	}else{
		search = &textsearch;
	}
	search->IgnoreCase( m_bNoCase );

	// パターン設定
	if ( search->Pattern( str ) ){

		int	len = _tcslen( rep );
		// 検索実行
		const TCHAR	*text;
		int	cnt=0;

		DWORD	start,tmp0,tmp,d1,d2;
		d1 = 0;
		d2 = 0;
		tmp0 = start = GetTickCount();

		bool	cont = false;
		while ( text = GetBuffer(),
			search->Search( true ,text,fst,fed ) ){

			tmp = GetTickCount();
			d1 += ( tmp - tmp0 );
			tmp0 = tmp;
			// 発見
			SetSel( fst,fed );		// そこを選択して
			ReplaceSel( rep );		// 置換

			tmp = GetTickCount();
			d2 += ( tmp - tmp0 );
			tmp0 = tmp;

			fst += len;
			cnt++;

			if ( tmp - start > 30000 && !cont ){
				CString title;
				title.LoadString( IDS_CONFIRM );
				CString	msg;
				msg.LoadString(IDS_CONFIRM_REPLACE);

				int ret = MessageBox( msg , title ,MB_OKCANCEL );
				if ( ret == IDOK ){
					cont = true;
				}else{
					break;
				}
			}
		}
		CString strRep;
		strRep.Format(	_T("search = %d[ms]\n") 
						_T("replace= %d[ms]\n") , d1 , d2 );
		OutputDebugString( strRep );

		if ( cnt > 0  ){
			CString title;
			title.LoadString( IDS_CONFIRM );
			CString	fmt;
			fmt.LoadString(IDS_REPORT_REPLACE);
			CString msg;
			msg.Format( fmt , cnt );

			MessageBox( msg , title ,MB_OK );
		}
	}
}

void	CEditor::SetColor( int code , DWORD color )
{
	SendMessage( WM_SETCOLOR , (WPARAM)code , (LPARAM)color );
}

// テンプレート用選択文字列取得
CString	CEditor::GetSelectedStr()
{
	// 全テキスト取得
	const TCHAR	*text = GetBuffer();
	// 選択位置取得
	int st,ed;
	GetSel(st,ed);

	int	len = ed - st;

	// EDITOR_SELSTR_MAX文字以上選択されていたら打ち切る
	if ( len > EDITOR_SELSTR_MAX){
		len = EDITOR_SELSTR_MAX;
	}

	TCHAR	*buff = new TCHAR[len+1];
	_tcsncpy( buff , text+st , len );
	buff[len] = _T('\0');

	CString	ret = buff;

	DELA( buff );
	return ret;
}

// 行間設定
void	CEditor::SetSpacing(int	spacing)
{
	SendMessage( WM_SETSPACING , spacing );
}

// フラグ設定
void	CEditor::SetUnderline( BOOL value )
{
	SendMessage( WM_SETFLAGS , NEDIT_FLAGS_UNDERLINE , value?NEDIT_FLAGS_UNDERLINE:0 );
}
void	CEditor::SetNotFocusIme( BOOL value )
{
	SendMessage( WM_SETFLAGS , NEDIT_FLAGS_NOTFOCUSIME , value?NEDIT_FLAGS_NOTFOCUSIME:0 );
}
void	CEditor::SetBsWmChar( BOOL value )
{
	SendMessage( WM_SETFLAGS , NEDIT_FLAGS_BSWMCHAR , value?NEDIT_FLAGS_BSWMCHAR:0 );
}

void	CEditor::SetDrawLineNumber( BOOL value )
{
	SendMessage( WM_SETFLAGS , NEDIT_FLAGS_LINENUMBER , value?NEDIT_FLAGS_LINENUMBER:0 );
}


void CEditor::Relocate()
{
	// キャレット位置（選択位置）が画面中央に来るように再配置処理を入れる
	SendMessage( EM_SCROLLCARET , 1);
}


void CEditor::SetWordwrapPos( unsigned int portrait ,unsigned int landscape )
{
	m_wordwrap_pos_p =portrait;
	m_wordwrap_pos_l =landscape;

	unsigned int	pos = DRA::GetDisplayMode()==DRA::Portrait ? m_wordwrap_pos_p : m_wordwrap_pos_l;
	// ワードラップ位置の設定
	SendMessage( WM_SETWORDWRAPPOS ,pos , 1 );

}

TCHAR * CEditor::GetBuffer()
{
	int length = GetWindowTextLength();

	if (m_bufferLen < length) {
		delete [] m_buffer;

		m_bufferLen = length + 1024;
		m_buffer = new TCHAR[m_bufferLen];
	}

	GetWindowText(m_buffer, m_bufferLen);

	return m_buffer;
}

BOOL CEditor::SetBuffer( const TCHAR * buff, int size )
{
	if (m_bufferLen < size + 1) {
		delete [] m_buffer;

		m_bufferLen = size + 1024;
		m_buffer = new TCHAR[m_bufferLen];
	}

	StringCbCopyN(m_buffer, m_bufferLen * sizeof(TCHAR), buff, size * sizeof(TCHAR));
	m_buffer[size] = '\0';

	SetWindowText(m_buffer);

	return TRUE;
}
