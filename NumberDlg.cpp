// NumberDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BTR.h"
#include "NumberDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CNumberDlg dialog

IMPLEMENT_DYNAMIC(CNumberDlg, CDialog)

CNumberDlg::CNumberDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CNumberDlg::IDD, pParent)
	, SurfNumber(0)
	, m_Text(_T(""))
{

}

CNumberDlg::~CNumberDlg()
{
}

void CNumberDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_Nsurf, SurfNumber);
	DDV_MinMaxInt(pDX, SurfNumber, -1, 10000);
	DDX_Text(pDX, IDC_Text, m_Text);
}


BEGIN_MESSAGE_MAP(CNumberDlg, CDialog)
END_MESSAGE_MAP()


// CNumberDlg message handlers
