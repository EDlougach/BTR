#if !defined(AFX_IONSOURCEDLG_H__B5BDC8C3_6401_11D5_9A52_006097D3F37D__INCLUDED_)
#define AFX_IONSOURCEDLG_H__B5BDC8C3_6401_11D5_9A52_006097D3F37D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// IonSourceDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CIonSourceDlg dialog

class CIonSourceDlg : public CDialog
{
// Construction
public:
	CIonSourceDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CIonSourceDlg)
	enum { IDD = IDD_IonSource_Dlg };
	int		m_Flat;
	double	m_Tilt;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CIonSourceDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CIonSourceDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IONSOURCEDLG_H__B5BDC8C3_6401_11D5_9A52_006097D3F37D__INCLUDED_)
