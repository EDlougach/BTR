#if !defined(AFX_FIELDSDLG_H__D1D1112E_82D7_476A_81BE_1A84A14E80C6__INCLUDED_)
#define AFX_FIELDSDLG_H__D1D1112E_82D7_476A_81BE_1A84A14E80C6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FieldsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFieldsDlg dialog

class CFieldsDlg : public CDialog
{
// Construction
public:
	CFieldsDlg(CWnd* pParent = NULL);   // standard constructor
	CDocument * doc;

// Dialog Data
	//{{AFX_DATA(CFieldsDlg)
	enum { IDD = IDD_Fields_Dlg };
	double	m_MFcoeff;
	int		m_MFON;
	double	m_Volt;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFieldsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CFieldsDlg)
	afx_msg void OnRIDfile();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedMffile();
	afx_msg void OnBnClickedGasfile();
	double m_GasCoeff;
	int m_GasON;
	CString m_MFfilename;
	CString m_Gasfilename;
	afx_msg void OnBnClickedGasAdvbtn();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FIELDSDLG_H__D1D1112E_82D7_476A_81BE_1A84A14E80C6__INCLUDED_)
