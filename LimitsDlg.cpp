// LimitsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BTR.h"
#include "LimitsDlg.h"


// CLimitsDlg dialog

IMPLEMENT_DYNAMIC(CLimitsDlg, CDialog)

CLimitsDlg::CLimitsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLimitsDlg::IDD, pParent)
	, m_Xmin(0)
	, m_Xmax(0)
	, m_StrMin(_T(""))
	, m_StrMax(_T(""))
	, m_Sstep(_T(""))
	, m_Step(0)
{

}

CLimitsDlg::~CLimitsDlg()
{
}

void CLimitsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_Xmin, m_Xmin);
	DDX_Text(pDX, IDC_Xmax, m_Xmax);
	DDX_Text(pDX, IDC_Smin, m_StrMin);
	DDX_Text(pDX, IDC_Smax, m_StrMax);
	DDX_Text(pDX, IDC_Sstep, m_Sstep);
	DDX_Text(pDX, IDC_Step, m_Step);
}


BEGIN_MESSAGE_MAP(CLimitsDlg, CDialog)
END_MESSAGE_MAP()


// CLimitsDlg message handlers
