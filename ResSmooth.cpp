// ResSmooth.cpp : implementation file
//

#include "stdafx.h"
#include "BTR.h"
#include "ResSmooth.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CResSmooth dialog


CResSmooth::CResSmooth(CWnd* pParent /*=NULL*/)
	: CDialog(CResSmooth::IDD, pParent)
{
	//{{AFX_DATA_INIT(CResSmooth)
	m_Degree = 0;
	//}}AFX_DATA_INIT
}


void CResSmooth::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CResSmooth)
	DDX_Text(pDX, IDC_Degree, m_Degree);
	DDV_MinMaxInt(pDX, m_Degree, 0, 100);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CResSmooth, CDialog)
	//{{AFX_MSG_MAP(CResSmooth)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CResSmooth message handlers
