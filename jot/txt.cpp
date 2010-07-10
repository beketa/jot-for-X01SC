//
// jot - Text Editor for Windows Mobile
//   Copyright (C) 2007-2008, Aquamarine Networks.  <http://pandora.sblo.jp/>
//
//    This program is EveryOne's ware; you can redistribute it and/or modify it 
//    under the terms of the NYSL version 0.9982 or later. <http://www.kmonos.net/nysl/>
//

#include "stdafx.h"

#include "txt.h"

#define	BUFF_BLOCK	4096

#define	CHARCODE_NONE			0x0000
#define	CHARCODE_SJISHANKANA	0x0001
#define	CHARCODE_SJISZENKAKU	0x0002
#define	CHARCODE_EUCHANKANA		0x0004
#define	CHARCODE_EUCZENKAKU		0x0008
#define	CHARCODE_JISNEW			0x0010
#define	CHARCODE_JISOLD			0x0020
#define	CHARCODE_JISROMA		0x0040
#define	CHARCODE_ASCIISHIFT		0x0080

#define	CHARCODE_SJIS			0x0003
#define	CHARCODE_EUC			0x000C
#define	CHARCODE_ISOJP			0x00F0

//--------------------------------------------------------

charset Txt::autoDetect(const TCHAR *fname, lbcode* lb)
{
	FileR f;
	if(!f.open( fname )) return AutoDetect;

	// �܂�������
	charset        cs = AutoDetect;
	const int maxlen = 65536*2;
	size_t  uppersize = f.size() > maxlen ? maxlen : f.size();

	// LineBreak �`�F�b�N
	*lb = checkMultiLB( f.base() , uppersize );

	// BOM check	// BOM������Ίm��
	if( f.size()>4 )
		if(AutoDetect != (cs = checkBOM(f)) )
			return cs;
	// ���s�R�[�h�`�F�b�N
	if( AutoDetect != (cs = checkUtf16(f.base() , uppersize) ) )
		return cs;

	// UTF8�Ŏ����ǂ�
	{
		// UTF8 �Ƃ��Đ����Ȃ� true�B
		// �������A���̂܂܂��ƁA���p�p���݂̂̃t�@�C����
		// UTF8N �Ƃ��ĔF������Ă��܂�� -> ����ł������H
		const UCHAR *p = f.base();
		const ULONG _size = uppersize;
		bool utf8=true;	
		for( size_t i=0; i+4<_size; )
		{
			if( (*p&0x80)==0x00 ){
				p++; 
				i++; 
				continue; 
			}else if( (*p&0xE0)==0xC0 && ((*(p+1))&0xC0)==0x80 ){ 
				p+=2; 
				i+=2; 
				continue; 
			}else if( (*p&0xF0)==0xE0 && ((*(p+1))&0xC0)==0x80 && ((*(p+2))&0xC0)==0x80 ){
				p+=3; 
				i+=3; 
				continue; 
			}else if( (*p&0xF8)==0xF0 && ((*(p+1))&0xC0)==0x80 && ((*(p+2))&0xC0)==0x80 && ((*(p+3))&0xC0)==0x80 ){
				p+=4; 
				i+=4; 
				continue; 
			} else { 
				utf8=false;
				break;
			}
		}
		if ( utf8 ){
			cs = UTF8N;
		}
	}

	// EUC - JIS - SJIS �`�F�b�N
	const UCHAR* p = f.base();

	int cEucjp=0;
	int cSjis=0;
	int diff;
	for( size_t pos1=0; pos1+3<uppersize; p++, pos1++ )
	{
		int pt = checkCharCode(p);

		// IsoJP�͎��ʂł���Έꔭ�Ŋm��
		if( ( pt &  CHARCODE_ISOJP )!=0 ){
			cs = IsoJP;
			break; 
		}
		// EUC���ۂ����̓J�E���^��
		if( cs == AutoDetect && ( pt & CHARCODE_EUC )!=0 ){
			cEucjp++;
		}
		// SJIS���ۂ������J�E���^��
		if( cs == AutoDetect && ( pt & CHARCODE_SJIS )!=0  ){ 
			cSjis++;
		}
		// EUC��SJIS�̍���32�����ȏ�J������m��
		diff = cEucjp - cSjis;
		if ( diff > 32 ){
			cs = EucJP; 
			break; 
		}else if (diff < -32 ){
			cs = SJIS;  
			break; 
		}
	}
	// �����܂łŌ��܂�Ȃ�������A�������ɂ���
	if ( cs == AutoDetect ){
		if ( diff > 0 ){
			cs = EucJP; 
		}else{
			// �����̎���SJIS�ɓ|��
			cs = SJIS;  
		}
	}

	//// �ǂ�ɂ��Y�����Ȃ�������Ƃ肠���� SJIS
	//if( cs==AutoDetect ) cs = SJIS;

	//// LB �`�F�b�N���v��Ȃ��Ȃ炱���� return
	//if( lb==NULL ) return cs;

	return cs;
}

