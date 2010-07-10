/**************************************************************************

	regex.c

	Copyright (C) 1996-2002 by Tomoaki Nakashima. All rights reserved.
		http://www.nakka.com/
		nakka@nakka.com

	参考文献:
		「Ｃプログラマのためのアルゴリズムとデータ構造 Part2」
		近藤嘉雪 著    ソフトバンク    1993年12月25日発行

**************************************************************************/

/**************************************************************************
	Include Files
**************************************************************************/

#include <windows.h>
#include <tchar.h>

#include "regex.h"


/**************************************************************************
	Define
**************************************************************************/

#define ToLower(c)				((c >= _T('A') && c <= _T('Z')) ? (c - _T('A') + _T('a')) : c)

#define CHAR_SPACE_SIZE			256
#define EPSILON_TRANS			-1		// ε遷移

#define CHAR_DOT				_T("\x09\x20-\ufffd")


/**************************************************************************
	Global Variables
**************************************************************************/

//Tree
typedef enum {
	TOKEN_CHAR,						// 通常の文字
	TOKEN_GROUP_OPEN,				// 正規表現のグループ化の開始
	TOKEN_GROUP_CLOSE,				// 正規表現のグループ化の終了
	TOKEN_INTERVAL_ZERO_MORE_NG,	// 任意の数(0を含む)の並び (non-greedy)
	TOKEN_INTERVAL_ONE_MORE_NG,		// 1回以上の並び (non-greedy)
	TOKEN_INTERVAL_ZERO_ONE_NG,		// 0回か1回の並び (non-greedy)
	TOKEN_INTERVAL_CLOSE_NG,		// 繰り返しの指定終了 (non-greedy)
	TOKEN_INTERVAL_ZERO_MORE,		// 任意の数(0を含む)の並び
	TOKEN_INTERVAL_ONE_MORE,		// 1回以上の並び
	TOKEN_INTERVAL_ZERO_ONE,		// 0回か1回の並び
	TOKEN_INTERVAL_OPEN,			// 繰り返しの指定開始
	TOKEN_INTERVAL_CLOSE,			// 繰り返しの指定終了
	TOKEN_ANY_CHAR,					// 改行を除く任意の1文字
	TOKEN_LIST_OPEN,				// 文字リストの指定開始
	TOKEN_LIST_CLOSE,				// 文字リストの指定終了
	TOKEN_LINE_BEGIN,				// 行の先頭
	TOKEN_LINE_END,					// 行の末尾
	TOKEN_ALTERNATION,				// 選択
	TOKEN_META_ESCAPE,				// メタエスケープ
	TOKEN_END,						// トークンの終了
	TOKEN_LIST_NOT,					// 文字リストの否定指定
	TOKEN_LIST_RANGE,				// 文字リストの範囲指定
	TOKEN_LIST_ESCAPE,				// 文字リストのメタエスケープ
	TOKEN_INTERVAL_RANGE,			// 繰り返しの範囲指定
} TOKEN;

static TCHAR *token_chars[] = {
	_T("c"),	_T("("),	_T(")"),	_T("*?"),	_T("+?"),	_T("??"),	_T("}?"),
	_T("*"),	_T("+"),	_T("?"),	_T("{"),	_T("}"),	_T("."),	_T("["),
	_T("]"),	_T("^"),	_T("$"),	_T("|"),	_T("\\"),
	_T(""),		_T("^"),	_T("-"),	_T("\\"),	_T(","),
};

typedef enum {
	OP_CHAR,						// 文字
	OP_HEAD,						// 行頭
	OP_TAIL,						// 行末
	OP_CHARCLASS,					// キャラクタクラス
	OP_CONCAT,						// XY
	OP_UNION,						// X|Y
	OP_CLOSURE,						// X*
	OP_CLOSURE_NG,					// X*?
	OP_EMPTY,						// 空
	OP_OPEN,						// (
	OP_CLOSE,						// )
} OP;


typedef struct _TREE {
	OP op;
	union {
		TCHAR *chr_class;
		int i;
		struct {
			TCHAR c1;
			TCHAR c2;
		} c;
		struct {
			struct _TREE *left;
			struct _TREE *right;
		} x;
	} u;
} TREE;

typedef struct _REGE_PARSER {
	TREE *tree;
	TOKEN current_token;
	TCHAR token_char1;
	TCHAR token_char2;
	TCHAR *p;
	TCHAR user_char_set[CHAR_SPACE_SIZE];
	int intervals_open, intervals_close;
	int rfcnt;
} REGE_PARSER;


//match
typedef struct _REGE_MATCH {
	REGE_NFA *nfa;
	REGE_REFER *rf;
	BOOL icase;
} REGE_MATCH;


/**************************************************************************
	Local Function Prototypes
**************************************************************************/

//memory
static void *mem_alloc(int size);
static void mem_free(void **mem);
static TCHAR *alloc_copy(const TCHAR *buf);

//string
static TCHAR *str_cpy_n(TCHAR *ret, TCHAR *buf, int len);
static int str_cmp_n(const TCHAR *buf1, const TCHAR *buf2, int len);
static long x2d(const TCHAR *str);

