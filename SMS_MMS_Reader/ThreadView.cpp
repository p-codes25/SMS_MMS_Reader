/*
 *	ThreadView.cpp
 *
 *	Based on MFC's CHtmlView, the CThreadView class implements the SMS/MMS conversation ("thread") view.  The HTML representation of the
 *	SMS/MMS message conversation is generated in CSMS_MMS_ReaderView::OnDblClkList(), which produces an HTML file with embedded
 *	attachments for things like MMS images, videos, audio clips, etc.
 */

#include "stdafx.h"
#include "SMS_MMS_Reader.h"
#include "ThreadView.h"
#include "ThreadDoc.h"
#include "FindTextDlg.h"


// CThreadView

IMPLEMENT_DYNCREATE(CThreadView, CHtmlView)

CThreadView::CThreadView()
{
	m_bDidInitialNavigate = FALSE;

	// Inherit the default open-attachment mode
	m_nOpenAttachmentMode = theApp.m_nOpenAttachmentMode;
}

CThreadView::~CThreadView()
{
}

void CThreadView::DoDataExchange(CDataExchange* pDX)
{
	CHtmlView::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CThreadView, CHtmlView)
	ON_COMMAND(ID_GO_BACK, OnGoBack)
	ON_COMMAND(ID_GO_FORWARD, OnGoForward)
	ON_COMMAND(ID_EDIT_FIND, &CThreadView::OnEditFind)
	ON_COMMAND(ID_OPENATTACHMENTS_INSAMEWINDOW, OnOpenAttachmentsInSameWindow)
	ON_UPDATE_COMMAND_UI(ID_OPENATTACHMENTS_INSAMEWINDOW, OnUpdateOpenAttachmentsInSameWindow)
	ON_COMMAND(ID_OPENATTACHMENTS_INNEWWINDOW, OnOpenAttachmentsInNewWindow)
	ON_UPDATE_COMMAND_UI(ID_OPENATTACHMENTS_INNEWWINDOW, OnUpdateOpenAttachmentsInNewWindow)
	ON_COMMAND(ID_OPENATTACHMENTS_INDEFAULTWINDOWSAPPLICATION, OnOpenAttachmentsInDefaultWindowsApplication)
	ON_UPDATE_COMMAND_UI(ID_OPENATTACHMENTS_INDEFAULTWINDOWSAPPLICATION, OnUpdateOpenAttachmentsInDefaultWindowsApplication)
END_MESSAGE_MAP()


// CThreadView diagnostics

#ifdef _DEBUG
void CThreadView::AssertValid() const
{
	CHtmlView::AssertValid();
}

void CThreadView::Dump(CDumpContext& dc) const
{
	CHtmlView::Dump(dc);
}
#endif //_DEBUG


// CThreadView message handlers


void CThreadView::OnInitialUpdate()
{
	CHtmlView::OnInitialUpdate();

	CThreadDoc *pDoc = (CThreadDoc *) GetDocument();

	ASSERT(pDoc->IsKindOf(RUNTIME_CLASS(CThreadDoc)));

	CString csFile = pDoc->GetPathName();

	// Bring up our input file in the HTML viewer (WebBrowser) control
	Navigate(csFile);

	m_bDidInitialNavigate = TRUE;
}


void CThreadView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
{
	CHtmlView::OnActivateView(bActivate, pActivateView, pDeactiveView);

	m_wndBrowser.SetFocus();
}

// "Back" button handler, mainly to return from viewing an image to the HTML conversation page
void CThreadView::OnGoBack()
{
	GoBack();
}

// We have a Back button, so why not have a Forward button too...
void CThreadView::OnGoForward()
{
	GoForward();
}


void CThreadView::OnEditFind()
{
	// Just bring up the web browser control's Find dialog (I didn't figure this out, someone on GitHub did!)
	ExecWB(OLECMDID_FIND, OLECMDEXECOPT_PROMPTUSER, nullptr, nullptr);
}

/*
 *	OnBeforeNavigate2
 *
 *	This is an event handler that gets called by the CHtmlView base class, when the HTML browser window is about to navigate to a new page.
 *	This happens in a few cases:
 *
 *		1. When opening a new SMS/MMS conversation thread, it gets called for the .html file we generated.  We use a flag to know
 *			whether we've done the initial file-open navigation, and if not, we let the HTML viewer load and display the page.
 *
 *		2. We get called here if the user clicks on an image, video or other type of MMS message attachment (file) in the conversation.
 *			We can handle that in a few ways:
 *
 *			- Bring up the attachment in the default Windows viewer application for that file type;
 *			- Open the attachment in a new CThreadView window in our own reader application;
 *			- Navigate to the attachment in the same (currently open) CThreadView window; in this case, the user has to click the
 *				Back toolbar button in order to get back to the main conversation display, so it's a little unwieldy.
 *
 *		3. We get called here if the user has navigated to an attachment in the same CThreadView window and then clicks the Back
 *			toolbar button.  In this case, we let the HTML viewer handle it normally, i.e. by returning to the SMS/MMS conversation view.
 */

