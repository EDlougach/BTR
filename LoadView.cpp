// LoadView.cpp : implementation file
//
#include "stdafx.h"
#include "BTR.h"
#include "LoadView.h"
#include "BTRDoc.h"
#include "InvertText.h"
#include "BMP_utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CLoadView * pLoadView;
//CMainView  * pMainView;
/////////////////////////////////////////////////////////////////////////////
// CLoadView

IMPLEMENT_DYNCREATE(CLoadView, CScrollView)

CLoadView::CLoadView()
{
	pLoadView = this;
	STOP = FALSE;

/*	
font.CreateFont(-12, 0, 0, 0, 100, FALSE, FALSE, 0,
		0, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
		DEFAULT_PITCH | FF_MODERN, "Courier");// too wide // list of AS
	smallfont.CreateFont(-14, 0, 0, 0, 400, FALSE, FALSE, 0,
		ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
		DEFAULT_PITCH | FF_MODERN, "Arial");//"Courier New"); // AS Info
*/	

	font.CreateFont(-14, 0, 0, 0, 600, FALSE, FALSE, 0,
		0, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
		DEFAULT_PITCH | FF_MODERN, "Arial");// too wide // list of AS
	smallfont.CreateFont(-12, 0, 0, 0, 500, FALSE, FALSE, 0,
		ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
		DEFAULT_PITCH | FF_MODERN, "Arial");//"Courier New"); // AS Info
	bigfont.CreateFont(-16, 0, 0, 0, 700, FALSE, FALSE, 0,
		0, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
		DEFAULT_PITCH | FF_MODERN, "Arial");
}

CLoadView::~CLoadView()
{
}


BEGIN_MESSAGE_MAP(CLoadView, CScrollView)
	//{{AFX_MSG_MAP(CLoadView)
	ON_WM_LBUTTONDBLCLK()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLoadView drawing

void CLoadView::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();

	CSize sizeTotal;
	// TODO: calculate the total size of this view
	sizeTotal.cx = 200; sizeTotal.cy = 5000;
	SetScrollSizes(MM_TEXT, sizeTotal);
	ShowLoad = FALSE;
	Xmax = 1; Ymax = 1; Xmin = 0; Ymin = 0; 
	Load = NULL;
	LoadRect = NULL;
	Contours = FALSE;
	STOP = FALSE;
	LabelArray.RemoveAll();

}

void CLoadView::OnDraw(CDC* pDC)
{
	STOP = FALSE;
	CBTRDoc* pDoc = (CBTRDoc*)GetDocument();
	CPen pen, thickpen;
	pen.CreatePen(PS_SOLID, 1, RGB(200,200,200));
	thickpen.CreatePen(PS_DOT, 3, RGB(150,150,0));
	CPen * pOldPen = pDC->SelectObject(&pen);
	CFont* pOldFont;// = pDC->SelectObject(&smallfont);
	pDC->SetTextColor(RGB(0,0,0));
	//ShowLoad = TRUE;
	//if (Load == NULL) ShowLoad = FALSE;
	if (pDoc->pMarkedPlate == NULL) ShowLoad = FALSE; 

	CRect rect;
	GetClientRect(rect);
	UpdateScales(rect);
	pDC->SelectObject(pOldPen);
	CString S, S0;
	CPlate * plate;
	C3Point Orig;// plate origin!!
		
	if (ShowLoad) {
		//plate = pDoc->pMarkedPlate;// Plate is already set by SetPlate
		pOldFont = pDC->SelectObject(&bigfont);//(&smallfont);
		pDC->Rectangle(LoadRect);
		pDC->TextOut(OrigX+10, 10, Plate->Comment + "  MAP");
		
		//BeginWaitCursor();
		//Load->DrawLoadLimited(this, pDC, Xmin, Ymin, Xmax, Ymax); //old
		
		if (Plate->Load != NULL) // NEW Show Map/Contours if exists
			if (Contours) 
				Plate->Load->DrawContours(Plate->leftX, Plate->rightX, Plate->botY, Plate->topY);
			else 
				Plate->Load->DrawMapLimited(Plate->leftX, Plate->rightX, Plate->botY, Plate->topY);
		
		// show scale - even for NULL Load 
		pDC->SetROP2(R2_NOT);
		//ShowGlobalScale(pDC);//old
		ShowLocalScale(pDC);

		if (Plate->Load != NULL)
			ShowLabels(pDC);

		//pDC->SelectObject(&thickpen);
		if (Plate->Number == 1000) // poloidal - draw FW
			ShowFWdata();

		//ShowPlateBound(pDC);
		Plate->DrawPlateBound();
		//EndWaitCursor();
		//pDC->SelectObject(pOldFont);

	} // Show Load selected

	else { // show Surf/Loads list
		pOldFont = pDC->SelectObject(&smallfont);
		if (pDoc->LoadSelected < 1) {
			//int N = pDoc->PlateCounter; 
			POSITION pos = pDoc->PlatesList.GetHeadPosition(); 
			//AddPlatesList.GetHeadPosition();
			int k = 0;
			pDC->TextOut(10, 5, " ALL SURFACES (1-solid, 0-transparent) ");
			while (pos != NULL) {
				plate = pDoc->PlatesList.GetNext(pos);
				Orig = plate->Orig;
				S.Format("%3d {%d}  %s       (%7.3f, %7.3f, %7.3f)", //  %3d    %10.3e    %10.3e     
					plate->Number, plate->Solid, plate->Comment, Orig.X, Orig.Y, Orig.Z);
				pDC->TextOut(10, 25 + k*15, S);
				k++;
			}//pos
		}// if Load list empty
 	
		else {// Show Loads 	//pDoc->ShowStatus();
			//DrawBMP(pDC);
			if (pDoc->Sorted_Load_Ind.GetSize() > 1) // sorted 
				S0 = " - Sorted";
			else S0 = "";// - Unsorted";*/
		
			pDC->TextOut(10,5, "NON-ZERO Power maps" + S0);
			S.Format(" Num      Total, W          Max, W/m2      Steps      Comment      Particles");
			pDC->TextOut(10, 20, S); 
		
			double percent, PDmax, Psum;
			int k, line = 0;
			//if (pDoc->LoadSelected < 1) S = "NO loads in the list";
			for (int i = 0; i < pDoc->LoadSelected; i++) {
				k = i;
				if (pDoc->Sorted_Load_Ind.GetSize() > 1) // sorted 
					k = pDoc->Sorted_Load_Ind[i]; 
				plate = pDoc->Load_Arr[k];
				PDmax = plate->Load->MaxVal;
				Psum = plate->Load->Sum;
				if (Psum < 1e-3 || PDmax < 1.e-3) continue;// skip ZERO maps
			/*	percent = (plate->Load->Sum) / (pDoc->NeutrPower)/10000;
				S.Format(" %d    %10.4e    %10.4e    %0.4f", plate->Number, plate->Load->Sum,  plate->Load->MaxVal,  percent);*/
				S.Format(" %3d    %10.3e    %10.3e    %5g / %5g    ", 
				plate->Number, Psum,  PDmax, plate->Load->StepX, plate->Load->StepY);
				S += plate->Comment; S += "  ";
				S += plate->Load->Particles;
				pDC->TextOut(10, 35 + line*15, S);
				line++;
			} // i
		
		} // Show Loads
	}// show Surf/Loads list
	pDC->SelectObject(pOldFont);
	pDC->SelectObject(pOldPen);
	
}

