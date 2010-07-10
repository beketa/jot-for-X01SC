//
// jot - Text Editor for Windows Mobile
//   Copyright (C) 2007-2008, Aquamarine Networks.  <http://pandora.sblo.jp/>
//
//    This program is EveryOne's ware; you can redistribute it and/or modify it 
//    under the terms of the NYSL version 0.9982 or later. <http://www.kmonos.net/nysl/>
//
#pragma once

class CSharedMru
{
protected:
	HANDLE		m_hMap;
	BYTE*		m_pMapPtr;
	CString		m_filename;
	BYTE		m_sig[4];
	HANDLE		m_hSema;

public:
	CSharedMru(const TCHAR *mrufile,const char *sig);
	virtual	~CSharedMru(void);

	void	Load();			// ファイルからロード
	void	Save();			// ファイルにセーブ
	void	Update();		// 共有メモリから読込
	void	Commit();		// 共有メモリに書込
protected:
	virtual	BYTE *CommitData(BYTE *p)=0;
	virtual	BYTE *UpdateData(BYTE *p)=0;
};
