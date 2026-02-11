/*
 *	SMS_MMS_ReaderView.cpp : implementation of the CSMS_MMS_ReaderView class
 *
 *	This is the main document view, which contains the list showing all the SMS/MMS conversations (threads).
 */

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "SMS_MMS_Reader.h"
#endif

#include "SMS_MMS_ReaderDoc.h"
#include "SMS_MMS_ReaderView.h"
#include "FilterConversationsDlg.h"

#include "ThreadDoc.h"
#include "base64.h"
#include "StatusDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


/*
 *	AdjustTime
 *
 *	Takes a time_t and sets the hour/minute/second portion to the specified values.  This is a helper used in
 *	filtering, since we only allow filtering by date and not time.
 */

void AdjustTime(time_t *pt, WORD wHour, WORD wMinute, WORD wSecond)
{
	SYSTEMTIME st;

	// Make a CTime out of the given time_t
	CTime t(*pt);

	t.GetAsSystemTime(st);

	st.wHour = wHour;
	st.wMinute = wMinute;
	st.wSecond = wSecond;

	// Convert the adjusted date/time back to a CTime
	CTime tOut(st, -1);

	*pt = tOut.GetTime();
}

// CSMS_MMS_ReaderView

IMPLEMENT_DYNCREATE(CSMS_MMS_ReaderView, CScrollView)

BEGIN_MESSAGE_MAP(CSMS_MMS_ReaderView, CScrollView)
	// Standard printing commands -- not supported for now
	// ON_COMMAND(ID_FILE_PRINT, &CScrollView::OnFilePrint)
	// ON_COMMAND(ID_FILE_PRINT_DIRECT, &CScrollView::OnFilePrint)
	// ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CScrollView::OnFilePrintPreview)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, &OnUpdateFileSave)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_AS, &OnUpdateFileSaveAs)

	ON_NOTIFY(NM_DBLCLK, 101, OnDblClkList)
	ON_NOTIFY(LVN_COLUMNCLICK, 101, OnColumnClkList)
	ON_WM_SIZE()
	ON_COMMAND(ID_VIEW_THREAD, OnViewThread)
	ON_COMMAND(ID_EDIT_FIND, &CSMS_MMS_ReaderView::OnEditFind)
	ON_COMMAND(ID_VIEW_FILTERCONVERSATIONS, &CSMS_MMS_ReaderView::OnViewFilterconversations)
END_MESSAGE_MAP()

// CSMS_MMS_ReaderView construction/destruction

void CSMS_MMS_ReaderView::OnUpdateFileSave(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(FALSE);
}

void CSMS_MMS_ReaderView::OnUpdateFileSaveAs(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(FALSE);
}

CSMS_MMS_ReaderView::CSMS_MMS_ReaderView()
{
	// TODO: add construction code here
	m_nSortColumn = 0;
	m_bSortAscending = TRUE;
	m_nFilterMessages = 0;
}

CSMS_MMS_ReaderView::~CSMS_MMS_ReaderView()
{
}

BOOL CSMS_MMS_ReaderView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CScrollView::PreCreateWindow(cs);
}

// CSMS_MMS_ReaderView drawing

void CSMS_MMS_ReaderView::OnDraw(CDC* /*pDC*/)
{
	CSMS_MMS_ReaderDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: add draw code for native data here
}

void CSMS_MMS_ReaderView::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();

	CRect rc;
	int nColumn = 0;

	GetClientRect(&rc);

	if (!m_ctlThreadList.CreateEx(0, WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_REPORT, rc, this, 101))
		AfxThrowResourceException();

	m_ctlThreadList.SetExtendedStyle(m_ctlThreadList.GetExtendedStyle() | LVS_EX_FULLROWSELECT);

	// Search on ADD_NEW_LIST_COLUMNS_HERE tags for other places in the code to update when adding/removing/changing columns here!
	m_ctlThreadList.InsertColumn(nColumn++, _T("Address"), LVCFMT_LEFT, 250, -1);
	m_ctlThreadList.InsertColumn(nColumn++, _T("Name"), LVCFMT_LEFT, 250, -1);
	m_ctlThreadList.InsertColumn(nColumn++, _T("# Messages"), LVCFMT_LEFT, 100, 0);
	m_ctlThreadList.InsertColumn(nColumn++, _T("First Date/Time"), LVCFMT_LEFT, 150, 1);
	m_ctlThreadList.InsertColumn(nColumn++, _T("Last Date/Time"), LVCFMT_LEFT, 150, 1);
	m_ctlThreadList.InsertColumn(nColumn++, _T("Total Size"), LVCFMT_LEFT, 100, 2);
	m_ctlThreadList.Invalidate();

	// I guess we need to SetScrollSizes(), even though we're not using the view's scrollbars here...
	CSize sizeTotal;
	
	sizeTotal.cx = sizeTotal.cy = 100;
	SetScrollSizes(MM_TEXT, sizeTotal);

	InitFilters();

	RefreshList();
}