//tree
static TCHAR *get_esc_string(TCHAR *c, TCHAR *ret, BOOL expand_flag);
static void get_token(REGE_PARSER *parser);
static TREE *make_tree_node(REGE_PARSER *parser, OP op, TREE *left, TREE *right);
static TREE *copy_tree(REGE_PARSER *parser, TREE *p);
static TREE *primary(REGE_PARSER *parser);
static TREE *factor(REGE_PARSER *parser);
static TREE *term(REGE_PARSER *parser);
static TREE *regexp(REGE_PARSER *parser);
static REGE_PARSER *parse(TCHAR *pattern);
static void free_tree(TREE *p);

//NFA
static NFA_LIST *add_transition(REGE_NFA *nfa, int from, int to, int eps, OP op);
static void generate_nfa(REGE_NFA *nfa, TREE *tree, int entry, int way_out);
static int generate_nfa_count(TREE *tree);
static REGE_NFA *build_nfa(REGE_PARSER *parser);

//match
static BOOL charclass_match(TCHAR *p, TCHAR *c);
static TCHAR *rege_match(REGE_MATCH *rm, int i, TCHAR *c,int depth);


/******************************************************************************

	mem_alloc

	バッファを確保

******************************************************************************/

static void *mem_alloc(int size)
{
	return LocalAlloc(LMEM_FIXED, size);
}


/******************************************************************************

	mem_free

	バッファを解放

******************************************************************************/

static void mem_free(void **mem)
{
	if(*mem != NULL){
		LocalFree(*mem);
		*mem = NULL;
	}
}


/******************************************************************************

	alloc_copy

	バッファを確保して文字列をコピーする

******************************************************************************/

static TCHAR *alloc_copy(const TCHAR *buf)
{
	TCHAR *ret;

	if(buf == NULL){
		return NULL;
	}
	ret = (TCHAR *)mem_alloc(sizeof(TCHAR) * (_tcslen(buf) + 1));
	if(ret != NULL){
		_tcscpy(ret, buf);
	}
	return ret;
}


/******************************************************************************

	str_cpy_n

	指定された文字数まで文字列をコピーする

******************************************************************************/

static TCHAR *str_cpy_n(TCHAR *ret, TCHAR *buf, int len)
{
	if(buf == NULL || len <= 0){
		*ret = _T('\0');
		return ret;
	}
	while(--len && (*(ret++) = *(buf++)));
	*ret = _T('\0');
	if(len != 0) ret--;
	return ret;
}


/******************************************************************************

	str_cmp_n

	２つの文字列を文字数分比較を行う

******************************************************************************/

static int str_cmp_n(const TCHAR *buf1, const TCHAR *buf2, int len)
{
	int i = 0;

	for(; *buf1 == *buf2 && *buf1 != _T('\0') && i < len; i++, buf1++, buf2++);
	return ((i == len) ? 0 : *buf1 - *buf2);
}


/******************************************************************************

	x2d

	16進表記の文字列を数値に変換する

******************************************************************************/

static long x2d(const TCHAR *str)
{
	int num = 0;
	int m;

	for(; *str != _T('\0'); str++){
		if(*str >= _T('0') && *str <= _T('9')){
			m = *str - _T('0');
		}else if(*str >= _T('A') && *str <= _T('F')){
			m = *str - _T('A') + 10;
		}else if(*str >= _T('a') && *str <= _T('f')){
			m = *str - _T('a') + 10;
		}else{
			break;
		}
		num = 16 * num + m;
	}
	return num;
}


/******************************************************************************

	get_esc_string

	エスケープ文字の変換

******************************************************************************/

static TCHAR *get_esc_string(TCHAR *c, TCHAR *ret, BOOL expand_flag)
{
	if(expand_flag == FALSE){
		switch(*(c + 1))
		{
		case _T('\\'):
		case _T('w'):
		case _T('W'):
		case _T('d'):
		case _T('D'):
		case _T('s'):
		case _T('S'):
			*ret = *(c++);
			*(ret + 1) = *(c++);
			*(ret + 2) = _T('\0');
			return c;
		}
	}

	*(ret + 1) = _T('\0');
	c++;

	switch(*c)
	{
	case _T('r'):
	case _T('R'):
		*ret = _T('\r');
		break;

	case _T('n'):
	case _T('N'):
		*ret = _T('\n');
		break;

	case _T('t'):
	case _T('T'):
		*ret = _T('\t');
		break;

	case _T('w'):
		_tcscpy(ret, _T("0-9a-zA-Z_"));
		break;

	case _T('W'):
		_tcscpy(ret, _T("^0-9a-zA-Z_"));
		break;

	case _T('d'):
		_tcscpy(ret, _T("0-9"));
		break;

	case _T('D'):
		_tcscpy(ret, _T("^0-9"));
		break;

	case _T('s'):
		_tcscpy(ret, _T("\t\r\n "));
		break;

	case _T('S'):
		_tcscpy(ret, _T("^\t\r\n ") );
		break;

	case _T('x'):
	case _T('X'):
		//Char (Hex)
		c++;
		if(*c != _T('\0')){
			TCHAR tmp[10], *r;

			for(r = c; (r - c) < 2 && ((*r >= _T('0') && *r <= _T('9'))
				|| (*r >= _T('A') && *r <= _T('F'))
				|| (*r >= _T('a') && *r <= _T('f'))); r++);
			str_cpy_n(tmp, c, r - c + 1);
			*ret = (TCHAR)x2d(tmp);
			c += r - c - 1;
		}
		break;
	case _T('u'):
	case _T('U'):
		//Unicode Char (Hex)
		c++;
		if(*c != _T('\0')){
			TCHAR tmp[10], *r;

			for(r = c; (r - c) < 4 && ((*r >= _T('0') && *r <= _T('9'))
				|| (*r >= _T('A') && *r <= _T('F'))
				|| (*r >= _T('a') && *r <= _T('f'))); r++);
			str_cpy_n(tmp, c, r - c + 1);
			*ret = (TCHAR)x2d(tmp);
			c += r - c - 1;
		}
		break;

	default:
		*ret = *c;
		break;
	}
	return (c + 1);
}


