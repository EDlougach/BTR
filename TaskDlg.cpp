// TaskDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BTR.h"
#include "TaskDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTaskDlg dialog


CTaskDlg::CTaskDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTaskDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTaskDlg)
	m_Task = _T("");
	//}}AFX_DATA_INIT
}


void CTaskDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTaskDlg)
	DDX_Text(pDX, IDC_TASK, m_Task);
	DDV_MaxChars(pDX, m_Task, 1024);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTaskDlg, CDialog)
	//{{AFX_MSG_MAP(CTaskDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTaskDlg message handlers
