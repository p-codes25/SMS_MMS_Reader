#pragma once


// CFindTextDlg dialog

class CFindTextDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CFindTextDlg)

public:
	CFindTextDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CFindTextDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_FIND };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString m_strText;
};
