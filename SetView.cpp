// SetView.cpp : implementation file
//

#include "stdafx.h"
#include "BTR.h"
#include "SetView.h"
#include  "config.h"
#include <math.h>
#include "BTRDoc.h"
#include "BMP_utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CSetView * pSetView;
extern int NofCalculated;

CTimeSpan GetElapse(CTime t0, CTime t1) 
{
	CTimeSpan  dt= t1- t0;
	return dt;
}
/////////////////////////////////////////////////////////////////////////////
// CSetView

IMPLEMENT_DYNCREATE(CSetView, CScrollView)

CSetView::CSetView()
{
	pSetView = this;

/*	BOOL CreateFont( int nHeight, int nWidth, int nEscapement, int nOrientation, 
					int nWeight, BYTE bItalic, BYTE bUnderline, BYTE cStrikeOut, 
					BYTE nCharSet, BYTE nOutPrecision, BYTE nClipPrecision, BYTE nQuality, 
					BYTE nPitchAndFamily, LPCTSTR lpszFacename );
*/
	font.CreateFont(-16, 0, 0, 0, 100, FALSE, FALSE, 0,
		0, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
		DEFAULT_PITCH | FF_MODERN, "System");

	smallfont.CreateFont(-12, 0, 0, 0, 400, FALSE, FALSE, 0,
		ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
		DEFAULT_PITCH | FF_MODERN, "Arial");//"Courier New");

/*	font.CreateFont(-12, 0, 0, 0, 100, FALSE, FALSE, 0,
		ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
		DEFAULT_PITCH | FF_ROMAN, "Times New Roman Cyr");*/
}

CSetView::~CSetView()
{
}


BEGIN_MESSAGE_MAP(CSetView, CScrollView)
	//{{AFX_MSG_MAP(CSetView)
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSetView drawing

void CSetView::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();
	
	CSize sizeTotal;
	// TODO: calculate the total size of this view
	sizeTotal.cx = 100; sizeTotal.cy = 1000;
	SetScrollSizes(MM_TEXT, sizeTotal);

	Xmax = 1; Ymax = 1; Zmax = 1;
	double Fi = PI * 40/180;
	double Teta = PI * 20/180;
	SinFi = sin(Fi); CosFi = cos(Fi);
	SinTeta = sin(Teta); CosTeta = cos(Teta);
	Load = NULL;
	ShowProf = FALSE;

}
void CSetView::OnSize(UINT nType, int cx, int cy) 
{
	CScrollView::OnSize(nType, cx, cy);
	ShowProf = FALSE;
/*	CRect rect;
	GetClientRect(rect);
	UpdateScales(rect);
	STOP = FALSE;*/
}
void CSetView::OnDraw(CDC* pDC)
{
	//AfxMessageBox("CSetView OnDraw");
	//STOP = FALSE;
/*	CPen pen;
	pen.CreatePen(PS_SOLID, 1, RGB(200,200,200));
	CPen * pOldPen = pDC->SelectObject(&pen);
	CRect rect;
	GetClientRect(rect);
	pDC->Rectangle(rect);*/
	CBTRDoc* pDoc = (CBTRDoc*)GetDocument();
	//bool ShowProf = pDoc->ShowProfiles;
	ShowProf = TRUE;
	if (pDoc->pMarkedPlate == NULL || !(pDoc->ShowProfiles)) ShowProf = FALSE; 
	//if (pDoc->ShowProfiles && Load != NULL) ShowProfiles(); 
	if (ShowProf) ShowProfiles(); 
	else ShowStatus();

	return;

////////////////////////////////////////////////////////
/*
	rect.DeflateRect(30, 50, 30, 20);
	UpdateScales(rect);
	pDC->SelectObject(pOldPen);
	CBTRDoc* pDoc = (CBTRDoc*)GetDocument();
	// CPlate * plate = pDoc->pMarkedPlate;
	
	//if (pDoc->OptCombiPlot == -1) Load = NULL;
	//else	Load = plate->Load;

	if (Load != NULL && Load->MaxVal > 1.e-6) {
		if (pDoc->ShowProfiles ) {
			//ScaleX = rect.Size().cx / Xmax;
			//ScaleY = rect.Size().cx / Ymax;
			OrigX = rect.left;
			ymin1 =  Height / 2 + 5;
			ymin2 =  Height + 25;
			//OrigY = rect.bottom;
			//DrawLoadProfiles(pDC);
			
			if (Load->iProf >= 0 && Load ->jProf >= 0) ShowLoadProfiles(pDC);
			else ShowIntegralProfiles(pDC);
			ShowGlobalScale(pDC);
			//ShowLocalScale(pDC);
			//Load->DrawLoadProfiles(this, pDC);
		//	ShowStatus(pDC);
		}// Profiles

		else {
			BeginWaitCursor();
			
			ShowCoord(pDC); 	
			Show3DLoad(pDC);
			EndWaitCursor();
		}// 3D

		
	}// Load != NULL

	else ShowStatus(pDC); 
	//pDC->SelectObject(pOldPen);
	//Load = NULL;
*/
}
void CSetView:: ShowProfiles()
{
	CDC* pDC = GetDC();
	STOP = FALSE;
	CPen pen;
	pen.CreatePen(PS_SOLID, 1, RGB(200,200,200));
	CPen * pOldPen = pDC->SelectObject(&pen);
	CRect rect;
	GetClientRect(rect);
	pDC->Rectangle(rect);
	rect.DeflateRect(30, 50, 30, 20);
	UpdateScales(rect);
	pDC->SelectObject(pOldPen);
	CBTRDoc* pDoc = (CBTRDoc*)GetDocument();
	// CPlate * plate = pDoc->pMarkedPlate;

	if (Load != NULL && Load->MaxVal > 1.e-6) {
		//if (pDoc->ShowProfiles ) {
			OrigX = rect.left;
			ymin1 =  Height / 2 + 5;
			ymin2 =  Height + 25;
			
			if (Load->iProf >= 0 && Load ->jProf >= 0) 
				ShowLoadProfiles(pDC);
			else ShowIntegralProfiles(pDC);

			ShowGlobalScale(pDC);
			//ShowLocalScale(pDC);
			//Load->DrawLoadProfiles(this, pDC);
	}// Profiles
	
	ReleaseDC(pDC);
}

