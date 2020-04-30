// NBIconfDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BTR.h"
#include "BTRDoc.h"
#include "NBIconfDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNBIconfDlg dialog


CNBIconfDlg::CNBIconfDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CNBIconfDlg::IDD, pParent)
	, m_AddDuct(FALSE)
{
	//{{AFX_DATA_INIT(CNBIconfDlg)
	m_CalOpen = -1;
	m_NeutrChannels = 0;
	m_RIDChannels = 0;
	m_NeutrGapIn = 0.0;
	m_NeutrGapOut = 0.0;
	m_NeutrThickIn = 0.0;
	m_NeutrThickOut = 0.0;
	m_RIDGapIn = 0.0;
	m_RIDGapOut = 0.0;
	m_RIDThick = 0.0;
	m_Volt = 0.0;
	m_CalculWidth = FALSE;
	//}}AFX_DATA_INIT
}


void CNBIconfDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNBIconfDlg)
	DDX_Radio(pDX, IDC_CalOpen, m_CalOpen);
	DDX_Text(pDX, IDC_NeutrChannels, m_NeutrChannels);
	DDX_Text(pDX, IDC_RIDChannels, m_RIDChannels);
	DDX_Text(pDX, IDC_NeutrGapIn, m_NeutrGapIn);
	DDX_Text(pDX, IDC_NeutrGapOut, m_NeutrGapOut);
	DDX_Text(pDX, IDC_NeutrThickIn, m_NeutrThickIn);
	DDX_Text(pDX, IDC_NeutrThickout, m_NeutrThickOut);
	DDX_Text(pDX, IDC_RIDGapIn, m_RIDGapIn);
	DDX_Text(pDX, IDC_RIDGapOut, m_RIDGapOut);
	DDX_Text(pDX, IDC_RIDThick, m_RIDThick);
	DDX_Text(pDX, IDC_Volt, m_Volt);
	DDX_Check(pDX, IDC_CalculWidth, m_CalculWidth);
	//}}AFX_DATA_MAP
	DDX_Check(pDX, IDC_AddDuct, m_AddDuct);
}


BEGIN_MESSAGE_MAP(CNBIconfDlg, CDialog)
	//{{AFX_MSG_MAP(CNBIconfDlg)
	ON_BN_CLICKED(IDC_CalculWidth, OnCalculWidth)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNBIconfDlg message handlers

void CNBIconfDlg::OnCalculWidth() 
{
	UpdateData(TRUE);
	CBTRDoc * pDoc = (CBTRDoc*) doc;
	pDoc->NeutrChannels = m_NeutrChannels;
	pDoc->RIDChannels = m_RIDChannels;

	if (m_CalculWidth) { 
		pDoc->SetChannelWidth();
		//m_CalculWidth = FALSE;
		m_NeutrGapIn = pDoc->NeutrInW * 1000;
		m_NeutrGapOut = pDoc->NeutrOutW * 1000;
		m_RIDGapIn = pDoc->RIDInW * 1000;
		m_RIDGapOut = pDoc->RIDOutW * 1000;
	//	m_CalculWidth = !m_CalculWidth;
		UpdateData(FALSE);
		InvalidateRect(NULL, TRUE);
	}
		m_CalculWidth = !m_CalculWidth;

}