/******************************************************************************

	get_token

	トークンの取得

******************************************************************************/

static void get_token(REGE_PARSER *parser)
{
	TOKEN i;
	int l;
	TCHAR *c;

	parser->token_char1 = _T('\0');
	parser->token_char2 = _T('\0');
	parser->current_token = TOKEN_CHAR;

	for(i = TOKEN_CHAR + 1; i < TOKEN_END; i++){
		if(*parser->p == _T('\0')){
			parser->current_token = TOKEN_END;
			return;
		}
		if((l = _tcslen(*(token_chars + i))) &&
			str_cmp_n(parser->p, *(token_chars + i), l) == 0){
			parser->current_token = i;
			break;
		}
	}

	c = parser->p;
	parser->p += _tcslen(*(token_chars + parser->current_token));

	switch(parser->current_token)
	{
	case TOKEN_CHAR:
		parser->token_char1 = *c;
		//if(IsDBCSLeadByte((BYTE)*c)){
		//	parser->token_char2 = *parser->p++;
		//}
		break;

	case TOKEN_ANY_CHAR:
		parser->current_token = TOKEN_LIST_CLOSE;
		_tcscpy(parser->user_char_set, CHAR_DOT);
		break;

	case TOKEN_LINE_BEGIN:
		parser->token_char1 = *c;
		break;

	case TOKEN_LINE_END:
		parser->token_char1 = *c;
		break;

	case TOKEN_INTERVAL_OPEN:
		if(*parser->p != _T('\0')){
			int *i = &parser->intervals_open;

			parser->intervals_open = 0;
			parser->intervals_close = 0;
			c = parser->p++;
			while(*c != _T('\0')){
				if(*c == **(token_chars + TOKEN_INTERVAL_CLOSE)){
					if(i == &parser->intervals_open){
						parser->intervals_close = parser->intervals_open;
					}
					parser->current_token = TOKEN_INTERVAL_CLOSE;
					break;

				}else if(*c == **(token_chars + TOKEN_INTERVAL_RANGE)){
					i = &parser->intervals_close;
					c = parser->p;
					parser->p++;

				}else if(*c >= _T('0') && *c <= _T('9')){
					*i = *i * 10 + (*c - _T('0'));
					c = parser->p++;

				}else{
					c = parser->p++;
				}
			}
		}
		if(parser->current_token != TOKEN_INTERVAL_CLOSE){
			parser->current_token = TOKEN_END;
		}else{
			if(*parser->p == _T('?')){
				parser->current_token = TOKEN_INTERVAL_CLOSE_NG;
				parser->p++;
			}
		}
		break;

	case TOKEN_LIST_OPEN:
		if(*parser->p != _T('\0')){
			int i = 0;

			c = parser->p++;
			if(*c == **(token_chars + TOKEN_LIST_CLOSE)){
				*(parser->user_char_set + i++) = **(token_chars + TOKEN_LIST_CLOSE);
				c = parser->p;
				parser->p++;
			}

			if(*c == **(token_chars + TOKEN_LIST_NOT) &&
				*parser->p == **(token_chars + TOKEN_LIST_CLOSE)){
				*(parser->user_char_set + i++) = **(token_chars + TOKEN_LIST_NOT);
				c = parser->p++;
				*(parser->user_char_set + i++) = **(token_chars + TOKEN_LIST_CLOSE);
				c = parser->p++;
			}

			while(*c != _T('\0')){
				if(*c == **(token_chars + TOKEN_LIST_CLOSE)){
					*(parser->user_char_set + i) = _T('\0');
					parser->current_token = TOKEN_LIST_CLOSE;
					break;
				}else if(*c == **(token_chars + TOKEN_LIST_ESCAPE)){
					TCHAR tmp[CHAR_SPACE_SIZE];
					parser->p = get_esc_string(c, tmp, FALSE);
					_tcscpy(parser->user_char_set + i, tmp);
					i += _tcslen(tmp);
				}else{
					//if(IsDBCSLeadByte((BYTE)*c) == TRUE){
					//	*(parser->user_char_set + i++) = *c;
					//	c = parser->p++;
					//}
					*(parser->user_char_set + i++) = *c;
				}
				c = parser->p++;
			}
		}
		if(parser->current_token != TOKEN_LIST_CLOSE) {
			parser->current_token = TOKEN_END;
		}
		break;

	case TOKEN_META_ESCAPE:
		if(*parser->p != _T('\0')){
			parser->p = get_esc_string(c, parser->user_char_set, TRUE);
			parser->current_token = TOKEN_LIST_CLOSE;

			if(_tcslen(parser->user_char_set) == 1){
				parser->current_token = TOKEN_CHAR;
				parser->token_char1 = *parser->user_char_set;
			}
		}
		break;
	}
}


