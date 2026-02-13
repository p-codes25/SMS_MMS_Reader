/*
 *	File reader module -- reads and parses the SMS Backup & Restore XML-format backup file.
 *
 *	This code was originally designed for a version of SMS Backup & Restore from around 2019 or so...
 *	it was then updated to work with SMS Backup & Restore 10.16.001 and now works with version 10.21.001.
 */

#include "stdafx.h"
#include "SMS_MMS_ReaderFile.h"
#include <io.h>

// Windows doesn't apparently provide this, so we do our own mapping...
#ifdef _UNICODE
	#define _tmemchr	wmemchr
	#define _tmemcpy	wmemcpy
#else
	#define _tmemchr	memchr
	#define _tmemcpy	memcpy
#endif

/*
 *	ReadMoreData
 *
 *	Reads a chunk of data from the file.  This seems to be efficient enough that I don't see the app spending much time in here.
 *	It reallocates *ppcBuf as necessary in order to hold a full XML tag (typically only MMS attachments are large enough to require
 *	growing the buffer).
 *
 *	The buffer is handled using a sliding window - new data is read in at the end, provided there's room; as data is consumed from
 *	the beginning of the buffer, *puStartPos is advanced to point to the new start of data to be read from the buffer.  When we
 *	run out of room to read new data, we shift the (typically small) amount of existing data to the start of the buffer and read
 *	more data in after it.
 */

BOOL CSMSThreadList::ReadMoreData(FILE *fp, TCHAR **ppcBuf, size_t *puBufSize, size_t *puInBuffer, size_t *puStartPos)
{
	// Additional buffer size to allocate, in TCHARs
	size_t uIncr = 1048576;

	// Number of TCHARs read
	size_t uRead;

	TCHAR *pcNew;

	// First, shift the existing data in the buffer to the front, if needed
	if (*puStartPos)
	{
		// _tmemcpy takes the count in TCHARs
		_tmemcpy(*ppcBuf, *ppcBuf + *puStartPos, *puInBuffer - *puStartPos);

		*puInBuffer -= *puStartPos;
		*puStartPos = 0;
	}

	// See if the buffer has enough room to add a chunk of new data...
	if (*puBufSize < *puInBuffer + uIncr)
	{
		// Grow the buffer (TBD: could use a geometric growth algorithm, but we don't grow it that often so this is fine for now)
		if ((pcNew = (TCHAR *)realloc(*ppcBuf, (*puBufSize + uIncr) * sizeof(TCHAR))) == NULL)
			AfxThrowMemoryException();

		*ppcBuf = pcNew;
		*puBufSize += uIncr;
	}

	// Some time is spent in here, because it's doing the UTF-8 to Windows Unicode (which is UTF-16) conversion...
	uRead = fread(*ppcBuf + *puInBuffer, sizeof(TCHAR), uIncr, fp);

	if (uRead == 0)
		return FALSE;

	*puInBuffer += uRead;

	return TRUE;
}

/*
 *	ReadFile
 *
 *	Main function for reading an SMS/MMS backup file.  This needs to be as fast as possible in order to handle large backup files,
 *	so I've done a bit of tuning on it.  On my 6th-gen Core i7 box (in release build), this code reads a backup of 500MB in about
 *	5 seconds, so that's fast to me  :-)
 */

