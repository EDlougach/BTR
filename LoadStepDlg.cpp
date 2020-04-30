// LoadStepDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BTR.h"
#include "LoadStepDlg.h"

//IMPLEMENT_DYNAMIC(LoadStepDlg, CDialog)

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// LoadStepDlg dialog

LoadStepDlg::LoadStepDlg(CWnd* pParent /*=NULL*/)
	: CDialog(LoadStepDlg::IDD, pParent),
	 m_StepX(0),
	 m_StepY(0),
	 m_Xmin(0),
	 m_Xmax(0),
	 m_Ymin(0),
	 m_Ymax(0),
	 m_Zmin(0),
	 m_Zmax(0)
{
/*	//{{AFX_DATA_INIT(LoadStepDlg)
	m_StepX = 0.0;
	m_StepY = 0.0;
	m_Xmin = 0.0;
	m_Xmax = 0.0;
	m_Ymin = 0.0;	
	m_Ymax = 0.0;
	m_Zmin = 0.0;
	m_Zmax = 0.0;
	
	//}}AFX_DATA_INIT*/
	//m_Def = FALSE;
}

/*LoadStepDlg::~LoadStepDlg()
{
}*/


void LoadStepDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(LoadStepDlg)

	DDX_Text(pDX, IDC_Ymin, m_Ymin);
	DDX_Text(pDX, IDC_Ymax, m_Ymax);

	DDX_Text(pDX, IDC_Xmin, m_Xmin);
	DDX_Text(pDX, IDC_Xmax, m_Xmax);
		
	DDX_Text(pDX, IDC_Zmin, m_Zmin);
	DDX_Text(pDX, IDC_Zmax, m_Zmax);

	DDX_Text(pDX, IDC_StepX, m_StepX);
	DDX_Text(pDX, IDC_StepY, m_StepY);

	//}}AFX_DATA_MAP
	
}


BEGIN_MESSAGE_MAP(LoadStepDlg, CDialog)
	
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// LoadStepDlg message handlers

void LoadStepDlg::OnDefault() 
{
		int res = AfxMessageBox("These steps will be applied to all next AS, defined manually. ",
			MB_ICONQUESTION | MB_YESNOCANCEL);
		//if (res == IDYES) m_Def = TRUE;
		//else m_Def = FALSE;
}
