// MultiPlotDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BTR.h"
#include "PlotDlg.h"
#include "MultiPlotDlg.h"
#include "config.h"
#include "BTRDoc.h"
#include "BMP_utils.h"
#include <math.h>
#include "LimitsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CArray<double, double> SumReiPowerX; /// array for Reion sum power deposited within StepX
extern CArray<double, double> SumAtomPowerX; /// array for Atom sum power deposited within StepX
extern double SumPowerStepX;// step for sum power
/////////////////////////////////////////////////////////////////////////////
// CMultiPlotDlg dialog
/*CMultiPlotDlg::CMultiPlotDlg(CDocument * doc){
	CDialog::CDialog(CMultiPlotDlg::IDD, NULL);
	pDoc = doc;
}*/

CMultiPlotDlg::CMultiPlotDlg(CWnd* pParent /*=NULL*/)
	//: CDialog(CMultiPlotDlg::IDD, pParent)
	: CPlotDlg(pParent)
{
	//CDialog::CDialog(CMultiPlotDlg::IDD, pParent);
	//{{AFX_DATA_INIT(CMultiPlotDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CMultiPlotDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMultiPlotDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMultiPlotDlg, CDialog)
	//{{AFX_MSG_MAP(CMultiPlotDlg)
		// NOTE: the ClassWizard will add message map macros here
		ON_WM_PAINT()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDOK, &CMultiPlotDlg::OnBnClickedOk)
END_MESSAGE_MAP()
/*
/////////////////////////////////////////////////////////////////////////////
// CMultiPlotDlg message handlers
// MultiPlotDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BTR.h"
#include "MultiPlotDlg.h"


// CMultiPlotDlg dialog

IMPLEMENT_DYNAMIC(CMultiPlotDlg, CDialog)

CMultiPlotDlg::CMultiPlotDlg(CWnd* pParent /*=NULL*//*)
	: CDialog(CMultiPlotDlg::IDD, pParent)
{

}

CMultiPlotDlg::~CMultiPlotDlg()
{
}

void CMultiPlotDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CMultiPlotDlg, CDialog)
END_MESSAGE_MAP()

*/
// CMultiPlotDlg message handlers

void CMultiPlotDlg::OnPaint() 
{
//	CPaintDC dc(this); // device context for painting

//	SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

	PAINTSTRUCT pn;
	CDC * pDC;
	CRect rect;
	pDC = BeginPaint(&pn);
//	CString S;
	
	//GetWindowRect(&rect);
	GetClientRect(&rect);
	rect.DeflateRect(10,50,10,50);
	//SizeTotal.x = rect.Width();
	//SizeTotal.y = rect.Height();

//	pDC->SetBkColor(RGB(255,255,255));

	CBrush YellowBrush;
	YellowBrush.CreateSolidBrush(RGB(240,250,210));
	CBrush * oldbrush = pDC->SelectObject(&YellowBrush);
	pDC->Rectangle(rect);
	pDC->SelectObject(oldbrush);
	
//	if (Plot->LimitX * Plot->LimitY < 1.e-12) InitPlot();
	
	//InitPlot();
	DrawPlot(pDC, &rect);

	EndPaint(&pn);
	
/*	CWnd* hc;

	hc = GetDlgItem(IDOK);
	hc->SetWindowText("BACK");
	hc = GetDlgItem(IDCANCEL);
	hc->SetWindowText("CLOSE");
	
	hc = GetDlgItem(IDC_LEFT);
	if (ZOOM > 1) hc->EnableWindow(TRUE);
	else hc->EnableWindow(FALSE);
	hc = GetDlgItem(IDC_RIGHT);
	if (ZOOM > 1) hc->EnableWindow(TRUE);
	else hc->EnableWindow(FALSE);
	hc = GetDlgItem(IDC_UP);
	if (ZOOM > 1) hc->EnableWindow(TRUE);
	else hc->EnableWindow(FALSE);
	hc = GetDlgItem(IDC_DOWN);
	if (ZOOM > 1) hc->EnableWindow(TRUE);
	else hc->EnableWindow(FALSE);
	*/

	// Do not call CDialog::OnPaint() for painting messages
}


