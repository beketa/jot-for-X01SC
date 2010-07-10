//
// jot - Text Editor for Windows Mobile
//   Copyright (C) 2007-2008, Aquamarine Networks.  <http://pandora.sblo.jp/>
//
//    This program is EveryOne's ware; you can redistribute it and/or modify it 
//    under the terms of the NYSL version 0.9982 or later. <http://www.kmonos.net/nysl/>
//

#pragma once

// メモリリーク検出用new-delete記録モジュール

#if	defined(_DEBUG)	&& defined( TRACK_MEMORY_LEAK )
#undef	DEBUG_NEW
void* operator new(size_t nSize, LPCTSTR lpszFileName, int nLine);
#define DEBUG_NEW new(_T(__FILE__), __LINE__)

#define	new DEBUG_NEW

void	operator delete(void* p, LPCTSTR lpszFileName, int nLine);
void	_dellog(void* p);

#define	DEL( p )	{ if ( p!=NULL ){ _dellog( p );  delete p; } p=NULL; }
#define	DELA( p )	{ if ( p!=NULL ){ _dellog( p );  delete[] p; } p=NULL; }

void	_OutputMemoryLeak();

#else

#define	DEL( p )	{ if ( p!=NULL ) delete p; p=NULL; }
#define	DELA( p )	{ if ( p!=NULL ) delete[] p; p=NULL; }
#define	_OutputMemoryLeak()	
#define	_dellog(p)
#endif

////
// このモジュールを使用してメモリリークの追跡を行うためには、
// afx.hの CObjectクラスに以下の宣言を追加する必要があります。
//#if defined(_DEBUG)			// jmori
//	void* PASCAL operator new(size_t nSize, LPCTSTR lpszFileName, int nLine);
//	void operator delete(void* pMem, LPCTSTR lpszFileName, int nLine);
//#endif

// 時間測定用クラス
class CCountTime {
#ifdef	OUTPUT_TICK_LOG
private:
	DWORD s1;
	DWORD s2;
	TCHAR *mes;
	static	DWORD	s_tick;
	static	TCHAR	s_buf[1024];
public:
	CCountTime( TCHAR *m=_T("") ){
		s1 = GetTickCount();
		mes = m;
	}
	virtual ~CCountTime(){
		static TCHAR buff[256];
		s2 = GetTickCount();
		int delta = s2-s1;
		_stprintf( buff , _T("%s %d[ms]\n"), mes , delta );
		OutputDebugString( buff );
	}
	static	void	printTime(TCHAR *str);
#else
public:
	CCountTime( TCHAR *m=_T("") ){}
	inline	static	void	printTime(TCHAR *str){}
#endif
};

#ifndef	OUTPUT_LOG
	#undef OutputDebugString
	#define	OutputDebugString(a)
#endif

