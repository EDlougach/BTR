#include "stdafx.h"
#include "MainView.h"
#include "LoadView.h"
#include "SetView.h"
#include "config.h"
#include <math.h>
#include "BTRDoc.h"
//#include <iostream>
#include <algorithm>
#include <vector>
#include <cmath>
using namespace std;


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

typedef pair<double, double> pdd; // x, y

//vector<pdd> points;
 vector <tDistInfo> dists;

 class C3Point;
CLoadView * pLoadView;
CMainView  * pMainView;
CSetView * pSetView;
void ShowNothing();

extern double DuctExitYmin, DuctExitYmax, DuctExitZmin, DuctExitZmax;
extern double LevelStep, LevelMin, LevelMax;
extern BOOL LevelWrite;

COLORREF Color[11];


//--------------------------------------------------------------------
//  class C3Point //  3-D Point
//-----------------------------------------------------
C3Point:: C3Point()
{
	X = 0; Y = 0; Z = 0;
}

C3Point:: C3Point(double x, double y, double z)
{
	X = x; Y = y;  Z = z;
}
void C3Point::operator = (double  a)
{
	 X = a; Y = a;  Z = a;
}
C3Point  C3Point::operator * (double  a) const
{
	//C3Point P;	 P.X = X*a; P.Y = Y*a; P.Z = Z*a;
	 return C3Point(X*a, Y*a, Z*a);
}
C3Point   C3Point::operator / (double  a) const
{
	//C3Point P;	 P.X = X/a; P.Y = Y/a; P.Z = Z/a;
	 return C3Point(X/a, Y/a, Z/a);
}
void C3Point::operator = (const C3Point & P)
{
	 X = P.X;  Y = P.Y;  Z = P.Z;
}
BOOL C3Point::operator < (const C3Point & P)
{
	if (X < P.X) return TRUE;//  Y = P.Y;  Z = P.Z;
	else if (X == P.X && Y < P.Y) return TRUE;
	else if (X == P.X && Y == P.Y && Z < P.Z) return TRUE;
	else return FALSE;
}
C3Point  C3Point::operator + (const C3Point & P) const
{
	//C3Point P1;
	 //P1.X = X + P.X;  P1.Y = Y + P.Y;   P1.Z = Z + P.Z;
	 return C3Point(X + P.X, Y + P.Y, Z + P.Z);
}
/*C3Point  C3Point::operator + (const C3Point & P1, const C3Point & P2) const
{
	return C3Point(P1.X + P2.X, P1.Y + P2.Y, P1.Z + P2.Z);
}*/
C3Point  C3Point::operator - (const C3Point & P) const
{
	//C3Point P1;
	// P1.X = X - P.X;  P1.Y = ;   P1.Z = Z - P.Z;
	 return C3Point(X - P.X, Y - P.Y, Z - P.Z);
}
/*C3Point  C3Point::operator - (const C3Point & P1, const C3Point & P2) const
{
	return C3Point(P1.X - P2.X, P1.Y - P2.Y, P1.Z - P2.Z);
}*/

double   C3Point::operator *(const C3Point &P) const //scal product
{
	return X * P.X + Y * P.Y + Z * P.Z;
}

/*C3Point  C3Point::operator %(const C3Point &P) const
{
	return C3Point(
		Y * P.Z - Z * P.Y,
		Z * P.X - X * P.Z,
		X * P.Y - Y * P.X);
}*/

//---------------------------------------------------------
// STL example VECTOR
//---------------------------------------------------------

bool operator<(const tDistInfo &p1, const tDistInfo &p2)
{
	return (p1.d<p2.d);
}

//int N;

double dist(double x1, double y1, double x2, double y2)
{
	return sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));
}

void printInfo(tDistInfo &p)
{
	cout << p.n1 << " " << p.n2 << " " << p.d << endl;
}

vector <tDistInfo> & ConnectPoints(vector <pdd> &points)//returns sorted vector of distances
{
	dists.clear();
	int N = points.size();
	//cin >> N;
/*	for (int i = 0; i < N; i++)
	{
		pdd cur;
		cin >> cur.first >> cur.second;
		points.push_back(cur);
	}*/
	for (int i = 0; i < N-1; i++)
	{
		for (int j = i+1; j < N; j++)
		{
			tDistInfo di(i, j, dist(points[i].first, points[i].second, points[j].first, points[j].second));
			dists.push_back(di);
		}
	}
	sort(dists.begin(), dists.end());
//	for_each(dists.begin(), dists.end(), printInfo);
	return (dists);
}

void WriteLevel(CPlate * plate, double level, std::vector <pdd> & points)
{
	CString  sn, name;
	FILE * fout;
	double x, y;
	C3Point Ploc, Pgl;
	sn.Format("%g", level);
	
	name = "level" + sn + ".txt"; 
	fout = fopen(name, "w");
	fprintf(fout," %g MW/m2 contour \n\n", level);
	fprintf(fout," X,m  \t  Y,m  \t   Z,m  \n"); 
	fprintf(fout, "-------------------------------------------\n");
		
	for (int k = 0; k < (int)points.size(); k++) {
		Ploc.X = points[k].first;
		Ploc.Y = points[k].second;
		Ploc.Z = 0;
		Pgl = plate->GetGlobalPoint(Ploc);
		if (Pgl.X > 999) Pgl.X = 0;
		
		fprintf(fout, "  %4.3f  %4.3f  %4.3f \n", Pgl.X, Pgl.Y, Pgl.Z);
	}
	fclose(fout);
	
}

// --------------------------------------------------------
// Class CLoad
//--------------------------------------------------------- 
CLoad ::CLoad()
{
	Nx = 0;
	Ny = 0;
	Comment = "";
	Sum = 0;
	MaxVal = 0;
	
}

void CLoad:: SetGridOptimal(void)
{
	/*double hx, hy;
	hx = hy = 0.01;
	if (target) {
		if (dist > 10)  hx = hy = 0.02; 
		if (dist > 15)  hx = hy = 0.05;
	}
	else {
		hx = 0.05; hy = 0.02;
		if (dist > 10) hy = 0.05;
		
	}

	StepX = hx;
	StepY = hy;*/
	C3Point H;
	
	H = GetMantOrder(Xmax * 0.05);
	StepX = H.X * H.Z;

	H = GetMantOrder(Ymax * 0.05);
	StepY = H.X * H.Z;

	Nx = (int) ceil(Xmax / StepX);
	Ny = (int) ceil(Ymax / StepY);

	if (Nx < 10) {
		StepX *= 0.1; 
		Nx = (int) floor(Xmax / StepX);
	}
	if (Ny < 10) {
		StepY *= 0.1;
		Ny = (int) floor(Ymax / StepY);
	}
	
}

CLoad ::CLoad(double xmax, double ymax)
{
	Xmax = xmax;
	Ymax = ymax;
	SetGridOptimal();

	int i;
	Val = new double * [Nx+1];
	for (i = 0; i <= Nx; i++) {
		Val[i] = new double [Ny+1];
		for (int j = 0; j <=Ny; j++) Val[i][j] = 0;
	}
	Sum = 0;
	MaxVal = 0;
	/*
	wh = new double * [Nx+1];
	wv = new double * [Nx+1];
	for (i = 0;  i <= Nx; i++) {
		wh[i] = new double [Ny+1];
		wv[i] = new double [Ny+1];
	} // i
*/
	Clear();
	SmoothDegree = 0;
	SetCrosses();
	Comment = "";
	Particles = "ALL";
	
}

CLoad ::CLoad(double xmax, double ymax, double stepx, double stepy)
{
	Xmax = xmax;
	Ymax = ymax;
	StepX = stepx;
	StepY = stepy;
	Nx = (int)(Xmax / StepX);
	Ny = (int)(Ymax / StepY);
	if (Xmax - Nx*StepX > 0.1 * StepX) Nx++;
	if (Ymax - Ny*StepY > 0.1 * StepY) Ny++;
		
	int i;
	Val = new double * [Nx+1];
	for (i = 0; i <= Nx; i++) {
		Val[i] = new double [Ny+1];
		for (int j = 0; j <=Ny; j++) Val[i][j] = 0;
	}
	Sum = 0;
	MaxVal = 0;
	/*wh = new double * [Nx+1];
	wv = new double * [Nx+1];
	for (i = 0;  i <= Nx; i++) {
		wh[i] = new double [Ny+1];
		wv[i] = new double [Ny+1];
	} // i*/

	Clear();
	SmoothDegree = 0;
	
	SetCrosses();
	Comment = "";
	Particles = "ALL";
	
}

void CLoad:: SetCrosses()
{ 
	double nx, ny;
	
	double hx = 1;
	double hy = 1;
	double crossX = Xmax/5;
	double crossY = Ymax/5;
	BOOL stop = FALSE;
	do {
		nx = (int)(crossX/hx);
		ny = (int)(crossY/hy);
		if (nx < 1) hx /= 10;
		if (ny < 1) hy /= 10;
		if (nx > 2) hx *= 2;
		if (ny > 2) hy *= 2;
		if (nx>=1 && nx<=2 && ny>=1 && ny<=2) stop = TRUE;
	} while (!stop);

	crossX = hx*nx;
	crossY = hy*ny;

	CrossX = crossX;
	CrossY = crossY;
}

void CLoad:: Clear()
{
	Sum = 0;
	MaxVal = 0;
	if (Nx == 0&& Ny == 0)  return; 
	for (int i = 0; i <= Nx; i++) {
		for (int j = 0; j <= Ny; j++) Val[i][j] = 0;
	}
		
}

void CLoad:: SetSumMax()
{
//	Sum = 0;
	double sum = 0;
	MaxVal = 0;
	iMaxBound = 0;
	iMaxVal = 0; 
	jMaxVal = 0;
	iProf =  0;
	jProf =  0;
	double val;
	double cell = StepX * StepY;
	int i, j;
	if (Nx < 2 && Ny < 2)  return; 

	for (i = 0; i <= Nx; i++) {
		for (j = 0; j <= Ny; j++) {
				
			val = Val[i][j]; // density
			sum += val * cell;// * StepX * StepY; // power
			if (val >= MaxVal) {
				MaxVal = val;
				iMaxVal = i; iProf =  i;
				jMaxVal = j; jProf =  j;
				
			}
			
		}
	}
	if (MaxVal < 1.e-16) { SetProf(1000,1000); return;} 

	for (i = 0; i <= Nx; i++) { // define map "bound"
		for (j = 0; j <= Ny; j++) {
			val = Val[i][j]; // density
			if (val > 0.01 * MaxVal) iMaxBound = Max(i, iMaxBound);
		}
	}

	if (sum > 1.e-16) Sum = sum;
	
	double nz;
	double hz = 1.e6;
	double crossZ = MaxVal / 5;
	BOOL stop = FALSE;
	do {
		nz = (int)(crossZ/hz);
		if (nz < 1) hz /= 10;
		if (nz > 2) hz *= 2;
		if (nz>=1 && nz<=2) stop = TRUE;
	} while (!stop);

	crossZ = hz*nz;
	CrossZ = crossZ;
}

double CLoad:: GetVal(double x, double y)
{
	double val = 0, val1, val2, val3, dx, dy, dx1, dy1;
	int i = (int)floor(x/StepX);
	int j = (int)floor(y/StepY);
	if (i < 0 || i > Nx || j < 0 || j > Ny) return val;
	if (i == Nx && j == Ny) return Val[Nx][Ny]; 
	if (i == Nx && j < Ny) {
		dy = y - j*StepY;
		val = Val[Nx][j] + dy/StepY*(Val[Nx][j+1] - Val[Nx][j]);

		return val;
	}
	if (j == Ny && i < Nx) {
		dx = x - i*StepX;
		val = Val[i][Ny] + dx/StepX*(Val[i+1][Ny] - Val[i][Ny]);
		return val;
	}
	if (i < Nx && j < Ny) {
		dx = x - i*StepX;
		if (dx >= StepX) { i++; dx = x - i*StepX; }
		dy = y - j*StepY;
		if (dy >= StepY) { j++; dy = y - j*StepY; }
		if (dx < 1.e-6) dx = 1.e-6;
		if (dy < 1.e-6) dy = 1.e-6;

		if (dy >= dx*StepY/StepX) { // upper-left triangle
			dx1 = dx*StepY/dy;
			val1 = Val[i][j+1] + dx1/StepX*(Val[i+1][j+1] - Val[i][j+1]);
			val2 = Val[i][j] + dy/StepY * (val1 - Val[i][j]);
		//	return val;
		}
		else { // lower-right triangle
			dy1 = dy*StepX/dx;
			val1 = Val[i+1][j] + dy1/StepY*(Val[i+1][j+1] - Val[i+1][j]);
			val2 = Val[i][j] +  dx/StepX * (val1 - Val[i][j]);
		//	return val;
		}

		if (dy <= (StepX-dx)*StepY/StepX) { // lower-left triangle
			dx1 = dx*StepY/(StepY - dy);
			val1 = Val[i][j] + dx1/StepX*(Val[i+1][j] - Val[i][j]);
			val3 = val1 + dy/StepY * (Val[i][j+1] - val1);
		//	return val;
		}
		else { // upper-right triangle
			dy1 = StepY - (StepY - dy) * StepX/dx;
			val1 = Val[i+1][j] + dy1/StepY*(Val[i+1][j+1] - Val[i+1][j]);
			val3 = Val[i][j+1] +  dx/StepX * (val1 - Val[i][j+1]);
		//	return val;
		}

		return (val2 + val3)*0.5;
	} // i<Nx j<Ny
	else return 0;

}

C3Point CLoad:: GetValGrad(double x, double y)
{
	C3Point Grad;
	double dx = StepX / 10; 
	double dy = StepY / 10; 
	Grad.Z = 0;
	Grad.X = (GetVal(x+dx,y) - GetVal(x,y))/dx;
	Grad.Y = (GetVal(x,y+dy) - GetVal(x,y))/dy;
	return Grad;

}

void  CLoad:: SetProf(double x, double y)
{
	if (x <= 0 || x >= Xmax || y <= 0 || y >= Ymax) {
		iProf = -1; jProf = -1;
		/*iProf = (Nx+1)/2; 
		jProf = (Ny+1)/2;
		iMaxVal = 0;//iProf;
		jMaxVal = 0;//jProf;*/
		return;
	}
	else {
		iProf = (int)(x/StepX);
		if (fabs(x - iProf*StepX) > 0.5*StepX) iProf++;
		jProf = (int)(y/StepY);
		if (fabs(y - jProf*StepY) > 0.5*StepY) jProf++;
	}

}


CLoad :: ~CLoad()
{
	int i;
	if (Nx > 0 || Ny > 0) {
		for (i = 0; i <= Nx; i++ ) 	delete Val[i];
		delete [] Val;
	}
		
	/*	for (i = 0;  i <=  Nx; i++) {
			delete [] wh[i];
			delete [] wv[i];
		} // i
		delete [] wh;
		delete [] wv;
	}*/
	//if (Nx > 0 || Ny > 0) delete [] Val;
	//delete this;

}

void CLoad::operator = (double ** fromarr)
{
	for (int i = 0; i <= Nx; i++) {
		for (int j = 0; j <= Ny; j++) {
			Val[i][j] = fromarr[i][j]; // density
		}
	}
}

void CLoad::SetValArray(double ** arr, int nx, int ny)
{
	if (Nx != nx || Ny != ny) return;
	for (int i = 0; i <= Nx; i++) {
		for (int j = 0; j <= Ny; j++) {
			Val[i][j] = arr[i][j]; // density
		}
	}
}

CLoad * CLoad:: Smoothed(int smdegree) //(CLoad * load, int smdegree)
{
	CLoad * pLoad = new CLoad(Xmax, Ymax, StepX, StepY);
	//pLoad->Nx = load->Nx;	pLoad->Ny = load->Ny;
	for (int i = 0; i <= Nx; i++) {
		for (int j = 0; j <= Ny; j++) {
			pLoad->Val[i][j] = Val[i][j]; // density
		}
	}
	
	if (smdegree > 0) pLoad->SmoothLoad(smdegree);
	else pLoad->SmoothDegree = 0;
	pLoad->SetSumMax();
	return pLoad;
}

void CLoad:: Copy(CLoad * load)
{
	//CLoad * pLoad = new CLoad(Xmax, Ymax, StepX, StepY);
	for (int i = 0; i <= Nx; i++) {
		for (int j = 0; j <= Ny; j++) {
			Val[i][j] = load->Val[i][j]; // density
		}
	}
	Sum = load->Sum;
	MaxVal = load->MaxVal;
//	pLoad->SetSumMax();
}

