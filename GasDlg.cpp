// GasDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BTR.h"
#include "GasDlg.h"
#include "config.h"
#include "PreviewDlg.h"
#include "PlotDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGasDlg dialog


CGasDlg::CGasDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGasDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CGasDlg)
	m_Filename = _T("");
	m_Y = _T("");
	m_X = _T("");
	m_Caption = _T("");
	//}}AFX_DATA_INIT
	Arr = new CArray<C3Point, C3Point>;
	
	
}


void CGasDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGasDlg)
	DDX_Text(pDX, IDC_FILENAME, m_Filename);
	DDX_Text(pDX, IDC_Pres, m_Y);
	DDX_Text(pDX, IDC_X, m_X);
	DDX_Text(pDX, IDC_COLUMN1, m_Head1);
	DDX_Text(pDX, IDC_COLUMN2, m_Head2);
	//}}AFX_DATA_MAP
	SetWindowText(m_Caption);
}


BEGIN_MESSAGE_MAP(CGasDlg, CDialog)
	//{{AFX_MSG_MAP(CGasDlg)
	ON_BN_CLICKED(IDC_READFILE, OnReadfile)
	ON_BN_CLICKED(IDC_NEW, OnNew)
	ON_BN_CLICKED(IDC_Save, OnSave)
	ON_BN_CLICKED(IDC_Plot, OnPlot)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGasDlg message handlers

void CGasDlg::OnReadfile() 
{
	CFileDialog *fname_dia;
	CString infile;
	char s[1024];

	fname_dia=new CFileDialog(TRUE, "dat; txt | * ", infile,
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"2 columns file (*.dat);(*.txt) | *.dat; *.DAT; *.txt; *.TXT| All Files (*.*) | *.*||",
		NULL);

	if(fname_dia->DoModal()==IDCANCEL){
		delete[] fname_dia;
		return;
	}
	infile=fname_dia->GetPathName();
	delete[] fname_dia;

	CPreviewDlg dlg;
	dlg.m_Filename = infile;
	if (dlg.DoModal() == IDCANCEL) return;
		
	FILE *fp=fopen(infile,"rt");
	if (fp==NULL){
		AfxMessageBox("problem opening data file", MB_ICONSTOP | MB_OK);
		return;
	}

//	n = CBTRDoc.GField->ReadData(fp);
	m_X.Empty();
	m_Y.Empty();
	int res;
	double x, p;
	char buf[1024];

	fgets(buf, 1024, fp);
	int n = 0;
	while (!feof(fp)) {
	//do{
		if(fgets(s,1024,fp)==NULL) break;
		res = sscanf(s, "%lf %lf", &x, &p); 
		if (res != 2) continue;
		sprintf(s,"%f\r\n",x); m_X += s;
		sprintf(s,"%le\r\n",p); m_Y += s;
		n++;
		
	} //while(1>0);
	fclose(fp);

	SetDlgItemText(IDC_X, m_X);
	SetDlgItemText(IDC_Pres, m_Y);
	SetDlgItemText(IDC_FILENAME, infile);

	
	if (n<=1) {
		sprintf(s,"Invalid format of data");
		AfxMessageBox(s,MB_ICONSTOP | MB_OK);
	}
	else { 
		//sprintf(s,"Gas data is Read (%d lines)", n);
		CString S;
		S.Format("EDIT GAS - read %d data lines\n from file %s", n, infile);
	//	AfxMessageBox(S);
		std::cout << S << std::endl;
		m_Filename = infile;
		UpdateData(FALSE);
	}
/*	CWnd* hc;
	hc=GetDlgItem(IDC_Plot);
	if (Arr->GetSize() < 1) hc->EnableWindow(FALSE);
	else hc->EnableWindow(TRUE);
*/	
}


void CGasDlg::OnNew() 
{
	m_X.Empty();
	m_Y.Empty();
	SetDlgItemText(IDC_X, m_X);
	SetDlgItemText(IDC_Pres, m_Y);
	SetDlgItemText(IDC_FILENAME, "new data");
	
/*	CWnd* hc;
	hc=GetDlgItem(IDC_Plot);
	if (Arr->GetSize() < 1) hc->EnableWindow(FALSE);
*/	
}

