
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <locale.h>
#include "regex.h"

extern int testmain();
extern int testmaint();

int main(void)
{
/*
	REGE_NFA *nfa;
	REGE_REFER *rf;
	TCHAR buf[256];
	TCHAR *regex;
	TCHAR *p;
	int rc;
	int i;

//�}�b�`���镶����Ɗ��ʂɑΉ�����������̕\�� (����Q��)

	//�R���p�C��
	regex = _T("((abc|bef)[12345]*)xyz");
	_tprintf(_T("regex: %s\n"), regex);
	nfa = rege_compile(regex);

	//����
	p = _T("sdfabc423xyzaadf");
	_tprintf(_T("string: %s\n"), p);
	rc = reg_match(nfa, p, &rf, FALSE);
	if(rc > 0){
		for(i = 0; i < rc; i++){
			if(rf[i].st == NULL || rf[i].en == NULL){
				break;
			}
			lstrcpyn(buf, rf[i].st, rf[i].en - rf[i].st + 1);
			if(i == 0){
				//�}�b�`����������
				_tprintf(_T("match: %s\n"), buf);
			}else{
				//���ʂɑΉ�����������
				_tprintf(_T("%d: %s\n"), i, buf);
			}
		}
	}else{
		_tprintf(_T("no match\n"));
	}
	free_refer(rf);
	free_nfa(nfa);
	rf = NULL;
	nfa = NULL;

	_tprintf(_T("\n"));
	
//�}�b�`������̂�S�ĕ\��

	//�R���p�C��
	regex = _T("[^-]*");
	_tprintf(_T("regex: %s\n"), regex);
	nfa = rege_compile(regex);

	//����
	p = _T("abc-de-f-g");
	_tprintf(_T("string: %s\n"), p);
	rc = reg_match(nfa, p, &rf, FALSE);
	while(rc > 0){
		if(p == rf[0].st && p == rf[0].en){
			if(*(rf[0].en + 1) == _T('\0')){
				free_refer(rf);
				rf = NULL;
				break;
			}
			p = rf[0].en + 1;
		}else{
			//�}�b�`����������̕\��
			lstrcpyn(buf, rf[0].st, rf[0].en - rf[0].st + 1);
			_tprintf(_T("match: %s\n"), buf);
			p = rf[0].en;
		}
		free_refer(rf);
		rf = NULL;
		if(nfa->head_flag == TRUE){
			// ^
			for(; *p != _T('\0') && *p != _T('\r') && *p != _T('\n'); p++);
			for(; *p == _T('\r') || *p == _T('\n'); p++);
			if(*p == _T('\0')){
				break;
			}
		}
		//���̌���
		rc = reg_match(nfa, p, &rf, FALSE);
	}
	free_refer(rf);
	free_nfa(nfa);
	rf = NULL;
	nfa = NULL;
*/
	setlocale(LC_ALL, "Japanese_Japan.932");

	testmain();
	testmaint();
	getchar();
	return 0;
}
