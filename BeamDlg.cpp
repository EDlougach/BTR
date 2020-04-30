// BeamDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BTR.h"
#include <math.h>

#include "BTRDoc.h"
#include "BeamDlg.h"
#include "ReionDlg.h"
#include "BeamletDlg.h"
#include "ParticlesDlg.h"
#include "ThickDlg.h"
#include "ThinDlg.h"
//#include "InjectDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBeamDlg dialog


CBeamDlg::CBeamDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CBeamDlg::IDD, pParent)
	, m_Reflect(FALSE)
	, m_AtomPower(FALSE)
	, m_NegIon_Power(FALSE)
	, m_PosIon_Power(FALSE)
{
	//{{AFX_DATA_INIT(CBeamDlg)
	m_D = -1;
	m_OptPlasma = FALSE;
	m_OptReion = FALSE;
	m_OptRID = FALSE;
	m_OptSINGAP = -1;
	m_OptThick = FALSE;
	m_Gauss = -1;
	m_Atoms = FALSE;
	m_IonPower = FALSE;
	//}}AFX_DATA_INIT
	doc = NULL;

}


void CBeamDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBeamDlg)
	DDX_Control(pDX, IDC_Beamlet, m_Beamlet);
	DDX_Control(pDX, IDC_Particles, m_Particles);
	DDX_Control(pDX, IDC_Reionisation, m_Reionisation);
	DDX_Control(pDX, IDC_Neutr, m_Neutralisation);
	DDX_Control(pDX, IDC_BeamPlasma, m_BeamPlasma);
	DDX_Radio(pDX, IDC_D, m_D);
	DDX_Check(pDX, IDC_Plasma, m_OptPlasma);
	DDX_Check(pDX, IDC_Reion, m_OptReion);
	DDX_Check(pDX, IDC_RID, m_OptRID);
	DDX_Radio(pDX, IDC_SINGAP, m_OptSINGAP);
	DDX_Check(pDX, IDC_Thick, m_OptThick);
	DDX_Radio(pDX, IDC_Gauss, m_Gauss);
	DDX_Check(pDX, IDC_ATOMS, m_Atoms);
	DDX_Check(pDX, IDC_IONPOWER, m_IonPower);
	//}}AFX_DATA_MAP
	DDX_Check(pDX, IDC_Reflect, m_Reflect);
	DDX_Check(pDX, IDC_Atom_Power, m_AtomPower);
	DDX_Check(pDX, IDC_NegIon_Power, m_NegIon_Power);
	DDX_Check(pDX, IDC_PosIons_Power, m_PosIon_Power);
}


BEGIN_MESSAGE_MAP(CBeamDlg, CDialog)
	//{{AFX_MSG_MAP(CBeamDlg)
	ON_BN_CLICKED(IDC_Neutr, OnNeutralisation)
	ON_BN_CLICKED(IDC_BeamPlasma, OnBeamPlasma)
	ON_BN_CLICKED(IDC_Reionisation, OnReionisation)
	ON_BN_CLICKED(IDC_Beamlet, OnBeamlet)
	ON_BN_CLICKED(IDC_Particles, OnParticles)
	ON_BN_CLICKED(IDC_Reion, OnOptReion)
	ON_BN_CLICKED(IDC_E, OnE)
	ON_BN_CLICKED(IDC_D, OnOptD)
	ON_BN_CLICKED(IDC_H, OnOptH)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_Thick, OnOptThickNeutr)
	ON_BN_CLICKED(IDC_Plasma, OnPlasma)
	ON_BN_CLICKED(IDC_File, OnFile)
	ON_BN_CLICKED(IDC_IONPOWER, OnIonPower)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBeamDlg message handlers