// CSMS_MMS_ReaderView printing

BOOL CSMS_MMS_ReaderView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CSMS_MMS_ReaderView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CSMS_MMS_ReaderView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}


// CSMS_MMS_ReaderView diagnostics

#ifdef _DEBUG
void CSMS_MMS_ReaderView::AssertValid() const
{
	CScrollView::AssertValid();
}

void CSMS_MMS_ReaderView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}

CSMS_MMS_ReaderDoc* CSMS_MMS_ReaderView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CSMS_MMS_ReaderDoc)));
	return (CSMS_MMS_ReaderDoc*)m_pDocument;
}
#endif //_DEBUG


// CSMS_MMS_ReaderView message handlers


void CSMS_MMS_ReaderView::OnSize(UINT nType, int cx, int cy)
{
	CScrollView::OnSize(nType, cx, cy);

	if (m_ctlThreadList.GetSafeHwnd())
		m_ctlThreadList.MoveWindow(0, 0, cx, cy);
}

// timeStamp is passed in as milliseconds since 1/1/1970, GMT, and so must be converted to local time for display
void TimeStampToString(__int64 timeStamp, TCHAR *pszBuf, int nMaxBuf)
{
	time_t tTime;
	struct tm theTime;

	tTime = timeStamp / 1000;

	if (localtime_s(&theTime, &tTime) == 0)
		_tcsftime(pszBuf, nMaxBuf, _T("%m/%d/%Y %I:%M:%S%p"), &theTime);
	else
		_sntprintf_s(pszBuf, nMaxBuf, _TRUNCATE, _T("Invalid time value: %lu"), (unsigned long)tTime);
}

void CSMS_MMS_ReaderView::InitFilters()
{
	CSMS_MMS_ReaderDoc* pDoc = GetDocument();

	ASSERT_VALID(pDoc);

	if (!pDoc)
		return;

	CSMSThreadList *pList = pDoc->GetThreadList();

	/*
	 *	Init some filtering stuff - in particular, we default the filter date range to the range of the loaded messages.
	 *	We scale the times from milliseconds to seconds, and then we adjust the start date down to the start of the
	 *	oldest message's day, and adjust the end date to the end of the newest message's day.
	 */

	m_tFilterStartDate = pList->m_OldestOverallMessage / 1000;
	m_tFilterEndDate = pList->m_NewestOverallMessage / 1000;

	// We need the start date's time to be set to 00:00:00 (midnight)
	AdjustTime(&m_tFilterStartDate, 0, 0, 0);
	AdjustTime(&m_tFilterEndDate, 23, 59, 59);

	m_tDefaultStartDate = m_tFilterStartDate;
	m_tDefaultEndDate = m_tFilterEndDate;
}

/*
 *	RefreshList
 *
 *	Reloads the conversation list, based on the current filtering and other settings.
 */

