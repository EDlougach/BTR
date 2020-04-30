#if !defined(AFX_PREVIEWDLG_H__E5ED3A39_2B49_4B0B_9236_25DCC6055233__INCLUDED_)
#define AFX_PREVIEWDLG_H__E5ED3A39_2B49_4B0B_9236_25DCC6055233__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PreviewDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPreviewDlg dialog

class CPreviewDlg : public CDialog
{
// Construction
public:
	CPreviewDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPreviewDlg)
	enum { IDD = IDD_PREVIEW_Dlg };
	CString	m_Filename;
	CString	m_Text;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPreviewDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPreviewDlg)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PREVIEWDLG_H__E5ED3A39_2B49_4B0B_9236_25DCC6055233__INCLUDED_)
