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
	// �ێ����Ă�����e
	BYTE	m_bComAttr[BUFFERSIZE+1];	// ����������̑���
	DWORD	m_dwCompCls[BUFFERSIZE+1];	// �R���|�W�V����
	TCHAR	m_szComStr[BUFFERSIZE+1];	// ����������̃o�b�t�@
	int		m_nComSize;					// ����������T�C�Y
	int		m_nComCursorPos;			// ����������ɂ����錻�݂̃J�[�\���ʒu
	DWORD	m_nComAttrSize;				// ����������̑����̃T�C�Y
	POINT	m_CandPos;					// ��⑋�̏ꏊ
	int		m_nCompClsSize;				// ����������̕��ߐ�
	int		m_nPlatform;				// PPC/Smartphone���[�h// 0:PPC 1:Smartphone
	POINT	m_CaretPos;					// �L�����b�g�ʒu
	DWORD	m_nColor[32];				// �J���[�R�[�h
	int		m_spacing;					// �s�Ԑݒ�
	HFONT	m_hFont;
}	COMPBUF;


/*
 * DrawChar - ������̕`�您��ѕ`��͈͌v�Z
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
 * DrawCompString - �ϊ�������̕`��(�܂�Ԃ�)
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
			// ���͕����̏ꍇ
			hPen = ::CreatePen( PS_DASH, 1, color[1]);
			break;
		case 0x01:
			// �ҏW�\�Ȍ��̏ꍇ
			if ( pf == 0 ){		// PPC�̎�
				// �j���{�n�C���C�g
				hPen = ::CreatePen( PS_DASH, 1, color[1]);
				reverse = true;
			}else{				// Smartphone�̎�
				// �������ځ@�n�C���C�g����
				hPen = ::CreatePen( PS_SOLID, 2, color[1]);
				reverse = false;
			}
			break;
		default:
			// �ϊ����ꂽ�����̏ꍇ
			hPen = ::CreatePen( PS_SOLID, 1, color[1]);
			break;
		}
		if ( reverse ){
			// ���]
			SetTextColor(hdc, color[3]);
			SetBkColor(hdc, color[2]);
			hBrush = ::CreateSolidBrush( color[2] );
		}else{
			// �ʏ�
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

		// i�����ڂ̕`��̈���擾
		DrawChar( hdc , str[i]  , &drect , false);

		// ��ʉE�[���z����悤�ł����
		if ( drect.right > prect->right ){
			if ( bdraw ){
				// �`��ς݂̕����ɉ���������
				MoveToEx(hdc , nXStart + 1, drect.bottom+spacing-1 , NULL);
				LineTo(hdc , nXEnd - 1, drect.bottom+spacing-1);
			}
			//int	w = drect.right - drect.left;
			//int h = drect.bottom - drect.top + spacing;
			// �`��ʒu�����Z�b�g
			drect.top  = drect.bottom + spacing;
			drect.left = prect->left;
			// �`��̈���Ď擾
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
		if ( i==0 ){		// �ꕶ���ڂŌ�⑋�̈ʒu���`�F�b�N
			if ( atr == 0x01 ){
				// �ҏW�\�Ȍ��̏ꍇ
				cand->x = nXStart;
				cand->y = drect.bottom+1;
			}
		}
	}
	if ( bdraw ){
		// ����������
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
 * GetCompRgn - �`��̈�̃��[�W���������߂�
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

	// ���ɂ͂ݏo����
	if ( ret ){
		if ( !retry ){
			// ���ɓ�����悤�ɂ��čČv�Z
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
 * imecomp_proc - IME �R���|�W�V�����E�B���h�E�v���V�[�W��
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

		// �o�b�t�@�̏�����
		ZeroMemory( &bf->m_bComAttr , sizeof(BYTE)*(BUFFERSIZE+1) );
		ZeroMemory( &bf->m_dwCompCls ,sizeof(DWORD)*(BUFFERSIZE+1) );
		ZeroMemory( &bf->m_szComStr , sizeof(TCHAR)*(BUFFERSIZE+1) );

		bf->m_hFont = NULL;

		// �J���[�R�[�h������
		bf->m_nColor[0] = GetSysColor( COLOR_WINDOW );
		bf->m_nColor[1] = GetSysColor( COLOR_WINDOWTEXT );
		bf->m_nColor[2] = GetSysColor( COLOR_HIGHLIGHT );
		bf->m_nColor[3] = GetSysColor( COLOR_HIGHLIGHTTEXT );

		// �s�ԏ�����
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
					// ���ߖ���=�S������\��
					POINT	pt = bf->m_CaretPos;
					DrawCompString(hdc,&bf->m_szComStr[0] ,-1 ,&pt ,
						bf->m_bComAttr[0] ,&bf->m_CandPos , bf->m_nPlatform	,&rect,true,bf->m_nColor,bf->m_spacing);

				}else{
					// �ϊ����߂̉����̕\��
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
				// �J�[�\���ʒu�̕\��
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
		// �R���|�W�V�����C�x���g
		if ((bf = (COMPBUF *)GetWindowLong(hWnd, GWL_USERDATA)) != NULL ) {
			if (lParam & GCS_COMPSTR) {
				HIMC	hIMC = (HIMC)(wParam);
				RECT rect;
				// �E�B���h�E�T�C�Y���O�ŏ�����
				rect.top = rect.left = rect.right = rect.bottom = 0;

				// ������������擾���܂�
				bf->m_nComSize = ImmGetCompositionString(hIMC, GCS_COMPSTR, (LPVOID)bf->m_szComStr, sizeof(TCHAR) * (BUFFERSIZE));

				// �f�[�^���肩�G���[���Ȃ��A�Ȃ�΃f�[�^�擾
				if (  bf->m_nComSize != IMM_ERROR_NODATA && bf->m_nComSize != IMM_ERROR_GENERAL){

					bf->m_szComStr[bf->m_nComSize/2]=_T('\0');

					// ������������ł̕�����������уJ�[�\���ʒu���擾���܂�
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
			// �e�E�C���h�E�̏ꏊ
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
		// �t�H���g��ݒ�
		if ((bf = (COMPBUF *)GetWindowLong(hWnd, GWL_USERDATA)) != NULL) {
			bf->m_hFont = (HFONT) wParam;
		}
		break;

	case WM_SETPLATFORM:
		// �v���b�g�t�H�[���̐ݒ�
		if ((bf = (COMPBUF *)GetWindowLong(hWnd, GWL_USERDATA)) != NULL ) {
			bf->m_nPlatform = wParam;
		}
		return 0;
	case WM_IMECOMP_SETCOLOR:
		// �J���[�R�[�h�ݒ�
		if ((bf = (COMPBUF *)GetWindowLong(hWnd, GWL_USERDATA)) != NULL ) {
			if ( wParam < 4 ){
				bf->m_nColor[wParam] = lParam;
			}
		}
		return 0;
	case WM_SETSPACING:
		// �s�Ԑݒ�
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
 * imecomp_regist - �N���X�̓o�^
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