charset Txt::checkBOM(FileR& f)
{
	const UCHAR* p = (const UCHAR*)(f.base());
	//UTF-8 signature:      EF BB - BF
	//UTF-16(LE) signature: FF FE
	//UTF-16(BE) signature: FE FF
	if( p[0]==0xff && p[1]==0xfe )
		// UTF-16LE
		return UTF16l;
	else if( p[0]==0xfe && p[1]==0xff )
		// UTF-16BE
		return UTF16b;
	else if( p[0] == 0xEF && p[1] == 0xBB && p[2] == 0xBF )
		// UTF-8
		return UTF8;

	return AutoDetect;
}

Txt::lbcode Txt::checkMultiLB(const UCHAR *base, const ULONG size)
{
	UINT i=0;
	const UCHAR *p=base;
	const USHORT *ps;
	while( i < size )
	{
		// 2 byte
		if( (i%2)==0 && i+1 < size )
		{
			ps = reinterpret_cast<const USHORT*>(&p[i]);
			if( *ps==0x000a || *ps==0x0a00 ) return LF;
			if( *ps==0x000d || *ps==0x0d00 )
			{
				ps = reinterpret_cast<const USHORT*>(&p[i+2]);
				if( *ps==0x000a || *ps==0x0a00 ) return CRLF;
				else                             return CR;
			}
		}

		// 1 byte
		if(p[i]=='\r'){
			if(p[i+1]=='\n') return CRLF;
			else             return CR;
		}else if(p[i]=='\n') return LF;

		i++;
	}
	return CRLF;
}

charset Txt::checkUtf16(const UCHAR *base, const ULONG size)
{
	UINT i=0;
	const USHORT *ps=(const USHORT*)base;
	while( i+1 < size/2 )
	{
		if ( *ps==0x000a || *ps==0x000d ) return UTF16LE;
		if ( *ps==0x0a00 || *ps==0x0d00 ) return UTF16BE;
		i++;
		ps++;
	}
	return AutoDetect;
}

/*
�����̎�ʂ�\���l
�E����str�̐擪���������p�J�i�̏ꍇ�A�֐��l��1
�E����str�̐擪2�������V�t�gJIS�R�[�h�̏ꍇ�A�֐��l��2
�E����str�̐擪2������EUC���p�J�i�R�[�h�̏ꍇ�A�֐��l��4
�E����str�̐擪2������EUC�S�p�R�[�h�̏ꍇ�A�֐��l��8
�E����str�̐擪3�������VJIS(X0208-1983)�V�t�g�R�[�h�̏ꍇ�A
�@�֐��l��16
�E����str�̐擪3��������JIS(X0208-1978)�V�t�g�R�[�h�̏ꍇ�A
�@�֐��l��32
�E����str�̐擪3������JIS���[�}��(X0201)�V�t�g�R�[�h�̏ꍇ�A
�@�֐��l��64
�E����str�̐擪3������ASCII �V�t�g�R�[�h�̏ꍇ�A�֐��l��128
�E��̃P�[�X�����������ɋN����ꍇ�́A�֐��l�͂����̘_���a
�E��̂�����ł��Ȃ��ꍇ�́A�֐��l��0
*/

