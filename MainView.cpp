// MainView.cpp : implementation file
//

#include "stdafx.h"
#include "BTR.h"
#include "BTRDoc.h"
#include "MainView.h"
//#include "LoadView.h"
#include "config.h"
#include "ViewSizeDlg.h"
#include "LoadStepDlg.h"
#include "ResSmooth.h"
#include "BMP_utils.h"
#include "NumberDlg.h"
//#include "SurfDlg.h"
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//extern CLoadView * pLoadView;
extern CMainView  * pMainView;
//extern CDataView * pDataView;
extern int NofCalculated;

extern void ShowNothing();
/////////////////////////////////////////////////////////////////////////////
// CMainView

IMPLEMENT_DYNCREATE(CMainView, CScrollView)

CMainView::CMainView()
{
	pMainView = this;
	MenuPop.LoadMenu(IDR_MENUPOP);
	//	if (!BlackPen.CreatePen(PS_SOLID,   1, RGB(0,0,0)))	return;
	BlackPen.CreateStockObject(BLACK_PEN);
	if (!YellowPen.CreatePen(PS_SOLID, 2, RGB(50, 255, 100))) return;
	if (!ThinPen.CreatePen(PS_SOLID,   1, RGB(100,100,100))) return;
	if (!ThickPen.CreatePen(PS_SOLID, 2, RGB(200,200,200)))	return;
	if (!DotPen.CreatePen(PS_DOT,    1, RGB(100,100,100)))	return;
	if (!MarkPen.CreatePen(PS_SOLID, 2, RGB(255,0,200)))	return;
	if (!RedPen.CreatePen(PS_SOLID,   3, RGB(255,0,0)))	return;
	if (!RosePen.CreatePen(PS_SOLID,  1, RGB(255,0,0)))	return;
	if (!MamugPen.CreatePen(PS_SOLID, 1, RGB(200,50,255)))	return;
	if (!SingapPen.CreatePen(PS_SOLID, 1, RGB(200,50,255)))	return;
	
	if (!GreenPen.CreatePen(PS_SOLID, 1, RGB(0,150,0)))	return;
	if (!BluePen.CreatePen(PS_SOLID,   1, RGB(0,0,255)))	return;
	if (!AtomPen.CreatePen(PS_SOLID,  1, RGB(50,0,255)))	return;
	//	if (YellowBrush.CreateSolidBrush(RGB(255,255,220))) return;

	// coordinates/labels font
	smallfont.CreateFont(-10, 0, 0, 0, 600, FALSE, FALSE, 0,
		0, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
		DEFAULT_PITCH | FF_MODERN, "Arial");//"Courier"); 

	//
	font.CreateFont(-10, 0, 0, 0, 100, FALSE, FALSE, 0,
		ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
		DEFAULT_PITCH | FF_MODERN, "System");

	// comments font
	midfont.CreateFont(-14, 0, 0, 0, 600, FALSE, FALSE, 0,
		ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
		DEFAULT_PITCH | FF_MODERN, "Courier");
	//   CPen * pOldPen = pDC->SelectObject( &GreyPen);

}

CMainView::~CMainView()
{
}


BEGIN_MESSAGE_MAP(CMainView, CScrollView)
	//{{AFX_MSG_MAP(CMainView)
	ON_COMMAND(ID_VIEW_SIZE, OnViewSize)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_PLATE_LOAD, OnPlateLoad)
	ON_WM_LBUTTONDBLCLK()
	ON_COMMAND(ID_PLATE_EMIT, OnPlateEmit)
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	//	ON_COMMAND(ID_PLATE_SELECT, OnPlateSelect)
	//	ON_COMMAND(ID_STOP, OnStop)
	ON_COMMAND(ID_PLATE_PROPERTIES, OnPlateProperties)
	ON_COMMAND(ID_PLATE_CLEAR, OnPlateClear)
	ON_COMMAND(ID_PLATE_SCALE, OnPlateScale)
	ON_WM_MOUSEMOVE()
	ON_COMMAND(ID_PLATE_DELETE, OnPlateDelete)
	ON_COMMAND(ID_PLATE_SAVE, OnPlateSave)
	ON_COMMAND(ID_PLATE_SMOOTH, OnPlateSmooth)
	ON_WM_LBUTTONUP()
	ON_COMMAND(ID_PLATE_SELECTALLPLATES, OnPlateSelectallplates)
	ON_COMMAND(ID_PLATE_3DPLOT, OnPlate3dplot)
	ON_COMMAND(ID_PLATE_MAXPROFILES, OnPlateMaxprofiles)
	ON_COMMAND(ID_PLATE_ZOOMIN, OnPlateZoomin)
	ON_COMMAND(ID_PLATE_ZOOMOUT, OnPlateZoomout)
	ON_COMMAND(ID_VIEW_FITONOFF, OnViewFitonoff)
	ON_UPDATE_COMMAND_UI(ID_VIEW_FITONOFF, OnUpdateViewFitonoff)
	ON_COMMAND(ID_VIEW_BEAM, OnViewBeam)
	ON_UPDATE_COMMAND_UI(ID_VIEW_BEAM, OnUpdateViewBeam)
	ON_COMMAND(ID_VIEW_FIELDS, OnViewFields)
	ON_UPDATE_COMMAND_UI(ID_VIEW_FIELDS, OnUpdateViewFields)
	ON_COMMAND(ID_VIEW_NUMBERING, OnViewNumbering)
	ON_UPDATE_COMMAND_UI(ID_VIEW_NUMBERING, OnUpdateViewNumbering)
	ON_COMMAND(ID_PLATE_CONTOURS, OnPlateContours)
	//}}AFX_MSG_MAP
	ON_UPDATE_COMMAND_UI(ID_PLATE_SELECT, &CMainView::OnUpdatePlateSelect)
	//	ON_COMMAND(ID_STOP, &CMainView::OnStop)
	ON_WM_RBUTTONDBLCLK()
	ON_WM_VSCROLL()
	ON_WM_SIZE()
//	ON_COMMAND(ID_LOG_SAVE, &CMainView::OnLogSave)
//	ON_UPDATE_COMMAND_UI(ID_LOG_SAVE, &CMainView::OnUpdateLogSave)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMainView drawing

void CMainView::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();

	CSize sizeTotal;// = GetTotalSize();
	sizeTotal.cx = 500; sizeTotal.cy = 100;
	SetScrollSizes(MM_TEXT, sizeTotal);

	/*	CSize sizeTotal(2100, 1800);//(11520, 15120); // 8 x 10.5 inches 1800
	CSize sizePage(	sizeTotal.cx /2, 	sizeTotal.cy /2);
	CSize sizeLine(	sizeTotal.cx /100, sizeTotal.cy /100);
	SetScrollSizes(MM_LOMETRIC, sizeTotal, sizePage, sizeLine);
	m_rectPrint = CRect(0, -100, 1900, -2800);
	//SetScrollSizes(MM_TEXT, sizeTotal);
	//	SetScrollSizes(MM_TEXT, sizeTotal, sizePage, sizeLine);*/

	MODE_FIT = FALSE;// TRUE;
	//	MODE_ZOOM = FALSE;
	STOP = TRUE;
	//	MODE_SELECT = FALSE;
	//	Progress = 0;
	//	MODE_BOX = FALSE;
	BoxRect.SetRect(0,0, 0,0); //sizeTotal.cx,sizeTotal.cy);
	//	ZOOM_FINISHED = TRUE;
	SHOW_FIELDS = FALSE;
	SHOW_NUMBERS = TRUE;
	SHOW_BEAM = TRUE;
	ZOOM_IN = FALSE;
	ZoomCoeffX = 1;
	ZoomCoeffY = 1;
	ZoomCoeffZ = 1;


}

void CMainView::OnDraw(CDC* pDC)
{
	//AfxMessageBox("CMainView OnDraw");

	pMainView = this;
	CBTRDoc* pDoc = (CBTRDoc*) GetDocument();
	CPen pen;
	pen.CreatePen(PS_SOLID, 1, RGB(200,200,200));
	CPen * pOldPen = pDC->SelectObject(&pen);

	CRect rect, rectPlan, rectSide;
	CSize size, OldSize;

	//if (MODE_FIT) {// && !MODE_ZOOM) {
	GetClientRect(rect);
	size = rect.Size();
	/*}
	else { // (!MODE_FIT) { // large view
	size = GetTotalSize();
	rect.SetRect(-BoxRect.left, -BoxRect.top, size.cx - BoxRect.left, size.cy - BoxRect.top);
	}*/
	pDC->Rectangle(rect);

	double width = pDoc->AreaHorMax - pDoc->AreaHorMin;
	double height = pDoc->AreaVertMax - pDoc->AreaVertMin;
	double wh_ratio = width/(width+height);
	if (wh_ratio < 0.3) wh_ratio = 0.3;
	if (wh_ratio > 0.7) wh_ratio = 0.7;
	CSize sizeP;
	sizeP.cx = size.cx; 
	sizeP.cy =  (int)(size.cy * wh_ratio); 
	if (!MODE_FIT) // real scale
		sizeP.cy =  (int)(size.cy/2);
	rectPlan.top = rect.top+10; rectPlan.bottom = rectPlan.top + sizeP.cy -5;  //rect.Size().cy / 2 -5;
	rectSide.top = rectPlan.bottom + 5; rectSide.bottom = rect.bottom-10;
	rectPlan.left = rectSide.left = rect.left +10; 
	rectPlan.right = rectSide.right = rect.right-10;

	UpdateScales(rectPlan, rectSide);
	RectPlan = rectPlan; RectSide = rectSide;

	pDC->Rectangle(RectPlan);
	pDC->Rectangle(RectSide);

	/*	if (MODE_FIT) {
	CBrush brush;
	brush.CreateStockObject(LTGRAY_BRUSH);
	CBrush * pOldBrush = pDC->SelectObject(&brush);
	pDC->FloodFill(10,RectPlan.bottom + 2, RGB(200,200,200));
	pDC->SelectObject(pOldBrush);
	}
	*/
	pDC->SelectObject(pOldPen);

	C3Point Origin(OrigX, OrigY, OrigZ);
	C3Point Scale(ScaleX, ScaleY, ScaleZ);
	double xmax = pDoc->AreaLong;
	double Ymin, Ymax, Zmin, Zmax;

	//STOP = FALSE;
/*	if (SHOW_BEAM) {
		if (pDoc->OptSINGAP) ShowSINGAP();
		else ShowMAMuG();
	} // show beam
	else
		ShowBeamPlanes(); // beam axial crosssections
*/
	if (pDoc->OptSINGAP) ShowSINGAP();
	else ShowMAMuG();
	// draw beam centerline
	pDC->SetROP2(R2_NOT);
	C3Point P = pDoc->GetBeamFootLimits(xmax, Ymin, Ymax, Zmin, Zmax);
	pDC->MoveTo(OrigX, OrigY); 
	pDC->LineTo(OrigX + (int)(xmax * ScaleX), OrigY - (int)(P.Y * ScaleY));
	pDC->MoveTo(OrigX, OrigZ); 
	pDC->LineTo(OrigX + (int)(xmax * ScaleX), OrigZ - (int)(P.Z * ScaleZ));
	pDC->SetROP2(R2_COPYPEN);

	if (SHOW_FIELDS) {
		//if (pDoc->TaskRID && *(pDoc->AreaLong) < 10 ) zoom = 5000;

		if (pDoc->FieldLoaded && pDoc->MFcoeff != 0) { // MF switched ON

			if (pDoc->MagFieldDim == 1)
				pDoc->BField->DrawMF(pDC, Origin, Scale);
			else
			{
				pDoc->BField3D->DrawMF(pDC, Origin, Scale, xmax, 0);
				pDoc->BField3D->DrawMF(pDC, Origin, Scale, xmax, -0.7);
				pDoc->BField3D->DrawMF(pDC, Origin, Scale, xmax, 0.7);
			}

		} // MF switched ON

		if (pDoc->PressureLoaded && pDoc->GasCoeff > 1.e-6) { // Gas switched ON
			switch (pDoc->PressureDim) {
			case 0: break;
			case 1: pDoc->GField->DrawGas(pDC, Origin, Scale); break;
			case 2: pDoc->GFieldXZ->DrawGas(pDC, Origin, Scale); break;
			}

		}

		pDoc->RIDField->DrawU(pDC, Origin, Scale);
	} // show fields

	if (!pDoc->OptFree) ShowComments();// Dias, Comp names
		
	ShowNBLine();	
	
	ShowCoord();
	//if (pDoc->OptBeamInPlasma) 	
	ShowTor();
	//ShowTraced(pDC); // too slow

	CFont * pOldFont = (CFont *)(pDC->SelectObject(&midfont));
	pDC->SetTextColor(RGB(0,150,0));
	pDC->TextOut(rectPlan.left + 100, rectPlan.top +1, "HORIZONTAL  (PLAN)  VIEW");
	pDC->TextOut(rectSide.left + 100, rectSide.top +1, "VERTICAL  (SIDE)  VIEW");
	pDC->SelectObject(pOldFont);

	pDC->SetTextColor(RGB(0,0,0));

}