void CBeamDlg::OnNeutralisation() 
{
	UpdateData(TRUE);
	CBTRDoc * pDoc = (CBTRDoc*)doc;
	pDoc->SetNeutralisation();
	/*if (!m_OptThick) {
		CThinDlg dlg;
		dlg.m_Xlim = *pDoc->NeutrXmax;
		dlg.m_Neff = *pDoc->NeutrPart * 100;
		dlg.m_PosYield = *pDoc->PosIonPart * 100;
		if (dlg.DoModal() == IDOK) {
			*pDoc->NeutrXmax = dlg.m_Xlim;
			*pDoc->NeutrPart = dlg.m_Neff * 0.01;
			*pDoc->PosIonPart = dlg.m_PosYield * 0.01;
		}
	}

	else {
		CThickDlg dlg;
		dlg.m_Xmin = *pDoc->NeutrXmin;
		dlg.m_Xmax = *pDoc->NeutrXmax;
		dlg.m_Step = *pDoc->NeutrStepL;
		dlg.m_SigmaNeutr = *pDoc->NeutrSigma * 10000; // -> cm2
		dlg.m_SigmaIon = *pDoc->ReionSigma * 10000;
		dlg.m_Sigma2Strip = *pDoc->TwoStripSigma * 10000;
		dlg.m_SigmaExch = *pDoc->PosExchSigma * 10000;
		if (dlg.DoModal() == IDOK) {
			*pDoc->NeutrXmin = dlg.m_Xmin;
			*pDoc->NeutrXmax = dlg.m_Xmax;
			*pDoc->NeutrStepL = dlg.m_Step;
			*pDoc->NeutrSigma = dlg.m_SigmaNeutr * 0.0001; // -> m2
			*pDoc->ReionSigma = dlg.m_SigmaIon * 0.0001;
			*pDoc->TwoStripSigma = dlg.m_Sigma2Strip * 0.0001;
			*pDoc->PosExchSigma = dlg.m_SigmaExch * 0.0001;
		}
	}

	*/
}

void CBeamDlg::OnBeamPlasma() 
{
	CBTRDoc * pDoc = (CBTRDoc*)doc;
	UpdateData(TRUE);
	BOOL PlasmaLoaded  = pDoc->PlasmaLoaded;
	/*if (pDoc->MaxPlasmaDensity * pDoc->MaxPlasmaTe < 1.e-6)// || pDoc->pTorCrossPlate == NULL) 
		PlasmaLoaded = FALSE;
	
	if (m_OptPlasma && !PlasmaLoaded) { // traced
		AfxMessageBox("Please, set plasma PSI/Ne/Te/Zeff profiles before choosing this option");
		m_OptPlasma = 0; // canceled
		UpdateData(FALSE);
	}*/
	
	//if (PlasmaLoaded) 
	pDoc->OnPlotPenetration(); //PlotBeamDecay();
}

void CBeamDlg::OnReionisation() 
{
	UpdateData(TRUE);
	CBTRDoc * pDoc = (CBTRDoc*)doc;
	pDoc->SetReionization();
/*	CReionDlg dlg;

	switch (pDoc->TracePartType) { // 0:e, 1:H+, 2:D+, -1:H-, -2:D-, 10:H0, 20:H0
	case 0:	dlg.m_Caption = "e -> e+"; break;
	case 1:
	case -1:
	case 10: dlg.m_Caption = "Ho -> H+"; break;
	case 2:
	case -2:
	case 20: dlg.m_Caption = "Do -> D+"; break;
	default: dlg.m_Caption = "Neutral atom Zo -> Ion Z+"; break; 
	}
	dlg.m_IonStepL = *pDoc->IonStepL;
	dlg.m_Lstep = *pDoc->ReionStepL;
	dlg.m_Percent =	pDoc->ReionPercent;
	dlg.m_Sigma = *(pDoc->ReionSigma);
	dlg.m_Xmin = *(pDoc->ReionXmin);
	dlg.m_Xmax = *(pDoc->ReionXmax);

	if (dlg.DoModal() == IDOK) {
		*(pDoc->ReionSigma) = dlg.m_Sigma;
		*(pDoc->ReionXmin) = dlg.m_Xmin;
		*(pDoc->ReionXmax) = dlg.m_Xmax;
		*(pDoc->ReionStepL) = dlg.m_Lstep;
		*(pDoc->IonStepL) = dlg.m_IonStepL;
	
		pDoc->SetReionPercent();
		CString S;
		if (pDoc->PressureLoaded)	S.Format("Re-ionization loss = %3.1f %%", pDoc->ReionPercent);
		else S.Format("Pressure profile not defined. \n Re-ionized current = 0");
		AfxMessageBox(S);
	}
	*/
}

