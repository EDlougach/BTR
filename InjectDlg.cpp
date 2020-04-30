// InjectDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BTR.h"
#include "BTRDoc.h"
#include "InjectDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CInjectDlg dialog


CInjectDlg::CInjectDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CInjectDlg::IDD, pParent),
	m_Energy1(0),
	m_Density(0),
	m_Te(0),
	m_Rmajor(0),
	m_Rminor(0),
	m_Sigma0(0),
	m_Enhance(0),
	m_OptEl(0),
	m_OptExch(0),
	m_OptIon(0),
	m_Sort(-1),
	m_TorCentreX(0),
	m_TorCentreY(0),
	m_TorCentreZ(0),
	m_Rays(0),
	m_Path(0)
	
/*
	//{{AFX_DATA_INIT(CInjectDlg)
	m_Energy1 = 0.0;
	m_Density = 0.0;
	m_Te = 0.0;
	m_Rmajor = 0.0;
	m_Rminor = 0.0;
	//m_AimR = 0.0;
	m_TorCentreX = 0.0;
	m_TorCentreY = 0.0;
	m_TorCentreZ = 0.0;
	m_Enhance = 0.0;
	m_OptEl = FALSE;
	m_OptExch = FALSE;
	m_OptIon = FALSE;
	m_Sort = -1;
	//}}AFX_DATA_INIT*/
	
{
	doc = NULL;

}


void CInjectDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CInjectDlg)
	DDX_Text(pDX, IDC_Energy1, m_Energy1);
	DDX_Text(pDX, IDC_Density, m_Density);
	DDX_Text(pDX, IDC_Te, m_Te);
	DDX_Text(pDX, IDC_Rmajor, m_Rmajor);
	DDX_Text(pDX, IDC_Rminor, m_Rminor);
	//DDX_Text(pDX, IDC_AimR, m_AimR);
	DDX_Text(pDX, IDC_SigmaBase, m_Sigma0);
	DDX_Text(pDX, IDC_Enhance, m_Enhance);
	DDX_Check(pDX, IDC_OptEl, m_OptEl);
	DDX_Check(pDX, IDC_OptExch, m_OptExch);
	DDX_Check(pDX, IDC_OptIon, m_OptIon);
	DDX_Radio(pDX, IDC_Sort, m_Sort);

	DDX_Text(pDX, IDC_TorCentreX, m_TorCentreX);
	DDX_Text(pDX, IDC_TorCentreY, m_TorCentreY);
	DDX_Text(pDX, IDC_TorCentreZ, m_TorCentreZ);
	//}}AFX_DATA_MAP
	//DDV_MinMaxDouble(pDX, m_Enhance, -1, 10);
	//DDV_MinMaxDouble(pDX, m_TorCentreZ, -10, 10);
//	DDX_Radio(pDX, IDC_Ray, m_Rays);
//	DDV_MinMaxInt(pDX, m_Rays, 0, 3);
//	DDX_Radio(pDX, IDC_Coord, m_Path);
//	DDX_Text(pDX, IDC_SigmaBase, m_Sigma0);
	//DDV_MinMaxDouble(pDX, m_Sigma0, 0, 1e10);
}


BEGIN_MESSAGE_MAP(CInjectDlg, CDialog)
	//{{AFX_MSG_MAP(CInjectDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_PlotDecay, &CInjectDlg::OnBnClickedPlotdecay)
	ON_BN_CLICKED(IDC_BUTTON3, &CInjectDlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON2, &CInjectDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_PolFluxBtn, &CInjectDlg::OnBnClickedPolfluxbtn)
	//ON_EN_CHANGE(IDC_Enhance, &CInjectDlg::OnEnChangeEnhance)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInjectDlg message handlers


void CInjectDlg::OnBnClickedPlotdecay()
{
	UpdateData(TRUE);
	CBTRDoc * pDoc = (CBTRDoc*)doc;
	//pDoc->SetPlasmaTarget();
	pDoc->CalculateBeamPlasma(); // account of beamopt(0/1/2), calcopt (path/psi)
	int bopt = pDoc->pPlasma->BeamOpt;// 0 - thin, 1 - par, 2 - focus
	int Nrays = pDoc->pPlasma->Nrays;
	
	if ( Nrays < 0) {
		AfxMessageBox("Nrays < 0\n Beam not APPLYed");
		return;
		
	}
	pDoc->PlotDecayArray(bopt); //  
	
}


void CInjectDlg::OnBnClickedButton3()// cross-sect
{
	CBTRDoc * pDoc = (CBTRDoc*)doc;
	pDoc->OnEditCrossSect();
}

void CInjectDlg::OnBnClickedButton2() // NTZ profiles
{
	CBTRDoc * pDoc = (CBTRDoc*)doc;
	pDoc->OnEditPlasmaTeNe();// all profiles
	m_Te = pDoc->MaxPlasmaTe;
	m_Density = pDoc->MaxPlasmaDensity;
	UpdateData(FALSE);
	
}


void CInjectDlg::OnBnClickedPolfluxbtn() // pol flux (PSI)
{
	CBTRDoc * pDoc = (CBTRDoc*)doc;
	pDoc->OnPlasmaPsi();
}


/*void CInjectDlg::OnEnChangeEnhance()
{
	// TODO:  Если это элемент управления RICHEDIT, то элемент управления не будет
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Добавьте код элемента управления
}*/
