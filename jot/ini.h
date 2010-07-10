//
// jot - Text Editor for Windows Mobile
//   Copyright (C) 2007-2008, Aquamarine Networks.  <http://pandora.sblo.jp/>
//
//    This program is EveryOne's ware; you can redistribute it and/or modify it 
//    under the terms of the NYSL version 0.9982 or later. <http://www.kmonos.net/nysl/>
//

#pragma once

#define	MAX_SECTION 100

//  ini ファイルを扱うクラス。

class Ini
{
public:
	Ini();
	Ini(TCHAR* ininame);
	~Ini();

	//@{ 開いて //@}
	void open(const TCHAR* ininame);
	//@{ 閉じて //@}
	void close();

	//@{ 詠んで //@}
	CString readString(const TCHAR* lpSection, const TCHAR* lpKey) const;
	int readInt(const TCHAR* lpSection, const TCHAR* lpKey, int defval ) const;
	//@{ 書いて //@}
	bool writeString(const TCHAR* lpSection, const TCHAR* lpKey, const TCHAR* lpValue);
	bool writeInt(const TCHAR* lpSection, const TCHAR* lpKey, int value);

private:
	//@{ 初期化 //@}
	void readfile(FILE *fp);
	bool bUpdate;
private:
	CString _filename;
	CArray <CString> _vSection;
	CArray <CString> _vKey[MAX_SECTION];
};

//--------------------------------------------------------

inline Ini::Ini()
	{
		bUpdate = false;
	}

inline Ini::Ini(TCHAR* ininame)
	{ 
		bUpdate = false;
		open(ininame); 
	}

inline Ini::~Ini()
	{ close(); }


