//
// jot - Text Editor for Windows Mobile
//   Copyright (C) 2007-2008, Aquamarine Networks.  <http://pandora.sblo.jp/>
//
//    This program is EveryOne's ware; you can redistribute it and/or modify it 
//    under the terms of the NYSL version 0.9982 or later. <http://www.kmonos.net/nysl/>
//

#pragma	once

// txt.h
//  テキストの Encode/Decode についての
//  基本的なクラスがここ。
#include "file.h"

//@{
//	変換可能な文字コード。
//@}
enum charset
{
	AutoDetect=0,// 自動判別

	SJIS,      // 日本語１ (Shift_JIS)
	EucJP,     // 日本語２ (日本語EUC)
	IsoJP,     // 日本語３ (ISO-2022-JP)

	UTF8,      // Unicode  (UTF-8)   : BOM有り
	UTF8N,     // Unicode  (UTF-8N)  : BOM無し
	UTF16b,    // Unicode  (UTF-16)  : BOM有り BE
	UTF16l,    // Unicode  (UTF-16)  : BOM有り LE
	UTF16BE,   // Unicode  (UTF-16BE): BOM無し
	UTF16LE,   // Unicode  (UTF-16LE): BOM無し

	Latin,	   // CP-1252 (Latin ISO-8859-1) 
};

//@{
//	文字コード判定のクラス。
//@}
class Txt
{
public:
	enum lbcode{ CR = 1, LF = 2, CRLF = 3 };

public:
	static charset autoDetect(const TCHAR *fname, lbcode* lb=NULL);
	static int     checkCharCode(const unsigned char* str);

private:
	static	charset checkBOM(FileR& r);
	static	charset checkUtf16(const UCHAR *p, const ULONG size);
	static lbcode  checkMultiLB(const UCHAR *p, const ULONG size);

};

//@{
//	テキスト読み込み用クラス。
//@}
class TxtDecoder
{
public:
	TxtDecoder(int charset);
	~TxtDecoder();

public:
	bool open(const TCHAR* fname);
	void close();

public:
	TCHAR	*readLine();
	int state()
    { return _state; }
	int charcode()
	{ return _charset; }

private:
	void	MbsToWcs( UINT CodePage );
	TCHAR	ChangeEndian( USHORT wc , bool le);
	void	Utf16ToWcs( bool le);

private:
	int			  _state;
	FileR         _fp;
	int           _charset;
	TCHAR*		  _buff;
	int			  _pos;
	int			  _size;
	const unsigned char *	_base;
	int			  _buffsize;
};

//@{
//	テキスト書き込み用クラス。
//@}
class TxtEncoder
{
public:
	TxtEncoder(int charset, int linebreak);
	~TxtEncoder();

public:
	bool open(const TCHAR* fname);
	void close();

public:
	void writeLine(const TCHAR *s, int len,
		bool firstline=false,
		bool lastline=false);

private:
	void writeBOM( );
	void writeLB( );
	void writeLn(const TCHAR* s,int len);
	char *alloc( int size );
private:
	FileW         _fp;
	int           _charset;
	int           _lb;
	char*		  _buff;
	int			  _buffsize;
};

//----------------------------------------------------------------
// JIS/EUC <-> SJIS変換の基本クラス
class JisTxt 
{
protected:
	char *m_cbuff;

public:
	JisTxt(): m_cbuff(NULL){}

	virtual ~JisTxt(){
		DELA( m_cbuff );
	}
	virtual char*	ToSjis(const char* in, int len ,int &sjislen )=0;
	virtual char*	FromSjis(const char* in, int sjislen , int &length )=0;
protected:
	virtual int getWriteBufSize(const char* in,int sjislen)=0;

	// JIS -> SJIS のサブ関数。
	unsigned short jis2sjis(unsigned char c1, unsigned char c2)
	{
		if (c1 % 2) {
			c1 = ((c1 + 1) / 2) + 0x70;
			c2 = c2 + 0x1f;
		} else {
			c1 = (c1 / 2) + 0x70;
			c2 = c2 + 0x7d;
		}
		if (c1 >= 0xa0) { c1 = c1 + 0x40; }
		if (c2 >= 0x7f) { c2 = c2 + 1; }

		return (c1 << 8) | c2;
	}

