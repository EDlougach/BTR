// ViewSizeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BTR.h"
#include "ViewSizeDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CViewSizeDlg dialog


CViewSizeDlg::CViewSizeDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CViewSizeDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CViewSizeDlg)
	m_Hor = 0;
	m_Vert = 0;
	//}}AFX_DATA_INIT
}


void CViewSizeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CViewSizeDlg)
	DDX_Text(pDX, IDC_Hor, m_Hor);
	DDV_MinMaxInt(pDX, m_Hor, 100, 10000);
	DDX_Text(pDX, IDC_Vert, m_Vert);
	DDV_MinMaxInt(pDX, m_Vert, 100, 10000);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CViewSizeDlg, CDialog)
	//{{AFX_MSG_MAP(CViewSizeDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CViewSizeDlg message handlers
