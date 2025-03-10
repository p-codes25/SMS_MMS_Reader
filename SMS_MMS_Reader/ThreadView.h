#pragma once

#ifdef _WIN32_WCE
#error "CHtmlView is not supported for Windows CE."
#endif 

/*
 *	CThreadView document - this view class represents a single SMS/MMSconversation with all its messages, displayed
 *	using the CHtmlView's web browser control.
 */

class CThreadView : public CHtmlView
{
	DECLARE_DYNCREATE(CThreadView)

protected:
	CThreadView();           // protected constructor used by dynamic creation
	virtual ~CThreadView();

	BOOL m_bDidInitialNavigate;

	// Mode for opening attachments when the user clicks on them; takes one of the enum eOpenAttachmentMode values in SMS_MMS_Reader.h
	int m_nOpenAttachmentMode;

public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual void OnInitialUpdate();
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);
	virtual void OnBeforeNavigate2(LPCTSTR pszURL, DWORD dwFlags, LPCTSTR pszTargetFrameName, CByteArray& baPostedData, LPCTSTR pszHeaders, BOOL *pbCancel);

	afx_msg void OnGoBack();
	afx_msg void OnGoForward();
	afx_msg void OnEditFind();

	afx_msg void OnOpenAttachmentsInSameWindow();
	afx_msg void OnUpdateOpenAttachmentsInSameWindow(CCmdUI *pCmdUI);
	afx_msg void OnOpenAttachmentsInNewWindow();
	afx_msg void OnUpdateOpenAttachmentsInNewWindow(CCmdUI *pCmdUI);
	afx_msg void OnOpenAttachmentsInDefaultWindowsApplication();
	afx_msg void OnUpdateOpenAttachmentsInDefaultWindowsApplication(CCmdUI *pCmdUI);
};