void CLoad:: Distribute(double xloc, double yloc, double power)  //
{
	//CCriticalSection cs;
	if (Nx * Ny < 2) { // NO MAP
		//Sum += power;	MaxVal = 0;
		return;
	}

	double x, y, p, dp;

	if (xloc < -0.0001 || xloc >= Xmax + 0.0001 || yloc < -0.0001 || yloc > Ymax + 0.0001) 
		return;// out of bound 
	//on bound
	if (xloc < 0) xloc = 0;
	if (xloc > Xmax) xloc = Xmax;
	if (yloc < 0) yloc = 0;
	if (yloc > Ymax) yloc = Ymax;

	int i = (int)floor(xloc/StepX);
	int j = (int)floor(yloc/StepY);
	if (i<0 || j<0 || i>Nx || j>Ny) {
		AfxMessageBox("Error in Load->Distribute"); return;
	}
	double cell = StepX * StepY;// cell square

	double a = xloc - i * StepX;
	double b = StepX - a;
	double c = yloc - j * StepY;
	double d = StepY - c;
	
	if (i < Nx) {
		if (j < Ny) { // common - inner - case
			Val[i][j] += power * b*d / cell / cell;    
			Val[i+1][j] += power * a*d / cell / cell;
			Val[i][j+1] += power * b*c / cell / cell; 
			Val[i+1][j+1] += power * a*c / cell / cell;
		}
		else { // j = Ny  upper bound 
			Val[i][j] += power * b / StepX / cell;    
			Val[i+1][j] += power * a / StepX / cell;
		}// j = Ny
	}// i < Nx          //a+b = StepX  c+d = StepY
				
	else { // i = Nx right bound
		if (j < Ny) { // right bound
			Val[i][j] += power * d / StepY / cell;  
			Val[i][j+1] += power * c / StepY / cell; 
		}
		else { // j = Nx right-upper corner
			Val[i+1][j+1] += power / cell / cell;
		}
	} // i = Nx

	Sum += power;
	return;

}


void  CLoad :: DrawEmptyLoad() //not used
{
	CLoadView * pLV = (CLoadView *) pLoadView;
	CDC* pDC = pLV->GetDC();
	int xmin = pLV->OrigX;
	int ymin = pLV->OrigY;
	int xmax = xmin +  (int)(Xmax * pLV->ScaleX);
	int ymax = ymin -(int)(Ymax * pLV->ScaleY);

	CBrush brush;
	CBrush * oldbrush;
	brush.CreateSolidBrush(RGB(245,245,245)); 
	oldbrush = pDC->SelectObject(&brush);
	pDC->Rectangle(xmin, ymin, xmax, ymax);
	pDC->SelectObject(oldbrush);

	CFont* pOldFont = pDC->SelectObject(&pLV->bigfont);
	//int w = pDC->GetTextExtent(S).cx;
	pDC->TextOut(xmin +10, 10, Comment + "  Particles spots");
	pDC->SelectObject(pOldFont);
	
	pLV->ReleaseDC(pDC);
}

void  CLoad :: DrawLoad(CView * pView, CDC* pDC) //not used
{
	AfxMessageBox("DrawLoad");
	MSG message;
	CLoadView * pLV = (CLoadView *) pView;
	pLV->STOP = FALSE;
	CFont* pOldFont = pDC->SelectObject(&pLV->smallfont);
	pDC->SetTextColor(RGB(0, 0, 0));
//	pDC->SetBkMode(0);
	int ix, jy, xProf, yProf;
	int xmin = pLV->OrigX;
	int ymin = pLV->OrigY;
	int xmax = xmin +  (int)(Xmax * pLV->ScaleX);
	int ymax = ymin -(int)(Ymax * pLV->ScaleY);
	CBrush brush;
	CBrush * oldbrush;
	if (pLV->Contours) brush.CreateSolidBrush(RGB(255,255,220)); 
	else brush.CreateSolidBrush(RGB(230,230,190)); //- haki//(RGB(255,255,220)) - yellow
	oldbrush = pDC->SelectObject(&brush);
	pDC->Rectangle(xmin, ymin, xmax, ymax);
	pDC->SelectObject(oldbrush);
	CString S;
	
	// int icross = (int)floor(Xmax / CrossX);
	// int jcross = (int)floor(Ymax / CrossY);
	
	double ratio = 0;
	int i, j;
	double x, y;
	COLORREF color;
	
	int ii, jj, Nxx, Nyy;
	double hx, hy;
	hx = StepX/5; hy = StepY/5;

	Nxx = (int)floor(Xmax / hx); 
	Nyy = (int)floor(Ymax / hy);

	int rx = (int)(hx * pLV->ScaleX / 2)+1;//  = (int)(StepX *  pLV->ScaleX / 2)+1;
	if (rx<1) rx =1;
	int ry = (int)(hy * pLV->ScaleY / 2)+1;//  = (int)(StepY *  pLV->ScaleY / 2)+1;
	if (ry<1) ry = 1;
	
//	if (MaxVal >1.e-10 && !pLV->Contours) {
	if (!pLV->Contours) {
	for ( ii = 0; ii < Nxx; ii++) {
		for ( jj = 0; jj < Nyy; jj++) {
			x = (ii+0.5)*hx; y = (jj+0.5)*hy;
			ix = xmin + (int)(x * pLV->ScaleX);
			jy = ymin - (int)(y * pLV->ScaleY);
			if (MaxVal >1.e-10)	ratio = GetVal(x,y) / MaxVal;// Val[i][j] / MaxVal;
			else ratio = 1;
	//		color = RGB(Red(ratio), Green(ratio), Blue(ratio));
	//		color = RGB(Gray(ratio), Gray(ratio), Gray(ratio));
			color = GetColor10(ratio);
			//if (ratio > 1.e-10 && ratio <= 1 )
			if (ratio <= 1 )
			DrawLoadPoint(ix, jy, rx, ry, color,  pDC);
			
			if (::PeekMessage(&message, NULL, 0,0, PM_REMOVE)) {
				::TranslateMessage(&message);
				::DispatchMessage(&message);
			}
			if (pLV->STOP) return;
		} // j
	} // i
	} //if (MaxVal >1.e-10)

	CPen BlackPen;
	BlackPen.CreatePen(PS_SOLID, 1, RGB(0,0,0));
	CPen * pOldPen = pDC->SelectObject(&BlackPen);
//	pDC->SetROP2(R2_NOT); 

	ix = xmin + (int)(iMaxVal*StepX*pLV->ScaleX);
	jy = ymin - (int)(jMaxVal*StepY*pLV->ScaleY);
	// int r = (int) ((rx+ry) *0.5);
	pDC->Ellipse(ix-5, jy-5, ix+5, jy+5);

//	xProf = xmin + (int)(iProf*StepX*pLV->ScaleX);
//	yProf = ymin - (int)(jProf*StepY*pLV->ScaleY);

	xProf = xmin + (int)(pLV->Cross.X * pLV->ScaleX);
	yProf = ymin - (int)(pLV->Cross.Y * pLV->ScaleY);

	CPen Pen;
	Pen.CreatePen(PS_DASHDOT, 1, RGB(0,0,0));
	pDC->SelectObject(&Pen);
	pDC->SetROP2(R2_COPYPEN);
	
	//pDC->SetROP2(R2_NOT); 
	pDC->MoveTo(xmin, yProf); pDC->LineTo(xmax, yProf);
	pDC->MoveTo(xProf, ymin); pDC->LineTo(xProf, ymax);
	
	//pDC->SelectObject(&pLV->font);
	pDC->SetTextColor(RGB(0,0,0));
	S.Format("Total = %12.3le W", Sum);
	pDC->TextOut(xmin, ymin +20, S);
	S.Format("Max = %12.3le W/m2", MaxVal);
	pDC->TextOut(xmin, ymin +35, S);
//	pDC->SelectObject(pOldFont);
	pDC->SelectObject(pOldPen);
	
	if (pLV->Contours) 
	SetLevels(pLV, pDC, 0,0,0,0);

	pDC->SelectObject(&pLV->bigfont);
	//int w = pDC->GetTextExtent(S).cx;
	pDC->TextOut(xmin +10, 10, Comment);
	pDC->SelectObject(pOldFont);

}

void CLoad:: DrawContours(double left, double right, double bot, double top)
{
	CLoadView * pLV = (CLoadView *)pLoadView;
	CDC* pDC = pLV->GetDC();
	MSG message;
	pLV->STOP = FALSE;// stopped by left mouse click

	CFont* pOldFont = pDC->SelectObject(&pLV->smallfont);
	pDC->SetTextColor(RGB(0, 0, 0));
	
	CPen BlackPen;
	BlackPen.CreatePen(PS_SOLID, 1, RGB(0,0,0));
	CPen * pOldPen = pDC->SelectObject(&BlackPen);
//	pDC->SetROP2(R2_NOT); 
	CBrush brush;
	CBrush * oldbrush;
	//if (pLV->Contours) 
	brush.CreateSolidBrush(RGB(255,255,220)); 
	//else brush.CreateSolidBrush(RGB(230,230,190)); //- haki//(RGB(255,255,220)) - yellow
	oldbrush = pDC->SelectObject(&brush);
	pDC->Rectangle(pLV->LoadRect);
	pDC->SelectObject(oldbrush);
	CString S;
	int ix, jy, xProf, yProf;
	int xmin = pLV->OrigX;
	int ymin = pLV->OrigY;
	int xmax = xmin +  (int)(Xmax * pLV->ScaleX);
	int ymax = ymin -(int)(Ymax * pLV->ScaleY);
	//pDC->Rectangle(xmin, ymin, xmax, ymax);
	
	ix = xmin + (int)(iMaxVal*StepX*pLV->ScaleX);
	jy = ymin - (int)(jMaxVal*StepY*pLV->ScaleY);
	// int r = (int) ((rx+ry) *0.5);
	pDC->Ellipse(ix-2, jy-2, ix+2, jy+2);

//	xProf = xmin + (int)(iProf*StepX*pLV->ScaleX);
//	yProf = ymin - (int)(jProf*StepY*pLV->ScaleY);

	xProf = xmin + (int)(pLV->Cross.X * pLV->ScaleX);
	yProf = ymin - (int)(pLV->Cross.Y * pLV->ScaleY);

	CPen Pen;
	Pen.CreatePen(PS_DASHDOT, 1, RGB(0,0,0));
	pDC->SelectObject(&Pen);
	pDC->SetROP2(R2_COPYPEN);
	
	//pDC->SetROP2(R2_NOT); 
	pDC->MoveTo(xmin, yProf); pDC->LineTo(xmax, yProf);
	pDC->MoveTo(xProf, ymin); pDC->LineTo(xProf, ymax);
	
	//pDC->SelectObject(&pLV->font);
	SetLevels(pLV, pDC, left, bot, right, top);
	
	pDC->SetTextColor(RGB(0,0,0));
	S.Format("Total = %12.3le W", Sum);
	pDC->TextOut(xmin, ymin +30, S);
	S.Format("Max = %12.3le W/m2", MaxVal);
	pDC->TextOut(xmin, ymin +50, S);
//	pDC->SelectObject(pOldFont);
		
	//if (pLV->Contours) 
	pDC->SelectObject(&pLV->bigfont);
	//int w = pDC->GetTextExtent(S).cx;
	pDC->TextOut(xmin +10, 10, Comment);
	//S.Format("%d", pLV->Plate->Number);
	//pDC->TextOut(xmin + 10, 10, S + ": " + pLV->Plate->Comment);
	pDC->SelectObject(pOldFont);
	pDC->SelectObject(pOldPen);
	pLV->ReleaseDC(pDC);
}

void  CLoad::DrawMapLimited(double left, double right, double bot, double top)//NEW
{
	CLoadView * pLV = (CLoadView *)pLoadView;
	CDC* pDC = pLV->GetDC();
	MSG message;
	pLV->STOP = FALSE;// stopped by left mouse click 
	CFont* pOldFont = pDC->SelectObject(&pLV->smallfont);
	pDC->SetTextColor(RGB(0, 0, 0));
	
	int origX = pLV->OrigX;
	int origY = pLV->OrigY;
	double DimX = right - left;
	double DimY = top - bot;
	//int xmax = xmin + (int)(Xmax * pLV->ScaleX);
	//int ymax = ymin - (int)(Ymax * pLV->ScaleY);

	CBrush brush;
	CBrush * oldbrush;
	brush.CreateSolidBrush(RGB(245, 245, 245));
	oldbrush = pDC->SelectObject(&brush);
	//pDC->Rectangle(xmin, ymin, xmax, ymax);
	pDC->Rectangle(pLV->LoadRect);
	pDC->SelectObject(oldbrush);
	//CFont* pOldFont = pDC->SelectObject(&pLV->bigfont);
	//int w = pDC->GetTextExtent(S).cx;
	//pDC->TextOut(origX + 10, 10, Comment + " MAP");
	//pDC->SelectObject(pOldFont);

	double Val, ratio = 0;
	//int i, j, k;
	double x, y;
	COLORREF color;
	CString S;

	int ii, jj, Nxx, Nyy, ix, jy;
	double coef = 0.2; // refine load steps 
	if (Nx < 100) coef = 0.1;
	double hx = StepX * coef;
	coef = 0.2;
	if (Ny < 100) coef = 0.1;
	double hy = StepY * coef; 

	Nxx = (int)floor(DimX / hx);
	Nyy = (int)floor(DimY / hy);

	int rx = (int)(hx * pLV->ScaleX / 2) + 2;//  = (int)(StepX *  pLV->ScaleX / 2)+1;
	if (rx<1) rx = 1; // fill point 'radius'
	int ry = (int)(hy * pLV->ScaleY / 2) + 2;//  = (int)(StepY *  pLV->ScaleY / 2)+1;
	if (ry<1) ry = 1; // fill point 'radius'

	for (ii = 0; ii < Nxx; ii++) {
		x = left + (ii + 0.5)*hx;
			for (jj = 0; jj < Nyy; jj++) {
				y = bot + (jj + 0.5)*hy;
				ix = origX + (int)((x - left) * pLV->ScaleX);
				jy = origY - (int)((y - bot) * pLV->ScaleY);
				if (MaxVal >1.e-10)	
					ratio = GetVal(x, y) / MaxVal;// Val[i][j] / MaxVal;
				else ratio = 1; // too small
				
				color = GetColor10(ratio);
				
				//		color = RGB(Red(ratio), Green(ratio), Blue(ratio));
				//		color = RGB(Gray(ratio), Gray(ratio), Gray(ratio));

				if (ratio <= 1)
					DrawLoadPoint(ix, jy, rx, ry, color, pDC);


				if (::PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) {
					::TranslateMessage(&message);
					::DispatchMessage(&message);
				}
				if (pLV->STOP) return; // left mouse clicked
				
			} // jj
		} // ii

	CPen BlackPen;
	BlackPen.CreatePen(PS_SOLID, 1, RGB(0,0,0));
	CPen * pOldPen = pDC->SelectObject(&BlackPen);
	ix = origX + (int)((iMaxVal*StepX - left) * pLV->ScaleX);
	jy = origY - (int)((jMaxVal*StepY - bot) * pLV->ScaleY);
	// int r = (int) ((rx+ry) *0.5);
	pDC->Rectangle(ix-3, jy-3, ix+3, jy+3);

	//pDC->SetTextColor(RGB(0, 0, 0));
	S.Format("Total = %12.3le W (%s)", Sum, Particles);
	pDC->TextOut(origX, origY + 30, S);
	S.Format("Max = %12.3le W/m2", MaxVal);
	pDC->TextOut(origX, origY + 50, S);

	pDC->SelectObject(&pLV->bigfont);
	//int w = pDC->GetTextExtent(S).cx;
	//S.Format("%d", pLV->Plate->Number);
	//pDC->TextOut(origX + 10, 10, S + ": " + pLV->Plate->Comment);
	pDC->TextOut(origX +10, 10, Comment);
	pDC->SelectObject(pOldFont);
	pDC->SelectObject(pOldPen);
	pLV->ReleaseDC(pDC);

}

