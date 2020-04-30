#ifndef __partfld_h__
#define __partfld_h__

#include <vector>

//////////   Global operations for Particles, Fields, Config ------------------------------------------------ 
class CBTRDoc;
void SetDocument(CBTRDoc * doc);
void  SetConfig(PtrList* List);
//double DistPointToMag(double X, double Y, double dist0, double t);
double Gauss(double argum, double alfa, double alfaH, double partH);
double SumGauss(double sum, double alfa); 
double SumGauss(double sum, double alfa, double alfaH, double partH);
double GaussDensity(double argum, double alfa, double alfaH, double partH);
void  Decil(int PolarNumber, int type, double * PolarArray, double * CurrentArray, double alfa, double alfaH, double partH);
C3Point Bstray(double X, double Y, double Z);
void DrawMF(CDC* pDC, double y, double z);
void SetReionArea(double xmin, double xmax);
//void SetBstrayArea(double xmin, double xmax);
 //void ReadMFData();
 C3Point GetMF(double x);
 double Bremnant(double X);
 void ShowTestField();
 void SetReionSigma();
 void SetNeutrSigma(double NeutrPart, double NeutrXmax);
 //void ReadGas();
 void ShowGas(CDC* pDC);
 double GetDensity(double x);
 void DeleteArrays();
 void SortDoubleArray(CArray<double, double> & Arr);
 int GetDoubleArrayPos(CArray <double, double> & Arr, double elem);

///// Particles --------------------------------------------------------------
	#define	   Qe		(double)   1.602192e-19  // {Q}
    #define	   Mp		(double)   1.672614e-27 // {kg}
	#define	   Me		(double)	9.109558e-31 // (kg)
   
enum MODELofMOVE {SLOW, FAST} static; // Particle moving calculation "style" 
enum STATE {STOPPED, MOVING, CHANGING} static; // Set of Particles States  
enum SORT {  NeuD, NeuH,   // neutral component
			 NegIonD, NegIonH,
			 PosIonH,  PosIonD,    
			 Electron } static; // 

struct ATTRIBUTES // Attributes of a Particle
{
	double X, Y, Z, L;
	double Vx, Vy, Vz;
	double StartCurrent, Current;
	double dCurrY, dCurrZ;
	double StartPower, Power;
	STATE State;
	MODELofMOVE Model;
};

typedef std::vector<ATTRIBUTES> TVectorAttr;

double RelMass(double EMeV); // relativistic electron
double RelV(double EMeV); // relativistic electron
//double IntegralErr(double Lim, double div);


struct BEAMLET_ENTRY // SINGAP Beamlets
{
	double PosY, PosZ; // hor/vert positions
	double AlfY, AlfZ; // hor/vert aimings
	double DivY, DivZ; // divergence
	double Fraction; // current
	int i, j;
	BOOL Active;
};

struct BEAM_RAY // to inject to TARGET
{
	double EeV;// energy eV total
	int A;
	double Vmod;// calc from E
	double pow; // Power0 = 1 default
	C3Point Pos; // X, Y, Z
	C3Point Vn; // Vx Vy Vz normalized 
	BEAM_RAY() : EeV(0), A(1), Vmod(1), pow(1), Pos(), Vn() {}
	BEAM_RAY(double e, int a, C3Point pos, C3Point vn)
	{
		EeV = e;
		A = a;
		Vmod = sqrt(2 * Qe*EeV / A / Mp);
		pow = 1; // default
		Pos = pos;
		Vn = vn;
	}
};

typedef CArray<BEAM_RAY> BeamArray1D;
//typedef CArray<BeamArray1D> BeamMatrix2D;

struct RECT_DIA // rectangular windows accross the beam axis (similar to PDP)
{
	C3Point Corn[4];
	int Number; // = PDP notation
	int Channel;
};


