// PlasmaDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BTR.h"
#include "PlasmaDlg.h"
#include "BTRDoc.h"


// CPlasmaDlg dialog

IMPLEMENT_DYNAMIC(CPlasmaDlg, CDialog)

CPlasmaDlg::CPlasmaDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPlasmaDlg::IDD, pParent)
	, m_Hw(0)
	, m_Dw(0)
	, m_Tw(0)
	, m_HeW(0)
	, m_LiW(0)
	, m_BeW(0)
	, m_Bw(0)
	, m_Cw(0)
	, m_Nw(0)
	, m_Ow(0)
	, m_FeW(0)
	, m_H(FALSE)
	, m_D(FALSE)
	, m_T(FALSE)
	, m_He(FALSE)
	, m_Li(FALSE)
	, m_Be(FALSE)
	, m_B(FALSE)
	, m_C(FALSE)
	, m_N(FALSE)
	, m_O(FALSE)
	, m_Fe(FALSE)
{

}

CPlasmaDlg::~CPlasmaDlg()
{
}

void CPlasmaDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_Hw, m_Hw);
	DDV_MinMaxDouble(pDX, m_Hw, 0, 1000);
	DDX_Text(pDX, IDC_Dw, m_Dw);
	DDV_MinMaxDouble(pDX, m_Dw, 0, 1000);
	DDX_Text(pDX, IDC_Tw, m_Tw);
	DDV_MinMaxDouble(pDX, m_Tw, 0, 1000);
	DDX_Text(pDX, IDC_Hew, m_HeW);
	DDV_MinMaxDouble(pDX, m_HeW, 0, 1000);
	DDX_Text(pDX, IDC_Liw, m_LiW);
	DDV_MinMaxDouble(pDX, m_LiW, 0, 1000);
	DDX_Text(pDX, IDC_Bew, m_BeW);
	DDV_MinMaxDouble(pDX, m_BeW, 0, 1000);
	DDX_Text(pDX, IDC_Bw, m_Bw);
	DDV_MinMaxDouble(pDX, m_Bw, 0, 1000);
	DDX_Text(pDX, IDC_Cw, m_Cw);
	DDV_MinMaxDouble(pDX, m_Cw, 0, 1000);
	DDX_Text(pDX, IDC_Nw, m_Nw);
	DDV_MinMaxDouble(pDX, m_Nw, 0, 1000);
	DDX_Text(pDX, IDC_Ow, m_Ow);
	DDV_MinMaxDouble(pDX, m_Ow, 0, 1000);
	DDX_Text(pDX, IDC_Few, m_FeW);
	DDV_MinMaxDouble(pDX, m_FeW, 0, 1000);
	DDX_Check(pDX, IDC_H, m_H);
	DDX_Check(pDX, IDC_D, m_D);
	DDX_Check(pDX, IDC_H3, m_T);
	DDX_Check(pDX, IDC_He, m_He);
	DDX_Check(pDX, IDC_Li, m_Li);
	DDX_Check(pDX, IDC_Be, m_Be);
	DDX_Check(pDX, IDC_B, m_B);
	DDX_Check(pDX, IDC_C, m_C);
	DDX_Check(pDX, IDC_N, m_N);
	DDX_Check(pDX, IDC_O, m_O);
	DDX_Check(pDX, IDC_Fe, m_Fe);
}


BEGIN_MESSAGE_MAP(CPlasmaDlg, CDialog)
	ON_BN_CLICKED(IDC_PSI, &CPlasmaDlg::OnBnClickedPsi)
	ON_BN_CLICKED(IDC_NTprofiles, &CPlasmaDlg::OnBnClickedNtprofiles)
END_MESSAGE_MAP()


// CPlasmaDlg message handlers

void CPlasmaDlg::OnBnClickedPsi()
{
	// TODO: Add your control notification handler code here
	CBTRDoc * pDoc = (CBTRDoc*)doc;
	pDoc->OnPlasmaPsi();
}

void CPlasmaDlg::OnBnClickedNtprofiles()
{
	CBTRDoc * pDoc = (CBTRDoc*)doc;
	pDoc->OnEditPlasmaTeNe();
}
