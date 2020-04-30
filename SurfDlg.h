#if !defined(AFX_SURFDLG_H__912C4197_CA1B_4CD6_854F_45215B38DFDF__INCLUDED_)
#define AFX_SURFDLG_H__912C4197_CA1B_4CD6_854F_45215B38DFDF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SurfDlg.h : header file
//
/////////////////////////////////////////////////////////////////////////////
// CSurfDlg dialog

class CSurfDlg : public CDialog
{
// Construction
public:
	CSurfDlg(CWnd* pParent = NULL);   // standard constructor
	CDocument * doc;


// Dialog Data
	//{{AFX_DATA(CSurfDlg)
	enum { IDD = IDD_Surf_Dlg };
	int		m_Solid;
	double	m_X1;
	double	m_X2;
	double	m_X3;
	double	m_X4;
	double	m_Y1;
	double	m_Y2;
	double	m_Y3;
	double	m_Y4;
	double	m_Z1;
	double	m_Z2;
	double	m_Z3;
	double	m_Z4;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSurfDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSurfDlg)
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnReadfile();
	BOOL OptRead;
	int m_N;
	CString m_Comment;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SURFDLG_H__912C4197_CA1B_4CD6_854F_45215B38DFDF__INCLUDED_)