void  CLoad :: DrawLoadLimited(CView * pView, CDC* pDC, double X0, double Y0, double X1, double Y1) //
{
	MSG message;
	CLoadView * pLV = (CLoadView *) pView;
	pLV->STOP = FALSE;
	CFont* pOldFont = pDC->SelectObject(&pLV->smallfont);
	pDC->SetTextColor(RGB(0, 0, 0));
//	pDC->SetBkMode(0);
	double dX = X1 - X0;
	double dY = Y1 - Y0;
	int ix, jy, xProf, yProf;
	int xmin = pLV->OrigX;
	int ymin = pLV->OrigY;
	int xmax = xmin + (int)(dX * pLV->ScaleX);
	int ymax = ymin - (int)(dY * pLV->ScaleY);
	CBrush brush;
	CBrush * oldbrush;
	if (pLV->Contours) brush.CreateSolidBrush(RGB(255,255,220)); 
	else brush.CreateSolidBrush(RGB(230,230,190)); //- haki//(RGB(255,255,220)) - yellow
	oldbrush = pDC->SelectObject(&brush);
	pDC->Rectangle(xmin, ymin, xmax, ymax);
	pDC->SelectObject(oldbrush);
	CString S;
	
	// int icross = (int)floor(dX / CrossX);
	// int jcross = (int)floor(dY / CrossY);
	
	double Val, ratio = 0;
	int i, j, k;
	double x, y;
	COLORREF color;
	
	int ii, jj, Nxx, Nyy;
	double hx, hy;
	hx = StepX/5; hy = StepY/5;

	Nxx = (int)floor(Xmax / hx); 
	Nyy = (int)floor(Ymax / hy);

	int rx = (int)(hx * pLV->ScaleX / 2)+1;//  = (int)(StepX *  pLV->ScaleX / 2)+1;
	if (rx<1) rx = 1;
	int ry = (int)(hy * pLV->ScaleY / 2)+1;//  = (int)(StepY *  pLV->ScaleY / 2)+1;
	if (ry<1) ry = 1;
	
//	if (MaxVal >1.e-10 && !pLV->Contours) {
	if (!pLV->Contours) { // color map
	for ( ii = 0; ii < Nxx; ii++) {
		x = (ii+0.5)*hx; 
		if (x < X0 || x > X1) continue;
		for ( jj = 0; jj < Nyy; jj++) {
			y = (jj+0.5)*hy;
			if (y < Y0 || y > Y1) continue;
			ix = xmin + (int)((x - X0) * pLV->ScaleX);
			jy = ymin - (int)((y - Y0) * pLV->ScaleY);
			if (MaxVal >1.e-10)	ratio = GetVal(x,y) / MaxVal;// Val[i][j] / MaxVal;
			else ratio = 1;
			color = GetColor10(ratio);
	//		color = RGB(Red(ratio), Green(ratio), Blue(ratio));
	//		color = RGB(Gray(ratio), Gray(ratio), Gray(ratio));
			
			if (ratio <= 1 )
			DrawLoadPoint(ix, jy, rx, ry, color,  pDC);
			
			if (::PeekMessage(&message, NULL, 0,0, PM_REMOVE)) {
				::TranslateMessage(&message);
				::DispatchMessage(&message);
			}
			if (pLV->STOP) return;
		} // j
	} // i
	} //if (!Contours) 

	CPen BlackPen;
	BlackPen.CreatePen(PS_SOLID, 1, RGB(0,0,0));
	CPen * pOldPen = pDC->SelectObject(&BlackPen);
//	pDC->SetROP2(R2_NOT); 

	ix = xmin + (int)((iMaxVal*StepX - X0) * pLV->ScaleX);
	jy = ymin - (int)((jMaxVal*StepY - Y0) * pLV->ScaleY);
	// int r = (int) ((rx+ry) * 0.5);
	pDC->Ellipse(ix-5, jy-5, ix+5, jy+5);

	xProf = xmin + (int)((pLV->Cross.X - X0) * pLV->ScaleX);
	yProf = ymin - (int)((pLV->Cross.Y - Y0) * pLV->ScaleY);
	if (xProf < xmin) xProf = xmin;
	if (yProf > ymin) yProf = ymin;

	CPen Pen;
	Pen.CreatePen(PS_DASHDOT, 1, RGB(0,0,0));
	pDC->SelectObject(&Pen);
	pDC->SetROP2(R2_COPYPEN);
	
	pDC->MoveTo(xmin, yProf); pDC->LineTo(xmax, yProf);
	pDC->MoveTo(xProf, ymin); pDC->LineTo(xProf, ymax);
	
	//pDC->SelectObject(&pLV->font);
	pDC->SetTextColor(RGB(0,0,0));
	S.Format("Total = %12.3le W (%s)", Sum, Particles);
	pDC->TextOut(xmin, ymin +20, S);
	S.Format("Max = %12.3le W/m2", MaxVal);
	pDC->TextOut(xmin, ymin +35, S);
	pDC->SelectObject(pOldPen);

	if (!pLV->Contours && MaxVal > 1.e-6) { // colour bar
		pDC->Rectangle(xmin, ymin + 60, xmax, ymin + 80);
		for ( ii = 0; ii <= Nxx; ii++) {
			x = (ii+0.5) * hx; 
			Val = x / Xmax;  
			if (MaxVal >1.e-10)	ratio = Val;// Val[i][j] / MaxVal;
			else ratio = 1;
			color = GetColor10(ratio);
			ix = xmin + (int)((x - X0) * pLV->ScaleX);
			jy = ymin + 70;
			ry = 10;
			DrawLoadPoint(ix, jy, rx, ry, color,  pDC);
		}

		int kmax = 10;
		double order = GetMantOrder(MaxVal).Z;

		for (int k = 0; k < kmax; k++) {
			ix = xmin + (int) (k * dX * pLV->ScaleX / kmax);
			jy = ymin + 80;
			Val = k * MaxVal / kmax;
			S.Format("%0.2f", Val/order);//"%0.1f"
			if (pDC->GetTextExtent(S).cx > 0.7 * (xmax - xmin) / kmax) 
				S.Format("%0.1f", Val/order);
			pDC->MoveTo(ix, jy + 5); pDC->LineTo(ix, jy);
			pDC->TextOut(ix - 5, jy + 5, S);
		}
		S.Format("%4.0e", order); pDC->TextOutA(ix + 5, ymin + 60, S);
		
	} // color bar

	pDC->SelectObject(&pLV->bigfont);
	if (pLV->Contours) 
	{
		SetLevels(pLV, pDC, X0, Y0, X1, Y1);
	}
	
	//int w = pDC->GetTextExtent(S).cx;
	S.Format("%d", pLV->Plate->Number);
	pDC->TextOut(xmin - 10, 10, S + ": " + pLV->Plate->Comment);
	pDC->SelectObject(pOldFont);

}

void  CLoad :: SetLevels(CView * pView, CDC* pDC, double X0, double Y0, double X1, double Y1) //
{
	if (LevelMin < LevelMax && LevelStep < 1.e-12) {
		AfxMessageBox("invalid level range or step"); return;
	}
	if (LevelMin < 1.e-6 && LevelMax - LevelMin < LevelStep) {
		AfxMessageBox("invalid level range or step"); return;
	}
	
	MSG message;
	CLoadView * pLV = (CLoadView *) pView;
	//pLV->STOP = FALSE;	
	CPen BlackPen;
	BlackPen.CreatePen(PS_SOLID, 1, RGB(0,0,0));
	CPen * pOldPen = pDC->SelectObject( &BlackPen);
	if (pLV->Contours == FALSE)  pDC->SetROP2(R2_NOT); 
	double lev, LEV; // W MW
	double val1, val2, dx, dy;
	int i, j,  Nxx, Nyy;
	double hx, hy;
	double x, y;
	int ix, jy;
	double dX = X1 - X0;
	double dY = Y1 - Y0;
	int xmin = pLV->OrigX;
	int ymin = pLV->OrigY;
	int xmax = xmin + (int)(dX * pLV->ScaleX);
	int ymax = ymin - (int)(dY * pLV->ScaleY);
	pDC->FloodFill(xmin, ymin + 60, RGB(255, 255, 255));
	pDC->SetTextColor(RGB(255, 0, 0));
//	pDC->TextOut(xmin, ymin + 60, "Click here to STOP");
	
	double ratio = StepY/StepX;
	if (ratio > 1) { hx = StepX/2; hy = hx;}
	else { hy = StepY/2; hx = hy;}

	Nxx = (int)ceil(Xmax / hx); 
	Nyy = (int)ceil(Ymax / hy);

	if (Nxx * Nyy < 900) {
		ratio =  900 / (Nxx*Nyy);
		hx /= ratio; hy /= ratio;
		Nxx = (int)ceil(Xmax / hx); 
		Nyy = (int)ceil(Ymax / hy);
	}


	//if (pLV->STOP) return;
	pdd point;
	vector <pdd> points;
	vector <tDistInfo> Dist;// = ConnectPoints(points);//
	CArray <int, int> ties;
	C3Point p1, p2, g1, g2, vp;
//	double gabs1, gabs2;
	double dlim = 3 * sqrt(hx*hx + hy*hy);
//	BOOL found = FALSE;
	
	if (MaxVal < 1.e-10) goto label1;
	/*for (int k = 1; k < 10; k ++) {
		lev = 0.05 + 0.1 * k;*/
		LEV = LevelMin; //MW/m2; 
	//for (int l = 1; l < 4; l++) { // l < 11
		while (LEV <= LevelMax) {
		if (LEV < 1.e-6) LEV = LevelStep;
		lev = LEV * 1000000;
		ties.RemoveAll();
	//	pDC->SetROP2(R2_NOT);
		points.clear();
		Dist.clear();
		dists.clear();
		x = hx/2; 
		while (x < Xmax) {
		//	if (pLV->STOP) break;
			y = hy/2;
			while (y < Ymax) {
			val1 = GetVal(x,y);// / MaxVal;//
			val2 = GetVal(x+hx,y);// / MaxVal;//
			if ((lev - val1)*(lev - val2) <= 0) {
				dx = hx * (lev - val1)/(val2 - val1);
				ix = xmin + (int)((x+dx - X0) * pLV->ScaleX);
				jy = ymin - (int)((y - Y0) * pLV->ScaleY);
				if (pLV->Contours == FALSE)	pDC->SetPixel(ix, jy, RGB(0,0,0));
				point.first = x+dx; point.second = y;
				points.push_back(point);
				ties.Add(0);
			//	found = TRUE;
			//	hx = hx/2;
			//	x = x - 0.5*hx;
			} // lev between points
			/*else { 
			//	found = FALSE;
				hx = hx *2;
				if (hx > StepX/2) hx = StepX/2;
			}*/
			
			val2 = GetVal(x,y+hy);// / MaxVal;//
			if ((lev - val1)*(lev - val2) <= 0) {
				dy = hy * (lev - val1)/(val2 - val1);
				ix = xmin + (int)((x - X0) * pLV->ScaleX);
				jy = ymin - (int)((y+dy - Y0) * pLV->ScaleY);
				if (pLV->Contours == FALSE)	pDC->SetPixel(ix, jy, RGB(0,0,0));
				point.first = x; point.second = y+dy;
				points.push_back(point);
				ties.Add(0);
			
			} // lev between points
			
			if (::PeekMessage(&message, NULL, 0,0, PM_REMOVE)) {
				::TranslateMessage(&message);
				::DispatchMessage(&message);
			}
			if (pLV->STOP) {
				//pDC->TextOut(xmin, ymin + 60, "STOPPED");
				goto label1;//{ pDC->SelectObject(pOldPen); return; 
			}

			y = y + hy;
				
		} // y
			x = x + hx; 
	} // x

	
//	pDC->SetROP2(R2_COPYPEN);
	if (pLV->Contours) { // sort points + connect points
		if (points.size() > 10000) {
			int reply = AfxMessageBox("Operation may take few minutes.\n Continue", 3);
			if (reply != IDYES) {
				//pDC->TextOut(xmin, ymin + 60, "STOPPED");
				goto label1; //{ points.clear(); ties.RemoveAll(); return; 
			}
		}
		Dist = ConnectPoints(points);//returns sorted vector of distances

	
	for (int k = 0; k <(int)Dist.size(); k++) {
		//if (pLV->STOP) break;
		tDistInfo D = Dist[k];
		i = D.n1;
		j = D.n2;
		double d = D.d;
		if (d < dlim) {
			p1.X = points[i].first;	p1.Y = points[i].second;
			p2.X = points[j].first;	p2.Y = points[j].second;
		
			if (ties[i] < 2 && ties[j] < 2)
			{
				ix = xmin + (int)((p1.X - X0) * pLV->ScaleX);
				jy = ymin - (int)((p1.Y - Y0) * pLV->ScaleY);
				pDC->MoveTo(ix, jy); 
				ix = xmin + (int)((p2.X - X0) * pLV->ScaleX);
				jy = ymin - (int)((p2.Y - Y0) * pLV->ScaleY);
				if (ix > xmin && ix < xmax) pDC->LineTo(ix, jy); 
				else pDC->MoveTo(ix, jy);
				ties[i]++; ties[j]++;

				if (::PeekMessage(&message, NULL, 0,0, PM_REMOVE)) {
					::TranslateMessage(&message);
					::DispatchMessage(&message);
				}
				if (pLV->STOP) {
					//pDC->TextOut(xmin, ymin + 60, "STOPPED");
					goto label1; //{ pDC->SelectObject(pOldPen); return; 
				}
			} // (ties[i] < 2 && ties[j] < 2)
		} // d < dlim
	} // k <(int)Dist.size() 
	} // conturs = true
	if (LevelWrite) {
		CPlate * plate = pLV->Plate;
		WriteLevel(plate, LEV, points);
	}
	LEV += LevelStep;
	} // LEV <= LevMax  //l < 4

label1:
	CString S = "STOPPED";
	
	pDC->Rectangle(xmin+1, ymin + 50, xmax, ymin + 80);
	pDC->FloodFill(xmin+1, ymin + 60, RGB(255, 255, 255));
	//pDC->TextOutA(xmin + 50, ymin + 60, S);

	points.clear();
	Dist.clear();
	dists.clear();
	ties.RemoveAll();
	pDC->SelectObject(pOldPen);
	pDC->SetTextColor(RGB(0, 0, 0));

}



void CLoad :: WriteProfiles(int num, double X, double dHor, double dVert)
{
	CString  sn, name;
	FILE * fout;
	double x, y, load;
	sn.Format("%d", num);
	
	name = "prof" + sn + "_hor" + ".dat"; // horizontal
	fout = fopen(name, "w");
	fprintf(fout," Horizontal PD profile on Surf %d (X = %4.2f m from GG) \n\n", num, X);
	fprintf(fout," \t Dist,m  \t  PD, W/m2 \n"); 
	fprintf(fout, "-------------------------------------------\n");
		
	for (int i = 0; i <= Nx; i++) {
		x = i*StepX + dHor;
		load = Val[i][jProf];
		fprintf(fout, " \t %-7.3f \t %-12.3le \n", x, load);
	}
	fclose(fout);
	
	name = "prof" + sn + "_vert" + ".dat"; // vertical
	fout = fopen(name, "w");
	fprintf(fout," Vertical PD profile on Surf %d (X = %4.2f m from GG) \n\n", num, X);
	fprintf(fout," \t Dist,m  \t  PD, W/m2 \n"); 
	fprintf(fout, "-------------------------------------------\n");
	for (int j = 0; j <= Ny; j++) {
		y = j*StepY + dVert;
		load = Val[iProf][j];
		fprintf(fout, " \t %-7.3f \t %-12.3le \n", y, load);
	}
	fclose(fout);
	
}
void  CLoad :: DrawLoadProfiles(CView * pView, CDC* pDC) //not used
{
	CSetView * pSV = (CSetView *) pView;
	CFont* pOldFont = pDC->SelectObject(&pSV->smallfont);
	pDC->SetTextColor(RGB(0, 0, 0));
//	pDC->SetBkMode(1);
	int x, y;
	int i, j;
	double ix, jy;
	int xmin = 20;
	int ymin1 =  pSV->Height  / 2 +5;
	int ymin2 =  pSV->Height + 25;

	pSV->RectArrayX.RemoveAll();
	pSV->RectArrayY.RemoveAll();

	MaxProf = 0;
	double val;
	for (i = 0; i <= Nx; i++) {
		val = Val[i][jProf];
		if (val >= MaxProf) MaxProf = val;
	}
	for (j = 0; j <= Ny; j++) {
		val = Val[iProf][j];
		if (val >= MaxProf) MaxProf = val;
	}
	double ScaleX = (pSV->Width) / Xmax; //ScaleX * 1.5;
	double ScaleY = (pSV->Width) / Ymax; //ScaleY;
	double ScaleZ =  (pSV->Height* 0.4)/MaxProf;
	CString S;
	int pos, pos1;
	CString S1, S2, S3;
	double order;
	double mant;
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
	 CrossZ = 0.5*order;
	 if (mant > 3.5) CrossZ = order;
	  if (mant > 6) CrossZ = 2*order;

	int xmax1 = xmin +  (int)(Xmax * ScaleX);
	int xmax2 = xmin +  (int)(Ymax * ScaleY);
	int ymax1 = ymin1 - (int)ceil(mant*order *ScaleZ);
	int ymax2 = ymin2 - (int)ceil(mant*order *ScaleZ);
	CBrush brush1, brush2;
	CBrush * oldbrush;
//	brush.CreateSolidBrush(RGB(250,250,220));
	brush1.CreateSolidBrush(RGB(255,220,255));
	oldbrush = pDC->SelectObject(&brush1);
	pDC->Rectangle(xmin, ymin1, xmax1, ymax1);
	brush2.CreateSolidBrush(RGB(220,255,255));
	pDC->SelectObject(&brush2);
	pDC->Rectangle(xmin, ymin2, xmax2, ymax2);
	pDC->SelectObject(oldbrush);
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

	int icross1 = (int)(Xmax / CrossX);
	int jcross = (int)(mant*order / CrossZ);

////////// upper magenta
	for ( i = 1; i <= icross1; i++) {
		 x = xmin + (int)(i*CrossX* ScaleX);
		 y = ymin1; pDC->MoveTo(x,y);
		 ix = i*CrossX;
		 S.Format("%d", (int)ix);
		 if (CrossX < 10)  S.Format("%0.0f", ix);
		 if (CrossX < 1)  S.Format("%0.1f", ix);
		 if (CrossX < 0.1)  S.Format("%0.2f", ix);
		 if (CrossX < 0.01)  S.Format("%0.3f", ix);
		 pDC->TextOut(x-5, y, S);
		 y = ymax1; pDC->LineTo(x,y);
	}
	for ( j = 1; j <= jcross; j++) {
		 y = ymin1 - (int)(j*CrossZ* ScaleZ);
		 x = xmin; pDC->MoveTo(x,y);
		 jy = j*CrossZ;
		 if (CrossZ/order < 10)  S.Format("%0.0f", jy/order);
		 if (CrossZ/order < 1)  S.Format("%0.1f", jy/order);
		 pDC->TextOut(x-15, y-5, S);
		 x = xmax1; pDC->LineTo(x,y);
	}

//////////// lower blue
		int icross2 = (int)(Ymax / CrossY);
		for ( i = 1; i <= icross2; i++) {
		 x = xmin + (int)(i*CrossY* ScaleY);
		 y = ymin2; pDC->MoveTo(x,y);
		 ix = i*CrossY;
		 S.Format("%d", (int)ix);
		 if (CrossY < 10)  S.Format("%0.0f", ix);
		 if (CrossY < 1)  S.Format("%0.1f", ix);
		 if (CrossY < 0.1)  S.Format("%0.2f", ix);
		 if (CrossY < 0.01)  S.Format("%0.3f", ix);
		 pDC->TextOut(x-5, y, S);
		 y = ymax2; pDC->LineTo(x,y);
	}
	for ( j = 1; j <= jcross; j++) {
		 y = ymin2 - (int)(j*CrossZ* ScaleZ);
		 x = xmin; pDC->MoveTo(x,y);
		 jy = j*CrossZ;
		 if (CrossZ/order < 10)  S.Format("%0.0f", jy/order);
		 if (CrossZ/order < 1)  S.Format("%0.1f", jy/order);
		 pDC->TextOut(x-15, y-5, S);
		 x = xmax2; pDC->LineTo(x,y);
	}
///////////////////////////////////////////////////////////
	pDC->MoveTo(xmin, ymin1);
	for (i = 0; i <= Nx; i++) {
		x = xmin + (int) (i*StepX* ScaleX);
		y = ymin1 - (int)(Val[i][jProf] * ScaleZ);
		pDC->LineTo(x, y);
	}
	pDC->MoveTo(xmin, ymin2);
	for (j = 0; j <= Ny; j++) {
		x = xmin + (int) (j*StepY* ScaleY);
		y = ymin2 - (int)(Val[iProf][j] * ScaleZ);
		pDC->LineTo(x, y);
	}

	CRect rect;
	CPen Pen;
	if(!Pen.CreatePen(PS_SOLID, 2, RGB(255,0,0))) return;
	CPen * pOldPen = pDC->SelectObject( &Pen);

	for (i = 0; i <= Nx; i++) {
		x = xmin + (int) (i*StepX* ScaleX);
		y = ymin1 - (int)(Val[i][jProf] * ScaleZ);
		pDC->Ellipse(x-3, y-3, x+3, y+3);
		rect = new CRect();
		rect.SetRect(x-3, y-3, x+3, y+3);
		pSV->RectArrayX.Add(rect);
	}
	S.Format("Vert.Dist = %0.2f", jProf*StepY);
	pDC->TextOut(xmax1 - 80, ymax1 - 20, S);
	for (j = 0; j <= Ny; j++) {
		x = xmin + (int) (j*StepY* ScaleY);
		y = ymin2 - (int)(Val[iProf][j] * ScaleZ);
		pDC->Ellipse(x-3, y-3, x+3, y+3);
		rect = new CRect();
		rect.SetRect(x-3, y-3, x+3, y+3);
		pSV->RectArrayY.Add(rect);
	}
	S.Format("Hor.Dist = %0.2f", iProf*StepX);
	pDC->TextOut(xmax2 - 80, ymax2 - 20, S);
	
	pDC->SelectObject(pOldPen);
	pDC->SelectObject(pOldFont);

	}

