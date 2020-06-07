// BTRDoc.h : interface of the CBTRDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_BTRDOC_H__EF11C37D_1F7A_11D5_9A4F_006097D3F37D__INCLUDED_)
#define AFX_BTRDOC_H__EF11C37D_1F7A_11D5_9A4F_006097D3F37D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <MAPI.h>
#include "config.h"
#include "partfld.h"
#include "tracer.h"
//#include <vector>
using namespace std;

#define   MaxThreadNumber		21
#define   NofPlasmaImpurMax		16

#define   BTRVersion			5.0 
#define   MAXSCENLIMIT			32
#define   MAXLOADLIMIT			1024
// defaults to change

class CAskDlg;
class CBtrDoc;

int FindDataColumns(FILE * file);

//typedef pair<C3Point, C3Point> pPP;
//ypedef vector <pPP> tracks;

struct THREADINFO
{
	int m_ID; //thread number
	CDocument * pDoc;//CBTRDoc      
	CView * pView;// CMainView
	POINT BML; // min/max limits
	//CTracer * pTracer;
	//HWND hWnd;
};

#include <fstream>

using namespace std;

//extern bool LogOFF;
class logstream // copy outut to log-file
			// taken from http://www.cplusplus.com/forum/general/19489/
{
  public:
	 bool LogON;
	 ofstream log;
	 logstream(void){}
	 ~logstream(void){}
	 void SetLogSave(bool bSave) { LogON = bSave; } // save Log-file on disk 
	 void SetFile(char * path) { log.open(path, ios::out); }// | ios::app); }
	 void CloseFile() { log.close(); }
	 void ResetFile(char * path) { // append
		 if (log.is_open()) log.close(); 
		 log.open(path, ios::app);
	 }//
};

template <class T>
logstream& operator<< (logstream& f, T val)
{
	cout << val;
	cout.flush();
	if (f.LogON == FALSE) { // Log-file switched OFF
		//cout << "\t\t\t ----- logOFF\n";
		return f;
	}

	if (f.log.is_open())
		f.log << val;
	else cout << "---> skip log-out to file (closed) \n ";// << val; "\n";
	//f.log.flush();
	return f;
};


//bool LogOFF = FALSE; // write log-file, (stdout -- is always!)

class CBTRDoc : public CDocument
{
	friend class CMainView;
protected: // create from serialization only
	CBTRDoc();
	DECLARE_DYNCREATE(CBTRDoc)

private:
	int  Run_BTR_Fast();
	void InitTracer(int thread, CTracer * pTracer);
	void InitTracers();
	
	static UINT ThreadFunc(LPVOID pParam);// called by Run_BTR_Fast
	static UINT _ThreadFunc(LPVOID pParam);// not used
	void SuspendTracer(int iIndex, bool bRun);
	void SuspendAll(bool bRun);
	void ClearArrays();
			
// Attributes
private:
	CArray<CWinThread *, CWinThread *> m_ThreadArray; // new
	CWinThread * m_pTraceThread[MaxThreadNumber]; //old
	CTracer m_Tracer[MaxThreadNumber];
	//TVectorAttr m_AttrVectors[MaxThreadNumber];
	vector <minATTR> m_GlobalVector; // global Falls Array!
	//deque <SATTR> m_GlobalDeque;
	vector <minATTR> m_AttrVector[MaxThreadNumber];
	vector <EXIT_RAY> m_ExitVector[MaxThreadNumber];
	vector <FALL_ATTR> m_FallVector[MaxThreadNumber];
	vector <CString> m_GlobalLog;
	vector <CString> m_Log[MaxThreadNumber];

public:
	//double Version;
	int SCEN, MAXSCEN; // Scenario 
	int RUN, MAXRUN; // part of Scenario (max 3 runs per scenario)
	long long MemFree, MemUsed, MemFalls, ArrSize; 
	int ThreadNumber; // active threads for current run
	int IofCalculated;
	// tracked params across scens
	CArray<C3Point> PowSolid; // power on solids for scen(1,2,3 - RUNs)
	CArray<double> PowInjected; // power on DuctExit (or AreaLimit)
	CArray<double> PowNeutral;

	PtrList  PlatesList;
	PtrList  AddPlatesList;
	vector <VOLUME> VolumeVector;
	//vector <EXIT_RAY> ExitVector;

