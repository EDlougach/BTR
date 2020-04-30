// FieldsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BTR.h"
#include "FieldsDlg.h"
#include "BTRDoc.h"
#include "PreviewDlg.h"
#include "AddGasDlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFieldsDlg dialog


CFieldsDlg::CFieldsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFieldsDlg::IDD, pParent)
	, m_GasCoeff(0)
	, m_GasON(1)
	, m_MFfilename(_T(""))
	, m_Gasfilename(_T(""))
{
	//{{AFX_DATA_INIT(CFieldsDlg)
	m_MFON = 1;
	m_MFcoeff = 0.0;
	m_Volt = 0.0;
	//}}AFX_DATA_INIT
}


void CFieldsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFieldsDlg)
	DDX_Radio(pDX, IDC_MFON, m_MFON);
	DDX_Text(pDX, IDC_MFcoeff, m_MFcoeff);
	DDX_Text(pDX, IDC_Volt, m_Volt);
	//}}AFX_DATA_MAP
	DDX_Text(pDX, IDC_GasCoeff, m_GasCoeff);
	DDX_Radio(pDX, IDC_GasON, m_GasON);
	DDX_Text(pDX, IDC_MFFileName, m_MFfilename);
	DDX_Text(pDX, IDC_GasFileName, m_Gasfilename);
}


BEGIN_MESSAGE_MAP(CFieldsDlg, CDialog)
	//{{AFX_MSG_MAP(CFieldsDlg)
	ON_BN_CLICKED(IDC_RIDfile, OnRIDfile)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_MFfile, &CFieldsDlg::OnBnClickedMffile)
	ON_BN_CLICKED(IDC_GasFile, &CFieldsDlg::OnBnClickedGasfile)
	ON_BN_CLICKED(IDC_GasAdvBtn, &CFieldsDlg::OnBnClickedGasAdvbtn)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFieldsDlg message handlers

void CFieldsDlg::OnRIDfile() 
{
	CBTRDoc * pDoc = (CBTRDoc*)doc;
	pDoc->ReadRIDfield();
	//m_Volt = pDoc->RIDU;
	UpdateData(FALSE);
	InvalidateRect(NULL, TRUE);
}

void CFieldsDlg::OnBnClickedMffile()
{
	CBTRDoc * pDoc = (CBTRDoc*)doc;
	CFileDialog * fname_dlg;
	CString infile;
	char s[1024];
	int lines;
	int columns;
	CString S;

	fname_dlg = new CFileDialog(TRUE, "dat; txt  | * ",	infile,	OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"MF data file (*.txt);(*.dat) | *.txt; *.TXT; *.dat; *.DAT | All Files (*.*) | *.*||",	NULL);

	
	if(fname_dlg->DoModal() == IDCANCEL){
		delete [] fname_dlg;
		if (!pDoc->FieldLoaded) return;
		else {
			if (AfxMessageBox("Do you want to clear the existing Mag Field?", MB_YESNO) == IDYES) {
				//pDoc->GasFileName = "";
				m_MFON = 1;
				m_MFcoeff = 0;
				pDoc->ClearB();//FieldLoaded = FALSE;
				SetDlgItemText(IDC_MFFileName, "");
				UpdateData(FALSE);
				return;
			}
			return;
		}
	} // CANCEL


	infile = fname_dlg->GetPathName();
	CString shortname = fname_dlg->GetFileName();
	delete [] fname_dlg;

	CPreviewDlg dlg;
	dlg.m_Filename = infile;
	if (dlg.DoModal() == IDCANCEL) return;

	columns = 0;
	FILE * fp = fopen(infile,"rt");
	if (fp == NULL){
		AfxMessageBox("failed to open data file", MB_ICONSTOP | MB_OK);
		return;
	}

	char name[256];
	strcpy(name, infile);
	
	columns = FindDataColumns(fp);//(fp);

	fclose(fp);

	if (columns == 0)  {
		sprintf(s,"Invalid format of data", MB_ICONSTOP | MB_OK);
		AfxMessageBox(s, MB_ICONINFORMATION);
		return;
	}
	else { // columns !=0
		sprintf(s, "Data Columns = %d", columns);
		//AfxMessageBox(s, MB_ICONINFORMATION);
	}
	
	lines = 0; // data lines
	if (columns == 4) {
		lines = pDoc->ReadMF_4col(name); // x bx by bz
		pDoc->MagFieldFileName = shortname; //dlg.m_Filename;
		//pDoc->MagFieldDim = 1;

	}
	else if (columns > 5 && columns < 8) {
		lines = pDoc->ReadMF_7col(name);//BField3D->ReadData(name); // x y z bx by bz
		pDoc->MagFieldFileName = shortname; //dlg.m_Filename;
		//pDoc->MagFieldDim = 3;
	}
	else { AfxMessageBox("This format is not supported"); return; }

	S.Format("accepted %d data lines\n from file %s", lines, name);
	//AfxMessageBox(S);

	if (lines < 0)  {
		sprintf(s,"no data accepted)", MB_ICONSTOP | MB_OK);
		AfxMessageBox(s,MB_ICONINFORMATION);
	}
	else {
		//OptStrayField = TRUE;
		pDoc->FieldLoaded = TRUE;
		m_MFcoeff = 1.;
		m_MFON = 0;
		SetDlgItemText(IDC_MFFileName, shortname); // dlg.m_Filename);
	}
	UpdateData(FALSE);
	//InvalidateRect(NULL, TRUE);

}

