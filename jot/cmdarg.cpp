//
// jot - Text Editor for Windows Mobile
//   Copyright (C) 2007-2008, Aquamarine Networks.  <http://pandora.sblo.jp/>
//
//    This program is EveryOne's ware; you can redistribute it and/or modify it 
//    under the terms of the NYSL version 0.9982 or later. <http://www.kmonos.net/nysl/>
//

#include "stdafx.h"

#include "cmdarg.h"

CString Argv::_cmdLine;

void Argv::parse()
{
	// コマンドライン文字列を半角スペースで分解。
	// ただし、ダブルクォートで囲まれている部分の
	// 半角スペースは区切りとみなさない。

	// 一旦コピー
	TCHAR *p = new TCHAR[ _cmdLine.GetLength()+1 ];
	_tcscpy( p, _cmdLine );

	TCHAR *pp = p, *pStart=p;
	bool bDQ = false; // ""の内部かどうかのフラグ
	while( 1 )
    {
		if( (*pp==_T(' ') || *pp==_T('\0')) && !bDQ )
        {
			// " の文字は省こう
			if( *pStart==_T('\"') ) pStart++;

			if( *pp==_T('\0') ){
				if( *(pp-1)==_T('\"') ) *(pp-1) = _T('\0');
				_vArgv.Add( pStart );
				break;
			}else{
				if( *(pp-1)==_T('\"') ) *(pp-1) = _T('\0');
				*pp = _T('\0');
				_vArgv.Add( pStart );
				pStart = pp+1;
			}
		}
		if( *pp==_T('\"') ) bDQ = bDQ ? false : true;
		pp++;
   }

	DELA( p );
}

//void Argv::setCmdLine( LPSTR s )
//{
//	_cmdLine = s; 
//}

void Argv::setCmdLine( LPTSTR s )
{
	_cmdLine = s; 
}