/******************************************************************************

	make_tree_node

	ノードの作成

******************************************************************************/

static TREE *make_tree_node(REGE_PARSER *parser, OP op, TREE *left, TREE *right)
{
	TREE *p;

	if((p = mem_alloc(sizeof(TREE))) == NULL){
		return NULL;
	}
	ZeroMemory(p, sizeof(TREE));
	p->op = op;
	p->u.x.left = left;
	p->u.x.right = right;
	return p;
}


/******************************************************************************

	copy_tree

	treeのコピー

******************************************************************************/

static TREE *copy_tree(REGE_PARSER *parser, TREE *p)
{
	TREE *x;

	if(p == NULL){
		return NULL;
	}
	switch(p->op)
	{
	case OP_CONCAT:
	case OP_UNION:
		x = make_tree_node(parser, p->op,
			copy_tree(parser, p->u.x.left), copy_tree(parser, p->u.x.right));
		break;

	case OP_CLOSURE:
		x = make_tree_node(parser, p->op,
			copy_tree(parser, p->u.x.left), NULL);
		break;

	case OP_CHAR:
		x = make_tree_node(parser, p->op, NULL, NULL);
		x->u.c.c1 = p->u.c.c1;
		x->u.c.c2 = p->u.c.c2;
		break;

	case OP_CHARCLASS:
		x = make_tree_node(parser, p->op, NULL, NULL);
		x->u.chr_class = alloc_copy(p->u.chr_class);
		break;

	default:
		x = make_tree_node(parser, p->op, NULL, NULL);
		break;
	}
	return x;
}


/******************************************************************************

	primary

	X

******************************************************************************/

static TREE *primary(REGE_PARSER *parser)
{
	TREE *x = NULL;
	TREE *o, *c;

	switch(parser->current_token)
	{
	case TOKEN_CHAR:
		// X
		x = make_tree_node(parser, OP_CHAR, NULL, NULL);
		x->u.c.c1 = parser->token_char1;
		x->u.c.c2 = parser->token_char2;
		get_token(parser);
		break;

	case TOKEN_LINE_BEGIN:
		// ^
		x = make_tree_node(parser, OP_HEAD, NULL, NULL);
		x->u.c.c1 = parser->token_char1;
		get_token(parser);
		break;

	case TOKEN_LINE_END:
		// $
		x = make_tree_node(parser, OP_TAIL, NULL, NULL);
		x->u.c.c1 = parser->token_char1;
		get_token(parser);
		break;

	case TOKEN_LIST_CLOSE:
		// [...]
		x = make_tree_node(parser, OP_CHARCLASS, NULL, NULL);
		x->u.chr_class = alloc_copy(parser->user_char_set);
		get_token(parser);
		break;

	case TOKEN_GROUP_OPEN:
		// (...)
		parser->rfcnt++;
		o = make_tree_node(parser, OP_OPEN, NULL, NULL);
		o->u.i = parser->rfcnt;
		c = make_tree_node(parser, OP_CLOSE, NULL, NULL);
		c->u.i = parser->rfcnt;

		get_token(parser);
		x = make_tree_node(parser, OP_CONCAT, o,
			make_tree_node(parser, OP_CONCAT, regexp(parser), c));
		get_token(parser);
		break;
	}
	return x;
}


/******************************************************************************

	factor

	X*

******************************************************************************/

