// RIDDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BTR.h"
#include "RIDDlg.h"



// CRIDDlg dialog

IMPLEMENT_DYNAMIC(CRIDDlg, CDialog)

CRIDDlg::CRIDDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CRIDDlg::IDD, pParent)
	, m_EpsLimit(1.e-6)
	, m_IterLimit(10000)
	, m_Iter(0)
	, m_Eps(0)
	, m_StepX(0.01f)
	, m_StepY(0.01f)
	, m_Nx(0)
	, m_Ny(0)
	, m_account(FALSE)
{
	Nx = Ny = 0;
	Wrelax = 0.5;
	EpsLimit = 1.e-6;
	IterMax = 100000;
	Iter = 0;
	Eps = 0;
	STOP = TRUE;
	CONTINUE = FALSE;

}

CRIDDlg::~CRIDDlg()
{
	if (Nx * Ny > 0) DeleteFields();
}

void CRIDDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EpsLimit, m_EpsLimit);
	DDV_MinMaxDouble(pDX, m_EpsLimit, 0, 1000);
	DDX_Text(pDX, IDC_IterLimit, m_IterLimit);
	DDV_MinMaxLong(pDX, m_IterLimit, 0, 1000000);
	DDX_Text(pDX, IDC_Iter, m_Iter);
	DDX_Text(pDX, IDC_Eps, m_Eps);
	DDX_Text(pDX, IDC_StepX, m_StepX);
	DDX_Text(pDX, IDC_StepY, m_StepY);
	DDX_Text(pDX, IDC_Nx, m_Nx);
	DDX_Text(pDX, IDC_Ny, m_Ny);
	DDX_Check(pDX, IDC_RIDcharge, m_account);
}


BEGIN_MESSAGE_MAP(CRIDDlg, CDialog)
	ON_BN_CLICKED(IDC_Calculate, &CRIDDlg::OnBnClickedCalculate)
	ON_BN_CLICKED(IDC_Stop, &CRIDDlg::OnBnClickedStop)
	ON_BN_CLICKED(IDC_Write, &CRIDDlg::OnBnClickedWrite)
END_MESSAGE_MAP()


// CRIDDlg message handlers

void CRIDDlg:: SetDocument(CBTRDoc * doc)
{
	pDoc = doc;
}

void CRIDDlg:: DeleteFields()
{
	if (Nx * Ny < 1) return;
	int i;
	for (i = 0; i <= Nx; i++) {
		delete [] NodeU[i];
	}
	delete [] NodeU;

	for (i = 0; i <= Nx; i++) {
		delete [] NodeUnew[i];
	}
	delete [] NodeUnew;

	for (i = 0; i <= Nx; i++) {
		delete [] NodeQ[i];
	}
	delete [] NodeQ;

	for (i = 0; i <= Nx; i++) {
		delete [] NodeState[i];
	}
	delete [] NodeState;
}

void CRIDDlg:: SetGeometry() // size, steps, Eps, Coeff
{
	UpdateData(1);

	Yground = 0; //config.GroundY - config.FieldMinY;
	Xin = pDoc->RIDInX; Xout = pDoc->RIDOutX; //RID plate
	Thickness = pDoc->RIDTh; //RID plate thickness
	Coeff = pDoc->RIDU * 1000;//config.RID_potential;

	Y0 = 0; //config.FieldMinY;
	double gap = pDoc->RIDInW + Thickness;
	Ymax = gap; //config.FieldMaxY - config.FieldMinY;

	X0 = pDoc->NeutrOutX - 3*gap; //X0 = Xin - 4*gap;// Xmin
	//if (X0 < *pDoc->NeutrOutX) X0 = *pDoc->NeutrOutX; //config.FieldMinX;
	Xmax = Xout + 3*gap;//*pDoc->CalInX; //config.FieldMaxX - config.FieldMinX;

	StepX = m_StepX; //config.FieldStepX;
	StepY = m_StepY; //config.FieldStepY;
	
	int Nx_ = (int)ceil((Xmax - X0)/StepX);
	int Ny_ = (int)ceil((Ymax - Y0)/StepY);
	//m_Nx = Nx_; m_Ny = Ny_;
	//UpdateData(0);

	if (Nx_== Nx && Ny_== Ny) { 
		CONTINUE = TRUE; return;
	}// already allocated
	
	else { 
		DeleteFields();
		Nx = Nx_; Ny = Ny_; 
		Iter = 0;
		Eps = 0;
	} 
	
	int i;

	NodeU = new double * [Nx+1];
	for (i = 0; i <= Nx; i++) {
		NodeU[i] = new double [Ny+1];
	}

	NodeUnew = new double * [Nx+1];
	for (i = 0; i <= Nx; i++) {
		NodeUnew[i] = new double [Ny+1];
	}

	NodeQ = new double * [Nx+1];
	for (i = 0; i <= Nx; i++) {
		NodeQ[i] = new double [Ny+1];
	}

	NodeState = new int * [Nx+1];
	for (i = 0; i <= Nx; i++) {
		NodeState[i] = new int [Ny+1];
	}

}

