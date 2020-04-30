// AtomDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BTR.h"
#include "AtomDlg.h"


// CAtomDlg dialog

IMPLEMENT_DYNAMIC(CAtomDlg, CDialog)

CAtomDlg::CAtomDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAtomDlg::IDD, pParent)
	, m_PosX(0)
	, m_PosY(0)
	, m_PosZ(0)
	, m_Vx(0)
	, m_Vy(0)
	, m_Vz(0)
	, m_OptReion(FALSE)
	, m_Option(_T(""))
	, m_Step(0)
	, m_Energy(0)
{

}

CAtomDlg::~CAtomDlg()
{
}

void CAtomDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_PosX, m_PosX);
	DDX_Text(pDX, IDC_PosY, m_PosY);
	DDX_Text(pDX, IDC_PosZ, m_PosZ);
	DDX_Text(pDX, IDC_Vx, m_Vx);
	DDX_Text(pDX, IDC_Vy, m_Vy);
	DDX_Text(pDX, IDC_Vz, m_Vz);
	DDX_Check(pDX, IDC_OptReion, m_OptReion);
	DDX_Text(pDX, IDC_Text, m_Option);
	DDX_Text(pDX, IDC_Step, m_Step);
	DDX_Text(pDX, IDC_Energy, m_Energy);
	DDV_MinMaxDouble(pDX, m_Energy, 0, 10000000);
	DDV_MinMaxDouble(pDX, m_Step, 1e-12, 1);
}


BEGIN_MESSAGE_MAP(CAtomDlg, CDialog)
END_MESSAGE_MAP()


// CAtomDlg message handlers
