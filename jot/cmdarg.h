//
// jot - Text Editor for Windows Mobile
//   Copyright (C) 2007-2008, Aquamarine Networks.  <http://pandora.sblo.jp/>
//
//    This program is EveryOne's ware; you can redistribute it and/or modify it 
//    under the terms of the NYSL version 0.9982 or later. <http://www.kmonos.net/nysl/>
//


#pragma	once

//@{
//  �R�}���h���C���p�[�T�B
//  �C���X�^���X�𐶐����ĕ���������悵�A
//  static �� getCommandLine �ŕ�����𓾂���悵�B
//  (CE �ɂ� GetCommandLine �Ȃ������)
//@}
class Argv
{
public:
	Argv();
	~Argv();

public:
	////@{ ��������󂯎�� //@}
	//static void setCmdLine( LPSTR s );
	//@{ ��������󂯎�� //@}
	static void setCmdLine( TCHAR* s );
	//@{ ������擾 //@}
	static const TCHAR* getCmdLine();

public:
	//@{ ����Get //@}
	const TCHAR* operator[]( ULONG i ) const;
	//@{ �T�C�YGet //@}
	size_t size() const;

private:
	//@{ �R�}���h���C���̕��� //@}
	void parse();

private:
	//@{ ���̕���������̂܂ܕێ� //@}
	static CString _cmdLine;
	//@{ ������̕�������i�[���� vector //@}
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