void CSetView:: DrawLoadProfiles(CDC* pDC)// not used
{
	//CClientDC dc(this);
	//OnPrepareDC(&dc);
	//CDC* pDC = &dc;
	//if (Load == NULL) return;
	//if (Load->Nx * Load->Ny == 0) return;
	//MSG message;
	CFont* pOldFont = pDC->SelectObject(&smallfont);
	pDC->SetTextColor(RGB(0, 0, 0));
	CBrush brush1, brush2;
	CBrush * oldbrush;
	CPen Pen;
	CPen * pOldPen;
	if(!Pen.CreatePen(PS_SOLID, 2, RGB(255,0,0))) return;
	brush1.CreateSolidBrush(RGB(255,220,255));
	brush2.CreateSolidBrush(RGB(220,255,255));
//	pDC->SetBkMode(1);
	int x, y;
	int i, j;
	double ix, jy;
	double val;
	int xmin = 20;
	int ymin1 =  Height  / 2 +5;
	int ymin2 =  Height + 25;

	RectArrayX.RemoveAll();
	RectArrayY.RemoveAll();

	Load->MaxProf = 0;
	
	for (i = 0; i <= Load->Nx; i++) {
		val = Load->Val[i][Load->jProf];
		if (val >= Load->MaxProf) Load->MaxProf = val;
	}
	for (j = 0; j <= Load->Ny; j++) {
		val = Load->Val[Load->iProf][j];
		if (val >= Load->MaxProf) Load->MaxProf = val;
	}
	double scaleX = Width / Xmax; //ScaleX * 1.5;
	double scaleY = Width / Ymax; //ScaleY;
	double scaleZ =  Height* 0.4 / Load->MaxProf;
	CString S;
	int pos, pos1;
	CString S1, S2, S3;
	double order, mant;
	
	 S1.Format("%8.1e", Load->MaxProf);
	 pos = S1.Find("e",0);
	  pos1 = S1.Find(".",0);
	 if (pos>0) S = "1.0" + S1.Mid(pos);
	 order = atof(S);
	 S = S1.Mid(pos);
	 S.MakeUpper();

	 S2 = S1.Left(pos);
	 mant = atof(S2);
	 S3 = S1.Mid(pos1, 1);
	 if (atof(S3) > 5.0) mant += 1.;
	 Load->CrossZ = 0.5*order;
	 if (mant > 3.5) Load->CrossZ = order;
	  if (mant > 6) Load->CrossZ = 2*order;

	int xmax1 = xmin +  (int)(Xmax * scaleX);
	int xmax2 = xmin +  (int)(Ymax * scaleY);
	int ymax1 = ymin1 - (int)ceil(mant*order * scaleZ);
	int ymax2 = ymin2 - (int)ceil(mant*order * scaleZ);
	
	oldbrush = pDC->SelectObject(&brush1);
	pDC->Rectangle(xmin, ymin1, xmax1, ymax1);
	pDC->SelectObject(&brush2);
	pDC->Rectangle(xmin, ymin2, xmax2, ymax2);
	//pDC->SelectObject(oldbrush);

	pDC->TextOut(xmin-10, ymin1, "0");
	pDC->TextOut(xmin-10, ymin2, "0");
	pDC->TextOut(xmin-10, ymax1-15, S);
	pDC->TextOut(xmin-10, ymax2-15, S);
	S = "HORIZONTAL dist.";
	int w = pDC->GetTextExtent(S).cx;
	pDC->TextOut((xmin + xmax1) / 2 - w/2, ymin1+15, S);
	S = "VERTICAL dist.";
	w = pDC->GetTextExtent(S).cx;
	pDC->TextOut((xmin + xmax2) / 2 - w/2, ymin2+15, S);
	//pDC->TextOut(xmax1+5, ymin1-10, "X");
	//pDC->TextOut(xmax2+5, ymin2-10, "Y");
	S = "POWER PROFILES";
	w = pDC->GetTextExtent(S).cx;
	pDC->TextOut((xmin + xmax1) / 2 - w/2, 5, S);

	int icross1 = (int)(Xmax / (Load->CrossX));
	int jcross = (int)(mant*order / (Load->CrossZ));

////////// upper magenta
	for ( i = 1; i <= icross1; i++) {
		 x = xmin + (int)(i *(Load->CrossX) * scaleX);
		 y = ymin1; pDC->MoveTo(x,y);
		 ix = i*(Load->CrossX);
		 S.Format("%g", ix);
		 /*if (CrossX < 10)  S.Format("%0.0f", ix);
		 if (CrossX < 1)  S.Format("%0.1f", ix);
		 if (CrossX < 0.1)  S.Format("%0.2f", ix);
		 if (CrossX < 0.01)  S.Format("%0.3f", ix);*/
		 pDC->TextOut(x-5, y, S);
		 y = ymax1; pDC->LineTo(x,y);		 
	}
	for ( j = 1; j <= jcross; j++) {
		 y = ymin1 - (int)(j*(Load->CrossZ) * scaleZ);
		 x = xmin; pDC->MoveTo(x,y);
		 jy = j*(Load->CrossZ);
		 S.Format("%g", jy/order);
		 /*if (CrossZ/order < 10)  S.Format("%0.0f", jy/order);
		 if (CrossZ/order < 1)  S.Format("%0.1f", jy/order);*/
		 pDC->TextOut(x-15, y-5, S);
		 x = xmax1; pDC->LineTo(x,y);		
	}

//////////// lower blue
		int icross2 = (int)(Ymax / Load->CrossY);
		for ( i = 1; i <= icross2; i++) {
		 x = xmin + (int)(i*(Load->CrossY)* scaleY);
		 y = ymin2; pDC->MoveTo(x,y);
		 ix = i*(Load->CrossY);
		 S.Format("%g", ix);
		/* if (CrossY < 10)  S.Format("%0.0f", ix);
		 if (CrossY < 1)  S.Format("%0.1f", ix);
		 if (CrossY < 0.1)  S.Format("%0.2f", ix);
		 if (CrossY < 0.01)  S.Format("%0.3f", ix);*/
		 pDC->TextOut(x-5, y, S);
		 y = ymax2; pDC->LineTo(x,y);		 
	}
	for ( j = 1; j <= jcross; j++) {
		 y = ymin2 - (int)(j*(Load->CrossZ)* scaleZ);
		 x = xmin; pDC->MoveTo(x,y);
		 jy = j*Load->CrossZ;
		 S.Format("%g", jy/order);
		/* if (CrossZ/order < 10)  S.Format("%0.0f", jy/order);
		 if (CrossZ/order < 1)  S.Format("%0.1f", jy/order);*/
		 pDC->TextOut(x-15, y-5, S);
		 x = xmax2; pDC->LineTo(x,y);		 
	}
///////////////////////////////////////////////////////////
	pDC->MoveTo(xmin, ymin1);
	for (i = 0; i <= Load->Nx; i++) {
		x = xmin + (int) (i*(Load->StepX) * scaleX);
		y = ymin1 - (int)(Load->Val[i][Load->jProf] * scaleZ);
		pDC->LineTo(x, y);
	}
	pDC->MoveTo(xmin, ymin2);
	for (j = 0; j <= Load->Ny; j++) {
		x = xmin + (int) (j*(Load->StepY)* scaleY);
		y = ymin2 - (int)(Load->Val[Load->iProf][j] * scaleZ);
		pDC->LineTo(x, y);
	}

	CRect rect;
	
	pOldPen = pDC->SelectObject(&Pen);

	for (i = 0; i <= Load->Nx; i++) {
		x = xmin + (int) (i*Load->StepX* scaleX);
		y = ymin1 - (int)(Load->Val[i][Load->jProf] * scaleZ);
		pDC->Ellipse(x-3, y-3, x+3, y+3);
		rect = new CRect();
		rect.SetRect(x-3, y-3, x+3, y+3);
		RectArrayX.Add(rect);
		
	}
	S.Format("Vert.Dist = %0.2f", Load->jProf*Load->StepY);
	pDC->TextOut(xmax1 - 80, ymax1 - 20, S);

	for (j = 0; j <= Load->Ny; j++) {
		x = xmin + (int) (j*Load->StepY* scaleY);
		y = ymin2 - (int)(Load->Val[Load->iProf][j] * scaleZ);
		pDC->Ellipse(x-3, y-3, x+3, y+3);
		rect = new CRect();
		rect.SetRect(x-3, y-3, x+3, y+3);
		RectArrayY.Add(rect);
		
	}
	S.Format("Hor.Dist = %0.2f", Load->iProf*Load->StepX);
	pDC->TextOut(xmax2 - 80, ymax2 - 20, S);
	
	pDC->SelectObject(pOldPen);
	pDC->SelectObject(pOldFont);
	pDC->SelectObject(oldbrush);

	//GdiFlush();

	}

