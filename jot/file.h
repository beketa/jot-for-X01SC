//
// jot - Text Editor for Windows Mobile
//   Copyright (C) 2007-2008, Aquamarine Networks.  <http://pandora.sblo.jp/>
//
//    This program is EveryOne's ware; you can redistribute it and/or modify it 
//    under the terms of the NYSL version 0.9982 or later. <http://www.kmonos.net/nysl/>
//

#pragma once

//=========================================================================
//@{
//  ファイル読込
//@}
//=========================================================================

class FileR{
public:
	FileR();
	~FileR();

	//@{
	//	開く
	//	@param fname ファイル名
	//	@return 開けたかどうか
	//@}
	bool open( const TCHAR* fname );

	//@{
	//	閉じる
	//@}
	void close();

public:
	//@{ ファイルサイズ //@}
	unsigned long size() const;

	//@{ ファイル内容をマップしたアドレス取得 //@}
	const unsigned char* base() const;

private:
	//@{ コピー禁止オブジェクト //@}
	FileR( const FileR& );
	FileR& operator=( const FileR& );

private:
	HANDLE      _handle;
	HANDLE      _fmo;
	ULONG       _size;
	const void* _basePtr;
};

//-------------------------------------------------------------------------

inline ULONG FileR::size() const
	{ return _size; }

inline const UCHAR* FileR::base() const
	{ return static_cast<const UCHAR*>(_basePtr); }


//=========================================================================
//@{
//	ファイル書き込み
//@}
//=========================================================================
class FileW
{
public:

	FileW();
	~FileW();

	//@{ 開く //@}
	bool open( const TCHAR* fname, bool creat=true );

	//@{ 閉じる //@}
	void close();

	//@{ 書く //@}
	void write( const void* buf, ULONG siz );

	//@{ 一文字書く //@}
	void writeC( UCHAR ch );

public:

	void flush();

private:

	HANDLE      _handle;
	UCHAR*      _buf;
	ULONG       _bPos;
	const int   BUFSIZE;

private:
	//@{ コピー禁止オブジェクト //@}
	FileW( const FileW& );
	FileW& operator=( const FileW& );

};

//-------------------------------------------------------------------------
