/*
 * nEdit
 *
 * nEdit.c
 *
 * Copyright (C) 1996-2005 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

/* Include Files */
#include	"stdafx.h"

#define _INC_OLE
#include <windows.h>
#undef  _INC_OLE
#include <tchar.h>
#ifdef OP_XP_STYLE
#include <uxtheme.h>
#include <tmschema.h>
#endif	// OP_XP_STYLE

#include "nEdit.h"
#include "imecomp.h"

/* Define */
#define RESERVE_BUF						1024
#define RESERVE_INPUT					256
#define RESERVE_LINE					256
#define RESERVE_UNDO					256

#define DRAW_LEN						256
#define LINE_MAX						32768
#define TAB_STOP						8

// �z�C�[�����b�Z�[�W
#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL					0x020A
#endif
// XP�e�[�}�ύX�ʒm
#ifndef WM_THEMECHANGED
#define WM_THEMECHANGED					0x031A
#endif

#define SWAP(a, b)						{a = b - a; b -= a; a += b;}
#define BUF_LEN							(bf->buf_len + bf->input_len - bf->del_len - bf->ip_len)

#define	IS_FLAG_ON( a )					( (bf->flags & (a))!=0 )
#define	IS_FLAG_OFF( a )				( (bf->flags & (a))==0 )

/* Global Variables */

/* Struct */
typedef struct _UNDO {
	BYTE type;

	DWORD st;
	DWORD len;
	TCHAR *buf;
} UNDO;

typedef struct _BUFFER {
	// �ێ����Ă�����e
	TCHAR *buf;
	DWORD buf_size;
	DWORD buf_len;

	// ���̓o�b�t�@
	TCHAR *input_buf;
	DWORD input_size;
	DWORD input_len;

	// �\���s���̃I�t�Z�b�g
	DWORD *line;
	int line_size;
	int line_len;
	int line_add_index;
	int line_add_len;

	// UNDO�o�b�t�@
	UNDO *undo;
	int undo_size;
	int undo_len;
	int undo_pos;

	// ���͊J�n�ʒu
	TCHAR *ip;
	DWORD ip_len;
	// �폜�J�n�ʒu
	TCHAR *dp;
	DWORD del_len;

	// �L�����b�g�̈ʒu
	DWORD cp;
	// �I���ʒu
	DWORD sp;
	// �㉺�ړ����̃L�����b�g��X���W
	int cpx;

	// 1�s�̕�����
	int line_max;
	// �s�̍ő啝
	int line_width;
	// �E�B���h�E�̕�
	int width;

	// �X�N���[�� �o�[
	int pos_x;
	int max_x;
	int pos_y;
	int max_y;

	// �^�u�X�g�b�v
	int tab_stop;
	// ���}�[�W��
	int left_margin;
	// ��}�[�W��
	int top_margin;
	// �E�}�[�W��
	int right_margin;
	// ���}�[�W��
	int bottom_margin;
	// �s��
	int spacing;

	// �`��p���
	HDC mdc;
	HBITMAP ret_bmp;
	HRGN hrgn;
	HFONT hfont;
	HFONT ret_font;

	// �t�H���g
	int font_height;
	int char_width;

	// �L�����b�g����
	int	caret_width;

	// �܂�Ԃ��t���O
	BOOL wordwrap;
	// �t�H�[�J�X�������Ă��I��\��
	BOOL no_hide_sel;
	// �������ɕϊ�
	BOOL lowercase;
	// �啶���ɕϊ�
	BOOL uppercase;
	// ���b�N�t���O
	BOOL lock;
	// �C���t���O
	BOOL modified;
	// ���̓��[�h
	BOOL insert_mode;
	// �I���t���O
	BOOL sel;
	// �}�E�X���t���O
	BOOL mousedown;
	// ���͒�����
	DWORD limit_len;
	// �P��I�����[�h
	BOOL wordselect;
	// �P��I�����̃L�����b�g�̈ʒu
	DWORD wordselect_cp;
	// �P��I�����̑I���ʒu
	DWORD wordselect_sp;

	// ���[�h���b�v�ʒu
	DWORD wordwrap_pos;

	// �s�ԍ�����
	DWORD linenum_size;

	// �����̕�
	BYTE cwidth[0x10000];

	// �R���g���[�����ʎq
	int id;
	// IME
	HIMC himc;
	// �R���|�W�V�����E�B���h�E
	HWND hCompWnd;

	// PPC/Smartphone���[�h
	int nPlatform;			// 0:PPC 1:Smartphone

	// �L���\��
	DWORD	drawsymbol;

#ifdef OP_XP_STYLE
	// XP
	HMODULE hModThemes;
	HTHEME hTheme;
#endif	// OP_XP_STYLE

	// �������}�b�s���O
#define	MAPNUM	16
	// �������}�b�s���O�n���h��
	HANDLE	hMmap[MAPNUM];
	// �������}�b�s���O�|�C���^
	void*	pMmap[MAPNUM];

	// �F
	DWORD	color[NEDIT_COLOR_MAX];

	// �݊��t���O
	DWORD	flags;
} BUFFER;



/* Local Function Prototypes */
static void *mem_alloc(BUFFER *bf,const DWORD size);
static void *mem_calloc(BUFFER *bf,const DWORD size);
static void mem_free(BUFFER *bf,void **mem);

static BOOL string_to_clipboard(const HWND hWnd, const TCHAR *st, const TCHAR *en);
static void notify_message(const HWND hWnd, BUFFER *bf, const int code);
static BOOL get_edit_rect(const HWND hWnd, BUFFER *bf, RECT *rect);
static void set_scrollbar(const HWND hWnd, BUFFER *bf);

static BOOL is_lead_byte(BUFFER *bf, TCHAR *p);
static int get_char_extent(BUFFER *bf, TCHAR *str, int *ret_len);
static TCHAR *char_next(BUFFER *bf, TCHAR *p);
static TCHAR *char_prev(BUFFER *bf, TCHAR *p);
static TCHAR *index_to_char(BUFFER *bf, const DWORD index);
static DWORD char_to_index(BUFFER *bf, const TCHAR *p);
static int index_to_line(BUFFER *bf, const DWORD index);

static BOOL line_alloc(BUFFER *bf, const int i, const int size);
static BOOL line_move(BUFFER *bf, const int i, const int len);
static DWORD line_get(BUFFER *bf, const int lindex);
static void line_set(BUFFER *bf, const int lindex, const DWORD len);
static void line_flush(BUFFER *bf);
static void line_add_length(BUFFER *bf, const int index, const int len);
static int line_get_length(BUFFER *bf, const DWORD index);
static DWORD line_get_next_index(BUFFER *bf, const DWORD index);
static BOOL line_set_info(BUFFER *bf);
static int line_get_count(BUFFER *bf, const int lindex);
static int line_set_count(BUFFER *bf, const int lindex, const int old_cnt, const int line_cnt);
static void line_refresh(const HWND hWnd, BUFFER *bf, const DWORD p, const DWORD r);

static BOOL undo_alloc(BUFFER *bf);
static void undo_free(BUFFER *bf, const int index);
static BOOL undo_set(BUFFER *bf, const int type, const DWORD st, const DWORD len);
static BOOL undo_exec(const HWND hWnd, BUFFER *bf);
static BOOL redo_exec(const HWND hWnd, BUFFER *bf);

static BOOL string_init(BUFFER *bf);
static BOOL string_set(const HWND hWnd, BUFFER *bf, const TCHAR *str, const DWORD len);
static BOOL string_insert(const HWND hWnd, BUFFER *bf, TCHAR *str, const int len, const BOOL insert_mode);
static void string_delete(const HWND hWnd, BUFFER *bf, DWORD st, DWORD en);
static void string_delete_char(const HWND hWnd, BUFFER *bf, DWORD st);
static BOOL string_flush(BUFFER *bf, const BOOL undo_flag);

static BOOL draw_init(const HWND hWnd, BUFFER *bf);
static void draw_free(BUFFER *bf);
static void draw_rect(const HWND hWnd, const HDC mdc, BUFFER *bf, const int left, const int top, const int right, const int bottom);
static int draw_string(const HWND hWnd, const HDC mdc, BUFFER *bf, const RECT *drect, const int left, const int top, const TCHAR *str, const int len, const BOOL sel);
static void draw_line(const HWND hWnd, const HDC mdc, BUFFER *bf, const int i, const int left, const int right,const bool caretline);

static	DWORD	draw_getcolor( BUFFER *bf , int code );
static	void	draw_setcolor( BUFFER *bf , int code , DWORD color );

static void caret_set_size(const HWND hWnd, BUFFER *bf);
static int caret_char_to_caret(const HDC mdc, BUFFER *bf, const int i, const DWORD cp);
static DWORD caret_point_to_caret(BUFFER *bf, const int x, const int y);
static void caret_get_token(BUFFER *bf);
static void caret_move(const HWND hWnd, BUFFER *bf, const int key);

static LRESULT CALLBACK nedit_proc(const HWND hWnd, const UINT msg, const WPARAM wParam, const LPARAM lParam);

/*
 * mem_alloc - �o�b�t�@���m��
 */
class	COomEx	{
public:
	int	size;
	COomEx( int s ){
		size = s;
	}
};


// ���������}�b�s���O�Ŏ��Ƃ���臒l�B������傫����΃}�b�s���O����
#define	MAP_THRESHOLD	(256*1024)
//#define	MAP_THRESHOLD	(256)
static void *mem_alloc(BUFFER *bf,const DWORD size)
{
//	return LocalAlloc(LMEM_FIXED, size);
	// 臒l���傫����
	if ( bf!=NULL && size > MAP_THRESHOLD ){
		int	i;
		// �e�[�u���̋󂫂�T��
		for( i=0;i<MAPNUM;i++ ){
			// �󂫂������
			if ( bf->hMmap[i]==NULL ){
				// �������}�b�s���O�Ń������m��
				void *buff = NULL;
				HANDLE hMap = CreateFileMapping((HANDLE)INVALID_HANDLE_VALUE,NULL,PAGE_READWRITE|SEC_COMMIT,0,size,NULL );
				if ( hMap != NULL ){
					buff = MapViewOfFile( hMap , FILE_MAP_WRITE , 0 , 0 , 0 );
					// �e�[�u���ɓo�^
					bf->hMmap[i] = hMap;
					bf->pMmap[i] = buff;
					return buff;
				}
			}
		}
	}// 臒l�ȉ����A�e�[�u���ɋ󂫂��Ȃ���Βʏ�ʂ�

	void	*ret = new BYTE[size];
	if ( ret == NULL ){
		throw COomEx(size);
	}
	return ret;
}

/*
 * mem_calloc - ���������ăo�b�t�@���m��
 */
static void *mem_calloc(BUFFER *bf,const DWORD size)
{
//	return LocalAlloc(LPTR, size);
	void	*ret = mem_alloc(bf, size );
	if ( ret != NULL ){
		ZeroMemory( ret , size );
	}
	return ret;
}

/*
 * mem_free - �o�b�t�@�����
 */
static void mem_free(BUFFER *bf,void **mem)
{
	//if (*mem != NULL) {
	//	LocalFree(*mem);
	//	*mem = NULL;
	//}

	int	i;
	// �e�[�u������|�C���^��T��
	for( i=0;i<MAPNUM;i++ ){
		// �e�[�u���ɂ����
		if ( bf->pMmap[i]==*mem ){
			UnmapViewOfFile( *mem );
			CloseHandle(bf->hMmap[i]);
			bf->hMmap[i] = NULL;
			bf->pMmap[i] = NULL;
			*mem=NULL;
			return ;
		}
	}
	DELA( *mem );
}

/*
 * string_to_clipboard - ��������N���b�v�{�[�h�ɐݒ�
 */
static BOOL string_to_clipboard(const HWND hWnd, const TCHAR *st, const TCHAR *en)
{
	HANDLE hMem;
	TCHAR *buf;

	if (OpenClipboard(hWnd) == FALSE) {
		return FALSE;
	}
	if (EmptyClipboard() == FALSE) {
		CloseClipboard();
		return FALSE;
	}

	// ���s�̐��𐔂���
	int crcnt = 0;
	{
		const TCHAR *src = st;
		for( int i=0;i<en - st ;i++){
			if ( *src++ == _T('\r') ){
				crcnt ++;
			}
		}
	}

	if ((hMem = GlobalAlloc(GHND, sizeof(TCHAR) * (en - st + 1 + crcnt))) == NULL) {
		CloseClipboard();
		return FALSE;
	}
	if ((buf = (TCHAR*)GlobalLock(hMem)) == NULL) {
		GlobalFree(hMem);
		CloseClipboard();
		return FALSE;
	}
//	CopyMemory(buf, st, sizeof(TCHAR) * (en - st));
	// cr -> crlf �̕ϊ����s���Ȃ���R�s�[
	{
		const TCHAR *src = st;
		TCHAR *dst = buf;
		for( int i=0; i< en - st; i++ ){
			if ( ( *dst++ = *src++ )==_T('\r') ){
				*dst++ = _T('\n');
			}
		}
		*dst++ = _T('\0');
	}
//	*(buf + (en - st)) = TEXT('\0');
	GlobalUnlock(hMem);
#ifdef UNICODE
	SetClipboardData(CF_UNICODETEXT, hMem);
#else
	SetClipboardData(CF_TEXT, hMem);
#endif

	CloseClipboard();
	return TRUE;
}

/*
 * notify_message - �e�E�B���h�E�ɒʒm
 */
static void notify_message(const HWND hWnd, BUFFER *bf, const int code)
{
	SendMessage(GetParent(hWnd), WM_COMMAND, MAKEWPARAM(bf->id, code), (LPARAM)hWnd);
}

/*
 * get_edit_rect - �`��̈�̎擾
 */
static BOOL get_edit_rect(const HWND hWnd, BUFFER *bf, RECT *rect)
{
	BOOL ret;

	ret = GetClientRect(hWnd, rect);

	rect->left += bf->left_margin;
	rect->top += bf->top_margin;
	rect->right -= bf->right_margin;
	rect->bottom -= bf->bottom_margin;
	return ret;
}

/*
 * set_scrollbar - �X�N���[���o�[�̐ݒ�
 */
static void set_scrollbar(const HWND hWnd, BUFFER *bf)
{
	SCROLLINFO si;
	RECT rect;
	
	int	linewidth = bf->left_margin + bf->line_width + bf->right_margin;

//	get_edit_rect(hWnd, bf, &rect);
	GetClientRect( hWnd , &rect );

	// ���X�N���[���o�[
	if (/*bf->wordwrap == FALSE &&*/ (rect.right - rect.left) < linewidth ) {
		int width = linewidth / bf->char_width + 1;

//		EnableScrollBar(hWnd, SB_HORZ, ESB_ENABLE_BOTH);

		bf->max_x = width - ((rect.right - rect.left) / bf->char_width);
		bf->pos_x = (bf->pos_x < bf->max_x) ? bf->pos_x : bf->max_x;

		ZeroMemory(&si, sizeof(SCROLLINFO));
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_POS | SIF_RANGE | SIF_PAGE;
		si.nPage = (rect.right - rect.left) / bf->char_width;
		si.nMax = width - 1;
		si.nPos = bf->pos_x;
		SetScrollInfo(hWnd, SB_HORZ, &si, TRUE);
	} else {
//		EnableScrollBar(hWnd, SB_HORZ, ESB_DISABLE_BOTH);

		bf->pos_x = bf->max_x = 0;

		ZeroMemory(&si, sizeof(SCROLLINFO));
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_POS | SIF_PAGE | SIF_RANGE;
		si.nMax = (bf->wordwrap == TRUE) ? 0 : 1;
		SetScrollInfo(hWnd, SB_HORZ, &si, TRUE);
	}

	// �c�X�N���[���o�[
	if (((rect.bottom - rect.top) / bf->font_height) < bf->line_len) {
//		EnableScrollBar(hWnd, SB_VERT, ESB_ENABLE_BOTH);

		bf->max_y = bf->line_len - ((rect.bottom - rect.top) / bf->font_height);
		bf->pos_y = (bf->pos_y < bf->max_y) ? bf->pos_y : bf->max_y;

		ZeroMemory(&si, sizeof(SCROLLINFO));
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_POS | SIF_RANGE | SIF_PAGE;
		si.nPage = (rect.bottom - rect.top) / bf->font_height;
		si.nMax = bf->line_len - 1;
//		si.nMax = bf->max_y-1;
		si.nPos = bf->pos_y;
		SetScrollInfo(hWnd, SB_VERT, &si, TRUE);
	} else {
//		EnableScrollBar(hWnd, SB_VERT, ESB_DISABLE_BOTH);

		bf->pos_y = bf->max_y = 0;

		ZeroMemory(&si, sizeof(SCROLLINFO));
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_POS | SIF_PAGE | SIF_RANGE;
		si.nMax = 1;
		SetScrollInfo(hWnd, SB_VERT, &si, TRUE);
	}
}

/*
 * ensure_visible - �L�����b�g�ʒu��\��
 */