	// SJIS -> JIS のサブ関数。
	unsigned short sjis2jis(unsigned char c1, unsigned char c2)
	{
		if (c1 >= 0xe0) { c1 = c1 - 0x40; }
		if (c2 >= 0x80) { c2 = c2 - 1; }
		if (c2 >= 0x9e) {
			c1 = (c1 - 0x70) * 2;
			c2 = c2 - 0x7d;
		} else {
			c1 = ((c1 - 0x70) * 2) - 1;
			c2 = c2 - 0x1f;
		}
		return (c1 << 8) | c2;
	}
};


class EucTxt :public JisTxt
{
public:
	// EUC -> SJIS
	char*	ToSjis(const char* in, int len ,int &sjislen )
	{
		m_cbuff = new char[len+1];
		char *out = m_cbuff;
		const char *end = in + len;
		// 配列変換。
		for(; in<end ;)
		{
			// 半角ならなにもしない
			if( (UCHAR)*in <= 0x7f || in[1]==NULL ){
				*out++ = *in++; 
				continue; 
			}

			// 半角カナ
			if( (UCHAR)*in == 0x8e ){ 
				in++;
				*out++ = *in++;
				continue; 
			}

			unsigned short s = e2s(in[0], in[1]);
			*out++ = (char)( s >> 8 );
			*out++ = (char)( s & 0x00ff );
			in+=2;
		}
		*out = '\0';
		sjislen = out - m_cbuff;
		return m_cbuff;
	}

	// SJIS ->EUC
	char*	FromSjis(const char* in, int sjislen , int &length )
	{
		length = getWriteBufSize(in,sjislen);

		m_cbuff = new char[length+1];
		char *out = m_cbuff;
		const char *end = in + sjislen;

		// 配列変換
		for( ; in<end ; )
		{
			// 半角カナ
			if( (UCHAR)*in >= 0xa0 && (UCHAR)*in <= 0xdf ){ 
				*out++ = (char)0x8e; 
				*out++ = *in++; 
				continue; 
			}
			// 半角
			if( (UCHAR)*in <=0x7f || in[1]==NULL ){ 
				*out++ = *in++; 
				continue; 
			}

			unsigned short e = s2e(in[0], in[1]);
			*out++ = (char)(e>>8);
			*out++ = (char)(e&0x00ff);
			in += 2;
		}
		*out = '\0';
		return m_cbuff;
	}

private:
	int getWriteBufSize(const char* in,int sjislen)
	{
		int len = 0;
		const char *end = in + sjislen;
		for(; in<end; ){
			// 半角カナ
			if( (UCHAR)*in >= 0xa0 && (UCHAR)*in <= 0xdf ){ 
				len+=2;
				in++;
			}else
			// 半角
			if( (UCHAR)*in <=0x7f || in[1]==NULL ){ 
				len++;
				in++;
			}
			// その他全角
			else{
				len +=2;
				in +=2;
			}
		}
		return len;
	}

	// EUC -> SJIS のサブ関数。
	unsigned short e2s(unsigned char c1, unsigned char c2)
	{
		c1 &= ~0x80;
		c2 &= ~0x80;

		return jis2sjis( c1,c2 );
	}

	// SJIS -> EUC のサブ関数。
	unsigned short s2e(unsigned char c1, unsigned char c2)
	{
		return sjis2jis(c1,c2) | 0x8080;
	}

};

//----------------------------------------------------------------
class Iso2022Txt : public JisTxt
{
protected:
	enum codeset{ ASCII, JIS, KANA };
	codeset _cs;

public:
	Iso2022Txt():_cs(ASCII)	{}

public:

