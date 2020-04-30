#if !defined(AFX_OPTVIEW_H__EF11C389_1F7A_11D5_9A4F_006097D3F37D__INCLUDED_)
#define AFX_OPTVIEW_H__EF11C389_1F7A_11D5_9A4F_006097D3F37D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COptView

class COptView : public CPropertySheet
{
	DECLARE_DYNAMIC(COptView)

// Construction
public:
	COptView(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	COptView(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COptView)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~COptView();

	// Generated message map functions
protected:
	//{{AFX_MSG(COptView)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPTVIEW_H__EF11C389_1F7A_11D5_9A4F_006097D3F37D__INCLUDED_)