static void ensure_visible(const HWND hWnd, BUFFER *bf ,bool force=false )
{
	RECT rect;
	int i;
	int x, y;

	get_edit_rect(hWnd, bf, &rect);

	i = index_to_line(bf, bf->cp);
	x = caret_char_to_caret(bf->mdc, bf, i, bf->cp) + (bf->pos_x * bf->char_width);
	y = i;

	// x
	i = bf->pos_x;
	if (x < (bf->pos_x + 5) * bf->char_width) {
		bf->pos_x = x / bf->char_width - 5;
		if (bf->pos_x < 0) {
			bf->pos_x = 0;
		}
	}
	if (x > ((bf->pos_x - 5) * bf->char_width) + (rect.right - rect.left)) {
		bf->pos_x = x / bf->char_width - (rect.right - rect.left) / bf->char_width + 5;
		if (bf->pos_x > bf->max_x) {
			bf->pos_x = bf->max_x;
		}
	}
	if (i != bf->pos_x) {
		SetScrollPos(hWnd, SB_HORZ, bf->pos_x, TRUE);
		ScrollWindowEx(hWnd, (i - bf->pos_x) * bf->char_width, 0, NULL, &rect, NULL, NULL, SW_INVALIDATE | SW_ERASE);
	}

	// y
	i = bf->pos_y;
#if 0
	if (y < bf->pos_y) {
		bf->pos_y = y;
	}
	if (y > bf->pos_y + (rect.bottom - rect.top) / bf->font_height - 1) {
		bf->pos_y = y - (rect.bottom - rect.top) / bf->font_height + 1;
		if (bf->pos_y > bf->max_y) {
			bf->pos_y = bf->max_y;
		}
	}
#else
	if (y < bf->pos_y ||
	    y > bf->pos_y + (rect.bottom - rect.top) / bf->font_height - 1 ||
		force ) {
		
		if (y == bf->pos_y - 1) {
			// 1�s��
			bf->pos_y = y;
		}else if (y == bf->pos_y + (rect.bottom - rect.top) / bf->font_height) {
			// 1�s��
			bf->pos_y = y - (rect.bottom - rect.top) / bf->font_height + 1;
		}else{
			// ����ȊO�̓L�����b�g����ʒ����ɗ���悤�ɏC��
			bf->pos_y = y - (rect.bottom - rect.top) / 2 / bf->font_height + 1;
		}

		if (bf->pos_y < 0) {
			bf->pos_y = 0;
		}
		if (bf->pos_y > bf->max_y) {
			bf->pos_y = bf->max_y;
		}
	}
#endif
	if (i != bf->pos_y) {
		rect.left = 0;
		SetScrollPos(hWnd, SB_VERT, bf->pos_y, TRUE);
		ScrollWindowEx(hWnd, 0, (i - bf->pos_y) * bf->font_height, NULL, &rect, NULL, NULL, SW_INVALIDATE | SW_ERASE);
	}
}

/*
 * ensure_inrect - �L�����b�g�ʒu��\���̈�܂ňړ�
 */
static void ensure_inrect(const HWND hWnd, BUFFER *bf)
{
	RECT rect;

	get_edit_rect(hWnd, bf, &rect);

	POINT	pt;
	if (GetCaretPos(&pt) != FALSE) {
		if (bf->cpx == 0) {
			bf->cpx = pt.x;
		}
		int	oldcp = bf->cp;

		// x
		if ( pt.x < rect.left ){
			bf->cp = caret_point_to_caret(bf, rect.left,pt.y );
		}else if ( pt.x > rect.right ){
			bf->cp = caret_point_to_caret(bf, rect.right-1,pt.y);
		}

		// y
		int	bottom = (rect.bottom - rect.top) / bf->font_height;
		bottom *= bf->font_height;

		if ( pt.y < rect.top ){
			bf->cp = caret_point_to_caret(bf, bf->cpx, rect.top);
		}else if ( pt.y > bottom ){
			bf->cp = caret_point_to_caret(bf, bf->cpx, bottom-1);
		}

		if ( bf->cp != oldcp ){
			if (GetKeyState(VK_SHIFT) >= 0) {
				bf->sp = bf->cp;
			}
			line_refresh(hWnd, bf, bf->cp, bf->cp);
		}
	}
}
/*
 * is_lead_byte - �}���`�o�C�g�̐擪�o�C�g���`�F�b�N
 */
static BOOL is_lead_byte(BUFFER *bf, TCHAR *p)
{
#ifdef UNICODE
	return FALSE;
#else
	if (IsDBCSLeadByte((BYTE)*p) == TRUE && char_to_index(bf, char_next(bf, p)) < BUF_LEN) {
		return TRUE;
	}
	return FALSE;
#endif
}

/*
 * get_char_extent - �����̕`�敝�̎擾
 */
static int get_char_extent(BUFFER *bf, TCHAR *str, int *ret_len)
{
	SIZE sz;
	int ret = 0;

#ifdef UNICODE
	*ret_len = 1;
	//if (HIBYTE(*str) == 0 && *(bf->cwidth + LOBYTE(*str)) != 0) {
	//	ret = *(bf->cwidth + LOBYTE(*str));
	//} else {
	//	GetTextExtentPoint32(bf->mdc, str, *ret_len, &sz);
	//	ret = sz.cx;
	//	if (HIBYTE(*str) == 0 && sz.cx < 256) {
	//		*(bf->cwidth + LOBYTE(*str)) = (BYTE)sz.cx;
	//	}
	//}
	if ( bf->cwidth[ *str ] == 0) {
		GetTextExtentPoint32(bf->mdc, str, *ret_len, &sz);
		bf->cwidth[ *str ] = (BYTE)sz.cx;
	}
	ret = bf->cwidth[ *str ];
#else
	*ret_len = (is_lead_byte(bf, str) == TRUE) ? 2 : 1;
	if (*ret_len == 1 && *(bf->cwidth + (unsigned char)*str) != 0) {
		ret = *(bf->cwidth + (unsigned char)*str);
	} else {
		GetTextExtentPoint32(bf->mdc, str, *ret_len, &sz);
		ret = sz.cx;
		if (*ret_len == 1 && sz.cx < 256) {
			*(bf->cwidth + (unsigned char)*str) = (BYTE)sz.cx;
		}
	}
#endif
	return ret;
}

/*
 * char_next - ���̕������擾
 */
static TCHAR *char_next(BUFFER *bf, TCHAR *p)
{
	if (bf->dp != NULL && p == bf->dp - 1) {
		return (bf->dp + bf->del_len);
	}

	if (bf->ip != NULL) {
		if (p == bf->ip - 1) {
			return bf->input_buf;
		} else if (p == bf->input_buf + bf->input_len - 1) {
			return (bf->ip + bf->ip_len);
		}
	}
	return (p + 1);
}

/*
 * char_prev - �O�̕������擾
 */
static TCHAR *char_prev(BUFFER *bf, TCHAR *p)
{
	if (bf->dp != NULL && p == bf->dp + bf->del_len) {
		return (bf->dp - 1);
	}

	if (bf->ip != NULL) {
		if (p == (bf->ip + bf->ip_len)) {
			return (bf->input_buf + bf->input_len - 1);
		} else if (p == bf->input_buf) {
			return (bf->ip - 1);
		}
	}
	return (p - 1);
}

/*
 * index_to_char - �����C���f�b�N�X���當���ʒu���擾
 */
static TCHAR *index_to_char(BUFFER *bf, const DWORD index)
{
	TCHAR *p;

	if (bf->dp != NULL && index >= (DWORD)(bf->dp - bf->buf)) {
		p = bf->buf + index + bf->del_len;
	} else if (bf->ip != NULL && index >= (DWORD)(bf->ip - bf->buf)) {
		if (index < (DWORD)(bf->ip - bf->buf + bf->input_len)) {
			p = bf->input_buf + (index - (bf->ip - bf->buf));
		} else {
			p = bf->buf + index - bf->input_len + bf->ip_len;
		}
	} else {
		p = bf->buf + index;
	}
	return p;
}

/*
 * char_to_index - �����ʒu���當���C���f�b�N�X���擾
 */
static DWORD char_to_index(BUFFER *bf, const TCHAR *p)
{
	DWORD i;

	if (bf->dp != NULL && p >= bf->dp) {
		i = p - bf->buf - bf->del_len;
	} else if (bf->ip != NULL && !(p >= bf->buf && p < bf->ip)) {
		if (p >= bf->input_buf && p <= bf->input_buf + bf->input_len) {
			i = (bf->ip - bf->buf) + (p - bf->input_buf);
		} else {
			i = (p - bf->buf) + bf->input_len - bf->ip_len;
		}
	} else {
		i = p - bf->buf;
	}
	return i;
}

/*
 * index_to_line - �����C���f�b�N�X����s�ʒu���擾
 */
static int index_to_line(BUFFER *bf, const DWORD index)
{
	int low = 0;
	int high = bf->line_len - 1;
	int i;

	while (low <= high) {
		i = (low + high) / 2;

		if (line_get(bf, i) > index) {
			if (i > 0 && line_get(bf, i - 1) <= index) {
				return (i - 1);
			}
			high  = i - 1;
		} else if (line_get(bf, i) < index) {
			if (i < bf->line_len - 1 && line_get(bf, i + 1) > index) {
				return i;
			}
			low = i + 1;
		} else {
			return i;
		}
	}
	return (bf->line_len - 1);
}

/*
 * line_alloc - �s���̊m��
 */
static BOOL line_alloc(BUFFER *bf, const int i, const int size)
{
	int *ln;

	bf->line_size += size;
	if ((ln = (int*)mem_alloc(bf,sizeof(int) * bf->line_size)) == NULL) {
		return FALSE;
	}
	if (bf->line != NULL) {
		CopyMemory(ln, bf->line, sizeof(int) * i);
		mem_free(bf,(void**)&bf->line);
	}
	bf->line = (DWORD*)ln;
	return TRUE;
}

/*
 * line_move - �s���̈ړ�
 */
static BOOL line_move(BUFFER *bf, const int i, const int len)
{
	int *ln;

	if (bf->line_len + len + 1 > bf->line_size) {
		// �ǉ�
		bf->line_size = bf->line_len + len + 1 + RESERVE_LINE;
		if ((ln = (int*)mem_alloc(bf,sizeof(int) * bf->line_size)) == NULL) {
			return FALSE;
		}
		CopyMemory(ln, bf->line, sizeof(int) * i);
		CopyMemory(ln + i + len, bf->line + i, sizeof(int) * (bf->line_len - i + 1));
		mem_free(bf,(void**)&bf->line);
		bf->line = (DWORD*)ln;
	} else {
		MoveMemory(bf->line + i + len, bf->line + i, sizeof(int) * (bf->line_len - i + 1));
	}
	bf->line_len += len;

	if (bf->line_len + 1 < bf->line_size - RESERVE_LINE) {
		// ���
		bf->line_size = bf->line_len + 1 + RESERVE_LINE;
		if ((ln = (int*)mem_alloc(bf,sizeof(int) * bf->line_size)) == NULL) {
			return FALSE;
		}
		CopyMemory(ln, bf->line, sizeof(int) * (bf->line_len + 1));
		mem_free(bf,(void**)&bf->line);
		bf->line = (DWORD*)ln;
	}
	return TRUE;
}

/*
 * line_get - �s���C���f�b�N�X�̎擾
 */
static DWORD line_get(BUFFER *bf, const int lindex)
{
	if (bf->line_add_len != 0 && bf->line_add_index <= lindex) {
		return (*(bf->line + lindex) + bf->line_add_len);
	}
	if ( lindex < 0 ){
		return -1;
	}
	return *(bf->line + lindex);
}

/*
 * line_set - �s���C���f�b�N�X��ݒ�
 */
static void line_set(BUFFER *bf, const int lindex, const DWORD len)
{
	if (bf->line_add_len != 0 && bf->line_add_index <= lindex) {
		*(bf->line + lindex) = len - bf->line_add_len;
	} else {
		*(bf->line + lindex) = len;
	}
}

/*
 * line_flush - �s���̔��f
 */
static void line_flush(BUFFER *bf)
{
	int i;

	for (i = bf->line_add_index; i <= bf->line_len; i++) {
		*(bf->line + i) += bf->line_add_len;
	}
	bf->line_add_index = 0;
	bf->line_add_len = 0;
}

/*
 * line_add_length - �s���ɕ�������ǉ�
 */
static void line_add_length(BUFFER *bf, const int index, const int len)
{
	int i;

	if (bf->line_add_index != 0 && bf->line_add_index < index) {
		for (i = bf->line_add_index; i < index; i++) {
			*(bf->line + i) += bf->line_add_len;
		}
	}
	bf->line_add_index = index;
	bf->line_add_len += len;
}

/*
 * line_get_length - �s�̒������擾
 */
static int line_get_length(BUFFER *bf, const DWORD index)
{
	int i, j;

	i = line_get(bf, index_to_line(bf, index) + 1) - 1;
	j = line_get(bf, index_to_line(bf, index));
	for (; i >= j &&
		(*index_to_char(bf, i) == TEXT('\r') /*|| *index_to_char(bf, i) == TEXT('\n')*/); i--);
	return (i - j + 1);
}

/*
 * line_get_next_index - ���̍s���̕����C���f�b�N�X���擾
 */
static DWORD line_get_next_index(BUFFER *bf, const DWORD index)
{
	TCHAR *p;
	DWORD word = -1;
	DWORD i;
	int line_size;
	int cnt = 0;
	int offset = 0; //bf->left_margin;
	int word_offset;
	int width;
	int tab;
	int clen;

	if (bf->wordwrap == TRUE && bf->width <= 0) {
		return BUF_LEN;
	}

	if ( bf->wordwrap_pos == 0 ){
		line_size = bf->width - bf->left_margin - bf->right_margin;
	}else{
		// ���[�h���b�v�ʒu�̐ݒ肪���鎞�́A���ϕ������������Őݒ�
		line_size = bf->char_width * bf->wordwrap_pos;
	}

	for (i = index; i < BUF_LEN; i++) {
		p = index_to_char(bf, i);
		switch (*p) {
		case TEXT('\r'):
//		case TEXT('\n'):
			// new line
			//if (*p == TEXT('\r') && (i + 1) < BUF_LEN && *(char_next(bf, p)) == TEXT('\n')) {
			//	i++;
			//}
			if ( bf->line_width < offset ) {
				bf->line_width = offset ;
			}
			return i;

		case TEXT('\t'):
			// tab
			tab = bf->tab_stop * bf->char_width - (offset % (bf->tab_stop * bf->char_width));
			if (tab < bf->char_width) {
				tab += bf->tab_stop * bf->char_width;
			}
			if ((bf->wordwrap == TRUE && offset + tab > line_size) ||
				(bf->line_max > 0 && cnt + bf->tab_stop - (cnt % bf->tab_stop) > bf->line_max)) {
				// �܂�Ԃ�
				if (word != -1 && word != index && clen == 1) {
					i = word;
					offset = word_offset;
				}
				if ( bf->line_width < offset ) {
					bf->line_width = offset ;
				}
				return (i + ((cnt == 0) ? 1 : 0));
			} else {
				cnt += bf->tab_stop - (cnt % bf->tab_stop);
				offset += tab;
			}
			word = -1;
			break;

		default:
			// char
			width = get_char_extent(bf, p, &clen);
			if ((bf->wordwrap == TRUE && offset + width > line_size) ||
				(bf->line_max > 0 && cnt + 1 > bf->line_max)) {
				// �܂�Ԃ�
				if (word != -1 && word != index && clen == 1) {
					i = word;
					offset = word_offset;
				}
				if ( bf->line_width < offset ) {
					bf->line_width = offset ;
				}
				return (i + ((cnt == 0) ? clen : 0));
			} else {
				cnt += clen;
				offset += width;
			}
			if (clen == 2) {
				i++;
				word = -1;
			} else if (*p != TEXT(' ') && *p < 0x100 ) {
				if (word == (DWORD)-1) {
					// �P��̊J�n�ʒu
					word = i;
					word_offset = offset;
				}
			} else {
				word = -1;
			}
			break;
		}
	}
	if ( bf->line_width < offset ) {
		// 1�s�̒��� (�X�N���[���o�[�p)
		bf->line_width = offset;
	}
	return i;
}

/*
 * line_set_info - �s���̐ݒ�
 */
static BOOL line_set_info(BUFFER *bf)
{
	DWORD index;
	int line = 1;
	int	limit;
	int	ret_len;
	int	numwidth = get_char_extent(bf, _T("0"), &ret_len);
	if ( bf->width == 0 ){
		return FALSE;
	}

restart:
	// �s�ԍ��\���L��̎�
	if ( IS_FLAG_ON( NEDIT_FLAGS_LINENUMBER ) ){
		// ���݂̌�������ő�s�������߂�
		limit = 1;
		for( unsigned int i=0;i<bf->linenum_size;i++){
			limit *=10;
		}
		// �s�ԍ��u���b�N�̕������߂�
		bf->left_margin = (bf->linenum_size+1) * numwidth;
	}else{
		bf->left_margin = 1;
	}
	bf->line_width =0;
	line=1;
	index = line_get_next_index(bf, 0);
	while (index < BUF_LEN) {
		switch (*index_to_char(bf, index)) {
		case TEXT('\r'):
//		case TEXT('\n'):
			// �s���̃C���f�b�N�X��ݒ�
			line_set(bf, line, index + 1);
			if (++line >= bf->line_size && line_alloc(bf, line, BUF_LEN / 30) == FALSE) {
				return FALSE;
			}
			index++;
			break;

		default:
			// �s���̃C���f�b�N�X��ݒ� (�܂�Ԃ�)
			line_set(bf, line, index);
			if (++line >= bf->line_size && line_alloc(bf, line, BUF_LEN / 30) == FALSE) {
				return FALSE;
			}
			break;
		}
		index = line_get_next_index(bf, index);

		// �s�ԍ��\���L��̎�
		if ( IS_FLAG_ON( NEDIT_FLAGS_LINENUMBER ) ){
			// �s�ԍ������オ���������
			if ( limit <= line ){
				bf->linenum_size++;
				limit *= 10;
				if ( bf->wordwrap == TRUE && bf->wordwrap_pos == 0 ){
					// �s�[�܂�Ԃ��̎��́A�ŏ�����v�蒼��
					goto restart;
				}else{
					// �������Œ�̎��́A�}�[�W�����Čv�Z����
					bf->left_margin = (bf->linenum_size+1) * numwidth;
				}
			}
		}
	}
	line_set(bf, line, index);
	bf->line_len = line;
	return TRUE;
}

/*
 * line_get_count - �_���s���̎擾
 */
static int line_get_count(BUFFER *bf, const int lindex)
{
	TCHAR *p;
	int i;

	for (i = lindex + 1; i < bf->line_len; i++) {
		p = index_to_char(bf, line_get(bf, i) - 1);
		if (*p == TEXT('\r') /*|| *p == TEXT('\n')*/) {
			// �_���s�̏I�[
			break;
		}
	}
	return (i - lindex);
}

/*
 * line_set_count - �s���̐ݒ�
 */
static int line_set_count(BUFFER *bf, const int lindex, const int old_cnt, const int line_cnt)
{
	TCHAR *p;
	DWORD index;
	int i = lindex;
	int rcnt = 0;

	index = line_get(bf, lindex - 1);
	while (1) {
		index = line_get_next_index(bf, index);
		if ((i - lindex) >= old_cnt) {
			// �}���s���ړ�
			line_move(bf, i, 1);
		}
		p = index_to_char(bf, index);
		if (*p == TEXT('\r') /*|| *p == TEXT('\n')*/) {
			// �s���̃C���f�b�N�X��ݒ�
			index++;
			line_set(bf, i++, index);
			if (++rcnt >= line_cnt) {
				break;
			}
		} else {
			// �s���̃C���f�b�N�X��ݒ� (�܂�Ԃ�)
			line_set(bf, i++, index);
			if (index >= BUF_LEN) {
				break;
			}
		}
	}

	if ((i - lindex) < old_cnt) {
		// �s���̍폜
		line_move(bf, i + old_cnt - (i - lindex), -(old_cnt - (i - lindex)));
	}
	return (i - lindex);
}

