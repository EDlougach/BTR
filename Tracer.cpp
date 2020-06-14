// Tracer.cpp : implementation file
//

#include "stdafx.h"
#include <math.h>
#include "BTR.h"
#include "BTRDoc.h"
#include "MainView.h"
#include "SetView.h"
#include "Tracer.h"
#include "psapi.h"
//#include <iostream>
//#include "geom_util.h"

// global counters
extern int NofCalculated;
extern double AtomPower1;
extern double AtomPower2;
extern double AtomPower3;

extern CArray <RECT_DIA, RECT_DIA> Dia_Array; // diaphragms array
extern CArray<double, double> SumReiPowerX; // array for Reion sum power deposited within StepX
extern CArray<double, double> SumAtomPowerX; // array for Reion sum power deposited within StepX

extern double SumPowerStepX;// step for sum power
extern CArray<double, double> SumPowerAngle; // array for sum power deposited within Angle
extern double SumPowerAngleStep;// degrees, step for angular profile

extern logstream logout;

// CTracer
CTracer::CTracer() 
: 
tattr(NULL),
exitarr(NULL),
fattr(NULL),
m_pDoc(NULL), 
m_pViewWnd(NULL), 
m_Continue(FALSE), 
m_Min(-1), m_Max(-1),
m_Pos(C3Point(-1,0,0)),
m_V(C3Point(1,0,0)),
m_iSource(-1),
m_iRay(-1),
m_Step(0.5),
m_Dead(FALSE),
m_Stopped(FALSE),
m_Transformed(FALSE),
m_OptWriteFall(TRUE),
m_Charge(0),
m_Power(0),
m_StartPower(0),
m_State(SRCION),
m_Mode(THIN),
m_Draw(TRUE),
m_Falls(0),
m_Run(0),
m_AtomPower1(0),
m_AtomPower2(0),
m_AtomPower3(0)
{
	
}

//CTracer::CTracer(TVectorAttr &p):, vattr(p){}

CTracer::~CTracer()
{
}

// CTracer member functions

void CTracer::SetColor()
{
	switch (m_Charge) {
		case -1: m_Color = RGB(0, 200, 50); break;
		case 0:  m_Color = RGB(0, 0, 255); break;
		case 1: m_Color = RGB(255, 0, 0); break;
		default: m_Color = RGB(0,0,0); 
	}
	//if (m_Power < 1.e-12) m_Color = RGB(200,200,0);
	//if (m_State == SRCION) m_Color = RGB(255,50,0);
}

void CTracer::SetStartPoint(int isource)
{
	CBTRDoc * pDoc = (CBTRDoc*) m_pDoc;
	BEAMLET_ENTRY be = pDoc->BeamEntry_Array.GetAt(isource);
	
	m_Pos.X = 0;
	m_Pos.Y = be.PosY;
	m_Pos.Z = be.PosZ;
	m_iSource = isource;
	m_State = SRCION;
	m_Charge = pDoc->TracePartQ;	
	SetColor();
	m_Stopped = FALSE;
	m_Transformed = FALSE;
	m_Dead = FALSE;
	m_Falls = 0;
	
	m_NL = 0; //Sum (GetPressure(x,z) * dL; // dL = *NeutrStepL
	m_NeutrPos = pDoc->NeutrXmin;
	
	/*	CString S;
		S.Format("%d source pos %g/%g/%g",m_iSource, m_Pos.X, m_Pos.Y, m_Pos.Z);
		AfxMessageBox(S);
	*/
		
	//Randomize();

}
void CTracer::Randomize()
{
	double step = m_Step;
	double Vabs = ModVect(m_V);
	double val = (double)rand() / (double) RAND_MAX;
	double dx = step * val * m_V.X / Vabs; 
	double dy = step * val * m_V.Y / Vabs; 
	double dz = step * val * m_V.Z / Vabs; 
	m_Pos.X += dx; 
	m_Pos.Y += dy; 
	m_Pos.Z += dz;
	
}

double CTracer:: GetVstart()
{
	CBTRDoc * pDoc = (CBTRDoc*) m_pDoc;
	double E = pDoc->IonBeamEnergy * 1000000;
	double Mass = pDoc->TracePartMass;
	double V = sqrt(2.* E * Qe / Mass);
	return V; // {m/c}
}

void CTracer::SetRaysAttr(CArray<ATTRIBUTES, ATTRIBUTES> * pRattr) 
// must be called before SetStartVP!!!
{
	pAttr = pRattr;
}

void CTracer::SetStartVP(int iray) //  called from InitTracer!!
{
	CBTRDoc * pDoc = (CBTRDoc*) m_pDoc;
	BEAMLET_ENTRY be = pDoc->BeamEntry_Array.GetAt(m_iSource);
	//pAttr = &(pDoc->Attr_Array);
	ATTRIBUTES at = pAttr->GetAt(iray);

	if (at.Current < 1.e-12 && iray >0) { m_Dead = TRUE; return;}

	//m_Dead = FALSE;
	//m_Transformed = FALSE;
		
	pDoc->RotateVert(be.AlfZ, at.Vx, at.Vy, at.Vz); 
	pDoc->RotateHor(be.AlfY, at.Vx, at.Vy, at.Vz);

	double V = GetVstart();

	m_V.X = at.Vx * V;
	m_V.Y = at.Vy * V;
	m_V.Z = at.Vz * V;

	double dy = at.Y;
	double dz = at.Z;
	double focus = pDoc->BeamFocusX; // initiual radius of a beamlet at GG due to "focusing" - not used now >> 0 
	if (fabs(focus) < 100) {
		dy = - focus * at.Vy / at.Vx; //=0
		dz = - focus * at.Vz / at.Vx; //=0
	}
	
	//m_Pos.X = 0; 
	m_Pos.Y += dy; 
	m_Pos.Z += dz;

	double BeamPower = pDoc->IonBeamPower;
	int	Ntotal = pDoc->NofBeamletsTotal; //BeamEntry_Array.GetSize();
	double fraction = be.Fraction;
	
	double  BeamletPower = BeamPower * 1000000. / Ntotal ; //W per beamlet 
	m_StartPower = at.Current  * BeamletPower * fraction;//W 
	m_Power = m_StartPower;
	m_Step = pDoc->TraceStepL;

	m_iRay = iray;
	SetColor();
	
	//SATTR tat(m_Pos, m_Power, m_Charge, m_Color, SOURCE);
	//tattr->push_back(tat);

	/*C3Point P(-1, 0,0);
	SetNeutrPoint(P);
	SetReionPoint(P);*/

	m_NL = 0; //Sum (GetPressure(x,z) * dL; // dL = *NeutrStepL
//	m_Dead = FALSE;
//	m_Transformed = FALSE;
	
	CString S;
	S.Format("%d state(2000) %d source %d ray transformed %d\n power %le",
			m_State, m_iSource, m_iRay, m_Transformed, m_Power);
	//AfxMessageBox(S);
}

void CTracer::SetNeutrPoint(C3Point P) // keep parent pos/V/power
{
	CBTRDoc * pDoc = (CBTRDoc*) m_pDoc;
	double dL = 0;
	double Dens = 0;
	// double Xmin = pDoc->NeutrXmin;
	// double Xmax = pDoc->NeutrXmax;
	 
	dL =  pDoc->NeutrStepL;// GetDistBetween(m_NeutrPos, P); //
	Dens = (pDoc->GetPressure(P));
		
	m_NeutrPos = P;//m_Pos;
	m_NeutrV = m_V;
	m_NeutrPower = m_Power; //src ion power
	
	m_NL += Dens * dL;
	
}

void CTracer::GetNeutrPoint()
{
	m_Pos = m_NeutrPos;
	m_V = m_NeutrV;
	m_Power = m_NeutrPower;//src ion power
}

int CTracer:: GetNeutrPointNumber() // for 1D gas profile
{
	CBTRDoc * pDoc = (CBTRDoc*) m_pDoc;
	int res = -1;
	int N =  pDoc->NeutrArray.GetUpperBound();
	if (N<1) return -1;
	for (int i = 0; i < N; i++) {
		if (Between(pDoc->NeutrArray[i].X, m_Pos.X, pDoc->NeutrArray[i+1].X)) res = i;
	}
	return (res);
}

void CTracer::SetReionPoint(C3Point P) // keep parent pos/V/power
{
	m_ReionPos = P;//m_Pos;
	m_ReionV = m_V;
	m_ReionPower = m_Power;
}

void CTracer::GetReionPoint()
{
	m_Pos = m_ReionPos;
	m_V = m_ReionV;
	m_Power = m_ReionPower;
}

int CTracer:: GetReionPointNumber() // for 1D gas profile
{
	CBTRDoc * pDoc = (CBTRDoc*) m_pDoc;
	int res = -1;
	int N =  pDoc->ReionArray.GetUpperBound();
	if (N<1) return -1;
	for (int i = 0; i < N; i++) {
		if (Between(pDoc->ReionArray[i].X, m_Pos.X, pDoc->ReionArray[i+1].X)) res = i;
	}
	return (res);
}
void CTracer:: SetPosIonState()// secondary ions H+/D+ after neutralisation
{
	if (m_Dead) return;
	if (m_Pos.X < 0) {
		m_Stopped = FALSE;	m_Dead = TRUE;	return;
	}
	CBTRDoc * pDoc = (CBTRDoc*) m_pDoc;
	m_State = POSION; 
	m_Charge = 1; 
	m_Step = pDoc->IonStepL;

	//if (pDoc->OptNeutrStop) { m_Stopped = TRUE; return; }
	
	if (pDoc->TracePartQ > 0) {// H+/D+ - source ions, no other positive ions generated 
		//m_Stopped = TRUE; 
		//return;
		m_Power = 0;
	}

	else { // H-/D-

//////// H-/D- - source ions
	double PosIonFr;
	double NegCurr, PosCurr, NeutrCurr; //j-, j+, j0
	double A, B, C, D;
	double S01, S10, S_10, S_11;
	double Dens =  pDoc->GetPressure(m_Pos);
	double dL = pDoc->NeutrStepL;
	double NL = m_NL - Dens * dL * 0.5;

	if (m_Mode == THIN) { // THIN
		PosIonFr = pDoc->PosIonPart;
		m_Power = m_StartPower * PosIonFr;
		if (m_Power < 1.e-6) m_Stopped = TRUE;
		if (pDoc->OptNeutrStop) m_Stopped = TRUE;
	}

	else { // THICK
		/*S01 = pDoc->ReionSigma;
		S10 = pDoc->PosExchSigma;
		S_10 = pDoc->NeutrSigma;
		S_11 = pDoc->TwoStripSigma;
		A = S01 + S10; // H-/D-
		B = S_11 - S01; // H-/D-
		C = S01; // H-/D-
		D = S_10 + S_11;// H-/D-
				
		if (NL < 1.e-30 || fabs(D) < 1.e-30) NegCurr = 1; 
		else  NegCurr = exp(-D * NL);
		if (NL < 1.e-30 || fabs(A) < 1.e-30 || fabs(D) < 1.e-30 || fabs(A-D) < 1.e-30) PosCurr = 0;
		else  PosCurr = C/A - (C/A + B/(A-D)) * exp(-A * NL) + B/(A-D) * exp(-D * NL);
		if (PosCurr < 0) PosCurr = 0;
		NeutrCurr = 1 - NegCurr - PosCurr;

		PosIonFr = (NeutrCurr * S01 - PosCurr * S10 + NegCurr * S_11) * dL * Dens; // (dj+/dl)*dL
		*/
		C3Point Rate = pDoc->GetCurrentRate(m_Pos.X, NL, 1);
		PosIonFr = Rate.X;
		m_Power = m_StartPower * PosIonFr;//m_StartPower 
		if (m_Power < 1.e-6) m_Stopped = TRUE;
		/*int i = GetNeutrPointNumber();
		if (i > 0) {
			PosIonFr = pDoc->NeutrArray[i].Z - pDoc->NeutrArray[i-1].Z;
			m_Power = m_StartPower * PosIonFr;
		}
		else { m_Power = 0; m_Stopped = TRUE; return; }*/
	} //THICK
	} // H-/D-

	SetColor();
	
	if (pDoc->OptNeutrStop) m_Stopped = TRUE;
	//else m_Stopped = FALSE;

	if (m_Power < 1.e-6) m_Stopped = TRUE;
	
}


void CTracer:: SetReionState()// reionized positive ion 
{
	if (m_Dead) return;
	if (m_Pos.X < 0) return;	//{ m_Stopped = FALSE;	m_Dead = TRUE;	return;	}

	CBTRDoc * pDoc = (CBTRDoc*) m_pDoc;
	if (pDoc->OptReionStop) { m_Stopped = TRUE; return; }

	m_State = REION; 
	m_Charge = 1; 
	m_Step = pDoc->GetIonStep(m_Pos.X); //*pDoc->IonStepL;
	SetColor();
	
	double Dens = pDoc->GetPressure(m_Pos);
	double Sigma = pDoc->ReionSigma;// source ion H+/D+
		
	double dL; // = m_Step;

	if (m_Pos.X >= pDoc->RXspec0 && m_Pos.X <= pDoc->RXspec1)
		dL = pDoc->ReionStepLspec;//special area for AP
	else dL = pDoc->ReionStepL;
		
	double ReionFr = Dens * Sigma * dL; 

	if (pDoc->OptAddPressure) {
		double ADens = pDoc->AddGField->GetPressure(m_Pos.X);
		double ASigma = pDoc->AddReionSigma;
		ReionFr = ReionFr + (ADens * ASigma * dL);
	}
	m_Power = m_Power * ReionFr;

	/*int i = GetReionPointNumber();
	if (i > 0) {
		ReIonFr = pDoc->ReionArray[i].Y;
		m_Power = m_Power * ReIonFr;
		if (m_Power < 1.e-12) { m_Stopped = TRUE; return; }
		if (pDoc->OptReionStop) { m_Stopped = TRUE; return; }
	}
	else { m_Power = 0; m_Stopped = TRUE; return;}*/

	m_Stopped = FALSE;
	if (m_Power < 1.e-12) m_Stopped = TRUE;

}
void CTracer:: SetSrcIonState() // remaining after neutralisation
{
	if (m_Dead) return;
	CBTRDoc * pDoc = (CBTRDoc*) m_pDoc;
	if (!pDoc->OptThickNeutralization && pDoc->OptNeutrStop) { m_Dead = TRUE; return;}
	if (pDoc->OptThickNeutralization && pDoc->OptNeutrStop && m_Pos.X > pDoc->NeutrXmax) 
	{ m_Dead = TRUE; return;}
	
	if (m_Pos.X < 0) { 	m_Stopped = FALSE;	m_Dead = TRUE;	return;	}
		
	m_State = SRCION; 
	m_Charge = pDoc->TracePartQ;//; 
	m_Step = pDoc->IonStepL; //NeutrStepL;

	double NeutrFr, PosIonFr, NegIonFr;// gone away from primary ion at a neutr-point (dj/dl*step)
	double NegCurr, PosCurr, NeutrCurr; //j-, j+, j0 - total (remained) current
	if (m_Mode == THIN) { // THIN
		NeutrFr = pDoc->NeutrPart;
		PosIonFr = pDoc->PosIonPart;
		NegCurr = 1.0 - PosIonFr - NeutrFr;
		if (m_Charge < 0) m_Power = m_StartPower * NegCurr;// H-/D-
		else m_Power = m_StartPower * (1.0 - NeutrFr); // H+/D+ NegFr = 0 always
	}
	else { // THICK
		
		double A, B, C, D;
		double S01, S10, S_10, S_11;
		double Dens =  pDoc->GetPressure(m_Pos);
		double dL = pDoc->NeutrStepL;
		double NL = m_NL - Dens * dL * 0.5;
		S01 = pDoc->ReionSigma;
		S10 = pDoc->PosExchSigma;
		S_10 = pDoc->NeutrSigma;
		S_11 = pDoc->TwoStripSigma;

	if (m_Charge < 0) { // H-/D-
	/*	A = S01 + S10; // H-/D-
		B = S_11 - S01; // H-/D-
		C = S01; // H-/D-
		D = S_10 + S_11;// H-/D-
		if (NL < 1.e-30 || fabs(D) < 1.e-30) NegCurr = 1; 
		else  NegCurr = exp(-D * NL);//H-/D- */
		C3Point Curr = pDoc->GetCurrentRate(m_Pos.X, NL, 0);
		NegCurr = Curr.Y;
		m_Power = m_StartPower * NegCurr; //m_StartPower 
	} //// H-/D-
	
	else {//// H+/D+
		A = S10;  
		B = S01;	
		C = A + B;
		if (NL < 1.e-30 || fabs(C) < 1.e-30) PosCurr = 1;
		else PosCurr = B/C + (1 - B/C) * exp(-C * NL); // H+/D+
		m_Power = m_StartPower * PosCurr;
	} //// H+/D+

		/*int i = GetNeutrPointNumber();
		if (i >= 0) {
			//m_Step = *pDoc->NeutrStepL;
			NegIonFr = pDoc->NeutrArray[i+1].Y;//0.5 * (pDoc->NeutrArray[i-1].Y + pDoc->NeutrArray[i].Y);
			PosIonFr = pDoc->NeutrArray[i+1].Z;
			if (m_Charge < 0) m_Power = m_StartPower * NegIonFr; // H-/D-
			else m_Power = m_StartPower * PosIonFr; // H+/D+ NegFr = 0 always
		}*/

	}//THICK

	SetColor();
	m_Stopped = FALSE;
	if (m_Power < 1.e-12) m_Dead = TRUE;

}

void CTracer:: SetSingleParticle(int state, C3Point pos, C3Point v, double step, double energy)
{
	CBTRDoc * pDoc = (CBTRDoc*) m_pDoc;
	m_State = state;
	double Mass(0), Vtot(0), Vmod(1);
	
	m_Power = 1;
	m_Pos = pos;
	//m_V = v;
	m_Stopped = FALSE;	
	m_Dead = FALSE;
	m_Step = step;
	/*switch (state) {
		case ATOM: m_Charge = 0; m_Step = pDoc->ReionStepL; break;
		case POSION: m_Charge = 1; m_Step = pDoc->IonStepL; break;
		case ELECTRON: m_Charge = -1; m_Step = 0.001; break;
	}
	SetColor();*/
	switch (state) {
		case ATOM: Mass = pDoc->TracePartMass; Vtot = sqrt(2.* energy * Qe / Mass); break;
		case POSION: Mass = pDoc->TracePartMass; Vtot = sqrt(2.* energy * Qe / Mass); break;
		case ELECTRON: Mass = Me; Vtot = sqrt(2.* energy * Qe / Mass); break;
		case RELECTRON:  
			double EeV = energy; //Me * Vabs * Vabs * 0.5 / Qe;
			double EMeV = EeV * 1.e-6;
			//RelMass = (1.+ EMeV/0.511);
			Mass = Me * RelMass(EMeV);
			Vtot = RelV(EMeV);
			break;
	}

	Vmod = ModVect(v);
	
	m_V.X = Vtot * v.X / Vmod;
	m_V.Y = Vtot * v.Y / Vmod;
	m_V.Z = Vtot * v.Z / Vmod;

	CString S;
	S.Format("Mass = %g EeV = %g \nVx = %g, Vy = %g, Vz = %g", 
		Mass, energy, m_V.X, m_V.Y, m_V.Z);
	//AfxMessageBox(S);
	
}

