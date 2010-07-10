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
//  �t�@�C���Ǎ�
//@}
//=========================================================================

class FileR{
public:
	FileR();
	~FileR();

	//@{
	//	�J��
	//	@param fname �t�@�C����
	//	@return �J�������ǂ���
	//@}
	bool open( const TCHAR* fname );

	//@{
	//	����
	//@}
	void close();

public:
	//@{ �t�@�C���T�C�Y //@}
	unsigned long size() const;

	//@{ �t�@�C�����e���}�b�v�����A�h���X�擾 //@}
	const unsigned char* base() const;

private:
	//@{ �R�s�[�֎~�I�u�W�F�N�g //@}
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
//	�t�@�C����������
//@}
//=========================================================================
class FileW
{
public:

	FileW();
	~FileW();

	//@{ �J�� //@}
	bool open( const TCHAR* fname, bool creat=true );

	//@{ ���� //@}
	void close();

	//@{ ���� //@}
	void write( const void* buf, ULONG siz );

	//@{ �ꕶ������ //@}
	void writeC( UCHAR ch );

public:

	void flush();

private:

	HANDLE      _handle;
	UCHAR*      _buf;
	ULONG       _bPos;
	const int   BUFSIZE;

private:
	//@{ �R�s�[�֎~�I�u�W�F�N�g //@}
	FileW( const FileW& );
	FileW& operator=( const FileW& );

};

//-------------------------------------------------------------------------