/*
 * line_refresh - �w��͈͕�����̂���s���ĕ`��Ώۂɂ���
 */
static void line_refresh(const HWND hWnd, BUFFER *bf, const DWORD p, const DWORD r)
{
	RECT rect;
	int i, j;

	// �����ʒu����s�̃C���f�b�N�X���擾
	i = index_to_line(bf, p);
	if (p == r) {
		j = i;
	} else {
		j = index_to_line(bf, r);
	}
	if (i > j) {
		SWAP(i, j);
	}
	i -= bf->pos_y;
	j -= bf->pos_y;

	// �ĕ`��
	get_edit_rect(hWnd, bf, &rect);
	if (i * bf->font_height > (rect.bottom - rect.top) || j * bf->font_height + bf->font_height < 0) {
		return;
	}
	if (j * bf->font_height + bf->font_height < (rect.bottom - rect.top)) {
		rect.bottom = j * bf->font_height + bf->font_height + bf->top_margin;
	}
	if (i * bf->font_height > 0) {
		rect.top = i * bf->font_height + bf->top_margin;
	}
	rect.left = 0;	//test
	InvalidateRect(hWnd, &rect, FALSE);
}

/*
 * undo_alloc - UNDO�̊m��
 */
static BOOL undo_alloc(BUFFER *bf)
{
	UNDO *ud;

	bf->undo_size += RESERVE_UNDO;
	if ((ud = (UNDO*)mem_calloc(bf,sizeof(UNDO) * bf->undo_size)) == NULL) {
		return FALSE;
	}
	if (bf->undo != NULL) {
		CopyMemory(ud, bf->undo, sizeof(UNDO) * bf->undo_len);
		mem_free(bf,(void**)&bf->undo);
	}
	bf->undo = ud;
	return TRUE;
}

/*
 * undo_free - UNDO�̉��
 */
static void undo_free(BUFFER *bf, const int index)
{
	int i;

	for (i = index; i < bf->undo_size; i++) {
		(bf->undo + i)->type = 0;
		mem_free(bf,(void**)&(bf->undo + i)->buf);
	}
}

/*
 * undo_set - UNDO�̃Z�b�g
 */
static BOOL undo_set(BUFFER *bf, const int type, const DWORD index, const DWORD len)
{
	undo_free(bf, bf->undo_len);
	if (bf->undo_len + 1 >= bf->undo_size && undo_alloc(bf) == FALSE) {
		return FALSE;
	}
	(bf->undo + bf->undo_len)->type = type;
	(bf->undo + bf->undo_len)->st = index;
	(bf->undo + bf->undo_len)->len = len;
	switch (type) {
	case UNDO_TYPE_INPUT:
		// ����
		break;

	case UNDO_TYPE_DELETE:
		// �폜
		if (((bf->undo + bf->undo_len)->buf = (TCHAR*)mem_alloc(bf,sizeof(TCHAR) * len)) == NULL) {
			return FALSE;
		}
		CopyMemory((bf->undo + bf->undo_len)->buf, bf->buf + index, sizeof(TCHAR) * len);
		break;
	}
	bf->undo_len++;
	return TRUE;
}

/*
 * undo_exec - UNDO�̎��s
 */
static BOOL undo_exec(const HWND hWnd, BUFFER *bf)
{
	int i;

	string_flush(bf, TRUE);

	i = bf->undo_len - 1;
	if (i < 0) {
		return TRUE;
	}
	bf->sp = bf->cp = (bf->undo + i)->st;
	switch ((bf->undo + i)->type) {
	case UNDO_TYPE_INPUT:
		if ((bf->undo + i)->buf == NULL) {
			if (((bf->undo + i)->buf = (TCHAR*)mem_alloc(bf,sizeof(TCHAR) * (bf->undo + i)->len)) == NULL) {
				return FALSE;
			}
			CopyMemory((bf->undo + i)->buf, bf->buf+(bf->undo+i)->st,sizeof(TCHAR)*(bf->undo+i)->len);
		}
		string_delete(hWnd, bf, (bf->undo + i)->st, (bf->undo + i)->st + (bf->undo + i)->len);
		break;

	case UNDO_TYPE_DELETE:
		string_insert(hWnd, bf, (bf->undo + i)->buf, (bf->undo + i)->len, TRUE);
		break;
	}
	string_flush(bf, FALSE);
	bf->undo_len--;

	SendMessage(hWnd, EM_SETMODIFY, (bf->undo_len == bf->undo_pos) ? FALSE : TRUE, 0);

	set_scrollbar(hWnd, bf);
	ensure_visible(hWnd, bf);
	InvalidateRect(hWnd, NULL, FALSE);
	return TRUE;
}

/*
 * redo_exec - REDO�̎��s
 */
static BOOL redo_exec(const HWND hWnd, BUFFER *bf)
{
	int i;

	string_flush(bf, TRUE);

	i = bf->undo_len;
	if ((bf->undo + i)->type == 0) {
		return TRUE;
	}
	bf->sp = bf->cp = (bf->undo + i)->st;
	switch ((bf->undo + i)->type) {
	case UNDO_TYPE_INPUT:
		string_insert(hWnd, bf, (bf->undo + i)->buf, (bf->undo + i)->len, TRUE);
		break;

	case UNDO_TYPE_DELETE:
		string_delete(hWnd, bf, (bf->undo + i)->st, (bf->undo + i)->st + (bf->undo + i)->len);
		break;
	}
	string_flush(bf, FALSE);
	bf->undo_len++;

	SendMessage(hWnd, EM_SETMODIFY, (bf->undo_len == bf->undo_pos) ? FALSE : TRUE, 0);

	set_scrollbar(hWnd, bf);
	ensure_visible(hWnd, bf);
	InvalidateRect(hWnd, NULL, FALSE);
	return TRUE;
}

/*
 * string_init - ������
 */
static BOOL string_init(BUFFER *bf)
{
	// free
	mem_free(bf,(void**)&bf->input_buf);
	mem_free(bf,(void**)&bf->line);
	undo_free(bf, 0);
	mem_free(bf,(void**)&bf->undo);

	// input init
	bf->input_size = RESERVE_INPUT;
	if ((bf->input_buf = (TCHAR*)mem_alloc(bf,sizeof(TCHAR) * bf->input_size)) == NULL) {
		return FALSE;
	}
	*bf->input_buf = TEXT('\0');
	bf->input_len = 0;
	bf->ip = NULL;
	bf->ip_len = 0;

	// line init
	bf->line_size = RESERVE_LINE;
	if ((bf->line = (DWORD*)mem_calloc(bf,sizeof(int) * bf->line_size)) == NULL) {
		return FALSE;
	}
	bf->line_len = 0;
	bf->line_width = 0;

	// undo init
	bf->undo_size = RESERVE_UNDO;
	if ((bf->undo = (UNDO*)mem_calloc(bf,sizeof(UNDO) * bf->undo_size)) == NULL) {
		return FALSE;
	}
	bf->undo_len = 0;

	bf->pos_x = bf->max_x = 0;
	bf->pos_y = bf->max_y = 0;
	bf->sp = bf->cp = 0;
	bf->cpx = 0;
	bf->dp = NULL;
	bf->del_len = 0;
	bf->modified = FALSE;
	return TRUE;
}

/*
 * string_set - ������̐ݒ�
 */
static BOOL string_set(const HWND hWnd, BUFFER *bf, const TCHAR *str, const DWORD len)
{
	HCURSOR old_cursor;
	RECT rect;

	old_cursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

	if (string_init(bf) == FALSE) {
		SetCursor(old_cursor);
		return FALSE;
	}

	// ������ݒ�
	mem_free(bf,(void**)&bf->buf);
	bf->buf_len = len;
	bf->buf_size = bf->buf_len + 1 + RESERVE_BUF;
	if ((bf->buf = (TCHAR*)mem_alloc(bf,sizeof(TCHAR) * bf->buf_size)) == NULL) {
		SetCursor(old_cursor);
		return FALSE;
	}
	CopyMemory(bf->buf, str, sizeof(TCHAR) * (bf->buf_len + 1));
	if (bf->lowercase == TRUE) {
		// �������ɕϊ�
		CharLowerBuff(bf->buf, bf->buf_len);
	} else if (bf->uppercase == TRUE) {
		// �啶���ɕϊ�
		CharUpperBuff(bf->buf, bf->buf_len);
	}

	// �s���̐ݒ�
	GetClientRect(hWnd, &rect);
	bf->width = (rect.right - rect.left);
	bf->linenum_size = 3;		// ���X�^�[�g
	line_set_info(bf);
	set_scrollbar(hWnd, bf);
	InvalidateRect(hWnd, NULL, FALSE);

	SetCursor(old_cursor);
	return TRUE;
}

/*
 * string_insert - ������̒ǉ�
 */
static BOOL string_insert(const HWND hWnd, BUFFER *bf, TCHAR *str, const int len, const BOOL insert_mode)
{
	TCHAR *p;
	DWORD ip_len;
	int lcnt1, lcnt2;
	int ret_cnt = 0;
	int st;
	int i;
	int ilen = len;
	BOOL sel = FALSE;

	// ���͐��� (�}�����[�h)
	if ((insert_mode == TRUE || (bf->cp != bf->sp)) && bf->limit_len != 0 && 
		(BUF_LEN - ((bf->cp > bf->sp) ? bf->cp - bf->sp : bf->sp - bf->cp)) + ilen > bf->limit_len) {
		i = bf->limit_len - (BUF_LEN - ((bf->cp > bf->sp) ? bf->cp - bf->sp : bf->sp - bf->cp));
		//for (ilen = 0; ilen < i; ilen++) {
		//	if (IsDBCSLeadByte((BYTE)*(str + ilen)) == TRUE ||
		//		(*(str + ilen) == TEXT('\r') && *(str + ilen + 1) == TEXT('\n'))) {
		//		if (ilen + 1 >= i) {
		//			break;
		//		}
		//		ilen++;
		//	}
		//}
		ilen = i;
		// �e�E�B���h�E�ɒʒm
		notify_message(hWnd, bf, EN_MAXTEXT);
		if (ilen <= 0) {
			return FALSE;
		}
	}
	if (bf->cp != bf->sp) {
		// �I�𕶎��̍폜
		string_delete(hWnd, bf, bf->cp, bf->sp);
		if (bf->cp > bf->sp) {
			SWAP(bf->cp, bf->sp);
		}
		bf->sp = bf->cp;
		sel = TRUE;
	}
	if (bf->dp != NULL) {
		string_flush(bf, TRUE);
	}
	// ���͐��� (�㏑�����[�h)
	if (insert_mode == FALSE && sel == FALSE && bf->limit_len != 0) {
		for (i = bf->cp, p = str; i - (int)bf->cp < ilen && (p - str) < ilen; i++, p++){
			if ((DWORD)i >= BUF_LEN || *index_to_char(bf, i) == TEXT('\r') /*|| *index_to_char(bf, i) == TEXT('\n')*/) {
				// �����ƍs���̏ꍇ�͈ȍ~��}�����[�h�Ɠ�������
				if (BUF_LEN + ilen > bf->limit_len) {
					i = bf->limit_len - BUF_LEN;
					//for (ilen = (p - str); ilen < i; ilen++) {
					//	if (IsDBCSLeadByte((BYTE)*(str + ilen)) == TRUE ||
					//		(*(str + ilen) == TEXT('\r') && *(str + ilen + 1) == TEXT('\n'))) {
					//		if (ilen + 1 >= i) {
					//			break;
					//		}
					//		ilen++;
					//	}
					//}
					ilen = i;
				}
				break;
			}
			if (IsDBCSLeadByte((BYTE)*p) == TRUE && (int)(p - str) < ilen) {
				if (is_lead_byte(bf, index_to_char(bf, i)) == TRUE) {
					// 2�o�C�g�R�[�h��2�o�C�g�̏㏑��
					i++;
				} else if ((BUF_LEN - (i - bf->cp)) + (p - str) + 1 > bf->limit_len) {
					// 1�o�C�g�R�[�h��2�o�C�g���㏑�����Đ����𒴂���ꍇ�͈ȍ~��؎̂�
					ilen = p - str;
					break;
				}
				p++;
			}
		}
		if (ilen != len) {
			// �e�E�B���h�E�ɒʒm
			notify_message(hWnd, bf, EN_MAXTEXT);
		}
		if (ilen <= 0) {
			return FALSE;
		}
	}

	// ���s���̎擾
	for (p = str; (p - str) < ilen; p++) {
		if (*p == TEXT('\r') /*|| *p == TEXT('\n')*/) {
			//if (*p == TEXT('\r') && *(p + 1) == TEXT('\n')) {
			//	p++;
			//}
			ret_cnt++;
		}
	}
	// �ǉ��O�̕\���s��
	lcnt1 = line_get_count(bf, index_to_line(bf, (bf->ip == NULL) ? bf->cp : ((bf->ip - bf->buf) + bf->input_len)));

	// ���̓o�b�t�@�̐ݒ�
	if (bf->input_len + ilen + 1 > bf->input_size) {
		bf->input_size = bf->input_len + ilen + 1 + RESERVE_INPUT;
		if ((p = (TCHAR*)mem_alloc(bf,sizeof(TCHAR) * bf->input_size)) == NULL) {
			return FALSE;
		}
		CopyMemory(p, bf->input_buf, sizeof(TCHAR) * bf->input_len);
		mem_free(bf,(void**)&bf->input_buf);
		bf->input_buf = p;
	}
	CopyMemory(bf->input_buf + bf->input_len, str, sizeof(TCHAR) * ilen);
	if (bf->lowercase == TRUE) {
		// �������ɕϊ�
		CharLowerBuff(bf->input_buf + bf->input_len, ilen);
	} else if (bf->uppercase == TRUE) {
		// �啶���ɕϊ�
		CharUpperBuff(bf->input_buf + bf->input_len, ilen);
	}
	bf->input_len += ilen;
	if (bf->ip == NULL) {
		bf->ip = index_to_char(bf, bf->cp);
	}
	bf->cp += ilen;
	bf->sp = bf->cp;

	// �㏑�����[�h
	for (p = str, ip_len = 0; sel == FALSE && insert_mode == FALSE && (p - str) < ilen; p++, ip_len++) {
		if (char_to_index(bf, bf->ip + bf->ip_len + ip_len) >= BUF_LEN ||
			*(bf->ip + bf->ip_len + ip_len) == TEXT('\r') /*|| *(bf->ip + bf->ip_len + ip_len) == TEXT('\n')*/) {
			break;
		}
		if (is_lead_byte(bf, bf->ip + bf->ip_len + ip_len) == TRUE) {
			ip_len++;
		}
		if (IsDBCSLeadByte((BYTE)*p) == TRUE && (p - str) < ilen) {
			p++;
		}
	}
	bf->ip_len += ip_len;

	// �s���̐ݒ�
	st = (bf->ip - bf->buf) + (bf->input_len - ilen);
	i = index_to_line(bf, st) + 1;

	lcnt2 = (ret_cnt > 0) ? line_set_count(bf, i, 0, ret_cnt) : 0;
	lcnt2 += line_set_count(bf, i + lcnt2, lcnt1, 0);
	line_add_length(bf, i + lcnt2, ilen - ip_len);
	// �e�E�B���h�E�ɒʒm
	notify_message(hWnd, bf, EN_UPDATE);

	if (lcnt1 > lcnt2) {
		// �s������
		InvalidateRect(hWnd, NULL, FALSE);
	} else if (ret_cnt > 0 || lcnt1 != lcnt2) {
		// �����s
		line_refresh(hWnd, bf, line_get(bf, i - 1), bf->buf_len + bf->input_len);
	} else if (lcnt2 > 1) {
		// �܂�Ԃ����܂߂Ĕ��f
		line_refresh(hWnd, bf, line_get(bf, i - 1), line_get(bf, i + lcnt2 - 2));
	} else {
		// �P��s
		line_refresh(hWnd, bf, st, st);
	}

	set_scrollbar(hWnd, bf);
	ensure_visible(hWnd, bf);

	if (bf->modified == FALSE) {
		bf->undo_pos = bf->undo_len;
	}
	bf->modified = TRUE;
	// �e�E�B���h�E�ɒʒm
	notify_message(hWnd, bf, EN_CHANGE);
	return TRUE;
}

/*
 * string_delete - ������̍폜
 */
static void string_delete(const HWND hWnd, BUFFER *bf, DWORD st, DWORD en)
{
	int lcnt1, lcnt2;
	int ret_cnt;
	int i;

	if (st > en) {
		SWAP(st, en);
	}
	if (bf->ip != NULL || (bf->dp != NULL && bf->dp != bf->buf + st && bf->dp != bf->buf + en)) {
		string_flush(bf, TRUE);
	}
	if (en - st > bf->buf_len - bf->del_len || st >= bf->buf_len - bf->del_len) {
		return;
	}

	// �s���擾
	ret_cnt = index_to_line(bf, en) - index_to_line(bf, st);
	lcnt1 = line_get_count(bf, index_to_line(bf, en));

	// �폜�ʒu�ݒ�
	bf->dp = bf->buf + st;
	bf->del_len += en - st;

	// �s���̐ݒ�
	i = index_to_line(bf, st) + 1;
	if (ret_cnt > 0) {
		line_move(bf, i + ret_cnt, -ret_cnt);
	}
	//if ( i > 1 ){	// �擪�s�ȍ~�Ȃ�A�O�̍s����Đ��`������
	//	i--;
	//	ret_cnt++;
	//	lcnt1++;
	//}
	lcnt2 = line_set_count(bf, i, lcnt1, 0);
	line_add_length(bf, i + lcnt2, st - en);

	// �e�E�B���h�E�ɒʒm
	notify_message(hWnd, bf, EN_UPDATE);

	if (ret_cnt > 0 || lcnt1 != lcnt2) {
		// �����s
		InvalidateRect(hWnd, NULL, FALSE);
	} else if (lcnt2 > 1) {
		// �܂�Ԃ����܂߂Ĕ��f
		line_refresh(hWnd, bf, line_get(bf, i - 1), line_get(bf, i + lcnt2 - 2));
	} else {
		// �P��s
		line_refresh(hWnd, bf, st, en);
	}
	if (bf->modified == FALSE) {
		bf->undo_pos = bf->undo_len;
	}
	bf->modified = TRUE;
	// �e�E�B���h�E�ɒʒm
	notify_message(hWnd, bf, EN_CHANGE);
}

