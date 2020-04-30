#if !defined(AFX_VIEWSIZEDLG_H__035E6213_2745_11D5_9A4F_006097D3F37D__INCLUDED_)
#define AFX_VIEWSIZEDLG_H__035E6213_2745_11D5_9A4F_006097D3F37D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ViewSizeDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CViewSizeDlg dialog

class CViewSizeDlg : public CDialog
{
// Construction
public:
	CViewSizeDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CViewSizeDlg)
	enum { IDD = IDD_ViewSize_Dlg };
	int		m_Hor;
	int		m_Vert;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CViewSizeDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CViewSizeDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VIEWSIZEDLG_H__035E6213_2745_11D5_9A4F_006097D3F37D__INCLUDED_)
