# SMS_MMS_Reader

## Overview

This is a Windows application which loads and displays backup files created by the [SMS Backup & Restore app for Android, by SyncTech](https://www.synctech.com.au/).  I have no affiliation with them - I just wrote this program using their [documented backup file format](https://www.synctech.com.au/sms-backup-restore/fields-in-xml-backup-files/).

SMS Backup & Restore is an awesome app for backing up SMS/MMS messages and phone call logs from an Android phone.  It writes the backups to an XML-formatted file which can be transferred to a Windows PC and opened and viewed using the SMS_MMS_Reader app.

## Features

The SMS_MMS_Reader app offers the following capabilities:

* It's built as a Windows MFC app for x86 (32-bit) or x64 (64-bit) systems running Windows 7 or later (possibly older versions too).

* It's fairly fast -- a release build can load a 1GB SMS/MMS backup file in around 10 seconds on a reasonably fast system.

* You can drag-and-drop to load multiple backup files into the viewer, into a single window or into separate windows.

* You can view the list of conversations, with the name(s) and phone number(s) of each, along with the number of messages and total storage size used in total by each conversation.

* You can view and sort the list of message conversations (threads) in each window.

* You can select a conversation (message thread) and double-click or press Enter to view that conversation's messages in a separate window.

* The conversation view displays images inline as thumbnails, and you can click to view an image full-size (or click to play an audio or video attachment, or open/save other attachment types using an external viewer/player/etc.)

* You can filter the conversation list by address or contact name, by date range and by message text.

## Known Problems / Limitations

* The conversation view is done by generating simple HTML, which then gets displayed by the MFC CHtmlView class, which in turn uses the Windows WebBrowser control, so it's more or less like an old IE (Internet Explorer) control.  The conversation display looks pretty basic as a result.  The look might be improved by adding some more CSS tricks or even writing a custom display engine.

* I'm not sure the contact names are correctly matched up with contact addresses (phone numbers) in all cases.  See comments above GetFromName() in SMS_MMS_ReaderFile.h.  I'm not sure if this is a problem with that code or with the XML file format produced by SMS Backup & Restore.

## Source Code

I built the project using Visual Studio 2015, but it should build and run in any later Visual Studio version as well.  There are x86 (32-bit) and x64 (64-bit) Windows targets, and I've tested both in Debug and Release mode.

Release mode is significantly faster, so you will probably want to use that for loading large backup files.