void CTracer:: SetAtomState(int from)
{
	if (m_Dead) return;
	CBTRDoc * pDoc = (CBTRDoc*) m_pDoc;
	if (m_Pos.X < 0) { // no parent
		m_Stopped = FALSE;	m_Dead = TRUE;	return;
	}
	m_State = ATOM; 
	m_Charge = 0; 

	if (!pDoc->OptTraceAtoms) {  m_Stopped = TRUE; return; } // to be converted next

	double  NegCurr, PosCurr, NeutrCurr, ReionFr, NeutrFr, dL;
	double Dens = pDoc->GetPressure(m_Pos);
	dL = pDoc->NeutrStepL;//m_Step;
	double NL = m_NL - Dens * dL * 0.5;
	

	if (from == POSION) { // from pos ion state in neutr-point

	if (m_Mode == THIN) { // THIN
		NeutrFr = pDoc->NeutrPart;
		m_Power = m_StartPower * NeutrFr;
	}

	else { // THICK
		double A, B, C, D;
		double S01, S10, S_10, S_11;
		
		S01 = pDoc->ReionSigma;
		S10 = pDoc->PosExchSigma;
		S_10 = pDoc->NeutrSigma;
		S_11 = pDoc->TwoStripSigma;
		
	if (pDoc->TracePartQ < 0) { // H-/D-
	/*	A = S01 + S10; // H-/D-
		B = S_11 - S01; // H-/D-
		C = S01; // H-/D-
		D = S_10 + S_11;// H-/D-
		if (NL < 1.e-30 || fabs(D) < 1.e-30) NegCurr = 1; 
		else  NegCurr = exp(-D * NL);
		if (NL < 1.e-30 || fabs(A) < 1.e-30 || fabs(D) < 1.e-30 || fabs(A-D) < 1.e-30) PosCurr = 0;
		else  PosCurr = C/A - (C/A + B/(A-D)) * exp(-A * NL) + B/(A-D) * exp(-D * NL);
		if (PosCurr < 0 ) PosCurr = 0;
		NeutrCurr = 1 - NegCurr - PosCurr;
		//PosIonFr = (NeutrCurr * S01 - PosCurr * S10 + NegCurr * S_11) * dL * Dens; // (dj+/dl)*dL
		//NeutrFr = (NegCurr * S_10 - NeutrCurr * S01 + PosCurr * S10) * dL * Dens;  // (djo/dl)*dL
		NeutrFr = (NegCurr * S_10 + PosCurr * S10) * dL * Dens;// not re-ionised
		if (NeutrFr < 0) NeutrFr = 0;*/
		C3Point Rate = pDoc->GetCurrentRate(m_Pos.X, NL, 1);
		NeutrFr = Rate.Z;
		m_Power = m_StartPower * NeutrFr;// m_StartPower
	} //// H-/D-
	
	else {//// H+/D+
		A = S10;  
		B = S01;	
		C = A + B;
		if (NL < 1.e-30 || fabs(C) < 1.e-30) PosCurr = 1;
		else PosCurr = B/C + (1 - B/C) * exp(-C * NL); // H+/D+
		NeutrCurr = 1 - PosCurr;
		NeutrFr = (PosCurr * S10 - NeutrCurr * S01) * dL * Dens; 
		m_Power = m_StartPower * NeutrFr;
		
	} //// H+/D+

		/*int i = GetNeutrPointNumber(); // 1D pressure
		if (i > 0) {
			PosIonFr = pDoc->NeutrArray[i+1].Z - pDoc->NeutrArray[i].Z; // < 0 if source H+/D+
			NegDrop = pDoc->NeutrArray[i-1].Y - pDoc->NeutrArray[i].Y; // = 0 if source H+/D+
			NeutrFr = NegDrop - PosIonFr; // > 0
			m_Power = m_StartPower * NeutrFr;
		}
		else { m_Power = 0; m_Stopped = TRUE; return; }*/

	} // THICK

	}// after POSION

	else { // from = REION
		double Sigma = pDoc->ReionSigma;// source ion H+/D+				
		if (m_Pos.X >= pDoc->RXspec0 && m_Pos.X <= pDoc->RXspec1) 
			dL = pDoc->ReionStepLspec;//special area for AP
		else dL = pDoc->ReionStepL;
		
		ReionFr = Dens * Sigma * dL; 
		if (pDoc->OptAddPressure) {
			double ADens = pDoc->AddGField->GetPressure(m_Pos.X);
			double ASigma = pDoc->AddReionSigma;
			ReionFr += ADens * ASigma * dL;
		}
		m_Power = m_Power * (1 - ReionFr);
	
		/*	i = GetReionPointNumber(); // 1D pressure
		if (i > 0) {
			ReionFr = pDoc->ReionArray[i].Y;
			m_Power = m_Power * (1 - ReionFr);
			
		} */
	
	
	}// from = REION
	
	if (m_Pos.X >= pDoc->RXspec0 && m_Pos.X <= pDoc->RXspec1) 
			m_Step = pDoc->ReionStepLspec;//special area for AP
	else m_Step = pDoc->ReionStepL;

	SetColor();
	m_Stopped = FALSE;
	if (m_Power < 1.e-12) m_Stopped = TRUE;
		
}
void CTracer::SetNextState() // if stopped -> set next particle
{
	if (!m_Continue) return;
	if (m_Dead) return; // if dead -> emit next source particle
	
	int state = m_State; 
	//int n;// 0 - neutralization, 1 - reionization 

	switch (state) {
		case SRCION:  m_Dead = TRUE;  m_Transformed = FALSE; break;
		case POSION: GetNeutrPoint(); SetAtomState(POSION); m_Transformed = TRUE; break;//Randomize();
		case ATOM:   GetNeutrPoint(); SetSrcIonState(); m_Transformed = TRUE;  break;
		case REION:  GetReionPoint(); SetAtomState(REION);  break;
		default: break;
	}

	SetColor();
	m_Falls = 0;
}

void CTracer:: CheckNeutrBound(C3Point P1, C3Point P2)
{
	CBTRDoc * pDoc = (CBTRDoc*) m_pDoc;
	double NeutrX = pDoc->NeutrXmax; 
	if (Between(P1.X, NeutrX, P2.X)) {
		C3Point Pend(NeutrX, 0, 0);
		double decay = pDoc->GetNeutrDecay(m_NeutrPos, Pend);
		m_Power *= decay; 
	}
}


void CTracer::CheckNeutrState(C3Point P1, C3Point P2) // check SRCION for neutralization
{
	if (!m_Continue) return;
	if (m_Stopped || m_Dead) return;
	if (m_State != SRCION) return;
	CBTRDoc * pDoc = (CBTRDoc*) m_pDoc;
	m_Transformed = FALSE;

	if (!pDoc->OptThickNeutralization) { // THIN
		m_Mode = THIN;
		double NeutrX = pDoc->NeutrXmax; //thin neutralization
		if (Between(P1.X, NeutrX, P2.X)) {
			//if (m_iSource == 0) AfxMessageBox("SetNeutrPoint for source 0");
			//if (m_iSource == 1) AfxMessageBox("SetNeutrPoint for source 1");
			//C3Point Pcross = P1 + (P2-P1) * (NeutrX - P1.X) / (P2.X - P1.X); 
			//SetNeutrPoint(Pcross); // keep pos/V/power
			SetNeutrPoint(P2);	
			m_Pos = P2;	
			SetPosIonState();
			//m_Pos = Pcross;
			m_Transformed = TRUE;
		}
		else { m_Pos = P2; return;}
	}
	else { // THICK
		m_Mode = THICK;
		double Xmin = pDoc->NeutrXmin;
		double Xmax = pDoc->NeutrXmax;

		C3Point Mid = P2;//(P1 + P2) * 0.5;
		if (Between(Xmin, Mid.X, Xmax)) { // within the area of neutr-n 
			
			/*for (int i = 0; i < pDoc->NeutrArray.GetSize(); i++) {
			double NeutrX = pDoc->NeutrArray[i].X;
			if (Between(P1.X, NeutrX, P2.X)) {*/
			//C3Point Pcross = P1 + (P2-P1) * (NeutrX - P1.X) / (P2.X - P1.X); 
			SetNeutrPoint(P2);	
			m_Pos = P2;	
			SetPosIonState();
			m_Transformed = TRUE;
		} // if		//} // for
		else { 
			m_Pos = P2;
			if (pDoc->OptThickNeutralization && pDoc->OptNeutrStop && m_Pos.X > Xmax) 
				m_Dead = TRUE;
			return;
		}

		//m_Step = *pDoc->IonStepL; // not neutralised
		//m_Pos = P2;
	} //THICK
	//SetColor();
}

BOOL CTracer::CheckReionState(C3Point P1, C3Point P2) // check ATOM for reionization
{
	if (!m_Continue) return 0;
	if (m_Stopped || m_Dead) return 0;
	if (m_State != ATOM) return 0;
	CBTRDoc * pDoc = (CBTRDoc*) m_pDoc;
	if (!pDoc->PressureLoaded) return 0;

	double Xmin = pDoc->ReionXmin;// + *pDoc->ReionStepL;
	double Xmax = pDoc->ReionXmax;
		
	C3Point Mid = (P1 + P2) * 0.5;
	if (Between(Xmin, Mid.X, Xmax)) { // within the area of reion-n
		SetReionPoint(P2);//Mid
		SetReionState();
		//SetColor();
		m_Pos = P2;//Mid;
		return 1;
	}
	return 0;
}

int CTracer::CheckFall(C3Point P1, C3Point & P2)
{
	if (!m_Continue) return -1;
	// int n = 0, nadd = 0;
	// CBTRDoc * pDoc = (CBTRDoc*) m_pDoc;

	//if (pDoc->AddPlatesNumber > 0) nadd = GetFallAdd(P1, P2);

	//if (IsVolumeEmpty(P2)) return nadd;
	
	// n = GetFall(P1, P2);	// fall on Solid -> Dead = TRUE, n = plate Number
	return GetFall(P1, P2); // n + nadd;
}

BOOL CTracer:: IsVolumeEmpty(C3Point P)
{
	CBTRDoc * pDoc = (CBTRDoc*) m_pDoc;
	if (pDoc->OptFree) return TRUE;
	std::vector<VOLUME> & VolVect = pDoc->VolumeVector;
	VOLUME vol;
	double x0, x1, y0min, y0max, z0min, z0max, y1min, y1max, z1min, z1max, ymin, ymax, zmin, zmax;
	//if (P.X < *(pDoc->NeutrInX) - 0.2) return TRUE;
	//if (P.X > *pDoc->AreaLong) return TRUE;
	for (int i = 0; i < (int)VolVect.size(); i++) {
		vol = VolVect[i];
		x0 = vol.lb0.X; x1 = vol.lb1.X;
		if (fabs(x1-x0) < 1.e-3) continue;
		y0min = vol.lb0.Y; y1min = vol.lb1.Y; 
		y0max = vol.ru0.Y; y1max = vol.ru1.Y;
		z0min = vol.lb0.Z; z1min = vol.lb1.Z; 
		z0max = vol.ru0.Z; z1max = vol.ru1.Z;

		if ((P.X - x0) * (x1 - P.X) > 1.e-6) {
			ymin = y0min + (P.X - x0) * (y1min - y0min) / (x1 - x0);
			ymax = y0max + (P.X - x0) * (y1max - y0max) / (x1 - x0);
			if ((P.Y - ymin) * (ymax - P.Y) > 1.e-6) {
				zmin = z0min + (P.X - x0) * (z1min - z0min) / (x1 - x0);
				zmax = z0max + (P.X - x0) * (z1max - z0max) / (x1 - x0);
				if ((P.Z - zmin) * (zmax - P.Z) > 1.e-6) return TRUE;
			} //if y
		} // if x
	} // i

/*	RECT_DIA dia0, dia1;
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
*/
	return FALSE;
}

void CTracer::Calculate_V()
{
	CBTRDoc * pDoc = (CBTRDoc*) m_pDoc;
	
	C3Point E = pDoc->RIDField->GetE(m_Pos.X, m_Pos.Y, m_Pos.Z);
	E.Z = 0; //!!!!!!!!!!! = U
	C3Point B = C3Point(0,0,0);
	if (pDoc->FieldLoaded  && fabs(pDoc->MFcoeff) > 1.e-6) {
		B = pDoc->GetB(m_Pos.X, m_Pos.Y, m_Pos.Z);// MFcoeff is included!  
	}
	double Mass = pDoc->TracePartMass;// atoms and ions
	C3Point V = m_V;
	double Vabs = ModVect(m_V);
	//Vabs = sqrt(2.* EkeV*1000 * Qe / Mass);
	//V = sqrt(2.* E * Qe / Mass);
	
	if (m_State == ELECTRON) Mass = Me;
	if (m_State == RELECTRON) { 
		double EeV = Me * Vabs * Vabs * 0.5 / Qe;
		double EMeV = EeV * 1.e-6;
		//RelMass = (1.+ EMeV/0.511);
		Mass = Me * RelMass(EMeV);
		m_V = RelV(EMeV);
		V = m_V;
	}
		
	C3Point A = (E + VectProd(V,B)) * m_Charge * Qe / Mass; // [m/s2]
	
	m_V = V + A * (m_Step/Vabs);// [m/s]
			
}

C3Point CTracer:: MakeStep()
{
	if (!m_Continue) return m_Pos;
	
	double dl = m_Step;
	if (dl < 1.e-6) dl = 1.e-6;
	if (m_Charge != 0) { // change V
		Calculate_V();
	}
	double Vabs = ModVect(m_V);
	if (Vabs < 1.e-6) {
		m_Stopped = TRUE;
		return m_Pos;
	}
	return (m_Pos +  m_V * (dl/ Vabs));
}

C3Point CTracer::MoveIonToX(double Xlim) // Make ION STEP - no track / no keep falls / no draw
{
	double dLx = Xlim - m_Pos.X; // check if < 0!
	if (fabs(m_V.X) < 1.e-6 || dLx < 1.e-6) {
		m_Stopped = TRUE;
		return m_Pos;
	}
	//m_Step = pDoc->GetIonStep(m_Pos.X);
	//double modV = ModVect(m_V); // const
	//double eps = 1.e-6;//bool metlimit = 0; // solid or area bound

	bool stopped = FALSE;
	double dt, maxlen = Xlim * 2;
	int maxstep = (int)(maxlen / m_Step);
	C3Point P1 = m_Pos;
	C3Point P2;
	
	int i = 0;
	while (!stopped && i < maxstep) {
		m_Pos = P1;
		//Track.Add(m_Pos);
		P2 = MakeStep();//CalcV, P2 = P1 + ortV * dist;
		if (m_Stopped) break;
		int n = GetFall(P1, P2);// check for crossing SOLID 
								//GetFalls(P1, P2, metlimit);// Last(solid)>>P2
		if (n >= 0) {// met solid
			m_Stopped = TRUE;
			stopped = TRUE;
		}// interrupted (pDoc->STOP)

		if (P2.X >= Xlim) {
			dLx = Xlim - P1.X;
			dt = dLx / m_V.X;// can be <0
			P2 = P1 + m_V * dt; // stop at Xlimit
			stopped = TRUE;
			break;
		}
		P1 = P2;
		i++;
	}
	m_Pos = P2;
	return m_Pos;
	//double dt = dLx / m_V.X;// can be <0
	//return (m_Pos + m_V * dt);
}

C3Point CTracer:: MakeStepToX(double Xlim) // Make ATOM STEP - straight line
{
	//if (!m_Continue) return m_Pos;
	double dLx = Xlim - m_Pos.X; // can be <0
	if (fabs(m_V.X) < 1.e-6 || fabs(dLx) < 1.e-6) {
		m_Stopped = TRUE;
		return m_Pos;
	}
	double dt = dLx / m_V.X;// can be <0
	C3Point P1 = m_Pos;
	C3Point P2 = m_Pos + m_V * dt;
	int n = GetFall(P1, P2);// check for crossing SOLID 
							//GetFalls(P1, P2, metlimit);// Last(solid)>>P2
	if (n >= 0) // met solid
		m_Stopped = TRUE;
		
	return P2;
}

C3Point CTracer::MakeStepL(double dl)
{
	//if (!m_Continue) return m_Pos;
	double dL = dl; // can be < 0
	double Vabs = ModVect(m_V);
	if (Vabs < 1.e-6 || fabs(dL) < 1.e-6) {
		m_Stopped = TRUE;
		return m_Pos;
	}
	double dt = dL / Vabs;// can be <0
	return (m_Pos + m_V * dt);
}


void CTracer::DoNextStep() // old (before 4.5)
{
	CBTRDoc * pDoc = (CBTRDoc*)m_pDoc;
	CString Slog;
	if (!m_Continue) {
		DumpArrays();
		return;
	}
		
	if (m_Max < 0) { // 
		switch (m_State) {
			case ATOM:	TraceSingleAtom(); break;// 
			case POSION: TraceSinglePosIon(); break;
			case ELECTRON: TraceSingleElectron(); break;
			case RELECTRON: TraceSingleRelectron(); break;
		}
		//TraceAtomFast(); 
		return; 
	}

	
	//BEAMLET_ENTRY be = pDoc->BeamEntry_Array.GetAt(m_iSource);
	int Nattr = pDoc->Attr_Array.GetUpperBound();

	C3Point P, Pnext, Pold;
	
	//if (m_State == SRCION && m_iSource < 2) m_Dead = TRUE;
	if (m_State == SRCION && m_Dead) {  goto label0; }
	if (m_State == ATOM && pDoc->OptReionStop) {
		TraceAtomFast();
		goto label0;
	} //<<<<<<<<<<<<< Fast tracing of Atoms

	if (m_State == REION) m_Step = pDoc->GetIonStep(m_Pos.X); //set var step along X
	
	Pnext = MakeStep(); //m_Pos +  m_V * (dl/ Vabs);
	Pold = m_Pos;

	// fall on surfaces 
	P = Pnext;
	int n = CheckFall(Pold, P); // no fall -> n = -1	// if n>0, P->stoppoint // if n<0 -> P = Pnext
		
	// transformations
	if (m_State == SRCION) {
		if (!m_Transformed) { 
			if (GetDistBetween(P,Pnext) > 1.e-6) Pold = P;
			CheckNeutrState(Pold, Pnext); 
			if (m_Transformed && !m_Stopped) { m_Pos = Pnext; return; }
		} 
		if (m_Stopped) {m_Pos = Pnext; SetNextState(); return;}	
		
	} // source Ion

	if (m_State == ATOM) { // not fast, reions are traced
		if (pDoc->OptThickNeutralization) CheckNeutrBound(Pold, P);// to check!!!
		if (n>0) Pold = P;
		if (CheckReionState(Pold, Pnext)) {
			return;  // reion emitted at midpoint
		}
	}

	m_Pos = Pnext;

	if (m_State == REION && n < 0) m_Pos = P; // refine near surf

		
label0:

	if (m_Stopped) 	SetNextState();	
	// if dead -> emit next source particle

	/*if (!m_Continue) {
		DumpArrays();
		return;
	}*/
	
	bool lastbml = FALSE;

	if (m_Dead) { // source ion particle finished
		if (m_iRay < Nattr) { // next ray
			m_iRay++;
			SetStartPoint(m_iSource);
			SetRaysAttr(&(pDoc->Attr_Array));// 4.5 uses only main  Attr array
			SetStartVP(m_iRay);
		}
		else { // m_iRay >= Nattr -> next Src
			
			if (m_iSource < m_Max && m_Max > 0) lastbml = FALSE;// next source and tot sources > 1
			else lastbml = TRUE;
			
			if (!lastbml) { // not last src
				if (!m_Draw) ShowBeamlet(m_iSource);
				
				//Slog.Format("<%d> doing Src %4d Bml %4d\n", m_ID, m_iSource + 1, NofCalculated + 1);
				//logarr->push_back(Slog);
				m_iSource++;
				
				cs.Lock();

				pDoc->AddFalls(m_ID, m_iSource, tattr); // 
				// add falls after each bml, add line to log: "done"
				//pDoc->AddLog(logarr);
				NofCalculated++;

				pDoc->ShowStatus();
				::GdiFlush();

				cs.Unlock();
				
				m_iRay = 0;
				SetStartPoint(m_iSource);
				SetRaysAttr(&(pDoc->Attr_Array)); // 4.5!
				SetStartVP(m_iRay);
				
				ClearArrays(); // renew for each bml
				
			}
			else { // last : m_iSource >= m_Max // last source
				SetContinue(FALSE);
				CString s;
				s.Format("Thread <%d>:\n Beamlets %d - %d finished", m_ID, m_Min+1, m_Max+1);
				//AfxMessageBox(s); // not shown - to calculate a pure run time
				
				Slog.Format("<%d>  Src %4d Bml %4d >>>>>>>>>>> THREAD FINISHED\n", m_ID, m_iSource+1, NofCalculated + 1);
				//logarr->push_back(Slog);
				m_iSource++;

				cs.Lock();

				pDoc->AddFalls(m_ID, m_iSource, tattr); // add falls after the last BML in a tracer
				//pDoc->AddLog(logarr);
				NofCalculated++;

				if (NofCalculated >= pDoc->NofBeamlets) {
					pDoc->STOP = TRUE;
					pDoc->StopTime = CTime::GetCurrentTime();	
					//NofCalculated = 0;
				}
				pDoc->ShowStatus();
				::GdiFlush();

				cs.Unlock();
				
			}
		}// m_iRay >= Nattr
	}// source ion particle finished

	if (m_State == SRCION && pDoc->OptThickNeutralization) m_Transformed = FALSE;
	
	if (!m_Continue && !lastbml) DumpArrays();
	
}