int CRIDDlg:: GetState(double x, double y) // in/out AREA, boundary
{
	int i = (int) floor(x/StepX);
	int j = (int) floor(y/StepY);
	// double dx = x - i*StepX;
	// double dy = y - j*StepY;

	if (x < 0.0 || x > Xmax || y < 0 || y > Ymax ) return -1;
	if (x ==0 || x == Xmax) return 1;
	if (y == 0) return 2; 
	if (y == Ymax) return 3; 

	if (NodeState[i][j] == 10 && NodeState[i+1][j+1] == 10) return 10;
	if (NodeState[i][j] == 11 && NodeState[i+1][j+1] == 11) return 11;
	
	return 0;
}

int CRIDDlg:: GetState(C3Point P)
{
	return GetState(P.X, P.Y);
}

void CRIDDlg:: Init() // set Nodes state/potential before calculation
{
	EpsLimit = m_EpsLimit;
	IterMax = m_IterLimit;
	if (CONTINUE) return;
	
	delta = 0.5 * Thickness;
	double x,y;
	
	for (int i = 0; i <= Nx; i++) {
		x = X0 + StepX * i;
		for (int j = 0; j <= Ny; j++) {
			NodeState[i][j] = 0;
			NodeU[i][j] = 0.0;
			NodeQ[i][j] = 0.0;

			if (i == 0 || i == Nx) {
				NodeState[i][j] = 1; // U = 0 left, right
				NodeU[i][j] = 0.0;
			}

			if (j == 0) NodeState[i][j] = 2; // bottom - 0 !symmetry
			if (j == Ny) NodeState[i][j] = 3; // top - symmetry
			
			if (x >= Xin && x <= Xout) {
				y = Y0 + StepY * j;
				if (y >= Yground - delta && y <= Yground + delta) {
					NodeState[i][j] = 10; // Urid = 0
					NodeU[i][j] = 0.0;
				}
				if (y >= Ymax - delta) {
					NodeState[i][j] = 11; // Urid = Coeff
					NodeU[i][j] = Coeff;
				}
				if (y > Yground + delta && y < Ymax - delta){
					NodeState[i][j] = 0;
					NodeU[i][j] = Coeff * (y - (Yground + delta)) / (Ymax - Y0 - 2*delta);
				}
			}
			
			NodeUnew[i][j] = NodeU[i][j];

		}
	}
}

void CRIDDlg:: UpdateU()
{
	for (int i = 0; i <= Nx; i++) 
		for (int j = 0; j <= Ny; j++) 
			NodeU[i][j] = NodeUnew[i][j];
}

double CRIDDlg:: SetU(double u0, double u1, double u2, double u3, double u4, double f0) // new potential in node
{
	double W = Wrelax;
	double Unew;
	double A = StepY*StepY;//1/StepX/StepX;
	double B = A; //1/StepX/StepX;
	double C = StepX*StepX;//1/StepY/StepY;
	double D = C;//1/StepY/StepY;
	double E = 2 * (A + C);
	double F = f0*A*C; //
	double Res = A*u1 + B*u2 + C*u3 + D*u4 - E*u0 - F;
	double dU = W/E * Res;
	Unew =  u0 + dU;
	if (Unew > 0) Unew = 0; /// Compensation by electrons!!!
	//if (Unew < Coeff) Unew = Coeff; // Slow pos ions saturation
	dU = Unew - u0;
	Eps = Max(Eps, fabs(dU/Coeff));
	return Unew;
}

