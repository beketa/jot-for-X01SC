//
// jot - Text Editor for Windows Mobile
//   Copyright (C) 2007-2008, Aquamarine Networks.  <http://pandora.sblo.jp/>
//
//    This program is EveryOne's ware; you can redistribute it and/or modify it 
//    under the terms of the NYSL version 0.9982 or later. <http://www.kmonos.net/nysl/>
//


#pragma	once

//@{
//  コマンドラインパーサ。
//  インスタンスを生成して分解するもよし、
//  static の getCommandLine で文字列を得るもよし。
//  (CE には GetCommandLine ないからね)
//@}
class Argv
{
public:
	Argv();
	~Argv();

public:
	////@{ 文字列を受け取る //@}
	//static void setCmdLine( LPSTR s );
	//@{ 文字列を受け取る //@}
	static void setCmdLine( TCHAR* s );
	//@{ 文字列取得 //@}
	static const TCHAR* getCmdLine();

public:
	//@{ 引数Get //@}
	const TCHAR* operator[]( ULONG i ) const;
	//@{ サイズGet //@}
	size_t size() const;

private:
	//@{ コマンドラインの分解 //@}
	void parse();

private:
	//@{ 生の文字列をそのまま保持 //@}
	static CString _cmdLine;
	//@{ 分解後の文字列を格納する vector //@}
	CArray<CString> _vArgv;
};

//---------------------------------------------------------
inline Argv::Argv()
	{ parse(); }

inline Argv::~Argv()
	{}


inline const TCHAR* Argv::getCmdLine()
{ 
	return _cmdLine; 
}

inline size_t Argv::size() const
{ 
	return _vArgv.GetCount(); 
}

inline const TCHAR* Argv::operator[]( ULONG i ) const
{ 
	return _vArgv[i];
}

