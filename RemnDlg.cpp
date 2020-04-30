// RemnDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BTR.h"
#include "RemnDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRemnDlg dialog


CRemnDlg::CRemnDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CRemnDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CRemnDlg)
	m_B1 = 0.0;
	m_B2 = 0.0;
	m_B3 = 0.0;
	m_X1 = 0.0;
	m_X2 = 0.0;
	m_X3 = 0.0;
	//}}AFX_DATA_INIT
}


void CRemnDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRemnDlg)
	DDX_Text(pDX, IDC_B1, m_B1);
	DDX_Text(pDX, IDC_B2, m_B2);
	DDX_Text(pDX, IDC_B3, m_B3);
	DDX_Text(pDX, IDC_X1, m_X1);
	DDX_Text(pDX, IDC_X2, m_X2);
	DDX_Text(pDX, IDC_X3, m_X3);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CRemnDlg, CDialog)
	//{{AFX_MSG_MAP(CRemnDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRemnDlg message handlers