void CMainView:: UpdateScales(CRect Plan, CRect Side)
{
	CBTRDoc* pDoc = (CBTRDoc*)GetDocument();
	CRect areaPlan = Plan;  areaPlan.DeflateRect(40, 20, 30, 20);
	CRect areaSide = Side;  areaSide.DeflateRect(40, 20, 30, 30);

	Xmin = 0;
	Xmax = pDoc->AreaLong;
	Ymin = pDoc->AreaHorMin;
	Ymax = pDoc->AreaHorMax;
	Zmin = pDoc->AreaVertMin;
	Zmax = pDoc->AreaVertMax;

	ScaleX = areaPlan.Size().cx / Xmax;
	ScaleY = areaPlan.Size().cy / (Ymax - Ymin);
	ScaleZ = areaSide.Size().cy / (Zmax - Zmin);

	if (!MODE_FIT) { // && !MODE_ZOOM) {
		ScaleY = Min(ScaleY, ScaleZ);
		ScaleY = Min(ScaleY, ScaleX);
		ScaleZ = ScaleY;
		ScaleX = ScaleY;
		//double Ky = areaPlan.Size().cy /(Ymax - Ymin)/ScaleY;
		//double Kz = areaSide.Size().cy /(Zmax - Zmin)/ScaleZ;
		Ymin *= 1.2; Ymax *= 1.2; Zmin *= 1.2; Zmax *= 1.2;

	}

	OrigX = areaPlan.left;
	OrigY = areaPlan.top + (int)(areaPlan.Size().cy * Ymax / (Ymax - Ymin));
	if (OrigY < areaPlan.top) OrigY = areaPlan.top +10;
	OrigZ = areaSide.top + (int)(areaSide.Size().cy * Zmax / (Zmax - Zmin));
	if (OrigZ < areaSide.top) OrigZ = areaSide.top +10;
	//if (!MODE_FIT) OrigZ = areaSide.top + (int)(areaSide.Size().cy * 2/ 5);

	if (ZOOM_IN) {// && MODE_FIT) {
		if (ZoomPoint.Z > 999) { // ZOOM Plan view
			//ZoomCoeffY *= 1.5;
			OrigY += (int)((ZoomCoeffY - 1) * ZoomPoint.Y * ScaleY); 
			ScaleY *= ZoomCoeffY;
		}
		else if (ZoomPoint.Y > 999) { // ZOOM Side view
			//ZoomCoeffZ *= 1.5;
			OrigZ += (int)((ZoomCoeffZ - 1) * ZoomPoint.Z * ScaleZ); 
			ScaleZ *= ZoomCoeffZ;
		}
		OrigX -= (int)((ZoomCoeffX - 1) * ZoomPoint.X * ScaleX);
		ScaleX *= ZoomCoeffX;
		//ZOOM_IN = FALSE;
	}

}

void CMainView:: ShowCoord()
{
	CDC* pDC = GetDC();
	int x,y, w;
	CString S;
	CFont * pOldFont = (CFont *)(pDC->SelectObject(&smallfont));
	pDC->SetTextColor(RGB(0,0,0));
	x = OrigX+ (int)(Xmin*ScaleX)-2;   y = OrigY;			pDC->MoveTo(x,y); 
	//pDC->TextOut(x-10, y-7, "0");
	x = OrigX + (int)(Xmax*ScaleX)+5;  y = OrigY;		pDC->LineTo(x,y); 
	pDC->TextOut(x+10, y-5, "X");
	x = OrigX;  y = OrigY - (int)(Ymin*ScaleY)+1;			pDC->MoveTo(x,y); 
	x = OrigX;  y = OrigY - (int)(Ymax*ScaleY)-1;			pDC->LineTo(x,y); 
	pDC->TextOut(x-5, y-6, "Y");
	x = OrigX+ (int)(Xmin*ScaleX)-2; y =  OrigZ;			pDC->MoveTo(x,y); 
	//pDC->TextOut(x-10, y-7, "0");
	x = OrigX + (int)(Xmax*ScaleX)+5; y = OrigZ;			pDC->LineTo(x,y); 
	pDC->TextOut(x+10, y-5, "X");
	x = OrigX;  y =  OrigZ - (int)(Zmin*ScaleZ)+1;			pDC->MoveTo(x,y); 
	x = OrigX;  y =  OrigZ - (int)(Zmax*ScaleZ)-1;			pDC->LineTo(x,y); 
	pDC->TextOut(x-5, y-6, "Z");

	double hx = 1;
	double Ysum = Ymax - Ymin;//RectPlan.Height() / ScaleY;
	C3Point P = GetMantOrder(Ysum * 0.2);
	double hy = P.X * P.Z;//0.2;
	double Zsum = Zmax - Zmin;//RectSide.Height() / ScaleZ; //
	P = GetMantOrder(Zsum * 0.2);
	double hz = P.X * P.Z;//0.5;

	int imax = (int)floor(Xmax/hx);
	int jmin = (int)ceil(Ymin/hy);
	int kmin = (int)ceil(Zmin/hz);
	int jmax = (int)floor(Ymax/hy);
	int kmax = (int)floor(Zmax/hz);

	for (int i = 1; i <= imax; i++) {
		x = OrigX + (int)(i*hx*ScaleX); 	
		y = OrigY - 3;  pDC->MoveTo(x,y); 
		y = OrigY + 3; pDC->LineTo(x,y); 
		S.Format("%d", (int)(i*hx));
		//if (i / 5 * 5 == i)	
		pDC->TextOut(x-3, y, S);
		y = OrigZ - 3;  pDC->MoveTo(x,y); 
		y = OrigZ + 2; pDC->LineTo(x,y); 
		//if (i / 5 * 5 == i) 
		pDC->TextOut(x-3, y, S);
	}
	for (int j = jmin; j <= jmax; j++) {
		y = OrigY - (int)(j*hy*ScaleY); 
		x = OrigX - 3; pDC->MoveTo(x,y); 
		x = OrigX + 3; pDC->LineTo(x,y); 
		S.Format("%g", (j*hy));
		if(pDC->m_hAttribDC != NULL) w = pDC->GetTextExtent(S).cx; //w = pDC->GetTextExtent(S).cx;
		else w = 15;
		pDC->TextOut(x-10-w, y-5, S);
	}
	for (int k = kmin; k <= kmax; k++) {
		y = OrigZ - (int)(k*hz*ScaleZ); 
		x = OrigX - 2; pDC->MoveTo(x,y); 
		x = OrigX + 3; pDC->LineTo(x,y); 
		S.Format("%g", (k*hz));
		if(pDC->m_hAttribDC != NULL) w = pDC->GetTextExtent(S).cx;//w = pDC->GetTextExtent(S).cx;
		else w = 15;
		pDC->TextOut(x-10-w, y-5, S);
	}

	pDC->SelectObject(pOldFont);
	ReleaseDC(pDC);

}

void CMainView:: ShowTor()
{
	CBTRDoc* pDoc = (CBTRDoc*)GetDocument();
	if (!pDoc->OptBeamInPlasma) return;
	if (pDoc->AreaLong < pDoc->TorCentreX) return;

	CDC* pDC = GetDC();
	CPen * pOldPen = pDC->SelectObject(&ThinPen);

	double X, Y, Z, Z1, Z2, dX, dY;
	int x, y, z, r;
	int x1, y1, x2, y2, x3, y3, x4, y4;
	// centre (cross)
	X = pDoc->TorCentre.X;
	Y = pDoc->TorCentre.Y;
	Z = pDoc->TorCentre.Z;

	x = OrigX + (int)(X * ScaleX) - 20;
	y = OrigY - (int)(Y * ScaleY);
	pDC->MoveTo(x, y);
	x = OrigX + (int)(X * ScaleX) + 20;
	pDC->LineTo(x, y);
	x = OrigX + (int)(X * ScaleX);
	y = OrigY - (int)(Y * ScaleY) - 20;
	pDC->MoveTo(x, y);
	y = OrigY - (int)(Y * ScaleY) + 20;
	pDC->LineTo(x, y);

	x = OrigX + (int)(X * ScaleX) - 20;
	z = OrigZ - (int)(Z * ScaleZ);
	pDC->MoveTo(x, z);
	x = OrigX + (int)(X * ScaleX) + 20;
	pDC->LineTo(x, z);
	x = OrigX + (int)(X * ScaleX);
	z = OrigZ - (int)(Z * ScaleZ) - 20;
	pDC->MoveTo(x, z);
	z = OrigZ - (int)(Z * ScaleZ) + 20;
	pDC->LineTo(x, z);

	// Rmajor arc
	pDC->SelectObject(&BlackPen); // (&ThickPen);
	Y = pDoc->AreaHorMin;
	if (!MODE_FIT) {
		Y = (OrigY - RectPlan.bottom) / ScaleY;
		if (Y < pDoc->TorCentre.Y - pDoc->FWRmax) Y = pDoc->TorCentre.Y - pDoc->FWRmax;
	}
	dY = Y - pDoc->TorCentre.Y;
	dX = sqrt(pDoc->FWRmax * pDoc->FWRmax - dY * dY);
	X = pDoc->TorCentre.X - dX;
	x = OrigX + (int)(X * ScaleX);
	y = OrigY - (int)(Y * ScaleY);
	pDC->MoveTo(x, y);
	x4 = x; y4 = y; // arc start
	X = pDoc->TorCentre.X + dX;
	x = OrigX + (int)(X * ScaleX);
	pDC->MoveTo(x, y); //pDC->LineTo(x,y);
	x3 = x; y3 = y; // arc end
	X = pDoc->TorCentre.X;
	Y = pDoc->TorCentre.Y;
	x = OrigX + (int)(X * ScaleX);
	y = OrigY - (int)(Y * ScaleY);
	X = pDoc->TorCentre.X - pDoc->FWRmax;
	Y = pDoc->TorCentre.Y + pDoc->FWRmax;
	x1 = OrigX + (int)(X * ScaleX); // left top of bound rect 
	y1 = OrigY - (int)(Y * ScaleY);
	X = pDoc->TorCentre.X + pDoc->FWRmax;
	Y = pDoc->TorCentre.Y - pDoc->FWRmax;
	x2 = OrigX + (int)(X * ScaleX); // right bottom of bound rect
	y2 = OrigY - (int)(Y * ScaleY);

	pDC->Arc(x1, y1, x2, y2, x3, y3, x4, y4);

	// Rminor arc
	Y = pDoc->AreaHorMin;
	if (!MODE_FIT) {
		Y = (OrigY - RectPlan.bottom) / ScaleY;
		if (Y < pDoc->TorCentre.Y - pDoc->FWRmin) Y = pDoc->TorCentre.Y - pDoc->FWRmin;
	}
	dY = Y - pDoc->TorCentre.Y;
	if (fabs(dY) <=  pDoc->FWRmin) {
		dX = sqrt(pDoc->FWRmin * pDoc->FWRmin - dY * dY);
		X = pDoc->TorCentre.X - dX;
		x = OrigX + (int)(X * ScaleX);
		y = OrigY - (int)(Y * ScaleY);
		pDC->MoveTo(x, y);
		x4 = x; y4 = y; // arc start
		X = pDoc->TorCentre.X + dX;
		x = OrigX + (int)(X * ScaleX);
		pDC->MoveTo(x, y); //pDC->LineTo(x,y);
		x3 = x; y3 = y; // arc end
		X = pDoc->TorCentre.X;
		Y = pDoc->TorCentre.Y;
		x = OrigX + (int)(X * ScaleX);
		y = OrigY - (int)(Y * ScaleY);
		X = pDoc->TorCentre.X - pDoc->FWRmin;
		Y = pDoc->TorCentre.Y + pDoc->FWRmin;
		x1 = OrigX + (int)(X * ScaleX); // left top of bound rect 
		y1 = OrigY - (int)(Y * ScaleY);
		X = pDoc->TorCentre.X + pDoc->FWRmin;
		Y = pDoc->TorCentre.Y - pDoc->FWRmin;
		x2 = OrigX + (int)(X * ScaleX); // right bottom of bound rect
		y2 = OrigY - (int)(Y * ScaleY);

		pDC->Arc(x1, y1, x2, y2, x3, y3, x4, y4);
	} // fabs(dY) <  pDoc->FWRmin

	// vertical view
	C3Point P;
	int Zmin = RectSide.top;
	int Zmax = RectSide.bottom;
	// FW profile along beam flight Y = 0
	CPen pen;
	pen.CreatePen(PS_DASH, 1, RGB(200,200,230));
	pDC->SelectObject(&pen);
	pDC->SetTextColor(RGB(200,200,230));
	X = pDoc->TorCentre.X; //Z = pDoc->TorCentre.Z;
	x = OrigX + (int)(X * ScaleX);
	z = RectSide.top + 30;//OrigZ - (int)(Z * ScaleZ);
	if (!MODE_FIT) pDC->TextOutA(x - 20, z, "Y = 0");

	// scan from Xmin to Xmax, then back
	X = pDoc->PlasmaXmin + 0.001;//TorCentre.X - pDoc->FWRmax;
	Y = 0; // pDoc->TorCentre.Y +pDoc->FWRmax;
	P = pDoc->GetFW_Z(X,Y,1); // z > 0
	Z = P.Y;
	if (Z > 999) Z = 0;
	x = OrigX + (int)(X * ScaleX);  
	z = OrigZ - (int)(Z * ScaleZ);
	pDC->MoveTo(x,z);
	
	Z = P.Z;
	if (Z > 999) Z = 0;
	x = OrigX + (int)(X * ScaleX);  
	z = OrigZ - (int)(Z * ScaleZ);
	pDC->LineTo(x,z);
	
	while (X < pDoc->PlasmaXmax) //TorCentre.X + pDoc->FWRmax) 
	{
		X += 0.001;
		P = pDoc->GetFW_Z(X,Y,1); // z > 0
		Z = P.Y;
		if (Z > 999) Z = 0;
		x = OrigX + (int)(X * ScaleX);  
		z = OrigZ - (int)(Z * ScaleZ);
		if (z>Zmin && z<Zmax) pDC->LineTo(x,z);
		else pDC->MoveTo(x,z);
		Z = P.Z;
		if (Z > 999) Z = 0;
		x = OrigX + (int)(X * ScaleX);  
		z = OrigZ - (int)(Z * ScaleZ);
		if (z>Zmin && z<Zmax) pDC->LineTo(x,z);// in rect
		else pDC->MoveTo(x,z);
	}
	while (X > pDoc->PlasmaXmin) //TorCentre.X - pDoc->FWRmax) 
	{
		X -= 0.001;
		P = pDoc->GetFW_Z(X,Y,-1); // z < 0
		Z = P.Y;
		if (Z > 999) Z = 0;
		x = OrigX + (int)(X * ScaleX);  
		z = OrigZ - (int)(Z * ScaleZ);
		if (z>Zmin && z<Zmax) pDC->LineTo(x,z);// in rect
		else pDC->MoveTo(x,z);
		Z = P.Z;
		if (Z > 999) Z = 0;
		x = OrigX + (int)(X * ScaleX);  
		z = OrigZ - (int)(Z * ScaleZ);
		if (z>Zmin && z<Zmax) pDC->LineTo(x,z); // in rect
		else pDC->MoveTo(x,z);
	}

	// FW profile at Y = TorCentre.Y
	pDC->SelectObject(&BlackPen);//(&ThickPen);
	int Nf = pDoc->FWdata.GetUpperBound();
	X = pDoc->TorCentre.X + pDoc->FWdata[0].X;//R
	Z = pDoc->TorCentre.Z + pDoc->FWdata[0].Y;
	x = OrigX + (int)(X * ScaleX);  
	z = OrigZ - (int)(Z * ScaleZ);
	double Ztop = pDoc->FWdata[0].Y; // top/bottom to connect 
	double Xtop = pDoc->FWdata[0].X;
	pDC->MoveTo(x, z);
	for (int i = 1; i <= Nf; i++) { // right half
		X = pDoc->TorCentre.X + pDoc->FWdata[i].X;//R
		Z = pDoc->TorCentre.Z + pDoc->FWdata[i].Y;
		x = OrigX + (int)(X * ScaleX);  
		z = OrigZ - (int)(Z * ScaleZ);
		if (pDoc->FWdata[i].Y > Ztop) { // top/bottom to connect 
			Xtop = pDoc->FWdata[i].X;
			Ztop = pDoc->FWdata[i].Y;
		}
		if (z>Zmin && z<Zmax) pDC->LineTo(x,z);// in rect
		else pDC->MoveTo(x,z);
	}

	X = pDoc->TorCentre.X - pDoc->FWdata[0].X;//R
	Z = pDoc->TorCentre.Z + pDoc->FWdata[0].Y;
	x = OrigX + (int)(X * ScaleX);  
	z = OrigZ - (int)(Z * ScaleZ);
	pDC->MoveTo(x,z);
	for (int i = 1; i <= Nf; i++) { // left half
		X = pDoc->TorCentre.X - pDoc->FWdata[i].X;//R
		Z = pDoc->TorCentre.Z + pDoc->FWdata[i].Y;
		x = OrigX + (int)(X * ScaleX);  
		z = OrigZ - (int)(Z * ScaleZ);
		if (z>Zmin && z<Zmax) pDC->LineTo(x,z);
		else pDC->MoveTo(x,z);
	}
	// top line
	X = pDoc->TorCentre.X + Xtop;
	Z = pDoc->TorCentre.Z + Ztop;
	x = OrigX + (int)(X * ScaleX);
	z = OrigZ - (int)(Z * ScaleZ);
	pDC->MoveTo(x, z);
	X = pDoc->TorCentre.X - Xtop;//Z = Ztop;
	x = OrigX + (int)(X * ScaleX); // z = OrigZ - (int)(Z * ScaleZ);
	if (z>Zmin && z<Zmax) pDC->LineTo(x, z);
	// bottom line
	X = pDoc->TorCentre.X + Xtop;
	Z = pDoc->TorCentre.Z - Ztop;
	x = OrigX + (int)(X * ScaleX);
	z = OrigZ - (int)(Z * ScaleZ);
	pDC->MoveTo(x, z);
	X = pDoc->TorCentre.X - Xtop;//Z = -Ztop;
	x = OrigX + (int)(X * ScaleX); // z = OrigZ - (int)(Z * ScaleZ);
	if (z>Zmin && z<Zmax) pDC->LineTo(x, z);
		
	pDC->SelectObject(pOldPen);
	ReleaseDC(pDC);

}