BOOL CSMSThreadList::ReadFile(LPCTSTR pszFileName, LPTSTR pszError, int nMaxError, bool (*pfnCallback)(void * pUserData, double dFractionComplete), void *pUserData)
{
	BOOL bRetVal = TRUE;
	DWORD dwStart = GetTickCount();
	DWORD dwElapsed;
	CString cs, csTag;

	// Points to the start of the allocated buffer into which we read the backup file
	TCHAR *pcBuf = NULL;

	// Size of the allocated buffer in TCHARs
	size_t uBufSize = 0;

	// Number of TCHARs present in the buffer
	size_t uInBuffer = 0;

	// Offset in TCHARs where data starts in pcBuf - this gets incremented to 'consume' TCHARs at the start of the buffer,
	// to avoid having to shift the buffer left via memcpy()
	size_t uStartPos = 0;

	TCHAR *pcTag = NULL;
	TCHAR *pc;
	TCHAR *pcEndQuote;
	TCHAR *pcBufEnd;
	BOOL bGotIt;
	TCHAR cQuote;
	BOOL bDone = FALSE;
	CThreadMap::iterator i;

	// Interval in ms to update the progress bar
	const DWORD dwUpdateTicks = 250;

	DWORD dwNextUpdate = dwStart + dwUpdateTicks;

	FILE *fp;
	int nFd;
	__int64 llPos, llEnd;
	double dFractionComplete;

	// The backup files use UTF-8 encoding; fopen/fread will convert it to Windows Unicode (UTF-16) for us...
	if (_wfopen_s(&fp, pszFileName, L"r, ccs=UTF-8") != 0)
	{
		wnsprintf(pszError, nMaxError, _T("Failed opening input file <%s>"), pszFileName);
		return FALSE;
	}

	// Set a decent read buffer for fread()
	setvbuf(fp, NULL, _IOFBF, 65536);

	// Figure out the raw file size for our status percentage
	nFd = _fileno(fp);

	llPos = _lseeki64(nFd, 0, SEEK_CUR);
	llEnd = _lseeki64(nFd, 0, SEEK_END);
	_lseeki64(nFd, llPos, SEEK_SET);

	for (;;)
	{
		if (GetTickCount() >= dwNextUpdate)
		{
			llPos = _lseeki64(nFd, 0, SEEK_CUR);

			dFractionComplete = (double)llPos / (double)llEnd;

			dwNextUpdate = GetTickCount() + dwUpdateTicks;

			if (!pfnCallback(pUserData, dFractionComplete))
			{
				// Callback wants to cancel loading
				wnsprintf(pszError, nMaxError, _T("Reading file was cancelled by user request."));
				bRetVal = FALSE;
				break;
			}
		}

		/*
		 *	This is a very simplistic XML reader; it reads raw text and processes one tag at a time, as
		 *	delimited by <> brackets; and it builds up the conversation and message hierarchy into our
		 *	C++ class object collections.
		 */

		while (uStartPos == uInBuffer || (pcTag = _tmemchr(pcBuf + uStartPos, _T('<'), uBufSize - uStartPos)) == NULL)
		{
			// Try to read more data; if we don't read anything more, we're done
			if (!ReadMoreData(fp, &pcBuf, &uBufSize, &uInBuffer, &uStartPos))
			{
				bDone = TRUE;
				break;
			}
		}

		if (bDone)
			break;

		// Found the start... now read until we have the whole tag in our buffer
		pcBufEnd = pcBuf + uInBuffer;

		bGotIt = FALSE;

		for (pc = pcTag + 1; pc < pcBufEnd; ++pc)
		{
			if (*pc == '>')
			{
				bGotIt = TRUE;
				++pc;
				break;
			}

			// Not in quotes -- see if we're starting a quoted string
			if (*pc == '\'' || *pc == '\"')
			{
				cQuote = *pc++;

				if (pc >= pcBufEnd)
					break;

				// Do a (hopefully) fast search for the ending quote char
				pcEndQuote = _tmemchr(pc, cQuote, pcBufEnd - pc);

				if (pcEndQuote == NULL)
					break;

				// Otherwise, continue at the end-quote (the for() loop will increment past the closing quote char)
				pc = pcEndQuote;
				continue;
			}

			// Regular unquoted character... keep going
			continue;
		}

		if (!bGotIt)
		{
			// We don't have a complete XML tag... try to read more data; if we don't read anything more, we're done
			if (!ReadMoreData(fp, &pcBuf, &uBufSize, &uInBuffer, &uStartPos))
				break;

			continue;
		}

		// Got the whole tag!
		csTag = CString(pcTag, (int)(pc - pcTag));

		// Send the completed XML tag off for processing
		if (!ProcessTag(csTag))
			break;

		// Adjust uStartPos forward to 'consume' the tag we just read
		uStartPos = pc - pcBuf;

		continue;
	}

	fclose(fp);

	// Do some processing on the SMS message threads (conversations) to prepare them for the U/I
	for (i = m_Threads.begin(); i != m_Threads.end(); ++i)
	{
		CSMSThread &theThread = i->second;

		// Set the thread's total size member
		theThread.ComputeTotalSize();

		// Sort the messages in the thread, oldest to newest
		theThread.SortIt();
	}

//#ifdef _DEBUG	// This can be useful in the release build too
	// Show loading time & size for performance measurement
	dwElapsed = GetTickCount() - dwStart;

	cs.Format(L"Loading time: %lu.%03lu sec.\n", dwElapsed / 1000, dwElapsed % 1000);

	OutputDebugString(cs);

	cs.Format(L"Total size: %I64u\n", this->GetSize());
	OutputDebugString(cs);
//#endif

	free(pcBuf);

	return bRetVal;
}

