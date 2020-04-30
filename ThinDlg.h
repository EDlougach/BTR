#if !defined(AFX_THINDLG_H__2A5D2562_DB06_46BB_A210_54E78F762E05__INCLUDED_)
#define AFX_THINDLG_H__2A5D2562_DB06_46BB_A210_54E78F762E05__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ThinDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CThinDlg dialog

class CThinDlg : public CDialog
{
// Construction
public:
	CThinDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CThinDlg)
	enum { IDD = IDD_Thin_Dlg };
	double	m_Neff;
	double	m_PosYield;
	double	m_Xlim;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CThinDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CThinDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_THINDLG_H__2A5D2562_DB06_46BB_A210_54E78F762E05__INCLUDED_)