int Txt::checkCharCode(const unsigned char* str)
{
	int val = CHARCODE_NONE;
	unsigned char b1, b2, b3;

	b1 = *str++;
	b2 = *str++;
	b3 = *str;
	if (b1 == 0x1b) {
		if (b2 == '$' && b3 == 'B') return CHARCODE_JISNEW;
		if (b2 == '$' && b3 == '@') return CHARCODE_JISOLD;
		if (b2 == '(' && b3 == 'J') return CHARCODE_JISROMA;
		if (b2 == '(' && b3 == 'B') return CHARCODE_ASCIISHIFT;
		return CHARCODE_NONE;
	}
	if ( b1 >= 0xa0 && b1 <= 0xdf){
		val |= CHARCODE_SJISHANKANA;
	}
	if ((b1 >= 0x81 && b1 <= 0x9f ||
		 b1 >= 0xe0 && b1 <= 0xfc) && 
		 (b2 >= 0x40 && b2 <= 0xfc && b2 != 0x7f)){
		val |= CHARCODE_SJISZENKAKU;
	}
	if (b1 == 0x8e && (b2 >= 0xa0 && b2 <= 0xdf)){
		val |= CHARCODE_EUCHANKANA; 
	}
	if ((b1 >= 0xa1 && b1 <= 0xfe) &&
		(b2 >= 0xa1 && b1 <= 0xfe)){
		val |= CHARCODE_EUCZENKAKU;
	}
	return val;
}

//--------------------------------------------------------

TxtDecoder::TxtDecoder(int charset)
     :_charset(charset),_buff(NULL),_state(0),_buffsize(0)
{}

TxtDecoder::~TxtDecoder()
{
	close();
	DELA( _buff );
}

bool TxtDecoder::open(const TCHAR* fname)
{
	if( !_fp.open(fname) ) return false;

	// ��{�I�ɉ��s�R�[�h�͎������ʁB
	// 

	_pos = 0;
	_base = _fp.base();
	_size = _fp.size();
	_state = 1;

	return true;
}

void TxtDecoder::close()
{
	_fp.close();
}


void	TxtDecoder::MbsToWcs( UINT CodePage )
{
	// EOF �܂ŗ����� _state �� 0 �ɂ���
	const UCHAR *p = &_base[_pos];
	size_t start = _pos, length=0;
	_state = 1;
	while(1){
		if( _pos>=_size ){
			_state = 0;
			break;
		}
		if( *p=='\n' ){
			_pos++;
			break;
		}else if( *p=='\r' ){
			_pos++;
			if( p[1]=='\n' ){ 
				_pos++;
			}
			break;
		}
		p++;
		_pos++;
		length++;
	}

	// �}���`�o�C�g���烏�C�h�����ւ̕ϊ��Ńo�C�g�����������Ƃ������Ƃ͂Ȃ��̂�
	// ���������o�C�g���Ƃ��ăo�b�t�@���m�ۂ���
	int bufflen = length;			

	if ( bufflen+1 > _buffsize ){
		DELA( _buff );
		_buffsize = bufflen+1+BUFF_BLOCK;
		_buff = new TCHAR[_buffsize];
	}

	const char* spos = (const char*)_base+start;

	switch( _charset ){
	case SJIS:
		bufflen = MultiByteToWideChar(CP_ACP , 0,	spos , length,	_buff, bufflen );
		break;
	case EucJP:
		{
			EucTxt	euc;
			int		sjislen=0;
			char	*sjis = euc.ToSjis( spos , length , sjislen );	// EUC->SJIS�ɕϊ�
			bufflen = MultiByteToWideChar(CP_ACP , 0,	sjis , sjislen,	_buff, bufflen );
		}
		break;
	case IsoJP:
		{
			Iso2022Txt	jis;
			int		sjislen=0;
			char	*sjis = jis.ToSjis( spos , length , sjislen );	// EUC->SJIS�ɕϊ�
			bufflen = MultiByteToWideChar(CP_ACP , 0,	sjis , sjislen,	_buff, bufflen );
		}
		break;
	case UTF8:		// Unicode  (UTF-8): BOM�L��
	case UTF8N:		// Unicode  (UTF-8): BOM����
		bufflen = MultiByteToWideChar(CP_UTF8, 0,	spos , length,	_buff, bufflen );
		break;
	case Latin:
		bufflen = MultiByteToWideChar(1252 , 0,	spos , length,	_buff, bufflen );
		break;
	}


	_buff[bufflen]=_T('\0');
}

TCHAR	TxtDecoder::ChangeEndian( USHORT wc , bool  le )
{
	// �r�b�O�G���f�B�A���̎���HL����ւ�
	if ( !le ){
		wc = MAKEWORD( HIBYTE(wc) , LOBYTE(wc) );
	}
	return wc;
}