void CRIDDlg:: CalculateNode(int i, int j)
{
	double Unew = 0;
	double c = 3.e+8; // light velocity
	double	EPSILO = 1.e+7 / 4 / PI /c/c; // eps0
	double f = - NodeQ[i][j] / EPSILO; // 0 - LAPLAS; (-ro/eps0/Coeff) - POISSON
	switch (NodeState[i][j]) {
			case 10: //RID plate
				Unew = 0; break; 
			case 11: //RID plate U = Coeff
				Unew = Coeff; break; 
			case 1: // X-limits
				Unew = 0; break; 
			case 4: // left bound
				Unew = SetU(NodeU[i][j], NodeU[i+1][j], NodeU[i+1][j], NodeU[i][j-1], NodeU[i][j+1], f); 
				break;
			case 5: // right bound
				Unew = SetU(NodeU[i][j], NodeU[i-1][j], NodeU[i-1][j], NodeU[i][j-1], NodeU[i][j+1], f); 
				break;
			case 2: // bottom 
				Unew = 0;
				//Unew = SetU(NodeU[i][j], NodeU[i-1][j], NodeU[i+1][j], NodeU[i][j+1], NodeU[i][j+1], f); 
				break;
			case 3: // top 
				//if (i > 0 && i < Nx)	
				Unew = SetU(NodeU[i][j], NodeU[i-1][j], NodeU[i+1][j], NodeU[i][j-1], NodeU[i][j-1], f); 
				//else if (i == 0) Unew = SetU(NodeU[i][j], NodeU[i+1][j], NodeU[i+1][j], NodeU[i][j-1], NodeU[i][j-1], f); 
				//else Unew = SetU(NodeU[i][j], NodeU[i-1][j], NodeU[i-1][j], NodeU[i][j-1], NodeU[i][j-1], f); 
				break;
			case 0: // inner point 
				Unew = SetU(NodeU[i][j], NodeU[i-1][j], NodeU[i+1][j], NodeU[i][j-1], NodeU[i][j+1], f); 
				break;

			default: break;
			} //case

	NodeUnew[i][j] = Unew;
}

void CRIDDlg:: Calculate()
{
//	UpdateData(0);
	MSG message;
	Eps = 0;
	STOP = FALSE;
	int i,j;
	int iter = 0;
	Iter  += iter; 
	
	
	while (!STOP) {
		Eps = 0;
		for (i = 1; i < Nx; i++) 
		for (j = 0; j <= Ny; j++) 
				CalculateNode(i,j); // Unew
		//Compensate();
		UpdateU(); //SetLaplas(FALSE); // U = Unew
//------------------------------------------
		for (i = Nx-1; i > 0; i--) 
		for (j = Ny; j >= 0; j--) 
				CalculateNode(i,j);
		//Compensate();
		UpdateU(); //SetLaplas(FALSE);
//------------------------------------------
		for (j = 0; j <= Ny; j++) 
		for (i = 1; i < Nx; i++) 
				CalculateNode(i,j);
		//Compensate();
		UpdateU(); //SetLaplas(FALSE);
//------------------------------------------
		for (j = Ny; j >= 0; j--) 
		for (i = Nx-1; i > 0; i--) 
				CalculateNode(i,j);
		//Compensate();
		UpdateU(); //SetLaplas(FALSE);
//------------------------------------------		
	iter++;
	Iter++; 
	
//	if (1000 * (int)(Iter/1000) == Iter) Write();
	
	if (Eps <= EpsLimit || iter >= IterMax) STOP = TRUE;
	if (iter < 2) STOP = FALSE;
	
		m_Nx = Nx; m_Ny = Ny;
		m_Iter = Iter; //	UpdateData(0);
		if (Iter % 100 == 0 || STOP)  {
			m_Eps = (float)Eps; //UpdateData(0);
		}
		
		UpdateData(0);
		
		if (::PeekMessage(&message, NULL, 0,0, PM_REMOVE)) {
		::TranslateMessage(&message);
		::DispatchMessage(&message);
		} // stopped by user
	} // while !STOP

//	Compensate();

}

void CRIDDlg:: Compensate()
{
	for (int i = 0; i <= Nx; i++) 
		for (int j = 0; j <= Ny; j++)
		if (NodeUnew[i][j] > 0) NodeUnew[i][j] = 0; // compensate positive potential

}

void CRIDDlg:: Clear_Q()
{
	for (int i = 0; i <= Nx; i++) 
		for (int j = 0; j <= Ny; j++)
			NodeQ[i][j] = 0;
}

