#if !defined(AFX_LOADSTEPDLG_H__D3916A63_3A5B_11D5_980F_00C0CA30DB54__INCLUDED_)
#define AFX_LOADSTEPDLG_H__D3916A63_3A5B_11D5_980F_00C0CA30DB54__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LoadStepDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// LoadStepDlg dialog

class LoadStepDlg : public CDialog
{
// Construction
	//DECLARE_DYNAMIC(LoadStepDlg)
public:
	LoadStepDlg(CWnd* pParent = NULL);   // standard constructor
	//virtual ~LoadStepDlg();

// Dialog Data
	//{{AFX_DATA(LoadStepDlg)
	enum { IDD = IDD_LoadStep_Dlg };
	double	m_StepX;
	double	m_StepY;
	double  m_Xmin;
	double  m_Xmax;
	double  m_Ymin;
	double  m_Ymax;
	double  m_Zmin;
	double  m_Zmax;
	//}}AFX_DATA
	


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(LoadStepDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(LoadStepDlg)
	
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	public:
		afx_msg void OnDefault();
		//BOOL m_Def;
	
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LOADSTEPDLG_H__D3916A63_3A5B_11D5_980F_00C0CA30DB54__INCLUDED_)
