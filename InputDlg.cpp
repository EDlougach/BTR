// InputDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BTR.h"
#include "InputDlg.h"


// InputDlg dialog

IMPLEMENT_DYNAMIC(InputDlg, CDialog)

InputDlg::InputDlg(CWnd* pParent /*=NULL*/)
	: CDialog(InputDlg::IDD, pParent)
	, m_Mode(0)
{

}

InputDlg::~InputDlg()
{
}

void InputDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Radio(pDX, IDC_RADIO_Stand, m_Mode);
}


BEGIN_MESSAGE_MAP(InputDlg, CDialog)
END_MESSAGE_MAP()


// InputDlg message handlers