void	TxtDecoder::Utf16ToWcs( bool le)
{
	// EOF �܂ŗ����� _state �� 0 �ɂ���
	const USHORT *p = (const USHORT*)&_base[_pos];
	size_t start = _pos, length=0;
	_state = 1;
	while(1){
		if( _pos>=_size ){
			_state = 0;
			break;
		}
		USHORT	wc = ChangeEndian( *p , le );
		if( wc==0x000A ){
			_pos+=2;
			break;
		}else if( wc==0x000d ){
			_pos+=2;
			if( ChangeEndian( p[1] , le )==0x000a ){ 
				_pos+=2;
			}
			break;
		}
		p++;
		_pos+=2;
		length++;
	}
	int bufflen = length;
	_buff = new TCHAR[bufflen+1];

	p = (const USHORT*)&_base[start];
	for( size_t i=0;i<length;i++){
		_buff[i] = ChangeEndian( p[i], le );
	}
	_buff[bufflen]=_T('\0');
}


TCHAR	*TxtDecoder::readLine()
{
	//if ( _buff!=NULL ){
	//	DELA(_buff);
	//	_buff = NULL;
	//}

	switch( _charset ){
	case UTF8:		// Unicode  (UTF-8): BOM�L��
		if( _pos==0 ) _pos = 3;
	case UTF8N:		// Unicode  (UTF-8): BOM����
	case SJIS:
	case EucJP:
	case IsoJP:
	case Latin:
		MbsToWcs( _charset );
		break;
	case UTF16b:    // Unicode  (UTF-16)  : BOM�L�� BE
		if( _pos==0 ) _pos = 2;
	case UTF16BE:   // Unicode  (UTF-16BE): BOM����
		Utf16ToWcs( false);
		break;
	case UTF16l:    // Unicode  (UTF-16)  : BOM�L�� LE
		if( _pos==0 ) _pos = 2;
	case UTF16LE:   // Unicode  (UTF-16LE): BOM����
		Utf16ToWcs( true);
		break;
	}
	return _buff;
}



//--------------------------------------------------------

TxtEncoder::TxtEncoder(int charset, int linebreak)
     :_charset(charset),_lb(linebreak),_buff(NULL),_buffsize(0)
{}

TxtEncoder::~TxtEncoder()
{
	DELA( _buff );
}

bool TxtEncoder::open(const TCHAR *fname)
{
	if( !_fp.open(fname) ) return false;

	return true;
}

void TxtEncoder::close()
{
	_fp.close();
}

void TxtEncoder::writeBOM( )
{
	switch ( _charset ){
	case UTF8:      // Unicode  (UTF-8)   : BOM�L��
		{
			const UCHAR p[3] = {0xef, 0xbb, 0xbf};
			_fp.write( (const void*)p, 3);
		}
		break;
	case UTF16b:    // Unicode  (UTF-16)  : BOM�L�� BE
        {
			const UCHAR p[2] = {0xfe, 0xff};
			_fp.write( (const void*)p, 2);
        }
		break;
	case UTF16l:    // Unicode  (UTF-16)  : BOM�L�� LE
        {
			const UCHAR p[2] = {0xff, 0xfe};
			_fp.write( (const void*)p, 2);
        }
		break;
	}
}

void TxtEncoder::writeLB( )
{
	switch( _charset ){
	case SJIS:
	case EucJP:
	case IsoJP:
	case UTF8:
	case UTF8N:
	case Latin:
		{
			// CR, LF, CRLF
			const USHORT lbstr[] = { 0x0d, 0x0a, 0x0a0d };
			const    int lblen[] = {    1,    1,      2 };

			_fp.write( (const void *)&lbstr[_lb-1], lblen[_lb-1] );
		}
		break;

	case UTF16b:    // Unicode  (UTF-16)  : BOM�L�� BE
	case UTF16BE:   // Unicode  (UTF-16BE): BOM����
		{
			const int lblen[] = { 2, 2, 4 };
			// CR, LF, CRLF
			const USHORT lbstr[]={0x0D00, 0x0A00, 0x0D00, 0x0A00};
			_fp.write( &lbstr[_lb-1], lblen[_lb-1] );
		}
		break;
	case UTF16l:    // Unicode  (UTF-16)  : BOM�L�� LE
	case UTF16LE:   // Unicode  (UTF-16LE): BOM����
		{
			const int lblen[] = { 2, 2, 4 };
			// CR, LF, CRLF
			const USHORT lbstr[]={0x000D, 0x000A, 0x000D, 0x000A};
			_fp.write( &lbstr[_lb-1], lblen[_lb-1] );
		}
		break;
	}
}