	// JIS -> SJIS
	char*	ToSjis(const char* in, int len ,int &sjislen )
	{
		m_cbuff = new char[len+1];
		char *out = m_cbuff;
		const char *end = in + len;
		// 配列変換。
		for(; in<end ;)
		{
			if( checkCodeSet(in) ){
				in+=3; 
				continue; 
			}

			if( _cs==ASCII )
			{
				*out++ = *in++;
			}
			else if( _cs==KANA )
			{
				*out++ = *in++ + 0x80;
			}
			else
			{
				unsigned short s = j2s(in[0],in[1]);
				*out++ = (char)( s >> 8 );
				*out++ = (char)( s & 0x00ff );
				in+=2;
			}
		}
		*out = '\0';
		sjislen = out - m_cbuff;
		return m_cbuff;
	}
	// SJIS -> JIS
	char*	FromSjis(const char* in, int sjislen , int &length )
	{
		length = getWriteBufSize(in,sjislen);

		m_cbuff = new char[length+1];
		char *out = m_cbuff;
		const char *end = in + sjislen;

		// 配列変換
		for( ; in<end ; )
		{
			// 半角カナ
			if( (UCHAR)*in >= 0xa0 && (UCHAR)*in <= 0xdf )
			{
				if( _cs!=KANA ) 
					outputCodeSet(out, KANA), out+=3, _cs=KANA;
				 *out++ = *in++ - 0x80;
			}
			// 半角
			else if( (UCHAR)*in <=0x7f || in[1]==NULL )
			{
				if( _cs!=ASCII )
					outputCodeSet(out, ASCII), out+=3, _cs=ASCII;
				*out++ = *in++;
			}
			else
			{
				if( _cs!=JIS ) 
					outputCodeSet(out, JIS), out+=3, _cs=JIS;
				unsigned short j = s2j(in[0],in[1]);
				*out++ = (char)(j>>8);
				*out++ = (char)(j&0x00ff);
				in += 2;
			}
		}
		outputCodeSet(out, ASCII), out+=3, _cs=ASCII;
		*out = '\0';
		return m_cbuff;
	}

private:
	int getWriteBufSize(const char* in,int sjislen)
	{
		int len = 3;
		const char *end = in + sjislen;
		codeset cs=ASCII;
		for(; in<end; ){
			// 半角カナ
			if( (UCHAR)*in >= 0xa0 && (UCHAR)*in <= 0xdf )
			{
				if( cs!=KANA ) len+=3, cs=KANA;
				len++, in++;
			}
			// 半角
			else if( (UCHAR)*in <=0x7f || in[1]==NULL )
			{
				if( cs!=ASCII ) len+=3, cs=ASCII;
				len++, in++;
			}
			// 全角(2byte必要)
			else
			{
				if( cs!=JIS ) 
					len+=3, cs=JIS;
				len+=2, in+=2;
			}
		}
		return len;
	}


	// JIS -> SJIS のサブ関数。
	unsigned short j2s(unsigned char hib, unsigned char lob)
	{
		return jis2sjis( hib,lob );
	}


	// SJIS -> JIS のサブ関数。
	unsigned short s2j(unsigned char hib, unsigned char lob)
	{
		return sjis2jis(hib ,lob);
	}

	// CodeSet判定
	bool checkCodeSet(const char* p)
	{
		//
		if( p[0]!=0x1B ) return false;
		if( p[1]==0x24 && p[2]==0x42 )
		{
			_cs = JIS; return true;
		}
		else if( p[1]==0x28 )
		{
			if( p[2]==0x42 )
			{
				_cs = ASCII; return true;
			}
			else if( p[2]==0x49 )
			{
				_cs = KANA;  return true;
			}
		}
		return false;
	}

	// CodeSet 出力
	void outputCodeSet(char* p, codeset cs)
	{
		if( cs==ASCII )
		{
			*p++ = 0x1B; *p++ = 0x28; *p++ = 0x42;
		}
		else if( cs==KANA )
		{
			*p++ = 0x1B; *p++ = 0x28; *p++ = 0x49;
		}
		else if( cs==JIS )
		{
			*p++ = 0x1B; *p++ = 0x24; *p++ = 0x42;
		}
	}

};