static TREE *factor(REGE_PARSER *parser)
{
	TREE *p, *r, *x;
	int i;

	x = primary(parser);
	switch(parser->current_token)
	{
	case TOKEN_INTERVAL_ZERO_MORE:
		// X*
		x = make_tree_node(parser, OP_CLOSURE, x, NULL);
		get_token(parser);
		break;

	case TOKEN_INTERVAL_ZERO_MORE_NG:
		// X*?
		x = make_tree_node(parser, OP_CLOSURE_NG, x, NULL);
		get_token(parser);
		break;

	case TOKEN_INTERVAL_ONE_MORE:
		// X+
		p = copy_tree(parser, x);
		x = make_tree_node(parser, OP_CONCAT, x,
			make_tree_node(parser, OP_CLOSURE, p, NULL));
		get_token(parser);
		break;

	case TOKEN_INTERVAL_ONE_MORE_NG:
		// X+?
		p = copy_tree(parser, x);
		x = make_tree_node(parser, OP_CONCAT, x,
			make_tree_node(parser, OP_CLOSURE_NG, p, NULL));
		get_token(parser);
		break;

	case TOKEN_INTERVAL_ZERO_ONE:
		// X?
		x = make_tree_node(parser, OP_UNION,
			make_tree_node(parser, OP_EMPTY, NULL, NULL), x);
		get_token(parser);
		break;

	case TOKEN_INTERVAL_ZERO_ONE_NG:
		// X??
		x = make_tree_node(parser, OP_UNION, x,
			make_tree_node(parser, OP_EMPTY, NULL, NULL));
		get_token(parser);
		break;

	case TOKEN_INTERVAL_CLOSE:
		// X{n,m}
		if(parser->intervals_close == 0){
			r = copy_tree(parser, x);
			p = make_tree_node(parser, OP_CLOSURE, r, NULL);
		}else{
			p = make_tree_node(parser, OP_EMPTY, NULL, NULL);
		}
		for(i = parser->intervals_close; i > parser->intervals_open; i--){
			r = copy_tree(parser, x);
			p = make_tree_node(parser, OP_CONCAT, p,
				make_tree_node(parser, OP_UNION,
				make_tree_node(parser, OP_EMPTY, NULL, NULL), r));
		}
		for(i = parser->intervals_open; i > 0; i--){
			r = copy_tree(parser, x);
			p = make_tree_node(parser, OP_CONCAT, r, p);
		}
		if(parser->intervals_open == 0){
			p = make_tree_node(parser, OP_UNION,
				make_tree_node(parser, OP_EMPTY, NULL, NULL), p);
		}
		free_tree(x);
		x = p;
		get_token(parser);
		break;

	case TOKEN_INTERVAL_CLOSE_NG:
		// X{n,m}?
		if(parser->intervals_close == 0){
			r = copy_tree(parser, x);
			p = make_tree_node(parser, OP_CLOSURE_NG, r, NULL);
		}else{
			p = make_tree_node(parser, OP_EMPTY, NULL, NULL);
		}
		for(i = parser->intervals_close; i > parser->intervals_open; i--){
			r = copy_tree(parser, x);
			p = make_tree_node(parser, OP_CONCAT, p,
				make_tree_node(parser, OP_UNION, r,
				make_tree_node(parser, OP_EMPTY, NULL, NULL)));
		}
		for(i = parser->intervals_open; i > 0; i--){
			r = copy_tree(parser, x);
			p = make_tree_node(parser, OP_CONCAT, r, p);
		}
		if(parser->intervals_open == 0){
			p = make_tree_node(parser, OP_UNION, p,
				make_tree_node(parser, OP_EMPTY, NULL, NULL));
		}
		free_tree(x);
		x = p;
		get_token(parser);
		break;
	}
	return x;
}


/******************************************************************************

	term

	XY

******************************************************************************/

static TREE *term(REGE_PARSER *parser)
{
	TREE *x;

	if(parser->current_token == TOKEN_ALTERNATION
		|| parser->current_token == TOKEN_GROUP_CLOSE
		|| parser->current_token == TOKEN_END){
		x = make_tree_node(parser, OP_EMPTY, NULL, NULL);

	}else{
		x =  factor(parser);
		while(parser->current_token != TOKEN_ALTERNATION
			&& parser->current_token != TOKEN_GROUP_CLOSE
			&& parser->current_token != TOKEN_END){
			x = make_tree_node(parser, OP_CONCAT, x, factor(parser));
		}
	}
	return x;
}


/******************************************************************************

	regexp

	X|Y

******************************************************************************/

static TREE *regexp(REGE_PARSER *parser)
{
	TREE *x;

	x = term(parser);
	while(parser->current_token == TOKEN_ALTERNATION){
		get_token(parser);
		x = make_tree_node(parser, OP_UNION, term(parser), x);
	}
	return x;
}


/******************************************************************************

	parse

	構文木の生成

******************************************************************************/

static REGE_PARSER *parse(TCHAR *pattern)
{
	REGE_PARSER *parser;

	if((parser = mem_alloc(sizeof(REGE_PARSER))) == NULL){
		return NULL;
	}
	ZeroMemory(parser, sizeof(REGE_PARSER));

	parser->p = pattern;
	get_token(parser);

	parser->tree = regexp(parser);
	return parser;
}


/******************************************************************************

	free_tree

	構文木の解放

******************************************************************************/

static void free_tree(TREE *p)
{
	if(p == NULL){
		return;
	}
	switch (p->op)
	{
	case OP_CONCAT:
	case OP_UNION:
		free_tree(p->u.x.right);
		p->u.x.right = NULL;
	case OP_CLOSURE:
	case OP_CLOSURE_NG:
		free_tree(p->u.x.left);
		p->u.x.left = NULL;
		break;

	case OP_CHARCLASS:
		mem_free(&p->u.chr_class);
		break;

	default:
		break;
	}
	mem_free(&p);
}


/******************************************************************************

	add_transition

	NFA に状態遷移を追加する

******************************************************************************/

