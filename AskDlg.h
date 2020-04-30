#if !defined(AFX_ASKDLG_H__DEFC146E_F2E8_40C5_8DC4_574ECBFDD52B__INCLUDED_)
#define AFX_ASKDLG_H__DEFC146E_F2E8_40C5_8DC4_574ECBFDD52B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AskDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAskDlg dialog

class CAskDlg : public CDialog
{
// Construction
public:
	BOOL Create();
	CAskDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAskDlg)
	enum { IDD = IDD_ASK_Dlg };
	CListBox	m_ListQuest;
	CString	m_Answer;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAskDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAskDlg)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeLISTQuest();
	virtual void OnCancel();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void SetAnswer();
	void FillAnsList();
	void FillQuestList();
	CArray <CString, CString> AnsArr;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ASKDLG_H__DEFC146E_F2E8_40C5_8DC4_574ECBFDD52B__INCLUDED_)
