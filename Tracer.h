#pragma once


// CTracer command target

//inline BOOL Between(double a, double b, double c) { return (a-b)*(c-b) <= 0 ? TRUE : FALSE; }

#include <algorithm>
#include <vector>


#define  ATOM	1000
#define  SRCION	2000
#define  POSION	3000
#define	 REION	4000
#define  ELECTRON 5000
#define  RELECTRON 5001
#define  THIN	0
#define  THICK	1

extern BOOL InvUser; 
/*
struct TATTR // Attributes of a thread Particle - 40 bytes
{
	C3Point Pos;
	double Power;
	int Charge;
//	COLORREF Color;
	int Nfall;

	TATTR()
	{
		
	}
	TATTR(const TATTR & t)
	{
		Pos = t.Pos;
		Power = t.Power;
		Charge = t.Charge;
//		Color = t.Color;
		Nfall = t.Nfall;
	}
	TATTR(C3Point pos, double power, int charge, COLORREF color, int nfall)
	{
		Pos = pos;
		Power = power;
		Charge = charge;
//		Color = color;
		Nfall = nfall;
	}
};
*/


//UINT ThreadDraw(PVOID pParam);
//static UINT ThreadFunc(LPVOID pParam); // declared in BTRDOc  private

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

struct SATTR // Attributes of a thread Particle - 20 bytes
{
	float X;// 4 bytes 3.4E +/- 38 (7 digits)
	float Y;// 4
	float Power; // 4
	signed char Charge; //1 byte -128...127
	unsigned short int Nfall; //char Nfall; 2 bytes 0 ...65535
	//short adeg; // = shortint (angle from the surf, 90-normal fall) 	//2 bytes –32768...32767
	float amrad; //4 angle from the surf, rad


	SATTR()
	{
	}
	SATTR(const SATTR & t)
	{
		X = t.X; Y = t.Y;
		Power = t.Power;
		Charge = t.Charge;
		Nfall = t.Nfall;
		amrad = t.amrad;
	}
	SATTR operator = (const SATTR & t) const
	{
		return SATTR(t);
	}
	SATTR(float x, float y, float power, signed char charge, unsigned short nfall, float am)
	{
		X = x; Y = y;
		Power = power;
		Charge = charge;
		Nfall = nfall;
		amrad = am;
	}
	SATTR(double x, double y, double power, int charge, int nfall, double am)
	{
		X = (float)x; Y = (float)y;
		Power = (float)power;
		Charge = (signed char)charge;
		Nfall = (unsigned short)nfall;
		amrad = (float)am;
	}
		
	SATTR(C3Point pos, double power, int charge, int nfall)
	{
		X = (float) pos.X; //(short) (pos.X * 1000); //short
		Y = (float) pos.Y; //(short) (pos.Y * 1000); //short
		Power = (float) power;
		Charge = (signed char) charge;
		Nfall = (unsigned short) nfall;
		amrad = -1; //100000; // not calculated 
	}
	SATTR(C3Point pos, double power, int charge, int nfall, double afall)
	{
		X = (float) pos.X; //(short) (pos.X * 1000); //short
		Y = (float) pos.Y; //(short) (pos.Y * 1000); //short
		Power = (float) power;
		Charge = (signed char) charge;
		Nfall = (unsigned short) nfall;
		amrad = float(afall * 1000);
	}
};

struct EXIT_RAY // position, velocity, neutral power of particle at duct exit
{
	C3Point Position;
	C3Point Velocity;
	double Power;

	
	EXIT_RAY()
	{
	}

	EXIT_RAY(const EXIT_RAY & r)
	{
		Position = r.Position;
		Velocity = r.Velocity;
		Power = r.Power;
	}

	EXIT_RAY(C3Point pos, C3Point v, double pow)
	{
		Position = pos;
		Velocity = v;
		Power = pow;
	}
};

struct FALL_ATTR // position, velocity, power of particle at fall point
{
	signed char Charge; //particle Charge;
	unsigned char Nfall; //plate number
	C3Point Position; // in global cs
	float Angle; // from plate surface
	float Power;

	
	FALL_ATTR()
	{
	}

	FALL_ATTR(const FALL_ATTR & f)
	{
		Charge = f.Charge;
		Nfall = f.Nfall;
		Position = f.Position;
		Angle = f.Angle;
		Power = f.Power;
	}

	FALL_ATTR(signed char q, unsigned char n, C3Point pos, float a, float pow)
	{
		Charge = q;
		Nfall = n;
		Position = pos;
		Angle = a;
		Power = pow;
	}
};
/*
class CVectorCleanup {
public:
	explicit CVectorCleanup(vector<minATTR> *v) {
		m_v = v;
	}
	~CVectorCleanup() {
		vector<minATTR>().swap(*m_v);
	}
private:
	vector<minATTR> *m_v;
};
*/

class CTracer : public CObject
{
private:
	CDocument * m_pDoc;
	//CRIDField * RIDField;
	CWnd* m_pViewWnd;
	CWnd* m_pStatus;
	volatile BOOL  m_Continue; //* m_pbContinue; 
	int m_Min, m_Max;// source beamlets
	C3Point m_Pos, m_V, m_PrevPos; 
	C3Point m_NeutrPos, m_NeutrV;
	C3Point m_ReionPos, m_ReionV;
	int m_iSource; // beamlet
	int m_iRay; // particle
	double m_Step;
	BOOL m_Dead;
	BOOL m_Stopped;
	BOOL m_Transformed;
	BOOL m_OptWriteFall;
	int m_Charge;
	double m_Power, m_StartPower;
	double m_NeutrPower, m_ReionPower;
	double m_NL; //sum thickness for neutralisation 
	COLORREF m_Color;
	int m_State; //NEGION, POSION, REION, ATOM
	int m_Mode; // THIN or THICK
	BOOL m_Draw; // show particles
	int m_Falls; // number of solid falls for reflected ions
	int m_ID; // thread personal Number
		
public :
	CArray<ATTRIBUTES, ATTRIBUTES> * pAttr; // current bml rays Attr Array
	//std::vector<CString> * logarr;
	std::vector<minATTR> * tattr;
	std::vector<EXIT_RAY> * exitarr;
	std::vector<FALL_ATTR> * fattr; 
	