struct VOLUME // empty volume bounded by 4(6) plates
{
	C3Point lb0, ru0, lb1, ru1;
	int Ncomp;
	CPlate * plate[6];
	VOLUME()
	{
		Ncomp = 0;
		for (int i = 0; i < 6; i++) plate[i] = NULL;
	}
	VOLUME(const VOLUME & v)
	{
		lb0 = v.lb0; ru0 = v.ru0;
		lb1 = v.lb1; ru1 = v.ru1;
		Ncomp = v.Ncomp;
		for (int i = 0; i < 6; i++) plate[i] = v.plate[i];
	}
	VOLUME(C3Point LB0, C3Point RU0, C3Point LB1, C3Point RU1, int n)
	{
		lb0 = LB0; ru0 = RU0;
		lb1 = LB1; ru1 = RU1;
		Ncomp = n;
	}
};

class CParticle : public CObject
{
public:
	int Charge;
	double Mass;
	double TimeStep;
//	double Power; // =  BeamletPower * Attr.Current
	ATTRIBUTES Attr;
	SORT Sort;
	COLORREF Color;
//	CBTRDoc * pDoc;
	BOOL Central;
	double Vstart;

	C3Point OrtH, OrtV;

public:
	CParticle();
//	~CParticle();
	double RandAttr(double step);
	double SetV(double Ekev); //sqrt(2.* EkeV*1000 * Qe/mass); // {m/c}
	double GetV();// sqrt(Vx*Vx+Vy*Vy+Vz*Vz)
	void SetPartSort(enum SORT sort);
	void SetPartType(int type);
	void SetPartAttr(ATTRIBUTES Attr0,  double TStep);
	void SetOrts();// replaced by SetOrtsNew
	void SetAtomSort();
	void SetPosIonSort();
   	void MoveOneTimeStep();
	void MoveOneStep(double dX);
	void BackToX(double X, C3Point Prev);
	void DrawParticle(); //(CView* pView, CDC* pDC);
	void DrawParticle(COLORREF color);
	void DrawParticleTrack(C3Point P1, C3Point P2);
	void TraceParticle(double  power);
	int  GetNeutrPointNumber(C3Point P1, C3Point P2);

	void TraceElectron();
	void TraceSourceIon(double Xmax);
	void TraceSourceIonWithNeutralization();
	void TraceAtomFast();
	void TraceIon();
	void EmitPosIon(int n);
	void EmitAtom(int n);

	BOOL IsVolumeEmpty(C3Point P);
	int  GetPartState(C3Point P1, C3Point P2);
	C3Point GetFirstSolid(C3Point P1, C3Point P2); 
	void LoadsBetween(C3Point P1, C3Point P2); 
	CParticle * EmitParticle(SORT psort, CParticle* Parent, double lost,  double TStep);
	void DecreaseCurrent(double lost);
	
	//int Drive();
	//void FastMove();
	//void SlowMove();
	//void ProcessChanging();//
	//int Reionisation();
	//int ThinNeutralisation();
	//int FullNeutralisation(); // for Thick Neutralisation started at GG // AIK proposal 27/28 Nov.2000
};
class CMagField // 1D field  Bx By Bz
{
public:
	 CArray <double, double> ArrX;
	 CArray <C3Point, C3Point> ArrB;
	 double Xmin, Xmax, Bmax;
	 int Nx;
 public:
	 CMagField();
	 ~CMagField();
	 void Clear();
	 // CString OpenDefault(char* name, double Mult);
	//int Init(char * name); // define + set geometry
	int ReadData(char * name); // fill array
	C3Point GetB(double x);
	C3Point GetB(C3Point P);
	void DrawMF(CDC* pDC, C3Point & Origin, C3Point & Scale);
	
 };
/*public:
	int Nx, Nz;
	double StepX1, StepX2, StepZ;
	C3Point **  ValB; 
	double Xtr, Xmax, Bmax, Zmin, Zmax; // Xtr - point of Step1-Step2 change
public:
	CMagField();
	~CMagField();
	void Init(int nz);
	void SetGeometry(double stepx1, double stepx2, double stepz, double xtr);
	CString OpenDefault(char* name, double Mult);
	int ReadData(FILE* fin); 
	void SmoothB();
	C3Point  GetB(double x, double z);
	void DrawMF(CDC* pDC, C3Point & Origin, C3Point & Scale, double Zoom);
};*/

