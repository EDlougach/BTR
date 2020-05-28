#include "stdafx.h"
#include "MainView.h"
#include "BTRDoc.h"
#include "config.h"
#include "partfld.h"
#include <math.h>
//#include <iostream>
#include <fstream>

double CPlate::DefStepX1 = 0.01;
double CPlate::DefStepY1 = 0.01;

double CPlate::DefStepX2 = 0.02;
double CPlate::DefStepY2 = 0.02;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern BOOL TaskRemnantField, TaskReionization, ThickNeutralization;
PtrList * PlatesList; // data stored in Document
extern CMainView * pMainView;

CBTRDoc * pDoc;

double MagnetX, MagnetY,  MagnetTang, MagnetWidth, MagnetAxisDist;
double MagExtern = 0.5; // dist to magnetaxis
//CField * pU;
double MultCoeff;

double Reion_Xmin = 25, Reion_Xmax = 26;//13.63;
double Bstray_Xmin = 25, Bstray_Xmax = 26;//13.63;

double MFStepR = 0.1;
int MFSize;
double MFBmax = 1;
//char * MFname = "MFKstar.txt";
CArray<C3Point, C3Point> MF; // at Z = 0
CArray<double, double> Density; // 
CArray<double, double> DensX;
//char * GasName = "GasKstar.txt";
double MaxDensity;
extern double Bremnant1, Bremnant2, Bremnant3, Xremnant1, Xremnant2, Xremnant3;
extern double NeutrSigma;
extern double DuctExitYmin, DuctExitYmax, DuctExitZmin, DuctExitZmax;

CArray<double, double> SumReiPowerX; /// array for Reion sum power deposited within StepX
CArray<double, double> SumAtomPowerX; /// array for Atom sum power deposited within StepX
double SumPowerStepX = 0.1;// step for sum power

CArray<double, double> SumPowerAngle; // array for sum power deposited within Angle
double SumPowerAngleStep = 1;// degrees, step for angular profile

extern CArray<C3Point, C3Point> Exit_Array; // Beam Foot: X-Curr, Y-posY, Z-posZ
extern CArray <RECT_DIA, RECT_DIA> Dia_Array; // diaphragms array
//////////   Global operations for Particles, Fields, Config ------------------------------------------------ 

/*void SetConfig(PtrList* List)
{
	PlatesList = List;
}*/

double RelMass(double EMeV) // relativistic electron
{
	double Krel = 1.+ EMeV/0.511;
	return Krel;
}
double RelV(double EMeV) // relativistic electron
{
	double Mass = 1.+ EMeV/0.511;
	double c = 3.e+10; // cm/s
	double V = c * sqrt(1- (1./Mass/Mass)); // cm/s
	return (V*0.01); // m/s
}

//////////////  BEAM ////////////////////////////////////////////////////////////////////////////////////////////////
double GaussDensity(double argum, double alfa, double alfaH, double partH)
{
	double x = argum, Jcore, Jhalo;
	double alfa2 = alfa*alfa;
	double alfaH2 = alfaH * alfaH;
	Jcore = (1. - partH) /sqrt(PI) /alfa * exp(-x*x/alfa2);
	Jhalo = partH /sqrt(PI) /alfaH * exp(-x*x/alfaH2);
	return (Jcore + Jhalo);// * alfa * alfa;
}

double Gauss(double argum, double alfa) // for Equal Angle division dAlfa =Const Current =var
{
	double Sum = 0, x = 0;
	x = argum;
	//Sum = 1.0 - (1.- HaloPart) * exp(-x*x/AlfaCore/AlfaCore) - HaloPart*exp(-x*x/AlfaHalo/AlfaHalo);
	Sum = 1.0 - exp(-x*x/alfa/alfa);
	return(Sum);
}

double Gauss(double argum, double alfa, double alfaH, double partH) // for Equal Angle division dAlfa =Const Current =var
{
	double a, aH;
	a = alfa; aH = alfaH; 
	if (fabs(a) < 1.e-16) a = 0.0001;
	if (fabs(aH) < 1.e-16) aH = 0.0001;
	double Sum = 0, x = argum;
	double dAlfa;
	if (a > 0.0001) dAlfa = 0.0001*a;
	else dAlfa = 0.0001*aH;

	Sum = 1.0 - (1.- partH) * exp(-x*x /a/a) - partH * exp(-x*x/aH/aH);
	//Sum = 1.0 - exp(-x*x/alfa/alfa);
	return(Sum);
}

double SumGauss(double sum, double alfa) // for Equal Current Division: dAlfa = Var; Current = Const 
{
	double Sum = 0, x = 0;
	double dAlfa = 0.0001* alfa;
	while (Sum < sum) {
		x += dAlfa;
		Sum = 1.0 - exp(-x*x/alfa/alfa);
		//	Sum += ((1.-HaloPart)*exp(-x*x/AlfaCore/AlfaCore) + HaloPart*exp(-x*x/AlfaHalo/AlfaHalo)) *dAlfa;
	}
	return(x);
}

double SumGauss(double sum, double alfa, double alfaH, double partH) // for Equal Current Division: dAlfa = Var; Current = Const 
{
	double a, aH;
	a = alfa; aH = alfaH; 
	if (fabs(a) < 1.e-16) a = 0.0001;
	if (fabs(aH) < 1.e-16) aH = 0.0001;
	double Sum = 0, x = 0;
	
	double dAlfa;
	if (a > 0.0001) dAlfa = 0.0001*a;
	else dAlfa = 0.0001*aH;

	while (Sum < sum) {
		x += dAlfa;
	//	Sum = 1.0 - exp(-x*x/alfa/alfa);
		Sum = 1.0 - (1.- partH) * exp(-x*x/a/a) - partH * exp(-x*x/aH/aH);
	}
	return(x);
}

void  Decil(int PolarNumber, int type, double * PolarArray, double * CurrentArray, 
				double alfa, double alfaH, double partH)
{
	double AlfaMax = 3 * Max(alfa, alfaH); // 
	double TotalCurrent = Gauss(AlfaMax, alfa, alfaH, partH);
//	AlfaMax = SumGauss(0.999, alfa, alfaH, partH); // reduce
	double GroupCurrent;
	if  (PolarNumber <1) GroupCurrent = 1;
	else GroupCurrent = TotalCurrent / PolarNumber;//TotalCurrent/AngleNmax; ///EnergyNmax;
	double  GroupAngle;
	if  (PolarNumber <1) GroupAngle = 0;
	else GroupAngle = AlfaMax / PolarNumber;
	double SumCurrent = 0;
	double SumAngle = 0;
	double Angle0,Angle1;
	int i;
	
if  (type == 0) {   /// Equal Current Division: dAlfa = Var; Current = Const
	PolarArray[0] = 0.0; CurrentArray[0] = 0;//GroupCurrent/TotalCurrent;
	if (PolarNumber <1) {
		PolarArray[0] = 0.0; CurrentArray[0] = 1;
	}
	Angle0 = 0;
	for (i=1; i<=PolarNumber; i++) {
		SumCurrent = i*GroupCurrent;
		CurrentArray[i] = GroupCurrent/TotalCurrent;
		Angle1 = SumGauss(SumCurrent, alfa, alfaH, partH);
		PolarArray[i] = 0.5 *(Angle0 + Angle1);//Angle1
		Angle0 = Angle1;
	} // i = PolarNumber-1
	//CurrentArray[PolarNumber] = 0;//GroupCurrent/TotalCurrent;//0;
	SumCurrent = 0;
	for (int i=0; i<=PolarNumber; i++)  {
		SumCurrent +=  CurrentArray[i];
	}
}/////    Equal Current  ----------------------------------------------------

else if (type == 1) {  //// Equal Angle Division: dAlfa = Const; Current = Var
	SumCurrent = 0;
	PolarArray[0] = 0.0; CurrentArray[0] = 0.0;//GroupCurrent/TotalCurrent;
	if (PolarNumber <1) {
		PolarArray[0] = 0.0; CurrentArray[0] = 1;
	}
	for (i=1; i<=PolarNumber; i++) {
		SumAngle = i* GroupAngle;//		SumCurrent = i*GroupCurrent;
		SumCurrent += CurrentArray[i-1];
		CurrentArray[i] = (Gauss(SumAngle, alfa, alfaH, partH) - SumCurrent);
		PolarArray[i] = (i-0.5)* GroupAngle;//SumAngle;
	} // i = PolarNumber-1 
	SumCurrent = 0;
	for (i=1; i<=PolarNumber; i++)  {
		CurrentArray[i] /= TotalCurrent;
		SumCurrent +=  CurrentArray[i];
	}

}/////   file -----------------------------------------------------------------

/*	FILE *fout = fopen("decil.dat", "w");
	fprintf(fout, "Core Div =  %g  Halo Div = %g Halo Part = %g \n",  alfa, alfaH, partH);
	fprintf(fout, "Splitting Model = %d  (0 - Equal Current; 1 - Equal Angle) \n",  type);
	fprintf(fout, "Total =  %le  Summa = %le \n",  TotalCurrent, SumCurrent);
	fprintf(fout, " i \t \t  Angle  \t Current \t  Summa \n");
	SumCurrent = 0;
	for (i=0; i<=PolarNumber; i++) {
		SumCurrent += CurrentArray[i];
		fprintf(fout, "%d    %le   %le   %le \n", i,  PolarArray[i], CurrentArray[i], SumCurrent); 
	}
	fclose(fout);
*/
}

void RandFile(double rlimit)
{	
	double x, y, dy;
	char buf[1024];
	int result, n = 0;
	srand(4);

	char name[1024];
	CFileDialog dlg(TRUE, "dat; txt | * ", "", OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"X Y data (*.txt);(*.dat)  | *.txt; *.TXT; *.dat; *.DAT | All Files (*.*) | *.*||", NULL);
	
	if (dlg.DoModal() == IDOK) {
			strcpy(name, dlg.GetPathName());
		
	} // IDOK
	else return;

	FILE *fout = fopen("random.dat", "w");
	fprintf(fout, "random limits  %g,  %g \n",  -rlimit, rlimit);
	fprintf(fout, " x\t  y\t  dy\t  y+dy \n");
 
	FILE *fin = fopen(name, "r");
	while  (!feof(fin)) 
	{
		if (fgets(buf, 1024, fin) == NULL) break;
		result = sscanf(buf, "%le %le",  &x, &y);
		if (result != 2) {
		//	fgets(buf, 1024, fin);
			continue;
		}
		double val = (double)rand() / (double) RAND_MAX;
		dy = -rlimit + 2 * rlimit * val; 
		fprintf(fout, "  %f\t  %le\t  %f\t  %le\n", x, y, dy, y * (1 + dy)); 
		n++;
	}
	fprintf(fout, "\n Total %d lines processed\n", n);
		
	fclose(fout);
	fclose(fin);
}



double IntegralErr(double Lim, double div)
{
	if (div < 1.e-16) return 0;
	double sum = 0;
	double dx = 0.0001;
	double x = 0;
	while (x <= Lim/div) {
		sum += dx * exp(-x*x);
		x += dx;
	}
	return (sum /sqrt(PI));

}

BOOL Odd(int N)
{
	if (N - 2*(N/2) == 0) return FALSE;
	else return TRUE;
}
//----------------------------------------------------------------------
//   class CParticle
//----------------------------------------------------------------------
CParticle::CParticle()
{
	Attr.L=0;
	Central = FALSE;
	OrtH = C3Point(0,1,0);
	OrtV = C3Point(0,0,1);
}

/*CParticle:: ~CParticle()
{	
}*/

void SetDocument(CBTRDoc * doc)
{
	pDoc = doc;
}

void 
CParticle::SetPartAttr( ATTRIBUTES Attr0, double TStep)
{
	Attr = Attr0;
	TimeStep = TStep;
}

void 
CParticle::SetPartSort(enum SORT sort)
{
	Sort = sort;
	switch (Sort) {
		case Electron:  Charge = -1; Mass = Me * RelMass(pDoc->IonBeamEnergy);	Color = RGB(50,250,150);  break;	
		case NeuD:		Charge = 0;	 Mass = 2*Mp;	Color = RGB(0,200,250);	 break;
		case NeuH:		Charge = 0;	 Mass = Mp;		Color = RGB(0,100,250);	 break;
		case PosIonD:   Charge = 1;	 Mass = Mp*2;	Color = RGB(250,50,50);  break;
		case PosIonH:   Charge = 1;	 Mass = Mp;		Color = RGB(250,150,50);  break;
		case NegIonD:   Charge = -1; Mass = Mp*2;	Color = RGB(50,200,50);  break;
		case NegIonH:   Charge = -1; Mass = Mp;		Color = RGB(50,150,50);  break;
		default : 	    Charge = 0;	 Mass = Mp;		Color = RGB(0,0,0);  break;	
	}
}

void 
CParticle::SetPartType(int type)
{
	switch (type) {
	case 0: SetPartSort(Electron); break;
	case -1: SetPartSort(NegIonH); break;
	case -2: SetPartSort(NegIonD); break;
	case 1: SetPartSort(PosIonH);  break;
	case 2: SetPartSort(PosIonD);  break;
	case 10: SetPartSort(NeuH);  break;
	case 20: SetPartSort(NeuD);  break;
	default: Charge = pDoc->TracePartQ; Mass = pDoc->TracePartMass; Color = RGB(250,0,250);  break;
	}
}

void CParticle:: SetAtomSort()
{
	SORT NewSort;
	switch (Sort) {
		case Electron:  NewSort = Electron;  break;	
		case NeuD:	NewSort = Sort;	 break;
		case NeuH:	NewSort = Sort;		 break;
		case PosIonD:   NewSort = NeuD;	 break;
		case PosIonH:   NewSort = NeuH;  break;
		case NegIonD:   NewSort = NeuD;  break;
		case NegIonH:   NewSort = NeuH;  break;
		default : 	 NewSort = NeuH;     break;	
	}
	SetPartSort(NewSort);
}

void CParticle:: SetPosIonSort()
{
	SORT NewSort;
	switch (Sort) {
		case Electron:  NewSort = Electron;  break;	
		case NeuD:	NewSort = PosIonD;	 break;
		case NeuH:	NewSort = PosIonH;		 break;
		case PosIonD:   NewSort = Sort;	 break;
		case PosIonH:   NewSort = Sort;  break;
		case NegIonD:   NewSort = PosIonD;  break;
		case NegIonH:   NewSort = PosIonH;  break;
		default : 	 NewSort = PosIonH;     break;	
	}
	SetPartSort(NewSort);
}
double CParticle:: SetV(double EkeV)
{
	Vstart = sqrt(2.* EkeV*1000 * Qe / Mass);
	 return(Vstart); // {m/c}
}

double CParticle:: GetV()
{
	return (sqrt(Attr.Vx*Attr.Vx + Attr.Vy*Attr.Vy + Attr.Vz*Attr.Vz));
}

void CParticle:: SetOrts()
{
//	if (ModVect(OrtV) + ModVect(OrtH) < 1.e-16) return;
	C3Point Vxy = C3Point(Attr.Vx, Attr.Vy, 0.);
	C3Point Vxyz = C3Point(Attr.Vx, Attr.Vy, Attr.Vz);
	C3Point OrtVz = C3Point(0.,0., 1.); 
	C3Point HorOrt = VectProd(OrtVz, Vxy);
	//if (Attr.Vy < 0) HorOrt = VectProd(Vxy, OrtVz);
	C3Point VertOrt = VectProd(Vxyz, HorOrt);
//	if (Attr.Vz < 0) VertOrt = VectProd(HorOrt, Vxyz);
	double modH = ModVect(HorOrt);
	double modV = ModVect(VertOrt);
	OrtH = HorOrt / modH;
	OrtV = VertOrt / modV;
}

void 
CParticle:: MoveOneTimeStep()   //-> Next Point Attr 
{
	double  X,Y,Z,L,  Vx,Vy,Vz,  Vx0,Vy0,Vz0, 
			Ax,Ay,Az,  Bx = 0, By = 0, Bz = 0,  dx, dy, dz;
	C3Point Bxyz = C3Point(0,0,0);
	C3Point Exyz = C3Point(0,0,0);
	double Ex =0, Ey = 0, Ez = 0;
	ATTRIBUTES Attr0 = Attr;
	C3Point D;
	
	double ts = TimeStep; // ndiv steps = One TimeStep!!! for ions
	switch (Sort){

	case NeuH: // neutral atoms
	case NeuD:
			dx =  Attr0.Vx * ts;
			dy =  Attr0.Vy * ts;
			dz =  Attr0.Vz * ts;
			//Attr.L = Attr0.L +  sqrt(dx*dx + dy*dy + dz*dz);
		
			  Attr.X = Attr0.X + dx;
			  Attr.Y = Attr0.Y + dy;
			  Attr.Z = Attr0.Z + dz;
			  Attr.Vx = Attr0.Vx;
			  Attr.Vy = Attr0.Vy;
			  Attr.Vz = Attr0.Vz;
		  break;

	case PosIonH: // ions, electrons
	case PosIonD:
	case NegIonD:
	case NegIonH:
	case Electron:
 		Vx0 = Attr0.Vx; Vy0 = Attr0.Vy; Vz0 = Attr0.Vz;
		X = Attr0.X; Y = Attr0.Y; Z = Attr0.Z; L = Attr0.L;
		Exyz = pDoc->RIDField->GetE(X,Y,Z);
		Ex = Exyz.X; Ey = Exyz.Y; Ez = 0; // !!!! Exyz.Z =U;

		if (pDoc->FieldLoaded) {
			Bxyz = pDoc->GetB(X, Y, Z); // MFcoeff is included!
			
			//double coeff = pDoc->MFcoeff;
			Bx = Bxyz.X; By = Bxyz.Y;	Bz = Bxyz.Z;
		}
//-----------------------------------------------------------
			Ax = Charge * Qe / Mass * (Ex  +  (-By*Vz0 + Bz*Vy0)); // [m/s2]
			Ay = Charge * Qe / Mass * (Ey  +  (-Bz*Vx0 + Bx*Vz0));
			Az = Charge * Qe / Mass * (Ez  +  (-Bx*Vy0 + By*Vx0));
			Vx = Vx0 + Ax * ts;	// [m/s]		
			Vy = Vy0 + Ay * ts;
			Vz = Vz0 + Az * ts;
			dx = 0.5*(Vx0 + Vx) * ts;
			dy = 0.5*(Vy0 + Vy) * ts;
			dz = 0.5*(Vz0 + Vz) * ts;
			X +=  dx; // [m]	
			Y +=  dy;
			Z +=  dz;
			//L += sqrt(dx*dx + dy*dy + dz*dz);
			//P.X = X; P.Y = Y; P.Z = Z;
			Vx0 = Vx; Vy0 = Vy; Vz0 = Vz;

Attr.X = X;  Attr.Y = Y;  Attr.Z = Z; //Attr.L = L;
Attr.Vx = Vx;   Attr.Vy = Vy;   Attr.Vz = Vz;
         break;
	} // switch Sort

}

void 
CParticle:: MoveOneStep(double dX)   //-> Next Point Attr 
{
	double  X,Y,Z,L,  Vx,Vy,Vz,  Vx0,Vy0,Vz0, 
			Ax,Ay,Az,  Bx = 0, By = 0, Bz = 0,  dx, dy, dz;
	C3Point Bxyz = C3Point(0,0,0);
	C3Point Exyz = C3Point(0,0,0);
	double Ex =0, Ey = 0, Ez = 0;
//	ATTRIBUTES Attr0 = Attr;
	C3Point D;
	
	double ts = dX / Attr.Vx; //TimeStep; // ndiv steps = One TimeStep!!! for ions

	switch (Sort){
	case NeuH: // neutral atoms
	case NeuD:
			dx = dX;//  Attr.Vx * ts;
			dy =  Attr.Vy * ts;
			dz =  Attr.Vz * ts;
		//	Attr.L = Attr.L +  sqrt(dx*dx + dy*dy + dz*dz);
			Attr.X += dx;  Attr.Y += dy;  Attr.Z += dz;
		  break;

	case PosIonH: // ions, electrons
	case PosIonD:
	case NegIonD:
	case NegIonH:
	case Electron:
//	default:
 		Vx0 = Attr.Vx; Vy0 = Attr.Vy; Vz0 = Attr.Vz;
		X = Attr.X; Y = Attr.Y; Z = Attr.Z; L = Attr.L;
		Exyz = pDoc->RIDField->GetE(X,Y,Z);
		Ex = Exyz.X; Ey = Exyz.Y; Ez = 0; //!!! Exyz.Z = U;

		if (pDoc->FieldLoaded) {
			Bxyz = pDoc->GetB(X,Y,Z); 
			
			//double coeff = pDoc->MFcoeff;
			Bx = Bxyz.X; By = Bxyz.Y;	Bz = Bxyz.Z;
		}
//-----------------------------------------------------------
			Ax = Charge * Qe / Mass * (Ex  +  (-By*Vz0 + Bz*Vy0)); // [m/s2]
			Ay = Charge * Qe / Mass * (Ey  +  (-Bz*Vx0 + Bx*Vz0));
			Az = Charge * Qe / Mass * (Ez  +  (-Bx*Vy0 + By*Vx0));
			Vx = Vx0 + Ax * ts;	// [m/s]		
			Vy = Vy0 + Ay * ts;
			Vz = Vz0 + Az * ts;
			dx = 0.5*(Vx0 + Vx) * ts;
			dy = 0.5*(Vy0 + Vy) * ts;
			dz = 0.5*(Vz0 + Vz) * ts;
			X +=  dx; // [m]	
			Y +=  dy;
			Z +=  dz;
		//	L += sqrt(dx*dx + dy*dy + dz*dz);
			Attr.X = X;  Attr.Y = Y;  Attr.Z = Z; //Attr.L = L;
			Attr.Vx = Vx;   Attr.Vy = Vy;   Attr.Vz = Vz;
         break;
	} // switch Sort

}


void
CParticle:: DrawParticleTrack(C3Point P1, C3Point P2)	//CView * pView, CDC * pDC)
{
	//CCriticalSection cs;

	CMainView* pMV = pMainView;
	CClientDC dc(pMV);
	pMV->OnPrepareDC(&dc);
	//CDC * pDC = pMV->GetDC();
	dc.SelectObject(pMV->AtomPen);
	CPoint p1, p2;
//	dc.DPtoLP(&pos);
	
	//cs.Lock();
	p1.x = pMV->OrigX + (int) (P1.X * (pMV->ScaleX));
	p1.y = pMV->OrigY - (int) (P1.Y * (pMV->ScaleY));
	dc.MoveTo(p1); //SetPixel(pos, Color);
	p2.x = pMV->OrigX + (int) (P2.X * (pMV->ScaleX));
	p2.y = pMV->OrigY - (int) (P2.Y * (pMV->ScaleY));
	dc.LineTo(p2);
		 
	p1.y = pMV->OrigZ - (int) (P1.Z*(pMV->ScaleZ));
	dc.MoveTo(p1);
	p2.y = pMV->OrigZ - (int) (P2.Z*(pMV->ScaleZ));
	dc.LineTo(p2);
	 //pMV->ReleaseDC(pDC);
	 //dc.SelectObject(pOldpen);
	//cs.Unlock();

}

void
CParticle:: DrawParticle()	//CView * pView, CDC * pDC)
{
	CCriticalSection cs;

	CMainView* pMV = pMainView;
	CClientDC dc(pMV);
	pMV->OnPrepareDC(&dc);
	//CDC * pDC = pMV->GetDC();
	CPoint pos;
	
	//cs.Lock();
	dc.DPtoLP(&pos);
	 pos.x = pMV->OrigX + (int) (Attr.X*(pMV->ScaleX));
	 pos.y = pMV->OrigY - (int) (Attr.Y*(pMV->ScaleY));
	 dc.SetPixel(pos, Color);
	 pos.y = pMV->OrigZ - (int) (Attr.Z*(pMV->ScaleZ));
	 dc.SetPixel(pos, Color);
	 //pMV->ReleaseDC(pDC);
	 //cs.Unlock();

}
void
CParticle:: DrawParticle(COLORREF color)	//CView * pView, CDC * pDC)
{
	CCriticalSection cs;

	CMainView* pMV = pMainView;
	CClientDC dc(pMV);
	pMV->OnPrepareDC(&dc);
	//CDC * pDC = pMV->GetDC();
	CPoint pos;

	cs.Lock();
	dc.DPtoLP(&pos);

	 pos.x = pMV->OrigX + (int) (Attr.X*(pMV->ScaleX));
	 pos.y = pMV->OrigY - (int) (Attr.Y*(pMV->ScaleY));
//	 dc.SetPixel(pos, color);
	 dc.SetPixel(pos.x+1, pos.y, color);

	 pos.y = pMV->OrigZ - (int) (Attr.Z*(pMV->ScaleZ));
//	 dc.SetPixel(pos, color);
	 dc.SetPixel(pos.x+1,pos.y, color);
	 
	 //pMV->ReleaseDC(pDC);
	 cs.Unlock();

}


double CParticle:: RandAttr(double step)
{
//	srand( (unsigned)time( NULL ) );   /* Display 10 numbers. */
//   for( i = 0;   i < 10;i++ )      printf( "  %6d\n", rand() );
//	srand(1);
	double val = (double)rand() / (double) RAND_MAX;
	double dx = step*val*Attr.Vx; 
	double dy = step*val*Attr.Vy; 
	double dz = step*val*Attr.Vz; 
	Attr.X += dx; Attr.Y += dy; Attr.Z += dz;
	return (step * val);
}

void CParticle:: BackToX(double X, C3Point Prev)
{
	C3Point Next = C3Point(Attr.X, Attr.Y, Attr.Z);
	C3Point Back;
	Back.X = X;
	Back.Y = Prev.Y + (Next.Y - Prev.Y) * (Back.X - Prev.X) / (Next.X - Prev.X);
	Back.Z = Prev.Z + (Next.Z - Prev.Z) * (Back.X - Prev.X) / (Next.X - Prev.X);
	Attr.X = Back.X; Attr.Y = Back.Y; Attr.Z = Back.Z;
}

void CParticle:: TraceElectron()
{
	MSG message;
	CMainView* pMV = pMainView;
	CClientDC dc(pMV);
	pMV->OnPrepareDC(&dc);

	// double V = GetV(); //sqrt(Attr.Vx*Attr.Vx + Attr.Vy*Attr.Vy + Attr.Vz*Attr.Vz);
	// double t0 = RandAttr(TimeStep);
	
	C3Point P1 = C3Point(Attr.X, Attr.Y, Attr.Z);
	C3Point P2, dP;
	C3Point P;

	while  (Attr.State != STOPPED) {// while not Stopped
		if (pDoc -> OptDrawPart) DrawParticle();
		MoveOneTimeStep();// AAA = C3Point(0,0,0) - VectProd(Vel, BBB) * Qe/(Mass*Me);
		SetOrts();
		P2 = C3Point(Attr.X, Attr.Y, Attr.Z);
		////////////// check up if crossed plate or stopped ///////////////////////////////////////////////////////
		if (Attr.X >= pDoc->AreaLong || Attr.X < -TimeStep) {
			Attr.State = STOPPED;
			if (Central && Attr.Y >= DuctExitYmin && Attr.Y <= DuctExitYmax 
				&& Attr.Z >= DuctExitZmin && Attr.Z <= DuctExitZmax ) 
			{
				P.X = Attr.Current;	P.Y = Attr.Y; P.Z = Attr.Z;
				Exit_Array.Add(P);
			}
		}// Area limit
			
	/*	int state = GetPartState(P1, P2);
		if (state==0) Attr.State = STOPPED;
*/
		C3Point Pstop = GetFirstSolid(P1, P2); // depose initial power!
		//if (pDoc -> OptDrawPart) DrawParticleTrack(P1, Pstop);
		if (Attr.Current >= 1.e-12) LoadsBetween(P1, Pstop);
			
		
		if (::PeekMessage(&message, NULL, 0,0, PM_REMOVE)) {
			::TranslateMessage(&message);
			::DispatchMessage(&message);
		}
		if (pDoc->STOP) Attr.State = STOPPED;

		dP = P2 - P1;
		Attr.L += ModVect(dP);//  sqrt(dx*dx + dy*dy + dz*dz);
		P1 = P2;
		} // while not Stopped
	pMV->ShowNBLine();

}

int CParticle:: GetNeutrPointNumber(C3Point P1, C3Point P2)
{
	for (int i = 0; i <= pDoc->NeutrArray.GetUpperBound(); i++) {
		if (P1.X <= pDoc->NeutrArray[i].X && P2.X > pDoc->NeutrArray[i].X) return i;
	}
	return (-1);
}

