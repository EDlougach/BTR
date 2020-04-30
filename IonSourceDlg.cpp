// IonSourceDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BTR.h"
#include "IonSourceDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CIonSourceDlg dialog


CIonSourceDlg::CIonSourceDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CIonSourceDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CIonSourceDlg)
	m_Flat = -1;
	m_Tilt = 0.0;
	//}}AFX_DATA_INIT
}


void CIonSourceDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CIonSourceDlg)
	DDX_Radio(pDX, IDC_Flat, m_Flat);
	DDX_Text(pDX, IDC_Tilt, m_Tilt);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CIonSourceDlg, CDialog)
	//{{AFX_MSG_MAP(CIonSourceDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CIonSourceDlg message handlers