void CTracer:: TraceSingleAtom() // DoNextStep - one track
				// with or without reionisation
{
	if (!m_Continue) return;
	m_Charge = 0;
	SetColor();
	CBTRDoc * pDoc = (CBTRDoc*) m_pDoc;
	C3Point P, Pnext, Pold;

	//if (m_State == ATOM && pDoc->OptReionStop)  TraceAtomFast(); // no reionization

	//else { // emit reions 

		Pnext = MakeStep(); //m_Pos +  m_V * (dl/ Vabs);
		Pold = m_Pos;
		// fall on surfaces 
		P = Pnext;
		int n = CheckFall(Pold, P); // no fall -> n = -1	// if n>0, P->stoppoint // if n<0 -> P = Pnext
		
		if (m_State == ATOM) { 
			if (n>0) Pold = P;
			//if (CheckReionState(Pold, Pnext)) return;  // reion emitted at midpoint
		}
		
	m_Pos = Pnext;

	//if (m_State == REION && n < 0) m_Pos = P; // refine near surf

	if (m_State == REION && m_Stopped) 	{ //SetNextState();	 
		GetReionPoint(); SetAtomState(REION); 
		m_Stopped  = FALSE; SetColor();
		return;
	}

 //} // trace  atoms - slow
	
	if (m_State == ATOM && m_Stopped) {
	SetContinue(FALSE);
	CString s;
	s.Format("0 finished");
	//AfxMessageBox(s);
	pDoc->STOP = TRUE;
	pDoc->ShowStatus();
	}
	
}

void CTracer:: TraceSinglePosIon() // DoNextStep - one track
{
	if (!m_Continue) return;
	m_Charge = 1;
	SetColor();
	CBTRDoc * pDoc = (CBTRDoc*) m_pDoc;
	C3Point P, Pnext, Pold;

	Pnext = MakeStep(); //m_Pos +  m_V * (dl/ Vabs);
	Pold = m_Pos;
	// fall on surfaces 
	P = Pnext;
	int n = CheckFall(Pold, P); // no fall -> n = -1	// if n>0, P->stoppoint // if n<0 -> P = Pnext
			
	m_Pos = Pnext;

	if (m_Stopped) {
		SetContinue(FALSE);
		CString s;
		s.Format("+ finished");
		//AfxMessageBox(s);
		pDoc->STOP = TRUE;
		pDoc->ShowStatus();
	}
}

void CTracer:: TraceSingleElectron() // DoNextStep - one track
{
	if (!m_Continue) return;
	m_Charge = -1;
	SetColor();
	CBTRDoc * pDoc = (CBTRDoc*) m_pDoc;
	C3Point P, Pnext, Pold;

	Pnext = MakeStep(); //m_Pos +  m_V * (dl/ Vabs);
	Pold = m_Pos;
	// fall on surfaces 
	P = Pnext;
	int n = CheckFall(Pold, P); // no fall -> n = -1	// if n>0, P->stoppoint // if n<0 -> P = Pnext
			
	m_Pos = Pnext;

	if (m_Stopped) {
		SetContinue(FALSE);
		CString s;
		s.Format("e finished");
		//AfxMessageBox(s);
		pDoc->STOP = TRUE;
		pDoc->ShowStatus();
	}
}

void CTracer:: TraceSingleRelectron() // DoNextStep - one track
{
	if (!m_Continue) return;
	m_Charge = -1;
	//SetColor();
	m_Color = RGB(0, 200, 100);
	CBTRDoc * pDoc = (CBTRDoc*) m_pDoc;
	C3Point P, Pnext, Pold;

	Pnext = MakeStep(); //m_Pos +  m_V * (dl/ Vabs);
	Pold = m_Pos;
	// fall on surfaces 
	P = Pnext;
	int n = CheckFall(Pold, P); // no fall -> n = -1	// if n>0, P->stoppoint // if n<0 -> P = Pnext
			
	m_Pos = Pnext;

	if (m_Stopped) {
		SetContinue(FALSE);
		CString s;
		s.Format("rele finished");
		//AfxMessageBox(s);
		pDoc->STOP = TRUE;
		pDoc->ShowStatus();
	}
}


void CTracer::ClearArrays()
{
	tattr->clear();
	tattr->resize(0); //shrink_to_fit();
			//logarr->clear();	//logarr->resize(0); //shrink_to_fit();
}

void CTracer::TestMem()
{
	CBTRDoc * pDoc = (CBTRDoc*)m_pDoc;
	if (pDoc->STOP || !m_Continue) {
		pDoc->SetTitle("STOPPED");
		return;
	}
	
	CString S, Slog;
	
	double V = GetVstart();	
	int Nattr = pDoc->Attr_Array.GetSize();
	int Kmax = 5;//"falls"
	//long sz = Nattr*(m_Max + 1 - m_Min) * Kmax + 1; // +1 extra
	long sz = Nattr * Kmax + 1; // size for 1 bml + 1 extra
	
	//long szmax = tattr->max_size();
	//long cap = tattr->capacity();
	//CVectorCleanup tattr_cleanup(tattr);
	
/*	TRY {
		tattr->reserve(sz);
	} CATCH (CMemoryException, e) {
		S.Format("Tests %d - %d \n reserve FAILED (low memory)",	m_Min + 1, m_Max + 1);
		AfxMessageBox(S);
		return;
	} END_CATCH;
	
	TRY{
		logarr->reserve(sz);
	} CATCH(CMemoryException, e) {
		S.Format("Tests %d - %d \n LOG reserve FAILED (low memory)", m_Min + 1, m_Max + 1);
		AfxMessageBox(S);
		log = FALSE;
	} END_CATCH;

*/
	/*try {
		tattr->reserve(sz);
	}
	catch (const std::bad_alloc& e) { //(exception &e) {
		AfxMessageBox(e.what);// (A2CT("Failed to increase track-vector size"));
	}*/
	
	minATTR tat0(C3Point(0, 0, 0), 0, 0, 0, 1, 1, 1); //(-1, -1, 0, 0, 0, 0);

	bool stopped = FALSE;
	bool log = TRUE;// write log

	//-------------- fill the vector ----------------

	for (int is = m_Min; is <= m_Max; is++) { // source beamlet number
		if (pDoc->STOP) break;

		SetStartPoint(is);// m_Pos.X = 0; m_Pos.Y = be.PosY; m_Pos.Z = be.PosZ; m_Charge = pDoc->TracePartQ;
		tat0.Xmm = unsigned short(fabs(m_Pos.Y) * 1000);
		tat0.Ymm = unsigned short(fabs(m_Pos.Z )* 1000);
		tat0.Charge = signed char(m_Charge);
		
		ClearArrays();	//CVectorCleanup tattr_cleanup(tattr);//tattr->clear();
		TRY{ // reserve attributes
			tattr->reserve(sz);
		} CATCH(CMemoryException, e) {
			S.Format("Thread %d\n reserve at %d FAILED (low memory)", m_ID, NofCalculated);
			AfxMessageBox(S);
			stopped = TRUE;
			break;
		} END_CATCH;
		
	/*	TRY{ // reserve log
			logarr->reserve(sz);
		} CATCH(CMemoryException, e) {
			S.Format("Thread %d\n LOG reserve FAILED (low memory)", m_ID);
			AfxMessageBox(S);
			log = FALSE;
		} END_CATCH;*/
		
		for (int jray = 0; jray < Nattr; jray++) {
			if (pDoc->STOP) {
				stopped = TRUE;
				break;
			}

			SetRaysAttr(&(pDoc->Attr_Array));
			SetStartVP(jray);//m_V.X = at.Vx * V;	m_V.Y = at.Vy * V;	m_V.Z = at.Vz * V; m_Power = m_StartPower;
			tat0.PowerW = float (m_Power);
			tat0.AXmrad = short (m_V.Y / V * 1000);
			tat0.AYmrad = short (m_V.Z / V * 1000);
			tat0.Nfall = unsigned short(is+1);// NofCalculated + 1;


			for (int k = 0; k < Kmax; k++) { //  imitaion of FALLS
				if (pDoc->STOP) {
					stopped = TRUE;
					break;
				}
				TRY { // add fall 
					tattr->push_back(tat0); // add Fall to current array
				} CATCH(CMemoryException, e) { //	(std::bad_alloc error) {
					stopped = TRUE;
					sz = tattr->size();
					S.Format("Thread %d\n Failed to push element\n Current size %ld", m_ID, sz);
					AfxMessageBox(S);
					break;
				} END_CATCH;
				
				
			/*	Slog.Format("{%d} Src %4d/%4d\t Fall%2d\t Pos %4d,%4d\t Vy/V %4d\t Vz/V %4d\t pow %6.2fW\t Q %2d\n",
					m_ID,  tat0.Nfall, jray +1, k+1, tat0.Xmm, tat0.Ymm, tat0.AXmrad, tat0.AYmrad, tat0.PowerW, tat0.Charge);
				TRY{ // add record to tracer log
					logarr->push_back(Slog);
				} CATCH(CMemoryException, e) { //	(std::bad_alloc error) {
					stopped = TRUE;
					sz = tattr->size();
					S.Format("Thread %d\n Failed to push log\n Current size %ld", m_ID, sz);
					AfxMessageBox(S);
					break;
				} END_CATCH;*/
				if (stopped) break;
				
	
			} // Kmax
			if (stopped) break;
		} //Nattr
		
		//if (stopped || pDoc->STOP) break;
				
		CSingleLock cs_lock(&cs, TRUE);
		bool added = pDoc->AddFalls(m_ID, is + 1, tattr);// calls OnStop() if failed
		if (!added || stopped) {
			stopped = TRUE;
			//pDoc->AddLog(logarr);
			break;
		}
		NofCalculated++;	// if not stopped! (success)	
		pDoc->ShowStatus();
		if (!m_Draw) ShowBeamlet(is);
		else ::GdiFlush();
	} //m_Max
	
	if (stopped || !pDoc->STOP) SetContinue(FALSE);

	if (stopped && NofCalculated < pDoc->NofBeamlets)
		S.Format("Thread %d\n %d - %d \n - stopped at beamlet %d", m_ID, m_Min + 1, m_Max + 1, NofCalculated);
	else S.Format("Thread %d\n %d - %d stopped", m_ID, m_Min + 1, m_Max + 1);
	AfxMessageBox(S);

	ClearArrays();
	CSingleLock cs_lock(&cs, TRUE);
	//NofCalculated = pDoc->NofBeamlets; // to check!!!
	pDoc->ShowStatus();
	//pDoc->STOP = TRUE;
	pDoc->StopTime = CTime::GetCurrentTime();
	
	::GdiFlush();
	
}

bool CTracer::AddFall(int n, C3Point Ploc, C3Point Vat, double power)
// called by GetFalls  5.0
{
	bool stopped = FALSE;
	double modV = ModVect(Vat);
	double ax = (Vat.X / modV);
	double ay = (Vat.Y / modV);

	minATTR tat(Ploc, ax, ay, power, n, m_Charge, m_Run); // ax, ay, Ploc are x1000 in minATTR()
	/*if (m_Charge == 0 && n == 7) { // NeutrExit plane
		if (m_Run == 1) m_AtomPower1 += power;
		if (m_Run == 2) m_AtomPower2 += power;//never!
		if (m_Run == 3) m_AtomPower3 += power;//never!
	}*/
/*	try {tattr->push_back(tat);
	} catch (std::length_error) { //(std::bad_alloc error) {
		long max = tattr->size();
		AfxMessageBox("Failed to increase atom track-vector size at plate %d\n Current size %ld", n, max);
	}*/
	
	long sz;
	CString S, Slog;
	TRY { // add fall
		tattr->push_back(tat); // add Fall to current array
	} CATCH(CMemoryException, e) { //	(std::bad_alloc error) {
	stopped = TRUE;
	sz = tattr->size();
	//e->GetErrorMessage(...)
	S.Format("Thread %d\n Failed to push fall\n Current size %ld", m_ID, sz);
	AfxMessageBox(S);
	} END_CATCH;

	/*Slog.Format("{%d} Src %4d/%4d\t Fall%2d\t Pos %4d,%4d\t Vy/V %4d\t Vz/V %4d\t pow %6.2fW\t Q %2d\n",
	m_ID, tat.Nfall, n, k + 1, tat0.Xmm, tat0.Ymm, tat0.AXmrad, tat0.AYmrad, tat0.PowerW, tat0.Charge);
	TRY{ // add record to tracer log
	logarr->push_back(Slog);
	} CATCH(CMemoryException, e) { //	(std::bad_alloc error) {
	stopped = TRUE;
	sz = tattr->size();
	S.Format("Thread %d\n Failed to push log\n Current size %ld", m_ID, sz);
	AfxMessageBox(S);
	break;
	} END_CATCH;*/

	if (stopped) return FALSE;

	return TRUE;
}

bool CTracer:: GetFalls(C3Point P1, C3Point  & P2, bool & metlimit) 
// called by: TraceAtom, TraceIon (TraceReion) - 5.0
// find ALL crosses with plates
// keep closest SOLID + all transparent before it
{
	metlimit = FALSE;//met solid
	bool stopped = FALSE;
	bool added = FALSE;
	CBTRDoc * pDoc = (CBTRDoc*)m_pDoc;
	if (pDoc->STOP) {
		stopped = TRUE;
		return FALSE;
	}
	CString S;
	PtrList & PlatesList = pDoc->PlatesList;
	POSITION pos = PlatesList.GetHeadPosition();
	CPlate * plate;
	CPlate * Plate0 = NULL;// closest
	C3Point Ploc, Ploc0, Vat;
	C3Point Pend, Pcross, Last = P2; // current and final stop-point
	double dist, dist0 = GetDistBetween(P1, P2);
	int n = -1;//closest plate number
	double decay = 1; // for atoms
		
	//----- find the 1st SOLID -> Last -----------------------------
	pos = PlatesList.GetHeadPosition();
	while (pos != NULL) {
		plate = PlatesList.GetNext(pos);
		if (!plate->Solid) continue; // find only among solid 
		
		if (plate->IsCrossed(P1, Last)) { // crossed 
			Ploc = plate->FindLocalCross(P1, Last);

			if (plate->WithinPoly(Ploc)) {// within polygon

				Pend = plate->GetGlobalPoint(Ploc);
				dist = GetDistBetween(P1, Pend);

				if (dist < dist0) { // plate is closer to P1
					dist0 = dist;
					Last = Pend;// + Ort * eps;
					Plate0 = plate;
					Ploc0 = Ploc;
					n = plate->Number;
				} // plate is closer to P1

			} // within polygon
		} // crossed 
	} // while pos

if (n>=0) { 
		
	/*if (n < 0) {
		S.Format("{%d} Getfalls:\n Src %d ray %d - SOLID fall not found ", m_ID, m_iSource, m_iRay);
		AfxMessageBox(S);
		return FALSE; //not found SOLID fall,  stopped = TRUE;
	}*/
	m_Stopped = TRUE;
	metlimit = TRUE;
	P2 = Last;
	Plate0->Touched = TRUE;
	decay = 1;// for ions
	if (m_State == ATOM) decay = GetAtomDecay(P1, Last); // -> 1 if not atom
	Vat = GetVatPlate(Plate0, m_V);
	added = AddFall(n, Ploc0, Vat, m_Power * decay);
	if (!added) { 
		metlimit = TRUE;
		return FALSE; // stopped
	}
} // metlimit

	//----- find all TRANSPARENT before Last point (n >= 0)  -----------------------------
	pos = PlatesList.GetHeadPosition();
	while (pos != NULL) {
		plate = PlatesList.GetNext(pos);
		if (plate->Solid) continue; // find only TRANSP, not SOLID

		if (plate->IsCrossed(P1, Last)) { // crossed 
			Ploc = plate->FindLocalCross(P1, Last);

			if (plate->WithinPoly(Ploc)) {// within polygon -> find Pcross -> add fall
				Pcross = plate->GetGlobalPoint(Ploc);
				Plate0 = plate;
				Ploc0 = Ploc;
				n = plate->Number;
				Plate0->Touched = TRUE;
				decay = 1;// for ions
				if (m_State == ATOM)	decay = GetAtomDecay(P1, Pcross); // -> 1 if not atom
				Vat = GetVatPlate(Plate0, m_V);
				added = AddFall(n, Ploc0, Vat, m_Power * decay);
				
				if (m_State == ATOM && n==7) // run = 1 NeutrExit
					m_AtomPower1 += m_Power*decay;
		
				if (!added) { 
					metlimit = TRUE;
					return FALSE; // stopped
				}
				
			} // within polygon
		} // crossed 
	} // while pos

	if (stopped){ 
		metlimit = TRUE;
		return FALSE; // stopped
	}
	
	return TRUE;
}

double CTracer:: GetNeutrDecay(C3Point P1, C3Point P2)
//used in new beam model 5 only
//only THIN
//called by GetAtomDecay
{
	double decay = 1;
	CBTRDoc * pDoc = (CBTRDoc*) m_pDoc;
	double NeutrX = pDoc->NeutrXmax; 
	if (Between(P1.X, NeutrX, P2.X)) {
		decay = pDoc->NeutrPart; 
		//C3Point Pend(NeutrX, 0, 0);
		//decay = pDoc->GetNeutrDecay(m_NeutrPos, Pend);//THICK
		//m_Power *= decay; 
	}
	return decay;
}

double CTracer::GetAtomDecay(C3Point P1, C3Point P2)
//used in new beam model 5 only
// atoms are emitted from ION SOURCE
{
	if (m_State != ATOM) return 1;// atom starts from GG
	CBTRDoc * pDoc = (CBTRDoc*)m_pDoc;
	double neutr_decay;
	double rei_decay; 
	double pl_decay;
	double Decay;//Decay = 1;

	//if (pDoc->OptThickNeutralization) 
	neutr_decay = GetNeutrDecay(P1, P2);// decrease at NeutrExit
	   //pDoc->GetNeutrDecay(P1, P2); // the best of all decays?
	
	//if (pDoc->OptReionAccount) 	
	rei_decay = pDoc->GetReionDecay(P2); // to change!
	
	pl_decay = 1;
	double dL = 0.05;
	if (pDoc->AreaLong > pDoc->TorCentreX) 
		pl_decay = pDoc->GetDecay(P1, P2, dL);// pDoc->GetDecay(P2);
	
	Decay = neutr_decay * rei_decay * pl_decay;
	if (Decay < 0 || Decay > 1) AfxMessageBox("GetAtomDecay: got invalid decay");
	return Decay;
}