void CLoadView::DrawBMP(CDC* pDC)
{
	//CTestBMPDoc* pDoc = GetDocument();
	//ASSERT_VALID(pDoc);

	CRect rect;
	GetClientRect(rect);
	CDC dcTemp;
	CBitmap cb;
	//cb.LoadBitmap(IDB_BITMAP1);// lighthouse
	cb.LoadBitmap(IDB_BITMAP2);// ITER people on sand
	dcTemp.CreateCompatibleDC(pDC);
	dcTemp.SelectObject(cb);

	pDC->BitBlt(0, 0, rect.Width(), rect.Height(), &dcTemp, 0, 0, SRCCOPY);
	pDC->StretchBlt(0, 0, rect.Width(), rect.Height(), &dcTemp, 
					0, 0, cb.GetBitmapDimension().cx, cb.GetBitmapDimension().cy, SRCCOPY);
}

void CLoadView:: ShowLabels(CDC* pDC) 
{
	if (Load == NULL || !ShowLoad) return;
	int n = LabelArray.GetUpperBound();
	int x, y;
	double val;
	double maxval = Load->MaxVal;
	double order = GetMantOrder(maxval).Z;
	C3Point P;
	CString S;

	for (int i = 0; i <= n; i++) {
			
		P = LabelArray[i];
			x = OrigX + (int)(P.X * ScaleX);
			y = OrigY - (int)(P.Y * ScaleY);
			val = P.Z;

			if (Contours) {
				pDC->SetTextColor(RGB(255,0,0));
				pDC->SetBkColor(RGB(255,255,220));
				//dc.SetBkMode(0);// OPAQUE
			}
			else {
				pDC->SetTextColor(RGB(0,255,0));
				pDC->SetBkColor(RGB(0,0,0));
				//dc.SetBkMode(1);// TRANSPARENT
			}
		
			S.Format("%0.2f", val/order);
			InvertText * text1 = new InvertText(S, *pDC);
			text1->display(*pDC, x, y);
			//pDC->TextOut(x, y, S);

		//	dc.SetBkMode(0);// OPAQUE
			S.Format("* %4.0e", order);
			int w;
			if (pDC->m_hAttribDC != NULL) w = pDC->GetTextExtent(S).cx;//w = dc.GetTextExtent(S).cx;
			else w = 60;
			pDC->TextOut(OrigX + (int)(Xmax * ScaleX) - w - 20, OrigY - (int)(Ymax * ScaleY) +10, S);
	}
}


void CLoadView:: SetLoad_Plate(CLoad * load, CPlate * plate)
{
	SetPlate(plate);
	//Plate = plate;  
 	Load = load;
	/*Xmin = 0;
	Ymin = 0;
	Xmax = plate->Xmax;//Load->Xmax;	Xmin = 0;
	Ymax = plate->Ymax; //Load->Ymax;	Ymin = 0;*/
/*	Xmin = Plate->GetLocal(Plate->Vmin).X;
	Xmax = Plate->GetLocal(Plate->Vmax).X;
	Ymin = Plate->GetLocal(Plate->Vmin).Y;
	Ymax = Plate->GetLocal(Plate->Vmax).Y;*/
	LabelArray.RemoveAll();
}