void CBeamDlg::OnBeamlet() 
{
	UpdateData(TRUE);
	CBTRDoc * pDoc = (CBTRDoc*)doc;
	pDoc->BeamSplitType = m_Gauss;
	BeamletDlg dlg;

	if (m_Gauss == 0) { 
		dlg.m_PolarNStr = "along Polar coord";
		dlg.m_AzimNStr = "along Azimuth coord";
		dlg.m_Npolar = (int)(pDoc->PolarNumber);
		dlg.m_Nazim = (int)(pDoc->AzimNumber);
	}
	else {
		dlg.m_PolarNStr = "along Horizontal axis";
		dlg.m_AzimNStr = "along Vertical axis";
		dlg.m_Npolar = (int)(pDoc->BeamSplitNumberY);
		dlg.m_Nazim = (int)(pDoc->BeamSplitNumberZ);
	}

	dlg.m_Common = pDoc->OptIndBeamlets;
	dlg.m_CoreDivHor = pDoc->BeamCoreDivY * 1000; // mrad
	dlg.m_CoreDivVert = pDoc->BeamCoreDivZ * 1000;
	//dlg.m_OptAngle = !(int)*(pDoc->DecilType);
	dlg.m_HaloDiv = pDoc->BeamHaloDiverg * 1000; // mrad
	dlg.m_HaloPart = pDoc->BeamHaloPart * 100; // %
	dlg.m_CutOffCurr = (float) pDoc->CutOffCurrent;
	dlg.m_Nbeamlets = pDoc->NofBeamletsTotal;
	dlg.m_BeamletCurr = pDoc->IonBeamCurrent / (pDoc->NofBeamletsTotal);

	int idres = dlg.DoModal();
	if (idres == IDOK) {

		if (m_Gauss == 0) { 
			pDoc->PolarNumber = dlg.m_Npolar;
			pDoc->AzimNumber = dlg.m_Nazim;
		}
		else {
			pDoc->BeamSplitNumberY = dlg.m_Npolar;
			pDoc->BeamSplitNumberZ = dlg.m_Nazim;
		}
	
		pDoc->OptIndBeamlets = (dlg.m_Common != 0);
		pDoc->BeamCoreDivY = dlg.m_CoreDivHor * 0.001;
		pDoc->BeamCoreDivZ = dlg.m_CoreDivVert * 0.001;
		pDoc->BeamHaloDiverg = dlg.m_HaloDiv * 0.001; // mrad -> rad
		pDoc->BeamHaloPart = dlg.m_HaloPart * 0.01; // % -> nornal
		pDoc->CutOffCurrent = dlg.m_CutOffCurr;
	
		CWaitCursor wait; 
		if (m_Gauss == 0) // Polar split
			pDoc->SetPolarBeamletCommon(); //SetBeamletAt(0);
		else 
			pDoc->SetGaussBeamlet(pDoc->BeamCoreDivY, pDoc->BeamCoreDivZ); 
		//pDoc->SetPolarBeamletReion();
		//pDoc->SetPolarBeamletResidual();

		pDoc->PlotBeamletFoot();
	
	}

/*	if (idres == ID_PLOT_BEAMLETCURR) {
	
	}

	if (idres == ID_PLOT_BEAMLETFOOT) {

	}
*/
	
}

