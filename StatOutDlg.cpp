// StatOutDlg.cpp: файл реализации
//

#include "stdafx.h"
#include "BTR.h"
#include "StatOutDlg.h"
//#include "afxdialogex.h"


// диалоговое окно CStatOutDlg

IMPLEMENT_DYNAMIC(CStatOutDlg, CDialog)

CStatOutDlg::CStatOutDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_StatOptDlg, pParent)
	, m_Atoms(0)
	, m_NegIons(0)
	, m_PosIons(0)
	, m_PosLoc(0)
	, m_PosGlob(0)
	, m_Afall(0)
	, m_Power(0)
	, m_Lines(0)
{

}

CStatOutDlg::~CStatOutDlg()
{
}

void CStatOutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_Atom_Power, m_Atoms);
	DDX_Check(pDX, IDC_NegIon_Power, m_NegIons);
	DDX_Check(pDX, IDC_PosIons_Power, m_PosIons);
	DDX_Check(pDX, IDC_PosLoc, m_PosLoc);
	DDX_Check(pDX, IDC_PosGlob, m_PosGlob);
	DDX_Check(pDX, IDC_Afall, m_Afall);
	DDX_Check(pDX, IDC_Power, m_Power);
	DDX_Text(pDX, IDC_Lines, m_Lines);
}


BEGIN_MESSAGE_MAP(CStatOutDlg, CDialog)
END_MESSAGE_MAP()


// обработчики сообщений CStatOutDlg