	FILE * ParticlesFile;
	CAskDlg * AskDlg;
	bool EnableAsk;
	CTime StartTime0;// start all SCEN (StartParallel)
	CTime StartTime, StopTime;//RUN Start 
	CTime TbeginSuspend, TendSuspend;
	CTimeSpan DataSaveTime;
	CTimeSpan SuspendedSpan;
	CString CurrentDirName;
	CString TopDirName;
	char * SingapFileName;
	CString MagFieldFileName;
	CString GasFileName;
	CString PDPgeomFileName; 
	CString BeamFileName;
	//CString MFManFileName;
	CString FW2DFileName;
	CString AddSurfDir;
	CString AddSurfName; // single file in config
	CString LogFilePath;
	CString LogFileName;// = "LOG_BTR5.txt"; //short name
	double ** ScenLoadSummary; // for final EXCEL(csv) table (PL)
	CStringArray ScenData[21]; // list of options for each scenario
	CStringArray SkipSurfClass; // list of substrings to find in plate Comment
	CStringArray ExceptSurf; // exceptions in skip
	CString m_Text;
	CStringArray  DataComment;
	CStringArray  DataName;
	CStringArray  DataSection;
	int DataSectionSize;
	int MaxOptRead, MaxDataRead, DataBegin[4], DataEnd[4];
	CString ConfigFileName;
	CString TaskName;
	CString TaskComment;
	CString TaskDate, TaskTime;
	CString ScenFileName;
	CArray<double*> DataValue;
	CArray<int, int> DataType;
	CArray<int, int> DataLineN;
	CArray<ATTRIBUTES, ATTRIBUTES> Attr_Array;// transport (max values)
	CArray<ATTRIBUTES, ATTRIBUTES> Attr_Array_Resid; // 10x12
	CArray<ATTRIBUTES, ATTRIBUTES> Attr_Array_Reion; // 10x12
	CArray<double, double> PolarAngle;  //Array [PolarNumber+1]
	CArray<double, double> PolarCurrent;  // Array [PolarNumber+1]
//	CArray<double, double> CartAngleY;  //Array [BeamSplitNumberY/2]
//	CArray<double, double> CartCurrentY;  // Array []
//	CArray<double, double> CartAngleZ;  //Array [BeamSplitNumberZ/2]
//	CArray<double, double> CartCurrentZ;  // Array []
	CArray<BEAMLET_ENTRY, BEAMLET_ENTRY> Singap_Array; // SINGAP
	CArray<BEAMLET_ENTRY, BEAMLET_ENTRY> BeamEntry_Array; // SINGAP
//	CArray<C3Point, C3Point> Exit_Array; // Beam Foot: X-Curr, Y-posY, Z-posZ
//	CArray<double, double> ReionArrayX;
//	CArray<double, double> ReionArrayCurr;
	CArray<C3Point, C3Point> ReionArray; // X, PosCurr, Neutr <- SetReionPercent
	CArray<C3Point, C3Point> NeutrArray; // X, Neg, PosIon <- NeutrCurrents
	CArray<C3Point, C3Point> StopArray; // X, Neutr, PosCurr (similar to Reion/Neutr)
	
	// plasma
	CPlasma * pPlasma; 
	CArray<C3Point, C3Point> DecayArray;//old (Ions,Dens,Neutr) <- SetDecayInPlasma()
	CArray<C3Point, C3Point> DecayPathArray;//old (Pos) <- SetDecayInPlasma()
	CArray<C3Point, C3Point> SumPSIArray; //old (dL,dth,Ions) at StepPSI // SetDecayInPlasma()
	
	CArray<C3Point, C3Point> CSarray;//E[keV/nu] SigmaStop [cm2] <- DLG EDIT cross-sections
	double StepPSI;// step for SumPSIArray;
	double StepPath;// for DecayPathArray
	
	//CArray<RECT_DIA, RECT_DIA> Dia_Array; // diaphragms array

	CArray<CPlate*, CPlate*> Load_Arr;
	CArray<int, int> Sorted_Load_Ind;
//	CArray<CPlate*, CPlate*> AddPlatesArray;
	auto_ptr<CMagField3D> BField3D;
	auto_ptr<CMagField> BField;
	auto_ptr<CGasField> GField;
	CGasField * AddGField;
	auto_ptr<CGasFieldXZ> GFieldXZ;
	CRIDField * RIDField;
	double MFcoeff;
	double GasCoeff;
	//CArray<C3Point, C3Point> MFdata;
	//CArray<double, double> MFXdata;
	CArray<C3Point, C3Point> FWdata;
	CArray<C3Point, C3Point> PlasmaProfileNTZ;
	CArray<double, double> PlasmaProfilePSI;
	CArray<EXIT_RAY> ExitArray;
	CArray<int, int> PlasmaImpurA;
	CArray<double, double> PlasmaImpurW;

	bool SINGAPLoaded;
	bool DataLoaded;
	bool FieldLoaded;
	bool RIDFieldLoaded;
	//bool MF_7;
	bool PressureLoaded;
	bool PDPgeomLoaded;
	bool FWdataLoaded;
	bool PlasmaLoaded;// PSI + profiles
	bool PSILoaded;
	bool ProfLoaded;
	bool TracksCalculated;
	bool ScenLoaded;

	volatile bool STOP;
	bool MemFallReserved;
	bool OptLogSave;// log-file output
	bool DINSELECT;
	bool NeedSave;
	bool ShowProfiles;
	bool OptDrawPart;
	int OptCombiPlot; ///-1 - no load, 0 - map is calculated, >0 - selected but not calculated 
	bool OptSINGAP;
	bool TaskRID;
	bool OptDeuterium;
	bool OptReionAccount;
	bool OptReionStop;
	bool OptNeutrStop;
	bool OptTraceAtoms;
	bool OptReflectRID;
//	bool OptStrayField;
	bool OptCalOpen;
	bool OptBeamInPlasma;
	bool OptThickNeutralization;
	bool OptIndBeamlets;
	bool OptPDP;
	//bool OptIonPower;
	bool OptAtomPower;
	bool OptNegIonPower;
	bool OptPosIonPower;
	bool OptParallel;
	bool OptFree;
	bool OptAddPressure;
	bool OptTraceSingle;
	bool OptAddDuct;
	int OptStart;
		
	bool OptCalcNeutralizer;
	bool OptCalcRID;
	bool OptCalcDuct;
	double CalcLimitXmin, CalcLimitXmax;
	
	
	int PlateCounter;
	int LoadSelected;
	int AddPlatesNumber;
	int Progress; // defining state of calcul 
	double  Alfa;
	int  SmoothDegree;
//	int  NofCalculated; //traced
	int  NofBeamlets; //active
	int  NofBeamletsTotal;//total active + inactive
	int  MultCoeff; // earlier - source part multiplication, now tot part/bml
	CPlate * pMarkedPlate;
	CPlate   emptyPlate;
	CPlate * pTorCrossPlate;
	CPlate * pBeamHorPlane;
	CPlate * pBeamVertPlane;
	int     EmitterNumber;// plate for next beam start
	int		PlasmaEmitter; // DuctExit or AreaLimit for trace in plasma
	int		DuctExit;// if defined
	