void CLoadView::SetPlate(CPlate * plate)
{
	Plate = plate;
	//Load = load;
	Xmin = plate->leftX;// 0;
	Ymin = plate->botY;// 0;
	Xmax = plate->rightX;// Xmax;
	Ymax = plate->topY;// Ymax;
	LabelArray.RemoveAll();
}

void CLoadView:: UpdateScales(CRect & rect)
{
	// CBTRDoc* pDoc = (CBTRDoc*)GetDocument();
	rect.DeflateRect(30, 50, 30, 100);
	
	ScaleX = rect.Size().cx /(Xmax - Xmin);
	ScaleY = rect.Size().cy /(Ymax - Ymin);
	OrigX = rect.left;
	OrigY = rect.bottom;
	if (ShowLoad) LoadRect = rect;
	else LoadRect = NULL;
}

void CLoadView::UpdateScales(CRect & rect, double DX, double DY) // not used
{
	// CBTRDoc* pDoc = (CBTRDoc*)GetDocument();
	rect.DeflateRect(30, 50, 30, 100);
	ScaleX = rect.Size().cx / DX;
	ScaleY = rect.Size().cy / DY;
	OrigX = rect.left;
	OrigY = rect.bottom;
	
	if (ShowLoad) LoadRect = rect; // for mouse events
	else LoadRect = NULL;
}


/////////////////////////////////////////////////////////////////////////////
// CLoadView diagnostics

#ifdef _DEBUG
void CLoadView::AssertValid() const
{
	CScrollView::AssertValid();
}

void CLoadView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CLoadView message handlers

void CLoadView::OnLButtonDblClk(UINT /* nFlags */, CPoint /* point */) 
{
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

	STOP = TRUE;
	
	HBITMAP bitmap = CaptureWindow(*this);

	CString name = "Fig.bmp";
	CFileDialog dlg(FALSE, "bmp | * ", name, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"Store Image as BMP-file (*.bmp) | *.bmp; *.BMP | All Files (*.*) | *.*||", NULL);
	if (dlg.DoModal() != IDOK) return;
	name = dlg.GetPathName();
		
	SaveBitmap(bitmap, name);
	
	//CScrollView::OnLButtonDblClk(nFlags, point);
}

void CLoadView::OnMouseMove(UINT  nFlags, CPoint point) 
{
	//STOP = TRUE;
	if (!ShowLoad) return;
	CBTRDoc* pDoc = (CBTRDoc*)GetDocument();
	if (!(pDoc->STOP))  {
		CScrollView::OnMouseMove(nFlags, point);
		return; // still running!
	}
	//if (pDoc->OptPDP) return;
	C3Point Local, Global;
	// int N = pDoc->pMarkedPlate->Number;
	CPlate * plate = pDoc->pMarkedPlate;
	Load = plate->Load;
	CString S;
	CClientDC dc(this);
	OnPrepareDC(&dc);
	dc.DPtoLP(&point);	
	CRect rect;
	GetClientRect(rect);
	CRect text;
	int w, h;
	
	//S.Format("Act Surf %d", N);
	//w = dc.GetTextExtent(S).cx;
	//h = dc.GetTextExtent(S).cy;
	//dc.TextOut(rect.right - w - 10, rect.bottom - h - 5, S);
	dc.SelectObject(&smallfont);
	dc.SetTextColor(RGB(200,0,0));
	CPen pen;
	pen.CreateStockObject(WHITE_PEN);
	CPen * pOldPen = dc.SelectObject(&pen);
	
	if ( LoadRect.PtInRect(point)){
			SetCursor(LoadCursor(NULL, IDC_CROSS));
			Local.X = Xmin + (point.x - OrigX) / ScaleX;
			Local.Y = Ymin + (OrigY - point.y) / ScaleY;
			Local.Z = 0;

			Global = plate->GetGlobalPoint(Local);

			S.Format("GLOBAL  X=%g Y=%g Z=%g", Global.X, Global.Y, Global.Z);//%4.2f
			w = dc.GetTextExtent(S).cx;
			text.SetRect(rect.right - w - 30, 1, rect.right, LoadRect.top-1);
			dc.Rectangle(text);
			dc.TextOut(text.left+10, text.top+2, S);
			S.Format("PD = %g", Load->GetVal(Local.X, Local.Y));
			w = dc.GetTextExtent(S).cx;
			dc.TextOut(text.right - w - 20, text.top + 22, S);
	
	}
	dc.SelectObject(pOldPen);
//	CScrollView::OnMouseMove(nFlags, point);
}

