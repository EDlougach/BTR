// MFDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BTR.h"
#include "MFDlg.h"
#include "config.h"
#include "PreviewDlg.h"
#include "PlotDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMFDlg dialog


CMFDlg::CMFDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMFDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMFDlg)
	m_Bdata = _T("");
	m_Filename = _T("");
	m_COLUMNS = _T("");
	//}}AFX_DATA_INIT
}


void CMFDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMFDlg)
	DDX_Text(pDX, IDC_Bdata, m_Bdata);
	DDX_Text(pDX, IDC_FILENAME, m_Filename);
	DDX_Text(pDX, IDC_COLUMNS, m_COLUMNS);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMFDlg, CDialog)
	//{{AFX_MSG_MAP(CMFDlg)
	ON_BN_CLICKED(IDC_NEW, OnNew)
	ON_BN_CLICKED(IDC_READFILE, OnReadfile)
	ON_BN_CLICKED(IDC_Save, OnSave)
	ON_BN_CLICKED(IDC_Plot, OnPlot)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMFDlg message handlers

void CMFDlg::OnNew() 
{
	m_Bdata.Empty();
	SetDlgItemText(IDC_Bdata, m_Bdata);
	SetDlgItemText(IDC_FILENAME, "new data");
}

void CMFDlg::OnReadfile() 
{
	CFileDialog *fname_dia;
	CString infile;
	char s[1024];

	fname_dia=new CFileDialog(
		TRUE,
		"dat; txt | * ",
		infile,
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"4-col MF data file (*.dat); (*.txt) | *.dat; *.DAT; *.txt; *.TXT | All Files (*.*) | *.*||",
		NULL);

	if(fname_dia->DoModal()==IDCANCEL){
		delete[] fname_dia;
		return;
	}
	infile = fname_dia->GetPathName();
	delete[] fname_dia;

	CPreviewDlg dlg;
	dlg.m_Filename = infile;
	if (dlg.DoModal() == IDCANCEL) return;

	int n =0;
	FILE *fp=fopen(infile,"rt");
	if (fp==NULL){
		AfxMessageBox("problem opening data file", MB_ICONSTOP | MB_OK);
		return;
	}

	m_Bdata.Empty();
	int res;
	double x, bx, by, bz;

	do{
		if(fgets(s,1024,fp)==NULL) break;
		res = sscanf(s, "%lf %lf %lf %lf", &x, &bx, &by, &bz); 
		if (res != 4) continue;
		sprintf(s,"%g\t %g\t %g\t %g\r\n",x, bx, by, bz); m_Bdata += s;
		n++;
		
	} while(1>0);

	fclose(fp);

	if (n<=1) {
		sprintf(s,"Invalid format of MF data");
		AfxMessageBox(s,MB_ICONSTOP | MB_OK);
	}
	else { 
		//sprintf(s,"Gas data is Read (%d lines)", n);
		CString S;
		S.Format("read %d data lines\n from file %s", n, infile);
		std::cout << "EDIT MF - " << S << std::endl;
	//	AfxMessageBox(S);
		m_Filename = infile;
		SetDlgItemText(IDC_Bdata, m_Bdata);
		SetDlgItemText(IDC_FILENAME, m_Filename);

		UpdateData(FALSE);
	}
	
}

void CMFDlg::OnSave() 
{
	UpdateData();
	StripMFData();

	CString Text = " MF along ITER NBI for BTR\n User Defined Data\n X,m\t\t Bx,T\t\t By,T\t\t Bz,T \n";
	char s[1024];
	int N = XArr->GetUpperBound();
	for (int i = 0; i <= N; i++) {
		sprintf(s, " %f\t %g\t %g\t %g \n", 
				XArr->GetAt(i), BArr->GetAt(i).X, BArr->GetAt(i).Y, BArr->GetAt(i).Z);
		Text += s;
	}

	CFileDialog dlg(FALSE, "dat; txt", "(*.dat); (*.txt)");
	if (dlg.DoModal() == IDOK) {
		FILE * fout;
		char name[1024];
		
		strcpy(name, dlg.GetPathName());
		fout = fopen(name, "w");
		fprintf(fout, Text);
		fclose(fout);
	//	sprintf(s,"Gas data is written in file %s", name);
	//	AfxMessageBox(s, MB_ICONINFORMATION);
		SetDlgItemText(IDC_FILENAME, name);
	}	
}