void  CLoad :: DrawLoad() //not used
{
	CLoadView * pLV = (CLoadView *) pLoadView;
	CDC* pDC = pLV->GetDC();
	pDC->Rectangle(10, 10, 100, 100);
	pDC->TextOut(10,10, "Power Deposition"); 
	pLV->ReleaseDC(pDC);
}

void CLoad :: DrawLoadPoint(int x, int y, int rx, int ry, COLORREF color, CDC* pDC)
{
	CPen Pen;
	if(!Pen.CreatePen(PS_SOLID, 1, color)) return;
	CPen * pOldPen = pDC->SelectObject( &Pen);
//	if (color == RGB(255,255,255)) 	pDC->SelectObject(pOldPen);

	CBrush brush;
	CBrush * oldbrush;
	brush.CreateSolidBrush(color);
	oldbrush = pDC->SelectObject(&brush);

	if (color == RGB(255,255,255)) {
		pDC->SelectObject(pOldPen);
		pDC->Ellipse(x-rx, y-ry, x+rx, y+ry);
	}
	else	pDC->Rectangle(x-rx, y-ry, x+rx, y+ry);

	pDC->SelectObject(pOldPen);
	pDC->SelectObject(oldbrush);
//	ReleaseDC(pDC);
}

void Smooth(double & v1, double & v2, double & v3)
{
	int coeff = 2;
	double delta1, delta2, delta, sum;
	delta1 =  v2 - v1;
	delta2 =  v2 - v3;
	if (delta1*delta2 <=0) return; // uniform changing (rise or drop)
	delta = 0.5*(delta1+delta2) / coeff;
	sum = v1 + v3;
	v2 -= delta;
	v1 += delta * v1 / sum;
	v3 += delta * v3 / sum;
}

void  CLoad::  SmoothLoad(int degree)
{
	SmoothDegree = degree;
	int i, j;
	double v1, v2, v3;
	for (int k=0; k < SmoothDegree; k++) {
		for ( i = 0; i <= Nx; i++) {
			for ( j = 1; j < Ny; j++) {
				v1 = Val[i][j-1]; v2 = Val[i][j]; v3 = Val[i][j+1];
				Smooth(v1, v2, v3);
				Val[i][j-1] = v1;  Val[i][j] = v2;  Val[i][j+1] =  v3;
			} // j
			for ( j = Ny-1; j > 0; j--) {
				v1 = Val[i][j+1]; v2 = Val[i][j]; v3 = Val[i][j-1];
				Smooth(v1, v2, v3);
				Val[i][j+1] = v1;  Val[i][j] = v2;  Val[i][j-1] =  v3;
			} // j
		}// i
		for ( j = 0; j <= Ny; j++) {
			for ( i = 1; i < Nx; i++) {
				v1 = Val[i-1][j]; v2 = Val[i][j]; v3 = Val[i+1][j];
				Smooth(v1, v2, v3);
				Val[i-1][j] = v1;  Val[i][j] = v2;  Val[i+1][j] =  v3;
			}//i
			for ( i = Nx-1; i > 0; i--) {
				v1 = Val[i+1][j]; v2 = Val[i][j]; v3 = Val[i-1][j];
				Smooth(v1, v2, v3);
				Val[i+1][j] = v1;  Val[i][j] = v2;  Val[i-1][j] =  v3;
			}//i
		} // j
	} // k = SmoothDegree
	
}


//---------------------------------------------------------
// Class  CPlate
//------------------------------------------------------------

CPlate:: CPlate()
{
	Solid = TRUE;
	Visible  = TRUE;
	MAP = TRUE;
	Loaded = FALSE;
	Load = NULL;
	SmLoad = NULL;
	Selected = FALSE;
	Touched  = FALSE;
	Number = 0;
	filename = "";
	SmoothDegree = 0;
	RectMark = new CRect;
	Comment = "";
	Fixed = 0; // //	PLAN = TRUE;
	OrtDirect = 1;
	Vmin = C3Point(0,0,0); Vmax = C3Point(0,0,0);
	AngularProfile.RemoveAll();
	MinAngle = 0;
	MaxAngle = 1580;
	StepAngle = 1; //mrad
	AtomPower = 0;
	NegPower = 0; 
	PosPower = 0;

}

void  CPlate::InitAbsLimits()
{
	AbsYmin = 1000; AbsYmax = -1000;
	AbsZmin = 1000; AbsZmax = -1000;
}

CPlate::~CPlate()
{
	delete RectMark;

	if (Loaded)  {
		 delete (Load);
		//if (SmLoad != NULL) delete SmLoad;
	}// loaded

	Falls.RemoveAll();
	AngularProfile.RemoveAll();
}

CPlate::  CPlate(C3Point p0, C3Point p1, C3Point p2, C3Point p3)  
{
	//CPlate::CPlate();
	Load = NULL;	
	Loaded = FALSE;
	SetLocals(p0, p1, p2, p3);
	Comment = "";
	Fixed = 0; // //	PLAN = TRUE;
	OrtDirect = 1;
	SmoothDegree = 0;
	MAP = TRUE;
	AtomPower = 0;
	NegPower = 0; 
	PosPower = 0;	
}

void CPlate::Shift(double X, double Y, double Z) // shift whole plate along vector
{
	C3Point p0, p1, p2, p3;
		
	for (int k = 0; k <4; k++) {
		Corn[k].X = Corn[k].X + X; 
		Corn[k].Y = Corn[k].Y + Y; 
		Corn[k].Z = Corn[k].Z + Z; 
	}
/*	MassCentre.X = MassCentre.X + X;
	MassCentre.Y = MassCentre.Y + Y;
	MassCentre.Z = MassCentre.Z + Z;
	SetOrts();*/

	p0 = Corn[0]; 	p1 = Corn[1]; 	p2 = Corn[3]; 	p3 = Corn[2];
	SetLocals(p0, p1, p2, p3);
}

void CPlate::ShiftVert(double Z1, double Z2) // vertically shift entry, exit
{
	C3Point p0, p1, p2, p3;
	for (int k = 0; k <4; k++) {
		if (Corn[k].X < MassCentre.X) Corn[k].Z = Corn[k].Z + Z1; 
		else Corn[k].Z = Corn[k].Z + Z2; 
	}
/*	MassCentre.Z = MassCentre.Z + 0.5*(Z1 + Z2);
	SetOrts();*/
	p0 = Corn[0]; 	p1 = Corn[1]; 	p2 = Corn[3]; 	p3 = Corn[2];
	SetLocals(p0, p1, p2, p3);

}

void CPlate::ShiftHor(double Y1, double Y2) // horizontally shift entry, exit
{
	C3Point p0, p1, p2, p3;
	for (int k = 0; k <4; k++) {
		if (Corn[k].X < MassCentre.X) Corn[k].Y = Corn[k].Y + Y1; 
		else Corn[k].Y = Corn[k].Y + Y2; 
	}
	/*MassCentre.Y = MassCentre.Y + 0.5*(Y1 + Y2);
	SetOrts();*/

	p0 = Corn[0]; 	p1 = Corn[1]; 	p2 = Corn[3]; 	p3 = Corn[2];
	SetLocals(p0, p1, p2, p3);

}

void  CPlate:: SetArbitrary(C3Point p0, C3Point p1, C3Point p2, C3Point p3) // sort corners 
{
	C3Point P, P0, P1, P2, P3; // P0 - orig, P1 - X, P2 - Y, P3 - diagonal
	C3Point Base = C3Point(0, -5, -5);
	int i;

	double dist[4];
	dist[0] = GetDistBetween(Base, p0);
	dist[1] = GetDistBetween(Base, p1);
	dist[2] = GetDistBetween(Base, p2);
	dist[3] = GetDistBetween(Base, p3);

	C3Point lb, rt, lb1, rt1, lb2, rt2;
	double d;
	d = dist[0]; lb = p0;
	if (dist[1] < d) { lb = p1; d = dist[1]; }
	if (dist[2] < d) { lb = p2; d = dist[2]; }
	if (dist[3] < d) { lb = p3; d = dist[3]; }
	
	C3Point v[4];
	v[0] = p0 - lb;
	v[1] = p1 - lb;
	v[2] = p2 - lb;
	v[3] = p3 - lb;

	C3Point Norm(0,0,0);
	

	P0 = lb; P3 = rt;
	if (fabs(lb.X - P2.X) > fabs(lb.X - P1.X)) { P = P2; P2 = P1; P1 = P;}
	
	SetLocals(P0, P1, P2, P3);

}


void  CPlate::  SortCorners(int ndir, C3Point p0, C3Point p1, C3Point p2, C3Point p3)// p0, p1, p2 define the plane
{
	C3Point P, P0, P1, P2, P3, V01, V02, V03, Norm13, Norm23, Norm12;
	double D0, D1, D2, D3, Dmin; 
	CString S; // = this->Comment;
	P0 = p0; P1 = p1; P2 = p2; P3 = p3;
	C3Point VMID = MassCentre; // (Vmin + Vmax) * 0.5;
	C3Point ref; // 'left-bottom corner' of the closest projection
	if (ndir == 1) // X-normal
		ref = C3Point(VMID.X, Vmin.Y, Vmin.Z);
	if (ndir == 2) // Y-normal
		ref = C3Point(Vmin.X, VMID.Y, Vmin.Z);
	if (ndir == 3) // Z-normal
		ref = C3Point(Vmin.X, Vmin.Y, VMID.Z);
	D0 = GetDistBetween(ref, P0); 
	Dmin = D0;
	D1 = GetDistBetween(ref, P1);
	D2 = GetDistBetween(ref, P2);
	D3 = GetDistBetween(ref, P3);
	// set P0
	if (D1 < Dmin) { Dmin = D1; P0 = p1; P1 = p2; P2 = p3; P3 = p0;}
	if (D2 < Dmin) { Dmin = D2; P0 = p2; P1 = p3; P2 = p0; P3 = p1;}
	if (D3 < Dmin) { Dmin = D3; P0 = p3; P1 = p0; P2 = p1; P3 = p2;}

	V01 = P1 - P0; V02 = P2 - P0; V03 = P3 - P0;
	Norm13 = VectProd(V01, V03); Norm23 = VectProd(V02, V03);
	if (ScalProd(Norm13, Norm23) > 0) { //V03 is not inside V01 and V02
		P = P2;  P2 = P3; P3 = P; // exchange P2-P3
	}
	V01 = P1 - P0; V02 = P2 - P0; V03 = P3 - P0;
	Norm13 = VectProd(V01, V03); Norm23 = VectProd(V02, V03);
	if (ScalProd(Norm13, Norm23) > 0) { // outside again
		 P = P1; P1 = P3; P3 = P; // exchange P1-P3
		}
	
	V01 = P1 - P0; V02 = P2 - P0; /*V03 = P3 - P0;
	if (V02.X > V01.X && V02.Z < V02.X) {//v2 close to X, XZ-plane*/
	if (ndir == 1 && fabs(V01.Y) < fabs(V02.Y)) {
		P = P1; P1 = P2; P2 = P; // exchange P1-P2
	}
	if (ndir == 2 && fabs(V01.X) < fabs(V02.X)) {
		P = P1; P1 = P2; P2 = P; // exchange P1-P2
	}
	if (ndir == 3 && fabs(V01.X) < fabs(V02.X)) {
		P = P1; P1 = P2; P2 = P; // exchange P1-P2
	}

/*	V01 = P1 - P0; V02 = P2 - P0; V03 = P3 - P0;
	if (V02.X > V01.X && V02.Y < V02.X) {//v2 close to X, XY-plane
		P = P1; P1 = P2; P2 = P; // exchange P1-P2
	}*/
	
	Corn[0] = P0; Corn[1] = P1; Corn[2] = P3; Corn[3] = P2;

	int N = 0;
	if (this->Number > 0) N = this->Number;
	CString Comm = "NO comment";
	if (this->Comment.GetLength() > 0) Comm = this->Comment;
	V01 = P1 - P0; V02 = P2 - P0; V03 = P3 - P0;
	Norm13 = VectProd(V01, V03); Norm23 = VectProd(V02, V03); 
	Norm12 = VectProd(V01, V02);
	
	if (ModVect(Norm12) < 1.e-3 || ModVect(Norm13) < 1.e-3 || ModVect(Norm23) < 1.e-3) {
		S.Format("Surf %s \n Bad vertex data", Comm);
		//AfxMessageBox(S);
	}
	if (ModVect(VectProd(Norm13, Norm23)) > 1.e-3 && N > 0) {
		S.Format("Surf %d (%s) \n Vertex out of the plane", N, Comm);
		//AfxMessageBox(S);
	}

}

void  CPlate:: SetVminVmax()
{ 
	C3Point p0, p1, p2, p3;
	p0 = Corn[0]; 	p1 = Corn[1]; 	p2 = Corn[3]; 	p3 = Corn[2];
	SetVminVmax(p0, p1, p2, p3);
	
}


void  CPlate:: SetVminVmax(C3Point p0, C3Point p1, C3Point p2, C3Point p3)
{ 
	//Corn[0] = p0; 	Corn[1] = p1; 	Corn[3] = p2; 	Corn[2] = p3;
	Vmin.X = Min(Min(p0.X, p1.X), Min(p2.X, p3.X));
	Vmin.Y = Min(Min(p0.Y, p1.Y), Min(p2.Y, p3.Y));
	Vmin.Z = Min(Min(p0.Z, p1.Z), Min(p2.Z, p3.Z));
	Vmax.X = Max(Max(p0.X, p1.X), Max(p2.X, p3.X));
	Vmax.Y = Max(Max(p0.Y, p1.Y), Max(p2.Y, p3.Y));
	Vmax.Z = Max(Max(p0.Z, p1.Z), Max(p2.Z, p3.Z));
	MassCentre = (p0 + p1 + p2 + p3) / 4;
}
void CPlate::SetAbsLimits() // to find CalcArea dimensions
{
	double min, max;
	AbsYmin = Min(AbsYmin, Vmin.Y);
	AbsYmax = Max(AbsYmax, Vmax.Y);
	AbsZmin = Min(AbsZmin, Vmin.Z);
	AbsZmax = Max(AbsZmax, Vmax.Z);
}