void CSMS_MMS_ReaderView::RefreshList()
{
	LV_ITEM item;
	TCHAR szBuf[256];
	UINT64 ldwSize;

	// Sorted list of threads, keyed by address
	vector<wstring> vSortedThreads;

	CThreadMap::iterator itThread;

	memset(&item, 0, sizeof(item));

	CSMS_MMS_ReaderDoc* pDoc = GetDocument();

	ASSERT_VALID(pDoc);

	if (!pDoc)
		return;

	// Get the "thread" list (list of SMS/MMS conversations... the term "thread" is unfortunate in this context...)
	CSMSThreadList *pList = pDoc->GetThreadList();

	m_ctlThreadList.SetRedraw(FALSE);
	m_ctlThreadList.DeleteAllItems();

	// Build a vector out of the threads (filtering out ones the user doesn't want, based on filtering settings)...
	for (itThread = pList->m_Threads.begin(); itThread != pList->m_Threads.end(); ++itThread)
	{
		CSMSThread& theThread = itThread->second;

		// See if we want to include this thread in the list
		if (!ThreadMatchesFilter(theThread))
			continue;

		vSortedThreads.push_back(theThread.m_strAddresses);
	}

	// ADD_NEW_LIST_COLUMNS_HERE -- define sorting for each column in the list-view control
	switch (m_nSortColumn)
	{
	case 0:
		// Address - forward (lexicographical) sorting order
		sort(vSortedThreads.begin(), vSortedThreads.end(),
			[&pList, this](wstring& a, wstring& b)
			{
				CThreadMap::iterator pa = pList->m_Threads.find(a);
				CThreadMap::iterator pb = pList->m_Threads.find(b);

				if (pa == pList->m_Threads.end() || pb == pList->m_Threads.end())
				{
#ifdef _DEBUG
					ASSERT(FALSE);
#endif
					return false;
				}

				return (pa->second.m_strAddresses.compare(pb->second.m_strAddresses) < 0) == this->m_bSortAscending;
			});

		break;

	case 1:
		// Name - forward (lexicographical) sorting order
		sort(vSortedThreads.begin(), vSortedThreads.end(),
			[&pList, this](wstring& a, wstring& b)
		{
			CThreadMap::iterator pa = pList->m_Threads.find(a);
			CThreadMap::iterator pb = pList->m_Threads.find(b);

			if (pa == pList->m_Threads.end() || pb == pList->m_Threads.end())
			{
#ifdef _DEBUG
				ASSERT(FALSE);
#endif
				return false;
			}

			return (pa->second.m_strContactNames.compare(pb->second.m_strContactNames) < 0) == this->m_bSortAscending;
		});

		break;

	case 2:
		// # messages - reverse sorting order for most messages to least
		sort(vSortedThreads.begin(), vSortedThreads.end(),
			[&pList, this](wstring& a, wstring& b)
		{
			CThreadMap::iterator pa = pList->m_Threads.find(a);
			CThreadMap::iterator pb = pList->m_Threads.find(b);

			if (pa == pList->m_Threads.end() || pb == pList->m_Threads.end())
			{
#ifdef _DEBUG
				ASSERT(FALSE);
#endif
				return false;
			}

			return (pa->second.m_Messages.size() > pb->second.m_Messages.size()) == this->m_bSortAscending;
		});

		break;

	case 3:
		// first message date/time - reverse sorting order for newest to oldest
		sort(vSortedThreads.begin(), vSortedThreads.end(),
			[&pList, this](wstring& a, wstring& b)
		{
			CThreadMap::iterator pa = pList->m_Threads.find(a);
			CThreadMap::iterator pb = pList->m_Threads.find(b);

			if (pa == pList->m_Threads.end() || pb == pList->m_Threads.end())
			{
#ifdef _DEBUG
				ASSERT(FALSE);
#endif
				return false;
			}

			return (pa->second.m_OldestThreadMessage > pb->second.m_OldestThreadMessage) == this->m_bSortAscending;
		});

		break;

	case 4:
		// last message date/time - reverse sorting order for newest to oldest
		sort(vSortedThreads.begin(), vSortedThreads.end(),
			[&pList, this](wstring& a, wstring& b)
		{
			CThreadMap::iterator pa = pList->m_Threads.find(a);
			CThreadMap::iterator pb = pList->m_Threads.find(b);

			if (pa == pList->m_Threads.end() || pb == pList->m_Threads.end())
			{
#ifdef _DEBUG
				ASSERT(FALSE);
#endif
				return false;
			}

			return (pa->second.m_NewestThreadMessage > pb->second.m_NewestThreadMessage) == this->m_bSortAscending;
		});

		break;

	case 5:
		// total size - reverse sorting order for largest to smallest
		sort(vSortedThreads.begin(), vSortedThreads.end(),
			[&pList, this](wstring& a, wstring& b)
		{
			CThreadMap::iterator pa = pList->m_Threads.find(a);
			CThreadMap::iterator pb = pList->m_Threads.find(b);

			if (pa == pList->m_Threads.end() || pb == pList->m_Threads.end())
			{
#ifdef _DEBUG
				ASSERT(FALSE);
#endif
				return false;
			}

			return (pa->second.m_ldwTotalSize > pb->second.m_ldwTotalSize) == this->m_bSortAscending;
		});

		break;

	default:
		ASSERT(FALSE);
	}

	vector<wstring>::iterator i;

	// This is the list control item index, which will normally match the index into vSortedThreads
	int nIndex = 0;

	// ADD_NEW_LIST_COLUMNS_HERE -- define text formatting for each column in the list-view control
	for (i = vSortedThreads.begin(); i != vSortedThreads.end(); ++i)
	{
		// Get the address key
		wstring& strAddresses = *i;

		// Now look up the thread
		CThreadMap::iterator p = pList->m_Threads.find(strAddresses);

		if (p == pList->m_Threads.end())
		{
#ifdef _DEBUG
			ASSERT(FALSE);
#endif
			continue;
		}

		CSMSThread& theThread = p->second;

		int nColumn = 0;

		item.mask = LVIF_TEXT;
		item.iItem = nIndex;
		item.iSubItem = nColumn++;
		item.pszText = const_cast<LPWSTR> (theThread.m_strAddresses.c_str());

		// Main item with address (aka phone # or email)
		m_ctlThreadList.InsertItem(&item);

		// Contact name(s)
		m_ctlThreadList.SetItemText(nIndex, nColumn++, theThread.m_strContactNames.c_str());

		// Number of messages
		_sntprintf_s(szBuf, _countof(szBuf), _TRUNCATE, _T("%lu"), (unsigned long)theThread.m_Messages.size());

		m_ctlThreadList.SetItemText(nIndex, nColumn++, szBuf);

		// Timestamp of oldest message in the thread
		TimeStampToString(theThread.m_OldestThreadMessage, szBuf, _countof(szBuf));

		m_ctlThreadList.SetItemText(nIndex, nColumn++, szBuf);

		// Timestamp of newest message in the thread
		TimeStampToString(theThread.m_NewestThreadMessage, szBuf, _countof(szBuf));

		m_ctlThreadList.SetItemText(nIndex, nColumn++, szBuf);

		// Sum of sizes of the thread's messages, in bytes/KB/MB
		ldwSize = theThread.m_ldwTotalSize;

		if (ldwSize < 1024)
			_sntprintf_s(szBuf, _countof(szBuf), _TRUNCATE, _T("0.%I64uK"), ldwSize / 103);
		else if (ldwSize < 1024 * 1024)
			_sntprintf_s(szBuf, _countof(szBuf), _TRUNCATE, _T("%I64u.%I64uK"), ldwSize / 1024, (ldwSize % 1024) / 103);
		else
			_sntprintf_s(szBuf, _countof(szBuf), _TRUNCATE, _T("%I64u.%I64uM"), ldwSize / 1048576, (ldwSize % (1048576)) / 104858);

		m_ctlThreadList.SetItemText(nIndex, nColumn++, szBuf);

		++nIndex;
	}

	m_ctlThreadList.SetRedraw(TRUE);
	m_ctlThreadList.Invalidate();
}

