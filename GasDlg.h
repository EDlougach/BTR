#if !defined(AFX_GASDLG_H__144C39C6_B348_40CA_835C_4946529D3741__INCLUDED_)
#define AFX_GASDLG_H__144C39C6_B348_40CA_835C_4946529D3741__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GasDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CGasDlg dialog
class C3Point;

class CGasDlg : public CDialog
{
// Construction
public:
	CGasDlg(CWnd* pParent = NULL);   // standard constructor
	CArray <C3Point, C3Point> * Arr;
	void StripData();

// Dialog Data
	//{{AFX_DATA(CGasDlg)
	enum { IDD = IDD_Gas_Dlg };
	CString	m_Filename;
	CString	m_Y; //2nd column data
	CString	m_X; //1st column data
	CString m_Caption;
	CString m_Head1, m_Head2; // Column headers
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGasDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CGasDlg)
	afx_msg void OnReadfile();
	afx_msg void OnNew();
	afx_msg void OnSave();
	virtual void OnOK();
	afx_msg void OnPlot();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GASDLG_H__144C39C6_B348_40CA_835C_4946529D3741__INCLUDED_)