class CMagField3D // 3D-field B(x,y,z)
{
	public:
	int Nx, Ny, Nz;
//	double StepX, StepY, StepZ; // - var
	C3Point ***  ValB; 
	CArray <double, double> ArrX;
	CArray <double, double> ArrY;
	CArray <double, double> ArrZ;
	double Xmin, Xmax, Ymin, Ymax, Zmin, Zmax, Bmax; // 
public:
	CMagField3D();
	~CMagField3D();
	int Init(char * name); // define + set geometry
	int ReadData(char * name); // fill array
	C3Point GetB(double x, double y, double z);
	C3Point GetB(C3Point P);
	void DrawMF(CDC* pDC, C3Point & Origin, C3Point & Scale, double xmax, double Z);
	C3Point GetB_XZ(double x, double z);
};

 class CGasField // 1D,  variable stepX 
 {
 public:
	 CArray <C3Point, C3Point> GasArray;
	 double Xmin, Xmax, Pmax;
	 int Nx;
 public:
	 CGasField();
	 ~CGasField();
	 void Clear();
	 //CString OpenDefault(char * name, double Mult);
	 int ReadData(char * name); 
	 double  GetPressure(double x);
	 void DrawGas(CDC* pDC, C3Point & Origin, C3Point & Scale);
 };

 class CGasFieldXZ // 2D,  variable stepX 
 {
 public:
	 //CArray <C3Point, C3Point> GasArray;
	 double ** ValN; // density
	 double StepX, StepZ;
	 double Xmin, Xmax, Zmin, Zmax, ValNmax;
	 int Nx, Nz;
 public:
	 CGasFieldXZ();
	 ~CGasFieldXZ();
	 int Init(char * name); // define + set geometry
	 int ReadData(char * name); // fill array
	 double  GetDensity(double x, double z);
	 void DrawGas(CDC* pDC, C3Point & Origin, C3Point & Scale);
 };

class CRIDField
{
public:
	int Nx, Ny;
	double ** NodeU;
	double hx, hy;
	double Umax;
	double Xmin, Xmax, Ymin, Ymax; // in global CS
//	double Height,  InW, OutW, InX, OutX, Thickness;
public:
	CRIDField();
	~CRIDField();
	void DeleteArray();
/*	void Set();
	void Set(double RIDU,  double RIDH, 
			double  RIDInW, double RIDOutW, 
			double RIDInX, double RIDOutX, 
			double RIDTh);*/
	C3Point  GetE(double x, double y, double z);
	int ReadFullField(char * name); //precalculated field
	int ReadFullFieldInfo(FILE * fin);
	void WriteTab(char * name);
	C3Point GetPoissonE(double xloc, double yloc);
	void DrawU(CDC* pDC, C3Point & Origin, C3Point & Scale);
	void Copy(int nx, int ny, double stepX, double stepY, double xmin, double ** nodeU);

};

class CPlasma;
//----------------------------------------------------
class CTarget : public CObject // virtual volume (now - for plasma)
{
public:
	int Nx, Ny, Nz;//cells
	C3Point Orig; // volume entry - Xmin
	C3Point Step; // cells
	C3Point OrtX, OrtY, OrtZ; //NOT APPLIED - local CS orts in Global CS
	double Density, Sigma; // const if no profile loaded
	int Npath;//if 0 - beam not traced in plasma
	double StepL;// particle path
	// temporary arrays - not to be kept by the target class (for multi-thread work)
	CArray <C3Point, C3Point> Path; // beam ray track (direct)
	CArray <double, double>  Value; // Temporary 1D array along path (common for SumThickness, ray decay, etc)
									// inject beam ray = set target thickness, then calculate decay	
	// View planes - vertical / horizontal Orig Nx, Ny, Nz
	double ** CutVert;
	double ** CutHor;
	double ** CutCS;
	