void CMultiPlotDlg:: DrawPlot(CDC* pDC, CRect * Bound)
{
	//if (Plot == NULL) return;
	//CBTRDoc * doc = (CBTRDoc*) pDoc;

	CFont font;
	font.CreateFont(-12, 0, 0, 0, 200, FALSE, FALSE, 0,
		ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
		DEFAULT_PITCH | FF_MODERN, "System");
	pDC->SelectObject(&font);

	SetDlgItemText(IDC_CAPTION, "Sum power along X-coordinate ");
	pDC->SetBkColor(RGB(240,250,210));
	pDC->SetBkMode(1);
	CPen DotPen;
	if (!DotPen.CreatePen(PS_DOT, 1, RGB(100,100,100)))	return;
	CPen * pOldPen;
	CString s;

	//CPoint Lim;
	CRect plotrect;
	plotrect.bottom = Bound->bottom - 40;
	plotrect.top = Bound->top + 40;
	plotrect.left = Bound->left + 50;
	plotrect.right = Bound->right - 40;

	Orig.x = SHIFT_X + plotrect.left;
	Orig.y = SHIFT_Y + plotrect.bottom / 2;
	
//	Lim.x = plotrect.right;
//	Lim.y = plotrect.top;
	pDC->MoveTo(plotrect.left, Orig.y); pDC->LineTo(plotrect.right, Orig.y);
	pDC->MoveTo(Orig.x, Orig.y); pDC->LineTo(Orig.x, plotrect.top);
	
	int Kx = (int)ceil((Xmax - Xmin)/ CrossX);
	int Ky = (int)ceil(Ymax / CrossY);
	ScaleX = fabs((SHIFT_X + plotrect.right - Orig.x) * ZOOM / (Xmax - Xmin));
	ScaleY = fabs((SHIFT_Y + plotrect.top - Orig.y) * ZOOM / Ymax);
	ScaleZ = fabs((SHIFT_Y + plotrect.top - Orig.y) * ZOOM / Zmax);

	// LABELS	
	int pos;
	CString S1, Sord;
	double order;

	int i,j;
	int ix, jy;
	double x,y;
	long sw, sh;
	int LegendX, LegendY;

	// Upper
	S1.Format("%le", Ymax);
	pos = S1.Find("e",0);
	if (pos>0) s = "1.0" + S1.Mid(pos);
	order = atof(s);
	Sord = S1.Mid(pos);
	Sord.MakeUpper();
	if (order == 1.0) Sord = "";
	sw = pDC->GetTextExtent(Sord).cx;
	sh = pDC->GetTextExtent(Sord).cy;

	pDC->TextOut(Orig.x, plotrect.top - sh - sh/2, Sord);
	LegendX =  Orig.x + sw/2 + 50;
	s.Format("%s", "Power, W"); 
	sw = pDC->GetTextExtent(s).cx;
	sh = pDC->GetTextExtent(s).cy;
	pDC->TextOut(LegendX, plotrect.top - sh - sh/2, s);
	LegendX =  LegendX + sw + 40;
	LegendY =  plotrect.top - sh;

	
	pOldPen = pDC->SelectObject(&DotPen);
	// grid
	for (i = 0; i < Kx; i++) {
		x = Xmin + i*CrossX; 
		ix = Orig.x + (int)(i*CrossX * ScaleX);
		pDC->MoveTo(ix, Orig.y); pDC->LineTo(ix, plotrect.top);
	}
	for (j = 0; j < Ky; j++) {
		y = Ymin + j*CrossY;
		jy = Orig.y - (int)(j*CrossY * ScaleY);
		pDC->MoveTo(plotrect.left, jy); pDC->LineTo(plotrect.right, jy);
	}

	int t = 2; // line thickness
	//CPen RedPen, GreenPen, BluePen;
	CPen pen, GreenPen;
	COLORREF color = plotcolor;
	if (!pen.CreatePen(PS_SOLID, t, color))	return;
	
	int r = 3; // circles radius
	// int w = 8; // line piece for legend

// Draw Data Points / linear interp

	pDC->SelectObject(pen);
	//int plotN = 0; // Number of data series
	x = Xmin;
	int k = (int)(Xmin / SumPowerStepX);
	y = DataY->ElementAt(k); //SumReiPowerX[k]; // W
	ix = Orig.x + (int)((x-Xmin) * ScaleX);
	jy = Orig.y - (int)((y-Ymin) * ScaleY);
	pDC->MoveTo(ix, jy);
	pDC->Ellipse(ix-r, jy-r, ix+r, jy+r);
	
	//PlotLines(pDC);

	while (x < Xmax && k < DataY->GetUpperBound()) {
		x += SumPowerStepX;
		k++;
		y = DataY->ElementAt(k); // W
		ix = Orig.x + (int)((x - Xmin) * ScaleX);
		jy = Orig.y - (int)((y - Ymin) * ScaleY);
		pDC->LineTo(ix, jy);
		pDC->Ellipse(ix-r, jy-r, ix+r, jy+r);
		//xprev = x; 

	}
	pDC->SelectObject(pOldPen);
	
	
	// AXES
	for (i = 0; i <= Kx; i++) { 
		x = Xmin + i*CrossX; 
		ix = Orig.x + (int)(i*CrossX * ScaleX);
		pDC->MoveTo(ix, Orig.y - 3); pDC->LineTo(ix, Orig.y + 3);
		s.Format("%g", x);
		if (fabs(x) <1.e-12) s.Format("0");
		sw = pDC->GetTextExtent(s).cx;
		pDC->TextOut(ix - sw/2, Orig.y +5, s);
	}
	for (j = 0; j <= Ky; j++) {
		y = Ymin + j*CrossY;
		jy = Orig.y - (int)(j*CrossY * ScaleY);
		pDC->MoveTo(Orig.x-3, jy); pDC->LineTo(Orig.x+3, jy);
		s.Format("%g", y/order);
		if (fabs(y) <1.e-12) s.Format("0");
		sw = pDC->GetTextExtent(s).cx; sh = pDC->GetTextExtent(s).cy;
		pDC->TextOut(Orig.x - sw - 5, jy - sh/2, s);
		
	}

	// label X
	s.Format("%s", "X, m");
	sw = pDC->GetTextExtent(s).cx; sh = pDC->GetTextExtent(s).cy;
	pDC->TextOut(plotrect.right +20 - sw , Orig.y - sh - sh/2, s);

	// diaphragms positions
	if (free) return; // no dias

	if (!GreenPen.CreatePen(PS_DASH, 2, RGB(0,150,0)))	return;
//	if (!BluePen.CreatePen(PS_SOLID, t, RGB(0,0,200)))	return;
	pDC->SelectObject(GreenPen);
	pDC->SetTextColor(RGB(0,150,0));

	int DiaMax = 11;
	int h = 50;
	pDC->TextOut(Orig.x-20, Orig.y + h + 2, "Dia");

	for (i = 0; i < DiaMax; i++)
	{
		x = Xdia[i];
		if (x < Xmin) continue;
		ix = Orig.x + (int)((x - Xmin) * ScaleX);
		pDC->MoveTo(ix, Orig.y + 2); pDC->LineTo(ix, Orig.y + h);
		s.Format("%d", i + 5);
		pDC->TextOut(ix-5, Orig.y + h + 2, s);
	}

	pDC->SelectObject(pOldPen);
	
	int limD = plotrect.right - Orig.x;
	int CrossD = limD / (DiaMax - 1);

	// AXES for Sum Plot
	pDC->MoveTo(Orig.x, plotrect.bottom); pDC->LineTo(Orig.x + limD, plotrect.bottom);
	for (i = 0; i <= DiaMax; i++) { 
		x = i*CrossD; 
		ix = Orig.x + (int)x;
		pDC->MoveTo(ix, plotrect.bottom - 3); 
		pDC->LineTo(ix, plotrect.bottom + 3);
		s.Format("%d", i+5);
		sw = pDC->GetTextExtent(s).cx;
		pDC->TextOut(ix - sw/2, plotrect.bottom + 5, s);
	}
	s.Format("%s", "Dia");
	sw = pDC->GetTextExtent(s).cx; sh = pDC->GetTextExtent(s).cy;
	pDC->TextOut(Orig.x - 40, plotrect.bottom + 5, s);
	
	/*pDC->MoveTo(Orig.x, plotrect.bottom); pDC->LineTo(Orig.x, Orig.y + h);
	for (j = 0; j <= Ky; j++) {
		y = Ymin + j*CrossY;
		jy = plotrect.bottom - (int)(j*CrossY * ScaleY);
		pDC->MoveTo(Orig.x-3, jy); pDC->LineTo(Orig.x+3, jy);
		s.Format("%g", y/order);
		if (fabs(y) <1.e-12) s.Format("0");
		sw = pDC->GetTextExtent(s).cx; sh = pDC->GetTextExtent(s).cy;
		pDC->TextOut(Orig.x - sw - 5, jy - sh/2, s);
		
	}*/
	
	pDC->SetTextColor(RGB(0,0,250));

	// draw power bars
	CBrush brush;
	brush.CreateSolidBrush(RGB(200,200,200));
	CBrush * oldbrush = pDC->SelectObject(&brush);
	//pDC->Rectangle(rect);
	
	for (i = 0; i < DiaMax-1; i++) { 
		x = i*CrossD; 
		ix = Orig.x + i*CrossD;
		y = PowerAtDia[i];
		jy = (int)(y * ScaleZ);
		pDC->Rectangle(ix+1, plotrect.bottom, ix + CrossD - 1, plotrect.bottom - jy);
		s.Format("%7g", y);
		sw = pDC->GetTextExtent(s).cx;
		pDC->TextOut(ix+2, plotrect.bottom - jy - 20, s);
	}
	pDC->SelectObject(oldbrush);
}

