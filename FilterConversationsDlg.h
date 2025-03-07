#pragma once
#include "atltime.h"
#include "afxdtctl.h"


// CFilterConversationsDlg dialog

class CFilterConversationsDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CFilterConversationsDlg)

public:
	CFilterConversationsDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CFilterConversationsDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_FILTER_CONVERSATIONS };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString m_csFilterText;
	CString m_csFilterAddress;
	CTime m_tFilterStartDate;
	CTime m_tFilterEndDate;
	CDateTimeCtrl m_ctlFilterStartDate;
	CDateTimeCtrl m_ctlFilterEndDate;

	// 0 = show all messages; 1 = show only messages that match the filters
	int m_nFilterMessages;

	// Date/time range of the whole file; passed in here so the reset-dates button can use it
	time_t m_tDefaultStartDate;
	time_t m_tDefaultEndDate;

	afx_msg void OnBnClickedAllDates();
};
