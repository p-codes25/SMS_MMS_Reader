
/*
 *
 *	SMS_MMS_Reader - Copyright (c) 2019-2025 by p_codes25
 *
 *	SMS_MMS_Reader.cpp : Defines the class behaviors for the application.
 *
 *	This app is a viewer for SMS/MMS message backup files generated in XML format by
 *	SyncTech's (https://synctech.com.au/) SMS Backup & Restore.
 */

#include "stdafx.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "SMS_MMS_Reader.h"
#include "MainFrm.h"

#include "ChildFrm.h"
#include "SMS_MMS_ReaderDoc.h"
#include "SMS_MMS_ReaderView.h"
#include "ThreadDoc.h"
#include "ThreadView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CSMS_MMS_ReaderApp

BEGIN_MESSAGE_MAP(CSMS_MMS_ReaderApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, &CSMS_MMS_ReaderApp::OnAppAbout)
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, &CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, &CWinApp::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, &CWinApp::OnFilePrintSetup)
	ON_COMMAND(ID_FILE_OPEN, &CSMS_MMS_ReaderApp::OnFileOpen)
END_MESSAGE_MAP()


// CSMS_MMS_ReaderApp construction

CSMS_MMS_ReaderApp::CSMS_MMS_ReaderApp()
{
	// support Restart Manager (but TBD: I haven't tested this)
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_ALL_ASPECTS;

#ifdef _MANAGED
	// If the application is built using Common Language Runtime support (/clr):
	//     1) This additional setting is needed for Restart Manager support to work properly.
	//     2) In your project, you must add a reference to System.Windows.Forms in order to build.
	System::Windows::Forms::Application::SetUnhandledExceptionMode(System::Windows::Forms::UnhandledExceptionMode::ThrowException);
#endif

	// TODO: replace application ID string below with unique ID string; recommended
	// format for string is CompanyName.ProductName.SubProduct.VersionInformation
	SetAppID(_T("NoCompany.SMS_MMS_Reader.NoSubProduct.NoVersion"));

	m_pReaderDocTemplate = NULL;
	m_pThreadDocTemplate = NULL;

	// Place all significant initialization in InitInstance
}

// The one and only CSMS_MMS_ReaderApp object

CSMS_MMS_ReaderApp theApp;


// CSMS_MMS_ReaderApp initialization

BOOL CSMS_MMS_ReaderApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;

	InitCtrls.dwSize = sizeof(InitCtrls);

	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;

	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();

	EnableTaskbarInteraction(FALSE);

	// AfxInitRichEdit2() is required to use RichEdit control	
	// AfxInitRichEdit2();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("SMS-MMS-Reader"));

	// Load standard INI file options (including 9 MRU files)
	LoadStdProfileSettings(9);

	// Read other profile settings - currently just the open-attachment mode
	m_nOpenAttachmentMode = GetProfileInt(_T("Settings"), _T("OpenAttachmentMode"), SMSMMS_OpenInNewWindow);

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views
	m_pReaderDocTemplate = new CMultiDocTemplate(IDR_SMS_MMS_ReaderTYPE,
		RUNTIME_CLASS(CSMS_MMS_ReaderDoc),
		RUNTIME_CLASS(CChildFrame), // custom MDI child frame
		RUNTIME_CLASS(CSMS_MMS_ReaderView));

	if (!m_pReaderDocTemplate)
		return FALSE;

	AddDocTemplate(m_pReaderDocTemplate);

	m_pThreadDocTemplate = new CMultiDocTemplate(IDR_ThreadViewTYPE,
		RUNTIME_CLASS(CThreadDoc),
		RUNTIME_CLASS(CChildFrame), // custom MDI child frame
		RUNTIME_CLASS(CThreadView));

	if (!m_pThreadDocTemplate)
		return FALSE;

	AddDocTemplate(m_pThreadDocTemplate);
	
	// create main MDI Frame window
	CMainFrame* pMainFrame = new CMainFrame;

	if (!pMainFrame || !pMainFrame->LoadFrame(IDR_MAINFRAME))
	{
		delete pMainFrame;
		return FALSE;
	}

	m_pMainWnd = pMainFrame;

	// call DragAcceptFiles only if there's a suffix
	//  In an MDI app, this should occur immediately after setting m_pMainWnd
	// Enable drag/drop open
	m_pMainWnd->DragAcceptFiles();

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;

	// Don't open a blank window by default
	cmdInfo.m_nShellCommand = CCommandLineInfo::FileNothing;

	ParseCommandLine(cmdInfo);

	// Enable DDE Execute open
	EnableShellOpen();
	RegisterShellFileTypes(TRUE);


	// Dispatch commands specified on the command line.  Will return FALSE if
	// app was launched with /RegServer, /Register, /Unregserver or /Unregister.
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// The main window has been initialized, so show and update it
	pMainFrame->ShowWindow(m_nCmdShow);
	pMainFrame->UpdateWindow();

	return TRUE;
}

int CSMS_MMS_ReaderApp::ExitInstance()
{
	// Save profile settings
	WriteProfileInt(_T("Settings"), _T("OpenAttachmentMode"), m_nOpenAttachmentMode);

	AfxOleTerm(FALSE);

	return CWinApp::ExitInstance();
}

