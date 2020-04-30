#if !defined(AFX_PROJECTBAR_H__720075B3_67BD_11D5_9A54_006097D3F37D__INCLUDED_)
#define AFX_PROJECTBAR_H__720075B3_67BD_11D5_9A54_006097D3F37D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ProjectBar.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CProjectBar dialog

class CProjectBar : public CDialog
{
// Construction
public:
	CProjectBar(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CProjectBar)
	enum { IDD = IDD_PROJECTBAR };
	CString	m_Name;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProjectBar)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CProjectBar)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROJECTBAR_H__720075B3_67BD_11D5_9A54_006097D3F37D__INCLUDED_)
