// ProjectBar.cpp : implementation file
//

#include "stdafx.h"
#include "BTR.h"
#include "ProjectBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CProjectBar dialog


CProjectBar::CProjectBar(CWnd* pParent /*=NULL*/)
	: CDialog(CProjectBar::IDD, pParent)
{
	//{{AFX_DATA_INIT(CProjectBar)
	m_Name = _T("Name");
	//}}AFX_DATA_INIT
}


void CProjectBar::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CProjectBar)
	DDX_Text(pDX, IDC_Name, m_Name);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CProjectBar, CDialog)
	//{{AFX_MSG_MAP(CProjectBar)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProjectBar message handlers