void CLoadView::OnLButtonDown(UINT /* nFlags */, CPoint point) 
{
	STOP = TRUE;
	if (!ShowLoad) return;
	if (!LoadRect.PtInRect(point)) return;
	CBTRDoc* pDoc = (CBTRDoc*)GetDocument();
	CClientDC dc(this);
	OnPrepareDC(&dc);
	dc.DPtoLP(&point);

	double x, y, val, maxval, order;
	CString S;
	C3Point Local, Global;
	CPlate * plate = pDoc->pMarkedPlate;
	x = Xmin + (point.x - OrigX) / ScaleX;
	y = Ymin + (OrigY - point.y) / ScaleY;

	CFont * pOldFont = dc.SelectObject(&font);
	//	dc.SetROP2(R2_NOT); 

	//	if (Contours) {
	if ( LoadRect.PtInRect(point)){
		SetCursor(LoadCursor(NULL, IDC_CROSS));
		//CFont * pOldFont = dc.SelectObject(&font);
		//dc.SetTextColor(RGB(0,0,0));

		Local.X = x; //(point.x - OrigX) / ScaleX;
		Local.Y = y; //(OrigY - point.y) / ScaleY;
		Local.Z = 0;
		Global = plate->GetGlobalPoint(Local);
		maxval = Load->MaxVal;
		order = GetMantOrder(maxval).Z;

		//text.SetRect(rect.right - w - 30, 1, rect.right, LoadRect.top-1);
		//dc.Rectangle(text);
		//dc.TextOut(text.left+10, text.top+2, S);
		if (Contours) {
			dc.SetTextColor(RGB(255,0,0));
			dc.SetBkColor(RGB(255,255,220));
			//dc.SetBkMode(0);// OPAQUE
		}
		else {
			dc.SetTextColor(RGB(0,0,0));
			//dc.SetBkColor(RGB(0,0,0));
			//dc.SetBkMode(1);// TRANSPARENT
		}
		val = Load->GetVal(Local.X, Local.Y);
		Local.Z = val;
		LabelArray.Add(Local);

		S.Format("%0.2f", val/order);//"%0.1f"
		//w = dc.GetTextExtent(S).cx;

		/*if (Contours) {
		dc.TextOut(point.x, point.y, S);
		dc.MoveTo(point.x-2, point.y-2);
		dc.LineTo(point.x+2, point.y+2);
		dc.MoveTo(point.x-2, point.y+2);
		dc.LineTo(point.x+2, point.y-2);
		}*/
		//else {
		InvertText * text1 = new InvertText(S, dc);
		text1->display(dc, point.x, point.y);
		//}

		//dc.SetBkMode(0);// OPAQUE
		S.Format("* %4.0e", order);
		int w;
		if (dc.m_hAttribDC != NULL) w = dc.GetTextExtent(S).cx;//w = dc.GetTextExtent(S).cx;
		else w = 60;
		dc.TextOut(OrigX + (int)((Xmax - Xmin) * ScaleX) - w - 20, OrigY - (int)((Ymax - Ymin) * ScaleY) +10, S);
	}
	//	} // Contours mode
	dc.SelectObject(pOldFont);

	//	CScrollView::OnLButtonDown(nFlags, point);
}

void CLoadView::OnRButtonDown(UINT /* nFlags */, CPoint point) 
{
	STOP = FALSE;
	CBTRDoc* pDoc = (CBTRDoc*)GetDocument();
	if (!ShowLoad) {
		pDoc->OnResultsSaveList(); return;
	}

	// in old version - show 3D view
	/*
	CBTRDoc* pDoc = (CBTRDoc*)GetDocument();
	pDoc->ShowProfiles = FALSE;
	pDoc->UpdateAllViews(this, NULL);	
//	CScrollView::OnRButtonDown(nFlags, point);*/

	//if (!LoadRect.PtInRect(point)) return;
	CClientDC dc(this);
	OnPrepareDC(&dc);
	dc.DPtoLP(&point);	
	double x, y;//, val, maxval, order;
	CString S;
	C3Point Local, Global;
	// CPlate * plate = pDoc->pMarkedPlate;
	x = Xmin + (point.x - OrigX) / ScaleX;
	y = Ymin + (OrigY - point.y) / ScaleY;
	
	Cross.X = x; Cross.Y = y;
	Load->SetProf(x,y);
	InvalidateRect(NULL, TRUE);//(LoadRect, FALSE);
	pDoc->ShowProfiles = TRUE;
	pDoc->UpdateAllViews(this, NULL);
	
/*	CPen Pen;
	Pen.CreatePen(PS_DASHDOT, 1, RGB(0,0,0));
	CPen * pOldPen = dc.SelectObject(&Pen);
	dc.SetROP2(R2_COPYPEN); 
	dc.MoveTo(OrigX, point.y); dc.LineTo(OrigX + (int)(Load->Xmax * ScaleX), point.y);
	dc.MoveTo(point.x, OrigY); dc.LineTo(point.x, OrigY - (int)(Load->Ymax * ScaleY));
	dc.SelectObject(pOldPen);*/

}

