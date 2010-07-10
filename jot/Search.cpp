//
// jot - Text Editor for Windows Mobile
//   Copyright (C) 2007-2008, Aquamarine Networks.  <http://pandora.sblo.jp/>
//
//    This program is EveryOne's ware; you can redistribute it and/or modify it 
//    under the terms of the NYSL version 0.9982 or later. <http://www.kmonos.net/nysl/>
//


#include "StdAfx.h"
#include "Search.h"

#include "regex.h"

int	CTextSearch::comp(const TCHAR *s1 , const TCHAR *s2 , int len )
{
	if ( m_nocase ){
		return _tcsnicmp( s1 , s2, len );
	}else{
		return _tcsncmp( s1 , s2, len );
	}
}
bool	CTextSearch::Pattern( const TCHAR *needle )
{
	m_pattern = needle;
	return true;
}

bool	CTextSearch::Search( bool forward , const TCHAR *hay , int &st , int &ed)
{
	bool	found=false;
	int i;
	int	len = _tcslen( m_pattern );

	if ( forward ){
		for( i = st ; hay[i]!=_T('\0') ;i++ ){
			if ( comp( hay+i , m_pattern , len )==0 ){
				found = true;
				st = i;
				ed = i+len;
				break;
			}
		}
	}else{
		for( i = st-1 ; i>=0 ;i-- ){
			if ( comp( hay+i , m_pattern , len )==0 ){
				found = true;
				st = i;
				ed = i+len;
				break;
			}
		}
	}
	return found;
}

CRegexSearch::CRegexSearch()
{
	m_nfa = NULL;
}

CRegexSearch::~CRegexSearch()
{
	if ( m_nfa != NULL ){
		free_nfa((REGE_NFA*)m_nfa);
	}
}

bool	CRegexSearch::Pattern( const TCHAR *needle )
{
	m_nfa = rege_compile(needle);
	return(  m_nfa !=NULL );
}


bool	CRegexSearch::Search( bool forward , const TCHAR *hay ,  int &st , int &ed)
{
	bool	found=false;
	int rc;
	REGE_REFER *rf=NULL;

	if (m_nfa!=NULL) {
		rc = reg_match_intext((REGE_NFA*)m_nfa, hay , &rf, m_nocase , st , forward?TRUE:FALSE );

		if (rc > 0) {
			st = rf[0].st-hay;
			ed = rf[0].en-hay;
			found = true;
		}
		free_refer(rf);
	}
	return found;
}