void CRIDDlg:: Distribute_Q(double Q, C3Point oldPos, C3Point pos)
{
	C3Point P = (oldPos + pos) * 0.5;
	C3Point Plocal = P;//ConvertToLocal(P);
	int i = (int) floor(Plocal.X/StepX);
	int j = (int) floor(Plocal.Y/StepY);
	if (i<0 || j<0) return;
	double dx = Plocal.X - i*StepX;
	double dy = Plocal.Y - j*StepY;
	double left = (StepX - dx) / StepX;
	double right = dx / StepX;
	double S = StepX * StepY;
	NodeQ[i][j] += Q * left * (StepY - dy) / StepY / S;
	NodeQ[i][j+1] += Q * left * dy / StepY / S;
	NodeQ[i+1][j] += Q * right * (StepY - dy) / StepY / S;
	NodeQ[i+1][j+1] += Q * right * dy / StepY / S;

}

void CRIDDlg:: WriteTab(char * name)
{
	CString n = name;
	FILE *fout = fopen(name, "w");
	fprintf(fout, " %d  %d # RID field \n", Nx, Ny);
	fprintf(fout, " %g  %g \n", X0, StepX);
	fprintf(fout, " %g  %g \n", Y0, StepY);
	fprintf(fout, " 0.0  %g \n", fabs(Coeff));//Scell Zmax
//	fprintf(fout, " i   j   Uij \n");
	for (int j=0; j <= Ny; j++) {
	for (int i=0; i <= Nx; i++) fprintf(fout, "%le\t", fabs(NodeU[i][j])); 
		fprintf(fout, "\n");
	}
	fclose(fout);
	AfxMessageBox(n+ " is written");
}

double CRIDDlg:: GetUnode(int i, int j, int type)
{
	if (i<0 || i>Nx || j<0 || j>Ny) return 0;
	switch (type) {
	case 0: return NodeU[i][j];
	case 1: return (NodeUnew[i][j] - NodeU[i][j]);
	case 2: return NodeUnew[i][j];
	default: return 0;
	}
}

C3Point CRIDDlg:: ConvertToLocal(C3Point Pglobal)
{
	C3Point Plocal;
	Plocal.X = Pglobal.X - X0;
	Plocal.Y = Pglobal.Y - Y0;
	return Plocal;
}

C3Point CRIDDlg:: GetEloc(double x, double y) // Ex, Ey, U in global point
{
	C3Point E(0,0,0);
	C3Point Ploc = C3Point(x, y, 0);
//	C3Point Ploc = ConvertToLocal(Pgl);
//	return E;

	int i = (int) floor(Ploc.X/StepX);
	int j = (int) floor(Ploc.Y/StepY);
	if (i<=0 || i>=Nx || j<=0 || j>=Ny) return E;
	double dx = Ploc.X - i*StepX;
	double dy = Ploc.Y - j*StepY;
	if (dx < 0) { i--; dx = Ploc.X - i*StepX; } 
	if (dy < 0 ) { j--; dy = Ploc.Y - j*StepY; } 
	if (dx >= StepX) { i++; dx = Ploc.X - i*StepX; } 
	if (dy >= StepY) { j++; dy = Ploc.Y - j*StepY; } 
	double u00, u10, u01, u11;
	int state = GetState(Ploc.X, Ploc.Y);
	switch (state){
		case -1: // out of area
				break;
		case 10: //RID plate U = 0
				E.Z = 0; break;
		case 11: //RID plate U = 1
				E.Z = Coeff; break;
		case 1: // X-limits
				break; 
		case 2: // bottom U = 0
			E.X = 0;
			E.Y = -(NodeUnew[i][1] - NodeUnew[i][0]) / StepY; 
			E.Z = 0;
				break;
		case 3: // top 
			E.X = -(NodeUnew[i+1][Ny] - NodeUnew[i][Ny]) / StepX;
			E.Y = 0;//-(NodeU[i][Ny] - NodeU[i][Ny-1]) / StepY; 
			E.Z = NodeUnew[i][Ny] + (NodeUnew[i+1][Ny] - NodeUnew[i][Ny]) * dx / StepX;
				break;
		case 0:
			u00 = NodeUnew[i][j];	u10 = NodeUnew[i+1][j];
			u01 = NodeUnew[i][j+1]; u11 = NodeUnew[i+1][j+1];
			E.X = - (u10 - u00 + (u11 - u10 - u01 + u00) *dy/StepY) / StepX;
			E.Y = - (u01 - u00 + (u11 - u01 - u10 + u00) *dx/StepX) / StepY;
		//	E.X = -(u10 + u11 - u00 - u01) * 0.5 / StepX;
		//	E.Y = -(u01 + u11 - u00 - u10) * 0.5 / StepY;
		//	E.X = -(u10 - u00) / StepX; E.Y = -(u01 - u00) / StepY;
			E.Z =  (u00 + (u10 - u00) * dx/StepX) * (StepY - dy)/StepY + (u01 + (u11 - u01) * dx/StepX) * dy/StepY;
			break;
	}
	return E;
}

