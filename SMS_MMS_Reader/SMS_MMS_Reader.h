
// SMS_MMS_Reader.h : main header file for the SMS_MMS_Reader application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// Enum for open-attachment modes (m_nOpenAttachmentMode in ThreadView and the app object)
typedef enum
{
	SMSMMS_OpenInSameWindow,
	SMSMMS_OpenInNewWindow,
	SMSMMS_OpenInExternalViewer
} eOpenAttachmentMode;

// CSMS_MMS_ReaderApp:
// See SMS_MMS_Reader.cpp for the implementation of this class
//

class CSMS_MMS_ReaderApp : public CWinApp
{
public:
	CSMS_MMS_ReaderApp();

	CMultiDocTemplate* m_pReaderDocTemplate;
	CMultiDocTemplate* m_pThreadDocTemplate;

	// This holds the initial/default open-attachment mode, but each conversation view has its own setting also, which is inherited/copied from here
	int m_nOpenAttachmentMode;

// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Implementation
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnFileOpen();
};

extern CSMS_MMS_ReaderApp theApp;

bool MakeTempFileName(TCHAR *pszFile, int nMaxFile, const TCHAR *pszBaseName);

