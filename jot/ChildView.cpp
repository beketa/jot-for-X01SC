//
// jot - Text Editor for Windows Mobile
//   Copyright (C) 2007-2008, Aquamarine Networks.  <http://pandora.sblo.jp/>
//
//    This program is EveryOne's ware; you can redistribute it and/or modify it 
//    under the terms of the NYSL version 0.9982 or later. <http://www.kmonos.net/nysl/>
//

// ChildView.cpp : CChildView クラスの実装
//

#include "stdafx.h"
#include "jot.h"
#include "ChildView.h"
#include "cmdarg.h"
#include "txt.h"
#include "SettingDlg.h"

#include "JumpLineNo.h"
#include "FindBar.h"
#include "ReplaceBar.h"
#include "SearchSettingDlg.h"
#include "CompatiDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ID変換関数　
struct CHARSET_ID{
	UINT nID;
	int charset;
};

static const CHARSET_ID s_charset_id[]={
	{ ID_CHARSET_SHIFTJIS,	SJIS },
	{ ID_CHARSET_JIS,	IsoJP },
	{ ID_CHARSET_EUC,	EucJP },
	{ ID_CHARSET_UTF8,	UTF8N },
	{ ID_CHARSET_UTF16,	UTF16l },
	{ ID_CHARSET_LATIN,	Latin },
	{ 0 , -1 }
};

static const CHARSET_ID s_lb_id[]={
	{ ID_LB_CR,	Txt::CR },
	{ ID_LB_CRLF,	Txt::CRLF },
	{ ID_LB_LF,	Txt::LF },
	{ 0 , -1 }
};

static UINT charset2id( int charset )
{
	for(int i=0;s_charset_id[i].nID != 0;i++ ){
		if ( s_charset_id[i].charset == charset ){
			return s_charset_id[i].nID;
		}
	}
	return -1;
}

static int id2charset( UINT nID )
{
	for(int i=0;s_charset_id[i].nID != 0;i++ ){
		if ( s_charset_id[i].nID == nID ){
			return s_charset_id[i].charset;
		}
	}
	return -1;
}

static UINT lb2id( int lb )
{
	for(int i=0;s_lb_id[i].nID != 0;i++ ){
		if ( s_lb_id[i].charset == lb ){
			return s_lb_id[i].nID;
		}
	}
	return -1;
}

static int id2lb( UINT nID )
{
	for(int i=0;s_lb_id[i].nID != 0;i++ ){
		if ( s_lb_id[i].nID == nID ){
			return s_lb_id[i].charset;
		}
	}
	return -1;
}



// CChildView

CChildView::CChildView()
	:m_mrulist(60,MRUFILE)
	,m_mrufind(MRUFIND)
	,m_mrurepl(MRUREPLACE)
	,m_template(40,TEMPLATEFILE)
{
	m_charset_autodetect = true;
	m_pfont = NULL;
	m_hLock = NULL;
	m_toolbar = NULL;
	ZeroMemory( m_property , sizeof(int)*16 );
}

CChildView::~CChildView()
{
//	SaveMruList();
	DEL( m_pfont );
}


BEGIN_MESSAGE_MAP(CChildView, CWnd)
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_WM_SETFOCUS()
	ON_MESSAGE(WM_JOT_TEXTCHANGED, OnTextChanged)
	ON_MESSAGE(WM_JOT_NOTIFYFOCUS, OnNotifyFocus)

	ON_COMMAND(ID_FILE_SAVEAS, &CChildView::OnFileSaveas)
	ON_COMMAND(ID_FILE_SAVE, &CChildView::OnFileSave)
	ON_COMMAND(ID_FILE_OPEN, &CChildView::OnFileOpen)
	ON_COMMAND(ID_FILE_NEW_, &CChildView::OnFileNew)
	ON_COMMAND(ID_CHARSET_AUTODETECT, &CChildView::OnCharsetAutodetect)
	ON_UPDATE_COMMAND_UI(ID_CHARSET_AUTODETECT, &CChildView::OnUpdateCharsetAutodetect)
	ON_COMMAND_EX(ID_CHARSET_UTF8, &CChildView::OnCharset)
	ON_COMMAND_EX(ID_CHARSET_UTF16, &CChildView::OnCharset)
	ON_COMMAND_EX(ID_CHARSET_SHIFTJIS, &CChildView::OnCharset)
	ON_COMMAND_EX(ID_CHARSET_JIS, &CChildView::OnCharset)
	ON_COMMAND_EX(ID_CHARSET_EUC, &CChildView::OnCharset)
	ON_COMMAND_EX(ID_CHARSET_LATIN, &CChildView::OnCharset)
	ON_UPDATE_COMMAND_UI(ID_CHARSET_UTF8, &CChildView::OnUpdateCharset)
	ON_UPDATE_COMMAND_UI(ID_CHARSET_UTF16, &CChildView::OnUpdateCharset)
	ON_UPDATE_COMMAND_UI(ID_CHARSET_SHIFTJIS, &CChildView::OnUpdateCharset)
	ON_UPDATE_COMMAND_UI(ID_CHARSET_JIS, &CChildView::OnUpdateCharset)
	ON_UPDATE_COMMAND_UI(ID_CHARSET_EUC, &CChildView::OnUpdateCharset)
	ON_UPDATE_COMMAND_UI(ID_CHARSET_LATIN, &CChildView::OnUpdateCharset)
	ON_COMMAND_EX(ID_LB_CR, &CChildView::OnLb)
	ON_COMMAND_EX(ID_LB_CRLF, &CChildView::OnLb)
	ON_COMMAND_EX(ID_LB_LF, &CChildView::OnLb)
	ON_UPDATE_COMMAND_UI(ID_LB_CR, &CChildView::OnUpdateLb)
	ON_UPDATE_COMMAND_UI(ID_LB_CRLF, &CChildView::OnUpdateLb)
	ON_UPDATE_COMMAND_UI(ID_LB_LF, &CChildView::OnUpdateLb)

	ON_COMMAND(ID_TOOL_OPTION, &CChildView::OnToolOption)
	ON_COMMAND(ID_JUMP_LINENO, &CChildView::OnJumpLineno)
	ON_COMMAND(ID_SEARCH_SEARCH, &CChildView::OnSearchSearch)
	ON_COMMAND(ID_SEARCH_REPLACE, &CChildView::OnSearchReplace)
	ON_COMMAND(ID_FILE_REOPEN, &CChildView::OnFileReopen)
	ON_UPDATE_COMMAND_UI(ID_FILE_REOPEN, &CChildView::OnUpdateFileReopen)
	ON_COMMAND_RANGE(IDM_MRU_START, IDM_MRU_END, &CChildView::OnMruOpen )
	ON_COMMAND(ID_TOOL_SEARCHOPTION, &CChildView::OnToolSearchoption)
	ON_WM_TIMER()
	ON_COMMAND(ID_VIEW_FONTDOWN, &CChildView::OnViewFontdown)
	ON_COMMAND(ID_VIEW_FONTUP, &CChildView::OnViewFontup)
	ON_COMMAND(ID_VIEW_WORDWRAP, &CChildView::OnViewWordwrap)
	ON_UPDATE_COMMAND_UI(ID_VIEW_WORDWRAP, &CChildView::OnUpdateViewWordwrap)
	ON_COMMAND(ID_PROP_AUTOSAVE, &CChildView::OnPropAutosave)
	ON_UPDATE_COMMAND_UI(ID_PROP_AUTOSAVE, &CChildView::OnUpdatePropAutosave)
	ON_COMMAND_RANGE(IDM_TEMPLATE_START, IDM_TEMPLATE_END, &CChildView::OnTemplate )
	ON_COMMAND(ID_TOOL_COMPATIBILITY, &CChildView::OnToolCompatibility)
	ON_COMMAND(ID_VIEW_RELOCATE, &CChildView::OnViewRelocate)
