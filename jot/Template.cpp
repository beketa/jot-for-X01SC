#include "StdAfx.h"
#include "Template.h"

#include "txt.h"

#if 0
・テンプレートはUTF-8(BOM無し)で記述される必要があります。

・基本文法
  ;で始まる行はコメントとなります。

  一行一件の形で、以下の形式で記述します。
  (表示名)=(挿入文字列)

  表示名の前後の空白は無視されます。

・以下のテンプレート変数が使用できます。
%year%		年	2008	(西暦4桁)
%month%		月	04	(2桁)
%date%		日	30	(2桁)
%dayofweekj%	曜日	金	(日本語表記)
%dayofweek%	曜日	Fri	(英語表記)
%hour%		時	01	(2桁)
%minuite%	分	02	(2桁)
%second%	秒	03	(2桁)
%%		%
%@%		選択文字列(最大100万文字)
%!%		テンプレート適用後カーソル位置
%n%		改行
%t%		タブ

上記以外の%～%	予約(空文字列に置換)

変数名の大文字小文字は区別されません。
%!%を使用しない場合、置換文字列の最後にカーソルが移動します。

・例
-------------------------------------
;コメント
;コメント
日付=%year%/%month%/%date% (%dayofweek%)
時刻=%hour%:%minuite%:%second%
括弧=「%@%%!%」
;↑は、選択文字列を「」でくるんだ上で、」の直前にカーソルを移動する
挨拶=こんにちは%n%元気ですか？
-------------------------------------


#endif

CTemplate::CTemplate(int max ,const TCHAR *filename)
{
	m_max = max;

	// 実行ファイルがあるディレクトリに ini がある。
	TCHAR temp[MAX_PATH*2]={0};
	GetModuleFileName(NULL, temp, MAX_PATH*2-1);
	CString s = temp;

	size_t pos = s.ReverseFind(_T('\\'));
	m_filename = s.Left(pos+1) + filename;

	TxtDecoder dec(UTF8N);

	if ( dec.open( m_filename ) ){
		int	cnt =0;
		while( dec.state() != 0 ){
			CString	str;
			str = dec.readLine();
			Temp *line = new Temp(str);
			if ( line->name.GetLength() >0 
				&& line->temp.GetLength()> 0 
				&& *line->name!=_T(';') ){
				m_list.Add(line);
				cnt ++;
				if ( cnt >= max ) break;
			}else{
				DEL( line );
			}
		}
		dec.close();
	}
}

CTemplate::~CTemplate(void)
{
	int	cnt = m_list.GetCount();
	for( int i=0;i<cnt;i++){
		Temp *p = m_list.GetAt(i);
		DEL( p );
	}
}



CString	CTemplate::ProcTemplate( int idx , CString src , int &caret )
{
	CString	ret;

	if ( idx < m_list.GetCount() ){

		// 現在日付の取得
		SYSTEMTIME  st;
		//  現在時刻の取得
		GetLocalTime( &st );

		CString	temp = m_list.GetAt(idx)->temp;
		
		int pos = 0;
		caret = -1;

		while(1){
			// %を探す
			int	f = temp.Find( _T('%') , pos );
			if ( f == -1 ){
				// %が無ければ最後までコピーして打ち切り
				ret += temp.Mid( pos );
				break;
			}else{
				// %があれば、その手前までコピー
				ret += temp.Mid( pos ,f-pos );
				pos = f+1;

				// 次の%を探す
				f = temp.Find( _T('%') , pos );
				if ( f==-1 ){
					// 閉じる%が無ければ打ち切り
					break;
				}else{
					// 閉じる%があれば変数名取得
					CString var = temp.Mid( pos , f-pos );
					pos = f+1;

					// 変数名を小文字化
					var = var.MakeLower();

					CString tmp;

					// 変数の処理
					if ( var == _T("year") ){
						tmp.Format( _T("%04d") , st.wYear );
						ret += tmp;
					}else
					if ( var == _T("month") ){
						tmp.Format( _T("%02d") , st.wMonth );
						ret += tmp;
					}else
					if ( var == _T("date") ){
						tmp.Format( _T("%02d") , st.wDay );
						ret += tmp;
					}else
					if ( var == _T("dayofweekj") ){
						const TCHAR *dow[] = { _T("日"),_T("月"),_T("火"),_T("水"),_T("木"),_T("金"),_T("土"),};
						ret += dow[st.wDayOfWeek];
					}else
					if ( var == _T("dayofweek") ){
						const TCHAR *dow[] = { _T("Sun"),_T("Mon"),_T("Tue"),_T("Wed"),_T("Thu"),_T("Fri"),_T("Sat"),};
						ret += dow[st.wDayOfWeek];
					}else
					if ( var == _T("hour") ){
						tmp.Format( _T("%02d") , st.wHour );
						ret += tmp;
					}else
					if ( var == _T("minute") ){
						tmp.Format( _T("%02d") , st.wMinute );
						ret += tmp;
					}else
					if ( var == _T("second") ){
						tmp.Format( _T("%02d") , st.wSecond );
						ret += tmp;
					}else
					if ( var == _T("!") ){
						caret = _tcslen( ret );
					}else
					if ( var == _T("@") ){
						ret += src;
					}else
					if ( var == _T("") ){
						ret += _T("%");
					}else
					if ( var == _T("n") ){
						ret += _T("\r");
					}else
					if ( var == _T("t") ){
						ret += _T("\t");
					}else
					{
					}


				}

			}
		}
		if ( caret == -1 ){
			caret = _tcslen(ret);
		}
	}
	return ret;
}


