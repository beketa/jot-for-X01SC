//
// jot - Text Editor for Windows Mobile
//   Copyright (C) 2007-2008, Aquamarine Networks.  <http://pandora.sblo.jp/>
//
//    This program is EveryOne's ware; you can redistribute it and/or modify it 
//    under the terms of the NYSL version 0.9982 or later. <http://www.kmonos.net/nysl/>
//

// JumpLineNo.cpp : 実装ファイル
//

#include "stdafx.h"
#include "jot.h"
#include "JumpLineNo.h"


// CJumpLineNo

IMPLEMENT_DYNAMIC(CJumpLineNo, CBottomBar)

CJumpLineNo::CJumpLineNo()
{

}

CJumpLineNo::~CJumpLineNo()
{
}


BEGIN_MESSAGE_MAP(CJumpLineNo, CBottomBar)
	ON_COMMAND(ID_JUMP, &CJumpLineNo::OnJump)
	ON_UPDATE_COMMAND_UI(ID_JUMP, &CJumpLineNo::OnUpdateJump)
END_MESSAGE_MAP()



// CJumpLineNo メッセージ ハンドラ



void CJumpLineNo::OnJump()
{
	CString	s;
	m_edit.GetWindowTextW( s );
	m_parentBottomBar->NotifyBottomBar( ID_JUMP ,&s );
}



void CJumpLineNo::OnUpdateJump(CCmdUI *pCmdUI)
{
	CString	s;
	m_edit.GetWindowTextW( s );

	pCmdUI->Enable( ( s.GetLength() > 0 )?TRUE:FALSE );
}
