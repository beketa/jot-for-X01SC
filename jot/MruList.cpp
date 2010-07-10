//
// jot - Text Editor for Windows Mobile
//   Copyright (C) 2007-2008, Aquamarine Networks.  <http://pandora.sblo.jp/>
//
//    This program is EveryOne's ware; you can redistribute it and/or modify it 
//    under the terms of the NYSL version 0.9982 or later. <http://www.kmonos.net/nysl/>
//

#include "StdAfx.h"
#include "MruList.h"
#include "file.h"

#define	SIG	"MRUF"

CMruList::CMruList(int max,const TCHAR* mrufile)
	:CSharedMru( mrufile , SIG )
{
	m_max = max;
}

CMruList::~CMruList(void)
{
}

POSITION	CMruList::GetPosition( const TCHAR *filename ){
	for( POSITION pos = m_list.GetHeadPosition();pos!=NULL; ){
		POSITION ret = pos;
		CMru mru = m_list.GetNext(pos);
		if ( mru.filename.CompareNoCase( filename ) == 0 ){
			return ret;
		}
	}
	return NULL;
}

void	CMruList::Add( const TCHAR*filename , int *p )
{
	if ( *filename == _T('\0') )
		return;
	// ���ڍ폜
	Remove( filename );
	// �擪�ɒǉ�
	CMru	mru( filename ,p );
	m_list.AddHead( mru );
}

void	CMruList::AddTail( const TCHAR*filename , int *p )
{
	if ( *filename == _T('\0') )
		return;
	// ���ڍ폜
	Remove( filename );
	// �擪�ɒǉ�
	CMru	mru( filename ,p );
	m_list.AddTail( mru );
}


void	CMruList::Remove( const TCHAR*filename )
{
	POSITION pos = GetPosition( filename );
	// ���X�g���ɂ���ꍇ�폜
	if ( pos != NULL ){		
		m_list.RemoveAt( pos );
	}
	// �T�C�Y�����Ă��镪��������폜
	while( m_list.GetCount()>=m_max ){
		m_list.RemoveTail();
	}
}

// MRUFILE�̃t�H�[�}�b�g
//	BYTE[4]	: �V�O�l�`��"MRUF"
//	int		: �t�@�C���S�̂̒���
//	(�ȉ��J��Ԃ�)
//	TCHAR[]	: �t�@�C���� (NULL�I�[)
//	int[16]	: �e��f�[�^
//	�t�@�C�����擪��NULL�������ƁA�t�@�C���I��

// ���L����������Ǎ�
BYTE *CMruList::UpdateData(BYTE *ptr)
{
	TCHAR *p = (TCHAR*)ptr;

	m_list.RemoveAll();
	while ( *p != _T('\0') ){
		TCHAR *s = p;

		p+= _tcslen(s)+1;

		CMru	mru( s , (int*)p );
		m_list.AddTail( mru );

		p+= sizeof(int)*16/sizeof(TCHAR);
	}
	return (BYTE*)p;
}

// ���L�������ɏ���
BYTE *CMruList::CommitData(BYTE *ptr)
{
	TCHAR *p = (TCHAR*)ptr;
	int	cnt = GetCount();
	for( int i=0;i<cnt;i++ ){
		_tcscpy( p , GetFilename(i) );
		p += _tcslen( p )+1;
		int	*data = GetData(i) ;
		memcpy( p , data , sizeof(int)*16 );
		p+= sizeof(int)*16/sizeof(TCHAR);
	}
	*p++ = _T('\0');
	return (BYTE*)p;
}
