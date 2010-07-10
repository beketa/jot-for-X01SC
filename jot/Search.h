//
// jot - Text Editor for Windows Mobile
//   Copyright (C) 2007-2008, Aquamarine Networks.  <http://pandora.sblo.jp/>
//
//    This program is EveryOne's ware; you can redistribute it and/or modify it 
//    under the terms of the NYSL version 0.9982 or later. <http://www.kmonos.net/nysl/>
//

#pragma once

class	CSearch {
protected:
	bool	m_nocase;
public:
	CSearch( ):	m_nocase(false)
	{
	}
	void	IgnoreCase( bool nocase )
	{
		m_nocase = nocase;
	}
	virtual	bool	Pattern( const TCHAR *needle )=0;
	virtual	bool	Search(bool forward, const TCHAR *hay ,int &st , int &ed)=0;
};

class	CTextSearch : public CSearch {
	int	comp(const TCHAR *s1 , const TCHAR *s2 , int len );
	CString	m_pattern;
public:
	virtual	bool	Pattern( const TCHAR *needle );
	virtual	bool	Search(bool forward, const TCHAR *hay , int &st , int &ed);
};

class	CRegexSearch : public CSearch {
	void *m_nfa;
public:
	CRegexSearch();
	virtual ~CRegexSearch();
	virtual	bool	Pattern( const TCHAR *needle );
	virtual	bool	Search(bool forward, const TCHAR *hay , int &st , int &ed);
};

