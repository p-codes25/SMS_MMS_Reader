// FilterConversationsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SMS_MMS_Reader.h"
#include "FilterConversationsDlg.h"
#include "afxdialogex.h"


// CFilterConversationsDlg dialog

IMPLEMENT_DYNAMIC(CFilterConversationsDlg, CDialogEx)

CFilterConversationsDlg::CFilterConversationsDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_FILTER_CONVERSATIONS, pParent)
	, m_csFilterText(_T(""))
	, m_csFilterAddress(_T(""))
	, m_tFilterStartDate(0)
	, m_tFilterEndDate(0)
{

}

CFilterConversationsDlg::~CFilterConversationsDlg()
{
}

void CFilterConversationsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_FILTER_TEXT, m_csFilterText);
	DDX_Text(pDX, IDC_FILTER_ADDRESS, m_csFilterAddress);
	DDX_DateTimeCtrl(pDX, IDC_DATETIMEPICKER1, m_tFilterStartDate);
	DDX_DateTimeCtrl(pDX, IDC_DATETIMEPICKER2, m_tFilterEndDate);
	DDX_Control(pDX, IDC_DATETIMEPICKER1, m_ctlFilterStartDate);
	DDX_Control(pDX, IDC_DATETIMEPICKER2, m_ctlFilterEndDate);
	DDX_Radio(pDX, IDC_SHOW_ALL_MESSAGES, m_nFilterMessages);
}


BEGIN_MESSAGE_MAP(CFilterConversationsDlg, CDialogEx)
	ON_BN_CLICKED(IDC_ALL_DATES, &CFilterConversationsDlg::OnBnClickedAllDates)
END_MESSAGE_MAP()


// CFilterConversationsDlg message handlers


void CFilterConversationsDlg::OnBnClickedAllDates()
{
	m_ctlFilterStartDate.SetTime(m_tDefaultStartDate);
	m_ctlFilterEndDate.SetTime(m_tDefaultEndDate);
}