END_MESSAGE_MAP()



// CChildView メッセージ ハンドラ

BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs) 
{
	if (!CWnd::PreCreateWindow(cs))
		return FALSE;

	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
		::LoadCursor(NULL, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW+1), NULL);

	return TRUE;
}

void CChildView::OnPaint() 
{
	CPaintDC dc(this); // 描画のデバイス コンテキスト
	
	// TODO: ここにメッセージ ハンドラ コードを追加します。
	
	// メッセージの描画のために CWnd::OnPaint() を呼び出さないでください。
}

// ファイル読み出し
bool CChildView::OpenFile(const TCHAR *filename ,bool arg )
{
	CString text;

	CCountTime::printTime( _T("start openfile1") );
	// すでにロックされたファイルなら、先に使っているインスタンスをアクティブにする
	HWND hWnd = LockFile( GetParent()->m_hWnd , filename );
	if ( hWnd != NULL ){
		::SetForegroundWindow( hWnd );
		GetParent()->PostMessage(WM_CLOSE);
		return false;
	}

	CCountTime::printTime( _T("start openfile2") );
	LoadProperty(filename);

	CCountTime::printTime( _T("start openfile3") );
	// 自動判定のフラグが有効で
	if ( m_charset_autodetect ){
		//if ( m_property[2]==0 ){
		//	// ファイルプロパティの自動判定がONの時自動判定を行う
			m_charset = Txt::autoDetect( filename , (Txt::lbcode*)&m_lbcode );
		//}else{
		//	// ファイルプロパティで文字コード指定があるとき、自動判定をオフにして指定した文字コードを使う
		//	m_charset_autodetect = false;
		//	m_charset = m_property[2];
		//}
	}
	//// 文字コードを保存
	//if ( m_charset_autodetect ){
	//	m_property[2]=0;			// 自動保存
	//}else{
	//	m_property[2]=m_charset;	// 指定のあるとき
	//}
	CCountTime::printTime( _T("start openfile4") );

	TxtDecoder dec(m_charset);

	CCountTime::printTime( _T("start openfile5") );

	if ( dec.open( filename ) ){
		CReadBuff	rbuf;
		{
			CCountTime	log( _T("file read" ) );
			while( dec.state() != 0 ){
				const TCHAR *str;
				try {
					str = dec.readLine();
					rbuf.AddBuff( str );
					if ( dec.state() != 0 ){
						//text += _T("\r\n");
//						rbuf.AddBuff( _T("\r\n") );
						rbuf.AddBuff( _T("\r") );
					}
					if ( rbuf.IsError() ){
						throw;
					}
				}
				catch(...){
					dec.close();
					// メモリ不足エラー
					CString err;
					err.LoadString( IDS_OUT_OF_MEMORY );
					err += filename;
					CString errtitle;
					errtitle.LoadString( IDS_ERROR );
					MessageBox( err ,errtitle ,MB_OK );
					return false;
				}
			}
			dec.close();
		}

		m_Edit.SetModify(FALSE);

		{
			CCountTime	log( _T("set text" ) );
			m_Edit.SetMemory( rbuf.GetBuff() , rbuf.GetLength() );
		}

		int	sel=m_property[0];
		m_Edit.SetSel(sel,sel);
		m_mrulist.Add( filename , m_property );
		m_mrulist.Commit();

		GetParent()->PostMessage( WM_JOT_RESIZEMENU );

		m_filename = filename;
		m_validfilename = true;
		SetTitle( FALSE );

		return true;
	} else{
		// オープンできないときは、エラー

//		m_mrulist.Update();
		m_mrulist.Remove(filename);		// 履歴から削除
		m_mrulist.Commit();

		CString err;
		err.LoadString( IDS_CANTOPEN_ERR );
		err += filename;
		CString errtitle;
		errtitle.LoadString( IDS_ERROR );
		// 起動時オプションでなければエラーメッセージ表示
		if ( !arg ){
			MessageBox( err ,errtitle ,MB_OK );
		}else{
			AfxMessageBox( err , MB_OK );
		}
		return false;
	}
}

void	CChildView::Savefile( const TCHAR *filename )
{


	DWORD	start,end;

	start = GetTickCount();
	const TCHAR	*buff =  m_Edit.GetMemory();
	end = GetTickCount();

	CString	strRep;
	strRep.Format( _T("get  text = %d[ms]\n") , end-start );
	OutputDebugString( strRep );

	TxtEncoder enc(m_charset,m_lbcode);
	if ( enc.open( filename ) ){
		{
			CCountTime log( _T("save file" ) );

			start = GetTickCount();
		
			int	len = _tcslen(buff);

			int	pos = 0;
			int strlen = len;
			bool firstline=true;
			while( pos < strlen ){
				int	epos=pos;
				while( epos < strlen && buff[epos] != _T('\r') ){
					epos ++;
				}
				if ( epos < strlen ){
					enc.writeLine( buff+pos , epos - pos ,firstline );
					firstline = false;
					pos = epos +1;									// 次の検索位置に進める
				}else{
					// 最終行の処理
					enc.writeLine( buff+pos ,strlen - pos ,false ,true );
					break;
				}
			}
			enc.close();
		}

		m_Edit.SetModify(FALSE);

		int st,ed;
		m_Edit.GetSel(st,ed);

		LoadProperty(filename);
		m_property[0] = st;

		//// 文字コードを保存
		//if ( m_charset_autodetect ){
		//	m_property[2]=0;			// 自動保存
		//}else{
		//	m_property[2]=m_charset;	// 指定のあるとき
		//}
		m_mrulist.Add( filename , m_property );
		m_mrulist.Commit();

		GetParent()->PostMessage( WM_JOT_RESIZEMENU );

		m_filename = filename;					// 新ファイル名を保存
		m_validfilename = true;					// ファイル名を有効に
		SetTitle( FALSE );
	}


}

