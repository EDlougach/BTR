// ParticlesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BTR.h"
#include "ParticlesDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CParticlesDlg dialog


CParticlesDlg::CParticlesDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CParticlesDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CParticlesDlg)
	m_EkeV = 0.0;
	m_Lstep = 0.0;
	m_Mass = 0.0;
	m_MaxPoints = 0;
	m_OptTime = -1;
	m_Caption = _T("");
	m_Nucl = 0;
	m_Q = 0;
	m_Current = 0.0;
	m_Tstep = 0.0f;
	//}}AFX_DATA_INIT
}


void CParticlesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CParticlesDlg)
	DDX_Text(pDX, IDC_Energy, m_EkeV);
	DDV_MinMaxDouble(pDX, m_EkeV, 0., 1000000.);
	DDX_Text(pDX, IDC_Lstep, m_Lstep);
	DDX_Text(pDX, IDC_Mass, m_Mass);
	DDX_Text(pDX, IDC_Npoints, m_MaxPoints);
	DDX_Radio(pDX, IDC_Time, m_OptTime);
	DDX_Text(pDX, IDC_CAPTION, m_Caption);
	DDX_Text(pDX, IDC_Nucl, m_Nucl);
	DDX_Text(pDX, IDC_Q, m_Q);
	DDX_Text(pDX, IDC_Current, m_Current);
	DDX_Text(pDX, IDC_Tstep, m_Tstep);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CParticlesDlg, CDialog)
	//{{AFX_MSG_MAP(CParticlesDlg)
	ON_BN_CLICKED(IDC_Time, OnOptTime)
	ON_BN_CLICKED(IDC_Length, OnOptLength)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CParticlesDlg message handlers

void CParticlesDlg::OnOptTime() 
{
	UpdateData(TRUE);
	SetButtons();	
}

void CParticlesDlg::SetButtons()
{
	CWnd* hc1 = GetDlgItem(IDC_Tstep);
	CWnd* hc2 = GetDlgItem(IDC_Lstep);

	if (m_OptTime == 0) {
		hc1->EnableWindow(TRUE);
		hc2->EnableWindow(FALSE);
	}
	else {
		hc1->EnableWindow(FALSE);
		hc2->EnableWindow(TRUE);
	}
}

void CParticlesDlg::OnOptLength() 
{
	UpdateData(TRUE);
	SetButtons();	
	
}

void CParticlesDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	SetButtons();		
	// Do not call CDialog::OnPaint() for painting messages
}