void CMultiPlotDlg::PlotLines(CDC* pDC)
{
	int i;
	int ix, jy, ixprev, jyprev;
	double x,y;
//	pDC->SelectObject(RedPen);
	if (Plot->Line == 0) return;
	//CPen Pen;//Pen.CreatePen(PS_SOLID, 1, RGB(0,0,0));
	CPen RedPen, GreenPen, BluePen;
	if (Plot->Line == 1) {
		if (!RedPen.CreatePen(PS_SOLID, 2, RGB(255,0,0)))	return;
		if (!GreenPen.CreatePen(PS_SOLID, 2, RGB(0,255,0)))	return;
		if (!BluePen.CreatePen(PS_SOLID, 2, RGB(0,0,255)))	return;
	}
	if (Plot->Line == 2) {
		if (!RedPen.CreatePen(PS_DOT, 2, RGB(255,0,0)))	    return;
		if (!GreenPen.CreatePen(PS_DOT, 2, RGB(0,255,0)))	return;
		if (!BluePen.CreatePen(PS_DOT, 2, RGB(0,0,255)))	return;
	}
//	CPen * pOldPen = pDC->SelectObject(&Pen);


	double xprev = Plot->DataX->GetAt(0);
	double yprev = Plot->DataY->GetAt(0);
	ix = Orig.x + (int)(xprev * ScaleX);
	jy = Orig.y - (int)(yprev * ScaleY);
	pDC->MoveTo(ix, jy);
	ixprev = ix; jyprev = jy;

	CPen * pOldPen = NULL; // = pDC->SelectObject(&RedPen);

	int plotN = 0;
	if (Plot->Interp == 1) {
		for (i = 0; i < Plot->N; i++) {
			x = Plot->DataX->GetAt(i);
			y = Plot->DataY->GetAt(i);
			ix = Orig.x + (int)(x * ScaleX);
			jy = Orig.y - (int)(y * ScaleY);
			//if (x < xprev && Plot->Line > 0) { 
			if (i == 0 || i == Plot->N1 || i == Plot->N2) {
				if (i == 0) plotN = 0;
				if (i == Plot->N1) plotN = 1;
				if (i == Plot->N2) plotN = 2;
			switch (plotN) {
			case 0: 
				pOldPen = pDC->SelectObject(&RedPen);
				break;
			case 1:
				pDC->SelectObject(GreenPen);
				break;
			case 2:
				pDC->SelectObject(BluePen);
				break;
			default: break;
			} //case
			} // of

		/*	if (i == N1 || i == N2) {
				plotN++;
				if (plotN == 1) pDC->SelectObject(GreenPen);
				else pDC->SelectObject(BluePen);
			}
		*/
			if (x>xprev) pDC->LineTo(ix, jy);
			else pDC->MoveTo(ix, jy);
			xprev = x; 
		} //for
	} // if interp == 1

	if (Plot->Interp == 0) {
		for (i = 0; i < Plot->N; i++) {
			x = Plot->DataX->GetAt(i);
			y = Plot->DataY->GetAt(i);
			ix = Orig.x + (int)(x * ScaleX);
			jy = Orig.y - (int)(y * ScaleY);
		//	if (x < xprev && Plot->Line > 0) { 
			if (i == 0 || i == Plot->N1 || i == Plot->N2) {
				if (i == 0) plotN = 0;
				if (i == Plot->N1) plotN = 1;
				if (i == Plot->N2) plotN = 2;
			switch (plotN) {
			case 0: 
				pOldPen = pDC->SelectObject(&RedPen);
				break;
			case 1:
				pDC->SelectObject(GreenPen);
				break;
			case 2:
				pDC->SelectObject(BluePen);
				break;
			default: break;
			} //case
			} // if

/*
			if (i == N1 || i == N2) {
				plotN++;
				if (plotN == 1) pDC->SelectObject(GreenPen);
				else pDC->SelectObject(BluePen);
			}
*/		
			if (x>=xprev){
				pDC->LineTo(ix, jyprev);
				if ((x - xprev)*ScaleX >= 10)
				pDC->LineTo(ix, Orig.y);
				pDC->LineTo(ix, jy);
			}

			else {
				pDC->LineTo(ixprev, Orig.y);
				pDC->MoveTo(ix, jy);
			}
			xprev = x; 
			ixprev = ix;
			jyprev = jy;
		} // for
	}

/*	if (Plot->Interp == 0) {
		SetSlopes();
		int k, kmax = 10; 
		double X1, X2, Y1, Y2, dx;
		for (i = 0; i < Plot->N-1; i++) {
			X1 = Plot->DataX->GetAt(i);
			Y1 = Plot->DataY->GetAt(i);
			X2 = Plot->DataX->GetAt(i+1);
			Y2 = Plot->DataY->GetAt(i+1);
			dx = (X2 - X1)/kmax;
			for (int k = 1; k <= kmax; k++) {
				x = X1 + k*dx;
				y = Y1
			ix = Orig.x + (int)(x * ScaleX);
			jy = Orig.y - (int)(y * ScaleY);
			if (x < xprev && Plot->Line > 0) { 
				plotN++;
				if (plotN == 1) pDC->SelectObject(GreenPen);
				else pDC->SelectObject(BluePen);
			}
		
			if (x>xprev) pDC->LineTo(ix, jy);
			else pDC->MoveTo(ix, jy);
			xprev = x; 
		}
	}*/
	pDC->SelectObject(pOldPen);

}

