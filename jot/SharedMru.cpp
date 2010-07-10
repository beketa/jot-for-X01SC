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

	// �~���[�e�b�N�X�J��
	m_hSema = CreateMutex( NULL , FALSE , semaname );

	// ���s�t�@�C��������f�B���N�g���� ini ������B
	TCHAR temp[MAX_PATH*2]={0};
	GetModuleFileName(NULL, temp, MAX_PATH*2-1);
	CString s = temp;

	size_t pos = s.ReverseFind(_T('\\'));
	m_filename = s.Left(pos+1) + mrufile;

	// ���L�������J��
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

// �t�@�C�����烍�[�h
void	CSharedMru::Load()
{
	if ( WaitForSingleObject( m_hSema ,INFINITE )==WAIT_OBJECT_0 ){
		if ( m_pMapPtr != NULL ){
			// ���łɃ��[�h����Ă���Ώ������Ȃ�
			if ( memcmp( m_sig , m_pMapPtr , 4 ) != 0 ){
				FileR	f;
				bool ret = f.open( m_filename );

				if ( ret ){
					// �Ǎ�����
					memcpy( m_pMapPtr , f.base() , f.size() );

					ret = false;
					// �V�O�l�`���`�F�b�N
					if ( memcmp( m_sig , m_pMapPtr , 4 ) == 0 ){
						// �����`�F�b�N
						int	len = *(int*)(m_pMapPtr+4);			// �����擾
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
// �t�@�C���ɃZ�[�u
void	CSharedMru::Save()
{
	if ( WaitForSingleObject( m_hSema ,INFINITE )==WAIT_OBJECT_0 ){

		if ( m_pMapPtr != NULL ){
			// ���łɃ��[�h����Ă����
			if ( memcmp( m_sig , m_pMapPtr , 4 ) == 0 ){
				FileW	f;
				f.open( m_filename );

				int	len = *(int*)(m_pMapPtr+4);			// �����擾

				f.write( m_pMapPtr , len );
			}
		}
		ReleaseMutex( m_hSema );
	}
}
// ���L����������Ǎ�
void	CSharedMru::Update()
{
	if ( WaitForSingleObject( m_hSema ,INFINITE )==WAIT_OBJECT_0 ){

		if ( m_pMapPtr != NULL ){
			// ���łɃ��[�h����Ă����
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
// ���L�������ɏ���
void	CSharedMru::Commit()
{
	if ( WaitForSingleObject( m_hSema ,INFINITE )==WAIT_OBJECT_0 ){
		if ( m_pMapPtr != NULL ){
			memcpy( m_pMapPtr , m_sig , 4 );		// �V�O�l�`��

			BYTE	*p = (m_pMapPtr +8);

			// �f�[�^
			p = CommitData( p );

			// ����
			*(int*)(m_pMapPtr + 4) = ((BYTE*)p) - m_pMapPtr;
		}

		ReleaseMutex( m_hSema );
	}
}
