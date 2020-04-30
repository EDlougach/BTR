#if !defined(AFX_TASKDLG_H__BA73D9D2_EC9A_4CB4_B683_DC102DA73E22__INCLUDED_)
#define AFX_TASKDLG_H__BA73D9D2_EC9A_4CB4_B683_DC102DA73E22__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TaskDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTaskDlg dialog

class CTaskDlg : public CDialog
{
// Construction
public:
	CTaskDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CTaskDlg)
	enum { IDD = IDD_Task_Dlg };
	CString	m_Task;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTaskDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTaskDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TASKDLG_H__BA73D9D2_EC9A_4CB4_B683_DC102DA73E22__INCLUDED_)