void CMainView:: ShowNBLine()
{
	CDC* pDC = GetDC();
	CFont * pOldFont = (CFont *)(pDC->SelectObject(&smallfont));
	CBTRDoc* pDoc = (CBTRDoc*)GetDocument();
	PtrList & List = pDoc->PlatesList;
	CPlate* pPlate;
	POSITION pos = List.GetHeadPosition();
	while (pos != NULL) {
		pPlate = List.GetNext(pos);
		pPlate->DrawPlate(this, pDC, SHOW_BEAM);
	}
	pDC->SelectObject(pOldFont);
	ReleaseDC(pDC);
}

/*void CMainView:: DrawManMF(CDC* pDC)
{
CBTRDoc* pDoc = (CBTRDoc*)GetDocument();
CPen RedPen, GreenPen, BluePen;
if (!RedPen.CreatePen(PS_DOT, 2, RGB(255, 100, 100))) return;
if (!GreenPen.CreatePen(PS_DOT, 2, RGB(100, 255, 150))) return;
if (!BluePen.CreatePen(PS_DOT, 2, RGB(100, 100, 255))) return;
int Top = 100;
double x, bx, by, bz;
C3Point P;
double Bmax = 0;

for (int i = 0; i <= pDoc->MFdata.GetUpperBound(); i++) {
bx = pDoc->MFdata[i].X; by = pDoc->MFdata[i].Y; bz = pDoc->MFdata[i].Z;
Bmax = Max(fabs(bx), Bmax); Bmax = Max(fabs(by), Bmax); Bmax = Max(fabs(bz), Bmax); 
}

CPen * pOldPen = pDC->SelectObject(&RedPen);
x = 0;  
pDC->MoveTo(OrigX, OrigZ); 
while (x < Xmax) {
P = pDoc->GetManMF(x);
pDC->LineTo(OrigX+(int)(x * ScaleX), OrigZ - (int)(P.X * Top / Bmax));
x += 0.05;
}

pDC->SelectObject(&GreenPen);
x = 0;  
pDC->MoveTo(OrigX, OrigZ); 
while (x < Xmax) {
P = pDoc->GetManMF(x);
pDC->LineTo(OrigX+(int)(x * ScaleX), OrigZ - (int)(P.Y * Top / Bmax));
x += 0.05;
}

pDC->SelectObject(&BluePen);
x = 0;  
pDC->MoveTo(OrigX, OrigZ); 
while (x < Xmax) {
P = pDoc->GetManMF(x);
pDC->LineTo(OrigX+(int)(x * ScaleX), OrigZ - (int)(P.Z * Top / Bmax));
x += 0.05;
}
pDC->SelectObject(pOldPen);
}
*/
void CMainView::ShowBeamPlanes()
{
	// temporary removed
/*	MSG message;
	STOP = FALSE;
	CDC* pDC = GetDC();
	CBTRDoc* pDoc = (CBTRDoc*)GetDocument();
	int origX = OrigX;
	int origY = OrigY;
	int origZ = OrigZ;
	
	CPlate * plane1 = pDoc->pBeamHorPlane;
	CPlate * plane2 = pDoc->pBeamVertPlane;
	
	double Xmax = plane1->Xmax;
	double DimX = Xmax;//DimX = right - left;
	double Ymax = plane1->Ymax;
	double DimY = Ymax;//DimY = top - bot;
	double Zmax = plane2->Ymax;
	double DimZ = Zmax;//
	double StepX = plane1->Load->StepX;
	double StepY = plane1->Load->StepY;
	double StepZ = plane2->Load->StepY;
	int xmin = origX + (int)(plane1->Orig.X * ScaleX); // origX + (int)((x) * ScaleX);
	int ymin = origY - (int)(plane1->Orig.Y * ScaleY); //plane1->Orig.Y;
	int zmin = origZ - (int)(plane2->Orig.Z * ScaleZ); //plane2->Orig.Y;
	int xmax = xmin + (int)(Xmax * ScaleX);
	int ymax = ymin - (int)(Ymax * ScaleY);
	int zmax = zmin - (int)(Zmax * ScaleZ);
	double MaxVal1 = plane1->Load->MaxVal;
	double MaxVal2 = plane2->Load->MaxVal;
	double MaxVal = max(MaxVal1, MaxVal2);

	CBrush brush;
	CBrush * oldbrush;
	brush.CreateSolidBrush(RGB(245, 245, 245));
	oldbrush = pDC->SelectObject(&brush);
	pDC->Rectangle(xmin, ymin, xmax, ymax);
	pDC->Rectangle(xmin, zmin, xmax, zmax);
	//pDC->Rectangle(pLV->LoadRect);
	pDC->SelectObject(oldbrush);
	
	if (MaxVal < 1.e-10){
		ReleaseDC(pDC);
		return;
	}

////////////////////////////
	double Val, ratio1 = 0, ratio2 = 0;
	//int i, j, k;
	double x, y, z;
	COLORREF color1, color2;
	//CString S;

	int ii, jj, kk, Nxx, Nyy, Nzz, ix, jy, kz;
	double hx = StepX * 0.3; // resolution = load steps divided by 2
	double hy = StepY * 0.3; 
	double hz = StepZ * 0.3;

	Nxx = (int)floor(DimX / hx);
	Nyy = (int)floor(DimY / hy);
	Nzz = (int)floor(DimZ / hz);

	int rx = (int)(hx * ScaleX / 2) + 3;//  = (int)(StepX *  pLV->ScaleX / 2)+1;
	//if (rx<1) rx = 1; // fill point 'radius'
	int ry = (int)(hy * ScaleY / 2) + 3;//  = (int)(StepY *  pLV->ScaleY / 2)+1;
	//if (ry<1) ry = 1; // fill point 'radius'
	int rz = (int)(hz * ScaleZ / 2) + 3;//  = (int)(StepY *  pLV->ScaleY / 2)+1;
	//if (rz<1) rz = 1; // fill point 'radius'

	for (ii = 0; ii < Nxx; ii++) {
		x = (ii + 0.5)*hx; //left + (ii + 0.5)*hx
		for (jj = 0; jj < Nyy; jj++) {
			y = (jj + 0.5) * hy; //bot + (jj + 0.5)*hy;
			ix = xmin + (int)((x) * ScaleX);
				//ix = origX + (int)((x - left) * pLV->ScaleX);
			jy = ymin - (int)((y) * ScaleY);
				//jy = origY - (int)((y - bot) * pLV->ScaleY);
			
			if (MaxVal1 > 1.e-10) 	
					ratio1 = plane1->Load->GetVal(x, y) / MaxVal1;// Val[i][j] / MaxVal;
					
			else 	ratio1 = 1; // too small
						
			color1 = GetColor10(ratio1);
					//		color = RGB(Red(ratio), Green(ratio), Blue(ratio));
					//		color = RGB(Gray(ratio), Gray(ratio), Gray(ratio));
			if (ratio1 <= 1)
					plane1->Load->DrawLoadPoint(ix, jy, rx, ry, color1, pDC);
		} // jj

		for (kk = 0; kk < Nzz; kk++) {
			z = (kk + 0.5) * hz;
			ix = xmin + (int)((x) * ScaleX);
				//ix = origX + (int)((x - left) * pLV->ScaleX);
			kz = zmin - (int)((z) * ScaleZ);
				
			if (MaxVal2 > 1.e-10) 	
					ratio2 = plane2->Load->GetVal(x, z) / MaxVal2;
			else	ratio2 = 1;
						
			color2 = GetColor10(ratio2);
				//		color = RGB(Red(ratio), Green(ratio), Blue(ratio));
				//		color = RGB(Gray(ratio), Gray(ratio), Gray(ratio));
			if (ratio2 <= 1)
					plane2->Load->DrawLoadPoint(ix, kz, rx, rz, color2, pDC);
		} // kk

				if (::PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) {
					::TranslateMessage(&message);
					::DispatchMessage(&message);
				}
				if (STOP) break; //return; // 
	} // ii

	ReleaseDC(pDC);
	*/
}

