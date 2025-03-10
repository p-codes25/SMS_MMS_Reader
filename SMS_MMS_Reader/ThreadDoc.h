#pragma once

#include <vector>
#include <string>

using namespace std;

/*
 *	CThreadDoc document - this document represents a single conversation with all its messages.
 *	The SMS/MMS conversation is output as an HTML file plus graphics/media file attachements;
 *	all those files are described in the m_vTempFiles vector.
 */

class CThreadDoc : public CDocument
{
	DECLARE_DYNCREATE(CThreadDoc)

public:
	CThreadDoc();
	virtual ~CThreadDoc();

	// CString m_strThreadFileName;

	vector<wstring> m_vTempFiles;

#ifndef _WIN32_WCE
	virtual void Serialize(CArchive& ar);   // overridden for document i/o
#endif
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	virtual BOOL OnNewDocument();

	DECLARE_MESSAGE_MAP()
};