bool CTracer::TraceSourceIonTHIN() //track until NeutrXmax - stepL
{
	bool stopped = FALSE;
	m_Stopped = FALSE;
	/*if (m_Power < 1.e-12) m_Power = 1.e-12; {
		std::cout << " !!! Ray = " << m_iRay << " Zero power !!! \n";
		m_Stopped = TRUE;
		return FALSE;
	}*/
	CBTRDoc * pDoc = (CBTRDoc*)m_pDoc;
	if (pDoc->STOP) {
		stopped = TRUE;
		m_Stopped = TRUE;
		return FALSE;
	}
	//m_Charge = -1;//m_State = SRCION;
	//m_Pos = Pos0; m_V = V0;
	//m_Step = pDoc->TraceStepL;// pDoc->GetIonStep(m_Pos.X);
	//SetColor();
	// <- already set in SetStartPoint(is)

	C3Point P1 = m_Pos;
	C3Point P2, Pstop, ortV;
	C3Point V = m_V;
	CArray<C3Point> Track; // track to draw
	
	//double dist = pDoc->AreaLong + 1.0;
	double modV = ModVect(V); // const
	double eps = 1.e-6;
	bool done;

	if (modV < eps) {
		m_Stopped = TRUE;
		return TRUE; // stopped 
	}

	bool metlimit = 0; // solid or area bound
	double maxlen = pDoc->AreaLong;
	int maxstep = (int)(maxlen / m_Step);// steps number limit
	double Xlimit = pDoc->NeutrXmax - 0.5 * m_Step; // must be < NeutrLimit!!! 

	int i = 0;
	while (metlimit == 0 && !stopped && i < maxstep) {
		m_Pos = P1;
		Track.Add(m_Pos);
		P2 = MakeStep();//CalcV, P2 = P1 + ortV * dist;
		
		done = GetFalls(P1, P2, metlimit);// Last(solid)>>P2
		if (!done) stopped = TRUE;// interrupted (pDoc->STOP)
		if (metlimit) break;

		if (P2.X >= Xlimit) { // return back to limit for passed through
			double dLx = Xlimit - P1.X;
			double dt = dLx / m_V.X;// can be <0
			P2 = P1 + m_V * dt; // stop at Xlimit
			break;
		}
		
		//done = GetFalls(P1, P2, metlimit);// Last(solid)>>P2
		//if (!done) stopped = TRUE;// interrupted (pDoc->STOP)
		
		P1 = P2;
		i++;
	}
	m_Pos = P2;
	Track.Add(m_Pos);
	if (m_Draw) DrawPartTrack(Track);//if (m_Draw) DrawParticlePos(m_Pos, m_Color);

	if (stopped) {
		m_Stopped = TRUE;
		return FALSE;
	}
	return TRUE;
}

bool CTracer::TraceIon(int charge, C3Point Pos0, C3Point V0, double power) 
//calculate ion track + DRAW
//power = const
{
	bool stopped = FALSE;
	CBTRDoc * pDoc = (CBTRDoc*)m_pDoc;
	if (pDoc->STOP) {
		stopped = TRUE;
		return FALSE;
	}
	m_Charge = charge;
	m_Pos = Pos0;
	m_V = V0;
	m_Power = power;
	if (m_Charge > 0) m_State = POSION; 
	if (m_Charge < 0) m_State = SRCION; //NEGION;

	SetColor();
	C3Point P1 = Pos0;
	C3Point P2 = P1, Pstop, ortV;
	C3Point V = V0;
	CArray<C3Point> Track; // track to draw
			
	m_Step = pDoc->GetIonStep(m_Pos.X);
	//double dist = pDoc->AreaLong + 1.0;
	double modV = ModVect(V0); // const
	double eps = 1.e-6;
	bool done;

	if (power < eps || modV < eps) return TRUE; // stop ion

	bool metlimit = FALSE; // solid or area bound
	double maxlen = pDoc->AreaLong;
	int maxstep = (int)(maxlen / m_Step);
	int i = 0;
	while (!metlimit && !stopped && i < maxstep) {
		m_Pos = P1;
		Track.Add(m_Pos);
		P2 = MakeStep();//CalcV, P2 = P1 + ortV * dist;
		done = GetFalls(P1, P2, metlimit);// Last(solid)>>P2
		if (!done) stopped = TRUE;
		P1 = P2;
		i++;
	}	
	m_Pos = P2;
	Track.Add(m_Pos);
	//if (m_Draw) DrawParticlePos(m_Pos, m_Color);
	
	if (m_Draw) DrawPartTrack(Track);
	 
	if (stopped) return FALSE;
	return TRUE;

	//if (!m_Continue) return;
	
	
	if (m_Stopped) {
		SetContinue(FALSE);
		CString s;
		s.Format("+ finished");
		logout << "TraceIon Stopped \n";//AfxMessageBox(s);
		pDoc->STOP = TRUE;
		pDoc->ShowStatus();
	}
}

void CTracer:: DrawPartTrack(CArray<C3Point> & Track)
{
	CMainView* pMainView = (CMainView*)m_pViewWnd;
	cs.Lock();
	pMainView->DrawPartTrack(Track, m_Charge, m_Color);
	//::GdiFlush();
	cs.Unlock();
}

void CTracer:: DistributeTrack(CArray<C3Point> & Track, double power0)
{
	//if (m_Charge != 0) return;
	int n = Track.GetSize();
	if (n < 1) return;
	
	CBTRDoc * pDoc = (CBTRDoc*)m_pDoc;
	
	CArray<C3Point> Path; // refined
	CArray<double> Pow;
	
	if (n < 2) {//single point
		Path.Add(Track[0]);
		Pow.Add(power0);
		cs.Lock();
		pDoc->DistributeTrack(Path, Pow);
		cs.Unlock();
		return;
	}
	
	C3Point Pend = Track[n-1];
	C3Point P0 = Track[0];
	C3Point dP = (Pend - P0) * 0.001;
	double step = 0.01; // ModVect(dP);
	Path.Add(P0);	
	Pow.Add(power0);
	C3Point P1, P, dp;//
	double power, Dist;
	int k; // split step

	for (int i = 1; i < n; i++) {	//	(P <= Pend) {
		P0 = Track[i-1];// already added to Path
		P1 = Track[i];
		Dist = GetDistBetween(P0, P1);// between 2 neibour points
		if (Dist > step) { // split Dist
			k = (int)ceil(Dist / step); //>1
			dp = (P1 - P0)  / k;
			for (int j = 1; j <=k; j++) { // include right end
				P = P0 + dp * j;
				power = power0 * GetAtomDecay(P0, P);
				Path.Add(P);
				Pow.Add(power);
			}// j = k
		} // Dist > step

		else { // Dist <= step
			P = P1;//P0 + dP * i;
			power = power0 * GetAtomDecay(P0, P);  
			//pBeamHorPlane->Load->Distribute(P.X, P.Y, power);
			//pBeamVertPlane->Load->Distribute(P.X, P.Z, power);
			Path.Add(P);
			Pow.Add(power);
		} //no split
	} // i < n

	cs.Lock();
	pDoc->DistributeTrack(Path, Pow);
	cs.Unlock();
	Path.RemoveAll();
	Pow.RemoveAll();
}

bool CTracer::TraceAtom() //called by TraceRay
//calculate (no drawing) till AreaLimit
// + DRAW
{
	m_State = ATOM;
	m_Charge = 0;
	SetColor();
	bool stopped = FALSE;
	CBTRDoc * pDoc = (CBTRDoc*)m_pDoc;
	if (pDoc->STOP) {
		stopped = TRUE;
		return FALSE;
	}
	
	C3Point P1 = m_Pos;
	C3Point P2 = P1, Pstop, ortV;
	CArray<C3Point> Track; 
	Track.Add(P1);
			
	double dist = pDoc->AreaLong + 1.0;
	double modV = ModVect(m_V);
	if (modV > 1.e-6)
	ortV = m_V / modV;
	P2 = P1 + ortV * dist;
	//Track.Add(P2);
	//m_Stopped = TRUE;
	//Pstop = GetAtomSolidFall(P1, P2); // decay is calculated
	//GetAtomFallsBetween(P1, Pstop); // decay is calculated (if m_Power >= 1.e-12)
	bool metlimit = 0;
	bool done = GetFalls(P1, P2, metlimit);// Last(Solid) >> P2
	if (!done) stopped = TRUE;
	//Track.Add(m_Pos);
	Track.Add(P2);//= Last 
	if (m_Draw) DrawPartTrack(Track);

	DistributeTrack(Track, m_Power);
	 
	if (stopped) return FALSE;
	
	return TRUE;
}

bool CTracer::GeneratePosIon(int jray) // NOT called (ResIons after Neutraliser)
{
	bool stopped = FALSE;
	CString S;
	CBTRDoc * pDoc = (CBTRDoc*)m_pDoc;
	if (pDoc->STOP) {
		stopped = TRUE;
		return FALSE;
	}

	//SetRaysAttr(&(pDoc->Attr_Array_Resid)); // done in TraceBeamletAtomsResIons
	SetStartVP(jray);//m_Pos, m_V, m_Power, m_Step, m_iRay = iray
	
	m_State = POSION; 
	m_Charge = 1;
	bool done = TRUE;
	double NeutrX = pDoc->NeutrXmax;
	m_Pos = MakeStepToX(NeutrX);
	m_Power = m_Power * (pDoc->PosIonPart);
	m_Step = pDoc->GetIonStep(NeutrX);

	if (pDoc->OptNeutrStop == FALSE) 
		done = TraceIon(m_Charge, m_Pos, m_V, m_Power);
	
	if (!done || pDoc->STOP) {
		stopped = TRUE;
		S.Format("TracePosIon {%d} interrupted\n Src %d ray %d", m_ID, m_iSource, jray);
		AfxMessageBox(S);
	}
	
	if (stopped) return FALSE;
	return TRUE;
}

bool CTracer::GenerateNegIon(int jray) // NOT called (ResIons after Neutraliser)
{
	bool stopped = FALSE;
	CString S;
	CBTRDoc * pDoc = (CBTRDoc*)m_pDoc;
	if (pDoc->STOP) {
		stopped = TRUE;
		return FALSE;
	}

	//SetRaysAttr(&(pDoc->Attr_Array_Resid));// done in TraceBeamletResIons
	SetStartVP(jray);//m_iRay = iray
	m_State = SRCION;//NEGION; 
	m_Charge = -1;
	bool done = TRUE;
	double NeutrX = pDoc->NeutrXmax;
	m_Pos = MakeStepToX(NeutrX);
	m_Pos = MoveIonToX(NeutrX);
	m_Power = m_Power * (1. - pDoc->NeutrPart - pDoc->PosIonPart);
	m_Step = pDoc->GetIonStep(NeutrX);

	if (pDoc->OptNeutrStop == FALSE) 
		done = TraceIon(m_Charge, m_Pos, m_V, m_Power);
	
	if (!done || pDoc->STOP) {
		stopped = TRUE;
		S.Format("TraceNegIon {%d} interrupted\n Src %d ray %d", m_ID, m_iSource, jray);
		AfxMessageBox(S);
	}
	
	if (stopped) return FALSE;
	return TRUE;
}

bool CTracer::GenerateReions(int jray)
{
	bool stopped = FALSE;
	CString S;
	CBTRDoc * pDoc = (CBTRDoc*)m_pDoc;
	if (pDoc->STOP) {
		stopped = TRUE;
		return FALSE;
	}
	if (pDoc->OptReionStop) return TRUE;

	//CArray<C3Point> Pgen; 

	//SetRaysAttr(&(pDoc->Attr_Array_Reion)); // don ein TraceBeamletReions
	
	m_iRay = jray;
	SetStartVP(jray);//m_V,   m_iRay = iray, m_Power

	//m_State = ATOM;// 
	//m_Charge = 0;
	//m_Step = pDoc->ReionStepL; // GetReionStep!!! (ReionStepSpec) 
	
	bool done = TRUE;
	double ReiXmin = pDoc->ReionXmin;
	double ReiXmax = pDoc->ReionXmax;
	
	C3Point PiStart;
	
	//m_Pos = MakeStepToX(ReiXmin);// atom position = Reion start pos
		
	m_Stopped = FALSE;
	
	if (pDoc->FieldLoaded && fabs(pDoc->MFcoeff) > 1.e-6) { // Move Ion //m_State = SRCION; m_Charge = -1;
		PiStart = MoveIonToX(ReiXmin); // Move (no falls) Source ions to Reions start point
	}
	else 	
		PiStart = MakeStepToX(ReiXmin); // Make (no falls) one straight step (ATOM) 

	if (m_Stopped) { // met solid before ReiXmin
		return TRUE; // stop atom ray
	}
		
	C3Point Vgen = m_V; // Reion Vstart = const!!
	C3Point AtomPos = PiStart;	//Pgen.Add(m_Pos);
	double Xgen;// = ReiXmin;
		
	double AtomPart, ReionPart;// = REionStep * Dens * Sigma; 
	double AtomPower, AtomPower0 = m_Power * pDoc->NeutrPart;// before reionization
	
	///////////// TRACE ATOM - to get AtomPower /////////////////////
		//if (!m_Stopped && pDoc->OptTraceAtoms) {//  as only residuals can be traced next
/*		m_Power = AtomPower0;//m_Power * (pDoc->NeutrPart);
		m_State = ATOM;	
		m_Charge = 0;
		m_Pos = PiStart;
		m_V = Vgen;
		done = TraceAtom();*/

	m_AtomPower3 += AtomPower0;
		
	m_Charge = 1;// used in TraceIon
	
	int n = pDoc->ReionArray.GetSize();//Pgen.GetSize();
	for (int i = 0; i < n; i++) {
		m_Pos = AtomPos;
		m_V = Vgen;
		Xgen = pDoc->ReionArray[i].X;
		
		//m_Stopped = FALSE;
		AtomPos = MakeStepToX(Xgen);//shift AtomPos -> m_Pos with Atom V!
		if (GetFall(m_Pos, AtomPos) > -1) return TRUE;
		//if (m_Stopped) return TRUE;

		ReionPart = pDoc->ReionArray[i].Y;// = ReionStep * Dens * Sigma;
		AtomPart = pDoc->ReionArray[i].Z;
		AtomPower = AtomPower0 * AtomPart;
		
		m_Power = AtomPower * ReionPart;// reionized power
		m_Pos = AtomPos; // Ion Pos  =  AtomPos moved with m_V = Vgen - no change
		done = TraceIon(m_Charge, m_Pos, m_V, m_Power);//-> m_Step = pDoc->GetIonStep(m_Pos.X);
		if (!done) break;
	
	}
		
	if (!done || pDoc->STOP) {
		stopped = TRUE;
		S.Format("GenerateReions {%d} interrupted\n Src %d ray %d", m_ID, m_iSource, jray);
		AfxMessageBox(S);
	}
	
	if (stopped) return FALSE;
	return TRUE;
}

bool CTracer::TraceRay(int jray) // called by TraceBeamletAtoms
{
	bool stopped = FALSE;
	CString S;
	CBTRDoc * pDoc = (CBTRDoc*)m_pDoc;
	if (pDoc->STOP) {
		stopped = TRUE;
		return FALSE;
	}
	SetStartPoint(m_iSource);// m_iSource = isource 	m_State = SRCION;
	SetRaysAttr(&(pDoc->Attr_Array));// must be called before SetStartVP!!!
	//pAttr = &(pDoc->Attr_Array);
	int Nattr = pAttr->GetSize();
	//S.Format(" Total %d rays  ->  TraceRay(%d) \n", Nattr, jray);
	//std::cout <<  S;

	SetStartVP(m_iRay);//m_Power,  m_Step = pDoc->TraceStepL;
	//m_Pos.X = 0;
	//m_iRay = jray; // SetStartPoint(isource) - called before
	//TraceAtomFast();// with decay, reions are stopped 
	//MakeStep(); // slow trace: emit reions

	bool done = TRUE;
	m_Stopped = FALSE;

	// TRACE SRCION until NeutrXmax 
	if (pDoc->FieldLoaded && fabs(pDoc->MFcoeff) > 1.e-6) {
		//SetStartVP(jray);//m_Pos, m_V, m_Power, m_Step, m_iRay = iray
		done = TraceSourceIonTHIN();// up to NeutrXmax
	}

	if (!done || pDoc->STOP) {
		stopped = TRUE;
		S.Format(" TraceSourceIonTHIN {%d} interrupted\n Src %d ray %d\n", m_ID, m_iSource, jray);
		logout << S; //AfxMessageBox(S);
		return FALSE; // interrupted
	}
	
	// if ION is alive (!m_Stopped) - TRACE ATOM after NeutrXmax 
	if (!m_Stopped && pDoc->OptTraceAtoms) {//  as only residuals can be traced next
		//SetStartVP(jray);//m_Pos, m_V, m_Power, m_Step, m_iRay = iray
		done = TraceAtom();
	}
	
	if (!done || pDoc->STOP) {
		stopped = TRUE;
		S.Format(" TraceRay {%d} interrupted\n Src %d ray %d\n", m_ID, m_iSource, jray);
		logout << S; //AfxMessageBox(S);
		return FALSE; // interrupted
	}

	m_Stopped = TRUE;// atom finished
	return TRUE;
}

bool CTracer:: TraceBeamletReions(int isource) // called by TraceAll - for SINGLE RUN 
											   // also called by TraceAllReions - for MULTI-RUN 
// calls GenerateReIons->TracePosIon
// number of BML rays is changed!!! -> Attr_Array_Reion
{
	CString S, Slog;
	CBTRDoc * pDoc = (CBTRDoc*)m_pDoc;
	if (pDoc->OptReionStop) return TRUE;
	m_Run = 3;// pDoc->RUN=13 for SINGLE mode
	SetRaysAttr(&(pDoc->Attr_Array_Reion)); // pAttr = &(pDoc->Attr_Array_Reion);
	int Nattr = pAttr->GetSize();
	if (Nattr < 1) SetRaysAttr(&(pDoc->Attr_Array));
	Nattr = pAttr->GetSize();
	
	S.Format("\t[%d] TraceBeamletReions - bml %d TotRays %d \n",m_ID, isource, Nattr);
	logout << S;// << std::endl;
	
	int jstop = -1;
	C3Point Vstart, Pstart;
		
	m_iSource = isource;
	SetStartPoint(isource);// m_iSource = isource

	bool stopped = FALSE;
	bool done = TRUE;
	//if (stopped || !pDoc->STOP) SetContinue(FALSE);
	
	for (int jray = 0; jray < Nattr; jray++) {
		if (pDoc->STOP) {
			stopped = TRUE;
			jstop = jray;
			S.Format("TraceBeamletReions {%d} interrupted\n Src %d ray %d", m_ID, isource, jray);
			AfxMessageBox(S);
			break;
		}
		
	// TRACE POS ray -------------
		m_iRay = jray;
		Vstart = m_V;
		Pstart = m_Pos;
	
		SetStartPoint(isource);// Init m_Pos, m_iSource = isource
		done = GenerateReions(jray); 

		if (!done || pDoc->STOP) {
			stopped = TRUE;
			jstop = jray;
			break;
		}
		
		if (stopped || pDoc->STOP) {
			jstop = jray;
			stopped = TRUE;
			break;
		}
	} //ray = Nattr
	if (stopped) { SetContinue(FALSE); return FALSE; }
	return TRUE;
}