void CFieldsDlg::OnBnClickedGasfile()
{
	CBTRDoc * pDoc = (CBTRDoc*)doc;
	CFileDialog * fname_dlg;
	CString infile;
	char s[1024];
	int lines;
	int columns;
	CString S;

	fname_dlg = new CFileDialog(TRUE, "dat; txt  | * ",	infile,	OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"Gas Density file (*.txt);(*.dat) | *.txt; *.TXT; *.dat; *.DAT | All Files (*.*) | *.*||",	NULL);

	if(fname_dlg->DoModal() == IDCANCEL){
		delete [] fname_dlg;
		if (!pDoc->PressureLoaded) return;
		else {
			if (AfxMessageBox("Do you want to clear the existing Gas Field?", MB_YESNO) == IDYES) {
				//pDoc->GasFileName = "";
				m_GasON = 1;
				m_GasCoeff = 0;
				pDoc->ClearPressure(0);//PressureLoaded = FALSE;
				SetDlgItemText(IDC_GasFileName, "");
				UpdateData(FALSE);
				return;
			}
			return;
		}
	} // CANCEL

	infile = fname_dlg->GetPathName();
	CString shortname = fname_dlg->GetFileName();
	delete [] fname_dlg;

	CPreviewDlg dlg;
	dlg.m_Filename = infile;
	if (dlg.DoModal() == IDCANCEL) return;

	columns = 0;
	FILE * fp = fopen(infile,"rt");
	if (fp == NULL){
		AfxMessageBox("failed to open data file", MB_ICONSTOP | MB_OK);
		return;
	}

	char name[256];
	strcpy(name, infile);
	
	columns = FindDataColumns(fp);//(fp);

	fclose(fp);

	if (columns == 0)  {
		sprintf(s,"Invalid format of data", MB_ICONSTOP | MB_OK);
		AfxMessageBox(s, MB_ICONINFORMATION);
		return;
	}
	else { // columns !=0
		sprintf(s, "Data Columns = %d", columns);
		//AfxMessageBox(s, MB_ICONINFORMATION);
	}
	
	lines = 0; // data lines
	if (columns == 2) {
		lines = pDoc->ReadGas_2col(name, 0); // x dens
		pDoc->GasFileName = shortname; //dlg.m_Filename;
	}
	else if (columns > 2 && columns < 4) {
		lines = pDoc->ReadGas_3col(name);//x z p
		pDoc->GasFileName = shortname;//dlg.m_Filename;
	}
	else { AfxMessageBox("This format is not supported"); return; }

	S.Format("accepted %d data lines\n from file %s", lines, name);
	//AfxMessageBox(S);

	if (lines < 0)  {
		sprintf(s,"No data accepted)", MB_ICONSTOP | MB_OK);
		AfxMessageBox(s,MB_ICONINFORMATION);
	}
	else { // lines > 0
		pDoc->PressureLoaded = TRUE;
		m_GasON = 0;
		m_GasCoeff = 1;
		SetDlgItemText(IDC_GasFileName, shortname); //dlg.m_Filename);
	}
	UpdateData(FALSE);
	//InvalidateRect(NULL, TRUE);
	
}

void CFieldsDlg::OnBnClickedGasAdvbtn()
{
	CBTRDoc * pDoc = (CBTRDoc*)doc;
	CAddGasDlg dlg;

	dlg.doc = pDoc;
	dlg.m_Sigma = pDoc->AddReionSigma;

	if (dlg.DoModal() == IDOK) {
		pDoc->AddReionSigma = dlg.m_Sigma;
	}
}