	int     DecilType;
//	int 	NofChannelsHor,	NofChannelsVert, NofBeamletsHor, NofBeamletsVert; // Beam Source
//	double  AppertRadius; // m
//	double  AppertCurrDensity; //A/m2 D-
	double  AreaLongMax;
	double  IonBeamPower;
	double  NeutrPower;
	double  AbsHorMin, AbsHorMax, AbsVertMin, AbsVertMax;
	int     PressureDim;
	int     MagFieldDim;

// PLASMA -------------------------------
	//input
	double PlasmaMajorR, PlasmaMinorR; 
	double PlasmaEllipticity; // k = b/a
	double TorCentreX, TorCentreY, TorCentreZ;
	double TorSegmentNumber;
	double MaxPlasmaDensity;//1.e20; //m-3
	double MaxPlasmaTe; // keV 
	double SigmaBase; //  SigmaEnhancement = 0;
	double PlasmaDensityOrd; // parabolic order
	double PlasmaTeOrd;// parabolic order

	// calculated or set in dlg
	double PlasmaXmin, PlasmaXmax, FWRmin, FWRmax, FWZmax, FWZmin;
	//	double InjectAimR, InjectAimZ; // tangency rad, dZ at tanrad point 
	C3Point TorCentre;	// in NBI frame (double TorCentreY;	double TorCentreX;)
 	int    NofPlasmaImpur;
	double PlasmaWeightH, PlasmaWeightD, PlasmaWeightT;
	int    MagSurfDim;// number of PSI-surfaces
	double SigmaEnhancement;// for plasma dlg

//  Dynamical DATA Arrays
	CArray<double> PSIvolume;
	CArray <double, double> SegmCentreHor;
	CArray <double, double> SegmCentreVert;
	double   SegmSizeHor, SegmSizeVert;  
	CArray <bool, bool> ActiveCh;
	CArray <bool, bool> ActiveRow;
	CArray <CRect, CRect> RectIS;//vert channel
	CArray <CRect, CRect> RectISrow; // horiz row
	double *    SectionCommon;
	double *    SectionMAMuG;
	CArray <double, double>	BeamletAngleHor;
	CArray <double, double>	BeamletAngleVert;
	CArray <double, double>	BeamletPosHor;
	CArray <double, double>	BeamletPosVert;

	int  TracePartType; // 0:e, 1:H+, 2:D+, -1:H-, -2:D-, 10:H0, 20:H0
	int	TracePartQ;
	int	TracePartNucl;
	double   TracePartMass;
	double   TraceTimeStep;
	int		TracePoints;
	int		TraceOption;
	double  TraceStepL; /// step for source ions

	int SingleBP;
	C3Point SingleBPpos;
	C3Point	SingleBPvel;
	double	SingleBPstep, SingleBPenergy;

////////    List of changeable data (CONFIGURATION + BEAM) ///////////////////////////////

//	double *	NofActiveChannels;
//	double *	NofActiveRows;

	int	NofActiveChannels;
	int	NofActiveRows;
// MAMuG Beam
	double	NofChannelsHor;// Beam Source
	double	NofChannelsVert;
	double	NofBeamletsHor;
	double	NofBeamletsVert; 
	double	SegmStepHor;
	double	SegmStepVert;
	double	BeamAimHor; 
	double	BeamAimVert;
	double	AppertStepHor;
	double	AppertStepVert;
	double	AppertAimHor; 
	double	AppertAimVert;

	double	BeamMisHor;
	double	BeamMisVert;
	double	BeamVertTilt;
	double	VertInclin;

//	double	AppertCurrent;
	double	IonBeamCurrent;
	double	IonBeamEnergy;
//	double	IonBeamPower;
//	double	InjectedPower;
	double	BeamRadius;///removed from Data List in v5!!!
	double	BeamFocusX;///removed from Data List in v5!!! 

	double	NeutrXmin;
	double	NeutrXmax;
	double	NeutrStepL;
	double	NeutrPart;
	double	PosIonPart;
//	double	NeutrPower;
//	double	TStepNeg;
//	double	TStepPos;
//	double	TStepNeu;

	double NeutrSigma;// = 1.3e-20;
	double ReionSigma;// = 3.2e-21;
	double TwoStripSigma; // 0.7e-21
	double PosExchSigma; // 0.03e-21
	double AddReionSigma; // Additional Gas Profile for reionization

	double IonStepL; /// step for reionized/residual ions
	double IonStepLspec; /// special step within Xspec0, Xspec1;
	double Xspec0, Xspec1; 
	double MinStepL; /// min step limit near surface for reionized ions
	double ReionXmin;
	double ReionXmax;
	double ReionStepL; /// re-ionization step
	double ReionStepLspec; /// special re-ion step within RXspec0, RXspec1;
	double RXspec0, RXspec1; 
	double ReionPercent;


//	double BeamDivergVert;
//	double BeamDivergHor;
	double BeamSplitType;
	double BeamSplitNumberY;///removed from Data List in v5!!!
	double BeamSplitNumberZ;///removed from Data List in v5!!!
	double BeamCoreDivY;
	double BeamCoreDivZ;
//	double BeamCoreDiverg;
	double BeamHaloDiverg;
	double BeamHaloPart;
//	double DecilType;
	double PolarNumber;///removed from Data List in v5!!!
	double AzimNumber; ///removed from Data List in v5!!!
	double PolarNumberAtom;
	double AzimNumberAtom;
	double PolarNumberReion;
	double AzimNumberReion;
	double PolarNumberResid;
	double AzimNumberResid;
	double CutOffCurrent;
	double PartDimHor, PartDimVert;