void CSMS_MMS_ReaderView::OnColumnClkList(NMHDR *pNMHDR, LRESULT *plResult)
{
	NM_LISTVIEW *pNMListView = (NM_LISTVIEW *)pNMHDR;

	// Clicking selected column toggles sorting order
	if (pNMListView->iSubItem == m_nSortColumn)
	{
		m_bSortAscending = !m_bSortAscending;
	}
	else
	{
		m_nSortColumn = pNMListView->iSubItem;
		m_bSortAscending = TRUE;
	}

	RefreshList();
}

// Handler for VK_RETURN in the thread-list view... same as double-clicking on a thread
void CSMS_MMS_ReaderView::OnViewThread()
{
	LRESULT lDummy;

	OnDblClkList(NULL, &lDummy);
}

/*
 *	When you double-click a conversation (thread) in the list, this opens the conversation in a new window.
 *	This works by exporting the messages to an HTML file and opening a CHtmlView-based window (CThreadDoc /
 *	CThreadView) to view it.
 */

void CSMS_MMS_ReaderView::OnDblClkList(NMHDR *_pNMHDR, LRESULT *plResult)
{
	// This isn't used, because we don't need it and because OnViewThread() doesn't supply it!
	// NM_LISTVIEW *pNMListView = (NM_LISTVIEW *) pNMHDR;

	const DWORD dwUpdateTicks = 250;

	DWORD dwNextUpdate = 0;
	POSITION pos;
	int nIndex;
	int nLen;
	FILE *fp;
	TCHAR szFileName[MAX_PATH];
	CString strError;

	// MMS part attachment file temp path for HTML <img> tag to refer to
	TCHAR szMMSFile[MAX_PATH];

	TCHAR *pszMMSFileName;

	// List of attachment files we're creating -- we hand this off to the CThreadView document so it can clean them up
	vector<wstring> vTempFiles;

	*plResult = 0;

	pos = m_ctlThreadList.GetFirstSelectedItemPosition();

	if (pos == NULL)
		return;

	nIndex = m_ctlThreadList.GetNextSelectedItem(pos);

	wstring strAddresses = m_ctlThreadList.GetItemText(nIndex, 0);

	CSMS_MMS_ReaderDoc* pDoc = GetDocument();

	ASSERT_VALID(pDoc);

	CSMSThreadList *pList = pDoc->GetThreadList();

	CThreadMap::iterator p = pList->m_Threads.find(strAddresses);

	if (p == pList->m_Threads.end())
	{
#ifdef _DEBUG
		ASSERT(FALSE);
#endif

		return;
	}

	CSMSThread& theThread = p->second;

	// Make up a temporary filename
	UUID uuid;
	TCHAR *pszUUID = NULL;

	if (UuidCreate(&uuid) != RPC_S_OK ||
		UuidToString(&uuid, (RPC_WSTR *)&pszUUID) != RPC_S_OK ||
		GetTempPath(_countof(szFileName), szFileName) == 0)
	{
		AfxMessageBox(_T("Failed creating temporary filename ID!"), MB_OK);
		return;
	}

	if ((nLen = (int) _tcslen(szFileName)) > 0 && szFileName[nLen - 1] != '\\')
	{
		_tcscat_s(szFileName, _countof(szFileName), _T("\\"));
	}

	_tcscat_s(szFileName, _countof(szFileName), pszUUID);
	_tcscat_s(szFileName, _countof(szFileName), _T(".html"));

	RpcStringFree((RPC_WSTR *) &pszUUID);

	if (_tfopen_s(&fp, szFileName, _T("w")) != 0)
	{
		int nError = errno;
		TCHAR szError[256];

		_tcserror_s(szError, _countof(szError), errno);
		strError.Format(_T("Error %d (%s) creating HTML output file '%s'!"), nError, szError, szFileName);
		AfxMessageBox(strError, MB_OK);
		return;
	}

	/*
	 *	We got the file; now build an HTML document to display the conversation, with inline MMS attachments, etc.  The HTML code here is
	 *	pretty simplistic and could probably be improved...
	 */

	_ftprintf(fp, _T("<html>\n\n"));
	_ftprintf(fp, _T("<head>\n\n"));
	_ftprintf(fp, _T("<style>\n\n"));

	// General styles for the conversation HTML page
	_ftprintf(fp, _T("	body { background-color: white; font-family: Arial, Helvetica, sans-serif; }\n\n"));

	// Styles for conversation thread heading
	_ftprintf(fp, _T("	.conversationHeading { background-color: grey; color: white; }\n\n"));

	// Styles for sent messages
	_ftprintf(fp, _T("	.messageSent { background-color: #30E030; color: black; border-radius: 40px; margin-left: 20px; padding-top: 12px; padding-bottom: 12px; padding-left: 12px; padding-right: 12px; margin-top: 4px; margin-bottom: 4px; }\n\n"));

	// Styles for received messages
	_ftprintf(fp, _T("	.messageReceived { background-color: #E0E0E0; color: black; border-radius: 40px; margin-right: 20px; padding-top: 12px; padding-bottom: 12px; padding-left: 12px; padding-right: 12px; margin-top: 4px; margin-bottom: 4px; }\n\n"));

	// Styles for inline images in MMS messages
	_ftprintf(fp, _T("	.messageImage { width:150px; }\n\n"));

	_ftprintf(fp, _T("</style>\n\n"));
	_ftprintf(fp, _T("</head>\n\n"));

	_ftprintf(fp, _T("<body>\n\n"));

	// TBD: pick the names & addresses apart... names are separated with commas, addresses are separated by tildes?  They don't necessarily always match up one-to-one?
	_ftprintf(fp, _T("<div class=\"conversationHeading\">Conversation with %s (%s)</div>\n"),
		theThread.m_strContactNames.c_str(),
		theThread.m_strAddresses.c_str());

	CStatusDlg dlg;

	if (!dlg.Create((UINT)IDD_STATUS, AfxGetMainWnd()))
		AfxThrowResourceException();

	// TBD...
	dlg.m_ctlStatusText.SetWindowTextW(_T("Processing Conversation..."));

	dlg.ShowWindow(SW_SHOW);
	dlg.CenterWindow();
	dlg.UpdateWindow();

	// Now loop through the messages and write them out
	vector<CSMSMessage>::iterator iMessage;

	int nTotalMessages = (int) theThread.m_Messages.size();
	int nMessage;

	// Loop through the messages in this conversation
	for (iMessage = theThread.m_Messages.begin(), nMessage = 0;
		iMessage < theThread.m_Messages.end();
		++iMessage, ++nMessage)
	{
		CSMSMessage msg = *iMessage;
		vector<CMessagePart>::iterator iPart;
		LPCTSTR pszClass;
		time_t tTime;
		struct tm theTime;
		TCHAR szTime[128];
	
		// If we are filtering messages, and this one doesn't match the filters, skip it
		if (m_nFilterMessages && !MessageMatchesFilter(msg))
			continue;

		if (GetTickCount() >= dwNextUpdate)
		{
			if (!dlg.CheckCancelButton())
			{
				// Break out, but let the window open anyway in case the user just wants to view part of it
				break;
			}

			dlg.SetFractionComplete((double)nMessage / (double)nTotalMessages);

			dwNextUpdate = GetTickCount() + dwUpdateTicks;
		}

		// Apply different CSS styles for sent vs received messages.  TBD: colorize different 'from' addresses in group messages?
		if (msg.m_Type == CSMSMessage::SMS_RECEIVED)
			pszClass = _T("messageReceived");
		else
			pszClass = _T("messageSent");

		_ftprintf(fp, _T("<div class=\"%s\">\n"),
			pszClass);

		tTime = msg.m_DateTime / 1000;

		// I think SMS conversations don't include the from-address? Maybe supply it from the CSMSThread?
		if (msg.m_Type == CSMSMessage::SMS_RECEIVED &&
			msg.m_strFromAddress.length())
		{
			wstring strFromName = msg.GetFromName();

			//if (strFromName.length() == 0 && msg.m_Type == CSMSMessage::SMS_SENT)
			//	strFromName = _T("Me");

			// If we have a name, show it; otherwise just show the address
			if (strFromName.length())
				_ftprintf(fp, _T("From: %s (%s)<br>\n"), strFromName.c_str(), msg.m_strFromAddress.c_str());
			else
				_ftprintf(fp, _T("From: %s<br>\n"), msg.m_strFromAddress.c_str());
		}
#if 0
		else
		{
			// Show sent/received status
			if (msg.m_Type == CSMSMessage::SMS_RECEIVED)
				_ftprintf(fp, _T("Received from %s (%s):<br>\n"),
					theThread.m_strContactNames.c_str(),
					theThread.m_strAddresses.c_str());
			else
				_ftprintf(fp, _T("Sent:<br>\n"));
		}
#endif

		if (localtime_s(&theTime, &tTime) == 0)
			_tcsftime(szTime, _countof(szTime), _T("%a %m/%d/%Y %I:%M:%S%p"), &theTime);
		else
			_sntprintf_s(szTime, _countof(szTime), _TRUNCATE, _T("Invalid time value: %lu"), (unsigned long)tTime);

		_ftprintf(fp, _T("%s<br>\n"), szTime);

		for (iPart = msg.m_Parts.begin(); iPart < msg.m_Parts.end(); ++iPart)
		{
			CMessagePart thePart = *iPart;

			// ADD_NEW_MMS_TYPES_HERE... all defined MMS_xxx types must have a case here!
			switch (thePart.m_PartType)
			{
			case CMessagePart::MMS_TEXT:

				if (thePart.m_strMessageData.size())
					_ftprintf(fp, _T("%s\n"),
						thePart.m_strMessageData.c_str());
				else
					_ftprintf(fp, _T("Text: [empty?!]\n"));

				break;

			case CMessagePart::MMS_GIF:
			case CMessagePart::MMS_JPEG:
			case CMessagePart::MMS_PNG:
			case CMessagePart::MMS_UNKNOWN:
			{
				// For an MMS attachment (image, etc.) we write it to a temp file and add an <a> reference to it in the HTML document
				FILE *fpMMS = NULL;
				vector<unsigned char> vData;

				vData = base64_decode(thePart.m_strMessageData);

				if (!MakeTempFileName(szMMSFile, _countof(szMMSFile), thePart.m_strName.c_str()))
				{
					strError.Format(_T("Failed creating MMS temporary filename: %s"), szMMSFile);

					AfxMessageBox(strError, MB_OK);
					return;
				}

				// Write this out in BINARY mode!
				if (_tfopen_s(&fpMMS, szMMSFile, _T("wb")) != 0)
				{
					AfxMessageBox(_T("Failed creating MMS output file!"), MB_OK);
					return;
				}

				if (fwrite(&vData[0], 1, vData.size(), fpMMS) != vData.size())
				{
					AfxMessageBox(_T("Failed writing MMS output file!"), MB_OK);
					fclose(fpMMS);
					return;
				}

				fclose(fpMMS);

				// Add temp file name to list for cleanup
				vTempFiles.push_back(wstring(szMMSFile));

				if (thePart.m_PartType == CMessagePart::MMS_UNKNOWN)
				{
					for (pszMMSFileName = _tcschr(szMMSFile, '\0'); pszMMSFileName >= szMMSFile; --pszMMSFileName)
						if (*pszMMSFileName == '\\' || *pszMMSFileName == ':')
							break;

					// Point to the first character after the colon or backslash, or else point to the start of the filename string
					++pszMMSFileName;

					// Just make a link so the user can open the file, whatever the heck it is :)
					_ftprintf(fp, _T("<a href=\"%s\">Attachment: %s</a>\n"),
						szMMSFile,
						pszMMSFileName);
				}
				else
				{
					// It's one of the image types we can preview in the browser -- write an <img> tag for the image and surround it with an <a> tag
					// to make a hyperlink out of it.  When the user clicks it, it brings up the image in an external viewer or another window,
					// depending on settings.  See CThreadView::OnBeforeNavigate2(), which is where this happens...
					_ftprintf(fp, _T("<a href=\"%s\"> <img class=\"messageImage\" src=\"file://%s\"/> </a>\n"),
						szMMSFile,
						szMMSFile);
				}

				break;
			}

			default:
#ifdef _DEBUG
				// Unhandled MMS_xxx type -- add support above!
				ASSERT(FALSE);
#endif
				break;
			}
		}

		_ftprintf(fp, _T("</div>\n"));
	}

	_ftprintf(fp, _T("</body>\n\n"));

	_ftprintf(fp, _T("</html>\n\n"));

	fclose(fp);

	// Open a CThreadDoc / CThreadView window to view the HTML file
	CThreadDoc *pThreadDoc = (CThreadDoc *) theApp.m_pThreadDocTemplate->OpenDocumentFile(szFileName, FALSE, TRUE);

	if (pThreadDoc == NULL)
		return;
	
	ASSERT(pThreadDoc->IsKindOf(RUNTIME_CLASS(CThreadDoc)));

	// Can probably get this from GetPathName(), but let's be safe since we're going to delete the file!
	// pThreadDoc->m_strThreadFileName = szFileName;
	vTempFiles.push_back(wstring(szFileName));

	pThreadDoc->m_vTempFiles = vTempFiles;
}