bool CTracer:: TraceBeamletResIons(int isource) 
// called by TraceAll after TraceBeamletAtoms  - for SINGLE RUN 
// also called by TraceAllResIons(is) - for MULTI-run
// calls GeneratePosIon(ray) / TraceNegIon(ray)
// number of BML rays is changed -> Attr_Array_Resid
{
	CString S, Slog;
	CBTRDoc * pDoc = (CBTRDoc*)m_pDoc;
	
	if (pDoc->OptNeutrStop) return TRUE;
	m_Run = 2;// pDoc->RUN=13 for SINGLE mode

	//SetRaysAttr(&(pDoc->Attr_Array_Resid));
	pAttr = &(pDoc->Attr_Array_Resid);
	int Nattr = pAttr->GetSize();
	if (Nattr < 1) SetRaysAttr(&(pDoc->Attr_Array));
	
	Nattr = pAttr->GetSize();

	S.Format("\t[%d] TraceBeamletResiduals - bml %d TotRays %d\n",m_ID, isource, Nattr);
	logout << S;// << std::endl;
	int jstop = -1;
	C3Point Vstart, P0start, PiStart;

	double NeutrX = pDoc->NeutrXmax;
	double RayPower = 1;

	m_iSource = isource;
	SetStartPoint(isource);//SRCION m_Pos  m_iSource = isource    //m_iRay = jray;
	P0start = m_Pos; //GG - fixed
	m_Step = pDoc->GetIonStep(NeutrX);

	bool stopped = FALSE;
	bool done = TRUE;
	//if (stopped || !pDoc->STOP) SetContinue(FALSE);
	
	for (int jray = 0; jray < Nattr; jray++) {
		if (pDoc->STOP) {
			stopped = TRUE;
			jstop = jray;
			S.Format("TraceBeamletResiduals {%d} interrupted\n Src %d ray %d\n", m_ID, isource, jray);
			AfxMessageBox(S);
			logout << S;
			break;
		}

		m_iRay = jray;
		SetStartVP(jray);// m_V, m_Power, m_Step, m_iRay = iray
		RayPower = m_Power; //source ray
		Vstart = m_V; // fix
		m_Pos = P0start;// SetStartPoint(isource);
		m_V = Vstart;  //SetStartVP(jray);
		m_Stopped = FALSE;
		//PiStart = MakeStepToX(NeutrX); // Make (no falls) one straight step (ATOM) m_V = const 

		if (pDoc->FieldLoaded && fabs(pDoc->MFcoeff) > 1.e-6) { // Move Ion
			//m_State = SRCION; m_Charge = -1;
			//m_Pos = P0start;
			PiStart = MoveIonToX(NeutrX); // Move (no falls) Source ions to Reions start point
		}
		else 	PiStart = MakeStepToX(NeutrX); // Make (no falls) one straight step (ATOM) 

		C3Point Vn0 = m_V; // at NeutrX

		if (m_Stopped) { // met solid
			continue; // GO TO next ray
		}
///////////// TRACE ATOM - to get AtomPower /////////////////////
		//if (!m_Stopped && pDoc->OptTraceAtoms) {//  as only residuals can be traced next
		/*m_Power = RayPower * (pDoc->NeutrPart);
		m_State = ATOM;	
		m_Charge = 0;
		m_Pos = PiStart;
		m_V = Vn0;//Vstart;
		done = TraceAtom();*/
		
		m_AtomPower2 += RayPower * (pDoc->NeutrPart);

		if (!done || pDoc->STOP) {
			stopped = TRUE;
			jstop = jray;
			S.Format("TracePosIon {%d} interrupted\n Src %d ray %d\n", m_ID, m_iSource, jray);
			AfxMessageBox(S);
			logout << S;
			break;
		} //done = GeneratePosIon(jray); 
			
		if (stopped) {// interrupted
			SetContinue(FALSE);
			return FALSE;
		}
		
///////////// TRACE POS ray ---> GeneratePosIon
		m_Power = RayPower * (pDoc->PosIonPart);
		m_State = POSION;
		m_Charge = 1;
		m_Pos = PiStart;
		m_V = Vn0;//Vstart;
		done = TraceIon(m_Charge, m_Pos, m_V, m_Power); // get falls

		if (!done || pDoc->STOP) {
			stopped = TRUE;
			jstop = jray;
			S.Format("TracePosIon {%d} interrupted\n Src %d ray %d\n", m_ID, m_iSource, jray);
			AfxMessageBox(S);
			logout << S;
			break;
		} //done = GeneratePosIon(jray); 
		
		if (stopped) {// interrupted
			SetContinue(FALSE);
			return FALSE;
		}

///////////// TRACE NEG ray ---> GenerateNegIon
		m_Power = RayPower * (1. - pDoc->NeutrPart - pDoc->PosIonPart);//m_Power = m_Power * (pDoc->PosIonPart);
		m_State = SRCION;
		m_Charge = -1;
		m_Pos = PiStart;
		m_V = Vn0;//Vstart;
		done = TraceIon(m_Charge, m_Pos, m_V, m_Power); // get falls

		if (!done || pDoc->STOP) {
			stopped = TRUE;
			jstop = jray;
			S.Format("TraceNegIon {%d} interrupted\n Src %d ray %d\n", m_ID, m_iSource, jray);
			AfxMessageBox(S);
			logout << S;
			break;
		}	// done = GenerateNegIon(jray); // to change: number of rays!!!
		
	/*	Slog.Format("{%d} Src %4d Ray %4d (tot%4d)\t Pos %.2f,%.2f,%.2f\t Vel %9.2e,%9.2e,%9.2e\t Q[%d] - OK\n",
					m_ID, isource + 1, jray + 1, Nattr, m_Pos.X, m_Pos.Y, m_Pos.Z, m_V.X, m_V.Y, m_V.Z,  m_Charge);
		TRY{ // add record to tracer log
			logarr->push_back(Slog);
		} CATCH(CMemoryException, e) { //	(std::bad_alloc error) {
			S.Format("TraceBeamletAtoms:\n Thread %d\n Failed to push log", m_ID);
			AfxMessageBox(S);
		} END_CATCH;*/
	} //ray = Nattr
	
/*	if (stopped) {
		SetContinue(FALSE);
		Slog.Format("{%d} Src %4d  Ray %4d\t tot%4d\t Pos %4.2f,%4.2f,%4.2f\t Q%2d - INTERRUPTED\n",
			m_ID, isource + 1, jstop + 1, Nattr, m_Pos.X, m_Pos.Y, m_Pos.Z, m_Charge);
		TRY{ // add record to tracer log
			logarr->push_back(Slog);
		} CATCH(CMemoryException, e) { //	(std::bad_alloc error) {
			S.Format("TraceBeamletAtoms interrupted:\n Thread %d\n Failed to push log", m_ID);
			AfxMessageBox(S);
		} END_CATCH;
		return FALSE; // if stopped
	}*/

	if (stopped) { 
		SetContinue(FALSE); 
		return FALSE; 
	}
	return TRUE;
}

bool CTracer:: TraceBeamletAtoms(int isource)
// called by TraceAll-SINGLE RUN (replaced Draw)
// also called by TraceAllAtoms-MULTI-RUN
{
	CString S, Slog;
	m_Run = 1;// pDoc->RUN=13 for SINGLE mode
	
	CBTRDoc * pDoc = (CBTRDoc*)m_pDoc;
	if (pDoc->STOP || !GetContinue()) return FALSE;
	pAttr = &(pDoc->Attr_Array); // default attr - for transport
	int Nattr = pAttr->GetSize();//!!!!
	S.Format("\t[%d] TraceBeamletAtoms - bml %d  TotRays %d\n",m_ID, isource, Nattr);
	logout << S;// << std::endl;

	int jstop = -1;
	//return TRUE;// test	
	SetStartPoint(isource);// m_iSource = isource 	m_State = SRCION;

	bool stopped = FALSE;
	//if (stopped || !pDoc->STOP) SetContinue(FALSE);
	
	for (int jray = 0; jray < Nattr; jray++) {
		if (pDoc->STOP) {
			stopped = TRUE;
			jstop = jray;
			S.Format("TraceBeamletAtoms {%d} interrupted\n Src %d ray %d", 
				m_ID, isource, jray);
			AfxMessageBox(S);
			break;
		}
		
		// TRACE RAY -------------
		m_iRay = jray;

		bool done = TraceRay(jray);
		if (!done || pDoc->STOP) {
			stopped = TRUE;
			jstop = jray;
			break;
		}
		if (stopped || pDoc->STOP) {
			jstop = jray;
			stopped = TRUE;
			break;
		}
	} //ray = Nattr
	
	if (stopped) 	SetContinue(FALSE);
	
	return TRUE;//not stopped
	
	/*	Slog.Format("{%d} Src %4d Ray %4d (tot%4d)\t Pos %.2f,%.2f,%.2f\t Vel %9.2e,%9.2e,%9.2e\t Q[%d] - OK\n",
					m_ID, isource + 1, jray + 1, Nattr, m_Pos.X, m_Pos.Y, m_Pos.Z, m_V.X, m_V.Y, m_V.Z,  m_Charge);
		TRY{ // add record to tracer log
			logarr->push_back(Slog);
		} CATCH(CMemoryException, e) { //	(std::bad_alloc error) {
			S.Format("TraceBeamletAtoms:\n Thread %d\n Failed to push log", m_ID);
			AfxMessageBox(S);
		} END_CATCH;*/

	//S.Format(" ------ {%d} ------ Traced Src %d --  %d rays \n", m_ID, isource, Nattr);
	//std::cout << S;// << std::endl;
	
}

void CTracer::TraceAlltest() // replace old Draw(); based on TestMem
// not called - test only
{
	bool stopped = FALSE;
	CString S;

	CBTRDoc * pDoc = (CBTRDoc*)m_pDoc;
	for (int is = m_Min; is <= m_Max; is++) {
		//ShowBeamlet(is);
		if (stopped || pDoc->STOP) break;
		ClearArrays();
		m_iSource = is;
		
		bool done = TRUE;
		done = TraceBeamletAtoms(is); // atoms with decay
		
		if (!done) stopped = TRUE; // !done-> add last bml to log 
		if (pDoc->STOP) {
			SetContinue(FALSE);// m_Continue = FALSE;
			stopped = true;//  break after adding log!!
		}

		// ADDING BML FALLS ------------------------------
		CSingleLock cs_lock(&cs, TRUE);
		bool added = pDoc->AddFalls(m_ID, is + 1, tattr);// calls OnStop() if failed
		if (!added) stopped = true;
		//pDoc->AddFallsToLoads(m_ID, is + 1, tattr);

		//pDoc->ShowStatus();//problem with log!!
		if (is == m_Max) { // finished - is = m_Max
			ClearArrays();// clear last BML log
						  //stopped = TRUE;
		/*	S.Format("{%d} BMLs (%d - %d) - FINISHED at src %d\n", m_ID, m_Min + 1, m_Max + 1, is + 1);
			logarr->push_back(S);
			pDoc->AddLog(logarr);*/
		}
		NofCalculated++;
		if (!m_Draw) ShowBeamlet(is);
		::GdiFlush();
	}
	SetContinue(FALSE);// m_Continue = FALSE;
	return; // test
}

bool CTracer::TraceAllAtoms()//MULTI - run ALL beamlets // no memory reserve test
{							// TraceAlltest();// TEST trace //
	CString S, Slog;
	S.Format("\t[%d]--- TraceAllAtoms start bml %d ... %d\n",m_ID, m_Min, m_Max);
	logout << S;
	CBTRDoc * pDoc = (CBTRDoc*)m_pDoc;
	pAttr = &(pDoc->Attr_Array); // default attr - for transport
	int Nattr = pAttr->GetSize();//!!!!
	int Nscen = pDoc->SCEN;
	m_Run = pDoc->RUN;// 1
	if (pDoc->STOP || !GetContinue()) return FALSE; // - ???
	bool log = TRUE;// write log
	bool stopped = FALSE;
	bool done = TRUE;
	
	//------Trace beamlets--- rewrite the tattr-vector and log for each beamlet --------------
	// skip memory reservation !!!
	int istop = -1;
	for (int is = m_Min; is <= m_Max; is++) { // source beamlet number
		ClearArrays();	//CVectorCleanup tattr_cleanup(tattr);//tattr->clear();
		m_iSource = is;
		istop = is;
		m_AtomPower1 = 0; // set in GetFalls for SINGLE
				// - added to global counters after each BML RUN
		
		done = TraceBeamletAtoms(is);//----- TRACE ATOMS with decay + Write LOG --------
		
		if (!done || pDoc->STOP) {
			SetContinue(FALSE);// m_Continue = FALSE;
			stopped = TRUE;//  break after adding log!!
			break;
		}

		// ADD BML FALLS 
		cs.Lock();//------------
		pDoc->AddFallsToLoads(m_ID, is + 1, tattr);
		bool added = pDoc->AddFallsToFalls(m_ID, is + 1, tattr);
		
		AtomPower1 += m_AtomPower1;

		/*bool added = pDoc->AddFalls(m_ID, is + 1, tattr);// calls OnStop() if failed
		cs.Unlock();
		if (!added) {
			SetContinue(FALSE);// m_Continue = FALSE;
			stopped = TRUE;
			break;
		}
		cs.Lock();*/
		NofCalculated++;	// - if not stopped	
		int Ncalc = NofCalculated;
		cs.Unlock();//----------------

		if (!added) logout << "!!!!! Falls not added !!!!!!!!!!!!!!\n";

		//ClearArrays();// clear last BML tattr
		//pDoc->ShowStatus();// not working for multi-run!!!
		
		if (!m_Draw) ShowBeamlet(is);// locked inside
		ShowProgress(Ncalc); // locked inside
		::GdiFlush();
	} // is = m_Max 
	
	ClearArrays();// after last BML - even stopped
	
	S.Format("\t------- [%d] TraceAllAtoms END ----------\n", m_ID);
	logout << S;

	if (stopped) return FALSE;
	return TRUE; // success
}

bool CTracer::TraceAllResIons() // from ALL beamlets
{
	CString S, Slog;
	S.Format("\t[%d]---- TraceAllResions start bml %d ... %d \n ",m_ID, m_Min, m_Max);
	logout << S;
	CBTRDoc * pDoc = (CBTRDoc*)m_pDoc;
	//pAttr = &(pDoc->Attr_Array); // default attr - for transport
	
	SetRaysAttr(&(pDoc->Attr_Array_Resid));//pAttr = &(pDoc->Attr_Array_Resid);
	int Nattr = pAttr->GetSize();
	if (Nattr < 1) SetRaysAttr(&(pDoc->Attr_Array));
	Nattr = pAttr->GetSize();
	
	int Nscen = pDoc->SCEN;
	m_Run = pDoc->RUN;// 2
	if (pDoc->STOP || !GetContinue()) return FALSE; // - ???
	bool log = TRUE;// write log
	bool stopped = FALSE;
	bool done = TRUE;
	
	//------Trace beamlets--- rewrite the tattr-vector and log for each beamlet --------------
	// skip memory reservation !!!
	int istop = -1;
	for (int is = m_Min; is <= m_Max; is++) { // source beamlet number
		ClearArrays();	//CVectorCleanup tattr_cleanup(tattr);//tattr->clear();
		m_iSource = is;
		istop = is;
		m_AtomPower2 = 0; // set in TreceBMLResIons for SINGLE
					// - added to global counters after each BML RUN
		done = TraceBeamletResIons(is);//----- TRACE Residuals
						//TraceBeamletAtoms(is);//----- TRACE ATOMS with decay 
		if (!done || pDoc->STOP) {
			SetContinue(FALSE);// m_Continue = FALSE;
			stopped = TRUE;//  break after adding log!!
			break;
		}

		// ADD BML FALLS 
		cs.Lock();//-----------------
		pDoc->AddFallsToLoads(m_ID, is + 1, tattr);
		bool added = TRUE;// pDoc->AddFallsToFalls(m_ID, is + 1, tattr);

		AtomPower2 += m_AtomPower2;

		/*bool added = pDoc->AddFalls(m_ID, is + 1, tattr);// calls OnStop() if failed
		cs.Unlock();
		if (!added) {
			SetContinue(FALSE);// m_Continue = FALSE;
			stopped = TRUE;
			break;
		}
		cs.Lock();*/
		NofCalculated++;	// - if not stopped	
		int Ncalc = NofCalculated;
		cs.Unlock();//------------------

		if (!added) logout << "!!!!! Falls not added !!!!!!!!!!!!!!\n";

		ClearArrays();// clear last BML tattr
		//pDoc->ShowStatus();// not working for multi-run!!!
		
		if (!m_Draw) ShowBeamlet(is);// locked inside
		ShowProgress(Ncalc); // locked inside
		::GdiFlush();
	} // is = m_Max 
	
	ClearArrays();// after last BML - even stopped
	
	S.Format("\t------- [%d] TraceAllResiduals END ----------\n", m_ID);
	logout << S;
	if (stopped) return FALSE;
	return TRUE; // success
}

/*	//------Trace beamlets--- rewrite the tattr-vector and log for each beamlet --------------
	// skip memory reservation !!!
	int istop = -1;
	for (int is = m_Min; is <= m_Max; is++) { // source beamlet number
		ClearArrays();	//CVectorCleanup tattr_cleanup(tattr);//tattr->clear();
		m_iSource = is;
		istop = is;

		done = TraceBeamletResIons(is);//----- TRACE Residuals + Write LOG --------

		if (!done) stopped = TRUE; // !done-> add last bml to log 
		if (pDoc->STOP) { //-???
			SetContinue(FALSE);// m_Continue = FALSE;
			stopped = TRUE;//  break after adding log!!
		}

		// ADD BML FALLS 
		cs.Lock();
		bool added = pDoc->AddFalls(m_ID, is + 1, tattr);// calls OnStop() if failed
		if (!added) stopped = TRUE;
		cs.Unlock();

		// ADD BML LOG - if success 
		if (is == m_Max) { // finished - is = m_Max
			ClearArrays();// clear last BML log
						  //stopped = TRUE;// no!! otherwise all threads will stop!
		/*	S.Format("    {%d} Scen%d  ResIons BMLs (%d - %d) - FINISHED at src %d\n",
				m_ID, Nscen, m_Min + 1, m_Max + 1, is + 1);
			logarr->push_back(S);

			cs.Lock();
			logout << S;//std::cout << S; // << std::endl;///////////////
			pDoc->AddLog(logarr);
			cs.Unlock();
		}*/
		// interrupted or failed: is < m_Max
	/*	else if (stopped) { // is < m_Max - interrupted by any reason -> dump log
		/*	S.Format("    {%d} Scen%d ResIons BMLs (%d - %d) - STOP at src %d\n",
				m_ID, Nscen, m_Min + 1, m_Max + 1, is + 1);
			logarr->push_back(S);

			cs.Lock();
			logout << S;// << std::endl;////////////////
			pDoc->AddLog(logarr);
			cs.Unlock();
			break; // NofCalculated not increased!
		}
		if (stopped) break; // for cycle
		cs.Lock();
		NofCalculated++;	// - if not stopped	
		int Ncalc = NofCalculated;
		cs.Unlock();
		//if (!m_Draw) ShowBeamlet(is);// locked inside
		ShowProgress(Ncalc); // locked inside
		//::GdiFlush();
	} // is = m_Max 

	
	ClearArrays();// after last BML

	//S.Format("    ResIons Thread %d (%d - %d) stopped\n at Src %d", m_ID, m_Min + 1, m_Max + 1, istop + 1); // NofCalculated);
	if (stopped)
	{
		cs.Lock();
		//pDoc->StopTime = CTime::GetCurrentTime();
		pDoc->STOP = TRUE;// -> stop all threads if this one aborted
		cs.Unlock();
	}
	
	S.Format("\t[%d] TraceAllResions end \n", m_ID);
	logout << S;
	

	if (stopped) return FALSE;
	return TRUE; // success
}*/