void CParticle:: TraceSourceIonWithNeutralization()
{
	MSG message;
	double J0 = Attr.StartCurrent;
	double P0 = Attr.StartPower;
	if (Attr.Current < 1.e-10) return;
	C3Point P1, P2, dP;
	double V = GetV(); //	RandAttr(Attr, TimeStep * V);
	double dL = pDoc->NeutrStepL;
	double Nstep = dL / V;
	
	if (TimeStep * V < pDoc->NeutrXmin)	TraceSourceIon(pDoc->NeutrXmin);
/*	else {
		MoveOneStep(dL); 
		SetOrts();
		P2 = C3Point(Attr.X, Attr.Y, Attr.Z);
		C3Point Pstop = GetFirstSolid(P1, P2); // depose initial power!
		if (Attr.Current >= 1.e-12) LoadsBetween(P1, Pstop);
		if (pDoc -> OptDrawPart) DrawParticle();
	}
*/
	if (Attr.State == STOPPED) return;
	
		P1 = C3Point(Attr.X, Attr.Y, Attr.Z);
		TimeStep = Nstep;
		RandAttr(TimeStep);
		if (pDoc -> OptDrawPart) DrawParticle();
		int n;
		for (n = 1; n <= pDoc->NeutrArray.GetUpperBound(); n++) {
			if (Attr.State == STOPPED) break;
			// double Xnext = pDoc->NeutrArray.GetAt(n).X;
			MoveOneStep(dL); 
			SetOrts();
			
			if (Attr.State == STOPPED) break;
			Attr.Current = J0 * pDoc->NeutrArray.GetAt(n).Y;
			Attr.Power = P0 * pDoc->NeutrArray.GetAt(n).Y;
			if (Attr.Current < 1.e-16) continue; 

			P2 = C3Point(Attr.X, Attr.Y, Attr.Z);
		/*	int state = GetPartState(P1, P2);
			if (state==0) Attr.State = STOPPED;*/
			C3Point Pstop = GetFirstSolid(P1, P2); // depose initial power!
			//if (pDoc -> OptDrawPart) DrawParticleTrack(P1, Pstop);
			//if (Attr.Current >= 1.e-16) 
			LoadsBetween(P1, Pstop);
			if (pDoc -> OptDrawPart) DrawParticle();
			
			
			if (::PeekMessage(&message, NULL, 0,0, PM_REMOVE)) {
				::TranslateMessage(&message);
				::DispatchMessage(&message);
			}
			if (pDoc->STOP) Attr.State = STOPPED;
			if (Attr.State == STOPPED) break;

			//	emit (always) & trace pos ion (if !OptNeutrStop)
			if (!pDoc->OptNeutrStop)
				EmitPosIon(n);

			if (pDoc->OptTraceAtoms)
				EmitAtom(n);

			dP = P2 - P1;
			Attr.L += ModVect(dP);
			P1 = P2;
		} // n
	
	if (Attr.State != STOPPED && pDoc->OptNeutrStop == FALSE)  {//	trace neg ion after neutralizer 
		TimeStep = pDoc->IonStepL / V;
		Attr.Current = J0 * pDoc->NeutrArray.GetAt(n-1).Y;
		Attr.Power = P0 * pDoc->NeutrArray.GetAt(n-1).Y;
	//	if (Attr.Current < 1.e-16) return; 

		TraceIon();
	}


}

void CParticle:: EmitPosIon(int n)
{
	ATTRIBUTES OldAttr = Attr; // parent neg ion
	SORT OldSort = Sort;
	double OldStep = TimeStep;
	double J0 = Attr.StartCurrent;
	double P0 = Attr.StartPower;
	C3Point OldOrtV = OrtV;
	C3Point OldOrtH = OrtH;
	if (J0 < 1.e-10) return;

	double V = GetV();
	double PosStep = pDoc->IonStepL / V;
	
	double partcurr;

	if (n<0) // THIN Neutralization
		partcurr = pDoc->PosIonPart;
	//	if (*pDoc->NeutrPart + poscurr > 1)
		
	else // THICK
		partcurr = pDoc->NeutrArray.GetAt(n).Z - pDoc->NeutrArray.GetAt(n-1).Z;

	Attr.Current = J0 * partcurr ;
	Attr.Power = P0 * partcurr;
	if (Attr.Current < 1.e-16) {
		SetPartSort(OldSort);
		SetPartAttr(OldAttr, OldStep);
		OrtV = OldOrtV; OrtH = OldOrtH;
		return;
	}

	SetPosIonSort();
	SetPartAttr(Attr, PosStep);

	 TraceIon();

// give old attributes to parent 
	SetPartSort(OldSort);
	SetPartAttr(OldAttr, OldStep);
	OrtV = OldOrtV; OrtH = OldOrtH;
	
}

void CParticle:: EmitAtom(int n)
{
	ATTRIBUTES OldAttr = Attr; // parent neg ion
	SORT OldSort = Sort;
	double OldStep = TimeStep;
	double J0 = Attr.StartCurrent;
	double P0 = Attr.StartPower;
	C3Point OldOrtV = OrtV;
	C3Point OldOrtH = OrtH;
	if (J0 < 1.e-10) return;
	
	//double V = GetV();
	//double PosStep = *pDoc->IonStepL / V;
	double poscurr, negdrop;
	
	if (n<0) {//THIN neutralization
		poscurr = pDoc->PosIonPart;
		negdrop = pDoc->NeutrPart + poscurr;
	}
	else {// THICK
		poscurr	= pDoc->NeutrArray.GetAt(n).Z - pDoc->NeutrArray.GetAt(n-1).Z;
		negdrop = pDoc->NeutrArray.GetAt(n-1).Y - pDoc->NeutrArray.GetAt(n).Y;
	}
	
	Attr.Current = J0 * (negdrop - poscurr);
	Attr.Power = P0 * (negdrop - poscurr);
	if (Attr.Current < 1.e-16) {
		SetPartSort(OldSort);
		SetPartAttr(OldAttr, OldStep);
		OrtV = OldOrtV;	OrtH = OldOrtH;
		return;
	}

	SetAtomSort();

	TraceAtomFast();

	// give old attributes to parent 
	SetPartSort(OldSort);
	SetPartAttr(OldAttr, OldStep);
	OrtV = OldOrtV;	OrtH = OldOrtH;
	
}

void CParticle:: TraceSourceIon(double Xmax)
{
	MSG message;
//	double V = GetV(); 	RandAttr(Attr, TimeStep * V);

	if (Attr.X >= Xmax) {
		//Attr.X = Xmax; 
		return;
	}
	
	C3Point P1 = C3Point(Attr.X, Attr.Y, Attr.Z);
	C3Point P2, dP;
	RandAttr(TimeStep);

	if (pDoc -> OptDrawPart) DrawParticle();
	
	C3Point P;
	BOOL Finish = FALSE;
	Attr.State = MOVING;

	while  (Attr.State != STOPPED && !Finish) {// while not Stopped
		
		MoveOneTimeStep();// AAA = C3Point(0,0,0) - VectProd(Vel, BBB) * Qe/(Mass*Me);
		SetOrts();
					
		if (Attr.X >= Xmax) {// check up if stopped 
			BackToX(Xmax, P1);
			Finish = TRUE;
			
		} // Neutr limit
		
		P2 = C3Point(Attr.X, Attr.Y, Attr.Z);

	/*	int state = GetPartState(P1, P2);
		if (state==0) Attr.State = STOPPED;*/
		C3Point Pstop = GetFirstSolid(P1, P2); // depose initial power!
	//	if (pDoc -> OptDrawPart) DrawParticleTrack(P1, Pstop);
		if (Attr.Current >= 1.e-12) LoadsBetween(P1, Pstop);

		if (pDoc -> OptDrawPart) DrawParticle();
		
		if (::PeekMessage(&message, NULL, 0,0, PM_REMOVE)) {
			::TranslateMessage(&message);
			::DispatchMessage(&message);
		}
		if (pDoc->STOP) Attr.State = STOPPED;

		dP = P2 - P1;
		Attr.L += ModVect(dP);
		P1 = P2;
		} // while not Stopped
//	pMV->ShowNBLine(&dc);

}

void CParticle:: TraceAtomFast()
{
	if (Attr.Current < 1.e-12 && !Central) return;
	C3Point P1 = C3Point(Attr.X, Attr.Y, Attr.Z);
	C3Point P2, Pstop;

	P2.X = pDoc->AreaLong + 0.2;
	P2.Y = P1.Y + Attr.Vy / Attr.Vx * (P2.X - P1.X);
	P2.Z = P1.Z + Attr.Vz / Attr.Vx * (P2.X - P1.X);
	Attr.State = STOPPED;
//	SetOrts();

	Pstop = GetFirstSolid(P1, P2); // depose initial power!

	if (pDoc -> OptDrawPart) DrawParticleTrack(P1, Pstop);
	
	if (Attr.Current >= 1.e-12) LoadsBetween(P1, Pstop);
	
	C3Point Pout;
//	if (Central && fabs(P2.Y) <= 0.5 *(*pDoc->Duct6W) && fabs(P2.Z) <= 0.5 *(*pDoc->Duct6H)) 
		if (Central && Attr.Y >= DuctExitYmin && Attr.Y <= DuctExitYmax 
				&& Attr.Z >= DuctExitZmin && Attr.Z <= DuctExitZmax ) 
			{
				Pout.X = Attr.Current;	Pout.Y = Attr.Y; Pout.Z = Attr.Z;
				Exit_Array.Add(Pout);
			}

	if (Attr.Current < 1.e-12) return;
//-------- re-ionisation ----------------
	if (pDoc->OptReionStop) return;

	ATTRIBUTES AtomAttr = Attr; // keep initial particle 
	AtomAttr.Vx = Attr.Vx; AtomAttr.Vy = Attr.Vy; AtomAttr.Vz = Attr.Vz;
	AtomAttr.Current = Attr.Current; AtomAttr.Power = Attr.Power;
	AtomAttr.L = Attr.L;
	C3Point AtomOrtV = OrtV;
	C3Point AtomOrtH = OrtH;

	double V = GetV(); 
//	double AtomPower = Attr.Power;
	//double Xion, Yion, Zion, lost;
	double ionstep = pDoc->IonStepL / V;
	double dist, lost;
	C3Point Pion; 

	SetPosIonSort();
	
	for (int i = 0; i <= pDoc->ReionArray.GetUpperBound(); i++) {
		Pion.X = pDoc->ReionArray[i].X;
		if (Pion.X >= Pstop.X) return; // beyond atom stop
		Pion.Y = P1.Y + AtomAttr.Vy / AtomAttr.Vx * (Pion.X - P1.X);
		Pion.Z = P1.Z + AtomAttr.Vz / AtomAttr.Vx * (Pion.X - P1.X);
		Attr.X = Pion.X; Attr.Y = Pion.Y; Attr.Z = Pion.Z; 
		dist = GetDistBetween(P1, Pion);
		Attr.L = AtomAttr.L + dist;
		Attr.Vx = AtomAttr.Vx; Attr.Vy = AtomAttr.Vy; Attr.Vz = AtomAttr.Vz; 
		OrtV = AtomOrtV; OrtH = AtomOrtH;
		lost = pDoc->ReionArray[i].Y;
		Attr.Current = lost * AtomAttr.Current;
		if (Attr.Current < 1.e-100) continue;
		Attr.Power = AtomAttr.Power * lost; 
		AtomAttr.Current -= Attr.Current;
		AtomAttr.Power -= Attr.Power; 
		SetPartAttr(Attr, ionstep);
		TraceIon();
	} // i

//	dP = P2 - P1;
//	Attr.L += ModVect(dP);

}

void CParticle:: TraceIon()
{
	MSG message;
//	double V = GetV(); 	RandAttr(Attr, TimeStep * V);

	C3Point P1 = C3Point(Attr.X, Attr.Y, Attr.Z);
	C3Point P2, dP;
	//RandAttr(TimeStep);
//	if (pDoc -> OptDrawPart) DrawParticle();
	
	C3Point P;
	Attr.State = MOVING;

	while  (Attr.State != STOPPED) {// while not Stopped
		if (pDoc -> OptDrawPart) DrawParticle();

		MoveOneTimeStep();// AAA = C3Point(0,0,0) - VectProd(Vel, BBB) * Qe/(Mass*Me);
		SetOrts();
		

// check up if stopped 	
		if (pDoc->STOP) Attr.State = STOPPED;

		if (Attr.X >= pDoc->AreaLong) {// Area limit 
			Attr.State = STOPPED;
		} 
		
		P2 = C3Point(Attr.X, Attr.Y, Attr.Z);
	
		C3Point Pstop = GetFirstSolid(P1, P2); // depose initial power!
		if (Attr.Current >= 1.e-16) LoadsBetween(P1, Pstop);


		if (::PeekMessage(&message, NULL, 0,0, PM_REMOVE)) {
			::TranslateMessage(&message);
			::DispatchMessage(&message);
		}
		if (pDoc->STOP) Attr.State = STOPPED;

		dP = P2 - P1;
		Attr.L += ModVect(dP);
		P1 = P2;
	} // while not Stopped

	//	pMV->ShowNBLine(&dc);

}

void CParticle:: TraceParticle(double  power) // not used now
{
	MSG message;
	CParticle * NeutrParticle;
	CParticle * ReionParticle;
	CParticle * PositiveIon;

	Attr.Power = power;
	C3Point P1, P2, dP;// Ploc, Pend;
	P1 = C3Point(Attr.X, Attr.Y, Attr.Z);
	double V = GetV(); 

	if ((Sort == NegIonD || Sort == NegIonH)  && Attr.X < pDoc->NeutrInX) RandAttr(TimeStep);

	if (Sort == NeuH || Sort == NeuD) pDoc->ReionPercent = 0; 
	while  (Attr.State != STOPPED) {
		if (pDoc -> OptDrawPart) DrawParticle();
		MoveOneTimeStep();
		SetOrts();
		P2 = C3Point(Attr.X, Attr.Y, Attr.Z);
	//	DrawParticle(); //(this, pDC); // Pglobal = Plocal for central channel 

	/////////////// check up if crossed plate or stopped ///////////////////////////////////////////////////////
		if (Attr.X >= pDoc->AreaLong) { // Area limit
			Attr.State = STOPPED;
			//if (Central && fabs(Attr.Y) <= 0.5 *(*pDoc->Duct6W) && fabs(Attr.Z) <= 0.5 *(*pDoc->Duct6H)) 
			if (Central && Attr.Y >= DuctExitYmin && Attr.Y <= DuctExitYmax 
				&& Attr.Z >= DuctExitZmin && Attr.Z <= DuctExitZmax ) 
				{
				C3Point P;
				P.X = Attr.Current;
				P.Y = Attr.Y;
				P.Z = Attr.Z;
				Exit_Array.Add(P);
			}
		} // Area limit

	/*	int state = GetPartState(P1, P2);
		if (state==0) Attr.State = STOPPED;*/
		C3Point Pstop = GetFirstSolid(P1, P2); // depose initial power!
		//if (pDoc -> OptDrawPart) DrawParticleTrack(P1, Pstop);
		if (Attr.Current >= 1.e-12) LoadsBetween(P1, Pstop);
	
		if ((Sort == PosIonD || Sort == PosIonH) && Attr.Current < 1.e-100) Attr.State = STOPPED;

		
////////////////  process  conversions ///////////////////////////////////////////////////////////////////////////////
		SORT newsort; // sort of neutrals
		double NeutrX = pDoc->NeutrXmax; 
		double PosIonPart = pDoc->PosIonPart; //(1 - *pDoc->NeutrPart) * 0.5;

// ---------  Neutralisation -------------------------------------------------------------------------
		if (Attr.State != STOPPED) {

			if (Sort == NegIonD || Sort == NegIonH ) { // D-/H-

				// Thin Neutralization -> Split At Neutraliser exit
				if (!ThickNeutralization  &&  
					//P1.X < *pDoc->NeutrXmax  &&   P2.X >= *pDoc->NeutrXmax) {
					NeutrX - P2.X <= TimeStep*V && NeutrX - P2.X > 0.) {
					
					newsort = NeuD; // Do
					if (Sort == NegIonH) newsort = NeuH; //Ho
					if (pDoc->OptNeutrStop) { // stop ions after Netr -> Emit neutrals(even if Current = 0!)
						NeutrParticle = EmitParticle(newsort, this, pDoc->NeutrPart, pDoc->TraceTimeStep);
					delete NeutrParticle;
					}

					newsort = PosIonD; // D+
					if (Sort == NegIonH) newsort = PosIonH; //H+
					if (!pDoc->OptNeutrStop && Attr.Current > 1.e-100) { // D+ (with Current!=0) 
					PositiveIon = EmitParticle(newsort, this, PosIonPart, pDoc->TraceTimeStep);
					delete PositiveIon;
					}

					if (Attr.Current > 1.e-100) // Reduce D- curr
						DecreaseCurrent(pDoc->NeutrPart + PosIonPart);
					
					if (pDoc->OptNeutrStop) Attr.State = STOPPED;//  Stop parent ion (D-)
				} // Thin 

// *********** ATTENTION!  Thick Neutralization - NO POSITIVE IONS !!!! *********************************
// ***********this function is NOT VALID yet - but not used now
				else if (ThickNeutralization)  { 
						if (P2.X <= pDoc->NeutrXmax) {
						 newsort = NeuD;
						 if (Sort == NegIonH) newsort = NeuH; //Ho
						double Ngas =  pDoc->GField->GetPressure(P2.X); //1.e+18;
						double	NeutrCurr = Ngas * NeutrSigma * TimeStep * V;
						NeutrParticle = EmitParticle(newsort, this, NeutrCurr, pDoc->TraceTimeStep);
						delete NeutrParticle;
						if (Attr.Current > 1.e-100) 
							DecreaseCurrent(NeutrCurr);
						//ReionPercent += ReionCurr * 100;
						} // if within Neutraliser
						else { // after Neutraliser
							if (pDoc->OptNeutrStop) Attr.State = STOPPED;
						}
				} // Thick
				
			} // if NegIon
		} // not Stopped
	//	if (Attr.Current < 1.e-100) Attr.State = STOPPED;
		

//  Reionisation -----------------------------------------------------------------------------------

		//	double ReionSigma = 3.2e-21; // m2
		if (Attr.State != STOPPED && pDoc->PressureLoaded) {
			if (Sort == NeuH ||  Sort == NeuD) { // Do
				if (!pDoc->OptNeutrStop) Attr.State = STOPPED; // stop neutrals if ions are traced (for RID) 
				if (P2.X > pDoc->ReionXmin &&   P2.X <= pDoc->ReionXmax) {
					 newsort = PosIonD;
					 if (Sort == NeuH) newsort = PosIonH; //Ho
					double Ngas = pDoc->GField->GetPressure(P2.X); //1.e+18;
					double	ReionCurr = Ngas * pDoc->ReionSigma * pDoc->ReionStepL; //TimeStep * V;
					if (pDoc->OptReionAccount && pDoc->PressureLoaded) {
						if (!pDoc->OptReionStop) { 
							ReionParticle = EmitParticle(newsort, this, ReionCurr, (pDoc->IonStepL)/V);
							if (Attr.Current > 1.e-100)
								DecreaseCurrent(ReionCurr);
							delete ReionParticle;
						}
						else if (Attr.Current > 1.e-100)
							DecreaseCurrent(ReionCurr);//ion stopped
						if (pDoc -> OptDrawPart) DrawParticle(RGB(200, 50,50));
						pDoc->ReionPercent += ReionCurr * 100;
					} // ReionAccount = true
				}// (P2.X > *pDoc->ReionXmin &&   P2.X <= *pDoc->ReionXmax)
			} // if sort = Neutral
		} // non-Stopped and PressureLoaded
		if ((Sort == PosIonD || Sort == PosIonH) && Attr.Current < 1.e-100) Attr.State = STOPPED;

		if (::PeekMessage(&message, NULL, 0,0, PM_REMOVE)) {
			::TranslateMessage(&message);
			::DispatchMessage(&message);
		}
		if (pDoc->STOP) Attr.State = STOPPED;

		dP = P2 - P1;
		Attr.L += ModVect(dP);
		P1 = P2;
		} // while not Stopped

}

int  CParticle:: GetPartState(C3Point P1, C3Point P2) // not used // return 1 if moving forward; 0 - stopped
{
	//PtrList & 
		PlatesList = &(pDoc->PlatesList);
	POSITION pos = PlatesList->GetHeadPosition();
	CPlate * plate;
	C3Point Pend, Ploc;
	double rad = 0;// = sqrt(pDoc->PartDispX * pDoc->PartDispY) * Attr.L;
	int k, kmax = 4;
	double dt = 2*PI / kmax;
	double dx, dy, power;

		while (pos != NULL) {
			plate = PlatesList->GetNext(pos); 
			if (plate->IsCrossed(P1, P2)) { // crossed
				Ploc = plate->FindLocalCross(P1, P2);
				
				if (Ploc.X < plate->Xmax && Ploc.X > 0 && Ploc.Y < plate->Ymax && Ploc.Y > 0)
					{
						if (plate->Loaded) { // Loaded
							if (rad < 0.5 * plate->Load->StepX && rad < 0.5 * plate->Load->StepY) 
							// distribute as one point 
							plate->Load->Distribute(Ploc.X, Ploc.Y, Attr.Power);
							else {
								power = 0.5 * Attr.Power;
								plate->Load->Distribute(Ploc.X, Ploc.Y, power);
								power = 0.5 * Attr.Power / kmax;
								for (k = 0; k < kmax; k++) {
									dx = rad * cos(dt * (k+0.5)); dy = rad * sin(dt * (k+0.5));
									plate->Load->Distribute(Ploc.X + dx, Ploc.Y + dy, power);
								}
							}
									
							Pend = plate->GetGlobalPoint(Ploc);
						} // Loaded
						else if (pDoc->DINSELECT && Attr.Power > 1.e-16) { // plate not loaded & DINSELECT
							pDoc->SelectPlate(plate);
							//plate->Load->Distribute(Ploc.X, Ploc.Y, Attr.Power);
							if (rad < 0.5 * plate->Load->StepX && rad < 0.5 * plate->Load->StepY) 
							// distribute as one point 
							plate->Load->Distribute(Ploc.X, Ploc.Y, Attr.Power);
							else {
								power = 0.5 * Attr.Power;
								plate->Load->Distribute(Ploc.X, Ploc.Y, power);
								power = 0.5 * Attr.Power / kmax;
								for (k = 0; k < kmax; k++) {
									dx = rad * cos(dt * (k+0.5)); dy = rad * sin(dt * (k+0.5));
									plate->Load->Distribute(Ploc.X + dx, Ploc.Y + dy, power);
								}
							}
							Pend = plate->GetGlobalPoint(Ploc);
						}
						if (plate->Solid) { // return back to cross-point
							Attr.X = Pend.X; Attr.Y = Pend.Y; Attr.Z = Pend.Z;
							return 0;// STOPPED;
						} // solid
					
					} // within plate
			} // crossed
		} // while pos
		return 1; // MOVING
}


BOOL CParticle:: IsVolumeEmpty(C3Point P)
{
	//CBTRDoc * pDoc = (CBTRDoc*) m_pDoc;
	RECT_DIA dia0, dia1;
	double x0, x1, y0min, y0max, z0min, z0max, y1min, y1max, z1min, z1max, ymin, ymax, zmin, zmax;
	int Nmax = Dia_Array.GetUpperBound();
	for (int n = 0; n < Nmax; n++) {
		dia0 = Dia_Array.GetAt(n); dia1 = Dia_Array.GetAt(n+1);
		if (dia0.Number < 5) continue;
		x0 = dia0.Corn[0].X; x1 = dia1.Corn[0].X;
		y0min = dia0.Corn[0].Y; y1min = dia1.Corn[0].Y; 
		y0max = dia0.Corn[2].Y; y1max = dia1.Corn[2].Y;
		z0min = dia0.Corn[0].Z; z1min = dia1.Corn[0].Z; 
		z0max = dia0.Corn[2].Z; z1max = dia1.Corn[2].Z;

		if ((P.X - x0) * (x1 - P.X) > 1.e-12) {
			ymin = y0min + (P.X - x0) * (y1min - y0min) / (x1 - x0);
			ymax = y0max + (P.X - x0) * (y1max - y0max) / (x1 - x0);
			if ((P.Y - ymin) * (ymax - P.Y) > 1.e-12) {
				zmin = z0min + (P.X - x0) * (z1min - z0min) / (x1 - x0);
				zmax = z0max + (P.X - x0) * (z1max - z0max) / (x1 - x0);
				if ((P.Z - zmin) * (zmax - P.Z) > 1.e-12) return TRUE;
			} //if y
		} // if x
	}// n

	return FALSE;
}
void CParticle:: LoadsBetween(C3Point P1, C3Point P2)
{
	//if ( IsVolumeEmpty(P2)) return;
	//PtrList & 
		PlatesList = &(pDoc->PlatesList);
	POSITION pos = PlatesList->GetHeadPosition();
	CPlate * plate;
	C3Point Ploc, Pend;
//	double rad;// = sqrt(pDoc->PartDispX * pDoc->PartDispY) * Attr.L;
//	int k, kmax = 4;
//	double dt = 2*PI / kmax;
//	double dx, dy, power;
	double dimH, dimV, dim;
	C3Point Vel = C3Point(Attr.Vx, Attr.Vy, Attr.Vz);
	double V = GetV();
	CString S;
	double h;
	C3Point H;


		while (pos != NULL) {
			plate = PlatesList->GetNext(pos); 
			if (plate->Solid == TRUE) continue; // only transparent!

			if (plate->IsCrossed(P1, P2)) { // crossed
				Ploc = plate->FindLocalCross(P1, P2);
				Pend = plate->GetGlobalPoint(Ploc);
				double dist = GetDistBetween(P1, Pend);

				dimH = pDoc->PartDimHor * (Attr.L + dist);
				dimV = pDoc->PartDimVert * (Attr.L + dist);
				dim = sqrt(dimH*dimV);
							
				if (Ploc.X < plate->Xmax && Ploc.X > 0 && Ploc.Y < plate->Ymax && Ploc.Y > 0)
					{
						if (plate->Loaded) { // Loaded
							if ((int)pDoc->BeamSplitType == 0) // polar splitting
								//|| dimH < 1.e-16	|| dimV < 1.e-16) 
							// distribute as one point 
								plate->Load->Distribute(Ploc.X, Ploc.Y, Attr.Power);
							else {// cartesian splitting
							//	plate->CoverByParticle(Attr.Power, Vel, OrtH, OrtV, Ploc, dimH, dimV); 
								plate->CoverByParticle(Attr.Power, Attr.dCurrY, Attr.dCurrZ,
									Ploc, OrtH*dimH, OrtV*dimV, 
									Vel, OrtH*V*(pDoc->PartDimHor), OrtV*V*(pDoc->PartDimVert));
														}
							//Pend = plate->GetGlobalPoint(Ploc);
						} // Loaded
						else if (pDoc->DINSELECT && Attr.Power > 1.e-16) { // plate not loaded & DINSELECT
														
							if ((int)pDoc->BeamSplitType == 0) {   // polar splitting
								
							//	h = 0.002;
							//	while (dim > 0.2 * h && h < 0.1) h += 0.001;
								h = dim * 4;
								H = GetMantOrder(h);
								h = H.X*H.Z;
								if (h>0.1) h = 0.1;
								if (h<0.005) h = 0.005;
								pDoc->SelectPlate(plate, h, h);
								// distribute as one point 
								plate->Load->Distribute(Ploc.X, Ploc.Y, Attr.Power);
							}
							else {// cartesian splitting
							//	h = 0.05;
							//	while (dim < h && h > 0.005) h *= 0.5;
								h = dim * 0.5;
								H = GetMantOrder(h);
								h = H.X*H.Z;
								if (h<0.005) h = 0.005;
								if (h>0.1) h = 0.1;
								
								//pDoc->SelectPlate(plate);// apply default steps
								pDoc->SelectPlate(plate, h, h);
								plate->CoverByParticle(Attr.Power, Attr.dCurrY, Attr.dCurrZ,
									Ploc, OrtH*dimH, OrtV*dimV, 
									Vel, OrtH*V*(pDoc->PartDimHor), OrtV*V*(pDoc->PartDimVert));
							}
							//Pend = plate->GetGlobalPoint(Ploc);
						}
					} // within plate
			} // crossed
		} // while pos
}

