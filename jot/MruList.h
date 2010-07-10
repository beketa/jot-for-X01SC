//
// jot - Text Editor for Windows Mobile
//   Copyright (C) 2007-2008, Aquamarine Networks.  <http://pandora.sblo.jp/>
//
//    This program is EveryOne's ware; you can redistribute it and/or modify it 
//    under the terms of the NYSL version 0.9982 or later. <http://www.kmonos.net/nysl/>
//
#pragma once
#include	"sharedmru.h"

class CMruList: public CSharedMru
{
private:
	class CMru {
	public:
		CString	filename;
		int	data[16];
		CMru( ){ZeroMemory( &data , sizeof(data));}
		//CMru( const TCHAR*f , int p ){
		//	filename = f;
		//	ZeroMemory( &data , sizeof(data));
		//	data[0] = p;
		//}
		CMru( const TCHAR*f , int *p ){
			filename = f;
			memcpy( data , p , sizeof(int)*16 );
		}

		int	GetPos(){	return data[0];	}
	};
	CList<CMru>	m_list;
	int	m_max;
	POSITION	GetPosition( const TCHAR *filename );

	int			m_dummy[16];
public:
	CMruList(int max,const TCHAR* mrufile);
	virtual ~CMruList(void);
	void	Add( const TCHAR*filename , int *p );
	void	AddTail( const TCHAR*filename , int *p );
	void	Remove( const TCHAR*filename );
	int	GetCount(){
		return m_list.GetCount();
	}
	int	GetMax(){	return m_max; }
	const TCHAR	*GetFilename( int i ){
		POSITION pos = m_list.FindIndex(i);
		return m_list.GetAt(pos).filename;
	}
	int	*GetData( int i ){
		POSITION pos = m_list.FindIndex(i);
		return m_list.GetAt(pos).data;
	}
	int	*GetData( const TCHAR *filename )
	{
		POSITION pos = GetPosition( filename );
		if ( pos!=NULL ){
			CMru mru = m_list.GetAt(pos);
			memcpy( m_dummy , mru.data , sizeof(int)*16 );
		}else{
			ZeroMemory( m_dummy , sizeof(int)*16 );
		}
		return m_dummy;
	}
	virtual	BYTE *CommitData(BYTE *p);
	virtual	BYTE *UpdateData(BYTE *p);

};