bool CTracer::TraceAllReions() // from ALL beamlets
{
	CString S, Slog;
	S.Format("\t[%d] TraceAllReions start bml %d ... %d \n", m_ID, m_Min, m_Max);
	logout << S;
	CBTRDoc * pDoc = (CBTRDoc*)m_pDoc;
	//pAttr = &(pDoc->Attr_Array); // default attr - for transport

	SetRaysAttr(&(pDoc->Attr_Array_Reion));//pAttr = &(pDoc->Attr_Array_Resid);
	int Nattr = pAttr->GetSize();
	if (Nattr < 1) SetRaysAttr(&(pDoc->Attr_Array));
	Nattr = pAttr->GetSize();

	int Nscen = pDoc->SCEN;
	m_Run = pDoc->RUN;// 3
	if (pDoc->STOP || !GetContinue()) return FALSE; // - ???
	bool log = TRUE;// write log
	bool stopped = FALSE;
	bool done = TRUE;
	
	//------Trace beamlets--- rewrite the tattr-vector and log for each beamlet --------------
	// skip memory reservation !!!
	int istop = -1;
	for (int is = m_Min; is <= m_Max; is++) { // source beamlet number
		ClearArrays();	//CVectorCleanup tattr_cleanup(tattr);//tattr->clear();
		m_iSource = is;
		istop = is;
		m_AtomPower3 = 0; // set in GenerateReions for SINGLE
				// - added to global counters after each BML RUN
		done = TraceBeamletReions(is);// Trace REIONS
					//TraceBeamletResIons(is);//----- TRACE Residuals
						//TraceBeamletAtoms(is);//----- TRACE ATOMS with decay 
		if (!done || pDoc->STOP) {
			SetContinue(FALSE);// m_Continue = FALSE;
			stopped = TRUE;//  break after adding log!!
			break;
		}

		// ADD BML FALLS 
		cs.Lock();//-----------------------
		pDoc->AddFallsToLoads(m_ID, is + 1, tattr);
		bool added = TRUE; //pDoc->AddFallsToFalls(m_ID, is + 1, tattr);

		AtomPower3 += m_AtomPower3;

		/*bool added = pDoc->AddFalls(m_ID, is + 1, tattr);// calls OnStop() if failed
		cs.Unlock();
		if (!added) {
			SetContinue(FALSE);// m_Continue = FALSE;
			stopped = TRUE;
			break;
		}
		cs.Lock();*/
		NofCalculated++;	// - if not stopped	
		int Ncalc = NofCalculated;
		cs.Unlock();//----------------------

		if (!added) logout << "!!!!! Falls not added !!!!!!!!!!!!!!\n";

		ClearArrays();// clear last BML tattr
		//pDoc->ShowStatus();// not working for multi-run!!!
		
		if (!m_Draw) ShowBeamlet(is);// locked inside
		ShowProgress(Ncalc); // locked inside
		::GdiFlush();
	} // is = m_Max 
	
	ClearArrays();// after last BML - even stopped
	
	S.Format("\t------- [%d] TraceAllReions END ----------\n", m_ID);
	logout << S;
	if (stopped) return FALSE;
	return TRUE; // success
}
	/*
	int istop = -1;
	for (int is = m_Min; is <= m_Max; is++) { // source beamlet number
		ClearArrays();	//CVectorCleanup tattr_cleanup(tattr);//tattr->clear();
		m_iSource = is;
		istop = is;

		done = TraceBeamletReions(is);//----- TRACE ATOMS with decay + Write LOG --------

		if (!done) stopped = TRUE; // !done-> add last bml to log 
		if (pDoc->STOP) { //-???
			SetContinue(FALSE);// m_Continue = FALSE;
			stopped = TRUE;//  break after adding log!!
		}

		// ADD BML FALLS 
		cs.Lock();
		bool added = pDoc->AddFalls(m_ID, is + 1, tattr);// calls OnStop() if failed
		if (!added) stopped = TRUE;
		cs.Unlock();

		// ADD BML LOG - if success 
		if (is == m_Max) { // finished - is = m_Max
			ClearArrays();// clear last BML log
						  //stopped = TRUE;// no!! otherwise all threads will stop!
		/*	S.Format("    {%d} Scen%d  Reions BMLs (%d - %d) - FINISHED at src %d\n",
				m_ID, Nscen, m_Min + 1, m_Max + 1, is + 1);
			logarr->push_back(S);
			cs.Lock();
			logout << S;// << std::endl;////////////////
			pDoc->AddLog(logarr);
			cs.Unlock(); 
		}*/
		// interrupted or failed: is < m_Max
		//else if (stopped) { // is < m_Max - interrupted by any reason -> dump log
		/*	S.Format("    {%d} Scen%d Reions BMLs (%d - %d) - STOP at src %d\n",
				m_ID, Nscen, m_Min + 1, m_Max + 1, is + 1);
			logarr->push_back(S);

			cs.Lock();
			logout << S;// << std::endl;///////////
			pDoc->AddLog(logarr);
			cs.Unlock();
			break; // NofCalculated not increased!
		}*/
	/*	if (stopped) break; // for cycle
		cs.Lock();
		NofCalculated++;	// - if not stopped	
		int Ncalc = NofCalculated;
		cs.Unlock();
		//if (!m_Draw) ShowBeamlet(is);// locked inside
		ShowProgress(Ncalc); // locked inside
		//::GdiFlush();
	} // is = m_Max 

	/*	if (stopped) break; // for cycle
		cs.Lock();
		NofCalculated++;	// - if not stopped	
		int Ncalc = NofCalculated;
		S.Format("\t\t\t  + REIONS Traced BML %d\n", Ncalc);
		//logout << S;
		ShowProgress(Ncalc);// locked inside
		//pStatus->ShowStatus(pDC);
		//cs.Unlock();
		//pDoc->ShowStatus();// not working for multi-run!!!
		//if (!m_Draw) ShowBeamlet(is);// locked inside
		//::GdiFlush();
		cs.Unlock();
	} // is = m_Max */
	/*ClearArrays();// after last BML

	S.Format("    Reions Thread %d (%d - %d) stopped\n at Src %d\n", m_ID, m_Min + 1, m_Max + 1, istop + 1); // NofCalculated);
	logout << S;	//AfxMessageBox(S);
	if (stopped)
	{
		cs.Lock();
		//pDoc->StopTime = CTime::GetCurrentTime();
		pDoc->STOP = TRUE;// -> stop all threads if this one aborted
		cs.Unlock();
	}

	//SetContinue(FALSE);// thread m_Continue = FALSE; - ????
	S.Format("\t[%d] TraceAllReions end\n", m_ID);
	logout << S;

	if (stopped) return FALSE;
	return TRUE; // success
}*/

void CTracer::TraceAll() // called by ThreadFunc()  - replace old Draw(), based on (new) TestMem
// trace beamlets [m_Min, m_Max]
// the array of falls is refilled by each beamlet,  appended to m_GlobalVector (then cleared)
{
	CString S, Slog;
	S.Format("\n Tracer %d starts (TraceAll)....\n", m_ID);
	logout << S;

	CBTRDoc * pDoc = (CBTRDoc*)m_pDoc;
	//CSetView * pStatus = (CSetView *)m_pStatus;
	//CDC* pDC = pStatus->GetDC();
	//ClearArrays();
	
	pAttr = &(pDoc->Attr_Array); // default attr - for transport
	int Nattr = pAttr->GetSize();//!!!!
	//S.Format("TraceAll: Attr array size %d", Nattr);
	//AfxMessageBox(S);

	if (pDoc->STOP || !GetContinue()) return;
	
	bool log = TRUE;// write log
	bool stopped = FALSE;
	bool done = TRUE;
	// TraceAlltest();// TEST trace 

	int MaxScen = pDoc->MAXSCEN;

	m_AtomPower1 = 0; // set in GetFalls for SINGLE
	m_AtomPower2 = 0; // set in TreceBMLResIons for SINGLE
	m_AtomPower3 = 0; // set in GenerateReions for SINGLE
	// they are added to global counters after each BML RUN
	
	if (MaxScen > 1) { ////////// MULTI //////////////////////////////////
		
		S.Format("\t[%d]------ TraceALL for SCEN %d -------- \n", m_ID, pDoc->SCEN);
		logout << S;

		if (done && pDoc->OptTraceAtoms)
			done = TraceAllAtoms(); // calls TraceBeamletAtoms(is)
		if (done && !(pDoc->OptNeutrStop))
			done = TraceAllResIons(); // calls TraceBeamletResIons(is)
		if (done && !(pDoc->OptReionStop))
			done = TraceAllReions(); // calls TraceBeamletReions(is)
				
		SetContinue(FALSE);// thread m_Continue = FALSE; 
		return;

	} // MULTI-SCEN  ///////////////////////////////////////////////////////
	
/////////////////  else - SINGLE SCENARIO -> Trace each BML /////////////////////////////		
	S.Format("\t [%d]----- TraceALL for SINGLE RUN -------\n", m_ID);
	logout << S;

	int Kmax = 5;//appr "falls" to reserve
	long sz = Nattr * Kmax; // size stored for 1 bml
	
	//------Trace beamlets--- rewrite the tattr-vector and log for each beamlet ----------------
	int istop = -1;
	for (int is = m_Min; is <= m_Max; is++) { // source beamlet number

		/*if (::PeekMessage(&message, NULL, 0,0, PM_REMOVE)) {// no effect for single-run
				::TranslateMessage(&message);
				::DispatchMessage(&message);
			}    if (pDoc->STOP) break;*/ // no effect for single-run

		ClearArrays();	//CVectorCleanup tattr_cleanup(tattr);//tattr->clear();
	
		TRY{ // reserve attributes
			tattr->reserve(sz);
		} CATCH(CMemoryException, e) {
			//S.Format("Thread %d\n reserve at %d FAILED (low memory)", m_ID, NofCalculated);
			//AfxMessageBox(S);
			S.Format("Thread %d reserve FAILED (low memory)\n", m_ID);
			logout << S;
			stopped = TRUE;
			break;
		} END_CATCH;
 /// TO CHECK!!!!!

	/*	TRY { // reserve log
			logarr->reserve(sz);
		} CATCH(CMemoryException, e) {} END_CATCH; */

		//----- SINGLE-RUN (IS) BEAMLETS (from GG) ///////////////
		m_iSource = is;
		istop = is;
		done = TRUE;
		m_AtomPower1 = 0; // set in GetFalls for SINGLE
		m_AtomPower2 = 0; // set in TreceBMLResIons for SINGLE
		m_AtomPower3 = 0; // set in GenerateReions for SINGLE

		if (done) // && pDoc->OptTraceAtoms) //----- TRACE ATOMS with decay --------------
			done = TraceBeamletAtoms(is); 
		if (done && !(pDoc->OptNeutrStop)) //----- TRACE RESIDUALS ------------------
			done = TraceBeamletResIons(is); // Residual ions pos/neg - from NeutrX
		if (done && !(pDoc->OptReionStop)) //----- TRACE REIONS ----------------------
			done = TraceBeamletReions(is);
		
		//if (done) //TRACE in PLASMA
		//	done = GenIonsPlasma(is);
		
		//if (!done) stopped = TRUE; // !done-> add last bml to log 
		if (!done || pDoc->STOP) {
			SetContinue(FALSE);// m_Continue = FALSE;
			stopped = TRUE;//  break after adding log!!
			break;
		}

	// ADD BML FALLS (SINGLE)///////////////////
									//CSingleLock cs_lock(&cs, TRUE);

		CorrectFallsPower(); //to fit balance with run1 before Loads Calculations
				// uses m_AtomPowers to correct all tattr.power
		
		cs.Lock();
		pDoc->AddFallsToLoads(m_ID, is + 1, tattr);// BeamPlanes -> SetSumMax
		//bool added = pDoc->AddFallsToFalls(m_ID, is + 1, tattr);
		// - TO SWITCH ON in future
		
		//if (!added) logout << "!!!!! Falls not added !!!!!!!!!!!!!!\n";

		/*bool added = pDoc->AddFalls(m_ID, is + 1, tattr);// calls OnStop() if failed
		if (!added) {
			SetContinue(FALSE);// m_Continue = FALSE;
			stopped = TRUE;	break;
		}*/

		NofCalculated++;	// - if not stopped	
		int Ncalc = NofCalculated;
		if (NofCalculated == pDoc->NofBeamletsTotal)  
			stopped = TRUE;// -> leads to STOP all threads in doc
		cs.Unlock();

		
		ClearArrays();// clear last BML tattr
		//pDoc->ShowStatus();// not working for multi-run!!!
		if (!m_Draw) ShowBeamlet(is);// locked inside
		ShowProgress(Ncalc); // locked inside
		::GdiFlush();
		
	} // is = m_Max 

	SetContinue(FALSE);// finished m_Max or stopped -> m_Continue = FALSE;

	S.Format("\t [%d] Thread (%d - %d) stopped\n at Src %d\n", 
		 m_ID, m_Min + 1, m_Max + 1, istop + 1); // NofCalculated);
	logout << S;
	
	ClearArrays();

	//CSingleLock cs_lock(&cs, TRUE);
	if (stopped) {
		//cs.Lock();
		pDoc->STOP = TRUE;// -> stop all threads if one aborted
		CTime tm = CTime::GetCurrentTime();
		pDoc->StopTime = tm;
		CString Date, Time;
		Date.Format("%02d-%02d-%04d", tm.GetDay(), tm.GetMonth(), tm.GetYear());
		Time.Format("%02d:%02d:%02d", tm.GetHour(), tm.GetMinute(), tm.GetSecond());
		S.Format(" BTR %g ---- SINGLE RUN is DONE ----\n ----- Date %s  Time %s -----\n\n",
			BTRVersion, Date, Time);
		logout << S;

		// SHOW TRACKED INFO - SINGLE //
		pDoc->WriteReport("TXT");
		pDoc->WriteReport("CSV");
		
		pDoc->ResetLogFile();

		//Beep(100, 200);
		if (NofCalculated == pDoc->NofBeamletsTotal){
			AfxMessageBox("SINGLE RUN is DONE!");//\n Click on the Main Screen ");
			pDoc->OnShow();// to check
		}
		else {
			AfxMessageBox("SINGLE RUN is STOPPED!");
			pDoc->OnShow();// to check
		}

		//cs.Unlock();
	} // if stopped
	
	//pStatus->ReleaseDC(pDC);
	//pDoc->ShowStatus();//  breaks sometimes
}

void CTracer:: CorrectFallsPower()// for SINGLE
{
	double Coeff, power;
	int run;
	double Coeff2 = 1;
	if (m_AtomPower2 > 1e-3) Coeff2 = m_AtomPower1 / m_AtomPower2;
	double Coeff3 = 1;
	if (m_AtomPower3 > 1e-3) Coeff3 = m_AtomPower1 / m_AtomPower3;

	for (int i = 0; i < (int)tattr->size(); i++) {
		//minATTR &tattr = parr->at(i);
		power  = tattr->at(i).PowerW;
		run =  tattr->at(i).run;
		Coeff = 1;// run 1 - not correct
		if (run == 2) Coeff = Coeff2;
		if (run == 3) Coeff = Coeff3;

		tattr->at(i).PowerW  = (float)(power * Coeff);
	}
}
void CTracer::DumpArrays()
{
/*	CBTRDoc * pDoc = (CBTRDoc*)m_pDoc;
	CString Slog;
	Slog.Format("<%d>  Src %4d Bml %4d - STOPPED (Dumped) \n", m_ID, m_iSource + 1, NofCalculated + 1);
	logarr->push_back(Slog);
	cs.Lock();
	pDoc->AddFalls(m_ID, m_iSource, tattr); // add falls after the last BML in a tracer
	pDoc->AddLog(logarr);
	logout << Slog;
	cs.Unlock(); */
}

void CTracer::Draw() // old (before 4.5) 
{	
	DoNextStep();
		
	if (!m_Draw) return; // do not show particles
	CMainView* pMainView = (CMainView*)m_pViewWnd;
	cs.Lock();
	{
		if (!m_Dead) {
			pMainView->ShowParticlePos(m_Pos, m_Color);
		}
		
		GdiFlush();
	}
	cs.Unlock();

	//CDC* pDC = m_pViewWnd->GetDC();
	//CDC* pDC = pMainView->GetDC();
	
	// int oldray = m_iRay;
	/*int x1, y1, z1, x2, y2, z2;
	x1 = pMainView->OrigX + (int)(m_Pos.X * pMainView->ScaleX);
	y1 = pMainView->OrigY - (int)(m_Pos.Y * pMainView->ScaleY);
	z1 = pMainView->OrigZ - (int)(m_Pos.Z * pMainView->ScaleZ);*/

	//DoNextStep();
	
	/*x2 = pMainView->OrigX + (int)(m_Pos.X * pMainView->ScaleX);
	y2 = pMainView->OrigY - (int)(m_Pos.Y * pMainView->ScaleY);
	z2 = pMainView->OrigZ - (int)(m_Pos.Z * pMainView->ScaleZ);*/
	
		
	//pMainView->ReleaseDC(pDC);
}

long long CTracer:: GetMemUsedkB()
{
	HANDLE hProcess = GetCurrentProcess();
	PROCESS_MEMORY_COUNTERS pmc;
	if (NULL == hProcess)  return 0;

	long long MemUsed = 0;
	if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc)) )
		MemUsed = pmc.WorkingSetSize /1024; // currently used by BTR
	return MemUsed;	
}

long long CTracer:: GetMemFreekB()
{
	MEMORYSTATUSEX statex;
	statex.dwLength = sizeof (statex);
	GlobalMemoryStatusEx (&statex);

	long long MemFree = 0;
	MemFree = (long long) (statex.ullAvailPhys / 1024); // available on the system
	return MemFree;
}

long long CTracer:: GetGlobFalls()
{
	CBTRDoc * pDoc = (CBTRDoc*) m_pDoc;
	long long Falls = pDoc->ArrSize;//m_GlobalVector.GetSize(); // Global falls array size
	return Falls;
}

/*void CBTRDoc:: GetMemState()
{
	int DIV = 1024;
	MEMORYSTATUSEX statex;
	statex.dwLength = sizeof (statex);
	GlobalMemoryStatusEx (&statex);
	MemFree = (long) (statex.ullAvailPhys /DIV); // available on the system
	HANDLE hProcess = GetCurrentProcess();
	PROCESS_MEMORY_COUNTERS pmc;
	if (NULL == hProcess)  return;
	if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc)) )
		MemUsed = pmc.WorkingSetSize /DIV; // currently used by BTR
	MemFalls = 0;// m_AttrVector[0].size() * ThreadNumber * sizeof(SATTR) / DIV;
	ArrSize = m_GlobalVector.size();//ArrSize = Falls;
}*/

void CTracer:: ShowProgress(int Ncalc) // new - static loads
{
	if (Ncalc % 20 != 0) return;
	CBTRDoc * pDoc = (CBTRDoc*) m_pDoc;
	if (Ncalc == 0 || pDoc->STOP) return; // tracing in progress
/////////////////////////////////////////////////
	bool SINGLE = (pDoc->MAXSCEN == 1);
	int Maxscen = pDoc->MAXSCEN; // total runs
	int Nscen = pDoc->SCEN;
	int Nrun = pDoc->RUN;
	int Ntot = pDoc->NofBeamlets;
	CTime Tbegin = pDoc->StartTime;
	long long MEM_BTR_kB = GetMemUsedkB();//pDoc->MemUsed;// kb
	long long Falls = GetGlobFalls();
	CSetView * pStatus = (CSetView *)m_pStatus;
	CDC* pDC = pStatus->GetDC(); //or GetWindowDC();  
	pDC->SetTextColor(RGB(0,0,255));
	//CFont* pOldFont = pDC->SelectObject(&font);
	CString S;
	//cs.Lock();
		int BMLrays = pAttr->GetSize();//pDoc->Attr_Array.GetSize();
		//pDoc->GetMemState();
		CTime t = CTime::GetCurrentTime();
		
		CTime Tend = t;
		CTimeSpan Telapsed;
		
		int h, m, s, h0, m0, s0, dh, dm, ds, sec;
		long mspb, msleft, sleft, mleft, hleft;
		h =	t.GetHour(); m = t.GetMinute(); s = t.GetSecond();// current time
		h0 = Tbegin.GetHour(); m0 = Tbegin.GetMinute(); s0 = Tbegin.GetSecond();
		Telapsed = t - Tbegin; // t - current
		dh = Telapsed.GetHours(); dm = Telapsed.GetMinutes(); ds = Telapsed.GetSeconds();
		sec = ds + dm * 60 + dh * 3600;
		mspb = sec * 1000 / Ncalc;
		msleft = mspb * (Ntot - Ncalc); // single run // (Ntot * (Maxscen - Nscen + 1) - Ncalc);
		sleft = msleft / 1000;
		mleft = sleft / 60;
		hleft = mleft / 60;

		cs.Lock();
		S.Format("\n%02d:%02d:%02d --- PROGRESS --- SCEN %d RUN %d ---\n", 
						h,m,s, Nscen, Nrun);
		pDC->TextOut(10, 105, S);
		logout << S;

		S.Format("Traced BML  %d                   \n", Ncalc);
		pDC->TextOut(10, 125, S); 
		logout << S;

		S.Format("Passed   %02d:%02d:%02d                \n", dh, dm, ds);
		pDC->TextOut(10,145, S); //logout << S;

		S.Format("Time / BML  %d ms            \n", mspb);
		pDC->TextOut(10,165, S); logout << S;

		S.Format("Cuckoo for this Run   %02d:%02d:%02d  \n", 
					hleft,  mleft - hleft*60, sleft - mleft*60);
		pDC->TextOut(10,185, S);//logout << S;

		S.Format("BTR holds   %ld kB         \n", MEM_BTR_kB);
		pDC->TextOut(10,205, S); logout << S;

		//S.Format("Mem left       %ld kB      \n", MemFreeKB);
		//pDC->TextOut(10,225, S); logout << S;

		S.Format("Falls Size  %d               \n", Falls);
		pDC->TextOut(10, 225, S); 
		logout << S;
		logout << "-----------------------\n";
		//if (SINGLE) ::GdiFlush();
		cs.Unlock();

	pStatus->ReleaseDC(pDC);
}