static NFA_LIST *add_transition(REGE_NFA *nfa, int from, int to, int eps, OP op)
{
	NFA_LIST *p;

	if((p = mem_alloc(sizeof(NFA_LIST))) == NULL){
		return NULL;
	}
	ZeroMemory(p, sizeof(NFA_LIST));

	p->eps = eps;
	p->op = op;
	p->to = to;
	p->next = *(nfa->states + from);
	*(nfa->states + from) = p;
	return p;
}


/******************************************************************************

	generate_nfa

	構文木 tree に対する NFA を生成

******************************************************************************/

static void generate_nfa(REGE_NFA *nfa, TREE *tree, int entry, int way_out)
{
	NFA_LIST *p;
	int a1, a2;

	if(tree == NULL){
		return;
	}
	switch(tree->op)
	{
	case OP_CHAR:
		p = add_transition(nfa, entry, way_out, 0, tree->op);
		if(p != NULL){
			p->u.c.c1 = tree->u.c.c1;
			p->u.c.c2 = tree->u.c.c2;
		}
		break;

	case OP_HEAD:
		if(entry == nfa->entry) {
			nfa->head_flag = TRUE;
			add_transition(nfa, entry, way_out, EPSILON_TRANS, tree->op);
		}
		break;

	case OP_TAIL:
		if(way_out == nfa->exit){
			nfa->tail_flag = TRUE;
			add_transition(nfa, entry, way_out, EPSILON_TRANS, tree->op);
		}
		break;

	case OP_CHARCLASS:
		p = add_transition(nfa, entry, way_out, 0, tree->op);
		if(p != NULL){
			p->u.chr_class = alloc_copy(tree->u.chr_class);
		}
		break;

	case OP_OPEN:
	case OP_CLOSE:
		p = add_transition(nfa, entry, way_out, EPSILON_TRANS, tree->op);
		if(p != NULL){
			p->u.i = tree->u.i;
		}
		break;

	case OP_EMPTY:
		add_transition(nfa, entry, way_out, EPSILON_TRANS, tree->op);
		break;

	case OP_UNION:
		generate_nfa(nfa, tree->u.x.left, entry, way_out);
		generate_nfa(nfa, tree->u.x.right, entry, way_out);
		break;

	case OP_CLOSURE:
		a1 = nfa->nstate++;
		a2 = nfa->nstate++;
		add_transition(nfa, entry, a1, EPSILON_TRANS, tree->op);
		add_transition(nfa, a2, a1, EPSILON_TRANS, tree->op);
		add_transition(nfa, a1, way_out, EPSILON_TRANS, tree->op);
		generate_nfa(nfa, tree->u.x.left, a1, a2);
		break;

	case OP_CLOSURE_NG:
		a1 = nfa->nstate++;
		a2 = nfa->nstate++;
		add_transition(nfa, entry, a1, EPSILON_TRANS, tree->op);
		generate_nfa(nfa, tree->u.x.left, a1, a2);
		add_transition(nfa, a2, a1, EPSILON_TRANS, tree->op);
		add_transition(nfa, a1, way_out, EPSILON_TRANS, tree->op);
		break;

	case OP_CONCAT:
		a1 = nfa->nstate++;
		generate_nfa(nfa, tree->u.x.left, entry, a1);
		generate_nfa(nfa, tree->u.x.right, a1, way_out);
		break;

	default:
		break;
	}
}


/******************************************************************************

	generate_nfa_count

	生成する状態リストの数の取得

******************************************************************************/

static int generate_nfa_count(TREE *tree)
{
	int ret = 0;

	if(tree == NULL){
		return 0;
	}
	switch (tree->op)
	{
	case OP_UNION:
		ret = 0;
		ret += generate_nfa_count(tree->u.x.left);
		ret += generate_nfa_count(tree->u.x.right);
		break;

	case OP_CLOSURE:
		ret = 2;
		ret += generate_nfa_count(tree->u.x.left);
		break;

	case OP_CLOSURE_NG:
		ret = 2;
		ret += generate_nfa_count(tree->u.x.left);
		break;

	case OP_CONCAT:
		ret = 1;
		ret += generate_nfa_count(tree->u.x.left);
		ret += generate_nfa_count(tree->u.x.right);
		break;

	default:
		break;
	}
	return ret;
}


/******************************************************************************

	build_nfa

	NFA の生成

******************************************************************************/

static REGE_NFA *build_nfa(REGE_PARSER *parser)
{
	REGE_NFA *nfa;
	int i;

	//NFA 情報の確保
	if((nfa = mem_alloc(sizeof(REGE_NFA))) == NULL){
		return NULL;
	}
	ZeroMemory(nfa, sizeof(REGE_NFA));

	//状態リストの確保
	i = generate_nfa_count(parser->tree);
	nfa->states = mem_alloc(sizeof(NFA_LIST *) * (i + 2));
	ZeroMemory(nfa->states, sizeof(NFA_LIST *) * (i + 2));

	//NFA の生成
	nfa->nstate = 0;
	nfa->entry = nfa->nstate++;
	nfa->exit = nfa->nstate++;
	nfa->head_flag = FALSE;
	nfa->tail_flag = FALSE;
	nfa->rfcnt = parser->rfcnt + 1;
	generate_nfa(nfa, parser->tree, nfa->entry, nfa->exit);
	return nfa;
}