C3Point CParticle:: GetFirstSolid(C3Point P1, C3Point P2) 
{
	//if ( IsVolumeEmpty(P2)) return P2;
	//PtrList & 
		PlatesList = &(pDoc->PlatesList);
	POSITION pos = PlatesList->GetHeadPosition();
	CPlate * plate;
	CPlate * Plate0 = NULL;
	C3Point Pend, Ploc, Ploc0;
	C3Point Last = P2;
	double dist, dist0 = GetDistBetween(P1, P2);
	double dimV = 0, dimH = 0, dim = 0;

	Attr.State = MOVING;
	C3Point Vel = C3Point(Attr.Vx, Attr.Vy, Attr.Vz);
	double V = GetV();
	double hx, hy, h;
	C3Point H;
	CString S;

	while (pos != NULL) {
			plate = PlatesList->GetNext(pos); 
			if (plate->Solid == FALSE) continue;

			if (plate->IsCrossed(P1, Last)) { // crossed solid
				Ploc = plate->FindLocalCross(P1, Last);
				
				if (Ploc.X < plate->Xmax && Ploc.X > 0 
					&& Ploc.Y < plate->Ymax && Ploc.Y > 0) {
					
		
					Pend = plate->GetGlobalPoint(Ploc);
					dist = GetDistBetween(P1, Pend);
					 
					if (dist < dist0) { // check if plate is closer than previous
						dist0 = dist;
						Last = Pend;
						Plate0 = plate;
						Ploc0 = Ploc;
						dimH = pDoc->PartDimHor * (Attr.L + dist0);
						dimV = pDoc->PartDimVert * (Attr.L + dist0);
						dim = sqrt(dimH*dimV);
					}
				/*	else {
						S.Format("Crossed %d", plate->Number);
						AfxMessageBox(S);
					}*/
				} // within plate
			} // crossed solid
			//if (Plate0 != NULL) break;
		} // while pos
	

	Attr.X = Last.X; Attr.Y = Last.Y; Attr.Z = Last.Z;

	if ((this->Sort == NeuH || this->Sort == NeuD))// && pDoc->OptIonPower)
		return (Last);// NEUTRALS miss power distribution

		if (Plate0 != NULL) { // found solid between P1 and P2

			if (Plate0 -> Loaded)  {// Loaded-> depose FULL particle power (Re-ionized decrease not accounted) 
				if ((int)pDoc->BeamSplitType == 0)   // polar splitting
					// distribute as one point 
					Plate0->Load->Distribute(Ploc0.X, Ploc0.Y, Attr.Power);
						
				else { // cartesian splitting
					Plate0->CoverByParticle(Attr.Power, Attr.dCurrY, Attr.dCurrZ,
						Ploc0, OrtH*dimH, OrtV*dimV, 
						Vel, OrtH*V*(pDoc->PartDimHor), OrtV*V*(pDoc->PartDimVert));
									//	(Attr.Power, Vel, OrtH, OrtV, Ploc0, dimH, dimV); 
							
				}
				//Pend = plate->GetGlobalPoint(Ploc);
			} //Plate0 -> Loaded)

			else if (pDoc->DINSELECT && Attr.Power > 1.e-16) { // plate not loaded & DINSELECT
				
				if ((int)pDoc->BeamSplitType == 0) { // polar splitting
					
					//h = 0.002;
					//while (dim > 0.2 * h && h < 0.1) h += 0.001;
					h = dim * 4;
					H = GetMantOrder(h);
					h = H.X*H.Z;
					if (h<0.005) h = 0.005;
					if (h>0.1) h = 0.1;
					pDoc->SelectPlate(Plate0, h, h);
					
					// distribute as one point 
					Plate0->Load->Distribute(Ploc0.X, Ploc0.Y, Attr.Power);
				}

				else { // cartesian splitting
					if (fabs(Plate0->OrtX.X) < 1.e-6) { // e.g. target Cross-plane 
						//h = 0.05;
						//while (dim < h && h > 0.005) h *= 0.5;
						h = dim * 0.5;	
						H = GetMantOrder(h);
						h = H.X*H.Z;
						if (h<0.005) h = 0.005;
						if (h>0.1) h = 0.1;
						hx = h; hy = h;
					}
					else { // walls
						hx = 0.05; h = dim;//0.05;
						H = GetMantOrder(h);
						h = H.X*H.Z;
						if (h<0.005) h = 0.005;
						if (h>0.1) h = 0.1;
						hy = h;
						//while (dimH < hy && hy > 0.005) hy *= 0.5;
					}
								
					pDoc->SelectPlate(Plate0, hx, hy);
					//pDoc->SelectPlate(Plate0);// apply default steps

					Plate0->CoverByParticle(Attr.Power, Attr.dCurrY, Attr.dCurrZ, 
						Ploc0, OrtH*dimH, OrtV*dimV, 
						Vel, OrtH*V*(pDoc->PartDimHor), OrtV*V*(pDoc->PartDimVert));

				}
				//Pend = plate->GetGlobalPoint(Ploc);
			}// plate not loaded & DINSELECT

			Attr.State = STOPPED;

			// fill SumPowerX counter if ION is stopped by any solid surface   
			int k = (int) (Last.X / SumPowerStepX);
			if (k < SumReiPowerX.GetSize()) 
				SumReiPowerX[k] += Attr.Power;

		} // Solid plate found: Plate0 != NULL
		
					
	//	Attr.X = Last.X; Attr.Y = Last.Y; Attr.Z = Last.Z;

		if (Last.X < 0) Attr.State = STOPPED;
		return (Last);
}

					

CParticle* CParticle:: EmitParticle(enum SORT psort, CParticle* Parent, double lost,  double TStep)
{
	CParticle * Particle = new CParticle();
//	Particle->Sort = psort;
	Particle->SetPartSort(psort);
	Particle->SetPartAttr(Parent->Attr, TStep);
	Particle->Central = Parent->Central;
	Particle->Attr.StartCurrent = Parent->Attr.Current * lost;
	Particle->Attr.Current = Particle->Attr.StartCurrent;
//	Particle->SetDocument(Parent->pDoc);
	double power = Parent->Attr.Power * lost;
	Particle->TraceParticle(power);  
	
	return Particle;
}

void CParticle:: DecreaseCurrent(double lost)
{
	// decrease Current, Power of Parent particle
//	Parent->Attr.Current -= Particle->Attr.Current;
//	Parent->Power -= Particle->Power;
	Attr.Current *= 1 - lost;
	Attr.Power *= 1 - lost;
}

/////////////////////////////////////////////////////////////////////////////
///  CMagField ///////////////////////////////////////////////////////////////
/* public:
	 CArray <double, double> ArrX;
	 CArray <C3Point, C3Point> ArrB;
	 double Xmin, Xmax, Bmax;
 public:
	 CMagField();
	 ~CMagField();
	// CString OpenDefault(char* name, double Mult);
	int Init(char * name); // define + set geometry
	int ReadData(char * name); // fill array
	C3Point GetB(double x);
	C3Point GetB(C3Point P);
	void DrawMF(CDC* pDC, C3Point & Origin, C3Point & Scale, double xmax);
	*/

CMagField:: CMagField()
{
	Nx = 0;
	Bmax = 0;
	Xmin = Xmax = 0;
	
}

CMagField::~CMagField()
{
	ArrX.RemoveAll();
	ArrB.RemoveAll();
}

void CMagField::Clear()
{
	ArrX.RemoveAll();
	ArrB.RemoveAll();
}
int CMagField:: ReadData(char * name)
{
	ArrX.RemoveAll();
	ArrB.RemoveAll();
	C3Point B;
	double x, bx, by, bz;
	char buf[1024];
	int  result;
	int n = 0;
	CString S;
	
	FILE * fin = fopen(name, "r");
	if (fin == NULL) {
		S.Format("%s MF-file error / not found at Home\n ..\\ %s ", name, pDoc->CurrentDirName);
		AfxMessageBox(S, MB_ICONSTOP | MB_OK);
		std::cout << S << std::endl;
		return 0;
	}

	
	Bmax = 0; Xmax = 0; Xmin = 1000;
	
	while  (!feof(fin)) 
	{
		fgets(buf, 1024, fin);
		result  = sscanf(buf, "%le %le %le %le", &x, &bx, &by, &bz);
		if (result != 4) continue;
	//	fgets(buf, 1024, fin);
		B.X = bx; B.Y = by; B.Z = bz; 
		ArrX.Add(x); ArrB.Add(B);
		Xmax = Max(Xmax, x);
		Xmin = Min(Xmin, x);
		Bmax = Max(Bmax, fabs(bx)); Bmax = Max(Bmax, fabs(by)); Bmax = Max(Bmax, fabs(bz));
		n++;
	}
	fclose(fin); 
	Nx = n-1;
	std::cout << name << " MF - read :  " << Nx << " data" << std::endl;
	return (n);
}

C3Point CMagField::GetB(double x)
{
	C3Point B(0,0,0);
	double x0, x1;
	C3Point B0, B1;
	if (x < Xmin || x > Xmax) return B;
	for (int i = 0; i < Nx-1; i++) {
		 if (x >= ArrX[i] && x < ArrX[i+1]) {
			  x0 = ArrX[i];	x1 = ArrX[i+1];
			  B0 = ArrB[i]; B1 = ArrB[i+1];
			  B = B0 + (B1 - B0) * (x-x0) / (x1-x0);
			  return B;
		 } //if
	 } // for
	 return B;
}

C3Point CMagField::GetB(C3Point P)
{
	return GetB(P.X);
}

void CMagField::DrawMF(CDC* pDC, C3Point & Origin, C3Point & Scale)
{
	int i, X0, Y0, Z0, Top = 50;
	X0 = (int) Origin.X;
	Y0 = (int) Origin.Y; 
	Z0 = (int) Origin.Z;
	double x, Bx, By, Bz;
	CPen  RedPen, GreenPen, BluePen;
	
	if (!RedPen.CreatePen(PS_DOT, 2, RGB(200, 0, 0))) return;
	if (!GreenPen.CreatePen(PS_DOT, 2, RGB(0, 200, 0))) return;
	if (!BluePen.CreatePen(PS_DOT, 2, RGB(0, 0, 200))) return;

	CPen * pOldPen = pDC->SelectObject(&RedPen); 
	x = ArrX[0];
	Bx = ArrB[0].X;
	pDC->MoveTo(X0 + (int)(x * Scale.X), Y0 - (int)(Bx * Top / Bmax)); 
	for (i = 1; i < Nx; i++) {
		x = ArrX[i]; Bx = ArrB[i].X;
		pDC->LineTo(X0 + (int)(x * Scale.X), Y0 - (int)(Bx * Top / Bmax));
	}

	pDC->SelectObject(&GreenPen); 
	x = ArrX[0];
	By = ArrB[0].Y;
	pDC->MoveTo(X0 + (int)(x * Scale.X), Y0 - (int)(By * Top / Bmax)); 
	for (i = 1; i < Nx; i++) {
		x = ArrX[i]; By = ArrB[i].Y;
		pDC->LineTo(X0 + (int)(x * Scale.X), Y0 - (int)(By * Top / Bmax));
	}

	pDC->SelectObject(&BluePen); 
	x = ArrX[0];
	Bz = ArrB[0].Z;
	pDC->MoveTo(X0 + (int)(x * Scale.X), Y0 - (int)(Bz * Top / Bmax)); 
	for (i = 1; i < Nx; i++) {
		x = ArrX[i]; Bz = ArrB[i].Z;
		pDC->LineTo(X0 + (int)(x * Scale.X), Y0 - (int)(Bz * Top / Bmax));
	}

	pDC->SelectObject(pOldPen);
}

/*
{
	Nx = 1000;//(int)(Xtr/StepX1) + (int)((Xmax-Xtr)/StepX2) + 1; 
	Nz = 10;
	ValB = new C3Point * [Nx];  // B
	for (int i = 0; i < Nx; i++) {
		ValB[i] = new C3Point[Nz];
		for (int j = 0; j < Nz; j++) ValB[i][j] = 0.;
	}
	Bmax = 0;
	Xmax = 0;
	Zmin = Zmax = 0;	
}

CMagField::~CMagField()
{
	for (int i = 0; i < Nx; i++) {
		delete [] ValB[i];
	}
	delete [] ValB;
}

void CMagField:: Init(int nz)
{
	int i;
	for (i = 0; i < Nx; i++) {
		delete [] ValB[i];
	}
	delete [] ValB;

	Nx = (int)(Xtr/StepX1) + (int)((Xmax-Xtr)/StepX2) + 1; 
	Nz = nz;
	ValB = new C3Point * [Nx];  // B
	for (i = 0; i < Nx; i++) {
		ValB[i] = new C3Point[Nz];
		for (int j = 0; j < Nz; j++) ValB[i][j] = 0.;
	}
	Bmax = 0;
	Xmax = 0;
	Zmin = Zmax = 0;
}

void CMagField:: SetGeometry(double stepx1, double stepx2, double stepz, double xtr)
{
	StepX1 = stepx1;
	StepX2 = stepx2;
	StepZ = stepz;
	Xtr =  xtr;
	Xmax = 23.5;
	Zmin = -0.7;
	Zmax = 0.7;
}

CString CMagField:: OpenDefault(char* name, double Mult)  ///// regular file

{
//	double Mult = 900;
//	char buf[1024];
//	int result;
	FILE * fin;
	CString S;

	CString newname;
	WIN32_FIND_DATA fData;
	HANDLE h;
	h = ::FindFirstFile("MFIter*", &fData);
	if (h != (HANDLE) 0xFFFFFFFF) newname = fData.cFileName;

	//char fullname[1024];
	if ((fin= fopen(newname, "r")) == NULL) {
		CString S ="Please, set path to file \n ";
		S+= name;
		AfxMessageBox(S);
			
		CFileDialog dlg(TRUE, NULL, name);
		if (dlg.DoModal() == IDOK) {
		//	strcpy(newname, dlg.GetPathName());
			newname = dlg.GetPathName();
			fin = fopen(newname, "r");
		}
	}

	int n = 0;
	n = ReadData(fin);

	fclose(fin);
	
//	S.Format("Xlast = %g imax = %d z = %g jmax = %d", x, i, z, j);
//	AfxMessageBox(S);

	//SmoothB();
	return (newname);
}

int CMagField:: ReadData(FILE * fin)
{
	char buf[1024];
	int result;

	double Bx, By, Bz, BB, BBmax = 0;
	double  x, y, z;
	C3Point P;
	int i,j, iprev, jprev;
//	double Zmin = -0.7;// -(Nz-1)*0.5*StepZ;
	Xmax = 0;
	int itr = (int)(Xtr/StepX1);
	iprev = 0; jprev = 0;
	int n = 0;
	while  (!feof(fin)) 
	{
		if(fgets(buf, 1024, fin) == NULL) { fclose(fin); break; }
	//	result = sscanf(buf, "%le %le %le %le %le %le",  &x, &y, &z, &Bx, &By, &Bz);
		result = fscanf(fin, "%lf%lf%lf%le%le%le%le\n",  &x, &y, &z, &Bx, &By, &Bz, &BB);
		if (result != 7) {
			fgets(buf, 1024, fin);
			continue;
		}
		n++;
		//BB = sqrt(Bx*Bx + By*By + Bz*Bz);
		BBmax = Max(fabs(BBmax), fabs(BB));
		Xmax = Max(Xmax, x);

	    (x <= Xtr) ? i = (int) ceil(x/StepX1) : i = itr + (int) ceil((x - Xtr)/StepX2);
		//i = (int) ceil(x/StepX1);
		if (i>= Nx) AfxMessageBox("i = %d  line %d", i, n);
		j = (int) ceil ((z - Zmin)/StepZ);
		if (j <= jprev && i<iprev) { fclose(fin); break; }
		if (j>= Nz) AfxMessageBox("j = %d line %d", j, n);
			
		P.X = Bx; P.Y = By; P.Z = Bz;
		ValB[i][j] = P;
//		B[i][j].X = Bx;		B[i][j].Y = By;		B[i][j].Z = Bz;
	//	if (BB < 1.e-5) { fclose(fin); break; }
		iprev = i; jprev = j;
	}
	fclose(fin);
	Bmax = fabs(BBmax);
	return (n);
}

void CMagField:: SmoothB()
{
	for (int j = 0; j < Nz; j++)
		for (int i = 1; i < Nx-1; i++) {
			if (fabs(ValB[i][j].X) < 1.e-16) ValB[i][j].X = (ValB[i-1][j].X + ValB[i+1][j].X) * 0.5;
			if (fabs(ValB[i][j].Y) < 1.e-16) ValB[i][j].Y = (ValB[i-1][j].Y + ValB[i+1][j].Y) * 0.5;
			if (fabs(ValB[i][j].Z) < 1.e-16) ValB[i][j].Z = (ValB[i-1][j].Z + ValB[i+1][j].Z) * 0.5;
		}

}


C3Point  CMagField:: GetB(double x, double z) // 
{
	double dx, dz, stepx;
	C3Point B0, B1, B2, B3, Bleft, Bright, Bup, Bdown, Result1, Result2, Result(0,0,0); //
	int i, ii, j;

//	double Zmin = - 0.7; //(Nz-1)*0.5*StepZ;
	if (x > Xmax) return Result;
//	(x <= Xtr) ? i = (int) ceil(x/StepX1) : i = itr + (int) ceil((x - Xtr)/StepX2);
	if 	(x <= Xtr) {
		i = (int) ceil(x/StepX1)-1;
		if (i <0) i = 0;
		stepx = StepX1;
		dx = x - i*stepx;
		if (dx >= stepx) {
			i++;
			dx = x - i*stepx;
		}
		if (dx < 0) {
			i--;
			dx = x - i*stepx;
		}
	}
	else {
	  	ii =  (int) ceil(Xtr/StepX1);
		 i = ii + (int) ceil((x-Xtr)/StepX2)-1; 
		 stepx = StepX2;
		 dx = x - (Xtr + (i-ii)*stepx);
		 if (dx >= stepx) {
			i++;
			dx = x - (Xtr + (i-ii)*stepx);
		}
		 if (dx < 0) {
			 i--;
			 dx = x - (Xtr + (i-ii)*stepx);
		 }
	}
	j = (int) ceil((z - Zmin)/StepZ) -1;
//	if (j<0) j = 0;
	dz = z - j*StepZ;
	if (dz >= StepZ) {
			j++;
			dz = z - j*StepZ;
		}
		
	if (j >=0 && j < Nz) {
		B0 =  ValB[i][j];	B1 =  ValB[i][j+1]; B2 =  ValB[i+1][j+1];	B3 =  ValB[i+1][j];
	}
	if (j >= Nz) {
			B0 =  ValB[i][Nz];	B1 =  ValB[i][Nz]; B2 =  ValB[i+1][Nz];	B3 =  ValB[i+1][Nz];
	}
	if (j < 0) {
			B0 =  ValB[i][0];	B1 =  ValB[i][0]; B2 =  ValB[i+1][0];	B3 =  ValB[i+1][0];
	}

	Bleft.X = B0.X + (B1.X - B0.X) *dz / StepZ;
	Bleft.Y = B0.Y + (B1.Y - B0.Y) *dz / StepZ;
	Bleft.Z = B0.Z + (B1.Z - B0.Z) *dz / StepZ;
	Bright.X  = B3.X + (B2.X - B3.X) *dz / StepZ;
	Bright.Y  = B3.Y + (B2.Y - B3.Y) *dz / StepZ;
	Bright.Z  = B3.Z + (B2.Z - B3.Z) *dz / StepZ;
	
	Result1.X = Bleft.X + (Bright.X-Bleft.X) *dx / stepx;
	Result1.Y = Bleft.Y + (Bright.Y-Bleft.Y) *dx / stepx;
	Result1.Z = Bleft.Z + (Bright.Z-Bleft.Z) *dx / stepx;

	Bup.X = B1.X + (B2.X-B1.X)*dx/stepx; 
	Bup.Y = B1.Y + (B2.Y-B1.Y)*dx/stepx; 
	Bup.Z = B1.Z + (B2.Z-B1.Z)*dx/stepx; 
	Bdown.X = B0.X + (B3.X-B0.X)*dx/stepx; 
	Bdown.Y = B0.Y + (B3.Y-B0.Y)*dx/stepx; 
	Bdown.Z = B0.Z + (B3.Z-B0.Z)*dx/stepx; 
	
	Result2.X = Bdown.X + (Bup.X-Bdown.X) *dz / StepZ;
	Result2.Y = Bdown.Y + (Bup.Y-Bdown.Y) *dz / StepZ;
	Result2.Z = Bdown.Z + (Bup.Z-Bdown.Z) *dz / StepZ;

	Result = B0;
//	Result = Result1;
//	return MultCoeff*Value;
	return Result;
}

void CMagField:: DrawMF(CDC* pDC, C3Point & Origin, C3Point & Scale, double Zoom)
  {
//	CString S;
//	S.Format("Bmax = %g", Bmax);
//	AfxMessageBox(S); 
	int X0, Y0, Z0, Top = 100;
//	double x, z;
	X0 = (int) Origin.X;
	Y0 = (int) Origin.Y; 
	Z0 = (int) Origin.Z;
	C3Point B0;
	double BB;
	double step = 0.2;
	double K = Zoom;
	int Zlev;// = (int) ceil(z * Scale.Z);
	
	CPen  RedPen, GreenPen, BluePen;
	CPoint prevPlan, prevSide, nextPlan, nextSide;

	if (!RedPen.CreatePen(PS_DOT, 2, RGB(255, 100, 100))) return;
	if (!GreenPen.CreatePen(PS_DOT, 2, RGB(100, 255, 150))) return;
	if (!BluePen.CreatePen(PS_DOT, 2, RGB(100, 100, 255))) return;

	CPen * pOldPen = pDC->SelectObject(&RedPen); 

	CPoint P;
	int i, j;
	int itr =  (int) floor(Xtr/StepX1);

//	for (j = 0; j<Nz; j=j+2) {
	j = 2;
		pDC->SelectObject(pOldPen);
		Zlev = (int)((-0.7 + j*StepZ)*Scale.Z);
		P.x = X0; P.y = Z0 - Zlev;
		pDC->MoveTo(P);
		P.x = X0 + (int)(Xmax * Scale.X);
		pDC->LineTo(P);

	    pDC->SelectObject(&RedPen);	
		P.x = X0; P.y = Z0 - Zlev;
		pDC->MoveTo(P);
			for (i = 0; i < Nx; i++) {
				BB = ValB[i][j].X;
				if (i<=itr)	P.x = X0 + (int)(i * StepX1 * Scale.X);
				else P.x = X0 + (int)((itr * StepX1 + (i-itr)* StepX2) * Scale.X);
				P.y = Z0 - Zlev - (int)(BB / Bmax* Top *K);
				pDC->LineTo(P);
			} // i
//		if (fabs(BB) < 1.e-6) AfxMessageBox("Bx = 0");

		pDC->SelectObject(&GreenPen);	
		P.x = X0; P.y = Z0 - Zlev;
		pDC->MoveTo(P);
			for (i = 0; i < Nx; i++) {
				BB = ValB[i][j].Y;
				if (i<=itr)	P.x = X0 + (int)(i * StepX1 * Scale.X);
				else P.x = X0 + (int)((itr * StepX1 + (i-itr)* StepX2) * Scale.X);
				P.y = Z0 - Zlev - (int)(BB / Bmax* Top *K);
				pDC->LineTo(P);
			} // i
//		if (fabs(BB) < 1.e-6) AfxMessageBox("By = 0");


		pDC->SelectObject(&BluePen);	
		P.x = X0; P.y = Z0 - Zlev;
		pDC->MoveTo(P);
			for (i = 0; i < Nx; i++) {
				BB = ValB[i][j].Z;
				if (i<=itr)	P.x = X0 + (int)(i * StepX1 * Scale.X);
				else P.x = X0 + (int)((itr * StepX1 + (i-itr)* StepX2) * Scale.X);
				P.y = Z0 - Zlev - (int)(BB / Bmax* Top *K);
				pDC->LineTo(P);
			} // i
	//	if (fabs(BB) < 1.e-6) AfxMessageBox("Bz = 0");


		pDC->SelectObject(pOldPen);
//	} // j

//		AfxMessageBox("OK");
/*
	for (j = 1; j<Nz; j=j+2) {
		z = -0.7 + j*StepZ;
		Zlev = (int)((-0.7 + j*StepZ)*Scale.Z);
		pDC->SelectObject(pOldPen);
		P.x = X0; P.y = Z0 - Zlev;
		pDC->MoveTo(P);
		P.x = X0 + (int)(Xmax * Scale.X);
		pDC->LineTo(P);

	    pDC->SelectObject(&RedPen);	
		P.x = X0; P.y = Z0 - Zlev;
		x = 0; 
		pDC->MoveTo(P);
			for (i = 0; i < Nx; i++) {
				BB = GetB(x,z).X;
				if (i<=itr)	 x = i * StepX1; 
				else  x = itr * StepX1 + (i-itr)* StepX2;
				P.x = X0 + (int)(x * Scale.X);
				P.y = Z0 - Zlev - (int)(BB / Bmax* Top *K);
				pDC->LineTo(P);
			} // i
		if (fabs(BB) < 1.e-6) AfxMessageBox("Bx = 0");

		pDC->SelectObject(&GreenPen);	
		P.x = X0; P.y = Z0 - Zlev;
		x = 0;
		pDC->MoveTo(P);
			for (i = 0; i < Nx; i++) {
				BB = GetB(x,z).Y;
				if (i<=itr)	 x = i * StepX1; 
				else  x = itr * StepX1 + (i-itr)* StepX2;
				P.x = X0 + (int)(x * Scale.X);
				P.y = Z0 - Zlev - (int)(BB / Bmax* Top *K);
				pDC->LineTo(P);
			} // i
		if (fabs(BB) < 1.e-6) AfxMessageBox("By = 0");


		pDC->SelectObject(&BluePen);	
		P.x = X0; P.y = Z0 - Zlev;
		x = 0;
		pDC->MoveTo(P);
			for (i = 0; i < Nx; i++) {
				BB = GetB(x,z).Z;
				if (i<=itr)	 x = i * StepX1; 
				else  x = itr * StepX1 + (i-itr)* StepX2;
				P.x = X0 + (int)(x * Scale.X);
				P.y = Z0 - Zlev - (int)(BB / Bmax* Top *K);
				pDC->LineTo(P);
			} // i
		if (fabs(BB) < 1.e-6) AfxMessageBox("Bz = 0");


		pDC->SelectObject(pOldPen);
	} // j


	pDC->SelectObject(pOldPen);

}*/

/////////////////////////////////////////////////////////////////////////////////////////
//class CMagField3D // 3D-field B(x,y,z)
/*
	public:
	int Nx, Ny, Nz;
	double StepX, StepY, StepZ;
	C3Point ***  ValB; 
	double Xmin, Xmax, Ymin, Ymax, Zmin, Zmax, Bmax;  
	*/

CMagField3D:: CMagField3D()
{
	Nx = Ny = Nz = 0;
	ValB = new C3Point ** [Nx];  // B
	for (int i = 0; i < Nx; i++) {
		ValB[i] = new C3Point * [Ny];
		for (int j = 0; j < Ny; j++) {
			ValB[i][j] = new C3Point[Nz];
			for (int k = 0; k < Nz; k++) ValB[i][j][k] = 0;
		} // j
	} //i
	Bmax = 0;
	Xmin = Xmax = 0;
	Ymin = Ymax = 0;
	Zmin = Zmax = 0;
	//StepX = StepY = StepZ = 1;
}

CMagField3D:: ~CMagField3D()
{
	for (int i = 0; i < Nx; i++) {
		for (int j = 0; j < Ny; j++) {
			delete [] ValB[i][j];
		}
		delete [] ValB[i];
	}
	delete [] ValB;
}

