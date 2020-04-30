// LevelsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BTR.h"
#include "LevelsDlg.h"


// CLevelsDlg dialog

IMPLEMENT_DYNAMIC(CLevelsDlg, CDialog)

CLevelsDlg::CLevelsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLevelsDlg::IDD, pParent)
	, m_Min(0)
	, m_Max(0)
	, m_Step(0)
	, m_Write(FALSE)
{

}

CLevelsDlg::~CLevelsDlg()
{
}

void CLevelsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_Level_Min, m_Min);
	DDV_MinMaxDouble(pDX, m_Min, 0, 1000000000);
	DDX_Text(pDX, IDC_Level_Max, m_Max);
	DDV_MinMaxDouble(pDX, m_Max, 0, 1000000000000);
	DDX_Text(pDX, IDC_Level_Step, m_Step);
	DDV_MinMaxDouble(pDX, m_Step, 0, 1000000000);
	DDX_Check(pDX, IDC_Level_Write, m_Write);
}


BEGIN_MESSAGE_MAP(CLevelsDlg, CDialog)
END_MESSAGE_MAP()


// CLevelsDlg message handlers
