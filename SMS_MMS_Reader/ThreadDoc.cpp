// ThreadDoc.cpp : implementation file
//

#include "stdafx.h"
#include "SMS_MMS_Reader.h"
#include "ThreadDoc.h"


// CThreadDoc

IMPLEMENT_DYNCREATE(CThreadDoc, CDocument)

CThreadDoc::CThreadDoc()
{
}

BOOL CThreadDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;
	return TRUE;
}

CThreadDoc::~CThreadDoc()
{
	// If we received a document filename, remove the file as we destruct our document
	//if (!m_strThreadFileName.IsEmpty())
	//	_tunlink(m_strThreadFileName);

	vector<wstring>::iterator i;

	// If we received a list of temp attachment files, remove those files too
	for (i = m_vTempFiles.begin(); i < m_vTempFiles.end(); ++i)
	{
		_tunlink(i->c_str());
	}
}


BEGIN_MESSAGE_MAP(CThreadDoc, CDocument)
END_MESSAGE_MAP()


// CThreadDoc diagnostics

#ifdef _DEBUG
void CThreadDoc::AssertValid() const
{
	CDocument::AssertValid();
}

#ifndef _WIN32_WCE
void CThreadDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif
#endif //_DEBUG

#ifndef _WIN32_WCE
// CThreadDoc serialization

void CThreadDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}
#endif


// CThreadDoc commands