	double AreaLong; // Area limits
	double AreaHorMin;
	double AreaHorMax;
	double AreaVertMin;
	double AreaVertMax;
	
	double MovX; // Target (Movable) plate 1
	double MovVert;
	double MovHor;
	double MovShiftVert;

	double Mov2X; // Target plate 2
	double Mov2Vert;
	double Mov2Hor;
	double Mov2ShiftVert;

	double Mov3X; // Target plate 3
	double Mov3Vert;
	double Mov3Hor;
	double Mov3ShiftVert;

	double Mov4X; // Target plate 4
	double Mov4Vert;
	double Mov4Hor;
	double Mov4ShiftVert;

	double LoadStepX;
	double LoadStepY;

	double NeutrChannels;
	double NeutrInX;
	double NeutrOutX;
	double NeutrH;
	double NeutrInW;
	double NeutrOutW;
	double NeutrInTh;
	double NeutrOutTh;
	double NeutrInW4, NeutrOutW4;
	double NeutrBiasInVert;
	double NeutrBiasOutVert;
	double NeutrBiasInHor;
	double NeutrBiasOutHor;

	double RIDChannels;
	double RIDInX;
	double RIDOutX;
	double RIDH;
	double RIDInW;
	double RIDOutW;
	double RIDTh;
	double RIDU;
	double RIDInW4, RIDOutW4;
	double RIDBiasInVert;
	double RIDBiasOutVert;
	double RIDBiasInHor;
	double RIDBiasOutHor;


	double CalInX;
	double CalOutX;
	double CalInW;
	double CalOutW;
	double CalH;

	//double OptDuct; 
	double PreDuctX; //Scr1X
	double PreDuctW; //Scr1W
	double PreDuctH; //Scr1H
	double DiaBiasHor; //Scr1BiasHor
	double DiaBiasVert;//Scr1BiasVert
	
	double DDLinerInX; //Scr2X
	double DDLinerInW; //Scr2W
	double DDLinerInH; //Scr2H
	double LinerBiasInVert; //Scr2BiasVert
	double LinerBiasInHor; //Scr2BiasHor

	double DDLinerOutX; //Duct0X
	double DDLinerOutW; //Duct0W
	double DDLinerOutH; //Duct0H
	double LinerBiasOutVert; //Duct0BiasVert
	double LinerBiasOutHor; //Duct0BiasHor

	double Duct1X, Duct2X;
	double Duct3X, Duct4X;
	double Duct5X, Duct6X;
	double Duct7X, Duct8X;
	double Duct1W, Duct2W;
	double Duct3W, Duct4W;
	double Duct5W, Duct6W;
	double Duct7W, Duct8W;
	double Duct1H, Duct2H;
	double Duct3H, Duct4H;
	double Duct5H, Duct6H;
	double Duct7H, Duct8H;
	double Duct1BiasVert, Duct1BiasHor;
	double Duct2BiasVert, Duct2BiasHor;
	double Duct3BiasVert, Duct3BiasHor;
	double Duct4BiasVert, Duct4BiasHor;
	double Duct5BiasVert, Duct5BiasHor;
	double Duct6BiasVert, Duct6BiasHor;
	double Duct7BiasVert, Duct7BiasHor;
	double Duct8BiasVert, Duct8BiasHor;
	
	double	VShiftNeutr, VShiftRID, VShiftCal;// shifts through Inclination
	double  VShiftDia;//VShiftScr1
	double	VShiftLinerIn; // VShiftScr2
	double  VShiftLinerOut; // VShiftDuct0
	double  VShiftDuct1, VShiftDuct2, VShiftDuct3, VShiftDuct4; 
	double	VShiftDuct5, VShiftDuct6, VShiftDuct7, VShiftDuct8;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBTRDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual void DeleteContents();
	virtual void OnCloseDocument();
	virtual BOOL CanCloseFrame(CFrameWnd* pFrame);
	void CreateCopyMessage(CString server);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBTRDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;


#endif

	public:

	//UINT ThreadProc(LPVOID pParam);
	void OpenLogFile();
	void CloseLogFile();
	void ResetLogFile();
	void AddData(char*  comment, char* name, double& value, int type);
//	int * AddData(char*  comment, char* name, int  value, int type);
	void  UpdateDataArray();
	void  ClearData();
	void  InitData();
	void  InitFields();
	void  SetFields();
	void  InitOptions();
	void  InitTrackArrays();
	void  CreateScenLoadTracks();
	void  ClearScenLoadTracks();// set zero tracks
	//void  InitScenLoadTrack(int Nscen, int Nload);
	void  InitBeam();
	void  InitNBI();
	void  InitTokamak();
	void  InitPlasma();
	void  InitMamug();
	void  InitTaskRID();
	void  InitTaskReionization();
	void  InitTaskTransport(); 
	void  InitFWarray();
	void  InitScenDefault();
	void  SetMagnetPoints();
	int   SetData(CString & Line, int i);
	int   SetOption(CString & Line);
	void  FormDataText(bool fullinfo);
//	void  FormFileText();
	void  OnShow();
	void  SaveData();
	int   ReadData();
	void  ReadScenFile(); // read scenario data
	int   ReadPDPinput_old(char * name);
	int   ReadPDPinput(char * name); // config file for PDP 
	void  WritePDPinput(char * name);
	void  AddCommentToPDP(char * name);
	bool  CorrectPDPInput();
	int   OldPDPinput(char * name);

