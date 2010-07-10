#pragma once

class CTemplate
{
private:
	class Temp	{
	public:
		CString	name;
		CString	temp;
		Temp(CString &t){
			int f = t.Find( _T("=") );
			if ( f!= -1 ){
				name = t.Left( f ).Trim();
				temp = t.Mid( f+1 );
			}
		}
	};

	CArray<Temp*>	m_list;
	int	m_max;
	CString	m_filename ;
public:
	CTemplate(int max , const TCHAR *filename);
	virtual	~CTemplate(void);

	int	GetCount(){
		return m_list.GetCount();
	}

	const TCHAR *GetName(int i){
		if ( i< m_list.GetCount() ){
			return m_list.GetAt(i)->name;
		}
		return _T("");
	}

	CString	ProcTemplate( int idx , CString src , int &caret );

};