bool CChildView::SaveAs()
{
	bool ret = true;

	// SP(std)で使えるように、CFileDialogではなくGetOpenFileName()を使う
	OPENFILENAME ofn;
	TCHAR szFile[MAX_PATH] = _T("\0");
	if ( m_validfilename ){
		_tcscpy( szFile , m_filename );
	}else{
//		_tcscpy( szFile , _T(".txt") );
	}
	memset( &ofn , 0 , sizeof(ofn) );
	ofn.lStructSize = sizeof(ofn);
	ofn.hInstance = theApp.m_hInstance;
	ofn.hwndOwner = this->m_hWnd;
	ofn.lpstrFilter = GetFilter();	//_T("テキストファイル(*.txt)\0*.txt\0iniファイル(*.ini)\0*.ini\0全てのファイル(*.*)\0*.*\0\0");
	ofn.lpstrFile = szFile;
	ofn.Flags = OFN_EXPLORER | OFN_OVERWRITEPROMPT;
	ofn.nMaxFile = MAX_PATH;
	CString savefile;
	savefile.LoadString( IDS_SAVEFILE );
	ofn.lpstrTitle = savefile;
//	ofn.lpstrInitialDir = _T("\\");
//	ofn.lpstrDefExt = _T( ".txt" );

	UINT res = 	GetSaveFileNameEx(&ofn);

	if ( res == TRUE ){
		HWND hwnd = LockFile(NULL ,szFile) ;
		if ( hwnd == NULL || hwnd == GetParent()->m_hWnd ){
			// 使われていないか、自分自身の時OK
			UnlockFile();
			LockFile( GetParent()->m_hWnd ,szFile);
			Savefile( szFile );
		}else{
			// 他のインスタンスで開かれていたらエラー
			CString	msg1;
			msg1.LoadString( IDS_ALREADY_EDITING );
			CString	msg2;
			msg2.LoadString( IDS_ERROR );
			MessageBox( msg1,msg2,MB_OK );
			ret = false;
		}
	}else{
		ret =false;
	}
	return ret;
}


bool CChildView::SaveOverwrite()
{
	if ( m_validfilename ){
		// ファイル名有効時はそのまま上書き保存
		Savefile( m_filename );
		return true;
	}else{
		// ファイル名無効時はSaveAsに流す
		return SaveAs();
	}
}

void CChildView::NewFile()
{
	// 新規作成
	SetNewFileCharCode();
	m_filename = _T("");
	m_validfilename = false;
	m_Edit.SetModify(FALSE);
	m_Edit.SetWindowTextW( _T("") );
	m_Edit.SetSel(0,0);
	SetTitle( FALSE );
}

bool CChildView::ConfirmSave()
{
	bool ret = true;
	// エディターに改変があるか確認
	if ( m_Edit.GetModify() ){
		if ( m_property[1]==0 ){		// 自動保存がオフの時
			//  フレームを正面に出す
			Sleep(5);
			((CFrameWnd*)GetParent())->SetWindowPos( &wndTop , 0,0,0,0, SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW ); 
			((CFrameWnd*)GetParent())->ActivateFrame();
			Sleep(5);

			if (AfxGetAygshellUIModel() == Smartphone){
		//#ifdef WIN32_PLATFORM_WFSP
				// SPではYESNOCANCELが使えないので、質問を二回に分ける
				CString msg1;
				msg1.LoadString( IDS_QUES_SAVEFILE );
				CString msg2;
				msg2.LoadString( IDS_CONF_SAVE );

				int result = MessageBox(msg1,msg2 ,MB_YESNO|MB_ICONQUESTION  );
				
				switch ( result ){
				case IDYES:
					// ファイル保存
					ret = SaveOverwrite();
					break;
				case IDNO:
					{
						CString msg3;
						msg3.LoadString( IDS_CLOSEFILE );
						CString msg4;
						msg4.LoadString( IDS_CONFIRM );
						int res = MessageBox(msg3,msg4,MB_OKCANCEL|MB_ICONQUESTION  );
						switch ( res ){
						case IDOK:
							break;
						case IDCANCEL:
							ret = false;
							break;
						}
					}
					break;
				}
			}else{
		//#ifdef WIN32_PLATFORM_PSPC
				CString msg1;
				msg1.LoadString( IDS_QUES_SAVEFILE );
				CString msg2;
				msg2.LoadString( IDS_CONF_SAVE );

				int result = MessageBox(msg1 , msg2 ,MB_YESNOCANCEL|MB_ICONQUESTION  );
				
				switch ( result ){
				case IDYES:
					// ファイル保存
					ret = SaveOverwrite();
					break;
				case IDNO:
					break;
				case IDCANCEL:
					ret = false;
					break;
				}
			}
		}else{
			// 自動保存がONの時は何も聞かずに保存
			// ファイル保存
			ret = SaveOverwrite();
		}
	}
	if ( ret ){
		int st,ed;
		m_Edit.GetSel(st,ed);


		LoadProperty(m_filename);
		m_property[0] = st;
		//// 文字コードを保存
		//if ( m_charset_autodetect ){
		//	m_property[2]=0;			// 自動保存
		//}else{
		//	m_property[2]=m_charset;	// 指定のあるとき
		//}
		m_mrulist.Add( m_filename , m_property );
		m_mrulist.Commit();

		GetParent()->PostMessage( WM_JOT_RESIZEMENU );
	}
	return ret;
}


void CChildView::SetTitle( BOOL modify )
{
	// エディタから変更通知を受けたらタイトルを変更する
	CString title;
	if ( m_validfilename ){
		// ファイル名部分を抽出
		CFile	f(m_filename,CFile::modeRead);
		title = f.GetFileName();
		f.Close();

	}else{
		title = APPTITLE;
	}
	if ( modify != FALSE ){
		title +=_T("*");
	}
	GetParent()->SetWindowTextW(title);
}

void CChildView::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);
	Layout();
}

void CChildView::Layout()
{
	CRect rect;
	GetClientRect(&rect);

	int	toolbarheight = 0;

	if ( m_toolbar!=NULL ){
		toolbarheight = m_toolbar->height();
	}
	// エディタ部分計算
	CRect editrect = rect;
	editrect.bottom -= toolbarheight;
	m_Edit.MoveWindow( &editrect );

	// ツールバー部分
	if ( m_toolbar!=NULL ){
		CRect toolbarrect = rect;
		toolbarrect.top = editrect.bottom +1 ;
		m_toolbar->MoveWindow( toolbarrect );
		m_toolbar->Layout();

	}
}

int CChildView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	CCountTime::printTime(_T("childview start on create") );

	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	LoadSettings();
