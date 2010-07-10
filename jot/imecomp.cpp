//
// jot - Text Editor for Windows Mobile
//   Copyright (C) 2007-2008, Aquamarine Networks.  <http://pandora.sblo.jp/>
//
//    This program is EveryOne's ware; you can redistribute it and/or modify it 
//    under the terms of the NYSL version 0.9982 or later. <http://www.kmonos.net/nysl/>
//

#include "stdafx.h"

#define _INC_OLE
#include <windows.h>
#undef  _INC_OLE
#include <tchar.h>

#include "imecomp.h"

#define BUFFERSIZE				64

typedef struct {
	// 保持している内容
	BYTE	m_bComAttr[BUFFERSIZE+1];	// 複合文字列の属性
	DWORD	m_dwCompCls[BUFFERSIZE+1];	// コンポジション
	TCHAR	m_szComStr[BUFFERSIZE+1];	// 複合文字列のバッファ
	int		m_nComSize;					// 複合文字列サイズ
	int		m_nComCursorPos;			// 複合文字列における現在のカーソル位置
	DWORD	m_nComAttrSize;				// 複合文字列の属性のサイズ
	POINT	m_CandPos;					// 候補窓の場所
	int		m_nCompClsSize;				// 複合文字列の文節数
	int		m_nPlatform;				// PPC/Smartphoneモード// 0:PPC 1:Smartphone
	POINT	m_CaretPos;					// キャレット位置
	DWORD	m_nColor[32];				// カラーコード
	int		m_spacing;					// 行間設定
	HFONT	m_hFont;
}	COMPBUF;


/*
 * DrawChar - 文字列の描画および描画範囲計算
 */
static	void	DrawChar( HDC hdc , TCHAR ch , RECT *prect , bool bDraw=true ) 
{
	UINT flags = DT_NOPREFIX|DT_LEFT|DT_TOP|DT_SINGLELINE;
	if ( !bDraw ){
		flags |= DT_CALCRECT;
	}
	DrawText(hdc, &ch , 1 , prect , flags );
}
/*
 * DrawCompString - 変換文字列の描画(折り返し)
 */
static	bool	DrawCompString(HDC hdc,TCHAR *str,int len,POINT *pt,BYTE atr,
						POINT *cand,int pf,RECT* prect,bool bdraw ,DWORD color[],int spacing,
						SIZE *sz=NULL , HRGN rgn = NULL)
{
	HPEN	hPen;
	bool	reverse = false;
	bool	ret = false;

	int	width=0;

	HGDIOBJ	pOldPen ;
	HBRUSH	hBrush;

	if ( bdraw ){
		switch(atr){
		case 0x00:
			// 入力文字の場合
			hPen = ::CreatePen( PS_DASH, 1, color[1]);
			break;
		case 0x01:
			// 編集可能な語句の場合
			if ( pf == 0 ){		// PPCの時
				// 破線＋ハイライト
				hPen = ::CreatePen( PS_DASH, 1, color[1]);
				reverse = true;
			}else{				// Smartphoneの時
				// 下線太目　ハイライト無し
				hPen = ::CreatePen( PS_SOLID, 2, color[1]);
				reverse = false;
			}
			break;
		default:
			// 変換された文字の場合
			hPen = ::CreatePen( PS_SOLID, 1, color[1]);
			break;
		}
		if ( reverse ){
			// 反転
			SetTextColor(hdc, color[3]);
			SetBkColor(hdc, color[2]);
			hBrush = ::CreateSolidBrush( color[2] );
		}else{
			// 通常
			SetTextColor(hdc, color[1]);
			SetBkColor(hdc, color[0]);
			hBrush = ::CreateSolidBrush( color[0] );
		}
		pOldPen = SelectObject(hdc , hPen);
	}

	RECT	drect;
	drect.left = pt->x;
	drect.top  = pt->y;
	int nXStart = drect.left;
	int nXEnd = nXStart;
	if ( len == -1 ){
		len = _tcslen( str );
	}

	for( int i=0;i<len;i++ ){

		// i文字目の描画領域を取得
		DrawChar( hdc , str[i]  , &drect , false);

		// 画面右端を越えるようであれば
		if ( drect.right > prect->right ){
			if ( bdraw ){
				// 描画済みの部分に下線を引く
				MoveToEx(hdc , nXStart + 1, drect.bottom+spacing-1 , NULL);
				LineTo(hdc , nXEnd - 1, drect.bottom+spacing-1);
			}
			//int	w = drect.right - drect.left;
			//int h = drect.bottom - drect.top + spacing;
			// 描画位置をリセット
			drect.top  = drect.bottom + spacing;
			drect.left = prect->left;
			// 描画領域を再取得
			DrawChar( hdc , str[i] , &drect ,  false);

			nXStart = drect.left;

			if ( drect.bottom+spacing > prect->bottom ){
				ret = true;
			}
		}
		RECT	srect = drect;
		srect.top += spacing/2;
		srect.bottom += spacing/2;
		srect.bottom -= 1;
		drect.bottom += spacing;
		if ( bdraw ){
			FillRect( hdc , &drect , hBrush );
			DrawChar( hdc , str[i] , &srect , true );
		}
		if ( rgn != NULL ){
			HRGN hr = CreateRectRgn( drect.left,drect.top,drect.right,drect.bottom );
			CombineRgn( rgn ,rgn ,hr ,RGN_OR );
			DeleteObject( hr );
		}
		width += drect.right - drect.left;
		nXEnd = drect.right;
		drect.left = drect.right;
		if ( i==0 ){		// 一文字目で候補窓の位置をチェック
			if ( atr == 0x01 ){
				// 編集可能な語句の場合
				cand->x = nXStart;
				cand->y = drect.bottom+1;
			}
		}
	}
	if ( bdraw ){
		// 下線を引く
		MoveToEx(hdc , nXStart + 1, drect.bottom-1 , NULL);
		LineTo(hdc , nXEnd - 1, drect.bottom-1);
		SelectObject(hdc , pOldPen);
		DeleteObject( hPen );
		DeleteObject( hBrush );
	}

	pt->x = drect.right;
	pt->y = drect.top;
	if ( sz !=NULL ){
		sz->cx = width;
		sz->cy = drect.bottom - drect.top;
	}
	return ret;
}