void CThreadView::OnBeforeNavigate2(LPCTSTR pszURL, DWORD dwFlags, LPCTSTR pszTargetFrameName, CByteArray& baPostedData, LPCTSTR pszHeaders, BOOL *pbCancel)
{
	// TBD: special handling for http/https links - always open in external browser? have a View menu option for that too?

	if (!m_bDidInitialNavigate)
	{
		// This is the initial document load - let it proceed
		*pbCancel = 0;
		return;
	}

	/*
	 *	Otherwise, this is the user clicking on an attachment/file/link in the SMS/MMS message conversation.  We offer 3 different modes for
	 *	opening the attachment:
	 *		1) Open it in the same/current window, like a web page; the user can click the Back toolbar button to return to the conversation view.
	 *		2) Open it in a new MDI window.
	 *		3) Open it in the default Windows application that handles that file type.
	 */

	switch (m_nOpenAttachmentMode)
	{
	case SMSMMS_OpenInSameWindow:
		// Navigate to the attachment in the same, current window
		*pbCancel = 0;
		return;

	case SMSMMS_OpenInNewWindow:
	{
		// Open the attachment in a new MDI window.  The new window has to be responsible for cleaning up its own attachment file,
		// so we make a new copy of the attachment file for it.
		TCHAR szTempFile[MAX_PATH];
		TCHAR szHTMLFile[MAX_PATH];
		const TCHAR *pszTempFileName;
		FILE *fpHTML = NULL;

		// Set the cancel flag, so that the original conversation-view window stays where it is
		*pbCancel = 1;

		// Find the filename portion of pszURL (which is a fully-qualified path to the attachment file the user clicked on)
		for (pszTempFileName = _tcschr(pszURL, '\0'); pszTempFileName >= pszURL; --pszTempFileName)
			if (*pszTempFileName == '\\' || *pszTempFileName == ':')
				break;

		// Point to the first character after the colon or backslash, or else point to the start of the filename string
		++pszTempFileName;

		// Make a copy of the attachment file, to a new temporary file
		if (!MakeTempFileName(szTempFile, _countof(szTempFile), pszTempFileName))
		{
			AfxMessageBox(_T("Failed creating attachment temporary filename!"), MB_OK);
			return;
		}

		if (!CopyFile(pszURL, szTempFile, FALSE))
		{
			AfxMessageBox(_T("Failed copying attachment file!"), MB_OK);
			return;
		}

		/*
		 *	We also build a little HTML file to encapsulate the attachment file; this was done because otherwise, the CHtmlView will sometimes pop up
		 *	an "Open or Save" dialog... e.g. it seemed to always do that for .png files, but not for .jpeg files.  Creating a simple HTML page to
		 *	contain the attachment seems to solve that.
		 */

		if (!MakeTempFileName(szHTMLFile, _countof(szHTMLFile), _T("Attach.html")))
		{
			AfxMessageBox(_T("Failed creating attachment HTML temporary filename!"), MB_OK);
			return;
		}

		if (_tfopen_s(&fpHTML, szHTMLFile, _T("wt")) != 0)
		{
			AfxMessageBox(_T("Failed creating HTML temp output file!"), MB_OK);
			return;
		}

		_ftprintf(fpHTML, _T(""));

		_ftprintf(fpHTML, _T("<html>\n\n"));
		_ftprintf(fpHTML, _T("<body>\n\n"));

		// Now the image...
		_ftprintf(fpHTML, _T("<img src=\"file://%s\"/>"), szTempFile);

		_ftprintf(fpHTML, _T("</body>\n\n"));

		_ftprintf(fpHTML, _T("</html>\n\n"));
		
		fclose(fpHTML);

		// Open the attachment in a new CThreadView window
		CThreadDoc *pThreadDoc = (CThreadDoc *)theApp.m_pThreadDocTemplate->OpenDocumentFile(szHTMLFile, FALSE, TRUE);

		if (pThreadDoc == NULL)
			return;

		ASSERT(pThreadDoc->IsKindOf(RUNTIME_CLASS(CThreadDoc)));

		// Give the document the HTML and temp filenames - it will remove both files when the document/window is closed
		pThreadDoc->m_vTempFiles.push_back(wstring(szHTMLFile));
		pThreadDoc->m_vTempFiles.push_back(wstring(szTempFile));

		return;
	}

	case SMSMMS_OpenInExternalViewer:
		// Launch the default Windows app for this attachment/file type.  We don't want to complete the navigation in the local window,
		// so set the cancel flag...
		*pbCancel = 1;

		// TBD: any security risks here? Prompt the user first? Restrict this action by file type (i.e. extension)?
		ShellExecute(GetSafeHwnd(), NULL, pszURL, NULL, NULL, SW_SHOWNORMAL);
		return;

	default:
		ASSERT(FALSE);
	}
}

// View menu handlers for the three attachment viewing modes
void CThreadView::OnOpenAttachmentsInSameWindow()
{
	m_nOpenAttachmentMode = theApp.m_nOpenAttachmentMode = SMSMMS_OpenInSameWindow;
}


void CThreadView::OnUpdateOpenAttachmentsInSameWindow(CCmdUI *pCmdUI)
{
	pCmdUI->SetRadio(m_nOpenAttachmentMode == SMSMMS_OpenInSameWindow);
}


void CThreadView::OnOpenAttachmentsInNewWindow()
{
	m_nOpenAttachmentMode = theApp.m_nOpenAttachmentMode = SMSMMS_OpenInNewWindow;
}


void CThreadView::OnUpdateOpenAttachmentsInNewWindow(CCmdUI *pCmdUI)
{
	pCmdUI->SetRadio(m_nOpenAttachmentMode == SMSMMS_OpenInNewWindow);
}


void CThreadView::OnOpenAttachmentsInDefaultWindowsApplication()
{
	m_nOpenAttachmentMode = theApp.m_nOpenAttachmentMode = SMSMMS_OpenInExternalViewer;
}


void CThreadView::OnUpdateOpenAttachmentsInDefaultWindowsApplication(CCmdUI *pCmdUI)
{
	pCmdUI->SetRadio(m_nOpenAttachmentMode == SMSMMS_OpenInExternalViewer);
}