C3Point CRIDDlg:: GetEloc(C3Point Ploc) // Ex, Ey, U in global point
{
	return GetEloc(Ploc.X, Ploc.Y);
}

void CRIDDlg::OnBnClickedCalculate()
{
	CONTINUE = FALSE;
	SetGeometry();
	Init();
	Calculate(); // laplas

	if (m_account) { // account space charge
	for (int k = 0; k < 5; k++) { // volume charge iterations
		Clear_Q();
		EmitBeam(-1);
		EmitBeam(1);
		Calculate();
	}
	} // account

}

void CRIDDlg::OnBnClickedStop()
{
	STOP = TRUE;
	m_Eps = (float) Eps;
	m_Iter = Iter;
	UpdateData(0);
}

void CRIDDlg::OnBnClickedWrite()
{
	STOP = TRUE;
	WriteTab("U.txt");
}

// Emit beams ------------------------------------

void CRIDDlg:: EmitBeam(int icharge)
{
	CWaitCursor wait;
	double dL = Min(StepX, StepY) * 0.2; // trace step
	double dS = StepY * 0.2; // emit step
	double E = pDoc->IonBeamEnergy * 1000000; // eV
	double SrcCurr = pDoc->IonBeamCurrent;//A
	double Mass = pDoc->TracePartMass;
	C3Point Vstart(0,0,0);
	Vstart.X = sqrt(2 * E * Qe / Mass);
	double dT = dL / Vstart.X;
	double BeamW = pDoc->RIDInW; //Ymax - Thickness;
	double BeamH = pDoc->RIDH;// beam size
	double fract;
	if (icharge < 0) fract = 1.0 - pDoc->NeutrPart - pDoc->PosIonPart;
	else fract = pDoc->PosIonPart;
	double CurrDens = SrcCurr * fract / BeamW / BeamH;
	double PartCurr = CurrDens * dS;
	double PartQ = icharge * PartCurr * dT; // carried by BigParticle

	
	double Ystart = Thickness * 0.5;
	C3Point Start(0, Ystart, 0);
	while (Ystart <= Ymax - Thickness * 0.5) {
		Trace(Start, Vstart, icharge, Mass, dT, PartQ);
		Ystart += dS;
	}
	//AfxMessageBox("Emitted");
}

void CRIDDlg:: Trace(C3Point Start, C3Point Vstart, int icharge, double Mass, double dT, double PartQ)
{
	BOOL finished = FALSE; // particle
	double Path = 0;
	double MaxPath = 5;
	int Count = 0;
	int MaxCount = 1000;
	C3Point Pos0 = Start;
	C3Point Vel0 = Vstart;

	while (!finished) { // do one step
		Count++;
		C3Point E = GetEloc(Pos0);
		E.Z = 0; //GetE.Z = U!!!
		C3Point A = E * Qe * icharge / Mass;
		C3Point Vel = Vel0 + A * dT;
		C3Point dPos = (Vel0 + Vel) * dT * 0.5;
		C3Point Pos = Pos0 + dPos;
		Path += ModVect(dPos);
		Distribute_Q(PartQ, Pos0, Pos);
		Pos0 = Pos; Vel0 = Vel;
		finished = IsStopped(Pos);
		if (Path > MaxPath) finished = TRUE;
		if (Count > MaxCount) finished = TRUE;
	}
}


BOOL CRIDDlg:: IsStopped(C3Point Pos)
{
	if (Pos.X < 0) return TRUE; 
	if (Pos.X >= Xmax - X0) return TRUE; 
	if (Pos.Y <= 0 || Pos.Y >= Ymax - Y0) return TRUE; 
	//if (Pos.X >= Xin - X0 &&  Pos.X < Xout - X0 && Pos.Y >= Ymax - Thickness * 0.5 - Y0) return TRUE; 
	//if (Pos.X >= Xin - X0 &&  Pos.X < Xout - X0 && Pos.Y <= Thickness * 0.5 - Y0) return TRUE;
	
	return FALSE;
}