/*
 *	SMS_MMS_ReaderFile.h - defines classes used to store message parts, messages, message conversations, and the file reader class.
 */

#pragma once

#include <list>
#include <map>
#include <set>
#include <vector>
#include <string>
#include <algorithm>

using namespace std;

/*
 *	CMessagePart
 *
 *	Contains and describes a single part of an SMS or MMS message, which may contain text, or a picture, etc.
 */

class CMessagePart
{
public:
	CMessagePart()
	{
		m_PartType = MMS_UNKNOWN;
	}

	// ADD_NEW_MMS_TYPES_HERE
	enum
	{
		MMS_UNKNOWN,
		MMS_TEXT,
		MMS_JPEG,
		MMS_PNG,
		MMS_GIF
	} m_PartType;

	// Name assigned to the part via the 'name' attibute
	wstring m_strName;

	// Temp filename used for HTML page
	wstring m_strHTMLFile;

	// This is the 'body' (SMS) or 'data' (MMS) value; for MMS it seems always to be base64-encoded?
	wstring m_strMessageData;

	// Storage size of this message, for finding which conversations take up the most space, etc.
	UINT64 GetSize() { return m_strMessageData.size();  }
};

/*
 *	CSMSMessage
 *
 *	Describes a single SMS or MMS message (both types are handled by this class).  MMS messages can contain multiple parts of varying types;
 *	SMS messages contain only a single MMS_TEXT part.  But otherwise all message types are handled more or less the same.
 */

class CSMSMessage
{
public:
	CSMSMessage()
	{
		ClearData();
	}

	void ClearData()
	{
		m_Type = SMS_INVALID;
		m_strAddresses.clear();
		m_strFromAddress.clear();
		m_strContactNames.clear();
		m_DateTime = 0;

		m_Parts.clear();
	}

	// More of a status than a type, but it's called "type" in the XML file...
	enum { SMS_INVALID, SMS_RECEIVED, SMS_SENT, SMS_DRAFT, SMS_OUTBOX, SMS_FAILED, SMS_QUEUED } m_Type;

	// List of addresses (phone numbers, or can be email addresses or maybe other types?) this message is associated with;
	// this identifies the message conversation this message belongs to
	wstring	m_strAddresses;

	// "From" address for group conversation messages
	wstring	m_strFromAddress;

	wstring m_strContactNames;

	// This stores the date/time of the message, in milliseconds since 1/1/1970
	__int64	m_DateTime;

	// List of message parts in this message
	vector<CMessagePart> m_Parts;

	/*
	 *	GetFromName() extracts the 'from' contact name for this message, by looking m_strFromAddress in the list of addresses in m_strAddresses,
	 *	and then looking up the contact name from m_strContactNames that matches the matching address index.
	 *
	 *	HOWEVER... in at least one case I saw, the list of addresses did *not* seem to match the list of contact names correctly - there was one
	 *	address in the middle that did not have a contact name, but there was no extra separator in the list of contact names to indicate this...
	 *	so this may *not* always report the correct contact name until such time as a fix is identified...
	 */

	wstring GetFromName()
	{
		size_t uAddressesIndex, uContactNamesIndex, uNextAddr, uNextContact;
		wstring strAddress, strContactName;

		if (m_strFromAddress.length() == 0)
			return _T("");

		for (uAddressesIndex = uContactNamesIndex = 0;
			uAddressesIndex < m_strAddresses.length() && uContactNamesIndex < m_strContactNames.length();
			)
		{
			uNextAddr = m_strAddresses.find_first_of('~', uAddressesIndex);
			uNextContact = m_strContactNames.find_first_of(',', uContactNamesIndex);

			if (uNextAddr == std::string::npos)
				strAddress = m_strAddresses.substr(uAddressesIndex);
			else
				strAddress = m_strAddresses.substr(uAddressesIndex, uNextAddr - uAddressesIndex);

			if (uNextContact == std::string::npos)
				strContactName = m_strContactNames.substr(uContactNamesIndex);
			else
				strContactName = m_strContactNames.substr(uContactNamesIndex, uNextContact - uContactNamesIndex);

			// Strip white space
			strAddress.erase(0, strAddress.find_first_not_of(' '));
			strAddress.erase(strAddress.find_last_not_of(' ') + 1);

			if (strAddress == m_strFromAddress)
			{
				strContactName.erase(0, strContactName.find_first_not_of(' '));
				strContactName.erase(strContactName.find_last_not_of(' ') + 1);

				return strContactName;
			}

			if (uNextAddr == std::string::npos || uNextContact == std::string::npos)
				break;

			uAddressesIndex = uNextAddr + 1;
			uContactNamesIndex = uNextContact + 1;
		}

		return _T("");
	}