/*
 * GetCompRgn - 描画領域のリージョンを求める
 */
static	HRGN	GetCompRgn( HWND hWnd , COMPBUF *bf )
{
	RECT rect;
	HWND hParentWnd = GetParent( hWnd );
	GetClientRect( hParentWnd , & rect );
	bool retry = false;

	HDC	hdc = GetDC(hWnd);

	HGDIOBJ hOldFont = SelectObject( hdc , bf->m_hFont );
retry:
	HRGN	rgn = CreateRectRgn(0,0,0,0);
	POINT	pt = bf->m_CaretPos;
	SIZE	sz;
	bool ret = DrawCompString(hdc, bf->m_szComStr ,-1 , &pt ,
		0 , &bf->m_CandPos , bf->m_nPlatform	,&rect,false,bf->m_nColor,bf->m_spacing,&sz,rgn);

	// 下にはみ出たら
	if ( ret ){
		if ( !retry ){
			// 左に逃げるようにして再計算
			bf->m_CaretPos.x = rect.right - sz.cx;
			if ( bf->m_CaretPos.x<rect.left ){
				bf->m_CaretPos.x=rect.left;
			}
			DeleteObject( rgn );		
			retry = true;
			goto retry;
		}

	}
	SelectObject( hdc , hOldFont );
	ReleaseDC(hWnd, hdc);

	return rgn;
}


/*
 * imecomp_proc - IME コンポジションウィンドウプロシージャ
 */
