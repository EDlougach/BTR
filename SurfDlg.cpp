// SurfDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BTR.h"
#include "SurfDlg.h"
#include "config.h"
#include "BTRDoc.h"
#include "PreviewDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSurfDlg dialog


CSurfDlg::CSurfDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSurfDlg::IDD, pParent)
	, m_N(0)
	, m_Comment(_T(""))
{
	//{{AFX_DATA_INIT(CSurfDlg)
	m_Solid = -1;
	m_X1 = 0.0;
	m_X2 = 0.0;
	m_X3 = 0.0;
	m_X4 = 0.0;
	m_Y1 = 0.0;
	m_Y2 = 0.0;
	m_Y3 = 0.0;
	m_Y4 = 0.0;
	m_Z1 = 0.0;
	m_Z2 = 0.0;
	m_Z3 = 0.0;
	m_Z4 = 0.0;
	//}}AFX_DATA_INIT
	OptRead = FALSE;
}


void CSurfDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSurfDlg)
	DDX_Radio(pDX, IDC_Solid, m_Solid);
	DDX_Text(pDX, IDC_X1, m_X1);
	DDX_Text(pDX, IDC_X2, m_X2);
	DDX_Text(pDX, IDC_X3, m_X3);
	DDX_Text(pDX, IDC_X4, m_X4);
	DDX_Text(pDX, IDC_Y1, m_Y1);
	DDX_Text(pDX, IDC_Y2, m_Y2);
	DDX_Text(pDX, IDC_Y3, m_Y3);
	DDX_Text(pDX, IDC_Y4, m_Y4);
	DDX_Text(pDX, IDC_Z1, m_Z1);
	DDX_Text(pDX, IDC_Z2, m_Z2);
	DDX_Text(pDX, IDC_Z3, m_Z3);
	DDX_Text(pDX, IDC_Z4, m_Z4);
	//}}AFX_DATA_MAP
	DDX_Text(pDX, IDC_N, m_N);
	DDX_Text(pDX, IDC_Comment, m_Comment);
}


BEGIN_MESSAGE_MAP(CSurfDlg, CDialog)
	//{{AFX_MSG_MAP(CSurfDlg)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_READFILE, OnReadfile) //&CSurfDlg::OnBnClickedReadfile)
	//ON_BN_CLICKED(IDHELP, OnReadfile)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSurfDlg message handlers

void CSurfDlg::OnOK() 
{
	OptRead = FALSE;
	UpdateData(TRUE);
	C3Point P1(m_X1, m_Y1, m_Z1);
	C3Point P2(m_X2, m_Y2, m_Z2);
	C3Point P3(m_X3, m_Y3, m_Z3);
	C3Point P4(m_X4, m_Y4, m_Z4);

	int res = 0;
	if (GetDistBetween(P1, P2) < 1.e-6) res = 1;
	if (GetDistBetween(P1, P3) < 1.e-6) res = 1;
	if (GetDistBetween(P1, P4) < 1.e-6) res = 1;
	if (GetDistBetween(P2, P3) < 1.e-6) res = 1;
	if (GetDistBetween(P2, P4) < 1.e-6) res = 1;
	if (GetDistBetween(P3, P4) < 1.e-6) res = 1;
	
	if (res == 1) {
		AfxMessageBox("At least 2 corners coincide");
		return;
	}
	
	else CDialog::OnOK();
}

/*void CSurfDlg::OnReadAdd() 
{
	CBTRDoc * pDoc = (CBTRDoc*)doc;
//	int n = pDoc->ReadAddPlates();//	pDoc->SINGAPLoaded = FALSE;

	char name[1024];
	char buf[1024];
	CString line = "";
	int i, pos, num, res;
	char *endptr;
	CPlate * plate;
	double x, y, z;
	C3Point P[4];
	int total = 0;
	BOOL solid;


	CFileDialog dlg(TRUE, "txt; dat | * ", "", OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"Addit. Surf. list for BTR (*.txt); (*.dat) | *.txt; *.TXT; *.dat; *.DAT | All Files (*.*) | *.*||", NULL);
	if (dlg.DoModal() == IDOK) {
		strcpy(name, dlg.GetPathName());

		CPreviewDlg dlg;
		dlg.m_Filename = name;
		if (dlg.DoModal() == IDCANCEL) return;

		FILE * fin;
		fin = fopen(name, "r");
		if (fin == NULL) {
			AfxMessageBox("problem 1 opening data file", MB_ICONSTOP | MB_OK);
			total  = 0; return;
		}

	//	total = 0;  
		while (!feof(fin)) {
			if (fgets(buf, 1024, fin) == NULL) break;
			line = buf;
			pos = line.Find("#");
			if (pos < 0) continue;
		
			line = line.Mid(pos+1);
			strcpy(buf, line);
			num = strtol(buf, &endptr, 10);
			if (line.Find("SOLID") > 0) solid = TRUE;
			else solid = FALSE;
			for (i = 0; i < 4; i++) {
				if (fgets(buf, 1024, fin) == NULL) break;
				res = sscanf(buf, "%le %le %le", &x, &y, &z);
				P[i].X = x; P[i].Y = y; P[i].Z = z; 
			}
		//	plate = pDoc->AddPlate(solid, P[0], P[1], P[2], P[3]);
			plate = new CPlate();
			plate->SetArbitrary(P[0], P[1], P[2], P[3]);
			// pPlate->SetFromLimits(p0, p3);
			plate->Solid = solid;
			plate->Visible = solid;
	 
			// AddPlate(pPlate);
			pDoc->PlatesList.AddTail(plate);
			plate->Comment = "Additional " + line;
		//	AddPlate(plate);
		//	AddPlatesList.AddTail(plate);
			total++;
				
		}// feof
		fclose(fin);
	} // IDOK
		//return total;

	//if (total > 0)	pDoc->AppendAddPlates();
	OnOK();
}*/

void CSurfDlg::OnReadfile()
{
	/*CBTRDoc * pDoc = (CBTRDoc*)doc;
	int n = pDoc->ReadAddPlates();
	pDoc->AddPlatesNumber += n;
	//OnOK();*/
	OptRead = TRUE;
	CDialog::OnCancel();
	//pDoc->OnShow();
	//return;
}
