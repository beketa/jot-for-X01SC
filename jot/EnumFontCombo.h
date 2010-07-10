//
// jot - Text Editor for Windows Mobile
//   Copyright (C) 2007-2008, Aquamarine Networks.  <http://pandora.sblo.jp/>
//
//    This program is EveryOne's ware; you can redistribute it and/or modify it 
//    under the terms of the NYSL version 0.9982 or later. <http://www.kmonos.net/nysl/>
//

#pragma once

//�w�b�_�t�@�C��
class CEnumFontCombo : public CComboBox
{
	DECLARE_DYNAMIC(CEnumFontCombo)
public:
    CEnumFontCombo();
    virtual ~CEnumFontCombo();

protected:
    void EnumFont(BYTE CharSet = ANSI_CHARSET, BOOL bDelPrevFonts = FALSE);
    //���X�g�Ƀt�H���g��񋓂���
    //��P�����F�ΏۂƂ���L�����N�^�[�Z�b�g���w��
    //��Q�����F�O��܂łɗ񋓂����t�H���g���폜���邩�ǂ����i�폜����ꍇ�� TRUE �A���Ȃ��ꍇ�� FALSE)���w��

    BOOL GetLogFont(LPLOGFONT lpLogFont);
    //�I������Ă���t�H���g�̃f�[�^�iLOGFONT �\���́j���擾����
    //��������� lpLogFont ���w�� LOGFONT �\���̂Ƀt�H���g�̃f�[�^���R�s�[����ATRUE��Ԃ�
    //���s����� FALSE ��Ԃ�

    //�R�[���o�b�N�֐�
    static int CALLBACK EnumFontFamExProc(ENUMLOGFONTEX *lpelfe, NEWTEXTMETRICEX *lpntme, int FontType, LPARAM lParam);

protected:
    //�񋓂��ꂽ�t�H���g�f�[�^�iLOGFONG �\���́j�ւ̃|�C���^�̔z��
    CArray<LOGFONT*, LOGFONT*> m_pLogFontArray;

    afx_msg void OnDestroy();
	afx_msg void OnCbnSelchange();
    DECLARE_MESSAGE_MAP()

public:
	void	Init( const TCHAR *nonefontname , const TCHAR *initfont )
	{
		// �R���{�{�b�N�X�Ƀt�H���g����ݒ�
		AddString( nonefontname );		// �t�H���g���ݒ�̍���
		SetCurSel( 0 );
		EnumFont(DEFAULT_CHARSET);

		// �R���{�{�b�N�X�̌��ݐݒ肳��Ă���t�H���g��I��
		if ( initfont != NULL ){
			int len = GetCount();
			for(int i=1;i<len;i++ ){
				CString item;
				GetLBText(i,item);
				if ( item == initfont ){
					SetCurSel( i );
				}
			}
		}
	}

};

