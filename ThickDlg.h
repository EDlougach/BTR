#if !defined(AFX_THICKDLG_H__BC974A5D_C29F_47D7_9A38_B2CE7B00ED6F__INCLUDED_)
#define AFX_THICKDLG_H__BC974A5D_C29F_47D7_9A38_B2CE7B00ED6F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ThickDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CThickDlg dialog

class CThickDlg : public CDialog
{
// Construction
public:
	CThickDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CThickDlg)
	enum { IDD = IDD_Thick_Dlg };
	double	m_SigmaIon;
	double	m_SigmaNeutr;
	double	m_Step;
	double	m_Xmax;
	double	m_Xmin;
	double	m_Sigma2Strip;
	double	m_SigmaExch;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CThickDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CThickDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_THICKDLG_H__BC974A5D_C29F_47D7_9A38_B2CE7B00ED6F__INCLUDED_)