	void  CheckData();
	bool  CheckRegularity(int Iy, int Jz); // SINGAP or MAMuG ?
	void  WriteRegularBeam(char * name);
	void  WriteSingapBeam(char * name);

	void  ShowFileText(char * fname);

	void  SetTraceParticle(int type);
	void  SetTraceParticle(int nucl, int q); // 0:e, 1:H+, 2:D+, -1:H-, -2:D-, 10:H0, 20:H0

	void  SetStatus();
	void  AdjustAreaLimits();
	void  SetAreaMinMax();
	void  SetPlates();//free config
	void  ModifyArea(bool fixed); //change limits
	void  SetPlatesNBI();// standard config
	void  SetChannelWidth();
	void  SetPlatesNeutraliser();
	void  SetPlatesRID();
	C3Point GetStartPoint(double f, double axisR);
	C3Point GetNextFWcoord(C3Point prev, double dl);
	void  SetPlatesDuct();
	void  SetPlatesDuctFull(); // all walls
	void  SetDuctDia(); // duct crosses
	void  SetPlatesTor();
	void  InitAddPlates();
	void  SetAddPlates();
	CPlate * GetPlateByNumber(int N);
	bool  AddCond(CPlate * plate);// add to PlateList if condition
	void  AddPlate(CPlate * plate);// Add plate to AddSurfList
	int   FindPlateClones(CPlate * plate);
	CPlate * AddPlate(bool isSolid, C3Point p0, C3Point p1, C3Point p2, C3Point p3);// not used
	void  AppendAddPlates();// append to PlatesList
	void  CheckClosePlates();
	bool  SelectPlate(CPlate* plate);
	bool  SelectPlate(CPlate* plate, double hx, double hy);
	void  ClearAllPlates();
	void  SelectAllPlates();
	void  GetCombiPlotLimits(C3Point & lb, C3Point & rt);
	void  GetCombiPlotOrts(C3Point & OrtX, C3Point & OrtY);
	//void  GetCombiPlotPoints(C3Point & p0, C3Point & p1, C3Point & p2, C3Point & p3);
	void  SetPolarArrayBase(double DivY, double DivZ); // (Based AvCoreDiv = 1; Called for changed: Npolar, Nazim, CoreDiv/HaloDiv, HaloPart
	void  SetGaussBeamlet(double DivY, double DivZ);
	void  SetPolarBeamletCommon();
	void  SetPolarBeamletIndividual(double DivY, double DivZ);
	void  SetPolarBeamletReion(); // Attr_Arr for reionisation - called from InitData
	void  SetPolarBeamletResidual(); //Attr_Arr for residuals - called from InitData
	void  SetBeam();
	void  ReadSINGAP();
	bool  IsBeamletsFile(char * name);
	bool  OldBeamletsFormat(char * name);
	int   ReadBeamletsOld(char * name);
	int   ReadBeamletsNew(char * name);
	void  SetSINGAP();
	void  SetSINGAPfromMAMUG();
	void  SetVShifts();
	void  SetReionPercent();
	double GetReionDecay(C3Point P);
	double GetNeutrDecay(C3Point P1, C3Point P2);
	void  SetNeutrCurrents();
	C3Point GetCurrentRate(double x, double NL, bool getRate); // 0 - currents, 1 - rates
	void  SetNeutralisation();
	void  SetReionization();
	int   EmitBeam();
	int   EmitElectrons(bool optSINGAP);
	int   EmitSINGAP();
	int	  EmitMAMuG();
	void  EmitSourceParticle(CParticle * Particle);
	void  EmitBeamlet(BEAMLET_ENTRY be);
	void  ShowBeamlet(BEAMLET_ENTRY be);
	double GetNeutrOutput(int A, int K, double EkeV);// inf neutralization target 
	void PlotBeamletCurrent();
	void PlotBeamletFoot();
	
	double  GetNL(double xmin, double xmax);
	void  PlotPSIArrays();
	void  PlotDecayArray(int opt); // 0 - thin, 1 - thick
	void  PlotBeamDecay(int bopt, int popt); // beamopt
	void  SetBeamDecay();
	void  PlotStopArray();
	void  CalculateBeamPlasma();
	void  SetDecayInPlasma(); // decay between 2 points
	void  SetBeamRayDecay(C3Point P0, C3Point P1);
	void  SetPlasmaGeom();
	void  SetPlasmaTarget(); //init plasma object
	void  CalculateThinDecay(); //run axis beam
	//void  CalculateThickDecay(); // run set of beams 
	void  CalculateThickParallel(); // run parallel set of rays
	void  CalculateThickFocused(); // run real set of axis-rays
	void  SetStopArrays(C3Point P0, C3Point P1, double dL); // X, Neutr, PosIon - similar to Neutr/Reion
	void  StartAtomsFromEmitter();// trace atoms in plasma
	void  FillArrayA_Suzuki(int A, double * CoeffA);
	void  FillArrayB_Suzuki(int A, double * CoeffB); // 
	double GetSumHDT_Suzuki(int A, double E, double Density, double Te);
	double GetSumSz(int A, double E, double Density, double Te);
	void  SetImpurNumber();
	int   GetFirstImp();
	C3Point GetFW_Z(double x, double y, int nz); // FW profile - ShowTor()
	C3Point GetBeamFootLimits(double Xtarget, double & Ymin, double & Ymax, double & Zmin, double & Zmax);
	double GetPlasmaDensityParabolic(double r, int order);
	double GetPlasmaTeParabolic(double r, int order);
	double GetR(double X, double Y, double Z); // poloidal radius of plasma
	C3Point GetTorRZ(double X, double Y, double Z); // get R/Z in tokamak frame
	double GetPSI(double R, double Z); // get poloidal flux value (normalized to 1 on separatrix)
	double GetPSIrdr(double psi);
	double GetSigmaExchange(int A, double EkeV);//
	double GetSigmaIon(int A, double EkeV);//
	double GetSigmaElectron(int A, double EkeV, double TkeV);
	double GetDecay(C3Point P);// simplified - by StopArray along X
	double GetDecay(C3Point P0, C3Point P1, double dL); // real for each ray
	double GetFillDecayBetween(C3Point P0, C3Point P1, double Power);
	double GetDecayBetween(C3Point P0, C3Point P1);
	double GetSigmaEkeV(double EkeV);
	double GetStopSigma(double PSI);
	void CalculatePowerTrack(C3Point Start, C3Point Finish, double StartPower, double Vel); // project power track on plane
	void CalculateTracks();
	
