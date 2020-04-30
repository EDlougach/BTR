#if !defined(AFX_NBICONFDLG_H__C08C0D4B_C73F_4968_8C47_3D3370A33143__INCLUDED_)
#define AFX_NBICONFDLG_H__C08C0D4B_C73F_4968_8C47_3D3370A33143__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NBIconfDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CNBIconfDlg dialog

class CNBIconfDlg : public CDialog
{
// Construction
public:
	CNBIconfDlg(CWnd* pParent = NULL);   // standard constructor
	CDocument * doc;


// Dialog Data
	//{{AFX_DATA(CNBIconfDlg)
	enum { IDD = IDD_NBI_Dlg };
	int		m_CalOpen;
	int		m_NeutrChannels;
	int		m_RIDChannels;
	double	m_NeutrGapIn;
	double	m_NeutrGapOut;
	double	m_NeutrThickIn;
	double	m_NeutrThickOut;
	double	m_RIDGapIn;
	double	m_RIDGapOut;
	double	m_RIDThick;
	double	m_Volt;
	BOOL	m_CalculWidth;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNBIconfDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CNBIconfDlg)
	afx_msg void OnCalculWidth();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	BOOL m_AddDuct;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NBICONFDLG_H__C08C0D4B_C73F_4968_8C47_3D3370A33143__INCLUDED_)