void CTracer:: ShowProgressFalls(int Ncalc)
{
	bool lout = FALSE;
	if (Ncalc % 20 == 0) lout = TRUE; 
	if (!lout) return;
	CBTRDoc * pDoc = (CBTRDoc*) m_pDoc;
	if (Ncalc == 0 || pDoc->STOP) return; // tracing in progress
/////////////////////////////////////////////////
	bool SINGLE = (pDoc->MAXSCEN == 1);
	int Maxscen = pDoc->MAXSCEN; // total runs
	int Nscen = pDoc->SCEN;
	int Nrun = pDoc->RUN;
	int Ntot = pDoc->NofBeamlets;
	//cs.Lock();
	CTime Tbegin = pDoc->StartTime;
	long long Falls = GetGlobFalls();// pDoc->ArrSize;
	//cs.Unlock();

	long long MEM_BTR_kB = GetMemUsedkB();//pDoc->MemUsed;// kb
	long long MemFreeKB = GetMemFreekB();//pDoc->MemFree;// kb
	//long Falls = GetGlobFalls();//pDoc->ArrSize; // Global falls array size
	long long FallsBML;// falls per BML
	long long MemBML; // bytes per BML
	long long TotMemB;//(memb) bytes - all falls
	long long leftBML; // 
	int MemFallB = sizeof(minATTR);// bytes per 1 fall = 16..20b
	
	CSetView * pStatus = (CSetView *)m_pStatus;
	CDC* pDC = pStatus->GetDC(); //or GetWindowDC();  
	pDC->SetTextColor(RGB(0,0,255));
	//CFont* pOldFont = pDC->SelectObject(&font);
	CString S;
	//cs.Lock();
		int BMLrays = pAttr->GetSize();//pDoc->Attr_Array.GetSize();
		//pDoc->GetMemState();
		CTime t = CTime::GetCurrentTime();
		
		CTime Tend = t;
		CTimeSpan Telapsed;
		
		int h, m, s, h0, m0, s0, dh, dm, ds, sec;
		long mspb, msleft, sleft, mleft, hleft;
		h =	t.GetHour(); m = t.GetMinute(); s = t.GetSecond();// current time
		h0 = Tbegin.GetHour(); m0 = Tbegin.GetMinute(); s0 = Tbegin.GetSecond();
		Telapsed = t - Tbegin; // t - current
		dh = Telapsed.GetHours(); dm = Telapsed.GetMinutes(); ds = Telapsed.GetSeconds();
		sec = ds + dm * 60 + dh * 3600;
		mspb = sec * 1000 / Ncalc;
		msleft = mspb * (Ntot - Ncalc); // single run // (Ntot * (Maxscen - Nscen + 1) - Ncalc);
		sleft = msleft / 1000;
		mleft = sleft / 60;
		hleft = mleft / 60;
			
		//double SelCurr = pDoc->IonBeamCurrent / pDoc->NofChannelsHor / pDoc->NofChannelsVert
						//* pDoc->NofActiveChannels * pDoc->NofActiveRows;// source current traced
		// show RUN starttime	
		//if (NofCalculated > 0) S.Format("Beam is started    ");
	/*	if (pDoc->STOP || Ncalc == Ntot) 
			S.Format(" *** STOPPED                      ");
		else S.Format("Run START at %02d:%02d:%02d       ", h0, m0, s0);*/
		//long MemFalls = (pDoc->ArrSize) * sizeof(minATTR);//can be > max long!!!
		//if (MemFalls < 0) MemFalls = 0;
				
		if (Ncalc > 0) // impossible
			FallsBML = Falls / Ncalc; // calculated falls per BML
		else FallsBML = BMLrays * 5; //5 - "aver" falls per ray Falls / Ntot; 
		MemBML = FallsBML * MemFallB; // bytes per 1 BML
		TotMemB = Falls * MemFallB;// all falls bytes
		if (TotMemB < 1) TotMemB = 1;// impossible
		//long leftFalls = (pDoc->MemFreeKB / MemFallB) * 1024;
		
		//leftBML = (MemFreeKB / MemBML) * 1024;
		leftBML = MemFreeKB * 1024 / MemBML;
		//long Nleft = (pDoc->MemFree * 1024 - MemFalls) / TotMemB;
		//if (Nleft < 0) Nleft = 0;
		
	/*	S.Format("BTR holds       %ld kB  ", pDoc->MemUsed);
		S.Format("Available mem   %ld kB  ", pDoc->MemFree);
		S.Format("Falls arr: %ld elem x %d Bytes = %ld Bytes   ", pDoc->ArrSize, sizeof(minATTR), MemFalls);
		*/
		
		//if (Ncalc > 0 && Ncalc <= Ntot && !pDoc->STOP) { // tracing in progress
			//if (Ncalc < Ntot && !pDoc->STOP ) { 
			cs.Lock();
				S.Format("\n%02d:%02d:%02d --- PROGRESS --- SCEN %d RUN %d ---\n", 
						h,m,s, Nscen, Nrun);
				pDC->TextOut(10, 105, S);
				logout << S;

				S.Format("Traced BML  %d                \n", Ncalc);
				pDC->TextOut(10, 125, S);
				logout << S;

				S.Format("Passed   %02d:%02d:%02d            \n", dh, dm, ds);
				pDC->TextOut(10,145, S); //logout << S;

				S.Format("Time / BML      %d ms          \n", mspb);
				pDC->TextOut(10,165, S); logout << S;

				S.Format("Cuckoo for this Run   %02d:%02d:%02d  \n", 
					hleft,  mleft - hleft*60, sleft - mleft*60);
				pDC->TextOut(10,185, S);//logout << S;

				S.Format("BTR holds      %ld kB      \n", MEM_BTR_kB);
				pDC->TextOut(10,205, S); logout << S;

				S.Format("Mem left       %ld kB      \n", MemFreeKB);
				pDC->TextOut(10,225, S); logout << S;
				
				S.Format("Mem / BML      %ld B               \n", MemBML);
				pDC->TextOut(10,245, S);  logout << S;
  
				S.Format("Falls arr      %ld               \n", Falls);
				//x %d Bytes    \n", //, MemFallB);
				pDC->TextOut(10, 265, S); logout << S;
				
				S.Format("BML left   %ld           \n", leftBML);
				pDC->TextOut(10, 285, S); logout << S;
				S.Format("BML limit  %ld           \n", (long)Ncalc + leftBML);
				pDC->TextOut(10, 305, S); logout << S << "-----------------------\n";
				//if (SINGLE) ::GdiFlush();
			cs.Unlock();
			//}// calculated < Total 
		//} // if 
	//::GdiFlush();

	pStatus->ReleaseDC(pDC);
}

void CTracer:: ShowBeamlet(int isource)
{
	CBTRDoc * pDoc = (CBTRDoc*) m_pDoc;
	BEAMLET_ENTRY be = pDoc->BeamEntry_Array[isource];//.GetAt(k);
	CMainView* pMainView = (CMainView*)m_pViewWnd; 
	CDC* pDC = pMainView->GetDC();
	int x1, y1, z1, x2, y2, z2;
	double y0 = be.PosY;
	double z0 = be.PosZ;
	double	x = Min(pDoc->PlasmaXmax, pDoc->AreaLong);
	double	y = y0 + x * tan(be.AlfY); 
	double	z = z0 + x * tan(be.AlfZ); 
	
	x1 = pMainView->OrigX;
	y1 = pMainView->OrigY - (int)(y0 * pMainView->ScaleY);
	z1 = pMainView->OrigZ - (int)(z0 * pMainView->ScaleZ);
	x2 = pMainView->OrigX + (int)(x * pMainView->ScaleX);
	y2 = pMainView->OrigY - (int)(y * pMainView->ScaleY);
	z2 = pMainView->OrigZ - (int)(z * pMainView->ScaleZ);
	
	//HDC DC = pMainView->GetDC()->GetSafeHdc();
	cs.Lock();
	CPen * pOldPen = pDC->SelectObject(&pMainView->AtomPen);//DotPen);
	pDC->MoveTo(x1, y1); pDC->LineTo(x2, y2);
	pDC->MoveTo(x1, z1); pDC->LineTo(x2, z2);
	pDC->SelectObject(pOldPen);
	::GdiFlush();
	cs.Unlock();

	pMainView->ReleaseDC(pDC);
}

int CTracer:: GetFall(const C3Point P1, C3Point & P2) 
//old, used in 4.5-  assume 1 SOLID + 1 TRANSPAR crossed within one step
// in 5.0 - only check cross event (for atoms before reions or residuals emit)
{
	CBTRDoc * pDoc = (CBTRDoc*) m_pDoc;
	PtrList & PlatesList = pDoc->PlatesList;
	POSITION pos = PlatesList.GetHeadPosition();
	CPlate * plate;
	CPlate * PlateSol = NULL;
	CPlate * PlateTr = NULL;
	C3Point Pend, Ploc, Ploc0, PlocTr;
	C3Point Last = P2; // stop
	C3Point Cross = P2; // transp
	double sdist;// find dist to closest solid (stop)
	double dist; // to transpar, must be <= sdist
	double dist0 = GetDistBetween(P1, P2);// for solid
	int n = -1;//solid plate -> Last
	int nt = -1;//transpar (before solid) -> Cross
	BOOL MetSolid = FALSE;
	BOOL Found = FALSE; // transp

	// Find closest solid
	while (pos != NULL) {
			plate = PlatesList.GetNext(pos); 
			if (plate->Solid == FALSE) continue;
			if (plate->IsCrossed(P1, Last)) { // crossed 
				Ploc = plate->FindLocalCross(P1, Last);
				if (plate->WithinPoly(Ploc)) {
					Pend = plate->GetGlobalPoint(Ploc);
					sdist = GetDistBetween(P1, Pend);
					if (sdist < dist0) { // check if solid closer
						dist0 = sdist; // min dist to solid
						Last = Pend;
						PlateSol = plate;
						n = PlateSol->Number;
						Ploc0 = Ploc;
						MetSolid = TRUE;
						break;
					}
				} // within polygon
			} // crossed 
		} // while pos
	 
	return n;// 1st solid cross between P1, P2

	// Find transparent - only one assumed!!
	pos = PlatesList.GetHeadPosition();
	while (pos != NULL && !Found) {
			plate = PlatesList.GetNext(pos); 
			if (plate->Solid) continue;
			if (plate->IsCrossed(P1, Last)) { // crossed 
				Ploc = plate->FindLocalCross(P1, Last);
				if (plate->WithinPoly(Ploc)) {
					Pend = plate->GetGlobalPoint(Ploc);
					dist = GetDistBetween(P1, Pend);
					if (dist < dist0) { // check if trnasp closer min to solid
						//dist = dist;
						Cross = Pend; // Last - for stop
						PlateTr = plate;
						nt = PlateTr->Number;
						PlocTr = Ploc;
						Found = TRUE;//1st found
						
					}
				} // within polygon
			} // crossed 
		} // while pos

	// check if went out of area
	//if (!WithinCalculatedLimits(Last)) { // out of area
	//if (Last.X >= pDoc->AreaLong || Last.X < -1 ) { // out of area
	//	if (m_State == SRCION) m_Dead = TRUE;
	//	else m_Stopped = TRUE; //SetNextState(); // for ions
	//}

	C3Point Vat;
	double modV, ax, ay;

	if (nt > 0)  // Found transpar plate
	{
		PlateTr->Touched = TRUE;
		// add stoppoint
		Vat = GetVatPlate(PlateTr, m_V);
		modV = ModVect(m_V);
		ax = (Vat.X / modV);
		ay = (Vat.Y / modV);
	/*	minATTR tat1(PlocTr, ax, ay, m_Power, nt, m_Charge); 
		try {
			tattr->push_back(tat1);
		}catch (std::bad_alloc error) {}*/
	}

	if (n > 0)  // crossed SOLID plate
	{
		//P2 = Last;// + (P2-Last) * 0.01; // shift cross-point forward
		//if (IsCalculated(Plate0) && WithinCalculatedLimits(Last)) {
			PlateSol->Touched = TRUE;
			// add stoppoint
			//double angle = GetVangle(Plate0, m_V);// * 180 / PI); 
			//if (angle > 90) angle = 90;
			Vat = GetVatPlate(PlateSol, m_V);
			modV = ModVect(m_V);
			ax = (Vat.X / modV);
			ay = (Vat.Y / modV);
		/*	minATTR tat2(Ploc0, ax, ay, m_Power, n, m_Charge); // ax, ay, Ploc are x1000 in minATTR()
			// (Ploc0, m_Power, m_Charge, n, angle);
		try {
			tattr->push_back(tat2);
		} catch (std::bad_alloc error) {
			// jacob: ???
			// long max = tattr->size();
			//AfxMessageBox(A2CT("Failed to increase ion track-vector size"));
			//AfxMessageBox("Failed to increase ion track-vector size at plate %d\n Current size %ld", n, max);
		} */
		//	WriteParticleFall(Last, Plate0);
		/*if (Pend.X > *pDoc->Duct7X && Pend.X < *pDoc->Duct8X && m_OptWriteFall) { // option ON
			double angle = GetVangle(Plate0, m_V);  
			FALL_ATTR fall(m_Charge, n, Pend, float(angle), m_Power);
			fattr->push_back(fall); 
		} // fill fallvector at fall*/
  
	//} //  +  within calc limits

	//////// Met Solid -> change state

		//if (!WithinCalculatedLimits(Last)) { 
			//if (m_State == SRCION) m_Dead = TRUE;
			//else m_Stopped = TRUE; //SetNextState(); // for ions

			// fill SumPowerX counter if ION is stopped by any solid surface 
		/*	if (m_State == REION) {
				int k = (int) (Last.X / SumPowerStepX);
				if (k < SumReiPowerX.GetSize()) {
					cs.Lock();
					SumReiPowerX[k] += m_Power;
					cs.Unlock();
				}
		
			} // reion
			*/
		/*	if (m_State == ATOM) {// fill SumPowerX counter if ION is stopped by any solid surface
				int k = (int) (Last.X / SumPowerStepX);
				if (k < SumAtomPowerX.GetSize()) {
					cs.Lock();
					SumAtomPowerX[k] += m_Power;
					cs.Unlock();
				}
			} // atom */
		//} // met Solid plate
	} //  crossed solid plate (n>0)

	
	P2 = Last;
	int nfinal = n;
	if (n<=0 && nt > 0) {
		P2 = Cross;
		nfinal = nt;
	}

	if (MetSolid || !WithinCalculatedLimits(P2)) { 
		if (m_State == SRCION) m_Dead = TRUE;
		else m_Stopped = TRUE; //SetNextState(); // for ions
	}
	
	return nfinal; // solid!!
}

double ReflPower(double Amrad)
{
	double Pcoeff = 1;
	//Pcoeff = 0.55617 * exp(-Amrad / 11.06757) + 0.13471 * exp(-Amrad /29.79121);//old
	Pcoeff = 0.62907 * exp(-Amrad / 14.74106) + 0.12864 * exp(-Amrad /53.56969);//new corrected 12/07/11
	return Pcoeff;
}

double ReflEnergy(double Amrad)
{
	double Ecoeff = 1;
	//Ecoeff = 0.62907 * exp(-Amrad / 14.74106) + 0.12864 * exp(-Amrad / 53.56969);//old
	Ecoeff = 0.05656 + 0.54738 * exp(-Amrad / 15.64203) + 0.26681 * exp(-Amrad / 46.8388); // new corrected 12/07/11
	return Ecoeff;
}


double CTracer:: ReflectPosIon(C3Point Pgl, CPlate * Plate)
{
	if (m_Falls > 0) return 0;
	C3Point V0 = m_V;
	double V0abs = ModVect(V0);
	CBTRDoc * pDoc = (CBTRDoc*) m_pDoc;
	//double E = *pDoc->IonBeamEnergy * 1000000;
	double Mass = pDoc->TracePartMass;
	//double V = sqrt(2.* E * Qe / Mass);
	double Energy = Mass * V0abs * V0abs * 0.5 / Qe; // eV
	if (Energy < 800000) return 0; // < 0.8 MeV 

	double power0 = m_Power;
	C3Point Normal = Plate->OrtZ;
	double VproectN = ScalProd(V0, Normal);
	//if (VproectN < 0) return;  
	C3Point Vn = Normal * VproectN; // V projection on normal
	C3Point Vb = V0 - Vn; // = const
	C3Point V1 = Vb - Vn; // = V0 - 2*Vn , reflected V
	double alfa0 = atan(ModVect(Vn) / ModVect(Vb));// rad, counted from the surface
	if (alfa0 < 0 || alfa0 > 0.25) return 0;// no reflect after 250 mrad

	double Ecoeff = ReflEnergy(alfa0 * 1000);
	double Vcoeff = sqrt(Ecoeff);
	C3Point Vrefl = V1 * Vcoeff; //mrad
	C3Point Vtang = Vb * ModVect(Vrefl) / ModVect(Vb);// all V let be along the surf 
	/*C3Point Vtang2 = VectProd(Vb, Vn);
	double dTeta = PI * 10 / 180;
	double dVteta = ModVect(Vtang) * tan(dTeta);
	C3Point Vt = Vtang2 / ModVect(Vtang2) * dVteta;*/
	m_Power = power0 * ReflPower(alfa0 * 1000); 
	
	//m_State = POSION; 
	if (m_Charge > 0) { // from D+
		m_V = Vrefl; //Vn * (-Vcoeff);
		m_Color = RGB(150, 50, 150); 
	}
	else { // from D-
		m_V = Vrefl; //Vtang; // along the surf
		m_Color = RGB(255, 150, 150);
	}
	
	m_Stopped = FALSE;
	m_Dead = FALSE;
	m_Pos = Pgl;
	m_Charge = 1; // >0!
	m_Falls++; 
	return m_Power;
}

double CTracer:: GetVangle(CPlate * plate, C3Point V) const // angle of V from normal
{
	C3Point Normal = plate->OrtZ;
	double VproectN = ScalProd(V, Normal);
	C3Point Vn = Normal * VproectN; // V projection on normal
	C3Point Vb = V - Vn; // = const
	double alfa = atan(ModVect(Vb) / ModVect(Vn)); // from Normal
	//if (ModVect(Vb) > 1.e-6) alfa = atan(ModVect(Vn) / ModVect(Vb));// rad, counted from surface
	//else alfa = PI * 0.5;
	return alfa;
}

C3Point CTracer::GetVatPlate(CPlate * plate, C3Point V) const
// local direct
// similar to Plate->GetLocal
{
	C3Point Vat;// in plate CS
	C3Point Normal = plate->OrtZ;
	//double VproectN = ScalProd(V, Normal);
	Vat.X = ScalProd(V, plate->OrtX);
	Vat.Y = ScalProd(V, plate->OrtY);
	Vat.Z = ScalProd(V, Normal); //VproectN;
	return Vat;
}

void CTracer:: WriteParticleFall(C3Point P, CPlate * plate)// write particles n/q/point/power/E/current/angle
{
	if (plate->Solid == FALSE) return;
	//AfxMessageBox("WriteParticleFall");
	int n = plate->Number;
	int q = m_Charge;
	C3Point V0 = m_V;
	double V0abs = ModVect(V0);
	CBTRDoc * pDoc = (CBTRDoc*) m_pDoc;
	//double E = *pDoc->IonBeamEnergy * 1000000;
	double Mass = pDoc->TracePartMass;
	//double V = sqrt(2.* E * Qe / Mass);
	double Energy = Mass * V0abs * V0abs * 0.5 / Qe; // eV
	double power = m_Power;
	double current = power / Energy;
	C3Point Normal = plate->OrtZ;
	double VproectN = ScalProd(V0, Normal);
	C3Point Vn = Normal * VproectN; // V projection on normal
	C3Point Vb = V0 - Vn; // = const
	//C3Point V1 = Vb - Vn; // = V0 - 2*Vn , reflected V
	double alfa = atan(ModVect(Vn) / ModVect(Vb));// rad, counted from the surface

	FILE * fin  = pDoc->ParticlesFile;
	cs.Lock();
	// AfxMessageBox("print");
	fprintf(fin, "%d  %d   %g  %g  %g      %g    %g    %g \n", n, q, P.X, P.Y, P.Z, power, current, alfa);
	cs.Unlock();
}