void CSetView:: ShowGlobalScale(CDC* pDC)
{
	CBTRDoc* pDoc = (CBTRDoc*)GetDocument();
	CPlate * plate = pDoc->pMarkedPlate;
	C3Point Local, Global;
	//pDC->SetROP2(R2_NOT);
	pDC->SelectObject(&smallfont);
	int x, y;
	//int xmin = 20;
	int xmax1 = OrigX +  (int)(Xmax * ScaleX);
	int xmax2 = OrigX +  (int)(Ymax * ScaleY);
	int xmin = OrigX;

	CString S, Shor, Svert;
	double StepH = 1;
	if (plate->Xmax < 0.3) StepH = 0.05;
	if (plate->Xmax < 0.1) StepH = 0.01;
	if (plate->Xmax < 0.03) StepH = 0.005;
	if (plate->Xmax >= 0.3) StepH = 0.1;
	if (plate->Xmax > 1) StepH = 0.2;
	if (plate->Xmax > 2) StepH = 0.5;
	if (plate->Xmax > 5) StepH = 1;
	double StepV = 1;
	if (plate->Ymax < 0.3) StepV = 0.05;
	if (plate->Ymax < 0.1) StepV = 0.01;
	if (plate->Ymax < 0.03) StepV = 0.005;
	if (plate->Ymax >= 0.3) StepV = 0.1;
	if (plate->Ymax > 1) StepV = 0.2;
	if (plate->Ymax > 2) StepV = 0.5;
	if (plate->Ymax > 5) StepV = 1;

	int DirX = 1, DirY = 2; // X-1 Y-2 Z-3
	if (fabs(plate->OrtX.Y) > 0.5 && fabs(plate->OrtX.Y) > fabs(plate->OrtX.X)) DirX = 2;
	if (fabs(plate->OrtX.Z) > 0.5 && fabs(plate->OrtX.Z) > fabs(plate->OrtX.Y)) DirX = 3;
	if (fabs(plate->OrtY.Y) > 0.5 && fabs(plate->OrtY.Y) > fabs(plate->OrtY.X)) DirY = 2;
	if (fabs(plate->OrtY.Z) > 0.5 && fabs(plate->OrtY.Z) > fabs(plate->OrtY.Y)) DirY = 3;
	double Pgl;
	
	switch (DirX) {
		case 1: Shor = "axis X"; break;
		case 2: Shor = "axis Y"; break;
		case 3: Shor = "axis Z"; break;
		default: Shor = "axis "; break;
	}
	switch (DirY) {
		case 1: Svert = "axis X"; break;
		case 2: Svert = "axis Y"; break;
		case 3: Svert = "axis Z"; break;
		default: Svert = "axis "; break;
	}

	/*Shor = "axis X";
	if (plate->OrtX.Y > 0.5) Shor = "axis Y";
	else if (plate->OrtX.Z > 0.5) Shor = "axis Z";

	Svert = "axis X";
	if (plate->OrtY.Y > 0.5) Svert = "axis Y";
	else if (plate->OrtY.Z > 0.5) Svert = "axis Z";*/
	
/*	Global = C3Point(0,0,0);
	Local = plate->GetLocal(Global);
	if (Local.X > 0 && Local.X < plate->Xmax) {
		x = OrigX + (int)(Local.X * ScaleX);
		pDC->MoveTo(x, ymin1 + 2); 
		pDC->LineTo(x, ymin1 - (int)(Zmax * ScaleZ) - 2);
		pDC->TextOut(x, ymin1 + 2, "0");
	}
	
	if (Local.Y > 0 && Local.Y < plate->Ymax) {
		x = OrigX + (int)(Local.Y * ScaleY);
		pDC->MoveTo(x, ymin2 + 2); 
		pDC->LineTo(x, ymin2 - (int)(Zmax * ScaleZ) - 2);
		pDC->TextOut(x, ymin2 + 2, "0");
	}
*/

///////// upper chart	
//	if (plate->OrtX.X > 0.5) {
	Global = plate->MassCentre; //C3Point(0,Pgl,0);//Global.Y = Pgl;
	Pgl = -10;
	pDC->TextOut((OrigX + xmax1) / 2, ymin1 + 15, Shor);
	while (Pgl < 50) {
		Pgl += StepH;
		if (fabs(Pgl) < 1.e-6) Pgl = 0;//continue;
		//Global = plate->MassCentre; //C3Point(Pgl,0,0);
		/*if (plate->OrtX.Y > 0.5) Global = C3Point(0,Pgl,0);//Global.Y = Pgl;
		else if (plate->OrtX.Z > 0.5) Global = C3Point(0,0,Pgl);//Global.Z = Pgl;*/

		switch (DirX) {
		case 1: Global.X = Pgl; break;
		case 2: Global.Y = Pgl; break;
		case 3: Global.Z = Pgl; break;
		default:  break;
	}
		Local = plate->GetLocal(Global);
		if (Local.X < 0 || Local.X > Xmax) continue; 
		x = xmin + (int)(Local.X * ScaleX);
		pDC->MoveTo(x, ymin1 + 2); pDC->LineTo(x, ymin1 - (int)(Zmax * ScaleZ) - 2);
		S.Format("%g", Pgl);
		pDC->TextOut(x, ymin1 + 2, S);
	} // while

		
/*	
} // if plate->OrtX.X > 0.5
	else { // plate->OrtX.X < 0.5
		Pgl = 0;
		pDC->TextOut((xmin + xmax1) / 2, ymin1 + 15, "global Y");
		//pDC->TextOut(OrigX + (int)(Xmax * 0.6 * ScaleX), OrigY + 25, "global Y");
		while (Pgl < *pDoc->AreaHorMax) {
		Pgl += StepH;
		Global.Y = Pgl;
		Local = plate->GetLocal(Global);
		if (Local.X < 0 || Local.X > Xmax) continue; 
		x = xmin + (int)(Local.X * ScaleX);
		pDC->MoveTo(x, ymin1 + 2); pDC->LineTo(x, ymin1 - (int)(Zmax * ScaleZ) - 2);
		S.Format("%g", Pgl);
		pDC->TextOut(x, ymin1 + 2, S);
		} // while

		Pgl = 0;
		while (Pgl > *pDoc->AreaHorMin) {
		Pgl -= StepH;
		Global.Y = Pgl;
		Local = plate->GetLocal(Global);
		if (Local.X < 0 || Local.X > Xmax) continue; 
		x = xmin + (int)(Local.X * ScaleX);
		pDC->MoveTo(x, ymin1 + 2); pDC->LineTo(x, ymin1 - (int)(Zmax * ScaleZ) - 2);
		S.Format("%g", Pgl);
		pDC->TextOut(x, ymin1 + 2, S);
		} // while
	}// else (plate->OrtX.X < 0.5)
*/
///////////// lower chart
	//if (plate->OrtY.Y > 0.5) {
	Global = plate->MassCentre; //C3Point(0,Pgl,0);//Global.Y = Pgl;
	Pgl = -50;
	pDC->TextOut((xmin + xmax2) / 2, ymin2 + 15, Svert);
	while (Pgl < 50) {
		Pgl += StepV;
		if (fabs(Pgl) < 1.e-6) Pgl = 0;//continue;
	switch (DirY) {
		case 1: Global.X = Pgl; break;
		case 2: Global.Y = Pgl; break;
		case 3: Global.Z = Pgl; break;
		default:  break;
	}
		
		/*if (plate->OrtY.X > 0.5) Global = C3Point(Pgl,0,0);//Global.X = Pgl;
		else if (plate->OrtY.Z > 0.5) Global = C3Point(0,0,Pgl);//Global.Z = Pgl;*/
		Local = plate->GetLocal(Global);
		if (Local.Y < 0 || Local.Y > Ymax) continue; 
		x = xmin + (int)(Local.Y * ScaleY);
		pDC->MoveTo(x, ymin2 + 2); pDC->LineTo(x, ymin2 - (int)(Zmax * ScaleZ) - 2);
		S.Format("%g", Pgl);
		pDC->TextOut(x, ymin2 + 2, S);
	} // while
	pDC->SetBkColor(RGB(100,100,100));
	pDC->SetTextColor(RGB(255,255,255));
	//pDC->TextOut(xmin + 5, ymin1 - (int)(Zmax * ScaleZ) +2, "BTR-ITER");
	//pDC->TextOut(xmin + 5, ymin2 - (int)(Zmax * ScaleZ) +2, "BTR-ITER");
	//S.Format("Hor.Dist = %0.2f", Load->iProf*Load->StepX);
	//pDC->TextOut(xmax2 - 80, ymax2 - 20, S);
		
/*		Pgl = 0;
		while (Pgl > *pDoc->AreaHorMin) {
		Pgl -= StepV;
		Global.Y = Pgl;
		Local = plate->GetLocal(Global);
		if (Local.Y < 0 || Local.Y > Ymax) continue; 
		x = xmin + (int)(Local.Y * ScaleY);
		pDC->MoveTo(x, ymin2 + 2); pDC->LineTo(x, ymin2 - (int)(Zmax * ScaleZ) - 2);
		S.Format("%g", Pgl);
		pDC->TextOut(x, ymin2 + 2, S);
		} // while
	} // if

	else { // OrtY.Y < 0.5
		Pgl = 0;
		pDC->TextOut((xmin + xmax2) / 2, ymin2 + 15, "global Z");
		while (Pgl < *pDoc->AreaVertMax) {
		Pgl += StepV;
		Global.Z = Pgl;
		Local = plate->GetLocal(Global);
		if (Local.Y < 0 || Local.Y > Ymax) continue; 
		x = xmin + (int)(Local.Y * ScaleY);
		pDC->MoveTo(x, ymin2 + 2); pDC->LineTo(x, ymin2 - (int)(Zmax * ScaleZ) - 2);
		S.Format("%g", Pgl);
		pDC->TextOut(x, ymin2 + 2, S);
		} // while
		//pDC->TextOut(OrigX - 10, OrigY - (int)(Ymax * ScaleY) - 15, "Z"); 

		Pgl = 0;
		while (Pgl > *pDoc->AreaVertMin) {
		Pgl -= StepV;
		Global.Z = Pgl;
		Local = plate->GetLocal(Global);
		if (Local.Y < 0 || Local.Y > Ymax) continue; 
		x = xmin + (int)(Local.Y * ScaleY);
		pDC->MoveTo(x, ymin2 + 2); pDC->LineTo(x, ymin2 - (int)(Zmax * ScaleZ) - 2);
		S.Format("%g", Pgl);
		pDC->TextOut(x, ymin2 + 2, S);
		} // while
	} // else
*/	
}
void CSetView:: ShowLoadProfiles(CDC* pDC)
{
	CBTRDoc* pDoc = (CBTRDoc*)GetDocument();
	//if (pDoc->OptCombiPlot == -1) return;
	CPlate * plate = pDoc->pMarkedPlate;
	//Load = plate->Load;
	Xmax = Load->Xmax + Load->StepX;
	Ymax = Load->Ymax + Load->StepY;
	CFont* pOldFont = pDC->SelectObject(&smallfont);
	pDC->SetTextColor(RGB(0, 0, 0));
	CBrush brush1, brush2;
	CBrush * oldbrush;
	CPen Pen, BoundPen;
	CPen * pOldPen;
	if(!Pen.CreatePen(PS_SOLID, 2, RGB(255,0,0))) return; // profiles circles
	if(!BoundPen.CreatePen(PS_DASH, 3, RGB(200,200,0))) return; // load view limits
	brush1.CreateSolidBrush(RGB(255,220,255)); // magenta - horizontal
	brush2.CreateSolidBrush(RGB(220,255,255)); // blue - vertical
//	pDC->SetBkMode(1);
	int x, y;
	int i, j;
	double ix, jy;
	double val;
	int xmin = OrigX; // 20;
	//int ymin1 =  Height  / 2 +5;
	//int ymin2 =  Height + 25;
	int ip = Load->iProf;
	int jp = Load->jProf;

	if (ip < 0 || ip > Load->Nx - 1 || jp < 0 || jp > Load->Ny - 1) return;

	RectArrayX.RemoveAll();
	RectArrayY.RemoveAll();

	Load->MaxProf = 0;
	
	for (i = 0; i <= Load->Nx; i++) {
		val = Load->Val[i][jp];
		if (val >= Load->MaxProf) Load->MaxProf = val;
	}
	for (j = 0; j <= Load->Ny; j++) {
		val = Load->Val[ip][j];
		if (val >= Load->MaxProf) Load->MaxProf = val;
	}

	double scaleX =  Width / Load->Xmax; //ScaleX * 1.5;
	double scaleY =  Width / Load->Ymax; //ScaleY;
	double scaleZ =  Height* 0.4 / Load->MaxProf;
	CString S;
	int pos, pos1;
	CString S1, S2, S3;
	double order, mant;
	
	 S1.Format("%8.1e", Load->MaxProf);
	 pos = S1.Find("e",0);
	 pos1 = S1.Find(".",0);
	 if (pos>0) S = "1.0" + S1.Mid(pos);
	 else return;
	 order = atof(S);
	 S = S1.Mid(pos);
	 S.MakeUpper();

	 S2 = S1.Left(pos);
	 mant = atof(S2);
	 S3 = S1.Mid(pos1, 1);
	 if (atof(S3) > 5.0) mant += 1.;
	 Load->CrossZ = 0.5*order;
	 if (mant > 3.5) Load->CrossZ = order;
	 if (mant > 6) Load->CrossZ = 2*order;

	int xmax1 = xmin +  (int)ceil(Xmax * scaleX);
	int xmax2 = xmin +  (int)ceil(Ymax * scaleY);
	int ymax1 = ymin1 - (int)ceil(mant*order * scaleZ);
	int ymax2 = ymin2 - (int)ceil(mant*order * scaleZ);
	int w;

	ScaleX = scaleX; ScaleY = scaleY; ScaleZ = scaleZ; 
	Zmax = mant*order;

	oldbrush = pDC->SelectObject(&brush1);
	pDC->Rectangle(xmin, ymin1, xmax1, ymax1);
	pDC->SelectObject(&brush2);
	pDC->Rectangle(xmin, ymin2, xmax2, ymax2);

	pDC->TextOut(xmin-20, ymax1-15, S);
	pDC->TextOut(xmin-20, ymax2-15, S);

	C3Point Local = C3Point(Load->iProf*Load->StepX, Load->jProf*Load->StepY, 0);
	C3Point Global = plate->GetGlobalPoint(Local);
	if (plate->Number != 1000)// not PDP table
	S.Format("POWER PROFILES  at  X=%g Y=%g Z=%g", Global.X, Global.Y, Global.Z);
	else { // PDP load table
		if (Global.X > 100) 
			S.Format("POWER PROFILES  at Y=%g Z=%g", Global.Y, Global.Z); // %0.2f
		else if (Global.Y > 100) 
			S.Format("POWER PROFILES  at X=%g Z=%g", Global.X, Global.Z);
		else if (Global.Z > 100) 
			S.Format("POWER PROFILES  at X=%g Y=%g", Global.X, Global.Y);
	}
	pDC->TextOut(xmin+30, 5, S);
		
	int jcross = (int)(mant*order / (Load->CrossZ));

////////// upper - horiz lines

	for ( j = 1; j <= jcross; j++) {
		 y = ymin1 - (int)(j*(Load->CrossZ) * scaleZ);
		 x = xmin; pDC->MoveTo(x,y);
		 jy = j*(Load->CrossZ);
		 S.Format("%g", jy/order);
		 w = pDC->GetTextExtent(S).cx;
		 pDC->TextOut(x-w-5, y-5, S);
		 x = xmax1; pDC->LineTo(x,y);		
	}

//////////// lower - horiz lines

	for ( j = 1; j <= jcross; j++) {
		 y = ymin2 - (int)(j*(Load->CrossZ)* scaleZ);
		 x = xmin; pDC->MoveTo(x,y);
		 jy = j*Load->CrossZ;
		 S.Format("%g", jy/order);
		 w = pDC->GetTextExtent(S).cx;
		 pDC->TextOut(x-w-5, y-5, S);
		 x = xmax2; pDC->LineTo(x,y);		 
	}
///////////////////////// profiles //////////////////////////////////
	pDC->MoveTo(xmin, ymin1);
	for (i = 0; i <= Load->Nx; i++) {
		x = xmin + (int) (i*(Load->StepX) * scaleX);
		y = ymin1 - (int)(Load->Val[i][Load->jProf] * scaleZ);
		//pDC->MoveTo(x,y);y = ymax1;
		pDC->LineTo(x, y);
	}
	pDC->MoveTo(xmin, ymin2);
	for (j = 0; j <= Load->Ny; j++) {
		x = xmin + (int) (j*(Load->StepY)* scaleY);
		y = ymin2 - (int)(Load->Val[Load->iProf][j] * scaleZ);
		//pDC->MoveTo(x,y);y = ymax2;
		pDC->LineTo(x, y);
	}

	CRect rect;
	pOldPen = pDC->SelectObject(&Pen); // red

	for (i = 0; i <= Load->Nx; i++) {
		x = xmin + (int) (i*(Load->StepX)* scaleX);
		y = ymin1 - (int)(Load->Val[i][Load->jProf] * scaleZ);
		pDC->Ellipse(x-3, y-3, x+3, y+3);
		rect = new CRect();
		rect.SetRect(x-3, y-3, x+3, y+3);
		RectArrayX.Add(rect);
		
	}
	//S.Format("Vert.Dist = %0.2f", Load->jProf*Load->StepY);
	//pDC->TextOut(xmax1 - 80, ymax1 - 20, S);

	for (j = 0; j <= Load->Ny; j++) {
		x = xmin + (int) (j*(Load->StepY)* scaleY);
		y = ymin2 - (int)(Load->Val[Load->iProf][j] * scaleZ);
		pDC->Ellipse(x-3, y-3, x+3, y+3);
		rect = new CRect();
		rect.SetRect(x-3, y-3, x+3, y+3);
		RectArrayY.Add(rect);
		
	}
	//S.Format("Hor.Dist = %0.2f", Load->iProf*Load->StepX);
	//pDC->TextOut(xmax2 - 80, ymax2 - 20, S);
/*	pDC->SelectObject(&BoundPen);
//  plate bounds (yellow)	
	double X0, X1, Y0, Y1;
	C3Point Ploc;
	Ploc = plate->GetLocal(plate->Vmin);
	X0 = Ploc.X; Y0 = Ploc.Y;
	Ploc = plate->GetLocal(plate->Vmax);
	X1 = Ploc.X; Y1 = Ploc.Y;
// horizontal limits
	x = xmin + (int)(X0 * scaleX);	y = ymin1 + 2;
	pDC->MoveTo(x, y);
	y = ymin1 - (int)(Zmax * scaleZ) - 2;
	if (x > xmin) pDC->LineTo(x, y);
	x = xmin + (int)(X1 * scaleX);	y = ymin1 + 2;
	pDC->MoveTo(x, y);
	y = ymin1 - (int)(Zmax * scaleZ);
	if (x < xmax1) pDC->LineTo(x, y);
// vertical limits
	x = xmin + (int)(Y0 * scaleY);	y = ymin2 + 2;
	pDC->MoveTo(x, y);
	y = ymin2 - (int)(Zmax * scaleZ) - 2;
	if (x > xmin) pDC->LineTo(x, y);
	x = xmin + (int)(Y1 * scaleY);	y = ymin2 + 2;
	pDC->MoveTo(x, y);
	y = ymin2 - (int)(Zmax * scaleZ);
	if (x < xmax2) pDC->LineTo(x, y);
*/	
	pDC->SelectObject(pOldPen);
	pDC->SelectObject(pOldFont);
	pDC->SelectObject(oldbrush);

	//GdiFlush();

	}

