#ifndef __config_h__
#define __config_h__

/////#define     VERSION   4.5

#define		NAME(x)		#x 
#define		CHAR(c)		#@c
#define     IVAL_STR(param,s0)  itoa((int)param, s0, 10)
#define	    PARAM_STR(param, s, s0)  strcpy(s, #param); \
															strcat(s, " = "); \
															strcat(s, IVAL_STR(*param,s0)) ; \
															strcat(s, "\n")
#define		PRINT_INT(param, f)   fprintf(f, #param " = %d \n", *param)
#define		PRINT_FLOAT(param, f)   fprintf(f, #param " = %f \n", *param)
#define		PRINT_DOUBLE(param, f)   fprintf(f, #param " = %le \n", *param)
#define		PRINT_MACRO(param, f, type)  PRINT ## type ## (param,f)

#define			PI		 3.1415926535897932384626433832795 //3.1415926

////#define    VERSION(s, num, s0)   strcpy(s, "Version "); \ strcat(s, ltoa((double)num, s0, 10))

class C3Point; 
class CPlate;
//class lstream; // copy cout to log-file


///// operations with C3Point, Vectors, Local Coord Systems ----------------------
C3Point GetMantOrder(double Value); // return integer, decimal parts and exp order
double ModVect(const C3Point vect);
double  ScalProd(const C3Point v1, const C3Point v2);
C3Point VectProd(const C3Point v1, const C3Point v2);
double GetDistBetween(C3Point P1, C3Point P2);
C3Point  VectSum(C3Point v1, C3Point v2, int c1, int c2);
C3Point  GetLocalVect(C3Point Pvect, C3Point vectX, C3Point vectY, C3Point vectZ);
C3Point  GetLocalPoint(C3Point Pgl, C3Point Orig, C3Point  P1, C3Point P2);
double IntegralErr(double Lim, double div);
BOOL Odd(int N);
void RandFile(double rlimit);
////  common operations ---------------------------------------
 inline int Max(int a, int b) {return a > b ? a : b; }
 inline int Min(int a, int b) {return a < b ? a : b; }
 inline double Max(double a, double b) {return a > b ? a : b; }
 inline double Min(double a, double b) {return a < b ? a : b; }
 inline BOOL Between(double a, double b, double c) { return (a-b)*(c-b) <= 0; }

int Red(double Ratio);
int Green(double Ratio);
int Blue(double Ratio);
int Gray(double Ratio);
COLORREF GetColor10(double Ratio);
void SetColors();

inline void Message_NotSelected()  {AfxMessageBox("NO plate selected!", MB_ICONINFORMATION | MB_OK); }
//inline void Message_Version() { AfxMessageBox("This is BTR 4.5!", MB_OK); }

/////////// C3Point ------------------------------------------------------
 class C3Point
 {
 public: 
		double X, Y, Z;
 public:
		 C3Point();
		 C3Point(double x, double y, double z);
		 void operator = (double  a);
		 void operator = (const C3Point & P);
		 BOOL operator<(const C3Point & P);
		 double   operator *(const C3Point &P) const;
		 C3Point  operator * (double  a) const;
		 C3Point  operator / (double  a) const;
		 //C3Point  operator %(const C3Point &P) const;
		 C3Point  operator + (const C3Point & P) const;
		 C3Point  operator - (const C3Point & P) const;
		// C3Point  operator + (const C3Point & P1, const C3Point & P2) const;
		// C3Point  operator - (const C3Point & P1, const C3Point & P2) const;
 };
 //////////////////////////////////////////////
 struct minATTR // size minimized - 15(16) bytes
{
	unsigned short Xmm;// 2 bytes 0 ...65535
	unsigned short Ymm;// 2 bytes 0 ...65535
	short AXmrad; // 2 bytes –32768...32767
	short AYmrad; // 2 bytes –32768...32767
	float PowerW; // 4 bytes 3.4E +/- 38 (7 digits)
	unsigned short Nfall; // 2 bytes 0 ...65535
	signed char Charge; // 1 byte -128...127

	minATTR() {}