void CGasDlg::OnSave() 
{
	UpdateData();
	StripData();
	//CString Text = " Gas Profile along ITER NBI for BTR\n User Defined Data\n X, m\t\t  Density, m-3\n";
	CString Text;
	GetWindowText(Text);
	Text += "\n";
	CString S1, S2;
	GetDlgItemText(IDC_COLUMN1, S1);
	Text += S1 + "\t";
	GetDlgItemText(IDC_COLUMN2, S2);
	Text += S2 + "\n";
	
	char s[1024];
	int N = Arr->GetUpperBound();
	for (int i = 0; i <= N; i++) {
		sprintf(s, " %f\t %le\n", Arr->GetAt(i).X, Arr->GetAt(i).Y);
		Text += s;
	}

	CFileDialog dlg(FALSE, "dat;txt | * ", "Prof.txt", OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"Text file (*.dat); (*.txt) | *.dat; *.DAT; *.txt; *.TXT | All Files (*.*) | *.*||", NULL);
	
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

void CGasDlg::OnOK() 
{
	UpdateData();
	StripData();
	
	CDialog::OnOK();
}

void CGasDlg:: StripData()
{
	int k;
	char *buff,*nptr,*endptr;
	double z;
	
	Arr->RemoveAll();

	int Npoints=0;
	C3Point P;

	if((m_X.GetLength()==0) || (m_Y.GetLength()==0)) return;

	k = m_X.GetLength()*2;
	buff = (char *)calloc(k,sizeof(char));
	strcpy(buff, m_X);
	nptr = buff;
	while (sscanf(nptr,"%lf",&z)!=EOF){
		z = strtod(nptr,&endptr );
		
		if(nptr == endptr) nptr++; //catch special case
		else nptr = endptr;
	/*	if(Npoints>0){ // enforce monotonicity
			if (z <= Arr->GetAt(Npoints-1).X)
				break;
		}*/
		P.X = z; P.Y = 0; P.Z = 0;
		Arr->Add(P);
		Npoints++;
	}
	free(buff);

	Npoints = 0;
	k = m_Y.GetLength()*2;
	buff=(char *)calloc(k,sizeof(char));
	strcpy(buff, m_Y);
	nptr = buff;
	while (sscanf(nptr,"%le",&z)!=EOF){
		z = strtod(nptr,&endptr );
		if(nptr == endptr) nptr++;
		else nptr = endptr;
		
		P = Arr->GetAt(Npoints);
		P.Y = z;
		Arr->SetAt(Npoints, P);
		Npoints++;
	}
	free(buff);
}	

void CGasDlg::OnPlot() 
{
	UpdateData();
	StripData();

	CArray<double, double>  ArrX;
	CArray<double, double>  ArrY;
	double Xmax =0;
	
	PLOTINFO GasPlot;
	GasPlot.Caption = m_Caption;//"Gas Distribution along NBL";
	GasPlot.LabelX = m_Head1;//"X, m";
	GasPlot.LabelY = m_Head2;//"Gas Density, 1/m3";
	GasPlot.LimitX = 0;//25;
	GasPlot.LimitY = 0;//1.e20;
	GasPlot.CrossX = 0;// 5;
	GasPlot.CrossY = 0;// 1.e19;
	GasPlot.Line = 1;
	GasPlot.Interp = 1;
	GasPlot.N = Arr->GetSize();
	GasPlot.N1 = GasPlot.N +1;
	GasPlot.N2 = GasPlot.N +1;
	GasPlot.PosNegY = TRUE; //FALSE;
	GasPlot.PosNegX = FALSE;
	for (int i = 0; i < GasPlot.N; i++) {
		double x = Arr->GetAt(i).X;
		double y = Arr->GetAt(i).Y;
		ArrX.Add(x); ArrY.Add(y);
		Xmax = Max(Xmax, x);
	}
	GasPlot.DataX = &ArrX;
	GasPlot.DataY = &ArrY;
	GasPlot.LimitX = 0;// Xmax;
	
	CPlotDlg dlg;
	dlg.Plot = &GasPlot;
	dlg.InitPlot();
	dlg.DoModal();

	ArrX.RemoveAll();
	ArrY.RemoveAll();
	
}