/*
 * string_delete_char - �����̍폜
 */
static void string_delete_char(const HWND hWnd, BUFFER *bf, DWORD st)
{
	int i = 1;

	if (bf->cp != bf->sp) {
		// �I�𕶎��̍폜
		string_delete(hWnd, bf, bf->cp, bf->sp);
		if (bf->cp > bf->sp) {
			SWAP(bf->cp, bf->sp);
		}
		bf->sp = bf->cp;
	} else {
		// �ꕶ���폜
#ifndef UNICODE
		if (is_lead_byte(bf, index_to_char(bf, st)) == TRUE) {
			i++;
		}
#endif
		//if (*(index_to_char(bf, st)) == TEXT('\r') && *(index_to_char(bf, st + 1)) == TEXT('\n')) {
		//	i++;
		//}
		string_delete(hWnd, bf, st, st + i);
	}
	set_scrollbar(hWnd, bf);
	ensure_visible(hWnd, bf);
}

/*
 * string_flush - �폜�Ɠ��̓o�b�t�@�̔��f
 */
static BOOL string_flush(BUFFER *bf, const BOOL undo_flag)
{
	TCHAR *p;

	if (bf->dp != NULL) {
		if (undo_flag == TRUE) {
			// undo�ɒǉ�
			undo_set(bf, UNDO_TYPE_DELETE, bf->dp - bf->buf, bf->del_len);
		}

		// �폜������̔��f
		MoveMemory(bf->dp, bf->dp + bf->del_len, sizeof(TCHAR) * (bf->buf_len - (bf->dp - bf->buf + bf->del_len) + 1));
		bf->buf_len -= bf->del_len;
		bf->dp = NULL;
		bf->del_len = 0;

		if (bf->buf_len + 1 < bf->buf_size - RESERVE_BUF) {
			bf->buf_size = bf->buf_len + 1 + RESERVE_BUF;
			p = (TCHAR*)mem_alloc(bf,sizeof(TCHAR) * bf->buf_size);
			if (p == NULL) {
				return FALSE;
			}
			CopyMemory(p, bf->buf, sizeof(TCHAR) * (bf->buf_len + 1));
			mem_free(bf,(void**)&bf->buf);
			bf->buf = p;
		}
		// �s���̔��f
		line_flush(bf);
		return TRUE;
	}
	if (bf->ip == NULL) {
		return TRUE;
	}

	if (undo_flag == TRUE) {
		// undo�ɒǉ�
		if (bf->ip_len > 0) {
			undo_set(bf, UNDO_TYPE_DELETE, bf->ip - bf->buf, bf->ip_len);
		}
		undo_set(bf, UNDO_TYPE_INPUT, bf->ip - bf->buf, bf->input_len);
	}

	// ���̓o�b�t�@�̔��f
	if (bf->buf_len + bf->input_len - bf->ip_len + 1 > bf->buf_size) {
		bf->buf_size = bf->buf_len + bf->input_len - bf->ip_len + 1 + RESERVE_BUF;
		if ((p = (TCHAR*)mem_alloc(bf,sizeof(TCHAR) * bf->buf_size)) == NULL) {
			return FALSE;
		}
		if (bf->ip != bf->buf) {
			CopyMemory(p, bf->buf, sizeof(TCHAR) * (bf->ip - bf->buf));
		}
		CopyMemory(p + (bf->ip - bf->buf), bf->input_buf, sizeof(TCHAR) * bf->input_len);
		CopyMemory(p + (bf->ip - bf->buf) + bf->input_len, bf->ip + bf->ip_len, sizeof(TCHAR) * (bf->buf_len - (bf->ip + bf->ip_len - bf->buf) + 1));
		mem_free(bf,(void**)&bf->buf);
		bf->buf = p;
	} else {
		MoveMemory(bf->ip + bf->input_len, bf->ip + bf->ip_len, sizeof(TCHAR) * (bf->buf_len - (bf->ip + bf->ip_len - bf->buf) + 1));
		CopyMemory(bf->ip, bf->input_buf, sizeof(TCHAR) * bf->input_len);
	}
	bf->buf_len += bf->input_len - bf->ip_len;

	// ���̓o�b�t�@�̉��
	if (bf->input_size > RESERVE_INPUT) {
		mem_free(bf,(void**)&bf->input_buf);
		bf->input_size = RESERVE_INPUT;
		if ((bf->input_buf = (TCHAR*)mem_alloc(bf,sizeof(TCHAR) * bf->input_size)) == NULL) {
			return FALSE;
		}
	}
	*bf->input_buf = TEXT('\0');
	bf->input_len = 0;
	bf->ip = NULL;
	bf->ip_len = 0;

	// �s���̔��f
	line_flush(bf);
	return TRUE;
}

/*
 * draw_init - �`����̏�����
 */
static BOOL draw_init(const HWND hWnd, BUFFER *bf)
{
	HDC hdc;
	HBITMAP hBmp;
	HRGN hrgn[2];
	RECT rect;
	TEXTMETRIC tm;

	GetClientRect(hWnd, &rect);
	GetTextMetrics(bf->mdc, &tm);

	hdc = GetDC(hWnd);
	hBmp = CreateCompatibleBitmap(hdc, rect.right, tm.tmHeight + bf->spacing);
	bf->ret_bmp = (HBITMAP)SelectObject(bf->mdc, hBmp);
	ReleaseDC(hWnd, hdc);

	// ���[�W�����̍쐬
	hrgn[0] = CreateRectRgnIndirect(&rect);
	bf->hrgn = CreateRectRgnIndirect(&rect);
	// �������郊�[�W�����̍쐬
	get_edit_rect(hWnd, bf, &rect);
	rect.left = 0;
	hrgn[1] = CreateRectRgnIndirect(&rect);
	// ���[�W�����̌���
	CombineRgn(bf->hrgn, hrgn[0], hrgn[1], RGN_DIFF);
	DeleteObject(hrgn[0]);
	DeleteObject(hrgn[1]);

	return TRUE;
}

/*
 * draw_free - �`����̉��
 */
static void draw_free(BUFFER *bf)
{
	HBITMAP hBmp;

	hBmp = (HBITMAP)SelectObject(bf->mdc, bf->ret_bmp);
	DeleteObject(hBmp);
	DeleteObject(bf->hrgn);
	bf->hrgn = NULL;
}

/*
 * draw_rect - ��`�`��
 */
static void draw_rect(const HWND hWnd, const HDC mdc, BUFFER *bf, const int left, const int top, const int right, const int bottom)
{
	RECT trect;
	HBRUSH hBrush;

	if (right < 0) {
		return;
	}
	if (bf->no_hide_sel == FALSE && GetFocus() != hWnd) {
		return;
	}
	SetRect(&trect, left, top, right, bottom);

//	hBrush = CreateSolidBrush(GetSysColor(COLOR_HIGHLIGHT));
	hBrush = CreateSolidBrush(draw_getcolor(bf , NEDIT_COLOR_HIGHLIGHT));
	FillRect(mdc, &trect, hBrush);
	DeleteObject(hBrush);
}

/*
 * draw_symbol - �V���{���`��
 */
static void draw_symbol(const HWND hWnd, const HDC mdc, BUFFER *bf,  int left,  int top,  int right,  int bottom , TCHAR symbol , BOOL sel)
{
	RECT trect;
	HBRUSH hBrush;
	HPEN hPen;

	if (right < 0) {
		return;
	}

	int	pentype = PS_SOLID;
	if ( symbol == _T('�@') ){
		pentype = PS_DASH;
	}

	if ( sel==TRUE && (bf->no_hide_sel == TRUE || GetFocus() == hWnd ) ) {
		// �I��L��
		hBrush = CreateSolidBrush(draw_getcolor(bf , NEDIT_COLOR_HIGHLIGHT));
		hPen = ::CreatePen( pentype, 1, draw_getcolor(bf , NEDIT_COLOR_HIGHLIGHTTEXT));
	}else{
		// �I�𖳂�
		hBrush = CreateSolidBrush(draw_getcolor(bf , NEDIT_COLOR_WINDOW));
		hPen = ::CreatePen( pentype, 1, draw_getcolor(bf , NEDIT_COLOR_HIGHLIGHT));
	}
	SetRect(&trect, left, 0, right, bottom);
	FillRect(mdc, &trect, hBrush);

	// �L���`��
	HGDIOBJ	pOldPen = SelectObject(mdc , hPen);

	bottom -= top/2;
	switch( symbol ){
	case _T('\r'):		// ���s�L��
		if ( (bf->drawsymbol & DS_LINEBREAK )!=0 ){
			int	delta = (right-left)/2;
			int	center = (left+right)/2;
			int	height = bottom-top;
			int upper = top + height*2/10;
			int lower = top + height*8/10-1;
			MoveToEx(mdc , center, upper , NULL);
			LineTo(mdc ,  center, lower );
			LineTo(mdc ,  center-delta , lower-delta);

			MoveToEx(mdc , center, lower , NULL);
			LineTo(mdc ,  center +delta ,lower - delta);
		}
		break;
	case _T('\t'):		// �^�u����
		if ( (bf->drawsymbol & DS_TAB )!=0 ){
			MoveToEx(mdc , left+1 , (bottom+top)/2 - 4 , NULL);
			LineTo(mdc ,  left+5 , (bottom+top)/2 );
			LineTo(mdc ,  left+1 , (bottom+top)/2 + 4 );
		}
		break;
	case _T(' '):		// ���p�X�y�[�X
		if ( (bf->drawsymbol & DS_SPACE )!=0 ){
			int	center = (top+bottom)*4/5;
			MoveToEx(mdc , left+1 ,center, NULL);
			LineTo(mdc ,  left+1 , bottom-2 );
			LineTo(mdc ,  right-2 , bottom-2 );
			LineTo(mdc ,  right-2 , center-1 );
		}
		break;
	case _T('�@'):		// �S�p�X�y�[�X
		if ( (bf->drawsymbol & DS_WIDESPACE )!=0 ){
			MoveToEx(mdc , left+1 ,top+1, NULL);
			LineTo(mdc ,  left+1 , bottom-2 );
			LineTo(mdc ,  right-2 , bottom-2 );
			LineTo(mdc ,  right-2 , top+1 );
			LineTo(mdc ,  left+1 , top+1 );

		}
		break;
	case 0xFFFF:		// EOF
		break;
	}
	SelectObject(mdc , pOldPen);
	DeleteObject(hBrush);
	DeleteObject(hPen);
}

/*
 * draw_string - ������`��
 */
static int draw_string(const HWND hWnd, const HDC mdc, BUFFER *bf, const RECT *drect, const int left, const int top, const TCHAR *str, const int len, const BOOL sel)
{
	RECT rect;
	SIZE sz;

	GetTextExtentPoint32(bf->mdc, str, len, &sz);
	if (left + sz.cx >= drect->left) {
		if (sel == TRUE && (bf->no_hide_sel == TRUE || GetFocus() == hWnd)) {
			SetTextColor(mdc, draw_getcolor(bf , NEDIT_COLOR_HIGHLIGHTTEXT));
			SetBkColor(mdc, draw_getcolor(bf , NEDIT_COLOR_HIGHLIGHT));
//			draw_rect(hWnd, mdc, bf, left, drect->top, left + sz.cx, drect->bottom);
		}
		SetRect(&rect, left, drect->top, left + sz.cx, drect->bottom);
		ExtTextOut(mdc, left, top, ETO_OPAQUE, &rect, str, len, NULL);
		if (sel == TRUE) {
			SetTextColor(mdc, draw_getcolor(bf , NEDIT_COLOR_WINDOWTEXT));
			SetBkColor(mdc, draw_getcolor(bf , NEDIT_COLOR_WINDOW));
		}
	}
	return sz.cx;
}

/*
 * draw_line - 1�s�`��
 */
static void draw_line(const HWND hWnd, const HDC mdc, BUFFER *bf, const int i, const int left, const int right,const bool caretline)
{
	RECT drect;
	TCHAR *p, *r, *s;
	DWORD j;
	int offset;
	int top;
	int width;
	int tab;

	offset = bf->left_margin - (bf->pos_x * bf->char_width);
	width = (bf->pos_x * bf->char_width) + right;
	top = bf->spacing / 2;

	drect.left = left;
	drect.top = 0;
	drect.bottom = drect.top + bf->font_height;

	SetTextAlign( mdc , TA_LEFT|TA_TOP|TA_NOUPDATECP );

	for (j = line_get(bf, i), s = p = index_to_char(bf, line_get(bf, i)); j < line_get(bf, i + 1) && offset <= width; j++, p = char_next(bf, p)) {
		if (s != p) {
			if (bf->ip != NULL && (p == bf->ip + bf->ip_len || p == bf->input_buf)) {
				// ���̓o�b�t�@�̏o��
				r = (p == bf->input_buf) ? bf->ip : (bf->input_buf + bf->input_len);
				offset += draw_string(hWnd, mdc, bf, &drect, offset, top, s, r - s, bf->sel);
				s = p;
			} else if (bf->dp != NULL && p == bf->dp + bf->del_len) {
				// �폜������
				offset += draw_string(hWnd, mdc, bf, &drect, offset, top, s, bf->dp - s, bf->sel);
				s = p;
			} else if ((j % DRAW_LEN) == 0) {
				offset += draw_string(hWnd, mdc, bf, &drect, offset, top, s, p - s, bf->sel);
				s = p;
			}
		}
		if ((j >= bf->sp && j < bf->cp) || (j >= bf->cp && j < bf->sp)) {
			if (bf->sel == FALSE) {
				// �I���J�n
				offset += draw_string(hWnd, mdc, bf, &drect, offset, top, s, p - s, bf->sel);
				s = p;
				bf->sel = TRUE;
			}
		} else if (bf->sel == TRUE) {
			// �I���I��
			offset += draw_string(hWnd, mdc, bf, &drect, offset, top, s, p - s, bf->sel);
			s = p;
			bf->sel = FALSE;
		}

		if (*p == TEXT('\r') /*|| *p == TEXT('\n')*/) {
			// return
			offset += draw_string(hWnd, mdc, bf, &drect, offset, top, s, p - s, bf->sel);
			s = p;
			//if (bf->sel == TRUE && offset + bf->char_width >= 0) {
			//	draw_rect(hWnd, mdc, bf, offset, top - (bf->spacing / 2),
			//		offset + bf->char_width, top - (bf->spacing / 2) + bf->font_height);
			//}
			if ( offset + bf->char_width >= 0) {
				draw_symbol(hWnd, mdc, bf, offset, top ,
					offset + bf->char_width, drect.bottom , _T('\r') , bf->sel);
			}
			break;
		} else if (*p == TEXT('\t')) {
			// Tab
			offset += draw_string(hWnd, mdc, bf, &drect, offset, top, s, p - s, bf->sel);

			tab = bf->tab_stop * bf->char_width -
				((offset - bf->left_margin + (bf->pos_x * bf->char_width)) % (bf->tab_stop * bf->char_width));
			if (tab < bf->char_width) {
				tab += bf->tab_stop * bf->char_width;
			}
			//if (bf->sel == TRUE && offset + tab >= 0) {
			//	draw_rect(hWnd, mdc, bf, offset, top - (bf->spacing / 2),
			//		offset + tab, top - (bf->spacing / 2) + bf->font_height);
			//}
			if ( offset + tab >= 0) {
				draw_symbol(hWnd, mdc, bf, offset, top ,
					offset + tab, drect.bottom , _T('\t'),bf->sel );
			}
			offset += tab;

			s = char_next(bf, p);
			continue;
		} else if ( 
			(*p == TEXT(' ') && ((bf->drawsymbol & DS_SPACE)!=0) )||
			(*p == TEXT('�@') && ((bf->drawsymbol & DS_WIDESPACE)!=0) ) 
			)
		{
			// �S�p/���p�X�y�[�X
			offset += draw_string(hWnd, mdc, bf, &drect, offset, top, s, p - s, bf->sel);

			int	space = bf->cwidth[ *p ];
			draw_symbol(hWnd, mdc, bf, offset, top ,
				offset + space, drect.bottom , *p ,bf->sel );

			offset += space;

			s = char_next(bf, p);
			continue;
		}


#ifndef UNICODE
		if (is_lead_byte(bf, p) == TRUE && char_next(bf, p) == (p + 1)) {
			p = char_next(bf, p);
			j++;
		}
#endif
	}
	if (s != p && bf->ip != NULL && (p == bf->ip + bf->ip_len || p == bf->input_buf)) {
		r = (p == bf->input_buf) ? bf->ip : (bf->input_buf + bf->input_len);
	} else if (s != p && bf->dp != NULL && p == bf->dp + bf->del_len) {
		r = bf->dp;
	} else {
		r = p;
	}
	draw_string(hWnd, mdc, bf, &drect, offset, top, s, r - s, bf->sel);

	// �s�ԍ��\���L��̎�
	if ( IS_FLAG_ON( NEDIT_FLAGS_LINENUMBER ) ){
		RECT nrect;
		TCHAR	buff[20];
		UINT	len;
		int	ret_len;
		int	numwidth = get_char_extent(bf, _T("0"), &ret_len);

		nrect.left = 0;
		nrect.top = top;
		nrect.right = nrect.left + (bf->linenum_size+1) * numwidth ;
		nrect.bottom = nrect.top + bf->font_height;

		_stprintf( buff , _T("%d") , i+1 );
		len = _tcslen(buff);

		SetTextAlign( mdc , TA_RIGHT|TA_TOP|TA_NOUPDATECP );
		ExtTextOut(mdc, nrect.right-numwidth+1, top, ETO_OPAQUE, &nrect, buff, len, NULL);

		// ��؂��
		HPEN	hPen = ::CreatePen( PS_SOLID, 1, draw_getcolor(bf , NEDIT_COLOR_WINDOWTEXT) );
		HGDIOBJ	pOldPen = SelectObject(mdc , hPen);

		MoveToEx(mdc , nrect.right-numwidth/2 , drect.top , NULL);
		LineTo(mdc , nrect.right-numwidth/2 , drect.top + bf->font_height + bf->spacing );

		SelectObject( mdc , pOldPen);
		DeleteObject( hPen );
	}
	// �L�����b�g�ʒu�ɉ���������
	if ( caretline && IS_FLAG_ON( NEDIT_FLAGS_UNDERLINE ) ){
		HPEN	hPen = ::CreatePen( PS_SOLID, 1, draw_getcolor(bf , NEDIT_COLOR_UNDERLINE) );
		HGDIOBJ	pOldPen = SelectObject(mdc , hPen);

		MoveToEx(mdc , left , drect.bottom-1 , NULL);
		LineTo(mdc , right , drect.bottom-1 );

		SelectObject( mdc , pOldPen);
		DeleteObject( hPen );
	}

}