void CBeamDlg::OnParticles() 
{
	UpdateData(TRUE);
	CBTRDoc * pDoc = (CBTRDoc*)doc;
	CParticlesDlg dlg;

	switch (pDoc->TracePartType) {
	case 0: dlg.m_Caption = "ELECTRONS"; break;
	case 1: dlg.m_Caption = "HYDROGEN ions"; break;
	case 2:	dlg.m_Caption = "DEUTERIUM ions"; break;
	case -1: dlg.m_Caption = "HYDROGEN ions"; break;
	case -2: dlg.m_Caption = "DEUTERIUM ions"; break;
	case 10: dlg.m_Caption = "HYDROGEN atoms"; break;
	case 20: dlg.m_Caption = "DEUTERIUM atoms"; break;
	default: dlg.m_Caption = "SOURCE PARTICLES"; break; 
	}

	dlg.m_EkeV = pDoc->IonBeamEnergy *1000; // MeV->keV
	dlg.m_Mass = pDoc->TracePartMass;
	dlg.m_Q = pDoc->TracePartQ;
	dlg.m_Nucl = pDoc->TracePartNucl;
	dlg.m_Tstep = (float)pDoc->TraceTimeStep;
	dlg.m_Lstep = pDoc->TraceStepL;
	dlg.m_MaxPoints = pDoc->TracePoints;
	dlg.m_OptTime = pDoc->TraceOption;
	dlg.m_Current = pDoc->IonBeamCurrent;

		if (dlg.DoModal() == IDOK) {
			//pDoc->TracePartType = m_D;
			pDoc->IonBeamEnergy = dlg.m_EkeV / 1000;
			pDoc->TracePartMass = dlg.m_Mass;
			pDoc->TracePartQ = dlg.m_Q;
			pDoc->TracePartNucl = dlg.m_Nucl;
			pDoc->TraceTimeStep = dlg.m_Tstep;
			pDoc->TraceStepL = dlg.m_Lstep;
			pDoc->TracePoints = dlg.m_MaxPoints;
			pDoc->TraceOption = dlg.m_OptTime;
			pDoc->IonBeamCurrent = dlg.m_Current; 
		
			pDoc->SetTraceParticle(dlg.m_Nucl, dlg.m_Q);
			
			pDoc->IonBeamPower = pDoc->IonBeamCurrent * pDoc->IonBeamEnergy; 
			pDoc->NeutrPower = pDoc->IonBeamPower * pDoc->NeutrPart;
		}

	m_D = -1;
	if (pDoc->TracePartType == -2 || pDoc->TracePartType == 2 || pDoc->TracePartType == 20) m_D = 0;
	if (pDoc->TracePartType == -1 || pDoc->TracePartType == 1 || pDoc->TracePartType == 10) m_D = 1;
	if (pDoc->TracePartType == 0) m_D = 2;
	
	UpdateData(TRUE);

	InvalidateRect(NULL, TRUE);
	
}

void CBeamDlg::OnOptReion() // check-button stop/trace reion particles
{
	UpdateData(TRUE);
	CBTRDoc * pDoc = (CBTRDoc*)doc;
	if (m_OptReion && !pDoc->FieldLoaded) { // traced with no field
		AfxMessageBox("Please, set MF distribution before choosing this option");
		m_OptReion = 0; // stopped
		UpdateData(FALSE);
	}
	if (m_OptReion && !pDoc->PressureLoaded) {// traced with no gas
		AfxMessageBox("Please, set gas profile before choosing this option");
		m_OptReion = 0;
		UpdateData(FALSE);
	}
}