int CMagField3D:: Init(char * name) // define + set geometry
{
	// to be added - data file analisys
	FILE * fin = fopen(name, "r");
	if (fin == NULL) return -1;
	int commlines = 0, datalines = 0;
	char buf[1024];
	int result = 0;
	double  x, y, z, Bx, By, Bz;
	double eps = 0.001; // 1 mm precision
	int i,j,k;
	BOOL exists = FALSE;
	Xmin  = 1000; Xmax = -1000;
	Ymin  = 1000; Ymax = -1000;
	Zmin  = 1000; Zmax = -1000;
	Nx = 0, Ny = 0, Nz = 0;
	ArrX.RemoveAll(); ArrY.RemoveAll(); ArrZ.RemoveAll();

	while  (!feof(fin)) 
	{
		fgets(buf, 1024, fin);
		//result = sscanf(buf, "%lf %lf %lf %lf %lf %lf", &x, &y, &z, &Bx, &By, &Bz);
		result = fscanf(fin, "%le %le %le %le %le %le",  &x, &y, &z, &Bx, &By, &Bz);
		//result = sscanf(buf, "%le %le %le %le %le %le",  &x, &y, &z, &Bx, &By, &Bz);
		if (result < 6) {
			//if (datalines < 1)	// only header
			commlines++;   	
			continue;
		}
		else datalines++; // result = 6

		exists = FALSE; // x
		for (i = 0; i <= ArrX.GetUpperBound(); i++) {
			if (fabs(ArrX[i] - x) < eps) {
				exists = TRUE;// equal
				break;
			}
		}
		if (!exists) {	ArrX.Add(x); Xmin = Min(Xmin, x); Xmax = Max(Xmax, x);	}

		exists = FALSE; // y
		for (i = 0; i <= ArrY.GetUpperBound(); i++) {
			if (fabs(ArrY[i] - y) < eps) {
				exists = TRUE;// equal
				break;
			}
		}
		if (!exists) {	ArrY.Add(y); Ymin = Min(Ymin, y); Ymax = Max(Ymax, y);	}

		exists = FALSE; // z
		for (i = 0; i <= ArrZ.GetUpperBound(); i++) {
			if (fabs(ArrZ[i] - z) < eps) {
				exists = TRUE;// equal
				break;
			}
		}
		if (!exists) {	ArrZ.Add(z); Zmin = Min(Zmin, z); Zmax = Max(Zmax, z);	}

	} // feof
	fclose(fin);

	SortDoubleArray(ArrX); SortDoubleArray(ArrY); SortDoubleArray(ArrY); 
	Nx = ArrX.GetSize(); Ny = ArrY.GetSize(); Nz = ArrZ.GetSize();

	CString S, S1;
	S.Format("%s\nComments %d; Data %d lines\n Nx = %d Ny = %d Nz = %d\n Xmib = %le Xmax = %le\n Ymin = %le Ymax = %le\n Zmin = %le Zmax = %le",
		//name, commlines, datalines, Nx, Ny, Nz, Xmin, Xmax, Ymin, Ymax, Zmin, Zmax);
		name, commlines, datalines, Nx, Ny, Nz, ArrX[0], ArrX[Nx-1], ArrY[0], ArrY[Ny-1], ArrZ[0], ArrZ[Nz-1]);
	if (Nx*Ny*Nz != datalines) 
		S1.Format("\n\n  Matrix dimensions Nx*Ny*Nz should be equal to data lines number\n Some data are missed");
	else S1.Format("\n\n Matrix is OK");
	AfxMessageBox(S + S1);
		
	return 1; //comlines;
}

int CMagField3D:: ReadData(char * name) // fill B array
{
	if (Init(name) < 0) return -1;
	FILE * fin = fopen(name, "r");
	if (fin == NULL) return -1;
	char buf[1024];
	int result;
	double Bx, By, Bz, BB, BBmax = 0;
	double  x, y, z;
	C3Point P;
	int i,j,k, n;
		
	ValB = new C3Point ** [Nx];  // B
	for (int i = 0; i < Nx; i++) {
		ValB[i] = new C3Point * [Ny];
		for (int j = 0; j < Ny; j++) {
			ValB[i][j] = new C3Point[Nz];
			for (int k = 0; k < Nz; k++) ValB[i][j][k] = 0;
		} // j
	} //i

	Bmax = 0;

	n = 0;
	while  (!feof(fin)) 
	{
		fgets(buf, 1024, fin);
		result = fscanf(fin, "%le %le %le %le %le %le",  &x, &y, &z, &Bx, &By, &Bz);
		if (result < 6) continue;
	
		i = GetDoubleArrayPos(ArrX, x);
		if (i < 0 || i >= Nx) continue; 
		j = GetDoubleArrayPos(ArrY, y);
		if (j < 0 || j >= Ny) continue; 
		k = GetDoubleArrayPos(ArrZ, z);
		if (k < 0 || k >= Nz) continue; 
				
		BB = sqrt(Bx*Bx + By*By + Bz*Bz);
		BBmax = Max(BBmax, BB);
	
		P.X = Bx; P.Y = By; P.Z = Bz;
		ValB[i][j][k] = P;
		n++;
	
	}
	fclose(fin);
	Bmax = BBmax;
	return (n);
}

C3Point CMagField3D:: GetB(double x, double y, double z)
{
	C3Point B(0,0,0);
	if (Ny < 2) { // 2 dimensions X-Z
		if (x < Xmin || x > Xmax) return B;
		if (z <= Zmin) z = Zmin;
		if (z >= Zmax) z = Zmax;
		return GetB_XZ(x,z);
	}
	int i,j,k;
	i = GetDoubleArrayPos(ArrX, x);
	j = GetDoubleArrayPos(ArrY, y);
	k = GetDoubleArrayPos(ArrZ, z);
	if (i < 0 || i >= Nx || j < 0 || j >= Ny || k < 0 || k >= Nz) return C3Point(0,0,0);
	//else return ValB[i][j][k];

	double Vol, a1, a2, b1, b2, c1, c2;
	double StepX, StepY, StepZ;
	C3Point B1, B2, B3, B4, B5, B6, B7, B8;
	int i1, j1, k1;
	i1 = i+1;	if (i1 >= Nx) i1 = i-1;
	j1 = j+1;	if (j1 >= Ny) j1 = j-1;
	k1 = k+1;	if (k1 >= Nz) k1 = k-1;
	
	StepX = fabs(ArrX[i] - ArrX[i1]);
	StepY = fabs(ArrY[j] - ArrY[j1]);
	StepZ = fabs(ArrZ[k] - ArrZ[k1]);

	Vol = StepX * StepY * StepZ;
	a1 = x - ArrX[i]; //(x - Xmin) - i * StepX;	
	if (a1 < 0) a1 = 0;
	a2 = fabs(ArrX[i1] - x); //StepX - a1;
	b1 = y - ArrY[j]; //(y - Ymin) - j * StepY;	
	if (b1 < 0) b1 = 0;
	b2 = fabs(ArrY[j1] - y); //StepY - b1;
	c1 = z - ArrZ[k]; //(z - Zmin) - k * StepZ;	
	if (c1 < 0) c1 = 0;
	c2 = fabs(ArrZ[k1] - z); //StepZ - c1;


	B1 = ValB[i][j][k];
	B2 = ValB[i1][j][k];
	B3 = ValB[i1][j1][k];
	B4 = ValB[i][j1][k];
	B5 = ValB[i][j][k1];
	B6 = ValB[i1][j][k1];
	B7 = ValB[i1][j1][k1];
	B8 = ValB[i][j1][k1];

	B = (B1 * a2 * b2 * c2 + B2 * a1 * b2 * c2 + B3 * a1 * b1 * c2 + B4 * a2 * b1 * c2 +
		 B5 * a2 * b2 * c1 + B6 * a1 * b2 * c1 + B7 * a1 * b1 * c1 + B8 * a2 * b1 * c1) / Vol;

	return B;
}

C3Point CMagField3D:: GetB_XZ(double x, double z)
{
	int i, k;
	i = GetDoubleArrayPos(ArrX, x);
	k = GetDoubleArrayPos(ArrZ, z);
	if (i < 0) return C3Point(0,0,0);
	
	double StepX,  StepZ;
	double  a1, a2,  c1, c2;
	C3Point B1, B2, B3, B4, Bleft, Bright, Bup, Bdown, Result1, Result2, Result;
	int i1,  k1;
	i1 = i+1;	if (i1 >= Nx) i1 = i-1;
	k1 = k+1;	if (k1 >= Nz) k1 = k-1;
	
	StepX = fabs(ArrX[i] - ArrX[i1]);
	StepZ = fabs(ArrZ[k] - ArrZ[k1]);
	
	a1 = x - ArrX[i]; //(x - Xmin) - i * StepX;	
	if (a1 < 0) a1 = 0;
	a2 = fabs(ArrX[i1] - x); //StepX - a1;
	
	c1 = z - ArrZ[k]; //(z - Zmin) - k * StepZ;	
	if (c1 < 0) c1 = 0;
	c2 = fabs(ArrZ[k1] - z); //StepZ - c1;

	B1 = ValB[i][0][k];
	B2 = ValB[i1][0][k];
	B3 = ValB[i][0][k1];
	B4 = ValB[i1][0][k1];

	Bleft = B1 + (B3 - B1) * c1 / StepZ;
	Bright = B2 + (B4 - B2) * c1 / StepZ;
	Result1 = Bleft + (Bright - Bleft) * a1 / StepX;

	Bup = B3 + (B4 - B3) * a1 / StepX;
	Bdown = B1 + (B2 - B1) * a1 / StepX;
	Result2 = Bdown + (Bup - Bdown) * c1 / StepZ;
	
	Result = (Result1 + Result2) / 2;

	return Result;
}

C3Point CMagField3D:: GetB(C3Point P)
{
	return GetB(P.X, P.Y, P.Z);
}



void CMagField3D:: DrawMF(CDC* pDC, C3Point & Origin, C3Point & Scale, double xmax, double Z)
{
	int X0, Y0, Z0, Top = 50;

	CPoint P;
	int i, j, k;
	double x, y, z;
	X0 = (int) Origin.X;
	//Y0 = (int) Origin.Y; 
	Z0 = (int) Origin.Z;
	C3Point B0;
	double BB;
//	double step = 0.2;
	double K = 1;//Zoom;
	int Yax, Zax;// = (int) ceil(z * Scale.Z);
	
	
	CPen  RedPen, GreenPen, BluePen;
	CPoint prevPlan, prevSide, nextPlan, nextSide;

	if (!RedPen.CreatePen(PS_DOT, 2, RGB(200, 0, 0))) return;
	if (!GreenPen.CreatePen(PS_DOT, 2, RGB(0, 200, 0))) return;
	if (!BluePen.CreatePen(PS_DOT, 2, RGB(0, 0, 200))) return;

	CPen * pOldPen = pDC->SelectObject(&RedPen); 

	x = 0.0;
	y = 0.0;
	z = Z;//0.0;
	Yax = (int)(y * Scale.Y);
	Zax = (int)(z * Scale.Z);

	//C3Point Max = GetBmaxYZ(xmax, y, z);
	double AbsMax = Bmax; //ModVect(Max);
	if (AbsMax < 1.e-12) return;

	pDC->SelectObject(pOldPen);
	P.x = X0; P.y = Z0 - Zax;
	pDC->MoveTo(P);
	P.x = X0 + (int)(xmax * Scale.X);
	pDC->LineTo(P); // axis

	    pDC->SelectObject(&RedPen);	
		x = 0;
		P.x = X0 + (int)(x * Scale.X); 
		BB = GetB(x, y, z).X;
		P.y = Z0 - Zax - (int)(BB/AbsMax * Top * K);
		pDC->MoveTo(P);
			//while (x < xmax) {
			for (int i = 1; i < Nx; i++) {
				x = ArrX[i];
				//x += 0.2;
				BB = GetB(x, y, z).X;
				P.x = X0 + (int)(x * Scale.X);
				P.y = Z0 - Zax - (int)(BB/AbsMax * Top *K);
				pDC->LineTo(P);
			} // x < Xmax

		pDC->SelectObject(&GreenPen);
		x = 0;
		P.x = X0 + (int)(x * Scale.X);
		BB = GetB(x, y, z).Y;
		P.y = Z0 - Zax - (int)(BB/AbsMax * Top * K);
		pDC->MoveTo(P);
			//while (x < xmax) {
			for (int i = 1; i < Nx; i++) {
				x = ArrX[i];
				//x += 0.2;
				BB = GetB(x, y, z).Y;
				P.x = X0 + (int)(x * Scale.X);
				P.y = Z0 - Zax - (int)(BB/AbsMax * Top *K);
				pDC->LineTo(P);
			} // x < Xmax


		double BzL = 0;
		double dL;
		double Bold;
		pDC->SelectObject(&BluePen);
		x = 0;
		P.x = X0 + (int)(x * Scale.X); 
		BB = GetB(x, y, z).Z;
		Bold = BB;
		//BzL  = BB;
		P.y = Z0 - Zax - (int)(BB/AbsMax * Top * K);
		pDC->MoveTo(P);
			//while (x < xmax) {
			for (int i = 1; i < Nx; i++) {
				dL = ArrX[i] - x;
				x = ArrX[i];
				//x += 0.2;
				BB = GetB(x, y, z).Z;
				//if (x <= 1.01)
				BzL += (Bold + BB) * 0.5 * dL;
				Bold = BB;
				P.x = X0 + (int)(x * Scale.X);
				P.y = Z0 - Zax - (int)(BB/AbsMax * Top *K);
				pDC->LineTo(P);
			} // x < Xmax
			CString S;
			S.Format("Z = %g  BL = %g", Z, BzL);
			pDC->TextOut(P.x, P.y, S);


		pDC->SelectObject(pOldPen);
}

/////////////////////////////////////////////////////////////////////////////////////////
CGasField:: CGasField()
{
	Pmax = 0;
	Xmax = 0;
	Nx = 0;
}

CGasField:: ~CGasField()
{
	GasArray.RemoveAll();
}

void CGasField:: Clear()
{
	GasArray.RemoveAll();
}
/*CString  CGasField:: OpenDefault(char* name, double Mult)
{
	FILE * fin;
	CString newname;
	WIN32_FIND_DATA fData;
	HANDLE h;
	h = ::FindFirstFile("gasprof*", &fData);
	if (h != (HANDLE) 0xFFFFFFFF) newname = fData.cFileName;
	
	//char fullname[1024];
	if ((fin= fopen(newname, "r")) == NULL) {
		CString S ="Please, set path to file \n ";
		S+= name;
		AfxMessageBox(S);
			
		CFileDialog dlg(TRUE, NULL, name);
		if (dlg.DoModal() == IDOK) {
			//strcpy(newname, dlg.GetPathName());
			newname = dlg.GetPathName();
			fin = fopen(newname, "r");
		}
	}

	ReadData(newname);
	fclose(fin);
	return(newname);
}*/

int CGasField:: ReadData(char * name)
{
	GasArray.RemoveAll();
	C3Point px;
	char buf[1024];
	int  result;
	int n = 0;
	
	CString S;

	double x, p;
	Pmax = 0; Xmax = 0; Xmin = 1000;
		
	FILE * fin = fopen(name, "r");
	if (fin == NULL) {
		S.Format("%s GAS-file error / not found at Home\n ..\\ %s ", name, pDoc->CurrentDirName);
		AfxMessageBox(S, MB_ICONSTOP | MB_OK);//AfxMessageBox(S);
		std::cout << S << std::endl;
		
		return 0;
	}

	while  (!feof(fin)) 
	{
		fgets(buf, 1024, fin);
		result  = sscanf(buf, "%le  %le", &x, &p);
	//	result = fscanf(fin, "%le  %le", &x, &p);
		if (result < 2) continue;
	//	if (result > 3) fgets(buf, 1024, fin);// ???
		px.X = x; px.Y = p; px.Z = 0; 
		GasArray.Add(px);
		Xmax = Max(Xmax, x);
		Xmin = Min(Xmin, x);
		Pmax = Max(Pmax, p);
		n++;
	}
		
	fclose(fin);
	Nx = n-1;
	std::cout << name << "  GAS - read  " << Nx << " data" << std::endl;
	return (n);
}

 void  CGasField:: DrawGas(CDC* pDC, C3Point & Origin, C3Point & Scale)
 {
	int X0, Y0, Z0, Top = 50;
	X0 = (int) Origin.X;
	Y0 = (int) Origin.Y; 
	Z0 = (int) Origin.Z;
	double x, Pr;
	CPen  Pen;
	if (!Pen.CreatePen(PS_SOLID, 3, RGB(55, 255, 255))) return;
	CPen * pOldPen = pDC->SelectObject(&Pen);
	x = 0;  
	Pr = GetPressure(x);
	pDC->MoveTo(X0+(int)(x * Scale.X), Y0 - (int)(Pr * Top / Pmax));
	x += 0.01;
	while (x <= Xmax) {
		Pr = GetPressure(x);
		pDC->LineTo(X0+(int)(x * Scale.X), Y0 - (int)(Pr * Top / Pmax));
		x += 0.01;
	}
	pDC->SelectObject(pOldPen);
 }

double CGasField:: GetPressure(double x)
 {
	 double P = 0;
	 double x0, x1, P0, P1;
	 if (x < Xmin || x > Xmax) return P;
	 for (int i = 0; i < Nx; i++) {
		 if (x >= GasArray[i].X && x <= GasArray[i+1].X) {
			  x0 = GasArray[i].X;  x1 = GasArray[i+1].X;
			  P0 = GasArray[i].Y;  P1 = GasArray[i+1].Y;
			  P = P0 + (P1-P0) * (x-x0) / (x1-x0);
			  return P;
		 } //if
	 } // for
	 return P;
 }
////// class CGasFieldXZ // 2D const stepX, stepZ 
 
CGasFieldXZ::CGasFieldXZ() 
{
	Xmin = 0; Xmax = 0;
	Zmin = 0; Zmax = 0;
	ValNmax = 0;
	Nx = Nz = 0;
	ValN = NULL;
}

CGasFieldXZ::~CGasFieldXZ()
{
	for (int i = 0; i < Nx; i++) {
		delete [] ValN[i];
	}
	delete [] ValN;
}

int CGasFieldXZ:: Init(char * name) // define + set geometry
{
	// to be added - data file analisys
	FILE * fin = fopen(name, "r");
	if (fin == NULL) return -1;
	int commlines = 0, datalines = 0;
	char buf[1024];
	int result = 0;
	double  x0, z0, x, z, Dens;
	double dx, dz, eps = 0.001; // 1 mm precision
	int i,j,k;
	//BOOL exists = FALSE;
	Xmin  = 1000; Xmax = -1000;
	Zmin  = 1000; Zmax = -1000;
	Nx = 0, Nz = 0;
	x0 = -1000, z0 = -1000;
	StepX = 1000; StepZ = 1000;
	
	while  (!feof(fin)) 
	{
		fgets(buf, 1024, fin);
		result = sscanf(buf, "%lf %lf %lf",  &x, &z, &Dens);
		
		if (result < 3) {
			//fgets(buf, 1024, fin);
			commlines++;   	
			continue;
		}
		else datalines++; // result = 3
		
		dx = fabs(x - x0);
		if (dx > eps) StepX = Min(StepX, dx);
		
		dz = fabs(z - z0);
		if (dz > eps) StepZ = Min(StepZ, dz);

		Xmin = Min(Xmin, x); Xmax = Max(Xmax, x);	
		Zmin = Min(Zmin, z); Zmax = Max(Zmax, z);	

		x0 = x; z0 = z;

	} // feof
	fclose(fin);

	Nx = (int)((Xmax - Xmin) / StepX); 
	Nz = (int)((Zmax - Zmin) / StepZ);

	CString S, S1;
	S.Format("%s\nComments %d; Data %d lines\n Nx = %d Nz = %d\n Xmin = %le Xmax = %le\n Zmin = %le Zmax = %le\n hx = %f Hz = %f",
		name, commlines, datalines, Nx, Nz, Xmin, Xmax, Zmin, Zmax, StepX, StepZ);
		
	//if (Nx*Ny*Nz != datalines) S1.Format("\n\n  Matrix Nx*Ny*Nz should be equal to lines number\n Some data are missed");
	//else S1.Format("\n\n Matrix is OK");
	AfxMessageBox(S);// + S1);
		
	return 1; //comlines;
}

int CGasFieldXZ:: ReadData(char * name) // fill ValN array
{
	if (Init(name) < 0) return -1;

	FILE * fin = fopen(name, "r");
	if (fin == NULL) return -1;
	char buf[1024];
	int result;
	double Dens, Dmax = 0;
	double  x, z;
	//C3Point P;
	int i,k, n;
		
	ValN = new double * [Nx+1];  // N (density)
	for (int i = 0; i <= Nx; i++) {
		ValN[i] = new double [Nz+1];
		for (int k = 0; k <= Nz; k++) ValN[i][k] = 0;
	} //i

	Dmax = 0;

	while  (!feof(fin)) 
	{
		fgets(buf, 1024, fin);
		result = sscanf(buf, "%lf %lf %lf",  &x, &z, &Dens);
		if (result < 3) continue;
	
		i = (int)floor((x - Xmin) / StepX);
		if (i < 0 || i > Nx) continue; 
		
		k = (int)floor((z-Zmin) / StepZ);
		if (k < 0 || k > Nz) continue; 
				
		Dmax = Max(Dmax, Dens);
	
		ValN[i][k] = Dens;
	
	}
	fclose(fin);
	ValNmax = Dmax;
	return (1);
}


double CGasFieldXZ:: GetDensity(double x, double z)
{
	double xloc = x - Xmin;
	double zloc = z - Zmin;
	double hx = StepX;
	double hz = StepZ;
	int i = (int) floor(xloc/hx);
	int k = (int) floor(zloc/hz);
	if (i<0 || i >= Nx || k<0 || k >= Nz) return 0;
	double dx = xloc - i*hx;
	double dz = zloc - k*hz;
	double n00, n10, n01, n11;

	n00 = ValN[i][k];	
	n10 = ValN[i+1][k];
	n01 = ValN[i][k+1]; 
	n11 = ValN[i+1][k+1];
	
	double N =  (n00 + (n10 - n00) * dx/hx) * (hz - dz)/hz + (n01 + (n11 - n01) * dx/hx) * dz/hz;// 
	return N; 
}
void CGasFieldXZ:: DrawGas(CDC* pDC, C3Point & Origin, C3Point & Scale)
{
	int X0, Y0, Z0, Top = 100;
	X0 = (int) Origin.X;
	Y0 = (int) Origin.Y; 
	Z0 = (int) Origin.Z;
	double x, z, Pr;
	CPen  Pen;
	if (!Pen.CreatePen(PS_SOLID, 3, RGB(35, 255, 255))) return;
	CPen * pOldPen = pDC->SelectObject(&Pen);
	x = Xmin; Pr = GetDensity(x, 0);  
	pDC->MoveTo(X0 + (int)(x * Scale.X), Y0 - (int)(Pr * Top / ValNmax));//(X0, Y0); 
	x += 0.01;
	while (x < Xmax) {
		Pr = GetDensity(x, 0);
		pDC->LineTo(X0 + (int)(x * Scale.X), Y0 - (int)(Pr * Top / ValNmax));
		x += 0.01;
	}
/*
	x = 0;  X0 = (int) Origin.X;
	z = (Zmin + Zmax) / 2;	Z0 = (int) (Origin.Z - z * Scale.Z);
	pDC->SelectObject(pOldPen);//axis
	pDC->MoveTo(X0, Z0);
	pDC->LineTo(X0 + (int)(Xmax * Scale.X), Z0);
	
	pDC->SelectObject(&Pen);
	pDC->MoveTo(X0, Z0); 
	while (x < Xmax) {
		Pr = GetDensity(x, z);
		pDC->LineTo(X0+(int)(x * Scale.X), Z0 - (int)(Pr * Top / ValNmax));
		x += 0.01;
	}*/

	x = 0.5;  X0 = (int)(Origin.X + x * Scale.X);
	z =  Zmin;	Z0 = (int)(Origin.Z);
	pDC->SelectObject(pOldPen); //axis
	pDC->MoveTo(X0, Z0 - (int)(Zmin * Scale.Z));
	pDC->LineTo(X0, Z0 - (int)(Zmax * Scale.Z));
	
	pDC->SelectObject(&Pen);
	Pr = GetDensity(x, z);
	pDC->MoveTo(X0 + (int)(Pr * Top / ValNmax), Z0 - (int)(z * Scale.Z)); 
	while (z < Zmax) {
		Pr = GetDensity(x, z);
		pDC->LineTo(X0 + (int)(Pr * Top / ValNmax), Z0 - (int)(z * Scale.Z));
		z += 0.01;
	}

	pDC->SelectObject(pOldPen);
}

//////////////////////////////////////////////////////////////////////////////
CRIDField::CRIDField()
{
	Nx = 0; Ny = 0; Umax = 0;
/*	Nx = 700; //max index
	Ny = 49; //max index
	hx = 0.005;
	hy = 0.005;
	Umax = 0;

	NodeU = new double * [Nx+1];
	for (int i = 0; i <= Nx; i++) {
		NodeU[i] = new double [Ny+1];
		for (int j = 0; j <= Ny; j++) NodeU[i][j] = 0;
	}
*/
}
CRIDField::~CRIDField()
{
	if (Nx * Ny > 0) DeleteArray();
}

void CRIDField:: DeleteArray()
{
	for (int i = 0; i <= Nx; i++) {
		delete [] NodeU[i];
	}
	delete [] NodeU;
}

/*
void CRIDField:: Set(double RIDU,  double RIDH, 
						double  RIDInW, double RIDOutW, 
						double RIDInX, double RIDOutX, 
						double RIDTh)
{
	U = RIDU; 
	Height = RIDH;
	InW = RIDInW;
	OutW = RIDOutW;
	InX =  RIDInX;
	OutX = RIDOutX; 
	Thickness = RIDTh;
}

void CRIDField:: Set()
{
	U = *pDoc->RIDU; 
	Height = *pDoc->RIDH;
	InW = *pDoc->RIDInW;
	OutW = *pDoc->RIDOutW;
	InX =  *pDoc->RIDInX;
	OutX = *pDoc->RIDOutX; 
	Thickness = *pDoc->RIDTh;
}*/

C3Point  CRIDField:: GetE(double x, double y, double z)
{
	if (pDoc->OptFree) return C3Point(0,0,0);
	int N = (int)pDoc->RIDChannels; // vert channels
	double StepIn = pDoc->RIDInW + pDoc->RIDTh; // inlet step between adjacent channels axes (= between panels)
	double StepOut = pDoc->RIDOutW + pDoc->RIDTh;// outlet step between adjacent channels axes
	double YminIn = - StepIn * N * 0.5 + pDoc->RIDBiasInHor; // right-most panel Y  
	double YminOut = - StepOut * N * 0.5 + pDoc->RIDBiasOutHor;// right-most panel Y  
	double U = pDoc->RIDU; 
//	double Height = *pDoc->RIDH;
	double Zmin = pDoc->VShiftRID - 0.5 * pDoc->RIDH + pDoc->RIDBiasInVert;
	double Zmax = pDoc->VShiftRID + 0.5 * pDoc->RIDH + pDoc->RIDBiasInVert;
	double InX =  pDoc->RIDInX;
	double OutX = pDoc->RIDOutX; 
	double Yin, Yout, Yx, GapX;
	double Eabs; 
	double ExtX = 0;//3*StepIn;

	double xf, yf; // local for field area

	C3Point E(0,0,0);
	C3Point Eloc;

	if (!pDoc->RIDFieldLoaded) {//(fabs(Umax) < 0.001) { // FLAT field - not calculated or from file, use RIDU

	if (x < InX - ExtX || x > OutX || z < Zmin || z > Zmax) return E;

	for (int i = 0; i < N; i++) {
		Yin = YminIn + (i+0.5) * StepIn; // dist to channel axis at RID inlet
		Yout = YminOut + (i+0.5) * StepOut;// dist to channel axis at RID outlet
		Yx = Yin + (x - InX) * (Yout - Yin) / (OutX - InX);// dist to channel axis at x
		GapX = StepIn + (x - InX) * (StepOut - StepIn) / (OutX - InX);// gap's width at x
		if (fabs(y - Yx) < 0.5 * GapX) {// y within the channel
			if (x < InX) { // Edge-fold
				Eabs = 0;
				//Eabs = -U *1000 / (GapX - *pDoc->RIDTh) * (x - InX + ExtX) / ExtX;
			}
			else { // E-plato
				Eabs = -U * 1000 / (GapX - pDoc->RIDTh);
			}
			
			if (Odd(i) == FALSE) E.Y = Eabs; // Channels 0,2 ... E directed Up
			else E.Y = -Eabs; // Channels 1,3 ... E directed Down

			E.Z = U * 1000;

			return E;
		}
	}
} // not from file

else { // calculated or taken from file RIDFieldLoaded = TRUE

	if (x <= Xmin || x > Xmin + Nx*hx || z < Zmin || z > Zmax) return E;

	// Full Field option -ALL CHANNELS are calculated by other code !!
/*	xf = x - Xmin;// xloc
	yf = y - Ymin;// yloc
	Eloc = GetPoissonE(xf, yf);
	E = Eloc;
	//E.Z = U; //!!!!!!
	return E;
*/			

// one channel is calculated by BTR
	for (int i = 0; i < N; i++) { // Field in one channel is calculated
		Yin = YminIn + (i+0.5) * StepIn; // dist to channel axis at RID inlet
		Yout = YminOut + (i+0.5) * StepOut;// dist to channel axis at RID outlet
		Yx = Yin + (x - InX) * (Yout - Yin) / (OutX - InX);// dist to channel axis at x
		GapX = StepIn + (x - InX) * (StepOut - StepIn) / (OutX - InX);// gap's width at x
		xf = x - Xmin;// xloc
		//	yf = y - (Yx - 0.5 * GapX);
		if (fabs(y - Yx) <= 0.5 * GapX) {// y within a channel
			if (Odd(i) == FALSE) // Channels 0,2 ... E directed Up, 0 at the bottom
				yf = y - (Yx - 0.5 * GapX); 
			else yf = (Yx + 0.5 * GapX)- y;
			Eloc = GetPoissonE(xf, yf);
			E.X = Eloc.X;
			E.Z = Eloc.Z;
			if (Odd(i) == FALSE) E.Y = Eloc.Y; // Channels 0,2 ... E directed Up
			else E.Y = -Eloc.Y; // Channels 1,3 ... E directed Down
		
			return E;
		} // if y within channel
	}  // one channel option

} // taken from file
	

	return E;
}