	minATTR(const minATTR & t)
	{
		Xmm = t.Xmm; Ymm = t.Ymm; 
		AXmrad = t.AXmrad;
		AYmrad = t.AYmrad;
		PowerW = t.PowerW;
		Nfall = t.Nfall;
		Charge = t.Charge;
	}
	
	minATTR(C3Point pos, double ax, double ay, double power, int nfall, int charge)
	{
		double X = pos.X * 1000;
		if (X < 0) X = 0.0;
		double Y = pos.Y * 1000;
		if (Y < 0) Y = 0.0;
		Xmm = (unsigned short)(X); 
		Ymm = (unsigned short)(Y); 
		AXmrad = (short)(ax * 1000);
		AYmrad = (short)(ay * 1000);
		PowerW = (float)power;
		Nfall = (unsigned short)nfall;
		if (Nfall <= 0) Nfall = 0;
		Charge = (signed char)charge;
	}
};

 ////////// CLoad (Counter) -------------------------------------------
 class CLoad : public CObject
 {
 public:
	 int Nx, Ny;
	 double StepX, StepY;
	 double Xmax, Ymax;
	 double CrossX, CrossY, CrossZ; // for Sheet draw
	 double ** Val;
	 double ** wh;
	 double ** wv;
	 double Sum, MaxVal, MaxProf;
	 int  iMaxVal, jMaxVal, iProf, jProf, iMaxBound;
	 int SmoothDegree;
	 CString Comment;
	 CString Particles;
	 
 public:
	 CLoad();
	 CLoad(double xmax, double ymax, double stepx, double stepy);// set direct grid
	 CLoad(double xmax, double ymax);//optimize grid
	 ~CLoad();
	 void operator=(double ** fromarr);
	 void SetValArray(double ** arr, int nx, int ny);
	 void SetGridOptimal(void);
	 void SetCrosses();
	 void Copy(CLoad * load);
	 void  Distribute(double xloc, double yloc, double power);  //
	 void  Clear();
	 void  SetSumMax();
	 void  SetProf(double x, double y);
	 void  SetLevels(CView * pView, CDC* pDC, double X0, double Y0, double X1, double Y1); //
	 
	 void  DrawLoad(CView * pView, CDC* pDC); //not used
	 void  DrawContours(double left, double right, double bot, double top); // NEW
	 void  DrawMapLimited(double left, double right, double bot, double top);// NEW
	 void  DrawLoadLimited(CView * pView, CDC* pDC, double X0, double Y0, double X1, double Y1); //

	 void  DrawEmptyLoad();
	 void  DrawLoad();
	 void  DrawLoadProfiles(CView * pView, CDC* pDC); // on sheet view
	 void  DrawLoadPoint(int x, int y, int rx, int ry, COLORREF color, CDC* pDC);
	 void   SmoothLoad(int degree);
	 double GetVal(double x, double y);
	 C3Point GetValGrad(double x, double y);
	 void  WriteProfiles(int num, double X, double dHor, double dVert);
	 
//	void  operator = (CLoad * load); 
 };