void CSetView:: ShowIntegralProfiles(CDC* pDC)
{
	CBTRDoc* pDoc = (CBTRDoc*)GetDocument();
	if (pDoc->OptCombiPlot == -1) return;
	CPlate * plate = pDoc->pMarkedPlate;
	Load = plate->Load;
	Xmax = Load->Xmax + Load->StepX;
	Ymax = Load->Ymax + Load->StepY;
	CFont* pOldFont = pDC->SelectObject(&smallfont);
	pDC->SetTextColor(RGB(0, 0, 0));
	CBrush brush1, brush2;
	CBrush * oldbrush;
	CPen Pen;
	CPen * pOldPen;
	if(!Pen.CreatePen(PS_SOLID, 2, RGB(0,0,0))) return;
	brush1.CreateSolidBrush(RGB(255,220,255));
	brush2.CreateSolidBrush(RGB(220,255,255));
//	pDC->SetBkMode(1);
	int x, y;
	int i, j;
	double ix, jy;
	double sumval;
	int xmin = OrigX; // 20;
	//int ymin1 =  Height  / 2 +5;
	//int ymin2 =  Height + 25;
	// int ip = Load->iProf;
	// int jp = Load->jProf;

	RectArrayX.RemoveAll();
	RectArrayY.RemoveAll();

	double MaxProf = 0;
	double Cell = Load->StepX * Load->StepY;
	
	for (i = 0; i <= Load->Nx; i++) {
		sumval = 0;
		for (j = 0; j <= Load->Ny; j++) sumval += Load->Val[i][j] * Cell;
		if (sumval >= MaxProf) MaxProf = sumval;
	}
	for (j = 0; j <= Load->Ny; j++) {
		sumval = 0;
		for (i = 0; i <= Load->Nx; i++)	sumval += Load->Val[i][j] * Cell;
		if (sumval >= MaxProf) MaxProf = sumval;
	}

	double scaleX =  Width / Load->Xmax; //ScaleX * 1.5;
	double scaleY =  Width / Load->Ymax; //ScaleY;
	double scaleZ =  Height* 0.4 / MaxProf;
	CString S;
	int pos, pos1;
	CString S1, S2, S3;
	double order, mant;
	CRect rect;
	
	 S1.Format("%8.1e", MaxProf);
	 pos = S1.Find("e",0);
	 pos1 = S1.Find(".",0);
	 if (pos>0) S = "1.0" + S1.Mid(pos);
	 order = atof(S);
	 S = S1.Mid(pos);
	 S.MakeUpper();

	 S2 = S1.Left(pos);
	 mant = atof(S2);
	 S3 = S1.Mid(pos1, 1);
	 if (atof(S3) > 5.0) mant += 1.;
	 Load->CrossZ = 0.5*order;
	 if (mant > 3.5) Load->CrossZ = order;
	 if (mant > 6) Load->CrossZ = 2*order;

	int xmax1 = xmin +  (int)ceil(Xmax * scaleX);
	int xmax2 = xmin +  (int)ceil(Ymax * scaleY);
	int ymax1 = ymin1 - (int)ceil(mant*order * scaleZ);
	int ymax2 = ymin2 - (int)ceil(mant*order * scaleZ);
	int w;

	ScaleX = scaleX; ScaleY = scaleY; ScaleZ = scaleZ; 
	Zmax = mant*order;

	oldbrush = pDC->SelectObject(&brush1);
	pDC->Rectangle(xmin, ymin1, xmax1, ymax1);
	pDC->SelectObject(&brush2);
	pDC->Rectangle(xmin, ymin2, xmax2, ymax2);

	pDC->TextOut(xmin-20, ymax1-15, S);
	pDC->TextOut(xmin-20, ymax2-15, S);

/*	C3Point Local = C3Point(Load->iProf*Load->StepX, Load->jProf*Load->StepY, 0);
	C3Point Global = plate->GetGlobalPoint(Local);
	if (plate->Number != 1000)// not PDP table
	S.Format("POWER PROFILES  at  X=%0.2f Y=%0.2f Z=%0.2f", Global.X, Global.Y, Global.Z);
	else { // PDP load table
		if (Global.X > 100) 
			S.Format("POWER PROFILES  at Y=%0.2f Z=%0.2f", Global.Y, Global.Z);
		else if (Global.Y > 100) 
			S.Format("POWER PROFILES  at X=%0.2f Z=%0.2f", Global.X, Global.Z);
		else if (Global.Z > 100) 
			S.Format("POWER PROFILES  at X=%0.2f Y=%0.2f", Global.X, Global.Y);
	}*/
	S.Format("INTEGRAL POWER, W (Steps Hor = %g  Vert = %g)", Load->StepX, Load->StepY);
	pDC->TextOut(xmin+30, 5, S);
		
	int jcross = (int)(mant*order / (Load->CrossZ));

////////// upper magenta

	for ( j = 1; j <= jcross; j++) {
		 y = ymin1 - (int)(j*(Load->CrossZ) * scaleZ);
		 x = xmin; pDC->MoveTo(x,y);
		 jy = j*(Load->CrossZ);
		 S.Format("%g", jy/order);
		 w = pDC->GetTextExtent(S).cx;
		 pDC->TextOut(x-w-5, y-5, S);
		 x = xmax1; pDC->LineTo(x,y);		
	}

//////////// lower blue

	for ( j = 1; j <= jcross; j++) {
		 y = ymin2 - (int)(j*(Load->CrossZ)* scaleZ);
		 x = xmin; pDC->MoveTo(x,y);
		 jy = j*Load->CrossZ;
		 S.Format("%g", jy/order);
		 w = pDC->GetTextExtent(S).cx;
		 pDC->TextOut(x-w-5, y-5, S);
		 x = xmax2; pDC->LineTo(x,y);		 
	}

///////////////////////////////////////////////////////////
	pOldPen = pDC->SelectObject(&Pen);
	pDC->MoveTo(xmin, ymin1);
	for (i = 0; i <= Load->Nx; i++) {
		x = xmin + (int) (i*(Load->StepX) * scaleX);
		sumval = 0;
		for (j = 0; j <= Load->Ny; j++) sumval += Load->Val[i][j] * Cell;
		//y = ymin1 - (int)(Load->Val[i][Load->jProf] * scaleZ);
		y = ymin1 - (int)(sumval * scaleZ);
		pDC->LineTo(x, y);
		pDC->Ellipse(x-3, y-3, x+3, y+3);
		rect = new CRect();
		rect.SetRect(x-3, y-3, x+3, y+3);
		RectArrayX.Add(rect);
	}
	pDC->MoveTo(xmin, ymin2);
	for (j = 0; j <= Load->Ny; j++) {
		x = xmin + (int) (j*(Load->StepY)* scaleY);
		sumval = 0;
		for (i = 0; i <= Load->Nx; i++) sumval += Load->Val[i][j] * Cell;
		//y = ymin2 - (int)(Load->Val[Load->iProf][j] * scaleZ);
		y = ymin2 - (int)(sumval * scaleZ);
		pDC->LineTo(x, y);
		pDC->Ellipse(x-3, y-3, x+3, y+3);
		rect = new CRect();
		rect.SetRect(x-3, y-3, x+3, y+3);
		RectArrayX.Add(rect);
	}

/*	
	
	pOldPen = pDC->SelectObject(&Pen);

	for (i = 0; i <= Load->Nx; i++) {
		x = xmin + (int) (i*Load->StepX* scaleX);
		y = ymin1 - (int)(Load->Val[i][Load->jProf] * scaleZ);
		pDC->Ellipse(x-3, y-3, x+3, y+3);
		rect = new CRect();
		rect.SetRect(x-3, y-3, x+3, y+3);
		RectArrayX.Add(rect);
		
	}
	//S.Format("Vert.Dist = %0.2f", Load->jProf*Load->StepY);
	//pDC->TextOut(xmax1 - 80, ymax1 - 20, S);

	for (j = 0; j <= Load->Ny; j++) {
		x = xmin + (int) (j*Load->StepY* scaleY);
		y = ymin2 - (int)(Load->Val[Load->iProf][j] * scaleZ);
		pDC->Ellipse(x-3, y-3, x+3, y+3);
		rect = new CRect();
		rect.SetRect(x-3, y-3, x+3, y+3);
		RectArrayY.Add(rect);
		
	}
	//S.Format("Hor.Dist = %0.2f", Load->iProf*Load->StepX);
	//pDC->TextOut(xmax2 - 80, ymax2 - 20, S);
	*/

	pDC->SelectObject(pOldPen);
	pDC->SelectObject(pOldFont);
	pDC->SelectObject(oldbrush);

	//GdiFlush();

}