/******************************************************************************

	free_nfa

	NFA の解放

******************************************************************************/

void free_nfa(REGE_NFA *nfa)
{
	int i;
	NFA_LIST *p, *q;

	if(nfa == NULL) return;

	for(i = 0; i < nfa->nstate; i++){
		if(*(nfa->states + i) != NULL){
			p = *(nfa->states + i);
			while(p != NULL){
				q = p;
				p = p->next;
				if(q->op == OP_CHARCLASS){
					mem_free(&q->u.chr_class);
				}
				mem_free(&q);
			}
		}
	}
	mem_free((void **)&nfa->states);
	mem_free(&nfa);
}


/******************************************************************************

	charclass_match

	キャラクタクラスのマッチング

******************************************************************************/

static BOOL charclass_match(TCHAR *p, TCHAR *c)
{
	TCHAR tmp[CHAR_SPACE_SIZE];
	BOOL ret = FALSE;
	BOOL neg_flag = FALSE;

	if(*p == **(token_chars + TOKEN_LIST_NOT)){
		neg_flag = TRUE;
		p++;
	}

	while(*p != _T('\0') && ret == FALSE){
		//if(IsDBCSLeadByte((BYTE)*p) == TRUE){
		//	if(*(p + 2) == **(token_chars + TOKEN_LIST_RANGE) && IsDBCSLeadByte((BYTE)*(p + 3)) == TRUE){
		//		ret = ((unsigned)*c >= (unsigned)*p &&
		//			(unsigned)*(c + 1) >= (unsigned)*(p + 1) &&
		//			(unsigned)*c <= (unsigned)*(p + 3) &&
		//			(unsigned)*(c + 1) <= (unsigned)*(p + 4)) ? TRUE : FALSE;
		//		p += 4;
		//	}else{
		//		ret = (*p == *c && *(p + 1) == *(c + 1)) ? TRUE : FALSE;
		//		p++;
		//	}
		//	p++;
		//	continue;
		//}
		if(*(p + 1) == **(token_chars + TOKEN_LIST_RANGE) && *(p + 2) != _T('\0')){
			ret = ((unsigned)*c >= (unsigned)*p &&
				(unsigned)*c <= (unsigned)*(p + 2)) ? TRUE : FALSE;
			p += 2;
		}else{
			if(*p == **(token_chars + TOKEN_LIST_ESCAPE) && *(p + 1) != _T('\0')){
				p = get_esc_string(p, tmp, TRUE);
				ret = charclass_match(tmp, c);
				continue;
			}else{
				ret = (*p == *c) ? TRUE : FALSE;
			}
		}
		p++;
	}
	return ((neg_flag == FALSE) ? ret : !ret);
}


/******************************************************************************

	rege_match

	NFA で文字列のマッチング

******************************************************************************/

static TCHAR *rege_match(REGE_MATCH *rm, int i, TCHAR *c,int depth)
{
	NFA_LIST *p;
	TCHAR *ret;

	// 再帰が限界を超えたらエラー
	if ( depth > 12000 ){				// スタック256KBでの限界値
		return (TCHAR *)-1;
	}

	if(rm->nfa->exit == i){
		if(rm->nfa->tail_flag == TRUE &&
			*c != _T('\0') && *c != _T('\r') && *c != _T('\n')){
			return NULL;
		}
		//受理
		return c;
	}
	for(p = *(rm->nfa->states + i); p != NULL; p = p->next){
		if(p->eps == EPSILON_TRANS){
			//ε遷移
			switch(p->op)
			{
			case OP_OPEN:
				if(rm->rf == NULL){
					break;
				}
				(rm->rf + p->u.i)->st = c;
				break;

			case OP_CLOSE:
				if(rm->rf == NULL){
					break;
				}
				(rm->rf + p->u.i)->en = c;
				break;
			}
			if((ret = rege_match(rm, p->to, c,depth+1)) != NULL){
				return ret;
			}
			continue;
		}

		// 文字の比較
		if(*c == _T('\0')){
			continue;
		}
		if(rm->nfa->tail_flag == TRUE && (*c == _T('\r') || *c == _T('\n'))){
			continue;
		}
		//if(IsDBCSLeadByte((BYTE)*c) == TRUE){
		//	//2バイトコード
		//	if(p->op == OP_CHARCLASS){
		//		//キャラクタクラス
		//		if(charclass_match(p->u.chr_class, c) == TRUE &&
		//			(ret = rege_match(rm, p->to, c + 2)) != NULL){
		//			return ret;
		//		}
		//	}else if(p->u.c.c1 == *c && p->u.c.c2 == *(c + 1) &&
		//		(ret = rege_match(rm, p->to, c + 2)) != NULL){
		//		return ret;
		//	}
		//	continue;
		//}
		if(p->op == OP_CHARCLASS){
			//キャラクタクラス
			if(charclass_match(p->u.chr_class, c) == TRUE &&
				(ret = rege_match(rm, p->to, c + 1,depth+1)) != NULL){
				return ret;
			}
		}else if(rm->icase == TRUE){
			//ignore case
			if(ToLower(p->u.c.c1) == ToLower(*c) &&
				(ret = rege_match(rm, p->to, c + 1,depth+1)) != NULL){
				return ret;
			}
		}else if(p->u.c.c1 == *c &&
			(ret = rege_match(rm, p->to, c + 1,depth+1)) != NULL){
			return ret;
		}
	}
	return NULL;
}