	void  RandAttr(ATTRIBUTES & Attr, double step);
	void  ReadPressure();
	int   ReadMF_4col(char * name); // x bx by bz
	int   ReadMF_7col(char * name); // x  y  z  bx by bz  babs
	int   ReadGas_2col(char * name, int add); // x  p
	int   ReadGas_3col(char * name); // x  z  p
	void  ReadRIDfield();
	void  ReadPlasmaPSI();
	void  ShowStatus();
	void  ShowPlatePoints(bool draw);
	void  SetLoadArray(CPlate * plate, bool flag);
	void  SortLoadArrayAlongX();
	void  SaveLoads(bool free);
	void  DetachLoadFiles();
	CLoad * ReadLoad(char * name);
	void  ReadLoadInfo(CString & Text, int & Nx, int & Ny, double & StepX, double & StepY);
	BOOL  ReadPlateInfo(char* name, CPlate * Plate);
	int  ReadLoadArray(char* name, CLoad* Load, bool isfree);
	void  RotateVert(double angle, double & X,  double & Y,  double & Z);
	void  RotateHor(double angle, double & X,  double & Y,  double & Z);
	C3Point   CentralCS(C3Point  P0, int segmHor, int segmVert); //returns appert pos in Central CS
	C3Point   LocalCS(C3Point  P0,  int sign);

	double GetPressure(C3Point P);
	void  ClearPressure(int add);
	C3Point GetB(double x, double y, double z);
	C3Point GetB(C3Point P);
	void ClearB();
	//C3Point GetManMF(double x);
		
	int  RunPDP(char * name);
	void ShowBPdata(int n, int lines, int pos, int angle, int power);
	void ShowBPdata(int n, int lines);
	void ShowLogFile(int lines);
	void ShowReport();
	void CorrectFile(char * name);
	int  ReadAddPlates();
	int  ReadAddPlates(char * name);
	int  ReadAddPlatesDir(char * name);
	void WriteAddPlates(char * name);
	CLoad* ReadPDPLoadTable(char * name);
	void ReadEQDSKtable(char * name);
	void InitSumPower();
	void ClearSumPower();
	void WriteSumReiPowerX(char * name, float xmin, float xmax);
	void WriteSumAtomPowerX(char * name, float xmin, float xmax);
	void WriteSumPowerAngle(char * name);
	void ReadIonPowerX();
	void ReadAtomPowerX();
	void ReadPlasmaProfiles(char* name);
	C3Point GetPlasmaTeNe(double PSI); // N, T, Zeff profiles ref to poloidal flux
	
	double GetIonStep(double x);
	void PlotCurrentRates(bool rate); 

	void F_CalculateLoad(CPlate * plate);
	bool F_CrossedSolid(C3Point P1, C3Point P2);
	//bool F_GotThrough(C3Point P1, C3Point P2);
	void F_CalcRID();
	void F_CalcRIDsimple();
	void F_CalcDirect();

	void CreateBaseSingle(); // creates a new plate - pMarkedPlate proection
	void P_CalculateLoad(CPlate * plate);
	void P_CalculateLoad(CPlate * plate, vector<minATTR> * parr);// Atom/Neg/PosPower++ 
	void P_CalculateCombi(bool update);
	void P_CalculateAngularProf(CPlate * plate);
	void SetNullLoad(CPlate * plate);
	void CalculateAllLoads();
	BOOL SetDefaultMesh();
	void SetNullLoads(); // init maps for all "interesting" plates

	double GetNeutrPower();
	double GetInjectedPowerW(); // calculate Atom power at Duct Exit
	double GetTotSolidPower(); // sum power falled on solids // Area Exit excluded
	int  GetCompSolid(CStringArray & keys, double & SumPower, double & MaxPD);
	void GetCompTransparent(CStringArray & keys, double & Pentry, double & Pexit);
	
	void WriteExitVector();
	void WriteFallVector();
	void TraceSingleParticle(int state, C3Point pos, C3Point v, double step, double energy);
	void TraceSingleAgain(double X, double Y);
	void GetMemState();
	void SendReport(bool includetext);

	bool AddFalls(int tid, int isrc, std::vector<minATTR> * tattr);
	//bool AddFalls(int tid, int isrc, std::vector<minATTR> & attr);
	void AddFallsToLoads(int tid, int isrc, std::vector<minATTR> * tattr);
	bool AddFallsToFalls(int tid, int isrc, std::vector<minATTR> * tattr);
		
	bool AddLog(std::vector<CString> * log);
	void ShowPlasmaCut(int icut);
	//void OnBeaminplasmaVerticalplane();
	void ShowPlasmaCutDiagonal();
	