	// diag plane not used anymore
	C3Point CutOrig, CutDest; // diag min/max points
	C3Point CutOrtX, CutOrtY;  // diag Orts
	int CutNx, CutNy;  // diag dim
	double CutStepX, CutStepY; // diag Steps

	BEAM_RAY Ray; // parameters of one injected particle

	CTarget() { Nx = -1; Ny = -1; Nz = -1; Npath = -1; Orig = C3Point(); CutNx = -1; CutNy = -1; }
	CTarget(int nx, int ny, int nz) { Nx = nx; Ny = ny; Nz = nz; Npath = 0; }
	~CTarget();
	void SetOriginDim(C3Point start, int nx, int ny, int nz); // +3 cuts
	void SetZeroCuts();
	void SetCellSteps(C3Point step) { Step = C3Point(step);  }
	void SetMesh(C3Point start, C3Point end, int nx, int ny, int nz);
	void SetMeshSteps(C3Point start, C3Point end, double dx, double dy, double dz);
	void SetDensity(double dens);
	void SetSigma(double sigma);
	double GetDens(C3Point pos);
	double GetSigma(C3Point pos);
	// tracing rays or tracks
	void SetPath(C3Point P0, C3Point P1, double step);// set Path array
	void SetPathFrom(C3Point P0, C3Point Direct, double step);// set Path array
	void SetPathVal(int ival);// 0 - length, 1 - Target Thickness, 2- beam  decay
	void WritePathSingle(BEAM_RAY ray, CPlasma * plasma); // with tot Sigma, Decay, partial Rates
	void WritePathSet(BeamArray1D & rays, CPlasma * plasma); // Decay + rate for each ray
		
	void SetCutPlane(C3Point orig, C3Point destx, C3Point desty, double stepx, double stepy);
	C3Point GetLocal3D(C3Point P);
	C3Point GetCutLocal(C3Point P);
	int WithinTargetLoc(C3Point Ploc);
	// calculate Value
	int Distr2Dcell(C3Point Pgl, double val);
	void ClearArrays();
	void ClearCuts();
};

class CPlasma : public CTarget
{
public:
	int Nrays;// <=1 - thin, >1 - thick beam 
	int Npsi;//rad or psi surfaces - for dV volumes 
	int CalcOpt;// plot coord 0 - path, 1 - rad (parabolic) or flux (real)
	int BeamOpt; // 0 - ray, 1 - parallel, 2 - focused  
	double StepFlux;//0.1 rad or flux layers step
	double Sigma;// - "base" 
	double SigMult; // mult factor for Sigma base or total sigma array 
	double Nemax, Temax; //in parabolic or real model
	double PlasmaMinR, PlasmaMajR, PlasmaEll;
	C3Point TorCentre;
	double BeamVel;//[m/s]
	double RayDecay, SumDecay; // thin beam along path or thick along averpath
	BOOL PSIloaded;
	BOOL ProfilesLoaded;
	BOOL SigmaLoaded;
	BOOL DeltaLoaded;
	
	// along path - one ray
	CArray <double, double> Power;// Npower normalized
	CArray <double, double> NL; //Ne or Ne*Sigma
	CArray <double, double> IonRate; // Npower*dth
	// along path - averaged for many rays
	CArray <C3Point, C3Point> AverPath; // multi-ray beam "path"
	CArray <double, double> SumPower; // 1D array of Integrated (over many rays) Power
	CArray <double, double> SumNL; // 1D array of Integrated (over many rays) NL
	CArray <double, double> SumIonRate; // 1D array of Integrated (over many rays) IonRate
	CArray <int, int> SumPart; // number of reactions along stepL
	// along Flux or Rnorm [0..1]
	CArray <double, double> fPower;//<Npower>, integrated over many rays
	CArray <double, double> fNL;// <Density>, integrated over many rays
	CArray <double, double> fIonRate;// <Npower*dth>, integrated over many rays
	CArray <int, int> fSumPart; // number of reactions along stepStepFlux
	CArray <double, double> fVolume; // PSI[i] layer volume
	double ** CutRZ; //poloidal RZ [Npsi, Nz] StepR, StepZ
	double ** CutPsiZ; //poloidal Psi Z [Npsi, Nz] StepR, StepZ
	// FLUX 2D array
	int psiNr, psiNz;
	double StepR, StepZ;
	double Zmid; // = Zmag, vertical shift of magnetic axis ref to tor equator
	double Rmin, Rmax, Zmin, Zmax; // derived from EQDSK!!
	double ** PSI;