void CSMS_MMS_ReaderView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
{
	CScrollView::OnActivateView(bActivate, pActivateView, pDeactiveView);

	m_ctlThreadList.SetFocus();
}


void CSMS_MMS_ReaderView::OnEditFind()
{
	AfxMessageBox(_T("Find function not implemented yet, sorry!"));
}

void CSMS_MMS_ReaderView::OnViewFilterconversations()
{
	CFilterConversationsDlg dlg;

	// Initialize the dialog to the view's current values
	dlg.m_csFilterAddress = m_csFilterAddress;
	dlg.m_csFilterText = m_csFilterText;
	dlg.m_tFilterStartDate = m_tFilterStartDate;
	dlg.m_tFilterEndDate = m_tFilterEndDate;
	dlg.m_nFilterMessages = m_nFilterMessages;
	dlg.m_tDefaultStartDate = m_tDefaultStartDate;
	dlg.m_tDefaultEndDate = m_tDefaultEndDate;

	if (dlg.DoModal() != IDOK)
		return;

	// Update the view with the new values
	m_csFilterAddress = dlg.m_csFilterAddress;
	m_csFilterText = dlg.m_csFilterText;
	m_tFilterStartDate = dlg.m_tFilterStartDate.GetTime();
	m_tFilterEndDate = dlg.m_tFilterEndDate.GetTime();
	m_nFilterMessages = dlg.m_nFilterMessages;

	// We need the start date's time to be set to 00:00:00 (midnight, start of the day) and the end date's time to be 23:59:59 (end of the day)
	AdjustTime(&m_tFilterStartDate, 0, 0, 0);
	AdjustTime(&m_tFilterEndDate, 23, 59, 59);

	// Reload the list -- this does the actual filtering
	RefreshList();
}

