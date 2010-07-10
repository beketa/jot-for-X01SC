//
// jot - Text Editor for Windows Mobile
//   Copyright (C) 2007-2008, Aquamarine Networks.  <http://pandora.sblo.jp/>
//
//    This program is EveryOne's ware; you can redistribute it and/or modify it 
//    under the terms of the NYSL version 0.9982 or later. <http://www.kmonos.net/nysl/>
//

#pragma once

#define	MAX_SECTION 100

//  ini �t�@�C���������N���X�B

class Ini
{
public:
	Ini();
	Ini(TCHAR* ininame);
	~Ini();

	//@{ �J���� //@}
	void open(const TCHAR* ininame);
	//@{ ���� //@}
	void close();

	//@{ �r��� //@}
	CString readString(const TCHAR* lpSection, const TCHAR* lpKey) const;
	int readInt(const TCHAR* lpSection, const TCHAR* lpKey, int defval ) const;
	//@{ ������ //@}
	bool writeString(const TCHAR* lpSection, const TCHAR* lpKey, const TCHAR* lpValue);
	bool writeInt(const TCHAR* lpSection, const TCHAR* lpKey, int value);

private:
	//@{ ������ //@}
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