void CMFDlg:: StripMFData()
{
	int k;
	char *buff,*nptr,*endptr;
	double x, bx, by, bz;
//	char s[1024];
	CString S;
	
	XArr->RemoveAll();
	BArr->RemoveAll();

	int Npoints=0;
	C3Point P;
//	int res;

	if(m_Bdata.GetLength()==0) { AfxMessageBox("empty"); return;}
/*	do{
		res = sscanf(s, "%lf %lf %lf %lf", &x, &bx, &by, &bz); 
		if (res != 4) continue;
		P.X = bx; P.Y = by; P.Z = bz;
		XArr->Add(x); BArr->Add(P);
		Npoints++;
	} while(res!=EOF);
*/

	k = m_Bdata.GetLength()*2;
	buff = (char *)calloc(k,sizeof(char));
	strcpy(buff, m_Bdata);
	nptr = buff;
	while (sscanf(nptr,"%lf %lf %lf %lf",&x, &bx, &by, &bz) != EOF) {
		x = strtod(nptr,&endptr);
		if(nptr == endptr) nptr++; //catch special case
		else nptr = endptr;
		bx = strtod(nptr,&endptr);
		if(nptr == endptr) nptr++; //catch special case
		else nptr = endptr;
		by = strtod(nptr,&endptr);
		if(nptr == endptr) nptr++; //catch special case
		else nptr = endptr;
		bz = strtod(nptr,&endptr);
		if(nptr == endptr) nptr++; //catch special case
		else nptr = endptr;
		/*if(Npoints>0){ // enforce monotonicity
			if (x <= XArr->GetAt(Npoints-1)) break;
		}*/
		
		P.X = bx; P.Y = by; P.Z = bz;
		XArr->Add(x); BArr->Add(P);
		Npoints++;
		
	}
	free(buff);

	S.Format("%d, %d elements added", XArr->GetSize(), BArr->GetSize());
	//AfxMessageBox(S);
}	

void CMFDlg::OnOK() 
{
	UpdateData();
	StripMFData();
	CDialog::OnOK();
}

void CMFDlg::OnPlot() 
{
	UpdateData();
	StripMFData();

	CArray<double, double>  ArrX;
	CArray<double, double>  ArrY;
	double Xmax = 0;

	PLOTINFO MFPlot;
	MFPlot.Caption = "Magnetic Field Distribution along NBL (Scaling = 1)";
	MFPlot.LabelX = "X, m";
	MFPlot.LabelY = "Bx, By, Bz (Tesla)";
	MFPlot.LimitX = 0;
	MFPlot.LimitY = 0;//1.e-1;
	MFPlot.CrossX = 0;
	MFPlot.CrossY = 0; //2.e-2;
	MFPlot.Line = 1;
	MFPlot.Interp = 1;
	int Nmax = XArr->GetSize();
	MFPlot.N = 3 * Nmax;
	MFPlot.N1 = Nmax;
	MFPlot.N2 = 2 * Nmax;
	MFPlot.PosNegY = TRUE;
	MFPlot.PosNegX = FALSE;
	double x, y;
	int i;
	for (i = 0; i < Nmax; i++) {
		x = XArr->GetAt(i);
		y = BArr->GetAt(i).X;
		ArrX.Add(x); ArrY.Add(y);
		Xmax = Max(Xmax, x);
	}
	for (i = 0; i < Nmax; i++) {
		x = XArr->GetAt(i);
		y = BArr->GetAt(i).Y;
		ArrX.Add(x); ArrY.Add(y);
	}
	for (i = 0; i < Nmax; i++) {
		x = XArr->GetAt(i);
		y = BArr->GetAt(i).Z;
		ArrX.Add(x); ArrY.Add(y);
	}
	MFPlot.DataX = &ArrX;
	MFPlot.DataY = &ArrY;
	//MFPlot.LimitX = Xmax;
	
	CPlotDlg dlg;
	dlg.Plot = &MFPlot;
	dlg.InitPlot();
	dlg.DoModal();

	ArrX.RemoveAll();
	ArrY.RemoveAll();
}
