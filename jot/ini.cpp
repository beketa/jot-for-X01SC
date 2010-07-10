//
// jot - Text Editor for Windows Mobile
//   Copyright (C) 2007-2008, Aquamarine Networks.  <http://pandora.sblo.jp/>
//
//    This program is EveryOne's ware; you can redistribute it and/or modify it 
//    under the terms of the NYSL version 0.9982 or later. <http://www.kmonos.net/nysl/>
//

#include "stdafx.h"
#include "ini.h"




void Ini::open(const TCHAR* ininame)
{
	// ���s�t�@�C��������f�B���N�g���� ini ������B
	TCHAR temp[MAX_PATH*2]={0};
	GetModuleFileName(NULL, temp, MAX_PATH*2-1);
	CString s = temp;

	size_t pos = s.ReverseFind(_T('\\'));
	_filename = s.Left(pos+1) + ininame;

	// Ini�t�@�C�������݂���ΊJ���Ē��g��́B
	// �Ȃ���ΐV�K�쐬�B
	FILE  *fp;

	fp = _tfopen( _filename, _T("r") ) ;
	if( fp != NULL ){
		readfile( fp );
		fclose( fp );
	}
	bUpdate = false;
}

void Ini::close()
{
	FILE *fp;

	// �X�V����Ă���Ƃ������ۑ�
	if ( bUpdate ){
		fp = _tfopen( _filename , _T("w") ) ;
		if ( fp != NULL ){

			for( INT_PTR i=0; i< _vSection.GetCount(); i++ )
			{
				_ftprintf( fp , _T("%s\n") , _vSection[i] );
				for( INT_PTR j=0; j<_vKey[i].GetCount(); j++)
				{
					CString str = _vKey[i][j];
					_ftprintf( fp , _T( "%s\n" ) , str );
				}
			}
			fclose(fp);
		}
		bUpdate = false;
	}
}

void Ini::readfile(FILE *fp)
{
	// ��s�Âǂ�� String �Ɋi�[�B
	CString s;
	int  i=0;

	TCHAR buf[1024];

	while( _fgetts( buf , 1023 , fp ) !=NULL )
	{
		buf[_tcslen(buf)-1]=0;		// ���s�R�[�h���폜
		CString s = buf;
		if(*buf == '['){
			_vSection.Add( s );
			i++;
		}else{
			if ( i>0 ){
				_vKey[i-1].Add( s );
			}
		}
	}
}


CString Ini::readString(const TCHAR* lpSection, const TCHAR* lpKey) const
{
	int n=-1;

	size_t vsize = _vSection.GetCount();

	// �Z�N�V�������ǂ�����T��
	for(size_t i=0; i<vsize; i++)
	{
		CString ss = _vSection[i];
		
		// []�̒��g�����o��
		int pos1 = ss.Find( _T('[') );
		int pos2 = ss.Find( _T(']') );
		if ( pos1 !=-1 && pos2!=-1 ){
			CString sec = ss.Mid( pos1+1 , pos2-1 );
			if ( sec == lpSection ){
				n = (int)i;
				break;
			}
		}
	}
	// ������Ȃ���΋�
	if( n==-1 ) return _T("");

	// �L�[��T��
	for(INT_PTR j=0; j<_vKey[n].GetCount(); j++)
	{
		CString line = _vKey[n][j];

		// �R�����g�̏���
		int compos = line.Find( _T(';'));
		CString data ;
		if ( compos != -1 ){
			data = line.Left( compos );
		}else{
			data = line;
		}

		// '=' �ŕ���
		size_t eqpos = data.Find(_T('='));

		CString keydata = data.Left( eqpos );
		CString valdata = data.Right( data.GetLength() - eqpos - 1 );

		// ���ꂼ��󔒂��g����
		CString key = keydata.Trim( _T(" \t") );
		CString val = valdata.Trim( _T(" \t") );
		// �c�����L�[���r���Ĉ�v����Βl��Ԃ�
		if ( key == lpKey ){
			return val;
		}
	}
	return _T("");
}

int Ini::readInt(const TCHAR* lpSection, const TCHAR* lpKey , int defval) const
{
	CString s = readString(lpSection, lpKey);

	if(s=="") return defval;
	else      return _ttoi( s );
}


bool Ini::writeString(const TCHAR* lpSection,const  TCHAR* lpKey,const  TCHAR* lpValue)
{
	int n=-1;

	int vsize = _vSection.GetCount();

	bUpdate = true;

	// �Z�N�V�������ǂ�����T��
	for( n=0; n<vsize; n++)
	{
		CString ss = _vSection[n];
		
		// []�̒��g�����o��
		int pos1 = ss.Find( _T('[') );
		int pos2 = ss.Find( _T(']') );
		if ( pos1 !=-1 && pos2!=-1 ){
			CString sec = ss.Mid( pos1+1 , pos2-1 );
			if ( sec == lpSection ){
				break;
			}
		}
	}

	// �Ȃ��ꍇ�A�Z�N�V�������쐬
	if( n==vsize ) {
		CString s1;
		s1.Format( _T("[%s]") ,lpSection);
		_vSection.Add( s1 );
		
	}

	// �L�[�E�l�̕�����𐶐�
	CString s2;
	s2.Format( _T("%s=%s"), lpKey ,lpValue );

	// �L�[��T��
	for(INT_PTR  j=0; j<_vKey[n].GetCount() ; j++)
	{
		CString line = _vKey[n][j];

		// �R�����g�̏���
		int compos = line.Find( _T(';'));
		CString data ;
		if ( compos != -1 ){
			data = line.Left( compos );
		}else{
			data = line;
		}

		// '=' �ŕ���
		size_t eqpos = data.Find(_T('='));

		if ( eqpos != -1 ){
			CString keydata = data.Left( eqpos );

			// ���ꂼ��󔒂��g����
			CString key = keydata.Trim( _T(" \t") );
			// �c�����L�[���r���Ĉ�v����ΏI��
			if ( key == lpKey ){
				_vKey[n][j] = s2;
				return true;
			}
		}
	}
	// ������Ȃ���ΐV�K�Œǉ�
	_vKey[n].Add( s2 );

	return true;
}


bool Ini::writeInt(const TCHAR* lpSection,const  TCHAR* lpKey, int value)
{
	TCHAR temp[32] = {0};

	_itot(value, temp, 10);

	return writeString(lpSection, lpKey, temp);
}

