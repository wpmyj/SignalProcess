// FileSystem.cpp: implementation of the CFileSystem class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FileSystem.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
static int CALLBACK BrowseDirectoryCallback(
				HWND hWnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	// Context was pointer to CFolderDialog instance
	CFolderDialog* pFd = (CFolderDialog*)lpData;

	// Let the class handle it
	pFd->CallbackFunction(hWnd, uMsg, lParam);

	// Always return 0 as per SHBrowseFolder spec
	return 0;
}

CFolderDialog::CFolderDialog(LPCTSTR title,
							CWnd* pParentWnd,
							LPCTSTR lpszInitialFolder,
							DWORD dwFlags)
{
	// Use the supplied initial folder if non-null.
	if (lpszInitialFolder == NULL) m_strInitialFolderName = _T("");
	else m_strInitialFolderName = lpszInitialFolder;

	memset(&m_bi, 0, sizeof(BROWSEINFO));

	if (pParentWnd == NULL) m_bi.hwndOwner = 0;
	else m_bi.hwndOwner = pParentWnd->GetSafeHwnd();

	// Fill in the rest of the structure
	m_bi.pidlRoot = NULL;
	//m_bi.pszDisplayName = m_szDisplayName;
	m_bi.lpszTitle = title;
	m_bi.ulFlags = dwFlags | BIF_STATUSTEXT;
	m_bi.lpfn = BrowseDirectoryCallback;
	m_bi.lParam = (LPARAM)this;

}

CFolderDialog::~CFolderDialog()
{

}

void CFolderDialog::CallbackFunction(HWND hWnd, UINT uMsg,	LPARAM lParam)
{
	// Save the window handle. The Set* functions need it and they may
	//	be called by the virtual funtions.
	m_hDialogBox = hWnd;

	// Dispatch the two message types to the virtual functions
	switch (uMsg)
	{
	case BFFM_INITIALIZED:
		OnInitDialog();
		break;
	case BFFM_SELCHANGED:
		OnSelChanged((ITEMIDLIST*) lParam);
		break;
	}
}

int CFolderDialog::DoModal()
{
	int nReturn = IDOK;

	// initialize the result to the starting folder value
	m_strFinalFolderName = m_strInitialFolderName;

	ITEMIDLIST* piid = NULL;

	// call the shell function
	piid = ::SHBrowseForFolder(&m_bi);

	// process the result
	if (piid && ::SHGetPathFromIDList(piid, m_szPath))
	{
		m_strFinalFolderName = m_szPath;
		nReturn = IDOK;
	}
	else
	{
		nReturn = IDCANCEL;
	}

	// Release the ITEMIDLIST if we got one
	if (piid)
	{
		LPMALLOC lpMalloc;
		VERIFY(::SHGetMalloc(&lpMalloc) == NOERROR);
		lpMalloc->Free(piid);
		lpMalloc->Release();
	}

	return nReturn;
}

CString CFolderDialog::GetPathName() const
{
	return m_strFinalFolderName;
}

void CFolderDialog::EnableOK(BOOL bEnable)
{
	// Documentation is incorrect! It is the lParam, not the wParam, that
	//	controls the enable!
	//::SendMessage(m_hDialogBox, BFFM_ENABLEOK, (bEnable ? 1 : 0), 0);
	::SendMessage(m_hDialogBox, BFFM_ENABLEOK, 0, (bEnable ? 1 : 0));
}

void CFolderDialog::SetSelection(LPCTSTR pszSelection)
{
	::SendMessage(m_hDialogBox, BFFM_SETSELECTION, TRUE, (LPARAM) pszSelection);
}

void CFolderDialog::SetSelection(ITEMIDLIST* pIdl)
{
	::SendMessage(m_hDialogBox, BFFM_SETSELECTION, FALSE, (LPARAM) pIdl);
}

void CFolderDialog::SetStatusText(LPCTSTR pszStatusText)
{
	::SendMessage(m_hDialogBox, BFFM_SETSTATUSTEXT, 0, (LPARAM) pszStatusText);
}

CString CFolderDialog::ShortName(const CString& strName)
{
	CString strShort;
	if (strName.GetLength() <= 35)
		strShort = strName;
	else
		strShort = strName.Left(35) + _T("...");

	return strShort;
}

void CFolderDialog::OnInitDialog()
{
	// Default handing of the init dialog message sets the selection to 
	//	the initial folder
	SetSelection(m_strInitialFolderName);
	SetStatusText(ShortName(m_strInitialFolderName));
}

void CFolderDialog::OnSelChanged(ITEMIDLIST* pIdl)
{
	::SHGetPathFromIDList(pIdl, m_szPath);
	m_strFinalFolderName = m_szPath;
	SetStatusText(ShortName(m_strFinalFolderName));
}