void CSetView:: ShowStatus() //(CDC* pDC)
{
	CDC * pDC = GetDC();
		CBTRDoc* pDoc = (CBTRDoc*)GetDocument();
		RectArrayX.RemoveAll();
		RectArrayY.RemoveAll();
		CString S;
		CTime t = CTime::GetCurrentTime();
		CTime Tbegin = pDoc->StartTime0;
		CTime Tend = t;
		CTimeSpan Telapsed;
		int h, m, s, h0, m0, s0, dh, dm, ds, sec;
		long mspb, msleft, sleft, mleft, hleft;
		double SelCurr = pDoc->IonBeamCurrent / pDoc->NofChannelsHor / pDoc->NofChannelsVert
						* pDoc->NofActiveChannels * pDoc->NofActiveRows;// source current traced
		CFont* pOldFont = pDC->SelectObject(&font);
		pDC->SetTextColor(RGB(255,0,0));
		h =	t.GetHour(); m = t.GetMinute(); s = t.GetSecond();
		int Nscen = pDoc->SCEN;
		int Maxscen = pDoc->MAXSCEN;
		int Nrun = pDoc->RUN;
		int Maxrun = pDoc->MAXRUN;
		int Ntot = pDoc->NofBeamlets;
		int Ncalc = NofCalculated;
		//int Nattr = pDoc->Attr_Array.GetSize();
		//int Npart = pDoc->NofBeamlets * pDoc->MultCoeff;
		h0 = Tbegin.GetHour(); m0 = Tbegin.GetMinute(); s0 = Tbegin.GetSecond();

		pDoc->GetMemState();
		long long MemFalls = (pDoc->ArrSize) * sizeof(minATTR);
		if (MemFalls < 0) MemFalls = 0;
		long long memb;
		if (Ncalc > 0) memb = MemFalls / Ncalc;
		else memb = MemFalls;
		if (memb < 1) memb = 1;
		long long Nleft = (pDoc->MemFree * 1024 - MemFalls) / memb;
		if (Nleft < 0) Nleft = 0;
		Tend = pDoc->StopTime;
		Telapsed = GetElapse(Tbegin, Tend) - pDoc->SuspendedSpan; 
		dh = Telapsed.GetHours(); dm = Telapsed.GetMinutes(); ds = Telapsed.GetSeconds();
		sec = ds + dm * 60 + dh * 3600;
		if (Ncalc == 0)	mspb = 0;//sec * 1000 / (Ntot);
		else mspb = sec * 1000 / (Ncalc);

		S.Format("Time  %02d:%02d:%02d  V%g  Scens %d   ",
					h, m, s, BTRVersion,  Maxscen);//, Maxrun);
		pDC->TextOut(10,5, S);
		
		S.Format("START  %02d:%02d:%02d       ", h0, m0, s0);
		pDC->TextOut(10,25, S);

		S.Format("Channels %d Rows %d   ", (pDoc->NofActiveChannels), (pDoc->NofActiveRows));
		pDC->TextOut(10,45, S);

		S.Format("Active Source Current  %g A", SelCurr);
		pDC->TextOut(10,65, S);
		
		S.Format("Total active BMLs %d  ", Ntot);
		pDC->TextOut(10,85, S);

		S.Format("Traced BMLs   %d          ", Ncalc);
		pDC->TextOut(10,105, S);

		S.Format("BTR holds     %ld kB      ", pDoc->MemUsed);
		pDC->TextOut(10, 125, S);
		
		S.Format("Avail mem     %ld kB      ", pDoc->MemFree);
		pDC->TextOut(10, 145, S);

		S.Format("Av. BML time   %d ms         ", mspb);
			pDC->TextOut(10,165, S);
		
		S.Format("Falls arr: %ld elem         ", pDoc->ArrSize);//, sizeof(minATTR), MemFalls);
		pDC->TextOut(10, 185, S);
				
		
		if (Ncalc == Ntot) { // finished
				
			S.Format("Total Run  %02d:%02d:%02d            ", dh, dm, ds);
			pDC->TextOut(10,145, S);

			//S.Format("Av.time per BML  %d ms         ", mspb);
			//pDC->TextOut(10,165, S);
			
			//S.Format("Falls arr: %ld elem       ", pDoc->ArrSize);
							//, sizeof(minATTR), MemFalls);
			//pDC->TextOut(10, 185, S);
		} // Ncalc = Ntot

		if (Ncalc > 0 && pDoc->STOP)  { //  stopped
			S.Format(" !!! Abort Scen %d: Last %d of %d  ",Nscen-1, Ncalc, Ntot);
			if (Ncalc < Ntot) // stopped
				pDC->TextOut(10,205, S);
		} // STOP 

		//} //Ncalc > 0 // started or done
		pDC->SelectObject(pOldFont);
		ReleaseDC(pDC);
}

