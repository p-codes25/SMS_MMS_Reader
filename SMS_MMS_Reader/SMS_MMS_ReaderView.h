
// SMS_MMS_ReaderView.h : interface of the CSMS_MMS_ReaderView class
//

#pragma once


class CSMS_MMS_ReaderView : public CScrollView
{
protected: // create from serialization only
	CSMS_MMS_ReaderView();
	DECLARE_DYNCREATE(CSMS_MMS_ReaderView)

// Attributes
public:
	CSMS_MMS_ReaderDoc* GetDocument() const;

	CListCtrl m_ctlThreadList;

	int m_nSortColumn;
	BOOL m_bSortAscending;

	// Holds the text string used to filter conversations
	CString m_csFilterText;

	// String used to filter conversations by address/name
	CString m_csFilterAddress;

	time_t	m_tFilterStartDate;
	time_t	m_tFilterEndDate;

	time_t m_tDefaultStartDate;
	time_t m_tDefaultEndDate;

	// 0 = show all messages; 1 = show only messages that match the filters
	int m_nFilterMessages;

// Operations
public:
	void RefreshList();
	void InitFilters();

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual void OnInitialUpdate(); // called first time after construct
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

	bool ThreadMatchesFilter(CSMSThread& theThread);
	bool MessageMatchesFilter(CSMSMessage &msg);

// Implementation
public:
	virtual ~CSMS_MMS_ReaderView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnViewThread();
	afx_msg void OnDblClkList(NMHDR *pNMHDR, LRESULT *plResult);
	afx_msg void CSMS_MMS_ReaderView::OnColumnClkList(NMHDR *pNMHDR, LRESULT *plResult);
	afx_msg void OnUpdateFileSave(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileSaveAs(CCmdUI* pCmdUI);
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);
	afx_msg void OnEditFind();
	afx_msg void OnViewFilterconversations();
};

#ifndef _DEBUG  // debug version in SMS_MMS_ReaderView.cpp
inline CSMS_MMS_ReaderDoc* CSMS_MMS_ReaderView::GetDocument() const
   { return reinterpret_cast<CSMS_MMS_ReaderDoc*>(m_pDocument); }
#endif

