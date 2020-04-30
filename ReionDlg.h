#if !defined(AFX_REIONDLG_H__C39C5FE4_3FFE_11D5_981A_00C0CA30DB54__INCLUDED_)
#define AFX_REIONDLG_H__C39C5FE4_3FFE_11D5_981A_00C0CA30DB54__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ReionDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CReionDlg dialog

class CReionDlg : public CDialog
{
// Construction
public:
	CReionDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CReionDlg)
	enum { IDD = IDD_Reion_Dlg };
	double	m_Percent;
	CString	m_Caption;
	double	m_Xmax;
	double	m_Xmin;
	double	m_Sigma;
	double	m_Lstep;
	double	m_IonStepL;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CReionDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CReionDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnEnChangePercent();
	afx_msg void OnEnChangeEdit2();
	afx_msg void OnEnChangeEdit3();
	afx_msg void OnStnClickedCaption();
	afx_msg void OnEnChangeEdit4();
	afx_msg void OnEnChangeEdit1();
	double m_StepSpec;
	double m_Xspec0;
	double m_Xspec1;
	double m_StepSpecR;
	double m_XRspec0;
	double m_XRspec1;
//	double m_Polar;
	int m_Polar;
	int m_Azim;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_REIONDLG_H__C39C5FE4_3FFE_11D5_981A_00C0CA30DB54__INCLUDED_)
