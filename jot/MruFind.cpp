//
// jot - Text Editor for Windows Mobile
//   Copyright (C) 2007-2008, Aquamarine Networks.  <http://pandora.sblo.jp/>
//
//    This program is EveryOne's ware; you can redistribute it and/or modify it 
//    under the terms of the NYSL version 0.9982 or later. <http://www.kmonos.net/nysl/>
//

#include "StdAfx.h"
#include "MruFind.h"
#include "file.h"

#define	SIG	"MRUS"

CMruFind::CMruFind(const TCHAR* mrufile)
	:CSharedMru( mrufile , SIG )
{
	m_max = 5;
}

CMruFind::~CMruFind(void)
{
}

POSITION	CMruFind::GetPosition( const TCHAR *str ){
	for( POSITION pos = m_list.GetHeadPosition();pos!=NULL; ){
		POSITION ret = pos;
		CString mru = m_list.GetNext(pos);
		if ( mru.Compare( str ) == 0 ){
			return ret;
		}
	}
	return NULL;
}

void	CMruFind::Add( const TCHAR*str )
{
	if ( *str == _T('\0') )
		return;
	// 項目削除
	Remove( str );
	// 先頭に追加
	m_list.AddHead( str );
}

void	CMruFind::AddTail( const TCHAR*str  )
{
	if ( *str == _T('\0') )
		return;
	// 項目削除
	Remove( str );
	// 先頭に追加
	m_list.AddTail( str );
}


void	CMruFind::Remove( const TCHAR*str )
{
	POSITION pos = GetPosition( str );
	// リスト中にある場合削除
	if ( pos != NULL ){		
		m_list.RemoveAt( pos );
	}
	// サイズ超えている分を下から削除
	while( m_list.GetCount()>=m_max ){
		m_list.RemoveTail();
	}
}

// MRUSEARCHのフォーマット
//	BYTE[4]	: シグネチャ"MRUS"
//	int		: ファイル全体の長さ
//	(以下繰り返し)
//	TCHAR[]	: 文字列 (NULL終端)
//	文字列先頭がNULL文字だと、ファイル終了

BYTE *CMruFind::UpdateData(BYTE *ptr)
{
	TCHAR *p = (TCHAR*)ptr;

	m_list.RemoveAll();
	while ( *p != _T('\0') ){
		m_list.AddTail( p );
		p+= _tcslen(p)+1;
	}
	return (BYTE*)p;
}

BYTE *CMruFind::CommitData(BYTE *ptr)
{
	TCHAR *p = (TCHAR*)ptr;
	int	cnt = GetCount();
	for( int i=0;i<cnt;i++ ){
		_tcscpy( p , Getstr(i) );
		p += _tcslen( p )+1;
	}
	*p++ = _T('\0');
	return (BYTE*)p;
}