void  CPlate::  SetLocals(C3Point p0, C3Point p1, C3Point p2, C3Point p3) // Orig, Orts, Dimensions
{
	//Corn[0] = p0; Corn[1] = p1; 	Corn[3] = p2; 	Corn[2] = p3;
	SetVminVmax(p0, p1, p2, p3); // + mass centre
	SetAbsLimits(); //for area dimensions
	
	int ndir = NormDirect(p0, p1, p2, p3);
	if (ndir < 1) {
		AfxMessageBox("Invalid Normal vector");
		ndir = 1;
	}
	SortCorners(ndir, p0, p1, p2, p3); // Corn[0],..
	 
	SetOrtsNew();
	SetViewDim(); // display total rect  
	Loaded = FALSE;
	Touched  = FALSE;
	AtomPower = 0;
	NegPower = 0; 
	PosPower = 0;
}
int  CPlate::NormDirect(C3Point p0, C3Point p1, C3Point p2, C3Point p3)
{
	int direct = 0; // not defined
	C3Point v01, v02, v03, n12, n23, Norm;
	double len12, len23;
	v01 = p1 - p0;
	v02 = p2 - p0;
	v03 = p3 - p0;
	n12 = VectProd(v01, v02);
	n23 = VectProd(v01, v03);
	len12 = ModVect(n12);
	len23 = ModVect(n23);
	if (len12 > len23) Norm = n12;
	else Norm = n23;

	if (fabs(Norm.X) >= fabs(Norm.Y)) direct = 1; // major X 
	if (fabs(Norm.Z) >= fabs(Norm.X)) direct = 3; // major Z
	if (fabs(Norm.Y) >= fabs(Norm.X) && fabs(Norm.Y) >= fabs(Norm.Z)) direct = 2; // major Y
	return direct;
}

void CPlate::SetOrtsNew() // try
{
	C3Point Diag = Vmax - Vmin;
	Orig = Corn[0]; // defined in SortCorners as closest to Vmin
	double dx = Diag.X;
	double dy = Diag.Y;
	double dz = Diag.Z;
	int regular = 0; // not defined
	if (dx < 1.e-10) { // YZ plane
		OrtX = C3Point(0, 1, 0);
		OrtY = C3Point(0, 0, 1);
		OrtZ = VectProd(OrtX, OrtY);
		regular = 1; // X-norm
	}
	if (dy < 1.e-10) { // XZ plane
		OrtX = C3Point(1, 0, 0);
		OrtY = C3Point(0, 0, 1);
		OrtZ = VectProd(OrtX, OrtY);
		regular = 2; // Y-norm
	}
	if (dz < 1.e-10) { // XY plane
		OrtX = C3Point(1, 0, 0);
		OrtY = C3Point(0, 1, 0);
		OrtZ = VectProd(OrtX, OrtY);
		regular = 3; // Z-norm
	}

	if (regular < 1) SetOrtsFree(); // new - set Orig in the mass centre, set Orts

	SetMaxLocalDim();// + shift Orig (Orts fixed)  
}

void CPlate::SetOrtsFree() // irregular plates
{
	Orig = MassCentre;
	C3Point v01, v23, v03, vectX, vectY, norm;
	double modX, modY, modN;
	v01 = Corn[1] - Corn[0];
	v23 = Corn[2] - Corn[3];
	vectX = v01 + v23;
	modX = ModVect(vectX);
	OrtX = vectX / modX;
	v03 = Corn[3] - Corn[0];
	norm = VectProd(vectX, v03);
	modN = ModVect(norm);
	OrtZ = norm / modN;
	vectY = VectProd(vectX, norm);
	modY = - ModVect(vectY);
	OrtY = vectY / modY;

}

void CPlate:: SetOrts() //old - called by SetLocals, Orts and Orig are needed for GetLocal, GetGlobal
{
	C3Point p1, p2, p3;
	Orig = Corn[0]; p1 = Corn[1]; p2 = Corn[3]; p3 = Corn[2];
	C3Point vectX =  VectSum(p1, Orig, 1, -1); // local vectorX
	C3Point vectY =  VectSum(p2, Orig, 1, -1); // local vectorY
	C3Point vectZ =  VectProd(vectX, vectY);
	double mod_vectX = ModVect(vectX);
	double mod_vectY = ModVect(vectY);
	double mod_vectZ = ModVect(vectZ);
	OrtX = vectX / mod_vectX;
	OrtY = vectY / mod_vectY;
	OrtZ = vectZ / mod_vectZ;

	double dx = ScalProd(vectY, OrtX);
	// double dy = ScalProd(vectX, OrtY);
	if (dx < 0) Orig = VectSum(Orig, OrtX *dx, 1, 1);
	else p2 = VectSum(p2, OrtX * dx, 1, -1);
		
	vectX =  VectSum(p1, Orig, 1, -1); // local vectorX
	vectY =  VectSum(p2, Orig, 1, -1); // local vectorY

	mod_vectX = ModVect(vectX);
	mod_vectY = ModVect(vectY);
	OrtX = vectX / mod_vectX;
	OrtY = vectY / mod_vectY;
	
	vectZ =  VectProd(vectX, vectY);
	mod_vectZ = ModVect(vectZ);
	OrtZ = vectZ / mod_vectZ;

	//SetMaxLocalDim();
}

void CPlate::SetMaxLocalDim()
{
	C3Point Ploc;
	Xmax = 0;
	Ymax = 0;
	double xmin = 1000;
	double ymin = 1000;
	for (int k = 0; k < 4; k++) {
		Ploc = GetLocal(Corn[k]);
		Xmax = Max(Xmax, Ploc.X);
		Ymax = Max(Ymax, Ploc.Y);
		xmin = Min(xmin, Ploc.X);
		ymin = Min(ymin, Ploc.Y);
	}
	ShiftOrig(xmin, ymin); // move Orig, Orts are fixed
}

void CPlate::ShiftOrig(double dx, double dy) // + correct dimensions (Orts are fixed)
{
	C3Point DX = OrtX * dx; // vector 
	C3Point DY = OrtY * dy;
	C3Point Dsum = DX + DY;
	Orig = Orig + Dsum;
	Xmax = Xmax - dx;
	Ymax = Ymax - dy;
}

void CPlate::SetViewDim() // default - set max limits / copy of Load::SetCrosses
{
	//double Xmax, Ymax; // local Max dimensions for Load View (Xmin = Ymin = 0)  
	//double crossX, crossY; // View grid steps - for Load and Profiles
	//double leftX, rightX, botY, topY; //local View dimensions
	double nx, ny;
	double hx = 1.0;
	double hy = 1.0;
	leftX = 0;// "xmin"
	rightX = Xmax; // max limit
	botY = 0;// 'ymin'
	topY = Ymax; // max limit
	crossX = (rightX - leftX) / 5;
	crossY = (topY - botY) / 5;
	BOOL stop = FALSE;
	do {
		nx = (int)(crossX / hx);
		ny = (int)(crossY / hy);
		if (nx < 1) hx /= 10;
		if (ny < 1) hy /= 10;
		if (nx > 2) hx *= 2;
		if (ny > 2) hy *= 2;
		if (nx >= 1 && nx <= 2 && ny >= 1 && ny <= 2) stop = TRUE;
	} while (!stop);

	crossX = hx*nx;
	crossY = hy*ny;
}

void CPlate::SetViewDim(double left, double right, double bot, double top) // - set new limits / copy of Load::SetCrosses
{
	//double Xmax, Ymax; // local Max dimensions for Load View (Xmin = Ymin = 0)  
	//double crossX, crossY; // View grid steps - for Load and Profiles
	//double leftX, rightX, botY, topY; //local View dimensions
	double nx, ny;
	double hx = 1;
	double hy = 1;
	CString S;
	
	leftX = max(left, 0.0);// "xmin"
	leftX = min(leftX, Xmax*0.9); // ensure left < right
	rightX = min(right, Xmax); // max limit
	botY = max(bot, 0.0);// 'ymin'
	botY = min(botY, Ymax*0.9); // ensure bot < top
	topY = min(top, Ymax); // max limit
	
	crossX = (rightX - leftX) * 0.2;
	crossY = (topY - botY) * 0.2;
	if (crossX < 1.e-3 || crossY < 1.e-3) {
		S.Format("Dims: Hor %g - %g  Vert  %g - %g", leftX, rightX, botY, topY);
		AfxMessageBox("SetViewDim:\n Invalid View crosses\n" + S);
		crossX = 1; crossY = 1;
	}

	BOOL stop = FALSE;
	do {
		nx = (int)(crossX / hx);
		ny = (int)(crossY / hy);
		if (nx < 1) hx *= 0.1;
		if (ny < 1) hy *= 0.1;
		if (nx > 2) hx *= 2;
		if (ny > 2) hy *= 2;
		if (nx >= 1 && nx <= 2 && ny >= 1 && ny <= 2) stop = TRUE;
	} while (!stop);

	crossX = hx*nx;
	crossY = hy*ny;
	//AfxMessageBox("SetViewDim done");
}

void CPlate:: SetFromLimits(C3Point LB, C3Point RT) // LeftBottom, RightTop - only for regular coord surfaces!
{ // set ortX/ortY (p1, p2) for rectangular plates with Orts along X/Y/Z
	C3Point p0, p1, p2, p3;
	p0 = LB;	p3 = RT;
	p1 = 0; p2 = 0;
	// double dist = GetDistBetween(p0, p3);
	double dx = fabs(p3.X - p0.X);
	double dy = fabs(p3.Y - p0.Y);
	double dz = fabs(p3.Z - p0.Z);
	if (dx < 1.e-10) { // YZ plane
	//	if (dy < dz) {
		p1 = C3Point(p0.X,  p3.Y,  p0.Z);
		p2 = C3Point(p0.X,  p0.Y,  p3.Z);
	//	}
	/*	else {
		p2 = C3Point(p0.X,  p3.Y,  p0.Z);
		p1 = C3Point(p0.X,  p0.Y,  p3.Z);
		}*/
	}
	if (dy < 1.e-10) {// XZ		SIDE = TRUE;
		//if (dx<dz) {
		p1 = C3Point(p3.X,  p0.Y,  p0.Z);
		p2 = C3Point(p0.X,  p0.Y,  p3.Z);
		//}
		/*else {
		p2 = C3Point(p3.X,  p0.Y,  p0.Z);
		p1 = C3Point(p0.X,  p0.Y,  p3.Z);
		}*/
	}
	if (dz < 1.e-10) {// XY		PLAN = TRUE;
		/*if (dy<dx) {
		p1 = C3Point(p0.X,  p3.Y,  p0.Z);
		p2 = C3Point(p3.X,  p0.Y,  p0.Z);
		}*/
		//else {
		p2 = C3Point(p0.X,  p3.Y,  p0.Z);
		p1 = C3Point(p3.X,  p0.Y,  p0.Z);
		//}
	}

	SetLocals(p0, p1, p2, p3);
	MAP = TRUE;
//	RgnPlan = new CRgn;
//	RgnSide = new CRgn;
}


C3Point CPlate:: GetLocal(C3Point P) // get local CS coord of the global P
{
	C3Point Plocal;
	double x0, y0; // coordinates in ortogonal (old) system
	C3Point Pvect =  VectSum(P, Orig, 1, -1); // substract vectors in global CS

	Plocal.X = ScalProd(Pvect, OrtX);// /  mod_vectZ;
	Plocal.Y = ScalProd(Pvect, OrtY);// /  mod_vectZ;
	Plocal.Z = ScalProd(Pvect, OrtZ);// /  mod_vectZ;
	return Plocal;
}
C3Point CPlate:: GetGlobalPoint(C3Point Ploc) //global position
{
	C3Point Pglobal;
	C3Point Pvect; // vector Ploc in Global CS
	Pvect = OrtX*Ploc.X + OrtY*Ploc.Y + OrtZ*Ploc.Z;
	Pglobal = Orig + Pvect;
	return Pglobal;
}
C3Point CPlate:: GetGlobalVector(C3Point Vat) 
//get global V direction (normalized)
{
	C3Point Vglobal;
	//C3Point Pvect; // vector Ploc in Global CS
	Vglobal = OrtX*Vat.X + OrtY*Vat.Y + OrtZ*Vat.Z;
	//Pglobal = Orig + Pvect;
	return Vglobal;
}
C3Point CPlate:: GetGlobalVector(double VXloc, double VYloc) 
// get global V direction (normalized)
// 1st - define local Vz
{
	C3Point Vglobal;
	double sumXY = VXloc*VXloc + VYloc*VYloc;
	if (sumXY > 1.0) sumXY = 1.; 
	double VZloc = sqrt(1.0 - sumXY);
	//C3Point Pvect; // vector Ploc in Global CS
	Vglobal = OrtX*VXloc + OrtY*VYloc + OrtZ*VZloc;
	return Vglobal;
}

BOOL CPlate:: IsCrossed(C3Point p1, C3Point p2)// by line defined by 2 points
{
//	if (GetDistBetween(p1, p2) < 1.e-16) return FALSE;

	if (p1.X < Vmin.X && p2.X < Vmin.X) return FALSE;
	if (p1.Y < Vmin.Y && p2.Y < Vmin.Y) return FALSE;
	if (p1.Z < Vmin.Z && p2.Z < Vmin.Z) return FALSE;

	if (p1.X > Vmax.X && p2.X > Vmax.X) return FALSE;
	if (p1.Y > Vmax.Y && p2.Y > Vmax.Y) return FALSE;
	if (p1.Z > Vmax.Z && p2.Z > Vmax.Z) return FALSE;


	C3Point  pl1 = GetLocal(p1);
	C3Point  pl2 = GetLocal(p2);

	if (pl1.Z * pl2.Z < 0)  return TRUE;
	else return  FALSE;
}

C3Point CPlate:: FindLocalCross(C3Point p1, C3Point p2) //seek  CrossPoint if  IsCrossed = TRUE
{
	C3Point Plocal;
	Plocal.Z = 0; // set for cross-point
	C3Point  pl1 = GetLocal(p1);
	C3Point  pl2 = GetLocal(p2);
	Plocal.X =  pl1.X - pl1.Z * (pl2.X - pl1.X) / (pl2.Z - pl1.Z);
	Plocal.Y =  pl1.Y - pl1.Z * (pl2.Y - pl1.Y) / (pl2.Z - pl1.Z);
	return Plocal;
}



C3Point CPlate:: ProectCorners(CPlate * base, C3Point & p0, C3Point & p1, C3Point & p2, C3Point & p3)
{
	C3Point Size;
	p0 = base->GetLocal(Corn[0]); 
	p1 = base->GetLocal(Corn[1]);
	p2 = base->GetLocal(Corn[3]); 
	p3 = base->GetLocal(Corn[2]);
	Size.X = Max(fabs(p3.X - p0.X), fabs(p1.X - p0.X));
	Size.Y = Max(fabs(p2.Y - p0.Y), fabs(p3.Y - p0.Y));
	Size.Z = 0;
	return Size;

}
BOOL CPlate:: WithinPoly(C3Point P, C3Point p1, C3Point p2, C3Point p3, C3Point p4)
{
/*	double pr1 = ScalProd(P, v1);
	double pr2 = ScalProd(P, v2);
	if (pr1 >= 0 && pr1 <= ModVect(v1) && pr2 >= 0 && pr2 <= ModVect(v2)) return TRUE;
	else return FALSE;*/
	double eps = -1.e-6;
	C3Point v1, v2, v3, v4; // sides 
	C3Point r1, r2, r3, r4; // 
	C3Point vr1, vr2, vr3, vr4;
	v1 = p2 - p1; v2 = p3 - p2;
	v3 = p4 - p3; v4 = p1 - p4;
	r1 = P - p1; r2 = P - p2;
	r3 = P - p3; r4 = P - p4;
	vr1 = VectProd(v1, r1); vr2 = VectProd(v2, r2);
	vr3 = VectProd(v3, r3); vr4 = VectProd(v4, r4); 
	if (vr1.Z * vr2.Z > eps && vr2.Z * vr3.Z > eps  && vr1.Z * vr3.Z > eps  
		&& vr3.Z * vr4.Z > eps && vr4.Z * vr1.Z > eps && vr2.Z * vr4.Z > eps)
		return TRUE;
	else return FALSE;

}

