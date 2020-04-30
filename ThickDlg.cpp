// ThickDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BTR.h"
#include "ThickDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CThickDlg dialog


CThickDlg::CThickDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CThickDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CThickDlg)
	m_SigmaIon = 0.0;
	m_SigmaNeutr = 0.0;
	m_Step = 0.0;
	m_Xmax = 0.0;
	m_Xmin = 0.0;
	m_Sigma2Strip = 0.0;
	m_SigmaExch = 0.0;
	//}}AFX_DATA_INIT
}


void CThickDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CThickDlg)
	DDX_Text(pDX, IDC_SigmaIon, m_SigmaIon);
	DDX_Text(pDX, IDC_SigmaNeutr, m_SigmaNeutr);
	DDX_Text(pDX, IDC_Step, m_Step);
	DDX_Text(pDX, IDC_Xmax, m_Xmax);
	DDX_Text(pDX, IDC_Xmin, m_Xmin);
	DDX_Text(pDX, IDC_Sigma2Strip, m_Sigma2Strip);
	DDX_Text(pDX, IDC_SigmaExch, m_SigmaExch);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CThickDlg, CDialog)
	//{{AFX_MSG_MAP(CThickDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CThickDlg message handlers