// CSMS_MMS_ReaderApp message handlers


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
public:
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
	ON_WM_SIZE()
END_MESSAGE_MAP()

// App command to run the dialog
void CSMS_MMS_ReaderApp::OnAppAbout()
{
	CAboutDlg aboutDlg;

	aboutDlg.DoModal();
}

// CSMS_MMS_ReaderApp message handlers

void CSMS_MMS_ReaderApp::OnFileOpen()
{
	CWinApp::OnFileOpen();
}

/*
 *	MakeTempFileName
 *
 *	Creates a new temporary file and returns the filename (on return, the returned temporary filename will exist and be zero-length,
 *	but no file handle is returned, so the caller must reopen it).
 *
 *	Why did I write this instead of using the standard Windows GetTempFileName()?  Mainly so that the resultant temporary filename
 *	would be similar to the original filename and thus easier to identify when I was poking around the %TEMP% directory during
 *	development...
 *
 *	PARAMETERS:
 *		pszFile			On success, receives the new, unique temporary file name that was created; on failure, receives the error string.
 *		nMaxFile		Size of the buffer at *pszFile, in CHARACTERS (not bytes!)
 *		pszBaseName		Filename ONLY (without a directory/path) which the new temporary filename will be based on.  The filename
 *						at *pszBaseName MUST have a file extension (e.g. ".jpg").  For example, for a pszBaseName of "IMG_1234.jpg",
 *						a temporary filename of "IMG_1234_001.jpg" might be generated and returned in *pszFile.
 *
 *	RETURNS:
 *		bool			true if the filename was generated and stored in *pszFile; false if an error occurred, in which case the error
 *						string is placed in *pszFile.
 *
 *	NOTES:
 *		The input filename (pszBaseName) should be a filename ONLY (without a directory).  The output filename will be a fully-qualified
 *		path+filename.
 */

bool MakeTempFileName(TCHAR *pszFile, int nMaxFile, const TCHAR *pszBaseName)
{
	TCHAR szFileName[MAX_PATH];
	TCHAR szTry[MAX_PATH];
	int nLen, nError, nTry;
	unsigned int uSuffix;
	HANDLE hFile;

	GetTempPath(_countof(szFileName), szFileName);

	nLen = (int)_tcslen(szFileName);

	// Make sure we have room to add a suffix if needed
	if (nLen >= MAX_PATH - 10)
	{
		_sntprintf_s(pszFile, nMaxFile, _TRUNCATE, _T("Temp path is too long (%d characters)."), nLen);

		return false;
	}

	if (nLen > 0 && szFileName[nLen - 1] != '\\')
	{
		_tcscat_s(szFileName, _countof(szFileName), _T("\\"));
	}

	_tcscat_s(szFileName, _countof(szFileName), pszBaseName);

	// TBD: how many attempts should we perform? After 100 tries we should get a unique filename...
	for (nTry = 0; nTry < 100; ++nTry)
	{
		TCHAR szSuffix[32];
		int nSuffixLen;
		TCHAR *pszDot;

		_tcscpy_s(szTry, _countof(szTry), szFileName);

		// Tack on a random numeric suffis
		if (rand_s(&uSuffix) != 0)
		{
			_sntprintf_s(pszFile, nMaxFile, _TRUNCATE, _T("Failed to generate random file suffix!"));

			return false;
		}

		// Use the number as a suffix to the filename, to make it unique but still recognizable
		wnsprintf(szSuffix, _countof(szSuffix), _T("_%u"), uSuffix);

		nSuffixLen = (int)_tcslen(szSuffix);

		pszDot = _tcsrchr(szTry, _T('.'));

		// If no dot, then append our suffix to the end of the filename
		if (pszDot == NULL)
			pszDot = _tcschr(szTry, '\0');

		memmove((void *)(pszDot + nSuffixLen), (void *)pszDot, (_tcslen(pszDot) + 1) * sizeof(TCHAR));

		memcpy((void *)pszDot, (void *)szSuffix, nSuffixLen * sizeof(TCHAR));

		// Try to create a new file; if this filename already exists, this will fail
		hFile = CreateFile(szTry,
			GENERIC_WRITE,
			0,
			NULL,
			CREATE_NEW,
			FILE_ATTRIBUTE_NORMAL,
			NULL);

		if (hFile != INVALID_HANDLE_VALUE)
		{
			// Success case -- we created the file!  Close it and return its name to the caller
			CloseHandle(hFile);
			_tcscpy_s(pszFile, nMaxFile, szTry);
			return true;
		}

		if ((nError = GetLastError()) != ERROR_FILE_EXISTS)
		{
			// Some error other than the file already existing -- bail out now
			_sntprintf_s(pszFile, nMaxFile, _TRUNCATE, _T("Error %d creating temp file '%s'."), nError, szTry);
			return false;
		}

		// Otherwise keep looping, try the next file number
	}

	_sntprintf_s(pszFile, nMaxFile, _TRUNCATE, _T("Too many temporary files for '%s'."), szTry);
	return false;
}