/////////// CPlate -------------------------------------------------------
 class CPlate : public CObject
 {
 public:
	 static double AbsYmin, AbsYmax, AbsZmin, AbsZmax;
	 static double DefStepX1, DefStepY1, DefStepX2, DefStepY2;

	 C3Point  Corn[4];
	 C3Point  MassCentre, Orig, OrtX, OrtY, OrtZ, Vmin, Vmax; // in global CS // older Map limits
	
	 double Tang; // inclination of OrtY from Vertical
	 bool	 Loaded; // Marked by user for load calc (serial) or calculated already (parallel)
	 bool    Visible; // Shown  on views
	 bool    Solid; // stop particles (not transparent)
	 bool    MAP;// interesting data
	 bool    Selected; // shown red 
	 bool	 Touched; // hot-spot shown red (in parallel - needs to calculate load)
	 int     Number;
	 CString  filename;
	 CRect * RectMark;
	
	 CLoad * Load; // without Smoothing
	 CLoad * SmLoad; // Smoothed
	 int SmoothDegree;
	 CString Comment;
	 int Fixed; // 0 - plan, 1 - side, -1 - to define
	 int OrtDirect; // +1 - face, -1 - inner side // for earlier SetOrts, now switched off
	
	 CArray <double, double> AngularProfile;//
	 double MinAngle, MaxAngle, StepAngle; // mrad

	 double Xmax, Ymax; // local Max dimensions for Load View (Xmin = Ymin = 0)  
	 double crossX, crossY; // View grid steps - for Load and Profiles
	 double leftX, rightX, botY, topY; //local View dimensions

	 CArray <minATTR> Falls;


 public:
	 CPlate();
	 ~CPlate();
	 CPlate(C3Point p0, C3Point p1, C3Point p2, C3Point p3);
	 static void  InitAbsLimits();
	 void  SetAbsLimits();
	 void  SetArbitrary(C3Point p0, C3Point p1, C3Point p2, C3Point p3); // sort corners first 
	 void  SetOrts();
	 void  SetMaxLocalDim();
	 void  SetViewDim();// default max
	 void  SetViewDim(double left, double right, double bot, double top);// fixed
	 void  ShiftOrig(double dx, double dy);
	 void  SortCorners(int ndir, C3Point p0, C3Point p1, C3Point p2, C3Point p3);// p0, p1, p2 define the plane
	 void  SetLocals(C3Point p0, C3Point p1, C3Point p2, C3Point p3); // Orig, Orts, Dimensions
	 int   NormDirect(C3Point p0, C3Point p1, C3Point p2, C3Point p3);
	 void  SetOrtsNew();
	 void  SetOrtsFree();
	 void  SetFromLimits(C3Point LB, C3Point RT); // LeftBottom, RightTop
	 void  SetVminVmax(C3Point p0, C3Point p1, C3Point p2, C3Point p3);
	 void  SetVminVmax();

	 C3Point GetLocal(C3Point P);
	 C3Point GetGlobalPoint(C3Point Ploc); 
	 C3Point GetGlobalVector(C3Point Vat);
	 C3Point GetGlobalVector(double VXloc, double VYloc);

	 BOOL IsCrossed(C3Point p1, C3Point p2);// by line defined by 2 points
	 C3Point FindLocalCross(C3Point p1, C3Point p2); //seek of CrossPoint
	 C3Point ProectCorners(CPlate * base, C3Point & p0, C3Point & p1, C3Point & p2, C3Point & p3); // projection on base
	 BOOL  WithinPoly(C3Point P, C3Point p1, C3Point p2, C3Point p3, C3Point p4);
	 BOOL  WithinPoly(C3Point Ploc);
	 void  CoverByParticle(double power, double diffY, double diffZ, 
							   C3Point Ploc, C3Point DimOrtH, C3Point DimOrtV, 
							   C3Point Vel, C3Point dVH, C3Point dVV);
//	void  DrawPlate();
	void  DrawPlate(CView * pView, CDC* pDC); // show plate + recalc  RgnPlan,  RgnSide
	void  ApplyLoad(BOOL flag, double Hx, double Hy);
	void  ShowLoadState();
	void  ShowLoad();
	void  DrawLoadRect(); //NEW: called from MainView LBdown
	void  DrawPlateBound();//NEW:polygon, called from MainView LBdown
	void  ShowEmptyLoad();
	void  CorrectLoad(); // clear extra spots at the area exit plane
	void  WriteLoad(FILE * fout);
	void  WriteLoadAdd(FILE * fout);
	void  WriteProfiles();
	void  SetSmoothLoad();
	void  Shift(double X, double Y, double Z);
	void  ShiftVert(double Z1, double Z2);
	void  ShiftHor(double Y1, double Y2);
	void  InitAngleArray(double min, double max, double step);
	void  DistributeAngle(double angle, double power); // mrad
	int   CommonCorn(CPlate * plate);
	int   CheckPoints(C3Point p0, C3Point p1, C3Point p2, C3Point p3);
	int   Errors();
	double DistCorn(CPlate * plate, double tol);

};
////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef  CTypedPtrList<CObList, CPlate*>  PtrList;
/////////////////////////////////////////////////////////////////////////////////////////////////////////

 
struct tDistInfo {
	int n1, n2;
	double d;
	tDistInfo(int _n1, int _n2, double _d)
	{
		n1 = _n1;
		n2 = _n2;
		d = _d;
	}
};

 #endif