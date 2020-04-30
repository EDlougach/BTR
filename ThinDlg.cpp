// ThinDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BTR.h"
#include "ThinDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CThinDlg dialog


CThinDlg::CThinDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CThinDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CThinDlg)
	m_Neff = 0.0;
	m_PosYield = 0.0;
	m_Xlim = 0.0;
	//}}AFX_DATA_INIT
}


void CThinDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CThinDlg)
	DDX_Text(pDX, IDC_Neff, m_Neff);
	DDX_Text(pDX, IDC_PosYield, m_PosYield);
	DDX_Text(pDX, IDC_Xlim, m_Xlim);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CThinDlg, CDialog)
	//{{AFX_MSG_MAP(CThinDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CThinDlg message handlers