	static CCriticalSection cs;

	//TVectorAttr &vattr;
public:
	CTracer();
	virtual ~CTracer();
	//CTracer(TVectorAttr &p);
	int  Get_ID() { return m_ID; }
	int  GetBMLmin() { return m_Min; }
	int  GetBMLmax() { return m_Max; }
	long GetMemUsedkB();
	long GetMemFreekB();
	long GetGlobFalls();
//	void SetLogArray(std::vector <CString> * p) {logarr = p; }
	void SetAttrArray(std::vector <minATTR> * p) {tattr = p;}
	void SetExitArray(std::vector <EXIT_RAY> * q) {exitarr = q;}
	void SetFallArray(std::vector <FALL_ATTR> * f) {fattr = f;}
	BOOL GetContinue() { return m_Continue; }
	void SetContinue(BOOL cont) { m_Continue = cont; }
	CWnd* GetViewWnd() { return m_pViewWnd; }
	void SetViewWnd(CWnd* pWnd) { m_pViewWnd = pWnd; }
	void SetStatWnd(CWnd* pWnd) { m_pStatus = pWnd; }
	void SetDocument(CDocument* pDoc) {m_pDoc = pDoc; }
	void SetDraw(BOOL draw) { m_Draw = draw; }
	void Draw(); // OLD engine < 4.5
	//void DrawParticlePos(C3Point pos, COLORREF color);
	void DrawPartTrack(CArray<C3Point> & track);
	void TestMem();
	bool AddFall(int n, C3Point Ploc, C3Point Vat, double power);
	bool GetFalls(C3Point P1, C3Point & P2, bool & metlimit);
	double GetAtomDecay(C3Point P1, C3Point P2); //NEW
	double GetNeutrDecay(C3Point P1, C3Point P2); //NEW
	bool TraceSourceIonTHIN();
	bool TraceAtom();
	bool GeneratePosIon(int jray);
	bool GenerateNegIon(int jray);
	bool GenerateReions(int jray);
	bool TraceIon(int charge, C3Point Pos0, C3Point V0, double power);
	bool TraceRay(int jray);
	bool TraceBeamletAtoms(int isource);
	bool TraceBeamletResIons(int isource);
	bool TraceBeamletReions(int isource);
	void TraceAlltest();
	bool TraceAllAtoms();
	bool TraceAllResIons();
	bool TraceAllReions();
	void TraceAll();
	void DumpArrays();
	void ClearArrays();
	void ShowBeamlet(int isource);
	void ShowProgress(int Ncalc);
	void SetID(int id) { m_ID = id; }
	void SetLimits(int min, int max) { m_Min = min; m_Max = max;}
	void SetStartPoint(int isource);
	void SetStartVP(int iray);
	void SetStep(double step) { m_Step = step; }
	void SetColor();
	C3Point MakeStep();
	C3Point MoveIonToX(double Xlim);
	C3Point MakeStepToX(double Xlim);
	C3Point MakeStepL(double dl);
	void DoNextStep();
	void Randomize();
	int CheckFall(C3Point P1, C3Point & P2);
	BOOL IsVolumeEmpty(C3Point P);
	void CheckNeutrBound(C3Point P1, C3Point P2);
	void CheckNeutrState(C3Point P1, C3Point P2);
	BOOL CheckReionState(C3Point P1, C3Point P2);
	int GetFall(const C3Point P1, C3Point & P2);
	int GetFallAdd(const C3Point P1, C3Point & P2);
	double ReflectPosIon(C3Point Pgl, CPlate * Plate);
	void WriteParticleFall(C3Point P, CPlate * plate);
	double GetVangle(CPlate * plate, C3Point V) const;
	C3Point GetVatPlate(CPlate * plate, C3Point V) const;
	void SetNeutrPoint(C3Point P);
	void GetNeutrPoint();
	void SetReionPoint(C3Point P);
	void GetReionPoint();
	void SetNextState();
	void SetPosIonState();
	void SetSrcIonState();
	void SetAtomState(int from);
	void SetReionState(); // reionized ion
	void Calculate_V();
	double GetVstart();
	void   SetRaysAttr(CArray<ATTRIBUTES, ATTRIBUTES>* pRattr);
	int  GetNeutrPointNumber();
	int  GetReionPointNumber();
	//double GetNeutrDecay(C3Point P1, C3Point P2);
	void TraceAtomFast();
	C3Point GetAtomSolidFall(const C3Point P1, const C3Point P2) const;
	void GetAtomFallsBetween(const C3Point P1, const C3Point P2) const;
	void DrawAtomTrack(const C3Point P1, const C3Point P2);
	void SetSingleParticle(int state, C3Point pos, C3Point v, double step, double energy);
	void TraceSingleAtom(); // DoNextStep - one track
	void TraceSinglePosIon(); // DoNextStep - one track
	void TraceSingleElectron(); // DoNextStep - one track
	void TraceSingleRelectron(); // DoNextStep - one track
	BOOL IsCalculated(CPlate * plate) const;
	BOOL WithinCalculatedLimits(C3Point & P) const;

};


