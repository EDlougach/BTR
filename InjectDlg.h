#if !defined(AFX_INJECTDLG_H__A2EA816C_3A5F_4F7B_8D52_49F31248B29B__INCLUDED_)
#define AFX_INJECTDLG_H__A2EA816C_3A5F_4F7B_8D52_49F31248B29B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// InjectDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CInjectDlg dialog

class CInjectDlg : public CDialog
{
// Construction
public:
	CInjectDlg(CWnd* pParent = NULL);   // standard constructor
	CDocument * doc;
// Dialog Data
	//{{AFX_DATA(CInjectDlg)
	enum { IDD = IDD_Inject_Dlg };
	double	m_Energy1;
	double	m_Density;
	double	m_Te;
	double	m_Rmajor;
	double	m_Rminor;
	//double	m_AimR;
	double  m_Sigma0;
	double	m_Enhance;
	BOOL	m_OptEl;
	BOOL	m_OptExch;
	BOOL	m_OptIon;
	int		m_Sort;
	int     m_Rays;
	int		m_Path;
	double	m_TorCentreX;
	double  m_TorCentreY;
	double  m_TorCentreZ;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInjectDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CInjectDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedPlotdecay();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedPolfluxbtn();
	
	
	afx_msg void OnEnChangeEnhance();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INJECTDLG_H__A2EA816C_3A5F_4F7B_8D52_49F31248B29B__INCLUDED_)
