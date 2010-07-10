//
// jot - Text Editor for Windows Mobile
//   Copyright (C) 2007-2008, Aquamarine Networks.  <http://pandora.sblo.jp/>
//
//    This program is EveryOne's ware; you can redistribute it and/or modify it 
//    under the terms of the NYSL version 0.9982 or later. <http://www.kmonos.net/nysl/>
//
#pragma once
#include	"sharedmru.h"

class CMruFind : public CSharedMru
{
private:
	CList<CString>	m_list;
	int	m_max;
	POSITION	GetPosition( const TCHAR *str );

public:
	CMruFind(const TCHAR* mrufile);
	virtual ~CMruFind(void);
	void	Add( const TCHAR*str );
	void	AddTail( const TCHAR*str );
	void	Remove( const TCHAR*str );
	int	GetCount(){
		return m_list.GetCount();
	}
	int	GetMax(){	return m_max; }
	const TCHAR	*Getstr( int i ){
		POSITION pos = m_list.FindIndex(i);
		return m_list.GetAt(pos);
	}
	virtual	BYTE *CommitData(BYTE *p);
	virtual	BYTE *UpdateData(BYTE *p);

};
