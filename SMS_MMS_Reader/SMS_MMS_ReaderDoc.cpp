
/*
 *	SMS_MMS_ReaderDoc.cpp : implementation of the CSMS_MMS_ReaderDoc class
 *
 *	This is the main document class, which contains the list of SMS/MMS conversations (threads).
 */

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "SMS_MMS_Reader.h"
#endif

#include "SMS_MMS_ReaderDoc.h"
#include "SMS_MMS_ReaderView.h"
#include "resource.h"
#include "StatusDlg.h"

#include <propkey.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CSMS_MMS_ReaderDoc

IMPLEMENT_DYNCREATE(CSMS_MMS_ReaderDoc, CDocument)

BEGIN_MESSAGE_MAP(CSMS_MMS_ReaderDoc, CDocument)
END_MESSAGE_MAP()


// CSMS_MMS_ReaderDoc construction/destruction

CSMS_MMS_ReaderDoc::CSMS_MMS_ReaderDoc()
{
	// TODO: add one-time construction code here

}

CSMS_MMS_ReaderDoc::~CSMS_MMS_ReaderDoc()
{
}

BOOL CSMS_MMS_ReaderDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}




// CSMS_MMS_ReaderDoc serialization

void CSMS_MMS_ReaderDoc::Serialize(CArchive& ar)
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

#ifdef SHARED_HANDLERS

// Support for thumbnails
void CSMS_MMS_ReaderDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// Modify this code to draw the document's data
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	CString strText = _T("TODO: implement thumbnail drawing here");
	LOGFONT lf;

	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT)GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	lf.lfHeight = 36;

	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	CFont* pOldFont = dc.SelectObject(&fontDraw);
	dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
	dc.SelectObject(pOldFont);
}

// Support for Search Handlers
void CSMS_MMS_ReaderDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// Set search contents from document's data. 
	// The content parts should be separated by ";"

	// For example:  strSearchContent = _T("point;rectangle;circle;ole object;");
	SetSearchContent(strSearchContent);
}

void CSMS_MMS_ReaderDoc::SetSearchContent(const CString& value)
{
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CMFCFilterChunkValueImpl *pChunk = NULL;
		ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
		if (pChunk != NULL)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}

#endif // SHARED_HANDLERS

// CSMS_MMS_ReaderDoc diagnostics

#ifdef _DEBUG
void CSMS_MMS_ReaderDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CSMS_MMS_ReaderDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CSMS_MMS_ReaderDoc commands

bool ReadFileCallback(void *p, double dFractionComplete)
{
	CStatusDlg *pDlg = (CStatusDlg *) p;
	MSG msg;

	pDlg->SetFractionComplete(dFractionComplete);

	// Simple message loop to allow the Cancel button to work
	while (::PeekMessage(&msg, pDlg->GetSafeHwnd(), 0, 0, PM_REMOVE))
	{
		if (!::IsDialogMessage(pDlg->GetSafeHwnd(), &msg))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
	}

	if (pDlg->m_bCancelClicked)
		return false;

	return true;
}

// Called by the framework, and also called by our OnDropFiles() handler
BOOL CSMS_MMS_ReaderDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	return DoReadFile(lpszPathName);
}

BOOL CSMS_MMS_ReaderDoc::DoReadFile(LPCTSTR lpszPathName)
{
	TCHAR szError[1024];

	CStatusDlg dlg;
	CString csText;

	/*
	 *	Unfortunately we create a new status dialog for each new file we read... I didn't see an easy way around this for
	 *	supporting both framework-called OnOpenDocument() calls and our own drag-and-drop calls.  Maybe open a new, blank
	 *	document instead and call this for all the files instead of just the 2nd+ files?
	 */

	if (!dlg.Create((UINT)IDD_STATUS, AfxGetMainWnd()))
		AfxThrowResourceException();

	csText.Format(_T("Reading File %s ..."), lpszPathName);
	dlg.m_ctlStatusText.SetWindowTextW(csText);

	dlg.ShowWindow(SW_SHOW);
	dlg.CenterWindow();
	dlg.UpdateWindow();

	if (!m_Threads.ReadFile(lpszPathName, szError, _countof(szError), ReadFileCallback, (void *) &dlg))
	{
		AfxMessageBox(szError, MB_OK | MB_ICONSTOP);
		return FALSE;
	}

	return TRUE;
}

CSMS_MMS_ReaderView * CSMS_MMS_ReaderDoc::GetTheView()
{
	POSITION pos = GetFirstViewPosition();

	if (pos != NULL)
	{
		CView *pView = GetNextView(pos);

		if (pView != NULL && pView->IsKindOf(RUNTIME_CLASS(CSMS_MMS_ReaderView)))
			return (CSMS_MMS_ReaderView *) pView;
	}

#ifdef _DEBUG
	ASSERT(FALSE);
#endif

	return NULL;
}