	// Returns the total storage size of this message, as the sum of its message parts
	UINT64 GetSize()
	{
		UINT64 ldwSize = m_strAddresses.size();

		vector<CMessagePart>::iterator i;

		for (i = m_Parts.begin(); i != m_Parts.end(); ++i)
			ldwSize += i->GetSize();

		return ldwSize;
	}
};

/*
 *	CSMSThread
 *
 *	Container for a complete SMS/MMS conversation, with the set of messages, to/from a single set of addresses.  I called it
 *	a "thread" because it was easier to type :) but it's not to be confused with any multithreading features, of which this
 *	program has none!
 */

class CSMSThread
{
public:
	CSMSThread()
	{
		m_OldestThreadMessage = INT64_MAX;
		m_NewestThreadMessage = 0;
	}

	// List of addresses (phone numbers typically, but can be email addresses or possibly other types) which identify this conversation
	wstring m_strAddresses;

	// List of contact names - one per address, but see the note above GetFromName() above for an important caveat...
	wstring m_strContactNames;

	// Contains the list of messages in this conversation
	vector<CSMSMessage> m_Messages;

	// These store the date/time in milliseconds since 1/1/1970, of the oldest & most recent message in the thread
	__int64	m_OldestThreadMessage;
	__int64	m_NewestThreadMessage;

	// Total storage size of messages in this conversation
	__int64 m_ldwTotalSize;

	void SortIt()
	{
		sort(m_Messages.begin(), m_Messages.end(), [](CSMSMessage& a, CSMSMessage& b) { return a.m_DateTime < b.m_DateTime; } );
	}

	void ComputeTotalSize()
	{
		UINT64 ldwSize = m_strAddresses.size();
		vector<CSMSMessage>::iterator i;

		for (i = m_Messages.begin(); i != m_Messages.end(); ++i)
			ldwSize += i->GetSize();

		m_ldwTotalSize = ldwSize;
	}
};

/*
 *	CThreadMap
 *
 *	Provides quick lookup of a CSMSThread; the map key is an address list (as stored in CSMSMessage::m_strAddresses or CSMSThread::m_strAddresses)
 *	Thus, we organize messages by the full set of addresses which each message was received from or sent to.
 */

typedef map<wstring, CSMSThread> CThreadMap;

/*
 *	CSMSThreadList
 *
 *	This serves as the container for all the SMS/MMS threads loaded into a window; and also contains all the
 *	file reading code which populates the container.
 */

class CSMSThreadList
{
public:

	CSMSThreadList()
	{
		m_OldestOverallMessage = INT64_MAX;
		m_NewestOverallMessage = 0;
	}

	// Container for all message conversations loaded into a given application window
	CThreadMap m_Threads;

	BOOL ReadMoreData(FILE *fp, TCHAR **ppcBuf, size_t *puBufSize, size_t *puInBuffer, size_t *puStartPos);
	BOOL ReadFile(LPCTSTR pszFileName, LPTSTR pszError, int nMaxError, bool(*pfnCallback)(void *pUserData, double dFractionComplete), void *pUserData);

	__int64	m_OldestOverallMessage;
	__int64	m_NewestOverallMessage;

	UINT64 GetSize()
	{
		CThreadMap::iterator i;
		UINT64 ldwSize = 0;

		for (i = m_Threads.begin(); i != m_Threads.end(); ++i)
			ldwSize += i->second.m_ldwTotalSize;

		return ldwSize;
	}

protected:
	BOOL ProcessTag(LPCTSTR pcTag);
	void AddSMSMessage(CSMSMessage &smsMessage);

	// This is a temporary object which holds info on the current message being parsed by the reader, as it is built up across multiple tags
	CSMSMessage m_SMSMessage;

	// Working strings for ProcessTag()
	CString m_csType;
	CString m_csAddress;
};

