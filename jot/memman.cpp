//
// jot - Text Editor for Windows Mobile
//   Copyright (C) 2007-2008, Aquamarine Networks.  <http://pandora.sblo.jp/>
//
//    This program is EveryOne's ware; you can redistribute it and/or modify it 
//    under the terms of the NYSL version 0.9982 or later. <http://www.kmonos.net/nysl/>
//

#include	"stdafx.h"

#if	defined(_DEBUG)	&& defined( TRACK_MEMORY_LEAK )

// メモリリーク検出用new-delete記録モジュール

#undef	new


class	memmap{
public:
	void	*p;
	TCHAR	*s;
};

#define	MAX 4096
static	memmap	map[MAX];
static	bool	inited = false;

static	void	init()
{
	if ( !inited ){
		for( int i=0;i<MAX;i++){
			map[i].p=NULL;
			map[i].s=NULL;
		}
	}
	inited = true;
}
static	void	AddMem( void *p , TCHAR *str )
{
	init();
	
	int	empty=-1;
	for( int i=0;i<MAX;i++){
		if ( empty == -1 && map[i].p==NULL ){
			empty = i;
			break;
		}
	}
	if ( empty == -1 ){
		OutputDebugString( _T("map over flow!!\n") );
	}else{
		map[empty ].p = p;
		map[empty ].s = new TCHAR [ _tcslen(str)+1 ];
		_tcscpy( map[empty ].s , str );
	}
}

static	void	DelMem( void *p )
{
	init();
	
	int	empty=-1;
	for( int i=0;i<MAX;i++){
		if ( map[i].p==p ){
			empty = i;
			break;
		}
	}
	if ( empty == -1 ){
		OutputDebugString( _T("double deleted!!\n") );
	}else{
		map[empty ].p = NULL;
		delete(map[empty ].s);
		map[empty ].s = NULL;
	}
}

#undef	_OutputMemoryLeak
void	_OutputMemoryLeak()
{
	for( int i=0;i<MAX;i++){
		if ( map[i].p!=NULL ){
			OutputDebugString( map[i].s );
			delete(map[i].s);
		}
	}
}



static	TCHAR	buff[MAX_PATH];

void	*operator new(size_t nSize, LPCTSTR lpszFileName, int nLine)
{
	void	*ret = operator new( nSize );

	_stprintf( buff , _T("[%08X]-[%08X] alloc in [%s] line [%d]\n") ,ret , nSize ,lpszFileName,nLine );
	AddMem( ret , buff );
	return ret;
}


void	operator delete(void* p, LPCTSTR lpszFileName, int nLine)
{
	delete p ;

	//_stprintf( buff , _T("[%08X] deleted in [%s] line [%d]\n") ,p , lpszFileName,nLine );
	//OutputDebugString( buff );
	DelMem( p );
}

void* PASCAL CObject::operator new(size_t nSize, LPCTSTR lpszFileName, int nLine)
{
	void	*ret = operator new( nSize );

	_stprintf( buff , _T("[%08X]-[%08X] alloc in [%s] line [%d]\n") ,ret , nSize ,lpszFileName,nLine );
	AddMem( ret , buff );
	return ret;
}

void CObject::operator delete(void* pMem, LPCTSTR lpszFileName, int nLine)
{
	delete pMem;

	DelMem( pMem );
}

#undef	_dellog
void _dellog(void* p)
{
	//_stprintf( buff , _T("[%08X] deleted in [%s] line [%d]\n") ,p , lpszFileName,nLine );
	//OutputDebugString( buff );
	DelMem( p );
}


#endif 


#ifdef	OUTPUT_TICK_LOG
DWORD	CCountTime::s_tick=-1;
TCHAR	CCountTime::s_buf[1024];

void	CCountTime::printTime(TCHAR *str){
	DWORD	tick=GetTickCount();
	DWORD	time=0;
	if ( s_tick!=-1 ){
		time = tick - s_tick;
	}
	_stprintf( s_buf , _T("time=%d[ms] %s\n"),time,str );
	OutputDebugString(s_buf);
	s_tick = GetTickCount();
}
#endif