/*
 * draw_getcolor - �F�擾
 */
DWORD	draw_getcolor( BUFFER *bf , int code )
{
	DWORD	color = bf->color[code];
	// FFFFFFFF�̎��́A�V�X�e���J���[���擾
	if ( color == 0xFFFFFFFF ){
		switch(code){
		case NEDIT_COLOR_WINDOW:
			color = GetSysColor( COLOR_WINDOW );
			break;
		case NEDIT_COLOR_WINDOWTEXT:
			color = GetSysColor( COLOR_WINDOWTEXT );
			break;
		case NEDIT_COLOR_HIGHLIGHT:
			color = GetSysColor( COLOR_HIGHLIGHT );
			break;
		case NEDIT_COLOR_HIGHLIGHTTEXT:
			color = GetSysColor( COLOR_HIGHLIGHTTEXT );
			break;
		case NEDIT_COLOR_UNDERLINE:
			color = GetSysColor( COLOR_HIGHLIGHT );
			break;
		default:
			color = 0xFFFFFF;
			break;
		}
		// �ۑ����Ă����i�Ȃ̂ŁA�N�����̃e�[�}�̕ύX�ɂ͒ǐ��ł��Ȃ��Ȃ�j
		bf->color[code] = color;
	}
	return color;
}

/*
 * draw_getcolor - �F�ݒ�
 */
void	draw_setcolor( BUFFER *bf , int code , DWORD color )
{
	bf->color[code] = color;
}


/*
 * caret_set_size - �L�����b�g�̃T�C�Y�ݒ�
 */
static void caret_set_size(const HWND hWnd, BUFFER *bf)
{
	TCHAR *p;
	int csize;
	int len;

	p = index_to_char(bf, bf->cp);
	csize = get_char_extent(bf, p, &len);
	if (csize <= 0) {
		csize = bf->char_width;
	}
	DestroyCaret();
	CreateCaret(hWnd, NULL, csize, bf->font_height);
}

/*
 * caret_char_to_caret - �����ʒu����L�����b�g�̈ʒu�擾
 */
static int caret_char_to_caret(const HDC mdc, BUFFER *bf, const int i, const DWORD cp)
{
	SIZE sz;
	TCHAR *p, *r, *s;
	DWORD j;
	int offset;
	int tab;

	offset = bf->left_margin - (bf->pos_x * bf->char_width);
	for (j = line_get(bf, i), s = p = index_to_char(bf, line_get(bf, i)); j < line_get(bf, i + 1); j++, p = char_next(bf, p)) {
		r = NULL;
		if (s != p && bf->ip != NULL && (p == bf->ip + bf->ip_len || p == bf->input_buf)) {
			// ���̓o�b�t�@
			r = (p == bf->input_buf) ? bf->ip : (bf->input_buf + bf->input_len);
		} else if (s != p && bf->dp != NULL && p == bf->dp + bf->del_len) {
			// �폜������
			r = bf->dp;
		}
		if (r != NULL) {
			GetTextExtentPoint32(mdc, s, r - s, &sz);
			offset += sz.cx;
			s = p;
		}
		if (j >= cp) {
			break;
		}

		if (*p == TEXT('\r') /*|| *p == TEXT('\n')*/) {
			break;
		} else if (*p == TEXT('\t')) {
			// tab
			GetTextExtentPoint32(mdc, s, p - s, &sz);
			offset += sz.cx;
			tab = bf->tab_stop * bf->char_width - ((offset - bf->left_margin + (bf->pos_x * bf->char_width)) % (bf->tab_stop * bf->char_width));
			if (tab < bf->char_width) {
				tab += bf->tab_stop * bf->char_width;
			}
			offset += tab;
			s = char_next(bf, p);
			continue;
		}
#ifndef UNICODE
		if (is_lead_byte(bf, p) == TRUE) {
			p = char_next(bf, p);
			j++;
		}
#endif
	}
	if (s != p && bf->ip != NULL && (p == bf->ip + bf->ip_len || p == bf->input_buf)) {
		r = (p == bf->input_buf) ? bf->ip : (bf->input_buf + bf->input_len);
	} else if (s != p && bf->dp != NULL && p == bf->dp + bf->del_len) {
		r = bf->dp;
	} else {
		r = p;
	}
	GetTextExtentPoint32(mdc, s, r - s, &sz);
	return (offset + sz.cx);
}

/*
 * caret_point_to_caret - ���W����L�����b�g�̈ʒu�擾
 */
static DWORD caret_point_to_caret(BUFFER *bf, const int x, const int y)
{
	TCHAR *p;
	DWORD j;
	int offset, old;
	int tab;
	int clen = 1;
	int i;

	i = bf->pos_y + ((y - bf->top_margin) / bf->font_height);
	if (i < 0) {
		return 0;
	} else if (i >= bf->line_len) {
		i = bf->line_len - 1;
	}
	old = offset = bf->left_margin - (bf->pos_x * bf->char_width);
	for (j = line_get(bf, i), p = index_to_char(bf, line_get(bf, i)); j < line_get(bf, i + 1); j++, p = char_next(bf, p)) {
		if (*p == TEXT('\r') /*|| *p == TEXT('\n')*/) {
			return j;
		} else if (*p == TEXT('\t')) {
			// tab
			clen = 1;
			tab = bf->tab_stop * bf->char_width - ((offset - bf->left_margin + (bf->pos_x * bf->char_width)) % (bf->tab_stop * bf->char_width));
			if (tab < bf->char_width) {
				tab += bf->tab_stop * bf->char_width;
			}
			offset += tab;
			if (offset > x) {
				break;
			}
			old = offset;
		} else {
			offset += get_char_extent(bf, p, &clen);
			if (offset > x) {
				if ( old < bf->left_margin || (offset - old) / 2 < x - old) {
					j += clen;
				}
				break;
			}
			old = offset;
			if (clen == 2) {
				p = char_next(bf, p);
				j++;
			}
		}
	}
	if (j == line_get(bf, i + 1) && (i + 1) < bf->line_len) {
		j -= clen;
	}
	return j;
}

/*
 * caret_get_token - �L�����b�g�ʒu�̃g�[�N�����擾
 */
static void caret_get_token(BUFFER *bf , DWORD *cp , DWORD *sp)
{
	TCHAR *p, *r;
	int i;

	i = index_to_line(bf, *cp);
	for (; i > 0; i--) {
		// �_���s�̐擪�Ɉړ�
		p = index_to_char(bf, line_get(bf, i) - 1);
		if (*p == TEXT('\r') /*|| *p == TEXT('\n')*/) {
			break;
		}
	}

	p = index_to_char(bf, line_get(bf, i));
	while (char_to_index(bf, p) < BUF_LEN && *p != TEXT('\r') /*&& *p != TEXT('\n')*/) {
		if (is_lead_byte(bf, p) == TRUE) {
			r = p;
			while (is_lead_byte(bf, p) == TRUE) {
				p = char_next(bf, p);
				p = char_next(bf, p);
			}

		} else if (*p != TEXT(' ') && *p != TEXT('\t') && *p != TEXT('\r') /*&& *p != TEXT('\n')*/) {
			r = p;
			while (char_to_index(bf, p) < BUF_LEN && is_lead_byte(bf, p) == FALSE &&
				*p != TEXT(' ') && *p != TEXT(' ') && *p != TEXT('\t') && *p != TEXT('\r') /*&& *p != TEXT('\n')*/) {
				p = char_next(bf, p);
			}

		} else {
			r = p;
			p = char_next(bf, p);
		}
		if (p > index_to_char(bf, *cp)) {
			*cp = char_to_index(bf, p);
			*sp = char_to_index(bf, r);
			break;
		}
	}
}

/*
 * caret_move - �L�����b�g�̈ړ�
 */
static void caret_move(const HWND hWnd, BUFFER *bf, const int key)
{
	RECT rect;
	POINT pt;
	TCHAR *p;
	DWORD oldcp, oldsp;
	DWORD j;
	int i, t;

	get_edit_rect(hWnd, bf, &rect);

	oldcp = bf->cp;
	oldsp = bf->sp;
	i = index_to_line(bf, bf->cp);

	switch (key) {
	case VK_HOME:
		if (GetKeyState(VK_CONTROL) < 0) {
			// �S�̂̐擪
			bf->cp = 0;
		} else {
#if 0
			// �_���s��
			for (; *(index_to_char(bf, bf->cp)) == TEXT('\r') || *(index_to_char(bf, bf->cp)) == TEXT('\n'); bf->cp--)
				;
			for (; bf->cp > 0 && *(index_to_char(bf, bf->cp)) != TEXT('\r') && *(index_to_char(bf, bf->cp)) != TEXT('\n'); bf->cp--)
				;
			if (bf->cp > 0) {
				bf->cp++;
			}
#else
			// �\���s��
			bf->cp = line_get(bf, i);
#endif
		}
		InvalidateRect(hWnd, NULL, FALSE);
		break;

	case VK_END:
		if (GetKeyState(VK_CONTROL) < 0) {
			// �S�̖̂���
			bf->cp = BUF_LEN;
		} else {
#if 0
			// �_���s��
			for (; bf->cp < BUF_LEN && *(index_to_char(bf, bf->cp)) != TEXT('\r') && *(index_to_char(bf, bf->cp)) != TEXT('\n'); bf->cp++)
				;
#else
			// �\���s��
			// �ŏI�s�Ȃ�
			if ( ++i == bf->line_len ){
				// �S�̖̂���
				bf->cp = BUF_LEN;
			}else{
				// �ŏI�s�ȊO�Ȃ玟�̍s���̈�O
				bf->cp = line_get(bf, i )-1;
				
				//// �_���s���Ȃ�����ꕶ���O
				//if ( *(index_to_char(bf, bf->cp)) == TEXT('\n') ){
				//	bf->cp --;
				//}
			}
#endif
		}
		InvalidateRect(hWnd, NULL, FALSE);
		break;

	case VK_PRIOR:
		// Page UP
		if (GetCaretPos(&pt) == FALSE) {
			break;
		}
		if (bf->cpx == 0) {
			bf->cpx = pt.x;
		}
		bf->cp = caret_point_to_caret(bf, bf->cpx, pt.y - (rect.bottom - rect.top));
		if (GetKeyState(VK_SHIFT) >= 0) {
			bf->sp = bf->cp;
		}
		SendMessage(hWnd, WM_VSCROLL, SB_PAGEUP, 0);
		break;

	case VK_NEXT:
		// Page Down
		if (GetCaretPos(&pt) == FALSE) {
			break;
		}
		if (bf->cpx == 0) {
			bf->cpx = pt.x;
		}
		bf->cp = caret_point_to_caret(bf, bf->cpx, pt.y + (rect.bottom - rect.top));
		if (GetKeyState(VK_SHIFT) >= 0) {
			bf->sp = bf->cp;
		}
		SendMessage(hWnd, WM_VSCROLL, SB_PAGEDOWN, 0);
		break;

	case VK_UP:
		if (--i < 0) {
			if (GetKeyState(VK_SHIFT) >= 0) {
				bf->sp = bf->cp;
			}
			break;
		}
		if (GetCaretPos(&pt) == FALSE) {
			break;
		}
		if (bf->cpx == 0) {
			bf->cpx = pt.x;
		}
		bf->cp = caret_point_to_caret(bf, bf->cpx, pt.y - bf->font_height);
		break;

	case VK_DOWN:
		if (++i > bf->line_len - 1) {
			if (GetKeyState(VK_SHIFT) >= 0) {
				bf->sp = bf->cp;
			}
			break;
		}
		if (GetCaretPos(&pt) == FALSE) {
			break;
		}
		if (bf->cpx == 0) {
			bf->cpx = pt.x;
		}
		bf->cp = caret_point_to_caret(bf, bf->cpx, pt.y + bf->font_height);
		break;

	case VK_LEFT:
		bf->cpx = 0;
		if (bf->cp != bf->sp && GetKeyState(VK_SHIFT) >= 0) {
			// �I������
			if (bf->cp > bf->sp) {
				SWAP(bf->cp, bf->sp);
			}
			bf->sp = bf->cp;
			break;
		}
		if (bf->cp == line_get(bf, i) && --i < 0) {
			break;
		}
		for (j = line_get(bf, i); j < line_get(bf, i + 1) && j < bf->cp; j++) {
			t = j;
			p = index_to_char(bf, j);
			if (*p == TEXT('\r') /*|| *p == TEXT('\n')*/) {
				break;
			}
#ifndef UNICODE
			if (is_lead_byte(bf, p) == TRUE) {
				p = char_next(bf, p);
				j++;
			}
#endif
		}
		bf->cp = t;
		break;

	case VK_RIGHT:
		bf->cpx = 0;
		if (bf->cp != bf->sp && GetKeyState(VK_SHIFT) >= 0) {
			// �I������
			if (bf->cp < bf->sp) {
				SWAP(bf->cp, bf->sp);
			}
			bf->sp = bf->cp;
			break;
		}
		if (bf->cp >= BUF_LEN) {
			break;
		}
/*		if ((bf->cp + 1) < BUF_LEN &&
			*(index_to_char(bf, bf->cp)) == TEXT('\r') && *(index_to_char(bf, bf->cp + 1)) == TEXT('\n')) {
			bf->cp += 2;
		} else*/ if (*(index_to_char(bf, bf->cp)) == TEXT('\r') || *(index_to_char(bf, bf->cp)) == TEXT('\n')) {
			bf->cp++;
		} else {
#ifdef UNICODE
			bf->cp++;
#else
			if (is_lead_byte(bf, index_to_char(bf, bf->cp)) == TRUE) {
				bf->cp += 2;
			} else {
				bf->cp++;
			}
#endif
		}
		break;
	}
	if (GetKeyState(VK_SHIFT) >= 0) {
		bf->sp = bf->cp;
	}
	if (oldsp != bf->sp) {
		line_refresh(hWnd, bf, oldcp, oldsp);
	}
	line_refresh(hWnd, bf, oldcp, bf->cp);
	ensure_visible(hWnd, bf);
}



/*
 * nedit_proc - Edit�E�B���h�E�v���V�[�W��
 */