double CSetView::GetZmax(CLoad* load)
{
	C3Point P = GetMantOrder(load->MaxVal);
	return ((P.X+P.Y) * P.Z);
	/*
	CString S;
	int pos, pos1;
	CString S1, S2, S3;
	double order;
	double mant;
	 S1.Format("%8.1e", load->MaxVal);
	 pos = S1.Find("e",0);
	  pos1 = S1.Find(".",0);
	 if (pos>0) S = "1.0" + S1.Mid(pos);
	 order = atof(S);
//	 S = S1.Mid(pos);
//	 S.MakeUpper();

	 S2 = S1.Left(pos1);
	 mant = atof(S2);
	 S3 = S1.Mid(pos1, 1);
	 if (atof(S3) > 5.0) mant += 1.;
	 return (mant * order);
	 */
}

//void CSetView:: SetLoad(CLoad* load)
void CSetView:: SetLoad_Plate(CLoad * load, CPlate * plate)
{
	Plate = plate;
	Load = load;
	if (Load == NULL) return;
	 Xmax  =  Load->Xmax;// + Load->StepX;
	 Ymax  =  Load->Ymax;// + Load->StepY;
//	 Zmax  =  Load-> MaxVal; 
	 Zmax = GetZmax(Load);
	
}


void CSetView:: UpdateScales(CRect & rect)
{
	// CBTRDoc* pDoc = (CBTRDoc*)GetDocument();
	
	Width = rect.Size().cx;
	Height =  rect.Size().cy;
	double l = Ymax * SinFi;
	double r = Xmax * CosFi;
	OrigX = rect.left + (int)(l/(l+r) * rect.Size().cx);
	OrigY = rect.bottom;
//	OrigY1 =   rect.bottom / 2; OrigY2 =   rect.bottom - 5;
	ScaleX = rect.Size().cx * r / (l+r) / Xmax / CosFi;
	ScaleY = rect.Size().cx * l / (l+r) / Ymax / SinFi;
	ScaleX = Min(ScaleX, ScaleY);
	ScaleY = ScaleX;
	ScaleZ = (rect.Size().cy)/ Zmax/2; // CosTeta;
	LoadRect = rect; 
}

CPoint  CSetView:: ScreenPoint(C3Point P)
{
	CPoint point;
	point.x = OrigX + (int)(P.X*CosFi*ScaleX - P.Y*SinFi*ScaleY);
	point.y = OrigY -  (int)((P.X*SinFi*ScaleX + P.Y*CosFi*ScaleY) * SinTeta + P.Z*CosTeta*ScaleZ);
	return point;
}

void  CSetView:: ShowCoord(CDC* pDC)
{
//	int x, y;
	//double X, Y, Z;
	CBTRDoc* pDoc = (CBTRDoc*)GetDocument();
	CPlate * plate = pDoc->pMarkedPlate;
	CString S, Shor, Svert;
	CPoint point;
	C3Point P, Pgl;
	double coord;
	CFont* pOldFont = pDC->SelectObject(&smallfont);
	CPen DotPen;
//	DotPen.CreatePen(PS_SOLID, 1, RGB(150, 150, 150));
	pDC->SetBkMode(0);
	
	int w; 
	int pos;
	CString S1;
	double order;

	Shor = "axis X";
	if (plate->OrtX.Y > 0.5) Shor = "axis Y";
	else if (plate->OrtX.Z > 0.5) Shor = "axis Z";

	Svert = "axis X";
	if (plate->OrtY.Y > 0.5) Svert = "axis Y";
	else if (plate->OrtY.Z > 0.5) Svert = "axis Z";

//	pDC->TextOut(OrigX-2, OrigY, "0");
	pDC->MoveTo(OrigX, OrigY);
	P.X = Xmax; P.Y = 0;  P.Z = 0;
	point = ScreenPoint(P);
	pDC->LineTo(point);
	//pDC->TextOut(point.x+10, point.y-7, Shor);

	P.X = Xmax/2; P.Y = 0;  P.Z = 0;
	point = ScreenPoint(P);
	pDC->TextOut(point.x+10, point.y+20, Shor);

	P.X = Xmax; P.Y = Ymax;  P.Z = 0;
	point = ScreenPoint(P);
	pDC->LineTo(point);
	
	P.X = 0; P.Y = Ymax;  P.Z = 0;
	point = ScreenPoint(P);
	pDC->LineTo(point);

//	pDC->SelectObject(pOldPen);

	pDC->MoveTo(OrigX, OrigY);
	P.X = 0; P.Y = Ymax; P. Z = 0;
	point = ScreenPoint(P);
	pDC->LineTo(point);
	//pDC->TextOut(point.x-10, point.y-10, Svert);

	P.X = 0; P.Y = Ymax/4; P.Z = 0;
	point = ScreenPoint(P);
	w = pDC->GetTextExtent(Svert).cx;
	pDC->TextOut(point.x-w -30, point.y, Svert);

	CBrush brush;
	CBrush * oldbrush;
	brush.CreateSolidBrush(RGB(220,220,180)); //haki
	oldbrush = pDC->SelectObject(&brush);

	P.X = Xmax/2; P.Y = Ymax/2; P. Z = 0;
	point = ScreenPoint(P);
	pDC->FloodFill(point.x, point.y, RGB(0,0,0));
//	pDC->SelectObject(oldbrush);

	 S1.Format("%8.1e", Zmax);
	 pos = S1.Find("e",0);
	 if (pos>0) S = "1.0" + S1.Mid(pos);
	 order = atof(S);
	 S = S1.Mid(pos);
	 S.MakeUpper();
	// if(pDC->m_hAttribDC != NULL) w = pDC->GetTextExtent(S).cx; else w = 50;
	w = pDC->GetTextExtent(S).cx;

	P.X = Xmax; P.Y = 0; P.Z = 0;
	point = ScreenPoint(P);
	pDC->MoveTo(point);
	P.X = Xmax; P.Y = 0; P.Z = Zmax;
	point = ScreenPoint(P);
	pDC->LineTo(point);
	pDC->TextOut(point.x - w/2, point.y-20, S);

	P.X = Xmax; P.Y = Ymax; P.Z = 0;
	point = ScreenPoint(P);
	pDC->MoveTo(point);
	P.X = Xmax; P.Y = Ymax; P.Z = Zmax;
	point = ScreenPoint(P);
	pDC->LineTo(point);
	pDC->TextOut(point.x - w/2, point.y-20, S);

	P.X = 0; P.Y = Ymax; P.Z = 0;
	point = ScreenPoint(P);
	pDC->MoveTo(point);
	P.X = 0; P.Y = Ymax; P.Z = Zmax;
	point = ScreenPoint(P);
	pDC->LineTo(point);
//	pDC->TextOut(point.x, point.y-15, S);

	P.X = Xmax; P.Y = Ymax/3; P. Z = Zmax/3;
	point = ScreenPoint(P);
	pDC->FloodFill(point.x, point.y, RGB(0,0,0));
//	pDC->SelectObject(oldbrush);
	P.X = Xmax/3; P.Y = Ymax; P. Z = Zmax/3;
	point = ScreenPoint(P);
	pDC->FloodFill(point.x, point.y, RGB(0,0,0));
	pDC->SelectObject(oldbrush);

	CPen * pOldPen = pDC->SelectObject(&DotPen);
	double crossX = 0.5 * Load->Xmax; ///Load->CrossX;
	double crossY = 0.5 * Load->Ymax;//Load->CrossY;
	// int icross = (int)floor(Load->Xmax / crossX);
	// int jcross = (int)floor(Load->Ymax / crossY);
	for (int  i = 0; i <= 2; i++) { //icross; i++) {
		P.X = i*crossX; P.Y = 0; P.Z = 0;
		point = ScreenPoint(P);	pDC->MoveTo(point);
		Pgl = plate->GetGlobalPoint(P);
		 coord = Pgl.X; 
		 if (plate->OrtX.Y > 0.5) coord = Pgl.Y; 
			else if (plate->OrtX.Z > 0.5) coord = Pgl.Z; 
		S.Format("%g", coord);
		pDC->TextOut(point.x, point.y+3, S);
		P.X = i*crossX; P.Y = Ymax; P.Z = 0;
		point = ScreenPoint(P);	pDC->LineTo(point);
	}
	for (int  j = 0; j <= 2; j++) { //jcross; j++) {
		P.X = 0; P.Y = j*crossY; P.Z = 0;
		point = ScreenPoint(P);	pDC->MoveTo(point);
		Pgl = plate->GetGlobalPoint(P);
		 coord = Pgl.X; 
		 if (plate->OrtY.Y > 0.5) coord = Pgl.Y; 
			else if (plate->OrtY.Z > 0.5) coord = Pgl.Z; 
		S.Format("%g", coord);
				
		// if(pDC->m_hAttribDC != NULL) w = pDC->GetTextExtent(S).cx; else w = 15;
		w = pDC->GetTextExtent(S).cx;
		pDC->TextOut(point.x-10-w, point.y-5, S);
		P.X = Xmax; P.Y = j*crossY; P.Z = 0;
		point = ScreenPoint(P);	pDC->LineTo(point);
	}

	double crossZ = 0.5*order; //Zmax/5;
	if (Zmax/order >= 3) crossZ *= 2;
	int kcross = (int) floor (Zmax/crossZ);
	for (int  k = 1; k <= kcross; k++) {
		P.X = Xmax; P.Y = 0; P.Z = k*crossZ;
		point = ScreenPoint(P);	pDC->MoveTo(point);
		// S1.Format("%8.1e", P.Z);
		// pos = S1.Find("e",0);
		 //if (pos>0) S = S1.Left(pos);
		S.Format("%g", P.Z/order);
		if(pDC->m_hAttribDC != NULL) w = pDC->GetTextExtent(S).cx; //w = pDC->GetTextExtent(S).cx;
	    else	w = 10;
		 pDC->TextOut(point.x+2, point.y-5, S);
		P.X = Xmax; P.Y = Ymax; P.Z = k*crossZ;
		point = ScreenPoint(P);	pDC->LineTo(point);
		 pDC->TextOut(point.x+2, point.y-8, S);
		 P.X = 0; P.Y = Ymax; P.Z = k*crossZ;
		point = ScreenPoint(P);	pDC->LineTo(point);
		 pDC->TextOut(point.x-4-w, point.y-5, S);
	}
	pDC->SetTextColor(RGB(0,0,0));
	pDC->SelectObject(pOldFont);
	pDC->SelectObject(pOldPen);
}