	void CompleteRun();
	void CompleteScen();
	void CorrectRunPower();//fit balance before merge runs 
	void InitRun(int run);
	CString GetScenName(int scen);
	void CreateScenFolder();
	void InitScen();
	void InitScenOpt(int & optA, int & optRes, int & optRei);
	void RunScen(int iopt[3]);
	int  GetPrefFiles(CString pref, CStringArray & names);
	int  GetFilesSubstr(CString substr, CStringArray & fnames); 
	void SaveRUNLoads(); // create RUN folder with load results
	void CollectRUNLoads(); // add load for each surf -> create folder with merged loads
	void SaveRUNSummary(CString name); // create single RUN summary
	void CollectRUNSummary(CString SUMname, CStringArray & names);// merge diff RUN summaries to a single file SCEN#_SUMloads 
    void AddAllScenLoads();// add runs 1,2,3 on each plate
	void ResumeData();
	void WriteScenTracks();
	void WriteReport(CString ext);
	void ReadScenLoads3col(); // read SCEN results from Load_total (3-col)
	
/*
protected:
	friend UINT _Threadfunc(LPVOID param);
	friend UINT _ThreadCalcLoad(LPVOID param);
*/

// Generated message map functions
protected:
	//{{AFX_MSG(CBTRDoc)
	afx_msg int  OnDataGet();
	
