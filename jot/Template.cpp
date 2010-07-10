#include "StdAfx.h"
#include "Template.h"

#include "txt.h"

#if 0
�E�e���v���[�g��UTF-8(BOM����)�ŋL�q�����K�v������܂��B

�E��{���@
  ;�Ŏn�܂�s�̓R�����g�ƂȂ�܂��B

  ��s�ꌏ�̌`�ŁA�ȉ��̌`���ŋL�q���܂��B
  (�\����)=(�}��������)

  �\�����̑O��̋󔒂͖�������܂��B

�E�ȉ��̃e���v���[�g�ϐ����g�p�ł��܂��B
%year%		�N	2008	(����4��)
%month%		��	04	(2��)
%date%		��	30	(2��)
%dayofweekj%	�j��	��	(���{��\�L)
%dayofweek%	�j��	Fri	(�p��\�L)
%hour%		��	01	(2��)
%minuite%	��	02	(2��)
%second%	�b	03	(2��)
%%		%
%@%		�I�𕶎���(�ő�100������)
%!%		�e���v���[�g�K�p��J�[�\���ʒu
%n%		���s
%t%		�^�u

��L�ȊO��%�`%	�\��(�󕶎���ɒu��)

�ϐ����̑啶���������͋�ʂ���܂���B
%!%���g�p���Ȃ��ꍇ�A�u��������̍Ō�ɃJ�[�\�����ړ����܂��B

�E��
-------------------------------------
;�R�����g
;�R�����g
���t=%year%/%month%/%date% (%dayofweek%)
����=%hour%:%minuite%:%second%
����=�u%@%%!%�v
;���́A�I�𕶎�����u�v�ł���񂾏�ŁA�v�̒��O�ɃJ�[�\�����ړ�����
���A=����ɂ���%n%���C�ł����H
-------------------------------------


#endif

CTemplate::CTemplate(int max ,const TCHAR *filename)
{
	m_max = max;

	// ���s�t�@�C��������f�B���N�g���� ini ������B
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

		// ���ݓ��t�̎擾
		SYSTEMTIME  st;
		//  ���ݎ����̎擾
		GetLocalTime( &st );

		CString	temp = m_list.GetAt(idx)->temp;
		
		int pos = 0;
		caret = -1;

		while(1){
			// %��T��
			int	f = temp.Find( _T('%') , pos );
			if ( f == -1 ){
				// %��������΍Ō�܂ŃR�s�[���đł��؂�
				ret += temp.Mid( pos );
				break;
			}else{
				// %������΁A���̎�O�܂ŃR�s�[
				ret += temp.Mid( pos ,f-pos );
				pos = f+1;

				// ����%��T��
				f = temp.Find( _T('%') , pos );
				if ( f==-1 ){
					// ����%��������Αł��؂�
					break;
				}else{
					// ����%������Εϐ����擾
					CString var = temp.Mid( pos , f-pos );
					pos = f+1;

					// �ϐ�������������
					var = var.MakeLower();

					CString tmp;

					// �ϐ��̏���
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
						const TCHAR *dow[] = { _T("��"),_T("��"),_T("��"),_T("��"),_T("��"),_T("��"),_T("�y"),};
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


