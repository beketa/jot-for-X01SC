//
// jot - Text Editor for Windows Mobile
//   Copyright (C) 2007-2008, Aquamarine Networks.  <http://pandora.sblo.jp/>
//
//    This program is EveryOne's ware; you can redistribute it and/or modify it 
//    under the terms of the NYSL version 0.9982 or later. <http://www.kmonos.net/nysl/>
//
#include "StdAfx.h"
#include "SharedMru.h"
#include "file.h"

CSharedMru::CSharedMru(const TCHAR* mrufile , const char *sig )
{
	m_pMapPtr = NULL;

	memcpy( m_sig , sig ,4 );

	CString	semaname;
	semaname.Format( _T("//jot/%s/mutex") ,mrufile );

	CString	mapname;
	mapname.Format( _T("//jot/%s/share") ,mrufile );

	// ミューテックス開く
	m_hSema = CreateMutex( NULL , FALSE , semaname );

	// 実行ファイルがあるディレクトリに ini がある。
	TCHAR temp[MAX_PATH*2]={0};
	GetModuleFileName(NULL, temp, MAX_PATH*2-1);
	CString s = temp;

	size_t pos = s.ReverseFind(_T('\\'));
	m_filename = s.Left(pos+1) + mrufile;

	// 共有メモリ開く
	m_hMap = CreateFileMapping((HANDLE)INVALID_HANDLE_VALUE,NULL,PAGE_READWRITE|SEC_COMMIT,0,0x10000,mapname );
	if ( m_hMap != NULL ){
		m_pMapPtr = (BYTE*)MapViewOfFile( m_hMap , FILE_MAP_WRITE , 0 , 0 , 0 );
		Load();
	}
}

CSharedMru::~CSharedMru(void)
{
	Save();
	if ( m_pMapPtr != NULL ){
		Save();
		UnmapViewOfFile( m_pMapPtr );
	}
	if ( m_hMap != NULL ){
		CloseHandle( m_hMap );
	}
	CloseHandle( m_hSema );
}

// ファイルからロード
void	CSharedMru::Load()
{
	if ( WaitForSingleObject( m_hSema ,INFINITE )==WAIT_OBJECT_0 ){
		if ( m_pMapPtr != NULL ){
			// すでにロードされていれば処理しない
			if ( memcmp( m_sig , m_pMapPtr , 4 ) != 0 ){
				FileR	f;
				bool ret = f.open( m_filename );

				if ( ret ){
					// 読込正常
					memcpy( m_pMapPtr , f.base() , f.size() );

					ret = false;
					// シグネチャチェック
					if ( memcmp( m_sig , m_pMapPtr , 4 ) == 0 ){
						// 長さチェック
						int	len = *(int*)(m_pMapPtr+4);			// 長さ取得
						if ( len == f.size() ){
							ret = true;
						}
					}
				}
				Update();
			}
		}

		ReleaseMutex( m_hSema );
	}

}
// ファイルにセーブ
void	CSharedMru::Save()
{
	if ( WaitForSingleObject( m_hSema ,INFINITE )==WAIT_OBJECT_0 ){

		if ( m_pMapPtr != NULL ){
			// すでにロードされていれば
			if ( memcmp( m_sig , m_pMapPtr , 4 ) == 0 ){
				FileW	f;
				f.open( m_filename );

				int	len = *(int*)(m_pMapPtr+4);			// 長さ取得

				f.write( m_pMapPtr , len );
			}
		}
		ReleaseMutex( m_hSema );
	}
}
// 共有メモリから読込
void	CSharedMru::Update()
{
	if ( WaitForSingleObject( m_hSema ,INFINITE )==WAIT_OBJECT_0 ){

		if ( m_pMapPtr != NULL ){
			// すでにロードされていれば
			if ( memcmp( m_sig , m_pMapPtr , 4 ) == 0 ){
				BYTE	*p = (m_pMapPtr +8);
				UpdateData( p );
			}else{
				BYTE	buf[2] = {0x00,0x00};
				UpdateData( buf );
			}
		}
		ReleaseMutex( m_hSema );
	}
}
// 共有メモリに書込
void	CSharedMru::Commit()
{
	if ( WaitForSingleObject( m_hSema ,INFINITE )==WAIT_OBJECT_0 ){
		if ( m_pMapPtr != NULL ){
			memcpy( m_pMapPtr , m_sig , 4 );		// シグネチャ

			BYTE	*p = (m_pMapPtr +8);

			// データ
			p = CommitData( p );

			// 長さ
			*(int*)(m_pMapPtr + 4) = ((BYTE*)p) - m_pMapPtr;
		}

		ReleaseMutex( m_hSema );
	}
}
