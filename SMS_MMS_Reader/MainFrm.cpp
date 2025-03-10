
// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "SMS_MMS_Reader.h"
#include "SMS_MMS_ReaderDoc.h"

#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)
	ON_WM_CREATE()
	ON_WM_DROPFILES()
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT));

	// TODO: Delete these three lines if you don't want the toolbar to be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CMDIFrameWnd::PreCreateWindow(cs) )
		return FALSE;

	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return TRUE;
}

// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CMDIFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWnd::Dump(dc);
}
#endif //_DEBUG


// CMainFrame message handlers


// This handles files dragged-and-dropped from Explorer onto the SMS/MMS reader app window, whether you drop them
// onto an open area or onto an open conversation window...
void CMainFrame::OnDropFiles(HDROP hDropInfo)
{
	// By default, we open each dropped file in its own window, unless the user asks to load it all into one window
	bool bSeparateWindows = true;

	CWinApp* pApp = AfxGetApp();
	ASSERT(pApp != NULL);

	CSMS_MMS_ReaderDoc *pDoc = NULL;

	/*
	 *	This stuff was adapted from VS2015's version of CMDIFrameWnd::OnDropFiles()...
	 */

	SetActiveWindow();      // activate us first ! TBD: does this work??  It doesn't seem to make the app window topmost...

	UINT nFiles = ::DragQueryFile(hDropInfo, (UINT)-1, NULL, 0);

	if (nFiles > 1)
	{
		if (AfxMessageBox(_T("Do you want to open all the files in a single window?"), MB_YESNO) == IDYES)
			bSeparateWindows = false;
	}

	for (UINT iFile = 0; iFile < nFiles; iFile++)
	{
		TCHAR szFileName[_MAX_PATH];
		::DragQueryFile(hDropInfo, iFile, szFileName, _MAX_PATH);

		// For the first file, we always open a new window; after that, follow the user's choice
		if (iFile == 0 || bSeparateWindows)
		{
			pDoc = (CSMS_MMS_ReaderDoc *) pApp->OpenDocumentFile(szFileName);

			if (!pDoc)
				break;

			ASSERT(pDoc->IsKindOf(RUNTIME_CLASS(CSMS_MMS_ReaderDoc)));
		}
		else
		{
			// This reads subsequent files into the window we already opened
			ASSERT(pDoc);

			if (!pDoc->DoReadFile(szFileName))
				break;
		}
	}

	::DragFinish(hDropInfo);
}