/*
 *	ThreadMatchesFilter
 *
 *	Tests whether the given message thread (conversation) matches the user's filtering settings.
 */

bool CSMS_MMS_ReaderView::ThreadMatchesFilter(CSMSThread& theThread)
{
	// Do we have an address filter?
	if (!m_csFilterAddress.IsEmpty())
	{
		// See if our thread contains the filter name/address... if not, return false to not include this conversation
		if (theThread.m_strAddresses.find((const wchar_t *)m_csFilterAddress) == wstring::npos)
		{
			// No match on addresses... see if it matches any of the contact names...
			if (theThread.m_strContactNames.find((const wchar_t *)m_csFilterAddress) == wstring::npos)
			{
				// No dice... skip this conversation
				return false;
			}
		}
	}

	// Check the date filter against the thread's first/last message, since it's a quick check.  Note that thread date/times are in ms, but our filter is in seconds; hence the /1000 scaling.
	if ((theThread.m_NewestThreadMessage / 1000) < m_tFilterStartDate)
		return false;

	if ((theThread.m_OldestThreadMessage / 1000) > m_tFilterEndDate)
		return false;

	// Do we have a text or date/time filter?  If not, then we're done
	if (m_csFilterText.IsEmpty() &&
		m_tFilterStartDate == m_tDefaultStartDate &&
		m_tFilterEndDate == m_tDefaultEndDate)
	{
		return true;
	}

	// We have a message filter, so we have to see if any messages in this thread match the filters...
	vector<CSMSMessage>::iterator iMessage;

	int nMessage;

	for (iMessage = theThread.m_Messages.begin(), nMessage = 0;
		iMessage < theThread.m_Messages.end();
		++iMessage, ++nMessage)
	{
		CSMSMessage msg = *iMessage;

		if (MessageMatchesFilter(msg))
		{
			return true;
		}

		// Otherwise keep looking...
	}

	// No messages matched the filter, so don't include this thread in the list
	return false;
}

bool CSMS_MMS_ReaderView::MessageMatchesFilter(CSMSMessage &msg)
{
	// First check the date range filter
	if ((msg.m_DateTime / 1000) < m_tFilterStartDate)
		return false;

	if ((msg.m_DateTime / 1000) > m_tFilterEndDate)
		return false;

	const wchar_t *pwszFilterText = (const wchar_t *)m_csFilterText;

	vector<CMessagePart>::iterator iPart;

	// Walk the message parts; currently we only search MMS_TEXT parts, but we could add options to look for other attachment parts by type
	// (e.g. only show threads containing an image or a video, etc.)
	for (iPart = msg.m_Parts.begin(); iPart < msg.m_Parts.end(); ++iPart)
	{
		CMessagePart thePart = *iPart;

		// ADD_NEW_MMS_TYPES_HERE... at least for any MMS_xxx types that can be filtered on.
		switch (thePart.m_PartType)
		{
		case CMessagePart::MMS_TEXT:

			// If we have a match, return true immediately
			if (thePart.m_strMessageData.find(pwszFilterText) != wstring::npos)
				return true;

			// Otherwise keep looking...
			break;

		// TBD: add other types here if desired

		}
	}

	return false;
}