BOOL CPlate:: WithinPoly(C3Point Ploc)
{
	C3Point p1, p2, p3, p4;
	double eps = 1.e-20;
	p1 = GetLocal(Corn[0]);
	p2 = GetLocal(Corn[1]); 
	p3 = GetLocal(Corn[2]); 
	p4 = GetLocal(Corn[3]);
	C3Point v1, v2, v3, v4; // sides 
	C3Point r1, r2, r3, r4; // 
	C3Point vr1, vr2, vr3, vr4;
	v1 = p2 - p1; v2 = p3 - p2;
	v3 = p4 - p3; v4 = p1 - p4;
	r1 = Ploc - p1; r2 = Ploc - p2;
	r3 = Ploc - p3; r4 = Ploc - p4;
	vr1 = VectProd(v1, r1); vr2 = VectProd(v2, r2);
	vr3 = VectProd(v3, r3); vr4 = VectProd(v4, r4); 
	if (vr1.Z * vr2.Z > eps  && vr2.Z * vr3.Z > eps && vr1.Z * vr3.Z > eps
		&& vr3.Z * vr4.Z > eps && vr4.Z*vr1.Z > eps && vr2.Z * vr4.Z > eps)
		return TRUE;
	else return FALSE;

}

void  CPlate:: CoverByParticle(double power, double diffY, double diffZ, 
							   C3Point Ploc, C3Point DimOrtH, C3Point DimOrtV, 
							   C3Point Vel, C3Point dVH, C3Point dVV)
{
/*	Load->Distribute(Ploc.X, Ploc.Y, power);
	return;*/

	CString S;
	C3Point Pgl = GetGlobalPoint(Ploc); // particle rectangle centre
	C3Point Global[4]; // particle rectangle in global CS
	Global[0] = Pgl - DimOrtH - DimOrtV;
	Global[1] = Pgl + DimOrtH - DimOrtV;
	Global[2] = Pgl - DimOrtH + DimOrtV;
	Global[3] = Pgl + DimOrtH + DimOrtV;

	C3Point Local[4]; // particle rectangle in local CS
	int k;
	for (k = 0; k < 4; k++) Local[k] = GetLocal(Global[k]);
	
	// double V = ModVect(Vel);
	C3Point GlobVel[4]; // corners velocities
	GlobVel[0] = Vel - dVH - dVV;
	GlobVel[1] = Vel + dVH - dVV;
	GlobVel[2] = Vel - dVH + dVV;
	GlobVel[3] = Vel + dVH + dVV;
	C3Point LocVel[4];// = GetLocal(Vel);
	for (k = 0; k < 4; k++) LocVel[k] = GetLocal(GlobVel[k]);
	
//	C3Point LocVel = GetLocal(Vel);
	
	C3Point Poly[4]; // polygon on Plate Zloc = 0
	double dt;
	for (k = 0; k < 4; k++) {
		dt = - Local[k].Z / LocVel[k].Z;
		Poly[k].Z = 0;
		Poly[k].X = Local[k].X + LocVel[k].X * dt;
		Poly[k].Y = Local[k].Y + LocVel[k].Y * dt;
	}


	C3Point P;
	int i, j;

/*	for (i = 0;  i <= Load-> Nx; i++) 
	for (j = 0; j <= Load->Ny; j++) {
			Load->wh[i][j] = 0; Load->wv[i][j] = 0;
		} // j
*/
	int TotCells = 0;
	double TotWeight = 0;
	C3Point PP0;
	double modP, mod1, mod2, mod3, mod4, ratio;
	double x, y,  x1, y1, x2, y2, x3, y3, x4, y4,  xa, xb;
	C3Point v01, v02, v13, v23; //
	
	v01 = Poly[1] - Poly[0];//VectSum(Poly[1], Poly[0], 1, -1);
	v02 = Poly[2] - Poly[0];//VectSum(Poly[2], Poly[0], 1, -1);
	v13 = Poly[3] - Poly[1];
	v23 = Poly[3] - Poly[2];

	mod1 = ModVect(v01);
	mod2 = ModVect(v02);
	mod3 = ModVect(v13);
	mod4 = ModVect(v23);

	x1 = ScalProd(v01, v02/ mod2) ;
	y1 = sqrt(mod1*mod1 - x1*x1);
	x2 = ScalProd(v02, v01/ mod1) ;
	y2 = sqrt(mod2*mod2 - x2*x2);
	x3 = ScalProd(v13, v01/ mod1) ;
	y3 = sqrt(mod3*mod3 - x3*x3);
	x4 = ScalProd(v23, v02/ mod2) ;
	y4 = sqrt(mod4*mod4 - x4*x4);


	for (i = 0;  i <= Load-> Nx; i++) {
		P.Z = 0;
		P.X = i * Load->StepX;
		for (j = 0; j <= Load->Ny; j++) { 
			P.Y = j * Load->StepY;
			
			if (WithinPoly(P, Poly[0], Poly[1], Poly[3], Poly[2])){
				
				PP0 = P - Poly[0];
				modP = ModVect(PP0);
			//	if (modP < 1.e-16) AfxMessageBox("zero modP");
			
				// x - P0->P1
				x  = ScalProd(PP0, v01/mod1); 
				y = sqrt(modP*modP - x*x);
				
				xa = x2 * y / y2;
				xb = mod1 + x3 * y / y3;

				ratio = (x - xa) / (xb - xa);

			/*	Load->wh[i][j] = 1. - 0.5*diffY + ratio*diffY;
				if (Load->wh[i][j] < 1.e-16) Load->wh[i][j] = 0;*/
				
				// x - P0->P2
				x  = ScalProd(PP0, v02/mod2); 
				y = sqrt(modP*modP - x*x);
							
				xa = x1 * y / y1;
				xb = mod2 + x4 * y / y4;
				
				ratio = (x - xa) / (xb - xa);

			/*	Load->wv[i][j] = 1. - 0.5*diffZ + ratio*diffZ;
				if (Load->wv[i][j] < 1.e-16) Load->wv[i][j] = 0;
				TotWeight += Load->wh[i][j] * Load->wv[i][j];*/
				
				TotCells++;
			} // within poly
		} //j
	}//i

/*	if (TotWeight < 1.e-6 && Number >1) {
		S.Format("Cells %d, Weight %g, plate %d", TotCells, TotWeight, Number);
		AfxMessageBox(S);
	}*/
				
	if (TotCells <1 || TotWeight < 1.e-6) { // distribute as one point
		Load->Distribute(Ploc.X, Ploc.Y, power);
		return;

/*		S.Format("totcells = 0, plate %d", Number);
		AfxMessageBox(S);

		C3Point NewPoly[4];
		double step = Max(Load->StepX, Load->StepY); 
		C3Point dv = Poly[0] - Poly[3];//VectSum(Poly[0], Poly[3], 1, -1);
		NewPoly[0] = VectSum(Poly[0], dv, 1, step / ModVect(dv));
		NewPoly[3] = VectSum(Poly[3], dv, 1, -step / ModVect(dv));
		dv = Poly[1] - Poly[2]; //VectSum(Poly[1], Poly[2], 1, -1);
		NewPoly[1] = VectSum(Poly[1], dv, 1, step / ModVect(dv));
		NewPoly[2] = VectSum(Poly[2], dv, 1, -step / ModVect(dv));
		for (k = 0; k < 4; k++) Poly[k] = NewPoly[k];

	TotCells = 0;	TotWeight = 0;
	.....
	*/

	} // totcells < 1

	// double Cell = Load->StepX * Load->StepY; 
	// double AvCellPower = power / TotCells;
//	double power0 = power * (1 - 0.5 * diffY) * (1 - 0.5 * diffZ);
	double CellPower = 0;
	for (i = 0;  i <= Load-> Nx; i++) {
		for (j = 0; j <= Load->Ny; j++) { 
		/*	if (Load->wv[i][j] + Load->wh[i][j] < 1.e-16) continue;
			CellPower = Load->wh[i][j] * Load->wv[i][j] * power / TotWeight;*/
			
		//	Load->Val[i][j] += CellPower/Cell;
		//	Load->Sum += CellPower;//power;

			Load->Distribute(Load->StepX * i, Load->StepY * j, CellPower);
		} //j
	}//i
	

}

/*
void  CPlate:: DrawPlate(CView * pView, CDC* pDC) // show plate + recalc  RgnPlan,  RgnSide
{
	CMainView* pMV =  (CMainView* )pView;
	CPoint corn[4];
	CPoint centre, CenterPoint = (0,0);
	int k;
	CPen * pen = &pMV->BlackPen;
	pDC->SelectObject(pen);
//	pDC->SelectObject(&pMV->smallfont);
	pDC->SetTextColor(RGB(255,0,0));

	BOOL PLAN = FALSE;
 // Draw on Plan View -----------------------------------------
	 for (k = 0; k < 4; k++) {
		corn[k].x = pMV->OrigX + (int) (Corn[k].X*(pMV->ScaleX));
		corn[k].y = pMV->OrigY - (int) (Corn[k].Y*(pMV->ScaleY));
	 }
	 int shiftY = (int) (MassCentre.Y*(pMV->ScaleY));
	 for (k = 1; k < 4; k++) {
		 if (corn[k] == corn[0]) {
			 PLAN = TRUE;
		 }
		 else if (corn[0].y == corn[1].y && corn[2].y == corn[1].y){
			 PLAN = TRUE;
		 }
	 }
	 if (!Visible) 	 pDC->SelectObject(pMV->DotPen);
	 if (Loaded && PLAN)	 pDC->SelectObject(pMV->MarkPen);
	  if (Selected && PLAN)	  pDC->SelectObject(pMV->RedPen);
		 pDC->MoveTo(corn[0]);
		for (k = 1; k < 4; k++)  pDC->LineTo(corn[k]);
		pDC->LineTo(corn[0]);
		 pDC->SelectObject(pen);

//Draw on Side View ------------------------------------------------
	 PLAN = TRUE;
	 for (k = 0; k < 4; k++) {
		corn[k].y = pMV->OrigZ - (int) (Corn[k].Z*(pMV->ScaleZ));
	 }
	 int shiftZ =  (int) (MassCentre.Z*(pMV->ScaleZ));
	  for (k = 1; k < 4; k++) {
		 if (corn[k] == corn[0]) {
			 PLAN = FALSE;
		 }
		  else if (corn[0].y == corn[1].y && corn[2].y == corn[1].y){
			 PLAN = FALSE;
		 }
	 }
	
	 if (!Visible )   pDC->SelectObject(pMV->DotPen);
	 if (Loaded && !PLAN)	 pDC->SelectObject(pMV->MarkPen);
	  if (Selected && !PLAN)	  pDC->SelectObject(pMV->RedPen);
		 pDC->MoveTo(corn[0]);
		for (k = 1; k < 4; k++)  pDC->LineTo(corn[k]);
		pDC->LineTo(corn[0]);
		pDC->SelectObject(pen);

		if (OrtZ.Z < 1.e-16) PLAN = TRUE;
		else PLAN = FALSE;

		centre.x = pMV->OrigX + (int) (MassCentre.X*(pMV->ScaleX));

		if (abs(shiftY) == 0 && abs(shiftZ) != 0) PLAN = FALSE;
			// centre.y = pMV->OrigZ - shiftZ; 
		
		if (abs(shiftY) != 0 && abs(shiftZ) == 0) PLAN = TRUE;
			// centre.y = pMV->OrigY - shiftY;
	
		if (abs(shiftY) == 0 && abs(shiftZ) == 0) PLAN = TRUE;
			// centre.y = pMV->OrigY - shiftY;
		
		if (PLAN) centre.y = pMV->OrigY - shiftY;
		else centre.y = pMV->OrigZ - shiftZ; 
	
		CenterPoint = centre;
		RectMark->SetRect(CenterPoint.x -1, CenterPoint.y -2,
										CenterPoint.x +3, CenterPoint.y +2);
		pDC->Rectangle(RectMark);
		if (Loaded && pMV->SHOW_NUMBERS) {
			CString S;
			S.Format("%d", Number);
			int h = pDC->GetTextExtent(S).cy;
			pDC->TextOut(CenterPoint.x +2, CenterPoint.y - h -1, S);
		}

		pDC->SelectObject(pen);
		pDC->SetTextColor(RGB(0,0,0));
		
}
*/

void  CPlate:: DrawPlate(CView * pView, CDC* pDC) // taken from BTR_K
// show plate + recalc  RgnPlan,  RgnSide
{
	CMainView* pMV =  (CMainView* )pView;
	CPoint corn[4];
	CPoint centre, CenterPoint = (0,0);
	int k;
	CPen * pen = &pMV-> BlackPen;
	pDC->SelectObject(pen);
	
//	if(pDC->m_hAttribDC != NULL) pDC->SelectObject(&pMV->font);
//	else 
	pDC->SelectObject(&pMV->smallfont);

	pDC->SetTextColor(RGB(250,0,0));

	BOOL PLAN; // = FALSE;

/*	if (fabs(OrtZ.Z) < 0.5) PLAN = TRUE; // ortZ lays in XY plan
	else PLAN = FALSE;
*/
	switch (Fixed) {
		case -1:	if (fabs(OrtZ.Z) < fabs(OrtZ.Y)) PLAN = TRUE;
					else PLAN = FALSE; break;
		case 0: PLAN = TRUE; break;
		case 1: PLAN = FALSE; break;
		default: PLAN = FALSE;
	}

 // Draw on Plan View -----------------------------------------
	 for (k = 0; k < 4; k++) {
		corn[k].x = pMV->OrigX + (int) (Corn[k].X*(pMV->ScaleX));
		corn[k].y = pMV->OrigY - (int) (Corn[k].Y*(pMV->ScaleY));
	 }
	 int shiftY = (int) (MassCentre.Y*(pMV->ScaleY));
//	 if (fabs(OrtZ.Y) >= 0.5 || fabs(MassCentre.Y) > 1.e-6) PLAN = TRUE;
	 
	
	 if (!Visible) 	
		 //if(pDC->m_hAttribDC != NULL) 
			 pDC->SelectObject(&pMV->DotPen);
	 if (Loaded && Touched)// && PLAN
		 //if(pDC->m_hAttribDC != NULL)  
			pDC->SelectObject(&pMV->MarkPen);
	  if (Selected) // && PLAN)	
		 // if(pDC->m_hAttribDC != NULL)  
			  pDC->SelectObject(&pMV->RedPen);
		 pDC->MoveTo(corn[0]);
		for (k = 1; k < 4; k++)  pDC->LineTo(corn[k]);
		pDC->LineTo(corn[0]);
		 pDC->SelectObject(pen);

//Draw on Side View ------------------------------------------------
	 for (k = 0; k < 4; k++) {
		corn[k].y = pMV->OrigZ - (int) (Corn[k].Z*(pMV->ScaleZ));
	 }
	 int shiftZ =  (int) (MassCentre.Z*(pMV->ScaleZ));
//	 if (fabs(MassCentre.Y) < 1.e-6 && fabs(MassCentre.Z) > 1.e-6) PLAN = FALSE;
	
	 if (!Visible ) 
		 //if(pDC->m_hAttribDC != NULL) 
			 pDC->SelectObject(&pMV->DotPen);
	 if (Loaded && Touched) // && !PLAN
		 //if(pDC->m_hAttribDC != NULL)  
		 pDC->SelectObject(&pMV->MarkPen);
	  if (Selected)// && !PLAN)	
		 // if(pDC->m_hAttribDC != NULL)
		 pDC->SelectObject(&pMV->RedPen);
		
	  pDC->MoveTo(corn[0]);
		for (k = 1; k < 4; k++)  pDC->LineTo(corn[k]);
		pDC->LineTo(corn[0]);
		pDC->SelectObject(pen);

	
		centre.x = pMV->OrigX + (int) (MassCentre.X*(pMV->ScaleX));

	
		if (PLAN) centre.y = pMV->OrigY - shiftY;
		else centre.y = pMV->OrigZ - shiftZ; 
	
		//if (Touched && !Loaded) pDC->SelectObject(pMV->RedPen);
		if (Touched && Loaded) pDC->SelectObject(pMV->RedPen);

		CenterPoint = centre;
		RectMark->SetRect(CenterPoint.x -1, CenterPoint.y -2, CenterPoint.x +3, CenterPoint.y +2);
		
		if (pMV->SHOW_NUMBERS) pDC->Rectangle(RectMark);
		if (Loaded && Touched) pDC->SetTextColor(RGB(255, 0, 0));
		else pDC->SetTextColor(RGB(0,100,100));

		if (pMV->SHOW_NUMBERS && Loaded && Touched) {
			//pDC->Rectangle(RectMark);
			CString S;
			S.Format("%d", Number);
			int h;
			if(pDC->m_hAttribDC != NULL) 
				h = pDC->GetTextExtent(S).cy; // h = pDC->GetTextExtent(S).cy;
			else h = 15;
			pDC->TextOut(CenterPoint.x +2, CenterPoint.y - h -1, S);
		}

		pDC->SelectObject(pen);
		pDC->SetTextColor(RGB(0,0,0));
		
}

