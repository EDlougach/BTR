#if !defined(AFX_REMNDLG_H__C39C5FE3_3FFE_11D5_981A_00C0CA30DB54__INCLUDED_)
#define AFX_REMNDLG_H__C39C5FE3_3FFE_11D5_981A_00C0CA30DB54__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RemnDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CRemnDlg dialog

class CRemnDlg : public CDialog
{
// Construction
public:
	CRemnDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CRemnDlg)
	enum { IDD = IDD_Remn_Dlg };
	double	m_B1;
	double	m_B2;
	double	m_B3;
	double	m_X1;
	double	m_X2;
	double	m_X3;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRemnDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CRemnDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_REMNDLG_H__C39C5FE3_3FFE_11D5_981A_00C0CA30DB54__INCLUDED_)