int CRIDField:: ReadFullFieldInfo(FILE * fin)
{
	double StepH = 0, StepV = 0, Hmin, Vmin, Hmax, Vmax;//, Zmax, Scell;
	char buf[1024];
	int nx, ny;
	//CString line;

	if (fgets(buf,1024,fin) == NULL) { AfxMessageBox("Problem with line 1"); 
		fclose(fin); return 0; }
		int result = sscanf(buf, "%d %d", &nx, &ny); 
		if (result != 2) {
			while (!feof(fin) && result != 2) {
				fgets(buf, 1024, fin);
				result = sscanf(buf, "%d %d", &nx, &ny);
			}
			if (feof(fin)) { AfxMessageBox("Invalid Format of 1-st line", MB_ICONSTOP | MB_OK); 
				fclose(fin); return 0;}
			if (nx*ny <= 1.e-6) {AfxMessageBox("Invalid numbers Nx, Ny", MB_ICONSTOP | MB_OK);
				fclose(fin); return 0;}
		}

	/*	line = buf; // find comment
		int pos = line.Find("#");
		if (pos < 0) {
			plate0->Comment = "RID Full Field";// "NO COMMENT";
		}
		else {
			if (pos >= 0) line = line.Mid(pos+1);
			pos = line.Find("\n");
			if (pos > 0) line = line.Left(pos);
			plate0->Comment += line;
		}*/

		if (fgets(buf,1024,fin) == NULL) { AfxMessageBox("Problem with line 2"); 
			fclose(fin); return 0; }
		result = sscanf(buf, "%le %le", &Hmin, &StepH); 
		if (result != 2) {AfxMessageBox("Invalid line 2"); fclose(fin); return 0;}
		if (StepH < 1.e-6) {AfxMessageBox("Hor. Step = 0!",MB_ICONSTOP | MB_OK);
			fclose(fin); return 0;}
		//axisX = buf;

		if (fgets(buf,1024,fin) == NULL) { AfxMessageBox("Problem with line 3");
			fclose(fin); return 0; }
		result = sscanf(buf, "%le %le", &Vmin, &StepV); 
		if (result != 2) {AfxMessageBox("Invalid line 3"); 
			fclose(fin); return 0;}
		if (StepV < 1.e-6) {AfxMessageBox("Vert. StepY = 0!",MB_ICONSTOP | MB_OK); 
			fclose(fin); return 0;}
		//axisY = buf;
		
	/*	if (fgets(buf,1024,fin) == NULL) { AfxMessageBox("Problem with line 4"); fclose(fin); return NULL; }
		result = sscanf(buf, "%le %le", &Scell, &Zmax); 
		if (result != 2) {AfxMessageBox("Invalid line 4"); fclose(fin); return NULL;}
		if (Zmax < 1.e-12) {AfxMessageBox("Zmax < 1.e-12",MB_ICONSTOP | MB_OK); fclose(fin); return NULL;}
	*/

	Nx = nx; Ny = ny;
	Xmin = Hmin; Ymin = Vmin;
	hx = StepH; hy = StepV;
	Xmax = Xmin + Nx * hx;
	Ymax = Ymin + Ny * hy;
	return 1;
}

int CRIDField:: ReadFullField(char * name) //precalculated Full field
{
//	Fieldname = name;
	Umax = 0;
	int nx = Nx, ny = Ny, i, j, iter;
	double u, umax = 0;
	double x, y;
	char buf[1024];
	CString n = name;
	FILE * fin = fopen(name, "r");
	CString S;
	if (Nx != 0 || Ny != 0)  //old exists
		DeleteArray();

	if (fin == NULL) {
		AfxMessageBox("File not found - " + n);
		return 0;
	}
	else if (ReadFullFieldInfo(fin) == 0) {
		AfxMessageBox("information not found - " + n);
		fclose(fin);
		return 0;
	}

	if (Nx != 0 || Ny != 0) { //
		NodeU = new double * [Nx+1];
		for (i = 0; i <= Nx; i++) {
			NodeU[i] = new double [Ny+1];
			for (j = 0; j <= Ny; j++) NodeU[i][j] = 0;
		}
	}
//	Iter = iter;

	fin = fopen(name, "r");
	fgets(buf, 1024, fin);
	fgets(buf, 1024, fin);
	fgets(buf, 1024, fin);
	fgets(buf, 1024, fin);

	int lines = 0; // read data
	//while (!feof(fin)) {
	for (i = 0; i <= Nx; i++)
		for (j = 0; j <= Ny; j++) {
		fgets(buf, 1024, fin);
		int res = sscanf(buf, "%le %le %le", &x, &y, &u);
		if (res != 3) break; //continue;
		//xmin = Min(xmin, x); ymin = Min(ymin, y);
		umax = Max(umax, fabs(u));
		//i = (int)((x - Xmin)/hx);
		//j = (int)((y - Ymin)/hy);
		//if (i <= Nx && j <= Ny) {
			NodeU[i][j] = u; // Laplas all channels
			lines ++;
		//}
	}

	fclose(fin);
	if (lines < (Nx+1) * (Ny+1)) {
		AfxMessageBox("too short");
		return 0;
	}

	Umax = umax; //kV
	if (fabs(Umax) < 1.e-12) Umax = 1;
	for (i = 0; i <= Nx; i++) 
		for (j = 0; j <= Ny; j++)
			NodeU[i][j] *= (pDoc->RIDU * 1000) / Umax; //as for umax = 1kv
	
	//S.Format("RID potential is set to %g V!", Umax);
	//AfxMessageBox(S);
	//pDoc->RIDU = Umax/1000;//->kV
	//	return 1;
	//}

	S.Format("RID Nx = %d Ny = %d RIDU = %g kV \n data - %d x %d = %d lines  umax = %g", 
		Nx, Ny, pDoc->RIDU, i,j, lines, umax);

	//AfxMessageBox("RID Full field (5 channels) \n" +S);

	Umax = pDoc->RIDU * 1000;
	
	//WriteTab("RID-full.txt");

	return 1;

}

void CRIDField:: WriteTab(char * name)
{
	CString n = name;
	FILE *fout = fopen(name, "w");
	fprintf(fout, " %d  %d # RID Full Field \n", Nx, Ny);
	fprintf(fout, " %g  %g \n", Xmin, hx);
	fprintf(fout, " %g  %g \n", Ymin, hy);
	fprintf(fout, " 0  %g \n", fabs(Umax));//Scell Zmax
//	fprintf(fout, " i   j   Uij \n");
	for (int j=0; j <= Ny; j++) {
	for (int i=0; i <= Nx; i++) fprintf(fout, "%le\t", fabs(NodeU[i][j])); 
		fprintf(fout, "\n");
	}
	fclose(fout);
	AfxMessageBox(n+ " is written");
}

void CRIDField:: Copy(int nx, int ny, double stepX, double stepY, double xmin, double ** nodeU)
{
	if (Nx * Ny > 0) DeleteArray();

	Nx = nx; Ny = ny;
	hx = stepX; hy = stepY;
	Xmin = xmin;

	NodeU = new double * [Nx+1];
	for (int i = 0; i <= Nx; i++) {
		NodeU[i] = new double [Ny+1];
		for (int j = 0; j <= Ny; j++) {
			NodeU[i][j] = nodeU[i][j];
			Umax = Max(Umax, fabs(NodeU[i][j]));
		}
	}
}


C3Point CRIDField:: GetPoissonE(double xloc, double yloc) // Ex, Ey, U in a local point: U is taken from file
{
	C3Point E(0,0,0);

	int i = (int) floor(xloc/hx);
	int j = (int) floor(yloc/hy);
	if (i<=0 || i>=Nx || j<=0 || j>=Ny) return E;
	double dx = xloc - i*hx;
	double dy = yloc - j*hy;
	double u00, u10, u01, u11;

	u00 = NodeU[i][j];	
	u10 = NodeU[i+1][j];
	u01 = NodeU[i][j+1]; 
	u11 = NodeU[i+1][j+1];
	E.X = - (u10 - u00 + (u11 - u10 - u01 + u00) *dy/hy) / hx;
	E.Y = - (u01 - u00 + (u11 - u01 - u10 + u00) *dx/hx) / hy;
	E.Z =  (u00 + (u10 - u00) * dx/hx) * (hy - dy)/hy + (u01 + (u11 - u01) * dx/hx) * dy/hy;// u

	return E;
}

void  CRIDField:: DrawU(CDC* pDC, C3Point & Origin, C3Point & Scale)
 {
	 if (pDoc->OptFree) return;
	 double Xmax;// = Xmin + Nx * hx;
	 if (pDoc->RIDFieldLoaded) {
		 Xmax = Xmin + Nx * hx;
	 }
	 else { // flat model
		 Xmin = pDoc->NeutrOutX; //RIDInX;
		 Xmax = pDoc->CalInX; //RIDOutX;
		// Umax = pDoc->RIDU * 1000;
	 }
	Umax = pDoc->RIDU * 1000;

	CString S;
	S.Format("Xmin = %g Xmax = %g Umax = %g", Xmin, Xmax, Umax);
	//AfxMessageBox(S);

	int X0, Y0, Z0, Top = 150;
	X0 = (int) Origin.X;
	Y0 = (int) Origin.Y; 
	Z0 = (int) Origin.Z;
	double x, y, U;
	/*x = (Xmin + Xmax) * 0.5; y = 0.25;
	pDC->MoveTo(X0+(int)(x * Scale.X), Y0 + (int)(y * Scale.Y));
	pDC->LineTo(X0+(int)(x * Scale.X), Y0 - (int)(y * Scale.Y));
	x = Xmax - 0.2; y = 0.25;
	pDC->MoveTo(X0+(int)(x * Scale.X), Y0 + (int)(y * Scale.Y));
	pDC->LineTo(X0+(int)(x * Scale.X), Y0 - (int)(y * Scale.Y));
*/
	CPen  Pen;
	if (!Pen.CreatePen(PS_SOLID, 3, RGB(255, 107, 23))) return;
	CPen * pOldPen = pDC->SelectObject(&Pen);
	//horizontal profile
	x = Xmin;  
	y = - pDoc->RIDInW * 0.5; // lower
	pDC->MoveTo(X0+(int)(x * Scale.X), Y0);
	while (x < Xmax) {
		U = GetE(x,y,0).Z;
		pDC->LineTo(X0+(int)(x * Scale.X), Y0 + (int)(U * Top / Umax));
		x += 0.01;
	}
	/*x = Xmin;
	y = pDoc->RIDInW * 0.5; // upper
	pDC->MoveTo(X0 + (int)(x * Scale.X), Y0);
	while (x < Xmax) {
		U = -GetE(x, y, 0).Z;
		pDC->LineTo(X0 + (int)(x * Scale.X), Y0 + (int)(U * Top / Umax) - 10);
		x += 0.01;
	}*/
	//vertical profile mid
/*	x = (Xmin + Xmax) * 0.5;
	y = -0.25;
	pDC->MoveTo(X0 + (int)(x * Scale.X), Y0 - (int)(y * Scale.Y));
	while (y < 0.25) {
		U = GetE(x,y,0).Z;
		pDC->LineTo(X0 + (int)(x * Scale.X) - (int)(U * Top / Umax), Y0 - (int)(y * Scale.Y));
		y += 0.005;
	}
	//vertical profile at entry
	x = Xmin + 0.2;
	y = -0.25;
	pDC->MoveTo(X0 + (int)(x * Scale.X), Y0 - (int)(y * Scale.Y));
	while (y < 0.25) {
		U = GetE(x,y,0).Z;
		pDC->LineTo(X0 + (int)(x * Scale.X) - (int)(U * Top / Umax), Y0 - (int)(y * Scale.Y));
		y += 0.005;
	}*/
	pDC->SelectObject(pOldPen);
 }
 ///////////////////////////////////////////////////////////////////////////////////////
///// TARGET ///////////////////////////
void CTarget:: ClearArrays()
{
	Path.RemoveAll();
	Value.RemoveAll();
	Npath = 0;
}

void CTarget::ClearCuts()
{
	if (Nx < 1) return;
	for (int i = 0; i <= Nx; i++) delete[] CutHor[i];
	delete[] CutHor;
	for (int i = 0; i <= Nx; i++) delete[] CutVert[i];
	delete[] CutVert;
	if (Ny < 1) return;
	for (int i = 0; i <= Ny; i++) delete[] CutCS[i];
	delete[] CutCS;
	
}

CTarget::~CTarget()
{
	ClearArrays();
	ClearCuts();
}

void CTarget::SetOriginDim(C3Point start, int nx, int ny, int nz) // + 3 Cuts
{
	Orig = start;
	Nx = nx;
	Ny = ny;
	Nz = nz;
	CutHor = new double *[Nx + 1];
	for (int i = 0; i <= Nx; i++) {
		CutHor[i] = new double[Ny + 1];
		for (int j = 0; j <= Ny; j++) CutHor[i][j] = 0;
	}
	CutVert = new double * [Nx + 1];
	for (int i = 0; i <= Nx; i++) {
		CutVert[i] = new double[Nz + 1];
		for (int j = 0; j <= Nz; j++) CutVert[i][j] = 0;
	}
	CutCS = new double *[Ny + 1];
	for (int i = 0; i <= Ny; i++) {
		CutCS[i] = new double[Nz + 1];
		for (int j = 0; j <= Nz; j++) CutCS[i][j] = 0;
	}
	CString S;
	S.Format("CTarget\n Start %g, %g, %g\n Nx = %d", Orig.X, Orig.Y, Orig.Z, Nx);
	//AfxMessageBox(S);
}

void CTarget::SetZeroCuts()
{
	for (int i = 0; i <= Nx; i++) {
		for (int j = 0; j <= Ny; j++) CutHor[i][j] = 0;
	}
	for (int i = 0; i <= Nx; i++) {
		for (int j = 0; j <= Nz; j++) CutVert[i][j] = 0;
	}
	for (int i = 0; i <= Ny; i++) {
		for (int j = 0; j <= Nz; j++) CutCS[i][j] = 0;
	}
}

void CTarget::SetMeshSteps(C3Point start, C3Point end, double dx, double dy, double dz)
{
	// remesh all cuts
	ClearCuts();//delete
	//dx = max(dx, 0.01);
	//dy = max(dy, 0.01);
	//dz = max(dz, 0.01);
	Step = C3Point(dx, dy, dz);
	int nx = (int)floor((end.X - start.X)/dx);
	int ny = (int)floor((end.Y - start.Y)/dy);
	int nz = (int)floor((end.Z - start.Z)/dz);
	if (ny < 20) {
		ny = ny *2;
		Step.Y = (end.Y - start.Y) / ny;
	}
	if (nz < 20) {
		nz = nz * 2;
		Step.Z = (end.Z - start.Z) / nz;
	}

	
	SetOriginDim(start, nx, ny, nz); // allocate hor/vert/cs cuts
	
	CutStepX = dx; //not used
	CutStepY = dy; //not used
	
	//SetCellSteps(Step);
	//C3Point destX = C3Point(end.X, end.Y, start.Z);
	//C3Point destY = C3Point(start.X, start.Y, end.Z);
	//SetCutPlane(start, destX, destY, 0.1, 0.1); // allocate diag cut
}

void CTarget::SetMesh(C3Point start, C3Point end, int nx, int ny, int nz)
{
/*	if (nx == Nx && ny == Ny && nz == Nz) { // keep old mesh 
		Orig = start;
		CutOrig = start;
		SetZeroCuts();
		return;
	}
*/	
	// remesh all cuts
	ClearCuts();//delete
	SetOriginDim(start, nx, ny, nz); // allocate hor/vert cuts
	//C3Point Step = C3Point(0,0,0);
	if (nx > 0)	Step.X = (end.X - start.X) / nx;
	if (ny > 0)	Step.Y = (end.Y - start.Y) / ny;
	if (nz > 0)	Step.Z = (end.Z - start.Z) / nz;
	//SetCellSteps(Step);
	//C3Point destX = C3Point(end.X, end.Y, start.Z);
	//C3Point destY = C3Point(start.X, start.Y, end.Z);
	//SetCutPlane(start, destX, destY, 0.1, 0.1); // allocate diag cut
}

void CTarget::SetSigma(double sigma)
{
	Sigma = sigma;
}

double CTarget::GetSigma(C3Point pos) //  total  
{
	return Sigma; // if not loaded -> use default 
}
void CTarget::SetDensity(double dens)
{
	Density = dens;
}

double CTarget::GetDens(C3Point pos)
{
	// if not loaded -> use default 
	return Density; 
}


void CTarget:: SetPath(C3Point P0, C3Point P1, double step)// set stright Path array by end points
{
	Path.RemoveAll();
	C3Point vectL = (P1 - P0);
	double Len = ModVect(vectL);
	StepL = step;
	Npath = (int)(Len / StepL);
	//if (Npath < 2) Npath = 1; // last point number
	
	if (Nx > Npath) {
		Npath = Nx;
		StepL = Len / Nx;
	}

	C3Point vectS = vectL / Npath;
	for (int i = 0; i <= Npath; i++){
		Path.Add(P0 + vectS * i);
	}
}
void CTarget::SetPathFrom(C3Point P0, C3Point Direct, double step) 
// set stright Path array by start and V components
{
	Path.RemoveAll();
	double xmax = Orig.X + Nx * Step.X;
	double ldir = ModVect(Direct);
	double LenX = xmax - P0.X;//Nx * Step.X;
	if (Direct.X < 1.e-12) {
		AfxMessageBox("invalid Ray direction\n -> is set along X");
		Direct =  C3Point(1,0,0);
		//return;
	}
	if (ldir < 1.e-12) ldir = LenX * 0.1;
	C3Point Vect = Direct / Direct.X * LenX;
	C3Point P1 = P0 + Vect;
	//C3Point vectL = (P1 - P0);
	double Len = ModVect(Vect);
	StepL = step;
	Npath = (int)ceil(Len / StepL);
	
/*	if (Nx > Npath) {
		Npath = Nx;
		StepL = Len / Nx;
	}*/

	C3Point vectS = Vect / Npath;
	double SumL = 0;
	for (int i = 0; i <= Npath; i++) { // straight path
		Path.Add(P0 + vectS * i);
		//Value.Add(SumL); // starts from 0
		//SumL += StepL;
	}

}

void CTarget:: SetPathVal(int ival) // set Value^: 0 - len, 1 - NSigmaL, 2 - decay, 3 - rate
{
	double Pow0 = 1;// start from full
	C3Point Pos = Path[0];
	double PreVal = 0;
	switch (ival) {
	case 0: // reserved or path Length
		for (int i = 0; i <= Npath; i++)
			Value[i] = i * StepL;
		break;
	case 1: // SumThickness 
		double Dens, Sigma, dth;
		Value[0] = 0;
		for (int i = 1; i <= Npath; i++)
		{
			Pos = Path[i];
			Dens = GetDens(Pos);
			Sigma = GetSigma(Pos);
			dth = Dens * Sigma * StepL;
			Value[i] = Value[i-1] + dth;//SumThickness += dth;
		}
		break;
	case 2: // decay on SumThickness
		double SumThick, decay;
		Value[0] = Pow0; 
		for (int i = 1; i <= Npath; i++)
		{
			SumThick = Value[i];
			decay = Pow0 * exp(-SumThick);// Npower
			Value[i] = decay;//SumThickness += dth;
		}
		break;
	case 3: //ions generation rate == neutral flux decay along stepL
		double Rate;
		PreVal = Value[0]; // keep decay
		Value[0] = 0;
		for (int i = 1; i <= Npath; i++)
		{
			Rate = Value[i] - PreVal; // decay[i-1] 
			PreVal = Value[i];
			Value[i] = Rate;//IonPower = Npower * dth;
		}
		break;
	default: break;
	}
}

void CTarget::WritePathSingle(BEAM_RAY ray, CPlasma * plasma) // trace and write Value (param) along ray path
// called by calculateThinDecay
{
	CString name = "BTRthinInTarg.txt";//Nrays = 1;
	FILE * fout;
	fout = fopen(name, "w");
	fprintf(fout, "BTR >>> Beam AMU = %d  Energy = %5.0f eV, V = %g ", ray.A, ray.EeV, ray.Vmod);
	fprintf(fout, "\t Target AMU = 2\n");

	C3Point P0, P1, Pmid, Pathmid;
	double X0, Y0, Z0, X1, Y1, Z1;
	Pathmid = (Path[0] + Path[Npath]) * 0.5;
	X0 = Path[0].X;  Y0 = Path[0].Y; Z0 = Path[0].Z;
	X1 = Path[Npath].X; Y1 = Path[Npath].Y; Z1 = Path[Npath].Z;
	fprintf(fout, " Parameters along beam path [m] (NBI ref) entry(%4.3f,%4.3f,%4.3f) exit = (%4.3f,%4.3f,%4.3f) \n",
					X0, Y0, Z0, X1, Y1, Z1);

	P0 = plasma->GetTorRZ(X0, Y0, Z0); P1 = plasma->GetTorRZ(X1, Y1, Z1);// R R Z
	Pmid = plasma->GetTorRZ(Pathmid.X, Pathmid.Y, Pathmid.Z);
	fprintf(fout, " Path in Tor ref [m]: entry(%4.3f,%4.3f) mid(%4.3f,%4.3f) exit = (%4.3f,%4.3f) \n",
					P0.X, P0.Z, Pmid.X, Pmid.Z, P1.X, P1.Z);

	//fprintf(fout, "  Lambda = 1/(Ne*sigmaTot) Rates and Sigmas are taken from PREACT module\n");
	//fprintf(fout, "  X,m \tR,m \tZ,m  \t psi\t Vpsi\t Te,keV\t Ne,m-3 \t CX-SigmaV \t II-SigmaV \t IE-SigmaV \t Lambda \n");
	fprintf(fout, "\tX,m    \tY,m  \tZ,m   \tpsi \tDens,m-3 \tSigma \tSumThick \tDecay \tRate   \tRateCX \tRateII \tRateEI \n");
	
	C3Point P, Prz, Sigma3;
	//double Vsurf; // PSI-volume (within StepPSI)
	double Ne;// Te;
	double Sigma;// , Sigma0 = plasma->GetSigmaPSI(-1);// 1.e-20;
	double dth, SumThick;
	double Psi;
	double Vbeam = ray.Vmod;
	double Npow = ray.pow;
	double Rate;
	
	//double Vpsi1 = plasma->GetPlasmaVolPSI(0.999); // normalize Vol
	//if (Vpsi1 < 1.e-3) Vpsi1 = 1;
	if (StepL < 1.e-12) {
		AfxMessageBox("StepL is set 1cm");
		StepL = 0.01;
	}

	SetPathFrom(ray.Pos, ray.Vn, StepL);
	CArray<double> psi;
	CArray<double> sigma;
	CArray<double> dens;
	CArray<double> sumthick;
	CArray<double> decay;
	CArray<double> rate;
	CArray<double> rateCX;
	CArray<double> rateII;
	CArray<double> rateEI;
	Value.RemoveAll();
	
	SumThick = 0;
	for (int i = 0; i <= Npath; i++) { // redefine SetPathVal(1) - SumDens Dens*Sigma*dL
		P = Path[i];
		Prz = plasma->GetTorRZ(P.X, P.Y, P.Z);// R R Z
		Psi = plasma->GetPSI(Prz.X, Prz.Z);// flux 0..1
		psi.Add(Psi);
		//Vsurf = plasma->GetPlasmaVolPSI(psi) / Vpsi1;
		Ne = plasma->Nemax * plasma->GetPlasmaNePSI(Psi);//Nemax * GetPlasmaDensityParabolic(r, 2);
		dens.Add(Ne);
		//Te = plasma->Temax * plasma->GetPlasmaTePSI(psi);//Temax * GetPlasmaTeParabolic(r, 2);

		Sigma = plasma->GetSigmaPSI(Psi);// / Vbeam; // profiles are set for SigmaV!!!
		sigma.Add(Sigma);
		SumThick += Ne * Sigma * StepL;
		sumthick.Add(SumThick);
		Value.Add(SumThick);//Value[i] = Value[i - 1] + Ne * Sigma * StepL;
	}
	SetPathVal(2);// Value << decay
	for (int i = 0; i <= Npath; i++) decay.Add(Value[i]); // = exp(-SumThickness); // total neutrals decay along the path
	SetPathVal(3); // Value << rate
	for (int i = 0; i <= Npath; i++) rate.Add(-Value[i]); //= Npower * dth;//ions generation rate  = neutral flux decay along stepL
/*
	// calculate partial rates
	//Npow = 1;
	for (int i = 0; i <= Npath; i++) { // calculate CX rate
		Psi = psi[i];
		Ne = dens[i];
		Sigma3 = plasma->GetSigmaPSIpart(Psi, 1) / Vbeam;
		dth = Ne * Sigma3.X * StepL;
		Npow = decay[i];
		Rate = Npow * dth;
		rateCX.Add(Rate);
		//Npow = Npow - Rate;
	}
	//Npow = 1;
	for (int i = 0; i <= Npath; i++) { // calculate II rate
		Psi = psi[i];
		Ne = dens[i];
		Sigma3 = plasma->GetSigmaPSIpart(Psi, 1) / Vbeam;
		dth = Ne * Sigma3.Y * StepL;
		Npow = decay[i];
		Rate = Npow * dth;
		rateII.Add(Rate);
		//Npow = Npow - Rate;
	}
	//Npow = 1;
	for (int i = 0; i <= Npath; i++) { // calculate EI rate
		Psi = psi[i];
		Ne = dens[i];
		Sigma3 = plasma->GetSigmaPSIpart(Psi, 1) / Vbeam;
		dth = Ne * Sigma3.Z * StepL;
		Npow = decay[i];
		Rate = Npow * dth;
		rateEI.Add(Rate);
		//Npow = Npow - Rate;
	}
*/		
		
	for (int i = 0; i < Npath; i++) { // write output
		C3Point Ploc = GetLocal3D(Path[i]);
		int locstate = WithinTargetLoc(Ploc);
		//if (locstate != 1) continue;
	
		if (psi[i] > 1) continue;
		fprintf(fout, "%d   %-4.3f   %-4.3f   %-4.3f     %-4.3f    %-10.3le  %-10.3le  %-10.3le   %-10.3le  %-10.3le  %d\n",
					  i,  Path[i].X, Path[i].Y, Path[i].Z, psi[i], dens[i], sigma[i], sumthick[i], decay[i], rate[i], locstate);
		//rate[i], rateCX[i], rateII[i], rateEI[i]);
		//	fprintf(fout, "  %-4.3f %-4.3f %-4.3f %-4.3f  %-10.3le  %-10.3le  %-10.3le  %-10.3le  %-10.3le  %-10.3le  %-10.3le  %-10.3le\n", 
		//	      Path[i].X, Path[i].Y, Path[i].Z, psi[i], dens[i], sigma[i], sumthick[i], decay[i],
		//rate[i], rateCX[i], rateII[i], rateEI[i]);
	}
	fclose(fout);
	CString S;
	S.Format("Npath = %d\n Single Ray is written to %s", Npath, name);
	AfxMessageBox(S);
}