static LRESULT CALLBACK nedit_proc(const HWND hWnd, const UINT msg, const WPARAM wParam, const LPARAM lParam)
{
	BUFFER *bf;
	HDC hdc;
	RECT rect;
	TCHAR in[3];
	DWORD cp, sp;
	int len;
	int i, j;
#ifdef OP_XP_STYLE
	static FARPROC _OpenThemeData;
	static FARPROC _CloseThemeData;
	static FARPROC _DrawThemeBackground;
#endif	// OP_XP_STYLE
	HIMC hIMC;
	COMPOSITIONFORM cf;
	POINT pt;
//	BOOL ret;

	try {
		switch (msg) {
		case WM_CREATE:
			if ((bf = (BUFFER*)mem_calloc(NULL,sizeof(BUFFER))) == NULL) {
				return -1;
			}
			// �������}�b�s���O�̏�����
			for( i=0;i<MAPNUM;i++ ){
				bf->hMmap[i]=NULL;
				bf->pMmap[i]=NULL;
			}

	#ifdef OP_XP_STYLE
			// XP
			if ((bf->hModThemes = LoadLibrary(TEXT("uxtheme.dll"))) != NULL) {
				if (_OpenThemeData == NULL) {
					_OpenThemeData = GetProcAddress(bf->hModThemes, "OpenThemeData");
				}
				if (_OpenThemeData != NULL) {
					bf->hTheme = (HTHEME)_OpenThemeData(hWnd, L"Edit");
				}
			}
	#endif	// OP_XP_STYLE

			bf->id = (int)((LPCREATESTRUCT)lParam)->hMenu;

			i = GetWindowLong(hWnd, GWL_STYLE);
			bf->tab_stop = TAB_STOP;
			bf->left_margin = bf->top_margin = bf->right_margin = bf->bottom_margin = 1;
			bf->spacing = 0;
			bf->line_max = LINE_MAX;
			bf->wordwrap = (i & WS_HSCROLL) ? FALSE : TRUE;
			bf->insert_mode = TRUE;
			bf->lock = (i & ES_READONLY) ? TRUE : FALSE;
			bf->no_hide_sel = (i & ES_NOHIDESEL) ? TRUE : FALSE;
			bf->lowercase = (i & ES_LOWERCASE) ? TRUE : FALSE;
			bf->uppercase = (i & ES_UPPERCASE) ? TRUE : FALSE;

			bf->wordwrap_pos = 0;

			if (string_init(bf) == FALSE) {
				return -1;
			}
			// �o�b�t�@�̏�����
			bf->buf_size = RESERVE_BUF;
			if ((bf->buf = (TCHAR*)mem_alloc(bf,sizeof(TCHAR) * bf->buf_size)) == NULL) {
				return -1;
			}
			*bf->buf = TEXT('\0');

			// �F���̏�����
			draw_setcolor( bf , NEDIT_COLOR_HIGHLIGHT , -1 );
			draw_setcolor( bf , NEDIT_COLOR_HIGHLIGHTTEXT , -1 );
			draw_setcolor( bf , NEDIT_COLOR_WINDOW , -1 );
			draw_setcolor( bf , NEDIT_COLOR_WINDOWTEXT , -1 );
			draw_setcolor( bf , NEDIT_COLOR_UNDERLINE , -1 );

			// �`����̏�����
			hdc = GetDC(hWnd);
			bf->mdc = CreateCompatibleDC(hdc);
			ReleaseDC(hWnd, hdc);
			draw_init(hWnd, bf);

	//		SetMapMode(bf->mdc, MM_TEXT);
			SetTextCharacterExtra(bf->mdc, 0);
	//		SetTextJustification(bf->mdc, 0, 0);
			SetTextAlign(bf->mdc, TA_TOP | TA_LEFT);
			SetBkMode(bf->mdc, TRANSPARENT);

			bf->caret_width = GetSystemMetrics(SM_CXBORDER)*2;

			// buffer info to window long
			SetWindowLong(hWnd, GWL_USERDATA, (LPARAM)bf);
			SendMessage(hWnd, WM_REFLECT, 0, 0);

			// �R���|�W�V�����E�B���h�E����
			bf->hCompWnd = 
				CreateWindowEx(0, IMECOMP_WND_CLASS, TEXT(""),
					WS_VISIBLE | WS_CHILD ,
					0, 0, 0, 0,
					hWnd, NULL, NULL, NULL);

			// ����������̐ݒ�
			if (((LPCREATESTRUCT)lParam)->lpszName != NULL) {
				SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)((LPCREATESTRUCT)lParam)->lpszName);
			}

			bf->flags = 0;

			break;

		case WM_DESTROY:
			if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) != NULL) {
				SetWindowLong(hWnd, GWL_USERDATA, (LPARAM)0);

	#ifdef OP_XP_STYLE
				// XP
				if (bf->hTheme != NULL) {
					if (_CloseThemeData == NULL) {
						_CloseThemeData = GetProcAddress(bf->hModThemes, "CloseThemeData");
					}
					if (_CloseThemeData != NULL) {
						_CloseThemeData(bf->hTheme);
					}
				}
				if (bf->hModThemes != NULL) {
					FreeLibrary(bf->hModThemes);
				}
	#endif	// OP_XP_STYLE

				DestroyWindow( bf->hCompWnd );
				draw_free(bf);
				if (bf->hfont != NULL) {
					SelectObject(bf->mdc, bf->ret_font);
					DeleteObject(bf->hfont);
				}
				DeleteDC(bf->mdc);
				mem_free(bf,(void**)&bf->buf);
				mem_free(bf,(void**)&bf->input_buf);
				mem_free(bf,(void**)&bf->line);
				undo_free(bf, 0);
				mem_free(bf,(void**)&bf->undo);
				mem_free(bf,(void**)&bf);
			}
			if (GetFocus() == hWnd) {
				DestroyCaret();
			}
			return DefWindowProc(hWnd, msg, wParam, lParam);

		case WM_SETFOCUS:
			if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL) {
				break;
			}
			CreateCaret(hWnd, NULL, bf->caret_width, bf->font_height);
			ShowCaret(hWnd);
			line_refresh(hWnd, bf, bf->cp, bf->sp);

			if ( IS_FLAG_OFF( NEDIT_FLAGS_NOTFOCUSIME ) ){
				// IME��L���ɂ���
				hIMC = ImmGetContext(hWnd);
				ImmSetOpenStatus( hIMC,TRUE	);
				ImmReleaseContext(hWnd, hIMC);
			}
			// �e�E�B���h�E�ɒʒm
			notify_message(hWnd, bf, EN_SETFOCUS);
			break;

		case WM_KILLFOCUS:
			if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL) {
				break;
			}
			if ( IS_FLAG_OFF( NEDIT_FLAGS_NOTFOCUSIME ) ){
				// IME�𖳌��ɂ���
				hIMC = ImmGetContext(hWnd);
				ImmSetOpenStatus( hIMC,FALSE	);
				ImmReleaseContext(hWnd, hIMC);
			}
			HideCaret(hWnd);
			DestroyCaret();
			line_refresh(hWnd, bf, bf->cp, bf->sp);
			// �e�E�B���h�E�ɒʒm
			notify_message(hWnd, bf, EN_KILLFOCUS);
			break;

		case WM_SIZE:
			if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL) {
				break;
			}
			if (wParam == SIZE_MINIMIZED) {
				break;
			}
			GetClientRect(hWnd, &rect);
			bf->width = (rect.right - rect.left);
			if ( bf->width > 0 ){
				if (bf->wordwrap == TRUE ) {
					// �s���̍Đݒ�
					bf->line_len = 0;
					line_set_info(bf);
				}
				set_scrollbar(hWnd, bf);

				draw_free(bf);
				draw_init(hWnd, bf);

				InvalidateRect(hWnd, NULL, FALSE);

				// IME�̐ݒ�
				GetCaretPos(&pt);
				SetWindowPos( bf->hCompWnd , NULL , pt.x , pt.y ,0,0, SWP_NOSIZE|SWP_NOZORDER );
			}