//	LoadMruList();
	CCountTime::printTime(_T("Load setiig") );

	// コマンドライン解析
	CString	filename;
	bool	opthscroll=false;
	Argv arg;
	int size = arg.size();
	if ( size > 0 ){
		for( int i=0;i<size;i++){
			CString p=arg[i] ;
			if ( p.GetLength() > 0 ){
				if ( p[0] == _T('-') ){
					p = p.MakeLower();
					if ( p == _T("-nowordwrap") ){
						opthscroll = true;
					}
				}else{
					if ( filename.GetLength()==0 ){
						filename = p;
					}
				}
			}
		}
	}
	CCountTime::printTime(_T("analyze cmd arg") );

	// ワードラップ無しの対応(起動時のみ)
	DWORD hscrollstyle=0;
	if ( m_hscroll || opthscroll ){
		// ワードラップ無しの時は以下のスタイルを追加する
		hscrollstyle = WS_HSCROLL|ES_AUTOHSCROLL;
	}

	// 初回起動時関連付けを設定するか確認する
	if ( m_firstboot ){
		theApp.SetExtAssoc( 1 );
		theApp.m_inifile.writeInt( INI_SEC , INI_KEY_FIRST	, 1 );
	}

	CCountTime::printTime(_T("start create edit box") );
	CRect rect;
	GetClientRect(&rect);
	m_Edit.Create( WS_VISIBLE|WS_CHILD|WS_VSCROLL |ES_MULTILINE|ES_AUTOVSCROLL|ES_NOHIDESEL|hscrollstyle , 
					rect ,this,1333);
	//m_Edit.ModifyStyle( 0 , WS_HSCROLL|ES_AUTOHSCROLL ,0 );

	//m_charset = AutoDetect;			// デフォルト文字コード=自動検出
	//m_charset = UTF8N;				// デフォルト文字コード
	//m_lbcode = Txt::CRLF;			// デフォルト改行コード

	CCountTime::printTime(_T("end create edit box") );
	NewFile();
	CCountTime::printTime(_T("New file") );

	if ( filename.GetLength()>0 ){
		if ( !OpenFile( filename , true ) ){
			GetParent()->PostMessage( WM_CLOSE );	
			return 0;
		}
	}
	CCountTime::printTime(_T("Open file") );
	ChangeFont();

	if (AfxGetAygshellUIModel() == PocketPC){
//#ifdef WIN32_PLATFORM_PSPC			// es/ades用のテンキーでのメニューアクセス機能
		m_Edit.SetXscrawl( m_xscrawl );
//#endif
	}
	m_Edit.SetDrawSymbol(m_drawsymbol);
	m_Edit.SetRegex( m_bRegex==TRUE );
	m_Edit.SetNoCase( m_bIgnoreCase==TRUE );

	m_Edit.SetColor( 0 ,m_ColorWindow );
	m_Edit.SetColor( 1 ,m_ColorWindowText );
	m_Edit.SetColor( 2 ,m_ColorHighlight );
	m_Edit.SetColor( 3 ,m_ColorHighlightText );
	m_Edit.SetColor( 4 ,m_ColorUnderline );

	CScrollDialog::SetFullSc( m_bFullSc==TRUE );

	m_Edit.SetSpacing( m_linespacing );
	m_Edit.SetUnderline( m_bUnderline );
	m_Edit.SetNotFocusIme( m_bNotFocusIme );
	m_Edit.SetBsWmChar( m_bBsWmChar );
	m_Edit.SetWordwrapPos( m_wordwrap_pos_p ,m_wordwrap_pos_l );
	m_Edit.SetDrawLineNumber( m_bDrawLineNumber );

	theApp.SetBackAsBs( m_bBackAsBs!=FALSE );

	CCountTime::printTime(_T("childview end on create") );
	return 0;
}
void CChildView::OnSetFocus(CWnd* pOldWnd)
{
	// ボトムバーが出てればそちらにフォーカスを与える
	if ( m_toolbar != NULL ){
		m_toolbar->SetFocus();
	}else{
		// エディタにフォーカスを与える
		m_Edit.SetFocus();
	}
}