void CTarget::WritePathSet(BeamArray1D & rays, CPlasma * plasma) // called by CalculateThickParallel
{
	int Nrays = rays.GetSize();
	if (Nrays < 1) return;
	BEAM_RAY ray = rays[0];// BEAM_RAY(IonBeamEnergy * 1.e6, TracePartNucl, P0, Vn);
	//pPlasma->SetPathFrom(P0, Vn, 0.1);
	
	vector<double> decay;
	vector<double> rate;
	vector<vector<double>> decays;
	vector<vector<double>> rates;
	int Npath, Npath0;
	double Psi, Ne, SumThick = 0;
	double Vbeam = ray.Vmod;
	StepL = 0.1;
	
	CString name = "BTRMultiInTarg.txt";//Nrays = 1;
	FILE * fout;
	fout = fopen(name, "w");
	C3Point P, P0, P1, Prz, Vn;
	fprintf(fout, "BTR >>> Set of rays \n Beam AMU = %d,  Energy = %g eV, V = %g m/s", ray.A, ray.EeV, ray.Vmod);
	fprintf(fout, "\t Target AMU = 2\tRates,Sigmas << PREACT\n");
	for (int i = 0; i < Nrays; i++) {
		rate.resize(0);
		decay.resize(0);
		ray = rays[i];
		P0 = ray.Pos;
		Vn = ray.Vn;
		plasma->SetPathFrom(P0, Vn, StepL);
		Npath = Path.GetSize() - 1;
		P0 = Path[0]; P1 = Path[Npath];
		fprintf(fout, " ray #%2d path[m] in NBI ref: entry(%4.3f,%+4.3f,%+4.3f) exit = (%4.3f,%+4.3f,%+4.3f) \n",
			i, P0.X, P0.Y, P0.Z, P1.X, P1.Y, P1.Z);
		if (i == 0) Npath0 = Npath;
		/*for (int k = 0; k < Npath; k++) {
			P = Path[k];
			rate.push_back(P1.Y);
			decay.push_back(P1.Z);
		}*/
		SumThick = 0;
		for (int i = 0; i <= Npath; i++) { // redefine SetPathVal(1) - SumDens Dens*Sigma*dL
			P = Path[i];
			Prz = plasma->GetTorRZ(P.X, P.Y, P.Z);// R R Z
			Psi = plasma->GetPSI(Prz.X, Prz.Z);// flux 0..1
			//psi.Add(Psi);
			//Vsurf = plasma->GetPlasmaVolPSI(psi) / Vpsi1;
			Ne = plasma->Nemax * plasma->GetPlasmaNePSI(Psi);//Nemax * GetPlasmaDensityParabolic(r, 2);
			//dens.Add(Ne);
			//Te = plasma->Temax * plasma->GetPlasmaTePSI(psi);//Temax * GetPlasmaTeParabolic(r, 2);

			Sigma = plasma->GetSigmaPSI(Psi);// / Vbeam; // profiles are set for SigmaV!!!
			//sigma.Add(Sigma);
			SumThick += Ne * Sigma * StepL;
			//sumthick.Add(SumThick);
			Value[i] = SumThick;//Value[i] = Value[i - 1] + Ne * Sigma * StepL;
		}
		SetPathVal(2);// Value << decay
		for (int i = 0; i <= Npath; i++) decay.push_back(Value[i]); // = exp(-SumThickness); // total neutrals decay along the path
		SetPathVal(3); // Value << rate
		for (int i = 0; i <= Npath; i++) rate.push_back(-Value[i]); //= Npower * dth;//ions generation rate  = neutral flux decay along stepL

		decays.push_back(decay);
		rates.push_back(rate);
	}
	fprintf(fout, " Decay(atoms) / Rate(ions) along ray path\n");
	fprintf(fout, " X, m");
	for (int i = 0; i < Nrays; i++) fprintf(fout, "   \t#%2d", i);
	fprintf(fout, "\n");
	for (int k = 0; k < Npath0; k++) {
		fprintf(fout, "%5.3f", Path[k].X);
		for (int i = 0; i < Nrays; i++) fprintf(fout, "   %6.3f %6.3f", decays[i][k], rates[i][k]);
		fprintf(fout, "\n");
	}
	fclose(fout);

	AfxMessageBox("Multi-rays Path is written to " + name);
}

void CTarget::SetCutPlane(C3Point orig, C3Point destx, C3Point desty, double stepx, double stepy)
{ // find Nx, Ny
	C3Point dX = destx - orig;
	C3Point dY = desty - orig;
	CutNx = (int)ceil(ModVect(dX) / stepx);
	CutNy = (int)ceil(ModVect(dY) / stepy);

	CutOrig = orig;
	CutOrtX = dX/ ModVect(dX);
	CutOrtY = dY/ ModVect(dY);
	//vectZ = VectProd(vectX, vectY);
	CutStepX = stepx;
	CutStepY = stepy;
	CutDest = CutOrig + CutOrtX * CutStepX * CutNx + CutOrtY * CutStepY * CutNy;

/*	CutCS = new double *[CutNx + 1];
	for (int i = 0; i <= CutNx; i++) {
		CutCS[i] = new double[CutNy + 1];
		for (int j = 0; j <= CutNy; j++) CutCS[i][j] = 0;
	}
	*/
}

C3Point CTarget::GetLocal3D(C3Point P) // get local CS coord of the global P
{
	C3Point Plocal;
	double x0, y0; // coordinates in ortogonal (old) system
	C3Point Pvect = VectSum(P, Orig, 1, -1); // substract vectors in global CS
	Plocal.X = Pvect.X;// ScalProd(Pvect, OrtX);// /  mod_vectZ;
	Plocal.Y = Pvect.Y;// ScalProd(Pvect, OrtY);// /  mod_vectZ;
	Plocal.Z = Pvect.Z;// ScalProd(Pvect, OrtZ);// /  mod_vectZ;
	return Plocal;
}

C3Point CTarget::GetCutLocal(C3Point P)
{
	C3Point Plocal;
	C3Point Pvect = VectSum(P, CutOrig, 1, -1); // substract vectors in global CS
	Plocal.X = ScalProd(Pvect, CutOrtX);// /  mod_vectZ;
	Plocal.Y = ScalProd(Pvect, CutOrtY);// /  mod_vectZ;
	//Plocal.Z = Pvect - CutOrtX * Plocal.X - CutOrtY * Plocal.Y;
	return Plocal;

}

int CTarget::WithinTargetLoc(C3Point Ploc) // -1 out, 1 in, 10 - xlim, 20 - ylim, 30 - zlim
{
	int in = -1;// out of volume
	double LimX = Step.X * Nx;
	double LimY = Step.Y * Ny;
	double LimZ = Step.Z * Nz;
	double dx = 0.1 * Step.X;
	double dy = 0.1 * Step.Y;
	double dz = 0.1 * Step.Z;
	if (Ploc.X < -dx || Ploc.X > LimX+dx || Ploc.Y < -dy || Ploc.Y > LimY+dy || Ploc.Z < -dz || Ploc.Z > LimZ+dz) 
		return -1; // out
	if (Ploc.X > dx && Ploc.X < LimX-dx && Ploc.Y > dy && Ploc.Y < LimY-dy && Ploc.Z > dz && Ploc.Z < LimZ-dz) 
		return 1; // in
	
	if (fabs(Ploc.X) <= dx || fabs(Ploc.X - LimX) <= dx) 
		return 10; // xlimit
	if (fabs(Ploc.Y) <= dy || fabs(Ploc.Y - LimY) <= dy) 
		return 20; // ylimit
	if (fabs(Ploc.Z) <= dz || fabs(Ploc.Z - LimZ) <= dz) 
		return 30; // zlimit
	return 0; //unknown
}

int CTarget::Distr2Dcell(C3Point Pgl, double val)// - 3 projections: XY(hor)/XZ(vert)/YZ  
// return state: 1 - ok, 0 - out/bound
{
	C3Point Ploc = GetLocal3D(Pgl);// in a volume
	int locstate = WithinTargetLoc(Ploc);// 1 - inside, 10 - limX, 20 - limY, 30 - limZ
	if (locstate != 1) return 0; // only inner area -> out and bounds excluded!
	
	int i = (int)floor(Ploc.X / Step.X);
	int j = (int)floor(Ploc.Y / Step.Y);
	int k = (int)floor(Ploc.Z / Step.Z);

/*	C3Point PlocCut = GetCutLocal(Pgl);
	int icut = (int)floor(PlocCut.X / CutStepX);
	int jcut = (int)floor(PlocCut.Y / CutStepY);
	if (icut < 0) icut = 0;
	if (jcut < 0) jcut = 0;
	if (icut > CutNx) icut = CutNx;
	if (jcut > CutNy) jcut = CutNy;
*/
	double cellXY = Step.X * Step.Y;
	double cellXZ = Step.X * Step.Z;
	double cellYZ = Step.Y * Step.Z;
	//double Cutcell = CutStepX * CutStepY;

	double x0 = Ploc.X - i * Step.X;//a
	double x1 = Step.X - x0;//b
	double y0 = Ploc.Y - j * Step.Y;//c
	double y1 = Step.Y - y0;//d
	double z0 = Ploc.Z - k * Step.Z;
	double z1 = Step.Z - z0;

	double d00, d01, d10, d11, SQcell;

	//horizontal plane
	SQcell = cellXY * cellXY;
	d00 = x1 * y1; // add to i, j
	d10 = x0 * y1; // add to i+1, j
	d01 = x1 * y0; // add to i, j+1
	d11 = x0 * y0; // add to i+1, j+1
	CutHor[i][j] += val * d00 / SQcell; // cellXY / cellXY;
	CutHor[i + 1][j] += val * d10 / SQcell;// cellXY / cellXY;
	CutHor[i][j + 1] += val * d01 / SQcell;// cellXY / cellXY;
	CutHor[i + 1][j + 1] += val * d11 / SQcell;// cellXY / cellXY;

	// vertical plane
	SQcell = cellXZ * cellXZ;
	d00 = x1 * z1; // add to i, k
	d10 = x0 * z1; // add to i+1, k
	d01 = x1 * z0; // add to i, k+1
	d11 = x0 * z0; // add to i+1, k+1
	CutVert[i][k] += val*d00 / SQcell;// cellXZ / cellXZ;
	CutVert[i + 1][k] += val*d10 / SQcell;// cellXZ / cellXZ;
	CutVert[i][k + 1] += val*d01 / SQcell;// cellXZ / cellXZ;
	CutVert[i + 1][k + 1] += val*d11 / SQcell;// cellXZ / cellXZ;

	// CS plane YZ
	SQcell = cellYZ * cellYZ;
	d00 = y1 * z1; // add to j, k
	d10 = y0 * z1; // add to j+1, k
	d01 = y1 * z0; // add to j, k+1
	d11 = y0 * z0; // add to j+1, k+1
	CutCS[j][k] += val*d00 / SQcell;//cellYZ / cellYZ;
	CutCS[j + 1][k] += val*d10 / SQcell;// cellYZ / cellYZ;
	CutCS[j][k + 1] += val*d01 / SQcell;//cellYZ / cellYZ;
	CutCS[j + 1][k + 1] += val*d11 / SQcell;// cellYZ / cellYZ;

	//diagonal cut
/*	double xc0 = PlocCut.X - icut * CutStepX;//a
	double xc1 = CutStepX - xc0;//b
	double yc0 = PlocCut.Y - jcut * CutStepY;//c
	double yc1 = CutStepY - yc0;//d
	double d00 = xc1 * yc1; // add to i, j
	double d10 = xc0 * yc1; // add to i+1, j
	double d01 = xc1 * yc0; // add to i, j+1
	double d11 = xc0 * yc0; // add to i+1, j+1
	CutDiag[icut][jcut] += val * d00 / Cutcell / Cutcell;
	CutHor[icut + 1][jcut] += val * d10 / Cutcell / Cutcell;
	CutHor[icut][jcut + 1] += val * d01 / Cutcell / Cutcell;
	CutHor[icut + 1][jcut + 1] += val * d11 / Cutcell / Cutcell;*/

	return 1; // distributed

	
}
/////// PLasma ///////////////////////////////////

CPlasma::CPlasma() : CTarget()
{
	Nrays = -1;//thin beam 
	CalcOpt = 0; // along path
	Npsi = 50; // flux/rad steps
	StepFlux = 1. / Npsi;
	SigMult = 1.;
	
	StepR = StepFlux;
	
	PSIloaded = FALSE;
	ProfilesLoaded = FALSE;
	SigmaLoaded = FALSE;
	DeltaLoaded = FALSE;

	Nprof = 0;
	psiNr = 0;
	psiNz = 0;

	for (int i = 0; i < Npsi; i++) {
		double r = (i + 0.5) * StepFlux;
		double vol = 0.3  + r*r;// GetPlasmaVolPSI(i * StepFlux); // psi*psi
		fVolume.Add(vol);
	}
}

void CPlasma::ClearArrays() //calculated decays
{
	CTarget::ClearArrays(); // Path, Value
	Power.RemoveAll();
	NL.RemoveAll();
	IonRate.RemoveAll();
	AverPath.RemoveAll();
	SumNL.RemoveAll();
	SumIonRate.RemoveAll();
	SumPower.RemoveAll();
	SumPart.RemoveAll();
	
	fPower.RemoveAll();
	fNL.RemoveAll();
	fIonRate.RemoveAll();
	fSumPart.RemoveAll();
	//SetZeroCuts();

	Nrays = -1;
}
CPlasma::~CPlasma()
{
	ClearArrays();
	ClearCuts();
	ClearPSI();
	ClearProfiles();
	ClearSigma();
}

/*void CPlasma::SetOriginDim(C3Point start, int nx, int ny, int nz)
{
	
	CTarget::SetOriginDim(start, nx, ny, nz);
	CString S;
	S.Format("CPlasma\n Start %g, %g, %g\n Nx = %d", Orig.X, Orig.Y, Orig.Z, Nx);
	AfxMessageBox(S);
}*/

void CPlasma::SetPathPower() // sample array
{
	SumPower.RemoveAll();
	for (int i = 0; i <= Npath; i++) {
		double sval = i*i * StepL;// imitate
		SumPower.Add(sval);
	}
}

void CPlasma:: SetPlasmaParam(C3Point TorC, double a, double R, double Ell,  double Ne, double Te, double sigma)
//(CBTRDoc * pDoc, double sigma)
{
	Sigma = sigma;//used if array is not loaded = SigmaBase*Delta default 
	Nemax = Ne; //pDoc->MaxPlasmaDensity;
	Temax = Te; //pDoc->MaxPlasmaTe;
	Rmin = R - a;
	Rmax = R + a;
	PlasmaMinR = a; //pDoc->PlasmaMinorR;
	PlasmaMajR = R; // pDoc->PlasmaMajorR;
	PlasmaEll = Ell;
	Zmin = TorC.Z - a * Ell;
	Zmax = TorC.Z + a * Ell;
	TorCentre = TorC;
	Nrays = -1;
	//Nx = 100;
	ClearArrays();

}
void CPlasma::SetBeamRay(BEAM_RAY ray)
{
	Ray = ray;
}

void CPlasma::WritePathThin() // plasma parameters along path - without SIGMA !
// older - called by CalculateThinDecay
{
	CString name = "PathThin.txt";//Nrays = 1;
	FILE * fout;
	fout = fopen(name, "w");
	/*  Beam AMU(CX, II, EI) = 2  Energy = 500 keV (TIN) V = sqrt(2*e*EeV/Mass) ~ sqrt(2*EeV/amu) 	 
		Target AMU(CX, II) = 2
		Parameters along beam path[m](NBI ref) entry(19.984, 0.000, 0.000) exit = (25.016, 0.000, 0.000)
		Path in Tor ref[m]: entry(4.230, -0.500) mid(3.400, -0.500) exit = (4.230, -0.500)
		Rates and Sigmas are taken from PREACT module
		X, m 	 R, m 	 Z, m 	 psi	 Vpsi	 Te, keV	 Ne, m - 3 	 CX - SigmaV 	 II - SigmaV 	 IE - SigmaV
		*/
	C3Point P0, P1, Pmid, Pathmid;
	double X0, Y0, Z0, X1, Y1, Z1;
	Pathmid = (Path[0] + Path[Npath]) * 0.5;
	X0 = Path[0].X;  Y0 = Path[0].Y; Z0 = Path[0].Z;
	X1 = Path[Npath].X; Y1 = Path[Npath].Y; Z1 = Path[Npath].Z;
	P0 = GetTorRZ(X0, Y0, Z0); P1 = GetTorRZ(X1, Y1, Z1);// R R Z
	Pmid = GetTorRZ(Pathmid.X,Pathmid.Y,Pathmid.Z);
	fprintf(fout, " Parameters along beam path [m] (NBI ref) entry(%4.3f,%4.3f,%4.3f) exit = (%4.3f,%4.3f,%4.3f) \n",
																X0, Y0, Z0, X1, Y1, Z1);
	fprintf(fout, " Path in Tor ref [m]: entry(%4.3f,%4.3f) mid(%4.3f,%4.3f) exit = (%4.3f,%4.3f) \n", 
									P0.X, P0.Z,Pmid.X, Pmid.Z, P1.X, P1.Z);
									
	fprintf(fout, "  Lambda = 1/(Ne*sigmaTot) Rates and Sigmas are taken from PREACT module\n");
	fprintf(fout, "  X,m \tR,m \tZ,m  \t psi\t Vpsi\t Te,keV\t Ne,m-3 \t CX-SigmaV \t II-SigmaV \t IE-SigmaV \t Lambda \n");
	
	/*double decay = 1;
	double Npower = 1;
	double IonPower = 0;
	double dth, SumThickness = 0;*/
	//double r;//poloidal, rnorm = 1;
	C3Point P, Prz;
	double Vsurf; // PSI-volume (within StepPSI)
	double Ne, Te;
	//double Sigma0 = GetSigmaPSI(-1);// 1.e-20;
	//Sigma = plasma->GetSigmaPSI(Psi) / Vbeam; // profiles are set for SigmaV!!!
	double psi;
	//RayDecay = 0;
	double Vpsi1 = GetPlasmaVolPSI(0.999); // normalize Vol
	if (Vpsi1 < 1.e-3) Vpsi1 = 1;

	for (int i = 0; i <= Npath; i++) {
		P = Path[i];
		//r = GetR(P.X, P.Y, P.Z);// poloidal radius of plasma (double)
		//int ir = (int)ceil(r / PlasmaMinR / StepFlux); // index of flux layer 
		Prz = GetTorRZ(P.X, P.Y, P.Z);// R R Z
		psi = GetPSI(Prz.X, Prz.Z);// flux 0..1
		if (psi < 0 || psi >= 1) continue;
		Vsurf = GetPlasmaVolPSI(psi)/Vpsi1;
		Ne = Nemax * GetPlasmaNePSI(psi);//Nemax * GetPlasmaDensityParabolic(r, 2);
		Te = Temax * GetPlasmaTePSI(psi);//Temax * GetPlasmaTeParabolic(r, 2);
		fprintf(fout, "  %-4.3f %-4.3f %-4.3f  %-4.3f  %-4.3f   %-10.3le %-10.3le \n", P.X, Prz.X, Prz.Z, psi, Vsurf, Te, Ne);
		//fprintf(fout, " \t %-7.3f \t %-12.3le \n"...
		/*
		dth = Density * Sigma0 * StepL;
		SumThickness += dth;
		decay = exp(-SumThickness); // total neutrals decay along the path
		IonPower = Npower * dth;//ions generation rate  = neutral flux decay along stepL
		Npower = decay;// = Npower-IonPower= Npower0 * decay - result neutral flux after stepL
		Power.Add(Npower);
		NL.Add(psi);// (Density);
		IonRate.Add(IonPower);*/
	}
	fclose(fout);

	AfxMessageBox("Path is written in " + name);
}

CArray<double, double> * CPlasma::GetPowerDecayArray(C3Point P0, C3Point P1, double dL) 
// return power arr along path P0,P1 with step dL
// called by doc->SetStopArrays along beam axis
{
	double decay = 1;
	
	CArray<double, double> * Parr = new CArray<double, double>;
	double PathLen = ModVect(P1 - P0);
	int N = (int)ceil(PathLen / dL);
	C3Point PathVect = (P1 - P0) / PathLen;
	for (int i = 0; i < N; i++)  Parr->Add(1); // initial power values
	
	C3Point Pgl = P1;
	C3Point Ploc = GetLocal3D(Pgl);
	if (Ploc.X < 0) return Parr;//decay;

	double Npower0 = 1;
	//double IonPower = 0;
	double dth, SumThickness = 0;
	//double r;//, rnorm = 1, Rtor, Ztor;
	C3Point Prz;
	//double Vsurf; // PSI-volume (within StepPSI)
	double Density, Te, sigma;
	//double Sigma0 = GetSigmaPSI(-1);// 1.e-20;
	//Sigma = plasma->GetSigmaPSI(Psi) / Vbeam; // profiles are set for SigmaV!!!
							   //RayDecay = 0;
	for (int i = 0; i < N; i++) {
		Pgl = P0 + PathVect * (i * dL);
		Ploc = GetLocal3D(Pgl);
		if (WithinTargetLoc(Ploc) < 0) {
			Parr->SetAt(i, decay);
			continue; // out of target limits
		}
		Prz = GetTorRZ(Pgl.X, Pgl.Y, Pgl.Z);// R R Z
		double psi = GetPSI(Prz.X, Prz.Z);// flux 0..1
		if (psi < 0 || psi > 1) {
			Parr->SetAt(i, decay);
			continue; // out of psi data
		}
		
		Density = Nemax * GetPlasmaNePSI(psi);//Nemax * GetPlasmaDensityParabolic(r, 2);
		Te = Temax * GetPlasmaTePSI(psi);//Temax * GetPlasmaTeParabolic(r, 2);
		sigma = GetSigmaPSI(psi);
		// Sigma = GetSigmaPSI(psi) / Ray.Vmod; //<- if profiles are set for SigmaV!!!
		dth = Density * sigma * dL;
		SumThickness += dth;
		decay = exp(-SumThickness); // total neutrals decay along the path
									//IonPower = Npower * dth;//ions generation rate  = neutral flux decay along stepL
									//Npower = decay;// = Npower-IonPower= Npower0 * decay - result neutral flux after stepL
		Parr->SetAt(i, decay);
	} // N

	  //double Npower1 = decay;
	  //RayDecay = (1 - Npower1 / Npower0);// Npower0 = 1

	return Parr;// Npower final
}

double CPlasma::GetPowerDecay(C3Point P0, C3Point P1, double dL) 
// get result decay between 2 points
// used for tracers - no arrays / no path
// called by doc->GetDecay 
{
	double decay = 1;
	C3Point Pgl = P1;
	C3Point Ploc = GetLocal3D(Pgl);
	//if (WithinTargetLoc(Ploc) <= 0) return decay; // 1
	if (Ploc.X < 0) return decay; // end point before Xmin

	double Npower0 = 1;
	double dth, SumThickness = 0;
	C3Point Prz;
	//double Vsurf; // PSI-volume (within StepPSI)
	double Density, Te;
	double PathLen = ModVect(P1 - P0);
	int N = (int)ceil(PathLen / dL);
	C3Point PathVect = (P1 - P0) / PathLen;
	for (int i = 0; i < N; i++) {
		Pgl = P0 + PathVect * (i * dL);
		Ploc = GetLocal3D(Pgl);
		if (WithinTargetLoc(Ploc) <= 0) continue; // out of target limits

		Prz = GetTorRZ(Pgl.X, Pgl.Y, Pgl.Z);// R R Z
		double psi = GetPSI(Prz.X, Prz.Z);// flux 0..1
		if (psi < 0 || psi > 1) continue; // out of psi data
		
		Density = Nemax * GetPlasmaNePSI(psi);//Nemax * GetPlasmaDensityParabolic(r, 2);
		Te = Temax * GetPlasmaTePSI(psi);//Temax * GetPlasmaTeParabolic(r, 2);
		Sigma = GetSigmaPSI(psi);// / Ray.Vmod; // profiles are set for SigmaV!!!
		dth = Density * Sigma * dL;
		SumThickness += dth;
		decay = exp(-SumThickness); // total neutrals decay along the path
		//IonPower = Npower * dth;//ions generation rate  = neutral flux decay along stepL
		//Npower = decay;// = Npower-IonPower= Npower0 * decay - result neutral flux after stepL
				
	} // N

	//double Npower1 = decay;
	//RayDecay = (1 - Npower1 / Npower0);// Npower0 = 1
	
	return decay;// Npower final
}

void CPlasma::SetDecayArrays() //thin ray decay - typically called after SetPath
			//SetPath(C3Point P0, C3Point P1, double step)// set Path array
{
	Power.RemoveAll();
	NL.RemoveAll();
	IonRate.RemoveAll();
	CString name = "SetDecayArrays.txt";//Nrays = 1;
	//FILE * fout;
	//fout = fopen(name, "w");
	//fprintf(fout, "\tX,m    \tY,m  \tZ,m   \tpsi \tDens,m-3 \tSigma \tSumThick \tPower \tIonRate   \tRateCX \tRateII \tRateEI \n");

	//Nrays = 1;
	double decay = 1;
	double Npower0, Npower = 1;
	double IonPower = 0;// Npower  - Npower0
	double dth, SumThickness = 0;
	//int ipsi;// ipath;
	double r;//, rnorm = 1, Rtor, Ztor;
	C3Point P, Prz;
	//double Vsurf; // PSI-volume (within StepPSI)
	double psi, sigma, Density, Te;
	//double Sigma0 = GetSigmaPSI(-1);// 1.e-20;
	//Sigma = plasma->GetSigmaPSI(Psi) / Vbeam; // profiles are set for SigmaV!!!
	RayDecay = 0;

	for (int i = 0; i < Npath; i++) {
		P = Path[i];
		
		//P = (Path[i] + Path[i + 1]) * 0.5;
		//r = GetR(P);// poloidal radius of plasma (double)
		//int ir = (int)ceil(r / PlasmaMinR / StepFlux); // index of flux layer 
		Prz = GetTorRZ(P.X, P.Y, P.Z);// R R Z ref TorCentre
		psi = GetPSI(Prz.X, Prz.Z); // flux 0..1 - Ellipticity is accounted // R must be in Tor ref!!!
		
		if (psi < 0 || psi > 1) {
			Power.Add(Npower);
			NL.Add(0);// (Density);
			IonRate.Add(IonPower);
			//fprintf(fout, "%d   %-4.3f   %-4.3f   %-4.3f     %-4.3f    %-10.3le  %-10.3le \n",
				//          i, Path[i].X, Path[i].Y, Path[i].Z, psi,  Npower, IonPower);
			continue;
		}
					
		Density = Nemax * GetPlasmaNePSI(psi);//Nemax * GetPlasmaDensityParabolic(r, 2);
		Te = Temax * GetPlasmaTePSI(psi);//Temax * GetPlasmaTeParabolic(r, 2);
		sigma = GetSigmaPSI(psi);// / Ray.Vmod; // profiles are set for SigmaV!!!			
		dth = Density * sigma * StepL;
		SumThickness += dth; 
		decay = exp(-SumThickness); // total neutrals decay along the path
		//IonPower = Npower * dth;//generate ions before Npower decay
		Npower0 = Npower;
		Npower = decay;// = Npower-IonPower= Npower0 * decay - result neutral flux after stepL
		//IonPower = Npower * dth;//generate ions after Npower decay
		IonPower = Npower0 - Npower;

		Power.Add(Npower);
		NL.Add(psi);// (Density);
		IonRate.Add(IonPower);

		//fprintf(fout, "%d   %-4.3f   %-4.3f   %-4.3f     %-4.3f    %-10.3le  %-10.3le  %-10.3le   %-10.3le  %-10.3le \n",
			//		i, Path[i].X, Path[i].Y, Path[i].Z, psi, Density, sigma, SumThickness, Npower, IonPower);

	}
	//fclose(fout);

	Npower0 = Power[0];
	//double Npower1 = Npower;
	if (Npower0 > 1.e-12)	RayDecay = (1 - Npower / Npower0);
	else RayDecay = 0;
}

void  CPlasma::AddDecayArrays()// add Decay arrays (single ray) to Sum-arrays and convert path coord to flux CS
{
	if (Npath < 1) {//	axis path empty
		AfxMessageBox("Path not exists");
		return;
	}
	
	CString S;
	S.Format("CPlasma AddDecay\n Start %g, %g, %g\n Nx = %d\n Nrays = %d", Orig.X, Orig.Y, Orig.Z, Nx, Nrays);
	//AfxMessageBox(S);

	if (Nrays < 0) { //empty decay arrays, axis path not empty - SetDecayArray() was not called before
		C3Point vectPath = C3Point(Step.X, 0, 0);
		for (int ii = 0; ii <= Nx; ii++) { // initialize averpath and sumarrays
			AverPath.Add(Orig + vectPath * ii);//(Path[ii]);
			SumPower.Add(0);
			SumNL.Add(0);
			SumIonRate.Add(0);
			SumPart.Add(0);
			
		}
		ConvertArrays(); // path->flux
		SetZeroCuts();
		Nrays++;
		return;
	}
	else { //add to sumarrays
		for (int i = 0; i < Npath; i++) { // Npath - for current ray!! add sumarrays
			C3Point Ploc = GetLocal3D(Path[i]);
			int locstate = WithinTargetLoc(Ploc);
			if (locstate != 1) continue;
			double lx = Ploc.X; // Path[i].X - Orig.X;
			int ii = (int)floor(lx / Step.X);
			double x0 = lx - ii*Step.X;
			double x1 = Step.X - x0;
			if (ii >= 0 && ii < Nx) {
				SumPower[ii] += Power[i] * x1 / Step.X;
				SumPower[ii + 1] += Power[i] * x0 / Step.X;
				
				SumNL[ii] += NL[i] * x1 / Step.X;
				SumNL[ii + 1] += NL[i] * x0 / Step.X;
				
				SumIonRate[ii] += IonRate[i] * x1 / Step.X;
				SumIonRate[ii + 1] += IonRate[i] * x0 / Step.X;

				SumPart[ii]++; 
				SumPart[ii+1]++;
			}
		}
		
	}
	ConvertArrays(); // path->flux
	AddPowerToCutPlanes();
	Nrays++;
	double Npower0 = SumPower[0];
	double Npower1 = SumPower[Nx-1];
	if (Npower0 > 1.e-12)	SumDecay = (1 - Npower1 / Npower0);
	else SumDecay = 0;
}

