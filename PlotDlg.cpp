// PlotDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BTR.h"
#include "PlotDlg.h"
#include "config.h"
#include "BMP_utils.h"

#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPlotDlg dialog


CPlotDlg::CPlotDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPlotDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPlotDlg)
	m_Caption = _T("");
	//}}AFX_DATA_INIT
	Plot = NULL;
	ZOOM = 1;
	SHIFT_X = 0;
	SHIFT_Y = 0;


}


void CPlotDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPlotDlg)
	DDX_Text(pDX, IDC_CAPTION, m_Caption);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPlotDlg, CDialog)
	//{{AFX_MSG_MAP(CPlotDlg)
	ON_WM_PAINT()
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_BN_CLICKED(IDC_LEFT, OnLeft)
	ON_BN_CLICKED(IDC_RIGHT, OnRight)
	ON_BN_CLICKED(IDC_UP, OnUp)
	ON_BN_CLICKED(IDC_DOWN, OnDown)
	ON_BN_CLICKED(IDC_FIT, OnFit)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDOK, &CPlotDlg::OnBnClickedOk)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPlotDlg message handlers

void CPlotDlg::OnPaint() 
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

	//if (Plot->LimitX * Plot->LimitY < 1.e-12) InitPlot();

	DrawPlot(pDC, &rect);
		
	EndPaint(&pn);
	
	CWnd* hc;

/*	hc = GetDlgItem(IDOK);
	hc->SetWindowText("BACK");
	hc = GetDlgItem(IDCANCEL);
	hc->SetWindowText("CLOSE");*/

	//hc->EnableWindow(FALSE);
/*	bup = (CButton*)GetDlgItem(IDC_UP);
	bup->SetButtonStyle(BS_PUSHBUTTON, 1);
	bup->SetIcon(icon_UP);
*/
	

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


	// Do not call CDialog::OnPaint() for painting messages
}

