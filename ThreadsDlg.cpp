// ThreadsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BTR.h"
#include "ThreadsDlg.h"
#include "BTRDoc.h"

// CThreadsDlg dialog

IMPLEMENT_DYNAMIC(CThreadsDlg, CDialog)

CThreadsDlg::CThreadsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CThreadsDlg::IDD, pParent)
	, Nproc(0)
	, Nthreads(0)
	, m_MaxScen(0)
{

}

CThreadsDlg::~CThreadsDlg()
{
}

void CThreadsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_PROC, Nproc);
	DDV_MinMaxInt(pDX, Nproc, 1, 16);
	DDX_Text(pDX, IDC_THREADS, Nthreads);
	DDV_MinMaxInt(pDX, Nthreads, 1, 16);
	//DDX_Check(pDX, IDC_CalcNeutr, m_CalcNeutr);
	//DDX_Check(pDX, IDC_CalcRID, m_CalcRID);
	//DDX_Check(pDX, IDC_CalcDuct, m_CalcDuct);
	//DDX_Text(pDX, IDC_Xmin, m_Xmin);
	//DDX_Text(pDX, IDC_Xmax, m_Xmax);
	DDX_Text(pDX, IDC_MaxScen, m_MaxScen);
	DDV_MinMaxInt(pDX, m_MaxScen, 0, 100);
}


BEGIN_MESSAGE_MAP(CThreadsDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_PARAM, OnBnClickedButtonParam)  //&CThreadsDlg::
END_MESSAGE_MAP()


// CThreadsDlg message handlers


void CThreadsDlg::OnBnClickedButtonParam()
{
	// TODO: Add your control notification handler code here
	CBTRDoc * pDoc = (CBTRDoc*)doc;
	//pDoc->ReadRIDfield();
	
	pDoc->ReadScenFile();
	m_MaxScen = pDoc->MAXSCEN;
	CString S;
	if (m_MaxScen > 1)
		S.Format("MULTI (%d)", m_MaxScen);
	else S.Format("SINGLE");
	SetDlgItemText(IDC_BUTTON_PARAM, S);
	UpdateData(TRUE);
	InvalidateRect(NULL, TRUE);
	//return;
	
	
/*	CFileDialog * fname_dlg;
	CString infile;
	char s[1024];
	int lines;
	int columns;
	CString S;

	fname_dlg = new CFileDialog(TRUE, "dat; txt  | * ", infile, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"MF data file (*.txt);(*.dat) | *.txt; *.TXT; *.dat; *.DAT | All Files (*.*) | *.*||", NULL);


	if (fname_dlg->DoModal() == IDCANCEL) {
		delete[] fname_dlg;
		UpdateData(FALSE);
		return;
	} // CANCEL

	infile = fname_dlg->GetPathName();
	CString shortname = fname_dlg->GetFileName();
	delete[] fname_dlg;

	CPreviewDlg dlg;
	dlg.m_Filename = infile;
	if (dlg.DoModal() == IDCANCEL) return;

	
	FILE * fp = fopen(infile, "rt");
	if (fp == NULL) {
		AfxMessageBox("failed to open data file", MB_ICONSTOP | MB_OK);
		return;
	}

	char name[256];
	strcpy(name, infile);

	//columns = FindDataColumns(fp);//(fp);

	fclose(fp);*/
}