void CPlasma::ConvertArrays()  // path coord -> flux(rnorm)
// for verification with ASTRA-NUBEAM
{
	//int ipsi = (int)floor(r / PlasmaMinR / StepFlux); // index of magsurf (psi=const)
	if (Npath < 1) {//	axis path empty
		AfxMessageBox("Path not exists");
		return;
	}

	if (Nrays < 0) { //empty decay arrays, axis path not empty - SetDecayArray() was not called before
		fPower.RemoveAll();
		fNL.RemoveAll();
		fIonRate.RemoveAll();
		for (int ir = 0; ir < Npsi; ir++) { // initialize flux-arrays
			fPower.Add(0);
			fNL.Add(0);
			fIonRate.Add(0);
			fSumPart.Add(0);
		}
		//Nrays = 0; // done in AddDecayArrays
		return;
	}

	else { //N >= 0 -> add 
		for (int i = 0; i < Npath; i++) { // Npath - for current ray!! add sumarrays
			//double lx = Path[i].X - Orig.X;
			C3Point P = Path[i];
			//double r = GetR(P.X, P.Y, P.Z);// poloidal radius of plasma (double)
			//int ir = (int)floor(r / PlasmaMinR / StepFlux); // index of flux layer 
			double psi = GetPSI(P);
			if (psi < 0) continue;
			int ir = (int)floor(psi / StepFlux);
			//double Vol = GetPlasmaVolPSI(psi);
			if (ir >= 0 && ir < Npsi) {
				fPower[ir] += Power[i];
				fNL[ir] += NL[i];
				fIonRate[ir] += IonRate[i];
				fSumPart[ir] += 1;
			}
		}
		
		//Nrays++; // done in AddDecayArrays
	}
}

void CPlasma::AddPowerToCutPlanes() 
//distribute Npower along Cut-planes
//power is normalised (Power0 = 1 for each ray)
// not correct for real beam
{
	for (int i = 0; i < Npath; i++) { // Npath - for current ray!! add sumarrays
									  //double lx = Path[i].X - Orig.X;
		C3Point P = Path[i];
		int distr = Distr2Dcell(P, Power[i]); //returns state
/*		int icut = (int)floor((P.X - Orig.X) / Step.X);
		int jcut = (int)floor((P.Y - Orig.Y) / Step.Y);
		int kcut = (int)floor((P.Z - Orig.Z) / Step.Z);
		if (icut < 0 || icut > Nx) continue;
		if (jcut < 0 || jcut > Ny) continue;
		if (kcut < 0 || kcut > Nz) continue;
		//if (icut >= 0 && icut <= Nx && jcut >= 0 && jcut <= Ny && kcut >= 0 && kcut <= Nz)
		CutHor[icut][jcut] += Power[i];
		CutVert[icut][kcut] += Power[i];*/
	}
}
void CPlasma::AddPowerToCutPlanes(double power0) 
{
	for (int i = 0; i < Npath; i++) { // Npath - for current ray!!
		C3Point P = Path[i];
		int distr = Distr2Dcell(P, power0 * Power[i]); //returns state
	}
}
void CPlasma::AddIonRateToCutPlanes(double power0) 
{
	for (int i = 0; i < Npath; i++) { // Npath - for current ray!!
		C3Point P = Path[i];
		int distr = Distr2Dcell(P, power0 * IonRate[i]); //returns state
		
	}
}

double CPlasma:: GetPlasmaDensityParabolic(double psi, int order)
{
	double Dens = 1;
	if (psi > 1 || psi < 0) return 0;
	double w =  0.5;
	if (order < 0) w = 0.5 / abs(order); // negative order ->  gauss profiles 
	double ww = w*w;

	switch (order) {
	//case 0: Dens = 1; break;
	case 1: Dens = 1 - psi;  break;
	case 2: Dens = 1 - psi*psi; break;
	case 3: Dens = 1 - psi*psi*psi; break;
	case 4: Dens = 1 - psi*psi*psi*psi; break;
	case 8: Dens = 1 - psi*psi*psi*psi*psi*psi*psi*psi;	break;
	case 20: Dens = psi*psi; break;
	case 21: Dens = 1 + psi*psi; break;
	
	case 0:  Dens = exp(-psi / w); break;
	case -1: Dens = exp(-psi*psi / w); break; // w = 0.5
	case -2: Dens = exp(-psi*psi / w); break; // w = 0.25
	case -4: Dens = exp(-psi*psi / w); break; // w = 0.125
	case -8: Dens = exp(-psi*psi / w); break;

	//case -20: Dens = exp(-psi*psi); break;
	//case -30: Dens = exp(-psi*psi*psi); break;
	//case -40: Dens = exp(-psi*psi*psi*psi); break;

	default: Dens = 1;
	}
	return Dens;
}

double CPlasma:: GetPlasmaTeParabolic(double psi, int order)
{
	double Te = 1;
	if (psi > 1 || psi < 0) return 0;
	
	switch (order) {
	case 0: Te = 1; break;
	case 1: Te = 1 - psi;  break;
	case 2: Te = 1 - psi*psi; break;
	case 3: Te = 1 - psi*psi*psi; break;
	case 4: Te = 1 - psi*psi*psi*psi; break;
	case 8: Te = 1 - psi*psi*psi*psi*psi*psi*psi*psi;	break;
	default: Te = 1;
	}
	return Te;
}

double CPlasma:: GetR(double X, double Y, double Z) // abs poloidal radius of plasma ref to CS centre
{
	double x = X - TorCentre.X;
	double y = Y - TorCentre.Y;
	double z = Z - TorCentre.Z;
	double R, r;
	R = sqrt(x*x + y*y); // toroidal radius at z=0 
	r = fabs(R - PlasmaMajR); //poloidal radius at z=0
	return sqrt(r*r + z*z);
}

double CPlasma::GetR(C3Point P) // poloidal radius of plasma ref to CS centre
{
	double x = P.X - TorCentre.X;
	double y = P.Y - TorCentre.Y;
	double z = P.Z - TorCentre.Z;
	double R, r;
	R = sqrt(x*x + y*y); // toroidal radius at z=0 
	r = fabs(R - PlasmaMajR); //poloidal radius at z=0
	return sqrt(r*r + z*z);
}

C3Point CPlasma:: GetTorRZ(double X, double Y, double Z) // get R/Z in tokamak frame
{
	C3Point P(0,0,0);
	double x = X - TorCentre.X;
	double y = Y - TorCentre.Y;
	double z = Z - TorCentre.Z;
	double R = sqrt(x*x + y*y); // toroidal radius at z=0 
	P.X = R; P.Y = R; P.Z = z;
	return P;
}

double CPlasma::GetSigmaPSI(double psi) //  (CX + II + EI) * Delta 
{
	double TotCS = Sigma;//base *SigMult;// * Ray.Vmod; // if not loaded -> use default (SigmaBase*Delta) set by SetPlasmaTarget
	return TotCS;

	if (SigmaLoaded) {
		if (fabs(psi) > 1 || psi < 0) return TotCS;
		else {
			TotCS = 0;
			for (int i = 0; i < Nprof - 1; i++) {
				double p0 = PSIprof[i];
				double p1 = PSIprof[i + 1];
				if ((psi - p0) * (psi - p1) > 0) continue;
				else {// <= 0 between or bound  
					double dp0 = psi - p0;
					double p01 = p1 - p0;
					C3Point s0 = SigmArr[i];
					C3Point s1 = SigmArr[i + 1];
					C3Point sigma = s0 + (s1 - s0) * dp0 / p01;
					double d0 = DeltaArr[i];
					double d1 = DeltaArr[i + 1];
					double delta = d0 + (d1 - d0) * dp0 / p01;
					TotCS = (sigma.X + sigma.Y + sigma.Z) * delta * SigMult;
					
					return TotCS;
				}
			}//for
		} // psi in 0...1
	}

	 return TotCS; // if not loaded, 10-20 m
}
C3Point CPlasma::GetSigmaPSIpart(double psi, int enhance) //  CX or II or EI  (* Delta or not) 
{
	double pCS = Sigma * SigMult / 3;// * Ray.Vmod / 3; // if not loaded - equal parts 
	C3Point CS = C3Point(pCS,pCS,pCS); // if not loaded -> use default (SigmaBase*Delta) set by SetPlasmaTarget
	//CString Sreact = react;
	//int CX = 0;
	//int II = 0;
	//int EI = 0;
	
	//if (Sreact.Find("CX", 0) > 0) CX = 1;
	//if (Sreact.Find("II", 0) > 0) II = 1;
	//if (Sreact.Find("EI", 0) > 0) EI = 1;
	if (SigmaLoaded) {
		if (fabs(psi) > 1 || psi < 0) return CS;
		else {
			//TotCS = 0;
			for (int i = 0; i < Nprof - 1; i++) {
				double p0 = PSIprof[i];
				double p1 = PSIprof[i + 1];
				if ((psi - p0) * (psi - p1) > 0) continue;
				else {// <= 0 between or bound  
					double dp0 = psi - p0;
					double p01 = p1 - p0;
					C3Point s0 = SigmArr[i];
					C3Point s1 = SigmArr[i + 1];
					C3Point sigma = s0 + (s1 - s0) * dp0 / p01;
					double d0 = DeltaArr[i];
					double d1 = DeltaArr[i + 1];
					double delta = 1;
					if (enhance != 0) delta =  d0 + (d1 - d0) * dp0 / p01;
										 
					CS = sigma * delta * SigMult; //(sigma.X * CX + sigma.Y * II + sigma.Z * EI) * delta;

					return CS;
				}
			}//for
		} // psi in 0...1
	}

	return CS; // if not loaded, 10-20 m
}

double CPlasma::GetPlasmaTePSI(double psi)
{
	int ord = (int)pDoc->PlasmaTeOrd;
	double Te = 0;
	if (fabs(psi) > 1 || psi < 0) return Te;
	if (!ProfilesLoaded) Te = GetPlasmaTeParabolic(psi, ord); // 1 - psi*psi;//parabolic
	else {
		for (int i = 0; i < Nprof - 1; i++) {
			double p0 = PSIprof[i];
			double p1 = PSIprof[i + 1];
			if ((psi - p0) * (psi - p1) > 0) continue;
			else {
				double dp0 = psi - p0;
				double p01 = p1 - p0;
				double t0 = TeNorm[i];
				double t1 = TeNorm[i+1];
				Te = t0 + (t1 - t0) * dp0 / p01;
				break;
			}
		}
	}
	return Te;
}

double CPlasma::GetPlasmaNePSI(double psi)
{
	double Ne = 0;
	int ord = (int)pDoc->PlasmaDensityOrd;
	if (fabs(psi) > 1 || psi < 0) return Ne;
	if (!ProfilesLoaded)
		//Ne = 1;
		Ne = GetPlasmaDensityParabolic(psi, ord); // 1 - psi*psi;//parabolic
		//Ne = 1 - psi*psi*psi;//cube

	else {
		for (int i = 0; i < Nprof - 1; i++) {
			double p0 = PSIprof[i];
			double p1 = PSIprof[i + 1];
			if ((psi - p0) * (psi - p1) > 0) continue;
			else {
				double dp0 = psi - p0;
				double p01 = p1 - p0;
				double n0 = NeNorm[i];
				double n1 = NeNorm[i + 1];
				Ne = n0 + (n1 - n0) * dp0 / p01;
				//Ne = (NeNorm[i] + NeNorm[i + 1]) * 0.5;
				break;
			}
		}
	}
		
	return Ne;
}

double CPlasma::GetPlasmaVolPSI(double psi) // calculated or parabolic
{
	double Vol = 1;
	if (fabs(psi) > 1 || psi < 0) return Vol;
	
	// dV is almost linear to PSI (AD profile 22.01.19) 

	if (!PSIloaded) Vol = psi * psi;// psi = r/a
	//else  Vol = psi;
	//if (Vol < 1.e-6) Vol = 1.e-6;
	else {
		int i = (int)floor(psi / StepFlux);
		if (i < Npsi) Vol = fVolume[i];
	}
	return Vol;
}

void CPlasma::SetPSIarray(int nr, int nz, double rmin, double zmin, double stepR, double stepZ)// set double ** PSI
{
	//ClearPSI();
	Rmin = rmin;
	Zmin = zmin;
	StepR = stepR;
	StepZ = stepZ;
	psiNr = nr;
	psiNz = nz;
	
	int i;
	PSI = new double *[psiNr + 1];
	for (i = 0; i <= psiNr; i++) {
		PSI[i] = new double[psiNz + 1];
	}
	//fVolume.RemoveAll();// -> in ClearPSI
	for (int ir = 0; ir <= Npsi; ir++) { // initialize PSI Volumes
		fVolume.Add(0);
	}
}

void CPlasma::ClearPSI()
{
	int i;
	if (psiNr > 0 || psiNz > 0) {
		for (i = 0; i <= psiNr; i++) delete[] PSI[i];
		delete[] PSI;
	}
	psiNr = 0;
	psiNz = 0;
	fVolume.RemoveAll();
	PSIloaded = FALSE;
}

void CPlasma::ClearProfiles()
{
	PSIprof.RemoveAll();
	TeNorm.RemoveAll();
	NeNorm.RemoveAll();
	Zeff.RemoveAll();
	Nprof = 0;
	ProfilesLoaded = FALSE;
	ClearSigma();
}

void CPlasma::ClearSigma()
{
	SigmArr.RemoveAll();
	DeltaArr.RemoveAll();
	SigmaLoaded = FALSE;
	DeltaLoaded = FALSE;
}

void CPlasma::SetCutRZ()
{
	CutRZ = new double *[Npsi + 1];
	for (int i = 0; i <= Npsi; i++) {
		CutRZ[i] = new double[Nz + 1];
		for (int j = 0; j <= Nz; j++) CutRZ[i][j] = 0;
	}
	CutPsiZ = new double *[Npsi + 1];
	for (int i = 0; i <= Npsi; i++) {
		CutPsiZ[i] = new double[Nz + 1];
		for (int j = 0; j <= Nz; j++) CutPsiZ[i][j] = 0;
	}
}

void CPlasma::SetZeroCuts()
{
	CTarget::SetZeroCuts();

	for (int i = 0; i <= Npsi; i++) {
		for (int j = 0; j <= Nz; j++) CutRZ[i][j] = 0;
	}
	for (int i = 0; i <= Npsi; i++) {
		for (int j = 0; j <= Nz; j++) CutPsiZ[i][j] = 0;
	}
}

int CPlasma::Distr2Dcell(C3Point Pgl, double val)
{
	int distr = CTarget::Distr2Dcell(Pgl, val);
	//if (distr == 0) return 0; // out of area

	double r, psi, Z, cell, SQcell;
	//Z = Pgl.Z - Zmin;
	//double cellRZ = StepFlux * Step.Z;
	//double SQcell = cellRZ * cellRZ;
	int i, k;
	double x0, x1, z0, z1, d00, d01, d10, d11;
	//C3Point Prz = GetTorRZ(Pgl.X, Pgl.Y, Pgl.Z); // GetLocal3D(Pgl);// in a volume
	//r = GetR(Pgl); // poloidal!!! radius of plasma > 0!
	//if (R < Rmin || R > Rmax || Z < Zmin || Z > Zmax) return 0;
	
	// R-Z
	cell = StepR*StepZ; // poloidal steps are DEFINED in SetPlasmaTarget!!!
	SQcell = cell * cell;
	C3Point Prz = GetTorRZ(Pgl.X, Pgl.Y, Pgl.Z);
	double Z0 = TorCentre.Z - PlasmaMinR * PlasmaEll;
	double Z1 = TorCentre.Z + PlasmaMinR * PlasmaEll;
	//SetPlasmaParam:
	//Rmin = R - a; 	Rmax = R + a;
	//Zmin = TorC.Z - a * Ell; Zmax = TorC.Z + a * Ell;
	Z = Pgl.Z - Z0; 
	r = Prz.X - Rmin; //PlasmaMajR - PlasmaMinR; // <> 0
	if (r < 0 || r > Rmax - Rmin || Z < 0 || Z > Z1 - Z0) return 0;
	i = (int)floor(r / StepR);
	k = (int)floor(Z / StepZ);
	x0 = r - i * StepR;//a
	x1 = StepR - x0;//b
	z0 = Z - k * StepZ;
	z1 = StepZ - z0;
	d00 = x1 * z1; // add to i, k
	d10 = x0 * z1; // add to i+1, k
	d01 = x1 * z0; // add to i, k+1
	d11 = x0 * z0; // add to i+1, k+1
	CutRZ[i][k] += val*d00 / SQcell;//cellYZ / cellYZ;
	CutRZ[i + 1][k] += val*d10 / SQcell;// cellYZ / cellYZ;
	CutRZ[i][k + 1] += val*d01 / SQcell;//cellYZ / cellYZ;
	CutRZ[i + 1][k + 1] += val*d11 / SQcell;// cellYZ / cellYZ;
							
	// PSI-Z
	cell = StepFlux * StepZ;
	SQcell = cell * cell;
	psi = GetPSI(Pgl);// >> ro [0..1]  // Pgl - decart glob ref
	if (psi < 0 || psi > 1 || Z < 0 || Z > Z1 - Z0) return 0;
	
	i = (int)floor(psi / StepFlux);
	//k = (int)floor(Z / StepZ);
	x0 = psi - i * StepFlux;//a
	x1 = StepFlux - x0;//b
	//z0 = Z - k * StepZ; //same as RZ
	//z1 = StepZ - z0; // same
	
	d00 = x1 * z1; // add to i, k
	d10 = x0 * z1; // add to i+1, k
	d01 = x1 * z0; // add to i, k+1
	d11 = x0 * z0; // add to i+1, k+1
	CutPsiZ[i][k] += val*d00 / SQcell;//cellYZ / cellYZ;
	CutPsiZ[i + 1][k] += val*d10 / SQcell;// cellYZ / cellYZ;
	CutPsiZ[i][k + 1] += val*d01 / SQcell;//cellYZ / cellYZ;
	CutPsiZ[i + 1][k + 1] += val*d11 / SQcell;// cellYZ / cellYZ;

	return 0;
}

void CPlasma::ClearCuts()
{
	CTarget::ClearCuts();
	//if (Npsi < 1) return;
	for (int i = 0; i <= Npsi; i++) delete[] CutRZ[i];
	delete[] CutRZ;
	for (int i = 0; i <= Npsi; i++) delete[] CutPsiZ[i];
	delete[] CutPsiZ;
}



/*// EQDSK format is taken from A.Dnestrovsky message 08/11/2017
write(lun_geqdsk,2000) geqdsk_lbl,idum, nh(Nx), nv(Ny)

write(lun_geqdsk,2001) rdim(DR),zdim(DZ),rcentr,rleft(Rmin),zmid(Zmid)
write(lun_geqdsk,2001) rmaxis,zmaxis,zpsimag(PSIc),zpsibdy(PSIx),bcentr
write(lun_geqdsk,2001) pcur,zpsimag(PSIc),xdum,rmaxis,xdum
write(lun_geqdsk,2001) zmaxis,xdum,zpsibdy(PSIx),xdum,xdum

write(lun_geqdsk,2001) (fpol(i),i=1,nh) // f = Bt*R
write(lun_geqdsk,2001) (pres(i),i=1,nh) // pressure N/m**2
write(lun_geqdsk,2001) (ffprime(i),i=1,nh) // smth on flux grid
write(lun_geqdsk,2001) (pprime(i),i=1,nh)  // smth on flux grid

write(lun_geqdsk,2001) ((psirz(i,j),i=1,nh),j=1,nv) // PSI table [Nx,Ny]

write(lun_geqdsk,2001) (qpsi(i),i=1,nh)
write(lun_geqdsk,2002) nb,nblim
write(lun_geqdsk,2001) (rbdy(i),zbdy(i),i=1,nb)
write(lun_geqdsk,2001) (rlim(i),zlim(i),i=1,nblim)

//next part - from Kuyan 01/11/2010
Rmax = Rmin + DR
Zmin = Zmid - DZ/2
Zmax = Zmid + DZ/2
B0 = bcentr * rcentr / rmaxis // B on mag axis?

DO i1 = 1 , Nx
Rw(i1) = Rmin + (Rmax-Rmin)*(i1-1)/(Nx-1)
END DO
DO i2 = 1 , Ny
Z(i2) = Zmin + (Zmax-Zmin)*(i2-1)/(Ny-1)
END DO

! Приведение к нулю на магнитной оси
DO i2 = 1 , Ny
DO i1 = 1 , Nx
psirz(i1,i2) = psirz(i1,i2) - PSIc
END DO
END DO
PSIx = PSIx - PSIc // на сепаратрисе
*/
//val = (val - PSIc) / (PSIx - PSIc); //-> psi = psirz / PSIx

BOOL CPlasma::ReadPSI(char * name)
{
	FILE * fin;
	CString S;
	if ((fin = fopen(name, "r")) == NULL) {
		S = "Can't find/open ";
		S += name;
		AfxMessageBox(S, MB_ICONSTOP | MB_OK);
		return FALSE;
	}

	char buf[1024];
	int idum, nr, nz;
	char s[255];

	// line 1 : geqdsk_lbl,idum, nh(Nx), nv(Ny)
	if (fgets(buf, 1024, fin) == NULL) {
		AfxMessageBox("Problem with line 1");
		fclose(fin);
		return FALSE;
	}

	int res = sscanf(buf, "%s %d %d %d", s, &idum, &nr, &nz);
	if (res != 4) {
		AfxMessageBox("Invalid format in line 1", MB_ICONSTOP | MB_OK);
		fclose(fin);
		return FALSE;
	}
	if (nr * nz <= 1.e-6) {
		AfxMessageBox("Invalid numbers Nr, Nz", MB_ICONSTOP | MB_OK);
		fclose(fin);
		return FALSE;
	}
	//S.Format("Nr = %d Nz = %d", nr, nz); AfxMessageBox(S);
	
	double val;
	// line 2 : rdim(DR),zdim(DZ),rcentr,rleft(Rmin),zmid(Zmid)
	double DR, DZ, rmin, zmid; //read
	//double Rmax, Zmin, Zmax;   // calculated
	if (fgets(buf, 1024, fin) == NULL) {
		AfxMessageBox("Problem with line 2");
		fclose(fin);
		return FALSE;
	}

	res = sscanf(buf, "%le %le %le %le %le", &DR, &DZ, &val, &rmin, &zmid);
	if (res != 5) {
		AfxMessageBox("Invalid format in line 2", MB_ICONSTOP | MB_OK);
		fclose(fin);
		return FALSE;
	}
	if (DR*DZ <= 1.e-6) {
		AfxMessageBox("Invalid dimensions DR, DZ", MB_ICONSTOP | MB_OK);
		fclose(fin);
		return FALSE;
	}
	//S.Format("line 2: Rmin = %le Zmid = %le", rmin, zmid); AfxMessageBox(S);
	
	Rmin = rmin;
	Rmax = Rmin + DR;
	Zmid = zmid;// +TorCentre.Z;// !!!! moves with tokamak
	Zmin = Zmid - 0.5*DZ;
	Zmax = Zmid + 0.5*DZ;
	//Zmid = zmid;
	StepR = DR / (nr - 1);
	StepZ = DZ / (nz - 1);

	//S.Format("Rmin = %g Rmax = %g\n Zmin = %g Zmax = %g", Rmin, Rmax, Zmin, Zmax); AfxMessageBox(S);
	// line 3 : rmaxis,zmaxis,zpsimag(PSIc),zpsibdy(PSIx),bcentr
	double v1, v2; // read but not used
	double PSIc, PSIx;
	if (fgets(buf, 1024, fin) == NULL) {
		AfxMessageBox("Problem with line 3");
		fclose(fin);
		return FALSE;
	}

	res = sscanf(buf, "%le %le %le %le %le", &v1, &v2, &PSIc, &PSIx, &val);
	if (res != 5) {
		AfxMessageBox("Invalid format in line 3", MB_ICONSTOP | MB_OK);
		fclose(fin);
		return FALSE;
	}
	if (fabs(PSIc*PSIx) <= 1.e-6) {
		AfxMessageBox("PSIc or PSIx too small", MB_ICONSTOP | MB_OK);

	}
	//S.Format("line 3: PSIc = %le PSIx = %le", PSIc, PSIx); AfxMessageBox(S);

	// line 4 : read but not used ( pcur,zpsimag(PSIc),xdum,rmaxis,xdum )
	if (fgets(buf, 1024, fin) == NULL) {
		AfxMessageBox("Problem with line 4");
		fclose(fin);
		return FALSE;
	}
	//S.Format("line 4: %s", buf); AfxMessageBox(S);
	
	// line 5 : read but not used  ( zmaxis,xdum,zpsibdy(PSIx),xdum,xdum )
	if (fgets(buf, 1024, fin) == NULL) {
		AfxMessageBox("Problem with line 5");
		fclose(fin);
		return FALSE;
	}
	//S.Format("line 5: %s", buf); AfxMessageBox(S);

	// read arrays
	char *buff, *nptr, *endptr;
	CString text = "";

	while (!feof(fin)) { // read total text in file
		fgets(buf, 1024, fin);
		if (!feof(fin))	text += buf;
	}

	int size = text.GetLength();
	//S.Format("text size: %d ", size); AfxMessageBox(S);


	// parse text
	int length = text.GetLength() * 2;
	buff = (char *)calloc(length, sizeof(char));
	strcpy(buff, text);
	nptr = buff;
	/*	int count = 0;
	int len = (int)strlen(nptr);//sizeof(nptr);
	while (len > 0) { //!feof(fin)) {
	val = strtod(nptr, &endptr);
	count++;
	//plate0->Load->Val[i][j] = val;
	if (nptr == endptr) nptr++; //catch special case
	else nptr = endptr;
	len = (int)strlen(nptr);// sizeof(nptr);
	}
	S.Format("%d chars\n %d values", size, count);
	AfxMessageBox(S);
	*/

	// read 4 arrays [Nx]
	for (int j = 0; j < 4; j++) {
		for (int i = 0; i < nr; i++) {
			val = strtod(nptr, &endptr);
			if (nptr == endptr) nptr++; //catch special case
			else nptr = endptr;
		}
	}
	//AfxMessageBox("read 4 arrays");

	double PSIval;
//	int kpsi;
	double PSIxn = PSIx - PSIc; // на сепаратрисе
//	CPlate * plate0 = new CPlate();
//	CLoad * load;
	
/*	C3Point lb = C3Point(1000, Rmin, Zmin);
	C3Point rt = C3Point(1000, Rmax, Zmax);
	plate0->SetFromLimits(lb, rt);
	load = new CLoad(DR, DZ, stepR, stepZ);*/

	//S.Format("psi Nr=%d Nz=%d", psiNr, psiNz); AfxMessageBox(S);// must be 0 at 1st
	ClearPSI();	//AfxMessageBox("Cleared PSI");
	
	SetPSIarray(nr, nz, Rmin, Zmin, StepR, StepZ); //alloc mem for psi[psiNr = nr,psiNz = nz] fVolume[Npsi+1]
	//AfxMessageBox("Allocated memory for PSI");

	// read pol flux array [Nr, Nz]	1...Nr, 1...Nz
	int count = 0;
	for (int j = 0; j < nz; j++) {
		for (int i = 0; i < nr; i++) {
			val = strtod(nptr, &endptr);
			count++;
			//val = (val - PSIc) / (PSIx - PSIc); //-> psi = psirz / PSIx
			PSIval = (val - PSIc) / PSIxn; // normalised
			//load->Val[i][j] = PSI; //-> psi = psirz / PSIxval;
			PSI[i][j] = PSIval;
			if (nptr == endptr) nptr++; //catch special case
			else nptr = endptr;
			int kpsi = (int)floor(PSIval / StepFlux);
			if (kpsi >= 0 && kpsi < Npsi)	fVolume[kpsi] += 1;// stepR*stepZ
		}
	}
	// next lines - not used 
	fclose(fin);
	if (count < 1) return FALSE;

	S.Format("%d PSI values read from file /n Nr %d Nz %d", count, nr, nz);
	AfxMessageBox(S);
	PSIloaded = TRUE;
	return TRUE; // success - PSIloaded

	//plate0->Number = 1000;
	//pMarkedPlate = plate0;
	//pMarkedPlate->Load = load;
	//pTorCrossPlate = plate0;
	//pTorCrossPlate->Load = load;

	//PSILoaded = TRUE;
	//if (ProfLoaded) PlasmaLoaded = TRUE;
	//else PlasmaLoaded = FALSE;

	//if (scenario >0) pTorCrossPlate = plate0; else pTorCrossPlate = NULL;
	//OptParallel = FALSE;// not parallel calculation
	//OptCombiPlot = 0;
}

