#pragma once
#include "afxcmn.h"
#include "afxwin.h"


// CStatusDlg dialog

class CStatusDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CStatusDlg)

public:
	CStatusDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CStatusDlg();

	void SetFractionComplete(double dFractionComplete);

	BOOL m_bCancelClicked;

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_STATUS };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CProgressCtrl m_ctlProgressBar;
	CStatic m_ctlStatusText;
	virtual void OnCancel();
};
