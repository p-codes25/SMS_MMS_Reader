// FindTextDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SMS_MMS_Reader.h"
#include "FindTextDlg.h"
#include "afxdialogex.h"


// CFindTextDlg dialog

IMPLEMENT_DYNAMIC(CFindTextDlg, CDialogEx)

CFindTextDlg::CFindTextDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_FIND, pParent)
	, m_strText(_T(""))
{

}

CFindTextDlg::~CFindTextDlg()
{
}

void CFindTextDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT, m_strText);
}


BEGIN_MESSAGE_MAP(CFindTextDlg, CDialogEx)
END_MESSAGE_MAP()


// CFindTextDlg message handlers