/******************************************************************************

	rege_compile

	正規表現をコンパイル

******************************************************************************/

REGE_NFA *rege_compile(const TCHAR *ptn)
{
	REGE_PARSER *parser;
	REGE_NFA *nfa;

	//構文木の生成
	parser = parse((TCHAR*)ptn);

	//NFAの生成
	nfa = build_nfa(parser);
	free_tree(parser->tree);
	mem_free(&parser);
	return nfa;
}


/******************************************************************************

	reg_match

	正規表現のマッチング

******************************************************************************/

int reg_match(REGE_NFA *nfa,const TCHAR *str, REGE_REFER **rf, BOOL icase)
{
	REGE_MATCH rm;
	TCHAR *to;
	TCHAR *p;

	to = NULL;

	if(nfa == NULL){
		return 0;
	}

	ZeroMemory(&rm, sizeof(REGE_MATCH));
	rm.nfa = nfa;
	rm.icase = icase;
	rm.rf = mem_alloc(sizeof(REGE_REFER) * nfa->rfcnt);

	p = (TCHAR*)str;
	while(*p != _T('\0')){
		ZeroMemory(rm.rf, sizeof(REGE_REFER) * nfa->rfcnt);
		to = rege_match(&rm, 0, p,0);
		if(to == (TCHAR *)-1){
			to = NULL;
		}
		if(to != NULL){
			break;
		}
		if(nfa->head_flag == TRUE){
			for(; *p != _T('\0') && *p != _T('\r') && *p != _T('\n'); p++);
			for(; *p == _T('\r') || *p == _T('\n'); p++);
		}else{
			p++;
		}
	}

	if(to == NULL){
		mem_free(&rm.rf);
		return 0;
	}
	rm.rf->st = p;
	rm.rf->en = to;
	*rf = rm.rf;
	return nfa->rfcnt;
}

int reg_match_intext(REGE_NFA *nfa,const TCHAR *str, REGE_REFER **rf, BOOL icase , unsigned int st , BOOL forward)
{
	REGE_MATCH rm;
	TCHAR *to;
	TCHAR *p;

	to = NULL;

	if(nfa == NULL){
		return 0;
	}

	ZeroMemory(&rm, sizeof(REGE_MATCH));
	rm.nfa = nfa;
	rm.icase = icase;
	rm.rf = mem_alloc(sizeof(REGE_REFER) * nfa->rfcnt);

	p = (TCHAR*)str+st;
	if ( forward ){
		// 後方検索
		while(*p != _T('\0')){
			ZeroMemory(rm.rf, sizeof(REGE_REFER) * nfa->rfcnt);
			if(nfa->head_flag == TRUE){
				// 行頭フラグが立っていれば
				if ( p==str ){
					// テキスト先頭は行頭であるので何もしない
				}else{
					if ( *(p-1) != _T('\r') && *(p-1) != _T('\n') ){
						// 直前の文字が改行でなければ、次の改行までとばす
						for(; *p != _T('\0') && *p != _T('\r') && *p != _T('\n'); p++);
						for(; *p == _T('\r') || *p == _T('\n'); p++);
					}
				}
			}
			to = rege_match(&rm, 0, p,0);
			if(to == (TCHAR *)-1){
				to = NULL;
			}
			if(to != NULL){
				break;
			}
			p++;
		}
	}else{
		// 前方検索
		while(p > str){
			ZeroMemory(rm.rf, sizeof(REGE_REFER) * nfa->rfcnt);

			if(nfa->head_flag == TRUE){
				// 行頭フラグが立っていれば改行の直後まで戻す
				for(; p > str && *(p-1) != _T('\r') && *(p-1) != _T('\n'); p--);
			}else{
				if ( p>str ){
					p--;
				}
			}
			to = rege_match(&rm, 0, p,0);
			if(to == (TCHAR *)-1){
				to = NULL;
			}
			if(to != NULL){
				break;
			}
			if(nfa->head_flag == TRUE){
				// 行頭フラグが立っていれば改行の直前まで戻す
				for(; p >= str && *p == _T('\r') || *p == _T('\n'); p--);
			}
		}
	}

	if(to == NULL){
		mem_free(&rm.rf);
		return 0;
	}
	rm.rf->st = p;
	rm.rf->en = to;
	*rf = rm.rf;
	return nfa->rfcnt;
}


/******************************************************************************

	free_refer

	rege_refer の解放

******************************************************************************/

void free_refer(REGE_REFER *rf)
{
	if(rf == NULL) return;
	mem_free(&rf);
}
/* End of source */