void CSetView:: ShowLoad(CDC* pDC)
{
//	double V1, V2, V3, V4;
	CPoint point;
	C3Point P;
	C3Point p1, p2, p3, p4,  p12, p23, p34, p41, p13;
//	COLORREF color;
//	double ratio;
	int ii, jj, Nxx, Nyy;
	double hx, hy;
	hx = (Load->StepX)/3; hy = (Load->StepY)/3;
	Nxx = (int)ceil(Load->Xmax / hx); Nyy = (int)ceil(Load->Ymax / hy);
	for (ii = Nxx; ii >= 0; ii--) {
		for (jj = Nyy; jj >= 0; jj--) {
			p1.X = ii * hx;//(Load->StepX)* i;
			p1.Y = jj * hy;//(Load->StepY)* j;
			p1.Z = Load->GetVal(p1.X, p1.Y); //Val[i][j];
		//	P1 = ScreenPoint(P);		V1 = P.Z;
			p2.X = (ii+1) * hx;
			p2.Y = jj * hy;
			p2.Z = Load->GetVal(p2.X, p2.Y);// Load->Val[i+1][j];
		//	P2 = ScreenPoint(P);		V2 = P.Z;
			p3.X = (ii+1) * hx;
			p3.Y = (jj+1) * hy;
			p3.Z = Load->GetVal(p3.X, p3.Y); //Load->Val[i+1][j+1];
		//	P3 = ScreenPoint(P);		V3 = P.Z;
			p4.X = ii * hx;
			p4.Y = (jj+1) * hy;
			p4.Z = Load->GetVal(p4.X, p4.Y); //Load->Val[i][j+1];
		//  P4 = ScreenPoint(P);		V4 = P.Z;
		
		/*	DrawTriangle(pDC, p3, p4, p1);
			DrawTriangle(pDC, p1, p2, p3);
			DrawTriangle(pDC, p2, p3, p4);
			DrawTriangle(pDC, p4, p1, p2);*/
			p12 = (p1 + p2) * 0.5;
			p23 = (p2 + p3) * 0.5;
			p34 = (p3 + p4) * 0.5;
			p41 = (p4 + p1) * 0.5;
			p13 = (p1 + p3) * 0.5;

			DrawTriangle(pDC, p13, p1, p2);
			DrawTriangle(pDC, p13, p2, p3);
			DrawTriangle(pDC, p13, p3, p4);
			DrawTriangle(pDC, p13, p4, p1);


		/*	DrawTriangle(pDC, p3, p34, p23);
			DrawTriangle(pDC, p13, p23, p34);
			DrawTriangle(pDC, p2, p23, p12);
			DrawTriangle(pDC, p13, p12, p23);
			DrawTriangle(pDC, p4, p41, p34);
			DrawTriangle(pDC, p13, p34, p41);
			DrawTriangle(pDC, p1, p12, p41); 
			DrawTriangle(pDC, p13, p41, p12);
		*/
		} //j
	} //i


}

void CSetView:: DrawTriangle(CDC* pDC, C3Point p1, C3Point p2, C3Point  p3)
{
		COLORREF color;
		CPoint P1, P2, P3, P4;
		double ratio;
		if (Load->MaxVal < 1.e-10) ratio = 1;
			else ratio = (p1.Z + p2.Z + p3.Z) / 3 / Load->MaxVal;
			//color = RGB(Red(ratio), Green(ratio), Blue(ratio));
			color = GetColor10(ratio); //RGB(Gray(ratio), Gray(ratio),Gray(ratio));
			if (ratio > 1.e-10) {
				P1 = ScreenPoint(p1);
				P2 = ScreenPoint(p2);
				P3 = ScreenPoint(p3);
				P4 = ScreenPoint(p1);
				DrawFlatSurfaceElement(pDC, color, P1, P2, P3, P4);
			}
}

void CSetView:: DrawFlatSurfaceElement(CDC* pDC,  	COLORREF color,
										   CPoint  P1,	 CPoint  P2, CPoint  P3, CPoint  P4)
{
	CPoint  P0[4];
	P0[0] = P1; P0[1] = P2; P0[2] = P3; P0[3] = P4;
	CPoint point;
	point.x = (P1.x + P3.x) / 2;
	point.y = (P1.y +  P3.y) / 2;
	CPen Pen;
	Pen.CreatePen(PS_SOLID, 1, color);
	CPen * pOldPen = pDC->SelectObject(&Pen);
	CBrush brush;
	CBrush * oldbrush;
	if (!brush.CreateSolidBrush(color))	 brush.CreateStockObject(GRAY_BRUSH); //return;
	oldbrush = pDC->SelectObject(&brush);
	CRgn* pRgn = new CRgn;
	pRgn->CreatePolygonRgn(P0, 4, 1);
	pDC->PaintRgn(pRgn);
	//pDC->FillRgn(pRgn, &brush);
	delete pRgn;
	pDC->MoveTo(P1); pDC->LineTo(P2); 
	pDC->LineTo(P3); pDC->LineTo(P4); pDC->LineTo(P1);
	//pDC->FloodFill(point.x, point.y, RGB(0,0,0));

	pDC->SelectObject(pOldPen);
	pDC->SelectObject(oldbrush);
//	DeleteObject(brush);

}
	
void CSetView:: Show3DLoad(CDC* pDC)
{
	MSG message;
	if (Load == NULL || Load->MaxVal < 1.e-12) return;
//	double V1, V2, V3, V4;
	CPoint point;
	C3Point P;
	CPoint P1, P2;
	C3Point p1, p2;
	COLORREF color;
	double ratio;
	int ii, jj, Nxx, Nyy;
	double hx, hy;
	hx = (Load->StepX)/10; hy = (Load->StepY)/10;
	Nxx = (int)ceil(Load->Xmax / hx); 
	Nyy = (int)ceil(Load->Ymax / hy);

	if (Nxx <= Nyy) {
	for (jj = Nyy-1; jj >= 1; jj--) {
		for (ii = 1; ii < Nxx-1; ii++) {
			p1.X = ii * hx;//(Load->StepX)* i;
			p1.Y = jj * hy;//(Load->StepY)* j;
			p1.Z = Load->GetVal(p1.X, p1.Y); //Val[i][j];
			P1 = ScreenPoint(p1); //	V1 = P.Z;
			p2.X = (ii+1) * hx;
			p2.Y = jj * hy;
			p2.Z = Load->GetVal(p2.X, p2.Y);// Load->Val[i+1][j];
			P2 = ScreenPoint(p2); //	V2 = P.Z;
			ratio = (p1.Z + p2.Z) * 0.5 / (Load->MaxVal);
			color = GetColor10(ratio);
			if (ratio > 1.e-6)
			DrawLineElement(pDC, color, P1, P2);

			if (::PeekMessage(&message, NULL, 0,0, PM_REMOVE)) {
				::TranslateMessage(&message);
				::DispatchMessage(&message);
			}
		if (STOP) return;
		} //i
	} //j
	}

	else {
		for (ii = Nxx-1; ii >= 1; ii--) {
		for (jj = 1; jj < Nyy-1; jj++) {
			p1.X = ii * hx;//(Load->StepX)* i;
			p1.Y = jj * hy;//(Load->StepY)* j;
			p1.Z = Load->GetVal(p1.X, p1.Y); //Val[i][j];
			P1 = ScreenPoint(p1); //	V1 = P.Z;
			p2.X = ii * hx;
			p2.Y = (jj+1) * hy;
			p2.Z = Load->GetVal(p2.X, p2.Y);// Load->Val[i+1][j];
			P2 = ScreenPoint(p2); //	V2 = P.Z;
			ratio = (p1.Z + p2.Z) * 0.5 / (Load->MaxVal);
			color = GetColor10(ratio);
			if (ratio > 1.e-6)
			DrawLineElement(pDC, color, P1, P2);
			if (::PeekMessage(&message, NULL, 0,0, PM_REMOVE)) {
				::TranslateMessage(&message);
				::DispatchMessage(&message);
			}
		if (STOP) return;
		} //j
	} //i
	}
		
	pDC->MoveTo(OrigX, OrigY);
	P.X = Xmax; P.Y = 0;  P.Z = 0;
	point = ScreenPoint(P);
	pDC->LineTo(point);

	pDC->MoveTo(OrigX, OrigY);
	P.X = 0; P.Y = Ymax; P. Z = 0;
	point = ScreenPoint(P);
	pDC->LineTo(point);
}

