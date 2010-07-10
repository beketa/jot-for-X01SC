//
// jot - Text Editor for Windows Mobile
//   Copyright (C) 2007-2008, Aquamarine Networks.  <http://pandora.sblo.jp/>
//
//    This program is EveryOne's ware; you can redistribute it and/or modify it 
//    under the terms of the NYSL version 0.9982 or later. <http://www.kmonos.net/nysl/>
//

#include "stdafx.h"
#include "file.h"

FileR::FileR()
	: _handle ( INVALID_HANDLE_VALUE )
	, _fmo    ( NULL )
	, _basePtr( NULL )
{
}

FileR::~FileR()
{
	close();
}

bool FileR::open( const TCHAR* fname )
{
	close();

	// ファイルを読みとり専用で開く
	_handle = ::CreateFile(
		fname, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, NULL
	);
	if( _handle == INVALID_HANDLE_VALUE )
		return false;

	// サイズを取得
	_size = ::GetFileSize( _handle, NULL );

	if( _size==0 )
	{
		// 0バイトのファイルはマッピング出来ないので適当に回避
		_basePtr = &_size;
	}
	else
	{
		// マッピングオブジェクトを作る
		_fmo = ::CreateFileMapping(
			_handle, NULL, PAGE_READONLY, 0, 0, NULL );
		if( _fmo == NULL )
		{
			::CloseHandle( _handle );
			_handle = INVALID_HANDLE_VALUE;
			return false;
		}

		// ビュー
		_basePtr = ::MapViewOfFile( _fmo, FILE_MAP_READ, 0, 0, 0 );
		if( _basePtr == NULL )
		{
			::CloseHandle( _fmo );
			::CloseHandle( _handle );
			_handle = INVALID_HANDLE_VALUE;
			return false;
		}
	}
	return true;
}

void FileR::close()
{
	if( _handle != INVALID_HANDLE_VALUE )
	{
		// ヘンテコマッピングをしてなければここで解放
		if( _basePtr != &_size )
			::UnmapViewOfFile( _basePtr );
		_basePtr = NULL;

		if( _fmo != NULL )
			::CloseHandle( _fmo );
		_fmo = NULL;

		::CloseHandle( _handle );
		_handle = INVALID_HANDLE_VALUE;
	}
}


//=========================================================================

FileW::FileW()
	: BUFSIZE( 65536 )
	, _handle( INVALID_HANDLE_VALUE )
//	, _buf   ( new UCHAR[BUFSIZE] )
{
	_buf = new UCHAR[BUFSIZE];
}

FileW::~FileW()
{
	close();
	DELA( _buf );
}

inline void FileW::flush()
{
	DWORD dummy;
	::WriteFile( _handle, _buf, _bPos, &dummy, NULL );
	_bPos = 0;
}

bool FileW::open( const TCHAR* fname, bool creat )
{
	close();

	// ファイルを書き込み専用で開く
	_handle = ::CreateFile( fname,
		GENERIC_WRITE, FILE_SHARE_READ, NULL,
		creat ? CREATE_ALWAYS : OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, NULL );
	if( _handle == INVALID_HANDLE_VALUE )
		return false;

	_bPos = 0;
	return true;
}

void FileW::close()
{
	if( _handle != INVALID_HANDLE_VALUE )
	{
		flush();
		::CloseHandle( _handle );
		_handle = INVALID_HANDLE_VALUE;
	}
}

void FileW::write( const void* dat, ULONG siz )
{
	const UCHAR* udat = static_cast<const UCHAR*>(dat);

	while( (BUFSIZE-_bPos) <= siz )
	{
		memcpy( _buf+_bPos, udat, BUFSIZE-_bPos );
		siz  -= (BUFSIZE-_bPos);
		udat += (BUFSIZE-_bPos);
		_bPos = BUFSIZE;
		flush();
	}
	memcpy( _buf+_bPos, udat, siz );
	_bPos += siz;
}

void FileW::writeC( UCHAR ch )
{
	if( (BUFSIZE-_bPos) <= 1 )
		flush();

	_buf[_bPos++] = ch;
}