void CMainView:: ShowMAMuG() // IonSources
{
	CDC* pDC = GetDC();
	CBTRDoc* pDoc = (CBTRDoc*)GetDocument();
	CPen * pOldPen = pDC->SelectObject(&ThinPen);
	double x0, y0, z0;
	C3Point P0, P;
	double x,y,z;
	int left, top, right, bot;
	int Nh = (int)(pDoc->NofChannelsHor);
	int Nv = (int)(pDoc->NofChannelsVert);

	//	Nh = pDoc->SegmCentreHor.GetSize();
	//	Nv = pDoc->SegmCentreVert.GetSize();


	for (int  k = 0; k< Nh; k++) {
		x0 = 0.;
		y0 = pDoc->SegmCentreHor[k]; 

		left = OrigX + (int)(x0*ScaleX) - 3;	right = OrigX + (int)(x0*ScaleX) +1;
		top = OrigY - (int)((y0 + pDoc->SegmSizeHor*0.5)*ScaleY) - 3;
		bot = OrigY - (int)((y0 - pDoc->SegmSizeHor*0.5)*ScaleY) + 3;
		if (abs(top - bot) < 10) { top -=3; bot +=3; }
		pDoc->RectIS[k].SetRect(left, top, right, bot);
		//	pDC->SelectObject(&YellowPen);
		pDC->Rectangle(pDoc->RectIS[k]);
		double stepH = pDoc->AppertStepHor;
		double stepV = pDoc->AppertStepVert;
		int NH = (int)pDoc->NofBeamletsHor;
		int NV = (int)pDoc->NofBeamletsVert;


		for (int n = 0; n < Nv ; n++) {
			/// Emitters rect
			z0 = pDoc->SegmCentreVert[n]; 
			left = OrigX + (int)(x0*ScaleX) - 3;	
			right = OrigX + (int)(x0*ScaleX) +1;
			top = OrigZ - (int)((z0 + pDoc->SegmSizeVert*0.5)*ScaleZ) - 3;
			bot = OrigZ - (int)((z0 - pDoc->SegmSizeVert*0.5)*ScaleZ) + 3;
			if (abs(top - bot) < 10) { top -=3; bot +=3; }
			pDoc->RectISrow[n].SetRect(left, top, right, bot);
			pDC->Rectangle(pDoc->RectISrow[n]);

			// Emitters Axes
			if (pDoc->ActiveCh[k] && pDoc->ActiveRow[n]) pDC->SelectObject(&BlackPen);
			else pDC->SelectObject(&ThinPen);

			double BeamHorAngle, BeamVertAngle;
			double VInclin = 0;
			if (!pDoc->OptFree) VInclin = pDoc->VertInclin;

			BeamHorAngle = - atan(pDoc->SegmCentreHor[k] / pDoc->BeamAimHor);
			BeamVertAngle = - atan(pDoc->SegmCentreVert[n] / pDoc->BeamAimVert);
			//	P0.X = 0.; P0.Y = 0; P0.Z = 0; 	//	P = pDoc->CentralCS(P0, k, n);
			P.X = 0; P.Y = pDoc->SegmCentreHor[k]; P.Z = 0;
			pDC->MoveTo(OrigX + (int)(P.X*ScaleX), OrigY -(int)(P.Y*ScaleY)); 
			P.X = pDoc->PlasmaXmax; //*pDoc->AreaLong;//BeamAimHor; 
			P.Y = P.Y + P.X * tan(BeamHorAngle);//P.Y = 0;
			pDC->LineTo(OrigX + (int)(P.X*ScaleX), OrigY -(int)(P.Y*ScaleY)); 

			//	P0.X = 0.; P0.Y = 0; P0.Z = 0; 	//	P = pDoc->CentralCS(P0, k, n);
			P.X = 0; P.Y = 0; P.Z = pDoc->SegmCentreVert[n]; 
			pDC->MoveTo(OrigX + (int)(P.X*ScaleX), OrigZ -(int)(P.Z*ScaleZ)); 
			P.X = pDoc->AreaLong; //pDoc->PlasmaXmax;// //BeamAimVert; 
			P.Z = P.Z + P.X * tan(BeamVertAngle + VInclin + pDoc->BeamVertTilt);
			//	if (pDoc->Active[k])
			pDC->LineTo(OrigX + (int)(P.X*ScaleX), OrigZ -(int)(P.Z*ScaleZ)); 
			//----------
			if  (!pDoc->ActiveCh[k]) continue;
			if  (!pDoc->ActiveRow[n]) continue;

			if (pDoc->ActiveCh[k]) pDC->SelectObject(&MamugPen); //(&RosePen);
			pDC->Rectangle(pDoc->RectIS[k]);
			pDC->Rectangle(pDoc->RectISrow[n]);

			for (int i = 0; i < NH; i++) { //NofBeamletsHor
				int ii = k * NH + i; 
				if  (!pDoc->ActiveCh[k]) continue;
				for (int j = 0; j < NV; j++) { //NofBeamletsVert
					int jj = n * NV + j; 
					if  (!pDoc->ActiveRow[n]) continue;
					x = 0;
					y = stepH * (- (NH-1) *0.5 + i);	// in local (segment) CS 
					z = stepV * (- (NV-1) *0.5 + j);	// in local (segment) CS 

					P.X = 0.; 
					P.Y = pDoc->BeamletPosHor[ii];
					pDC->MoveTo(OrigX + (int)(P.X*ScaleX), OrigY -(int)(P.Y*ScaleY)); 

					P.X = pDoc->AreaLong; //pDoc->PlasmaXmax;// //BeamAimHor; //	P.X = x0 + *pDoc->AreaLong;
					P.Y = P.Y + P.X * tan(pDoc->BeamletAngleHor[ii]);
					pDC->LineTo(OrigX + (int)(P.X*ScaleX), OrigY -(int)(P.Y*ScaleY)); 

					P.X = 0.;
					P.Z = pDoc->BeamletPosVert[jj];
					pDC->MoveTo(OrigX + (int)(P.X*ScaleX), OrigZ -(int)(P.Z*ScaleZ)); 

					P.X = pDoc->AreaLong; //pDoc->PlasmaXmax;//*pDoc->AreaLong;//BeamAimVert; //P.X = x0 + *pDoc->AreaLong;
					P.Z = P.Z + P.X * tan(pDoc->BeamletAngleVert[jj]);
					pDC->LineTo(OrigX + (int)(P.X*ScaleX), OrigZ -(int)(P.Z*ScaleZ)); 
				} // j
			} // i 


		} //n vert
	} // k horiz
	pDC->SelectObject(pOldPen);
	//	pDC->SelectObject(&ThinPen);
	ReleaseDC(pDC);

}

void CMainView:: ShowSINGAP() // IonSources
{
	CDC* pDC = GetDC();
	CBTRDoc* pDoc = (CBTRDoc*)GetDocument();
	CPen * pOldPen = pDC->SelectObject(&SingapPen); //(&MagentaPen);
	double x0, y0, z0;
	double x,y,z;
	int NofBeamlets = pDoc->BeamEntry_Array.GetSize();
	BEAMLET_ENTRY be;
	C3Point P0, P;
	int left, top, right, bot;

	for (int i = 0; i < NofBeamlets; i++)
	{
		be = pDoc->BeamEntry_Array[i];
		if (be.Active == FALSE) continue;
		x0 = 0;
		y0 = be.PosY;	// in NBI CS 
		z0 = be.PosZ;	// in NBI CS 
		x = x0 + pDoc->AreaLong;
		y = y0 + x * tan(be.AlfY); 
		z = z0 + x * tan(be.AlfZ); 
		pDC->MoveTo(OrigX + (int)(x0*ScaleX), OrigY -(int)(y0*ScaleY)); 
		pDC->LineTo(OrigX + (int)(x*ScaleX), OrigY -(int)(y*ScaleY)); 

		pDC->MoveTo(OrigX + (int)(x0*ScaleX), OrigZ -(int)(z0*ScaleZ)); 
		pDC->LineTo(OrigX + (int)(x*ScaleX), OrigZ -(int)(z*ScaleZ)); 
	}

	//	pDC->SelectObject(&BlackPen);

	/*	for (int  k = (int)* pDoc->NofChannelsHor-1; k>=0; k--) {
	if (pDoc->ActiveCh[k]) pDC->SelectObject(&BlackPen);
	else pDC->SelectObject(&ThinPen);
	x0 = 0.;
	y0 = pDoc->SegmCentreHor[k]; 

	left = OrigX + (int)(x0*ScaleX) - 3;	right = OrigX + (int)(x0*ScaleX) +1;
	top = OrigY - (int)((y0 + pDoc->SegmSizeHor*0.5)*ScaleY);
	bot = OrigY - (int)((y0 - pDoc->SegmSizeHor*0.5)*ScaleY);
	pDoc->RectIS[k].SetRect(left, top, right, bot);
	//	pDC->Rectangle(pDoc->RectIS[k]); // no sources for SINGAP

	for (int n = 0; n < (int)* pDoc->NofChannelsVert; n++) {
	if (pDoc->ActiveRow[n]) pDC->SelectObject(&BlackPen);
	else pDC->SelectObject(&ThinPen);
	x0 = 0.;
	z0 = pDoc->SegmCentreVert[n]; 

	left = OrigX + (int)(x0*ScaleX) - 3;	right = OrigX + (int)(x0*ScaleX) +1;
	top = OrigZ - (int)((z0 + pDoc->SegmSizeVert*0.5)*ScaleZ);
	bot = OrigZ - (int)((z0 - pDoc->SegmSizeVert*0.5)*ScaleZ);
	pDoc->RectISrow[n].SetRect(left, top, right, bot);
	//  pDC->Rectangle(pDoc->RectISrow[n]); // no sources for SINGAP

	// Emitters Axes
	if (pDoc->ActiveCh[k] && pDoc->ActiveRow[n]) pDC->SelectObject(&BlackPen);
	else pDC->SelectObject(&ThinPen);

	P0.X = 0.; P0.Y = 0; P0.Z = 0; 		
	P = pDoc->CentralCS(P0, k, n);
	pDC->MoveTo(OrigX + (int)(P.X*ScaleX), OrigY -(int)(P.Y*ScaleY)); 
	P.X = *pDoc->BeamAimHor; 
	P.Y = 0;
	pDC->LineTo(OrigX + (int)(P.X*ScaleX), OrigY -(int)(P.Y*ScaleY)); 

	P0.X = 0.; P0.Y = 0; P0.Z = 0; 		
	P = pDoc->CentralCS(P0, k, n);
	pDC->MoveTo(OrigX + (int)(P.X*ScaleX), OrigZ -(int)(P.Z*ScaleZ)); 
	P.X =  *pDoc->BeamAimVert; 
	P.Z = P.X * tan(*pDoc->VertInclin + *pDoc->BeamVertTilt);
	//	if (pDoc->Active[k])
	pDC->LineTo(OrigX + (int)(P.X*ScaleX), OrigZ -(int)(P.Z*ScaleZ)); 

	} //n vert
	} // k horiz
	*/
	pDC->SelectObject(pOldPen);
	ReleaseDC(pDC);
}

void CMainView:: ShowTraced() // not called
{
	CDC* pDC = GetDC();
	CBTRDoc* pDoc = (CBTRDoc*)GetDocument();
	//if (!pDoc->OptDrawPart) return;
	int x, y, z;
	std::vector<minATTR> & arr = pDoc->m_AttrVector[MaxThreadNumber-1];
	for (int k = 0; k < pDoc->ThreadNumber; k++)
	{
		arr = pDoc->m_AttrVector[k];
		for (int i = 0; i < (int) arr.size(); i++) {
			minATTR &tattr = arr[i]; 
			//COLORREF color = tattr.Color;
			//if (tattr.Nfall > 0) color = RGB(255,0,0);
			x = OrigX + (int)(tattr.Xmm * 0.001 * ScaleX);
			y = OrigY - (int)(tattr.Ymm * 0.001 * ScaleY);
			//z = OrigZ - (int)(tattr.Z * ScaleZ);
			if (pDoc->OptDrawPart && tattr.Nfall > 0) 
			{
				//	pDC->SetPixel(x,y, color); 
				//	pDC->SetPixel(x,z, color);
			}
		}
	}
	ReleaseDC(pDC);

}
void CMainView:: ShowParticlePos(C3Point Pos, COLORREF color)
{
	CDC* pDC = GetDC();
	//CPen * pen = &GreenPen;
	//if (charge > 0) pen = &RosePen;// thick 1
	//CPen * pOldPen = pDC->SelectObject(pen);
	int x = OrigX + (int)(Pos.X * ScaleX);
	int y = OrigY - (int)(Pos.Y * ScaleY);
	int z = OrigZ - (int)(Pos.Z * ScaleZ);
	pDC->SetPixel(x, y, color);
	pDC->SetPixel(x, z, color);
	//pDC->Ellipse(x - 1, y - 1, x + 1, y + 1);
	//pDC->Ellipse(x - 1, z - 1, x + 1, z + 1);
	//pDC->SelectObject(pOldPen);
	ReleaseDC(pDC);

}
/*  CPen BlackPen;	CPen ThinPen; 	CPen ThickPen;
	CPen DotPen;	CPen MarkPen;	CPen RedPen;
	CPen RosePen;	CPen MamugPen;	CPen SingapPen;
	CPen YellowPen;	CPen GreenPen;	CPen BluePen;	CPen AtomPen; */

