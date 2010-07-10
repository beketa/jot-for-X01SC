/**************************************************************************

	regex.h

	Copyright (C) 1996-2002 by Tomoaki Nakashima. All rights reserved.
		http://www.nakka.com/
		nakka@nakka.com

**************************************************************************/

#pragma	warning( disable:4996 )

# ifdef __cplusplus
extern "C" {
# endif

/**************************************************************************
	Include Files
**************************************************************************/

/**************************************************************************
	Define
**************************************************************************/

/**************************************************************************
	Struct
**************************************************************************/

//NFA リスト
typedef struct _NFA_LIST {
	int op;
	union {
		TCHAR *chr_class;
		int i;
		struct {
			TCHAR c1;
			TCHAR c2;
		} c;
	} u;
	int eps;
	int to;
	struct _NFA_LIST *next;
} NFA_LIST;


//NFA 情報
typedef struct _REGE_NFA {
	struct _NFA_LIST **states;
	int entry;
	int exit;
	int nstate;
	int rfcnt;
	BOOL head_flag;
	BOOL tail_flag;
} REGE_NFA;


//後方参照リスト
typedef struct _REGE_REFER {
	const TCHAR *st;
	const TCHAR *en;
} REGE_REFER;


/**************************************************************************
	Function Prototypes
**************************************************************************/

void free_nfa(REGE_NFA *nfa);
REGE_NFA *rege_compile(const TCHAR *ptn);
int reg_match(REGE_NFA *nfa, const TCHAR *str, REGE_REFER **rf, BOOL icase);
void free_refer(REGE_REFER *rf);
int reg_match_intext(REGE_NFA *nfa, const TCHAR *str, REGE_REFER **rf, BOOL icase , unsigned int st , BOOL forward);

# ifdef __cplusplus
}
# endif