	afx_msg void OnDataGet_void();
	afx_msg void OnDataStore();
	afx_msg void OnOptionsAccountofreionization();
	afx_msg void OnUpdateOptionsAccountofreionization(CCmdUI* pCmdUI);
	afx_msg void OnOptionsStopionsafterneutraliser();
	afx_msg void OnUpdateOptionsStopionsafterneutraliser(CCmdUI* pCmdUI);
	afx_msg void OnOptionsStopreionizedparticles();
	afx_msg void OnUpdateOptionsStopreionizedparticles(CCmdUI* pCmdUI);
	afx_msg void OnOptionsStrayfieldonoff();
	afx_msg void OnUpdateOptionsStrayfieldonoff(CCmdUI* pCmdUI);
	afx_msg void OnOptionsOpencalorimeter();
	afx_msg void OnUpdateOptionsOpencalorimeter(CCmdUI* pCmdUI);
	afx_msg void OnTasksMagshield();
	afx_msg void OnUpdateTasksMagshield(CCmdUI* pCmdUI);
	afx_msg void OnTasksReionization();
	afx_msg void OnUpdateTasksReionization(CCmdUI* pCmdUI);
	afx_msg void OnTasksNeutralisation();
	afx_msg void OnUpdateTasksNeutralisation(CCmdUI* pCmdUI);
	afx_msg void OnResultsRead();
	afx_msg void OnResultsSaveall();
	afx_msg void OnUpdateOptionsRealionsourcestructure(CCmdUI* pCmdUI);
	afx_msg void OnDataSave();
	afx_msg void OnResultsReadall();
	afx_msg void OnDataActive();
	afx_msg void OnOptionsSingap();
	afx_msg void OnUpdateOptionsSingap(CCmdUI* pCmdUI);
	afx_msg void OnTasksRid();
	afx_msg void OnUpdateTasksRid(CCmdUI* pCmdUI);
	afx_msg void OnStart();
	afx_msg void OnStartParallel();
	//afx_msg void OnParallel();
	afx_msg void OnViewFullarea();
	afx_msg void OnViewNeutraliser();
	afx_msg void OnViewRid();
	afx_msg void OnTasksOther();
	afx_msg void OnUpdateTasksOther(CCmdUI* pCmdUI);
	afx_msg void OnViewFitonoff();
	afx_msg void OnUpdateViewFitonoff(CCmdUI* pCmdUI);
	afx_msg void OnViewBeam();
	afx_msg void OnUpdateViewBeam(CCmdUI* pCmdUI);
	afx_msg void OnViewFields();
	afx_msg void OnUpdateViewFields(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewFullarea(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewNeutraliser(CCmdUI* pCmdUI);
	afx_msg void OnViewNumbering();
	afx_msg void OnUpdateViewNumbering(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewRid(CCmdUI* pCmdUI);
	afx_msg void OnEditComments();
	afx_msg void OnEditTitle();
	afx_msg void OnFileNew();
	afx_msg void OnFileOpen();
	afx_msg void OnFileSave();
	afx_msg void OnStop();
		
	afx_msg void OnViewParticles();
	afx_msg void OnUpdateViewParticles(CCmdUI* pCmdUI);
	afx_msg void OnEditGas();
	afx_msg void OnEditMagfield();
	afx_msg void OnEditMagfield_7columns();
	afx_msg void OnUpdateEditMagfield_7columns(CCmdUI* pCmdUI);
	afx_msg void OnEditMagfield_4columns();
	afx_msg void OnUpdateEditMagfield_4columns(CCmdUI* pCmdUI);
	afx_msg void OnPlotSingappos();
	afx_msg void OnUpdatePlotSingappos(CCmdUI* pCmdUI);
	afx_msg void OnPlotGasprofile();
	afx_msg void OnUpdatePlotGasprofile(CCmdUI* pCmdUI);
	afx_msg void OnPlotMagneticfield();
	afx_msg void OnUpdatePlotMagneticfield(CCmdUI* pCmdUI);
	afx_msg void OnPlotMamugpositions();
	afx_msg void OnUpdatePlotMamugpositions(CCmdUI* pCmdUI);
	afx_msg void OnPlot3dload();
	afx_msg void OnUpdatePlot3dload(CCmdUI* pCmdUI);
	afx_msg void OnPlotBeamfoot();
	afx_msg void OnPlotBeamletfoot();
	afx_msg void OnPlotBeamletcurr();
	afx_msg void OnPlotLoadmap();
	afx_msg void OnUpdatePlotLoadmap(CCmdUI* pCmdUI);
	afx_msg void OnPlotMaxprofiles();
	afx_msg void OnUpdatePlotMaxprofiles(CCmdUI* pCmdUI);
	afx_msg void OnAsk();
	afx_msg void OnUpdateAsk(CCmdUI* pCmdUI);
	afx_msg void OnOptionsTraceneutralsinplasma();
	afx_msg	void OnUpdateOptionsTraceneutralsinplasma(CCmdUI* pCmdUI);
	afx_msg void OnPlotSigmas();
	afx_msg void OnPlotNeutrefficiency();
	afx_msg void OnNeutralizationCurrents();
	afx_msg void OnNeutralizationRates();
	afx_msg void OnPlotContours();
	afx_msg void OnUpdatePlotContours(CCmdUI* pCmdUI);
	afx_msg void OnUpdatePlotNeutrefficiency(CCmdUI* pCmdUI);
	afx_msg void OnUpdatePlotSigmas(CCmdUI* pCmdUI);
	afx_msg void OnUpdateAppExit(CCmdUI* pCmdUI);
	afx_msg void OnOptionsSurfacesAdd();
	afx_msg void OnOptionsBeam();
	afx_msg void OnOptionsFields();
	afx_msg void OnOptionsNBIconfig();
	afx_msg void OnOptionsSurfacesEnabledisableall();
	afx_msg void OnTasksBeamplasma();
	
	afx_msg void OnDataImport_PDP_SINGAP();
	afx_msg void OnTasksPDP();
	afx_msg void OnUpdateTasksPDP(CCmdUI* pCmdUI);
	afx_msg void OnResultsPdpoutputTable();
	afx_msg void OnFileSaveAs();
	afx_msg void OnDataImportSingapBeam();
	afx_msg void OnDataImportMamugBeam();
	afx_msg void OnPdpBeamlets();
	afx_msg void OnDataExportPDPgeom();
	afx_msg void OnDataExportRegularBeam();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPlotPenetration();
	afx_msg void OnPlotReionPower();
	afx_msg void OnSurfacesSort();
	afx_msg void OnUpdatePlotReionPower(CCmdUI *pCmdUI);
	afx_msg void OnUpdatePlateSelect(CCmdUI *pCmdUI);
	afx_msg void OnUpdateSurfacesSortincr(CCmdUI *pCmdUI);
	afx_msg void OnUpdateOptionsSurfacesAdd(CCmdUI *pCmdUI);
	afx_msg void OnPlotCombi();
	afx_msg void OnPlateClearSelect();
	afx_msg void OnEditFW2D();
	afx_msg void OnSendBtrInput();
	afx_msg void OnSendOther();
	afx_msg void OnEditPlasmaTeNe();
	
	afx_msg void OnBeaminplasmaVerticalplane();
	afx_msg void OnBeaminplasmaHorizontalplane();
	afx_msg void OnBeaminplasmaPoloidalRz();
	afx_msg void OnBeaminplasmaPoloidalPsiZ();
	afx_msg void OnPlotPSI();
	
	afx_msg void OnDataSaveaddsurf();
	afx_msg void OnDataReadaddsurf();
	afx_msg void OnUpdateDataSaveaddsurf(CCmdUI *pCmdUI);
	afx_msg void OnPlasmaPsi();
	afx_msg void OnUpdateTasksBeamplasma(CCmdUI *pCmdUI);
	afx_msg void OnUpdatePlotPenetration(CCmdUI *pCmdUI);
	afx_msg void OnTraceBigAtom();
	afx_msg void OnTraceBigIon();
	afx_msg void OnTraceBigElectron();
	afx_msg void OnOptionsPlasma();
	afx_msg void OnResultsSaveList();
	
	afx_msg void OnResultsSavesummary();
	afx_msg void OnGasProf_X();
	afx_msg void OnGasProf_XZ();
	afx_msg void OnUpdateOptionsNbiconfig(CCmdUI *pCmdUI);
	afx_msg void OnInputFree();
	afx_msg void OnInputStandard();
	afx_msg void OnUpdateInputFree(CCmdUI *pCmdUI);
	afx_msg void OnUpdateInputStandard(CCmdUI *pCmdUI);
	afx_msg void OnSurfacesRead();
	afx_msg void OnOptionInput();
	//afx_msg void OnOptionsInput();
	afx_msg void OnPlateSelect();
	afx_msg void OnPlateModify();
	afx_msg void OnPlateDeleteAllFree();
	afx_msg void OnPlateShowFile();
	afx_msg void OnPlotReionization();
	afx_msg void OnUpdatePlotReion(CCmdUI *pCmdUI);
	afx_msg void OnOptionsThreads();
	afx_msg void OnPlotSumReionX();
	afx_msg void OnPlotSumAtomX();
	afx_msg void OnPlotAngulardistr();
	afx_msg void OnUpdatePlotAngulardistr(CCmdUI *pCmdUI);
	
	afx_msg void OnLogView();
	afx_msg void OnLogSave();
	//	afx_msg void OnLogSave();
	afx_msg void OnUpdateLogSave(CCmdUI *pCmdUI);
	afx_msg void OnStatisticsView();
	afx_msg void OnStatisticsSet();
	afx_msg void OnSurfacesMesh();
	afx_msg void OnUpdatePlateFile(CCmdUI *pCmdUI);
	afx_msg void OnPlateLoadOptRecalc();
	afx_msg void OnEditCrossSect();
	afx_msg void OnPlateClearall();
	
	afx_msg void OnPlateScale();
	afx_msg void OnResultsReadScen();
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BTRDOC_H__EF11C37D_1F7A_11D5_9A4F_006097D3F37D__INCLUDED_)