//			// �g��Ȃ�����ImmCompositionWindow�̐ݒ���s��
//			hIMC = ImmGetContext(hWnd);
//			// �ʒu�̐ݒ�
//			cf.dwStyle = CFS_POINT | CFS_RECT;
//			cf.ptCurrentPos.x = pt.x;
//			cf.ptCurrentPos.y = pt.y + (bf->spacing / 2);
//			get_edit_rect(hWnd, bf, &cf.rcArea);
//			ImmSetCompositionWindow(hIMC, &cf);
//			ImmReleaseContext(hWnd, hIMC);
			break;

		case WM_GETDLGCODE:
			return DLGC_WANTALLKEYS;

		case WM_HSCROLL:
			if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL) {
				break;
			}
			get_edit_rect(hWnd, bf, &rect);
			i = bf->pos_x;
			switch ((int)LOWORD(wParam)) {
			case SB_TOP:
				bf->pos_x = 0;
				break;

			case SB_BOTTOM:
				bf->pos_x = bf->max_x;
				break;

			case SB_LINELEFT:
				bf->pos_x = (bf->pos_x > 0) ? bf->pos_x - 1 : 0;
				break;

			case SB_LINERIGHT:
				bf->pos_x = (bf->pos_x < bf->max_x) ? bf->pos_x + 1 : bf->max_x;
				break;

			case SB_PAGELEFT:
				bf->pos_x = (bf->pos_x - ((rect.right - rect.left) / bf->char_width) > 0) ?
					bf->pos_x - ((rect.right - rect.left) / bf->char_width) : 0;
				break;

			case SB_PAGERIGHT:
				bf->pos_x = (bf->pos_x + ((rect.right - rect.left) / bf->char_width) < bf->max_x) ?
					bf->pos_x + ((rect.right - rect.left) / bf->char_width) : bf->max_x;
				break;

			case SB_THUMBPOSITION:
			case SB_THUMBTRACK:
				{
					SCROLLINFO si;

					ZeroMemory(&si, sizeof(SCROLLINFO));
					si.cbSize = sizeof(SCROLLINFO);
					si.fMask = SIF_ALL;
					GetScrollInfo(hWnd, SB_HORZ, &si);
					bf->pos_x = si.nTrackPos;
				}
				break;
			}
			switch ((int)LOWORD(wParam)) {
			case SB_TOP:
			case SB_BOTTOM:
			case SB_LINELEFT:
			case SB_LINERIGHT:
			case SB_PAGELEFT:
			case SB_PAGERIGHT:
				if (i - bf->pos_x != 0) {
					// �e�E�B���h�E�ɒʒm
					notify_message(hWnd, bf, EN_HSCROLL);
				}
				break;
			}
			SetScrollPos(hWnd, SB_HORZ, bf->pos_x, TRUE);
			ScrollWindowEx(hWnd, (i - bf->pos_x) * bf->char_width, 0, NULL, &rect, NULL, NULL, SW_INVALIDATE | SW_ERASE);

			GetCaretPos(&pt);
			pt.x += (i - bf->pos_x) * bf->char_width;
			SetCaretPos(pt.x,pt.y);

			ensure_inrect(hWnd, bf);
			break;

		case WM_VSCROLL:
			if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL) {
				break;
			}
			get_edit_rect(hWnd, bf, &rect);
			rect.left = 0;

			i = bf->pos_y;
			switch ((int)LOWORD(wParam)) {
			case SB_TOP:
				bf->pos_y = 0;
				break;

			case SB_BOTTOM:
				bf->pos_y = bf->max_y;
				break;

			case SB_LINEUP:
				bf->pos_y = (bf->pos_y > 0) ? bf->pos_y - 1 : 0;
				break;

			case SB_LINEDOWN:
				bf->pos_y = (bf->pos_y < bf->max_y) ? bf->pos_y + 1 : bf->max_y;
				break;

			case SB_PAGEUP:
				bf->pos_y = (bf->pos_y - (((rect.bottom - rect.top) / bf->font_height) - 1) > 0) ?
					bf->pos_y - (((rect.bottom - rect.top) / bf->font_height) - 1) : 0;
				break;

			case SB_PAGEDOWN:
				bf->pos_y = (bf->pos_y + (((rect.bottom - rect.top) / bf->font_height) - 1) < bf->max_y) ?
					bf->pos_y + (((rect.bottom - rect.top) / bf->font_height) - 1) : bf->max_y;
				break;

			case SB_THUMBPOSITION:
			case SB_THUMBTRACK:
				{
					SCROLLINFO si;

					ZeroMemory(&si, sizeof(SCROLLINFO));
					si.cbSize = sizeof(SCROLLINFO);
					si.fMask = SIF_ALL;
					GetScrollInfo(hWnd, SB_VERT, &si);
					bf->pos_y = si.nTrackPos;
					bf->pos_y = (bf->pos_y < bf->max_y) ? bf->pos_y : bf->max_y;
				}
				break;
			}
			switch ((int)LOWORD(wParam)) {
			case SB_TOP:
			case SB_BOTTOM:
			case SB_LINEUP:
			case SB_LINEDOWN:
			case SB_PAGEUP:
			case SB_PAGEDOWN:
				if (i - bf->pos_y != 0) {
					// �e�E�B���h�E�ɒʒm
					notify_message(hWnd, bf, EN_VSCROLL);
				}
				break;
			}
			SetScrollPos(hWnd, SB_VERT, bf->pos_y, TRUE);
			ScrollWindowEx(hWnd, 0, (i - bf->pos_y) * bf->font_height, NULL, &rect, NULL, NULL, SW_INVALIDATE|SW_ERASE);

			GetCaretPos(&pt);
			pt.y += (i - bf->pos_y) * bf->font_height;
			SetCaretPos(pt.x,pt.y);

			ensure_inrect(hWnd, bf);
			break;

		case WM_KEYDOWN:
			if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL) {
				break;
			}
			switch (wParam) {
	#if 0
			case VK_APPS:
				SendMessage(hWnd, WM_CONTEXTMENU, 0, 0);
				break;
	#endif
			case VK_INSERT:
				if (GetKeyState(VK_CONTROL) < 0) {
					// �R�s�[
					SendMessage(hWnd, WM_COPY, 0, 0);
				} else if (GetKeyState(VK_SHIFT) < 0) {
					// �\��t��
					SendMessage(hWnd, WM_PASTE, 0, 0);
				} else {
					// ���̓��[�h�ؑ�
					bf->insert_mode = !bf->insert_mode;

					DestroyCaret();
					CreateCaret(hWnd, NULL, bf->caret_width, bf->font_height);
					ShowCaret(hWnd);
					line_refresh(hWnd, bf, bf->cp, bf->cp);
				}
				break;

			case VK_DELETE:
				if (bf->lock == TRUE) {
					break;
				}
				if (GetKeyState(VK_SHIFT) < 0) {
					// �؂���
					SendMessage(hWnd, WM_CUT, 0, 0);
				} else {
					// �폜
					string_delete_char(hWnd, bf, bf->cp);
				}
				break;

			case VK_BACK:
				
				if ( IS_FLAG_OFF( NEDIT_FLAGS_BSWMCHAR ) ){		// PPC��KEYDOWN��BS�L�[������
					if (bf->lock == TRUE) {
						break;
					}
					if (bf->cp == bf->sp) {
						if (bf->cp <= 0) {
							break;
						}
						caret_move(hWnd, bf, VK_LEFT);
					}
					string_delete_char(hWnd, bf, bf->cp);
				}
				break;

			case 'C':
				if (GetKeyState(VK_CONTROL) < 0) {
					SendMessage(hWnd, WM_COPY, 0, 0);
				}
				break;

			case 'X':
				if (GetKeyState(VK_CONTROL) < 0) {
					SendMessage(hWnd, WM_CUT, 0, 0);
				}
				break;

			case 'V':
				if (GetKeyState(VK_CONTROL) < 0) {
					SendMessage(hWnd, WM_PASTE, 0, 0);
				}
				break;

			case 'Z':
				if (GetKeyState(VK_CONTROL) < 0 && GetKeyState(VK_SHIFT) < 0) {
					// ��蒼��
					SendMessage(hWnd, EM_REDO, 0, 0);
				} else if (GetKeyState(VK_CONTROL) < 0) {
					// ���ɖ߂�
					SendMessage(hWnd, EM_UNDO, 0, 0);
				}
				break;

			case 'Y':
				if (GetKeyState(VK_CONTROL) < 0) {
					SendMessage(hWnd, EM_REDO, 0, 0);
				}
				break;

			case 'L':
				if (GetKeyState(VK_CONTROL) < 0) {
					SendMessage(hWnd, EM_SCROLLCARET , 1, 0);
				}
				break;

			case VK_HOME:
			case VK_END:
			case VK_PRIOR:
			case VK_NEXT:
			case VK_UP:
			case VK_DOWN:
			case VK_LEFT:
			case VK_RIGHT:
				string_flush(bf, TRUE);
				caret_move(hWnd, bf, wParam);
				break;
			}
			break;

		case WM_CHAR:
			if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL || bf->lock == TRUE) {
				break;
			}
			if (GetKeyState(VK_CONTROL) < 0) {
				break;
			}
			switch (wParam) {
			case VK_RETURN:
				// ���s
//				string_insert(hWnd, bf, TEXT("\r\n"), 2, TRUE);
				string_insert(hWnd, bf, TEXT("\r"), 1, TRUE);
				string_flush(bf, TRUE);
				break;

			case VK_BACK:
				if ( IS_FLAG_ON( NEDIT_FLAGS_BSWMCHAR ) ){		// Smartphone��CHAR��BS�L�[������
					if (bf->lock == TRUE) {
						break;
					}
					if (bf->cp == bf->sp) {
						if (bf->cp <= 0) {
							break;
						}
						caret_move(hWnd, bf, VK_LEFT);
					}
					string_delete_char(hWnd, bf, bf->cp);
				}
				break;

			case VK_ESCAPE:
				break;

			default:
				in[0] = wParam;
				string_insert(hWnd, bf, in, 1, bf->insert_mode);
				break;
			}
			break;

		case WM_IME_CHAR:
			if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL || bf->lock == TRUE) {
				break;
			}
			if (GetKeyState(VK_CONTROL) < 0) {
				break;
			}
	#ifdef UNICODE
			in[0] = wParam;
			string_insert(hWnd, bf, in, 1, bf->insert_mode);
	#else
			in[0] = wParam >> 8;
			in[1] = wParam & 0xFF;
			string_insert(hWnd, bf, in, 2, bf->insert_mode);
	#endif
			break;

		case WM_IME_STARTCOMPOSITION:
			if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) != NULL) {
				ensure_visible(hWnd, bf);

				if (bf->hfont != NULL) {
					// �t�H���g�̐ݒ�
					SendMessage( bf->hCompWnd , WM_SETFONT ,(WPARAM)bf->hfont , 0 );
				}
				// �ʒu�̐ݒ�
				GetCaretPos(&pt);
				SetWindowPos( bf->hCompWnd , NULL , pt.x , pt.y ,0,0, SWP_NOSIZE|SWP_NOZORDER );

//				// �ʒu�̐ݒ�
//				hIMC = ImmGetContext(hWnd);
//				cf.dwStyle = CFS_POINT | CFS_RECT;
//				cf.ptCurrentPos.x = pt.x;
//				cf.ptCurrentPos.y = pt.y + (bf->spacing / 2);
//				get_edit_rect(hWnd, bf, &cf.rcArea);
//				ImmSetCompositionWindow(hIMC, &cf);
//				ImmReleaseContext(hWnd, hIMC);

				HideCaret(hWnd);
			}
			return 0;

		case WM_IME_COMPOSITION:
			if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) != NULL ) {
				if ( bf->lock == FALSE && (lParam & GCS_RESULTSTR)) {
					TCHAR *buf;

					SetWindowPos( bf->hCompWnd , NULL , 0, 0 ,0,0, SWP_NOZORDER );

					// �m�蕶������o�b�t�@�ɒǉ�
					hIMC = ImmGetContext(hWnd);
					len = ImmGetCompositionString(hIMC, GCS_RESULTSTR, NULL, 0);
					if (  len > 0 ){
						buf = (TCHAR*)mem_calloc(bf,len + sizeof(TCHAR));
						if (buf != NULL) {
							ImmGetCompositionString(hIMC, GCS_RESULTSTR, buf, len);
							SendMessage( bf->hCompWnd , WM_IMECOMP_COPMSTR , (WPARAM)(hIMC) , -1 );
//							SendMessage( bf->hCompWnd , WM_IMECOMP_ENDCOMP , 0 , 0 );
							string_insert(hWnd, bf, buf, len / sizeof(TCHAR), bf->insert_mode);
							UpdateWindow(hWnd);
							mem_free(bf,(void**)&buf);
						}
					}
//					// �ʒu�̐ݒ�
//					GetCaretPos(&pt);
//					cf.dwStyle = CFS_POINT | CFS_RECT;
//					cf.ptCurrentPos.x = pt.x;
//					cf.ptCurrentPos.y = pt.y + (bf->spacing / 2);
//					get_edit_rect(hWnd, bf, &cf.rcArea);
//					ImmSetCompositionWindow(hIMC, &cf);
					ImmReleaseContext(hWnd, hIMC);
				}
				if ( lParam & GCS_COMPSTR ){
					CANDIDATEFORM Candidate;
					LONG pos;
					hIMC = ImmGetContext(hWnd);

					// �L�����b�g�̏ꏊ
					POINT	pt;
					//GetCaretPos(&pt);


					SendMessage( bf->hCompWnd , WM_IMECOMP_COPMSTR , (WPARAM)(hIMC) , lParam );

					//				SetWindowPos( bf->hCompWnd , NULL , pt.x , pt.y ,0,0, SWP_NOSIZE|SWP_NOZORDER );

					SendMessage( bf->hCompWnd , WM_IMECOMP_SETCOMPPOS , 0, 0);

					pos = SendMessage( bf->hCompWnd , WM_IMECOMP_GETCANDPOS , 0,0 );
	//				pt.x += LOWORD(pos);
	//				pt.y += HIWORD(pos);
					pt.x = LOWORD(pos);
					pt.y = HIWORD(pos);
					Candidate.dwIndex = 0;
					Candidate.dwStyle = CFS_CANDIDATEPOS;
					Candidate.ptCurrentPos = pt;
					ImmSetCandidateWindow(hIMC, &Candidate);

					
					// �g��Ȃ�����ImmCompositionWindow�̐ݒ���s��
					// �ʒu�̐ݒ�
					cf.dwStyle = CFS_POINT ;
					cf.ptCurrentPos.x = pt.x;
					cf.ptCurrentPos.y = pt.y - bf->font_height ;
					ImmSetCompositionWindow(hIMC, &cf);

					GetCaretPos(&pt);
					Candidate.dwIndex = 0;
					Candidate.dwStyle = CFS_EXCLUDE;
					CRect rect( pt.x , pt.y  ,pt.x+bf->caret_width, pt.y+ bf->font_height );
					Candidate.rcArea = rect;
					ImmSetCandidateWindow(hIMC, &Candidate);

					ImmReleaseContext(hWnd, hIMC);
					UpdateWindow(hWnd);
				}
			}
	//				break;
			return 0;
	//		return DefWindowProc(hWnd, msg, wParam, lParam);

		case WM_IME_ENDCOMPOSITION:
			if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) != NULL ) {
				SendMessage( bf->hCompWnd , WM_IMECOMP_ENDCOMP , 0 , 0 );
			}
			UpdateWindow(hWnd);
			ShowCaret(hWnd);
			return 0;
	//		return DefWindowProc(hWnd, msg, wParam, lParam);

		case WM_IME_SETCONTEXT:
			return DefWindowProc(hWnd, msg, wParam, lParam & ~ISC_SHOWUICOMPOSITIONWINDOW );


		case WM_MOUSEMOVE:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
			if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL) {
				break;
			}
			cp = bf->cp;
			sp = bf->sp;
			SetCursor(LoadCursor(0, IDC_IBEAM));
			if (msg == WM_MOUSEMOVE) {
				if (!(wParam & MK_LBUTTON) || bf->mousedown == FALSE) {
					break;
				}
			} else if (msg == WM_LBUTTONDOWN) {
				string_flush(bf, TRUE);
				SetCapture(hWnd);
				bf->mousedown = TRUE;
				bf->wordselect = FALSE;
				bf->cpx = 0;
			} else if (msg == WM_LBUTTONUP) {
				if (bf->mousedown == FALSE) {
					break;
				}
				ReleaseCapture();
				bf->mousedown = FALSE;
				bf->cpx = 0;
			}
			SetFocus(hWnd);

			{
				DWORD newcp = caret_point_to_caret(bf, (short)LOWORD(lParam), (short)HIWORD(lParam));
				if ( bf->wordselect == TRUE ){
					DWORD cp = newcp;
					DWORD sp = newcp;
					caret_get_token(bf,&cp,&sp);	// �J�[�\���ʒu�̒P����擾

					if ( cp <= bf->wordselect_sp ){
						bf->sp = bf->wordselect_cp;
						bf->cp = sp;
					}else if ( bf->wordselect_cp <= sp ){
						bf->sp = bf->wordselect_sp;
						bf->cp = cp;
					}else{
						bf->sp = sp;
						bf->cp = cp;
					}
				}else{
					bf->cp = newcp;
				}
			}
			ensure_visible(hWnd, bf);
			if (msg == WM_LBUTTONDOWN && GetKeyState(VK_SHIFT) >= 0) {
				bf->sp = bf->cp;
			}
			if (sp != bf->sp) {
				line_refresh(hWnd, bf, cp, sp);
			}
			line_refresh(hWnd, bf, cp, bf->cp);
			break;

		case WM_RBUTTONDOWN:
			if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL) {
				break;
			}
			string_flush(bf, TRUE);
			SetFocus(hWnd);
			bf->cpx = 0;
			cp = caret_point_to_caret(bf, (short)LOWORD(lParam), (short)HIWORD(lParam));
			if (!(bf->cp >= cp && bf->sp <= cp || bf->sp >= cp && bf->cp <= cp)) {
				bf->cp = bf->sp = cp;
				ensure_visible(hWnd, bf);
				InvalidateRect(hWnd, NULL, FALSE);
			}
			break;

		case WM_RBUTTONUP:
			SendMessage(hWnd, WM_CONTEXTMENU, (WPARAM)hWnd, lParam);
			break;

		case WM_LBUTTONDBLCLK:
			if (!(wParam & MK_LBUTTON)) {
				break;
			}
			if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL) {
				break;
			}
			string_flush(bf, TRUE);
			SetFocus(hWnd);
			SetCapture(hWnd);
			bf->mousedown = TRUE;
			bf->wordselect = TRUE;
			bf->cpx = 0;
			cp = bf->cp;
			bf->sp = bf->cp;
			sp = caret_point_to_caret(bf, (short)LOWORD(lParam), (short)HIWORD(lParam));
			// ������I��
			caret_get_token(bf,&bf->cp,&bf->sp);
			bf->wordselect_cp = bf->cp;		// ����ʒu���L��
			bf->wordselect_sp = bf->sp;

			ensure_visible(hWnd, bf);
			line_refresh(hWnd, bf, cp, sp);
			line_refresh(hWnd, bf, bf->cp, bf->sp);
			break;
		case WM_MOUSEWHEEL:
			for (i = 0; i < 3; i++) {
				SendMessage(hWnd, WM_VSCROLL, ((short)HIWORD(wParam) > 0) ? SB_LINEUP : SB_LINEDOWN, 0);
			}
			break;
		case WM_PAINT:
			if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) != NULL) {
				PAINTSTRUCT ps;
				HBRUSH hBrush;

				POINT	CaretPos;
				GetCaretPos( &CaretPos );
				hdc = BeginPaint(hWnd, &ps);
				GetClientRect(hWnd, &rect);
				SetRect(&rect, ps.rcPaint.left, 0, ps.rcPaint.right, bf->font_height);

				i = GetWindowLong(hWnd, GWL_STYLE);
				bf->no_hide_sel = (i & ES_NOHIDESEL) ? TRUE : FALSE;

				// caret pos
				j = -1;
				HWND h = GetFocus();
				if (GetFocus() == hWnd) {
					j = index_to_line(bf, bf->cp);
				}

				bf->sel = FALSE;
				SetTextColor(bf->mdc, draw_getcolor(bf , NEDIT_COLOR_WINDOWTEXT));
				SetBkColor(bf->mdc, draw_getcolor(bf , NEDIT_COLOR_WINDOW));

				hBrush = CreateSolidBrush(draw_getcolor(bf , NEDIT_COLOR_WINDOW));
				i = bf->pos_y + (ps.rcPaint.top / bf->font_height) - 1;
				for (; i < bf->line_len && i < bf->pos_y + (ps.rcPaint.bottom / bf->font_height) + 1; i++) {
					// 1�s�`��
					FillRect(bf->mdc, &rect, hBrush);
					draw_line(hWnd, bf->mdc, bf, i, ps.rcPaint.left, ps.rcPaint.right, (i==j) );
					BitBlt(hdc,
						ps.rcPaint.left, (i - bf->pos_y) * bf->font_height + bf->top_margin,
						ps.rcPaint.right, bf->font_height,
						bf->mdc, ps.rcPaint.left, 0, SRCCOPY);

					if (i == j) {
						// set caret
						if (bf->insert_mode == FALSE) {
							caret_set_size(hWnd, bf);
						}
						len = caret_char_to_caret(bf->mdc, bf, j, bf->cp);
						//SetCaretPos(len, (j - bf->pos_y) * bf->font_height + bf->top_margin);
						CaretPos.x = len;
						CaretPos.y = (j - bf->pos_y) * bf->font_height + bf->top_margin;

						j = -1;
					}
				}
				if ((i - bf->pos_y) * bf->font_height + bf->top_margin < ps.rcPaint.bottom) {
					// �����̗]���`��
					SetRect(&rect,
						ps.rcPaint.left, (i - bf->pos_y) * bf->font_height + bf->top_margin,
						ps.rcPaint.right, ps.rcPaint.bottom);
					FillRect(hdc, &rect, hBrush);
				}
				// �}�[�W���̈�̃N���A
				FillRgn(hdc, bf->hrgn, hBrush);
				DeleteObject(hBrush);

				if (j != -1 && GetFocus() == hWnd) {
					len = caret_char_to_caret(bf->mdc, bf, j, bf->cp);
	//				SetCaretPos(len, (j - bf->pos_y) * bf->font_height + bf->top_margin);
					CaretPos.x = len;
					CaretPos.y = (j - bf->pos_y) * bf->font_height + bf->top_margin;

				}
				EndPaint(hWnd, &ps);
				if ( GetFocus() == hWnd ){
					DestroyCaret();
					CreateCaret(hWnd, NULL, bf->caret_width, bf->font_height);
					SetCaretPos(CaretPos.x,CaretPos.y);
					
					if ( CaretPos.x >= bf->left_margin ){
						ShowCaret(hWnd);
					}
					// IME�ʒu�̐ݒ�
					//GetCaretPos(&pt);
					//SetWindowPos( bf->hCompWnd , NULL , pt.x , pt.y ,0,0, SWP_NOSIZE|SWP_NOZORDER );
					SendMessage( bf->hCompWnd , WM_IMECOMP_SETCOMPPOS , 0, 0);

				}
			}
			break;

	#ifdef OP_XP_STYLE
		case WM_NCPAINT:
			{
				HRGN hrgn;
				DWORD stats;
				RECT clip_rect;

				if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL || bf->hTheme == NULL) {
					return DefWindowProc(hWnd, msg, wParam, lParam);
				}
				// XP�p�̔w�i�`��
				if (_DrawThemeBackground == NULL) {
					_DrawThemeBackground = GetProcAddress(bf->hModThemes, "DrawThemeBackground");
				}
				if (_DrawThemeBackground == NULL) {
					return DefWindowProc(hWnd, msg, wParam, lParam);
				}
				// ��Ԃ̐ݒ�
				if (IsWindowEnabled(hWnd) == 0) {
					stats = ETS_DISABLED;
				} else if (GetFocus() == hWnd) {
					stats = ETS_FOCUSED;
				} else {
					stats = ETS_NORMAL;
				}
				// �E�B���h�E�g�̕`��
				hdc = GetDCEx(hWnd, (HRGN)wParam, DCX_WINDOW | DCX_INTERSECTRGN);
				if (hdc == NULL) {
					hdc = GetWindowDC(hWnd);
				}
				GetWindowRect(hWnd, &rect);
				OffsetRect(&rect, -rect.left, -rect.top);
				ExcludeClipRect(hdc, rect.left + GetSystemMetrics(SM_CXEDGE), rect.top + GetSystemMetrics(SM_CYEDGE),
					rect.right - GetSystemMetrics(SM_CXEDGE), rect.bottom - GetSystemMetrics(SM_CYEDGE));
				clip_rect = rect;
				_DrawThemeBackground(bf->hTheme, hdc, EP_EDITTEXT, stats, &rect, &clip_rect);
				ReleaseDC(hWnd, hdc);

				// �X�N���[���o�[�̕`��
				GetWindowRect(hWnd, (LPRECT)&rect);
				hrgn = CreateRectRgn(rect.left + GetSystemMetrics(SM_CXEDGE), rect.top + GetSystemMetrics(SM_CYEDGE),
					rect.right - GetSystemMetrics(SM_CXEDGE), rect.bottom - GetSystemMetrics(SM_CYEDGE));
				CombineRgn(hrgn, hrgn, (HRGN)wParam, RGN_AND);
				DefWindowProc(hWnd, WM_NCPAINT, (WPARAM)hrgn, 0);
				DeleteObject(hrgn);
			}
			break;

		case WM_THEMECHANGED:
			if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL || bf->hModThemes == NULL) {
				break;
			}
			// XP�e�[�}�̕ύX
			if (bf->hTheme != NULL) {
				if (_CloseThemeData == NULL) {
					_CloseThemeData = GetProcAddress(bf->hModThemes, "CloseThemeData");
				}
				if (_CloseThemeData != NULL) {
					_CloseThemeData(bf->hTheme);
				}
				bf->hTheme = NULL;
			}
			if (_OpenThemeData == NULL) {
				_OpenThemeData = GetProcAddress(bf->hModThemes, "OpenThemeData");
			}
			if (_OpenThemeData != NULL) {
				bf->hTheme = (HTHEME)_OpenThemeData(hWnd, L"Edit");
			}
			break;
	#endif	// OP_XP_STYLE

		case WM_SETTEXT:
			// �e�L�X�g��ݒ�
			if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL || lParam == 0) {
				break;
			}
			return string_set(hWnd, bf, (TCHAR *)lParam, lstrlen((TCHAR *)lParam));

		case WM_GETTEXT:
			// �e�L�X�g���擾
			if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL || lParam == 0) {
				break;
			}
			string_flush(bf, TRUE);
			_tcsncpy((TCHAR *)lParam, bf->buf, wParam);
			return lstrlen((TCHAR *)lParam);

		case WM_GETTEXTLENGTH:
			// �e�L�X�g�̒������擾
			if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL) {
				break;
			}
			return BUF_LEN;

		case WM_SETFONT:
			// �t�H���g��ݒ�
			{
				LOGFONT lf;

				if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL) {
					break;
				}
				if (bf->hfont != NULL) {
					SelectObject(bf->mdc, bf->ret_font);
					DeleteObject(bf->hfont);
				}
				ZeroMemory(bf->cwidth, sizeof(BYTE) * 0x10000);

				if (GetObject((HGDIOBJ)wParam, sizeof(LOGFONT), &lf) != 0) {
					bf->hfont = CreateFontIndirect((CONST LOGFONT *)&lf);
					bf->ret_font = (HFONT)SelectObject(bf->mdc, bf->hfont);
				}
				SendMessage(hWnd, WM_REFLECT, 0, 0);
				draw_free(bf);
				draw_init(hWnd, bf);
			}
			break;

		case WM_GETFONT:
			// �t�H���g���擾
			if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL) {
				break;
			}
			return (LRESULT)bf->hfont;

		case WM_CLEAR:
			// �폜
			if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL || bf->lock == TRUE) {
				break;
			}
			if (bf->cp != bf->sp) {
				string_delete_char(hWnd, bf, bf->cp);
			}
			break;

		case WM_COPY:
			// �R�s�[
			if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL) {
				break;
			}
			string_flush(bf, TRUE);
			if (bf->cp != bf->sp) {
				if (bf->sp > bf->cp) {
					string_to_clipboard(hWnd, index_to_char(bf, bf->cp), index_to_char(bf, bf->sp));
				} else {
					string_to_clipboard(hWnd, index_to_char(bf, bf->sp), index_to_char(bf, bf->cp));
				}
			}
			break;

		case WM_CUT:
			// �؂���
			if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL || bf->lock == TRUE) {
				break;
			}
			SendMessage(hWnd, WM_COPY, 0, 0);
			SendMessage(hWnd, WM_CLEAR, 0, 0);
			break;

		case WM_PASTE:
			// �\��t��
			if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL || bf->lock == TRUE) {
				break;
			}
	#ifdef UNICODE
			if (IsClipboardFormatAvailable(CF_UNICODETEXT) == 0) {
	#else
			if (IsClipboardFormatAvailable(CF_TEXT) == 0) {
	#endif
				break;
			}
			if (OpenClipboard(hWnd) != 0) {
				HANDLE hclip;
				TCHAR *p;
				HCURSOR old_cursor;

				old_cursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
	#ifdef UNICODE
				hclip = GetClipboardData(CF_UNICODETEXT);
	#else
				hclip = GetClipboardData(CF_TEXT);
	#endif
				int len = 0;
				void	*clipbuf = NULL;
				if ((p = (TCHAR*)GlobalLock(hclip)) != NULL) {
					len = _tcslen(p);
					clipbuf = mem_alloc( bf , len * sizeof(TCHAR)+1 );
					if ( clipbuf!=NULL ){
						// crlf -> cr �ϊ����s���Ȃ���R�s�[
						{
							const TCHAR *src = p;
							TCHAR *dst = (TCHAR*)clipbuf;
							TCHAR c;
							while( c = ( *dst++=*src++ ) ){
								if ( c == _T('\r') ){
									src ++;
								}
							}
						}
						string_insert(hWnd, bf, (TCHAR*)clipbuf, _tcslen((TCHAR*)clipbuf), TRUE);
						mem_free(bf,&clipbuf);
					}
					GlobalUnlock(hclip);
				}
				CloseClipboard();
				SetCursor(old_cursor);
			}
			break;

		case EM_CANUNDO:
			// UNDO�\���擾
			if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL) {
				break;
			}
			return ((bf->undo_len > 0 || bf->ip != NULL || bf->dp != NULL) ? TRUE : FALSE);

		case EM_CANREDO:
			// REDO�\���擾
			if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL) {
				break;
			}
			return (((bf->undo + bf->undo_len)->type != 0) ? TRUE : FALSE);

		case EM_EMPTYUNDOBUFFER:
			// UNDO�o�b�t�@���N���A
			if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL) {
				break;
			}
			undo_free(bf, 0);
			mem_free(bf,(void**)&bf->undo);

			bf->undo_size = RESERVE_UNDO;
			if ((bf->undo = (UNDO*)mem_calloc(bf,sizeof(UNDO) * bf->undo_size)) == NULL) {
				return FALSE;
			}
			bf->undo_len = 0;
			break;

		case EM_GETFIRSTVISIBLELINE:
			// ��ԏ�ɕ\������Ă���s�ԍ����擾
			if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL) {
				break;
			}
			return bf->pos_y;

		case EM_GETLINE:
			// 1�s�̕�������擾
			if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL || lParam == 0) {
				break;
			}
			if (wParam < 0 || wParam >= (WPARAM)bf->line_len) {
				return 0;
			}
			CopyMemory((TCHAR *)lParam,
				index_to_char(bf, line_get(bf, wParam)),
				sizeof(TCHAR) * line_get_length(bf, line_get(bf, wParam)));
			return line_get_length(bf, line_get(bf, wParam));

		case EM_GETLINECOUNT:
			// �s���擾
			if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL) {
				break;
			}
			return bf->line_len;

		case EM_GETMODIFY:
			// �ύX�t���O���擾
			if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL) {
				break;
			}
			return bf->modified;

		case EM_GETRECT:
			// �`��̈�̎擾
			if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL || lParam == 0) {
				break;
			}
			get_edit_rect(hWnd, bf, (RECT *)lParam);
			break;

		case EM_GETSEL:
			// �I������Ă���C���f�b�N�X���擾
			if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL) {
				break;
			}
			i = ((bf->sp < bf->cp) ? bf->sp : bf->cp);
			j = ((bf->sp < bf->cp) ? bf->cp : bf->sp);
			if (wParam != 0) {
				*((LPDWORD)wParam) = i;
			}
			if (lParam != 0) {
				*((LPDWORD)lParam) = j;
			}
			return MAKELPARAM(i, j);

		case EM_LIMITTEXT:
			// ���͕���������
			if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL) {
				break;
			}
			bf->limit_len = wParam;
			if (bf->limit_len < 0) {
				bf->limit_len = 0;
			}
			break;

		case EM_LINEFROMCHAR:
			// ���蕶���C���f�b�N�X���܂܂��s�ԍ����擾
			if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL) {
				break;
			}
			if (wParam == -1) {
				if (bf->sp < bf->cp) {
					return index_to_line(bf, bf->sp);
				} else {
					return index_to_line(bf, bf->cp);
				}
			}
			return index_to_line(bf, wParam);

		case EM_LINEINDEX:
			// �s�ԍ��̕����C���f�b�N�X���擾
			if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL) {
				break;
			}
			if (wParam == -1) {
				return line_get(bf, index_to_line(bf, bf->cp));
			}
			if (wParam < 0 || wParam >= (WPARAM)bf->line_len) {
				return -1;
			}
			return line_get(bf, wParam);

		case EM_LINELENGTH:
			// �s�̒������擾
			if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL) {
				break;
			}
			if (wParam == -1) {
				if (bf->sp == bf->cp) {
					return line_get_length(bf, bf->cp);
				}
				i = ((bf->sp < bf->cp) ? bf->sp : bf->cp);
				i = i - line_get(bf, index_to_line(bf, i));
				j = ((bf->sp < bf->cp) ? bf->cp : bf->sp);
				j = line_get_length(bf, j) - (j - line_get(bf, index_to_line(bf, j)));
				return (i + j);
			}
			return line_get_length(bf, wParam);

		case EM_LINESCROLL:
			// �����A�����ɃX�N���[��
			if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL) {
				break;
			}
			get_edit_rect(hWnd, bf, &rect);

			if (wParam != 0) {
				i = wParam;
				if (bf->pos_x + i < 0) {
					i = bf->pos_x * -1;
				}
				if (bf->pos_x + i > bf->max_x) {
					i = bf->max_x - bf->pos_x;
				}
				SetScrollPos(hWnd, SB_HORZ, bf->pos_x + i, TRUE);
				ScrollWindowEx(hWnd, -((int)i * bf->char_width), 0, NULL, &rect, NULL, NULL, SW_INVALIDATE | SW_ERASE);
				bf->pos_x += i;
			}
			if (lParam != 0) {
				rect.left = 0;
				i = lParam;
				if (bf->pos_y + i < 0) {
					i = bf->pos_y * -1;
				}
				if (bf->pos_y + i > bf->max_y) {
					i = bf->max_y - bf->pos_y;
				}
				SetScrollPos(hWnd, SB_VERT, bf->pos_y + i, TRUE);
				ScrollWindowEx(hWnd, 0, -((int)i * bf->font_height), NULL, &rect, NULL, NULL, SW_INVALIDATE | SW_ERASE);
				bf->pos_y += i;
			}
			ensure_inrect(hWnd, bf);
			break;

		case EM_REPLACESEL:
			// �I��͈͂̃e�L�X�g��u��������
			if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL || lParam == 0) {
				break;
			}
			string_insert(hWnd, bf, (TCHAR *)lParam, lstrlen((TCHAR *)lParam), TRUE);
			break;

		case EM_SCROLL:
			// �e�L�X�g�𐂒��ɃX�N���[��
			if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL) {
				break;
			}
			get_edit_rect(hWnd, bf, &rect);

			switch (wParam) {
			case SB_LINEUP:
				i = (bf->pos_y > 0) ? -1 : 0;
				break;

			case SB_LINEDOWN:
				i = (bf->pos_y < bf->max_y) ? 1 : 0;
				break;

			case SB_PAGEUP:
				i = (bf->pos_y - (((rect.bottom - rect.top) / bf->font_height) - 1) > 0) ?
					-(((rect.bottom - rect.top) / bf->font_height) - 1) : bf->pos_y;
				break;

			case SB_PAGEDOWN:
				i = (bf->pos_y + (((rect.bottom - rect.top) / bf->font_height) - 1) < bf->max_y) ?
					(((rect.bottom - rect.top) / bf->font_height) - 1) : bf->max_y - bf->pos_y;
				break;

			default:
				return FALSE;
			}
			if (i == 0) {
				return 0;
			}
			SendMessage(hWnd, WM_VSCROLL, wParam, 0);
			return MAKELRESULT(i, TRUE);

		case EM_SCROLLCARET:
			// �L�����b�g�ʒu�ɃX�N���[��
			if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL) {
				break;
			}
			ensure_visible(hWnd, bf , wParam!=0 );
			InvalidateRect(hWnd, NULL, FALSE);
			return !0;

		case EM_SETMODIFY:
			// �ύX�t���O���Z�b�g
			if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL) {
				break;
			}
			bf->modified = wParam;
			break;

		case EM_GETREADONLY:
			// �ǂݎ���p�t���O�̎擾
			if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL) {
				break;
			}
			return bf->lock;

		case EM_SETREADONLY:
			// �ǂݎ���p�̐ݒ�
			if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL) {
				break;
			}
			bf->lock = wParam;
			if (bf->lock == TRUE) {
				SetWindowLong(hWnd, GWL_STYLE, GetWindowLong(hWnd, GWL_STYLE) | ES_READONLY);
				if (bf->himc == (HIMC)NULL) {
					bf->himc = ImmAssociateContext(hWnd, (HIMC)NULL);
				}
			} else if (bf->lock == FALSE) {
				SetWindowLong(hWnd, GWL_STYLE, GetWindowLong(hWnd, GWL_STYLE) & ~ES_READONLY);
				if (bf->himc != (HIMC)NULL) {
					ImmAssociateContext(hWnd, bf->himc);
					bf->himc = (HIMC)NULL;
				}
			}
			return !0;

		case EM_SETRECT:
			// �`��̈�̐ݒ� (�ĕ`�悠��)
			if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL || lParam == 0) {
				break;
			}
			SendMessage(hWnd, EM_SETRECTNP, wParam, lParam);
			InvalidateRect(hWnd, NULL, FALSE);
			break;

		case EM_SETRECTNP:
			// �`��̈�̐ݒ� (�ĕ`��Ȃ�)
			if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL || lParam == 0) {
				break;
			}
			GetClientRect(hWnd, &rect);
			bf->left_margin = ((RECT *)lParam)->left + 1;
			bf->top_margin = ((RECT *)lParam)->top + 1;
			bf->right_margin = rect.right - ((RECT *)lParam)->right + 1;
			bf->bottom_margin = rect.bottom - ((RECT *)lParam)->bottom + 1;
			draw_free(bf);
			draw_init(hWnd, bf);
			SendMessage(hWnd, WM_REFLECT, 1, 0);
			break;

		case EM_SETSEL:
			// �I���C���f�b�N�X�̐ݒ�
			if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL) {
				break;
			}
			string_flush(bf, TRUE);

			i = bf->sp;
			j = bf->cp;

			sp = ((DWORD)wParam < (DWORD)lParam) ? wParam : lParam;
			cp = ((DWORD)wParam < (DWORD)lParam) ? lParam : wParam;
			if (sp < 0) {
				sp = 0;
			}
			if (sp > bf->buf_len) {
				sp = bf->buf_len;
			}
			if (cp < 0 || cp > bf->buf_len) {
				cp = bf->buf_len;
			}
			bf->sp = sp; bf->cp = cp;
			bf->cpx = 0;
			line_refresh(hWnd, bf, i, j);
			line_refresh(hWnd, bf, bf->cp, bf->sp);
			break;

		case EM_SETTABSTOPS:
			// �^�u�X�g�b�v�̐ݒ�
			if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL) {
				break;
			}
			if (wParam < 0 || lParam == 0) {
				return FALSE;
			} else if (wParam == 0) {
				bf->tab_stop = TAB_STOP;
			} else {
				bf->tab_stop = *((LPDWORD)lParam) / 4;
			}
			SendMessage(hWnd, WM_REFLECT, 0, 0);
			return TRUE;

		case WM_UNDO:
		case EM_UNDO:
			// ���ɖ߂�
			if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL || bf->lock == TRUE) {
				break;
			}
			undo_exec(hWnd, bf);
			break;

		case EM_REDO:
			// ��蒼��
			if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL || bf->lock == TRUE) {
				break;
			}
			redo_exec(hWnd, bf);
			break;

		case WM_GETBUFFERINFO:
			// �����o�b�t�@�̎擾
			if (lParam == 0) {
				break;
			}
			*((BUFFER **)lParam) = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA);
			break;

		case WM_REFLECT:
			// �`����̍X�V
			if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) != NULL) {
				TEXTMETRIC tm;

				// �t�H���g���̎擾
				GetTextMetrics(bf->mdc, &tm);
				bf->font_height = tm.tmHeight + bf->spacing;
				bf->char_width = tm.tmAveCharWidth;

				GetClientRect(hWnd, &rect);
				bf->width = rect.right - rect.left;
				if ( bf->width != 0 ){
					HCURSOR old_cursor;

					if (GetFocus() == hWnd) {
						DestroyCaret();
						CreateCaret(hWnd, NULL, bf->caret_width, bf->font_height);
						ShowCaret(hWnd);
					}
					old_cursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
				// �s���̍Đݒ�
					bf->line_width = 0;
					bf->line_len = 0;
					line_set_info(bf);
					set_scrollbar(hWnd, bf);
					if (wParam == 0) {
						InvalidateRect(hWnd, NULL, FALSE);
					}
					SetCursor(old_cursor);
				}
			}
			break;

		case WM_GETWORDWRAP:
			// �܂�Ԃ��t���O�̎擾
			if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL) {
				break;
			}
			i = GetWindowLong(hWnd, GWL_STYLE);
			return ((i & WS_HSCROLL) ? FALSE : TRUE);

		case WM_SETWORDWRAP:
			// �܂�Ԃ��ݒ�
			if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL) {
				break;
			}
			bf->wordwrap = wParam;
			i = bf->pos_y;
			set_scrollbar(hWnd, bf);
			bf->pos_y = i;
			SendMessage(hWnd, WM_REFLECT, 0, 0);
			ensure_visible(hWnd, bf);
			break;
		case WM_GETWORDWRAPPOS:
			// �܂�Ԃ��t���O�̎擾
			if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL) {
				break;
			}
			return bf->wordwrap_pos;

		case WM_SETWORDWRAPPOS:
			// �܂�Ԃ��ݒ�
			if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL) {
				break;
			}
			bf->wordwrap_pos = wParam;
			if ( lParam != 0 && bf->width>0 ){		// lParam ���O�̎��͐ݒ�ύX�����s��
				i = bf->pos_y;
				set_scrollbar(hWnd, bf);
				bf->pos_y = i;
				SendMessage(hWnd, WM_REFLECT, 0, 0);
				ensure_visible(hWnd, bf);
			}
			break;

		case WM_GETMEMSIZE:
			// �����������̃T�C�Y�擾
			if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL) {
				break;
			}
			return (BUF_LEN * sizeof(TCHAR));

		case WM_GETMEM:
			// �����������̎擾
			if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL || lParam == 0) {
				break;
			}
			string_flush(bf, TRUE);
			CopyMemory((TCHAR *)lParam, bf->buf, (BUF_LEN * sizeof(TCHAR)));
			return (BUF_LEN * sizeof(TCHAR));

		case WM_SETMEM:
			// ������������������ɐݒ�
			if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL || lParam == 0) {
				break;
			}
			return string_set(hWnd, bf, (TCHAR *)lParam, wParam / sizeof(TCHAR));

		case WM_GETLOCALMEM:
			// �����������A�h���X�̎擾
			if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) == NULL ) {
				break;
			}
			string_flush(bf, TRUE);
			return (LRESULT)(bf->buf);

		case WM_SETPLATFORM:
			// �v���b�g�t�H�[���̐ݒ�
			if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) != NULL ) {
				bf->nPlatform = wParam;
				SendMessage( bf->hCompWnd , WM_SETPLATFORM , wParam , lParam );
			}
			return 0;
		case WM_DRAWSYMBOL:
			// �V���{���`�惂�[�h�̕ύX
			if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) != NULL ) {
				bf->drawsymbol = wParam;
				InvalidateRect(hWnd, NULL, FALSE);
			}
			return 0;
		case WM_SETCOLOR:
			// �F�̐ݒ�
			if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) != NULL ) {
				draw_setcolor( bf , wParam , lParam );
				SendMessage( bf->hCompWnd , WM_IMECOMP_SETCOLOR , wParam,draw_getcolor(bf,wParam) );
				InvalidateRect(hWnd, NULL, FALSE);
			}
			return 0;

		case WM_CONTEXTMENU:
			if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) != NULL) {
				HMENU hMenu;
				POINT apos;
				DWORD st, en;
				WORD lang;

				SendMessage(hWnd, EM_GETSEL, (WPARAM)&st, (LPARAM)&en);
	//			lang = PRIMARYLANGID(LANGIDFROMLCID(GetThreadLocale()));
				lang = LANG_JAPANESE;
				// ���j���[�̍쐬
				hMenu = CreatePopupMenu();
				AppendMenu(hMenu, MF_STRING | (SendMessage(hWnd, EM_CANUNDO, 0, 0) == TRUE) ? 0 : MF_GRAYED, EM_UNDO,
					(lang != LANG_JAPANESE) ? TEXT("&Undo") : TEXT("���ɖ߂�(&U)"));
				AppendMenu(hMenu, MF_STRING | (SendMessage(hWnd, EM_CANREDO, 0, 0) == TRUE) ? 0 : MF_GRAYED, EM_REDO,
					(lang != LANG_JAPANESE) ? TEXT("&Redo") : TEXT("��蒼��(&R)"));
				AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
				AppendMenu(hMenu, MF_STRING | (st != en) ? 0 : MF_GRAYED, WM_CUT,
					(lang != LANG_JAPANESE) ? TEXT("Cu&t") : TEXT("�؂���(&T)"));
				AppendMenu(hMenu, MF_STRING | (st != en) ? 0 : MF_GRAYED, WM_COPY,
					(lang != LANG_JAPANESE) ? TEXT("&Copy") : TEXT("�R�s�[(&C)"));
				AppendMenu(hMenu, MF_STRING, WM_PASTE,
					(lang != LANG_JAPANESE) ? TEXT("&Paste") : TEXT("�\��t��(&P)"));
				AppendMenu(hMenu, MF_STRING | (st != en) ? 0 : MF_GRAYED, WM_CLEAR,
					(lang != LANG_JAPANESE) ? TEXT("&Delete") : TEXT("�폜(&D)"));
				AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
				AppendMenu(hMenu, MF_STRING, 1,
					(lang != LANG_JAPANESE) ? TEXT("Select &All") : TEXT("���ׂđI��(&A)"));

				// ���j���[�̕\��
				GetCursorPos((LPPOINT)&apos);
	//			i = TrackPopupMenu(hMenu, TPM_TOPALIGN | TPM_RETURNCMD, apos.x, apos.y, 0, hWnd, NULL , 0);
				i = TrackPopupMenuEx(hMenu, TPM_TOPALIGN | TPM_RETURNCMD, apos.x, apos.y, hWnd, NULL );
				DestroyMenu(hMenu);
				switch (i) {
				case 0:
					break;
				case 1:
					SendMessage(hWnd, EM_SETSEL, 0, -1);
					break;
				default:
					SendMessage(hWnd, i, 0, 0);
					break;
				}
			}
			break;
		case WM_SETSPACING:
			// �s�Ԑݒ�
			if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) != NULL) {
				bf->spacing = wParam;
				draw_free( bf );
				draw_init( hWnd, bf);
				SendMessage(hWnd, WM_REFLECT, 0, 0);
				ensure_visible(hWnd, bf);
				SendMessage( bf->hCompWnd , WM_SETSPACING , wParam, 0 );
				InvalidateRect(hWnd, NULL, FALSE);
			}
			return !0;
		case WM_SETFLAGS:
			// �t���O�ݒ� ( wparam=mask lparam=value)
			if ((bf = (BUFFER *)GetWindowLong(hWnd, GWL_USERDATA)) != NULL) {
				DWORD	oldflag = bf->flags;
				bf->flags &= ~wParam;				// mask
				bf->flags |= ( wParam & lParam );	// value

				if ( (wParam & NEDIT_FLAGS_LINENUMBER)!=0 ){
					if ( (oldflag & wParam) != (bf->flags & wParam ) ){
						SendMessage( hWnd , WM_REFLECT ,0,0 );
					}
				}
				InvalidateRect(hWnd, NULL, FALSE);
			}
			return !0;
		default:
			return DefWindowProc(hWnd, msg, wParam, lParam);
		}
	}
	catch( ... ){
		MessageBox( hWnd , _T("Out of Memory") , _T("Error!") , MB_OK|MB_ICONSTOP );
		SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)(_T("")) );
	}
	return 0;
}

/*
 * nedit_regist - �N���X�̓o�^
 */
BOOL nedit_regist(const HINSTANCE hInstance)
{
	WNDCLASS wc;

	imecomp_regist(hInstance);

	wc.style = CS_DBLCLKS;
	wc.lpfnWndProc = (WNDPROC)nedit_proc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(0, IDC_IBEAM);
//	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.hbrBackground = NULL;
	wc.lpszMenuName = 0;
	wc.lpszClassName = NEDIT_WND_CLASS;
	return RegisterClass(&wc);
}
/* End of source */
