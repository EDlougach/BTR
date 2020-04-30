// AddGasDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BTR.h"
#include "AddGasDlg.h"
#include "BTRDoc.h"
#include "PreviewDlg.h"


// CAddGasDlg dialog

IMPLEMENT_DYNAMIC(CAddGasDlg, CDialog)

CAddGasDlg::CAddGasDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAddGasDlg::IDD, pParent)
	, m_Sigma(0)
{

}

CAddGasDlg::~CAddGasDlg()
{
}

void CAddGasDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_Sigma, m_Sigma);
	DDV_MinMaxDouble(pDX, m_Sigma, 0, 1);
}


BEGIN_MESSAGE_MAP(CAddGasDlg, CDialog)
	ON_BN_CLICKED(IDC_AddFileBUTTON, &CAddGasDlg::OnBnClickedAddFilebutton)
END_MESSAGE_MAP()


// CAddGasDlg message handlers

void CAddGasDlg::OnBnClickedAddFilebutton()
{
	CBTRDoc * pDoc = (CBTRDoc*)doc;
	CFileDialog * fname_dlg;
	CString infile;
	char s[1024];
	int lines = 0;
	int columns;
	CString S;

	fname_dlg = new CFileDialog(TRUE, "dat; txt  | * ",	infile,	OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"Gas Density file (*.txt);(*.dat) | *.txt; *.TXT; *.dat; *.DAT | All Files (*.*) | *.*||",	NULL);

	if(fname_dlg->DoModal() == IDCANCEL){
		delete [] fname_dlg;
		if (!pDoc->OptAddPressure) return;
		else { // addit pressure is
			if (AfxMessageBox("Do you want to clear the existing Additional Gas?", MB_YESNO) == IDYES) {
				//pDoc->GasFileName = "";
							
				pDoc->ClearPressure(1); // clear addit pressure
				pDoc->OptAddPressure = FALSE;//PressureLoaded = FALSE;
				//SetDlgItemText(IDC_GasFileName, "");
				UpdateData(FALSE);
				return;
			}
			return;
		}
	} // CANCEL

	infile = fname_dlg->GetPathName();
	//CString shortname = fname_dlg->GetFileName();
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

	if (columns != 2)  {
		sprintf(s,"format not supported", MB_ICONSTOP | MB_OK);
		AfxMessageBox(s, MB_ICONINFORMATION);
		return;
	}
	else { 
		lines = pDoc->ReadGas_2col(name, 1); // x dens
		//pDoc->GasFileName = shortname; //dlg.m_Filename;
	}
	
	S.Format("accepted %d data lines\n from file %s", lines, name);
	//AfxMessageBox(S);

	if (lines < 1)  {
		sprintf(s,"No data accepted)", MB_ICONSTOP | MB_OK);
		AfxMessageBox(s,MB_ICONINFORMATION);
	}
	else { // lines > 0
		pDoc->OptAddPressure = TRUE; //PressureLoaded = TRUE;
		
		//SetDlgItemText(IDC_GasFileName, shortname); //dlg.m_Filename);
	}
	//UpdateData(FALSE);
	//InvalidateRect(NULL, TRUE);
}
