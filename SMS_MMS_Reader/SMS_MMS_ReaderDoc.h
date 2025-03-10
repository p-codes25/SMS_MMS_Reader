
// SMS_MMS_ReaderDoc.h : interface of the CSMS_MMS_ReaderDoc class
//

#pragma once

#include "SMS_MMS_ReaderFile.h"

class CSMS_MMS_ReaderDoc : public CDocument
{
protected: // create from serialization only
	CSMS_MMS_ReaderDoc();
	DECLARE_DYNCREATE(CSMS_MMS_ReaderDoc)

// Attributes
public:

// Operations
public:
	CSMSThreadList * GetThreadList() { return &m_Threads; }

	class CSMS_MMS_ReaderView * GetTheView();

// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
#ifdef SHARED_HANDLERS
	virtual void InitializeSearchContent();
	virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
#endif // SHARED_HANDLERS

// Implementation
public:
	virtual ~CSMS_MMS_ReaderDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

	// Here is the list of all the SMS/MMS conversations (threads) loaded from the file for this CDocument
	CSMSThreadList m_Threads;

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()

#ifdef SHARED_HANDLERS
	// Helper function that sets search content for a Search Handler
	void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS
public:
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);

	BOOL DoReadFile(LPCTSTR lpszPathName);

};
