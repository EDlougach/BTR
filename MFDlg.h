#if !defined(AFX_MFDLG_H__5EC21702_DB8C_4300_8F4B_5F719A7B5017__INCLUDED_)
#define AFX_MFDLG_H__5EC21702_DB8C_4300_8F4B_5F719A7B5017__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MFDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMFDlg dialog
class C3Point;

class CMFDlg : public CDialog
{
// Construction
public:
	CMFDlg(CWnd* pParent = NULL);   // standard constructor
	CArray <C3Point, C3Point> * BArr;
	CArray <double, double> * XArr;
	void StripMFData();


// Dialog Data
	//{{AFX_DATA(CMFDlg)
	enum { IDD = IDD_MF_Dlg };
	CString	m_Bdata;
	CString	m_Filename;
	CString	m_COLUMNS;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMFDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMFDlg)
	afx_msg void OnNew();
	afx_msg void OnReadfile();
	afx_msg void OnSave();
	virtual void OnOK();
	afx_msg void OnPlot();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MFDLG_H__5EC21702_DB8C_4300_8F4B_5F719A7B5017__INCLUDED_)