/*
 *	ProcessTag
 *
 *	Processes a single XML tag.  I'm sure this does not behave completely correctly according to any type of XML specification;
 *	it was written to handle SMS Backup & Restore's XML backup format, and handle it fast so as to be able to load large files
 *	within a reasonable amount of time.  It is definitely not a general-purpose XML reader!
 */

BOOL CSMSThreadList::ProcessTag(LPCTSTR pcTag)
{
	LPCTSTR pc, pcName, pcValue, pcType;
	CString csName, csValue, csErr;
	TCHAR cQuote;
	BOOL bEndTag = FALSE;
	CMessagePart smsPart;

	ASSERT(*pcTag == _T('<'));
	++pcTag;

	// First get the type
	for (pc = pcTag; *pc && *pc != ' ' && *pc != '>'; ++pc)
		;

	m_csType = CString(pcTag, (int)(pc - pcTag));

	pcType = (LPCTSTR)m_csType;

	if (*pcType)
	{
		if (*pcType == '/')
		{
			m_csType = m_csType.Mid(1);
			bEndTag = TRUE;
		}
		else if (pcType[m_csType.GetLength() - 1] == '/')
		{
#if _DEBUG
			ASSERT(FALSE);	// this branch not tested, haven't seen it happen yet
#endif
			m_csType = m_csType.Left(m_csType.GetLength() - 1);
			bEndTag = TRUE;
		}

		// TBD: handle bEndTag here if needed, for any empty tags or closing tags
	}

	// Get the pointer again, since we might have modified m_csType above
	pcType = (LPCTSTR)m_csType;

	// Ignore comments
	if (pcType[0] == '!' && pcType[1] == '-' && pcType[2] == '-')
		return TRUE;

	// Ignore meta tags too
	if (pcType[0] == '?')
		return TRUE;

	/*
	 *	Do any actions here for the start of the tag
	 */

	if (!bEndTag)
	{
		if (!_tcscmp(pcType, _T("sms")))
		{
			// Start a new message object
			m_SMSMessage.ClearData();
		}
		else if (!_tcscmp(pcType, _T("mms")))
		{
			// Start a new message object
			m_SMSMessage.ClearData();
		}
		else if (!_tcscmp(pcType, _T("part")) || !_tcscmp(pcType, _T("address")))
		{
			// MMS part or address tag: we'll update m_SMSMessage; nothing else to do
		}
	}

	/*
	 *	Main loop: walk through the tag's properties until the end of the tag
	 */

	while (*pc)
	{
		if (isspace(*pc))
		{
			++pc;
			continue;
		}

		if (*pc == '/' && pc[1] == '>')
		{
			// End of tag, tag is closed... handle it
			if (!_tcscmp(pcType, _T("mms")))
			{
				// TBD: handle end of mms tag here
			}
			else if (!_tcscmp(pcType, _T("part")))
			{
				// TBD: need to do anything here?
			}

			// If the tag ends with /> then it's being closed
			bEndTag = true;
			break;
		}

		if (*pc == '>')
		{
			// End of tag (it may or may not be a closing tag, bEndTag indicates this)
			break;
		}

		// This should be a property name
		pcName = pc;

		while (*pc && *pc != '=')
			++pc;

		if (*pc != '=')
		{
			csErr.Format(_T("Bad character after XML property name? '%c'"), *pc);
			AfxMessageBox(csErr, MB_OK | MB_ICONSTOP);
			return FALSE;
		}

		csName = CString(pcName, (int)(pc - pcName));

		// Skip past the = char
		++pc;

		// Most properties are quoted, but handle it if they're not...
		if (*pc == '\'' || *pc == '\"')
			cQuote = *pc++;
		else
		{
			// This branch not tested
#if _DEBUG
			ASSERT(FALSE);
#endif
			cQuote = 0;
		}

		pcValue = pc;

		while (*pc)
		{
			if (cQuote)
			{
				if (*pc == cQuote)
					break;
			}
			else
			{
				if (isspace(*pc))
					break;
			}

			++pc;
		}

		csValue = CString(pcValue, (int)(pc - pcValue));

		if (cQuote)
		{
			ASSERT(*pc == cQuote);
			++pc;
		}

		// Here we have pcType and csName and csValue set
		if (!_tcscmp(pcType, _T("smses")))
		{
			// handle smses properties here
		}
		else if (!_tcscmp(pcType, _T("sms")))
		{
			// TBD: look at date_sent?
			LPCTSTR pszName = csName;

			if (!_tcscmp(pszName, _T("address")))
			{
				m_SMSMessage.m_strAddresses = (TCHAR *)(LPCTSTR)csValue;
			}
			else if (!_tcscmp(pszName, _T("contact_name")))
			{
				m_SMSMessage.m_strContactNames = csValue;
			}
			else if (!_tcscmp(pszName, _T("date")))
			{
				// Store the date/time in milliseconds since 1/1/1970
#ifdef _UNICODE
				m_SMSMessage.m_DateTime = _wtoi64(csValue);
#else
				m_SMSMessage.m_DateTime = _atoi64(csValue);
#endif
			}
			else if (!_tcscmp(pszName, _T("type")))
			{
				// Really a message status indicator more than a type...
				if (csValue == "1")
					m_SMSMessage.m_Type = CSMSMessage::SMS_RECEIVED;
				else if (csValue == "2")
					m_SMSMessage.m_Type = CSMSMessage::SMS_SENT;
				else if (csValue == "3")
					m_SMSMessage.m_Type = CSMSMessage::SMS_DRAFT;
				else if (csValue == "4")
					m_SMSMessage.m_Type = CSMSMessage::SMS_OUTBOX;
				else if (csValue == "5")
					m_SMSMessage.m_Type = CSMSMessage::SMS_FAILED;
				else if (csValue == "6")
					m_SMSMessage.m_Type = CSMSMessage::SMS_QUEUED;
				else
				{
					csErr.Format(_T("Bad SMS type: '%s'"), csValue);
					AfxMessageBox(csErr, MB_OK | MB_ICONSTOP);
					return FALSE;
				}
			}
			else if (!_tcscmp(pszName, _T("body")))
			{
				smsPart.m_PartType = CMessagePart::MMS_TEXT;
				smsPart.m_strMessageData = (LPCTSTR) csValue;

				if (m_SMSMessage.m_Parts.size() == m_SMSMessage.m_Parts.capacity())
					m_SMSMessage.m_Parts.reserve(m_SMSMessage.m_Parts.capacity() * 2);

				m_SMSMessage.m_Parts.push_back(smsPart);
			}
		}
		else if (!_tcscmp(pcType, _T("mms")))
		{
			// TBD: look at date_sent?
			LPCTSTR pszName = csName;

			if (!_tcscmp(pszName, _T("address")))
			{
				// For a group conversation, m_strAddresses is the list of addresses in the thread
				m_SMSMessage.m_strAddresses = (TCHAR *)(LPCTSTR)csValue;
			}
			else if (!_tcscmp(pszName, _T("m_type")))
			{
				if (csValue == "128")
					m_SMSMessage.m_Type = CSMSMessage::SMS_SENT;
				else if (csValue == "132")
					m_SMSMessage.m_Type = CSMSMessage::SMS_RECEIVED;
				else
				{
					// Could be 134 (read receipt?), others?
					m_SMSMessage.m_Type = CSMSMessage::SMS_DRAFT;
				}
			}
			else if (!_tcscmp(pszName, _T("contact_name")))
			{
				// For a group conversation, this is the list of contact names in the thread
				m_SMSMessage.m_strContactNames = csValue;
			}
			else if (!_tcscmp(pszName, _T("date")))
			{
				// Store the date/time in milliseconds since 1/1/1970
#ifdef _UNICODE
				m_SMSMessage.m_DateTime = _wtoi64(csValue);
#else
				m_SMSMessage.m_DateTime = _atoi64(csValue);
#endif
			}
		}
		else if (!_tcscmp(pcType, _T("part")))
		{
			// TBD: handle mms attachment part properties here
			// TBD: look at date_sent?
			LPCTSTR pszName = csName;

			if (!_tcscmp(pszName, _T("ct")))
			{
				// ADD_NEW_MMS_TYPES_HERE... or somewhere near here, at least :)
				if (csValue == "application/smil")
				{
					// we ignore this one and don't create a part for it
					return TRUE;
				}
				else if (csValue == "image/jpeg")
				{
					smsPart.m_PartType = CMessagePart::MMS_JPEG;
				}
				else if (csValue == "image/png")
				{
					smsPart.m_PartType = CMessagePart::MMS_PNG;
				}
				else if (csValue == "image/gif")
				{
					smsPart.m_PartType = CMessagePart::MMS_GIF;
				}
				else if (csValue == "text/plain")
				{
					smsPart.m_PartType = CMessagePart::MMS_TEXT;
				}
				else
				{
					// This will save a binary attachment and link to it so the user can at least save it or view it in an external viewer
					smsPart.m_PartType = CMessagePart::MMS_UNKNOWN;
				}
			}
			else if (!_tcscmp(pszName, _T("name")) ||
				!_tcscmp(pszName, _T("fn")) ||
				!_tcscmp(pszName, _T("cl")))
			{
				// Don't store a null value; just store the first one we find, out of name, fn or cl fields
				if (csValue != "null" && smsPart.m_strName.empty())
					smsPart.m_strName = (LPCTSTR)csValue;
			}
			else if (!_tcscmp(pszName, _T("data")) || !_tcscmp(pszName, _T("text")))
			{
				if (!csValue.IsEmpty() && csValue != _T("null"))
				{
					smsPart.m_strMessageData = (LPCTSTR)csValue;

					if (m_SMSMessage.m_Parts.size() == m_SMSMessage.m_Parts.capacity())
						m_SMSMessage.m_Parts.reserve(m_SMSMessage.m_Parts.capacity() * 2);

					m_SMSMessage.m_Parts.push_back(smsPart);
				}
			}
		}
		else if (!_tcscmp(pcType, _T("addr")))
		{
			// TBD: handle mms addr properties here
			LPCTSTR pszName = csName;

			if (!_tcscmp(pszName, _T("address")))
			{
				m_csAddress = csValue;
			}
			else if (!_tcscmp(pszName, _T("type")))
			{
				unsigned int uType = _tstoi(csValue);

				// This is the PDUHeader.Type value, e.g. as defined here:
				//	https://android.googlesource.com/platform/frameworks/opt/mms/+/4bfcd8501f09763c10255442c2b48fad0c796baa/src/java/com/google/android/mms/pdu/PduHeaders.java
				switch (uType)
				{
				case 0x89:	// FROM
					// Flag the 'from' address
					m_SMSMessage.m_strFromAddress = m_csAddress;
					break;

				case 0x81:	// BCC
				case 0x82:	// CC
				case 0x97:	// TO
					// Handle these if needed
					break;
				}
			}
		}
		else if (!_tcscmp(pcType, _T("calls")))
		{
			AfxMessageBox(_T("This appears to be a phone call backup, not an SMS/MMS backup.  "
				"This app can't read phone call backups, sorry."),
				MB_OK | MB_ICONSTOP);
			return FALSE;
		}
		else
		{
			// TBD: what about parts, /parts, addrs, /addrs?  In particular, look through the <addrs> list for the type 137 ('FROM') -- that's who sent the message;
			// the rest will be 151 ("TO") or other types of recipients (BCC, etc.)
			CString cs;

			cs.Format(_T("Unsupported XML tag: <%s>"), pcType);
			AfxMessageBox(cs, MB_OK | MB_ICONSTOP);
			return FALSE;
		}

		continue;
	}

	// Do any actions here for the end of the tag
	if (bEndTag)
	{
		if (!_tcscmp(pcType, _T("sms")) || !_tcscmp(pcType, _T("mms")))
		{
			AddSMSMessage(m_SMSMessage);

			m_SMSMessage.ClearData();
		}
	}

	return TRUE;
}

