#if !defined(AFX_NEUTRDLG_H__C2A7ADD3_535B_11D5_9A50_006097D3F37D__INCLUDED_)
#define AFX_NEUTRDLG_H__C2A7ADD3_535B_11D5_9A50_006097D3F37D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NeutrDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CNeutrDlg dialog

class CNeutrDlg : public CDialog
{
// Construction
public:
	CNeutrDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CNeutrDlg)
	enum { IDD = IDD_Neutr_Dlg };
	int		m_Thin;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNeutrDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CNeutrDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NEUTRDLG_H__C2A7ADD3_535B_11D5_9A50_006097D3F37D__INCLUDED_)