void CMultiPlotDlg::InitPlot(CDocument * pDoc)
{
//	if (Plot == NULL) return;
	CBTRDoc * pdoc = (CBTRDoc*) pDoc;
	plotcolor = RGB(0,0,0);
	free = pdoc->OptFree;
	
	Xdia = new double[25];
	Xdia[0] = pdoc->PreDuctX; //dia 5
	Xdia[1] = pdoc->DDLinerInX; //dia 6
	Xdia[2] = pdoc->DDLinerOutX; //dia 7
	Xdia[3] = pdoc->Duct1X; //dia 8
	Xdia[4] = pdoc->Duct2X; //dia 9
	Xdia[5] = pdoc->Duct3X; //dia 10
	Xdia[6] = pdoc->Duct4X; //dia 11
	Xdia[7] = pdoc->Duct5X; //dia 12
	Xdia[8] = pdoc->Duct6X; //dia 13
	Xdia[9] = pdoc->Duct7X; //dia 14
	Xdia[10] = pdoc->Duct8X; //dia 15

	int i;
	PowerAtDia = new double[100];
	int imax = (int)(pdoc->AreaLong);
	for (i = 0; i < imax; i++) {
		PowerAtDia[i] = 0;
	}

	int kmax = DataY->GetUpperBound();
	double X, powerX;
	double MaxPower = 3.E+4, MaxPowerDia = 3.E+5;
	for (int k = 0; k < kmax; k++) 
	{
		X = SumPowerStepX * k;
		if (X < Xdia[0]) continue; 
		powerX = DataY->ElementAt(k);//SumReiPowerX[k];
		MaxPower = Max(MaxPower, powerX);
		for (i = 0; i < imax; i++) {
			if (X >= Xdia[i] && X < Xdia[i+1]) {
				PowerAtDia[i] += powerX;
				MaxPowerDia = Max(MaxPowerDia, PowerAtDia[i]);
				break;
			} // if
		} // for i
	} // for k

	Xmin = pdoc->ReionXmin; 
	Xmax = pdoc->AreaLong;//Xmax = *(pdoc->Duct8X);
	CLimitsDlg dlg;
	dlg.m_StrMin = "Xmin";
	dlg.m_StrMax = "Xmax";
	dlg.m_Sstep = "Step (Const)";
	dlg.m_Xmin = (float) Xmin;
	dlg.m_Xmax = (float) Xmax;
	dlg.m_Step = 1;
	if (dlg.DoModal() == IDOK) {
		Xmin = (double) dlg.m_Xmin;
		Xmax = (double) dlg.m_Xmax;
	}

	C3Point P;
	double x,y;
	
	P = GetMantOrder(Xmin);
	//Xmin = P.X * P.Z;
	Ymin = 0;   
	Ymax = MaxPower;
	if (Ymax < 1.e-12) Ymax = 1; 
	Zmax = MaxPowerDia;// * 1.2;
	if (Zmax < 1.e-12) Zmax = 1; 
		
	P = GetMantOrder((Xmax - Xmin) * 1.05);
	LimX = (P.X + P.Y) * P.Z;
	CrossX = LimX/10;
	P = GetMantOrder(CrossX);
	CrossX = P.X * P.Z;

	P = GetMantOrder(Ymax);
	LimY = (P.X + P.Y) * P.Z;
	CrossY = LimY/5;
	P = GetMantOrder(CrossY);
	CrossY = P.X * P.Z;
}


void CMultiPlotDlg::OnBnClickedOk()
{
/*	HBITMAP bitmap = CaptureWindow(this->m_hWnd);

	CString name = "BTR_Mplot.bmp";
	CFileDialog dlg(FALSE, "bmp | * ", name, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"Store Image as BMP-file (*.bmp) | *.bmp; *.BMP | All Files (*.*) | *.*||", NULL);
	if (dlg.DoModal() != IDOK) return;
	name = dlg.GetPathName();
		
	SaveBitmap(bitmap, name);*/

	OnOK();
}