void CLoadView::OnSize(UINT nType, int cx, int cy) 
{
	CScrollView::OnSize(nType, cx, cy);
	CRect rect;
	GetClientRect(rect);
	UpdateScales(rect);
	STOP = FALSE;

}
void CLoadView::ShowLocalScale(CDC* pDC)
{
	pDC->SetROP2(R2_NOT);
	//pDC->SelectObject(&smallfont);
	int x, y, w, h;
	int k, kmax;
	CString S;
	double StepH = Plate->crossX;
	double StepV = Plate->crossY;
	double left = Plate->leftX;// 0;
	double right = Plate->rightX;// 0;
	double bot = Plate->botY;// 0;
	double top = Plate->topY;// 0;
	double dimX = right - left;
	double dimY = top - bot;
	
	//horizontal
	double Xloc = left;
	k = 0;
	kmax = (int)ceil(dimX / StepH);//
	while (Xloc <= right) { //Xmax
		x = OrigX + (int)((Xloc - left) * ScaleX);
		pDC->MoveTo(x, OrigY + 2);
		pDC->LineTo(x, OrigY - (int)(dimY * ScaleY)-2);
		S.Format("%g", Xloc);
		w = pDC->GetTextExtent(S).cx / 2;
		//if (k < kmax)
		pDC->TextOut(x - w, OrigY + 2, S);
		Xloc += StepH;
		k++;
	}

	// vertical
	double Yloc = bot;
	k = 0;
	kmax = (int)ceil(dimY / StepV);
	while (Yloc <= top) { // Ymax
		y = OrigY - (int)((Yloc - bot) * ScaleY);
		pDC->MoveTo(OrigX - 2, y); 
		pDC->LineTo(OrigX + (int)(dimX * ScaleX) + 2, y);
		S.Format("%g", Yloc);
		w = pDC->GetTextExtent(S).cx; h = pDC->GetTextExtent(S).cy / 2;
		if (k > -1) {// always!
			pDC->TextOut(OrigX - w - 2, y - h, S);
			pDC->TextOut(OrigX + (int)(dimX * ScaleX) + 5, y - h, S);
		}
		Yloc += StepV;
		k++;
	}
}

void CLoadView:: ShowGlobalScale(CDC* pDC)
{
	CBTRDoc* pDoc = (CBTRDoc*)GetDocument();
	CPlate * plate = pDoc->pMarkedPlate;
	C3Point Local, Global;
	C3Point p0 = plate->Orig;
	pDC->SetROP2(R2_NOT);
	pDC->SelectObject(&smallfont);
	int x, y, w, h;
	CString S;
	double StepH = 0.1;
	if (plate->Load->Xmax < 0.3) StepH = 0.05;
	if (plate->Load->Xmax < 0.1) StepH = 0.01;
	if (plate->Load->Xmax < 0.03) StepH = 0.005;
	if (plate->Load->Xmax >= 0.3) StepH = 0.1;
	if (plate->Load->Xmax > 1) StepH = 0.2;
	if (plate->Load->Xmax > 2) StepH = 0.5;
	if (plate->Load->Xmax > 5) StepH = 1;
	double StepV = 0.1;
	if (plate->Load->Ymax < 0.3) StepV = 0.05;
	if (plate->Load->Ymax < 0.1) StepV = 0.01;
	if (plate->Load->Ymax < 0.03) StepV = 0.005;
	if (plate->Load->Ymax >= 0.3) StepV = 0.1;
	if (plate->Load->Ymax > 1) StepV = 0.2;
	if (plate->Load->Ymax > 2) StepV = 0.5;
	if (plate->Load->Ymax > 5) StepV = 1;
	
	int DirX = 1, DirY = 2; // X-1 Y-2 Z-3
	C3Point Norm = plate->OrtZ;
	if (fabs(Norm.X) >= fabs(Norm.Y))  // major X 
	{
		DirX = 2; // Y
		DirY = 3; // Z
	}
	if (fabs(Norm.Z) >= fabs(Norm.X))  // major Z
	{
		DirX = 1; // X
		DirY = 2; // Y
	}
	if (fabs(Norm.Y) >= fabs(Norm.X) && fabs(Norm.Y) >= fabs(Norm.Z)) // major Y
	{
		DirX = 1; // X
		DirY = 3; // Z
	}
	 
	//if (fabs(plate->OrtX.Y) > 0.5 && fabs(plate->OrtX.Y) > fabs(plate->OrtX.X)) DirX = 2;
	//if (fabs(plate->OrtX.Z) > 0.5 && fabs(plate->OrtX.Z) > fabs(plate->OrtX.Y)) DirX = 3;
	//if (fabs(plate->OrtY.Y) > 0.5 && fabs(plate->OrtY.Y) > fabs(plate->OrtY.X)) DirY = 2;
	//if (fabs(plate->OrtY.Z) > 0.5 && fabs(plate->OrtY.Z) > fabs(plate->OrtY.Y)) DirY = 3;
		
	double Pgl, P0gl = 0.0;
	CString Shor, Svert;
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


	// horizontal 
	Global = plate->MassCentre; 
	Pgl = -10;
	pDC->TextOut(OrigX + (int)((Xmax - Xmin) * 0.6 * ScaleX), OrigY + 25, Shor);
	while (Pgl < 50) {
		Pgl += StepH;
		if (fabs(Pgl) < 1.e-6) Pgl = 0; //continue;
	switch (DirX) {
		case 1: Global.X = Pgl; P0gl = p0.X; break;
		case 2: Global.Y = Pgl; P0gl = p0.Y; break;
		case 3: Global.Z = Pgl; P0gl = p0.Z; break;
		default:  break;
	}

	//	Global = p0 + plate->OrtX * (Pgl - P0gl);
		Local = plate->GetLocal(Global);
		//if (Local.X < Xmin + 0.5 * StepH || Local.X > Xmax - 0.5 * StepH) continue; 
		if (Local.X < Xmin || Local.X > Xmax) continue;
		x = OrigX + (int)((Local.X - Xmin) * ScaleX);
		pDC->MoveTo(x, OrigY + 2); pDC->LineTo(x, OrigY - (int)((Ymax - Ymin) * ScaleY) - 2);
		S.Format("%g", Pgl);
		w = pDC->GetTextExtent(S).cx /2;
		pDC->TextOut(x-w, OrigY + 2, S);
	} // while
	
	// vertical
	Global = plate->MassCentre;
	Pgl = -50;
	pDC->TextOut(OrigX - 20, OrigY - (int)((Ymax - Ymin) * ScaleY) - 18, Svert); 
	while (Pgl < 50) {
		Pgl += StepV;
		if (fabs(Pgl) < 1.e-6) Pgl = 0;//continue;
	switch (DirY) {
		case 1: Global.X = Pgl; P0gl = p0.X; break;
		case 2: Global.Y = Pgl; P0gl = p0.Y; break;
		case 3: Global.Z = Pgl; P0gl = p0.Z; break;
		default:  break;
	}
	
	//	Global = p0 + plate->OrtY * (Pgl - P0gl);
		Local = plate->GetLocal(Global);
		//if (Local.Y < Ymin + 0.5 * StepV || Local.Y > Ymax - 0.5 * StepV) continue; 
		if (Local.Y < 0 || Local.Y > Ymax) continue;
		y = OrigY - (int)((Local.Y - Ymin) * ScaleY);
		pDC->MoveTo(OrigX - 2, y); pDC->LineTo(OrigX + (int)((Xmax - Xmin) * ScaleX)+ 2, y); 
		S.Format("%g", Pgl);
		w = pDC->GetTextExtent(S).cx; h = pDC->GetTextExtent(S).cy / 2;
		pDC->TextOut(OrigX - w - 2, y - h, S); 
		pDC->TextOut(OrigX + (int)(Xmax * ScaleX)+ 5, y - h, S); 
	} // while

	pDC->SetBkColor(RGB(100,100,100));
	pDC->SetTextColor(RGB(255,255,255));
	//pDC->TextOut(OrigX + 2, OrigY - (int)(Ymax * ScaleY) +2, "BTR-ITER");
}