static LRESULT CALLBACK imecomp_proc(const HWND hWnd, const UINT msg, const WPARAM wParam, const LPARAM lParam)
{
	COMPBUF *bf;

	switch (msg) {
	case WM_CREATE:
		if ((bf = new COMPBUF) == NULL) {
			return -1;
		}
		// buffer info to window long
		SetWindowLong(hWnd, GWL_USERDATA, (LPARAM)bf);

		// バッファの初期化
		ZeroMemory( &bf->m_bComAttr , sizeof(BYTE)*(BUFFERSIZE+1) );
		ZeroMemory( &bf->m_dwCompCls ,sizeof(DWORD)*(BUFFERSIZE+1) );
		ZeroMemory( &bf->m_szComStr , sizeof(TCHAR)*(BUFFERSIZE+1) );

		bf->m_hFont = NULL;

		// カラーコード初期化
		bf->m_nColor[0] = GetSysColor( COLOR_WINDOW );
		bf->m_nColor[1] = GetSysColor( COLOR_WINDOWTEXT );
		bf->m_nColor[2] = GetSysColor( COLOR_HIGHLIGHT );
		bf->m_nColor[3] = GetSysColor( COLOR_HIGHLIGHTTEXT );

		// 行間初期化
		bf->m_spacing = 0;
		break;

	case WM_DESTROY:
		if ((bf = (COMPBUF *)GetWindowLong(hWnd, GWL_USERDATA)) != NULL) {
			SetWindowLong(hWnd, GWL_USERDATA, (LPARAM)0);

			DEL( bf );
		}
		return DefWindowProc(hWnd, msg, wParam, lParam);

	case WM_PAINT:
		if ((bf = (COMPBUF *)GetWindowLong(hWnd, GWL_USERDATA)) != NULL) {
//			CCountTime( _T("wmpaint") );
			RECT rect;
			GetClientRect( hWnd , &rect );

			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			HGDIOBJ hOldFont = SelectObject( hdc , bf->m_hFont );
			if ( bf->m_nComSize > 0){

				if ( bf->m_nCompClsSize == 0 ){
					// 文節無し=全文字を表示
					POINT	pt = bf->m_CaretPos;
					DrawCompString(hdc,&bf->m_szComStr[0] ,-1 ,&pt ,
						bf->m_bComAttr[0] ,&bf->m_CandPos , bf->m_nPlatform	,&rect,true,bf->m_nColor,bf->m_spacing);

				}else{
					// 変換文節の下線の表示
					int	nXStart = 0;
					POINT	pt = bf->m_CaretPos;

					for (int i = 0;i+1<bf->m_nCompClsSize ; i++){
						int ClsLen = -1;
						int CurrCls = bf->m_dwCompCls[i];
						if ( i+1 < bf->m_nCompClsSize ){
							ClsLen  = bf->m_dwCompCls[i+1] - CurrCls ;
						}

						DrawCompString(hdc,&bf->m_szComStr[CurrCls] ,ClsLen ,&pt ,
							bf->m_bComAttr[CurrCls] ,&bf->m_CandPos , bf->m_nPlatform	,&rect,true,bf->m_nColor,bf->m_spacing);


					}
				}
				// カーソル位置の表示
				{
					int cursor = bf->m_nComCursorPos;

					if ( cursor > 0 ){
						POINT	pt = bf->m_CaretPos;
						SIZE	sz;
						DrawCompString(hdc,&bf->m_szComStr[0] ,cursor , &pt ,
							0 , &bf->m_CandPos , bf->m_nPlatform	,&rect,false,bf->m_nColor,bf->m_spacing,&sz);

						HPEN	hPen;
						hPen = ::CreatePen( PS_SOLID, 1, bf->m_nColor[1]);

						HGDIOBJ	pOldPen = SelectObject(hdc , hPen);
						MoveToEx(hdc , pt.x-1 , pt.y, NULL);
						LineTo(hdc , pt.x-1, pt.y+sz.cy );
						SelectObject(hdc , pOldPen);
						DeleteObject( hPen );
					}
				}
			}
			SelectObject( hdc , hOldFont );
			EndPaint(hWnd, &ps);
		}
		break;



	case WM_IMECOMP_COPMSTR:
		// コンポジションイベント
		if ((bf = (COMPBUF *)GetWindowLong(hWnd, GWL_USERDATA)) != NULL ) {
			if (lParam & GCS_COMPSTR) {
				HIMC	hIMC = (HIMC)(wParam);
				RECT rect;
				// ウィンドウサイズを０で初期化
				rect.top = rect.left = rect.right = rect.bottom = 0;

				// 複合文字列を取得します
				bf->m_nComSize = ImmGetCompositionString(hIMC, GCS_COMPSTR, (LPVOID)bf->m_szComStr, sizeof(TCHAR) * (BUFFERSIZE));

				// データありかつエラーがなし、ならばデータ取得
				if (  bf->m_nComSize != IMM_ERROR_NODATA && bf->m_nComSize != IMM_ERROR_GENERAL){

					bf->m_szComStr[bf->m_nComSize/2]=_T('\0');

					// 複合文字列内での複合属性およびカーソル位置を取得します
					if (lParam & GCS_COMPATTR)
						bf->m_nComAttrSize = ImmGetCompositionString(hIMC, GCS_COMPATTR, bf->m_bComAttr, sizeof(bf->m_bComAttr));

					if (lParam & GCS_CURSORPOS)
						bf->m_nComCursorPos = ImmGetCompositionString(hIMC, GCS_CURSORPOS, NULL, 0);

					if (lParam & GCS_COMPCLAUSE)
						bf->m_nCompClsSize = ImmGetCompositionString(hIMC, GCS_COMPCLAUSE, bf->m_dwCompCls,sizeof(bf->m_dwCompCls)) / sizeof(DWORD);

					HRGN	rgn = GetCompRgn( hWnd , bf );
					SetWindowRgn( hWnd , rgn , true );
				}
				InvalidateRect( hWnd , NULL , FALSE );
			}
		}
		break;
	case WM_IMECOMP_ENDCOMP:
		if ((bf = (COMPBUF *)GetWindowLong(hWnd, GWL_USERDATA)) != NULL ) {
			bf->m_szComStr[0] = _T('\0');
			bf->m_nComSize = 0;
			SetWindowPos( hWnd , NULL , 0,0, 0,0, SWP_NOMOVE|SWP_NOZORDER );
			return TRUE;
		}
		break;

	case WM_IMECOMP_GETCANDPOS:
		if ((bf = (COMPBUF *)GetWindowLong(hWnd, GWL_USERDATA)) != NULL ) {
			return MAKELONG( bf->m_CandPos.x ,bf->m_CandPos.y  );
		}
		break;
	case WM_IMECOMP_SETCOMPPOS:
		if ((bf = (COMPBUF *)GetWindowLong(hWnd, GWL_USERDATA)) != NULL ) {
			// 親ウインドウの場所
			HWND hParentWnd = GetParent( hWnd );
			RECT ParentRect;
			GetClientRect( hParentWnd , & ParentRect );

			GetCaretPos(&bf->m_CaretPos);
			
			int w=0;
			int h=0;
			w = ParentRect.right-ParentRect.left;
			h = ParentRect.bottom- ParentRect.top + 32;
			MoveWindow( hWnd ,  ParentRect.left ,ParentRect.top ,w,h ,TRUE );
			HRGN	rgn = GetCompRgn( hWnd , bf );
			SetWindowRgn( hWnd , rgn , true );

			return 0;
		}
		break;

	case WM_SETFONT:
		// フォントを設定
		if ((bf = (COMPBUF *)GetWindowLong(hWnd, GWL_USERDATA)) != NULL) {
			bf->m_hFont = (HFONT) wParam;
		}
		break;

	case WM_SETPLATFORM:
		// プラットフォームの設定
		if ((bf = (COMPBUF *)GetWindowLong(hWnd, GWL_USERDATA)) != NULL ) {
			bf->m_nPlatform = wParam;
		}
		return 0;
	case WM_IMECOMP_SETCOLOR:
		// カラーコード設定
		if ((bf = (COMPBUF *)GetWindowLong(hWnd, GWL_USERDATA)) != NULL ) {
			if ( wParam < 4 ){
				bf->m_nColor[wParam] = lParam;
			}
		}
		return 0;
	case WM_SETSPACING:
		// 行間設定
		if ((bf = (COMPBUF *)GetWindowLong(hWnd, GWL_USERDATA)) != NULL ) {
			bf->m_spacing = wParam;
		}
		return 0;

	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

/*
 * imecomp_regist - クラスの登録
 */
BOOL imecomp_regist(const HINSTANCE hInstance)
{
	WNDCLASS wc;

	wc.style = CS_DBLCLKS|CS_IME;
	wc.lpfnWndProc = (WNDPROC)imecomp_proc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(0, IDC_IBEAM);
//	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.hbrBackground = NULL;
	wc.lpszMenuName = 0;
	wc.lpszClassName = IMECOMP_WND_CLASS;
	return RegisterClass(&wc);
}

