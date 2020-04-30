// BTR.h : main header file for the BTR application
//

#if !defined(AFX_BTR_H__EF11C377_1F7A_11D5_9A4F_006097D3F37D__INCLUDED_)
#define AFX_BTR_H__EF11C377_1F7A_11D5_9A4F_006097D3F37D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols
#include <windows.h>

extern double BTRVersion; // in BTRDoc

/////////////////////////////////////////////////////////////////////////////
// CBTRApp:
// See BTR.cpp for the implementation of this class
//

class CBTRApp : public CWinApp
{
public:
	CBTRApp();
	CString TopDirName; 

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBTRApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CBTRApp)
	afx_msg void OnAppAbout();
	afx_msg void OnHelpManual();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedButtonSite();
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BTR_H__EF11C377_1F7A_11D5_9A4F_006097D3F37D__INCLUDED_)