BOOL CChildView::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// フォーカスのあるところにコマンドを送る
	CWnd *pWnd = GetFocus();
	if ( pWnd == &m_Edit ){
		// エディタにコマンドを処理する機会を与えます。
		if (m_Edit.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
			return TRUE;
	}else{
		// ボトムバーが出てればそちらにコマンドを送る
		if ( m_toolbar != NULL ){
			if (m_toolbar->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
				return TRUE;
		}
	}

	return CWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}


afx_msg LRESULT CChildView::OnTextChanged(WPARAM wp, LPARAM lp)
{
	SetTitle( wp );
	return 0;
}

void CChildView::OnFileOpen()
{
	TCHAR	inidir[MAX_PATH];

	if ( !ConfirmSave() )
		return;

	// SP(std)で使えるように、CFileDialogではなくGetOpenFileName()を使う
	OPENFILENAME ofn;
	TCHAR szFile[MAX_PATH] = _T("\0");
	memset( &ofn , 0 , sizeof(ofn) );
	ofn.lStructSize = sizeof(ofn);
	ofn.hInstance = theApp.m_hInstance;
	ofn.hwndOwner = this->m_hWnd;
	ofn.lpstrFilter = GetFilter();	//_T("テキストファイル(*.txt)\0*.txt\0iniファイル(*.ini)\0*.ini\0全てのファイル(*.*)\0*.*\0\0");
	ofn.lpstrFile = szFile;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST;
	ofn.nMaxFile = MAX_PATH;
	CString openfile;
	openfile.LoadString( IDS_OPENFILE );
	ofn.lpstrTitle = openfile;
	ofn.lpstrInitialDir = _T("\\");

	m_mrulist.Update();
	if ( m_mrulist.GetCount()>0 ){
		// 履歴のトップを取得
		CString	filename = m_mrulist.GetFilename(0);
		TCHAR	drv[10];
		TCHAR	dir[MAX_PATH];
		TCHAR	nam[MAX_PATH];
		TCHAR	ext[MAX_PATH];
		_tsplitpath_s( filename,drv,10,dir,MAX_PATH,nam,MAX_PATH,ext,MAX_PATH);

		_tmakepath_s( inidir , MAX_PATH ,drv,dir,NULL,NULL );
		ofn.lpstrInitialDir = inidir;
	}

	UINT res = 	GetOpenFileNameEx(&ofn);

	if ( res == TRUE ){
		UnlockFile();
		OpenFile( szFile  );
	}
}

void CChildView::OnFileSaveas()
{
	SaveAs();
}

void CChildView::OnFileSave()
{
	SaveOverwrite();
}

void CChildView::OnFileNew()
{
	if ( !ConfirmSave() )
		return;
	UnlockFile();
	NewFile();
}


void CChildView::OnCharsetAutodetect()
{
	m_charset_autodetect = !m_charset_autodetect;
}

void CChildView::OnUpdateCharsetAutodetect(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck( m_charset_autodetect?TRUE:FALSE );
}

BOOL CChildView::OnCharset(UINT nID)
{
	m_charset = id2charset(nID);
	// コード変更時自動判定を解除(フラグ有効時)
	if ( m_bDisAutoDetect ){
		m_charset_autodetect = FALSE;
	}
	return TRUE;
}

void CChildView::OnUpdateCharset(CCmdUI *pCmdUI)
{

	int charset = id2charset( pCmdUI->m_nID );
	pCmdUI->SetCheck( (charset == m_charset)?TRUE:FALSE );

}


BOOL CChildView::OnLb(UINT nID)
{
	m_lbcode = id2lb(nID);
	return TRUE;
}
void CChildView::OnUpdateLb(CCmdUI *pCmdUI)
{

	int lbcode = id2lb( pCmdUI->m_nID );
	pCmdUI->SetCheck( (lbcode == m_lbcode)?TRUE:FALSE );

}



void CChildView::OnToolOption()
{
	CSettingDlg	csd;

	csd.m_fontname = m_fontname;
	csd.m_fontsize = m_fontsize;
	csd.m_tabsize = m_tabsize;
//	csd.m_bAllFile = m_allfile;
	csd.m_bMenuTenkey = m_menutenkey;

	switch( m_xscrawl ){		// XCrawlのメニュー増やしたところの辻褄合わせ
	case 0:
		csd.m_xscrawl = m_xscrawl;
		break;
	case 1:	case 2:	case 3:	case 4:	case 5:
		csd.m_xscrawl = m_xscrawl+1;
		break;
	case 6:
		csd.m_xscrawl = m_xscrawl-5;
		break;
	}
	csd.m_bHScroll = m_hscroll;

	csd.m_bDrawLinebreak = (m_drawsymbol & DS_LINEBREAK)!=0?TRUE:FALSE;
	csd.m_bDrawTab		 = (m_drawsymbol & DS_TAB)!=0?TRUE:FALSE;
	csd.m_bDrawSpace	 = (m_drawsymbol & DS_SPACE)!=0?TRUE:FALSE;
	csd.m_bDrawWideSpace = (m_drawsymbol & DS_WIDESPACE)!=0?TRUE:FALSE;
	csd.m_mrunum  = m_mrunum;
	csd.m_bFullSc = m_bFullSc;
	csd.m_bBoldFont	= 	m_bBoldFont;
	csd.m_bDisAutoDetect	= 	m_bDisAutoDetect;
	csd.m_bUnderline  = m_bUnderline;
	csd.m_linespacing = m_linespacing;
	csd.m_NewFileCharCode  = m_NewFileCharCode;
	csd.m_NewFileLineBreak = m_NewFileLineBreak;

	csd.m_wordwrap_pos_p = m_wordwrap_pos_p;
	csd.m_wordwrap_pos_l = m_wordwrap_pos_l;
	csd.m_bDrawLineNumber = m_bDrawLineNumber;

	UINT result = csd.DoModal();
	if ( result == IDOK ){
		m_fontname = csd.m_fontname;
		m_fontsize = csd.m_fontsize;
		m_tabsize = csd.m_tabsize;
		//m_allfile = csd.m_bAllFile;
		m_menutenkey = csd.m_bMenuTenkey;
		switch( csd.m_xscrawl ){		// XCrawlのメニュー増やしたところの辻褄合わせ
		case 0:
			m_xscrawl  = csd.m_xscrawl;
			break;
		case 1:	
			m_xscrawl = csd.m_xscrawl+5;
			break;
		case 2: case 3:	case 4:	case 5: case 6:
			m_xscrawl = csd.m_xscrawl-1;
			break;
		}
		if ( m_hscroll  != csd.m_bHScroll ){
			m_Edit.SetWordwrap( csd.m_bHScroll==TRUE?FALSE:TRUE );
			m_hscroll = csd.m_bHScroll;
		}
		m_drawsymbol = 
			(csd.m_bDrawLinebreak==TRUE?DS_LINEBREAK:0) |
			(csd.m_bDrawTab==TRUE?DS_TAB:0) |
			(csd.m_bDrawSpace==TRUE?DS_SPACE:0) |
			(csd.m_bDrawWideSpace==TRUE?DS_WIDESPACE:0);
		m_mrunum  = csd.m_mrunum;
		m_bFullSc = csd.m_bFullSc;
		m_bBoldFont	= 	csd.m_bBoldFont;
		m_bDisAutoDetect= 	csd.m_bDisAutoDetect;
		m_bUnderline  = csd.m_bUnderline;
		m_linespacing = csd.m_linespacing;
		m_NewFileCharCode  = csd.m_NewFileCharCode;
		m_NewFileLineBreak = csd.m_NewFileLineBreak;
		m_wordwrap_pos_p = csd.m_wordwrap_pos_p;
		m_wordwrap_pos_l = csd.m_wordwrap_pos_l;
		m_bDrawLineNumber = csd.m_bDrawLineNumber;

		SaveSettings();

		ChangeFont();
		GetParent()->PostMessage( WM_JOT_RESIZEMENU , m_menutenkey );

		if (AfxGetAygshellUIModel() == PocketPC){
//#ifdef WIN32_PLATFORM_PSPC			// es/ades用のテンキーでのメニューアクセス機能
			m_Edit.SetXscrawl( m_xscrawl );
//#endif
		}
		m_Edit.SetDrawSymbol(m_drawsymbol);

		m_Edit.SetSpacing( m_linespacing );
		m_Edit.SetUnderline( m_bUnderline );
		m_Edit.SetWordwrapPos( m_wordwrap_pos_p ,m_wordwrap_pos_l );
		m_Edit.SetDrawLineNumber( m_bDrawLineNumber );
	}

}

void	CChildView::SaveSettings()
{
	theApp.m_inifile.writeString( INI_SEC , INI_KEY_FONTNAME , m_fontname );
	theApp.m_inifile.writeInt( INI_SEC , INI_KEY_FONTSIZE , m_fontsize );
	theApp.m_inifile.writeInt( INI_SEC , INI_KEY_TABSIZE , m_tabsize );
	//theApp.m_inifile.writeInt( INI_SEC , INI_KEY_ALLFILE , m_allfile );
	theApp.m_inifile.writeInt( INI_SEC , INI_KEY_MENUTENKEY , m_menutenkey );
	theApp.m_inifile.writeInt( INI_SEC , INI_KEY_XSCRAWL , m_xscrawl );
	theApp.m_inifile.writeInt( INI_SEC , INI_KEY_HSCROLL , m_hscroll );
	theApp.m_inifile.writeInt( INI_SEC , INI_KEY_DRAWSYMBOL , m_drawsymbol );
	theApp.m_inifile.writeInt( INI_SEC , INI_KEY_MRUNUM , m_mrunum );

	theApp.m_inifile.writeInt( INI_SEC , INI_KEY_REGEX , m_bRegex );
	theApp.m_inifile.writeInt( INI_SEC , INI_KEY_IGNORECASE , m_bIgnoreCase  );
	theApp.m_inifile.writeInt( INI_SEC , INI_KEY_FULLSC , m_bFullSc  );

	theApp.m_inifile.writeInt( INI_SEC , INI_KEY_BOLDFONT	 	, m_bBoldFont	  );
	theApp.m_inifile.writeInt( INI_SEC , INI_KEY_DISAUTODETECT	, m_bDisAutoDetect  );
	theApp.m_inifile.writeInt( INI_SEC , INI_KEY_UNDERLINE		, m_bUnderline   );
	theApp.m_inifile.writeInt( INI_SEC , INI_KEY_LINESPACING	, m_linespacing  );
	theApp.m_inifile.writeInt( INI_SEC , INI_KEY_NOTFOCUSIME	, m_bNotFocusIme  );
	theApp.m_inifile.writeInt( INI_SEC , INI_KEY_NEWFILECHARCODE	, m_NewFileCharCode  );
	theApp.m_inifile.writeInt( INI_SEC , INI_KEY_NEWFILELINEBREAK	, m_NewFileLineBreak  );
	theApp.m_inifile.writeInt( INI_SEC , INI_KEY_BSWMCHAR		, m_bBsWmChar  );
	theApp.m_inifile.writeInt( INI_SEC , INI_KEY_BACKASBS	, m_bBackAsBs  );

	theApp.m_inifile.writeInt( INI_SEC , INI_KEY_WORDWRAP_POS_P	, m_wordwrap_pos_p  );
	theApp.m_inifile.writeInt( INI_SEC , INI_KEY_WORDWRAP_POS_L	, m_wordwrap_pos_l  );
	theApp.m_inifile.writeInt( INI_SEC , INI_KEY_DRAWLINENUMBER	, m_bDrawLineNumber  );
}

void	CChildView::LoadSettings()
{
	m_fontname = theApp.m_inifile.readString( INI_SEC , INI_KEY_FONTNAME );
	m_fontsize = theApp.m_inifile.readInt( INI_SEC , INI_KEY_FONTSIZE , INI_DEF_FONTSIZE );
	m_tabsize = theApp.m_inifile.readInt( INI_SEC , INI_KEY_TABSIZE , INI_DEF_TABSIZE );
	//m_allfile = theApp.m_inifile.readInt( INI_SEC , INI_KEY_ALLFILE , INI_DEF_ALLFILE );
	m_menutenkey = theApp.m_inifile.readInt( INI_SEC , INI_KEY_MENUTENKEY , INI_DEF_MENUTENKEY );
	m_xscrawl = theApp.m_inifile.readInt( INI_SEC , INI_KEY_XSCRAWL , INI_DEF_XSCRAWL );
	m_hscroll = theApp.m_inifile.readInt( INI_SEC , INI_KEY_HSCROLL , INI_DEF_HSCROLL );
	m_drawsymbol = theApp.m_inifile.readInt( INI_SEC , INI_KEY_DRAWSYMBOL , INI_DEF_DRAWSYMBOL );
	m_mrunum = theApp.m_inifile.readInt( INI_SEC , INI_KEY_MRUNUM , INI_DEF_MRUNUM );

	m_bRegex = theApp.m_inifile.readInt( INI_SEC , INI_KEY_REGEX , INI_DEF_REGEX );
	m_bIgnoreCase = theApp.m_inifile.readInt( INI_SEC , INI_KEY_IGNORECASE , INI_DEF_IGNORECASE );

	m_bFullSc = theApp.m_inifile.readInt( INI_SEC , INI_KEY_FULLSC , INI_DEF_FULLSC );

	m_bBoldFont      = theApp.m_inifile.readInt( INI_SEC , INI_KEY_BOLDFONT	 , INI_DEF_BOLDFONT	 );
	m_bDisAutoDetect = theApp.m_inifile.readInt( INI_SEC , INI_KEY_DISAUTODETECT	 , INI_DEF_DISAUTODETECT );
	m_firstboot		 = theApp.m_inifile.readInt( INI_SEC , INI_KEY_FIRST	 , INI_DEF_FIRST )==0;

	m_ColorHighlight	= LoadColor( MISC_KEY_COLOR_HIGHLIGHT	  );
	m_ColorHighlightText= LoadColor( MISC_KEY_COLOR_HIGHLIGHTTEXT );
	m_ColorWindow		= LoadColor( MISC_KEY_COLOR_WINDOW	  );
	m_ColorWindowText	= LoadColor( MISC_KEY_COLOR_WINDOWTEXT  );
	m_ColorUnderline	= LoadColor( MISC_KEY_COLOR_UNDERLINE  );

	m_bUnderline		 = theApp.m_inifile.readInt( INI_SEC , INI_KEY_UNDERLINE	 , INI_DEF_UNDERLINE );
	m_linespacing		 = theApp.m_inifile.readInt( INI_SEC , INI_KEY_LINESPACING	 , INI_DEF_LINESPACING );
	m_bNotFocusIme		 = theApp.m_inifile.readInt( INI_SEC , INI_KEY_NOTFOCUSIME	 , INI_DEF_NOTFOCUSIME );
	m_NewFileCharCode	 = theApp.m_inifile.readInt( INI_SEC , INI_KEY_NEWFILECHARCODE	 , INI_DEF_NEWFILECHARCODE );
	m_NewFileLineBreak	 = theApp.m_inifile.readInt( INI_SEC , INI_KEY_NEWFILELINEBREAK	 , INI_DEF_NEWFILELINEBREAK );

	// BSをWM_CHARで処理するかのフラグ、PPCとSPでデフォルトを切り分ける
	m_bBsWmChar			 = theApp.m_inifile.readInt( INI_SEC , INI_KEY_BSWMCHAR		 , 
		(AfxGetAygshellUIModel() == Smartphone)?INI_DEF_BSWMCHAR_SP:INI_DEF_BSWMCHAR_PPC
	);
	// BACKキーをBSとして処理するかのフラグ、PPCとSPでデフォルトを切り分ける
	m_bBackAsBs		 = theApp.m_inifile.readInt( INI_SEC , INI_KEY_BACKASBS	 , 
		(AfxGetAygshellUIModel() == Smartphone)?INI_DEF_BACKASBS_SP:INI_DEF_BACKASBS_PPC
	);

	m_wordwrap_pos_p	 = theApp.m_inifile.readInt( INI_SEC , INI_KEY_WORDWRAP_POS_P	 , INI_DEF_WORDWRAP_POS_P );
	m_wordwrap_pos_l	 = theApp.m_inifile.readInt( INI_SEC , INI_KEY_WORDWRAP_POS_L	 , INI_DEF_WORDWRAP_POS_L );
	m_bDrawLineNumber	 = theApp.m_inifile.readInt( INI_SEC , INI_KEY_DRAWLINENUMBER	 , INI_DEF_DRAWLINENUMBER );

}


void	CChildView::ChangeFont()
{
	// フォント再設定
	DEL( m_pfont );
	if ( m_fontname.GetLength() != 0 ){
		m_pfont = new CFont();
		m_pfont->CreatePointFont( m_fontsize*10 , m_fontname );

		if ( m_bBoldFont ){
			LOGFONT	lf;
			m_pfont->GetLogFont( &lf );
			lf.lfWeight = FW_BOLD;
			DEL( m_pfont );
			m_pfont = new CFont();
			m_pfont->CreateFontIndirect( &lf );
		}
	}
	m_Edit.SetFont( m_pfont );

	// タブサイズ設定
	m_Edit.SetTabStops(m_tabsize*4);

}

const TCHAR*	CChildView::GetFilter()
{
	// リソースに\0が入らないので、ここで|を\0に変換する
	CString	filter;
//#ifdef WIN32_PLATFORM_WFSP
//	if ( m_allfile==TRUE ){
//		filter.LoadString( IDS_FILTER_SP );		// ALLFILEがONなら*.*
//	}else{
//		filter.LoadString( IDS_FILTER_PPC );	// そうでなければ*.txt
//	}
//#endif
//#ifdef WIN32_PLATFORM_PSPC
	filter.LoadString( IDS_FILTER_PPC );		// PPCではフィルタが選べるので設定は関係なし
//#endif
	_tcsncpy( m_filbuf , filter ,MAX_PATH-1 );
	for( TCHAR*p=m_filbuf; *p!=_T('\0');p++ ){
		if ( *p==_T('|') ){
			*p = _T('\0');
		}
	}
	return m_filbuf;
}

void CChildView::OnJumpLineno()
{
	DEL( m_toolbar );
	m_toolbar =new CJumpLineNo();

	m_toolbar->Create(this,this);
	Layout();
	m_toolbar->SetFocus();

}

void CChildView::NotifyBottomBar(UINT endcode,CString *p1 , CString *p2)
{
	bool	close=true;

	switch( endcode ){
	case ID_JUMP:
		{
			int lineno = _ttoi( *p1 );
			m_Edit.JumpLineNo(lineno);
		}
		break;
	case ID_FIND:
		m_mrufind.Commit();
		m_Edit.FindString( *p1 , true , m_findfirst );
		m_findfirst = false;
		close = false;
		break;
	case ID_FIND_UP:
		m_mrufind.Commit();
		m_Edit.FindString( *p1 , false , m_findfirst );
		m_findfirst = false;
		close = false;
		break;
	case ID_REPLACE:
		m_mrufind.Commit();
		m_mrurepl.Commit();
		m_Edit.Replace( *p1 ,*p2, true );
		close = false;
		break;
	case ID_REPLACE_ALL:
		m_mrufind.Commit();
		m_mrurepl.Commit();
		m_Edit.ReplaceAll( *p1 ,*p2 );
		close = true;
		break;
	case ID_JUMP_CANCEL:
	case ID_FIND_CANCEL:

		break;
	}

	if ( close ){
		// ボトムバーを閉じる
		m_toolbar->DestroyWindow();
		DEL( m_toolbar );
		Layout();
		m_Edit.SetFocus();
	}
}


void CChildView::OnSearchSearch()
{
	DEL( m_toolbar );
	
	// 選択文字列の取得
	CString	inistr = m_Edit.GetFindInistr();

	m_mrufind.Update();
	m_toolbar =new CFindBar(inistr , &m_mrufind);
	m_findfirst =true;

	m_toolbar->Create(this,this);
	Layout();
	m_toolbar->SetFocus();
}

void CChildView::OnSearchReplace()
{
	DEL( m_toolbar );

	// 選択文字列の取得
	CString	inistr = m_Edit.GetFindInistr();

	m_mrufind.Update();
	m_mrurepl.Update();
	m_toolbar =new CReplaceBar(inistr , &m_mrufind,&m_mrurepl );
	m_findfirst =true;

	m_toolbar->Create(this,this);
	Layout();
	m_toolbar->SetFocus();
}

void CChildView::OnFileReopen()
{
	if ( m_filename.GetLength()> 0 ){
		if ( !ConfirmSave() )
			return;
		UnlockFile();
		OpenFile( m_filename );
	}
}

void CChildView::OnUpdateFileReopen(CCmdUI *pCmdUI)
{
	pCmdUI->Enable( m_filename.GetLength()>0?TRUE:FALSE );

}

HWND	CChildView::LockFile(HWND hWnd,CString fname)
{
	HWND ret ;
	// ファイルマッピング用のキーを作成
	CString mapname = _T("//jot/lock/");
	mapname += fname;
	mapname.Replace( _T('\\') , _T('/') );

	HANDLE hMap = CreateFileMapping((HANDLE)INVALID_HANDLE_VALUE,NULL,PAGE_READWRITE|SEC_COMMIT,0,sizeof(HWND),mapname );
	if ( hMap != NULL ){
		HWND *buff = (HWND*)MapViewOfFile( hMap , FILE_MAP_WRITE , 0 , 0 , 0 );
		ret = *buff;
		if ( *buff == NULL ){
			*buff = hWnd;
		}
		UnmapViewOfFile( buff );
		if ( ret != NULL || hWnd==NULL ){
			CloseHandle( hMap );
		}else{
			m_hLock = hMap;
		}
	}
	return ret;
}

void	CChildView::UnlockFile()
{
	if ( m_hLock != NULL ){
		CloseHandle( m_hLock );
		m_hLock = NULL;
	}
}

//void	CChildView::SaveMruList()
//{
//	int	cnt = m_mrulist.GetCount();
//	for( int i=0;i<cnt+1;i++ ){
//		CString	fname;
//		fname.Format( _T("%s%04d") , INI_KEY_FNAME , i );
//		CString pos;
//		pos.Format( _T("%s%04d") , INI_KEY_POS , i );
//
//		if ( i<cnt ){
//			theApp.m_inifile.writeString( INI_SEC_MRU , fname , m_mrulist.GetFilename(i) );
//			theApp.m_inifile.writeInt( INI_SEC_MRU , pos , m_mrulist.GetPos(i) );
//		}else{
//			// ターミネータ
//			theApp.m_inifile.writeString( INI_SEC_MRU , fname , _T("*") );
//			theApp.m_inifile.writeInt( INI_SEC_MRU , pos , 0 );
//		}
//	}
//
//	cnt = m_mrufind.GetCount();
//	for( int i=0;i<cnt+1;i++ ){
//		CString	fname;
//		fname.Format( _T("%s%04d") , INI_KEY_MRUFIND , i );
//
//		if ( i<cnt ){
//			theApp.m_inifile.writeString( INI_SEC_MRUFIND , fname , m_mrufind.Getstr(i) );
//		}else{
//			// ターミネータ
//			theApp.m_inifile.writeString( INI_SEC_MRUFIND , fname , _T("*") );
//		}
//	}
//
//	cnt = m_mrurepl.GetCount();
//	for( int i=0;i<cnt+1;i++ ){
//		CString	fname;
//		fname.Format( _T("%s%04d") , INI_KEY_MRUREPL , i );
//
//		if ( i<cnt ){
//			theApp.m_inifile.writeString( INI_SEC_MRUREPL , fname , m_mrurepl.Getstr(i) );
//		}else{
//			// ターミネータ
//			theApp.m_inifile.writeString( INI_SEC_MRUREPL , fname , _T("*") );
//		}
//	}
//
//}
//
//void	CChildView::LoadMruList()
//{
//	int	cnt = m_mrulist.GetMax();
//
//	for( int i=0;i<cnt;i++ ){
//		CString	fname;
//		fname.Format( _T("%s%04d") , INI_KEY_FNAME , i );
//		CString pos;
//		pos.Format( _T("%s%04d") , INI_KEY_POS , i );
//
//		CString	strfn = theApp.m_inifile.readString( INI_SEC_MRU , fname );
//		int		npos  = theApp.m_inifile.readInt( INI_SEC_MRU , pos , 0 );
//
//		if ( *strfn == _T('*') ){
//			break;
//		}
//		m_mrulist.AddTail( strfn , npos );
//	}
//
//	cnt = m_mrufind.GetMax();
//
//	for( int i=0;i<cnt;i++ ){
//		CString	fname;
//		fname.Format( _T("%s%04d") , INI_KEY_MRUFIND , i );
//
//		CString	strfn = theApp.m_inifile.readString( INI_SEC_MRUFIND , fname );
//
//		if ( *strfn == _T('*') ){
//			break;
//		}
//		m_mrufind.AddTail( strfn );
//	}
//
//	cnt = m_mrurepl.GetMax();
//
//	for( int i=0;i<cnt;i++ ){
//		CString	fname;
//		fname.Format( _T("%s%04d") , INI_KEY_MRUREPL , i );
//
//		CString	strfn = theApp.m_inifile.readString( INI_SEC_MRUREPL , fname );
//
//		if ( *strfn == _T('*') ){
//			break;
//		}
//		m_mrurepl.AddTail( strfn );
//	}
//
//	GetParent()->PostMessage( WM_JOT_RESIZEMENU );
//}

void CChildView::OnMruOpen(UINT nID)
{
	int	i = nID-IDM_MRU_START;

	m_mrulist.Update();
	if ( i<m_mrulist.GetCount() ){
		CString	filename = m_mrulist.GetFilename(i);

		if ( filename.GetLength()> 0 ){
			if ( !ConfirmSave() )
				return;
			UnlockFile();
			OpenFile( filename );
		}
	}
}


void CChildView::SetMruMenu(int menuno , HMENU hmenu)
{
	//if ( menuno== 0 ){
	// 左ソフトキー
	CMenu	*pMenu0 = CMenu::FromHandle(hmenu);

	m_mrulist.Update();
	for( UINT j=0;j<pMenu0->GetMenuItemCount();j++ ){
		if ( pMenu0->GetMenuItemID(j) == ID_FILE_REOPEN ){	// 開き直すの次が履歴
			CMenu	*pMenu2 = pMenu0->GetSubMenu(j+1);	
			//// 既存のメニューを全部削除
			int cnt = pMenu2->GetMenuItemCount();
			for( int i=0;i<cnt;i++ ){
				pMenu2->DeleteMenu(0,MF_BYPOSITION );
			}
			// 項目追加
			int	max = m_mrulist.GetCount();
			int i;
			for( i=0;i<max && i<m_mrunum ;i++){
				CString name = m_mrulist.GetFilename(i);
				int lastbs = name.ReverseFind( _T('\\') );
				name = name.Mid( lastbs+1 );
				pMenu2->AppendMenu( MF_STRING  , IDM_MRU_START+i , name  );
			}
			if ( i==0 ){
				pMenu2->AppendMenu( MF_STRING, IDM_MRU_START-1 , _T("(なし)")  );
			}
		}

		if ( pMenu0->GetMenuItemID(j) == ID_TEMPLATE ){		// テンプレートのプレースホルダの次
			CMenu	*pMenu2 = pMenu0->GetSubMenu(j+1);	
			//// 既存のメニューを全部削除
			int cnt = pMenu2->GetMenuItemCount();
			for( int i=0;i<cnt;i++ ){
				pMenu2->DeleteMenu(0,MF_BYPOSITION );
			}
			// 項目追加
			int	max = m_template.GetCount();
			int i;
			for( i=0;i<max ;i++){
				CString name = m_template.GetName(i) ;
				pMenu2->AppendMenu( MF_STRING  , IDM_TEMPLATE_START+i , name  );
			}
			if ( i==0 ){
				pMenu2->AppendMenu( MF_STRING, IDM_MRU_START-1 , _T("(なし)")  );
			}
		}
	}

	//}else if ( menuno==1 ){
	//	// 右ソフトキー
	//	//CMenu	*pRMenu0 = CMenu::FromHandle(hmenu);
	//	//CMenu	*pRMenu1 = pRMenu0/*->GetSubMenu(1)*/;


	//}

}

afx_msg LRESULT CChildView::OnNotifyFocus(WPARAM wp, LPARAM lp)
{
	HWND sender = (HWND)wp;
	// エディタ側にフォーカスが移ったらツールバー閉じる
	if ( sender == m_Edit.m_hWnd ){
		if ( m_toolbar != NULL ){
			m_toolbar->PostMessage( WM_COMMAND ,ID_BOTTOMBAR_CANCEL );
		}
	}
	return 0;
}



void CChildView::OnToolSearchoption()
{
	CSearchSettingDlg	csd;

	csd.m_bRegex = m_bRegex;
	csd.m_bIgnoreCase = m_bIgnoreCase;

	if ( csd.DoModal() == IDOK ){
		m_bRegex = csd.m_bRegex;
		m_bIgnoreCase = csd.m_bIgnoreCase;
		m_Edit.SetRegex( m_bRegex==TRUE );
		m_Edit.SetNoCase( m_bIgnoreCase==TRUE );
		SaveSettings();
	}
}

void CChildView::AutoSave(bool mode)
{
	if ( m_property[1]!=0 && mode ){
		// 5秒後にタイマーをセット
		SetTimer( 1 , 5000 , NULL );
	}else{
		// タイマー解除
		KillTimer( 1 );
	}
}

void CChildView::OnTimer(UINT_PTR nIDEvent)
{
	if ( nIDEvent == 1 ){
		// 自動保存
		
		// ファイル名が有効で変更が入っていれば
		if ( m_validfilename && m_Edit.GetModify() ){
			// 上書き保存
			SaveOverwrite();
		}
		KillTimer( nIDEvent );
	}
	__super::OnTimer(nIDEvent);
}

void CChildView::OnViewFontdown()
{
	if ( m_fontsize> 4){
		m_fontsize--;
		ChangeFont();
	}
}

void CChildView::OnViewFontup()
{
	if ( m_fontsize<96){
		m_fontsize++;
		ChangeFont();
	}
}

void CChildView::OnViewWordwrap()
{
	m_hscroll = (m_hscroll==TRUE)?FALSE:TRUE;
	m_Edit.SetWordwrap( m_hscroll==TRUE?FALSE:TRUE );
}

void CChildView::OnUpdateViewWordwrap(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck( m_hscroll );
}

void CChildView::OnPropAutosave()
{
	m_property[1] = m_property[1]==0?1:0;

#ifdef _DEBUG
	if ( m_property[1]==1 ){
		for( int i=3;i<16;i++ ){
			m_property[i] = i;
		}
	}
#endif
	m_mrulist.Add( m_filename , m_property );
	m_mrulist.Commit();
}

void CChildView::OnUpdatePropAutosave(CCmdUI *pCmdUI)
{
	pCmdUI->Enable( m_validfilename  );
	pCmdUI->SetCheck( m_property[1]!=0?TRUE:FALSE );
}

void	CChildView::LoadProperty(const TCHAR *filename)
{
	m_mrulist.Update();
	int	*ld = m_mrulist.GetData( filename );
	memcpy( m_property ,ld , sizeof(int)*16 );
}

void CChildView::OnTemplate(UINT nID)
{
	int	i = nID-IDM_TEMPLATE_START;

	// 選択位置取得
	int	st,ed;
	m_Edit.GetSel( st ,ed );

	// 選択文字列の取得
	CString	inistr = m_Edit.GetSelectedStr();

	// テンプレートの文字列取得
	int	newcar;
	CString	newstr = m_template.ProcTemplate( i , inistr , newcar );

	// 置換
	m_Edit.ReplaceSel( newstr ,TRUE );

	// 新しいカーソル位置を設定
	m_Edit.SetSel( st+newcar ,st+newcar );

}

void CChildView::OnToolCompatibility()
{
	CCompatiDlg	ccd;

	ccd.m_bNotFocusIme	= 	m_bNotFocusIme;

	ccd.m_bBsWmChar		=	m_bBsWmChar;
	ccd.m_bBackAsBs	=	m_bBackAsBs;

	UINT result = ccd.DoModal();
	if ( result == IDOK ){
		m_bNotFocusIme	= ccd.m_bNotFocusIme;
		m_bBsWmChar		= ccd.m_bBsWmChar;
		m_bBackAsBs	= ccd.m_bBackAsBs;

		SaveSettings();

		m_Edit.SetNotFocusIme( m_bNotFocusIme );
		m_Edit.SetBsWmChar( m_bBsWmChar );
		theApp.SetBackAsBs( m_bBackAsBs!=FALSE );
	}

}

void	CChildView::SetNewFileCharCode()
{
	m_charset = s_charset_id[m_NewFileCharCode].charset;	// デフォルト文字コード
	m_lbcode = s_lb_id[m_NewFileLineBreak].charset;			// デフォルト改行コード

}


void CChildView::OnViewRelocate()
{
	m_Edit.Relocate();
}
