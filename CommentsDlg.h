#if !defined(AFX_COMMENTSDLG_H__0E8764F8_4030_4230_B538_F7B0D4DCF9DD__INCLUDED_)
#define AFX_COMMENTSDLG_H__0E8764F8_4030_4230_B538_F7B0D4DCF9DD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CommentsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCommentsDlg dialog

class CCommentsDlg : public CDialog
{
// Construction
public:
	CCommentsDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CCommentsDlg)
	enum { IDD = IDD_CommentsDlg };
	CString	m_Comment;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCommentsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCommentsDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COMMENTSDLG_H__0E8764F8_4030_4230_B538_F7B0D4DCF9DD__INCLUDED_)
