// ReionDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BTR.h"
#include "ReionDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CReionDlg dialog


CReionDlg::CReionDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CReionDlg::IDD, pParent)
	, m_StepSpec(0)
	, m_Xspec0(0)
	, m_Xspec1(0)
	, m_StepSpecR(0)
	, m_XRspec0(0)
	, m_XRspec1(0)
	, m_Polar(0)
	, m_Azim(0)
{
	//{{AFX_DATA_INIT(CReionDlg)
	m_Percent = 0.0;
	m_Caption = _T("");
	m_Xmax = 0.0;
	m_Xmin = 0.0;
	m_Sigma = 0.0;
	m_Lstep = 0.0;
	m_IonStepL = 0.0;
	//}}AFX_DATA_INIT
}


void CReionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CReionDlg)
	DDX_Text(pDX, IDC_Percent, m_Percent);
	DDX_Text(pDX, IDC_CAPTION, m_Caption);
	DDX_Text(pDX, IDC_Xmax, m_Xmax);
	DDX_Text(pDX, IDC_Xmin, m_Xmin);
	DDX_Text(pDX, IDC_Sigma, m_Sigma);
	DDX_Text(pDX, IDC_Lstep, m_Lstep);
	DDX_Text(pDX, IDC_IonStepL, m_IonStepL);
	//}}AFX_DATA_MAP
	DDX_Text(pDX, IDC_StepSpec, m_StepSpec);
	DDX_Text(pDX, IDC_Xspec0, m_Xspec0);
	DDX_Text(pDX, IDC_Xspec1, m_Xspec1);
	DDX_Text(pDX, IDC_StepSpecR, m_StepSpecR);
	DDV_MinMaxDouble(pDX, m_StepSpecR, 0, 1000);
	DDX_Text(pDX, IDC_XRspec0, m_XRspec0);
	DDV_MinMaxDouble(pDX, m_XRspec0, 0, 1000);
	DDX_Text(pDX, IDC_XRspec1, m_XRspec1);
	DDV_MinMaxDouble(pDX, m_XRspec1, 0, 1000);
	//  DDX_Text(pDX, IDC_Polar, m_Polar);
	DDX_Text(pDX, IDC_Polar, m_Polar);
	DDX_Text(pDX, IDC_Azim, m_Azim);
}


BEGIN_MESSAGE_MAP(CReionDlg, CDialog)
	//{{AFX_MSG_MAP(CReionDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
	/*ON_EN_CHANGE(IDC_Percent, &CReionDlg::OnEnChangePercent)
	ON_EN_CHANGE(IDC_EDIT2, &CReionDlg::OnEnChangeEdit2)
	ON_EN_CHANGE(IDC_EDIT3, &CReionDlg::OnEnChangeEdit3)
	ON_STN_CLICKED(IDC_CAPTION, &CReionDlg::OnStnClickedCaption)
	ON_EN_CHANGE(IDC_EDIT4, &CReionDlg::OnEnChangeEdit4)
	ON_EN_CHANGE(IDC_EDIT1, &CReionDlg::OnEnChangeEdit1)*/
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CReionDlg message handlers

void CReionDlg::OnEnChangePercent()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}

void CReionDlg::OnEnChangeEdit2()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}

void CReionDlg::OnEnChangeEdit3()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}

void CReionDlg::OnStnClickedCaption()
{
	// TODO: Add your control notification handler code here
}

void CReionDlg::OnEnChangeEdit4()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}

void CReionDlg::OnEnChangeEdit1()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}
