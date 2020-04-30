// StartDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BTR.h"
#include "StartDlg.h"


// CStartDlg dialog

IMPLEMENT_DYNAMIC(CStartDlg, CDialog)

CStartDlg::CStartDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CStartDlg::IDD, pParent)
	, m_Start(0)
{

}

CStartDlg::~CStartDlg()
{
}

void CStartDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Radio(pDX, IDC_RADIO_New, m_Start);
}


BEGIN_MESSAGE_MAP(CStartDlg, CDialog)
END_MESSAGE_MAP()


// CStartDlg message handlers