char *TxtEncoder::alloc( int size )
{
	if ( size > _buffsize ){
		DELA( _buff );
		_buffsize = size+BUFF_BLOCK;
		_buff = new char[ _buffsize ];
	}
	return _buff;
}

void	TxtEncoder::writeLn(const TCHAR* s,int len)
{
	if ( len == 0 ){
		return;
	}
	switch( _charset ){
	case SJIS:
		{
			DWORD charlength = WideCharToMultiByte(CP_ACP, 0,s,len,NULL, 0, NULL, NULL);
			char* pChar = alloc(charlength+1);
			if ( pChar !=NULL ){
				WideCharToMultiByte(CP_ACP, 0, s,len, pChar, charlength, NULL, NULL);
				_fp.write( pChar, charlength );
				//DELA( pChar );
			}
		}
		break;
	case EucJP:
		{
			// �܂�WCHAR��SJIS�ɕϊ�
			DWORD charlength = WideCharToMultiByte(CP_ACP, 0,s,len,NULL, 0, NULL, NULL);
			char* pChar = alloc(charlength+1);
			//char* pChar = new char [charlength+1];
			if ( pChar !=NULL ){
				WideCharToMultiByte(CP_ACP, 0, s,len, pChar, charlength, NULL, NULL);

				// ����SJIS->EUC�ɕϊ�
				EucTxt	euc;
				int		euclen=0;
				char	*eucstr = euc.FromSjis( pChar, charlength , euclen );

				// ���ʂ��o��
				_fp.write( eucstr, euclen );
				//DELA( pChar );
			}
		}
		break;
	case IsoJP:
		{
			// �܂�WCHAR��SJIS�ɕϊ�
			DWORD charlength = WideCharToMultiByte(CP_ACP, 0,s,len,NULL, 0, NULL, NULL);
			char* pChar = alloc(charlength+1);
			//char* pChar = new char [charlength+1];
			if ( pChar !=NULL ){
				WideCharToMultiByte(CP_ACP, 0, s,len, pChar, charlength, NULL, NULL);

				// ����SJIS->EUC�ɕϊ�
				Iso2022Txt	jis;
				int		jislen=0;
				char	*jisstr = jis.FromSjis( pChar, charlength , jislen );

				// ���ʂ��o��
				_fp.write( jisstr, jislen );
				//DELA( pChar );
			}
		}
		break;
	case UTF8:
	case UTF8N:
		{
			DWORD charlength = WideCharToMultiByte(CP_UTF8, 0,s,len,NULL, 0, NULL, NULL);
			char* pChar = alloc(charlength+1);
			//char* pChar = new char [charlength+1];
			if ( pChar !=NULL ){
				WideCharToMultiByte(CP_UTF8, 0, s,len, pChar, charlength, NULL, NULL);
				_fp.write( pChar, charlength );
				//DELA( pChar );
			}
		}
		break;

	case UTF16b:    // Unicode  (UTF-16)  : BOM�L�� BE
	case UTF16BE:   // Unicode  (UTF-16BE): BOM����
		{   // Big Endian
			for(int i=0;i<len;i++){
				USHORT c = (UCHAR)(s[i] >> 8) | ((s[i] & 0xff) << 8);
				_fp.write( &c, sizeof(USHORT) );
			}
		}
		break;
	case UTF16l:    // Unicode  (UTF-16)  : BOM�L�� LE
	case UTF16LE:   // Unicode  (UTF-16LE): BOM����
		{   // Little Endian
			_fp.write( s, len*sizeof(wchar_t) );
		}
		break;
	case Latin:
		{
			DWORD charlength = WideCharToMultiByte(1252, 0,s,len,NULL, 0, NULL, NULL);
			char* pChar = alloc(charlength+1);
			if ( pChar !=NULL ){
				WideCharToMultiByte(1252, 0, s,len, pChar, charlength, NULL, NULL);
				_fp.write( pChar, charlength );
				//DELA( pChar );
			}
		}
		break;
	}
}


void TxtEncoder::writeLine(const TCHAR* s, int	len, 
						   bool firstline, bool lastline)
{
	// �ŏ��̍s�̂Ƃ���BOM���o��
	if( firstline ) writeBOM();

	writeLn(s,len);

	// �Ō�ɉ��s�R�[�h�B
	// �������A�ŏI�s�ɂ͉��s�͂���Ȃ�
	if( !lastline ) writeLB();
}