void  CPlate::ApplyLoad(BOOL flag,  double Hx, double Hy) 
// flag false => clear existing; true -> create or replace
{
	//CLoad * pOldLoad = Load;
	int i;
	double hx = Hx, hy = Hy;
	BOOL OptGrid = FALSE;
	BOOL TargetPlane = FALSE;
	C3Point H;
//	delete Load;
//	delete SmLoad;
	
	if (fabs(Hx*Hy) < 1.e-10)  // -> set 20 steps
	{
		OptGrid = TRUE;
		H = GetMantOrder(Xmax * 0.05);
		hx = H.X * H.Z;
		H = GetMantOrder(Ymax * 0.05);
		hy = H.X * H.Z;
		//hx = 0.001;// DefStepX2; 
		//hy = 0.001;//DefStepY2;
		
		if (fabs(OrtX.X) < 1.e-6) // Cross-planes -> show on planeView
			TargetPlane = TRUE;
	
	} // 20 steps

	if (!flag && Load == NULL) {
		Loaded = FALSE; 
	//	Number = 0;
		return;
	}
	if (!flag && Load != NULL) { // delete existing Load
	
		delete Load;
		if (SmLoad != NULL) 
			delete SmLoad;

		Load = NULL;
		SmLoad = NULL;
		Loaded = FALSE;
	//	Number = 0;
		return;
	}

	
	if (flag && !Loaded) { //Load == NULL) { // create new load
		if (OptGrid) Load = new CLoad(Xmax, Ymax);//, TargetPlane, MassCentre.X);
		else Load = new CLoad(Xmax, Ymax, hx, hy);
		SmLoad = NULL;
		//SmLoad = new CLoad(Xmax, Ymax, hx, hy);
		Loaded = TRUE; // indicates that load-array exists
		SmoothDegree = 0;
		Load->Comment = Comment;
		Load->Sum = 0;
	
		return;
	}
	if (flag && Loaded) { // delete old, create new
	
		delete Load;
		if (SmLoad != NULL) delete SmLoad;
		
		Load = new CLoad(Xmax, Ymax, hx, hy);
		//SmLoad = new CLoad(Xmax, Ymax, hx, hy);
		SmLoad = NULL;
		Loaded = TRUE;
		SmoothDegree = 0;
		Load->Comment = Comment;
		Load->Sum = 0;
	
		return;
	}
}

void  CPlate :: SetSmoothLoad()// not called
{
//	if (SmLoad->SmoothDegree == SmoothDegree) return; // smoothed already for this degree
//	else {
		CWaitCursor wait;
		double hx = Load->StepX;
		double hy = Load->StepY;
		if (SmLoad != NULL) SmLoad->Clear();
		else SmLoad = new CLoad(Xmax, Ymax, hx, hy);
		SmLoad->Copy(Load);
		SmLoad->SmoothLoad(SmoothDegree);
		SmLoad->SetSumMax();
		SmLoad->Comment = Comment;
//	}
}

void  CPlate:: CorrectLoad()// clear extra spots at the area exit plane - not used now
// not used now
{
	if (!Loaded) return;
	
	C3Point Ploc, Pgl;
	if (Orig.X <= 25) return;
	else {
		for (int i = 0; i <= Load->Nx; i++) 
			for (int j = 0; j <= Load->Ny; j++) {
				Ploc.X = i*Load->StepX;
				Ploc.Y = j*Load->StepY;
				Ploc.Z = 0;
				Pgl = GetGlobalPoint(Ploc); 
				if (Pgl.Y < DuctExitYmin || Pgl.Y > DuctExitYmax || Pgl.Z < DuctExitZmin || Pgl.Z > DuctExitZmax)
					Load->Val[i][j] = 0;
			} // for
	} // X > 23m
}

void  CPlate :: ShowLoad()
{
	CLoadView * pLV = (CLoadView *) pLoadView;
	if (!Loaded) 
		pSetView ->Load = NULL;
	pLV->SetPlate(this);
	pLV->ShowLoad = TRUE;
	if (Loaded)	{
		//CorrectLoad();
		pLV->ShowLoad = TRUE;
		pLV->SetLoad_Plate(Load, this);
		if (Load->MaxVal > 1.e-10) 
		pSetView->SetLoad_Plate(Load, this);
	}
	else {
		pLV->SetLoad_Plate(Load, this);
		pSetView->SetLoad_Plate(Load, this);
		//pLV->ShowLoad = FALSE;
	}

	pLV->InvalidateRect(NULL, TRUE);
//	pSetView->InvalidateRect(NULL, TRUE);
}

void  CPlate :: ShowEmptyLoad() //
{
	CLoadView * pLV = (CLoadView *) pLoadView;
	//pSetView ->Load = NULL;
	//pLoadView->Load = NULL;
	pLoadView->ShowLoad = TRUE; // NULL;
	//pSetView->Invalidate(TRUE);
	//pLV->Invalidate(TRUE);
	
	pLV->SetPlate(this);

	CDC* pDC = pLV->GetDC();
	CRect rect;
	pLV->GetClientRect(rect);
	pDC->Rectangle(rect);// clear background
	//pLV->Xmax = Xmax; pLV->Ymax = Ymax; 
	pLV->UpdateScales(rect);// , Xmax, Ymax);//NEW
	int xmin = pLV->OrigX;
	int ymin = pLV->OrigY;
	double DX = (pLV->Xmax - pLV->Xmin) * pLV->ScaleX;
	double DY = (pLV->Ymax - pLV->Ymin) * pLV->ScaleY;
	int xmax = xmin +(int)DX;
	int ymax = ymin -(int)DY;

	CBrush brush;
	CBrush * oldbrush;
	brush.CreateSolidBrush(RGB(240,240,240)); 
	oldbrush = pDC->SelectObject(&brush);
	pDC->Rectangle(xmin, ymin, xmax, ymax);
	pDC->SelectObject(oldbrush);

	CFont* pOldFont = pDC->SelectObject(&pLV->bigfont);
	//int w = pDC->GetTextExtent(S).cx;
	//pDC->SetTextColor(RGB(255, 0, 0));
	//pDC->TextOut(xmin +10, 10, Comment + " - Particles spots");
	pDC->SetTextColor(RGB(0, 0, 0));
	pDC->TextOut(xmin +10, 10, Comment);
	pDC->SelectObject(pOldFont);
	
	pLV->ReleaseDC(pDC);
	//AfxMessageBox("ShowEmptyLoad done");
}

void CPlate :: DrawLoadRect() //NEW: called from MainView LBdown
{
	CLoadView * pLV = (CLoadView *) pLoadView;
	CDC * pDC = pLV->GetDC();
	
	pLV->ShowLocalScale(pDC); //pLV->ShowGlobalScale(pDC);// to change!
	//pLV->ShowLabels(pDC);// isolines values
		
	pLV->ReleaseDC(pDC);
}

void CPlate ::  DrawPlateBound()//NEW:polygon, called from MainView LBdown
{
	CLoadView * pLV = (CLoadView *) pLoadView;
	CDC * pDC = pLV->GetDC();
	int OrigX, OrigY;
	double ScaleX, ScaleY;
	//CRect rect;
	//pLV->GetClientRect(rect);
	//pLV->UpdateScales(rect, Xmax, Ymax);// NEW
	
	CPen thickpen;
	//pen.CreatePen(PS_SOLID, 1, RGB(200,200,200));
	//CPen * pOldPen = pDC->SelectObject(&pen);
	thickpen.CreatePen(PS_DOT, 3, RGB(150,150,0));
	CPen * pOldpen = pDC->SelectObject(&thickpen);
	//pLV->ShowPlateBound(pDC);
	OrigX = pLV->OrigX;
	OrigY = pLV->OrigY;
	ScaleX = pLV->ScaleX;
	ScaleY = pLV->ScaleY;
	C3Point Local, Global;
	Global = Corn[0];
	Local = GetLocal(Global);
	pDC->MoveTo(OrigX + (int)(Local.X * ScaleX), OrigY - (int)(Local.Y * ScaleY));
	for (int k = 1; k < 4; k++) {
		Global = Corn[k];
		Local = GetLocal(Global);
		pDC->LineTo(OrigX + (int)(Local.X * ScaleX), OrigY - (int)(Local.Y * ScaleY));
	} // k
	Global = Corn[0];
	Local = GetLocal(Global);
	pDC->LineTo(OrigX + (int)(Local.X * ScaleX), OrigY - (int)(Local.Y * ScaleY));

	pDC->SelectObject(pOldpen);
	pLV->ReleaseDC(pDC);
}

void  CPlate :: ShowLoadState() // show info
{
	CLoadView * pLV = (CLoadView *) pLoadView;
	//pSetView ->Load = NULL;
	pLoadView->Load = NULL;
	pLoadView->ShowLoad = NULL;

	CDC* pDC = pLV->GetDC();
	CString S;
	C3Point Ploc, Pgl;
//	pLV->InvalidateRect(NULL, TRUE);
	CRect rect;
	pLV->GetClientRect(rect);
	pDC->Rectangle(rect);
	CFont * pOldFont = pDC->SelectObject(&pLV->smallfont);
	pDC->SetTextColor(RGB(100,0,0));
	if (!Loaded)  {
		if (Selected) S.Format(" - selected");
		else S.Format(" - unselected");
		pDC->TextOut(10,20, Comment + S); 
		pSetView->Load = NULL;
		pSetView->Invalidate(TRUE);
	}

	else {	//(Loaded)
		CLoad * L = Load; 
		/*if (L->MaxVal > 1.e-10) {
				pSetView->SetLoad_Plate(L, this);
		}
		else pSetView->SetLoad_Plate(NULL, NULL);
	*/
			
		S.Format( " (AS %d)", Number);
		pDC->TextOut(10,10, Comment + S); 
		pDC->SetTextColor(RGB(0,0,0));
		S.Format("Nx = %d  Ny = %d", L->Nx, L->Ny);
		pDC->TextOut(10, 30, S);
		S.Format("StepX = %g m  StepY =%g m", L->StepX, L->StepY);
		pDC->TextOut(10, 50, S);
		S.Format("TOTAL POWER = %g W (%s)", L->Sum, Load->Particles);
		pDC->SetTextColor(RGB(255,0,255));
		pDC->TextOut(10, 70, S);
		pDC->SetTextColor(RGB(255,0,0));
		S.Format("MAX = %g W/m2",  L->MaxVal);
		pDC->TextOut(10, 90, S);
		pDC->SetTextColor(RGB(0,0,0));
		Ploc = C3Point(L->iMaxVal * L->StepX, L->jMaxVal * L->StepY, 0);
		Pgl = GetGlobalPoint(Ploc);
		S.Format("Peak position, m:"); pDC->TextOut(10, 110, S);
		S.Format("LOCAL CS (%g, %g)", L->iMaxVal * L->StepX,  L->jMaxVal * L->StepY); 
		pDC->TextOut(10, 130, S);
		S.Format("GLOBAL CS (%g, %g, %g)",  Pgl.X, Pgl.Y, Pgl.Z);
		pDC->TextOut(10, 150, S);
			
		pDC->SetTextColor(RGB(0,200,0));
		S.Format("Orig, m:(%g, %g, %g) ",  Orig.X, Orig.Y, Orig.Z);
		pDC->TextOut(10, 170, S);
		S.Format("OrtX, m: (%g, %g, %g)",  OrtX.X, OrtX.Y, OrtX.Z);
		pDC->TextOut(10, 190, S);
		S.Format("OrtY, m: (%g,  %g,  %g) ",  OrtY.X, OrtY.Y, OrtY.Z);
		pDC->TextOut(10, 210, S);
	
		pDC->SetTextColor(RGB(0,0,0));
		//Ploc = C3Point(L->iMaxBound * L->StepX, 0, 0);
		//Pgl =  GetGlobalPoint(Ploc);
		//S.Format("Downstream Edge Position, m: X = %g m (%g m in Global CS)", L->iMaxBound * L->StepX, Pgl.X);
		//C3Point P0 = GetLocal(Corn[0]);
		//S.Format("Origin shift x,y:   %g, %g", -P0.X, -P0.Y); 
		
		S.Format("CORN, m: [0] (%g,%g,%g)  [1] (%g,%g,%g)  [2] (%g,%g,%g)  [3] (%g,%g,%g)", Corn[0].X, Corn[0].Y, Corn[0].Z,
			Corn[1].X, Corn[1].Y, Corn[1].Z, Corn[2].X, Corn[2].Y, Corn[2].Z, Corn[3].X, Corn[3].Y, Corn[3].Z);
		pDC->TextOut(10, 230, S);
		S.Format("Smooth = %d", SmoothDegree);// L->Smoothdegree
		pDC->TextOut(10, 250, S);
	
		//pDoc->ShowProfiles = TRUE;
		//pSetView->InvalidateRect(NULL, TRUE);
		pLV->ReleaseDC(pDC);

	}

//	pSetView->InvalidateRect(NULL, TRUE);
	
	pDC->SelectObject(pOldFont);
	pLV->ReleaseDC(pDC);

}

void CPlate::  WriteLoadAdd(FILE * fout)
{
	CString sol;
	C3Point P;
		if (Solid) sol = " (solid)";
		else sol = " (transpar)";
		fprintf(fout, "# %s\n", "Free Surf" + sol);
		for (int i = 0; i < 4; i++) {
			P = Corn[i];
			fprintf(fout, "%g    %g    %g\n", P.X, P.Y, P.Z);
		}

		if (Loaded && Touched) WriteLoad(fout);// non zero load
		//if (Loaded && Load->MaxVal > 1.e-6)  // write standard
		else { // write only header
			CString S;
			S.Format(" \t\t PARAMETERS  (Plate  %d)  \n", Number);
			fprintf(fout, Comment + S); 
		}
}

void  CPlate::CorrectLoad(double Coeff)
{
	//CLoad * L = Load;
	for (int i = 0; i <= Load->Nx; i++) {
		for (int j = 0; j <= Load->Ny; j++) {
			//double x = i*Load->StepX;
			//double y = j*Load->StepY;
			double oldval = Load->Val[i][j];
			Load->Val[i][j] = oldval * Coeff;
		} // i.j
	}

	double sum = Load->Sum;
	Load->Sum = sum * Coeff;
	double maxval = Load->MaxVal;
	Load->MaxVal = maxval * Coeff;

}

void  CPlate::AddLoads(CLoad * L1, CLoad * L2, CLoad * L3)
{
	for (int i = 0; i <= Load->Nx; i++) {
		for (int j = 0; j <= Load->Ny; j++) {
			Load->Val[i][j] = L1->Val[i][j] + L2->Val[i][j] + L3->Val[i][j];
		} // i.j
	}
	Load->SetSumMax();
}

void  CPlate:: WriteLoad(FILE * fout)
{
	CLoad * L; 
	if (SmoothDegree == 0) L = Load;
	else  {
		L = SmLoad;
//		SetSmoothLoad();
	}
	//L->SetSumMax();
	CString S;
	S.Format(" \t\t PARAMETERS  (Plate  %d)  \n", Number);
	fprintf(fout, Comment + S);  
	S.Format("\tNx = %d  Ny = %d \n", L->Nx, L->Ny);
	fprintf(fout, S);
	S.Format("\tStepX = %f m  StepY = %f m \n", L->StepX, L->StepY);
	fprintf(fout, S);
	S.Format("\tSumma = %le W  (%s)\n ", L->Sum, Load->Particles);
	fprintf(fout, S);
	S.Format("\tMaxLoad = %le W/m2 ",  L->MaxVal);
	fprintf(fout, S);
	S.Format("\t at X = %4.3f m Y = %4.3f m  \n",  
				Load->iMaxVal * L->StepX,  L->jMaxVal * L->StepY);
	fprintf(fout, S);
	S.Format("\tOrigin at  X = %f m Y = %f m Z = %f m \n ",  Orig.X, Orig.Y, Orig.Z);
	fprintf(fout, S);
	S.Format("\tOrtX  X = %f  Y = %f  Z = %f  \n ",  OrtX.X, OrtX.Y, OrtX.Z);
	fprintf(fout, S);
	S.Format("\tOrtY  X = %f  Y = %f  Z = %f  \n",  OrtY.X, OrtY.Y, OrtY.Z);
	fprintf(fout, S);
	S.Format("\tSmoothing Degree  =  %d \n", L->SmoothDegree);
	fprintf(fout, S);
	S.Format("\tDownstream Edge X = %g m \n", L->iMaxBound * L->StepX);
	fprintf(fout, S);
	fprintf(fout,"\n \t\t POWER DEPOSITION on the Plate \n");  
	fprintf(fout," \t X,m \t\t Y,m \t\t  Load, W/m2 \n");  
	for (int i = 0; i <= L->Nx; i++) 
		for (int j = 0; j <= L->Ny; j++) {
			double x = i*L->StepX;
			double y = j*L->StepY;
			double load = L->Val[i][j];
			fprintf(fout, "\t%-8.5f \t %-8.5f \t %-12.3le \n", x,y,load);
		} // i.j
}