void CPlotDlg:: DrawPlot(CDC* pDC, CRect * Bound)
{
	if (Plot == NULL) return;
	CFont font;
	font.CreateFont(-12, 0, 0, 0, 200, FALSE, FALSE, 0,
		ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
		DEFAULT_PITCH | FF_MODERN, "System");
	pDC->SelectObject(&font);

	SetDlgItemText(IDC_CAPTION, Plot->Caption);
	pDC->SetBkColor(RGB(240,250,210));
	pDC->SetBkMode(1);
	CPen DotPen;
	if (!DotPen.CreatePen(PS_DOT, 1, RGB(100,100,100)))	return;
	CPen * pOldPen;
	CString s;

	CPoint Lim;
	CRect plotrect;
	plotrect.bottom = Bound->bottom - 40;
	plotrect.top = Bound->top + 40;
	plotrect.left = Bound->left + 50;
	plotrect.right = Bound->right - 30;


	if (Plot->PosNegX)  // simmetrical
		Orig.x = SHIFT_X + plotrect.left + plotrect.Width()/2;
	else Orig.x = SHIFT_X + plotrect.left;
	if (Plot->PosNegY)  // simmetrical
		Orig.y = SHIFT_Y + plotrect.top + plotrect.Height()/2;
	else Orig.y = SHIFT_Y + plotrect.bottom;
	
//	Lim.x = plotrect.right;
//	Lim.y = plotrect.top;
	pDC->MoveTo(plotrect.left, Orig.y); pDC->LineTo(plotrect.right, Orig.y);
	pDC->MoveTo(Orig.x, plotrect.bottom); pDC->LineTo(Orig.x, plotrect.top);
	
	//double Xmin, Xmax;
	double Ymin, Ymax;
	//Xmin = 0; Xmax = Plot->LimitX;
	Ymin = 0; Ymax = Plot->LimitY;
	//if (Plot->PosNegX) Xmin = -Xmax;
	//if (Plot->PosNegY) Ymin = -Ymax;
		
	int Kx = (int)ceil((Xmax - Xmin)/ Plot->CrossX);
	int Ky = (int)ceil((Ymax - Ymin)/ Plot->CrossY);
	ScaleX = fabs((SHIFT_X + plotrect.right - Orig.x) *ZOOM / (Xmax - Xmin));
	ScaleY = fabs((SHIFT_Y + plotrect.top - Orig.y)/ Ymax); // ZOOM = 1!

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

	pDC->TextOut(Orig.x - sw/2, plotrect.top - sh - sh/2, Sord);
	LegendX =  Orig.x + sw/2 + 20;
	s.Format("%s", Plot->LabelY); 
	sw = pDC->GetTextExtent(s).cx;
	sh = pDC->GetTextExtent(s).cy;
	pDC->TextOut(LegendX, plotrect.top - sh - sh/2, s);
	LegendX =  LegendX + sw + 40;
	LegendY =  plotrect.top - sh;

	
	pOldPen = pDC->SelectObject(&DotPen);
	// grid
	for (i = 0; i <= Kx; i++) {
		x = i * Plot->CrossX; 
		ix = Orig.x + (int)((x) * ScaleX);
		pDC->MoveTo(ix, plotrect.bottom); pDC->LineTo(ix, plotrect.top);
	}
	if (Plot->PosNegX){
		for (i = 1; i <= Kx; i++) {
			x = -i * Plot->CrossX; 
			ix = Orig.x + (int)((x) * ScaleX);
			pDC->MoveTo(ix, plotrect.bottom); pDC->LineTo(ix, plotrect.top);
		}
	}
	for (j = 0; j <= Ky; j++) {
		y = j*Plot->CrossY;
		//if (y > Ymax) continue;
		jy = Orig.y - (int)(y * ScaleY);
		pDC->MoveTo(plotrect.left, jy); pDC->LineTo(plotrect.right, jy);
	}
	if (Plot->PosNegY) {
		for (j = 1; j <= Ky; j++) {
			y = -j*Plot->CrossY;
			//if (y > Ymax) continue;
			jy = Orig.y - (int)(y * ScaleY);
			pDC->MoveTo(plotrect.left, jy); pDC->LineTo(plotrect.right, jy);
		}
	}

	int t = 2; // line thickness
	CPen RedPen, GreenPen, BluePen;
	if (!RedPen.CreatePen(PS_SOLID, t, RGB(200,0,0)))	return;
	if (!GreenPen.CreatePen(PS_SOLID, t, RGB(0,200,0)))	return;
	if (!BluePen.CreatePen(PS_SOLID, t, RGB(0,0,200)))	return;
	
	// Draw Data Points / linear interp
	if (Plot->N <1) return;
	int plotN = 0; // Number of data series
	double xprev = Plot->DataX->GetAt(0);
	double yprev = Plot->DataY->GetAt(0);
	ix = Orig.x + (int)((xprev - Xmin) * ScaleX);
	jy = Orig.y - (int)(yprev * ScaleY);
	pDC->MoveTo(ix, jy);

/*	pDC->SelectObject(RedPen);
	pDC->Ellipse(LegendX -3, LegendY -3, LegendX +3, LegendY +3);
	LegendX += 20;
*/
	PlotLines(pDC);
	int r = 2; // circles radius
	int w = 8; // line piece for legend

	for (i = 0; i < Plot->N; i++) {
		x = Plot->DataX->GetAt(i);
		y = Plot->DataY->GetAt(i);
		ix = Orig.x + (int)((x - Xmin) * ScaleX);
		jy = Orig.y - (int)(y * ScaleY);
	//	if (x < xprev && Plot->Line > 0) {
		if (i == 0 || i == Plot->N1 || i == Plot->N2) {
			if (i == 0) plotN = 0;
			if (i == Plot->N1) plotN = 1;
			if (i == Plot->N2) plotN = 2;
			switch (plotN) {
			case 0: 
				pDC->SelectObject(RedPen);
				if (Plot->Line != 0) {
					pDC->MoveTo(LegendX - w, LegendY); pDC->LineTo(LegendX + w, LegendY);
				}
				if (x >= Xmin && x <= Xmax)
					pDC->Ellipse(LegendX - r, LegendY -r, LegendX +r, LegendY +r);
				LegendX += 30;
				break;
			case 1:
				pDC->SelectObject(GreenPen);
				if (Plot->Line != 0) {
					pDC->MoveTo(LegendX - w, LegendY); pDC->LineTo(LegendX + w, LegendY);
				}
				if (x >= Xmin && x <= Xmax)
					pDC->Ellipse(LegendX -r, LegendY -r, LegendX +r, LegendY +r);
				LegendX += 30;
				break;
			case 2:
				pDC->SelectObject(BluePen);
				if (Plot->Line != 0) {
					pDC->MoveTo(LegendX - w, LegendY); pDC->LineTo(LegendX + w, LegendY);
				}
				if (x >= Xmin && x <= Xmax)
					pDC->Ellipse(LegendX -r, LegendY -r, LegendX +r, LegendY +r);
			//	LegendX += 30;
				break;
			default: break;
			} //case
		}

	
		if (Plot->Interp != 0) pDC->Ellipse(ix-r, jy-r, ix+r, jy+r);
		xprev = x; 

	}
	pDC->SelectObject(pOldPen);
	
		
	// AXES
	for (i = 0; i <= Kx; i++) { 
		x = i*Plot->CrossX; 
		ix = Orig.x + (int)((x) * ScaleX);
		pDC->MoveTo(ix, Orig.y - 3); pDC->LineTo(ix, Orig.y + 3);
		s.Format("%g", (x + Xmin));
		//if (fabs(x + Xmin) <1.e-12) s.Format("0");
		sw = pDC->GetTextExtent(s).cx;
		pDC->TextOut(ix - sw/2, Orig.y +5, s);
	}
	for (j = 0; j <= Ky; j++) {
		y = Ymin + j*Plot->CrossY;
		jy = Orig.y - (int)(y * ScaleY);
		pDC->MoveTo(Orig.x-3, jy); pDC->LineTo(Orig.x+3, jy);
		s.Format("%g", y/order);
		//if (fabs(y) <1.e-12) s.Format("0");
		sw = pDC->GetTextExtent(s).cx; sh = pDC->GetTextExtent(s).cy;
		pDC->TextOut(Orig.x - sw - 5, jy - sh/2, s);
		
	}

	// label X
	s.Format("%s", Plot->LabelX);
	sw = pDC->GetTextExtent(s).cx; sh = pDC->GetTextExtent(s).cy;
	pDC->TextOut(plotrect.right +20 - sw , Orig.y - sh - sh/2, s);

}