void CBeamDlg::SetButtons()
{
	CBTRDoc * pDoc = (CBTRDoc*)doc;
	int type = pDoc->TracePartType;

	CWnd* hc1 = GetDlgItem(IDC_Neutr);
	CWnd* hc2 = GetDlgItem(IDC_Reionisation);
	CWnd* hc3 = GetDlgItem(IDC_BeamPlasma);
	CWnd* hc4 = GetDlgItem(IDC_Thick);
	CWnd* hc5 = GetDlgItem(IDC_Reion);
	CWnd* hc6 = GetDlgItem(IDC_RID);
	CWnd* hc7 = GetDlgItem(IDC_ATOMS);
	CWnd* hc8 = GetDlgItem(IDC_IONPOWER);
	
//	UpdateData(TRUE);
	if (type == 0) {
		hc1->EnableWindow(FALSE);
		hc2->EnableWindow(FALSE);
		hc3->EnableWindow(FALSE);
		hc4->EnableWindow(FALSE);
		hc5->EnableWindow(FALSE);
		hc6->EnableWindow(FALSE);
		hc7->EnableWindow(FALSE);
		hc8->EnableWindow(FALSE);
	}
	else {
		hc1->EnableWindow(TRUE);
		hc2->EnableWindow(TRUE);
		hc3->EnableWindow(TRUE);
		hc4->EnableWindow(TRUE);
		hc5->EnableWindow(TRUE);
		hc6->EnableWindow(TRUE);
		hc7->EnableWindow(TRUE);
		hc8->EnableWindow(TRUE);
	}
}

void CBeamDlg::OnE() 
{
	UpdateData(TRUE);
	CBTRDoc * pDoc = (CBTRDoc*)doc;
	pDoc->SetTraceParticle(0);
	SetButtons();
}

void CBeamDlg::OnOptD() 
{
	UpdateData(TRUE);
	CBTRDoc * pDoc = (CBTRDoc*)doc;
//	pDoc->TracePartQ = -1;
	pDoc->SetTraceParticle(-2);
	SetButtons();

}

void CBeamDlg::OnOptH() 
{
	UpdateData(TRUE);
	CBTRDoc * pDoc = (CBTRDoc*)doc;
//	pDoc->TracePartQ = -1;
	pDoc->SetTraceParticle(-1);
	SetButtons();
}

void CBeamDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	SetButtons();
	
	// Do not call CDialog::OnPaint() for painting messages
}

void CBeamDlg::OnOptThickNeutr() //button
{
	UpdateData(TRUE);
	CBTRDoc * pDoc = (CBTRDoc*)doc;
	if (m_OptThick && !pDoc->PressureLoaded) { // traced
		AfxMessageBox("Please, set gas profile before choosing this option");
		m_OptThick = 0; // canceled
		UpdateData(FALSE);
	}
	
}


void CBeamDlg::OnPlasma() // option!
{
	CBTRDoc * pDoc = (CBTRDoc*)doc;
	UpdateData(TRUE);
	BOOL PlasmaLoaded = pDoc->PlasmaLoaded; 
/*	if (pDoc->MaxPlasmaDensity * pDoc->MaxPlasmaTe < 1.e-6)// || pDoc->pTorCrossPlate == NULL) 
		PlasmaLoaded = FALSE;
	
	if (m_OptPlasma && !PlasmaLoaded) { // traced
		AfxMessageBox("Please, set plasma PSI/Ne/Te/Zeff profiles before choosing this option");
		m_OptPlasma = 0; // canceled
		UpdateData(FALSE);
	}*/
	//if (PlasmaLoaded && m_OptPlasma) pDoc->PlotBeamDecay();
	SetButtons();
}

void CBeamDlg::OnFile() 
{
	CBTRDoc * pDoc = (CBTRDoc*)doc;
//	pDoc->OnPdpBeamlets();
	pDoc->ReadSINGAP();//	pDoc->SINGAPLoaded = FALSE;
	pDoc->SetSINGAP();
	if (pDoc->OptSINGAP) m_OptSINGAP = 0;
	else m_OptSINGAP = 1;
	UpdateData(FALSE);

	InvalidateRect(NULL, TRUE);
	
}

void CBeamDlg::OnIonPower() 
{
	// TODO: Add your control notification handler code here
	
}
