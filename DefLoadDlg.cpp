// DefLoadDlg.cpp: файл реализации
//

#include "stdafx.h"
#include "BTR.h"
#include "DefLoadDlg.h"
//#include "afxdialogex.h"


// диалоговое окно DefLoadDlg

IMPLEMENT_DYNAMIC(DefLoadDlg, CDialog)

DefLoadDlg::DefLoadDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_DefLoad_Dlg, pParent)
	, NX(0)
	, NY(0)
	, Round(FALSE)
	, m_OptN(FALSE)
	, m_StepX(0)
	, m_StepY(0)
	, m_OptAtom(FALSE)
	, m_OptNeg(FALSE)
	, m_OptPos(FALSE)
{

}

DefLoadDlg::~DefLoadDlg()
{
}

void DefLoadDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_Nx, NX);
	DDV_MinMaxInt(pDX, NX, 1, 1000);
	DDX_Text(pDX, IDC_Ny, NY);
	DDV_MinMaxInt(pDX, NY, 1, 1000);
	DDX_Check(pDX, IDC_RoundStep, Round);
	DDX_Radio(pDX, IDC_OptN, m_OptN);
	DDX_Text(pDX, IDC_StepX, m_StepX);
	DDX_Text(pDX, IDC_StepY, m_StepY);
	DDX_Check(pDX, IDC_Atom_Power, m_OptAtom);
	DDX_Check(pDX, IDC_NegIon_Power, m_OptNeg);
	DDX_Check(pDX, IDC_PosIons_Power, m_OptPos);
}


BEGIN_MESSAGE_MAP(DefLoadDlg, CDialog)
END_MESSAGE_MAP()


// обработчики сообщений DefLoadDlg