/*
 *	AddSMSMessage
 *
 *	Adds a new SMS/MMS message to its corresponding conversation (thread).
 */

void CSMSThreadList::AddSMSMessage(CSMSMessage &smsMessage)
{
	CThreadMap::iterator p = m_Threads.find(smsMessage.m_strAddresses);
	CSMSThread theThread, *pThread;

	if (p == m_Threads.end())
	{
		// Not in the map -- add it and initialize it
		theThread.m_strAddresses = smsMessage.m_strAddresses;

		m_Threads[smsMessage.m_strAddresses] = theThread;

		pThread = &m_Threads[smsMessage.m_strAddresses];

	}
	else
	{
		// In the map -- just update it
		pThread = &p->second;
	}

	// Use geometric allocation growth here -- saves some time in reallocating on large conversations (helps in debug build,
	// but no noticeable difference in release build).
	if (pThread->m_Messages.size() == pThread->m_Messages.capacity())
		pThread->m_Messages.reserve(pThread->m_Messages.capacity() * 2 + 4);

	pThread->m_Messages.push_back(smsMessage);

	// Keep track of the oldest & newest message timestamps for the thread
	if (smsMessage.m_DateTime < pThread->m_OldestThreadMessage)
		pThread->m_OldestThreadMessage = smsMessage.m_DateTime;

	if (smsMessage.m_DateTime > pThread->m_NewestThreadMessage)
		pThread->m_NewestThreadMessage = smsMessage.m_DateTime;

	// Keep track of the oldest & newest message timestamps loaded for this document/window
	if (smsMessage.m_DateTime < m_OldestOverallMessage)
		m_OldestOverallMessage = smsMessage.m_DateTime;

	if (smsMessage.m_DateTime > m_NewestOverallMessage)
		m_NewestOverallMessage = smsMessage.m_DateTime;

	// Update the contact names if the message has the names field; check for mismatch
	if (smsMessage.m_strContactNames.size() > 0 &&
		smsMessage.m_strContactNames.compare(wstring(_T("(Unknown)"))) != 0)
	{
		if (pThread->m_strContactNames.size() == 0)
		{
			// Contact name(s) not stored -- just set it now
			pThread->m_strContactNames = smsMessage.m_strContactNames;
		}
#ifdef _DEBUG
		else if (pThread->m_strContactNames.compare(smsMessage.m_strContactNames) != 0)
		{
			// Names don't match?  Find out what's going on...
			CString str;

			str.Format(_T("Contact names mismatch: <%s> vs <%s>!\r\n"), pThread->m_strContactNames.c_str(), smsMessage.m_strContactNames.c_str());
			OutputDebugString((LPCTSTR)str);
		}
#endif	// _DEBUG
	}
}