void CMainView:: DrawPartTrack(CArray <C3Point> &Pos, int charge, COLORREF color)
{	
	int n = Pos.GetSize();
	if (n < 1) return;
	if (n < 2) {
		ShowParticlePos(Pos[0], color);
		return;
	}
	CDC* pDC = GetDC();
	CPen * pen = &BluePen;//&AtomPen;
	if (charge < 0) pen = &GreenPen;
	if (charge > 0) pen = &RosePen;// thick 1
	CPen * pOldPen = pDC->SelectObject(pen); 
	int x0 = OrigX + (int)(Pos[0].X * ScaleX);
	int y0 = OrigY - (int)(Pos[0].Y * ScaleY);
	int z0 = OrigZ - (int)(Pos[0].Z * ScaleZ);
	for (int i = 1; i < n; i++) {
		int x = OrigX + (int)(Pos[i].X * ScaleX);
		int y = OrigY - (int)(Pos[i].Y * ScaleY);
		int z = OrigZ - (int)(Pos[i].Z * ScaleZ);
		pDC->MoveTo(x0, y0);
		pDC->LineTo(x, y);
		pDC->MoveTo(x0, z0);
		pDC->LineTo(x, z);
		x0 = x;
		y0 = y;
		z0 = z;
	}
	pDC->SelectObject(pOldPen);
	ReleaseDC(pDC);

}
void CMainView:: ShowComments()
{
	CDC* pDC = GetDC();
	CBTRDoc* pDoc = (CBTRDoc*)GetDocument();

	/*CFont specfont; 
	if (!specfont.CreateFont(-12, 0, 0, 0, 600, FALSE, FALSE, 0,
	ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
	DEFAULT_PITCH | FF_MODERN, "Courier New")) return;*/
	CFont * pOldFont = (CFont *)(pDC->SelectObject(&midfont));

	//	pDC->SetBkMode(1);
	CPen pen;
	pen.CreatePen(PS_DOT, 1, RGB(200,0,0));
	CPen * pOldPen = pDC->SelectObject(&pen);
	//CPen* pOldPen =  pDC->SelectObject(&RedPen);
	int x, y, z, w, h;
	CString S;

	h = 30;//OrigZ - (int)(Zmin * ScaleZ) +15;

	pDC->SetTextColor(RGB(0,0,250));
	x = OrigX;
	y = OrigY  - (int)(Ymin * ScaleY);
	z = OrigZ  - (int)(pDoc->AreaVertMin * ScaleZ) + 5;
	//if (MODE_FIT) z = z+25;
	pDC->TextOut(x+1, y+1, "Dia");

	x = OrigX + (int)(pDoc->NeutrInX * ScaleX);
	pDC->MoveTo(x, y); 	pDC->LineTo(x, y-h); pDC->TextOut(x-5, y+1, "1");
	x = OrigX + (int)(pDoc->NeutrOutX * ScaleX);
	pDC->MoveTo(x, y); 	pDC->LineTo(x, y-h); pDC->TextOut(x-5, y+1, "2");
	S.Format("Neutralizer");
	//	if (pDC->GetOutputTextExtent(S, 11).cx == NULL) w = 20;
	if(pDC->m_hAttribDC != NULL) w = pDC->GetTextExtent(S).cx;
	else w = 50;
	x = OrigX + (int)((pDoc->NeutrInX + pDoc->NeutrOutX) * 0.5 * ScaleX);
	pDC->SetTextColor(RGB(250,0,0));
	pDC->TextOut(x - w/2, z+10, S);

	pDC->SetTextColor(RGB(0,0,250));
	x = OrigX + (int)(pDoc->RIDInX * ScaleX);
	pDC->MoveTo(x, y); 	pDC->LineTo(x, y-h); pDC->TextOut(x-5, y+1, "3");
	x = OrigX + (int)(pDoc->RIDOutX * ScaleX);
	pDC->MoveTo(x, y); 	pDC->LineTo(x, y-h); pDC->TextOut(x-5, y+1, "4");
	S =  "RID";
	if(pDC->m_hAttribDC != NULL) w = pDC->GetTextExtent(S).cx;
	else w = 15;
	x = OrigX + (int)((pDoc->RIDInX + pDoc->RIDOutX) * 0.5 * ScaleX);
	pDC->SetTextColor(RGB(250,0,0));
	pDC->TextOut(x - w/2, z, S);

	x = OrigX + (int)(pDoc->CalInX * ScaleX);
	//	pDC->MoveTo(x, y); 	pDC->LineTo(x, y-h); pDC->TextOut(x-5, y+1, "|");
	x = OrigX + (int)(pDoc->CalOutX * ScaleX);
	//	pDC->MoveTo(x, y); 	pDC->LineTo(x, y-h); pDC->TextOut(x-5, y+1, "|");
	S =  "Calorimeter";
	if(pDC->m_hAttribDC != NULL) w = pDC->GetTextExtent(S).cx;
	else w = 50;
	x = OrigX + (int)((pDoc->CalInX + pDoc->CalOutX) * 0.5 * ScaleX);
	pDC->TextOut(x - w/2, z+10, S);

	pDC->SetTextColor(RGB(0,0,250));
	x = OrigX + (int)(pDoc->PreDuctX * ScaleX);
	pDC->MoveTo(x, y); 	pDC->LineTo(x, y-h);pDC->TextOut(x-5, y+1, "5");
	S =  " Duct"; //"5";//"Scr";
	if(pDC->m_hAttribDC != NULL) w = pDC->GetTextExtent(S).cx;
	else w = 15;
	pDC->SetTextColor(RGB(250,0,0));
	pDC->TextOut(x, z, S);

	pDC->SetTextColor(RGB(0,0,250));
	x = OrigX + (int)(pDoc->DDLinerInX * ScaleX);
	pDC->MoveTo(x, y); 	pDC->LineTo(x, y-h); pDC->TextOut(x-5, y+1, "6");
	S =  "6";//"AV";
	/*	if(pDC->m_hAttribDC != NULL) w = pDC->GetTextExtent(S).cx;
	else w = 30;
	pDC->SetTextColor(RGB(250,0,0));
	pDC->TextOut(x, z+10, S);*/

	pDC->SetTextColor(RGB(0,0,250));
	x = OrigX + (int)(pDoc->DDLinerOutX * ScaleX);
	pDC->MoveTo(x, y); 	pDC->LineTo(x, y-h); pDC->TextOut(x-5, y+1, "7");
	S =  "7";//"Duct0";
	/*	if(pDC->m_hAttribDC != NULL) w = pDC->GetTextExtent(S).cx;
	else w = 30;
	pDC->SetTextColor(RGB(250,0,0));
	pDC->TextOut(x, z, S);*/

	pDC->SetTextColor(RGB(0,0,250));
	x = OrigX + (int)(pDoc->Duct1X * ScaleX);
	pDC->MoveTo(x, y); 	pDC->LineTo(x, y-h); pDC->TextOut(x-5, y+1, "8");
	S =  "8";//"Duct1";
	/*	if(pDC->m_hAttribDC != NULL) w = pDC->GetTextExtent(S).cx;
	else w = 20;
	pDC->SetTextColor(RGB(250,0,0));
	pDC->TextOut(x, z+10, S);*/

	pDC->SetTextColor(RGB(0,0,250));
	x = OrigX + (int)(pDoc->Duct2X * ScaleX);
	pDC->MoveTo(x, y); 	pDC->LineTo(x, y-h); pDC->TextOut(x-5, y+1, "9");
	S =  "9";//"Duct2";
	/*	if(pDC->m_hAttribDC != NULL) w = pDC->GetTextExtent(S).cx;
	else w = 20;
	pDC->SetTextColor(RGB(250,0,0));
	pDC->TextOut(x, z, S);*/

	pDC->SetTextColor(RGB(0,0,250));
	x = OrigX + (int)(pDoc->Duct3X * ScaleX);
	pDC->MoveTo(x, y); 	pDC->LineTo(x, y-h); pDC->TextOut(x-5, y+1, "10");
	S =  "10";//"Duct3";
	/*	if(pDC->m_hAttribDC != NULL) w = pDC->GetTextExtent(S).cx;
	else w = 20;
	pDC->SetTextColor(RGB(250,0,0));
	pDC->TextOut(x, z+10, S);*/

	pDC->SetTextColor(RGB(0,0,250));
	x = OrigX + (int)(pDoc->Duct4X * ScaleX);
	pDC->MoveTo(x, y); 	pDC->LineTo(x, y-h);  pDC->TextOut(x-5, y+1, "11");
	S =  "11";//"Duct4";
	/*	if(pDC->m_hAttribDC != NULL) w = pDC->GetTextExtent(S).cx;
	else w = 20;
	pDC->SetTextColor(RGB(250,0,0));
	pDC->TextOut(x, z, S);*/

	pDC->SetTextColor(RGB(0,0,250));
	x = OrigX + (int)(pDoc->Duct5X * ScaleX);
	pDC->MoveTo(x, y); 	pDC->LineTo(x, y-h);  pDC->TextOut(x-5, y+1, "12");
	S =  "12";//"Duct5";
	/*	if(pDC->m_hAttribDC != NULL) w = pDC->GetTextExtent(S).cx;
	else w = 20;
	pDC->SetTextColor(RGB(250,0,0));
	pDC->TextOut(x, z+10, S);*/

	pDC->SetTextColor(RGB(0,0,250));
	x = OrigX + (int)(pDoc->Duct6X * ScaleX);
	pDC->MoveTo(x, y); 	pDC->LineTo(x, y-h); pDC->TextOut(x-5, y+1, "13");
	S =  "13";//"Duct6";
	/*	if(pDC->m_hAttribDC != NULL) w = pDC->GetTextExtent(S).cx;
	else w = 20;
	pDC->SetTextColor(RGB(250,0,0));
	pDC->TextOut(x, z, S);*/

	pDC->SetTextColor(RGB(0,0,250));
	x = OrigX + (int)(pDoc->Duct7X * ScaleX);
	pDC->MoveTo(x, y); 	pDC->LineTo(x, y-h);  
	pDC->TextOut(x-5, y+1, "14");
	S =  "14";//"Duct7";
	/*	if(pDC->m_hAttribDC != NULL) w = pDC->GetTextExtent(S).cx;
	else w = 20;
	pDC->SetTextColor(RGB(250,0,0));
	pDC->TextOut(x, z+10, S); */

	pDC->SetTextColor(RGB(0,0,250));
	x = OrigX + (int)(pDoc->Duct8X * ScaleX);
	pDC->MoveTo(x, y); 	pDC->LineTo(x, y-h);  
	pDC->TextOut(x-5, y+1, "15");

	// targets in tor
	if (pDoc->OptBeamInPlasma) {

		pDC->SetTextColor(RGB(0,100,0));

		x = OrigX + (int)(pDoc->MovX * ScaleX);
		pDC->MoveTo(x, y); 	pDC->LineTo(x, y-h);  
		pDC->TextOut(x-5, y+1, "A");

		x = OrigX + (int)(pDoc->Mov2X * ScaleX);
		pDC->MoveTo(x, y); 	pDC->LineTo(x, y-h);  
		pDC->TextOut(x-5, y+1, "B");

		x = OrigX + (int)(pDoc->Mov3X * ScaleX);
		pDC->MoveTo(x, y); 	pDC->LineTo(x, y-h);  
		pDC->TextOut(x-5, y+1, "C");

		x = OrigX + (int)(pDoc->Mov4X * ScaleX);
		pDC->MoveTo(x, y); 	pDC->LineTo(x, y-h);  
		pDC->TextOut(x-5, y+1, "D");

		S = "PLASMA";
		x = OrigX + (int)(pDoc->TorCentre.X * ScaleX);
		pDC->SetTextColor(RGB(250,0,0));
		pDC->TextOut(x - 30, z+10, S);
	} // Beaminplasma

	pDC->SetTextColor(RGB(0,0,0));
	pDC->SelectObject(pOldFont);
	pDC->SelectObject(pOldPen);
	ReleaseDC(pDC);
}

void CMainView:: ShowProgress(int percent)
{
	CClientDC dc(this);
	/*	CFont specfont; 
	if (!specfont.CreateFont(-18, 0, 0, 0, 800, FALSE, FALSE, 0,
	ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
	DEFAULT_PITCH | FF_MODERN, "Courier New")) return;
	//	CFont * pOldFont = (CFont *)(pDC->SelectObject(&specfont));
	CFont * pOldFont = dc.SelectObject(&specfont);*/
	//if (percent < 100) return;
	int x = OrigX + (int)(Xmax*ScaleX)-200;
	int y = OrigZ - (int)(Zmax*ScaleY);
	dc.SetBkColor(RGB(255, 0, 255));
	dc.SetTextColor(RGB(255, 255, 0));
	if (percent ==100)	dc.TextOut(x, y, "Finished");  
	if (percent == 50) {
		dc.TextOut(x, y, "Interrupted");  
		//	dc.TextOut(x, y+20, "Load not saved");
	}
	dc.SetBkColor(RGB(255, 255, 255));
	dc.SetTextColor(RGB(0, 0, 0));
	//	dc.SelectObject(pOldFont);
}


/////////////////////////////////////////////////////////////////////////////
// CMainView diagnostics

#ifdef _DEBUG
void CMainView::AssertValid() const
{
	CScrollView::AssertValid();
}

void CMainView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}

/*CBTRDoc* CMainView::GetDocument() // non-debug version is inline
{
ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CBTRDoc)));
return (CBTRDoc*)m_pDocument;
}*/
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainView message handlers

void CMainView::OnViewFitonoff() 
{
	MODE_FIT = !MODE_FIT;
	if (MODE_FIT) {
		CRect rect;
		CSize size;
		GetClientRect(rect);
		size = rect.Size();
		BoxRect.SetRect(0,0, 0,0); //sizeTotal.cx,sizeTotal.cy);
		SetScrollSizes(MM_TEXT, size);
	}
	//	ZOOM_FINISHED = TRUE;
	//	MODE_ZOOM = FALSE;
	ZOOM_IN = FALSE;
	ZoomCoeffY = 1; ZoomCoeffZ = 1; 
	InvalidateRect(NULL, TRUE);
}

void CMainView::OnUpdateViewFitonoff(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(MODE_FIT);
}
void CMainView::OnViewSize() 
{
	CSize size = GetTotalSize();
	// TODO: calculate the total size of this view
	//sizeTotal.cx = 5000; sizeTotal.cy = 2000;
	CViewSizeDlg dlg;
	dlg.m_Hor = size.cx;
	dlg.m_Vert = size.cy;
	if (dlg.DoModal() == IDOK) {
		size.cx = dlg.m_Hor;
		size.cy = dlg.m_Vert;
		SetScrollSizes(MM_TEXT, size);
		InvalidateRect(NULL, TRUE);
	}
}

//// Context Menu Operations ----------------------------------------------------------
void CMainView::OnContextMenu(CWnd* /* pWnd */, CPoint point) 
{
	//	LastPoint.x = point.x; LastPoint.y = point.y;
	CClientDC dc(this);
	OnPrepareDC(&dc);
	dc.DPtoLP(&LastPoint);
	ZoomPoint.X = (LastPoint.x - OrigX) / ScaleX;
	if (PtInRect(RectPlan, LastPoint)) { ZoomPoint.Y = (OrigY - LastPoint.y) / ScaleY; ZoomPoint.Z = 1000; }
	if (PtInRect(RectSide, LastPoint)) { ZoomPoint.Y = 1000;  ZoomPoint.Z = (OrigZ - LastPoint.y) / ScaleZ; }

	MenuPop.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTALIGN  |  TPM_RIGHTBUTTON | DT_RIGHT, 
		point.x, point.y, this);

}

void CMainView::OnPlateZoomin() 
{
	ZOOM_IN = TRUE;
	if (ZoomPoint.Z > 999)  ZoomCoeffY *= 1.5; 
	if (ZoomPoint.Y > 999)  ZoomCoeffZ *= 1.5; 
	ZoomCoeffX *= 1.5; 
	//SHOW_BEAM = FALSE;
	InvalidateRect(NULL, TRUE);
	//	InvalidateRect(RectPlan, TRUE);
	//	InvalidateRect(RectSide, TRUE); 
	//	ZOOM_IN = FALSE;

}

void CMainView::OnPlateZoomout() 
{
	ZOOM_IN = FALSE;
	ZoomCoeffY = 1;
	ZoomCoeffZ = 1;
	ZoomCoeffX = 1;
	SHOW_BEAM = TRUE;
	InvalidateRect(NULL, TRUE);
	//	InvalidateRect(RectPlan, TRUE);
	//	InvalidateRect(RectSide, TRUE);
}