void CPlotDlg::PlotLines(CDC* pDC)
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
	ix = Orig.x + (int)((xprev - Xmin) * ScaleX);
	jy = Orig.y - (int)(yprev * ScaleY);
	pDC->MoveTo(ix, jy);
	ixprev = ix; jyprev = jy;

	CPen * pOldPen = NULL; // = pDC->SelectObject(&RedPen);

	int plotN = 0;
	BOOL jump = FALSE;
	if (Plot->Interp == 1) {
		for (i = 0; i < Plot->N; i++) {
			jump = FALSE;
			x = Plot->DataX->GetAt(i);
			y = Plot->DataY->GetAt(i);
			ix = Orig.x + (int)((x - Xmin) * ScaleX);
			jy = Orig.y - (int)(y * ScaleY);
			//if (x < xprev && Plot->Line > 0) { 
			if (i == 0 || i == Plot->N1 || i == Plot->N2) {
				jump = TRUE;
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

			//if (x>xprev)
			if (jump == FALSE && x >= Xmin && x <= Xmax)
				pDC->LineTo(ix, jy);
			else pDC->MoveTo(ix, jy);
			xprev = x; 
		} //for
	} // if interp == 1

	if (Plot->Interp == 0) {
		for (i = 0; i < Plot->N; i++) {
			x = Plot->DataX->GetAt(i);
			y = Plot->DataY->GetAt(i);
			ix = Orig.x + (int)((x - Xmin) * ScaleX);
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


void CPlotDlg::OnClose() 
{
//	delete Plot;
//	Plot = NULL;
	
	CDialog::OnClose();
}

void CPlotDlg::OnDestroy() 
{
	CDialog::OnDestroy();
	
	//delete Plot;
	//Plot = NULL;
	// TODO: Add your message handler code here
	
}

void CPlotDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CRect rect;
	GetClientRect(&rect);
	rect.DeflateRect(10,50,10,50);
	if (PtInRect(rect, point)) {
		ZOOM = ZOOM * 1.5;
		InvalidateRect(rect, TRUE);
	}
	
	CDialog::OnLButtonDown(nFlags, point);
}

void CPlotDlg::OnRButtonDown(UINT nFlags, CPoint point) 
{
	CRect rect;
	GetClientRect(&rect);
	rect.DeflateRect(10,50,10,50);
	if (PtInRect(rect, point)) {
		ZOOM = ZOOM / 1.5;
		InvalidateRect(rect, TRUE);
	}
	
	CDialog::OnRButtonDown(nFlags, point);
}


void CPlotDlg::OnLeft() 
{
	SHIFT_X += 50;
	CRect rect;
	GetClientRect(&rect);
	rect.DeflateRect(10,50,10,50);
	InvalidateRect(rect, TRUE);
	
}

void CPlotDlg::OnRight() 
{
	SHIFT_X -= 50;
	CRect rect;
	GetClientRect(&rect);
	rect.DeflateRect(10,50,10,50);
	InvalidateRect(rect, TRUE);
}

void CPlotDlg::OnUp() 
{
	SHIFT_Y += 50;
	CRect rect;
	GetClientRect(&rect);
	rect.DeflateRect(10,50,10,50);
	InvalidateRect(rect, TRUE);
	
}

void CPlotDlg::OnDown() 
{
	SHIFT_Y -= 50;
	CRect rect;
	GetClientRect(&rect);
	rect.DeflateRect(10,50,10,50);
	InvalidateRect(rect, TRUE);
	
}

void CPlotDlg::OnFit() 
{
	SHIFT_X = SHIFT_Y = 0;
	ZOOM = 1;
	CRect rect;
	GetClientRect(&rect);
	rect.DeflateRect(10,50,10,50);
	InvalidateRect(rect, TRUE);
}


void CPlotDlg::InitPlot() // old - no min/max defined 
{
	if (Plot == NULL) return;
	C3Point P;
	double x,y;
	double xmax = 0, ymax =0;
	double limX = 0, limY = 0;
	double crossX, crossY;
	for (int i = 0; i < Plot->N; i++) {
		x = Plot->DataX->GetAt(i);
		xmax = Max(xmax, fabs(x));
		y = Plot->DataY->GetAt(i);
		ymax = Max(ymax, fabs(y));
	}
	if (Plot->LimitX < 1.e-12) {
		P = GetMantOrder(xmax * 1.1);
		limX = (P.X + P.Y) * P.Z;
		if (P.X > 3.5) crossX = 1 * P.Z;
		else crossX = limX/5;
		P = GetMantOrder(crossX);
		Plot->CrossX = P.X * P.Z;

		Plot->LimitX = limX;
	}

	if (Plot->LimitY < 1.e-20) {
		P = GetMantOrder(ymax * 1.05);
		limY = (P.X + P.Y) * P.Z;
		if (P.X > 3.5) crossY = 1 * P.Z;
		else crossY = limY/5;
		P = GetMantOrder(crossY);
		Plot->CrossY = P.X * P.Z;

		Plot->LimitY = limY;
	}
	Xmin = 0; Xmax = xmax;
}

void CPlotDlg::InitPlot(double xmin, double xmax, double step) // new - Xmin/Xmax defined
{
	if (Plot == NULL) return;
	C3Point P;
	double x,y;
	double ymax = 0;
	double limX = 0, limY = 0;
	double crossX, crossY;
	Xmin = xmin; 
	Xmax = xmax;

/*	CLimitsDlg dlg; // not called now
	dlg.m_StrMin = "Min";
	dlg.m_StrMax = "Max";
	dlg.m_Xmin = (float) Xmin;
	dlg.m_Xmax = (float) Xmax;
	if (dlg.DoModal() == IDOK) {
		Xmin = (double) dlg.m_Xmin;
		Xmax = (double) dlg.m_Xmax;
	} */

	for (int i = 0; i < Plot->N; i++) {
		//x = Plot->DataX->GetAt(i);
		//xmax = Max(xmax, fabs(x));
		y = Plot->DataY->GetAt(i);
		ymax = Max(ymax, fabs(y));
	}
	if (Plot->LimitX < 1.e-12) {
	P = GetMantOrder((Xmax - Xmin) * 1.05);
	limX = (P.X + P.Y) * P.Z;
	if (P.X > 7) crossX = 1 * P.Z;
	else crossX = limX/10;
	P = GetMantOrder(crossX);
	Plot->CrossX = P.X * P.Z;

	Plot->LimitX = Xmax;//limX;
	}

	if (Plot->LimitY < 1.e-20) {
	P = GetMantOrder(ymax * 1.05);
	limY = (P.X + P.Y) * P.Z;
	if (P.X > 3.5) crossY = 1 * P.Z;
	else crossY = limY/5;
	P = GetMantOrder(crossY);
	Plot->CrossY = P.X * P.Z;

	Plot->LimitY = limY;
	}
}

void CPlotDlg::OnBnClickedOk()
{
/*	HBITMAP bitmap = CaptureWindow(this->m_hWnd);

	CString name = "BTR_plot.bmp";
	CFileDialog dlg(FALSE, "bmp | * ", name, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"Store Image as BMP-file (*.bmp) | *.bmp; *.BMP | All Files (*.*) | *.*||", NULL);
	if (dlg.DoModal() != IDOK) return;
	name = dlg.GetPathName();
		
	SaveBitmap(bitmap, name);*/

	OnOK();
}