int CTracer:: GetFallAdd(const C3Point P1, C3Point & P2) // not used now
{
	CBTRDoc * pDoc = (CBTRDoc*) m_pDoc;
	PtrList & PlatesList = pDoc->PlatesList;
	POSITION pos = PlatesList.GetHeadPosition();
	CPlate * plate;
	CPlate * Plate0 = NULL;
	C3Point Pend, Ploc, Ploc0;
	C3Point Last = P2;
	double dist, dist0 = GetDistBetween(P1, P2);
	int n = -1;
	BOOL MetSolid = FALSE;

	while (pos != NULL) {
			plate = PlatesList.GetNext(pos); 
			//if (plate->Solid == FALSE) continue;
			//if (!IsCalculated(plate)) continue; 
			if (plate->Comment.Find("Addit",0) < 0) continue; // check only addit plates

			if (plate->IsCrossed(P1, Last)) { // crossed 
				Ploc = plate->FindLocalCross(P1, Last);
				
				if (plate->WithinPoly(Ploc)) {
					
					Pend = plate->GetGlobalPoint(Ploc);
					dist = GetDistBetween(P1, Pend);
					if (plate->Solid) MetSolid = TRUE;
					//n = plate->Number;
					 
					if (dist < dist0) { // check if plate is closer than previous
						dist0 = dist;
						Last = Pend;
						Plate0 = plate;
						n = Plate0->Number;
						Ploc0 = Ploc;
					}
				
				} // within polygon
			} // crossed 
		} // while pos
	
	if (Last.X > pDoc->AreaLong) { // out of area
		if (m_State == SRCION) m_Dead = TRUE;
		else m_Stopped = TRUE; //
	}

	if (n > 0)  // crossed plate
	{
		P2 = Last;// + (P2-Last) * 0.01; // shift cross-point forward
		
	if (WithinCalculatedLimits(Last)) {

			/*if (m_State == REION) {
				if (m_Step > *pDoc->MinStepL) { // refine step near surface
					P2 = P1;
					m_Step = 0.5 * m_Step;
					m_Stopped = FALSE;
					return -1;
				}
				//else m_Step = pDoc->GetIonStep(m_Pos.X); //*pDoc->IonStepL;
				
			} */

			Plate0->Touched = TRUE;
			// add stoppoint
			C3Point Vat = GetVatPlate(Plate0, m_V);
			double modV = ModVect(m_V);
			double ax = (Vat.X / modV);
			double ay = (Vat.Y / modV);
			minATTR tat(Ploc0, ax, ay, m_Power, n, m_Charge, m_Run); // ax, ay, Ploc are x1000 in minATTR()
			//(Ploc0, m_Power, m_Charge, m_Color, n);
		try {
			tattr->push_back(tat);
		} catch (std::bad_alloc error) {
			long max = tattr->size();
			//AfxMessageBox(A2CT("Failed to increase ion track-vector size"));
			AfxMessageBox("Failed to increase ion track-vector size at plate %d\n Current size %ld", n, max);
		}

		/*if (Pend.X > *pDoc->Duct7X  && Pend.X < *pDoc->Duct8X && m_OptWriteFall) { // option ON
			double angle = GetVangle(Plate0, m_V);  
			FALL_ATTR fall(m_Charge, n, Pend, float(angle), m_Power);
			fattr->push_back(fall); 
		} // fill fallvector at fall*/

	} // within calc limits 
		
		if (MetSolid) { //(Plate0->Solid) {// fall on Solid
			if (m_State == SRCION) m_Dead = TRUE;
			else m_Stopped = TRUE; //SetNextState(); // for ions

			// fill SumPowerX counter if ION is stopped by any solid surface 
			if (m_State == REION) {
				int k = (int) (Last.X / SumPowerStepX);
				if (k < SumReiPowerX.GetSize()) {
					cs.Lock();
					SumReiPowerX[k] += m_Power;
					cs.Unlock();
				}

		/*	if (Pend.X > *pDoc->Duct7X && Pend.X < *pDoc->Duct8X && m_OptWriteFall) { // option ON
				double angle = GetVangle(Plate0, m_V) * 180 / PI;  
				int l = (int) (angle / SumPowerAngleStep);
				if (l < SumPowerAngle.GetSize()) {
					cs.Lock();
					SumPowerAngle[l] += m_Power;
					cs.Unlock();
				}
			} // ON within limits */
			} // REION
					
			if (m_State == ATOM) {
				int k = (int) (Last.X / SumPowerStepX);
				if (k < SumAtomPowerX.GetSize()) {
					cs.Lock();
					SumAtomPowerX[k] += m_Power;
					cs.Unlock();
				}
			} // atom
		} // Solid plate

	} //  crossed plate (n>0)
	

	return n; 
}

void CTracer:: TraceAtomFast() //with decay account, reions are stopped: pDoc->OptReionStop = TRUE
{
	CBTRDoc * pDoc = (CBTRDoc*) m_pDoc;
	if (!pDoc->OptTraceAtoms) {  m_Stopped = TRUE; return; } // to be converted next
	//if (Attr.Current < 1.e-12 && !Central) return;
	
	C3Point P1 = m_Pos;
	C3Point P2, Pstop;

	/*if (m_V.X > 1.e-6) {
		P2.X = *pDoc->AreaLong + 0.2;
		P2.Y = P1.Y + m_V.Y / m_V.X * (P2.X - P1.X);
		P2.Z = P1.Z + m_V.Z / m_V.X * (P2.X - P1.X);
	}*/
	double dist = pDoc->AreaLong + 0.2;
	C3Point Direct = m_V / ModVect(m_V);
	P2 = P1 + Direct * dist;  
	
	m_Stopped = TRUE;

	Pstop = GetAtomSolidFall(P1, P2); // decay is calculated
	if (pDoc->OptDrawPart) 	DrawAtomTrack(P1, Pstop);
	m_Pos = Pstop;
	if (m_Power >= 1.e-12) GetAtomFallsBetween(P1, Pstop); // decay is calculated
	
	
//-------- re-ionisation ----------------
//	if (pDoc->OptReionStop) return;
/*
	ATTRIBUTES AtomAttr = Attr; // keep initial particle 
	AtomAttr.Vx = Attr.Vx; AtomAttr.Vy = Attr.Vy; AtomAttr.Vz = Attr.Vz;
	AtomAttr.Current = Attr.Current; AtomAttr.Power = Attr.Power;
	AtomAttr.L = Attr.L;
	C3Point AtomOrtV = OrtV;
	C3Point AtomOrtH = OrtH;

	double V = GetV(); 

	double ionstep = *pDoc->IonStepL / V;
	double dist, lost;
	C3Point Pion; 

	SetPosIonSort();
	
	for (int i = 0; i <= pDoc->ReionArrayX.GetUpperBound(); i++) {
		Pion.X = pDoc->ReionArrayX[i];
		if (Pion.X >= Pstop.X) return; // beyond atom stop
		Pion.Y = P1.Y + AtomAttr.Vy / AtomAttr.Vx * (Pion.X - P1.X);
		Pion.Z = P1.Z + AtomAttr.Vz / AtomAttr.Vx * (Pion.X - P1.X);
		Attr.X = Pion.X; Attr.Y = Pion.Y; Attr.Z = Pion.Z; 
		dist = GetDistBetween(P1, Pion);
		Attr.L = AtomAttr.L + dist;
		Attr.Vx = AtomAttr.Vx; Attr.Vy = AtomAttr.Vy; Attr.Vz = AtomAttr.Vz; 
		OrtV = AtomOrtV; OrtH = AtomOrtH;
		lost = pDoc->ReionArrayCurr[i];
		Attr.Current = lost * AtomAttr.Current;
		if (Attr.Current < 1.e-100) continue;
		Attr.Power = AtomAttr.Power * lost; 
		AtomAttr.Current -= Attr.Current;
		AtomAttr.Power -= Attr.Power; 
		SetPartAttr(Attr, ionstep);
		TraceIon();
	} // i
*/
}

BOOL CTracer:: IsCalculated(CPlate * plate) const
{
	CBTRDoc * pDoc = (CBTRDoc*) m_pDoc;
	CString S = plate->Comment;
	S.MakeUpper();
	if (S.Find("NEUTRAL", 0) >= 0 && !pDoc->OptCalcNeutralizer) return FALSE;
	if (S.Find("RID", 0) >= 0 && !pDoc->OptCalcRID) return FALSE;
	if (S.Find("DUCT", 0) >= 0 && !pDoc->OptCalcDuct) return FALSE;
	return TRUE;
}

BOOL CTracer:: WithinCalculatedLimits(C3Point & P) const
{
	CBTRDoc * pDoc = (CBTRDoc*) m_pDoc;
	if (P.X < pDoc->CalcLimitXmin || P.X > pDoc->CalcLimitXmax) return FALSE;
	else return TRUE;
}

C3Point CTracer:: GetAtomSolidFall(const C3Point P1, const C3Point P2) const
// used by BTR 4.5
{
	CBTRDoc * pDoc = (CBTRDoc*) m_pDoc;
	PtrList & PlatesList = pDoc->PlatesList;
	POSITION pos = PlatesList.GetHeadPosition();
	CPlate * plate;
	CPlate * Plate0 = NULL;
	// CPlate * Plate1 = NULL;
	C3Point Pend, Ploc, Ploc0, Ploc1;
	C3Point Last1, Last = P2;
	double dist, dist1, dist0 = GetDistBetween(P1, P2);
	int n = -1;
	// int n1 = -1; // closest to n
	
	// double eps = 0.1; // tolerance
	C3Point vect = P2 - P1;
	C3Point Ort = vect /  ModVect(vect);
	double angle;
	//BOOL MetArea = FALSE;
	//minATTR tat;

	while (pos != NULL) {
			plate = PlatesList.GetNext(pos); 
			//if (IsCalculated(plate) == FALSE) continue; 
			//if (plate->Comment.Find("Limit") >= 0) MetArea = TRUE;
			if (!plate->Solid) continue; // including additional plates
			//if (plate->Comment.Find("Addit",0) < 0) continue;

			if (plate->IsCrossed(P1, Last)) { // crossed 
				Ploc = plate->FindLocalCross(P1, Last);
				
				if (plate->WithinPoly(Ploc)) {// within polygon
					
					Pend = plate->GetGlobalPoint(Ploc);
					dist = GetDistBetween(P1, Pend);
							
				/*	if (fabs(dist - dist0) < eps) { // very close to previous
						dist1 = dist;
						Last1 = Pend;// + Ort*eps;
						Plate1 = plate;
						Ploc1 = Ploc;
						n1 = plate->Number;
					} */
					if (dist < dist0) {
						//&& fabs(dist - dist0) >= eps) { // check if plate is closer to P1
						dist0 = dist;
						Last = Pend;// + Ort * eps;
						Plate0 = plate;
						Ploc0 = Ploc;
						n = plate->Number;
					}


				} // within polygon
			} // crossed 
		} // while pos

	/* if (n > 0) { // ChooseOneCross from (n, n1)
		if (n1 > 0 && n1 != n) {
			dist0 = GetDistBetween(P1, Last);
			dist1 = GetDistBetween(P1, Last1);
			if (dist1 < dist0) {
				Last = Last1;
				Plate0 = Plate1;
				Ploc0 = Ploc1;
				n =  n1;
			}
		}
	} */

	
	if (n>0) { // add stoppoint
		double neutr_decay = 1, rei_decay = 1, pl_decay = 1, decay = 1;
		Pend = Last;// - Ort*eps;
		if (pDoc->OptThickNeutralization) 
			neutr_decay = pDoc->GetNeutrDecay(P1, Pend);
		if (pDoc->OptReionAccount) 
			rei_decay = pDoc->GetReionDecay(Pend);
		pl_decay = 1;
		pl_decay = pDoc->GetDecay(Pend);// not used now (4.5)
		decay = neutr_decay * rei_decay * pl_decay;
		if (decay < 0 || decay > 1) AfxMessageBox("GetAtomSolidFall:\n invalid decay");

		//if (m_State == ATOM) {
		int k = (int) (Last.X / SumPowerStepX);
		if (k < SumAtomPowerX.GetSize()) {
			//cs.Lock();
			SumAtomPowerX[k] += m_Power;
			//cs.Unlock();
		}
			

		
	if (IsCalculated(Plate0) && WithinCalculatedLimits(Last)) { 

		Plate0->Touched = TRUE;
		C3Point Vat = GetVatPlate(Plate0, m_V);
		double modV = ModVect(m_V);
		double ax = (Vat.X / modV);
		double ay = (Vat.Y / modV);
		minATTR tat(Ploc0, ax, ay, m_Power*decay, n, m_Charge, m_Run); // ax, ay, Ploc are x1000 in minATTR()
		//double angle = GetVangle(Plate0, m_V);//* 180 / PI); 
		//if (angle > 90) angle = 90;
		//SATTR tat(Ploc0, m_Power * decay, m_Charge, n, angle);
		/*tat.Xmm = (float)Ploc.X; //(short) (pos.X * 1000); //short
		tat.Ymm = (float)Ploc.Y; //(short) (pos.Y * 1000); //short
		tat.PowerW = (float)(m_Power* decay);
		tat.Charge = (signed char)m_Charge;
		tat.Nfall = (unsigned short)n;
		tat.AXmrad = float(angle * 1000);
		tat.AYmrad = float(angle * 1000);*/
		try {
			tattr->push_back(tat);
		} catch (std::length_error) { //(std::bad_alloc error) {
			long max = tattr->size();
			//AfxMessageBox(A2CT("Failed to increase track-vector size"));
			AfxMessageBox("Failed to increase atom track-vector size at plate %d\n Current size %ld", n,max);
		}

		/*if (Pend.X > *pDoc->Duct7X && Pend.X < *pDoc->Duct8X && m_OptWriteFall) { // option ON
			angle = GetVangle(Plate0, m_V);  
			FALL_ATTR fall(m_Charge, n, Pend, float(angle), m_Power * decay);
			fattr->push_back(fall); 
		} // fill fallvector at fall*/

			/*if (Pend.X > *pDoc->Duct7X && Pend.X < *pDoc->Duct8X && m_OptWriteFall) { // option ON
				double angle = GetVangle(Plate0, m_V) * 180 / PI;  
				int l = (int) (angle / SumPowerAngleStep);
				if (l < SumPowerAngle.GetSize()) {
					//cs.Lock();
					SumPowerAngle[l] += m_Power * decay;
					//cs.Unlock();
				}
			} // within limits */
	}// within calc limits

	} // n>0

	return Last;
}

void CTracer:: GetAtomFallsBetween(const C3Point P1, const C3Point P2) const 
// used by BTR 4.5
// check transparent falls
{
	CBTRDoc * pDoc = (CBTRDoc*) m_pDoc;
	PtrList & PlatesList = pDoc->PlatesList;
	POSITION pos = PlatesList.GetHeadPosition();
	CPlate * plate;
	C3Point Ploc, Pcross = P1;
	//double dist, dist0 = GetDistBetween(P1, P2);
	int n = -1;
	double neutr_decay = 1, rei_decay = 1, pl_decay = 1, decay = 1;
	CString S;
	//minATTR tat;

	while (pos != NULL) {
			plate = PlatesList.GetNext(pos); 
			if (!IsCalculated(plate)) continue; 
			if (plate->Solid) continue; // check only transparent including addit plates
			//if (plate->Comment.Find("Limit") >= 0) continue; //MetSolid = TRUE;

			if (plate->IsCrossed(P1, P2)) { // crossed 
				Ploc = plate->FindLocalCross(P1, P2);
				
				if (plate->WithinPoly(Ploc)) {
				// within polygon
					Pcross = plate->GetGlobalPoint(Ploc);
					n = plate->Number;

				if (WithinCalculatedLimits(Pcross)) {
					plate->Touched = TRUE;
					neutr_decay = 1;
					if (pDoc->OptThickNeutralization) 
						neutr_decay = pDoc->GetNeutrDecay(P1, Pcross);
					if (pDoc->OptReionAccount)
						rei_decay = pDoc->GetReionDecay(Pcross);
					pl_decay = pDoc->GetDecay(Pcross);// not used now (4.5)
					decay = neutr_decay * rei_decay * pl_decay;
					if (decay < 0 || decay > 1) AfxMessageBox("GetAtomFallsBetween: \n Invalid decay");
										
					//S.Format("power=%g  at(%g/%g/%g)", m_Power * decay, Pcross.X, Pcross.Y, Pcross.Z);
					//AfxMessageBox(S);
					//SATTR tat(Ploc, m_Power * decay, m_Charge, m_Color, n);
					
					C3Point Vat = GetVatPlate(plate, m_V);
					double modV = ModVect(m_V);
					double ax = (Vat.X / modV);
					double ay = (Vat.Y / modV);
					minATTR tat(Ploc, ax, ay, m_Power * decay, n, m_Charge, m_Run); // ax, ay, Ploc are x1000 in minATTR()
					//double angle = GetVangle(plate, m_V);// * 180 / PI); 
					//if (angle > 90) angle = 90;
					/*tat.Xmm = (float)Ploc.X; //(short) (pos.X * 1000); //short
					tat.Ymm = (float)Ploc.Y; //(short) (pos.Y * 1000); //short
					tat.PowerW = (float)(m_Power* decay);
					tat.Charge = (signed char)m_Charge;
					tat.Nfall = (unsigned short)n;
					tat.AXmrad = float(angle * 1000);
					tat.AYmrad = float(angle * 1000);*/
					//tat = SATTR(Ploc, m_Power * decay, m_Charge, n, angle);
					//tattr->push_back(tat);
					try {
						tattr->push_back(tat);
					} catch (std::bad_alloc error) {
						long max = tattr->size();
						//AfxMessageBox(A2CT("Failed to increase track-vector size"));
						AfxMessageBox("Failed to increase atom track-vector size at plate %d\n Current size %ld", n,max);
						break;
					}

				} // within calc limits
					
					
			/*		S = plate->Comment;

					if (m_Charge == 0 && S.Find("Duct") > -1) { //Duct exit - for tracing in plasma
						EXIT_RAY ray(Pcross, m_V, m_Power * decay);
						exitarr->push_back(ray); 
					} // fill exitvector at duct exit plane
			*/		
				} // within polygon
			} // crossed 
		} // while pos

}

void CTracer:: DrawAtomTrack(const C3Point P1, const C3Point P2) 
{
	CMainView* pMV = (CMainView*)m_pViewWnd; 
	CDC* pDC = pMV->GetDC(); // also works!
	
	//CClientDC dc(pMV);
	//pMV->OnPrepareDC(&dc);
	pDC->SelectObject(pMV->AtomPen);
	CPoint p1, p2;
	
	cs.Lock();
	p1.x = pMV->OrigX + (int) (P1.X * (pMV->ScaleX));
	p1.y = pMV->OrigY - (int) (P1.Y * (pMV->ScaleY));
	pDC->MoveTo(p1); //SetPixel(pos, Color);
	p2.x = pMV->OrigX + (int) (P2.X * (pMV->ScaleX));
	p2.y = pMV->OrigY - (int) (P2.Y * (pMV->ScaleY));
	pDC->LineTo(p2);
		 
	p1.y = pMV->OrigZ - (int) (P1.Z*(pMV->ScaleZ));
	pDC->MoveTo(p1);
	p2.y = pMV->OrigZ - (int) (P2.Z*(pMV->ScaleZ));
	pDC->LineTo(p2);
	pMV->ReleaseDC(pDC);
	//pDC.SelectObject(pOldpen);
	::GdiFlush();
	cs.Unlock();
}

CCriticalSection CTracer::cs;