void CMainView::OnPlateLoad() 
{
	CBTRDoc* pDoc = (CBTRDoc*)GetDocument();
	CPlate * plate = pDoc->pMarkedPlate;
	pDoc->OnPlotLoadmap();
	return;

	///////////////////// NOT CALLED ////////////////
	if (pDoc->OptCombiPlot == -1) { // no plate selected
		//|| !(pDoc->pMarkedPlate->Loaded)) {
		Message_NotSelected();
		pDoc->ShowStatus(); return;
	}
	//CPlate * plate = pDoc->pMarkedPlate;
	if (pDoc->OptCombiPlot == 0) { //is calculated
		pDoc->OnPlotLoadmap();
		return;
	}
	// OptCombiPlot > 0 -> selected, maybe not calculated yet
	if (pDoc->OptParallel) { //parallel
		if (plate->Loaded == FALSE) { // not calculated yet
			//InvalidateRect(NULL, TRUE);
			pDoc->SetNullLoad(pDoc->pMarkedPlate);//plate->ApplyLoad(TRUE, 0, 0); // optimize grid 
			pDoc->SetLoadArray(pDoc->pMarkedPlate, TRUE);
		}

		plate->Selected = TRUE;
		pDoc->ShowProfiles = TRUE;
		pDoc->OnPlateClearSelect();

	} // parallel

	// non-parallel
	else if(!(pDoc->pMarkedPlate->Loaded)) {
		Message_NotSelected();
		pDoc->ShowStatus(); return;
	}

//	pDoc->CreateBaseSingle();// set new pMarkedPlate!!!
	pDoc->OnShow();
	pDoc->OnPlotLoadmap();

}



void CMainView::OnPlateContours() 
{
	CBTRDoc* pDoc = (CBTRDoc*)GetDocument();
	/*if (pDoc->pMarkedPlate == NULL || !(pDoc->pMarkedPlate->Loaded)) {
	Message_NotSelected();
	pDoc->ShowStatus(); return;
	}*/

	if (pDoc->OptCombiPlot == -1) { // not selected
		//|| !(pDoc->pMarkedPlate->Loaded)) {
		//Message_NotSelected();
		pDoc->ShowStatus(); return;
	}
	CPlate * plate = pDoc->pMarkedPlate;
	if (pDoc->OptCombiPlot == 0) { // base already exists (combi calculated)
		pDoc->OnPlotContours();
		return;
	}

	if (pDoc->OptParallel) { //parallel
		if (plate->Loaded == FALSE) { // not calculated yet
			//InvalidateRect(NULL, TRUE);
			pDoc->SetNullLoad(plate);//plate->ApplyLoad(TRUE, 0, 0); // optimize grid 
			pDoc->SetLoadArray(plate, TRUE);
		}

		plate->Selected = TRUE;
		//plate->Load->Clear();
		pDoc->ShowProfiles = TRUE;
		//pDoc->P_CalculateLoad(plate);
		//if (plate->Comment.Find("First")>=0) 
		pDoc->OnPlateClearSelect();
		//pDoc->CreateBaseSingle();// set new pMarkedPlate

		//pDoc->OnPlotCombi();//
		//pDoc->P_CalculateCombi(1);
		//plate->ShowLoadState();		

	} // parallel

	// non-parallel
	else if(!(pDoc->pMarkedPlate->Loaded)) {
		Message_NotSelected();
		pDoc->ShowStatus(); return;
	}

//	pDoc->CreateBaseSingle();// set new pMarkedPlate
	pDoc->OnShow();
	pDoc->OnPlotContours();

	//pDoc->OnPlotMaxprofiles();

}

void CMainView::OnPlate3dplot() 
{
	CBTRDoc* pDoc = (CBTRDoc*)GetDocument();
	/*if (pDoc->pMarkedPlate == NULL || !(pDoc->pMarkedPlate->Loaded)) {
	Message_NotSelected();
	pDoc->ShowStatus(); return;
	}*/
	pDoc->OnPlot3dload() ;
	/*CPlate * plate = pDoc->pBeamHorPlane; //pMarkedPlate;
	pDoc->ShowProfiles = TRUE;
	pDoc->OnPlotMaxprofiles();
	pDoc->OnPlotLoadmap();*/
	return;

///////////////////NOT CALLED//////////////////////////////////////
	if (pDoc->OptCombiPlot == -1) { //|| !(pDoc->pMarkedPlate->Loaded)) {
		//Message_NotSelected();
		pDoc->ShowStatus(); return;
	}
	CPlate * plate = pDoc->pMarkedPlate;
	if (pDoc->OptCombiPlot == 0) { // base already exists
		pDoc->OnPlot3dload();
		return;
	}

	if (pDoc->OptParallel) { //parallel
		if (plate->Loaded == FALSE) { // not calculated yet
			//InvalidateRect(NULL, TRUE);
			pDoc->SetNullLoad(plate);//plate->ApplyLoad(TRUE, 0, 0); // optimize grid 
			pDoc->SetLoadArray(plate, TRUE);
		}

		plate->Selected = TRUE;
		//plate->Load->Clear();
		pDoc->ShowProfiles = TRUE;
		//pDoc->P_CalculateLoad(plate);
		//if (plate->Comment.Find("First")>=0) 
		pDoc->OnPlateClearSelect();
		//pDoc->CreateBaseSingle();// set new pMarkedPlate

		//pDoc->OnPlotCombi();//
		//pDoc->P_CalculateCombi(1);
		//plate->ShowLoadState();		

	} // parallel

	// non-parallel
	else if(!(pDoc->pMarkedPlate->Loaded)) {
		Message_NotSelected();
		pDoc->ShowStatus(); return;
	}
//	pDoc->CreateBaseSingle();// set new pMarkedPlate
	pDoc->OnPlot3dload();
	/*if (!(pDoc->pMarkedPlate->Loaded)) return;
	pDoc->pMarkedPlate->Load->SetSumMax(); // max profiles
	pDoc->ShowProfiles = FALSE;
	pDoc->UpdateAllViews(NULL, NULL);	
	//	pSetView->InvalidateRect(NULL, TRUE);	*/
}

void CMainView::OnPlateMaxprofiles() 
{
	CBTRDoc* pDoc = (CBTRDoc*)GetDocument();
	//if (pDoc->pMarkedPlate == NULL) return;
	pDoc->OnPlotMaxprofiles();
	return;

/////////// NOT CALLED /////////////////////
	if (pDoc->OptCombiPlot == -1) { // not selected
		//|| !(pDoc->pMarkedPlate->Loaded)) {
		//Message_NotSelected();
		pDoc->ShowStatus(); return;
	}
	CPlate * plate = pDoc->pMarkedPlate;
	if (pDoc->OptCombiPlot == 0) { // base plate selected (one)
		pDoc->OnPlotMaxprofiles();
		//pDoc->pMarkedPlate->ShowLoadState();
		return;
	}

	/*	if (pDoc->pMarkedPlate == NULL || !(pDoc->pMarkedPlate->Loaded)) {
	Message_NotSelected();
	pDoc->ShowStatus(); return;
	}*/

	/*	if (pDoc->OptCombiPlot == -1) { //|| !(pDoc->pMarkedPlate->Loaded)) {
	//Message_NotSelected();
	pDoc->ShowStatus(); return;
	}*/
	//CPlate * plate = pDoc->pMarkedPlate;

	if (pDoc->OptParallel) { //parallel
		if (plate->Loaded == FALSE) { // not calculated yet
			//InvalidateRect(NULL, TRUE);
			pDoc->SetNullLoad(plate);//plate->ApplyLoad(TRUE, 0, 0); // optimize grid 
			pDoc->SetLoadArray(plate, TRUE);
		}

		//pDoc->P_CalculateLoad(plate);
		//if (plate->Comment.Find("First")>=0)
		pDoc->OnPlateClearSelect();
		//pDoc->CreateBaseSingle();// set new pMarkedPlate

		//pDoc->OnPlotCombi();//
		//pDoc->P_CalculateCombi(1);
		//plate->Selected = TRUE;
		//pDoc->ShowProfiles = TRUE;
		//plate->ShowLoadState();


	} // parallel

	// non-parallel
	else if(!(pDoc->pMarkedPlate->Loaded)) {
		Message_NotSelected();
		pDoc->ShowStatus(); return;
	}

	//pDoc->CreateBaseSingle();// set new pMarkedPlate
	pDoc->ShowProfiles = TRUE;
	
	//pDoc->pMarkedPlate->ShowLoadState();	//- removed, need check 

	
}

void CMainView::OnPlateSmooth() 
{
	CBTRDoc* pDoc = (CBTRDoc*)GetDocument();
	CResSmooth dlg;
	if (pDoc->pMarkedPlate == NULL) return;
	//if (pDoc->OptCombiPlot == -1 || !(pDoc->pMarkedPlate->Loaded)) {
	if (!(pDoc->pMarkedPlate->Loaded)) {
		Message_NotSelected();
		pDoc->ShowStatus(); return;
	}
	CPlate * plate = pDoc->pMarkedPlate;
	dlg.m_Degree = plate->SmoothDegree;
	
	if (dlg.DoModal() == IDOK) //	plate->SmoothDegree = dlg.m_Degree;
		pDoc->pMarkedPlate->SmoothDegree = dlg.m_Degree;
	else  //	plate->SmoothDegree = 0;
		pDoc->pMarkedPlate->SmoothDegree = 0;

	pDoc->ShowProfiles = TRUE;
	pDoc->pMarkedPlate->ShowLoadState();
	pDoc->OnPlotMaxprofiles();


/*	int Sdegree = plate->SmoothDegree;
	CLoad * OldLoad = plate->Load;
	CLoad * NewLoad = OldLoad->Smoothed(OldLoad, Sdegree);
	plate->Load = NewLoad;
	//pDoc->OnPlotLoadmap();*/

	//pDoc->ShowProfiles = TRUE;
	
/*	plate->Load = OldLoad;
	delete (NewLoad);
	plate->SmoothDegree = 0;
*/
}

void CMainView::OnLButtonDblClk(UINT /* nFlags */, CPoint point) 
{
	/*
	CString name = "Fig.wmf";
	CFileDialog dlg(FALSE, "wmf | * ", name, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
	"Store Image as Windows Metafile (*.wmf) | *.wmf; *.WMF | All Files (*.*) | *.*||", NULL);
	if (dlg.DoModal() != IDOK) return;
	name = dlg.GetPathName();
	CRect rect;
	GetClientRect(rect);
	CSize size = rect.Size();
	CMetaFileDC dcm;	
	dcm.Create(name);
	dcm.SetMapMode(MM_ANISOTROPIC);
	dcm.SetWindowOrg(0,0);
	dcm.SetWindowExt(size);

	OnDraw(&dcm);

	HMETAFILE hMF = CloseMetaFile(dcm);
	//	ASSERT(hMF != NULL);
	DeleteMetaFile(hMF);
	*/
	//SwapMouseButton(FALSE);

	CBTRDoc* pDoc = (CBTRDoc*)GetDocument();
	if (pDoc->STOP != TRUE) return; // still running!
	CClientDC dc(this);
	OnPrepareDC(&dc);
	dc.DPtoLP(&point);
	
	PtrList & List = pDoc->PlatesList;
	CPlate * plate;
	BOOL Marked = FALSE; // captured RectMark
	POSITION pos = List.GetHeadPosition();
	while (pos != NULL) {
		plate = List.GetNext(pos);
		if (plate->RectMark->PtInRect(point)) {
			Marked = TRUE;
			plate->Selected = TRUE;
			pDoc->pMarkedPlate = plate;
			//plate->ShowLoadState();			
		}// in RgnMark
		//else plate->Selected = FALSE;
	}//while

	if (Marked) { // user pushed hotpoint
		//OnPlateSelect();
		return;
	}

	// else - save image

	HBITMAP bitmap = CaptureWindow(*this);

	CString name = "Fig.bmp";
	CFileDialog dlg(FALSE, "bmp | * ", name, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"Save the Image as BMP-file (*.bmp) | *.bmp; *.BMP | All Files (*.*) | *.*||", NULL);
	if (dlg.DoModal() != IDOK) return;
	name = dlg.GetPathName();

	SaveBitmap(bitmap, name);

	//	CScrollView::OnLButtonDblClk(nFlags, point);
}

void CMainView::OnPlateEmit() 
{
	CBTRDoc* pDoc = (CBTRDoc*)GetDocument();
	pDoc->OnStart();

}

