// BeamletDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BTR.h"
#include "BeamletDlg.h"
#include "BTRDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// BeamletDlg dialog

BeamletDlg::BeamletDlg(CWnd* pParent /*=NULL*/)
	: CDialog(BeamletDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(BeamletDlg)
	m_Nazim = 0;
	m_Npolar = 0;
	m_BeamletCurr = 0.0;
	m_Nbeamlets = 0;
	m_AzimNStr = _T("");
	m_CoreDivHor = 0.0;
	m_CoreDivVert = 0.0;
	m_HaloDiv = 0.0;
	m_PolarNStr = _T("");
	m_HaloPart = 0.0;
	m_Common = -1;
	m_CutOffCurr = 0.0f;
	//}}AFX_DATA_INIT
}


void BeamletDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(BeamletDlg)
	DDX_Text(pDX, IDC_Azimuth, m_Nazim);
	DDX_Text(pDX, IDC_Polar, m_Npolar);
	DDX_Text(pDX, IDC_Curr, m_BeamletCurr);
	DDX_Text(pDX, IDC_Nbeamlets, m_Nbeamlets);
	DDX_Text(pDX, IDC_AzimNStr, m_AzimNStr);
	DDX_Text(pDX, IDC_CoreDivHor, m_CoreDivHor);
	DDX_Text(pDX, IDC_CoreDivVert, m_CoreDivVert);
	DDX_Text(pDX, IDC_HaloDiv, m_HaloDiv);
	DDX_Text(pDX, IDC_PolarNStr, m_PolarNStr);
	DDX_Text(pDX, IDC_HaloPart, m_HaloPart);
	DDX_Radio(pDX, IDC_Common, m_Common);
	DDX_Text(pDX, IDC_CutoffCurr, m_CutOffCurr);
	DDV_MinMaxFloat(pDX, m_CutOffCurr, 1.e-016f, 1.e-000f);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(BeamletDlg, CDialog)
	//{{AFX_MSG_MAP(BeamletDlg)
	ON_BN_CLICKED(IDC_Common, OnCommon)
	ON_BN_CLICKED(IDC_Individual, OnIndividual)
//	ON_BN_CLICKED(ID_PLOT_BEAMLETFOOT, OnPlotBeamletfoot)
//	ON_BN_CLICKED(ID_PLOT_BEAMLETCURR, OnPlotBeamletCurr)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// BeamletDlg message handlers

void BeamletDlg::OnCommon() 
{
	UpdateData(TRUE);
	SetButtons();	
	
}

void BeamletDlg::OnIndividual() 
{
	UpdateData(TRUE);
	SetButtons();	
	
}


void BeamletDlg::SetButtons()
{
	CWnd* hc1 = GetDlgItem(IDC_CoreDivHor);
	CWnd* hc2 = GetDlgItem(IDC_CoreDivVert);

	if (m_Common == 0) {
		hc1->EnableWindow(TRUE);
		hc2->EnableWindow(TRUE);
	}
	else {
		hc1->EnableWindow(FALSE);
		hc2->EnableWindow(FALSE);
	}
}

/*void BeamletDlg::OnPlotBeamletfoot() 
{
//	UpdateData(TRUE);
//	UpdateValues();
//	CBTRDoc * pDoc = (CBTRDoc*)doc;
//	pDoc->PlotBeamletFoot();
}

void BeamletDlg::OnPlotBeamletCurr() 
{
//	UpdateData(TRUE);
//	UpdateValues();
//	CBTRDoc * pDoc = (CBTRDoc*)doc;
//	pDoc->PlotBeamletCurrent();
}*/

void BeamletDlg:: UpdateValues()
{
	CBTRDoc * pDoc = (CBTRDoc*)doc;
	if ((int)pDoc->BeamSplitType ==0) { 
			pDoc->PolarNumberAtom = m_Npolar;
			pDoc->AzimNumberAtom =  m_Nazim;
		}
		else {
			pDoc->BeamSplitNumberY = m_Npolar;
			pDoc->BeamSplitNumberZ = m_Nazim;
		}
		pDoc->OptIndBeamlets = (m_Common != 0);
		pDoc->BeamCoreDivY = m_CoreDivHor * 0.001;
		pDoc->BeamCoreDivZ = m_CoreDivVert * 0.001;
		pDoc->BeamHaloDiverg = m_HaloDiv * 0.001; // mrad -> rad
		pDoc->BeamHaloPart = m_HaloPart * 0.01; // % -> nornal
		pDoc->CutOffCurrent = m_CutOffCurr;

		if (int(pDoc->BeamSplitType)==0) pDoc->SetPolarBeamletCommon(); // Polar split //SetBeamletAt(0);
		else pDoc->SetGaussBeamlet(pDoc->BeamCoreDivY, pDoc->BeamCoreDivZ); // cartesian? 
		//pDoc->SetPolarBeamletReion();
		//pDoc->SetPolarBeamletResidual();
		
}