double CPlasma::GetPSI(double R, double Z) // R,Z must be in TOR ref!!! >> return ro (norm radius) or PSI (if loaded)
{
	if (!PSIloaded) { // Zmag = 0  
		// circular plasma - old
		//double r = fabs(R - PlasmaMajR); //poloidal radius at z=0
		//return sqrt(r*r + Z*Z)/PlasmaMinR; // r norm
		double a, b, R0, ra, rb, ro, roa, rob;// da, Aright, Aleft;

	// elliptic
		a = PlasmaMinR;
		b = a * PlasmaEll;
		R0 = PlasmaMajR; // CS centre
		ra = R - R0; // rx - hor
		rb = Z;// -TorCentre.Z; // ry - vert
		roa = ra / a;
		rob = rb / b;
		ro = sqrt(roa*roa + rob*rob);
		return (ro);

	// bi-elliptic a = 0.5/1.5 PlasmaMinorR
	/*	R0 = PlasmaMajR - 0.5 * a; // plasma CS "centre"
		da = R - R0;
		r = sqrt(da*da + Z*Z);
		Aright = 1.5 * a * fabs(da / r);
		Aleft = 0.5 * a * fabs(da / r);
		if (da >= 0.) return (da / Aright); // outer half radius
		else return (-da / Aleft); // inner half radius*/
	}

	// PSI is loaded -----------------------------
	double r, z, dr, dz;
	r = R - Rmin;
	z = Z - Zmin;
	double psi = -1;
	if (r < 0 || r > Rmax - Rmin) return psi;
	if (z < 0 || z > Zmax - Zmin) return psi;

	int i = (int)floor(r / StepR);
	int j = (int)floor(z / StepZ);
	if (i <= 0 || i >= psiNr || j <= 0 || j >= psiNz) return psi; //-1 on bound too!
		
	dr = r - i*StepR;
	if (dr >= StepR) { i++; dr = r - i*StepR; }
	dz = z - j*StepZ;
	if (dz >= StepZ) { j++; dz = z - j*StepZ; }
		
	double dr1, dz1, psi1, psi2, psi3;
	if (dz >= dr*StepZ / StepR) { // upper-left triangle
			dr1 = dr*StepZ / dz;
			psi1 = PSI[i][j + 1] + dr1 / StepR*(PSI[i + 1][j + 1] - PSI[i][j + 1]);
			psi2 = PSI[i][j] + dz / StepZ * (psi1 - PSI[i][j]);
			//	return val;
		}
		else { // lower-right triangle
			dz1 = dz*StepR / dr;
			psi1 = PSI[i + 1][j] + dz1 / StepZ*(PSI[i + 1][j + 1] - PSI[i + 1][j]);
			psi2 = PSI[i][j] + dr / StepR * (psi1 - PSI[i][j]);
			//	return val;
		}

		if (dz <= (StepR - dr)*StepZ / StepR) { // lower-left triangle
			dr1 = dr*StepZ / (StepZ - dz);
			psi1 = PSI[i][j] + dr1 / StepR*(PSI[i + 1][j] - PSI[i][j]);
			psi3 = psi1 + dz / StepZ * (PSI[i][j + 1] - psi1);
			//	return val;
		}
		else { // upper-right triangle
			dz1 = StepZ - (StepZ - dz) * StepR / dr;
			psi1 = PSI[i + 1][j] + dz1 / StepZ*(PSI[i + 1][j + 1] - PSI[i + 1][j]);
			psi3 = PSI[i][j + 1] + dr / StepR * (psi1 - PSI[i][j + 1]);
			//	return val;
		}

		return (psi2 + psi3) * 0.5;
	//} // i<Nx j<Ny
		
	//	Ploc = pTorCrossPlate->GetLocal(Pgl);
	//	PSI = pTorCrossPlate->Load->GetVal(Ploc.X, Ploc.Y);
	
	//return psi;
}

double CPlasma::GetPSI(C3Point P) // P - in decart global ref
{
	/*if (!PSIloaded) {
		double r = GetR(P); //poloidal radius at z=0
		return (fabs(r) / PlasmaMinR); // r normalized
	}*/

	C3Point Prrz = GetTorRZ(P.X, P.Y, P.Z);
	double psi = GetPSI(Prrz.X, Prrz.Z);
	return psi;
}

BOOL CPlasma::ReadProfilesWithSigmas(char * name) // same as ReadProfiles + 4 colums (SigmaCX, II, EI, Delta)
{
	FILE * fin;
	//ifstream fin(name);
	CString S;
	if ((fin = fopen(name, "r")) == NULL) {
		S = "Can't find/open ";
		S += name;
		AfxMessageBox(S, MB_ICONSTOP | MB_OK);
		return FALSE;
	}
	
	ClearProfiles(); // with sigma delta

	double MaxDensity = 0;
	double MaxTe = 0;
	double MaxPSI = -100;
	double MinPSI = 100;
	double MaxSigma = 0;
	double MaxDelta = 0;
	char buf[1024];
	double psi, Te, Ne, Ti, Ni, Nz, Z, ro;
	double CX, II, EI, Delta;
	C3Point P, Sigma;
	int i, j = 0;
	string s;
		
	fgets(buf, 1024, fin);
	fgets(buf, 1024, fin);
	fgets(buf, 1024, fin);
	Delta = 1.2; // ITER default
	while (!feof(fin)) {
		//psi(Wb)	Te(eV)		ne(m - 3)	Ti(eV)		ni(m - 3)	Zeff(-)	 normalized rho
		//7 columns, - ITER provided 29.11 (Merilin Schneider)
		//int result = fscanf(fin, " %le  %le  %le  %le  %le  %le  %le", &psi, &Te, &Ne, &Ti, &Ni, &Z, &ro); //Te[eV] Ne[m-3]
		
	//while (fin >> psi >> Te >> Ne >> Ti >> Z >> )
		int result = fscanf(fin, " %le %le %le %le %le %le %le %le %le", 
			&psi, &Te, &Ne, &Ti, &Z, &CX, &II, &EI, &Delta); //Te,keV	Ne,10^19/m3
		if (result < 9) { // 9 with Delta 

		   //int result = fscanf(fin, " %le %le %le %le %le %le %le %le %le %le",
			//	&psi, &Te, &Ne, &Ti, &Z, &Ni, &Nz, &CX, &II, &EI); //here NO DELTA! (AD) Te,keV	Ne,10^19/m3
			// Ni, Nz - not applied !
			//if (result < 10) { // without Delta 
			fgets(buf, 1024, fin);
			continue;
			//fclose(fin);
			//return 0;
		}
		else {
			//fgets(buf, 1024, fin); // to the end of line 
			PSIprof.Add(psi);
			TeNorm.Add(Te);//Te,keV	
			NeNorm.Add(Ne);//Ne,10^19/m3
						   //Zeff.Add(Z);
			Sigma = C3Point(CX, II, EI);
			SigmArr.Add(Sigma);
			DeltaArr.Add(Delta);

			MaxDensity = Max(MaxDensity, Ne);
			MaxTe = Max(MaxTe, Te);
			MaxPSI = Max(MaxPSI, psi);
			MinPSI = Min(MinPSI, psi);
			MaxSigma = Max(MaxSigma, Sigma.X + Sigma.Y + Sigma.Z);
			MaxDelta = Max(MaxDelta, Delta);
			j++;// data line
		}
		//else	fgets(buf, 1024, fin);
	}
	fclose(fin);

	Nprof = j;//last read
	if (Nprof < 1) {
		S.Format("%d lines read by ", j);
		AfxMessageBox(S + "ReadProfilesWithSigmas \n error: invalid number of columns? (9)");
		ProfilesLoaded = FALSE;
		SigmaLoaded = FALSE;
		DeltaLoaded = FALSE;
		return FALSE;
	}
	double PSIc = PSIprof[0];
	double PSIx = PSIprof[Nprof - 1];

	for (i = 0; i < Nprof; i++) {
		psi = PSIprof[i];
		PSIprof[i] = (psi - PSIc) / (PSIx - PSIc);
	}

	if (MaxDensity > 1.e-3) Nemax = MaxDensity;
	if (MaxTe > 1.e-3) 	Temax = MaxTe;

	for (int i = 0; i < Nprof; i++) { // normalize profiles - div by max
		TeNorm[i] = TeNorm[i] / Temax;
		NeNorm[i] = NeNorm[i] / Nemax;
	}

	if (Nemax < 1000) Nemax *= 1.e+19; // correct coeff Nemax *= 1.e19;
	if (Temax > 1000) Temax *= 1.e-3; // eV -> keV

	S.Format("%d lines\n PSImax = %g Temax = %g keV Nemax = %g m-3\n PSIc = %g PSIx = %g\n MaxSigma %g, MaxDelta %g",
				Nprof, PSIprof[Nprof - 1], Temax, Nemax, PSIc, PSIx, MaxSigma, MaxDelta);
	AfxMessageBox(S);
	
	SigmaLoaded = TRUE;
	DeltaLoaded = TRUE;
	return TRUE;

}
void CPlasma::SetParProfiles()
{
	ClearProfiles();
	double dpsi = 0.05;
	double psi, Te, Ne;
	int N = 20;
	int ordTe = (int)pDoc->PlasmaTeOrd;
	int ordNe = (int)pDoc->PlasmaDensityOrd;
	for (int i = 0; i <= N; i++) {
		psi = i * dpsi;
		Te = GetPlasmaTeParabolic(psi, ordTe);
		Ne = GetPlasmaDensityParabolic(psi, ordNe);
		PSIprof.Add(psi);
		TeNorm.Add(Te);//Te,keV	
		NeNorm.Add(Ne);//Ne,10^19/m3
		//Zeff.Add(Z);
	}
	Nprof = N + 1;

	if (Nemax < 1000) Nemax *= 1.e+19; // correct coeff Nemax *= 1.e19;
	if (Temax > 1000) Temax *= 1.e-3; // eV -> keV
	
}

BOOL CPlasma::ReadProfiles(char * name)//psi(Wb)	Te(eV)		ne(m - 3)	Ti(eV)		ni(m - 3)	Zeff
{
	FILE * fin;
	CString S;
	if ((fin = fopen(name, "r")) == NULL) {
		S = "Can't find/open ";
		S += name;
		AfxMessageBox(S, MB_ICONSTOP | MB_OK);
		return FALSE;
	}

	ClearProfiles();

	double MaxDensity = 0;
	double MaxTe = 0;
	double MaxPSI = -100;
	double MinPSI = 100;
	
	char buf[1024];
	double psi, Te, Ne, Ti, Ni, Z, ro;
	C3Point P;
	int i, j = 0;
	//for (int k = 0; k <2; k ++) 
	fgets(buf, 1024, fin);
	while (!feof(fin)) {
		//psi(Wb)	Te(eV)		ne(m - 3)	Ti(eV)		ni(m - 3)	Zeff(-)	 normalized rho
		//7 columns, - ITER provided 29.11 (Merilin Schneider)
		/*int result = fscanf(fin, " %le  %le  %le  %le  %le  %le  %le", &psi, &Te, &Ne, &Ti, &Ni, &Z, &ro); //Te[eV] Ne[m-3] 
		if (result != 5) { // 1st 3 col
			fgets(buf, 1024, fin);
			continue;
		}*/
																										   
		// TIN																								   
		int result = fscanf(fin, " %le  %le  %le  %le  %le", &psi, &Te, &Ne, &Ti, &Z); //Te,keV	Ne,10^19/m3
		if (result != 5) { // 1st 3 col
			fgets(buf, 1024, fin);
			continue;
		}
		else {
			//fgets(buf, 1024, fin); // to the end of line 
			PSIprof.Add(psi);
			TeNorm.Add(Te);//Te,keV	
			NeNorm.Add(Ne);//Ne,10^19/m3
			//Zeff.Add(Z);
			
			MaxDensity = Max(MaxDensity, Ne);
			MaxTe = Max(MaxTe, Te);
			MaxPSI = Max(MaxPSI, psi);
			MinPSI = Min(MinPSI, psi);
			j++;// data line
		}
		//else	fgets(buf, 1024, fin);
	}
	fclose(fin);

	Nprof = j;//last read
	if (Nprof < 1) {
		AfxMessageBox("error\n invalid number of columns?(must be 5/7)");
		ProfilesLoaded = FALSE;
		return FALSE;
	}
	double PSIc = PSIprof[0];
	double PSIx = PSIprof[Nprof - 1];

	for (i = 0; i < Nprof; i++) {
		psi = PSIprof[i];
		PSIprof[i] = (psi - PSIc) / (PSIx - PSIc);
	}

	if (MaxDensity > 1.e-3) Nemax = MaxDensity;
	if (MaxTe > 1.e-3) 	Temax = MaxTe;

	for (int i = 0; i < Nprof; i++) { // normalize profiles - div by max
		TeNorm[i] = TeNorm[i] / Temax;
		NeNorm[i] = NeNorm[i] / Nemax;
	}

	if (Nemax < 1000) Nemax *= 1.e+19; // correct coeff Nemax *= 1.e19;
	if (Temax > 1000) Temax *= 1.e-3; // eV -> keV
	
	S.Format("%d lines\n PSImax = %g Temax = %g keV Nemax = %g m-3\n PSIc = %g PSIx = %g", 
					Nprof, PSIprof[Nprof-1], Temax, Nemax, PSIc, PSIx); 
	AfxMessageBox(S);
	return TRUE;
}


/////////////////////////////////////////////////////////////////////
C3Point Bstray(double X, double Y, double /* Z */)
{
	C3Point B0(0,0,0);
	if (pDoc->MFcoeff  == 0) return B0;
	double XmaxRemnant = Max(Xremnant1, Xremnant2);
	XmaxRemnant = Max(XmaxRemnant, Xremnant3);
	if (TaskRemnantField && X < XmaxRemnant) return B0;

	if (X < Bstray_Xmin || X > Bstray_Xmax ) return B0;
//	B0.Y = 0.1* sin(2*(X - MFXmin)) * (X - MFXmin);
//	B0.Z = 0.1* cos(2*(X - MFXmin)) * (X - MFXmin);
	double a = 13.63;
	double b =1.575;
	double alfa = atan((b + Y)/(a - X));
	double R = sqrt((a-X)*(a-X) + (b + Y)*(b + Y)); // dist from Tokamak centre
	int i = (int)(R / MFStepR);
	C3Point Btok = MF[i];
	B0.X = -(Btok.Y * cos(alfa) - Btok.X * sin(alfa));
	B0.Y = Btok.Y * sin(alfa) + Btok.X * cos(alfa);
	B0.Z = Btok.Z;

//	B0.X = 0; B0.Y = 0; B0.Z = 0.0001;

/*	int i = (int)((X + 14 - MFXmax)/ MFStepX);
	if (i>MFSize) return B0;
	B0 = MF[MFSize-i];
	if (X < 5) B0 = B0 * 0.01; */
	return B0;

}

/*
 void  ReadMFData()
 {
	 MF.RemoveAll();
	 C3Point B;
	char buf[1024];
	int  result;
	FILE * fin;
	if ((fin = fopen(MFname, "r")) == NULL) {
		CString S = "Can't find/open ";
		S += MFname;
		AfxMessageBox(S, MB_ICONSTOP | MB_OK);
		exit(0);
		return;
	}
	fgets(buf, 1024, fin); // read 1-st line
	int i = -1;
	double r, x, y, z,  Bx, By, Bz, modB, BB;
	MFBmax = 0;
		
	while  (!feof(fin)) 
	{
		result = fscanf(fin, "%f %f %f %f %le %le %le %le", &r, &x, &y, &z, &Bx, &By, &Bz, &modB);
		if (result != 8) break;
		fgets(buf, 1024, fin);
	//	if (!OptZeroBx) B.X = Bx; 
	//	if (!OptZeroBy) B.Y = By; 
	//	if (!OptZeroBz) B.Z = Bz; 
		B.X = Bx *1.e-4;
		B.Y = By *1.e-4;
		B.Z = Bz *1.e-4;
		BB = modB*1.e-4;
		MF.Add(B);
		if (r>5)
		MFBmax = Max(MFBmax, BB);
		i++;
		MFSize = i; 

	}
	fclose(fin);
  }
*/
 void 	SetMFBmax()
 {
	 C3Point B0;
	 double Bmax = 0;
	 double x = Bstray_Xmin;
	 while (x < Bstray_Xmax) {
	  	B0 = Bstray(x,0,0);
		Bmax =Max(B0.X, Bmax);
		Bmax =Max(B0.Y, Bmax);
		Bmax =Max(B0.Z, Bmax);
		x += 0.1;
	 }
	 MFBmax =Bmax;
}

 void DrawMF(CDC* pDC, double y, double z)
  {
	int X0, Y0, Z0, Top = 100;
	double ScaleX, ScaleY, ScaleZ;
//	CDC* pDC = pMainView->GetDC();
	X0 = pMainView->OrigX;
	Y0 = pMainView->OrigY; 
	Z0 = pMainView->OrigZ;
	ScaleX = pMainView->ScaleX;   
	ScaleY = pMainView->ScaleY;
	ScaleZ = pMainView->ScaleZ;
	C3Point B0;
	double x, B;
	int K = 1;
	int Ylev = (int)(y * ScaleY);
	int Zlev = (int)(z * ScaleZ);
	double Bmax = 1;
	SetMFBmax();
	if (MFBmax>1) Bmax = log(MFBmax);
	else Bmax = MFBmax;
	CPen  RedPen, GreenPen, BluePen;
	CPoint prevPlan, prevSide, nextPlan, nextSide;

	if (!RedPen.CreatePen(PS_SOLID, 1, RGB(255, 0, 0))) return;
	CPen * pOldPen = pDC->SelectObject(&RedPen);
	x = 0; /// Bx 
	prevPlan.x = X0; prevPlan.y = Y0 - Ylev; 
	prevSide.x = X0; prevSide.y = Z0 - Zlev; 
	while (x < Bstray_Xmax) {
		B0 = Bstray(x,y,z); // GetMF(x, z);
		B = 0;
		if (B0.X>1) B = log(B0.X); 
		if (B0.X<=1) B = B0.X;
		if (B0.X<-1) B = log(-B0.X); 
		nextPlan.x = X0 + (int)(x * ScaleX);
		nextPlan.y = Y0 - Ylev - (int)(B / Bmax * Top *K);
		nextSide.x = X0 + (int)(x * ScaleX);
		nextSide.y = Z0 - Zlev - (int)(B / Bmax* Top *K);
	//	pDC->MoveTo(prevPlan); pDC->LineTo(nextPlan);
		pDC->MoveTo(prevSide); pDC->LineTo(nextSide);
		prevPlan = nextPlan; prevSide = nextSide;
		x += 0.1;
	}
	
	if (!GreenPen.CreatePen(PS_SOLID, 1, RGB(0, 255, 0))) return;
	pDC->SelectObject(&GreenPen);
	x = 0;  /// By
	prevPlan.x = X0; prevPlan.y = Y0 - Ylev; 
	prevSide.x = X0; prevSide.y = Z0 - Zlev; 
	while (x < Bstray_Xmax) {
		B0 = Bstray(x,y,z); // GetMF(x, z);
		B = 0;
		if (B0.Y>1) B = log(B0.Y); 
		if (B0.Y<=1) B = B0.Y;
		if (B0.Y<-1) B = log(-B0.Y); 
		nextPlan.x = X0 + (int)(x * ScaleX);
		nextPlan.y = Y0 - Ylev - (int)(B / Bmax * Top *K);
		nextSide.x = X0 + (int)(x * ScaleX);
		nextSide.y = Z0 - Zlev - (int)(B / Bmax* Top *K);
	//	pDC->MoveTo(prevPlan); pDC->LineTo(nextPlan);
		pDC->MoveTo(prevSide); pDC->LineTo(nextSide);
		prevPlan = nextPlan; prevSide = nextSide;
		x += 0.1;
	}

	if (!BluePen.CreatePen(PS_SOLID, 1, RGB(0, 0, 255))) return;
	pDC->SelectObject(&BluePen);
	x = 0;  /// Bz
	prevPlan.x = X0; prevPlan.y = Y0 - Ylev; 
	prevSide.x = X0; prevSide.y = Z0 - Zlev; 
	while (x < Bstray_Xmax) {
		B0 = Bstray(x,y,z); // GetMF(x, z);
		B = 0;
		if (B0.Z>1) B = log(B0.Z); 
		if (B0.Z<=1) B = B0.Z;
		if (B0.Z<-1) B = log(-B0.Z); 
		nextPlan.x = X0 + (int)(x * ScaleX);
		nextPlan.y = Y0 - Ylev - (int)(B / Bmax* Top *K);
		nextSide.x = X0 + (int)(x * ScaleX);
		nextSide.y = Z0 - Zlev - (int)(B / Bmax* Top *K);
	//	pDC->MoveTo(prevPlan); pDC->LineTo(nextPlan);
		pDC->MoveTo(prevSide); pDC->LineTo(nextSide);
		prevPlan = nextPlan; prevSide = nextSide;
		x += 0.1;
	}
	
	pDC->SelectObject(pOldPen);

}

double Bremnant(double X)
{
	double Bz = 0;
	if (X < Xremnant1) Bz = Bremnant1 * 1.e-4; //T
	if (X >=Xremnant1 && X < Xremnant2) Bz = Bremnant2 * 1.e-4; //T
	if (X >=Xremnant2 && X < Xremnant3) Bz = Bremnant3 * 1.e-4; //T
	if (X >=Xremnant3) Bz = 0;
	return Bz;
}

void ShowTestField()
{
	int X0, Y0, Z0, Top =100;
	double ScaleX, ScaleY, ScaleZ;
	CDC* pDC = pMainView->GetDC();
	X0 = pMainView->OrigX;
	Y0 = pMainView->OrigY; 
	Z0 = pMainView->OrigZ;
	ScaleX = pMainView->ScaleX;   
	ScaleY = pMainView->ScaleY;
	ScaleZ = pMainView->ScaleZ;
	double x, B;

	double Bmax = Max(fabs(Bremnant1), fabs(Bremnant2));
	Bmax = Max(Bmax, fabs(Bremnant3));

	CPen  Pen;

	if (!Pen.CreatePen(PS_SOLID, 2, RGB(200, 200, 0))) return;
	CPen * pOldPen = pDC->SelectObject(&Pen);
	x = 0; 	B = 0;
	pDC->MoveTo(X0 + (int)(x * ScaleX), Z0 - (int)(B/Bmax*Top));
	x = 0; 	B = Bremnant1;
	pDC->LineTo(X0 + (int)(x * ScaleX), Z0 - (int)(B/Bmax*Top));
	x = Xremnant1;  B = Bremnant1;
	pDC->LineTo(X0 + (int)(x * ScaleX), Z0 - (int)(B/Bmax*Top));
	x = Xremnant1;  B = Bremnant2;
	pDC->LineTo(X0 + (int)(x * ScaleX), Z0 - (int)(B/Bmax*Top));
	x = Xremnant2;  B = Bremnant2;
	pDC->LineTo(X0 + (int)(x * ScaleX), Z0 - (int)(B/Bmax*Top));
	x = Xremnant2;  B = Bremnant3;
	pDC->LineTo(X0 + (int)(x * ScaleX), Z0 - (int)(B/Bmax*Top));
	x = Xremnant3;  B = Bremnant3;
	pDC->LineTo(X0 + (int)(x * ScaleX), Z0 - (int)(B/Bmax*Top));
	x = Xremnant3;  B = 0;
	pDC->LineTo(X0 + (int)(x * ScaleX), Z0 - (int)(B/Bmax*Top));
	
	pDC->SelectObject(pOldPen);
	pMainView->ReleaseDC(pDC);

}
/*
void ReadGas()
{
	Density.RemoveAll();
	DensX.RemoveAll();
	char buf[1024];
	int  result;
	FILE * fin;
	if ((fin = fopen(GasName, "r")) == NULL) {
		CString S = "Can't find/open ";
		S += GasName;
		AfxMessageBox(S, MB_ICONSTOP | MB_OK);
		exit(0);
		return;
	}
	fgets(buf, 1024, fin); // read 1-st line
	int i = -1;
	double  x, Dens;
	MaxDensity = 0;
		
	while  (!feof(fin)) 
	{
		result = fscanf(fin, "%le  %le", &x, &Dens);
		if (result != 2)  break;
		fgets(buf, 1024, fin);
		Density.Add(Dens*1000000);
		DensX.Add(x);
		MaxDensity = Max(MaxDensity, Dens*1000000);
	}
	fclose(fin);

}*/

void ShowGas(CDC* pDC)
{
	int X0, Y0, Z0, Top =100;
	double ScaleX, ScaleY, ScaleZ;
//	CDC* pDC = pMainView->GetDC();
	X0 = pMainView->OrigX;
	Y0 = pMainView->OrigY; 
	Z0 = pMainView->OrigZ;
	ScaleX = pMainView->ScaleX;   
	ScaleY = pMainView->ScaleY;
	ScaleZ = pMainView->ScaleZ;
	double x, D;
	int  Nmax = Density.GetUpperBound();

	CPen  Pen;

	if (!Pen.CreatePen(PS_SOLID, 1, RGB(0, 155, 155))) return;
	CPen * pOldPen = pDC->SelectObject(&Pen);
	x = 0; 	D = 0;
	pDC->MoveTo(X0 + (int)(x * ScaleX), Z0 - (int)(D/MaxDensity*Top));
	for (int i =0; i <= Nmax; i++) {
		x =  DensX[i]; D = Density[i];
		pDC->LineTo(X0 + (int)(x * ScaleX), Z0 - (int)(D/MaxDensity*Top));
	}
	pDC->SelectObject(pOldPen);
}
		

double GetDensity(double x)
{
	double Dens = 0;
	double D0, D1, X0, X1;
	int Nmax = Density.GetUpperBound();
	for (int i = 0; i < Nmax; i++) {
		X0 = DensX[i];	D0 = Density[i];
		X1 = DensX[i+1];	D1 = Density[i+1];
		if (x >= X0 && x < X1) {
			Dens = D0 + (D1 - D0) * (x - X0) / (X1-X0);
			return Dens;
			break;
		}
	}
	return Dens;

}

void SetReionSigma()
{
	if (pDoc->ReionPercent < 1.e-100) return; // Sigma is set
	double Summa = 0;
	double x = Reion_Xmin;
	double dL = 0.01;
	while (x < Reion_Xmax) {
		Summa += GetDensity(x) * dL;
		x += dL;
	}
	pDoc->ReionSigma = - log(1. - (pDoc->ReionPercent) * 0.01) / Summa;
}

void SetNeutrSigma(double NeutrPart, double NeutrXmax)
{
	if (NeutrPart < 1.e-100) return; // Sigma is set
	double Summa = 0;
	double x = 0; // Xmin
	double dL = 0.01;
	while (x < NeutrXmax) {
		Summa += GetDensity(x) * dL;
		x += dL;
	}
	NeutrSigma = - log(1. - NeutrPart) / Summa;
}

 void DeleteArrays()
{
	 MF.RemoveAll();
	 Density.RemoveAll();
	 DensX.RemoveAll();
}

void SortDoubleArray(CArray <double, double> & Arr)
{
	int n = Arr.GetUpperBound();
	double val;
	for (int i = 1; i <= n; i++) {
		for (int j = 0; j < i; j++) {
			if (Arr[i] < Arr[j]) { // remember Arr[i]
				val = Arr[i];
				for (int k = i; k > j; k--)  // shift element  right 1 pos
					Arr[k] = Arr[k-1];
				Arr[j] = val; // put former A[i] to A[j] place
			} // if found
		} // j
	} // i

}

int GetDoubleArrayPos(CArray <double, double> & Arr, double elem)
{
	double eps = 0.001;
	int n, i = -1;
	for (n = 0; n <= Arr.GetUpperBound(); n++) {
		if (fabs(Arr[n] - elem) < eps) {
			i = n; // exact pos
			break;
		}
	}
	if (i < 0) { //find interval
		for ( n = 0; n < Arr.GetUpperBound(); n++) {
		if (Arr[n] <= elem && Arr[n+1] > elem) {
			i = n; // left limit
			break;
		}
		}
	}

	return i;
}