	// Plasma profiles, Sigmas 
	int Nprof;
	CArray <double, double> PSIprof; // ref coord for mapping profiles - pol flux or Ro normalized
	CArray <double, double> TeNorm;// Te = Temax * TeNorm[]
	CArray <double, double> NeNorm;// Ne = Nemax * NeNorm[]
	CArray <double, double> Zeff; // not active
	CArray <C3Point, C3Point> SigmArr; // CX, II, EI
	CArray <double, double> DeltaArr; // 

	CPlasma();
	~CPlasma();

	void SetPSIarray(int nr, int nz, double rmin, double zmin, double stepR, double stepZ);// set double ** PSI
	BOOL ReadPSI(char * name);
	BOOL ReadProfiles(char * name);
	BOOL ReadProfilesWithSigmas(char * name);
	void SetParProfiles();
//	BOOL ReadSigmArr(); // --- reserved
	double GetPSI(double R, double Z);
	double GetPSI(C3Point P);
	void ClearPSI();
	void ClearProfiles();
	void ClearSigma();
	void SetCutRZ();
	void SetZeroCuts();//inherited + CutRZ
	int  Distr2Dcell(C3Point Pgl, double val); // inherited + CutRZ
	void ClearCuts();// inherited + CutRZ

	void SetPlasmaParam(C3Point TorC, double a, double R, double Ell, double Ne, double Te, double sigma);
	void SetSigMult(double mult) { SigMult = mult; }
	void SetBeamRay(BEAM_RAY ray);
	void SetBeamCalcOpt(int bopt, int popt) { BeamOpt = bopt;  CalcOpt = popt; }
	double GetR(double X, double Y, double Z);
	double GetR(C3Point P);
	C3Point GetTorRZ(double X, double Y, double Z);
	double GetSigmaPSI(double psi);
	C3Point GetSigmaPSIpart(double psi, int enhance);
	//double GetSigma();// Sigma = const - simplified case
	double GetPlasmaDensityParabolic(double psi, int order);
	double GetPlasmaTeParabolic(double psi, int order);
	double GetPlasmaTePSI(double psi);
	double GetPlasmaNePSI(double psi);
	double GetPlasmaVolPSI(double psi);
	void   SetPathPower(); // calculate SumValue - sample array (not used)
	double GetPowerDecay(C3Point P0, C3Point P1, double dL); // get result decay between 2 points (for tracers)- no arrays / no path
	void   WritePathThin();// plasma parameters of thin beam along path - without SIGMAs
	// for tracers
	CArray<double, double>* GetPowerDecayArray(C3Point P0, C3Point P1, double dL);// return power along new created path
	void SetDecayArrays(); // calculate NL, Power, IonDens along the Path set before
	void AddDecayArrays(); // -> integrated for thick beam // add ray Decays and convert path coord to flux CS
	void ConvertArrays(); // path coord -> flux(rnorm)
	void AddPowerToCutPlanes();//distribute Normalised  power along Cut-planes
	void AddPowerToCutPlanes(double power0);// distribute real power
	void AddIonRateToCutPlanes(double power0);// distribute ion rate 
	void ClearArrays();// decays and powers
};
//-----------------------------------------------
//---------------------------------------------------------------------------------------------------
#endif