void CLoadView:: ShowPlateBound(CDC* pDC)
{
	CBTRDoc* pDoc = (CBTRDoc*)GetDocument();
	CPlate * Plate = pDoc->pMarkedPlate;
	C3Point Local, Global;
	//double MinX, MinY, MaxX. MaxY;
	pDC->SetROP2(R2_COPYPEN);
		
	CPlate * plate = Plate;
	//POSITION pos = pDoc->PlatesList.GetHeadPosition();
		//while (pos != NULL) {
			//plate = pDoc->PlatesList.GetNext(pos);
			if (plate->Selected) { //  && plate->Solid) { // dont show transparent
				Global = plate->MassCentre;
				//Local = Plate->GetLocal(Global);
				//if (!Plate->WithinPoly(Local)) continue;
				
				//if (fabs(Local.Z) > 5) continue; // too far

				Global = plate->Corn[0];
				Local = Plate->GetLocal(Global);
				//if (Local.X < 1.1* Xmin || Local.X > 1.1* Xmax) continue;
				//if (Local.Y < 1.1* Ymin || Local.Y > 1.1* Ymax) continue;
				
				pDC->MoveTo(OrigX + (int)(Local.X * ScaleX), OrigY - (int)(Local.Y * ScaleY));
				for (int k = 1; k < 4; k++) {
					Global = plate->Corn[k];
					Local = Plate->GetLocal(Global);
					//if (Local.X < 1.1* Xmin || Local.X > 1.1* Xmax) break;
					//if (Local.Y < 1.1* Ymin || Local.Y > 1.1* Ymax) break;
					pDC->LineTo(OrigX + (int)(Local.X * ScaleX), OrigY - (int)(Local.Y * ScaleY));
				} // k
				Global = plate->Corn[0];
				Local = Plate->GetLocal(Global);
				//if (Local.X < 1.1* Xmin || Local.X > 1.1* Xmax) continue;
				//if (Local.Y < 1.1* Ymin || Local.Y > 1.1* Ymax) continue;
				pDC->LineTo(OrigX + (int)(Local.X * ScaleX), OrigY - (int)(Local.Y * ScaleY));

			}// selected
		//}//pos
}

void CLoadView::ShowFWdata()
{
	CDC * pDC = GetDC();
	CBTRDoc* pDoc = (CBTRDoc*)GetDocument();
	CPlate * Plate = pDoc->pMarkedPlate;
	if (Plate->Number != 1000) return;
	
	CPen thickpen;
	thickpen.CreatePen(PS_DASH, 5, RGB(0, 250, 250));
	CPen * pOldPen = pDC->SelectObject(&thickpen);
	//pDC->SetROP2(R2_COPYPEN);

	C3Point Global = pDoc->FWdata[0];// R, Z, 0
	C3Point Local = Plate->GetLocal(Global);
	pDC->MoveTo(OrigX + (int)(Local.X * ScaleX), OrigY - (int)(Local.Y * ScaleY));
	for (int i = 1; i <= pDoc->FWdata.GetUpperBound(); i++) {
		Global = pDoc->FWdata[i]; // data cm->m 
		Local  = Plate->GetLocal(Global);
		pDC->LineTo(OrigX + (int)(Local.X * ScaleX), OrigY - (int)(Local.Y * ScaleY));
	}
	pDC->SelectObject(pOldPen);
	ReleaseDC(pDC);
}

