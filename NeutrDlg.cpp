// NeutrDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BTR.h"
#include "NeutrDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNeutrDlg dialog


CNeutrDlg::CNeutrDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CNeutrDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNeutrDlg)
	m_Thin = -1;
	//}}AFX_DATA_INIT
}


void CNeutrDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNeutrDlg)
	DDX_Radio(pDX, IDC_Thin, m_Thin);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNeutrDlg, CDialog)
	//{{AFX_MSG_MAP(CNeutrDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNeutrDlg message handlers