void CPlate:: WriteProfiles()
{
	CString  sn, name;
	FILE * fout;
	double load;
	int i, j;
	C3Point Ploc, Pgl;
	sn.Format("%d", Number);
	double Cell = Load->StepX * Load->StepY;
	BOOL INTEGRAL = Load->iProf < 0 || Load->jProf < 0;

// HORIZONTAL -------------------------------------------------	
	if (!INTEGRAL) { // PD profiles
		name = "prof" + sn + "_hor" + ".txt"; // horizontal
		fout = fopen(name, "w");
		fprintf(fout," Horizontal PD profile on Surf %d \n\n", Number);
		fprintf(fout," \t X\t Y\t Z\t,m  \t  PD, W/m2 \n\n");
		Ploc.Y = Load->jProf * Load->StepY;
		Ploc.Z = 0;
		for (i = 0; i <= Load->Nx; i++) {
			Ploc.X = i * Load->StepX;
			load = Load->Val[i][Load->jProf];
			Pgl = GetGlobalPoint(Ploc);
			fprintf(fout, " \t %g \t %g \t %g\t %g \n", Pgl.X, Pgl.Y, Pgl.Z, load);
			//fprintf(fout, " \t %-7.3f \t %-7.3f \t %-7.3f\t %-12.3le \n", Pgl.X, Pgl.Y, Pgl.Z, load);
		}
	}

	else { // INTEGRAL power
		name = "intprof" + sn + "_hor" + ".txt"; // horizontal
		fout = fopen(name, "w");
		fprintf(fout," Integrated power along Surf %d - horizontal \n\n", Number);
		fprintf(fout," \t X\t Y\t Z\t,m  \t  Power, W \n\n"); 
		Ploc.Y = 0;
		Ploc.Z = 0;
		for (i = 0; i <= Load->Nx; i++) {
			Ploc.X = i * Load->StepX;
			load = 0;
			for (j = 0; j <= Load->Ny; j++) load += Load->Val[i][j] * Cell;
			Pgl = GetGlobalPoint(Ploc);
			fprintf(fout, " \t %g \t %g \t %g\t %g \n", Pgl.X, Pgl.Y, Pgl.Z, load);
			//fprintf(fout, " \t %-7.3f \t %-7.3f \t %-7.3f\t %-12.3le \n", Pgl.X, Pgl.Y, Pgl.Z, load);
		}
	}
	
	fclose(fout);
	AfxMessageBox("Horizontal profile is written in file: \n" + name); 

// VERTICAL ----------------------------------------------------------
	if (!INTEGRAL) { // PD profiles
		name = "prof" + sn + "_vert" + ".txt"; // horizontal
		fout = fopen(name, "w");
		fprintf(fout," Vertical PD profile on Surf %d \n\n", Number);
		fprintf(fout," \t X\t Y\t Z\t,m  \t  PD, W/m2 \n\n");
		Ploc.X = Load->iProf * Load->StepX;
		Ploc.Z = 0;
		for (j = 0; j <= Load->Ny; j++) {
			Ploc.Y = j * Load->StepY;
			load = Load->Val[Load->iProf][j];
			Pgl = GetGlobalPoint(Ploc);
			fprintf(fout, " \t %g \t %g \t %g\t %g \n", Pgl.X, Pgl.Y, Pgl.Z, load);
			//fprintf(fout, " \t %-7.3f \t %-7.3f \t %-7.3f\t %-12.3le \n", Pgl.X, Pgl.Y, Pgl.Z, load);
		}
	}

	else { // INTEGRAL power
		name = "intprof" + sn + "_vert" + ".txt"; // horizontal
		fout = fopen(name, "w");
		fprintf(fout," Integrated power along Surf %d - vertical \n\n", Number);
		fprintf(fout," \t X\t Y\t Z\t,m  \t  Power, W \n\n"); 
		Ploc.X = 0;
		Ploc.Z = 0;
		for (j = 0; j <= Load->Ny; j++) {
			Ploc.Y = j * Load->StepY;
			load = 0;
			for (i = 0; i <= Load->Nx; i++) load += Load->Val[i][j] * Cell;
			Pgl = GetGlobalPoint(Ploc);
			fprintf(fout, " \t %g \t %g \t %g\t %g \n", Pgl.X, Pgl.Y, Pgl.Z, load);
			//fprintf(fout, " \t %-7.3f \t %-7.3f \t %-7.3f\t %-12.3le \n", Pgl.X, Pgl.Y, Pgl.Z, load);
		}
	}
	
	fclose(fout);
	AfxMessageBox("Vertical profile is written in file: \n" + name); 
	
	
	
}
void CPlate :: InitAngleArray(double min, double max, double step)// mrad
{
	AngularProfile.RemoveAll();
	double AngleMin = 0;
	if (min > 1.e-12) AngleMin = min;
	double AngleMax = PI * 500; // mrad
	if (max > 1.e-12) AngleMax = max;
	double AngleStep = 1;
	if (step > 1.e-12) AngleStep = step;
	
	int kmax = (int)((AngleMax - AngleMin) / AngleStep);
	for (int k = 0; k <= kmax; k++) {
		AngularProfile.Add(0);
	}
	StepAngle = AngleStep;// mrad
	MinAngle = AngleMin;// mrad
	MaxAngle = AngleMax;// mrad
}

void CPlate :: DistributeAngle(double angle, double power) // mrad
{
	if (angle < MinAngle || angle > MaxAngle) return;
	int k = (int)((angle - MinAngle)/StepAngle);
	AngularProfile[k] += power;
}


int CPlate::CommonCorn(CPlate * plate)
{
	int n = 0; //common corners
	C3Point p, p0;
	double eps = 1.e-7;//DBL_EPSILON
	for (int i = 0; i < 4; i++) {
		p0 = Corn[i];
		for (int j = 0; j < 4; j++) {
			p = plate->Corn[j];
			if (GetDistBetween(p, p0) < eps) 
				n++;
		}
	}
	return n;
}

int CPlate::CheckPoints(C3Point p0, C3Point p1, C3Point p2, C3Point p3)
{
	int n = 0; //common corners
	//Corn[0] = p0; 
	Corn[1] = p1;	Corn[2] = p2; Corn[3] = p3;

	C3Point p;
	
	double eps = 1.e-7;//DBL_EPSILON
	for (int i = 1; i < 4; i++) {
		p = Corn[i];
		if (GetDistBetween(p, p0) < eps) n++; // cirners coinside
	}
	return n;
} 

int CPlate::Errors()
{
	int n = 0; //common corners
	C3Point p, p0 = Corn[0];
	double eps = 1.e-7;//DBL_EPSILON
	for (int i = 1; i < 4; i++) {
		p = Corn[i];
		if (GetDistBetween(p, p0) < eps) n++; // corners coinside
	}
	return n;
}

double CPlate::DistCorn(CPlate * plate, double tol) // find sum dist to plate corners
{
	/*for (int i = 0; i < 4; i++) {
		p0 = Corn[i];
		double mindist = 1000; // find closest corn to p0
		for (int j = 0; j < 4; j++) {
			p = plate->Corn[j];
			mindist = min(mindist, GetDistBetween(p, p0));
		}
		if (mindist > tol) return 1000; // too far
		else dist4 += mindist; // add to sum dist
	}*/

	// find sumdist of plate corners to this Plate 
	double toofar = 1000.0;
	double dist;
	C3Point p, p0;
	double dist4 = 0; // sum dist between corn pairs
	double mindist = 1000; // find closest corn to p0
	for (int j = 0; j < 4; j++) {
		p = plate->Corn[j];
		p0 = GetLocal(p);
		dist = fabs(p0.Z);// dist of corn to Plate
		if (dist > tol) return toofar; // too far
		if (!WithinPoly(p0)) return toofar;
		mindist = min(mindist, dist); // not used yet
		dist4 += dist; // add to sum dist
	}
		
	return dist4;
}

//---------------------------------------------------------------------------------------
void ShowNothing()
{
	CLoadView * pLV = (CLoadView *) pLoadView;
	CDC* pDC = pLV->GetDC();
	pLV->Load = NULL;
	pLV->ShowLoad = FALSE;
	pLV->InvalidateRect(NULL, TRUE);
	pLV->ReleaseDC(pDC);
}

//--------  Common Functions ----------------------------------------------------------
/*C3Point GetMantOrder(double Value) // return integer, decimal parts and exp order
{
	C3Point P;
	CString S;
	int pos, pos1;
	CString S1, S2, S3;
	double mant = 0, order = 0;
	double integer = 0, decimal = 0;
	double intval, decval;
	
	S1.Format("%8.1e", Value);
	pos = S1.Find("e",0);
	pos1 = S1.Find(".",0);
	if (pos>0) S = "1.0" + S1.Mid(pos);
	order = atof(S);
	if (pos>0) S = S1.Left(pos);
	mant = atof(S);

	 S2 = S1.Left(pos1);
	 intval = atof(S2);
	 S3 = S1.Mid(pos1, 1);
	 decval = atof(S3);

	 integer = intval;
	 decimal = decval;
	 
	// if (decval > 5) { integer += 1.; decimal = 0.;  }
	// if (decval > 0 && decval < 5) decimal = 5;
	 
	 if (mant > 1)   { integer = 1; decimal = 0.5; }
	 if (mant > 1.5) { integer = 2; decimal = 0.; }
	 if (mant > 2)   { integer = 2; decimal = 0.5; }
	 if (mant > 2.5) { integer = 3; decimal = 0.; }
	 if (mant > 3)   { integer = 5; decimal = 0.; }
	 if (mant > 5)   { integer = 10; decimal = 0.; }


	 P.X = integer;
	 P.Y = decimal;
	 P.Z = order;
	 return P;
}
*/
C3Point GetMantOrder(double Value) // return integer, decimal parts and exp order
{
	C3Point P;
	CString S;
	int pos, pos1;
	CString S1, S2, S3;
	double mant = 0, order = 0;
	double integer = 0, decimal = 0;
	double intval, decval;
	
	S1.Format("%8.2e", Value);
	pos = S1.Find("e",0);
	pos1 = S1.Find(".",0);
	if (pos>0) S = "1.0" + S1.Mid(pos);
	order = atof(S);
	if (pos>0) S = S1.Left(pos);
	mant = atof(S);

	 S2 = S1.Left(pos1);
	 intval = atof(S2);
	 S3 = S1.Mid(pos1, 1);
	 decval = atof(S3);

	 integer = intval;
	 decimal = decval;
	 
	// if (decval > 5) { integer += 1.; decimal = 0.;  }
	// if (decval > 0 && decval < 5) decimal = 5;
	 
	 if (mant > 1)   { integer = 1; decimal = 0.5; }
	 if (mant > 1.5) { integer = 2; decimal = 0.; }
	 if (mant > 2)   { integer = 2; decimal = 0.5; }
	 if (mant > 2.5) { integer = 3; decimal = 0.; }
	 if (mant > 3)   { integer = 5; decimal = 0.; }
	 if (mant > 5)   { integer = 7; decimal = 0.; }
	 if (mant > 7)   { integer = 10; decimal = 0.;}

	 P.X = integer;
	 P.Y = decimal;
	 P.Z = order;
	 return P;
}
double ModVect(const C3Point vect)
{
	return sqrt(vect.X*vect.X + vect.Y*vect.Y +  vect.Z*vect.Z); 
}

double  ScalProd(const C3Point v1, const C3Point v2)
{
	return (v1.X*v2.X + v1.Y*v2.Y + v1.Z*v2.Z);
}

C3Point VectProd(const C3Point v1, const C3Point v2)
{
	return C3Point(
		v1.Y*v2.Z - v1.Z*v2.Y,
		v1.Z*v2.X - v1.X*v2.Z,
		v1.X*v2.Y - v1.Y*v2.X);
}

double GetDistBetween(C3Point P1, C3Point P2)
{
	C3Point d;
	double dist;
	d.X = P2.X - P1.X;
	d.Y = P2.Y - P1.Y;
	d.Z = P2.Z - P1.Z;
	dist =  sqrt(d.X*d.X + d.Y*d.Y +  d.Z*d.Z);
	return dist;
}

C3Point  VectSum(C3Point v1, C3Point v2, int c1, int c2)
{
	C3Point Pvect;
	Pvect.X = c1*v1.X + c2*v2.X;
	Pvect.Y = c1*v1.Y + c2*v2.Y;
	Pvect.Z =  c1*v1.Z + c2*v2.Z;
	return Pvect;
}

C3Point  GetLocalVect(C3Point Pvect, C3Point vectX, C3Point vectY, C3Point vectZ)
{
	C3Point Plocal;
	double mod_vectX = ModVect(vectX);
	double mod_vectY = ModVect(vectY);
	double mod_vectZ = ModVect(vectZ);
	//double mod_Pvect = ModVect(Pvect);
// case when X & Y local axes are perpendicular !
	Plocal.X = ScalProd(Pvect, vectX) /  mod_vectX;
	Plocal.Y = ScalProd(Pvect, vectY) /  mod_vectY;
	Plocal.Z = ScalProd(Pvect, vectZ) /  mod_vectZ;
	return Plocal;
}

C3Point  GetLocalPoint(C3Point Pgl, C3Point Orig, C3Point  P1, C3Point P2)
// gets local coordinates in a System with Origin, P1 sets Xlocal direction, P2 -> Ylocal direction
{
	C3Point Plocal;
	C3Point vectP =  VectSum(Pgl, Orig, 1, -1); // substract vectors
	C3Point vectX =  VectSum(P1, Orig, 1, -1); // local vectorX
	C3Point vectY =  VectSum(P2, Orig, 1, -1); // local vectorY
	C3Point vectZ = VectProd(vectX, vectY);
	Plocal = GetLocalVect(vectP, vectX, vectY, vectZ);
	return Plocal;
}


int Red(double Ratio)
{
	int Max = 255;	
	int red = -1;
	if (Ratio < 0.2)  red  = (int) (Max*(0.2-Ratio)/0.2);
	if (Ratio >= 0.2 && Ratio <= 0.4) return 0;
	if (Ratio > 0.4 && Ratio < 0.6) red  = (int) (Max*(Ratio-0.4)/0.2);
	if (Ratio >= 0.6 && Ratio < 0.8) red = Max;
	if (Ratio >= 0.8) red  = (int) (Max*(1.0-Ratio)/0.2);
	return (red);
}

int Green(double Ratio)
{
	int Max = 255;
	int green = -1;
	if (Ratio <= 0.2 || Ratio >= 0.8) return 0;
	if (Ratio > 0.2 &&  Ratio < 0.4) green  = (int) (Max*(Ratio-0.2)/0.2);
	if (Ratio >= 0.4 && Ratio < 0.6) green = Max;
	if (Ratio >= 0.6) green  = (int) (Max*(0.8-Ratio)/0.2);
	return (green);
}
int Blue(double Ratio)
{
	int Max = 255;
	int blue = -1;
	if (Ratio > 0.4) return 0;
	if (Ratio > 0.2 && Ratio <= 0.4) blue  = (int) (Max*(0.4-Ratio)/0.2);
	if (Ratio <= 0.2) blue = Max;
	return (blue);
}

int Gray(double Ratio)
{
	int Max = 150;
	int gray;
	gray = (int) (Max * (Ratio));
	return (gray);
}

COLORREF GetColor10(double r)
{
	double ratio = 0.95*r;
//	if (r > 0.05) ratio = r-0.05;
	int i = (int)floor(ratio * 10);
	if (i >10) i = 9;
	if (i<0) i = -1;
	int red = 0, green = 0, blue = 0;
	switch (i) {
	case 0: red = (int)(220 * (0.1 - ratio)/0.1);
			green = (int)(220 * (0.1 - ratio)/0.1);
			blue = (int)(170 - (170 - 50) * ratio/0.1);
			break;
	case 1: red = 0;
			green = 0;
			blue = (int)(50 + (100 - 50) * (ratio - 0.1)/0.1);
			break;
	case 2: red = (int)(50 * (ratio - 0.2)/0.1);
			green = 0;
			blue = (int)(100 + (150 - 100) * (ratio-0.2)/0.1);
			break;
	case 3: red = (int)(50 + (150 - 50) * (ratio - 0.3)/0.1);
			green = 0;
			blue = 150;
			break;
	case 4: red = 150;
			green = 0;
			blue = (int)(150 - 150 * (ratio-0.4)/0.1);
			break;
	case 5: red = (int)(150 + (255 - 150) * (ratio - 0.5)/0.1);
			green = 0;
			blue = 0;
			break;
	case 6: red = 255;
			green = (int)(255 * (ratio - 0.6)/0.1);
			blue = 0;
			break;
	case 7: red = 255;
			green = 255;
			blue = (int)(200 * (ratio - 0.7)/0.1);
			break;
	case 8: red = 255;
			green = 255;
			blue = (int)(200 + (250 - 200) * (ratio - 0.8)/0.1);
			break;
	case 9: red = 255;
			green = 255;
			blue = (int)(250 + (255 - 250) * (ratio - 0.9)/0.1);
			break;
	
	default: red = 255;	green = 255; blue = 255; break;
	}
	//return Color[i];
	return RGB(red, green, blue);

}

void SetColors()
{
//	Color[0] = RGB(220,	220,220); // gray
	Color[0] = RGB(230,	230,180); // dark haki
	Color[1] = RGB(0,	0,	50); // dark blue
	Color[2] = RGB(0,	0,	100); // blue
	Color[3] = RGB(50,	0,	150); // dark magenta
	Color[4] = RGB(150,	0,	150); // magenta
	Color[5] = RGB(150,	0,	0); // dark red
	Color[6] = RGB(255,	0,	0); // bright red
	Color[7] = RGB(255,	255,0); // dark yellow
	Color[8] = RGB(255,	255,200); // yellow
	Color[9] = RGB(255,	255,250); // white
	Color[10] = RGB(255,255,255); // 
}