/*
void CLoadView:: ShowGlobalScale(CDC* pDC)// old
{
	CBTRDoc* pDoc = (CBTRDoc*)GetDocument();
	CPlate * plate = pDoc->pMarkedPlate;
	C3Point Local, Global;
	pDC->SetROP2(R2_NOT);
	pDC->SelectObject(&smallfont);
	int x, y, w, h;
	CString S;
	double StepH;
	if (plate->Load->Xmax < 0.3) StepH = 0.05;
	if (plate->Load->Xmax < 0.1) StepH = 0.01;
	if (plate->Load->Xmax >= 0.3) StepH = 0.1;
	if (plate->Load->Xmax > 1) StepH = 0.2;
	if (plate->Load->Xmax > 2) StepH = 0.5;
	if (plate->Load->Xmax > 5) StepH = 1;
	double StepV;
	if (plate->Load->Ymax < 0.3) StepV = 0.05;
	if (plate->Load->Ymax < 0.1) StepV = 0.01;
	if (plate->Load->Ymax >= 0.3) StepV = 0.1;
	if (plate->Load->Ymax > 1) StepV = 0.2;
	if (plate->Load->Ymax > 2) StepV = 0.5;
	if (plate->Load->Ymax > 5) StepV = 1;
	double Pgl;
	
	Global = C3Point(0,0,0);
	Local = plate->GetLocal(Global);
	if (Local.X > 0 && Local.X < plate->Xmax){
		x = OrigX + (int)(Local.X * ScaleX);
		pDC->MoveTo(x, OrigY + 2); pDC->LineTo(x, OrigY - (int)(Ymax * ScaleY) - 2);
		pDC->TextOut(x, OrigY + 2, "0");
	}
	if (Local.Y > 0 && Local.Y < plate->Ymax){
		y = OrigY - (int)(Local.Y * ScaleY);
		pDC->MoveTo(OrigX - 2, y); pDC->LineTo(OrigX + (int)(Xmax * ScaleX)+ 2, y); 
		pDC->TextOut(OrigX - 12, y - 5, "0" ); 
		pDC->TextOut(OrigX + (int)(Xmax * ScaleX)+ 5, y - 5, "0"); 
	}
	
	if (plate->OrtX.X > 0.5) {
		Pgl = 0;
		pDC->TextOut(OrigX + (int)(Xmax * 0.6 * ScaleX), OrigY + 25, "global X");
	while (Pgl < *pDoc->AreaLong) {
		Pgl += StepH;
		Global.X = Pgl;
		Local = plate->GetLocal(Global);
		if (Local.X < 0 || Local.X > Xmax) continue; 
		x = OrigX + (int)(Local.X * ScaleX);
		//y = OrigY - (int)(Local.Y * ScaleY);
		pDC->MoveTo(x, OrigY + 2); pDC->LineTo(x, OrigY - (int)(Ymax * ScaleY) - 2);
		//pDC->MoveTo(OrigX - 2, y); pDC->LineTo(OrigX + (int)(Xmax * ScaleX)+ 2, y);
		S.Format("%g", Pgl);
		w = pDC->GetTextExtent(S).cx /2;
		pDC->TextOut(x-w, OrigY + 2, S);
	//	pDC->TextOut(x, OrigY - (int)(Ymax * ScaleY) - 20, S);
	} // while
	} // if
	else { //OrtX.X <= 0.5
		Pgl = 0;
		pDC->TextOut(OrigX + (int)(Xmax * 0.6 * ScaleX), OrigY + 25, "global Y");
		while (Pgl < *pDoc->AreaHorMax) {
		Pgl += StepH;
		Global.Y = Pgl;
		Local = plate->GetLocal(Global);
		if (Local.X < 0 || Local.X > Xmax) continue; 
		x = OrigX + (int)(Local.X * ScaleX);
		//y = OrigY - (int)(Local.Y * ScaleY);
		pDC->MoveTo(x, OrigY + 2); pDC->LineTo(x, OrigY - (int)(Ymax * ScaleY) - 2);
		//pDC->MoveTo(OrigX - 2, y); pDC->LineTo(OrigX + (int)(Xmax * ScaleX)+ 2, y); 
		S.Format("%g", Pgl);
		w = pDC->GetTextExtent(S).cx /2;
		pDC->TextOut(x-w, OrigY + 2, S);
	//	pDC->TextOut(x, OrigY - (int)(Ymax * ScaleY) - 20, S);
		} // while

		Pgl = 0;
		while (Pgl > *pDoc->AreaHorMin) {
		Pgl -= StepH;
		Global.Y = Pgl;
		Local = plate->GetLocal(Global);
		if (Local.X < 0 || Local.X > Xmax) continue; 
		x = OrigX + (int)(Local.X * ScaleX);
		//y = OrigY - (int)(Local.Y * ScaleY);
		pDC->MoveTo(x, OrigY + 2); pDC->LineTo(x, OrigY - (int)(Ymax * ScaleY) - 2);
		//pDC->MoveTo(OrigX - 2, y); pDC->LineTo(OrigX + (int)(Xmax * ScaleX)+ 2, y); 
		S.Format("%g", Pgl);
		w = pDC->GetTextExtent(S).cx /2;
		pDC->TextOut(x - w, OrigY + 2, S);
	//	pDC->TextOut(x, OrigY - (int)(Ymax * ScaleY) - 20, S);
		} // while
	}// else //OrtX.X <= 0.5

	if (plate->OrtY.Y > 0.5) {
		Pgl = 0;
		while (Pgl < *pDoc->AreaHorMax) {
		Pgl += StepV;
		Global.Y = Pgl;
		Local = plate->GetLocal(Global);
		if (Local.Y < 0 || Local.Y > Ymax) continue; 
		//x = OrigX + (int)(Local.X * ScaleX);
		y = OrigY - (int)(Local.Y * ScaleY);
		//pDC->MoveTo(x, OrigY + 2); pDC->LineTo(x, OrigY - (int)(Ymax * ScaleY) - 2);
		pDC->MoveTo(OrigX - 2, y); pDC->LineTo(OrigX + (int)(Xmax * ScaleX)+ 2, y); 
		S.Format("%g", Pgl);
		w = pDC->GetTextExtent(S).cx; h = pDC->GetTextExtent(S).cy / 2;
		pDC->TextOut(OrigX - w - 2, y - h, S); 
		pDC->TextOut(OrigX + (int)(Xmax * ScaleX)+ 5, y - h, S); 
		} // while
		pDC->TextOut(OrigX - 10, OrigY - (int)(Ymax * ScaleY) - 15, "Y"); 

		Pgl = 0;
		while (Pgl > *pDoc->AreaHorMin) {
		Pgl -= StepV;
		Global.Y = Pgl;
		Local = plate->GetLocal(Global);
		if (Local.Y < 0 || Local.Y > Ymax) continue; 
		//x = OrigX + (int)(Local.X * ScaleX);
		y = OrigY - (int)(Local.Y * ScaleY);
		//pDC->MoveTo(x, OrigY + 2); pDC->LineTo(x, OrigY - (int)(Ymax * ScaleY) - 2);
		pDC->MoveTo(OrigX - 2, y); pDC->LineTo(OrigX + (int)(Xmax * ScaleX)+ 2, y); 
		S.Format("%g", Pgl);
		w = pDC->GetTextExtent(S).cx; h = pDC->GetTextExtent(S).cy / 2;
		pDC->TextOut(OrigX - w -2, y - h, S); 
		pDC->TextOut(OrigX + (int)(Xmax * ScaleX)+ 5, y - h, S);
		} // while
	} // if

	else { // OrtY.Y < 0.5
		Pgl = 0;
		while (Pgl < *pDoc->AreaVertMax) {
		Pgl += StepV;
		Global.Z = Pgl;
		Local = plate->GetLocal(Global);
		if (Local.Y < 0 || Local.Y > Ymax) continue; 
		//x = OrigX + (int)(Local.X * ScaleX);
		y = OrigY - (int)(Local.Y * ScaleY);
		//pDC->MoveTo(x, OrigY + 2); pDC->LineTo(x, OrigY - (int)(Ymax * ScaleY) - 2);
		pDC->MoveTo(OrigX - 2, y); pDC->LineTo(OrigX + (int)(Xmax * ScaleX)+ 2, y); 
		S.Format("%g", Pgl);
		w = pDC->GetTextExtent(S).cx; h = pDC->GetTextExtent(S).cy / 2;
		pDC->TextOut(OrigX - w -2, y - h, S); 
		pDC->TextOut(OrigX + (int)(Xmax * ScaleX)+ 5, y - h, S); 
		} // while
		pDC->TextOut(OrigX - 10, OrigY - (int)(Ymax * ScaleY) - 15, "Z"); 

		Pgl = 0;
		while (Pgl > *pDoc->AreaVertMin) {
		Pgl -= StepV;
		Global.Z = Pgl;
		Local = plate->GetLocal(Global);
		if (Local.Y < 0 || Local.Y > Ymax) continue; 
		//x = OrigX + (int)(Local.X * ScaleX);
		y = OrigY - (int)(Local.Y * ScaleY);
		//pDC->MoveTo(x, OrigY + 2); pDC->LineTo(x, OrigY - (int)(Ymax * ScaleY) - 2);
		pDC->MoveTo(OrigX - 2, y); pDC->LineTo(OrigX + (int)(Xmax * ScaleX)+ 2, y); 
		S.Format("%g", Pgl);
		w = pDC->GetTextExtent(S).cx; h = pDC->GetTextExtent(S).cy / 2;
		pDC->TextOut(OrigX - w -2, y - h, S); 
		pDC->TextOut(OrigX + (int)(Xmax * ScaleX)+ 5, y - h, S);
		} // while
	} // else
	
}*/