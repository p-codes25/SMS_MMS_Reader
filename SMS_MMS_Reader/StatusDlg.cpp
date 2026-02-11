// StatusDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SMS_MMS_Reader.h"
#include "StatusDlg.h"
#include "afxdialogex.h"


// CStatusDlg dialog

IMPLEMENT_DYNAMIC(CStatusDlg, CDialogEx)

CStatusDlg::CStatusDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_STATUS, pParent)
{
	m_bCancelClicked = FALSE;
}

CStatusDlg::~CStatusDlg()
{
}

void CStatusDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS1, m_ctlProgressBar);
	DDX_Control(pDX, IDC_STATUS_TEXT, m_ctlStatusText);

	m_ctlProgressBar.SetRange32(0, 65536);
}


BEGIN_MESSAGE_MAP(CStatusDlg, CDialogEx)
END_MESSAGE_MAP()


// CStatusDlg message handlers
void CStatusDlg::SetFractionComplete(double dFractionComplete)
{
	m_ctlProgressBar.SetPos((int)(dFractionComplete * 65536));
}


void CStatusDlg::OnCancel()
{
	m_bCancelClicked = TRUE;

	CDialogEx::OnCancel();
}

/*
 *	CheckCancelButton
 *
 *	RETURNS:
 *		bool		true if we should continue processing whatever user-cancelable operation there is; or
 *					false if the user clicked Cancel and we should stop.
 */

bool CStatusDlg::CheckCancelButton()
{
	MSG msg;

	// Simple message loop to allow the Cancel button to work
	while (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		if (!::IsDialogMessage(GetSafeHwnd(), &msg))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
	}

	if (m_bCancelClicked)
		return false;

	return true;
}