void CSetView:: DrawLineElement(CDC* pDC, COLORREF color, CPoint  P1, CPoint  P2)
{
	CPen Pen;
	Pen.CreatePen(PS_SOLID, 1, color);
	CPen * pOldPen = pDC->SelectObject(&Pen);
	pDC->MoveTo(P1); pDC->LineTo(P2); 
	pDC->SelectObject(pOldPen);
}
/////////////////////////////////////////////////////////////////////////////
// CSetView diagnostics

#ifdef _DEBUG
void CSetView::AssertValid() const
{
	CScrollView::AssertValid();
}

void CSetView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CSetView message handlers
void CSetView::OnLButtonDblClk(UINT /* nFlags */, CPoint /* point */) 
{
	STOP = TRUE;
	/*
	CString name = "Fig.wmf";
//	CFileDialog dlg(FALSE, "*.wmf", name);
	CFileDialog dlg(FALSE, "wmf | * ", name, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"Store Image as Windows Metafile (*.wmf) | *.wmf; *.WMF | All Files (*.*) | *.*||", NULL);

	if (dlg.DoModal() != IDOK) return;
	name = dlg.GetPathName();

	CRect rect;
	GetClientRect(rect);
	CSize size = rect.Size();
	CMetaFileDC dcm;	

//	VERIFY(dcm.Create(name));
//	CreateMetaFile(name);  //CreateEnhMetaFile(dcm, name, rect, "BTRLoad");
	dcm.Create(name);
	dcm.SetMapMode(MM_ANISOTROPIC);
	dcm.SetWindowOrg(0,0);
	dcm.SetWindowExt(size);

//	rect.DeflateRect(5,5,5,5);
//	dcm.Draw3dRect(rect, RGB(100,100,100), RGB(50,50,50));
	OnDraw(&dcm);

	HMETAFILE hMF = CloseMetaFile(dcm);
//	ASSERT(hMF != NULL);
	DeleteMetaFile(hMF);
	*/

	// save profiles in data-file - valid for cross-planes only!
	CBTRDoc* pDoc = (CBTRDoc*)GetDocument();
	if (Load == NULL || Load->MaxVal < 1.e-12) return;
	// int  num = pDoc->pMarkedPlate->Number;
	// double X = pDoc->pMarkedPlate->Orig.X;
	// double dHor = pDoc->pMarkedPlate->Orig.Y;
	// double dVert = pDoc->pMarkedPlate->Orig.Z;

	//if (pDoc->ShowProfiles) 
		pDoc->pMarkedPlate->WriteProfiles();

/*	// save image
	HBITMAP bitmap = CaptureWindow(*this);
	CString name = "Fig.bmp";
	CFileDialog dlg(FALSE, "bmp | * ", name, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"Store Image as BMP-file (*.bmp) | *.bmp; *.BMP | All Files (*.*) | *.*||", NULL);
	if (dlg.DoModal() != IDOK) return;
	name = dlg.GetPathName();
	SaveBitmap(bitmap, name);
*/	

//	CScrollView::OnLButtonDblClk(nFlags, point);
}


void CSetView::OnLButtonDown(UINT /* nFlags */, CPoint point) // taken from BTR-K
{
	STOP = TRUE;
	CBTRDoc* pDoc = (CBTRDoc*)GetDocument();
	if (RectArrayX.GetSize() < 1) return;
	if (RectArrayY.GetSize() < 1) return;

	if (!(pDoc->ShowProfiles)) return;
	if (Load == NULL || Load->MaxVal < 1.e-12) return;
	if (Load->iProf < 0 || Load->jProf < 0) return;
	CClientDC dc(this);
	OnPrepareDC(&dc);
	dc.DPtoLP(&point);	
	CPen Pen;
	Pen.CreatePen(PS_SOLID, 2, RGB(0, 200, 0)); //RGB(255, 107, 23) - ORANGE);
	CPen * pOldPen = dc.SelectObject(&Pen);
	CFont* pOldFont = dc.SelectObject(&smallfont);
	dc.SetTextColor(RGB(200, 0, 0));
	int icorr = -1, jcorr = -1;
	for (int i = 0; i <= Load->Nx; i++) {
		if (PtInRect(RectArrayX[i], point)) {
			dc.Ellipse(RectArrayX[i]);
			icorr = i; jcorr = Load->jProf;
			break;
		}
	}
	if (icorr < 0) {
	for (int j = 0; j <= Load->Ny; j++) {
		if (PtInRect(RectArrayY[j], point)) {
			dc.Ellipse(RectArrayY[j]);
			icorr = Load->iProf; jcorr = j;
			break;
		}
	}
	} //(icorr < 0)
	
	if (icorr < 0 || jcorr < 0 || icorr > Load->Nx-1 || jcorr > Load->Ny-1) return;
	
	double val = pDoc->pMarkedPlate->Load->Val[icorr][jcorr];
	CString S;
	S.Format("%g", val); //S.Format("%0.3g", val);
	dc.TextOut(point.x + 4, point.y - 15, S); 

	dc.SelectObject(pOldPen);
	dc.SelectObject(pOldFont);
	//CScrollView::OnLButtonDown(nFlags, point);
}
/*
void CSetView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if (Load == NULL || Load->MaxVal < 1.e-12) return;
	CClientDC dc(this);
	OnPrepareDC(&dc);
	dc.DPtoLP(&point);	
	CPen Pen;
	Pen.CreatePen(PS_SOLID, 2, RGB(0, 255, 0));
	CPen * pOldPen = dc.SelectObject(&Pen);
	int icorr = -1, jcorr = -1;
	for (int i = 0; i <= Load->Nx; i++) {
		if (PtInRect(RectArrayX[i], point)) {
			dc.Ellipse(RectArrayX[i]);
			icorr = i; jcorr = Load->jProf;
			break;
		}
	}
	if (icorr < 0) {
	for (int j = 0; j <= Load->Ny; j++) {
		if (PtInRect(RectArrayY[j], point)) {
			dc.Ellipse(RectArrayY[j]);
			icorr = Load->iProf; jcorr = j;
			break;
		}
	}
	} //(icorr < 0)

	dc.SelectObject(pOldPen);


	if (icorr < 0 && jcorr < 0) return;

		CWaitCursor wait;
		CBTRDoc* pDoc = (CBTRDoc*)GetDocument();
		double v1, v2, v3, v4, vcorr;

		if (pDoc->pMarkedPlate->SmLoad != NULL) 
			pDoc->pMarkedPlate->SmLoad->Clear();
		else 
			pDoc->pMarkedPlate->SmLoad = new CLoad(Load->Xmax, Load->Ymax, Load->StepX, Load->StepY);
			
		pDoc->pMarkedPlate->SmLoad->Copy(Load);
		//SmLoad->SmoothLoad(SmoothDegree);
		//vcorr = pDoc->pMarkedPlate->SmLoad->Val[icorr][jcorr];
		if (icorr > 0 && icorr < Load -> Nx ) {
			if (jcorr > 0 && jcorr < Load -> Ny) {
				v1 = pDoc->pMarkedPlate->SmLoad->Val[icorr-1][jcorr];
				v2 = pDoc->pMarkedPlate->SmLoad->Val[icorr+1][jcorr];
				v3 = pDoc->pMarkedPlate->SmLoad->Val[icorr][jcorr-1];
				v4 = pDoc->pMarkedPlate->SmLoad->Val[icorr][jcorr+1];
				vcorr = (v1 + v2 + v3 + v4) / 4;
			}
			if (jcorr = 0) {
				v3 = pDoc->pMarkedPlate->SmLoad->Val[icorr][jcorr+1];
				v4 = pDoc->pMarkedPlate->SmLoad->Val[icorr][jcorr+2];
				vcorr = v3 + (v3-v4);
			}
			if (jcorr = Load -> Ny) {
				v3 = pDoc->pMarkedPlate->SmLoad->Val[icorr][jcorr-1];
				v4 = pDoc->pMarkedPlate->SmLoad->Val[icorr][jcorr-2];
				vcorr = v3 + (v3-v4);
			}
			
		} // icorr > 0 && icorr < Load -> Nx
		if (icorr == 0) {
				v1 = pDoc->pMarkedPlate->SmLoad->Val[icorr+1][jcorr];
				v2 = pDoc->pMarkedPlate->SmLoad->Val[icorr+2][jcorr];
				vcorr = v1 + (v1-v2);
		} //icorr == 0
		if (icorr == Load -> Nx) {
				v1 = pDoc->pMarkedPlate->SmLoad->Val[icorr-1][jcorr];
				v2 = pDoc->pMarkedPlate->SmLoad->Val[icorr-2][jcorr];
				vcorr = v1 + (v1-v2);
		} // icorr == Load -> Nx

	
		pDoc->pMarkedPlate->SmLoad->Val[icorr][jcorr] = vcorr;

		pDoc->pMarkedPlate->SmLoad->SetSumMax();
		pDoc->pMarkedPlate->SmLoad->Comment = Load->Comment;

		pDoc->pMarkedPlate->SmoothDegree += 1;
		pDoc->pMarkedPlate->ShowLoadState();
	//	pDoc->pMarkedPlate->ShowLoad();

	
	CScrollView::OnLButtonDown(nFlags, point);
}
*/

void CSetView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	STOP = TRUE;
	CBTRDoc* pDoc = (CBTRDoc*)GetDocument();
	pDoc->ShowProfiles = TRUE;
	pDoc->pMarkedPlate->SmoothDegree = 0;
	pDoc->pMarkedPlate->ShowLoadState();
	
	CScrollView::OnRButtonDown(nFlags, point);
}