void CMainView::OnLButtonDown(UINT /* nFlags */, CPoint point) 
{
	//	Progress = 0;
	CBTRDoc* pDoc = (CBTRDoc*)GetDocument();
	if (pDoc->STOP != TRUE) return; // still running!

	LastPoint = point;
	CClientDC dc(this);
	OnPrepareDC(&dc);
	dc.DPtoLP(&point);
	
	PtrList & List = pDoc->PlatesList;
	CPlate * plate;
	CPlate * plate0 = NULL;
	BOOL Overlapped = FALSE;
	BOOL Marked = FALSE; // captured RectMark
	CString S;

/*	if (pDoc->OptCombiPlot == 0) { // SWITHCHED-OFF / combi is calculated - > clear selection
		pDoc->OnPlateClearSelect();
		pDoc->pMarkedPlate->Selected = FALSE;
		//pDoc->OptCombiPlot = -1;
	}*/ 
	pDoc->OptCombiPlot = -1; // no plate selected!!
	pDoc->pMarkedPlate = NULL;
	POSITION pos = List.GetHeadPosition();
	while (pos != NULL) {
		plate = List.GetNext(pos);
		if (plate->RectMark->PtInRect(point)) {
			Marked = TRUE;
			if (plate0 == NULL) plate0 = plate;
			else {
				Overlapped = TRUE;
				S.Format("%d, %d", plate->Number, plate0->Number);
				CNumberDlg dlg;
				dlg.m_Text = "Surfaces found:  " + S; 
				dlg.SurfNumber = plate->Number;
				if (dlg.DoModal() == IDOK) {
					if (plate->Number  == dlg.SurfNumber) plate0 = plate; // else plate0 is unchanged
				} // ok
				//AfxMessageBox("At least to surfaces are here");
			} // overlap

			//if (!pDoc->OptParallel) plate->Touched =  TRUE;//!plate->Touched;//
			/*pDoc->OptPDP = FALSE;
			plate0->Selected = TRUE; //!plate->Selected; //TRUE;
			pDoc->pMarkedPlate = plate0;
			pDoc->ShowProfiles = TRUE;
			plate0->ShowLoadState();	
			if (pDoc->OptParallel && pDoc->OptDrawPart)  //&& !plate->Loaded 
			{
			plate0->ShowEmptyLoad();
			pDoc->ShowPlatePoints();
			}*/

			//pDoc->OptCombiPlot = 1;
			//if (plate->Selected) plate->DrawPlate(this, &dc); // on MainView
			//else { InvalidateRect(NULL, TRUE); pDoc->OnShow();}
			//return;

		}// in RgnMark
		//else { // unselect other plates not marked
		//if (pDoc->OptCombiPlot > 1)	plate->Selected = FALSE; // unselect all the rest if combi done
		//}
	}//while pos

	//pLV->Invalidate(TRUE);
	//pDoc->OnShow();

	if (Marked) { // plate0
		pDoc->OptPDP = FALSE;
		pDoc->pMarkedPlate = plate0;
		//pDoc->ShowProfiles = TRUE;
	
// >> OLD version 
	/*	if (pDoc->OptParallel && pDoc->OptDrawPart)  //&& !plate->Loaded 
		{	plate0->ShowEmptyLoad();	pDoc->ShowPlatePoints();}
		else {pDoc->OnPlotMaxprofiles(); 	plate0->ShowLoadState();}*/
// << OLD version 
		pDoc->OptCombiPlot = 1; // IMPORTANT map is now selected (not calculated)
	// NEW	
		pDoc->OnPlateSelect(); 	//for pMarkedPlate Calls: SetLoadArray(plate, TRUE);OptCombiPlot = 0; P_CalculateLoad(plate);
		plate0->DrawPlate(this, &dc, SHOW_BEAM);// show red in the main view
		plate0->SetViewDim(); // - set max limits 
		plate0->ShowEmptyLoad(); // show total rect, pLV->SetPlate(this);
		plate0->DrawLoadRect(); //can be zero - calculate and Show Scale (local) for current limits!!
		plate0->DrawPlateBound(); // plate polygon - scale not recalculated
		//if(pDoc->OptDrawPart)
		
		//if (plate0->Number == pDoc->PlasmaEmitter)
		pDoc->ShowProfiles = TRUE;
		pDoc->ShowPlatePoints(TRUE);//(pDoc->OptDrawPart);
		pDoc->OnPlotMaxprofiles();
		// particle spots - scale not recalculated
		return;
	
	}// marked

	/*else { // no surf rect marked
	//InvalidateRect(NULL, TRUE); 
	//pDoc->OnShow();
	}*/

	//	if (Marked) { // hotpoint
	//if (pDoc->pMarkedPlate->Comment.Find("First")>=0) pDoc->CreateBaseSingle(); // set new pMarkedPlate
	//		pDoc->OptCombiPlot = 1;//FALSE;  //InvalidateRect(NULL, TRUE);
	//	}
	
	int i,k;

	if (!Marked) {// && !MODE_ZOOM) {
		for (k = 0; k < (int) pDoc->NofChannelsHor; k++) {
			if (pDoc->RectIS[k].PtInRect(point)){

				if (pDoc->OptSINGAP) {//pDoc->SetSINGAP();
					AfxMessageBox("Active beam is SINGAP.\n Groups can unselected only for MAMuG beam");
					break;
				}

				pDoc->ActiveCh[k] = !(pDoc->ActiveCh[k]);
				//pDoc->SetStatus();
				if (pDoc->ActiveCh[k]) pDoc->NofActiveChannels += 1;
				else pDoc->NofActiveChannels -= 1;

				if (pDoc->NofActiveChannels < 1)  // disable rows if no channels active
					for (i = 0; i < (int) pDoc->NofChannelsVert; i++) pDoc->ActiveRow[i] = FALSE;

				if (pDoc->NofActiveChannels == 1 && pDoc->NofActiveRows <1) // enable all rows if 1-st channel activated
					for (int i = 0; i < (int) pDoc->NofChannelsVert; i++) pDoc->ActiveRow[i] = TRUE;

				if (STOP) //(NofCalculated == 0)
					pDoc->SetStatus();
				pDoc->OnShow(); // calls  OnDataActive();
				Marked = TRUE;
				break;
			}// if RectIS
		} //k
	} // not Marked

	if (!Marked) {
		for (int n = 0; n < (int)pDoc->NofChannelsVert; n++) {
			if (pDoc->RectISrow[n].PtInRect(point)){

				if (pDoc->OptSINGAP) {//pDoc->SetSINGAP();
					AfxMessageBox("Active beam is SINGAP.\n Groups can unselected only for MAMuG");
					break;
				}

				pDoc->ActiveRow[n] = !(pDoc->ActiveRow[n]);
				//	pDoc->SetStatus();
				if (pDoc->ActiveRow[n]) pDoc->NofActiveRows += 1;
				else pDoc->NofActiveRows -= 1;

				if (pDoc->NofActiveRows < 1)  // disable channels if no rows active
					for (i = 0; i < (int)pDoc->NofChannelsHor; i++) pDoc->ActiveCh[i] = FALSE;

				if (pDoc->NofActiveRows == 1 && pDoc->NofActiveChannels < 1)  // enable all channels if 1-st row activated
					for (i = 0; i < (int)pDoc->NofChannelsHor; i++) pDoc->ActiveCh[i] = TRUE;

				if (STOP) //(NofCalculated == 0) 
					pDoc->SetStatus();
				pDoc->OnShow(); // calls  OnDataActive();
				Marked = TRUE;
				break;
			}//if RectISrow
		}//n
	} // !Marked 


	if (!Marked) { 
		if (pDoc->OptTraceSingle) {
			double X = (point.x - OrigX) / ScaleX;
			double Y = -(point.y - OrigY) / ScaleY;
			pDoc->TraceSingleAgain(X, Y);
		}
		else {
			InvalidateRect(NULL, TRUE); 
			pDoc->OnShow();
			//Message_NotSelected();
		}
	}
	
}

void CMainView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	CBTRDoc* pDoc = (CBTRDoc*)GetDocument();
	pDoc->OptTraceSingle = FALSE;
	
	CScrollView::OnRButtonDown(nFlags, point);
}

/*void CMainView::OnPlateSelect() 
{
//	MODE_SELECT = TRUE;
//	InvalidateRect(NULL, TRUE);
CPoint point = LastPoint;
CBTRDoc* pDoc = (CBTRDoc*)GetDocument();
PtrList & List = pDoc->PlatesList;
CPlate * plate;
BOOL Marked = FALSE; // captured RectMark
BOOL flag = FALSE; // Apply Load
//CRect rect1, rect2;
double h, hx, hy, dim, dimH, dimV, dist;
C3Point H;
CString S;

POSITION pos = List.GetHeadPosition();
///	pDoc->pMarkedPlate = NULL;
pDoc->OptCombiPlot = -1;
BOOL Captured  = FALSE;
while (pos != NULL && !Captured) {
plate = List.GetNext(pos);

if (plate->RectMark->PtInRect(point)) {// captured
Marked = TRUE;
pDoc->pMarkedPlate = plate;
Captured  = TRUE;
pDoc->OptCombiPlot = 1;

if (pDoc->OptParallel) { //parallel
if (plate->Loaded) S.Format("Recalculate the power map?");
else S.Format("Calculate the power map?");
if (AfxMessageBox(S, MB_ICONQUESTION | MB_YESNO) == IDYES) 
{
if (plate->Loaded == FALSE) { // not calculated yet
InvalidateRect(NULL, TRUE);
plate->ApplyLoad(TRUE, 0, 0); // optimize grid 
pDoc->SetLoadArray(plate, TRUE);
}

plate->Selected = TRUE;
//plate->Load->Clear();
pDoc->ShowProfiles = TRUE;
//pDoc->P_CalculateLoad(plate);
//if (plate->Comment.Find("First")>=0) pDoc->CreateBaseSingle();
pDoc->OnPlateClearSelect();
//pDoc->OnPlotCombi();//
//pDoc->P_CalculateCombi(1);

plate->ShowLoadState();
pDoc->OptCombiPlot = 1;

}
break;
}
else {//!(pDoc->OptParallel
plate->Selected = !plate->Selected;//TRUE;
pDoc->ShowProfiles = TRUE;
pDoc->OnPlateClearSelect();
///		if (!plate->Selected) pDoc->pMarkedPlate = NULL; 						
//plate->ShowLoadState();	
pDoc->ShowStatus();// ShowNothing();
pDoc->OptCombiPlot = 0;

break;
} // else / not parallel 

//	Sleep(1000);
//	plate->ShowLoadState(); // show summary (info)

}// captured RgnMark

else plate->Selected = FALSE;
}//while List not end

//if (Marked) InvalidateRect(NULL, TRUE);
if (!Marked) pDoc->ShowStatus();// ShowNothing();

}*/

/*void CMainView::OnStop() 
{
STOP = TRUE;
Beep(100,100);
//	MODE_SELECT = FALSE;
CBTRDoc* pDoc = (CBTRDoc*)GetDocument();
pDoc->STOP = TRUE;
pDoc->StopTime = CTime::GetCurrentTime();
pDoc->OnStop();
//	pLoadView->STOP = TRUE;
}*/


void CMainView::OnPlateProperties() 
{
	CBTRDoc* pDoc = (CBTRDoc*)GetDocument();
	CPlate * plate = pDoc->pMarkedPlate;
	if (plate == NULL) return;

	pDoc->ShowProfiles = TRUE;
	pDoc->pMarkedPlate->ShowLoadState();
	pDoc->OnPlotMaxprofiles();
	return;

/////////NOT CALLED //////////////////////////////////////////////////
	if (pDoc->OptCombiPlot == -1) { //|| !(pDoc->pMarkedPlate->Loaded)) {
		pDoc->ShowStatus(); return;
	}
	
	if (pDoc->OptCombiPlot == 0) { // base already exists
		pDoc->OnPlotMaxprofiles();
		plate->ShowLoadState();
		return;
	}

	if (pDoc->OptParallel) { //parallel
		if (plate->Loaded == FALSE) { // not calculated yet
			//InvalidateRect(NULL, TRUE);
			pDoc->pMarkedPlate->ApplyLoad(TRUE, 0, 0); // optimize grid 
			pDoc->pMarkedPlate->Loaded = TRUE;
			pDoc->SetLoadArray(plate, TRUE);
		}

		pDoc->OnPlateClearSelect();

	} // parallel

	// non-parallel
	else if(!(pDoc->pMarkedPlate->Loaded)) {
		Message_NotSelected();

	}
//	pDoc->CreateBaseSingle();// set new pMarkedPlate
	pDoc->OnShow();
	//pDoc->pMarkedPlate->ShowLoadState();
	pDoc->OnPlotMaxprofiles();

}

void CMainView::OnPlateClear() 
{
	//	CPoint point = LastPoint; 
	CBTRDoc* pDoc = (CBTRDoc*)GetDocument();
	if (pDoc->OptCombiPlot == -1 || pDoc->pMarkedPlate->Loaded == FALSE) { 
		Message_NotSelected();
		pDoc->ShowStatus(); return;
	}
	CPlate * plate = pDoc->pMarkedPlate;
	if (AfxMessageBox("Clear power map on this surface?", MB_ICONQUESTION | MB_YESNO) != IDYES) return;
	plate->Load->Clear(); // set val to 0, keep array 
	plate->Loaded = FALSE;
	plate->SmoothDegree = 0; //pDoc->SmoothDegree;
	plate->ShowLoadState();
	pDoc->SetLoadArray(plate, FALSE);
	//plate->ShowLoad();
	//InvalidateRect(NULL, TRUE);
	pDoc->OnShow();
}


void CMainView::OnPlateScale() 
{
	CBTRDoc* pDoc = (CBTRDoc*)GetDocument();
	pDoc->OnPlateScale();
	return;

	if (pDoc->OptCombiPlot == -1) { pDoc->ShowStatus(); Message_NotSelected(); return; }
	CPlate * plate = pDoc->pMarkedPlate;
	if (plate == NULL) { pDoc->ShowStatus(); return; }

	C3Point p0, p1, p2, p3;
	double stepX, stepY;// , left, right, bot, top;
	double xmin, xmax, ymin, ymax, zmin, zmax;
	CString S;
	/*	if (!pDoc->OptParallel) { 
	if (AfxMessageBox("Clear power map on this surface?", MB_ICONQUESTION | MB_YESNO) != IDYES) return;
	plate->Load->Clear();
	plate->Loaded = FALSE;
	}
	*/	
	//pDoc->OnPlateSelect(); 	//for pMarkedPlate Calls: SetLoadArray(plate, TRUE);OptCombiPlot = 0; P_CalculateLoad(plate);
	//plate->DrawPlate(this, &dc);// show red in the main view
	
	//plate->SetViewDim(); // - set max limits 
	plate->ShowEmptyLoad(); // show total rect, pLV->SetPlate(this);
	plate->DrawLoadRect(); //can be zero - calculate and Show Scale (local) for current limits!!
	plate->DrawPlateBound(); // plate polygon - scale not recalculated
	//if (pDoc->OptDrawPart)
	pDoc->ShowPlatePoints(pDoc->OptDrawPart); // particle spots - scale not recalculated

	//pDoc->OptCombiPlot = 1; // map selected but maybe not calculated!
/*	pDoc->pMarkedPlate->SetViewDim(); // - set max limits 
	pDoc->pMarkedPlate->ShowEmptyLoad(); // update LV scale, show total rect
	pDoc->pMarkedPlate->DrawLoadRect(); //can be zero - calculate and Show Scale (local) for current limits!!
	pDoc->pMarkedPlate->DrawPlateBound(); // plate polygon - scale not recalculated
	pDoc->ShowPlatePoints(); // particle spots - scale not recalculated*/
	//OnPlateLoad();

/*	if (pDoc->pMarkedPlate->Loaded == FALSE) { //  -> done by OnPlateLoad()
		pDoc->pMarkedPlate->ApplyLoad(TRUE, 0, 0);
		pDoc->pMarkedPlate->Loaded = TRUE;
		pDoc->SetLoadArray(pDoc->pMarkedPlate, TRUE);
	}*/

	LoadStepDlg dlg;
	dlg.m_StepX = plate->Load->StepX;
	dlg.m_StepY = plate->Load->StepY;
	dlg.m_Xmin = plate->leftX; //dlg.m_Xmin = plate->Vmin.X; dlg.m_Xmax = plate->Vmax.X;
	dlg.m_Xmax = plate->rightX;//dlg.m_Ymin = plate->Vmin.Y; dlg.m_Ymax = plate->Vmax.Y;
	dlg.m_Ymin = plate->botY;  //dlg.m_Zmin = plate->Vmin.Z; dlg.m_Zmax = plate->Vmax.Z;
	dlg.m_Ymax = plate->topY;
	dlg.m_Zmin = 0; 
	dlg.m_Zmax = plate->Load->MaxVal;//0.0; 
	//if (plate->Loaded) dlg.m_Zmax = 1.0;// 

		
	if (dlg.DoModal() == IDOK) {
		//plate->Load->StepX = dlg.m_StepX;
		//plate->Load->StepY = dlg.m_StepY;
		//plate->Vmin = C3Point(dlg.m_Xmin, dlg.m_Ymin, dlg.m_Zmin);
		//plate->Vmax = C3Point(dlg.m_Xmax, dlg.m_Ymax, dlg.m_Zmax);
		
		stepX = dlg.m_StepX; 
		stepY = dlg.m_StepY;
		xmin =  dlg.m_Xmin; 
		xmax =  dlg.m_Xmax;
		ymin =  dlg.m_Ymin;
		ymax =  dlg.m_Ymax;
		zmin =  dlg.m_Zmin;
		zmax =  dlg.m_Zmax;
		
		S.Format(" steps: %g / %g\n  Dims: X: %g - %g  Y: %g - %g ", stepX, stepY, xmin, xmax, ymin, ymax);
		AfxMessageBox(S);
		
		
		plate->ApplyLoad(TRUE, stepX, stepY); // individual Loaded->TRUE
				//SetNullLoad(plate); // create zero load with default mesh options
		pDoc->P_CalculateLoad(plate);
		

		//SetLoadArray(plate, TRUE);
		//pDoc->OnPlotMaxprofiles();
		//plate->ShowLoadState(); // show summary (info)
		if (xmax - xmin > stepX && ymax - ymin > stepY)
			plate->SetViewDim(xmin, xmax, ymin, ymax); // - set new limits 

		plate->ShowEmptyLoad(); // show total rect, pLV->SetPlate(this);
		plate->DrawLoadRect(); //can be zero - calculate and Show Scale (local) for current limits!!
		plate->DrawPlateBound(); // plate polygon - scale not recalculated
		//pDoc->ShowPlatePoints(); // particle spots - scale not recalculated*/

		/*if (plate->Number == 2000) {
			pDoc->CalculateTracks(); // beam planes in plasma
		}*/
		
	} //dlg IDOK
	//else return;// (dlg.DoModal() == IDCANCEL) 

	pDoc->ShowProfiles = TRUE;
	pDoc->OnPlotMaxprofiles();	
	//pDoc->pMarkedPlate->ShowLoadState(); // show summary (info)	
	plate->ShowLoadState(); // show summary (info)
	//pDoc->OnPlotLoadmap();
}

void CMainView::OnMouseMove(UINT nFlags, CPoint point) 
{
	CBTRDoc* pDoc = (CBTRDoc*)GetDocument();
	if (!(pDoc->STOP))  {
		CScrollView::OnMouseMove(nFlags, point);
		return; // still running!
	}

		
	int k;
	//CDC* pDC = GetDC();
	CString S;
	//pDC->SetTextColor(RGB(0, 100, 100));
	//pDC->SelectObject(&smallfont);
	CClientDC dc(this);
	OnPrepareDC(&dc);
	dc.DPtoLP(&point);
	//ValidateRect(NULL);
	
	PtrList & List = pDoc->PlatesList;
	CPlate * plate;
	POSITION pos = List.GetHeadPosition();
	while (pos != NULL) {
		plate = List.GetNext(pos);
		if (plate->RectMark->PtInRect(point)) {
			SetCursor(LoadCursor(NULL, IDC_CROSS));
			S.Format("%d", plate->Number);

			if (!plate->Loaded)	{ 
				dc.SetTextColor(RGB(0, 100, 100));
				dc.SelectObject(&smallfont);
			}
			else { 
				dc.SetTextColor(RGB(255, 0, 0)); 
				dc.SelectObject(&midfont);
			}
			dc.TextOut(plate->RectMark->left + 1, plate->RectMark->top - 15, S);
			dc.SetTextColor(RGB(0, 0, 0));
			return; //break;
		}
	}
	for (k = 0; k < (int)pDoc->NofChannelsHor; k++) {
		if (pDoc->RectIS[k].PtInRect(point)){
			SetCursor(LoadCursor(NULL, IDC_CROSS));
			return;//break;
		}
	}
	for (k = 0; k < (int)pDoc->NofChannelsVert; k++) {
		if (pDoc->RectISrow[k].PtInRect(point)){
			SetCursor(LoadCursor(NULL, IDC_CROSS));
			return;//break;
		}
	}

	/*	if (MODE_BOX) {
	dc.SelectObject(&DotPen);
	dc.SelectObject(GetStockObject(NULL_BRUSH));
	SetROP2(dc,R2_NOT); 
	CPoint oldTL =  BoxRect.TopLeft();
	CPoint oldBR =  BoxRect.BottomRight();
	dc.Rectangle(BoxRect);
	//dc.FrameRect(BoxRect, NULL_BRUSH);
	/*BoxRect.left -= 2; BoxRect.top -=2;
	int dx = oldBR.x +2 - point.x;
	BoxRect.right += dx;
	int dy = point.y - oldBR.y + 2;
	BoxRect.bottom += dy;
	InvalidateRect(&BoxRect, TRUE);*/
	/*	BoxRect.left = oldTL.x; BoxRect.top = oldTL.y;
	BoxRect.right = point.x; BoxRect.bottom = point.y;
	dc.Rectangle(BoxRect);
	//	dc.Rectangle(BoxRect.left-1,BoxRect.top-1, BoxRect.right+1,BoxRect.bottom+1);
	ZOOM_FINISHED = FALSE;
	}*/

	CScrollView::OnMouseMove(nFlags, point);
}

void CMainView::OnPlateDelete() 
{
	CBTRDoc* pDoc = (CBTRDoc*)GetDocument();
	if (pDoc->OptCombiPlot == -1) { 
		pDoc->ShowStatus(); return;
	}

	CPlate * plate = pDoc->pMarkedPlate;
	//if (AfxMessageBox("Remove the surface?", MB_ICONQUESTION | MB_YESNOCANCEL) == IDYES) {

	CPoint point = LastPoint; 

	/*	PtrList & AList = pDoc->AddPlatesList;
	POSITION pos = AList.GetHeadPosition();

	while (pos != NULL) {
	POSITION prevpos = pos;
	plate = AList.GetNext(pos);
	C3Point Dist = (plate->Orig) - pDoc->pMarkedPlate->Orig;
	C3Point Vsum1 = VectSum(plate->OrtX, pDoc->pMarkedPlate->OrtX, 1, -1);
	C3Point Vsum2 = VectSum(plate->OrtY, pDoc->pMarkedPlate->OrtY, 1, -1);
	if (ModVect(Dist) < 1.e-3 && ModVect(Vsum1) < 1.e-2 && ModVect(Vsum2) < 1.e-2) {
	//if (plate == pDoc->pMarkedPlate){
	AList.RemoveAt(prevpos);
	delete plate;
	break;
	}

	}//while */

	PtrList & List = pDoc->PlatesList;
	POSITION pos = List.GetHeadPosition();

	while (pos != NULL) {
		POSITION prevpos = pos;
		plate = List.GetNext(pos);
		if (plate == pDoc->pMarkedPlate){
			if (plate->Loaded)	pDoc->SetLoadArray(plate, FALSE);
			List.RemoveAt(prevpos);
			delete plate;
			break;  
		}

	}//while

	//pDoc->pMarkedPlate = NULL;
	pDoc->OptCombiPlot = -1;
	InvalidateRect(NULL, TRUE);
	//} // if yes
}

void CMainView::OnPlateSave() 
{

	CBTRDoc* pDoc = (CBTRDoc*)GetDocument();
	if (pDoc->OptCombiPlot == -1 || pDoc->pMarkedPlate->Loaded == FALSE) { 
		Message_NotSelected();
		pDoc->ShowStatus(); return;
	}
	CPlate * plate = pDoc->pMarkedPlate;
	if (plate == NULL) return;
	CString  sn;
	sn.Format("%d",plate->Number);
	CString name = "load" + sn + ".txt"; 
	CFileDialog dlg(FALSE, "*", name);
	if (dlg.DoModal() == IDOK) {
		FILE * fout;
		//CFile file;
		char buf[1024]; 
		strcpy(buf, dlg.GetPathName());
		fout = fopen(buf, "w");
		if (pDoc->OptFree) plate->WriteLoadAdd(fout);
		else plate->WriteLoad(fout);
		plate->filename = dlg.GetPathName();
		fclose(fout);
	}
	//} // marked
	/*	else  {
	Message_NotSelected();
	pDoc->ShowStatus();
	//		ShowNothing();
	}*/
}



void CMainView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	/*	CClientDC dc(this);
	OnPrepareDC(&dc);
	dc.DPtoLP(&point);
	int x1, x2, y1, y2;
	if (MODE_BOX) {
	dc.SelectObject(GetStockObject(NULL_BRUSH));
	SetROP2(dc,R2_NOT); 
	dc.SelectObject(&DotPen);
	dc.Rectangle(BoxRect);
	//InvalidateRect(&BoxRect, TRUE);
	BoxRect.right = point.x; BoxRect.bottom = point.y;
	dc.Rectangle(BoxRect);
	dc.LPtoDP(BoxRect);
	//dc.Rectangle(BoxRect.left-1,BoxRect.top-1, BoxRect.right+1,BoxRect.bottom+1);
	ReleaseCapture();
	MODE_BOX = FALSE;
	//		MODE_ZOOM = FALSE;
	ZOOM_FINISHED = TRUE;

	x1 = BoxRect.left; x2 = BoxRect.right;
	y1 = BoxRect.top; y2 = BoxRect.bottom;
	BoxRect.left = Min(x1, x2);
	BoxRect.right = Max(x1, x2);
	BoxRect.top = Min(y1, y2);
	BoxRect.bottom = Max(y1, y2);
	if (BoxRect.Size().cx >0 && BoxRect.Size().cy > 0)
	ZoomView();
	//InvalidateRect(NULL, TRUE);
	}*/
	//if (pDoc->STOP != TRUE) return; // still running!
	CScrollView::OnLButtonUp(nFlags, point);
}
/*
void CMainView::OnViewZoombox() 
{
//	MODE_ZOOM = TRUE;
//	MODE_BOX = FALSE;
//	ZOOM_FINISHED =  FALSE;
//	MODE_FIT = FALSE;

}*/

void CMainView::OnPlateSelectallplates() // Calculate all loads
{
	CBTRDoc* pDoc = (CBTRDoc*)GetDocument();
	pDoc->OnOptionsSurfacesEnabledisableall(); // Calculate all loads
	InvalidateRect(NULL, TRUE);

	/*	PtrList & List = pDoc->PlatesList;
	CPlate * plate;
	CString S;

	if (pDoc->OptParallel) { //parallel
	POSITION pos = List.GetHeadPosition();
	pDoc->pMarkedPlate = NULL;
	S.Format("Calculate all non-zero power maps?");
	if (AfxMessageBox(S, MB_ICONQUESTION | MB_YESNO) == IDYES) 
	{
	CWaitCursor wait;
	while (pos != NULL) {
	plate = List.GetNext(pos);
	if (plate->Touched == TRUE) { // non-zero

	plate->ApplyLoad(TRUE, 0, 0); // optimize grid 
	pDoc->SetLoadArray(plate, TRUE);
	plate->Load->Clear();
	pDoc->P_CalculateLoad(plate);
	}
	}
	pDoc->ShowStatus();
	InvalidateRect(NULL, TRUE);
	} // YES
	}//parallel

	else //!(pDoc->OptParallel
	pDoc->SelectAllPlates();
	*/
}

void CMainView::OnViewBeam() 
{
	SHOW_BEAM = !SHOW_BEAM;
	InvalidateRect(NULL, TRUE);
}

void CMainView::OnUpdateViewBeam(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(SHOW_BEAM);
}

void CMainView::OnViewFields() 
{
	SHOW_FIELDS = !SHOW_FIELDS;
	InvalidateRect(NULL, TRUE);
}

void CMainView::OnUpdateViewFields(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(SHOW_FIELDS);

}

void CMainView::OnViewNumbering() 
{
	SHOW_NUMBERS = !SHOW_NUMBERS;
	InvalidateRect(NULL, TRUE);
}

void CMainView::OnUpdateViewNumbering(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(SHOW_NUMBERS);
}




void CMainView::OnUpdatePlateSelect(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	CBTRDoc* pDoc = (CBTRDoc*)GetDocument();

	if (pDoc->OptParallel) pCmdUI->SetText("Calculate");
	else pCmdUI->SetText("Select / Unselect");

}


void CMainView::OnRButtonDblClk(UINT /* nFlags */, CPoint /* point */)
{
	// TODO: Add your message handler code here and/or call default
	//SwapMouseButton(FALSE);

	InvUser = FALSE;
	//CScrollView::OnRButtonDblClk(nFlags, point);
}

void CMainView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: Add your message handler code here and/or call default

	CScrollView::OnVScroll(nSBCode, nPos, pScrollBar);
	//InvalidateRect(NULL, TRUE);

}

void CMainView::OnSize(UINT /* nType*/ , int /* cx */, int /* cy */)
{
	//CScrollView::OnSize(nType, cx, cy);
	CRect rect;
	GetClientRect(rect);

	// TODO: Add your message handler code here
	//InvalidateRect(&rect, TRUE);
}

