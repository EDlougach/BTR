// BTRDoc.cpp : implementation of the CBTRDoc class
//

#include "stdafx.h"
#include "BTR.h"
#include <math.h>
#include <ctype.h>
#include <windows.h>
#include <stdio.h>
#include <vector>
#include <algorithm>
#include <time.h>
#include <fstream>

#include "psapi.h"
#include "Wininet.h"

#include "config.h"
#include "BTRDoc.h"
#include "DataView.h"
#include "MainView.h"
#include "LoadView.h"
#include "SetView.h"
#include "partfld.h"
#include "RemnDlg.h"
#include "ReionDlg.h"
#include "NeutrDlg.h"
#include "IonSourceDlg.h"
#include "ResSmooth.h"
#include "ProjectBar.h"
#include "TaskDlg.h"
#include "CommentsDlg.h"
#include "GasDlg.h"
#include "MFDlg.h"
#include "MultiPlotDlg.h"
#include "PlotDlg.h"
#include "PreviewDlg.h"
#include "AskDlg.h"
#include "InjectDlg.h"
#include "SurfDlg.h"
#include "BeamDlg.h"
#include "FieldsDlg.h"
#include "NBIconfDlg.h" 
#include "ThinDlg.h"
#include "ThickDlg.h"
#include "ThreadsDlg.h"
#include "SendMail.h"
#include "LevelsDlg.h"
#include "RIDDlg.h"
#include "AtomDlg.h"
#include "PlasmaDlg.h"
#include "InputDlg.h"
#include "StartDlg.h"
#include "LimitsDlg.h"
#include "DefLoadDlg.h"
#include "StatOutDlg.h"
#include "LoadStepDlg.h"
using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//using namespace System::Net::Mail;

class logstream;
logstream logout;

CString FindFileName(CString S) 
{
	CString S1 = S;
	int pos = S.Find( "\\", 0);
	while (pos >= 0) {
		S1 = S.Mid(pos+1);
		pos = S.Find("\\", pos+1);
	}
	return S1;
}

int SplitString(CString sbuf, CString sep, CStringArray SArr)
{
	CString sBuf = _T(sbuf);
	CString Separator = _T(sep);
	int Position = 0;
	CString Token;
	
	Token = sBuf.Tokenize(Separator, Position);
	//AfxMessageBox(Token);
	int found = 0;
	SArr.Add(Token);
	
	while (!Token.IsEmpty())
	{
		// Get next token.
		found++;
		Token = sBuf.Tokenize(Separator, Position);
		SArr.Add(Token);
		//AfxMessageBox(Token);
		
	}
	//AfxMessageBox(SArr[0] + SArr[1] + SArr[2]);
	return found;
}

int FindDataColumns(FILE * f)
{
	int N = 0;
	double x;
	char buf[1024];
	char *nptr, *endptr, Res;
	while (N < 2) {
		if (fgets(buf, 1024, f) == NULL) return 0;
	
		nptr = buf;
		N = 0;
		while (sscanf(nptr,"%f",&x) != EOF) {
			x = strtod(nptr,&endptr);
			if (nptr == endptr) break; //nptr++; //catch special case
			else nptr = endptr;
			N++;
		}
	}
	return N;
}

inline double GAUSS(double arg, double delta){	return exp(-arg*arg / (delta*delta)) / delta;}

extern CBTRApp theApp;
extern HWND ProjectBar;
extern CLoadView * pLoadView;
extern CMainView * pMainView;
extern CDataView * pDataView;
extern CSetView * pSetView ;
char * MagFilename = "Arrmag.txt";

BOOL TaskRemnantField, TaskReionization, ThickNeutralization, TaskBeamInPlasma;
double Bremnant1 = 0, Bremnant2 =  0, Bremnant3 = 0, Xremnant1 = 0, Xremnant2 = 0, Xremnant3 = 0;
//double ReionPercent = 0;// ReionSigma = 3.2e-21; // m2;
double NeutrSigma = 0;
double DuctExitYmin, DuctExitYmax, DuctExitZmin, DuctExitZmax;
// BTR-fast parameters 
double AreaXmax;
double RIDXmin, RIDchannelWidth;
double RID_A;
double Part_V;

extern vector<tDistInfo> dists;

extern CArray<double, double> SumReiPowerX; /// array for Reion sum power deposited within StepX
extern CArray<double, double> SumAtomPowerX; /// array for Atom sum power deposited within StepX
extern double SumPowerStepX;// step for sum power

extern CArray<double, double> SumPowerAngle; // array for sum power deposited within Angle
extern double SumPowerAngleStep;// degrees, step for angular profile

double CPlate::AbsYmin = 1000; 
double CPlate::AbsYmax = -1000;
double CPlate::AbsZmin = 1000; 
double CPlate::AbsZmax = -1000;

double LevelStep, LevelMin, LevelMax;
BOOL LevelWrite;

int DefLoadNX = 10, DefLoadNY = 10; // default Load settings
double DefLoadStepX = 0.01; // 1cm
double DefLoadStepY = 0.01; // 1cm
BOOL DefLoadStepRound = TRUE; // round step to reduce digits
BOOL DefLoadOptN = TRUE; // FALSE -> FIX STEP    TRUE -> FIX(+round) Cells Number

CArray <BEAMLET_ENTRY, BEAMLET_ENTRY> SourceArr; 
CArray <C3Point, C3Point> Exit_Array; // Beam Foot: X-Curr, Y-posY, Z-posZ
CArray <RECT_DIA, RECT_DIA> Dia_Array; // diaphragms array
int  NofCalculated; //traced beamlets
BOOL F_GotThrough(C3Point P1, C3Point P2);
CString  GetMachine();
CString  GetUser();

double AtomPower1;
double AtomPower2;
double AtomPower3;

BOOL InvUser;
//////////////////////////////////////////////////////////////////////////////////////////////
// CBTRDoc

IMPLEMENT_DYNCREATE(CBTRDoc, CDocument)

BEGIN_MESSAGE_MAP(CBTRDoc, CDocument)
	//{{AFX_MSG_MAP(CBTRDoc)
	ON_COMMAND(ID_DATA_GET, OnDataGet_void)
	ON_COMMAND(ID_DATA_STORE, OnDataStore)
	ON_COMMAND(ID_OPTIONS_ACCOUNTOFREIONIZATION, OnOptionsAccountofreionization)
	ON_UPDATE_COMMAND_UI(ID_OPTIONS_ACCOUNTOFREIONIZATION, OnUpdateOptionsAccountofreionization)
	ON_COMMAND(ID_OPTIONS_STOPIONSAFTERNEUTRALISER, OnOptionsStopionsafterneutraliser)
	ON_UPDATE_COMMAND_UI(ID_OPTIONS_STOPIONSAFTERNEUTRALISER, OnUpdateOptionsStopionsafterneutraliser)
	ON_COMMAND(ID_OPTIONS_STOPREIONIZEDPARTICLES, OnOptionsStopreionizedparticles)
	ON_UPDATE_COMMAND_UI(ID_OPTIONS_STOPREIONIZEDPARTICLES, OnUpdateOptionsStopreionizedparticles)
	ON_COMMAND(ID_OPTIONS_STRAYFIELDONOFF, OnOptionsStrayfieldonoff)
	ON_UPDATE_COMMAND_UI(ID_OPTIONS_STRAYFIELDONOFF, OnUpdateOptionsStrayfieldonoff)
	ON_COMMAND(ID_OPTIONS_OPENCALORIMETER, OnOptionsOpencalorimeter)
	ON_UPDATE_COMMAND_UI(ID_OPTIONS_OPENCALORIMETER, OnUpdateOptionsOpencalorimeter)
	ON_COMMAND(ID_TASKS_MAGSHIELD, OnTasksMagshield)
	ON_UPDATE_COMMAND_UI(ID_TASKS_MAGSHIELD, OnUpdateTasksMagshield)
	ON_COMMAND(ID_TASKS_REIONIZATION, OnTasksReionization)
	ON_UPDATE_COMMAND_UI(ID_TASKS_REIONIZATION, OnUpdateTasksReionization)
	ON_COMMAND(ID_TASKS_NEUTRALISATION, OnTasksNeutralisation)
	ON_UPDATE_COMMAND_UI(ID_TASKS_NEUTRALISATION, OnUpdateTasksNeutralisation)
	ON_COMMAND(ID_RESULTS_READ, OnResultsRead)
	ON_COMMAND(ID_RESULTS_SAVEALL, OnResultsSaveall)
	ON_UPDATE_COMMAND_UI(ID_OPTIONS_REALIONSOURCESTRUCTURE, OnUpdateOptionsRealionsourcestructure)
	ON_COMMAND(ID_DATA_SAVE, OnDataSave)
	ON_COMMAND(ID_RESULTS_READALL, OnResultsReadall)
	ON_COMMAND(ID_DATA_ACTIVE, OnDataActive)
	ON_COMMAND(ID_OPTIONS_SINGAP, OnOptionsSingap)
	ON_UPDATE_COMMAND_UI(ID_OPTIONS_SINGAP, OnUpdateOptionsSingap)
	ON_COMMAND(ID_TASKS_RID, OnTasksRid)
	ON_UPDATE_COMMAND_UI(ID_TASKS_RID, OnUpdateTasksRid)
	//ON_COMMAND(ID_START, OnStart)
	ON_COMMAND(ID_Parallel, OnStartParallel)
	ON_COMMAND(ID_VIEW_FULLAREA, OnViewFullarea)
	ON_COMMAND(ID_VIEW_NEUTRALISER, OnViewNeutraliser)
	ON_COMMAND(ID_VIEW_RID, OnViewRid)
	ON_COMMAND(ID_TASKS_OTHER, OnTasksOther)
	ON_UPDATE_COMMAND_UI(ID_TASKS_OTHER, OnUpdateTasksOther)
	ON_COMMAND(ID_VIEW_FITONOFF, OnViewFitonoff)
	ON_UPDATE_COMMAND_UI(ID_VIEW_FITONOFF, OnUpdateViewFitonoff)
	ON_COMMAND(ID_VIEW_BEAM, OnViewBeam)
	ON_UPDATE_COMMAND_UI(ID_VIEW_BEAM, OnUpdateViewBeam)
	ON_COMMAND(ID_VIEW_FIELDS, OnViewFields)
	ON_UPDATE_COMMAND_UI(ID_VIEW_FIELDS, OnUpdateViewFields)
	ON_UPDATE_COMMAND_UI(ID_VIEW_FULLAREA, OnUpdateViewFullarea)
	ON_UPDATE_COMMAND_UI(ID_VIEW_NEUTRALISER, OnUpdateViewNeutraliser)
	ON_COMMAND(ID_VIEW_NUMBERING, OnViewNumbering)
	ON_UPDATE_COMMAND_UI(ID_VIEW_NUMBERING, OnUpdateViewNumbering)
	ON_UPDATE_COMMAND_UI(ID_VIEW_RID, OnUpdateViewRid)
	ON_COMMAND(ID_EDIT_COMMENTS, OnEditComments)
	ON_COMMAND(ID_EDIT_TITLE, OnEditTitle)
	ON_COMMAND(ID_FILE_NEW, OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	ON_COMMAND(ID_FILE_SAVE, OnFileSave)
	ON_COMMAND(ID_STOP, OnStop)
	ON_COMMAND(ID_VIEW_PARTICLES, OnViewParticles)
	ON_UPDATE_COMMAND_UI(ID_VIEW_PARTICLES, OnUpdateViewParticles)
	ON_COMMAND(ID_EDIT_GAS, OnEditGas)
	ON_COMMAND(ID_EDIT_MAGFIELD, OnEditMagfield)
	ON_COMMAND(ID_EDIT_MAGFIELD_7COLUMNS, OnEditMagfield_7columns)
	ON_UPDATE_COMMAND_UI(ID_EDIT_MAGFIELD_7COLUMNS, OnUpdateEditMagfield_7columns)
	ON_COMMAND(ID_EDIT_MAGFIELD_4COLUMNS, OnEditMagfield_4columns)
	ON_UPDATE_COMMAND_UI(ID_EDIT_MAGFIELD_4COLUMNS, OnUpdateEditMagfield_4columns)
	ON_COMMAND(ID_PLOT_SINGAPPOS, OnPlotSingappos)
	ON_UPDATE_COMMAND_UI(ID_PLOT_SINGAPPOS, OnUpdatePlotSingappos)
	ON_COMMAND(ID_PLOT_GASPROFILE, OnPlotGasprofile)
	ON_UPDATE_COMMAND_UI(ID_PLOT_GASPROFILE, OnUpdatePlotGasprofile)
	ON_COMMAND(ID_PLOT_MAGNETICFIELD, OnPlotMagneticfield)
	ON_UPDATE_COMMAND_UI(ID_PLOT_MAGNETICFIELD, OnUpdatePlotMagneticfield)
	ON_COMMAND(ID_PLOT_MAMUGPOSITIONS, OnPlotMamugpositions)
	ON_UPDATE_COMMAND_UI(ID_PLOT_MAMUGPOSITIONS, OnUpdatePlotMamugpositions)
	ON_COMMAND(ID_PLOT_3DLOAD, OnPlot3dload)
	ON_UPDATE_COMMAND_UI(ID_PLOT_3DLOAD, OnUpdatePlot3dload)
	ON_COMMAND(ID_PLOT_BEAMFOOT, OnPlotBeamfoot)
	ON_COMMAND(ID_PLOT_BEAMLETFOOT, OnPlotBeamletfoot)
	ON_COMMAND(ID_PLOT_BEAMLETCURR, OnPlotBeamletcurr)
	ON_COMMAND(ID_PLOT_LOADMAP, OnPlotLoadmap)
	ON_UPDATE_COMMAND_UI(ID_PLOT_LOADMAP, OnUpdatePlotLoadmap)
	ON_COMMAND(ID_PLOT_MAXPROFILES, OnPlotMaxprofiles)
	ON_UPDATE_COMMAND_UI(ID_PLOT_MAXPROFILES, OnUpdatePlotMaxprofiles)
	ON_COMMAND(ID_ASK, OnAsk)
	ON_UPDATE_COMMAND_UI(ID_ASK, OnUpdateAsk)
	ON_COMMAND(ID_OPTIONS_TRACENEUTRALSINPLASMA, OnOptionsTraceneutralsinplasma)
	ON_UPDATE_COMMAND_UI(ID_OPTIONS_TRACENEUTRALSINPLASMA, OnUpdateOptionsTraceneutralsinplasma)
	ON_COMMAND(ID_PLOT_NEUTREFFICIENCY, OnPlotNeutrefficiency)
	ON_COMMAND(ID_PLOT_CONTOURS, OnPlotContours)
	ON_UPDATE_COMMAND_UI(ID_PLOT_CONTOURS, OnUpdatePlotContours)
	ON_UPDATE_COMMAND_UI(ID_PLOT_NEUTREFFICIENCY, OnUpdatePlotNeutrefficiency)
	ON_UPDATE_COMMAND_UI(ID_APP_EXIT, OnUpdateAppExit)
	ON_COMMAND(ID_OPTIONS_SURFACES_ADD, OnOptionsSurfacesAdd)
	ON_COMMAND(ID_OPTIONS_BEAM, OnOptionsBeam)
	ON_COMMAND(ID_OPTIONS_FIELDS, OnOptionsFields)
	ON_COMMAND(ID_OPTIONS_NBICONFIG, OnOptionsNBIconfig)
	ON_COMMAND(ID_OPTIONS_SURFACES_ENABLEDISABLEALL, OnOptionsSurfacesEnabledisableall)
	ON_COMMAND(ID_TASKS_BEAMPLASMA, OnTasksBeamplasma)
	ON_COMMAND(ID_PLOT_PENETRATION, OnPlotPenetration)
	ON_COMMAND(ID_DATA_IMPORT_PDPSINGAP, OnDataImport_PDP_SINGAP)
	ON_COMMAND(ID_TASKS_PDPCODE, OnTasksPDP)
	ON_UPDATE_COMMAND_UI(ID_TASKS_PDPCODE, OnUpdateTasksPDP)
	ON_COMMAND(ID_RESULTS_PDPOUTPUT_TABLE, OnResultsPdpoutputTable)
	ON_COMMAND(ID_FILE_SAVE_AS, OnFileSaveAs)
	ON_COMMAND(ID_DATA_IMPORT_SINGAP, OnDataImportSingapBeam)
//	ON_COMMAND(ID_DATA_IMPORT_MAMUGBEAMLETS, OnDataImportMamugBeam)
	ON_COMMAND(ID_PDP_BEAMLETS, OnPdpBeamlets)
	ON_COMMAND(ID_DATA_EXPORT_PDPGEOM, OnDataExportPDPgeom)
	ON_COMMAND(ID_FILE_SEND_MAIL, OnFileSendMail)
	ON_UPDATE_COMMAND_UI(ID_FILE_SEND_MAIL, OnUpdateFileSendMail)
	ON_COMMAND(ID_DATA_EXPORT_REGULARBEAM, OnDataExportRegularBeam)
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_PLOT_RE, &CBTRDoc::OnPlotReionPower)
	ON_COMMAND(ID_SURFACES_SORTINCR, &CBTRDoc::OnSurfacesSort)
	ON_UPDATE_COMMAND_UI(ID_PLOT_RE, &CBTRDoc::OnUpdatePlotReionPower)
	ON_UPDATE_COMMAND_UI(ID_PLATE_SELECT, &CBTRDoc::OnUpdatePlateSelect)
	ON_UPDATE_COMMAND_UI(ID_SURFACES_SORTINCR, &CBTRDoc::OnUpdateSurfacesSortincr)
	ON_UPDATE_COMMAND_UI(ID_OPTIONS_SURFACES_ADD, &CBTRDoc::OnUpdateOptionsSurfacesAdd)
	ON_COMMAND(ID_PLOT_COMBI, &CBTRDoc::OnPlotCombi)
	ON_COMMAND(ID_PLATE_CLEARSELECT, &CBTRDoc::OnPlateClearSelect)
	ON_COMMAND(ID_EDIT_FW_2D, &CBTRDoc::OnEditFW2D)
	ON_COMMAND(ID_SEND_BTRINPUT, &CBTRDoc::OnSendBtrInput)
	ON_COMMAND(ID_SEND_OTHER, &CBTRDoc::OnSendOther)
	ON_COMMAND(ID_PLASMA_TENE, &CBTRDoc::OnEditPlasmaTeNe)
	ON_COMMAND(ID_BEAMINPLASMA_VERTICALPLANE, &CBTRDoc::OnBeaminplasmaVerticalplane)
	ON_COMMAND(ID_BEAMINPLASMA_HORIZONTALPLANE, &CBTRDoc::OnBeaminplasmaHorizontalplane)
	ON_COMMAND(ID_DATA_SAVEADDSURF, &CBTRDoc::OnDataSaveaddsurf)
	ON_COMMAND(ID_DATA_READADDSURF, &CBTRDoc::OnDataReadaddsurf)
	ON_UPDATE_COMMAND_UI(ID_DATA_SAVEADDSURF, &CBTRDoc::OnUpdateDataSaveaddsurf)
	ON_COMMAND(ID_Plasma_PSI, &CBTRDoc::OnPlasmaPsi)
	ON_UPDATE_COMMAND_UI(ID_TASKS_BEAMPLASMA, &CBTRDoc::OnUpdateTasksBeamplasma)
	ON_UPDATE_COMMAND_UI(ID_PLOT_PENETRATION, &CBTRDoc::OnUpdatePlotPenetration)
	ON_COMMAND(ID_TRACEBIGPARTICLE_ATOM, &CBTRDoc::OnTraceBigAtom)
	ON_COMMAND(ID_TRACEBIGPARTICLE_ION, &CBTRDoc::OnTraceBigIon)
	ON_COMMAND(ID_TRACEBIGPARTICLE_ELECTRON, &CBTRDoc::OnTraceBigElectron)
	ON_COMMAND(ID_OPTIONS_PLASMA, &CBTRDoc::OnOptionsPlasma)
	ON_COMMAND(ID_RESULTS_SAVESUMMARY, &CBTRDoc::OnResultsSavesummary)
	ON_COMMAND(ID_GASPROFILE_X, &CBTRDoc::OnGasProf_X)
	ON_COMMAND(ID_GASPROFILE_X32907, &CBTRDoc::OnGasProf_XZ)
	ON_UPDATE_COMMAND_UI(ID_OPTIONS_NBICONFIG, &CBTRDoc::OnUpdateOptionsNbiconfig)
	ON_COMMAND(ID_INPUT_FREE, &CBTRDoc::OnInputFree)
	ON_COMMAND(ID_INPUT_STANDARD, &CBTRDoc::OnInputStandard)
	ON_UPDATE_COMMAND_UI(ID_INPUT_FREE, &CBTRDoc::OnUpdateInputFree)
	ON_UPDATE_COMMAND_UI(ID_INPUT_STANDARD, &CBTRDoc::OnUpdateInputStandard)
	ON_COMMAND(ID_SURFACES_READ, &CBTRDoc::OnSurfacesRead)
	
	//ON_COMMAND(ID_OPTIONS_INPUT, &CBTRDoc::OnOptionsInput)
	ON_COMMAND(ID_PLATE_SELECT, &CBTRDoc::OnPlateSelect)
	ON_COMMAND(ID_PLATE_MODIFYSURF, &CBTRDoc::OnPlateModify)
	ON_COMMAND(ID_PLATE_DELETEALLFREE, &CBTRDoc::OnPlateDeleteAllFree)
	ON_COMMAND(ID_PLATE_FILE, &CBTRDoc::OnPlateShowFile)
	ON_COMMAND(ID_PLOT_REION, &CBTRDoc::OnPlotReionization)
	ON_UPDATE_COMMAND_UI(ID_PLOT_REION, &CBTRDoc::OnUpdatePlotReion)
	ON_COMMAND(ID_OPTIONS_THREADS, &CBTRDoc::OnOptionsThreads)
	ON_COMMAND(ID_PD_REIONS, &CBTRDoc::OnPlotSumReionX)
	ON_COMMAND(ID_PD_ATOMS, &CBTRDoc::OnPlotSumAtomX)
	ON_COMMAND(ID_PLOT_ANGULARDISTR, &CBTRDoc::OnPlotAngulardistr)
	ON_UPDATE_COMMAND_UI(ID_PLOT_ANGULARDISTR, &CBTRDoc::OnUpdatePlotAngulardistr)
	ON_COMMAND(ID_NEUTRALIZATION_CURRENTS, &CBTRDoc::OnNeutralizationCurrents)
	ON_COMMAND(ID_NEUTRALIZATION_RATES, &CBTRDoc::OnNeutralizationRates)
		ON_COMMAND(ID_LOG_VIEW, &CBTRDoc::OnLogView)
		ON_COMMAND(ID_LOG_SAVE, &CBTRDoc::OnLogSave)
		ON_COMMAND(ID_STATISTICS_VIEW, &CBTRDoc::OnStatisticsView)
		ON_COMMAND(ID_STATISTICS_SET, &CBTRDoc::OnStatisticsSet)
		ON_COMMAND(ID_SURFACES_MESH, &CBTRDoc::OnSurfacesMesh)
		ON_UPDATE_COMMAND_UI(ID_PLATE_FILE, &CBTRDoc::OnUpdatePlateFile)
		ON_COMMAND(ID_PLATE_LOADOPT, &CBTRDoc::OnPlateLoadOptRecalc)
		ON_COMMAND(ID_EDIT_CROSS, &CBTRDoc::OnEditCrossSect)
		ON_COMMAND(ID_PLATE_CLEARALL, &CBTRDoc::OnPlateClearall)
		ON_COMMAND(ID_BEAMINPLASMA_POL_RZ, &CBTRDoc::OnBeaminplasmaPoloidalRz)
		ON_COMMAND(ID_BEAMINPLASMA_POL_PSIZ, &CBTRDoc::OnBeaminplasmaPoloidalPsiZ)
		ON_COMMAND(ID_PLOT_PSI, &CBTRDoc::OnPlotPSI)
		ON_COMMAND(ID_PLATE_SCALE, &CBTRDoc::OnPlateScale)
		ON_COMMAND(ID_RESULTS_READSCEN, &CBTRDoc::OnResultsReadScen)
		END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBTRDoc construction/destruction

CBTRDoc::CBTRDoc()
{
		AskDlg = new CAskDlg(NULL);
	//	AskDlg->Create();
		EnableAsk = TRUE;
		SetColors();
		OptLogSave = TRUE; //!LogOFF;
	
}

CBTRDoc::~CBTRDoc()
{
	delete AskDlg;
	// dists.~vector();

	// delete BField3D;
	// delete GField;
	delete AddGField;
	delete RIDField;
}

CString  DataString(char * name, double value, int type)
{
	CString S, strval;
	char end = '\n';
	CString eq =  " = ";
	switch (type) {
	case 0: strval.Format("%7d", (int)value); break;
	case 1: strval.Format("%7.1f", value);  break;
	case 2: strval.Format("%12.4e", value);  break;
	} // switch
	S = name + eq + strval + end;
	return S;
}

void CBTRDoc:: ClearData()
{
	if (!DataSection.IsEmpty()) DataSection.RemoveAll();
	//DataSection.FreeExtra();
	if (!DataName.IsEmpty()) DataName.RemoveAll();	
	//DataName.FreeExtra();
	if (!DataComment.IsEmpty()) DataComment.RemoveAll();
	//DataComment.FreeExtra();
	if (!SkipSurfClass.IsEmpty()) SkipSurfClass.RemoveAll(); // list of substrings to find in plate Comment
	if (!ExceptSurf.IsEmpty()) ExceptSurf.RemoveAll();// exceptions in skip
}

void CBTRDoc:: InitData()
{
	char buf[1024];
	::GetCurrentDirectory(1024, buf);
	TopDirName.Format("%s", buf);  
	CurrentDirName = TopDirName; // set in InitData;
	//strcpy(SingapDirName, CurrentDirName);

	// here we detect number of processors present in the system (for unknown purposes)
	SYSTEM_INFO si;
	::GetSystemInfo(&si);
	int num_proc = si.dwNumberOfProcessors;
	ThreadNumber  =  num_proc;
	if (ThreadNumber > 8) ThreadNumber = 8; // set limit
	
	LogFileName = "LOG_BTR5.txt"; //short name
	InitOptions();
	CreateScenLoadTracks();
	
//	AppertRadius = 0.007; // m
//	AppertCurrDensity = 203.; //A/m2 D-
	AreaLongMax = 26;// 40;
	ReionPercent = 0;
	PlasmaEmitter = -1;
	DuctExit = -1;
		
	//ClearData();

//	DataSection.Add("SINGAP parameters");
	DataSection.Add("BEAM TRACING");
	DataSection.Add("NBI CONFIGURATION");
	DataSection.Add("TOKAMAK  area");
	DataSection.Add("BEAM ARRAY (regular)");// "MAMuG beam (regular array)");
	DataSectionSize = DataSection.GetSize();
	
//	NofActiveChannels = AddData("Total Number of Active Vert.Channels ", NAME(NofActiveChannels), 4., 0);
//	NofActiveRows = AddData("Total Number of Active Horiz.Rows ", NAME(NofActiveRows), 4., 0);
//	LoadStepX = 0.02; LoadStepY = 0.02;

// ------------- Beam Tracing Options
	int k = 0;
	DataBegin[k] = 0;
	InitBeam();
	DataEnd[k] = DataName.GetUpperBound();
	//std::cout << "InitBeam done" << std::endl;
	// NBI
	k++;
	DataBegin[k] = DataName.GetUpperBound()+1;
	
	//if (!OptFree) 
	InitNBI();
	DataEnd[k] = DataName.GetUpperBound();
	//std::cout << "InitNBI done" << std::endl;
	
	// Tokamak area
	k++;
	DataBegin[k] = DataName.GetUpperBound()+1;
	//if (!OptFree) 
	InitTokamak(); // Plasma must be called after SetBeam , but before InitPlasma 
	DataEnd[k] = DataName.GetUpperBound();
	//std::cout << "InitTokamak done" << std::endl;
	// MAMuG array
	k++;
	DataBegin[k] = DataName.GetUpperBound()+1;
	InitMamug();
	DataEnd[k] = DataName.GetUpperBound();
	//std::cout << "InitMamug done" << std::endl;
	
	MaxDataRead = DataName.GetUpperBound()+1;
	//AdjustAreaLimits();
	CalcLimitXmin = 0;
	CalcLimitXmax = AreaLong + 1;

	InitAddPlates();// remove list, AddPlatesNumber = 0;
	//std::cout << "InitAddPlates done" << std::endl;

	InitSumPower();
	//std::cout << "InitSumPower done" << std::endl;
	
	SetBeam(); // MAMuG Channels/Beamlets, Aimings/Positions
	for (int i = 0; i < (int)NofChannelsHor; i++) ActiveCh[i] = TRUE;
	for (int j = 0; j < (int)NofChannelsVert; j++) ActiveRow[j] = TRUE;

	InitPlasma(); //called in InitData (after SetBeam!!!) // calls SetPlasmaTarget -> SetPlasmaGeom
	//std::cout << "InitPlasma done" << std::endl;

	ArrSize = 0;
	SetStatus();// numbers of active channels/rows/totbeamlets
	if ((int) BeamSplitType == 0) SetPolarBeamletCommon(); //SetBeamletAt(0);
	else SetGaussBeamlet(BeamCoreDivY, BeamCoreDivZ); 
	SetPolarBeamletReion(); // called only once
	SetPolarBeamletResidual();// called only once
	std::cout << "SetBeamlets Polar - " << PolarNumber << " Azim -" << AzimNumber << std::endl;

	SetPlates();// calls AdjustAreaLimits();
	//std::cout << "SetPlates done" << std::endl;
	
	//InitTrackArrays();// trackin Sum Loads for EXCEL(csv): SCEN = 1 loads = PlateCounter

	FormDataText(FALSE); //without internal names
	//std::cout << "FormDataText done" << std::endl;

	//ThreadNumber = 1;//num_proc; //4;

	SetSINGAPfromMAMUG();

	//SetDecayInPlasma();
	ConfigFileName = " Undefined ";
	//std::cout << "Config file:" << ConfigFileName << std::endl;

	Reflectors.RemoveAll();
	ReflectorNums.RemoveAll();
/*
	Reflectors.Add("|DD BOX INTERNAL LINER|FACET2");
	Reflectors.Add("|DD BOX INTERNAL LINER|FACET3");
	Reflectors.Add("|CDLINER TOP|FACET1");
	Reflectors.Add("|CDLINER BOTTOM|FACET2");
	Reflectors.Add("|ABSOLUTE_VALVE | RIGHT 1|FACET1");
	Reflectors.Add("|ABSOLUTE_VALVE | RIGHT 2|FACET2");
	Reflectors.Add("|ABSOLUTE_VALVE | RIGHT 3|FACET3");
	*/
}

void CBTRDoc::InitFields() // MF + GAS
{
	//if (PressureLoaded) GField->Clear(); 
	//if (FieldLoaded)   BField->Clear();
	GasFileName = "";  //"Gas Pressure data";
	MagFieldFileName = "";//"MF data";
	FieldLoaded = FALSE;
	PressureLoaded = FALSE;
	OptReionAccount = FALSE;
	
}

void CBTRDoc::SetFields() // if found MF + GAS
{
	char name[1024];
	int res = 0;
	double NegIonPart;
	if (MagFieldFileName.GetLength() > 3) { // MF file defined
		strcpy(name, MagFieldFileName);
		res = BField->ReadData(name);
		if (res > 0) {
			//FieldLoaded = true;
			FieldLoaded = TRUE;
			//MF_7 = FALSE;
			MagFieldDim = 1;
			if (MFcoeff == 0) MFcoeff = 1.;
			logout << " + MF is set from " + MagFieldFileName + "\n";
		}
		else  {
			FieldLoaded = FALSE;
			logout << " - MF in NOT read  " + MagFieldFileName + "\n";
		}
	}

	res = 0;
	if (GasFileName.GetLength() > 3) { // GAS file defined
		strcpy(name, GasFileName);
		res = GField->ReadData(name);
		if (res > 0) {
			//PressureLoaded = true;
			PressureLoaded = TRUE;
			PressureDim = 1;
			GasCoeff = 1;
			OptReionAccount = TRUE;//automatic when gas loaded
			OptThickNeutralization = TRUE; // not automatic 
			logout << " + GAS is set from " + GasFileName + "\n";
			SetReionPercent();
			SetNeutrCurrents();
		}

		else {
			PressureLoaded = FALSE;
			logout << " - GAS in NOT read  " + GasFileName + "\n";
			logout << "Neutral fraction         " << NeutrPart << "\n";
			logout << "Positive ions fraction   " << PosIonPart << "\n";
			//NeutrPart = 1 - NegIonPart - PosIonPart;
			NegIonPart = 1. - NeutrPart - PosIonPart;
			logout << "Negative ions fraction   " << NegIonPart << "\n";
		}
	}// // GAS file defined
	else { // NO gas file
		logout << " - GAS in NOT defined \n";
		logout << "Neutral fraction         " << NeutrPart << "\n";
		logout << "Positive ions fraction   " << PosIonPart << "\n";
		NegIonPart = 1. - NeutrPart - PosIonPart;
		logout << "Negative ions fraction   " << NegIonPart << "\n";
	}
}

void CBTRDoc:: SetAddPlates()
{
	//if (MagFieldFileName.GetLength() > 3) { // MF file defined
		//strcpy(name, MagFieldFileName);
	bool ReadDir = FALSE;
	int last = AddSurfName.GetLength();
	if (last < 4) return; // empty surf name

	int pos = AddSurfName.Find("\\", last - 3);
	if (pos > 0) {
		ReadDir = TRUE;// ended by "\" - folder
		AddSurfName.Left(pos);
	}
	
	char name[1024];
	strcpy(name, AddSurfName);
	if (!ReadDir) { //(AddSurfName.GetLength() > 3) {// read all addsurf from single file
		ReadAddPlates(name);
	}
	else { // read all surf files from folder
		ReadAddPlatesDir(name);
	}

}

void CBTRDoc:: InitOptions()
{
	//OptFree = FALSE;//TRUE; // free config

	STOP = TRUE; 
	InitFields();//FieldLoaded = FALSE; //PressureLoaded = FALSE;
				 //GasFileName = "";//"Gas Pressure data";
				 //MagFieldFileName = "";//"MF data";
	AddSurfName = "";
	AddSurfDir = "";
	TracksCalculated = 0;
	MemFallReserved = FALSE;
	//PDPgeomLoaded = FALSE;
	RIDFieldLoaded = FALSE;
	
	SINGAPLoaded = FALSE;
	NofCalculated = 0;
	//DataLoaded = FALSE;
	PressureDim = 0;
	MagFieldDim = 0;
	AtomPower1 = 0;
	AtomPower2 = 0;
	AtomPower3 = 0;

	//OptThickNeutralization = FALSE;
	OptReionAccount = FALSE;
	OptReionStop = TRUE;
	OptNeutrStop = TRUE;//FALSE;
	OptReflectRID = FALSE;
	OptTraceAtoms = TRUE;
	MAXRUN = 1;
	MAXSCEN = 1;
	OptLogSave = TRUE; //!LogOFF;
	
	//OptStrayField = FALSE;
	OptCalOpen = TRUE;
	//OptIonPower = FALSE;
	OptAtomPower = TRUE;
	OptNegIonPower = TRUE;
	OptPosIonPower = TRUE;

	TaskRemnantField = FALSE;
	TaskReionization = FALSE;
	TaskRID = FALSE;
	ThickNeutralization = FALSE;
	LoadSelected = 0;
	ShowProfiles = TRUE;
	SmoothDegree = 0;
	OptDrawPart = FALSE;// TRUE;
	OptSINGAP = FALSE;
	OptIndBeamlets = FALSE;
	//OptDeuterium = TRUE;
	OptPDP = FALSE;
	OptParallel = TRUE; //FALSE;
	OptCombiPlot = -1; ///-1 - no load, 0 - map is calculated, >0 - selected but not calculated 
	OptAddPressure = FALSE;
	
	TaskName = "";
	TaskComment = "";

	//MagFileName = "";
	//MF_7 = TRUE;
	
	PDPgeomFileName = ""; //"file";
	BeamFileName = "";//"Beam_Bert.TXT";
	FW2DFileName = "";//"Prof.txt"
	pMarkedPlate = &emptyPlate; // was new CPlate(), changed by jacob
	pTorCrossPlate = NULL;
	Progress = 0;
	MaxOptRead = 0;
	MaxDataRead = 0;
	MFcoeff = 0;
	GasCoeff = 0;
	DecilType = 1;
	LevelStep = 1; // MW/m2
	LevelMin = 0;
	LevelMax = 10; // MW/m2
	LevelWrite = FALSE;

	OptCalcNeutralizer = TRUE;
	OptCalcRID = TRUE;
	OptCalcDuct = TRUE;
	OptBeamInPlasma = TRUE;
	OptAddDuct = TRUE;//FALSE;
	PlasmaEmitter = -1;
	DuctExit = -1;
	OptCalcBeamTrack = FALSE;
	OptKeepFalls = FALSE;
}

void CBTRDoc:: InitTrackArrays()
{
	ClearScenLoadTracks();//set zero tracks
	//InitScenLoadTrack(MAXSCEN, PlateCounter);//PlatesList.GetSize());
	logout << "Set zero Scen Track arrays \n"; 
}

void CBTRDoc:: ClearScenLoadTracks()// set zero tracks
{
	PowSolid.RemoveAll();
	PowInjected.RemoveAll();
	PowNeutral.RemoveAll();
	for (int i = 0; i <= MAXSCEN; i++) {// Scen = 0 reserved  
			PowNeutral.Add(0); //run 1
			PowInjected.Add(0); //run 1
			PowSolid.Add(C3Point(0,0,0));// X - run1 Y - run2 Z - run3
	}
	for (int i = 0; i < MAXSCENLIMIT; i++) {
		//ScenLoadSummary[i] = new double[Nl];
		for (int j = 0; j < MAXLOADLIMIT; j++) // 0-load for tot bml/power calculated
			ScenLoadSummary[i][j] = 0;
	}
	logout << "Set zero Scen load Track array \n";
}

void CBTRDoc::CreateScenLoadTracks()
{
	int Ns = MAXSCENLIMIT;
	int Nl = MAXLOADLIMIT;
	ScenLoadSummary = new double *[Ns];
	for (int i = 0; i < Ns; i++) {
		ScenLoadSummary[i] = new double[Nl];
		for (int j = 0; j < Nl; j++) // 0-load for tot bml/power calculated
			ScenLoadSummary[i][j] = 0;
	}
	logout << "Created Load Track arrays \n";
}

void CBTRDoc:: InitBeam()
{
	AddData("Source Beam Current, A", NAME(IonBeamCurrent), IonBeamCurrent = 40, 1);
	AddData("Source Beam Energy, MeV", NAME(IonBeamEnergy), IonBeamEnergy = 1., 1);
	IonBeamPower = IonBeamCurrent * IonBeamEnergy; 
//	IonBeamPower = AddData("Ion Beam Power, MW", NAME(IonBeamPower),  IonPower, 1);
	//AddData("BML Radius at GG, m", NAME(BeamRadius), BeamRadius = 0, 1);
	//AddData("BML Focus dist from GG, m", NAME(BeamFocusX), BeamFocusX = 0, 1);
	//AddData("BML Split Type (default 0 - Polar)", NAME(BeamSplitType), BeamSplitType = 0, 0);
	AddData("BML Atoms Polar Split Number (Polar Split)", NAME(PolarNumberAtom), PolarNumberAtom = 100, 0);	
	AddData("BML Atoms Azimuth Split Number (Polar Split)", NAME(AzimNumberAtom), AzimNumberAtom = 120, 0); // one aperture
	//AddData("Beamlet Hor. Split Number (Cartesian)  (>0)", NAME(BeamSplitNumberY), BeamSplitNumberY = 10, 0);
	//AddData("Beamlet Vert. Split Number (Cartesian) (>0)", NAME(BeamSplitNumberZ), BeamSplitNumberZ = 10, 0);
	AddData("BML Reion Polar Split Number (Polar!)", NAME(PolarNumberReion), PolarNumberReion = 0, 0);
	AddData("BML Reion Azimuth Split Number (Polar!)", NAME(AzimNumberReion), AzimNumberReion = 0, 0);
	AddData("BML Resid Polar Split Number (Polar!)", NAME(PolarNumberResid), PolarNumberResid = 0, 0);
	AddData("BML Resid Azimuth Split Number (Polar!)", NAME(AzimNumberResid), AzimNumberResid = 0, 0);
	PolarNumber = PolarNumberAtom;
	AzimNumber = AzimNumberAtom;

//	DecilType = AddData("Beamlet Split Type (0-eq.curr; 1-eq.angle)", NAME(DecilType), 1., 0);	//  0 - equal current  1 - equal angle
//	BeamCoreDiverg = AddData("Beamlet Core Divergence, rad", NAME(BeamCoreDiverg), 0.005, 1);
	AddData("BML Core Horizontal Diverg, rad  (>0)", NAME(BeamCoreDivY), BeamCoreDivY = 0.005, 1);
	AddData("BML Core Vertical Diverg, rad  (>0)", NAME(BeamCoreDivZ), BeamCoreDivZ = 0.005, 1);
	AddData("BML Halo Divergence, rad", NAME(BeamHaloDiverg), BeamHaloDiverg = 0.015, 1);
	AddData("BML Halo Fraction", NAME(BeamHaloPart), BeamHaloPart = 0.15, 1);
	AddData("BML Current Cut-Off ratio (<<1)", NAME(CutOffCurrent), CutOffCurrent = 5.e-2, 2);
	
	PartDimHor = 0; PartDimVert = 0; 
	TracePoints = 1000000;
	TraceOption = 1; // 0-time, 1-length

	// SOURCE PARTICLES -----------------------------------------
	TracePartType = 1; //H+     // -2; D-
/*	TracePartQ  = -1;	TracePartNucl = 1;*/
		
	AddData("Source ions Tracking Step, m", NAME(TraceStepL), TraceStepL = 0.5, 1);
	AddData("Cross-Section H- -> Ho, m2", NAME(NeutrSigma), NeutrSigma = 1.3e-20, 2);
	AddData("Cross-Section Ho -> H+, m2", NAME(ReionSigma), ReionSigma = 3.2e-21, 2);
	AddData("Cross-Section H- -> H+, m2", NAME(TwoStripSigma), TwoStripSigma = 7.0e-22, 2);
	AddData("Cross-Section H+ -> Ho, m2", NAME(PosExchSigma), PosExchSigma = 3.0e-23, 2);
	AddReionSigma = ReionSigma;

	AddData("Neutralization Start, m", NAME(NeutrXmin), NeutrXmin = 1.6, 1);
	AddData("Neutralization End, m", NAME(NeutrXmax), NeutrXmax = 4.6, 1);
	AddData("Neutralization Step, m", NAME(NeutrStepL), NeutrStepL = 0.5, 1);
	AddData("Neutral fraction (Neutr. Efficiency)", NAME(NeutrPart), NeutrPart = 0.6, 1);
	NeutrPower = IonBeamPower * NeutrPart;
	AddData("Residual Positive Ions fraction", NAME(PosIonPart), PosIonPart = 0.2, 1);

	AddData("Re-ionization Start, m", NAME(ReionXmin), ReionXmin = 4.6, 1);
	AddData("Re-ionization End, m", NAME(ReionXmax), ReionXmax = 25, 1);
	AddData("Re-ionization Step, m", NAME(ReionStepL), ReionStepL = 0.1, 1);
	AddData("Reionized/Residual Ions Tracking Step, m", NAME(IonStepL), IonStepL = 0.05, 1);
	
	IonStepLspec = IonStepL; Xspec0 = 0; Xspec1 = 0; // specific area step - for reion tracking 
	ReionStepLspec = ReionStepL; RXspec0 = 0; RXspec1 = 0; //specific reion frequency
	
	char * strMisVert;
	char * strVertTilt;
	if (OptFree)
		strMisVert = "Beam Axis Vert.Tilt + Misalign Angle, rad";
	else
		strMisVert = "Beam Axis Vert. Misalign Angle, rad"; // standard
	if (OptFree)
		strVertTilt = "Beam Axis Vert.Inclination Angle, rad";
	else
		strVertTilt = "Beam Axis Vert. Tilting Angle, rad"; // standard
	AddData("Beam Axis Horiz. Misalign Angle, rad", NAME(BeamMisHor), BeamMisHor = 0, 1);
	//BeamMisVert = AddData("Beam Axis Vert. Misalign Angle, rad", NAME(BeamMisVert), 0, 1);
	//BeamVertTilt = AddData("Beam Axis Vert. Tilting Angle, rad", NAME(BeamVertTilt), 0, 1);
	AddData(strMisVert, NAME(BeamMisVert), BeamMisVert = 0, 1);
	AddData(strVertTilt, NAME(BeamVertTilt), BeamVertTilt = 0, 1);

	AddData("Calc. area Length, m", NAME(AreaLong), AreaLong = AreaLongMax, 1);
	AddData("Calc. area Horiz. min, m", NAME(AreaHorMin), AreaHorMin = -1, 1);
	AddData("Calc. area Horiz. max, m", NAME(AreaHorMax), AreaHorMax = 1, 1);
	AddData("Calc. area Bottom, m", NAME(AreaVertMin), AreaVertMin = -1.0, 1);
	AddData("Calc. area Top, m", NAME(AreaVertMax), AreaVertMax = 1.0, 1);
	 
	SetTraceParticle(TracePartType); // -> TraceTimeStep also
}

void CBTRDoc:: InitNBI() // not active for free config
{
	AddData("NBL Axis Vert. Inclination Angle, rad", NAME(VertInclin), VertInclin = 0, 1);
	AddData("Neutralizer Channels number",  NAME(NeutrChannels), NeutrChannels = 4, 0);//4
	AddData("Neutralizer Entry Coord. X, m",  NAME(NeutrInX), NeutrInX = 1.6, 1);
	AddData("Neutralizer Exit Coord. X, m",  NAME(NeutrOutX), NeutrOutX = 4.6, 1);
	AddData("Neutralizer Height, m",  NAME(NeutrH), NeutrH = 1.7, 1);
	AddData("Neutralizer Channel Entry Width, m",  NAME(NeutrInW), NeutrInW = 0.105, 1);
	AddData("Neutralizer Channel Exit Width, m",  NAME(NeutrOutW), NeutrOutW = 0.095, 1);
	NeutrInW4 = NeutrInW; NeutrOutW4 = NeutrOutW;
	AddData("Neutralizer Panel Entry Thickness, m",  NAME(NeutrInTh), NeutrInTh = 0.04406, 1);
	AddData("Neutralizer Panel Exit Thickness, m",  NAME(NeutrOutTh), NeutrOutTh = 0.03355, 1);
	AddData("Neutralizer Entry Vertical Shift, m",  NAME(NeutrBiasInVert), NeutrBiasInVert = 0, 1);
	AddData("Neutralizer Exit Vertical Shift, m",  NAME(NeutrBiasOutVert), NeutrBiasOutVert = 0, 1);
	AddData("Neutralizer Entry Horizontal Shift, m",  NAME(NeutrBiasInHor), NeutrBiasInHor = 0, 1);
	AddData("Neutralizer Exit Horizontal Shift, m",  NAME(NeutrBiasOutHor), NeutrBiasOutHor = 0, 1);

	AddData("RID Channels number",  NAME(RIDChannels), RIDChannels = 4, 0);//4
	AddData("RID Entry Coord. X, m",  NAME(RIDInX), RIDInX = 5.3, 1);
	AddData("RID Exit Coord. X, m",  NAME(RIDOutX), RIDOutX = 7.1, 1);
	AddData("RID Height, m",  NAME(RIDH), RIDH = 1.7, 1);
	AddData("RID Channel Entry Width, m",  NAME(RIDInW), RIDInW = 0.10376, 1);
	AddData("RID Channel Exit Width, m",  NAME(RIDOutW), RIDOutW = 0.09145, 1);
	RIDInW4 = RIDInW; RIDOutW4 = RIDOutW;
	AddData("RID Panel Thickness, m",  NAME(RIDTh), RIDTh = 0.02, 1);
	AddData("RID Potential, kV",  NAME(RIDU), RIDU = -20, 0);
	AddData("RID Entry Vertical Shift, m",  NAME(RIDBiasInVert), RIDBiasInVert = 0, 1);
	AddData("RID Exit Vertical Shift, m",  NAME(RIDBiasOutVert), RIDBiasOutVert = 0, 1);
	AddData("RID Entry Horizontal Shift, m",  NAME(RIDBiasInHor), RIDBiasInHor = 0, 1);
	AddData("RID Exit Horizontal Shift, m",  NAME(RIDBiasOutHor), RIDBiasOutHor = 0, 1);

	AddData("Calorimeter Entry Coord. X, m",  NAME(CalInX), CalInX = 7.54, 1);
	AddData("Calorimeter Exit Coord. X, m",  NAME(CalOutX), CalOutX = 10.36, 1);
	AddData("Calorimeter Entry Width, m",  NAME(CalInW), CalInW = 0.52, 1);
	AddData("Calorimeter Exit Width, m",  NAME(CalOutW), CalOutW = 0.52, 1);
	AddData("Calorimeter Height, m",  NAME(CalH), CalH = 1.6, 1);

	if (OptAddDuct == FALSE) {
		//AddData("Add Duct surfaces", NAME(OptDuct), OptDuct = 0, 1); 
		return;
	}

	//Scraper Entry -> Dia 5
	AddData("Dia 5 Dist X, m", NAME(PreDuctX), PreDuctX = 10.8,  1);
	AddData("Dia 5 Width, m", NAME(PreDuctW), PreDuctW = 0.45,  1);
	AddData("Dia 5 Height, m", NAME(PreDuctH), PreDuctH = 1.4,  1);
	AddData("Dia 5 Vertical Shift, m",  NAME(DiaBiasVert), DiaBiasVert = 0, 1);
	AddData("Dia 5 Horizontal Shift, m",  NAME(DiaBiasHor), DiaBiasHor = 0, 1);
	//Scraper Exit -> Dia 6
	AddData("Dia 6 Dist X, m", NAME(DDLinerInX), DDLinerInX = 11.73,  1);
	AddData("Dia 6 Width, m", NAME(DDLinerInW), DDLinerInW = 0.54,  1);
	AddData("Dia 6 Height, m", NAME(DDLinerInH), DDLinerInH = 1.4,  1);
	AddData("Dia 6 Vertical Shift, m",  NAME(LinerBiasInVert), LinerBiasInVert = 0, 1);
	AddData("Dia 6 Horizontal Shift, m",  NAME(LinerBiasInHor), LinerBiasInHor = 0, 1);
	// Duct0 -> Dia 7	
	AddData("Dia 7 Dist X, m", NAME(DDLinerOutX), DDLinerOutX = 12.83,  1);
	AddData("Dia 7 Width, m", NAME(DDLinerOutW), DDLinerOutW = 0.54,  1);
	AddData("Dia 7 Height, m", NAME(DDLinerOutH), DDLinerOutH = 1.1,  1);
	AddData("Dia 7 Vertical Shift, m",  NAME(LinerBiasOutVert), LinerBiasOutVert = 0, 1);
	AddData("Dia 7 Horizontal Shift, m",  NAME(LinerBiasOutHor), LinerBiasOutHor = 0, 1);
	// Duct1 -> Dia 8	
	AddData("Dia 8 Dist X, m ", NAME(Duct1X), Duct1X = 12.85,  1);//12.9
	AddData("Dia 8 Width, m", NAME(Duct1W), Duct1W = 0.56,  1);
	AddData("Dia 8 Height, m", NAME(Duct1H), Duct1H = 1.6,  1);//1.3
	AddData("Dia 8 Vertical Shift, m",  NAME(Duct1BiasVert), Duct1BiasVert = 0, 1);
	AddData("Dia 8 Horizontal Shift, m",  NAME(Duct1BiasHor), Duct1BiasHor = 0, 1);
	// Duct2 -> Dia 9	
	AddData("Dia 9 Dist X, m", NAME(Duct2X), Duct2X = 16.8, 1);//16.9
	AddData("Dia 9 Width, m", NAME(Duct2W), Duct2W = 0.56, 1);
	AddData("Dia 9 Height, m", NAME(Duct2H), Duct2H = 1.6, 1);//1.3
	AddData("Dia 9 Vertical Shift, m",  NAME(Duct2BiasVert), Duct2BiasVert = 0, 1);
	AddData("Dia 9 Horizontal Shift, m",  NAME(Duct2BiasHor), Duct2BiasHor = 0, 1);
	// Duct3 -> Dia 10	
	AddData("Dia 10 Dist X, m", NAME(Duct3X), Duct3X = 16.9,  1);//18.1
	AddData("Dia 10 Width, m", NAME(Duct3W), Duct3W = 0.54,  1);
	AddData("Dia 10 Height, m", NAME(Duct3H), Duct3H = 1.3,  1);//1.12
	AddData("Dia 10 Vertical Shift, m",  NAME(Duct3BiasVert), Duct3BiasVert = 0, 1);
	AddData("Dia 10 Horizontal Shift, m",  NAME(Duct3BiasHor), Duct3BiasHor = 0, 1);
	// Duct4 -> Dia 11	
	AddData("Dia 11 Dist X, m", NAME(Duct4X), Duct4X = 18.0,  1);//20.0
	AddData("Dia 11 Width, m", NAME(Duct4W), Duct4W = 0.54,  1);//0.52
	AddData("Dia 11 Height, m", NAME(Duct4H), Duct4H = 1.12,  1);//1.06
	AddData("Dia 11 Vertical Shift, m",  NAME(Duct4BiasVert), Duct4BiasVert = 0, 1);
	AddData("Dia 11 Horizontal Shift, m",  NAME(Duct4BiasHor), Duct4BiasHor = 0, 1);
	// Duct5 -> Dia 12	
	AddData("Dia 12 Dist X, m", NAME(Duct5X), Duct5X = 20.5, 1);//22.4
	AddData("Dia 12 Width, m", NAME(Duct5W), Duct5W = 0.52, 1);//0.52
	AddData("Dia 12 Height, m", NAME(Duct5H), Duct5H = 1.06, 1);//1.06
	AddData("Dia 12 Vertical Shift, m",  NAME(Duct5BiasVert), Duct5BiasVert = 0, 1);
	AddData("Dia 12 Horizontal Shift, m",  NAME(Duct5BiasHor), Duct5BiasHor = 0, 1);
	// Duct6 -> Dia 13	
	AddData("Dia 13 Dist X, m", NAME(Duct6X), Duct6X = 22.5, 1);//23.0
	AddData("Dia 13 Width, m", NAME(Duct6W), Duct6W = 0.52, 1);//0.52
	AddData("Dia 13 Height, m", NAME(Duct6H), Duct6H = 1.06, 1);
	AddData("Dia 13 Vertical Shift, m",  NAME(Duct6BiasVert), Duct6BiasVert = 0, 1);
	AddData("Dia 13 Horizontal Shift, m",  NAME(Duct6BiasHor), Duct6BiasHor = 0, 1);
	// Duct7 -> Dia 14	
	AddData("Dia 14 Dist X, m", NAME(Duct7X), Duct7X = 23.4, 1);
	AddData("Dia 14 Width, m", NAME(Duct7W), Duct7W = 0.52, 1);
	AddData("Dia 14 Height, m", NAME(Duct7H), Duct7H = 1.06, 1);
	AddData("Dia 14 Vertical Shift, m",  NAME(Duct7BiasVert), Duct7BiasVert = 0, 1);
	AddData("Dia 14 Horizontal Shift, m",  NAME(Duct7BiasHor), Duct7BiasHor = 0, 1);
	// Duct8 -> Dia 15	
	AddData("Dia 15 Dist X, m",   NAME(Duct8X), Duct8X = 25.5, 1);
	AddData("Dia 15 Width, m", NAME(Duct8W), Duct8W = 0.52, 1);
	AddData("Dia 15 Height, m", NAME(Duct8H), Duct8H = 1.08, 1);
	AddData("Dia 15 Vertical Shift, m",  NAME(Duct8BiasVert), Duct8BiasVert = 0, 1);
	AddData("Dia 15  Horizontal Shift, m",  NAME(Duct8BiasHor), Duct8BiasHor = 0, 1);

}

void CBTRDoc::InitTokamak()
{	
	AddData("Tor Sectors Number", NAME(TorSegmentNumber), TorSegmentNumber = 36, 0);
	AddData("Tor Centre X in NBI frame, m", NAME(TorCentreX), TorCentreX = 32, 1);//31.952 ITER
	AddData("Tor Centre Y in NBI frame, m", NAME(TorCentreY), TorCentreY = -5.3, 1);//-5.31  ITER
	AddData("Tor Centre Z in NBI frame, m", NAME(TorCentreZ), TorCentreZ = -1.4, 1);//-1.443 ITER
	AddData("Tor Major Radius (R), m", NAME(PlasmaMajorR), PlasmaMajorR = 6.2, 1);// ITER
	AddData("Tor Minor Radius (a), m", NAME(PlasmaMinorR), PlasmaMinorR = 2.2, 1);// ITER
	AddData("Tor Ellipticity, (b/a)", NAME(PlasmaEllipticity), PlasmaEllipticity = 2., 1);//?
	
	AddData("Plasma Max Ne, m-3", NAME(MaxPlasmaDensity), MaxPlasmaDensity = 1.0e20, 2);
	AddData("Plasma Ne profile ord, (0..4)", NAME(PlasmaDensityOrd), PlasmaDensityOrd = 2, 0);
	AddData("Plasma Max Te, keV", NAME(MaxPlasmaTe), MaxPlasmaTe = 10., 1); //FNS
	AddData("Plasma Te profile ord, (0..4)", NAME(PlasmaTeOrd), PlasmaTeOrd = 2, 0);
	AddData("Basic stopping CS, m2", NAME(SigmaBase), SigmaBase = 0.66e-20, 2);//for 500 keV/amu Te = 10keV
	SigmaEnhancement = 0;// 0.4;
		

	// target cross-sections
	AddData("Target Plane A Position X, m", NAME(MovX), MovX = 28.7, 1);
	AddData("Target Plane A Half-Width, m", NAME(MovHor), MovHor = 1.0, 1);
	AddData("Target Plane A Half-Height, m", NAME(MovVert), MovVert = 1.0, 1);
	AddData("Target Plane A Vertical Shift, m", NAME(MovShiftVert), MovShiftVert = 0, 1);

	AddData("Target Plane B Position X, m", NAME(Mov2X), Mov2X = 31.9, 1);
	AddData("Target Plane B Half-Width, m", NAME(Mov2Hor), Mov2Hor = 1.0, 1);
	AddData("Target Plane B Half-Height, m", NAME(Mov2Vert), Mov2Vert = 1.0, 1);
	AddData("Target Plane B Vertical Shift, m", NAME(Mov2ShiftVert), Mov2ShiftVert = 0, 1);

	AddData("Target Plane C Position X, m", NAME(Mov3X), Mov3X = 35.1, 1);
	AddData("Target Plane C Half-Width, m", NAME(Mov3Hor), Mov3Hor = 1.0, 1);
	AddData("Target Plane C Half-Height, m", NAME(Mov3Vert), Mov3Vert = 1.0, 1);
	AddData("Target Plane C Vertical Shift, m", NAME(Mov3ShiftVert), Mov3ShiftVert = 0, 1);

	AddData("Target Plane D Position X, m", NAME(Mov4X), Mov4X = 38.4, 1);
	AddData("Target Plane D Half-Width, m", NAME(Mov4Hor), Mov4Hor = 1.0, 1);
	AddData("Target Plane D Half-Height, m", NAME(Mov4Vert), Mov4Vert = 1.0, 1);
	AddData("Target Plane D Vertical Shift, m", NAME(Mov4ShiftVert), Mov4ShiftVert = 0, 1);
}

void CBTRDoc:: InitPlasma() // called by InitData, OnDataGet
{
//	if (!OptBeamInPlasma) return;
	
	FWdataLoaded = FALSE;
	PlasmaLoaded = FALSE;
	PSILoaded = FALSE;
	ProfLoaded = FALSE;

	//PlasmaMajorR = 6.2; // ITER default tor R
	//PlasmaMinorR = 2.2; // ITER default tor r
	//FWRmin = PlasmaMajorR - PlasmaMinorR; // 4.0  
	//FWRmax = PlasmaMajorR + PlasmaMinorR; // 8.4;// ITER
	
/*	if(IonBeamEnergy < 0.8) { //TIN TorCentreX < 25
		PlasmaMajorR = 3.2; // ITER default tor R
		PlasmaMinorR = 1.2; // ITER default tor r
		FWRmin = 2.0; FWRmax = 4.4;
		
	}*/
	InitFWarray();// FW not loaded yet

	TorCentre.X = TorCentreX; //31.952; // horiz distance from GG centre to InjectPoint
	TorCentre.Y = TorCentreY; //- InjectAimR;
	TorCentre.Z = TorCentreZ; //-1.443;// vert distance from GG centre to Tokamak Center line
	
	if (fabs(TorCentreY) > FWRmax) {
		AfxMessageBox("Beam injects beyond tokamak (FWRmax)");
		//std::cout << "Beam injects beyond tokamak (FWRmax)" << std::endl;
	}

	SetPlasmaTarget();// ->SetStopArrays()

	//MaxPlasmaDensity = 1.5e20; //1e20 TIN; //m-3
	//MaxPlasmaTe = 10;//10 TIN; // keV 
	//SigmaBase = 1.e-20;//6.0e-21; // m2,  1e-20 - for 250 keV/amu; 3e-20 - for 500 keV/amu
	//SigmaEnhancement = 0;// 0.4;
	
	//SetPlasmaGeom();// set beam-tor geometry, plasma Rminor/Rmajor, Xmin/Xmax - called by SetPlasmaTarget()
	//InitFWarray();// FW not loaded yet

	//PlasmaImpurA.RemoveAll();
	//PlasmaImpurW.RemoveAll();
	CSarray.RemoveAll();
	//CSarray.Add(C3Point(10, 1.e-15, 0));//E[keV/nu] SigmaStop [cm2]
	//CSarray.Add(C3Point(1000, 1.e-17, 0));//E[keV/nu] SigmaStop [cm2]
	PlasmaImpurA.SetSize(NofPlasmaImpurMax);
	PlasmaImpurW.SetSize(NofPlasmaImpurMax);
	PlasmaImpurA[0] = 2; // He
	PlasmaImpurA[1] = 3; // Li
	PlasmaImpurA[2] = 4; // Be
	PlasmaImpurA[3] = 5; // B
	PlasmaImpurA[4] = 6; // He
	PlasmaImpurA[5] = 7; // N
	PlasmaImpurA[6] = 8; // O
	PlasmaImpurA[7] = 26; // Fe
	NofPlasmaImpur = 0;
	PlasmaWeightH = 1;	PlasmaWeightD = 1; PlasmaWeightT = 1;
	//std::cout << "Init Plasma - done" << std::endl;
}

void CBTRDoc:: InitMamug()
{
	AddData("Beam Groups (""Segments""): Total Horiz. Number", NAME(NofChannelsHor), NofChannelsHor = 4, 0); // 4
	AddData("Beam Groups (""Segments""): Total Vert. Number", NAME(NofChannelsVert), NofChannelsVert = 4, 0);//4;
	AddData("Beam Groups (""Segments""): Horiz. Step, m", NAME(SegmStepHor), SegmStepHor = 0.160, 1);
	AddData("Beam Groups (""Segments""): Vert. Step, m", NAME(SegmStepVert), SegmStepVert = 0.396, 1);
	AddData("Beam Groups (""Segments""): Horiz. Aiming Dist, m", NAME(BeamAimHor), BeamAimHor = 25.5, 1); 
	AddData("Beam Groups (""Segments""): Vert. Aiming Dist, m", NAME(BeamAimVert), BeamAimVert = 25.5, 1);
	
	AddData("Beamlets: Horiz. Number per Segment", NAME(NofBeamletsHor), NofBeamletsHor = 5, 0);//5;
	AddData("Beamlets: Vert. Number per Segment", NAME(NofBeamletsVert), NofBeamletsVert = 16, 0);//16;
	AddData("Beamlets: Horiz. Step, m", NAME(AppertStepHor), AppertStepHor = 0.020, 1);
	AddData("Beamlets: Vert. Step, m", NAME(AppertStepVert), AppertStepVert = 0.022, 1);
	AddData("Beamlets: Horiz. Aiming Dist, m", NAME(AppertAimHor), AppertAimHor = 7.1, 1); 
	AddData("Beamlets: Vert. Aiming Dist, m", NAME(AppertAimVert), AppertAimVert = 999, 1);//11.75
	
	NofBeamletsTotal =  (int)((NofBeamletsHor) * (NofBeamletsVert) * (NofChannelsHor) * (NofChannelsVert));
	NofBeamlets = NofBeamletsTotal;
	
}

void CBTRDoc:: InitFWarray()
{
	FWdata.RemoveAll();
	int Nf = 36;
	C3Point P;
	int i;
	double df = 2* PI /Nf;
	double r0, R0, dr, Z, fi;
	double ell = PlasmaEllipticity;
	double a = PlasmaMinorR;
	double b = PlasmaMinorR * ell;
	FWRmin = PlasmaMajorR - a; // 4.0  
	FWRmax = PlasmaMajorR + a; // 8.4;// ITER
	FWZmin = TorCentreZ - b;
	FWZmax = TorCentreZ + b;
	
	//elliptic
	R0 = PlasmaMajorR; // to keep tor Rmin/Rmax
	for (int i = 0; i <= Nf; i++) {
		fi = i * df;
		dr = a * cos(fi);
		Z = b * sin(fi);
		P = C3Point(R0 + dr, Z, 0);
		FWdata.Add(P);
	}
	// bi-elliptic
 /*	int N4 = Nf / 4;
	R0 = PlasmaMajorR - 0.5*a; // to keep tor Rmin/Rmax
	for (i = -N4; i < N4; i++) { // external part
		fi = i * df;
		dr = 1.5 * a * cos(fi);
		Z = b * sin(fi);
		P = C3Point(R0 + dr, Z, 0);
		FWdata.Add(P);
	}
	for (i = N4; i <= 3*N4; i++) { // internal part
		fi = i * df;
		dr = 0.5 * a * cos(fi);
		Z = b * sin(fi);
		P = C3Point(R0 + dr, Z, 0);
		FWdata.Add(P);
	}
	*/
}
void CBTRDoc:: InitSumPower()
{
	//AfxMessageBox("InitSumPower");
	int kmax = (int)(AreaLong/SumPowerStepX);
	for (int k = 0; k < kmax; k++) {
		SumReiPowerX.Add(0);
		SumAtomPowerX.Add(1);
	}

	int lmax = (int)(90 / SumPowerAngleStep); // 90 deg
	for (int l = 0; l < lmax; l++) {
		SumPowerAngle.Add(0);
	}
}

void CBTRDoc:: ClearSumPower()
{
	SumReiPowerX.RemoveAll();
	SumAtomPowerX.RemoveAll();
	int kmax = (int)(AreaLong/SumPowerStepX);
	for (int k = 0; k < kmax; k++) {
		SumReiPowerX.Add(0);
		SumAtomPowerX.Add(0);
	}
	SumPowerAngle.RemoveAll();
	int lmax = (int) (90 / SumPowerAngleStep);
	for (int l = 0; l < lmax; l++) {
		SumPowerAngle.Add(0);
	}
}

void CBTRDoc:: InitTaskRID()
{
	AreaLong = RIDOutX + 0.5;//7.4;
	BeamSplitType = 0;
	PolarNumberAtom = 30;
	AzimNumberAtom = 36;
//	BeamSplitNumberY = 10;
//	BeamSplitNumberZ = 10;

	TraceStepL = 0.2;
	IonStepL = 0.01;
	TraceTimeStep = 5.0E-9;
	SetTraceParticle(TracePartType);

	CPlate::DefStepX2 = 0.05;
	CPlate::DefStepY2 = 0.05;
	OptReionAccount = FALSE;
	OptReionStop = FALSE;
	OptNeutrStop = FALSE;
	OptTraceAtoms = FALSE;
	OptReflectRID = TRUE;
	OptCalOpen = FALSE;
	//OptIonPower = TRUE;
	OptNegIonPower = TRUE;
	OptPosIonPower = TRUE;
	OptAtomPower = FALSE;
	
//	RIDField->Set();
	if ((int)BeamSplitType == 0) SetPolarBeamletCommon();//SetBeamletAt(0);
	else SetGaussBeamlet(BeamCoreDivY, BeamCoreDivZ); 
	//SetPolarBeamletReion();
	//SetPolarBeamletResidual();
//	SetSINGAP();
//	SetStatus();
}

void CBTRDoc:: InitTaskReionization()
{
	AreaLong = ReionXmax; //AreaLongMax;
	PolarNumber = PolarNumberReion; // 10;
	AzimNumber = AzimNumberReion; // 12;
	BeamSplitNumberY = 10;
	BeamSplitNumberZ = 10;
//	*TStepNeg = 5.E-8;
//	*TStepPos = 1.1E-8;
//	*TStepNeu = 1.E-8;
	CPlate::DefStepX2 = 0.05;
	CPlate::DefStepY2 = 0.05;
	OptReionAccount = TRUE;
	OptReionStop = FALSE;
	OptNeutrStop = TRUE;
	OptTraceAtoms = TRUE;
	OptReflectRID = FALSE;
	//OptIonPower = TRUE;
	//OnOptionsAccountofreionization();// OptReionAccount -> TRUE 
	OnOptionsStopreionizedparticles();// OptReionStop -> FALSE;
	OptCalOpen = TRUE;
	if (CalOutW < 1.e-6) CalOutW = CalInW;
//	for (int i = 0; i < NofChannelsHor; i++) ActiveCh[i] = TRUE; 
//	for (int j = 0; j < NofChannelsVert; j++) ActiveRow[j] = TRUE;
	
//	RIDField->Set();
	if ((int)BeamSplitType == 0) SetPolarBeamletCommon();//SetBeamletAt(0);
	else SetGaussBeamlet(BeamCoreDivY, BeamCoreDivZ); 
	//SetPolarBeamletReion();
	//SetPolarBeamletResidual();
//	SetSINGAP();
//	SetStatus();
}

void CBTRDoc:: InitTaskTransport()
{
	AreaLong = AreaLongMax;
	PolarNumberAtom = 100;
	AzimNumberAtom = 120;
	BeamSplitNumberY = 60;
	BeamSplitNumberZ = 60;
//	TStepNeg = 1.E-8;
//	TStepPos = 1.1E-8;
//	TStepNeu = 1.2E-8;
	CPlate::DefStepX1 = 0.02;
	CPlate::DefStepY1 = 0.02;
	CPlate::DefStepX2 = 0.05;
	CPlate::DefStepY2 = 0.05;
	OptReionAccount = FALSE;
	OptReionStop = TRUE;
	OptNeutrStop = TRUE;
	OptTraceAtoms = TRUE;
	OptReflectRID = FALSE;
	OptCalOpen = TRUE;
	//OptIonPower = FALSE;
	OptNegIonPower = FALSE;
	OptPosIonPower = FALSE;
	OptAtomPower = TRUE;
	if (CalOutW < 1.e-6) CalOutW = CalInW;
//	for (int i = 0; i < NofChannelsHor; i++) ActiveCh[i] = TRUE; 
//	for (int j = 0; j < NofChannelsVert; j++) ActiveRow[j] = TRUE;
	
//	RIDField->Set();
	if ((int)BeamSplitType ==0) SetPolarBeamletCommon(); //SetBeamletAt(0);
	else SetGaussBeamlet(BeamCoreDivY, BeamCoreDivZ); 
//	SetSINGAP();
//	SetStatus();
}

void CBTRDoc::InitScenDefault()
{
	MAXSCEN = 1;
	ScenFileName = "";
	SkipSurfClass.RemoveAll(); // list of substrings to find in plate Comment
	ExceptSurf.RemoveAll();
		
	for (int i = 0; i <= MAXSCEN; i++)	ScenData[i].RemoveAll();
	ScenLoaded = FALSE;// INIT default scenarios

	ScenData[0].Add("SCEN  = 0\n");// base scenario (config), not runned
	ScenData[0].Add("CONFIG  =" + CurrentDirName + "\\ " + ConfigFileName + "\n");//undefined here!

	ScenData[1].Add("SCEN = 1\n");
/*	ScenData[1].Add("BeamCoreDivY = 0.003 \n");
	ScenData[1].Add("BeamCoreDivZ = 0.003 \n");

	ScenData[2].Add("SCEN = 2\n");
	ScenData[2].Add("BeamCoreDivY = 0.005 \n");
	ScenData[2].Add("BeamCoreDivZ = 0.005 \n");

	ScenData[3].Add("SCEN = 3\n");
	ScenData[3].Add("BeamCoreDivY = 0.007 \n");
	ScenData[3].Add("BeamCoreDivZ = 0.007 \n");
	*/
}

void  CBTRDoc::ReadScenFile() // read scenario data
{
	logout << " Waiting for SCEN data...\n ";

	if (!SkipSurfClass.IsEmpty()) SkipSurfClass.RemoveAll(); // list of substrings to find in plate Comment
	if (!ExceptSurf.IsEmpty()) ExceptSurf.RemoveAll();// exceptions in skip
	
	for (int i = 0; i <= MAXSCEN; i++)	ScenData[i].RemoveAll();
	ScenLoaded = FALSE;// INIT default scenarios
	MAXSCEN = 1;

	char name[1024];
	char buf[1024];
	char dirname[1024];
	CString S;
	CString FileName, FilePath, FileDir;
				
	CFileDialog dlg(TRUE, "dat; txt | * ", "", OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"SCENARIO data (*.txt);(*.dat)  | *.txt; *.TXT; *.dat; *.DAT | All Files (*.*) | *.*||", NULL);

	if (dlg.DoModal() == IDOK) {
		//ResumeData(); // set default
		strcpy(name, dlg.GetPathName());
		ScenFileName = dlg.GetFileName();
		/*	
		FileName = dlg.GetFileName();
		FilePath = dlg.GetPathName();
		strcpy(name, FilePath);
		//ConfigFileName = FileName;
		int fpos = FilePath.Find(FileName, 0);
		if (fpos > 1) FileDir = FilePath.Left(fpos - 1);
		else { AfxMessageBox("Bad FileDir"); return 0; }*/
		//strcpy(dirname, dlg.GetFolderPath());
		//::GetCurrentDirectory(1024, CurrentDirName);
		logout << "READ SCEN file " << name  << "\n";
		
		/*CPreviewDlg pdlg;
		pdlg.m_Filename = name;//name;
		if (pdlg.DoModal() == IDCANCEL) return;*/

		FILE * fin;
		fin = fopen(name, "r");		//fprintf(fout, m_Text);
		DWORD error = ::GetLastError();
		if (fin == NULL || error != 0) { AfxMessageBox("failed to open file"); return; }
		//else AfxMessageBox("open file");
		//m_Text = "";

		// READ SCEN PARAM
		CString Sbuf, Sparam, Sconfig, Sopt, Sskip, Sexcept;
		CStringArray Sdata;// temp array to replace ScenData[i] 
		int maxscen = 0;
		//int maxparam = 0;
		int datsize;//scen data lines 
		int pos, pos1;
		int nskip = 0;
		int nexcept = 0;
		while (!feof(fin) && maxscen < 1) { // find string MAXSCEN, MAXPARAM
			fgets(buf, 1024, fin);
			Sbuf.Format("%s", buf);
			pos = Sbuf.Find("="); // parameters
			if (pos < 1) {
				pos1 = Sbuf.Find(":"); // options
				if (pos1 < 1) continue; // "=" not found
				else pos = pos1;
			} // "=" not found

			CString valname = Sbuf.Left(pos);
			CString valstr = Sbuf.Mid(pos + 1);	//double dVal = atof(valstr);
			//int iVal = atoi(valstr);
			if (valname.Find("MAXSCEN") >= 0) 
				maxscen = atoi(valstr);//iVal;
			//if (valname.Find("MAXPARAM") >= 0) maxparam = iVal;
			if (valname.Find("SKIP") >= 0){
				S = valstr.MakeUpper();
				S.Trim();
				SkipSurfClass.Add(S);
				Sskip += "\t" + S + "\n";
				//Sskip += "\n";
				nskip++;
			}
			if (valname.Find("EXCEPT") >= 0){
				S = valstr.MakeUpper();
				S.Trim();
				ExceptSurf.Add(S);
				Sexcept += "\t" + S + "\n";
				//Sexcept += "\n";
				nexcept++;
			}
		}

		if (nskip > 0) {
			S.Format("%d Load MAPS to skip", nskip);
			AfxMessageBox(S);
			S.Format("\nSKIP MAPS:\n%sEXCEPT:\n%s\n", Sskip, Sexcept);
			logout << S;
		}

		if (maxscen < 1) {
			S.Format("NO scenarios loaded\n"); 
			AfxMessageBox(S);
			logout << S;
			//AfxMessageBox("MAXSCEN = 0 (NO SCEN DATA)");
			return;
		}
		
		MAXSCEN = maxscen; // only if > 0!!!

		int scen = 0;
		int scenend = 0; // ";"
		while (!feof(fin) && scen <= MAXSCEN) {
			fgets(buf, 1024, fin);
			Sbuf.Format("%s", buf);
			pos = Sbuf.Find("="); // parameters
			if (pos < 1) continue; // "=" not found
			CString valname = Sbuf.Left(pos);
			CString valstr = Sbuf.Mid(pos + 1);	//double dVal = atof(valstr);
			int iVal = atoi(valstr);
			
			if (valname.Find("SCEN") >= 0) { // start read scen data
				Sdata.RemoveAll();// clear
				Sdata.Add(Sbuf);// SCEN = Num, 1st - scen caption
				scen = iVal; // SCEN number
				if (scen > MAXSCEN) continue;

				scenend = 0; // ";"
				while (!scenend) {
					fgets(buf, 1024, fin);
					Sparam.Format("%s", buf);
					if (Sparam.Find(";") > -1) scenend = 1;
					else Sdata.Add(Sparam);
				}

				datsize = Sdata.GetSize();
				if (datsize > 0) {
					ScenData[scen].RemoveAll();
					ScenData[scen].Append(Sdata);
				} // scen data set
				else {
					S.Format("No data found for SCEN = %d", scen);
					AfxMessageBox(S);
				}
				
			} // if "SCEN" found

		} // eof or scen > MAXSCEN
		fclose(fin);

		if (scen < MAXSCEN) {// last scen < SCEN limit
			MAXSCEN = scen;
		}
		S.Format("%d scenarios loaded\n", MAXSCEN); 
		AfxMessageBox(S);
		logout << S;// << " - file " << FileName << std::endl;
		
		//SetTitle(name); //AfxMessageBox("SetTitle OK");
		ScenLoaded = TRUE; //AfxMessageBox("Loaded OK");
		ScenData[0].SetAt(1, "CONFIG = " + CurrentDirName + "\\ " + ConfigFileName + "\n");// can be different from specified in ScenFile!!
		Sconfig = ScenData[0][1];
		Sopt = "";
		for (int i = 1; i <= MAXSCEN; i++)
			for (int j = 0; j < ScenData[i].GetSize(); j++)
				Sopt += ScenData[i][j];
		S.Format("%s%s\n", Sconfig, Sopt);
		logout << S; //config << Sopt << std::endl ;
		//AfxMessageBox("Scenario data\n" + Sconfig + Sopt);
		
		// Clear Track Arrays
		InitTrackArrays();// remove all, PowInjected.Add(0), PowSolid.Add(C3Point(0,0,0))
				
	} // file dlg DoModal = OK

	else {
		logout << " SCEN READ Canceled\n ";
		//AfxMessageBox("SCEN READ Canceled");
	}

	return;
}

BOOL CBTRDoc::OnNewDocument()
{
	MAXSCEN = 0; 

	if (!CDocument::OnNewDocument())
		return FALSE;
	
	OptStart = 0; // new analysis - not asked
	OptAddDuct = TRUE; // no duct (not asked)
	OptLogSave = TRUE; //!LogOFF;
	logout.SetLogSave(OptLogSave);// NOT WORKING
	logout.SetFile("logfile.txt");// test log before ReadData
	
	CString s;

	//std::cout << "start NewDocument" << std::endl;
	logout << "start NewDocument\n";

	SetDocument(this);
	OptStart = 0;
	OptFree = FALSE; // default in 4.5 and 5 - STANDARD + Read AddSurfaces
	OptTraceSingle = FALSE;
	
	int res, res1, res2;
	pPlasma = new CPlasma();// must be created before InitData! 
		
	InitData();// calls InitOptions
	//std::cout << "InitData done" << std::endl;
	//logout << "InitData done\n";

	InitScenDefault();
	s.Format("Init Default Scenarious: %d\n", MAXSCEN);
	//std::cout << "Init Default Scenarious: " << MAXSCEN << " (MAXSCEN)" <<  std::endl;
	logout << s;	//MAXSCEN = 1; //single 
	logout << "Close tmp log\n";
	logout.CloseFile();// test log

	GField.reset(new CGasField());
	AddGField = new CGasField();
	GFieldXZ.reset(new CGasFieldXZ());
	RIDField = new CRIDField();
	BField.reset(new CMagField());
	BField3D.reset(new CMagField3D());
	
/*	CStartDlg sdlg; // New task or Results review
	sdlg.m_Start = OptStart;
	if (sdlg.DoModal() != IDOK) return FALSE;
	OptStart = sdlg.m_Start; // 0 - New; 1 - Results; 2 - Demo */
	
	//BTRVersion = 5.0; // not asked now

	CString Sconfig = ScenData[0][1];
	CString Sopt = "";
	for (int i = 1; i <= MAXSCEN; i++)
		for (int j = 0; j < ScenData[i].GetSize(); j++)
			Sopt += ScenData[i][j];

	switch (OptStart) {
	case 0:  // New task
	/*	InputDlg dlg;// set Version! // OLDER - set mode Standard or Free
		dlg.m_Mode = 1;// 1- New, 0 - "Standard" version 4.5 
		if (dlg.DoModal() != IDOK) return FALSE; // not decided
		//OptFree = (dlg.m_Mode != 0);
		if (dlg.m_Mode != 0) Version = 5; // multitask
		else Version = 4.5; // OLD "Standard" + Free
		BTRVersion = Version; // global parameter*/
	
// 1st question
		s.Format(" Choose:\n YES - to specify the Config \n NO - to set default NBI geometry");
		res = AfxMessageBox(s, MB_ICONQUESTION | MB_YESNOCANCEL | MB_APPLMODAL);
		switch (res) {
		case IDYES: //res
			// 2nd question
		/*	s.Format("  Add Duct Surfaces? ");
			res2 = AfxMessageBox(s, MB_ICONQUESTION | MB_YESNOCANCEL | MB_APPLMODAL);
			switch (res2) {
			case IDYES:
				OptAddDuct = TRUE; 
				break;
			case IDNO:
			case IDCANCEL:
				OptAddDuct = FALSE; 
				break;
			} // switch res2 */
			
			if (OnDataGet() == 0) // ReadData, UpdateDataArray, SetBeam, InitPlasma, SetPlates, SetStatus, SetPolarBeamlets 
				SetPlates(); // if data not changed 
			break;
		case IDNO: //res
			OpenLogFile();
			logout << "-------- DEFAULT CONFIG ---------\n";
			break;
		case IDCANCEL: //res
			OpenLogFile();
			logout << "--------- INPUT CANCELLED --------\n";
			if (OptLogSave) 
				CloseLogFile();//logout.log.close();
			return FALSE; // close log before exit

		} //switch res
		//std::cout << "Config is read : " << ConfigFileName << std::endl;
		s.Format("Config is read :  %s\n", ConfigFileName);
		logout << s; // "Config is read : ";logout << ConfigFileName + "\n";

// 2nd question
		s.Format(" YES - set Scenario file now \n NO - set it later");
		res1 = AfxMessageBox(s, MB_ICONQUESTION | MB_YESNOCANCEL | MB_APPLMODAL);
		switch (res1) {
		case IDYES: //res1
			ReadScenFile();//	OnDataGet() == 0)
		/*	ScenData[0].SetAt(1, "CONFIG = " + CurrentDirName + "\ " + ConfigFileName + "\n");// can be different from specified in ScenFile!!
			Sconfig = ScenData[0][1];
			Sopt = "";
			for (int i = 1; i <= MAXSCEN; i++)
				for (int j = 0; j < ScenData[i].GetSize(); j++)
					Sopt += ScenData[i][j];

			AfxMessageBox("Scenario data\n" + Sconfig + Sopt);
			*/
			if (SkipSurfClass.GetSize() > 0) SetPlates();
			break;
		case IDNO: //res1
			//ScenData[0].SetAt(1, ConfigFileName);
			ScenData[0].SetAt(1, "CONFIG = " + CurrentDirName + "\\ "  + ConfigFileName + "\n");
			Sconfig = ScenData[0][1];
			//AfxMessageBox("Defaut Scenario is\n" + Sconfig + Sopt);
			//std::cout << "Scenario : set DEFAULT \n" << Sconfig << Sopt << std::endl;
			s.Format("Scenario : set DEFAULT\n %s %s\n", Sconfig, Sopt); 
			//logout << "Scenario : set DEFAULT \n" ;
			//logout << Sconfig;logout << Sopt;logout << "\n";
			break;
		case IDCANCEL: //res1
			if (OptLogSave) 
				CloseLogFile();//logout.log.close();
			return FALSE;
		} //switch res1
		
					
	/*	if (OptFree) { // FREE option is chosen -> add surfaces
			// not used now
			BOOL finished = FALSE;
			SetPlates();
			while (!finished) {
				s.Format("Free mode is active\n\n Please specify the path to Surf-file \n\n NO -> add no surf and Continue");
				res = AfxMessageBox(s, MB_ICONQUESTION | MB_YESNOCANCEL);
				switch (res) {
				case IDYES:
					OnDataReadaddsurf();
					break;
				case IDNO:
					finished = TRUE;
					break;
				case IDCANCEL:
					return FALSE;
					break;
				} //switch res
			} // finished to add surf = TRUE
		} // add surf to free config */
		break; // New task case 0:

	case 1: // OptStart - View results - switched off
		AfxMessageBox("Some functions can be OFF\n - due to RECONCTRUCTION");
		OnResultsReadall();
		break;
			 // Results view
	case 2: break;  //if (OptStart == 2) // Demo	AfxMessageBox("DEMO is not ready,\n SORRY"); 
	
	} // switch OptStart

	 /*
	{ "DD BOX INTERNAL LINER|FACET2 \n",
		"DD BOX INTERNAL LINER|FACET3 \n",
		"CDLINER TOP|FACET1 \n",
		"CDLINER BOTTOM|FACET2 \n",
		"ABSOLUTE_VALVE | RIGHT 1|FACET1 \n",
		"ABSOLUTE_VALVE | RIGHT 2|FACET2 \n",
		"ABSOLUTE_VALVE | RIGHT 3|FACET3 "
	}; */

	CString Names[10];
	int Nrefl = Reflectors.GetSize();
	for (int i = 0; i < Nrefl; i++)
		Names[i] = Reflectors.GetAt(i);

		
	if (Nrefl == 7) s.Format(" Present version collects Falls for the Surfaces:\n %s\n %s\n %s\n %s\n %s\n %s\n %s\n",
		Names[0], Names[1], Names[2], Names[3], Names[4], Names[5], Names[6]);
	else s.Format("- found reflectors = %d \n", Nrefl);
	
	//AfxMessageBox(s); //" This BTR version collects Falls on the Surfaces:\n" + Names[0] + Names[1] + Names[2] + Names[3]);
	logout << s;

	//OnDataActive();
	FormDataText(0); // no internal names

	ResetLogFile();
		
	return TRUE;
}

void CBTRDoc::AddData(char *  comment, char* name, double& value, int type)
{
	int len, n;

	CString Comment = comment;
	len = Comment.GetLength();
	if (len < 50) {
		n = 50 - len;
		//for (int i = 0; i < n; i++) Comment += "_";
	}			
	DataComment.Add(Comment);

	CString Name = name;
	len = Name.GetLength();
	if (len < 20) {
		n = 20 - len;
		//for (int i = 0; i < n; i++) Name += "_";
	}
	DataName.Add(Name);

	DataType.Add(type);

//	double val = value;
	/*double * pVal = new double;// &value;(double*)&value;//
	*pVal = value;*/
	DataValue.Add(&value);
//	DataValue.Add(&val);
}

void CBTRDoc:: FormDataText(bool fullinfo)// full - for config file, not full - for data view
{
	//AfxMessageBox("FormDataText");
	CString s0, strval, strcomm, strname;
//	int DataN = DataName.GetUpperBound(); 
	
	double value;
	int type;
//	int len;
	CString MFname;
	//if (MF_7) 
	MFname = MagFieldFileName;
	//else MFname = MFManFileName;

	m_Text = "";// .Empty();

	CTime tm = CTime::GetCurrentTime();
	//StartTime = tm;	StopTime = tm;
	CString CurrDate, CurrTime;
	CString Date, Time;
	CurrDate.Format("%02d:%02d:%04d", tm.GetDay(), tm.GetMonth(), tm.GetYear());
	CurrTime.Format("%02d:%02d:%02d", tm.GetHour(), tm.GetMinute(), tm.GetSecond());

	if (!fullinfo) { // on the screen
		if (DataLoaded) {	Date = TaskDate; Time = TaskTime;}
		else { Date = CurrDate; Time = CurrTime;}
	}
	else { // write to file
		Date = CurrDate; Time = CurrTime;
	}

//	s0.Format(" ACTIVE TASK SETTINGS and PARAMETERS (%d + %d) \t DATE %s\t TIME %s \n", MaxOptRead, MaxDataRead, Date, Time);
	s0.Format(" BTR-5 BASE CONFIG    date %s\t  time %s \n", Date, Time);
	m_Text += s0;			
/*	s0.Format("\t  ACTIVE TASK SETTINGS and PARAMETERS \t DATE  %02d:%02d:%04d \t TIME  %02d:%02d:%02d \n\n",
					tm.GetDay(), tm.GetMonth(), tm.GetYear(),
					tm.GetHour(), tm.GetMinute(), tm.GetSecond());*/
	s0.Format(" !!! Press F2 after modifying (or Data-UPDATE in MENU) \n");
	m_Text += s0;
	//TaskName += " > " + GetTitle();
	s0.Format(" TITLE: %s\n", TaskName);
	m_Text += s0; //s0.Format(" TITLE:   %s\n", TaskName);
	s0.Format(" COMMENTS:  %s\n", TaskComment); m_Text += s0;
//	s0.Format(" Current Directory:  %s \n", CurrentDirName); 
	m_Text += "-------------------------------------------------\n"; //s0;
	if (OptSINGAP) {
		s0.Format(" Source Beam:  SINGAP /"); m_Text += s0;
		if (OptIndBeamlets) m_Text += " individual optics \n";
		else m_Text += " common optics \n";
	}
	else {
		s0.Format(" Source Beam:  MAMuG \n");
		m_Text += s0;
	}

	s0.Format(" OPTIONS:  \n\t Magnetic Field:"); m_Text += s0;
	    if (FieldLoaded)  s0.Format(" file < %s >-", MFname); 
		else s0.Format(" < %s > NOT SET, " , MFname);
		m_Text += s0;
		if (fabs(MFcoeff) > 0) s0.Format(" ON");
		else s0.Format(" OFF");
		m_Text += s0;
		if (FieldLoaded)
		{ 
			s0.Format(" * %g", MFcoeff); 
			m_Text += s0;
		}
		m_Text += "\n";
		
		if (PressureLoaded) s0.Format("\t Gas Profile: file  < %s >\n", GasFileName);
		else s0.Format("\t Gas Profile: < %s > NOT SET \n", GasFileName);
		m_Text += s0;
	
		s0.Format("\t Add Surf file or DIR(\\ ended):  < %s > \n", AddSurfName);
		m_Text += s0;

		if (FWdataLoaded) s0.Format("\t Tokamak FW geom: file  < %s >\n", FW2DFileName);
		else s0.Format("\t First Wall 2D-profile: < %s > NOT SET \n", FW2DFileName);
		m_Text += s0;

		//if (PDPgeomLoaded) s0.Format("\t PDP-input: file  <%s>\n", PDPgeomFileName);
		//else s0.Format("\t PDP-input: file  <%s> NOT LOADED \n", PDPgeomFileName);
		//m_Text += s0;
		
		//if (SINGAPLoaded) s0.Format("\t Beamlets: file  < %s >\n", BeamFileName);
		//else s0.Format("\t Beamlets: file  <%s> NOT LOADED \n", BeamFileName);
		//m_Text += s0;

		s0.Format("\t Source Particle:  ");  m_Text += s0;
		switch (TracePartType) {
		case 0: s0.Format("e \n"); break;
		case 1: s0.Format("H+ \n"); break;
		case 2: s0.Format("D+ \n"); break;
		case -1: s0.Format("H- \n"); break;
		case -2: s0.Format("D- \n"); break;
		case 10: s0.Format("Ho \n"); break;
		case 20: s0.Format("Do \n"); break;
		default:  s0.Format("UNKNOWN \n"); break;
		}
		m_Text += s0;
		
		if (OptThickNeutralization) s0.Format("\t Neutralization: THICK \n");
		else s0.Format("\t Neutralization: THIN \n");
		m_Text += s0;
		
		if (OptTraceAtoms) 
			 s0.Format("\t ATOMS tracks after Neutr: ON (TRACE) \n");
		else s0.Format("\t ATOMS tracks after Neutr: OFF (STOP)\n");
		m_Text += s0;
		if (OptNeutrStop) // dont trace
			 s0.Format("\t IONS (resid) after Neutr: OFF (STOP) \n");
		else s0.Format("\t IONS (resid) after Neutr: ON (TRACE) \n");
		m_Text += s0;
		//if (OptReflectRID) s0.Format("\t Reflected ions tracing in RID: ON \n");
		//else s0.Format("\t Reflected ions tracing in RID: OFF \n");
		//m_Text += s0;
		if (OptReionAccount) 
			s0.Format("\t Reion decay calculation: ON - %3.1f %% \n", ReionPercent);
		else s0.Format("\t Reion decay calculation: OFF \n");
		m_Text += s0;
		if (OptReionStop)  // .. dont reace
			 s0.Format("\t REIONS tracks after birth: OFF (STOP)\n");
		else s0.Format("\t REIONS tracks after birth: ON (TRACE)\n");
		m_Text += s0;

		/*if (OptIonPower) s0.Format("\t Solid surfaces accept: IONIC power\n");
		else s0.Format("\t Solid Surfaces accept: TOTAL power\n");
		m_Text += s0;*/
				
		s0.Format("\t Active rows :    "); 
		m_Text += s0;
		if (OptSINGAP) { s0.Format("ALL"); m_Text += s0; }
		else
			for (int n = 0; n < (int)NofChannelsVert; n++) {
			s0.Format("   %d", n+1);
			if (ActiveRow[n]) m_Text += s0;
		} // n
		m_Text += "  \n";
		s0.Format("\t Active channels : "); 
		m_Text += s0;
			if (OptSINGAP) { s0.Format("ALL"); m_Text += s0; }
		else
			for (int k = 0; k < (int)NofChannelsHor; k++) {
			s0.Format("   %d", k+1);
			if (ActiveCh[k])  m_Text += s0;
		} // k
		m_Text += "   \n";
		

/*	if (fullinfo)	s0.Format("%-60s\t\t %-20s\t %-15s \n", "PARAMETER", "    NAME", "  VALUE");
	else			s0.Format("%-60s\t\t %-15s \n", "PARAMETER", "  VALUE");
	m_Text += s0;*/

	char end = '\n';
	CString eq =  '='; //"  = ";
	CString div =  '\t';
	CString stars = " .................. ";
//	char buf[1024];
	CString sresult;
//	len = stars.GetLength();
//	stars.Insert(len+1, div);

//	int CommentSize = 70;
//	int NameSize = 20;

	DataLineN.RemoveAll();
	int k = 0;
	for (int sect = 0; sect < 4; sect++) {
		if (OptFree && sect == 1) continue; // skip NBI geometry
		if (!OptBeamInPlasma && sect == 2) continue; // skip tokamak
		
		m_Text += end + stars + DataSection[sect] + stars + end;
	
		int ibegin = DataBegin[sect];
		int iend = DataEnd[sect];

	for (int i = ibegin; i <= iend; i++) {
		type =  DataType[i];
		value = *DataValue[i];
		switch (type) {
//			case -1: {	m_Text += stars + DataComment[i] + stars + end; break;}// Data Section name
			case 0: strval.Format(" %-15d", (int)value); break;
			case 1: strval.Format(" %-15g", value);  break;
			case 2: strval.Format(" %-15.2e", value);  break;
			} // switch

				if (DataName[i].Find("BeamVerTilt") >=0) {
					if (OptFree) DataComment[i] = "Beam Axis Vert. Inclination Angle, rad";
					else DataComment[i] = "Beam Axis Vert. Tilting Angle, rad"; 
				}
				if (DataName[i].Find("BeamMisVert") >=0) {
					if (OptFree) DataComment[i] = "Beam Axis Vert. Tilt + Misalign Angle, rad";
					else DataComment[i] = "Beam Axis Vert. Misalign Angle, rad";
				}
			strcomm.Format("%-50s", DataComment[i]);
			strname.Format("%-20s", DataName[i]);
			//if (fullinfo) 
				sresult = strcomm + div + strname + div  + eq + strval;
			//else sresult = strcomm + div + eq + strval;
	
	/*		strcomm = DataComment[i]; 
			len = strcomm.GetLength(); 
			if (len < CommentSize) 	for (k = len+1; k < CommentSize; k++) strcomm.Insert(k, '.');
			strname = DataName[i]; 
			len = strname.GetLength(); 
		//	if (len < NameSize) for (k = len+1; k < NameSize; k++) strname.Insert(k, '.');
			sresult = strcomm;
			sresult.Insert(81, " =  ");
			sresult.Insert(90, strname);
	*/	
		
		//if (fullinfo)	m_Text += strcomm + div + strname + div  + eq + strval + end;
		//else			m_Text += strcomm + div + eq + strval + end;

		m_Text += sresult + end;
		DataLineN.Add(k);
		k++;

	} // section end
	} // data end
}

int CBTRDoc:: SetData(CString & Line, int i) // returns at 1st line found
{
	int pos = Line.Find("="); // parameters
	if (pos < 1) return 0;
	int found = 0; // found data-lines with the same Name

	//CString name = DataName[i];//Line.Left(pos);
	CString valstr = Line.Mid(pos+1);
	double Value = atof(valstr);
		
/*	if (!DataLoaded) { // not read from file
			*DataValue[i] = Value;
			return (1);
		}
	
	else // loaded from file
	{*/
		int Nmax = DataName.GetUpperBound();
		for (int k = 0; k <= Nmax; k++) { // search name in data list
				if (Line.Find(DataName[k], 0) >= 0) {
				*DataValue[k] = Value;
				
				found++;

				if (DataName[k].Find("BeamVerTilt") >=0) {
					if (OptFree) DataComment[k] = "Beam Axis Vert. Inclination Angle, rad";
					else DataComment[k] = "Beam Axis Vert. Tilting Angle, rad"; 
				}
				if (DataName[k].Find("BeamMisVert") >=0) {
					if (OptFree) DataComment[k] = "Beam Axis Vert. Tilt + Misalign Angle, rad";
					else DataComment[k] = "Beam Axis Vert. Misalign Angle, rad";
				}
				
				//	DataChanged = TRUE;
				return (found); // return after 1st found!!!
			} // find
		} // nmax
	//} // else

		return found;// 0;

}


int CBTRDoc:: SetOption(CString & Line)
{
	int pos, pos1, pos2;
	Line.TrimLeft();
	pos = Line.Find(":"); // parameters
	if (pos < 1) return (0);
	bool flag = FALSE;
	CString name = Line.Left(pos);
	CString valstr = Line.Mid(pos+1);
	logout <<" + "<< Line << "\n";

//	for (k = 0; k < NofChannelsHor; k++) ActiveCh[k] = FALSE;
//	for (n = 0; n < NofChannelsVert; n++) ActiveRow[n] = FALSE;

	
//	Line.MakeUpper();
	pos1 = Line.Find("DATE");
	if (pos1 < 0) pos1 = Line.Find("date");
	if (pos1 >=0) { // DATE/TIME
		valstr = Line.Mid(pos1+4);
		valstr.TrimLeft();
		//valstr.TrimRight();
		pos2 = valstr.Find("TIME");
		if (pos2 < 0) pos2 = valstr.Find("time");
		if (pos2 >= 0) {
			TaskDate = valstr.Left(pos2);
			if (TaskDate.GetLength() > 10) TaskDate.SetAt(10,'\0'); 
		    pos2 = Line.Find("TIME");
			if (pos2 < 0) pos2 = Line.Find("time");
			valstr = Line.Mid(pos2+4);
			valstr.TrimLeft();
			valstr.TrimRight();
			TaskTime = valstr;
		}

		return (1);//1
	}
	
	if (name.Find("TITLE") > 0) {
		valstr.TrimLeft();
		TaskName = valstr;
		return(1);//2
	}
	if (name.Find("COMMENT") >= 0) {
		valstr.TrimLeft();
		TaskComment = valstr;
		return(1);//3
	}
	if (name.Find("Source") >= 0 && name.Find("Beam") > 0) {
		valstr.MakeUpper();
		if (valstr.Find("SINGAP") > 0) OptSINGAP = TRUE;
		else OptSINGAP = FALSE;
		if (OptSINGAP && valstr.Find("INDIVIDUAL") > 0) OptIndBeamlets = TRUE;
		else OptIndBeamlets = FALSE;
		return(1);//4
	}
/*	if (name.Find("Direct") > 0) {
		strcpy(CurrentDirName, valstr);
		::SetCurrentDirectory(CurrentDirName);
		return(1);//5
	}*/
	
	if (name.Find("Magnetic") >= 0) {
		valstr.MakeUpper();
		if (valstr.Find(" ON") > 0) MFcoeff = 1;
		else MFcoeff = 0;
		pos1 = valstr.Find("<");
		if (pos1 >= 0) { // file name
			CString filename = valstr.Mid(pos1+1);
			pos2 = filename.Find(">");
			if (pos2 >= 0) {
				MagFieldFileName = filename.Left(pos2);
				MagFieldFileName.Trim();
			}
		}
		//MFcoeff = 1;
		pos = valstr.Find("*");
		if (pos > 0) {
			CString s = valstr.Mid(pos+1);
			double Value = atof(s);
			MFcoeff = Value;
		} 
			return(1);//5
		}
	
	if (name.Find("Pressure") >= 0 || name.Find("Gas") >= 0){
		/*if (valstr.Find("NOT") > 0) PressureLoaded = FALSE;
			else PressureLoaded = TRUE;*/
		pos1 = valstr.Find("<");
		if (pos1 >= 0) {
			CString filename = valstr.Mid(pos1+1);
			pos2 = filename.Find(">");
			if (pos2 >= 0) {
				GasFileName = filename.Left(pos2);
				GasFileName.Trim();
			}
		}
		return(1);//6
	}
/*
	if (name.Find("PDP") > 0) {
		pos1 = valstr.Find("<");
		if (pos1 >= 0) {
			CString filename = valstr.Mid(pos1+1);
			pos2 = filename.Find(">");
			if (pos2>=0) PDPgeomFileName = filename.Left(pos2);
		}
		return(1);//7
	}
*/
	if (name.Find("Add") >= 0 && name.Find("Surf") >= 0 ) {
		pos1 = valstr.Find("<");
		if (pos1 >= 0) {
			CString name = valstr.Mid(pos1+1);
			pos2 = name.Find(">");
			if (pos2>=0) {
				AddSurfName = name.Left(pos2);
				AddSurfName.Trim();
			}
		}
		return(1);//7
	}

	if (name.Find("First") >= 0 || name.Find("Wall") > 0){
		pos1 = valstr.Find("<");
		if (pos1 >= 0) {
			CString filename = valstr.Mid(pos1+1);
			pos2 = filename.Find(">");
			if (pos2>=0) {
				FW2DFileName = filename.Left(pos2);
				FW2DFileName.Trim();
			}
		}
		return(1);//8
	}
	/*
	if (name.Find("Beamlets") > 0) {
		pos1 = valstr.Find("<");
		if (pos1 >= 0) {
			CString filename = valstr.Mid(pos1+1);
			pos2 = filename.Find(">");
			if (pos2>=0) BeamFileName = filename.Left(pos2);
		}
		return(1);//7
	}*/

	if (name.Find("Source") > -1 && name.Find("Particle") > -1) {
		//OptDeuterium  = FALSE; // new data version
		valstr.MakeUpper();
		if (valstr.Find("E") >= 0) TracePartType = 0;
		if (valstr.Find("H-") >= 0) TracePartType = -1;
		if (valstr.Find("H+") >= 0) TracePartType = 1;
		if (valstr.Find("D-") >= 0) { TracePartType = -2; } // OptDeuterium = TRUE;
	
		if (valstr.Find("D+") >= 0) TracePartType = 2; 
		if (valstr.Find("H0") >= 0 || valstr.Find("HO") >= 0) TracePartType = 10;
		if (valstr.Find("D0") >= 0 || valstr.Find("DO") >= 0) TracePartType = 20;
		//	else OptDeuterium = TRUE; // - old  version
		
		SetTraceParticle(TracePartType);
		logout << ">>>>>> SetOption: TracePartType " << TracePartType << valstr << "\n" ;
		return(1);//9
	}
	if (name.Find("Neutralization") > -1) {
		valstr.MakeUpper();
		if (valstr.Find("THICK") > 0) OptThickNeutralization = TRUE;
		else OptThickNeutralization = FALSE;

		//OptThickNeutralization = FALSE; // up to v 5.1 

		//AfxMessageBox("Neutralization SET");
		return(1);//10
	}
	/* // OLD CONFIG version < 5
	 Residual Ions tracing after Neutralization: OFF 
	 Atoms tracing after Neutralization: ON 
	 Reflected ions tracing in RID: OFF 
	 Re-ionization: ON - 5.5 
	 Re-ionized particles tracing: OFF 
	 */
	/* // NEW config version 5
	ATOMS tracks after Neutr: ON (TRACE) 
	IONS (resid) after Neutr: OFF (STOP)
	Reion decay calculation: ON - %3.1f %% \n", ReionPercent);
	REIONS tracks after birth: OFF (STOP)\n");
	*/	
	flag = FALSE;
	if (name.Find("Atoms") > -1 && name.Find("tracing") > -1) flag = TRUE;  // old
	if (name.Find("ATOMS") > -1 && name.Find("tracks") > -1) flag = TRUE; // new
	if (flag) { // ATOMS tracing
		valstr.MakeUpper();
		if (valstr.Find("OFF") >= 0) OptTraceAtoms = FALSE; //AfxMessageBox("Atoms STOPPED");}
		else OptTraceAtoms = TRUE; //AfxMessageBox("Atoms TRACED"); }
		return(1);//11
	}

	flag = FALSE;
	if (name.Find("Residual") > -1 && name.Find("tracing") > -1) flag = TRUE;  // old
	if (name.Find("IONS") > -1 && name.Find("resid") > -1) flag = TRUE;  // new
	if (flag) { // Residuals tracing
		valstr.MakeUpper();
		if (valstr.Find("OFF") > -1) OptNeutrStop = TRUE; //AfxMessageBox("Residual STOPPED");}
		else OptNeutrStop = FALSE; //AfxMessageBox("Residual TRACED"); }
		return(1);//12 Residuals tracing 
	}

	flag = FALSE;
	if (name.Find("Re-ionization") > -1) flag = TRUE;  // old
	if (name.Find("Reion")  > -1 && name.Find("decay")  > -1) flag = TRUE; // new   
	if (flag) //(name.Find("Re-ionization") > -1)
	{
		valstr.MakeUpper();
		if (valstr.Find("ON") >= 0) OptReionAccount = TRUE;
		else OptReionAccount = FALSE;
		return(1);//13 found reion account 
	}

	flag = FALSE;
	if (name.Find("Re-ionized") > -1 && name.Find("tracing") > -1) flag = TRUE;  // old
	if (name.Find("REIONS") > -1 && name.Find("tracks") > -1) flag = TRUE; // new
	if (flag) { // Re-ionized tracing
		valstr.MakeUpper();
		if (valstr.Find("OFF") >= 0) OptReionStop = TRUE;
		else OptReionStop = FALSE;
		return(1);//14 Re-ionized tracing
	}
		
/*
	if (name.Find("Reflected") > -1) {
		valstr.MakeUpper();
		if (valstr.Find("OFF") > 0) OptReflectRID = FALSE;
		else OptReflectRID = TRUE;
		//AfxMessageBox("Neutralization SET");
		return(1);//8
	}*/
	/*if (name.Find("Solid") > -1 && name.Find("surfaces") > -1) {
		valstr.MakeUpper();
		if (valstr.Find("IONIC") > 0) OptIonPower = TRUE;
		else OptIonPower = FALSE;
		return(1);//11
	}*/
	

/*	if (name.Find("Calorimeter") > 0) {
		valstr.MakeUpper();
		if (valstr.Find("OPEN") > 0) OptCalOpen = TRUE;
		else OptCalOpen = FALSE;
		return(1);//12
	}
*/	
	if (name.Find("Active rows") > -1) {
		valstr.MakeUpper();
	//	if (!OptSINGAP){
			if (valstr.Find("1") >= 0 || valstr.Find("ALL") >= 0) ActiveRow[0] = TRUE;
			else ActiveRow[0] = FALSE;
			if (valstr.Find("2") >= 0 || valstr.Find("ALL") >= 0) ActiveRow[1] = TRUE;
			else ActiveRow[1] = FALSE;
			if (valstr.Find("3") >= 0 || valstr.Find("ALL") >= 0) ActiveRow[2] = TRUE;
			else ActiveRow[2] = FALSE;
			if (valstr.Find("4") >= 0 || valstr.Find("ALL") >= 0) ActiveRow[3] = TRUE;
			else ActiveRow[3] = FALSE;
			if (valstr.Find("5") >= 0 || valstr.Find("ALL") >= 0) ActiveRow[4] = TRUE;
			else ActiveRow[4] = FALSE;
			if (valstr.Find("6") >= 0 || valstr.Find("ALL") >= 0) ActiveRow[5] = TRUE;
			else ActiveRow[5] = FALSE;
			if (valstr.Find("7") >= 0 || valstr.Find("ALL") >= 0) ActiveRow[6] = TRUE;
			else ActiveRow[6] = FALSE;
			if (valstr.Find("8") >= 0 || valstr.Find("ALL") >= 0) ActiveRow[7] = TRUE;
			else ActiveRow[7] = FALSE;
			if (valstr.Find("9") >= 0 || valstr.Find("ALL") >= 0) ActiveRow[8] = TRUE;
			else ActiveRow[8] = FALSE;
			if (valstr.Find("10") >= 0 || valstr.Find("ALL") >= 0) ActiveRow[9] = TRUE;
			else ActiveRow[9] = FALSE;
	//	}
		return(1); //15
	} // rows

	if (name.Find("Active channels") > 0) {
		valstr.MakeUpper();
//	if (!OptSINGAP){
			if (valstr.Find("1") >= 0 || valstr.Find("ALL") >= 0) ActiveCh[0] = TRUE;
			else ActiveCh[0] = FALSE;
			if (valstr.Find("2") >= 0 || valstr.Find("ALL") >= 0) ActiveCh[1] = TRUE;
			else ActiveCh[1] = FALSE;
			if (valstr.Find("3") >= 0 || valstr.Find("ALL") >= 0) ActiveCh[2] = TRUE;
			else ActiveCh[2] = FALSE;
			if (valstr.Find("4") >= 0 || valstr.Find("ALL") >= 0) ActiveCh[3] = TRUE;
			else ActiveCh[3] = FALSE;
			if (valstr.Find("5") >= 0 || valstr.Find("ALL") >= 0) ActiveCh[4] = TRUE;
			else ActiveCh[4] = FALSE;
			if (valstr.Find("6") >= 0 || valstr.Find("ALL") >= 0) ActiveCh[5] = TRUE;
			else ActiveCh[5] = FALSE;
			if (valstr.Find("7") >= 0 || valstr.Find("ALL") >= 0) ActiveCh[6] = TRUE;
			else ActiveCh[6] = FALSE;
			if (valstr.Find("8") >= 0 || valstr.Find("ALL") >= 0) ActiveCh[7] = TRUE;
			else ActiveCh[7] = FALSE;
			if (valstr.Find("9") >= 0 || valstr.Find("ALL") >= 0) ActiveCh[8] = TRUE;
			else ActiveCh[8] = FALSE;
			if (valstr.Find("10") >= 0 || valstr.Find("ALL") >= 0) ActiveCh[9] = TRUE;
			else ActiveCh[9] = FALSE;
	//	}
		return(1);	//16
	} // channels

	return(0);

}

void CBTRDoc:: UpdateDataArray()
{
//	m_Text.Empty();// = '\0';
	TaskName.Empty();// = "...";
	int k,n;
//	for (k=0; k < NofChannelsHor; k++) ActiveCh[k] = FALSE;
//	for (k=0; k < NofChannelsVert; k++) ActiveRow[k] = FALSE;

	CString text;
	text = m_Text;
	//if (m_Text.Find("NB") >=0 && m_Text.Find("CONFIG") >= 0) OptFree = FALSE;// standard NBI
	//else OptFree = TRUE;

	CString line;
	MaxOptRead = 0;
	MaxDataRead = 0;
	int opt = 0, data = 0;// numbers or successfully read options/data 
	int pos; // current position of symbol \n
	int i = 0; // attempt reading data
	while (text.GetLength() > 0) {
		pos =  text.FindOneOf("\n\0");// text.Find("\n", n);
		if (pos>0) {
			line = text.Left(pos) ;
			if (line.Find("=",0) > 0 && data <= DataName.GetUpperBound()) {
				int res = SetData(line, data);
				if (res > 0) data++;
				i++;
				//maxopt = 12; // options are before parameters
			}
			else if (line.Find(":") > 0 && data == 0)
				opt += SetOption(line);

			text.Delete(0, pos);// remove head!!!

		} // pos>0
		else // pos =0
			 text.Delete(0,1);	
		
	} // textLength >0
	MaxOptRead = opt;
	MaxDataRead = data;

/*	for (k=0; k < NofChannelsHor; k++) {
		if (Active[k] == TRUE) Nmax++;
	}*/
	if (MaxOptRead < 2) { //(Nmax==0) {
		for (k=0; k < (int)NofChannelsHor; k++) ActiveCh[k] = TRUE;
		for (n=0; n < (int)NofChannelsVert; n++) ActiveRow[n] = TRUE;
		TaskComment = "Options not found or contain forbidden symbols <=>";
	} 

	//AreaLong = AreaLongMax;
	//CheckData();// -> Later
	
	DataLoaded = TRUE;
	CString s;
	s.Format("- %d options (lines) identified : \n", opt);
	logout << s; //opt << " options (lines) identified "  << std::endl;
	logout << "Update DataArray - done\n";

}


void CBTRDoc:: CheckData()
{
	CString S, S1;

/*	if (AreaLong > TorCentreX  && !FWdataLoaded) {
		S = FW2DFileName;	S1 = FindFileName(S);
		AfxMessageBox("To do:\n Tokamak First Wall profile should be defined \n\n" + S1);
	}*/
	if (fabs(MFcoeff) > 0 && !FieldLoaded) {
		S = MagFieldFileName; //MFManFileName; //MagFieldFileName;	
		S1 = FindFileName(S);
		//AfxMessageBox("To do:\n MF should be defined \n\n" + S1); 
		MFcoeff = 0;
	}

	if (OptThickNeutralization && (!PressureLoaded || GasCoeff < 1.e-6)) {
		//S = GasFileName;	S1 = FindFileName(S);
		//AfxMessageBox("To do:\n Gas profile should be defined \n (for THICK neutralization)\n\n" + S1); 
		OptThickNeutralization = FALSE;
	}

	if (OptReionAccount && (!PressureLoaded || GasCoeff < 1.e-6)) OptReionAccount = FALSE;

	if (!OptReionAccount && !OptReionStop) OptReionStop = TRUE;
	/*if ((OptReionAccount || !OptReionStop)  && !PressureLoaded) {
		S = GasFileName;	S1 = FindFileName(S);
		//AfxMessageBox("To do:\n Gas profile should be defined \n (for Re-ionization)\n\n" + S1);
		OptReionAccount = FALSE;
		OptReionStop = TRUE;
	}

	if (PressureLoaded && GasCoeff > 1.e-6 && !OptReionStop) OptReionAccount = TRUE;
	*/
	
	if (!OptReionStop && !FieldLoaded) {
		S = MagFieldFileName; //MFManFileName; //	
		S1 = FindFileName(S);
		//AfxMessageBox("To do:\n MF should be defined\n (to trace Re-ionized particles)\n\n" + S1);
		OptReionStop = TRUE;
	}
	
	PolarNumber = PolarNumberAtom;
	AzimNumber = AzimNumberAtom;

	if ((int)AzimNumber < 4 && (int)PolarNumber > 0) {
		//AfxMessageBox("ErrInput:\n Azimuth Number should be > 3\n Otherwise Polar Splitting Number is set to 0");
		PolarNumber = 0;
	}

	if ((int)BeamSplitNumberY < 1) {
		//AfxMessageBox("CErrInput:\n Cartesian Splitting Numbers should be > 0!\n Number is set to 1");
		BeamSplitNumberY = 1;
	}

	if ((int)BeamSplitNumberZ <1) {
		//AfxMessageBox("CErrInput:\n Cartesian Splitting Numbers should be > 0!\n Number is set to 1");
		BeamSplitNumberZ = 1;
	}

	if (BeamCoreDivY < 1.e-16) {
		AfxMessageBox("Input:\n Beam Core Divergence should be > 0!\n Value is set to 0.001 rad");
		BeamCoreDivY = 0.001;
	}

	if (BeamCoreDivZ < 1.e-16) {
		AfxMessageBox("Input:\n Beam Core Divergence should be > 0!\n Value is set to 0.001 rad");
		BeamCoreDivZ = 0.001;
	}

	if (!OptThickNeutralization) {// THIN neutralization
	if (NeutrPart >= 1) {
		AfxMessageBox("Neutral fraction = 1\n Positive & Negative Fractions are set to 0");
		NeutrPart = 1; PosIonPart = 0;
	}

	if (NeutrPart < 1 &&  (1.00001 - NeutrPart - PosIonPart < 1.e-10)) {
	//	double posfr = 1 - NeutrPart;
		S.Format("Neutral + Positive fractions > 1!\n Positive Fraction is set to 0\n (Negative Fraction = 1 - Neutral - Positive)");
		AfxMessageBox(S);
		PosIonPart = 0;
	}
	}// THIN neutralization

	if (TraceStepL < 1.e-3) {
		S.Format(" -> FAST and THIN options are set\n when TraceStepL < 1.e-3");
		OptThickNeutralization = FALSE;
		AfxMessageBox(S);
	}
	/*
	if (OptThickNeutralization) {// THICK neutralization
	if (NeutrStepL < TraceStepL) {//THICK neutralization
		AfxMessageBox("SourceIon step should not exceed Neutralization step");
		NeutrStepL = TraceStepL;
	}
	}*/
	
	if (OptKeepFalls == TRUE)
		AfxMessageBox("Falls will be kept");


	//if (IonStepLspec > IonStepL)	IonStepLspec = IonStepL;

	//if (NeutrXmax > ReionXmin) ReionXmin = NeutrXmax;

/*	if (CalOutW < 1.e-6) OptCalOpen = FALSE;
	else OptCalOpen = TRUE;*/

///////// Beam Groups
	if ((int)NofChannelsHor > 1 && fabs(SegmStepHor) < 1.e-6) {
		S.Format("ErrInput:\n Beam Groups Horizontal Step must be non-zero!");
		AfxMessageBox(S);
	}
	if ((int)NofChannelsHor > 1 && fabs(BeamAimHor) < 1.e-6) {
		S.Format("ErrInput:\n Beam Groups Horizontal Aiming Distance must be non-zero!");
		AfxMessageBox(S);
	}
	if ((int)NofChannelsVert > 1 && fabs(SegmStepVert) < 1.e-6) {
		S.Format("ErrInput:\n Beam Groups Vertical Step must be non-zero!");
		AfxMessageBox(S);
	}
	if ((int)NofChannelsVert > 1 && fabs(BeamAimVert) < 1.e-6) {
		S.Format("ErrInput:\n Beam Groups Vertical Aiming Distance must be non-zero!");
		AfxMessageBox(S);
	}
////////// Beamlets
	if ((int)NofBeamletsHor > 1 && fabs(AppertStepHor) < 1.e-6) {
		S.Format("ErrInput:\n Beamlets Horizontal Step must be non-zero!");
		AfxMessageBox(S);
	}
	if ((int)NofBeamletsHor > 1 && fabs(AppertAimHor) < 1.e-6) {
		S.Format("ErrInput:\n Beamlets Horizontal Aiming Distance must be non-zero!");
		AfxMessageBox(S);
	}
	if ((int)NofBeamletsVert > 1 && fabs(AppertStepVert) < 1.e-6) {
		S.Format("ErrInput:\n Beamlets Vertical Step must be non-zero!");
		AfxMessageBox(S);
	}
	if ((int)NofBeamletsVert > 1 && fabs(AppertAimVert) < 1.e-6) {
		S.Format("ErrInput:\n Beamlets Vertical Aiming Distance must be non-zero!");
		AfxMessageBox(S);
	}
/*	if (fabs(TorCentreY) > PlasmaMajorR + PlasmaMinorR || fabs(TorCentreZ) > PlasmaMinorR* PlasmaEllipticity) {
		TorCentreY = PlasmaMajorR;
		TorCentreZ = 0;
		S.Format("ErrInput: corrected \n Beam is shot to plasma Rt = Rmajor, Zt = 0!");
		AfxMessageBox(S);
	}*/


	double Ymin, Ymax, Zmin, Zmax;
	GetBeamFootLimits(AreaLong, Ymin, Ymax, Zmin, Zmax);
	logout << "Check Data - done\n";
/*	
	AreaHorMin = Min(-1.0, Ymin);
	AreaHorMax = Max(1.0, Ymax);
	AreaVertMin = Min(-1.0, Zmin);
	AreaVertMax = Max(1.0, Zmax);
	AreaLong = AreaLongMax;
*/
	
}

void  CBTRDoc::	RotateVert(double angle, double & X,  double & /* Y */,  double & Z)// rotate velocity vector
{
	double alfa = angle, x = X, z = Z;
	//X = x * cos(alfa) + z * sin(alfa);
	//Z = -x * sin(alfa) + z * cos(alfa);
	X = x * cos(alfa) - z * sin(alfa);
	Z = x * sin(alfa) + z * cos(alfa);
	return;
}

void  CBTRDoc::	RotateHor(double angle, double & X,  double & Y,  double & /* Z */)// rotate velocity vector
{
	double  x = X, y = Y;
	double alfa = angle;
	//X = x * cos(alfa) + y * sin(alfa);
	//Y = -x * sin(alfa) + y * cos(alfa);
	X = x * cos(alfa) - y * sin(alfa);
	Y = x * sin(alfa) + y * cos(alfa);
	return;
}

C3Point  CBTRDoc:: CentralCS(C3Point  P0, int segmHor, int segmVert) //returns appert pos in Central CS
{
	//P0 - in local (segment) CS
	C3Point P, Centre;
	Centre.X = 0;
	Centre.Y =  SegmCentreHor[segmHor]; 
	Centre.Z =  SegmCentreVert[segmVert];
	P = P0 + Centre;
	return P;
}

C3Point  CBTRDoc:: LocalCS(C3Point  P0, int channel)
{
	if (channel == 0) return P0;
	C3Point P = P0;
/*	double alfa = channel * PI* (*AimBeam) /180.;
	double X0 = *SideIScentreX;
	double Y0 = (*AimDistX - *SideIScentreX)*tan(alfa);
	P.X = (P0.X - X0) * cos(alfa) - (P0.Y-Y0) * sin(alfa);
	P.Y =  (P0.X - X0) * sin(alfa) + (P0.Y-Y0) * cos(alfa);
	P.Z = P0.Z;*/
	return P;
}

void  CBTRDoc::SetTraceParticle(int nucl, int q) // 0:e, 1:H+, 2:D+, -1:H-, -2:D-, 10:H0, 20:D0
{
	switch (nucl) { // 0(e), 1(H-), 2(D-)
	case 0: SetTraceParticle(0); break;
	case 1: 
		switch (q) {
		case 0: SetTraceParticle(10); break; // Ho
		case 1: SetTraceParticle(1); break; // H+
		case -1: SetTraceParticle(-1); break; // H-
		default: SetTraceParticle(-1); break; // H-
		}
		break;
	case 2: 
		switch (q) {
		case 0: SetTraceParticle(20); break; // Do
		case 1: SetTraceParticle(2); break; // D+
		case -1: SetTraceParticle(-2); break; // D-
		default: SetTraceParticle(-2); break; // D-
		}
		break;
	}
}


void  CBTRDoc::SetTraceParticle(int type) // 0:e, 1:H+, 2:D+, -1:H-, -2:D-, 10:H0, 20:D0
{
	switch (type) { // 0(e), 1(H-), 2(D-)
	case 0:	// electron
			TracePartType = type;
			TracePartMass = Me;
			TracePartQ = -1; 
			TracePartNucl = 0;
			break;
	
	case 1:	// H+
			TracePartType = type; 
			TracePartMass = Mp;
			TracePartNucl = 1;
			TracePartQ = 1;
			break;
	case 2: // D+
			TracePartType = type; 
			TracePartMass = Mp * 2;
			TracePartNucl = 2;
			TracePartQ = 1;
			break;

	case -1:// H-
			TracePartType = type; 
			TracePartMass = Mp;
			TracePartNucl = 1;
			TracePartQ = -1;
			break;
	case -2: // D-
			TracePartType = type; 
			TracePartMass = Mp * 2;
			TracePartNucl = 2;
			TracePartQ = -1;
			break;

	case 10: // H0
			TracePartType = type; 
			TracePartMass = Mp;
			TracePartNucl = 1;
			TracePartQ = 0;
			break;

	case 20: // D0
			TracePartType = type; 
			TracePartMass = Mp * 2;
			TracePartNucl = 2;
			TracePartQ = 0;
			break;

	default: // H+ //any ion Z+ or Z-
			TracePartType = type; 
			TracePartMass = Mp;
			TracePartNucl = 1;
			TracePartQ = 1;
			break;
				
	}


	double EeV = IonBeamEnergy * 1000000;
	
	double V;
	if (type == 0) V = RelV(IonBeamEnergy); // MeV // electron
	else 	V = sqrt(2.* EeV * Qe / TracePartMass); // {m/c}
	
	//if (TraceOption == 0) TraceStepL = TraceTimeStep * V;
	//else  
	TraceTimeStep = TraceStepL / V;
					 
}

void CBTRDoc::SetStatus() // numbers of active channels/rows/ totbeamlets / tot particles to trace
{
	int N; //  beamlets per segment
	int Nrays; // rays per beamlet
	
	if (OptSINGAP) {
		NofBeamlets = BeamEntry_Array.GetSize(); 
		NofActiveChannels = (int)NofChannelsHor;
		NofActiveRows = (int)NofChannelsVert;
	}
	
	else {// MAMuG
		N = (int)NofBeamletsHor * (int)NofBeamletsVert;
		NofActiveChannels = 0;
		NofActiveRows = 0;
	
	for (int k = 0; k < (int)NofChannelsHor; k++) 
		if (ActiveCh[k]) NofActiveChannels += 1;
	for (int n = 0; n < (int)NofChannelsVert; n++) 
		if (ActiveRow[n]) NofActiveRows += 1;
		//NofBeamlets = N *(NofActiveChannels)*(NofActiveRows); // * NofChannelsVert;
		if (!SINGAPLoaded) SetSINGAPfromMAMUG();//NofBeamlets = BeamEntry_Array.GetSize(); 
	} // MAMuG

	Nrays = Attr_Array.GetSize();//Nattr = pDoc->Attr_Array.GetSize();

	MultCoeff = 1;// source particle multiplication

	int Kneutr = 0, Kpos = 0, Kreion = 0; // generated from source part
	
	if (!OptNeutrStop) {// Ions are traced in RID
		if (OptThickNeutralization) { 
			Kneutr = NeutrArray.GetSize();
			Kpos = Kneutr;
		}
		else { Kneutr = 1; Kpos = 1; } // THIN
	} // Ions are traced in RID
	else { // residual ions are stopped
		Kpos = 0;
		if (OptThickNeutralization) Kneutr = NeutrArray.GetSize();
		else Kneutr = 1; // THIN
	}

	if (!OptReionStop)  // Reions are traced
		Kreion = ReionArray.GetSize();
	else Kreion = 0;

	//if (TracePartQ == 0) 	MultCoeff = 1 + Kreion;
	//else MultCoeff = 1 + Kneutr + Kpos + Kneutr * Kreion;
	
	if (BTRVersion < 5.0) {// 4.5
		MultCoeff = Nrays * (1 + Kneutr + Kpos + Kneutr * Kreion);
		// Nrays = N of source neg ions
	}
	else { // >= 5.0
		int NResid = Attr_Array_Resid.GetSize();
		int NReion = Attr_Array_Reion.GetSize();
		Kpos = 0;
		if (!OptNeutrStop) Kpos = 1;// only THIN neutr
		MultCoeff = Nrays + NResid*Kpos*2 + NReion*Kreion;
		// Nrays = N of atoms
	}

	int runs = 0;
	if (OptTraceAtoms != 0) runs++;
	if (OptNeutrStop == 0) runs++;
	if (OptReionStop == 0) runs++;
	if (runs == 0) {
		OptTraceAtoms = 1;
		runs = 1;
	}
	MAXRUN = runs;
	 
	GetMemState();
	logout << "MEM : BTR takes " << MemUsed << " of " << MemFree << " available\n";

}

bool CBTRDoc::SelectPlate(CPlate* plate)
{
	bool flag;
	plate->Selected = TRUE; // !(plate->Selected);
	//plate->Loaded = !(plate->Loaded); // Mark / Unmark for Load
	flag = TRUE; // plate->Selected;
		
	SetNullLoad(plate);//plate->ApplyLoad(flag, 0,0); // *LoadStepX, *LoadStepY);
	SetLoadArray(plate, flag);
	
	return flag;
} 

bool CBTRDoc:: SelectPlate(CPlate* plate, double hx, double hy)
{
	bool flag;
	plate->Selected = TRUE;
	//plate->Loaded = TRUE;//!(plate->Loaded); // Mark / Unmark for Load
	flag = plate->Selected; //Loaded;
	//plate->Touched = FALSE;
	plate->ApplyLoad(flag, hx,hy); // *LoadStepX, *LoadStepY);
	SetLoadArray(plate, flag);

	return flag;
}

void CBTRDoc:: SetAreaMinMax()
{
	AbsHorMin = Min(AreaHorMin, -(NeutrInW + NeutrInTh) * NeutrChannels);
	AbsHorMax = Max(AreaHorMax, (NeutrInW + NeutrInTh) * NeutrChannels);
	AbsVertMin = Min(AreaVertMin, - NeutrH * 0.5);
	AbsVertMax = Max(AreaVertMax, NeutrH * 0.5);

	AbsHorMin = Min(AbsHorMin, -(RIDInW + RIDTh) * RIDChannels);
	AbsHorMax = Max(AbsHorMax,  (RIDInW + RIDTh) * RIDChannels);
	AbsVertMin = Min(AbsVertMin, - RIDH * 0.5);
	AbsVertMax = Max(AbsVertMax, RIDH * 0.5);

	AbsHorMin = Min(AbsHorMin, -CalInW * 0.5);
	AbsHorMax = Max(AbsHorMax,  CalInW * 0.5);
	AbsVertMin = Min(AbsVertMin, -CalH * 0.5);
	AbsVertMax = Max(AbsVertMax,  CalH * 0.5);

	AbsHorMin = Min(AbsHorMin, -DDLinerInW * 0.5);
	AbsHorMax = Max(AbsHorMax,  DDLinerInW * 0.5);
	AbsVertMin = Min(AbsVertMin, -DDLinerInH * 0.5);
	AbsVertMax = Max(AbsVertMax,  DDLinerInH * 0.5);

	AbsHorMin = Min(AbsHorMin, -DDLinerOutW * 0.5);
	AbsHorMax = Max(AbsHorMax,  DDLinerOutW * 0.5);
	AbsVertMin = Min(AbsVertMin, -DDLinerOutH * 0.5);
	AbsVertMax = Max(AbsVertMax,  DDLinerOutH * 0.5);

	AbsHorMin = Min(AbsHorMin, -Duct1W * 0.5);
	AbsHorMax = Max(AbsHorMax,  Duct1W * 0.5);
	AbsVertMin = Min(AbsVertMin, -Duct1H * 0.5);
	AbsVertMax = Max(AbsVertMax,  Duct1H * 0.5);

	AbsHorMin = Min(AbsHorMin, -Duct2W * 0.5);
	AbsHorMax = Max(AbsHorMax,  Duct2W * 0.5);
	AbsVertMin = Min(AbsVertMin, -Duct2H * 0.5);
	AbsVertMax = Max(AbsVertMax,  Duct2H * 0.5);

	AbsHorMin = Min(AbsHorMin, -Duct3W * 0.5);
	AbsHorMax = Max(AbsHorMax,  Duct3W * 0.5);
	AbsVertMin = Min(AbsVertMin, -Duct3H * 0.5);
	AbsVertMax = Max(AbsVertMax,  Duct3H * 0.5);

	AbsHorMin = Min(AbsHorMin, -Duct4W * 0.5);
	AbsHorMax = Max(AbsHorMax,  Duct4W * 0.5);
	AbsVertMin = Min(AbsVertMin, -Duct4H * 0.5);
	AbsVertMax = Max(AbsVertMax,  Duct4H * 0.5);

	AbsHorMin = Min(AbsHorMin, -Duct5W * 0.5);
	AbsHorMax = Max(AbsHorMax,  Duct5W * 0.5);
	AbsVertMin = Min(AbsVertMin, -Duct5H * 0.5);
	AbsVertMax = Max(AbsVertMax,  Duct5H * 0.5);

	AbsHorMin = Min(AbsHorMin, -Duct6W * 0.5);
	AbsHorMax = Max(AbsHorMax,  Duct6W * 0.5);
	AbsVertMin = Min(AbsVertMin, -Duct6H * 0.5);
	AbsVertMax = Max(AbsVertMax,  Duct6H * 0.5);

	AbsHorMin = Min(AbsHorMin, -Duct7W * 0.5);
	AbsHorMax = Max(AbsHorMax,  Duct7W * 0.5);
	AbsVertMin = Min(AbsVertMin, -Duct7H * 0.5);
	AbsVertMax = Max(AbsVertMax,  Duct7H * 0.5);

	AbsHorMin = Min(AbsHorMin, -Duct8W * 0.5);
	AbsHorMax = Max(AbsHorMax,  Duct8W * 0.5);
	AbsVertMin = Min(AbsVertMin, -Duct8H * 0.5);
	AbsVertMax = Max(AbsVertMax,  Duct8H * 0.5);

	//AbsHorMax = Max(AbsHorMax,  FWRmax + TorCentre.Y);
	
}

void CBTRDoc:: AdjustAreaLimits()
{
/*	AreaHorMin = -0.6;
	AreaHorMax = 0.6;
	AreaVertMin = -1;
	AreaVertMax = 1;
*/
	double VertMin = AreaVertMin;
	double VertMax = AreaVertMax;
	// double HorMin = AreaHorMin;
	// double HorMax = AreaHorMax;


	double VInclin = 0;
	if (!OptFree) VInclin = VertInclin;
	//double TotVertAngle = BeamMisVert + VInclin + BeamVertTilt;
	//double VertShift = tan(TotVertAngle) * AreaLong;
	//if (VertShift > AreaVertMax * 0.7) VertMax += VertShift; 
	//if (VertShift < AreaVertMin * 0.7) VertMin += VertShift; 
	AreaVertMin = Min(VertMin, AreaVertMin);
	AreaVertMax = Max(VertMax, AreaVertMax);

	double BeamMaxHor = NofChannelsHor * SegmSizeHor * 0.7;
	double BeamMaxVert = NofChannelsVert * SegmSizeVert * 0.7;

	AreaHorMin = Min(AreaHorMin, -BeamMaxHor);
	AreaHorMax = Max(AreaHorMax, BeamMaxHor);
	AreaVertMin = Min(AreaVertMin, -BeamMaxVert);
	AreaVertMax = Max(AreaVertMax, BeamMaxVert);

	if (!OptFree) SetAreaMinMax();
	
	AreaHorMin = Min(AreaHorMin, AbsHorMin);
	AreaHorMax = Max(AreaHorMax, AbsHorMax);
	AreaVertMin = Min(AreaVertMin, AbsVertMin);
	AreaVertMax = Max(AreaVertMax, AbsVertMax);

	AreaLong = Max(AreaLong, Duct5X); 
	AreaLong = Max(AreaLong, Duct6X); 
	AreaLong = Max(AreaLong, Duct7X); 
	AreaLong = Max(AreaLong, Duct8X); 
	AreaLong += 0.5;

	CalcLimitXmin = 0;
	CalcLimitXmax = AreaLong + 1;
	
}

void CBTRDoc:: SetChannelWidth()
{
	int N = (int)NeutrChannels;
	double Wmin = 0.01; // min width
	switch (N) {
	case 0: NeutrChannels = 1; // 1 channel
		//NeutrInW = 4 * NeutrInW4 + 3 *(NeutrInTh); //NeutrOutW = 4 * NeutrOutW4 + 3 *(NeutrOutTh);
		NeutrInW = SegmSizeHor + NeutrOutTh;
		NeutrOutW = NeutrInW;
		if ((int)NofChannelsHor > 1) { 
			NeutrInW = NofChannelsHor * SegmStepHor * (BeamAimHor - NeutrInX) / BeamAimHor; 
			NeutrOutW = NofChannelsHor * SegmStepHor * (BeamAimHor - NeutrOutX) / BeamAimHor; 
		}
		if (NeutrInW < Wmin) NeutrInW = Wmin;
		if (NeutrOutW < Wmin) NeutrOutW = Wmin;
		break; 
	case 1: // 1 channel
		//NeutrInW = 4 * NeutrInW4 + 3 *(NeutrInTh);//NeutrOutW = 4 * NeutrOutW4 + 3 *(NeutrOutTh);
		NeutrInW = SegmSizeHor + NeutrOutTh;
		NeutrOutW = NeutrInW;
		if ((int)NofChannelsHor > 1) { 
			NeutrInW = NofChannelsHor * SegmStepHor * (BeamAimHor - NeutrInX) / BeamAimHor; 
			NeutrOutW = NofChannelsHor * SegmStepHor * (BeamAimHor - NeutrOutX) / BeamAimHor; 
		}
		if (NeutrInW < Wmin) NeutrInW = Wmin;
		if (NeutrOutW < Wmin) NeutrOutW = Wmin;
	
		break; 
	case 2: // 2 channels
		//NeutrInW = 2 * NeutrInW4 + (NeutrInTh); //NeutrOutW = 2 * NeutrOutW4 + (NeutrOutTh); 
		NeutrInW = SegmStepHor * (BeamAimHor - NeutrInX) / BeamAimHor - NeutrInTh;
		NeutrOutW = SegmStepHor * (BeamAimHor - NeutrOutX) / BeamAimHor - NeutrOutTh;
		if ((int)NofChannelsHor == 4) { 
			NeutrInW = 2 * SegmStepHor * (BeamAimHor - NeutrInX) / BeamAimHor - NeutrInTh; 
			NeutrOutW = 2 * SegmStepHor * (BeamAimHor - NeutrOutX) / BeamAimHor - NeutrOutTh; 
		}
		if (NeutrInW < Wmin) NeutrInW = Wmin;
		if (NeutrOutW < Wmin) NeutrOutW = Wmin;
		break; 
	case 3: // 3 channels
		//NeutrInW = 1.5 * NeutrInW4 + (NeutrInTh); //NeutrOutW = 1.5 * NeutrOutW4 + (NeutrOutTh); 
		NeutrInW = SegmStepHor * (BeamAimHor - NeutrInX) / BeamAimHor - NeutrInTh;
		NeutrOutW = SegmStepHor * (BeamAimHor - NeutrOutX) / BeamAimHor - NeutrOutTh;
		if (NeutrInW < Wmin) NeutrInW = Wmin;
		if (NeutrOutW < Wmin) NeutrOutW = Wmin;
		break; 
	case 4: 
		//NeutrInW = NeutrInW4; //NeutrOutW = NeutrOutW4; 
		NeutrInW = (SegmStepHor) * (BeamAimHor - NeutrInX) / (BeamAimHor) - NeutrInTh;
		NeutrOutW = (SegmStepHor ) * (BeamAimHor - NeutrOutX) / (BeamAimHor) - NeutrOutTh;
		if (NeutrInW < Wmin) NeutrInW = Wmin;
		if (NeutrOutW < Wmin) NeutrOutW = Wmin;
		break;
	default: 
		NeutrInW = (SegmStepHor) * (BeamAimHor - NeutrInX) / (BeamAimHor) - NeutrInTh;
		NeutrOutW = (SegmStepHor) * (BeamAimHor - NeutrOutX) / (BeamAimHor) - NeutrOutTh;
		if (NeutrInW < Wmin) NeutrInW = Wmin;
		if (NeutrOutW < Wmin) NeutrOutW = Wmin;
		//NeutrInW = NeutrInW4; NeutrOutW = NeutrOutW4; 
		break;
	}

	int R = (int)RIDChannels;
	switch (R) {
	case 0: RIDChannels = 1; // 1 channel
		//RIDInW = 4 * RIDInW4 + 3 *(RIDInTh); //RIDOutW = 4 * RIDOutW4 + 3 *(RIDOutTh);
		RIDInW = SegmSizeHor + RIDTh;
		RIDOutW = RIDInW;
		if ((int)NofChannelsHor > 1) { 
			RIDInW = (NofChannelsHor) * (SegmStepHor) * (BeamAimHor - RIDInX) / (BeamAimHor); 
			RIDOutW = (NofChannelsHor) * (SegmStepHor) * (BeamAimHor - RIDOutX) / (BeamAimHor); 
		}
		if (RIDInW < Wmin) RIDInW = Wmin;
		if (RIDOutW < Wmin) RIDOutW = Wmin;
		break; 
	case 1: // 1 channel
		//RIDInW = 4 * RIDInW4 + 3 *(RIDInTh);//RIDOutW = 4 * RIDOutW4 + 3 *(RIDOutTh);
		RIDInW = SegmSizeHor + RIDTh;
		RIDOutW = RIDInW;
		if ((int)NofChannelsHor > 1) { 
			RIDInW = (NofChannelsHor) * (SegmStepHor) * (BeamAimHor - RIDInX) / (BeamAimHor); 
			RIDOutW = (NofChannelsHor) * (SegmStepHor) * (BeamAimHor - RIDOutX) / (BeamAimHor); 
		}
		if (RIDInW < Wmin) RIDInW = Wmin;
		if (RIDOutW < Wmin) RIDOutW = Wmin;
		break; 
	case 2: // 2 channels
		//RIDInW = 2 * RIDInW4 + (RIDInTh); //RIDOutW = 2 * RIDOutW4 + (RIDOutTh); 
		RIDInW = (SegmStepHor) * (BeamAimHor - RIDInX) / (BeamAimHor) - RIDTh;
		RIDOutW = (SegmStepHor) * (BeamAimHor - RIDOutX) / (BeamAimHor) - RIDTh;
		if ((int)NofChannelsHor == 4) { 
			RIDInW = 2 * (SegmStepHor) * (BeamAimHor - RIDInX) / (BeamAimHor) - RIDTh; 
			RIDOutW = 2 * (SegmStepHor) * (BeamAimHor - RIDOutX) / (BeamAimHor) - RIDTh; 
		}
		if (RIDInW < Wmin) RIDInW = Wmin;
		if (RIDOutW < Wmin) RIDOutW = Wmin;
		break; 
	case 3: // 3 channels
		//RIDInW = 1.5 * RIDInW4 + (RIDInTh); //RIDOutW = 1.5 * RIDOutW4 + (RIDOutTh); 
		RIDInW = (SegmStepHor) * (BeamAimHor - RIDInX) / (BeamAimHor) - RIDTh;
		RIDOutW = (SegmStepHor) * (BeamAimHor - RIDOutX) / (BeamAimHor) - RIDTh;
		if (RIDInW < Wmin) RIDInW = Wmin;
		if (RIDOutW < Wmin) RIDOutW = Wmin;
		break; 
	case 4: 
		//RIDInW = RIDInW4; //RIDOutW = RIDOutW4; 
		RIDInW = (SegmStepHor) * (BeamAimHor - RIDInX) / (BeamAimHor) - RIDTh;
		RIDOutW = (SegmStepHor) * (BeamAimHor - RIDOutX) / (BeamAimHor) - RIDTh;
		if (RIDInW < Wmin) RIDInW = Wmin;
		if (RIDOutW < Wmin) RIDOutW = Wmin;
		break;
	default: 
		RIDInW = (SegmStepHor) * (BeamAimHor - RIDInX) / (BeamAimHor) - RIDTh;
		RIDOutW = (SegmStepHor) * (BeamAimHor - RIDOutX) / (BeamAimHor) - RIDTh;
		if (RIDInW < Wmin) RIDInW = Wmin;
		if (RIDOutW < Wmin) RIDOutW = Wmin;
		//RIDInW = RIDInW4; RIDOutW = RIDOutW4; 
		break;
	}


}

void  CBTRDoc::SetPlatesNeutraliser()// -------------  NEUTRALIZER -------------------------------
{
	CString S;
	CPlate * pPlate;
	
	C3Point p0_, p1_, p2_, p3_; // in local CS of Side Channel
	C3Point p0, p1, p2, p3; // in Central Channel CS
	int N = (int)NeutrChannels; // vert channels
	double StepIn = NeutrInW + NeutrInTh; // inlet step between adjacent channels axes (= between panels)
	double StepOut = NeutrOutW + NeutrOutTh;// outlet step between adjacent channels axes
	double YminIn = - StepIn * N * 0.5; // right-most panel Y  
	double YminOut = - StepOut * N * 0.5;// right-most panel Y  
	double Yin, Yout;

	/* pPlate = new CPlate(); // Source exit 
	 p0 = C3Point(0.5, YminIn - 0.1, -NeutrH * 0.5 - 0.1);
	 p3 = C3Point(0.5, -YminIn + 0.1,  NeutrH * 0.5 + 0.1);
	 pPlate->SetFromLimits(p0, p3); 
	 pPlate->Shift(0, NeutrBiasInHor, VShiftNeutr + NeutrBiasInVert);
	// pPlate->ShiftVert(VShiftNeutr + NeutrVBiasIn, VShiftNeutr + NeutrVBiasOut);
	 pPlate->Solid = FALSE;
	 pPlate->Visible = FALSE;
	 S.Format("Source Exit plane X = %g m", 0.5);
	 pPlate->Comment = S; 
	 PlatesList.AddTail(pPlate);
	 if (TaskRID)  SelectPlate(pPlate);*/

	 pPlate = new CPlate(); // Entry 
	 //PlateCounter++;  pPlate->Number = PlateCounter; // -> in AddCond
	 p0 = C3Point(NeutrInX - 0.001, YminIn - 0.1, -NeutrH * 0.5 - 0.1);
	 p3 = C3Point(NeutrInX - 0.001, -YminIn + 0.1,  NeutrH * 0.5 + 0.1);
	 pPlate->OrtDirect = -1;
	 pPlate->SetFromLimits(p0, p3); 
	 pPlate->Shift(0, NeutrBiasInHor, VShiftNeutr + NeutrBiasInVert);
	// pPlate->ShiftVert(VShiftNeutr + NeutrVBiasIn, VShiftNeutr + NeutrVBiasOut);
	 pPlate->Solid = FALSE;
	 pPlate->Visible = FALSE;
	 pPlate->MAP = TRUE;
	 S.Format("NEUTRALIZER Entry plane X = %g m", NeutrInX - 0.2);
	 pPlate->Comment = S; 
	 AddCond(pPlate);//PlatesList.AddTail(pPlate);
	// if (TaskRID) SelectPlate(pPlate);
	
	 pPlate = new CPlate(); // Exit 
	//PlateCounter++;  pPlate->Number = PlateCounter; // -> in AddCond
	 p0 = C3Point(NeutrOutX + 0.2, YminOut - 0.1, -NeutrH * 0.5 - 0.1);
	 p3 = C3Point(NeutrOutX + 0.2, -YminOut + 0.1,  NeutrH * 0.5 + 0.1);
	 pPlate->OrtDirect = -1;
	 pPlate->SetFromLimits(p0, p3); 
	 pPlate->Shift(0, NeutrBiasOutHor, VShiftNeutr + NeutrBiasOutVert);
	// pPlate->ShiftVert(VShiftNeutr + NeutrVBiasIn, VShiftNeutr + NeutrVBiasOut);
	 pPlate->Solid = FALSE;
	 pPlate->Visible = FALSE;
	 pPlate->MAP = TRUE;
	 S.Format("NEUTRALIZER Exit plane X = %g m", NeutrOutX + 0.2);
	 pPlate->Comment = S; 
	 AddCond(pPlate);//PlatesList.AddTail(pPlate);
	// if (TaskRID)  SelectPlate(pPlate);

	 int i;
	 for (i = 0; i <= N; i++) {
		 Yin = YminIn + i*StepIn;
		 Yout = YminOut + i*StepOut;
	 pPlate = new CPlate(); //  In 
	 PlateCounter++;
	 pPlate->Number = PlateCounter; 
	 p0 = C3Point(NeutrInX, Yin - NeutrInTh *0.5  , -NeutrH * 0.5 );
	 p1 = C3Point(NeutrInX, Yin + NeutrInTh *0.5  , -NeutrH * 0.5 );
	 p2 = C3Point(NeutrInX, Yin - NeutrInTh *0.5  , NeutrH * 0.5 );
	 p3 = C3Point(NeutrInX, Yin + NeutrInTh *0.5  , NeutrH * 0.5 );
	 pPlate->OrtDirect = -1;
	 pPlate->SetLocals(p0, p1, p2, p3); //pPlate->SetFromLimits(p0, p3);
	 //pPlate->ShiftVert(VShiftNeutr + NeutrVBiasIn, VShiftNeutr + NeutrVBiasOut);
	 pPlate->Shift(0, NeutrBiasInHor, VShiftNeutr + NeutrBiasInVert);
	 //pPlate->Shift(0,0, VShiftNeutr);
	 S.Format("NEUTRALIZER: Leading Edge of panel %d", i+1);
	 pPlate->Comment = S; 
	 PlatesList.AddTail(pPlate);
 	
 	 pPlate = new CPlate(); // right
	 PlateCounter++;
	 pPlate->Number = PlateCounter; 
	 p0 = C3Point(NeutrInX,  Yin - NeutrInTh *0.5  , -NeutrH * 0.5 );
	 p1 = C3Point(NeutrOutX, Yout - NeutrOutTh *0.5  , -NeutrH * 0.5 );
	 p2 = C3Point(NeutrInX,  Yin - NeutrInTh *0.5  , NeutrH * 0.5 );
	 p3 = C3Point(NeutrOutX, Yout -NeutrOutTh *0.5, NeutrH * 0.5 );
	 pPlate->SetLocals(p0, p1, p2, p3);
	 //pPlate->Shift(0, NeutrHBias, VShiftNeutr + NeutrVBias);
	 //pPlate->Shift(0,0, VShiftNeutr);
	 pPlate->ShiftVert(VShiftNeutr + NeutrBiasInVert, VShiftNeutr + NeutrBiasOutVert);
	 pPlate->ShiftHor(NeutrBiasInHor,  NeutrBiasOutHor);
	 S.Format("NEUTRALIZER: left wall of channel %d", i);
	 pPlate->Comment = S; 
	 PlatesList.AddTail(pPlate);
	
	 pPlate = new CPlate(); // left
	 PlateCounter++;
	 pPlate->Number = PlateCounter; 
	 p0 = C3Point(NeutrInX,  Yin +NeutrInTh *0.5  , -NeutrH * 0.5 );
	 p1 = C3Point(NeutrOutX,  Yout +NeutrOutTh *0.5  , -NeutrH * 0.5 );
	 p2 = C3Point(NeutrInX,  Yin +NeutrInTh *0.5  , NeutrH * 0.5 );
	 p3 = C3Point(NeutrOutX,  Yout +NeutrOutTh *0.5  , NeutrH * 0.5 );
	 pPlate->OrtDirect = -1;
	 pPlate->SetLocals(p0, p1, p2, p3);
	 //pPlate->Shift(0, NeutrHBias, VShiftNeutr + NeutrVBias);
	 //pPlate->Shift(0,0, VShiftNeutr);
	 pPlate->ShiftVert(VShiftNeutr + NeutrBiasInVert, VShiftNeutr + NeutrBiasOutVert);
	 pPlate->ShiftHor(NeutrBiasInHor,  NeutrBiasOutHor);
	 S.Format("NEUTRALIZER: right wall of channel %d", i+1);
	 pPlate->Comment = S; 
	 PlatesList.AddTail(pPlate);
	 
	 pPlate = new CPlate(); //  Out 
	 PlateCounter++;
	 pPlate->Number = PlateCounter; 
	 p0 = C3Point(NeutrOutX, Yout -NeutrOutTh *0.5  , -NeutrH * 0.5 );
	 p1 = C3Point(NeutrOutX, Yout +NeutrOutTh *0.5  , -NeutrH * 0.5 );
	 p2 = C3Point(NeutrOutX, Yout -NeutrOutTh *0.5  ,  NeutrH * 0.5 );
	 p3 = C3Point(NeutrOutX, Yout + NeutrOutTh *0.5  , NeutrH * 0.5 );
	 pPlate->SetLocals(p0, p1, p2, p3);//pPlate->SetFromLimits(p0, p3);
	 pPlate->Shift(0, NeutrBiasOutHor, VShiftNeutr + NeutrBiasOutVert);
	 //pPlate->Shift(0, NeutrHBias, VShiftNeutr + NeutrVBias);
	 //pPlate->Shift(0,0, VShiftNeutr);
	 S.Format("NEUTRALIZER: Back of panel %d", i+1);
	 pPlate->Comment = S; 
	 PlatesList.AddTail(pPlate);

	 } // i

	 pPlate = new CPlate(); //  Bottom CUT at Neutr entrance
	 //PlateCounter++; pPlate->Number = PlateCounter; 
	 p0 = C3Point(0, AreaHorMin, AreaVertMin);
	 p1 = C3Point(0, AreaHorMax, AreaVertMin);
	 p2 = C3Point(NeutrInX, -YminIn + NeutrBiasInHor, 
		          -NeutrH * 0.5 + VShiftNeutr + NeutrBiasInVert);
	 p3 = C3Point(NeutrInX, YminIn + NeutrBiasInHor,  
		          -NeutrH * 0.5 + VShiftNeutr + NeutrBiasInVert);
	 pPlate->SetLocals(p0, p1, p2, p3);//pPlate->SetFromLimits(p0, p3);
	 pPlate->Visible = FALSE;
	 pPlate->Solid = TRUE;
	 pPlate->OrtDirect = -1;
	 pPlate->Fixed = 1; // side view
	 pPlate->MAP = FALSE;
	 S.Format("CUT-OFF LIMIT: Bottom");
	 pPlate->Comment = S; 
	 AddCond(pPlate); //PlatesList.AddTail(pPlate);

	 pPlate = new CPlate(); // Top CUT at Neutr entrance
	 //PlateCounter++; pPlate->Number = PlateCounter; 
	 p0 = C3Point(NeutrInX, -YminIn + NeutrBiasInHor, 
		          NeutrH * 0.5 + VShiftNeutr + NeutrBiasInVert);
	 p1 = C3Point(NeutrInX, YminIn + NeutrBiasInHor,  
		          NeutrH * 0.5 + VShiftNeutr + NeutrBiasInVert);
	 p2 = C3Point(0, AreaHorMin, AreaVertMax);
	 p3 = C3Point(0, AreaHorMax, AreaVertMax);
	 
	 pPlate->SetLocals(p0, p1, p2, p3);//pPlate->SetFromLimits(p0, p3);
	 pPlate->Visible = FALSE;
	 pPlate->Solid = TRUE;
	 pPlate->OrtDirect = -1;
	 pPlate->Fixed = 1; // side view
	 pPlate->MAP = FALSE;
	 S.Format("CUT-OFF LIMIT: Top");
	 pPlate->Comment = S; 
	 AddCond(pPlate); //PlatesList.AddTail(pPlate);

	 pPlate = new CPlate(); // Right CUT at Neutr entrance
	 //PlateCounter++; pPlate->Number = PlateCounter; 
	 p0 = C3Point(0, AreaHorMin, AreaVertMin);
	 p1 = C3Point(NeutrInX, YminIn + NeutrBiasInHor, 
		          -NeutrH * 0.5 + VShiftNeutr + NeutrBiasInVert);
	 p2 = C3Point(0, AreaHorMin, AreaVertMax);
	 p3 = C3Point(NeutrInX, YminIn + NeutrBiasInHor, 
		          NeutrH * 0.5 + VShiftNeutr + NeutrBiasInVert);
	 
	 pPlate->SetLocals(p0, p1, p2, p3);//pPlate->SetFromLimits(p0, p3);
	 pPlate->Visible = FALSE;
	 pPlate->Solid = TRUE;
	 pPlate->OrtDirect = -1;
	 //pPlate->Fixed = 0; // plan view
	 pPlate->MAP = FALSE;
	 S.Format("CUT-OFF LIMIT: Right");
	 pPlate->Comment = S; 
	 AddCond(pPlate); //PlatesList.AddTail(pPlate);

	 pPlate = new CPlate(); // Left CUT at Neutr entrance
	 //PlateCounter++; pPlate->Number = PlateCounter;
	 p0 = C3Point(NeutrInX, -YminIn + NeutrBiasInHor, 
		          -NeutrH * 0.5 + VShiftNeutr + NeutrBiasInVert);
	 p1 = C3Point(0, AreaHorMax, AreaVertMin);
	 p2 = C3Point(NeutrInX, -YminIn + NeutrBiasInHor, 
		          NeutrH * 0.5 + VShiftNeutr + NeutrBiasInVert);
	 p3 = C3Point(0, AreaHorMax, AreaVertMax);
		 
	 pPlate->SetLocals(p0, p1, p2, p3);//pPlate->SetFromLimits(p0, p3);
	 pPlate->Visible = FALSE;
	 pPlate->Solid = TRUE;
	 pPlate->OrtDirect = -1;
	 //pPlate->Fixed = 0; // plan view
	 pPlate->MAP = FALSE;
	 S.Format("CUT-OFF LIMIT: Left");
	 pPlate->Comment = S; 
	 AddCond(pPlate); //PlatesList.AddTail(pPlate);
	 
	 pPlate = new CPlate(); // Bottom of Neutralizer
	 PlateCounter++;
	 pPlate->Number = PlateCounter; //501;
	 p0 = C3Point(NeutrInX, YminIn - NeutrInTh *0.5, -NeutrH * 0.5 );
	 p1 = C3Point(NeutrOutX, YminOut - NeutrOutTh *0.5, -NeutrH * 0.5 );
	 p2 = C3Point(NeutrInX, -YminIn + NeutrInTh *0.5  , -NeutrH * 0.5 );
	 p3 = C3Point(NeutrOutX, -YminOut + NeutrOutTh *0.5 , -NeutrH * 0.5 );
	 pPlate->SetLocals(p0, p1, p2, p3);//pPlate->SetFromLimits(p0, p3);
	 pPlate->ShiftVert(VShiftNeutr + NeutrBiasInVert, VShiftNeutr + NeutrBiasOutVert);
	 pPlate->ShiftHor(NeutrBiasInHor,  NeutrBiasOutHor);
	 //pPlate->Shift(0, NeutrBiasOutHor, VShiftNeutr + NeutrBiasOutVert);
	 //pPlate->Shift(0, NeutrHBias, VShiftNeutr + NeutrVBias);
	 //pPlate->Shift(0,0, VShiftNeutr);
	 pPlate->Fixed = 1; // side view
	 pPlate->MAP = FALSE;
	 S.Format("NEUTRALIZER: Bottom %d", pPlate->Number);
	 pPlate->Comment = S; 
	 PlatesList.AddTail(pPlate);

	 pPlate = new CPlate(); //  Top of Neutralizer
	 PlateCounter++;
	 pPlate->Number = PlateCounter; //502;//
	 p0 = C3Point(NeutrInX, YminIn - NeutrInTh *0.5, NeutrH * 0.5 );
	 p1 = C3Point(NeutrOutX, YminOut - NeutrOutTh *0.5, NeutrH * 0.5 );
	 p2 = C3Point(NeutrInX, -YminIn + NeutrInTh *0.5  , NeutrH * 0.5 );
	 p3 = C3Point(NeutrOutX, -YminOut + NeutrOutTh *0.5 , NeutrH * 0.5 );
	 pPlate->OrtDirect = -1;
	 pPlate->SetLocals(p0, p1, p2, p3);//pPlate->SetFromLimits(p0, p3);
	 pPlate->ShiftVert(VShiftNeutr + NeutrBiasInVert, VShiftNeutr + NeutrBiasOutVert);
	 pPlate->ShiftHor(NeutrBiasInHor,  NeutrBiasOutHor);
	 //pPlate->Shift(0, NeutrBiasOutHor, VShiftNeutr + NeutrBiasOutVert);
	 //pPlate->Shift(0, NeutrHBias, VShiftNeutr + NeutrVBias);
	 //pPlate->Shift(0,0, VShiftNeutr);
	 pPlate->Fixed = 1; // side view
	 pPlate->MAP = FALSE;
	 S.Format("NEUTRALIZER: Top %d", pPlate->Number);
	 pPlate->Comment = S; 
	 PlatesList.AddTail(pPlate);

	 RECT_DIA diaphragm;
	 YminIn = - StepIn * (N-1) * 0.5;
	 for (i = 0; i < N; i++) {
		Yin = YminIn + i*StepIn;
		pPlate = new CPlate(); //  In 
		p0 = C3Point(NeutrInX, Yin - NeutrInW *0.5  , -NeutrH * 0.5 );
		p3 = C3Point(NeutrInX, Yin + NeutrInW *0.5  , NeutrH * 0.5 );
		//p0 = C3Point(NeutrInX, -0.2, -0.5); p3 = C3Point(NeutrInX, 0.2, 0.5);
		pPlate->SetFromLimits(p0, p3);
		pPlate->Shift(0, NeutrBiasInHor, VShiftNeutr + NeutrBiasInVert);
		for (int k = 0; k < 4; k++) diaphragm.Corn[k] = pPlate->Corn[k];
		diaphragm.Number = 1;
		diaphragm.Channel = i+1;
		Dia_Array.Add(diaphragm);
		delete pPlate;
	 }//i

	 YminOut = - StepOut * (N-1) * 0.5;
	 for (i = 0; i < N; i++) {
		Yout = YminOut + i*StepOut;
		pPlate = new CPlate(); //  Out 
		p0 = C3Point(NeutrOutX, Yout - NeutrOutW *0.5  , -NeutrH * 0.5 );
		p3 = C3Point(NeutrOutX, Yout + NeutrOutW *0.5  , NeutrH * 0.5 );
		pPlate->SetFromLimits(p0, p3);
		pPlate->Shift(0, NeutrBiasOutHor, VShiftNeutr + NeutrBiasOutVert);
		for (int k = 0; k < 4; k++) diaphragm.Corn[k] = pPlate->Corn[k];
		diaphragm.Number = 2;
		diaphragm.Channel = i+1;
		Dia_Array.Add(diaphragm);
		delete pPlate;
	 } // i

	VOLUME vol;
	C3Point lb0, ru0, lb1, ru1;
/*	lb0 = C3Point(0, AreaHorMin, AreaVertMin);
	ru0 = C3Point(0, AreaHorMax, AreaVertMax);
	lb1 = C3Point(NeutrXmin, AreaHorMin, AreaVertMin);
	ru1 = C3Point(NeutrXmin, AreaHorMax, AreaVertMax);
	vol = VOLUME(lb0, ru0, lb1, ru1, 1); // GG-Neutralizer gap
	VolumeVector.push_back(vol);*/
	for (i = 0; i < N; i++) {
		Yin = YminIn + i*StepIn;
		pPlate = new CPlate(); 
		p0 = C3Point(NeutrInX, Yin - NeutrInW *0.5  , -NeutrH * 0.5 );
		p3 = C3Point(NeutrInX, Yin + NeutrInW *0.5  , NeutrH * 0.5 );
		pPlate->SetFromLimits(p0, p3);
		pPlate->Shift(0, NeutrBiasInHor, VShiftNeutr + NeutrBiasInVert);
		lb0 = pPlate->Corn[0]; ru0 = pPlate->Corn[2];
		Yout = YminOut + i*StepOut;
		p0 = C3Point(NeutrOutX, Yout - NeutrOutW *0.5  , -NeutrH * 0.5 );
		p3 = C3Point(NeutrOutX, Yout + NeutrOutW *0.5  , NeutrH * 0.5 );
		pPlate->SetFromLimits(p0, p3);
		pPlate->Shift(0, NeutrBiasOutHor, VShiftNeutr + NeutrBiasOutVert);
		lb1 = pPlate->Corn[0]; ru1 = pPlate->Corn[2];
		vol = VOLUME(lb0, ru0, lb1, ru1, 1);
		VolumeVector.push_back(vol);
		delete pPlate;
	 }//i
	
}

void  CBTRDoc::SetPlatesRID()// -------------  RID -------------------------------
{
	CString S;
	CPlate * pPlate;
//	RECT_DIA diaphragm;
	C3Point p0_, p1_, p2_, p3_; // in local CS of Side Channel
	C3Point p0, p1, p2, p3; // in Central Channel CS
	int N = (int)RIDChannels; // vert channels
	double StepIn = RIDInW + RIDTh; // inlet step between adjacent channels axes (= between panels)
	double StepOut = RIDOutW + RIDTh;// outlet step between adjacent channels axes
	double YminIn = - StepIn * N * 0.5; // right-most panel Y  
	double YminOut = - StepOut * N * 0.5;// right-most panel Y  
	double Yin, Yout;

	 pPlate = new CPlate(); // Exit
	//PlateCounter++;  pPlate->Number = PlateCounter; // -> in AddCond
	 p0 = C3Point(RIDOutX + 0.2, YminOut - 0.15, -RIDH * 0.5 - 0.1);
	 p3 = C3Point(RIDOutX + 0.2, -YminOut + 0.15,  RIDH * 0.5 + 0.1);
	 pPlate->OrtDirect = -1;
	 pPlate->SetFromLimits(p0, p3); 
	 pPlate->Shift(0, RIDBiasInHor, VShiftRID + RIDBiasInVert);
	 //pPlate->Shift(0,0, VShiftRID);
	 pPlate->Solid = FALSE;
	 pPlate->Visible = FALSE;
	 pPlate->MAP = TRUE;
	 S.Format("RID Exit plane X = %g m", RIDOutX + 0.2);
	 pPlate->Comment = S;
	 AddCond(pPlate);// PlatesList.AddTail(pPlate);
	// if (TaskRID)  SelectPlate(pPlate);

	 int i;
	 for (i = 0; i <= N; i++) {
		 Yin = YminIn + i*StepIn;
		 Yout = YminOut + i*StepOut;
	 pPlate = new CPlate(); //  In 
	 PlateCounter++;
	 pPlate->Number = PlateCounter; 
	 p0 = C3Point(RIDInX, Yin - RIDTh *0.5  , -RIDH * 0.5 );
	 p3 = C3Point(RIDInX, Yin + RIDTh *0.5  , RIDH * 0.5 );
	 pPlate->OrtDirect = -1;
	 pPlate->SetFromLimits(p0, p3);
	 pPlate->Shift(0, RIDBiasInHor, VShiftRID + RIDBiasInVert);
	 //pPlate->Shift(0,0, VShiftRID);
	 S.Format("RID: Leading Edge of panel %d", i+1);
	 pPlate->Comment = S;
	 PlatesList.AddTail(pPlate);
 	
 	 pPlate = new CPlate(); // right
	 PlateCounter++;
	 pPlate->Number = PlateCounter; 
	 p0 = C3Point(RIDInX,  Yin - RIDTh *0.5  , -RIDH * 0.5 );
	 p1 = C3Point(RIDOutX, Yout - RIDTh *0.5  , -RIDH * 0.5 );
	 p2 = C3Point(RIDInX,  Yin - RIDTh *0.5  , RIDH * 0.5 );
	 p3 = C3Point(RIDOutX, Yout -RIDTh *0.5  , RIDH * 0.5 );
	 pPlate->SetLocals(p0, p1, p2, p3);
	 pPlate->ShiftVert(VShiftRID + RIDBiasInVert, VShiftRID + RIDBiasOutVert);
	 pPlate->ShiftHor(RIDBiasInHor,  RIDBiasOutHor);
	 //pPlate->Shift(0, RIDHBias, VShiftRID + RIDVBias);
	 //pPlate->Shift(0,0, VShiftRID);
	 S.Format("RID: left wall of channel %d", i);
	 pPlate->Comment = S;
	 PlatesList.AddTail(pPlate);
	 //if (TaskRID)  SelectPlate(pPlate);

	 pPlate = new CPlate(); // left
	 PlateCounter++;
	 pPlate->Number = PlateCounter; 
	 p0 = C3Point(RIDInX,  Yin +RIDTh *0.5  , -RIDH * 0.5 );
	 p1 = C3Point(RIDOutX,  Yout +RIDTh *0.5  , -RIDH * 0.5 );
	 p2 = C3Point(RIDInX,  Yin +RIDTh *0.5  , RIDH * 0.5 );
	 p3 = C3Point(RIDOutX,  Yout +RIDTh *0.5  , RIDH * 0.5 );
	 pPlate->OrtDirect = -1;
	 pPlate->SetLocals(p0, p1, p2, p3);
	 pPlate->ShiftVert(VShiftRID + RIDBiasInVert, VShiftRID + RIDBiasOutVert);
	 pPlate->ShiftHor(RIDBiasInHor,  RIDBiasOutHor);
	 //pPlate->Shift(0, RIDHBias, VShiftRID + RIDVBias);
	 //pPlate->Shift(0,0, VShiftRID);
	 S.Format("RID: right wall of channel %d", i+1);
	 pPlate->Comment = S;
	 PlatesList.AddTail(pPlate);
	// if (TaskRID)  SelectPlate(pPlate);

	 pPlate = new CPlate(); // Out 
	 PlateCounter++;
	 pPlate->Number = PlateCounter; 
	 p0 = C3Point(RIDOutX, Yout -RIDTh *0.5  , -RIDH * 0.5 );
	 p3 = C3Point(RIDOutX, Yout + RIDTh *0.5  , RIDH * 0.5 );
	 pPlate->SetFromLimits(p0, p3);
	 pPlate->Shift(0, RIDBiasOutHor, VShiftRID + RIDBiasOutVert);
	 //pPlate->Shift(0, RIDHBias, VShiftRID + RIDVBias);
	 //pPlate->Shift(0,0, VShiftRID);
	 S.Format("RID: Back of panel %d", i+1);
	 pPlate->Comment = S;
	 PlatesList.AddTail(pPlate);

	 }

	 RECT_DIA diaphragm;
	 YminIn = - StepIn * (N-1) * 0.5;
	 for (i = 0; i < N; i++) {
		Yin = YminIn + i*StepIn;
		pPlate = new CPlate(); //  In 
		p0 = C3Point(RIDInX, Yin - RIDInW *0.5  , -RIDH * 0.5 );
		p3 = C3Point(RIDInX, Yin + RIDInW *0.5  , RIDH * 0.5 );
		//p0 = C3Point(NeutrInX, -0.2, -0.5); p3 = C3Point(NeutrInX, 0.2, 0.5);
		pPlate->SetFromLimits(p0, p3);
		pPlate->Shift(0, RIDBiasInHor, VShiftRID + RIDBiasInVert);
		for (int k = 0; k < 4; k++) diaphragm.Corn[k] = pPlate->Corn[k];
		diaphragm.Number = 3;
		diaphragm.Channel = i+1;
		Dia_Array.Add(diaphragm);
		delete pPlate;
	 }//i

	 YminOut = - StepOut * (N-1) * 0.5;
	 for (i = 0; i < N; i++) {
		Yout = YminOut + i*StepOut;
		pPlate = new CPlate(); //  Out 
		p0 = C3Point(RIDOutX, Yout - RIDOutW *0.5  , -RIDH * 0.5 );
		p3 = C3Point(RIDOutX, Yout + RIDOutW *0.5  , RIDH * 0.5 );
		//p0 = C3Point(NeutrOutX, -0.2, -0.5); p3 = C3Point(NeutrOutX, 0.2, 0.5);
		pPlate->SetFromLimits(p0, p3);
		pPlate->Shift(0, RIDBiasOutHor, VShiftRID + RIDBiasOutVert);
		for (int k = 0; k < 4; k++) diaphragm.Corn[k] = pPlate->Corn[k];
		diaphragm.Number = 4;
		diaphragm.Channel = i+1;
		Dia_Array.Add(diaphragm);
		delete pPlate;
	 } // i

	VOLUME vol;
	C3Point lb0, ru0, lb1, ru1;
	for (i = 0; i < N; i++) {
		Yin = YminIn + i*StepIn;
		pPlate = new CPlate(); 
		p0 = C3Point(RIDInX, Yin - RIDInW *0.5  , -RIDH * 0.5 );
		p3 = C3Point(RIDInX, Yin + RIDInW *0.5  , RIDH * 0.5 );
		pPlate->SetFromLimits(p0, p3);
		pPlate->Shift(0, RIDBiasInHor, VShiftRID + RIDBiasInVert);
		lb0 = pPlate->Corn[0]; ru0 = pPlate->Corn[2];
		Yout = YminOut + i*StepOut;
		p0 = C3Point(RIDOutX, Yout - RIDOutW *0.5  , -RIDH * 0.5 );
		p3 = C3Point(RIDOutX, Yout + RIDOutW *0.5  , RIDH * 0.5 );
		pPlate->SetFromLimits(p0, p3);
		pPlate->Shift(0, RIDBiasOutHor, VShiftRID + RIDBiasOutVert);
		lb1 = pPlate->Corn[0]; ru1 = pPlate->Corn[2];
		vol = VOLUME(lb0, ru0, lb1, ru1, 1);
		VolumeVector.push_back(vol);
		delete pPlate;
	 }//i
}


C3Point CBTRDoc::GetStartPoint(double f, double axisR)
{
	C3Point P = FWdata[0];
	int N = (int)FWdata.GetSize();
	double f0, f1;
	C3Point P0, P1;
	P0 = FWdata[0];
	f0 = atan2(P0.Y, (P0.X - axisR));
	int i1;
	for (int i = 1; i < N; i++) {
		i1 = i + 1;
		if (i1 == N) i1 = 0;
		P1 = FWdata[i1];
		f1 = atan2(P1.Y, (P1.X - axisR));
		if ((f-f0)*(f-f1) <=0) return (P0 + P1)*0.5;
		P0 = P1;
		f0 = f1;
	}
	return P;
}

C3Point CBTRDoc::GetNextFWcoord(C3Point prev, double dl) // positive shift along FWdata by step dl
{
	C3Point next = prev;
	int N = (int)FWdata.GetSize();
	int i0, i1, iprev = 0;
	double d0, d1, d01, sumL;
	C3Point P0, P1;

	for (int i = 0; i < N; i++) {// locate prev point -> iprev
		i0 = i;
		i1 = i + 1;
		if (i1 == N) i1 = 0;
		P0 = FWdata[i0];
		P1 = FWdata[i1];
		d01 = GetDistBetween(P0, P1);
		if (d01 < 1.e-6) continue;
		d0 = GetDistBetween(P0, prev);
		d1 = GetDistBetween(P1, prev);
		if (d0 < d01 && d1 <= d01) break; // i0, i1 found
	}//find iprev

	sumL = 0;
	next = prev;
	int it = 0;
	if (dl <= d1)
		next = prev + (P1 - prev)* dl / d1;
	
	else while (dl > sumL && it < 5)
		{
			sumL += d1;// move to next node
			it++;
			P0 = P1;
			i0 = i1;
			i1++;
			if (i1 == N) i1 = 0;
			P1 = FWdata[i1];
			d01 = GetDistBetween(P0, P1);
			if (d01 < 1.e-3) {
				d1 = 0;
				continue;
			}
			if (dl - sumL < d01) {
				next = P0 + (P1 - P0) * (dl - sumL) / d01;
				sumL = dl * 1.1; // to stop
				break;
			}
			else {
				next = P1;
				d1 = d01; // jump to next node 
			}
		} // while dl < sumL
		

	return next;
}

void  CBTRDoc::SetPlatesTor()
{
	//AfxMessageBox("Enter SetPlatesTor");
	//if (!OptBeamInPlasma) return;
	CPlate * Plate;
	C3Point p0, p1, p2, p3;
	double r0, z0, r1, z1, Teta0, Teta1, f0, f1;
	double x00, x01, x10, x11, y00, y01, y10, y11;
	int Ntor = (int) TorSegmentNumber;//36; // along tor teta
	int Nf = FWdata.GetUpperBound(); // last point = 1st
	if (Nf < 1) return;
	
	double Ymin = TorCentreY; //AreaHorMin
	double Ymax = TorCentreY + FWRmax; //AreaHorMax;
	double Zmin = FWZmin; //AreaVertMin;
	double Zmax = FWZmax; //AreaVertMax;
/*	C3Point Av = GetBeamFootLimits(PlasmaXmax, Ymin, Ymax, Zmin, Zmax); //already called in SetPlates
	//double Yav = 0.5 * (Ymin + Ymax); Zav = 0.5 * (Zmin + Zmax);
	Ymin = Min(Ymin, AreaHorMin);
	Ymax = Max(Ymax, AreaHorMax);
	Zmin = Min(Zmin, AreaVertMin);
	Zmax = Max(Zmax, AreaVertMax);*/
	double sizeY = fabs(Ymax - Ymin);// plasma block size
	double sizeZ = fabs(Zmax - Zmin);// plasma block size
	double Yav = 0.5 * (Ymin + Ymax);
	double Zav = 0.5 * (Zmin + Zmax);
	double Tav = atan2((Yav - TorCentreY), (PlasmaXmax - TorCentreX)); // -Pi..Pi
	double Fav = atan2((Zav - TorCentreZ), (PlasmaXmax - TorCentreX));
	//if (Fav > PI / 2) Fav -= PI; 
	double FWdR = 0.5*(FWRmax - FWRmin);// = PlasmaMinorR if circle
	double TorR = FWRmin + FWdR; // = PlasmaMajorR if circle

	double TetaMax =  Tav + atan2(sizeY, FWRmax);// -pi/2...pi/2
	double TetaMin =  Tav - atan2(sizeY, FWRmax); // - 
	
	double Fmin = -PI * 0.5;// atan2((FWZmin), FWRmax);
	double Fmax = PI * 0.5; // atan2((FWZmax), FWRmax);
	//double Fmax = Fav + 2*atan2(sizeZ, FWRmax);   //atan((Zmax - TorCentre.Z)/FWdR);//
	//double Fmin = Fav - 2*atan2(sizeZ, FWRmax);   //atan((Zmin - TorCentre.Z)/FWdR); //
	
	double dTeta = 2 * PI / Ntor;// step of tor segments
	double T0 = dTeta * 0.5;
	double stepZ = sizeZ * 0.3; // step along FW profile (poloidal)

	double Fsum = 0; //sum rotation
	C3Point P0 = GetStartPoint(Fmin, TorR);//returns FWdata[0] if not found!!!
	C3Point P1 = P0;
	f1 = Fmin;
		
/*	double LimXmin = TorCentreX + FWRmax * cos(TetaMin) + 0.2; // start
	double LimXmax = TorCentreX + FWRmax * cos(TetaMax) + 0.2; // fin
	
	double LimYmin = Ymin-0.1;
	double LimYmax = Ymax+0.1;
	double LimZmin = Zmin-0.1;
	double LimZmax = Zmax+0.1;
		
	while (Fsum < 2*PI) {
		double step = stepZ;
		P0 = P1;
		r0 = P0.X; // FWdata[i].X;
		z0 = TorCentre.Z + P0.Y;
		f0 = f1; // atan(FWdata[i].Y / (r0 - TorRmin));
		P1 = GetNextFWcoord(P0, step);
		if (GetDistBetween(P0, P1) < 0.1*step) {
			step = 0.1 * step;
			P1 = GetNextFWcoord(P0, step);
		}
		if (GetDistBetween(P0, P1) < 1.e-3) {
			AfxMessageBox("FW step problem");
			return;
		}
		r1 = P1.X; // FWdata[i + 1].X;
		z1 = TorCentre.Z + P1.Y; // TorCentre.Z + FWdata[i + 1].Y;
		f1 = atan2(z1, (r1 - TorR)); // atan(FWdata[i+1].Y/(r1 - TorRmin));
		if (f0 > 0 && f1 < 0) f1 += 2 * PI;
		
		Fsum += f1 - f0;
		//if (f1 < 0) f1 += 2 * PI;
		//if (f0 > Fmax) continue;
		if (r0 - TorR < 0 || r1 - TorR < 0) continue;
		//if (0.5 * (z0 + z1) > Zmax || 0.5 * (z0 + z1) < Zmin) continue;
		
		for (int j = 0; j < Ntor; j++) { // along toroidal coord - Teta
			Teta0 = T0 + dTeta * j; Teta1 = T0 + dTeta * (j+1);
			if (Teta0 > TetaMax || Teta1 < TetaMin) continue;
			x00 = TorCentreX + r0 * cos(Teta0); x01 = TorCentreX + r0 * cos(Teta1);
			x10 = TorCentreX + r1 * cos(Teta0); x11 = TorCentreX + r1 * cos(Teta1);
			//if (x00 < TorCentre.X || x11 < TorCentre.X) break; 
			y00 = TorCentreY + r0 * sin(Teta0); y01 = TorCentreY + r0 * sin(Teta1);
			y10 = TorCentreY + r1 * sin(Teta0); y11 = TorCentreY + r1 * sin(Teta1);
			//if (Min(y00, y11) > Ymax || Max(y00, y11) < Ymin) continue;
			//if (Min(y01, y10) > Ymax || Max(y01, y10) < Ymin) continue;
			
			p0 = C3Point(x00, y00, z0);
			p1 = C3Point(x01, y01, z0); // r0, z0, Teta1
			p2 = C3Point(x10, y10, z1); // r1, y1, Teta0
			p3 = C3Point(x11, y11, z1);
			Plate = new CPlate(); // FW tile 
			PlateCounter++;
			Plate->Number = PlateCounter;
			Plate->OrtDirect = -1;
			Plate->SetLocals(p0, p1, p2, p3);
			Plate->Solid = FALSE;// TRUE;
			Plate->Comment = "First Wall";
			Plate->Fixed = 1; // side view
			PlatesList.AddTail(Plate);
		} // j
	} //i
	p0 = C3Point(LimXmin, LimYmin, LimZmin);
	p1 = C3Point(LimXmax, LimYmax, LimZmin); //
	p2 = C3Point(LimXmin, LimYmin, LimZmax); // r1, y1, Teta0
	p3 = C3Point(LimXmax, LimYmax, LimZmax);
*/
	double Yt = -TorCentreY;
	double alfa = asin(Yt / FWRmax);
	double Xt = TorCentreX + FWRmax * cos(alfa);
	
	double dS = PlasmaMinorR * 1.5;
	double Ytmin = + dS * cos(alfa);
	double Ytmax = - dS * cos(alfa);
	double Xtmin = Xt - dS * sin(alfa);
	double Xtmax = Xt + dS * sin(alfa);

	p0 = C3Point(Xtmin, Ytmin, Zmin);
	p1 = C3Point(Xtmax, Ytmax, Zmin); //
	p2 = C3Point(Xtmin, Ytmin, Zmax); // r1, y1, Teta0
	p3 = C3Point(Xtmax, Ytmax, Zmax);
	Plate = new CPlate(); // FW 
	PlateCounter++;
	Plate->Number = PlateCounter;
	Plate->OrtDirect = 1;
	Plate->SetLocals(p0, p1, p2, p3);
	Plate->Solid =  TRUE; //!!!!!!!! stop particles at FW
	Plate->Visible = FALSE;
	Plate->Comment = "FW combo";
	Plate->Fixed = -1; // plan view?
	PlatesList.AddTail(Plate);
	/*AreaLong = PlasmaXmax;
	AreaVertMin = min(AreaVertMin, Zmin);
	AreaVertMax = max(AreaVertMax, Zmax);*/

/*	CString S;
	Plate = new CPlate(); // Tor Entrance 
	PlateCounter++;
	Plate->Number = PlateCounter;
	double ymin, ymax, zmin, zmax;
	C3Point centre = GetBeamFootLimits(PlasmaXmin, ymin, ymax, zmin, zmax);
	//p0 = C3Point(PlasmaXmin, DuctExitYmin, DuctExitZmin);
	//p3 = C3Point(PlasmaXmin, DuctExitYmax, DuctExitZmax);
	p0 = C3Point(PlasmaXmin, ymin-0.2, zmin - 0.2);
	//p1 = C3Point(Xtmax, Ytmax, Zmin); //
	//p2 = C3Point(Xtmin, Ytmin, Zmax); // r1, y1, Teta0
	p3 = C3Point(PlasmaXmin, ymax + 0.2, zmax + 0.2);
	
	Plate->OrtDirect = 1;
	Plate->SetFromLimits(p0, p3);
	//pPlate->Shift(0, RIDBiasOutHor, VShiftRID + RIDBiasOutVert);
	Plate->Solid = FALSE;
	Plate->Visible = FALSE;
	S.Format("Plasma Emitter X = %g m", PlasmaXmin);
	Plate->Comment = S;
	PlatesList.AddTail(Plate);
	//	 if (TaskRID)  SelectPlate(pPlate);
	PlasmaEmitter = Plate->Number;*/
}

void  CBTRDoc::SetPlatesDuct() // only dias  (not called)
{
	SetDuctDia();
}

void  CBTRDoc::SetPlatesDuctFull()// called
{
	//AfxMessageBox("Enter SetPlatesDuct");
	CString S;
	CPlate * pPlate;
	C3Point p0_, p1_, p2_, p3_; // 
	C3Point p0, p1, p2, p3; // in Central Channel CS
	RECT_DIA diaphragm;
	VOLUME vol;
	C3Point lb0, ru0, lb1, ru1;
	double DuctExitX = -1; // max Duct coord - for DuctExit cross plane
	double X = DuctExitX;
	DuctExitX = Max(X, DuctExitX);

		
	// ------------- Pre-Duct Dia (Scraper WINGS )  -------------------------------
	diaphragm.Number = 5; // PreDuct = SCRAPER entry Dia 5
	 pPlate = new CPlate(); //Pre-Duct Diafragm
	 //PlateCounter++;	pPlate->Number = PlateCounter; 
	 p0 = C3Point(PreDuctX, -PreDuctW * 0.5, AreaVertMin);
	 p1 = C3Point(PreDuctX,  PreDuctW * 0.5, AreaVertMin);	
	 p2 = C3Point(PreDuctX, -PreDuctW * 0.5, -PreDuctH * 0.5);
	 p3 = C3Point(PreDuctX,  PreDuctW * 0.5, -PreDuctH * 0.5);
	 pPlate->OrtDirect = -1;
	 pPlate->SetLocals(p0, p1, p2, p3);
	 pPlate->Shift(0, DiaBiasHor, VShiftDia + DiaBiasVert);
	 pPlate->Corn[0].Z = AreaVertMin; pPlate->Corn[1].Z = AreaVertMin;
	 //pPlate->SetLocals(pPlate->Corn[0], pPlate->Corn[1], pPlate->Corn[3], pPlate->Corn[2]);
	 //pPlate->Shift(0,0, VShiftDia);
	 pPlate->Solid = TRUE;
	 pPlate->Comment = "SCRAPER 1: lower wing";
	 pPlate->Fixed = 1; // side view
	 AddCond(pPlate);//PlatesList.AddTail(pPlate);
	 diaphragm.Corn[0] = pPlate->Corn[3]; //p2
	// diaphragm.Corn[1] = pPlate->Corn[0];

	 pPlate = new CPlate(); //Pre-Duct Diafragm
	 //PlateCounter++;	 pPlate->Number = PlateCounter; 
	 p0 = C3Point(PreDuctX, -PreDuctW * 0.5, PreDuctH * 0.5);
	 p1 = C3Point(PreDuctX,  PreDuctW * 0.5, PreDuctH * 0.5);
	 p2 = C3Point(PreDuctX,  -PreDuctW * 0.5, AreaVertMax);
	 p3 = C3Point(PreDuctX,  PreDuctW * 0.5, AreaVertMax);
	 pPlate->OrtDirect = -1;
	 pPlate->SetLocals(p0, p1, p2, p3);
	 pPlate->Shift(0, DiaBiasHor, VShiftDia + DiaBiasVert);
	 pPlate->Corn[2].Z = AreaVertMax; pPlate->Corn[3].Z = AreaVertMax;
	 //pPlate->SetLocals(pPlate->Corn[0], pPlate->Corn[1], pPlate->Corn[3], pPlate->Corn[2]);
	 //pPlate->Shift(0,0, VShiftDia);
	 pPlate->Solid = TRUE;
	 pPlate->Comment = "SCRAPER 1: upper wing";
	 pPlate->Fixed = 1; // side view
	 AddCond(pPlate);//PlatesList.AddTail(pPlate);
	 diaphragm.Corn[2] = pPlate->Corn[1];//p1
	// diaphragm.Corn[3] = pPlate->Corn[0];

	 pPlate = new CPlate(); //Pre-Duct Diafragm
	 //PlateCounter++; pPlate->Number = PlateCounter; 
	 p0 = C3Point(PreDuctX, AreaHorMin, AreaVertMin);
	 p1 = C3Point(PreDuctX,  -PreDuctW * 0.5, AreaVertMin);
	 p2 = C3Point(PreDuctX, AreaHorMin, AreaVertMax);
	 p3 = C3Point(PreDuctX,  -PreDuctW * 0.5, AreaVertMax);
	 pPlate->OrtDirect = -1;
	 pPlate->SetLocals(p0, p1, p2, p3);
	 pPlate->Shift(0, DiaBiasHor, VShiftDia + DiaBiasVert);
	 pPlate->Corn[0].Z = AreaVertMin; pPlate->Corn[1].Z = AreaVertMin;
	 pPlate->Corn[2].Z = AreaVertMax; pPlate->Corn[3].Z = AreaVertMax;
	 //pPlate->SetLocals(pPlate->Corn[0], pPlate->Corn[1], pPlate->Corn[3], pPlate->Corn[2]);
	 //pPlate->Shift(0,0, VShiftDia);
	 pPlate->Solid = TRUE;
	 pPlate->Comment = "SCRAPER 1: right wing";
	 AddCond(pPlate);//PlatesList.AddTail(pPlate);

	 pPlate = new CPlate(); //Pre-Duct Diafragm
	 //PlateCounter++;	pPlate->Number = PlateCounter; 
	 p0 = C3Point(PreDuctX, PreDuctW * 0.5, AreaVertMin);
	 p1 = C3Point(PreDuctX, AreaHorMax, AreaVertMin);
	 p2 = C3Point(PreDuctX, PreDuctW * 0.5, AreaVertMax);
	 p3 = C3Point(PreDuctX, AreaHorMax, AreaVertMax);
	 pPlate->OrtDirect = -1;
	 pPlate->SetLocals(p0, p1, p2, p3);
	 pPlate->Shift(0, DiaBiasHor, VShiftDia + DiaBiasVert);
	 pPlate->Corn[0].Z = AreaVertMin; pPlate->Corn[1].Z = AreaVertMin;
	 pPlate->Corn[2].Z = AreaVertMax; pPlate->Corn[3].Z = AreaVertMax;
	 //pPlate->SetLocals(pPlate->Corn[0], pPlate->Corn[1], pPlate->Corn[3], pPlate->Corn[2]);
	 //pPlate->Shift(0,0, VShiftDia);
	 pPlate->Solid = TRUE;
 	 pPlate->Comment = "SCRAPER 1: left wing";
	 AddCond(pPlate);//PlatesList.AddTail(pPlate);

	 Dia_Array.Add(diaphragm);//5

	 lb0 = diaphragm.Corn[0]; ru0 = diaphragm.Corn[2];
	
	 X = PreDuctX;
	 DuctExitX = Max(X, DuctExitX);

	//--------- SCRAPER WALLS Dia 5-6-------------------------
	 // PreDuct -> Scr1
	 // DDLinerIn -> Scr2
	 diaphragm.Number = 6; // DDLinerIn

	 pPlate = new CPlate(); // left
	 //PlateCounter++;pPlate->Number = PlateCounter; 
	 p0 = C3Point(PreDuctX,  PreDuctW * 0.5, - PreDuctH * 0.5);
	 p1 = C3Point(DDLinerInX, DDLinerInW * 0.5, - DDLinerInH * 0.5);
	 p2 = C3Point(PreDuctX,  PreDuctW * 0.5,  PreDuctH * 0.5);
	 p3 = C3Point(DDLinerInX, DDLinerInW * 0.5,  DDLinerInH * 0.5);
	 pPlate->SetLocals(p0, p1, p2, p3);
	 pPlate->ShiftVert(DiaBiasVert + VShiftDia, LinerBiasInVert + VShiftLinerIn);
	 pPlate->ShiftHor(DiaBiasHor, LinerBiasOutHor);
	 //pPlate->Shift(0, LinerHBias, VShiftLinerIn + LinerVBias);
	 pPlate->Solid = TRUE;
	 pPlate->Comment = "SCRAPER 2: left wall";
	 AddCond(pPlate);//PlatesList.AddTail(pPlate);
//	if (TaskReionization)  SelectPlate(pPlate); //{ pPlate->Selected = TRUE; SetLoadArray(pPlate, TRUE); }
	 diaphragm.Corn[2] = pPlate->Corn[2];//p3

	pPlate = new CPlate(); // right
	//PlateCounter++;	pPlate->Number = PlateCounter; 
	 p0 = C3Point(PreDuctX,  -PreDuctW * 0.5, - PreDuctH * 0.5);
	 p1 = C3Point(DDLinerInX, -DDLinerInW * 0.5, - DDLinerInH * 0.5);
	 p2 = C3Point(PreDuctX,  -PreDuctW * 0.5,  PreDuctH * 0.5);
	 p3 = C3Point(DDLinerInX, -DDLinerInW * 0.5,  DDLinerInH * 0.5);
	 pPlate->OrtDirect = -1;
	 pPlate->SetLocals(p0, p1, p2, p3);
	 pPlate->ShiftVert(DiaBiasVert + VShiftDia, LinerBiasInVert + VShiftLinerIn);
	 pPlate->ShiftHor(DiaBiasHor, LinerBiasOutHor);
	 //pPlate->Shift(0, LinerHBias, VShiftLinerIn + LinerVBias);
	 pPlate->Solid = TRUE;
	 pPlate->Comment = "SCRAPER 2: right wall";
	 AddCond(pPlate);//PlatesList.AddTail(pPlate);
//	if (TaskReionization)  SelectPlate(pPlate); //{ pPlate->Selected = TRUE; SetLoadArray(pPlate, TRUE); }
	 diaphragm.Corn[0] = pPlate->Corn[1];//p1
	 Dia_Array.Add(diaphragm);//6

	 lb1 = diaphragm.Corn[0]; ru1 = diaphragm.Corn[2];
	 vol = VOLUME(lb0, ru0, lb1, ru1, 1);
	 VolumeVector.push_back(vol); //5-6
	 lb0 = lb1; ru0 = ru1;

	 pPlate = new CPlate(); // upper
	 //PlateCounter++;pPlate->Number = PlateCounter; 
	 p0 = C3Point(PreDuctX,  -PreDuctW * 0.5, PreDuctH * 0.5);
	 p1 = C3Point(DDLinerInX, -DDLinerInW * 0.5, DDLinerInH * 0.5);
	 p2 = C3Point(PreDuctX,  PreDuctW * 0.5,  PreDuctH * 0.5);
	 p3 = C3Point(DDLinerInX, DDLinerInW * 0.5,  DDLinerInH * 0.5);
	 pPlate->OrtDirect = -1;
	 pPlate->SetLocals(p0, p1, p2, p3);
	 pPlate->ShiftVert(DiaBiasVert + VShiftDia, LinerBiasInVert + VShiftLinerIn);
	 pPlate->ShiftHor(DiaBiasHor, LinerBiasOutHor);
	 //pPlate->Shift(0, LinerHBias, VShiftLinerIn + LinerVBias);
	 pPlate->Solid = TRUE;
	 pPlate->Fixed = 1; // side view
	 pPlate->Comment = "SCRAPER 2: top wall";
	 AddCond(pPlate);//PlatesList.AddTail(pPlate);
//	if (TaskReionization)  SelectPlate(pPlate); //{ pPlate->Selected = TRUE; SetLoadArray(pPlate, TRUE); }

	 pPlate = new CPlate(); // lower
	 //PlateCounter++; pPlate->Number = PlateCounter; 
	 p0 = C3Point(PreDuctX,  -PreDuctW * 0.5, -PreDuctH * 0.5);
	 p1 = C3Point(DDLinerInX, -DDLinerInW * 0.5, -DDLinerInH * 0.5);
	 p2 = C3Point(PreDuctX,  PreDuctW * 0.5,  -PreDuctH * 0.5);
	 p3 = C3Point(DDLinerInX, DDLinerInW * 0.5,  -DDLinerInH * 0.5);
	 pPlate->SetLocals(p0, p1, p2, p3);
	 pPlate->ShiftVert(DiaBiasVert + VShiftDia, LinerBiasInVert + VShiftLinerIn);
	 pPlate->ShiftHor(DiaBiasHor, LinerBiasOutHor);
	 //pPlate->Shift(0, LinerHBias, VShiftLinerIn + LinerVBias);
	 pPlate->Solid = TRUE;
	 pPlate->Fixed = 1; // side view
	 pPlate->Comment = "SCRAPER 2: bottom wall";
	 AddCond(pPlate);//PlatesList.AddTail(pPlate);
//	if (TaskReionization)  SelectPlate(pPlate); //{ pPlate->Selected = TRUE; SetLoadArray(pPlate, TRUE); }

	 X = DDLinerInX;
	 DuctExitX = Max(X, DuctExitX);
	//--------- DDLiner ----------------------------
/*	 pPlate = new CPlate(); // before Liner
	 p0 = C3Point(DDLinerInX - 0.1, -DDLinerInW * 0.5 - 0.1, - DDLinerInH * 0.5 - 0.1 );
	 p3 = C3Point(DDLinerInX - 0.1, DDLinerInW * 0.5 + 0.1,  DDLinerInH * 0.5 + 0.1);
	 pPlate->SetFromLimits(p0, p3);
	 pPlate->Shift(0, LinerBiasInHor, VShiftLinerIn + LinerBiasInVert);
	 //pPlate->Shift(0,0, VShiftDia);
	 pPlate->Solid = FALSE;
	 pPlate->Visible = FALSE;
	 S.Format("Cross-plane before Liner, X = %g m", DDLinerInX - 0.1);
	 pPlate->Comment = S;
	 PlatesList.AddTail(pPlate);
*/
	 diaphragm.Number = 7; // DDLinerOut
	 pPlate = new CPlate(); // right
	 //PlateCounter++; pPlate->Number = PlateCounter; 
	 p0 = C3Point(DDLinerInX,  DDLinerInW * 0.5, - DDLinerInH * 0.5);
	 p1 = C3Point(DDLinerOutX,  DDLinerOutW * 0.5, - DDLinerOutH * 0.5);
	 p2 = C3Point(DDLinerInX,  DDLinerInW * 0.5,  DDLinerInH * 0.5);
	 p3 = C3Point(DDLinerOutX,  DDLinerOutW * 0.5,  DDLinerOutH * 0.5);
	 pPlate->SetLocals(p0, p1, p2, p3);
	 pPlate->ShiftVert(LinerBiasInVert + VShiftLinerIn, LinerBiasOutVert + VShiftLinerOut);
	 pPlate->ShiftHor(LinerBiasInHor, LinerBiasOutHor);
	 //pPlate->Shift(0, LinerHBias, VShiftLinerIn + LinerVBias);
	 pPlate->Solid = TRUE;
	 pPlate->Comment = "AV(7-8): left wall";
	 AddCond(pPlate);//PlatesList.AddTail(pPlate);
//	if (TaskReionization)  SelectPlate(pPlate); //{ pPlate->Selected = TRUE; SetLoadArray(pPlate, TRUE); }
	 diaphragm.Corn[2] = pPlate->Corn[2];//p3

	 pPlate = new CPlate(); // left
	 //PlateCounter++; pPlate->Number = PlateCounter; 
	 p0 = C3Point(DDLinerInX,  -DDLinerInW * 0.5, - DDLinerInH * 0.5);
	 p1 = C3Point(DDLinerOutX,  -DDLinerOutW * 0.5, - DDLinerOutH * 0.5);
	 p2 = C3Point(DDLinerInX,  -DDLinerInW * 0.5,  DDLinerInH * 0.5);
	 p3 = C3Point(DDLinerOutX,  -DDLinerOutW * 0.5,  DDLinerOutH * 0.5);
	 pPlate->OrtDirect = -1;
	 pPlate->SetLocals(p0, p1, p2, p3);
	 pPlate->ShiftVert(LinerBiasInVert + VShiftLinerIn, LinerBiasOutVert + VShiftLinerOut);
	 pPlate->ShiftHor(LinerBiasInHor, LinerBiasOutHor);
	 //pPlate->ShiftVert(VShiftLinerIn, VShiftLinerOut);
	 pPlate->Solid = TRUE;
	 pPlate->Comment = "AV(7-8): right wall";
	 AddCond(pPlate); //PlatesList.AddTail(pPlate);
//	if (TaskReionization)  SelectPlate(pPlate);//{ pPlate->Selected = TRUE; SetLoadArray(pPlate, TRUE); }
	 diaphragm.Corn[0] = pPlate->Corn[1];//p1
	 Dia_Array.Add(diaphragm);//7

	 lb1 = diaphragm.Corn[0]; ru1 = diaphragm.Corn[2];
	 vol = VOLUME(lb0, ru0, lb1, ru1, 1);
	 VolumeVector.push_back(vol);//6-7
	 lb0 = lb1; ru0 = ru1;
	
	pPlate = new CPlate(); // upper
	//PlateCounter++;	pPlate->Number = PlateCounter; 
	 p0 = C3Point(DDLinerInX, - DDLinerInW * 0.5,  DDLinerInH * 0.5);
	 p2 = C3Point(DDLinerInX,  DDLinerInW * 0.5, DDLinerInH * 0.5);
	 p1 = C3Point(DDLinerOutX, - DDLinerOutW * 0.5,  DDLinerOutH * 0.5);
	 p3 = C3Point(DDLinerOutX,  DDLinerOutW * 0.5,  DDLinerOutH * 0.5);
	 pPlate->OrtDirect = -1;
	 pPlate->SetLocals(p0, p1, p2, p3);
	 pPlate->ShiftVert(LinerBiasInVert + VShiftLinerIn, LinerBiasOutVert + VShiftLinerOut);
	 pPlate->ShiftHor(LinerBiasInHor, LinerBiasOutHor);
	 //pPlate->ShiftVert(VShiftLinerIn, VShiftLinerOut);
	pPlate->Solid = TRUE;
	pPlate->Comment = "AV(7-8): top wall";
	pPlate->Fixed = 1; // side view
	AddCond(pPlate);//PlatesList.AddTail(pPlate);
//	if (TaskReionization)  SelectPlate(pPlate);//{ pPlate->Selected = TRUE; SetLoadArray(pPlate, TRUE); }

	pPlate = new CPlate(); // lower
	//PlateCounter++;	pPlate->Number = PlateCounter; 
	 p0 = C3Point(DDLinerInX, - DDLinerInW * 0.5,  - DDLinerInH * 0.5);
	 p2 = C3Point(DDLinerInX,  DDLinerInW * 0.5,  - DDLinerInH * 0.5);
	 p1 = C3Point(DDLinerOutX, - DDLinerOutW * 0.5, - DDLinerOutH * 0.5);
	 p3 = C3Point(DDLinerOutX,  DDLinerOutW * 0.5,  - DDLinerOutH * 0.5);
	 pPlate->SetLocals(p0, p1, p2, p3);
	 pPlate->ShiftVert(LinerBiasInVert + VShiftLinerIn, LinerBiasOutVert + VShiftLinerOut);
	 pPlate->ShiftHor(LinerBiasInHor, LinerBiasOutHor);
	 //pPlate->ShiftVert(VShiftLinerIn, VShiftLinerOut);
	pPlate->Solid = TRUE;
	pPlate->Comment = "AV(7-8): bottom wall";
	pPlate->Fixed = 1; // side view
	AddCond(pPlate);//PlatesList.AddTail(pPlate);
//	if (TaskReionization)  SelectPlate(pPlate); // { pPlate->Selected = TRUE; SetLoadArray(pPlate, TRUE); }

	X = DDLinerOutX;
	DuctExitX = Max(X, DuctExitX);
//--------- DUCT0 ----------------------------
// DDLinerOut -> Duct0
	diaphragm.Number = 8; // Duct1
	pPlate = new CPlate(); // left
	//PlateCounter++;	pPlate->Number = PlateCounter; 
	 p0 = C3Point(DDLinerOutX,  DDLinerOutW * 0.5, - DDLinerOutH * 0.5);
	 p1 = C3Point(Duct1X,  Duct1W * 0.5, - Duct1H * 0.5);
	 p2 = C3Point(DDLinerOutX,  DDLinerOutW * 0.5,  DDLinerOutH * 0.5);
	 p3 = C3Point(Duct1X,  Duct1W * 0.5, Duct1H * 0.5);
	 pPlate->SetLocals(p0, p1, p2, p3);
	 pPlate->ShiftVert(LinerBiasOutVert + VShiftLinerOut, Duct1BiasVert + VShiftDuct1);
	 pPlate->ShiftHor(LinerBiasOutHor, Duct1BiasHor);
	 //pPlate->ShiftVert(VShiftDuct1, VShiftDuct2);
	pPlate->Solid = TRUE;
	pPlate->Comment = "DUCTLINER(8-9): left wall";
	AddCond(pPlate);//PlatesList.AddTail(pPlate);
//	if (TaskReionization)  SelectPlate(pPlate);
	diaphragm.Corn[2] = pPlate->Corn[2];

	pPlate = new CPlate(); // right
	//PlateCounter++;	pPlate->Number = PlateCounter; 
	 p0 = C3Point(DDLinerOutX,  - DDLinerOutW * 0.5, - DDLinerOutH * 0.5);
	 p1 = C3Point(Duct1X,  - Duct1W * 0.5, - Duct1H * 0.5);
	 p2 = C3Point(DDLinerOutX, - DDLinerOutW * 0.5,  DDLinerOutH * 0.5);
	 p3 = C3Point(Duct1X, - Duct1W * 0.5, Duct1H * 0.5);
	 pPlate->OrtDirect = -1;
	 pPlate->SetLocals(p0, p1, p2, p3);
	 pPlate->ShiftVert(LinerBiasOutVert + VShiftLinerOut, Duct1BiasVert + VShiftDuct1);
	 pPlate->ShiftHor(LinerBiasOutHor, Duct1BiasHor);
	 //pPlate->ShiftVert(VShiftDuct1, VShiftDuct2);
	pPlate->Solid = TRUE;
	pPlate->Comment = "DUCTLINER(8-9): right wall";
	AddCond(pPlate);//PlatesList.AddTail(pPlate);
//	if (TaskReionization)  SelectPlate(pPlate);
	diaphragm.Corn[0] = pPlate->Corn[1];
	Dia_Array.Add(diaphragm);//8

	lb1 = diaphragm.Corn[0]; ru1 = diaphragm.Corn[2];
	 vol = VOLUME(lb0, ru0, lb1, ru1, 1);
	 VolumeVector.push_back(vol);//7-8
	 lb0 = lb1; ru0 = ru1;

	pPlate = new CPlate(); // upper
	//PlateCounter++;	pPlate->Number = PlateCounter; 
	 p0 = C3Point(DDLinerOutX,  -DDLinerOutW * 0.5,  DDLinerOutH * 0.5);
	 p1 = C3Point(Duct1X, -Duct1W * 0.5, Duct1H * 0.5);
	 p2 = C3Point(DDLinerOutX,  DDLinerOutW * 0.5,  DDLinerOutH * 0.5);
	 p3 = C3Point(Duct1X,  Duct1W * 0.5, Duct1H * 0.5);
	 pPlate->OrtDirect = -1;
	 pPlate->SetLocals(p0, p1, p2, p3);
	 pPlate->ShiftVert(LinerBiasOutVert + VShiftLinerOut, Duct1BiasVert + VShiftDuct1);
	 pPlate->ShiftHor(LinerBiasOutHor, Duct1BiasHor);
	 //pPlate->ShiftVert(VShiftDuct1, VShiftDuct2);
	pPlate->Solid = TRUE;
	pPlate->Comment = "DUCTLINER(8-9): top wall";
	pPlate->Fixed = 1; // side view
	AddCond(pPlate);//PlatesList.AddTail(pPlate);
//	if (TaskReionization)  SelectPlate(pPlate);

	pPlate = new CPlate(); // lower
	//PlateCounter++;	pPlate->Number = PlateCounter; 
	 p0 = C3Point(DDLinerOutX, -DDLinerOutW * 0.5, -DDLinerOutH * 0.5);
	 p1 = C3Point(Duct1X, -Duct1W * 0.5, -Duct1H * 0.5);
	 p2 = C3Point(DDLinerOutX, DDLinerOutW * 0.5, -DDLinerOutH * 0.5);
	 p3 = C3Point(Duct1X,  Duct1W * 0.5, -Duct1H * 0.5);
	 pPlate->SetLocals(p0, p1, p2, p3);
	 pPlate->ShiftVert(LinerBiasOutVert + VShiftLinerOut, Duct1BiasVert + VShiftDuct1);
	 pPlate->ShiftHor(LinerBiasOutHor, Duct1BiasHor);
	 //pPlate->ShiftVert(VShiftDuct1, VShiftDuct2);
	pPlate->Solid = TRUE;
	pPlate->Comment = "DUCTLINER(8-9): bottom wall";
	pPlate->Fixed = 1; // side view
	AddCond(pPlate);//PlatesList.AddTail(pPlate);
//	if (TaskReionization)  SelectPlate(pPlate);

	X = Duct1X;
	DuctExitX = Max(X, DuctExitX);

//--------- DUCT1 ----------------------------
	diaphragm.Number = 9; // Duct2
	
	pPlate = new CPlate(); // right
	//PlateCounter++;	pPlate->Number = PlateCounter; 
	 p0 = C3Point(Duct1X,  Duct1W * 0.5, - Duct1H * 0.5);
	 p1 = C3Point(Duct2X,  Duct2W * 0.5, - Duct2H * 0.5);
	 p2 = C3Point(Duct1X,  Duct1W * 0.5,  Duct1H * 0.5);
	 p3 = C3Point(Duct2X,  Duct2W * 0.5,  Duct2H * 0.5);
	 pPlate->SetLocals(p0, p1, p2, p3);
	 pPlate->ShiftVert(Duct1BiasVert + VShiftDuct1, Duct2BiasVert + VShiftDuct2);
	 pPlate->ShiftHor(Duct1BiasHor, Duct2BiasHor);
	 //pPlate->ShiftVert(VShiftDuct1, VShiftDuct2);
	pPlate->Solid = TRUE;
	pPlate->Comment = "DUCT(9-10): left wall";
	AddCond(pPlate);//PlatesList.AddTail(pPlate);
//	if (TaskReionization)  SelectPlate(pPlate);
	diaphragm.Corn[2] = pPlate->Corn[2];

	pPlate = new CPlate(); // left
	//PlateCounter++;	pPlate->Number = PlateCounter; 
	 p0 = C3Point(Duct1X,  -Duct1W * 0.5, - Duct1H * 0.5);
	 p1 = C3Point(Duct2X,  -Duct2W * 0.5, - Duct2H * 0.5);
	 p2 = C3Point(Duct1X,  -Duct1W * 0.5,  Duct1H * 0.5);
	 p3 = C3Point(Duct2X,  -Duct2W * 0.5,  Duct2H * 0.5);
	 pPlate->OrtDirect = -1;
	 pPlate->SetLocals(p0, p1, p2, p3);
	 pPlate->ShiftVert(Duct1BiasVert + VShiftDuct1, Duct2BiasVert + VShiftDuct2);
	 pPlate->ShiftHor(Duct1BiasHor, Duct2BiasHor);
	 //pPlate->ShiftVert(VShiftDuct1, VShiftDuct2);
	pPlate->Solid = TRUE;
	pPlate->Comment = "DUCT(9-10): right wall";
	AddCond(pPlate);//PlatesList.AddTail(pPlate);
//	if (TaskReionization)  SelectPlate(pPlate);
	diaphragm.Corn[0] = pPlate->Corn[1];
	Dia_Array.Add(diaphragm);//9

	lb1 = diaphragm.Corn[0]; ru1 = diaphragm.Corn[2];
	 vol = VOLUME(lb0, ru0, lb1, ru1, 1);
	 VolumeVector.push_back(vol);//8-9
	 lb0 = lb1; ru0 = ru1;
	
	pPlate = new CPlate(); // upper
	//PlateCounter++;	pPlate->Number = PlateCounter; 
	 p0 = C3Point(Duct1X, - Duct1W * 0.5,  Duct1H * 0.5);
	 p2 = C3Point(Duct1X,  Duct1W * 0.5, Duct1H * 0.5);
	 p1 = C3Point(Duct2X, - Duct2W * 0.5,  Duct2H * 0.5);
	 p3 = C3Point(Duct2X,  Duct2W * 0.5,  Duct2H * 0.5);
	 pPlate->OrtDirect = -1;
	 pPlate->SetLocals(p0, p1, p2, p3);
	 pPlate->ShiftVert(Duct1BiasVert + VShiftDuct1, Duct2BiasVert + VShiftDuct2);
	 pPlate->ShiftHor(Duct1BiasHor, Duct2BiasHor);
	 //pPlate->ShiftVert(VShiftDuct1, VShiftDuct2);
	pPlate->Solid = TRUE;
	pPlate->Comment = "DUCT(9-10): top wall";
	pPlate->Fixed = 1; // side view
	AddCond(pPlate);//PlatesList.AddTail(pPlate);
//	if (TaskReionization)  SelectPlate(pPlate);

	pPlate = new CPlate(); // lower
	//PlateCounter++;	pPlate->Number = PlateCounter; 
	 p0 = C3Point(Duct1X, - Duct1W * 0.5,  -Duct1H * 0.5);
	 p2 = C3Point(Duct1X,  Duct1W * 0.5, -Duct1H * 0.5);
	 p1 = C3Point(Duct2X, - Duct2W * 0.5,  -Duct2H * 0.5);
	 p3 = C3Point(Duct2X,  Duct2W * 0.5,  -Duct2H * 0.5);
	 pPlate->SetLocals(p0, p1, p2, p3);
	 pPlate->ShiftVert(Duct1BiasVert + VShiftDuct1, Duct2BiasVert + VShiftDuct2);
	 pPlate->ShiftHor(Duct1BiasHor, Duct2BiasHor);
	// pPlate->ShiftVert(VShiftDuct1, VShiftDuct2);
	pPlate->Solid = TRUE;
	pPlate->Comment = "DUCT(9-10): bottom wall";
	pPlate->Fixed = 1; // side view
	AddCond(pPlate);//PlatesList.AddTail(pPlate);
//	if (TaskReionization)  SelectPlate(pPlate);

	X = Duct2X;
	DuctExitX = Max(X, DuctExitX);
	//--------- DUCT2 ----------------------------
	diaphragm.Number = 10; // Duct3

	pPlate = new CPlate(); // 
	//PlateCounter++;	pPlate->Number = PlateCounter; 
	 p0 = C3Point(Duct2X,  Duct2W * 0.5, - Duct2H * 0.5);
	 p1 = C3Point(Duct3X,  Duct3W * 0.5, - Duct3H * 0.5);
	 p2 = C3Point(Duct2X,  Duct2W * 0.5,  Duct2H * 0.5);
	 p3 = C3Point(Duct3X,  Duct3W * 0.5,  Duct3H * 0.5);
	 pPlate->SetLocals(p0, p1, p2, p3);
	 pPlate->ShiftVert(Duct2BiasVert + VShiftDuct2, Duct3BiasVert + VShiftDuct3);
	 pPlate->ShiftHor(Duct2BiasHor, Duct3BiasHor);
	// pPlate->ShiftVert(VShiftDuct2, VShiftDuct3);
	pPlate->Solid = TRUE;
	pPlate->Comment = "DUCT2: left wall";
	AddCond(pPlate); //PlatesList.AddTail(pPlate);
//	if (TaskReionization)  SelectPlate(pPlate);
	diaphragm.Corn[2] = pPlate->Corn[2];

	pPlate = new CPlate(); // 
	//PlateCounter++;	pPlate->Number = PlateCounter; 
	 p0 = C3Point(Duct2X,  -Duct2W * 0.5, - Duct2H * 0.5);
	 p1 = C3Point(Duct3X,  -Duct3W * 0.5, - Duct3H * 0.5);
	 p2 = C3Point(Duct2X,  -Duct2W * 0.5,  Duct2H * 0.5);
	 p3 = C3Point(Duct3X,  -Duct3W * 0.5,  Duct3H * 0.5);
	 pPlate->OrtDirect = -1;
	 pPlate->SetLocals(p0, p1, p2, p3);
	 pPlate->ShiftVert(Duct2BiasVert + VShiftDuct2, Duct3BiasVert + VShiftDuct3);
	 pPlate->ShiftHor(Duct2BiasHor, Duct3BiasHor);
	// pPlate->ShiftVert(VShiftDuct2, VShiftDuct3);
	pPlate->Solid = TRUE;
	pPlate->Comment = "DUCT2: right wall";
	AddCond(pPlate);//PlatesList.AddTail(pPlate);
//	if (TaskReionization)  SelectPlate(pPlate);
	diaphragm.Corn[0] = pPlate->Corn[1];
	Dia_Array.Add(diaphragm);//10

	lb1 = diaphragm.Corn[0]; ru1 = diaphragm.Corn[2];
	 vol = VOLUME(lb0, ru0, lb1, ru1, 1);
	 VolumeVector.push_back(vol);//9-10
	 lb0 = lb1; ru0 = ru1;

	pPlate = new CPlate(); // upper
	//PlateCounter++;	pPlate->Number = PlateCounter; 
	 p0 = C3Point(Duct2X, - Duct2W * 0.5,  Duct2H * 0.5);
	 p1 = C3Point(Duct2X,  Duct2W * 0.5, Duct2H * 0.5);
	 p2 = C3Point(Duct3X,  Duct3W * 0.5,  Duct3H * 0.5);
	 p3 = C3Point(Duct3X, - Duct3W * 0.5,  Duct3H * 0.5);
	 pPlate->OrtDirect = -1;
	 pPlate->SetLocals(p0, p1, p2, p3);
	 pPlate->ShiftVert(Duct2BiasVert + VShiftDuct2, Duct3BiasVert + VShiftDuct3);
	 pPlate->ShiftHor(Duct2BiasHor, Duct3BiasHor);
	 //pPlate->ShiftVert(VShiftDuct2, VShiftDuct3);
	pPlate->Solid = TRUE;
	pPlate->Comment = "DUCT2: top wall";
	pPlate->Fixed = 1; // side view
	AddCond(pPlate);//PlatesList.AddTail(pPlate);
//	if (TaskReionization)  SelectPlate(pPlate);

	pPlate = new CPlate(); // lower
	//PlateCounter++;	pPlate->Number = PlateCounter; 
	 p0 = C3Point(Duct2X, - Duct2W * 0.5,  -Duct2H * 0.5);
	 p1 = C3Point(Duct2X,  Duct2W * 0.5, -Duct2H * 0.5);
	 p2 = C3Point(Duct3X,  Duct3W * 0.5,  -Duct3H * 0.5);
	 p3 = C3Point(Duct3X, - Duct3W * 0.5,  -Duct3H * 0.5);
	 pPlate->SetLocals(p0, p1, p2, p3);
	 pPlate->ShiftVert(Duct2BiasVert + VShiftDuct2, Duct3BiasVert + VShiftDuct3);
	 pPlate->ShiftHor(Duct2BiasHor, Duct3BiasHor);
	 //pPlate->ShiftVert(VShiftDuct2, VShiftDuct3);
	pPlate->Solid = TRUE;
	pPlate->Comment = "DUCT2: bottom wall";
	pPlate->Fixed = 1; // side view
	AddCond(pPlate);//PlatesList.AddTail(pPlate);
//	if (TaskReionization)  SelectPlate(pPlate);

	X = Duct3X;
	DuctExitX = Max(X, DuctExitX);
//--------- DUCT3 ----------------------------
	diaphragm.Number = 11; // Duct4

	pPlate = new CPlate(); // 
	//PlateCounter++;	pPlate->Number = PlateCounter; 
	 p0 = C3Point(Duct3X,  Duct3W * 0.5, - Duct3H * 0.5);
	 p1 = C3Point(Duct4X,  Duct4W * 0.5, - Duct4H * 0.5);
	 p2 = C3Point(Duct3X,  Duct3W * 0.5,  Duct3H * 0.5);
	 p3 = C3Point(Duct4X,  Duct4W * 0.5,  Duct4H * 0.5);
	 pPlate->SetLocals(p0, p1, p2, p3);
	 pPlate->ShiftVert(Duct3BiasVert + VShiftDuct3, Duct4BiasVert + VShiftDuct4);
	 pPlate->ShiftHor(Duct3BiasHor, Duct4BiasHor);
	// pPlate->ShiftVert(VShiftDuct3, VShiftDuct4);
	pPlate->Solid = TRUE;
	pPlate->Comment = "DUCT3: left wall";
	AddCond(pPlate);//PlatesList.AddTail(pPlate);
//	if (TaskReionization)  SelectPlate(pPlate);
	diaphragm.Corn[2] = pPlate->Corn[2];

	pPlate = new CPlate(); // 
	//PlateCounter++;	pPlate->Number = PlateCounter; 
	 p0 = C3Point(Duct3X,  -Duct3W * 0.5, - Duct3H * 0.5);
	 p1 = C3Point(Duct4X,  -Duct4W * 0.5, - Duct4H * 0.5);
	 p2 = C3Point(Duct3X,  -Duct3W * 0.5,  Duct3H * 0.5);
	 p3 = C3Point(Duct4X,  -Duct4W * 0.5,  Duct4H * 0.5);
	 pPlate->OrtDirect = -1;
	 pPlate->SetLocals(p0, p1, p2, p3);
	 pPlate->ShiftVert(Duct3BiasVert + VShiftDuct3, Duct4BiasVert + VShiftDuct4);
	 pPlate->ShiftHor(Duct3BiasHor, Duct4BiasHor);
	// pPlate->ShiftVert(VShiftDuct3, VShiftDuct4);
	pPlate->Solid = TRUE;
	pPlate->Comment = "DUCT3: right wall";
	AddCond(pPlate);//PlatesList.AddTail(pPlate);
//	if (TaskReionization)  SelectPlate(pPlate);
	diaphragm.Corn[0] = pPlate->Corn[1];
	Dia_Array.Add(diaphragm);//11

	lb1 = diaphragm.Corn[0]; ru1 = diaphragm.Corn[2];
	 vol = VOLUME(lb0, ru0, lb1, ru1, 1);
	 VolumeVector.push_back(vol);//10-11
	 lb0 = lb1; ru0 = ru1;

	pPlate = new CPlate(); // upper
	//PlateCounter++;	pPlate->Number = PlateCounter; 
	 p0 = C3Point(Duct3X, - Duct3W * 0.5,  Duct3H * 0.5);
	 p2 = C3Point(Duct3X,  Duct3W * 0.5, Duct3H * 0.5);
	 p1 = C3Point(Duct4X, - Duct4W * 0.5,  Duct4H * 0.5);
	 p3 = C3Point(Duct4X,  Duct4W * 0.5,  Duct4H * 0.5);
	 pPlate->OrtDirect = -1;
	 pPlate->SetLocals(p0, p1, p2, p3);
	 pPlate->ShiftVert(Duct3BiasVert + VShiftDuct3, Duct4BiasVert + VShiftDuct4);
	 pPlate->ShiftHor(Duct3BiasHor, Duct4BiasHor);
	// pPlate->ShiftVert(VShiftDuct3, VShiftDuct4);
	pPlate->Solid = TRUE;
	pPlate->Comment = "DUCT3: top wall";
	pPlate->Fixed = 1; // side view
	AddCond(pPlate);//PlatesList.AddTail(pPlate);
//	if (TaskReionization)  SelectPlate(pPlate);

	pPlate = new CPlate(); // lower
	//PlateCounter++;	pPlate->Number = PlateCounter; 
	 p0 = C3Point(Duct3X, - Duct3W * 0.5,  -Duct3H * 0.5);
	 p2 = C3Point(Duct3X,  Duct3W * 0.5, -Duct3H * 0.5);
	 p1 = C3Point(Duct4X, - Duct4W * 0.5,  -Duct4H * 0.5);
	 p3 = C3Point(Duct4X,  Duct4W * 0.5,  -Duct4H * 0.5);
	 pPlate->SetLocals(p0, p1, p2, p3);
	 pPlate->ShiftVert(Duct3BiasVert + VShiftDuct3, Duct4BiasVert + VShiftDuct4);
	 pPlate->ShiftHor(Duct3BiasHor, Duct4BiasHor);
	// pPlate->ShiftVert(VShiftDuct3, VShiftDuct4);
	pPlate->Solid = TRUE;
	pPlate->Comment = "DUCT3: bottom wall";
	pPlate->Fixed = 1; // side view
	AddCond(pPlate);//PlatesList.AddTail(pPlate);
//	if (TaskReionization)  SelectPlate(pPlate);

	X = Duct4X;
	DuctExitX = Max(X, DuctExitX);
	//--------- DUCT4 ----------------------------
	diaphragm.Number = 12; // Duct5

	pPlate = new CPlate(); // 
	//PlateCounter++;	pPlate->Number = PlateCounter; 
	 p0 = C3Point(Duct4X,  Duct4W * 0.5, - Duct4H * 0.5);
	 p1 = C3Point(Duct5X,  Duct5W * 0.5, - Duct5H * 0.5);
	 p2 = C3Point(Duct4X,  Duct4W * 0.5,  Duct4H * 0.5);
	 p3 = C3Point(Duct5X,  Duct5W * 0.5,  Duct5H * 0.5);
	 pPlate->SetLocals(p0, p1, p2, p3);
	 pPlate->ShiftVert(Duct4BiasVert + VShiftDuct4, Duct5BiasVert + VShiftDuct5);
	 pPlate->ShiftHor(Duct4BiasHor, Duct5BiasHor);
	// pPlate->ShiftVert(VShiftDuct4, VShiftDuct5);
	pPlate->Solid = TRUE;
	pPlate->Comment = "DUCT4: left wall";
	AddCond(pPlate);//PlatesList.AddTail(pPlate);
//	if (TaskReionization)  SelectPlate(pPlate);
	diaphragm.Corn[2] = pPlate->Corn[2];

	pPlate = new CPlate(); // 
	//PlateCounter++;	pPlate->Number = PlateCounter; 
	 p0 = C3Point(Duct4X,  -Duct4W * 0.5, - Duct4H * 0.5);
	 p1 = C3Point(Duct5X,  -Duct5W * 0.5, - Duct5H * 0.5);
	 p2 = C3Point(Duct4X,  -Duct4W * 0.5,  Duct4H * 0.5);
	 p3 = C3Point(Duct5X,  -Duct5W * 0.5,  Duct5H * 0.5);
	 pPlate->OrtDirect = -1;
	 pPlate->SetLocals(p0, p1, p2, p3);
	 pPlate->ShiftVert(Duct4BiasVert + VShiftDuct4, Duct5BiasVert + VShiftDuct5);
	 pPlate->ShiftHor(Duct4BiasHor, Duct5BiasHor);
	// pPlate->ShiftVert(VShiftDuct4, VShiftDuct5);
	pPlate->Solid = TRUE;
	pPlate->Comment = "DUCT4: right wall";
	AddCond(pPlate);//PlatesList.AddTail(pPlate);
//	if (TaskReionization)  SelectPlate(pPlate);
	diaphragm.Corn[0] = pPlate->Corn[1];
	Dia_Array.Add(diaphragm);//12

	lb1 = diaphragm.Corn[0]; ru1 = diaphragm.Corn[2];
	 vol = VOLUME(lb0, ru0, lb1, ru1, 1);
	 VolumeVector.push_back(vol);//11-12
	 lb0 = lb1; ru0 = ru1;

	pPlate = new CPlate(); // upper
	//PlateCounter++;	pPlate->Number = PlateCounter; 
	 p0 = C3Point(Duct4X, - Duct4W * 0.5,  Duct4H * 0.5);
	 p2 = C3Point(Duct4X,  Duct4W * 0.5, Duct4H * 0.5);
	 p1 = C3Point(Duct5X, - Duct5W * 0.5,  Duct5H * 0.5);
	 p3 = C3Point(Duct5X,  Duct5W * 0.5,  Duct5H * 0.5);
	 pPlate->OrtDirect = -1;
	 pPlate->SetLocals(p0, p1, p2, p3);
	 pPlate->ShiftVert(Duct4BiasVert + VShiftDuct4, Duct5BiasVert + VShiftDuct5);
	 pPlate->ShiftHor(Duct4BiasHor, Duct5BiasHor);
	// pPlate->ShiftVert(VShiftDuct4, VShiftDuct5);
	pPlate->Solid = TRUE;
	pPlate->Comment = "DUCT4: top wall";
	pPlate->Fixed = 1; // side view
	AddCond(pPlate);//PlatesList.AddTail(pPlate);
//	if (TaskReionization)  SelectPlate(pPlate);

	pPlate = new CPlate(); // lower
	//PlateCounter++;	pPlate->Number = PlateCounter; 
	 p0 = C3Point(Duct4X, - Duct4W * 0.5,  -Duct4H * 0.5);
	 p2 = C3Point(Duct4X,  Duct4W * 0.5, -Duct4H * 0.5);
	 p1 = C3Point(Duct5X, - Duct5W * 0.5,  -Duct5H * 0.5);
	 p3 = C3Point(Duct5X,  Duct5W * 0.5,  -Duct5H * 0.5);
	 pPlate->SetLocals(p0, p1, p2, p3);
	 pPlate->ShiftVert(Duct4BiasVert + VShiftDuct4, Duct5BiasVert + VShiftDuct5);
	 pPlate->ShiftHor(Duct4BiasHor, Duct5BiasHor);
	// pPlate->ShiftVert(VShiftDuct4, VShiftDuct5);
	pPlate->Solid = TRUE;
	pPlate->Comment = "DUCT4: bottom wall";
	pPlate->Fixed = 1; // side view
	AddCond(pPlate);//PlatesList.AddTail(pPlate);
//	if (TaskReionization)  SelectPlate(pPlate);

	X = Duct5X;
	DuctExitX = Max(X, DuctExitX);
	//--------- DUCT5 ----------------------------
	diaphragm.Number = 13; // Duct6

	pPlate = new CPlate(); // 
	//PlateCounter++;	pPlate->Number = PlateCounter; 
	 p0 = C3Point(Duct5X,  Duct5W * 0.5, - Duct5H * 0.5);
	 p1 = C3Point(Duct6X,  Duct6W * 0.5, - Duct6H * 0.5);
	 p2 = C3Point(Duct5X,  Duct5W * 0.5,  Duct5H * 0.5);
	 p3 = C3Point(Duct6X,  Duct6W * 0.5,  Duct6H * 0.5);
	 pPlate->SetLocals(p0, p1, p2, p3);
	 pPlate->ShiftVert(Duct5BiasVert + VShiftDuct5, Duct6BiasVert + VShiftDuct6);
	 pPlate->ShiftHor(Duct5BiasHor, Duct6BiasHor);
	// pPlate->ShiftVert(VShiftDuct5, VShiftDuct6);
	pPlate->Solid = TRUE;
	pPlate->Comment = "DUCT5: left wall";
	AddCond(pPlate);//PlatesList.AddTail(pPlate);
//	if (TaskReionization)  SelectPlate(pPlate);
	diaphragm.Corn[2] = pPlate->Corn[2];

	pPlate = new CPlate(); // 
	//PlateCounter++;	pPlate->Number = PlateCounter; 
	 p0 = C3Point(Duct5X,  -Duct5W * 0.5, - Duct5H * 0.5);
	 p1 = C3Point(Duct6X,  -Duct6W * 0.5, - Duct6H * 0.5);
	 p2 = C3Point(Duct5X,  -Duct5W * 0.5,  Duct5H * 0.5);
	 p3 = C3Point(Duct6X,  -Duct6W * 0.5,  Duct6H * 0.5);
	 pPlate->OrtDirect = -1;
	 pPlate->SetLocals(p0, p1, p2, p3);
	 pPlate->ShiftVert(Duct5BiasVert + VShiftDuct5, Duct6BiasVert + VShiftDuct6);
	 pPlate->ShiftHor(Duct5BiasHor, Duct6BiasHor);
	 //pPlate->ShiftVert(VShiftDuct5, VShiftDuct6);
	pPlate->Solid = TRUE;
	pPlate->Comment = "DUCT5: right wall";
	AddCond(pPlate);//PlatesList.AddTail(pPlate);
//	if (TaskReionization)  SelectPlate(pPlate);
	diaphragm.Corn[0] = pPlate->Corn[1];
	Dia_Array.Add(diaphragm);//13

	lb1 = diaphragm.Corn[0]; ru1 = diaphragm.Corn[2];
	 vol = VOLUME(lb0, ru0, lb1, ru1, 1);
	 VolumeVector.push_back(vol);//12-13
	 lb0 = lb1; ru0 = ru1;

	pPlate = new CPlate(); // upper
	//PlateCounter++;	pPlate->Number = PlateCounter; 
	 p0 = C3Point(Duct5X, - Duct5W * 0.5,  Duct5H * 0.5);
	 p2 = C3Point(Duct5X,  Duct5W * 0.5, Duct5H * 0.5);
	 p1 = C3Point(Duct6X, - Duct6W * 0.5,  Duct6H * 0.5);
	 p3 = C3Point(Duct6X,  Duct6W * 0.5,  Duct6H * 0.5);
	 pPlate->OrtDirect = -1;
	 pPlate->SetLocals(p0, p1, p2, p3);
	 pPlate->ShiftVert(Duct5BiasVert + VShiftDuct5, Duct6BiasVert + VShiftDuct6);
	 pPlate->ShiftHor(Duct5BiasHor, Duct6BiasHor);
	// pPlate->ShiftVert(VShiftDuct5, VShiftDuct6);
	pPlate->Solid = TRUE;
	pPlate->Comment = "DUCT5: top wall";
	pPlate->Fixed = 1; // side view
	AddCond(pPlate);//PlatesList.AddTail(pPlate);
//	if (TaskReionization)  SelectPlate(pPlate);

	pPlate = new CPlate(); // lower
	//PlateCounter++;	pPlate->Number = PlateCounter; 
	 p0 = C3Point(Duct5X, - Duct5W * 0.5,  -Duct5H * 0.5);
	 p2 = C3Point(Duct5X,  Duct5W * 0.5, -Duct5H * 0.5);
	 p1 = C3Point(Duct6X, - Duct6W * 0.5,  -Duct6H * 0.5);
	 p3 = C3Point(Duct6X,  Duct6W * 0.5,  -Duct6H * 0.5);
	 pPlate->SetLocals(p0, p1, p2, p3);
	 pPlate->ShiftVert(Duct5BiasVert + VShiftDuct5, Duct6BiasVert + VShiftDuct6);
	 pPlate->ShiftHor(Duct5BiasHor, Duct6BiasHor);
	 //pPlate->ShiftVert(VShiftDuct5, VShiftDuct6);
	pPlate->Solid = TRUE;
	pPlate->Comment = "DUCT5: bottom wall";
	pPlate->Fixed = 1; // side view
	AddCond(pPlate);//PlatesList.AddTail(pPlate);
//	if (TaskReionization)  SelectPlate(pPlate);

	X = Duct6X;
	DuctExitX = Max(X, DuctExitX);
	//--------- DUCT6 ----------------------------
	diaphragm.Number = 14; // Duct7

	pPlate = new CPlate(); // 
	//PlateCounter++;	pPlate->Number = PlateCounter; 
	 p0 = C3Point(Duct6X,  Duct6W * 0.5, - Duct6H * 0.5);
	 p1 = C3Point(Duct7X,  Duct7W * 0.5, - Duct7H * 0.5);
	 p2 = C3Point(Duct6X,  Duct6W * 0.5,  Duct6H * 0.5);
	 p3 = C3Point(Duct7X,  Duct7W * 0.5,  Duct7H * 0.5);
	 pPlate->SetLocals(p0, p1, p2, p3);
	 pPlate->ShiftVert(Duct6BiasVert + VShiftDuct6, Duct7BiasVert + VShiftDuct7);
	 pPlate->ShiftHor(Duct6BiasHor, Duct7BiasHor);
	// pPlate->ShiftVert(VShiftDuct5, VShiftDuct6);
	pPlate->Solid = TRUE;
	pPlate->Comment = "DUCT6: left wall";
	AddCond(pPlate);//PlatesList.AddTail(pPlate);
//	if (TaskReionization)  SelectPlate(pPlate);
	diaphragm.Corn[2] = pPlate->Corn[2];

	pPlate = new CPlate(); // 
	//PlateCounter++;	pPlate->Number = PlateCounter; 
	 p0 = C3Point(Duct6X,  -Duct6W * 0.5, - Duct6H * 0.5);
	 p1 = C3Point(Duct7X,  -Duct7W * 0.5, - Duct7H * 0.5);
	 p2 = C3Point(Duct6X,  -Duct6W * 0.5,  Duct6H * 0.5);
	 p3 = C3Point(Duct7X,  -Duct7W * 0.5,  Duct7H * 0.5);
	 pPlate->OrtDirect = -1;
	 pPlate->SetLocals(p0, p1, p2, p3);
	 pPlate->ShiftVert(Duct6BiasVert + VShiftDuct6, Duct7BiasVert + VShiftDuct7);
	 pPlate->ShiftHor(Duct6BiasHor, Duct7BiasHor);
	 //pPlate->ShiftVert(VShiftDuct5, VShiftDuct6);
	pPlate->Solid = TRUE;
	pPlate->Comment = "DUCT6: right wall";
	AddCond(pPlate);//PlatesList.AddTail(pPlate);
//	if (TaskReionization)  SelectPlate(pPlate);
	diaphragm.Corn[0] = pPlate->Corn[1];
	Dia_Array.Add(diaphragm);//14

	lb1 = diaphragm.Corn[0]; ru1 = diaphragm.Corn[2];
	 vol = VOLUME(lb0, ru0, lb1, ru1, 1);
	 VolumeVector.push_back(vol);//13-14
	 lb0 = lb1; ru0 = ru1;

	pPlate = new CPlate(); // upper
	//PlateCounter++;	pPlate->Number = PlateCounter; 
	 p0 = C3Point(Duct6X, - Duct6W * 0.5,  Duct6H * 0.5);
	 p2 = C3Point(Duct6X,  Duct6W * 0.5, Duct6H * 0.5);
	 p1 = C3Point(Duct7X, - Duct7W * 0.5,  Duct7H * 0.5);
	 p3 = C3Point(Duct7X,  Duct7W * 0.5,  Duct7H * 0.5);
	 pPlate->OrtDirect = -1;
	 pPlate->SetLocals(p0, p1, p2, p3);
	 pPlate->ShiftVert(Duct6BiasVert + VShiftDuct6, Duct7BiasVert + VShiftDuct7);
	 pPlate->ShiftHor(Duct6BiasHor, Duct7BiasHor);
	// pPlate->ShiftVert(VShiftDuct5, VShiftDuct6);
	pPlate->Solid = TRUE;
	pPlate->Comment = "DUCT6: top wall";
	pPlate->Fixed = 1; // side view
	AddCond(pPlate);//PlatesList.AddTail(pPlate);
//	if (TaskReionization)  SelectPlate(pPlate);

	pPlate = new CPlate(); // lower
	//PlateCounter++;	pPlate->Number = PlateCounter; 
	 p0 = C3Point(Duct6X, - Duct6W * 0.5,  -Duct6H * 0.5);
	 p2 = C3Point(Duct6X,  Duct6W * 0.5, -Duct6H * 0.5);
	 p1 = C3Point(Duct7X, - Duct7W * 0.5,  -Duct7H * 0.5);
	 p3 = C3Point(Duct7X,  Duct7W * 0.5,  -Duct7H * 0.5);
	 pPlate->SetLocals(p0, p1, p2, p3);
	 pPlate->ShiftVert(Duct6BiasVert + VShiftDuct6, Duct7BiasVert + VShiftDuct7);
	 pPlate->ShiftHor(Duct6BiasHor, Duct7BiasHor);
	 //pPlate->ShiftVert(VShiftDuct5, VShiftDuct6);
	pPlate->Solid = TRUE;
	pPlate->Comment = "DUCT6: bottom wall";
	pPlate->Fixed = 1; // side view
	AddCond(pPlate);//PlatesList.AddTail(pPlate);
//	if (TaskReionization)  SelectPlate(pPlate);

	X = Duct7X;
	DuctExitX = Max(X, DuctExitX);
	//--------- DUCT7 ----------------------------
	diaphragm.Number = 15; // Duct8

	pPlate = new CPlate(); // 
	//PlateCounter++;	pPlate->Number = PlateCounter; 
	 p0 = C3Point(Duct7X,  Duct7W * 0.5, - Duct7H * 0.5);
	 p1 = C3Point(Duct8X,  Duct8W * 0.5, - Duct8H * 0.5);
	 p2 = C3Point(Duct7X,  Duct7W * 0.5,  Duct7H * 0.5);
	 p3 = C3Point(Duct8X,  Duct8W * 0.5,  Duct8H * 0.5);
	 pPlate->SetLocals(p0, p1, p2, p3);
	 pPlate->ShiftVert(Duct7BiasVert + VShiftDuct7, Duct8BiasVert + VShiftDuct8);
	 pPlate->ShiftHor(Duct7BiasHor, Duct8BiasHor);
	// pPlate->ShiftVert(VShiftDuct5, VShiftDuct6);
	pPlate->Solid = TRUE;
	pPlate->Comment = "DUCT7: left wall";
	AddCond(pPlate);//PlatesList.AddTail(pPlate);
//	if (TaskReionization)  SelectPlate(pPlate);
	diaphragm.Corn[2] = pPlate->Corn[2];

	pPlate = new CPlate(); // 
	//PlateCounter++;	pPlate->Number = PlateCounter; 
	 p0 = C3Point(Duct7X,  -Duct7W * 0.5, - Duct7H * 0.5);
	 p1 = C3Point(Duct8X,  -Duct8W * 0.5, - Duct8H * 0.5);
	 p2 = C3Point(Duct7X,  -Duct7W * 0.5,  Duct7H * 0.5);
	 p3 = C3Point(Duct8X,  -Duct8W * 0.5,  Duct8H * 0.5);
	 pPlate->OrtDirect = -1;
	 pPlate->SetLocals(p0, p1, p2, p3);
	 pPlate->ShiftVert(Duct7BiasVert + VShiftDuct7, Duct8BiasVert + VShiftDuct8);
	 pPlate->ShiftHor(Duct7BiasHor, Duct8BiasHor);
	 //pPlate->ShiftVert(VShiftDuct5, VShiftDuct6);
	pPlate->Solid = TRUE;
	pPlate->Comment = "DUCT7: right wall";
	AddCond(pPlate);//PlatesList.AddTail(pPlate);
//	if (TaskReionization)  SelectPlate(pPlate);
	diaphragm.Corn[0] = pPlate->Corn[1];
	Dia_Array.Add(diaphragm);//15

	lb1 = diaphragm.Corn[0]; ru1 = diaphragm.Corn[2];
	 vol = VOLUME(lb0, ru0, lb1, ru1, 1);
	 VolumeVector.push_back(vol);//14-15
	 lb0 = lb1; ru0 = ru1;

	pPlate = new CPlate(); // upper
	//PlateCounter++;	pPlate->Number = PlateCounter; 
	 p0 = C3Point(Duct7X, - Duct7W * 0.5,  Duct7H * 0.5);
	 p2 = C3Point(Duct7X,  Duct7W * 0.5, Duct7H * 0.5);
	 p1 = C3Point(Duct8X, - Duct8W * 0.5,  Duct8H * 0.5);
	 p3 = C3Point(Duct8X,  Duct8W * 0.5,  Duct8H * 0.5);
	 pPlate->OrtDirect = -1;
	 pPlate->SetLocals(p0, p1, p2, p3);
	 pPlate->ShiftVert(Duct7BiasVert + VShiftDuct7, Duct8BiasVert + VShiftDuct8);
	 pPlate->ShiftHor(Duct8BiasHor, Duct8BiasHor);
	// pPlate->ShiftVert(VShiftDuct5, VShiftDuct6);
	pPlate->Solid = TRUE;
	pPlate->Comment = "DUCT7: top wall";
	pPlate->Fixed = 1; // side view
	AddCond(pPlate);//PlatesList.AddTail(pPlate);
//	if (TaskReionization)  SelectPlate(pPlate);

	pPlate = new CPlate(); // lower
	//PlateCounter++;	pPlate->Number = PlateCounter; 
	 p0 = C3Point(Duct7X, - Duct7W * 0.5,  -Duct7H * 0.5);
	 p2 = C3Point(Duct7X,  Duct7W * 0.5, -Duct7H * 0.5);
	 p1 = C3Point(Duct8X, - Duct8W * 0.5,  -Duct8H * 0.5);
	 p3 = C3Point(Duct8X,  Duct8W * 0.5,  -Duct8H * 0.5);
	 pPlate->SetLocals(p0, p1, p2, p3);
	 pPlate->ShiftVert(Duct7BiasVert + VShiftDuct7, Duct8BiasVert + VShiftDuct8);
	 pPlate->ShiftHor(Duct7BiasHor, Duct8BiasHor);
	 //pPlate->ShiftVert(VShiftDuct5, VShiftDuct6);
	pPlate->Solid = TRUE;
	pPlate->Comment = "DUCT7: bottom wall";
	pPlate->Fixed = 1; // side view
	AddCond(pPlate);//PlatesList.AddTail(pPlate);
//	if (TaskReionization)  SelectPlate(pPlate);

	X = Duct8X;
	DuctExitX = Max(X, DuctExitX);
	
	//if (DuctExitX < 0 && PlasmaXmin > 0) // if duct not loaded
		//DuctExitX = PlasmaXmin;//PlasmaEmitter;

	DuctExitYmin = -Duct8W * 0.5 + Duct8BiasHor;
	DuctExitYmax = Duct8W * 0.5 + Duct8BiasHor;
	DuctExitZmin = -Duct8H * 0.5 + VShiftDuct8 + Duct8BiasVert;
	DuctExitZmax = Duct8H * 0.5 + VShiftDuct8 + Duct8BiasVert; 

	 pPlate = new CPlate(); // at Duct Exit
	 //PlateCounter++;	 pPlate->Number = PlateCounter; 
	 p0 = C3Point(DuctExitX + 0.01, DuctExitYmin, DuctExitZmin);
	 p3 = C3Point(DuctExitX + 0.01, DuctExitYmax, DuctExitZmax);
	 pPlate->SetFromLimits(p0, p3);
	// p0 = C3Point(Duct8X + 0.02, - Duct8W, -Duct8H);
	//p1 = C3Point(Duct8X + 0.02,  Duct8W,  -Duct8H);
	//p2 = C3Point(Duct8X + 0.02, - Duct8W, Duct8H);
	// p3 = C3Point(Duct8X + 0.02,  Duct8W,  Duct8H);
	 pPlate->OrtDirect = -1;
	 //pPlate->SetLocals(p0, p1, p2, p3); \\ called in Setfromlimits
	 //pPlate->Shift(0, Duct8BiasHor, VShiftDuct8 + Duct8BiasVert);
	 //pPlate->Shift(0,0, VShiftDia);
	 pPlate->Solid = FALSE;
	 pPlate->Visible = FALSE;
	 pPlate->MAP = TRUE;
	 S.Format("Duct Exit Cross-plane, X = %g m", DuctExitX);
	 pPlate->Comment = S;
	 bool added = AddCond(pPlate);//PlatesList.AddTail(pPlate);

	 if (added) DuctExit = pPlate->Number;
		 //PlasmaEmitter = pPlate->Number; // else - keep AreaLimit
	 
}

void CBTRDoc::SetDuctDia() // duct cross-sections - transparent 
{
	C3Point p0, p1, p2, p3;
	CString S;
	CPlate * pPlate;
	
	//diaphragm.Number = 5; // PreDuct
	pPlate = new CPlate(); //Pre-Duct Diafragm
	//PlateCounter++;	pPlate->Number = PlateCounter;
	p0 = C3Point(PreDuctX, AreaHorMin, AreaVertMin);
	p1 = C3Point(PreDuctX, AreaHorMax, AreaVertMin);
	p2 = C3Point(PreDuctX, AreaHorMin, AreaVertMax);
	p3 = C3Point(PreDuctX, AreaHorMax, AreaVertMax);
	pPlate->OrtDirect = -1;
	pPlate->SetLocals(p0, p1, p2, p3);
	//pPlate->Shift(0, DiaBiasHor, VShiftDia + DiaBiasVert);
	//pPlate->Corn[0].Z = AreaVertMin; pPlate->Corn[1].Z = AreaVertMin;
	//pPlate->SetLocals(pPlate->Corn[0], pPlate->Corn[1], pPlate->Corn[3], pPlate->Corn[2]);
	//pPlate->Shift(0,0, VShiftDia);
	pPlate->Solid = FALSE;
	pPlate->Visible = FALSE;
	pPlate->Comment = "Dia5 (Scraper1)";
	pPlate->Fixed = 1; // side view
	AddCond(pPlate);//PlatesList.AddTail(pPlate);
	//diaphragm.Corn[0] = pPlate->Corn[3]; //p2
	
	//diaphragm.Number = 6; // DDLinerIn
	pPlate = new CPlate(); // 
	//PlateCounter++;	pPlate->Number = PlateCounter;
	p0 = C3Point(DDLinerInX, AreaHorMin, AreaVertMin);
	p1 = C3Point(DDLinerInX, AreaHorMax, AreaVertMin);
	p2 = C3Point(DDLinerInX, AreaHorMin, AreaVertMax);
	p3 = C3Point(DDLinerInX, AreaHorMax, AreaVertMax);
	pPlate->SetLocals(p0, p1, p2, p3);
	//pPlate->ShiftVert(DiaBiasVert + VShiftDia, LinerBiasInVert + VShiftLinerIn);
	//pPlate->ShiftHor(DiaBiasHor, LinerBiasOutHor);
	//pPlate->Shift(0, LinerHBias, VShiftLinerIn + LinerVBias);
	pPlate->Solid = FALSE;
	pPlate->Visible = FALSE;
	pPlate->Comment = "Dia6 (Scraper2)";
	pPlate->Fixed = 1; // side view
	AddCond(pPlate);//PlatesList.AddTail(pPlate);
	//diaphragm.Corn[2] = pPlate->Corn[2];//p3
	
	//diaphragm.Number = 7; //DDLinerOutX
	pPlate = new CPlate(); // 
	//PlateCounter++;	pPlate->Number = PlateCounter;
	p0 = C3Point(DDLinerOutX, AreaHorMin, AreaVertMin);
	p1 = C3Point(DDLinerOutX, AreaHorMax, AreaVertMin);
	p2 = C3Point(DDLinerOutX, AreaHorMin, AreaVertMax);
	p3 = C3Point(DDLinerOutX, AreaHorMax, AreaVertMax);
	pPlate->SetLocals(p0, p1, p2, p3);
	pPlate->Solid = FALSE;
	pPlate->Visible = FALSE;
	pPlate->Comment = "Dia7 (DDL)";
	pPlate->Fixed = 1; // side view
	AddCond(pPlate);//PlatesList.AddTail(pPlate);

	//diaphragm.Number = 8; //Duct1X
	pPlate = new CPlate(); // 
	//PlateCounter++;	pPlate->Number = PlateCounter;
	p0 = C3Point(Duct1X, AreaHorMin, AreaVertMin);
	p1 = C3Point(Duct1X, AreaHorMax, AreaVertMin);
	p2 = C3Point(Duct1X, AreaHorMin, AreaVertMax);
	p3 = C3Point(Duct1X, AreaHorMax, AreaVertMax);
	pPlate->SetLocals(p0, p1, p2, p3);
	pPlate->Solid = FALSE;
	pPlate->Visible = FALSE;
	pPlate->Comment = "Dia8 (Duct)";
	pPlate->Fixed = 1; // side view
	AddCond(pPlate);//PlatesList.AddTail(pPlate);

	//diaphragm.Number = 9; //Duct2X
	pPlate = new CPlate(); // 
	//PlateCounter++;	pPlate->Number = PlateCounter;
	p0 = C3Point(Duct2X, AreaHorMin, AreaVertMin);
	p1 = C3Point(Duct2X, AreaHorMax, AreaVertMin);
	p2 = C3Point(Duct2X, AreaHorMin, AreaVertMax);
	p3 = C3Point(Duct2X, AreaHorMax, AreaVertMax);
	pPlate->SetLocals(p0, p1, p2, p3);
	pPlate->Solid = FALSE;
	pPlate->Visible = FALSE;
	pPlate->Comment = "Dia9 (Duct)";
	pPlate->Fixed = 1; // side view
	AddCond(pPlate);//PlatesList.AddTail(pPlate);

	//diaphragm.Number = 10; //Duct3X
	pPlate = new CPlate(); // 
	//PlateCounter++;	pPlate->Number = PlateCounter;
	p0 = C3Point(Duct3X, AreaHorMin, AreaVertMin);
	p1 = C3Point(Duct3X, AreaHorMax, AreaVertMin);
	p2 = C3Point(Duct3X, AreaHorMin, AreaVertMax);
	p3 = C3Point(Duct3X, AreaHorMax, AreaVertMax);
	pPlate->SetLocals(p0, p1, p2, p3);
	pPlate->Solid = FALSE;
	pPlate->Visible = FALSE;
	pPlate->Comment = "Dia10 (Duct)";
	pPlate->Fixed = 1; // side view
	AddCond(pPlate);//PlatesList.AddTail(pPlate);

	//diaphragm.Number = 11; //Duct4X
	pPlate = new CPlate(); // 
	//PlateCounter++;	pPlate->Number = PlateCounter;
	p0 = C3Point(Duct4X, AreaHorMin, AreaVertMin);
	p1 = C3Point(Duct4X, AreaHorMax, AreaVertMin);
	p2 = C3Point(Duct4X, AreaHorMin, AreaVertMax);
	p3 = C3Point(Duct4X, AreaHorMax, AreaVertMax);
	pPlate->SetLocals(p0, p1, p2, p3);
	pPlate->Solid = FALSE;
	pPlate->Visible = FALSE;
	pPlate->Comment = "Dia11 (Duct)";
	pPlate->Fixed = 1; // side view
	AddCond(pPlate);//PlatesList.AddTail(pPlate);

	//diaphragm.Number = 12; //Duct5X
	pPlate = new CPlate(); // 
	//PlateCounter++;	pPlate->Number = PlateCounter;
	p0 = C3Point(Duct5X, AreaHorMin, AreaVertMin);
	p1 = C3Point(Duct5X, AreaHorMax, AreaVertMin);
	p2 = C3Point(Duct5X, AreaHorMin, AreaVertMax);
	p3 = C3Point(Duct5X, AreaHorMax, AreaVertMax);
	pPlate->SetLocals(p0, p1, p2, p3);
	pPlate->Solid = FALSE;
	pPlate->Visible = FALSE;
	pPlate->Comment = "Dia12 (Duct)";
	pPlate->Fixed = 1; // side view
	AddCond(pPlate);//PlatesList.AddTail(pPlate);

	//diaphragm.Number = 13; //Duct6X
	pPlate = new CPlate(); // 
	//PlateCounter++;	pPlate->Number = PlateCounter;
	p0 = C3Point(Duct6X, AreaHorMin, AreaVertMin);
	p1 = C3Point(Duct6X, AreaHorMax, AreaVertMin);
	p2 = C3Point(Duct6X, AreaHorMin, AreaVertMax);
	p3 = C3Point(Duct6X, AreaHorMax, AreaVertMax);
	pPlate->SetLocals(p0, p1, p2, p3);
	pPlate->Solid = FALSE;
	pPlate->Visible = FALSE;
	pPlate->Comment = "Dia13 (Duct)";
	pPlate->Fixed = 1; // side view
	AddCond(pPlate);//PlatesList.AddTail(pPlate);

	//diaphragm.Number = 14; //Duct7X
	pPlate = new CPlate(); // 
	//PlateCounter++;	pPlate->Number = PlateCounter;
	p0 = C3Point(Duct7X, AreaHorMin, AreaVertMin);
	p1 = C3Point(Duct7X, AreaHorMax, AreaVertMin);
	p2 = C3Point(Duct7X, AreaHorMin, AreaVertMax);
	p3 = C3Point(Duct7X, AreaHorMax, AreaVertMax);
	pPlate->SetLocals(p0, p1, p2, p3);
	pPlate->Solid = FALSE;
	pPlate->Visible = FALSE;
	pPlate->Comment = "Dia14 (Duct)";
	pPlate->Fixed = 1; // side view
	AddCond(pPlate);//PlatesList.AddTail(pPlate);
	
	/*lb1 = diaphragm.Corn[0]; ru1 = diaphragm.Corn[2];
	vol = VOLUME(lb0, ru0, lb1, ru1, 1);
	VolumeVector.push_back(vol);//14-15
	lb0 = lb1; ru0 = ru1;*/

	//diaphragm.Number = 15; //Duct8X
	pPlate = new CPlate(); // Duct Exit
	//PlateCounter++;	pPlate->Number = PlateCounter;
	//p0 = C3Point(Duct8X, -Duct8W * 0.5, -Duct8H * 0.5);
	//p3 = C3Point(Duct8X, Duct8W * 0.5, Duct8H * 0.5);
	// pPlate->SetFromLimits(p0, p3);
	/*p0 = C3Point(Duct8X, -Duct8W, -Duct8H);
	p1 = C3Point(Duct8X, Duct8W, -Duct8H);
	p2 = C3Point(Duct8X, -Duct8W, Duct8H);
	p3 = C3Point(Duct8X, Duct8W, Duct8H);*/
	p0 = C3Point(Duct8X, AreaHorMin, AreaVertMin);
	p1 = C3Point(Duct8X, AreaHorMax, AreaVertMin);
	p2 = C3Point(Duct8X, AreaHorMin, AreaVertMax);
	p3 = C3Point(Duct8X, AreaHorMax, AreaVertMax);
	pPlate->OrtDirect = -1;
	pPlate->SetLocals(p0, p1, p2, p3);
	//pPlate->Shift(0, Duct8BiasHor, VShiftDuct8 + Duct8BiasVert);
	pPlate->Solid = FALSE;
	pPlate->Visible = FALSE;
	//S.Format("Duct Exit Cross-plane, X = %g m", Duct8X + 0.01);
	pPlate->Comment = "Dia15 (Exit)";
	AddCond(pPlate);//PlatesList.AddTail(pPlate);

	DuctExitYmin = -Duct8W * 0.5 + Duct8BiasHor;
	DuctExitYmax = Duct8W * 0.5 + Duct8BiasHor;
	DuctExitZmin = -Duct8H * 0.5 + VShiftDuct8 + Duct8BiasVert;
	DuctExitZmax = Duct8H * 0.5 + VShiftDuct8 + Duct8BiasVert;
}

void  CBTRDoc:: ModifyArea(bool fixed)
{
	//AfxMessageBox("Enter ModifyArea");
	double Ymin, Ymax, Zmin, Zmax;
	double Xmin = -0.01; // GG
	if (!fixed) {
		GetBeamFootLimits(AreaLong, Ymin, Ymax, Zmin, Zmax);
		AreaHorMin = Min(CPlate::AbsYmin, Ymin);
		AreaHorMax = Max(CPlate::AbsYmax, Ymax);
		AreaVertMin = Min(CPlate::AbsZmin, Zmin);
		AreaVertMax = Max(CPlate::AbsZmax, Zmax);
	}

	CPlate * plate;
	C3Point p0, p1, p2, p3; 
	POSITION pos = PlatesList.GetHeadPosition();
	while (pos != NULL) {
		plate = PlatesList.GetNext(pos);
		
		if (plate->Number == 0 && plate->Comment.Find("GG") >= 0 && plate->Comment.Find("Plane") >=0) {
			p0 = C3Point(Xmin, AreaHorMin, AreaVertMin);
			p1 = C3Point(Xmin, AreaHorMax, AreaVertMin);
			p2 = C3Point(Xmin, AreaHorMin, AreaVertMax);
			p3 = C3Point(Xmin, AreaHorMax, AreaVertMax);
			
			if (plate->Loaded) {
				//if (AfxMessageBox("Keep the existing load on the Target Plane?", MB_YESNO) == IDNO) {
					plate->Load->Clear(); 
					plate->Loaded = FALSE;
					plate->SetLocals(p0, p1, p2, p3);
					SetLoadArray(plate, FALSE);
				//}
			}
			else plate->SetLocals(p0, p1, p2, p3);
		}

		if (plate->Number == 1 && plate->Comment.Find("Target") >= 0 && plate->Comment.Find("Plane") >=0) {
			p0 = C3Point(AreaLong, AreaHorMin, AreaVertMin);
			p1 = C3Point(AreaLong, AreaHorMax, AreaVertMin);
			p2 = C3Point(AreaLong, AreaHorMin, AreaVertMax);
			p3 = C3Point(AreaLong, AreaHorMax, AreaVertMax);
			
			if (plate->Loaded) {
				//if (AfxMessageBox("Keep the existing load on the Target Plane?", MB_YESNO) == IDNO) {
					plate->Load->Clear(); 
					plate->Loaded = FALSE;
					plate->SetLocals(p0, p1, p2, p3);
					SetLoadArray(plate, FALSE);
				//}
			}
			else 	plate->SetLocals(p0, p1, p2, p3);
		}
		if (plate->Number == 2 && plate->Comment.Find("Right") >= 0 && plate->Comment.Find("Limit") >=0) {
			p0 = C3Point(0, AreaHorMin, AreaVertMin);
			p1 = C3Point(AreaLong, AreaHorMin, AreaVertMin);
			p2 = C3Point(0, AreaHorMin, AreaVertMax);
			p3 = C3Point(AreaLong, AreaHorMin, AreaVertMax);
			
			if (plate->Loaded) {
				//if (AfxMessageBox("Keep the existing load on the Right Limit?", MB_YESNO) == IDNO) {
					plate->Load->Clear(); 
					plate->Loaded = FALSE;
					plate->SetLocals(p0, p1, p2, p3);
					SetLoadArray(plate, FALSE);
				//}
			}
			else plate->SetLocals(p0, p1, p2, p3);
			
		}
		if (plate->Number == 3 && plate->Comment.Find("Left") >= 0 && plate->Comment.Find("Limit") >=0) {
			p0 = C3Point(0, AreaHorMax, AreaVertMin);
			p1 = C3Point(AreaLong, AreaHorMax, AreaVertMin);
			p2 = C3Point(0, AreaHorMax, AreaVertMax);
			p3 = C3Point(AreaLong, AreaHorMax, AreaVertMax);
			
			if (plate->Loaded) {
				//if (AfxMessageBox("Keep the existing load on the Left Limit?", MB_YESNO) == IDNO) {
					plate->Load->Clear(); 
					plate->Loaded = FALSE;
					plate->SetLocals(p0, p1, p2, p3);
					SetLoadArray(plate, FALSE);
				//}
			}
			else plate->SetLocals(p0, p1, p2, p3);
			
		}
		if (plate->Number == 4 && plate->Comment.Find("Bottom") >= 0 && plate->Comment.Find("Limit") >=0) {
			p0 = C3Point(0, AreaHorMin, AreaVertMin);
			p1 = C3Point(AreaLong, AreaHorMin, AreaVertMin);
			p2 = C3Point(0, AreaHorMax, AreaVertMin);
			p3 = C3Point(AreaLong, AreaHorMax, AreaVertMin);
			
			if (plate->Loaded) {
				//if (AfxMessageBox("Keep the existing load on the Bottom limit?", MB_YESNO) == IDNO) {
					plate->Load->Clear(); 
					plate->Loaded = FALSE;
					plate->SetLocals(p0, p1, p2, p3);
					SetLoadArray(plate, FALSE);
				//}
			}
			else plate->SetLocals(p0, p1, p2, p3);
			
		}
		if (plate->Number == 5 && plate->Comment.Find("Top") >= 0 && plate->Comment.Find("Limit") >=0) {
			p0 = C3Point(0, AreaHorMin, AreaVertMax);
			p1 = C3Point(AreaLong, AreaHorMin, AreaVertMax);
			p2 = C3Point(0, AreaHorMax, AreaVertMax);
			p3 = C3Point(AreaLong, AreaHorMax, AreaVertMax);
			
			if (plate->Loaded) {
				//if (AfxMessageBox("Keep the existing load on the Top Limit?", MB_YESNO) == IDNO) {
					plate->Load->Clear(); 
					plate->Loaded = FALSE;
					plate->SetLocals(p0, p1, p2, p3);
					SetLoadArray(plate, FALSE);
				//}
			}
			else plate->SetLocals(p0, p1, p2, p3);
			break;
		}
		if (plate->Number < 5) continue;
	} // pos
	CalcLimitXmax = AreaLong + 1;
}

void  CBTRDoc::SetPlates() // free
{
	//AfxMessageBox("Enter SetPlates");
	CPlate * pPlate;
	CString S;
	PlateCounter = 0;
	C3Point p0, p1, p2, p3; // in Central Channel CS
		
	while (!PlatesList.IsEmpty()) {
		delete PlatesList.RemoveHead();// delete plate (head)
	}

	Sorted_Load_Ind.RemoveAll();
	SetLoadArray(NULL, FALSE);
	LoadSelected = 0;
	SmoothDegree = 0;
	OptCombiPlot = -1; ///-1 - no load, 0 - map is calculated, >0 - selected but not calculated 
	Dia_Array.RemoveAll();
	VolumeVector.clear();
	ExitArray.RemoveAll();
	ClearSumPower();
	
	CPlate::InitAbsLimits();
	//CPlate::AbsYmin = 1000; CPlate::AbsYmax = -1000;
	//CPlate::AbsZmin = 1000; CPlate::AbsZmax = -1000;

	double Ymin, Ymax, Zmin, Zmax;
	GetBeamFootLimits(AreaLong, Ymin, Ymax, Zmin, Zmax);
	AreaHorMin = Min(AreaHorMin, Ymin);
	AreaHorMax = Max(AreaHorMax, Ymax);
	AreaVertMin = Min(AreaVertMin, Zmin);
	AreaVertMax = Max(AreaVertMax, Zmax);
	CalcLimitXmax = AreaLong + 1;

	double Xmin = -0.01;
	pPlate = new CPlate(); // Xmin
	//PlateCounter++; set GG Number = 0 
	pPlate->Number = PlateCounter; // 0
	 p0 = C3Point(Xmin, AreaHorMin, AreaVertMin);
	 p1 = C3Point(Xmin, AreaHorMax, AreaVertMin);
	 p2 = C3Point(Xmin, AreaHorMin, AreaVertMax);
	 p3 = C3Point(Xmin, AreaHorMax, AreaVertMax);
	 pPlate->OrtDirect = 1;
	 pPlate->SetLocals(p0, p1, p2, p3);
//	 pPlate->SetFromLimits(p0, p3);
	 pPlate->Solid = TRUE;
	 pPlate->Visible = FALSE;
	 pPlate->MAP = FALSE;
	 S.Format("Area GG Plane X = %g m", Xmin);
	 pPlate->Comment = S;
//	 SelectPlate(pPlate);//pPlate->Selected = TRUE; SetLoadArray(pPlate, TRUE); 
	PlatesList.AddTail(pPlate);

	pPlate = new CPlate(); // Xmax
	PlateCounter++;//1
	pPlate->Number = PlateCounter; 
	 p0 = C3Point(AreaLong, AreaHorMin, AreaVertMin);
	 p1 = C3Point(AreaLong, AreaHorMax, AreaVertMin);
	 p2 = C3Point(AreaLong, AreaHorMin, AreaVertMax);
	 p3 = C3Point(AreaLong, AreaHorMax, AreaVertMax);
	 pPlate->OrtDirect = -1;
	 pPlate->SetLocals(p0, p1, p2, p3);
//	 pPlate->SetFromLimits(p0, p3);
	 pPlate->Solid = TRUE;
	 pPlate->Visible = FALSE;
	 pPlate->MAP = TRUE;//FALSE;
	 S.Format("Area Target Plane X = %g m", AreaLong);
	 pPlate->Comment = S;
//	 SelectPlate(pPlate);//pPlate->Selected = TRUE; SetLoadArray(pPlate, TRUE); 
	PlatesList.AddTail(pPlate);

	PlasmaEmitter = pPlate->Number; // if DuctExitX < 0	// will be revised in SetPlatesNBI

	pPlate = new CPlate(); // Right (HorMin) limit
	PlateCounter++;//2
	pPlate->Number = PlateCounter; 
	 p0 = C3Point(0, AreaHorMin, AreaVertMin);
	 p1 = C3Point(AreaLong, AreaHorMin, AreaVertMin);
	 p2 = C3Point(0, AreaHorMin, AreaVertMax);
	 p3 = C3Point(AreaLong, AreaHorMin, AreaVertMax);
	 pPlate->OrtDirect = -1;
	 pPlate->SetLocals(p0, p1, p2, p3);
	// pPlate->SetFromLimits(p0, p3);
	 pPlate->Solid = TRUE;
	 pPlate->Visible = FALSE;
	 pPlate->MAP = FALSE;
	 S.Format("Right Vertical Limit Y = %g m", AreaHorMin);
	 pPlate->Comment = S;
	PlatesList.AddTail(pPlate);

	pPlate = new CPlate(); // Left (HorMax) limit
	PlateCounter++;//3
	pPlate->Number = PlateCounter; 
	 p0 = C3Point(0, AreaHorMax, AreaVertMin);
	 p1 = C3Point(AreaLong, AreaHorMax, AreaVertMin);
	 p2 = C3Point(0, AreaHorMax, AreaVertMax);
	 p3 = C3Point(AreaLong, AreaHorMax, AreaVertMax);
	 pPlate->SetLocals(p0, p1, p2, p3);
	 //pPlate->SetFromLimits(p0, p3);
	 pPlate->Solid = TRUE;
	 pPlate->Visible = FALSE;
	 pPlate->MAP = FALSE;
	 S.Format("Left Vertical Limit Y = %g m", AreaHorMax);
	 pPlate->Comment = S;
	PlatesList.AddTail(pPlate);

	pPlate = new CPlate(); // Bottom (VertMin)
	PlateCounter++;//4
	pPlate->Number = PlateCounter; 
	 p0 = C3Point(0, AreaHorMin, AreaVertMin);
	 p1 = C3Point(AreaLong, AreaHorMin, AreaVertMin);
	 p2 = C3Point(0, AreaHorMax, AreaVertMin);
	 p3 = C3Point(AreaLong, AreaHorMax, AreaVertMin);
	 pPlate->SetLocals(p0, p1, p2, p3);
	 //pPlate->SetFromLimits(p0, p3);
	 pPlate->Solid = TRUE;
	 pPlate->Visible = FALSE;
	 pPlate->MAP = FALSE;
	 S.Format("Bottom Horizontal Limit Z = %g m", AreaVertMin);
	 pPlate->Fixed = 1; // side view
	 pPlate->Comment = S;
	PlatesList.AddTail(pPlate); 

	pPlate = new CPlate();// Top (VertMax)
	PlateCounter++;//5
	pPlate->Number = PlateCounter; 
	 p0 = C3Point(0, AreaHorMin, AreaVertMax);
	 p1 = C3Point(AreaLong, AreaHorMin, AreaVertMax);
	 p2 = C3Point(0, AreaHorMax, AreaVertMax);
	 p3 = C3Point(AreaLong, AreaHorMax, AreaVertMax);
	 pPlate->OrtDirect = -1;
	 pPlate->SetLocals(p0, p1, p2, p3);
	 //pPlate->SetFromLimits(p0, p3);
	 pPlate->Solid = TRUE;
	 pPlate->Visible = FALSE;
	 pPlate->MAP = FALSE;
	 S.Format("Top Horizontal Limit Z = %g m", AreaVertMax);
	 pPlate->Fixed = 1; // side view
	 pPlate->Comment = S;
	PlatesList.AddTail(pPlate);

	
	if (!OptFree) {
		SetPlatesNBI(); // standard config
		/*AreaHorMin = Min(CPlate::AbsYmin, AreaHorMin);
		AreaHorMax = Max(CPlate::AbsYmax, AreaHorMax);
		AreaVertMin = Min(CPlate::AbsZmin, AreaVertMin);
		AreaVertMax = Max(CPlate::AbsZmax, AreaVertMax);*/
		ModifyArea(TRUE);// beam is fixed = true, ignore beamfoot change
	} // adjust area limits

	
	int add = AddPlatesList.GetSize();
	S.Format(" AddPlates List size - %d, AddPlatesNumber - %d\n", add,  AddPlatesNumber);
	//std::cout << S;

	if (add > 0 && AddPlatesNumber > 0)
		AppendAddPlates();
		//PlateCounter+= AddPlatesNumber;
		//S.Format("added %d, total %d", AddPlatesNumber, PlateCounter);
	

	//ModifyArea(TRUE);// beam is fixed = true, ignore beamfoot change
	pMarkedPlate = &emptyPlate;// NULL;
	//AfxMessageBox("Exit SetPlates");
	//S.Format("SetPlates: %d TOTAL COUNT ", PlateCounter);
	//AfxMessageBox(S);
	
}

void  CBTRDoc::SetPlatesNBI() // NBI config
{
	//AfxMessageBox("Enter SetPlatesNBI");
	CWaitCursor wait;
	CString S;
	C3Point p0_, p1_, p2_, p3_; // in local CS of Side Channel
	C3Point p0, p1, p2, p3; // in Central Channel CS
	CPlate * pPlate;
	
	/*while (!PlatesList.IsEmpty()) {
		delete PlatesList.RemoveHead();// delete plate (head)
	}*/
	
	SetVShifts();

//	pMarkedPlate = NULL;

	//SetLoadArray(NULL, FALSE);
	
//	AdjustAreaLimits();
	/* RECT_DIA diaphragm;
	diaphragm.Corn[0] = C3Point(0.001, AreaHorMin, AreaVertMin);
	diaphragm.Corn[1] = C3Point(0.001, AreaHorMax, AreaVertMin);
	diaphragm.Corn[2] = C3Point(0.001, AreaHorMax, AreaVertMax);
	diaphragm.Corn[3] = C3Point(0.001, AreaHorMin, AreaVertMax);
	diaphragm.Number = 0;
	Dia_Array.Add(diaphragm);*/

	SetPlatesNeutraliser();
	SetPlatesRID();
	

// ----------Calorimeter -------------------------------------------
//	!OptCalOpen ? CalOutW = 0.0 : CalOutW = CalInW;
	if (!OptCalOpen) CalOutW = 0.0;
	
	pPlate = new CPlate(); // left
	PlateCounter++;
	pPlate->Number = PlateCounter; 
	 p0 = C3Point(CalInX,  CalInW * 0.5, -CalH * 0.5);
	 p1 = C3Point(CalOutX, CalOutW * 0.5, -CalH * 0.5);
	 p2 = C3Point(CalInX,  CalInW * 0.5, CalH * 0.5);
	 p3 = C3Point(CalOutX, CalOutW * 0.5, CalH * 0.5);
	pPlate->SetLocals(p0, p1, p2, p3);
	pPlate->Shift(0,0, VShiftCal);
	pPlate->Comment = "Calorimeter Left Wall";
	PlatesList.AddTail(pPlate);
//	if (TaskReionization)  SelectPlate(pPlate);


	pPlate = new CPlate(); // right
	PlateCounter++;
	pPlate->Number = PlateCounter; 
	 p0 = C3Point(CalInX,  -CalInW * 0.5, -CalH * 0.5);
	 p1 = C3Point(CalOutX, -CalOutW * 0.5, -CalH * 0.5);
	 p2 = C3Point(CalInX,  -CalInW * 0.5, CalH * 0.5);
	 p3 = C3Point(CalOutX, -CalOutW * 0.5, CalH * 0.5);	
	 pPlate->OrtDirect = -1;
	pPlate->SetLocals(p0, p1, p2, p3);
	pPlate->Shift(0,0, VShiftCal);
	pPlate->Comment = "Calorimeter Right Wall";
	PlatesList.AddTail(pPlate);
//	if (TaskReionization)  SelectPlate(pPlate);

	pPlate = new CPlate(); // Exit 
	//PlateCounter++;  pPlate->Number = PlateCounter; // -> in AddCond
	 //p0 = C3Point(CalOutX + 0.1, -CalOutW * 0.5 - 0.15, -CalH * 0.5 - 0.1);
	 //p3 = C3Point(CalOutX + 0.1, CalOutW * 0.5 + 0.15,  CalH * 0.5 + 0.1);
	 p0 = C3Point(CalOutX + 0.1, AreaHorMin, AreaVertMin);
	 p3 = C3Point(CalOutX + 0.1, AreaHorMax, AreaVertMax);
	 pPlate->OrtDirect = -1;
	 pPlate->SetFromLimits(p0, p3); 
	 //pPlate->Shift(0, RIDBiasOutHor, VShiftRID + RIDBiasOutVert);
	 pPlate->Shift(0,0, VShiftCal);
	 pPlate->Solid = FALSE;
	 pPlate->Visible = FALSE;
	 pPlate->MAP = TRUE;
	 S.Format("Calorimeter Exit plane X = %g m", CalOutX + 0.2);
	 pPlate->Comment = S;
	 AddCond(pPlate);// PlatesList.AddTail(pPlate);
//	 if (TaskRID)  SelectPlate(pPlate);

	 //PlasmaEmitter = pPlate->Number; // for next tracing (in Plasma)

	 //----------------DUCT ------------------------------------

	 if (OptAddDuct == TRUE) SetPlatesDuctFull();


//  ------------- PLASMA ------------------------------------
	 SetPlasmaGeom(); //if (!FWdataLoaded) 	InitFWarray();

	// Injection and Movable Target Planes ---set if size > 0 and within area
	double Ymin, Ymax, Zmin, Zmax;
	double shiftY, shiftZ;
	double dY, dZ, size;//size
	C3Point P;
	double eps = 1.e-4;
	 if (AreaLong > TorCentreX) {
		 if (DuctExit > 0) PlasmaEmitter = DuctExit;
		 else { // create new PlasmaEmitter
			double Rmax = PlasmaMajorR + PlasmaMinorR;
			double dx = sqrt(Rmax * Rmax - TorCentreY * TorCentreY);
			double Xinj = TorCentreX - dx;
			//P = GetBeamFootLimits(Xinj, Ymin, Ymax, Zmin, Zmax);
			//dY = 0.15; dZ = 0.1;
			//Ymin = Ymin - dY; Ymax = Ymax + dY;
			//Zmin = Zmin - dZ; Zmax = Zmax + dZ;
			pPlate = new CPlate();
			//PlateCounter++;	pPlate->Number = PlateCounter;
			p0 = C3Point(Xinj-0.1, AreaHorMin, AreaVertMin);
			p3 = C3Point(Xinj-0.1, AreaHorMax, AreaVertMax);
			pPlate->OrtDirect = -1;
			pPlate->SetFromLimits(p0, p3); 
			pPlate->Solid = FALSE;
			pPlate->Visible = FALSE;
			S.Format("Plasma Emitter X = %g m", Xinj);
			pPlate->Comment = S;
			// pPlate->Fixed = 1; // side view
			AddCond(pPlate);// PlatesList.AddTail(pPlate);
			PlasmaEmitter = pPlate->Number;
		 }
	 } // AreaLong > TorCentreX
	 //else (AreaLong < TorCentreX) - PlasmaEmitter = Area Limit [#1]
	
	//if (!OptBeamInPlasma) return;

	dY = MovHor; dZ = MovVert;
	size = fabs(dY*dZ);
	if (size > eps && MovX < AreaLong - 0.1) {
		pPlate = new CPlate();
		PlateCounter++;
		pPlate->Number = PlateCounter;
		p0 = C3Point(MovX, -MovHor, -MovVert);
		p1 = C3Point(MovX, MovHor, -MovVert);
		p2 = C3Point(MovX, -MovHor, MovVert);
		p3 = C3Point(MovX, MovHor, MovVert);
		pPlate->OrtDirect = -1;
		pPlate->SetLocals(p0, p1, p2, p3);
		// pPlate->Shift(0, 0, MovShiftVert);
		P = GetBeamFootLimits(MovX, Ymin, Ymax, Zmin, Zmax);
		shiftY = P.Y; //0.5 * (Ymin + Ymax);
		shiftZ = P.Z; //0.5 * (Zmin + Zmax);
		pPlate->Shift(0, shiftY, shiftZ);
		pPlate->Solid = FALSE;
		pPlate->Visible = FALSE;
		S.Format("Mov1 Target plane X = %g m", MovX);
		pPlate->Comment = S;
		// pPlate->Fixed = 1; // side view
		PlatesList.AddTail(pPlate);

		//PlasmaEmitter = pPlate->Number; // for next tracing (in Plasma)
	}

	dY = Mov2Hor; dZ = Mov2Vert;
	size = fabs(dY*dZ);
	
	if (size > eps && Mov2X < AreaLong - 0.1) {
		pPlate = new CPlate();
		PlateCounter++;
		pPlate->Number = PlateCounter;
		p0 = C3Point(Mov2X, -Mov2Hor, -Mov2Vert);
		p1 = C3Point(Mov2X, Mov2Hor, -Mov2Vert);
		p2 = C3Point(Mov2X, -Mov2Hor, Mov2Vert);
		p3 = C3Point(Mov2X, Mov2Hor, Mov2Vert);
		pPlate->OrtDirect = -1;
		pPlate->SetLocals(p0, p1, p2, p3);
		// pPlate->Shift(0, 0, Mov2ShiftVert);
		P = GetBeamFootLimits(Mov2X, Ymin, Ymax, Zmin, Zmax);
		shiftY = P.Y; //0.5 * (Ymin + Ymax);
		shiftZ = P.Z;//0.5 * (Zmin + Zmax);
		pPlate->Shift(0, shiftY, shiftZ);
		pPlate->Solid = FALSE;
		pPlate->Visible = FALSE;
		S.Format("Mov2 Target plane X = %g m", Mov2X);
		pPlate->Comment = S;
		// pPlate->Fixed = 1; // side view
		PlatesList.AddTail(pPlate);
	}

	dY = Mov3Hor; dZ = Mov3Vert;
	size = fabs(dY*dZ);
	
	if (size > eps && Mov3X < AreaLong - 0.1) {
		pPlate = new CPlate();
		PlateCounter++;
		pPlate->Number = PlateCounter;
		p0 = C3Point(Mov3X, -Mov3Hor, -Mov3Vert);
		p1 = C3Point(Mov3X, Mov3Hor, -Mov3Vert);
		p2 = C3Point(Mov3X, -Mov3Hor, Mov3Vert);
		p3 = C3Point(Mov3X, Mov3Hor, Mov3Vert);
		pPlate->OrtDirect = -1;
		pPlate->SetLocals(p0, p1, p2, p3);
		//pPlate->Shift(0, 0, Mov3ShiftVert);
		P = GetBeamFootLimits(Mov3X, Ymin, Ymax, Zmin, Zmax);
		shiftY = P.Y; //0.5 * (Ymin + Ymax);
		shiftZ = P.Z; //0.5 * (Zmin + Zmax);
		pPlate->Shift(0, shiftY, shiftZ);
		pPlate->Solid = FALSE;
		pPlate->Visible = FALSE;
		S.Format("Mov3 Target plane X = %g m", Mov3X);
		pPlate->Comment = S;
		//pPlate->Fixed = 1; // side view
		PlatesList.AddTail(pPlate);
	}

	dY = Mov4Hor; dZ = Mov4Vert;
	size = fabs(dY*dZ);
	
	if (size > eps && MovX < AreaLong - 0.1) {
		pPlate = new CPlate();
		PlateCounter++;
		pPlate->Number = PlateCounter;
		p0 = C3Point(Mov4X, -Mov4Hor, -Mov4Vert);
		p1 = C3Point(Mov4X, Mov4Hor, -Mov4Vert);
		p2 = C3Point(Mov4X, -Mov4Hor, Mov4Vert);
		p3 = C3Point(Mov4X, Mov4Hor, Mov4Vert);
		pPlate->OrtDirect = -1;
		pPlate->SetLocals(p0, p1, p2, p3);
		//pPlate->Shift(0, 0, Mov4ShiftVert);
		P = GetBeamFootLimits(Mov4X, Ymin, Ymax, Zmin, Zmax);
		shiftY = P.Y; //0.5 * (Ymin + Ymax);
		shiftZ = P.Z; //0.5 * (Zmin + Zmax);
		pPlate->Shift(0, shiftY, shiftZ);
		pPlate->Solid = FALSE;
		pPlate->Visible = FALSE;
		S.Format("Mov4 Target plane X = %g m", Mov4X);
		pPlate->Comment = S;
		// pPlate->Fixed = 1; // side view
		PlatesList.AddTail(pPlate);
	}

	if (PlasmaXmax - PlasmaXmin < eps) return; // beam misses plasma

	double Y0min, Y0max, Z0min, Z0max, Y1min, Y1max, Z1min, Z1max;
	C3Point P0, P1;
	P0 = GetBeamFootLimits(PlasmaXmin-1, Y0min, Y0max, Z0min, Z0max);
	P1 = GetBeamFootLimits(PlasmaXmax+1, Y1min, Y1max, Z1min, Z1max);
	Ymin = Min(Y0min, Y1min); Ymax = Max(Y0max, Y1max);
	Zmin = Min(Z0min, Z1min); Zmax = Max(Z0max, Z1max);

	/*p0 = C3Point(PlasmaXmin-1, Ymin - 0.05, 0.); //AreaHorMin
	p1 = C3Point(PlasmaXmax+1, Ymin - 0.05, 0.); //AreaHorMin
	p2 = C3Point(PlasmaXmin-1, Ymax + 0.05, 0.); //AreaHorMax
	p3 = C3Point(PlasmaXmax+1, Ymax + 0.05, 0.); //AreaHorMax*/
	/*
	// temporary removed
	p0 = C3Point(0, AreaHorMin, 0.); //AreaHorMin
	p1 = C3Point(AreaLong, AreaHorMin, 0.); //AreaHorMin
	p2 = C3Point(0, AreaHorMax, 0.); //AreaHorMax
	p3 = C3Point(AreaLong, AreaHorMax, 0.); 
	pBeamHorPlane = new CPlate(p0, p1, p2, p3); // new - total NBL plane
	pBeamHorPlane->Number = 3000;
	pBeamHorPlane->Solid = FALSE;
	pBeamHorPlane->Comment = "NB - Horizontal plane";
	//pBeamHorPlane->Comment = "NB in plasma - Horizontal plane";Number = 2000;
	pBeamHorPlane->ApplyLoad(TRUE, 10, 1);// 0.2, 0.02); //-> OFF
	*/

	/*p0 = C3Point(PlasmaXmin-1, 0., Zmin - 0.05); //AreaVertMin);
	p1 = C3Point(PlasmaXmax-1, 0., Zmin - 0.05); //AreaVertMin);
	p2 = C3Point(PlasmaXmin+1, 0., Zmax + 0.05); //AreaVertMax);
	p3 = C3Point(PlasmaXmax+1, 0., Zmax + 0.05); //AreaVertMax);*/
	
	/* // temporary removed
	p0 = C3Point(0, 0, AreaVertMin); //AreaVertMin);
	p1 = C3Point(AreaLong, 0, AreaVertMin); //AreaVertMin);
	p2 = C3Point(0, 0, AreaVertMax); //AreaVertMax);
	p3 = C3Point(AreaLong, 0, AreaVertMax); //AreaVertMax);
	pBeamVertPlane = new CPlate(p0, p1, p2, p3);
	pBeamVertPlane->Number = 4000;
	pBeamVertPlane->Solid = FALSE;
	pBeamVertPlane->Comment = "NB - Vertical plane";
	//pBeamVertPlane->Comment = "NB in plasma - Vertical plane";Number = 2000;
	pBeamVertPlane->ApplyLoad(TRUE, 0.2, 0.02);
	*/

	if (OptBeamInPlasma && AreaLong >= TorCentreX) 
		SetPlatesTor();
	//AppendAddPlates();
	//AfxMessageBox("Exit SetPlatesNBI");
}

bool CBTRDoc::AddCond(CPlate * plate)// add to PlateList if condition
{
	if (plate->Errors() > 0) {
		AfxMessageBox("Invalid Surf detected");  return FALSE;
	}
	//if (FindPlateClones(plate) > 0) return; //found clones in the main PlatesList
	
	CString comm = plate->Comment;
	//skip posX < -0.5 // 1st condition
	for (int i = 0; i < 4; i++) { // any corner!
		C3Point corn = plate->Corn[i];
		if (corn.X < -0.5 || corn.X > AreaLong) { 
			logout << comm << " - not added (out of AREA) \n";
			return FALSE; // skip adding
		}
	}

	//plate->MAP = TRUE;// default - interesting
	
	CString S = comm.MakeUpper();
	CString Sskip;
	CString Sexcept;
	int Nskip = SkipSurfClass.GetSize();
	int Nexc = ExceptSurf.GetSize();
	if (Nskip > 0) {
		for (int i = 0; i < Nskip; i++) {
			Sskip = SkipSurfClass[i];
			if (S.Find(Sskip,0) >= 0) { // found Skipped
				
				if (Nexc == 0) { // no exceptions
					plate->MAP = FALSE;
					//logout << S << " - MAP SKIP \n";
				} // No exceptions
				
				// check Exceptions 
				bool found = FALSE;
				for (int j = 0; j < Nexc; j++) {
					Sexcept = ExceptSurf[j];
					if (S.Find(Sexcept,0) >=0) 
						found = TRUE;//
				} // j = Nexc

				if (!found) { // not found in exceptions
					plate->MAP = FALSE;
					//logout << S << " - MAP SKIPPED \n";
				} // Sexcept not found 
		
			} // Sskip found 
		} // Nskip
	} // Nskip >0

	PlateCounter++; //OK
	plate->Number = PlateCounter;
	PlatesList.AddTail(plate); 
	if (!(plate->MAP))
		logout << comm << " - MAP SKIPPED \n";

	return TRUE;// added
}

void  CBTRDoc::AddReflectors(CString text)
{
	ReflectorNums.RemoveAll();
	//ReflectorNums.Add(PlasmaEmitter);
	
	CString s = text;
	CString S, Sn, Comm;
	bool found = true; // comma
	int pos; // comma pos
	int nval;// surf n
	int count = 0;
	CPlate * plate;
	
	//CString valstr = Line.Mid(pos + 1);
	//double Value = atof(valstr);

	while (found) {
		pos = s.Find(",", 0);
		if (pos > 0) {
			found = true;
			Sn = s.Left(pos);
			nval = (int)(atof(Sn));
			ReflectorNums.Add(nval);
			count++;
			logout << "--- added Surf " << nval << "  to Reflectors\n";
			s = s.Mid(pos + 1);
		}
		else break;
	}

	logout << " >>> total Reflectors - " << count << "\n";
	S = "If KEEP FALLS is ON, data will be kept on \n";
	for (int i = 0; i < count; i++) {
		nval = ReflectorNums[i];
		Sn.Format(" #%d - ", nval);
		plate = GetPlateByNumber(nval);
		Comm = plate->Comment;
		S += Sn + Comm + "\n";
	}
	AfxMessageBox(S);
	logout << S << "\n";

	if (OptKeepFalls == TRUE)
		S = " - The falls will be kept!";
	else
		S = " - The falls will be NOT kept";
	//AfxMessageBox(S);
	//logout << S << "\n";
}

void  CBTRDoc::InitAddPlates() // remove all
{
	//AfxMessageBox("InitAddPlates");
	while (!AddPlatesList.IsEmpty()) {
		delete AddPlatesList.RemoveHead();// delete plate (head)
	}
	AddPlatesNumber = 0;//900;
}

CPlate* CBTRDoc::AddPlate(bool isSolid, C3Point p0, C3Point p1, C3Point p2, C3Point p3)
{// not used now
	CPlate * pPlate = new CPlate(); // new Plate
	 pPlate->SetArbitrary(p0, p1, p2, p3);
	// pPlate->SetFromLimits(p0, p3);
	 pPlate->Solid = isSolid;
	 pPlate->Visible = isSolid;
	 if (isSolid) pPlate->Comment = "Solid surface"; 
	 else pPlate->Comment = "Transparent surface"; 
	
	// AddPlate(pPlate);
	 if (FindPlateClones(pPlate) > 0) //found clones in the main PlatesList
		 return NULL;
	 AddPlatesList.AddTail(pPlate);
	 AddPlatesNumber++;
	 return (pPlate);
}

void CBTRDoc::AddPlate(CPlate * plate)
{
	if (plate->Errors() > 0) {
		CString S;
		S.Format("Invalid %d Surf Corners\n (%s)\n", plate->Number, plate->Comment);
		AfxMessageBox(S); 
		logout << S;
		return;
	}

	if (FindPlateClones(plate) > 0) { //found clones in the main PlatesList
		//std::cout << " Found plate clone " << std::endl; 
		return;
	}
	AddPlatesList.AddTail(plate);
	AddPlatesNumber++;
}

int CBTRDoc::FindPlateClones(CPlate * plate) // scan PlatesList 
{
	int n = 0; // no clones
	POSITION pos = PlatesList.GetHeadPosition();
	CPlate * plate0;
	
	while (pos != NULL) {
		plate0 = PlatesList.GetNext(pos);
		if (plate0->CommonCorn(plate) > 2) // at least 3 corners common
			n++; // found clone
	}
		
	return n;
}

void CBTRDoc::AppendAddPlates() // append free surf to the main PlatesList
// called in SetPlates - after SetPlatesNBI
{
	CPlate * plate;
	CPlate * pPlate;

	int add = AddPlatesList.GetSize();
	if (add < 1) {
		logout << "-- empty AddPlatesList - not added\n";
		return;
	}

	POSITION pos = AddPlatesList.GetHeadPosition();
	C3Point p0, p1, p2, p3;
	CString S;
	
	S.Format("Adding %d free surf...\n to %d existing\n", add, PlateCounter);// PlateCounter - total plates
	//AfxMessageBox(S);
	logout << S;
	
	int clones = 0; // found, not added
	//moved from ReadAddPlates:
	//PlateCounter++;
	//plate->Number = PlateCounter;//AddPlatesNumber;
	//PlatesList.AddTail(plate);
	
	while (pos != NULL) {
		plate = AddPlatesList.GetNext(pos);
		if (FindPlateClones(plate) > 0) {
			clones++;
			continue;
		}
		pPlate = new CPlate(); // new Plate - copy
		//PlateCounter++;  pPlate->Number = PlateCounter; // -> in AddCond
		p0 = C3Point(plate->Corn[0]);
		p1 = C3Point(plate->Corn[1]);
		p2 = C3Point(plate->Corn[3]);
		p3 = C3Point(plate->Corn[2]);
		pPlate->SetLocals(p0, p1, p2, p3);
		
		pPlate->Solid = plate->Solid;
		pPlate->Visible = pPlate->Solid;
		pPlate->Fixed = -1;// plate->Fixed;// 0 - plan, 1 - side
		pPlate->Comment = plate->Comment;
		AddCond(pPlate); //PlatesList.AddTail(pPlate);
	}
	if (clones > 0) {
		S.Format("%d surf clones found (not added)\n", clones);
		//AfxMessageBox(S);
		logout << S;
	}
	S.Format("%d surf total in the PlatesList\n", PlateCounter);
	AfxMessageBox(S);
	logout << S;

	//CheckClosePlates();
		
}

void CBTRDoc::CheckClosePlates()
{
	CString S;
	CPlate * plate0; // fixed
	CPlate * plate;

	double minstep = max(TraceStepL, IonStepL);
	minstep = max(minstep, ReionStepL);
	
	double dist, mindist = minstep; // 0.1;
	double tol = 1; // tolerance
	int n = 0; // count close pairs
	POSITION pos0 = PlatesList.GetHeadPosition();// iterate base plate
	POSITION pos = pos0;// PlatesList.GetHeadPosition(); // iterate plates to compare with base
	int solid0, solid;

	while (pos0 != NULL) { // shift base
		plate0 = PlatesList.GetNext(pos0);
		solid0 = plate0->Solid;
		pos = pos0;
		if (pos != NULL) plate = PlatesList.GetNext(pos); // start for plate
		while (pos != NULL) {// scan to end
			plate = PlatesList.GetNext(pos);
			solid = plate->Solid;
			dist = plate0->DistCorn(plate, tol);
			
			if (solid0*solid == 0 && dist < mindist) { // one of them is transparent - attention!
				S.Format("Surfaces %d and %d are closer than %g", plate0->Number, plate->Number, mindist);
				//AfxMessageBox(S);
				n++;
			}
		} // pos
	} //pos0
	if (n > 0) {
		S.Format("found %d transp surfs closer than trace step!\n  TraceStepL = %g IonStepL = %g ReionStepL = %g",
				n, TraceStepL, IonStepL, ReionStepL);
		AfxMessageBox(S);
	}
}

void CBTRDoc:: SelectAllPlates()
{
	CPlate * plate;
	POSITION pos = PlatesList.GetHeadPosition();

	if (LoadSelected > 0) { // list not empty -> unselect all delete loads
		while (pos != NULL) {
			plate = PlatesList.GetNext(pos);
			if (plate->Loaded) {
				//plate->Load->Clear();
				delete plate->Load;
				if (plate->SmLoad != NULL) delete plate->SmLoad;
				plate->AngularProfile.RemoveAll();
				plate->Loaded = FALSE;
				plate->Selected = FALSE;
				plate->Touched = FALSE;
				//SelectPlate(plate); // -
			}// loaded
		}
	SetLoadArray(NULL, FALSE);//+
	//InitAddPlates();
	
	}
	
	else { // empty -> select all
		while (pos != NULL) {
		plate = PlatesList.GetNext(pos);
		SelectPlate(plate);// make selected = true
		}
	
	}
	pLoadView->ShowLoad = FALSE;
	pMainView->InvalidateRect(NULL, TRUE);
	ShowStatus();

}
CPlate * CBTRDoc::GetPlateByNumber(int N)
{
	CPlate * plate;
	POSITION pos = PlatesList.GetHeadPosition();
	while (pos != NULL) {
		plate = PlatesList.GetNext(pos);
		if (plate->Number == N) return plate;
	}
	return NULL;
}

void CBTRDoc:: ClearAllPlates() // clear loads
{
	CPlate * plate;
	POSITION pos = PlatesList.GetHeadPosition();
	while (pos != NULL) {
		plate = PlatesList.GetNext(pos);
		plate->Touched = FALSE;
		if (plate->Loaded) {
			//plate->Load->Clear();
			delete (plate->Load); //->Clear(); 
			if (plate->SmLoad != NULL) delete plate->SmLoad;
		}
		plate->Loaded = FALSE;
		plate->Selected = FALSE;
		plate->Load = NULL;
		plate->AtomPower = 0;
		plate->NegPower = 0;
		plate->PosPower = 0;
		plate->Falls.RemoveAll();
	}
	// temporary removed
	//pBeamHorPlane->Load->Clear(); // created in SetPlatesNBI
	//pBeamVertPlane->Load->Clear();// created in SetPlatesNBI
	
	TracksCalculated = 0;//map tracks on vert/horiz planes in plasma -> CalculateTracks

	ClearSumPower();
	SetLoadArray(NULL, FALSE);
	Load_Arr.RemoveAll();
	LoadSelected = 0;
	pLoadView->ShowLoad = FALSE;
	OptCombiPlot = -1; //-1 - no load, 0 - map is calculated, >0 - selected but not calculated 
	ArrSize = 0;
	pMainView->InvalidateRect(NULL, TRUE);

	//ShowNothing();
}


void CBTRDoc::ReadPressure()
{
	//ReadGas();
	PressureLoaded = TRUE;
}

int CBTRDoc:: ReadMF_4col(char * name) // x bx by bz
{
	int lines = BField->ReadData(name);
	MagFieldDim = 1;
	return lines;
}

int CBTRDoc:: ReadMF_7col(char * name) // x  y  z  bx by bz babs
{
	int lines = BField3D->ReadData(name);
	MagFieldDim = 3;
	return lines;
}

int CBTRDoc:: ReadGas_2col(char * name, int add)
{
	int lines;
	if (add != 0) { 
		lines = AddGField->ReadData(name); 
		return lines; 
	} // addit profile

	lines = GField->ReadData(name);
	PressureDim = 1;
	return lines;
}

int CBTRDoc:: ReadGas_3col(char * name)
{
	int lines = GFieldXZ->ReadData(name);
	PressureDim = 2;
	return lines;
} 

void CBTRDoc::ReadRIDfield()
{
	char name[1024];
	CFileDialog dlg(TRUE, "dat; txt | * ", "", OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"E-field data (*.txt);(*.dat)  | *.txt; *.TXT; *.dat; *.DAT | All Files (*.*) | *.*||", NULL);
	
	if (dlg.DoModal() == IDOK) {
			strcpy(name, dlg.GetPathName());
		
		/*	CPreviewDlg pdlg;
			pdlg.m_Filename = name;
			if (pdlg.DoModal() == IDCANCEL) {
				if (!RIDFieldLoaded) // not loaded before
					AfxMessageBox("No Field accepted from file\n default flat model is applied",
						MB_ICONINFORMATION | MB_OK);
				return;
			}
		*/
			RIDFieldLoaded = FALSE;
			int res = RIDField->ReadFullField(name);
			if (res < 1)	{
				AfxMessageBox("Invalid format",	MB_ICONSTOP | MB_OK); 
			}
			else RIDFieldLoaded = TRUE;

			return;
	} // IDOK
	
	else {
		if (!RIDFieldLoaded) // not loaded before
		AfxMessageBox("No Field accepted from file\n default flat model is applied", 
			MB_ICONINFORMATION | MB_OK);
		return;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CBTRDoc serialization

void CBTRDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CBTRDoc diagnostics

#ifdef _DEBUG
void CBTRDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CBTRDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CBTRDoc commands

void CBTRDoc::OnDataActive() 
{
	DataLoaded = FALSE;
 	pDataView->OnDataActive(); // call FormDataText
//	SetTitle(TaskName);
 }

void CBTRDoc::OnDataGet_void()
{
	OnDataGet();
	OnShow();
}

int CBTRDoc::OnDataGet() // load config; 
						 // ReadData, UpdateDataArray, SetBeam, InitPlasma, SetPlates, SetStatus, SetPolarBeamlets 
{
	STOP = TRUE;
	OnStop();
//	OptParallel = FALSE;
	//OptFree = FALSE; // by default read standard NBI geom
	std::cout << "Waiting for config file...\n";// LOG is not open!!
	DataLoaded = FALSE;
	CWaitCursor wait; 
	CString S;
	
	// default settings (used when reading old data)
	//OptDeuterium = TRUE;
	OptIndBeamlets = FALSE;
	OptThickNeutralization = FALSE;//default
	BeamSplitType = 0;
	VertInclin = 0;
	//OptIonPower = FALSE;
	OptNegIonPower = TRUE;
	OptPosIonPower = TRUE;
	OptAtomPower = TRUE;
	/// end ////////////
		
	int nread = ReadData(); // Read text-> to m_Text + Set Current Dir!!! 
						// open LOG
	

	if (nread == 0) { // file -> m_Text 
		S.Format("input not read \n");
		AfxMessageBox(S); 
		logout << S;// << std::endl;
		return 0; 
	} 
	else {
		S.Format(" - data read from file %s\n", ConfigFileName);
		logout << S;// << std::endl;
		//logout << endl;
		//std::cout << m_Text << std::endl;
	}

	if (m_Text.Find("NBI",0) > 0 
		&& m_Text.Find("CONFIG",0) > 0 
		&& m_Text.Find("Dia",0) > 0) OptFree = FALSE;
	
	/*else { // SWITCHED-OFF temporarily
		OptFree = TRUE;
		AfxMessageBox("Free Surf option is ON");
	}*/

	//InitData(); // calls FormDataText!!! InitOptions, InitFields, InitAddPlates()
	
	ActiveCh.SetSize(10);
	ActiveRow.SetSize(10);
	//CString S = m_Text;
	//InitFields(); // empty MF + GAS called from InitOptions
	InitOptions(); //MAXSCEN = 1  empty AddSurfName, Set default tracing options
	InitTrackArrays();// clear 
	
	UpdateDataArray(); // m_Text -> SetData SetOption CheckData 
	TaskName += "\n -> " + GetTitle();

/*	pDataView->m_rich.SetWindowText(m_Text);
	pDataView->m_rich.SetModify(FALSE);
*/	
	if (MaxDataRead <2) { 
		AfxMessageBox("Problem with data format\n Without internal names?", MB_ICONSTOP | MB_OK);
		DataLoaded = FALSE;
	}
	
	if (!DataLoaded) {
		AfxMessageBox("Configuration is not changed!", MB_ICONSTOP | MB_OK); 
		//pDataView->OnDataActive();
		return(0); // not a Configuration File
	}

//	RIDField->Set();
	SetFields(); // GAS + MF from files if found in Config
	CheckData();
	
	SetBeam(); // Aimings, IS positions
	InitPlasma(); // called in OnDataGet
	//SetPlasma();
	//SetDecayInPlasma();
	//	PlasmaMajorR = 6.2; // default tor R
	//	PlasmaMinorR = 2.2; // default tor r
	//  PlasmaMajorR = (FWRmin + FWRmax) * 0.5;
	//  PlasmaMinorR = (FWRmax - FWRmin) * 0.5;
/*	if (TorCentreX > 25) { //ITER
		FWRmin = 4.0; FWRmax = 8.4;//ITER
	}
	else {//TIN
		FWRmin = 2.0; FWRmax = 4.4;
	}
	*/
	InitAddPlates(); // Remove AddPlatesList, set AddPlatesNumber = 0;
	SetPlates();//SetPlasma is called from SetPlatesNBI
	SetAddPlates();//ReadAddPlates(AddSurfName);
	if (AddPlatesNumber > 0)	AppendAddPlates();
	SetStatus();
	
	if (NofActiveChannels < 1) {
		for (int k=0; k < (int)NofChannelsHor; k++) ActiveCh[k] = TRUE;
		SetStatus();
	}
	if (NofActiveRows < 1) {
		for (int n=0; n < (int)NofChannelsVert; n++) ActiveRow[n] = TRUE;
		SetStatus();
	}

	//if (OptDeuterium) TracePartType = -2;
	
	SetTraceParticle(TracePartType); // - already set in SetOption!()
	SetNeutrCurrents();

	if (OptSINGAP)//	SetSINGAP(); // form BEAMLET_ENTRY array (apply midalign, active chan)
		OnPdpBeamlets(); //	ReadSINGAPbeam , SetStatus();

	if ((int)BeamSplitType ==0) SetPolarBeamletCommon(); //Polar // SetBeamletAt(0);
	else SetGaussBeamlet(BeamCoreDivY, BeamCoreDivZ); // Cartesian
	std::cout << "SetPolarBeamletCommon Polar - " << PolarNumber << " Azim -" << AzimNumber << std::endl;
	SetPolarBeamletReion();
	std::cout << "SetPolarBeamletReion Polar - " << PolarNumber << " Azim -" << AzimNumber << std::endl;
	SetPolarBeamletResidual();
	std::cout << "SetPolarBeamletResid Polar - " << PolarNumber << " Azim -" << AzimNumber << std::endl;

	SetReionPercent();
	
	Progress = 0;
	ScenData[0].SetAt(1, "CONFIG = " + CurrentDirName + "\\ " + ConfigFileName + "\n");
				//InitAddPlates();
	OnShow();
	return 1;
}

void CBTRDoc::ResumeData()
{
	
	logout << "\n - Resuming data from " << CurrentDirName << " \\ " << ConfigFileName << "\n";

	FILE * fin;
	char buf[1024];
	fin = fopen(ConfigFileName, "r");		//fprintf(fout, m_Text);
	DWORD error = ::GetLastError();
	m_Text = "";
	while (!feof(fin)) {
		fgets(buf, 1024, fin);
		if (!feof(fin))	m_Text += buf;
	}
	fclose(fin);
	UpdateDataArray(); // SetData, SetOptions // CheckData
	
	SetFields(); // if found MF + GAS
	CheckData();

	//FormDataText();
	logout << " + config data resumed\n";
	//std::cout << m_Text << std::endl;

}

void CBTRDoc::OnDataImport_PDP_SINGAP() 
{
	if (!STOP) {
		AfxMessageBox("Stop calculations before reading new data! \n (Red cross on the Toolbar)");
		return;
	}
	
	int res;
	char name[1024];
//	char buf[1024];
	CFileDialog dlg(TRUE, "txt; bak | * ", "", OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"PDP INPUT file (*.txt); (*.bak) | *.txt; *.TXT; *.bak; *.BAK | All Files (*.*) | *.*||", NULL);
	if (dlg.DoModal() == IDOK) {
		strcpy(name, dlg.GetPathName());

		AddCommentToPDP(name);
	//	SetTitle(name);
	
		CPreviewDlg pdlg;
		pdlg.m_Filename = name;
		if (pdlg.DoModal() == IDCANCEL) return;

		if (ReadPDPinput(name) == 0) {
			res = AfxMessageBox("Data scan failed or not complete\n Do you want to edit the file?", 3); 
			if (res == IDYES) {
				ShowFileText(name); 
			} // edit file
			return; 
		} //fail
	
		else { // data loaded
			PDPgeomLoaded = TRUE; // scanned
			CheckData();
			if (!OptFree) { AdjustAreaLimits();	SetPlates();}
			SetReionPercent();
			SetNeutrCurrents();
		//	ShowFileText(name);
			OnShow();

		/*	res = AfxMessageBox("NBI geometry is accepted.\n\n Do you want to take the beam from file?",3);
			if (res == IDYES) OnPdpBeamlets();
			else { // NO -> MAMuG default */
				SetBeam(); // calculate ->IonBeam Power, Neutr Power NofBeamletsTotal
				for (int i = 0; i < (int)NofChannelsHor; i++) ActiveCh[i] = TRUE;
				for (int j = 0; j < (int)NofChannelsVert; j++) ActiveRow[j] = TRUE;
				SetStatus(); 
				if ((int)BeamSplitType ==0) SetPolarBeamletCommon(); //SetBeamletAt(0);
				else SetGaussBeamlet(BeamCoreDivY, BeamCoreDivZ);
				OnShow();

				res = AfxMessageBox("To set MAMuG beam configuration use the Green Panel.\n"
					"SINGAP beam is to be taken from file\n (Menu -> PDP -> Beam) ");
		} // data loaded
	
	}//OK
}

void CBTRDoc::OnDataStore() // Update + save (v5) actual Config 
{
	/*if (!STOP) {
		AfxMessageBox("Stop calculations before updating the data! \n (Red cross on the Toolbar)");
		return ;
	}*/

	STOP = TRUE;	//
	//OnStop();
	CWaitCursor wait; 
	DataLoaded = FALSE;
//	pDataView->OnDataStore();

	double NCH = NofChannelsHor;
	double NCV = NofChannelsVert;
	double SSH = SegmStepHor;
	double SSV = SegmStepVert;
	double BAH = BeamAimHor; 
	double BAV = BeamAimVert;
	
	double NBH = NofBeamletsHor;
	double NBV = NofBeamletsVert;
	double ASH = AppertStepHor;
	double ASV = AppertStepVert;
	double AAH = AppertAimHor; 
	double AAV = AppertAimVert;

	m_Text.Empty();
	CString S;
	pDataView->m_rich.GetWindowText(S); //pDoc->m_Text);
	m_Text = S; // S -> m_Text;
	if (m_Text.Find("BTR") <= 0 && m_Text.Find("INPUT") <= 0){
		OnDataActive();
		//OnShow();
		//return;
	}
	//if ((int) OptDuct == 0) OptAddDuct = FALSE;
	//else OptAddDuct = TRUE;

	ActiveCh.SetSize(10);
	ActiveRow.SetSize(10);	
	
	UpdateDataArray();// m_Text -> DataArray // calls SetData, SetOption
	CheckData();

	/* // automatic update
	char buf[1024];
	char * name = strcpy(buf, ConfigFileName);
	FILE * fout = fopen(name, "w");
	fprintf(fout, m_Text);
	fclose(fout);
	S.Format(">>>> Config-file %s is Rewritten! \n", ConfigFileName);
	logout << S;
	AfxMessageBox(S);*/

	pDataView->m_rich.SetFont(&pDataView->font, TRUE);
	pDataView->m_rich.SetBackgroundColor(FALSE, RGB(210,250,200));
	pDataView->m_rich.SetWindowText(m_Text);
	pDataView->m_rich.SetModify(FALSE);

	BOOL MAMuGchanged = FALSE;
	if (fabs(NCH - NofChannelsHor) > 1.e-6 || fabs(NCV - NofChannelsVert) > 1.e-6 
		|| fabs(SSH - SegmStepHor) > 1.e-6 || fabs(SSV - SegmStepVert) > 1.e-6 
		|| fabs(BAH - BeamAimHor) > 1.e-3 || fabs(BAV - BeamAimVert) > 1.e-3
		|| fabs(NBH - NofBeamletsHor) > 1.e-6 || fabs(NBV - NofBeamletsVert) > 1.e-6
		|| fabs(ASH - AppertStepHor) > 1.e-6 || fabs(ASV - AppertStepVert) > 1.e-6
		|| fabs(AAH - AppertAimHor)  > 1.e-3 || fabs(AAV - AppertAimVert)  > 1.e-3) 
	{
	//	AfxMessageBox("Active beam option - MAMuG"); 
		OptSINGAP = FALSE;
		MAMuGchanged = TRUE;
	//	for (int i = 0; i < (int)NofChannelsHor; i++) ActiveCh[i] = TRUE;
	//	for (int j = 0; j < (int)NofChannelsVert; j++) ActiveRow[j] = TRUE;
	}

	SetTraceParticle(TracePartType); // -> TraceTimeStep
	SetBeam(); // Mamug Aimings, IS positions */
	if (OptSINGAP)	SetSINGAP(); // form BEAMLET_ENTRY array (apply midalign, active chan)

	if (MAMuGchanged) {
		for (int i = 0; i < (int)NofChannelsHor; i++) ActiveCh[i] = TRUE;
		for (int j = 0; j < (int)NofChannelsVert; j++) ActiveRow[j] = TRUE;
	}
		
	int Ntot;
	//if ((int)BeamSplitType ==0) {
		SetPolarBeamletCommon(); //SetBeamletAt(0);
		Ntot = (int)(PolarNumber * AzimNumber);// base numbers - not used in v5
			
		SetPolarBeamletReion();
		SetPolarBeamletResidual();
	//}// split = polar - always
/*	else {
		SetGaussBeamlet(BeamCoreDivY, BeamCoreDivZ); 
		Ntot = (int)(BeamSplitNumberY * BeamSplitNumberZ);
	}*/

	/*if (Attr_Array.GetSize() * 3 < Ntot && Attr_Array.GetSize() > 1) {
		AfxMessageBox("Warning:\n Number of Model Particles is too small (for active Cut-off Current)",
						MB_ICONEXCLAMATION);  
	}
	if (Ntot > 1 && Attr_Array.GetSize() <= 1) {
		AfxMessageBox("ErrInput:\n Zero Number of Model Particles!\n Cut-off Current must be decreased!",
						MB_ICONEXCLAMATION);  
	}*/
	

    int res;// = IDNO; // ask to update geometry
	//res = AfxMessageBox("Keep the existing Surfaces?",3);//MB_ICONQUESTION | MB_YESNO);
	//if (res == IDNO) SetPlates(); 
	/*if (TorCentreX > 25) { //ITER
		FWRmin = 4.0; FWRmax = 8.4;//ITER
	}
	else {//TIN
		FWRmin = 2.0; FWRmax = 4.4;
	}*/
	ClearAllPlates();
	//InitPlasma();
	if (!FWdataLoaded) InitFWarray();

	if (!OptFree) //->standard
		SetPlates(); // create again//SetPlasma() is called from SetPlatesNBI;CheckClosePlates();
	else ModifyArea(TRUE);// beam is fixed = true, ignore beamfoot change 
		
	SetReionPercent();
	
	//if (OptThickNeutralization) 
	SetNeutrCurrents(); // switched back for all cases!!!

	//SetDecayInPlasma();

	ClearArrays();// attr vectors
	ClearScenLoadTracks();// clear tracked arrays
	
	SetStatus(); // NofBeamlets, Nofactive channels, InitTracers
	ShowStatus();
	logout << "-- Config is Updated by User [OnDataStore] ---\n"; 
	logout << "OptLogSave is " << OptLogSave << "\n";
	OnShow();
	
}
void CBTRDoc::ShowStatus()
{
	//CLoadView * pLV = (CLoadView *)pLoadView;
	pLoadView->Load = NULL;
	pLoadView->ShowLoad = FALSE;
	pLoadView->InvalidateRect(NULL, TRUE);

	pMainView->ShowNBLine();
	pMainView->ShowCoord();
	
	//if (STOP) pDataView->OnDataActive();
	//else ShowLogFile(50); // show log

	pDataView->OnDataActive();

	pSetView->Load = NULL;
	pSetView->ShowStatus(); //InvalidateRect(NULL, TRUE);

}

void CBTRDoc:: OnShow() 
{
//	char old[128];
//	::GetWindowText(NULL, old, 20);
	//SetTitle(CurrentDirName);//(TaskName);
	
	if (pDataView == NULL) return;
	if (pLoadView == NULL) return;
	if (pSetView == NULL) return;
	if (pMainView == NULL) return;

	CString S;
	S.Format("%g STOPPED", BTRVersion);
	//if (STOP) SetTitle(S);
	
	pDataView->OnDataActive();//OnDataActive();
	
	ShowProfiles = FALSE;
	pLoadView->Load = NULL;
	pLoadView->ShowLoad = FALSE;
	pLoadView->InvalidateRect(NULL, TRUE);
	
	pSetView->Load = NULL;
	pSetView->InvalidateRect(NULL, TRUE);

	pMainView->InvalidateRect(NULL, TRUE);
	
	S.Format("  [SHOW]\n");
	//SetTitle(S);//causes error!
	logout << S;// << std::endl;
}

void   CBTRDoc::SaveData()
{
	char name[1024];
	FormDataText(TRUE);// include internal names of parameters
	
	CFileDialog dlg(FALSE, "dat; txt | * ", "", OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"BTR Output file (*.txt);(*.dat) | *.txt; *.TXT; *.dat; *.DAT | All Files (*.*) | *.*||", NULL);
		if (dlg.DoModal() == IDOK) {
			FILE * fout;
			strcpy(name, dlg.GetPathName());
			fout = fopen(name, "w");
			fprintf(fout, m_Text);
			fclose(fout);
			SetTitle(name);// + TaskName);
		}
		delete dlg;

	/*	if (AddPlatesNumber > 0) {
			int reply = AfxMessageBox("Save the List of Additional Surfaces?", 3);
			if (reply != IDYES) return;  //in v5 the Add Surf is included to Config!

		CFileDialog dlg1(FALSE, "dat; txt | * ", "", OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"Additional Surfaces data file (*.txt);(*.dat) | *.txt; *.TXT; *.dat; *.DAT | All Files (*.*) | *.*||", NULL);
		if (dlg1.DoModal() == IDOK) {
			strcpy(name, dlg1.GetPathName());
			WriteAddPlates(name);
		}

		}*/

}

int   CBTRDoc::ReadData() // file text -> to m_Text
{
	DataLoaded = FALSE;
	char name[1024];
	char buf[1024];
	char dirname[1024];
	CString S;
	CString FileName, FilePath, FileDir;
	CurrentDirName = TopDirName; // set in InitData;
	
	CFileDialog dlg(TRUE, "dat; txt  | * ", "", OFN_EXPLORER,// OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"BTR Input file (*.dat); (*.txt) | *.dat; *.DAT; *.txt; *.TXT; | All Files (*.*) | *.*||", NULL);
		if (dlg.DoModal() == IDOK) {
			FileName = dlg.GetFileName();
			FilePath = dlg.GetPathName();
			strcpy(name, FilePath);
			ConfigFileName = FileName;
			int pos = FilePath.Find(FileName, 0);
			if (pos > 1) FileDir = FilePath.Left(pos-1);
			else { AfxMessageBox("Bad FileDir"); return 0; }
		//strcpy(dirname, dlg.GetFolderPath());
		//::GetCurrentDirectory(1024, CurrentDirName);
	
		CPreviewDlg pdlg;
		pdlg.m_Filename = FilePath;//name;
		if (pdlg.DoModal() == IDCANCEL) return 0;

		FILE * fin;
		fin = fopen(name, "r");		//fprintf(fout, m_Text);
		DWORD error = ::GetLastError();
		if (fin == NULL || error != 0) { AfxMessageBox("fin = NULL"); return 0;}
		
		m_Text = "";
		while (!feof(fin)) {
			fgets(buf, 1024, fin);
			if (!feof(fin))	m_Text += buf;
		}
		//if (m_Text.GetLength() < 1) { AfxMessageBox("Length = 0"); return 0;} 
		fclose(fin);
		//AfxMessageBox("Closed file");
		//char OpenDirName[1024]; 	::GetCurrentDirectory(1024, OpenDirName);
		//S.Format("GetCurrentDir OK\n OpenDirName %s", OpenDirName);	AfxMessageBox(S);
		//error = ::GetLastError();
		CurrentDirName = FileDir;//dirname; //dlg.GetFolderPath();//OpenDirName;
		::SetCurrentDirectory(CurrentDirName);	
		OpenLogFile();
		//S.Format("Set DirName %s", CurrentDirName);	AfxMessageBox(S);
		//SetTitle(OpenDirName);
		SetTitle(name); //AfxMessageBox("SetTitle OK");
		DataLoaded = TRUE; //AfxMessageBox("Loaded OK");
		
		return 1; //success
	}
	//if NOT OK
	OpenLogFile();// in BTR.exe folder
	return 0;
}
void  CBTRDoc:: OpenLogFile()//CString dir)
{
	//::SetCurrentDirectory(dir);	
	LogFilePath = CurrentDirName; // + "\\" + "_LOG_.txt";
	//CString Name = LogFileName; // "LOG_BTR5.txt";
	char buf[1024];
	char * path = strcpy(buf, LogFilePath + "\\" + LogFileName);  
	if (OptLogSave==1) 
		logout.SetFile(path); 
	//logout.log.open("LOG_BTR5.txt");//, ios::out);// | ios::ate); //, "w");
	CString s;
	CTime tm = StartTime0;//CTime::GetCurrentTime();
	CString Date, Time;
	Date.Format("%02d-%02d-%04d", tm.GetDay(), tm.GetMonth(), tm.GetYear());
	Time.Format("%02d:%02d:%02d", tm.GetHour(), tm.GetMinute(), tm.GetSecond());
	s.Format("\n BTR LOG ----  Date %s   Time %s --------\n", Date, Time);
	logout << s;
	logout << " BTR version " << BTRVersion << " /MULTI\n";
	logout << "HomeDirName :  " + CurrentDirName + "\n"; 
	s.Format("SET Log-file  %s\n", path);//, LogFilePath);
	logout << s;
	//logout << "NewDocument done\n"; // moved from OnNewDocument()
	//logout << "InitData done\n";
		
	s.Format("\n____Basic Config file:___ %s___\n", ConfigFileName);
	logout << s;
	
	//s.Format(" Default Scenarious: %d\n", MAXSCEN);
	//logout << s;	//MAXSCEN = 1; //single 
	//logout.log.flush(); 
	//logout.log.close();

}
void  CBTRDoc:: CloseLogFile()
{
	//return;
	//::SetCurrentDirectory(LogFilePath);
	logout << "---------- END / CLOSE LOG-file ----------\n";	 
	logout.CloseFile(); //log.close();
}

void  CBTRDoc:: ResetLogFile()
{
	//logout << "---------- RESET LOG-file ----------\n";	 
	char buf[1024];
	char * path = strcpy(buf, LogFilePath + "\\" + LogFileName);  
	if (OptLogSave) {
		logout.ResetFile(path);//close + reopen for append
		logout << "_________ Reset Log File __________\n";
		logout << "Log File output is ON\n"; 
	}
	else logout << "!!!! Log File output is OFF!!!!\n"; 
}

void  CBTRDoc:: WritePDPinput(char * name)
{

	FILE * fout;
	CString S;

		fout = fopen(name, "w"); //("PDP_input.dat", "w");
		fprintf(fout, "NBI geometry (for PDP-code), created by BTR 1.7 \n");

/*"1st line - ""Source"": Eo, Iacc, Ac, Ah "					 									
"        Eo=Beam energy, MeV,"														
"        Iacc=Beam Current, A"														
"        Ah=Beam ""halo"" angle, mrad"														
"        Ch=Part of D- current in ""halo"""	*/
		 
		fprintf(fout, "%g  %g  %g  %g \t\t !#1 Eo,MeV (Energy);   Iacc,A (Current);   Ah,mrad (halo angle)   Ch (halo fraction) \n", 
			IonBeamEnergy, IonBeamCurrent, BeamHaloDiverg *1000, BeamHaloPart); 

/*"2nd line - ""Numbers"": Lch, Nach, LchR "														
        LchN - number of channels in Neutraliser													
       Nach - number of beamlets per channel													
       LchR- number of channels in RID	*/	

		int BeamletsPerSegment = NofBeamletsTotal / (int)NofChannelsHor;
		fprintf(fout, "%d  %d  %d  \t\t\t !#2  LchN (Neutr. channels);   Nach (beamlets per segment);  LchR (RID channels)\n", 
			(int)NeutrChannels, BeamletsPerSegment, (int)RIDChannels);
		
/*"3d line  YSo(l), mm -horiz.coord.of segment column axis at X=0"	*/	
		
		double ys1=0, ys2=0, ys3=0, ys4=0;
		if ((int)NofChannelsHor > 4){
			S.Format("Number of Beam Segments in BTR (= %d)\n exceeds PDP Channels Limit(=4)!\n Horiz. centers are not transferred (line 3)", 
			(int)NofChannelsHor);
			AfxMessageBox(S); 
		}
		else { 
			if ((int)NofChannelsHor > 0)  ys1 = SegmCentreHor[0]*1000; 
			if ((int)NofChannelsHor > 1)  ys2 = SegmCentreHor[1]*1000; 
			if ((int)NofChannelsHor > 2)  ys3 = SegmCentreHor[2]*1000;
			if ((int)NofChannelsHor > 3)  ys4 = SegmCentreHor[3]*1000; 
		}
		fprintf(fout, "%g  %g  %g  %g  \t\t  !#3  YSo,mm (horiz.coord.of segment column axis at X=0)\n", ys1, ys2, ys3, ys4);

/*"4th line -  Xd(id), m -distance to an ""id""-set of channels diaphragms"													
"             (id=1…2 - neutralizer;  id=3...4 - RID, id=5 - scraper)"	*/
		
		
		fprintf(fout, "%g  %g  %g  %g  %g  %g  %g  %g  %g  %g  %g  %g  %g  %g  %g \t !#4  Xd,m (channels diaphragms X)\n",
			//"%4.2f  %4.2f  %4.2f  %4.2f  %4.2f  %4.2f  %4.2f  %4.2f  %4.2f  %4.2f  %4.2f  %4.2f  %4.2f  %4.2f  %4.2f  \t !(4)  Xd,m (channels diaphragms X)\n",
				NeutrInX, NeutrOutX, RIDInX, RIDOutX, PreDuctX, DDLinerInX, DDLinerOutX,
				Duct1X, Duct2X, Duct3X, Duct4X, Duct5X, Duct6X, Duct7X, Duct8X); 

/*"5th line - Wd(id), mm - horiz.width of diaphragms"*/	

		fprintf(fout, "%g  %g  %g  %g  %g  %g  %g  %g  %g  %g  %g  %g  %g  %g  %g \t  !#5  Wd, mm (horiz.width of diaphragms)\n",
			NeutrInW *1000, NeutrOutW *1000, RIDInW *1000, RIDOutW *1000, 
			PreDuctW *1000, DDLinerInW *1000, DDLinerOutW *1000,
			Duct1W *1000, Duct2W *1000, Duct3W *1000, Duct4W *1000,
			Duct5W *1000, Duct6W *1000, Duct7W *1000, Duct8W *1000);


/*"6th line - Hd(id), m - vert.hight of diaphragms"	*/	
		
		fprintf(fout, "%g  %g  %g  %g  %g  %g  %g  %g  %g  %g  %g  %g  %g  %g  %g \t  !#6  Hd, m (vert. height of diaphragms)\n",
			NeutrH, NeutrH, RIDH, RIDH, PreDuctH, DDLinerInH, DDLinerOutH,
			Duct1H, Duct2H, Duct3H, Duct4H,	Duct5H, Duct6H, Duct7H, Duct8H);

/*"7th line -  DPLY(id), mm -horiz.displacement of diaphragms axes (misalignement)"	*/	

		fprintf(fout, "%g  %g  %g  %g  %g  %g  %g  %g  %g  %g  %g  %g  %g  %g  %g \t  !#7  DPLY, mm (horiz. bias of diaphragms)\n",
			NeutrBiasInHor *1000, NeutrBiasOutHor *1000, RIDBiasInHor *1000, RIDBiasOutHor *1000, 
			DiaBiasHor *1000, LinerBiasInHor *1000, LinerBiasOutHor *1000,
			Duct1BiasHor *1000, Duct2BiasHor *1000, Duct3BiasHor *1000, Duct4BiasHor *1000,
		    Duct5BiasHor *1000, Duct6BiasHor *1000, Duct7BiasHor *1000, Duct8BiasHor *1000);
		
		
/*"8th line DPLZ(id), mm -vertic.displacement of diaphragms axes (misalignement)"*/	
		
		fprintf(fout, "%g  %g  %g  %g  %g  %g  %g  %g  %g  %g  %g  %g  %g  %g  %g \t  !#8  DPLZ, mm (vert. bias of diaphragms)\n",
			NeutrBiasInVert *1000, NeutrBiasOutVert *1000, RIDBiasInVert *1000, RIDBiasOutVert *1000, 
			DiaBiasVert *1000, LinerBiasInVert *1000, LinerBiasOutVert *1000,
			Duct1BiasVert *1000, Duct2BiasVert *1000, Duct3BiasVert *1000, Duct4BiasVert *1000,
		    Duct5BiasVert *1000, Duct6BiasVert *1000, Duct7BiasVert *1000, Duct8BiasVert *1000);
		

/*"9th line - ""NBL"" : AYmis,AZmis,XHaim,Reion,Fr,Ainc,Atil"													
"        AYmis=Horiz.beam angular misalignment, mrad"													
"        AZmis=Vertical beam angular misalignment, mrad"													
"        XHaim=Gorizontal aiming of channels and plates, m"													
"     //   Reion=Reionization, "													
        Fr=Neutralisation efficiency														
"       Ainc=Beam (Duct) average axis inclination angle, mrad"														
"       Atil= beam vertical tilting angle, mrad"*/

		fprintf(fout, "%g  %g  %g  %g  %g  %g  \t\t !#9 AYmis, mrad; AZmis, mrad; XHaim, m; Fr(Neutr. Eff); Ainc, mrad; Atil, mrad\n",
			BeamMisHor *1000, BeamMisVert *1000, BeamAimHor,//	ReionPercent, 
			NeutrPart, VertInclin *1000, BeamVertTilt *1000); 
		
	/*	if (ReionPercent < 1.e-6){
			S.Format("Warning:\nPressure not defined\nReionization = 0");
			AfxMessageBox(S); 
		}*/

/*10th line - For information only: Th1, Th2, Th3
	Th1=Thickness of the neutraliser entry plates, mm
	Th2=Thickness of the neutraliser exit plates, mm
"       Th3=Thickness of the RID plates, mm"*/	

		fprintf(fout, "%g   %g   %g   %g   %g  \t\t !#10  Panel Thickness, mm (Neutralizer Entry/Exit, RID), AppertAimHor,BeamAimVert \n",
			NeutrInTh *1000, NeutrOutTh *1000, RIDTh *1000, AppertAimHor, BeamAimVert );
		
		fclose(fout);
		
}

void CBTRDoc:: AddCommentToPDP(char * name)
{
	int res;
	char buf[1024];
	unsigned long Size1 = MAX_COMPUTERNAME_LENGTH + 1;
	unsigned long Size2 = MAX_COMPUTERNAME_LENGTH + 1;
	char user[MAX_COMPUTERNAME_LENGTH + 1];
	char machine[MAX_COMPUTERNAME_LENGTH + 1];
	CString CompName = "";
	CString UserName = "";
	
	CString text = ""; // rewrite file
	CString line = "";
	CString left = "";

	CString Fname = name;
	Fname.MakeUpper();
	if (Fname.Find("_BTR") <= 0 && Fname.Find("BTR.txt") <= 0) return; // corrected name!
	
	BOOL doit = TRUE;
	BOOL day = FALSE; // friday?
	BOOL askAK = FALSE; // quest to AK

	CTime tm = CTime::GetCurrentTime();
	if (Odd(tm.GetDay())) doit = TRUE; //1,3,5..
	else doit = FALSE; // 2,4,6...
	//if (tm.GetMonth() == 6) doit = FALSE; // dont do in june
	
	if (doit == FALSE) return; // 2,4,6... or June
	
	if (tm.GetDayOfWeek() == 6) day = TRUE; // 1 - Sunday

	if (::GetComputerName(machine, &Size1)!= 0) {
		CompName = machine; CompName.MakeUpper();}
		
	if  (::GetUserName(user, &Size2) != 0) {
		UserName = user;	UserName.MakeUpper(); }
		
		if (CompName.Find("PANAS")>=0 || UserName.Find("PANAS")>=0) return;
		if (CompName.Find("PAA")>=0 || UserName.Find("PAA")>=0) return;

		askAK = FALSE;
		if (CompName.Find("KRYLOV")>=0 || UserName.Find("KRYLOV")>=0) askAK = TRUE;
		if (CompName.Find("AK-OFF")>=0 || UserName.Find("AK-OFF")>=0) askAK = TRUE;
	//	if (CompName.Find("DED")>=0 || UserName.Find("DED")>=0) askAK = TRUE;
		
		if (askAK && day) {
			res = AfxMessageBox("Warning:\n Are you sure this is Valid name for PDP-input file?", 
							MB_ICONQUESTION | MB_YESNOCANCEL); 
			if (res == IDYES)  AfxMessageBox("Up to YOU!", MB_ICONSTOP | MB_OK); 
			else AfxMessageBox("Correct it and read the file again, SVP!", MB_ICONSTOP | MB_OK); 
		}
	
//	CString CurrDate, CurrTime;
//	CString Date, Time;
//	CurrDate.Format("%02d:%02d:%04d", tm.GetDay(), tm.GetMonth(), tm.GetYear());
//	CurrTime.Format("%02d:%02d:%02d", tm.GetHour(), tm.GetMinute(), tm.GetSecond());


	FILE * fin;
	fin = fopen(name, "r");
		if (fin==NULL){
			AfxMessageBox("problem 1 opening PDP-input file", MB_ICONSTOP | MB_OK);
			return;
		}

		text = ""; // rewrite file
		line = "";
		left = "";
		int l = 0;
		res = 0;
		double v1, v2, v3, v4;
		while (!feof(fin)) {
			fgets(buf, 1024, fin);
			text += buf;
			if (res < 1 && !feof(fin) ) res = sscanf(buf, "%le %le %le %le", &v1, &v2, &v3, &v4); // PDP format?
			l++;
		}
	fclose(fin);


	FILE * fout;

	if (text.Find("!#") > 0) return; // BTR generated PDP-input
		
	int posak = text.Find("PDP");
	
	if (posak < 0 && res == 4)  { // string "PDP" not found and true PDP format 

		line = "AK:  PDP\n";
	/*	int pos = text.Find("Input");
		//if (pos <= 0) pos = text.Find("file");
		if (pos <= 0) { // "Input file" not found
			line = "AK:  PDP \n";
			left = text; // data
		}

		if (pos > 100) {
			line = "AK:  PDP \n" + text.Mid(pos);
			left = text.Left(pos);
		}
		*/
		
	//	if (pos <= 0 || pos > 100) {
			fout = fopen(name, "w");
			if (fout == NULL){
				AfxMessageBox("problem 2 opening PDP-input file", MB_ICONSTOP | MB_OK);	
				return;
			}
			fprintf(fout, line); fprintf(fout, text);
			//fprintf(fout, left);
			fclose(fout);
	//	} // "Input file" found at the end
	
	}// "PDP" not found
}

int CBTRDoc::OldPDPinput(char * name)
{
	char buf[1024];
	FILE * fin;

	int result;

	fin = fopen(name, "r");
	if (fin==NULL){
		AfxMessageBox("PDP-input not found!", MB_ICONSTOP | MB_OK);
		return -1;
	}
	
	// line 1
	double Eo, Iacc, Ah, Ch, add;
	if (fgets(buf,1024,fin) == NULL) { AfxMessageBox("Problem 0 with file");fclose(fin); return -1; }
	result = sscanf(buf, "%lf %lf %lf %lf", &Eo, &Iacc, &Ah, &Ch); 
		if (result != 4) {
			while (!feof(fin) && result !=4) {
				fgets(buf, 1024, fin);
				result = sscanf(buf, "%lf %lf %lf %lf", &Eo, &Iacc, &Ah, &Ch);
			}
			if (feof(fin)) { AfxMessageBox("ErrInput:\n1st line data not found"); fclose(fin); return -1;}
			else {
				result = sscanf(buf, "%lf %lf %lf %lf %lf", &Eo, &Iacc, &Ah, &Ch, &add);
				if (result == 5) { AfxMessageBox("ErrInput:\n1st line contains Extra Data\n Invalid PDP-input format"); fclose(fin); return -1;}
			}
		}
		else {
			result = sscanf(buf, "%lf %lf %lf %lf %lf", &Eo, &Iacc, &Ah, &Ch, &add);
			if (result == 5) { AfxMessageBox("ErrInput:\n1st line contains Extra Data\n Invalid PDP-input format"); fclose(fin); return -1;}
		}

		// lines 2-8
		if (fgets(buf,1024,fin) == NULL) { fclose(fin); return -1; }//2
		if (fgets(buf,1024,fin) == NULL) { fclose(fin); return -1; }//3
		if (fgets(buf,1024,fin) == NULL) { fclose(fin); return -1; }//4
		if (fgets(buf,1024,fin) == NULL) { fclose(fin); return -1; }//5
		if (fgets(buf,1024,fin) == NULL) { fclose(fin); return -1; }//6
		if (fgets(buf,1024,fin) == NULL) { fclose(fin); return -1; }//7
		if (fgets(buf,1024,fin) == NULL) { fclose(fin); return -1; }//8
		
		// line 9
		double AYmis, AZmis, XHaim, Reion, Fr, Ainc, Atil;
		if (fgets(buf,1024,fin) == NULL) { fclose(fin); return -1; }//9
		result = sscanf(buf, " %lf %lf %lf %lf %lf %lf %lf", 
			&AYmis,&AZmis,&XHaim,&Reion,&Fr,&Ainc,&Atil);
		if (result == 7) {//OLD (Reionization included to line 9) PDP-10
			fclose(fin);
			return 1;// old
		}
		else { // <7
			result = sscanf(buf, " %lf %lf %lf %lf %lf %lf", 
			&AYmis,&AZmis,&XHaim,&Fr,&Ainc,&Atil); // no reion
			if (result != 6) {
				fclose(fin);
				return -1;// invalid
			}
		}

		// line 10
		double Th1, Th2, Th3, XBHaim, XVaim;
		if (fgets(buf,1024,fin) == NULL) { fclose(fin); return -1; }//10
		result = sscanf(buf, " %lf %lf %lf %lf %lf", &Th1, &Th2, &Th3, &XBHaim, &XVaim);
		if (result != 5) {
			result = sscanf(buf, " %lf %lf %lf", &Th1, &Th2, &Th3);
			if (result != 3) {//invalid
				fclose(fin);
				return -1; 
			}
			else {
				fclose(fin);
				return 1; // old
			}
		}
		
		fclose(fin);
	//	AfxMessageBox("OldPDPiput complete - NEW");
		return 0; // new version
}


int   CBTRDoc::ReadPDPinput_old(char * name) //used  for ITER before 2018 // config file for PDP *.txt
{  
//AfxMessageBox("ReadPDPiput begin");
//	DataLoaded = FALSE;
//	char name[1024];
	char buf[1024];
	FILE * fin;
	CString quest = "Read next data anyhow?";

	int result, reply;

	/*	fin = fopen(name, "r");
		if (fin==NULL){
			AfxMessageBox("PDP-input not found!", MB_ICONSTOP | MB_OK);
			return 0;
		}
*/
		int OldInput = OldPDPinput(name);
		if (OldInput < 0)  return 0;

		fin = fopen(name, "r");

//while (!feof(fin)) {
											
/*"1st line - ""Source"": Eo ,Iacc,  Ah ,  Ch "					 								
"        Eo=Beam energy, MeV,"													
"        Iacc=Beam Current, A"													
"        Ah=Beam ""halo"" angle, mrad"													
"        Ch=Part of D- current in ""halo"""	*/

		double Eo, Iacc, Ah, Ch;
		if (fgets(buf,1024,fin) == NULL) { AfxMessageBox("Problem 0 with file");fclose(fin); return 0; }
		result = sscanf(buf, "%lf %lf %lf %lf", &Eo, &Iacc, &Ah, &Ch); 
		if (result != 4) {
			while (!feof(fin) && result !=4) {
				fgets(buf, 1024, fin);
				result = sscanf(buf, "%lf %lf %lf %lf", &Eo, &Iacc, &Ah, &Ch);
			}
			if (feof(fin)) { AfxMessageBox("ErrInput:\n1st line data not found"); fclose(fin); return 0;}

		}
		
		if (Eo < 1.e-6) {
			reply = AfxMessageBox("Warning:\n Source Beam Energy < 1 eV!\n Maybe NOT geometry file (PDP-input)?  \n" + quest, MB_YESNOCANCEL);
			if (reply != IDYES) { fclose(fin); return 0;}
		}
		IonBeamEnergy = Eo;
		if (Iacc < 1.e-6) {
			reply = AfxMessageBox("Warning:\n Beam current < 1.e-6 A\n" + quest, MB_YESNOCANCEL);
			if (reply != IDYES) { fclose(fin); return 0;}
		}
		IonBeamCurrent = Iacc;
		if (Ah < 1.e-3) {
			reply = AfxMessageBox("Warning:\n Beam halo div < 1.e-3 mrad\n"+ quest, MB_YESNOCANCEL);
			if (reply != IDYES) { fclose(fin); return 0;}
		}
		BeamHaloDiverg = Ah * 0.001; 
	/*	if (Ch < 1.e-6) {
			reply = AfxMessageBox(" Beam halo part < 1.e-6 \n Continue?", MB_YESNOCANCEL);
			if (reply != IDYES) { fclose(fin); return 0;}
		}*/
		if (Ch > 1) {
			reply = AfxMessageBox("Warning:\n Beam halo part > 1 \n"+ quest, MB_YESNOCANCEL);
			if (reply != IDYES) { fclose(fin); return 0;}
		}
		BeamHaloPart = Ch; 

		if ((int)BeamSplitType ==0) SetPolarBeamletCommon(); //SetBeamletAt(0);
		else SetGaussBeamlet(BeamCoreDivY, BeamCoreDivZ); 
		
//AfxMessageBox("ReadPDPiput line 1 complete");

/*"2nd line - ""Numbers"": Lch, Nach "													
        LchN - number of channels in Neutraliser													
       Nach - number of beamlets per channel													
       LchR- number of channels in RID */		
		int LchN, Nach, LchR;
		if (fgets(buf,1024,fin) == NULL) { fclose(fin); return 0; }
		result = sscanf(buf, " %d %d %d", &LchN, &Nach, &LchR); 
		if (result != 3) {
			reply = AfxMessageBox("Warning:\nProblem with 2nd line! Not all data found.\n"+ quest, 3);
			if (reply != IDYES) { fclose(fin);return 0;}
		}
		
		if (LchN < 1.e-6) {
			reply = AfxMessageBox("Warning:\n Zero Number of Neutralizer channels\n" + quest, 3);
			if (reply != IDYES) { fclose(fin);return 0;}
			NeutrChannels = 0;
		}
		NeutrChannels = LchN; 
		// Nach not used in BTR
		if (LchR < 1.e-6) {
			reply = AfxMessageBox("Warning:\nZero Number of RID channels\n" + quest, 3);
			if (reply != IDYES) { fclose(fin);return 0;}
			RIDChannels = 0;
		}
		RIDChannels = LchR;


/*"3d line  YSo(l), mm -horiz.coord.of segment column axis at X=0" */													

		double ys0, ys1, ys2, ys3;
		if (fgets(buf,1024,fin) == NULL) { fclose(fin); return 0; }
		result = sscanf(buf, " %le %le %le %le", &ys0, &ys1, &ys2, &ys3); 
		if (result != 4) {
			reply = AfxMessageBox("Warning:\nProblem with 3rd line! Not all data found\n"+ quest, 3);
			if (reply != IDYES) { fclose(fin);return 0;}
		}

		if (fabs(ys0) < 1.e-6 && fabs(ys1) + fabs(ys2) + fabs(ys3)  > 1.e-6 ) {
			reply = AfxMessageBox(" Zero 1st channel Position!  \n"+ quest, 3);
			if (reply != IDYES) { fclose(fin);return 0;}
		}
/*
		SegmCentreHor[0] =  ys0 * 0.001; SegmCentreHor[1] =  ys1 * 0.001; 
		SegmCentreHor[2] =  ys2 * 0.001; SegmCentreHor[3] =  ys3 * 0.001; 
		double step1 = ys1 - ys0;
		double step2 = ys2 - ys1;
		double step3 = ys3 - ys2;
		if (fabs(step2 - step1) >1.e-6 || fabs(step3 - step2) >1.e-6) { 
			reply = AfxMessageBox(" Horiztal steps between beam groups are not equal!\n Continue?", 3);
			if (reply != IDYES) { fclose(fin); return 0;}
		//	SegmStepHor = 0;
		}
	//	else SegmStepHor = step1 * 0.001;
	//	SetBeam(); // Aimings, IS positions
	//  SetSINGAP();
	*/

/*"4th line -  Xd(id), m -distance to an ""id""-set of channels diaphragms"													
"             (id=1…2 - neutralizer;  id=3...4 - RID, id=5 - scraper)"
		max.number of diaphragms in the beamline = 15)*/		
 
		double xd1, xd2, xd3, xd4, xd5, xd6, xd7, xd8, xd9, xd10, xd11, xd12, xd13, xd14, xd15;
		if (fgets(buf,1024,fin) == NULL) { fclose(fin); return 0; }
		result = sscanf(buf, " %le %le %le %le %le %le %le %le %le %le %le %le %le %le %le", 
				&xd1, &xd2, &xd3, &xd4, &xd5, &xd6, &xd7, &xd8, &xd9, 
				&xd10, &xd11, &xd12, &xd13, &xd14, &xd15);
		if (result != 15) {
			reply = AfxMessageBox("Warning:\nProblem with line 4! Not all data found.\n" + quest, 3);
			if (reply != IDYES) { fclose(fin);return 0;}
		}
		if (xd1 * xd2 < 1.e-6) {
			reply = AfxMessageBox("Warning:\nZero X-limits for Neutralizer!  \n"+ quest, 3);
			if (reply != IDYES) { fclose(fin);return 0;}
		}
		if (xd1 > xd2) {
			reply = AfxMessageBox("Warning:\nNeutralizer Entrance is set AFTER Neutralizer Exit!  \n"+ quest, 3);
			if (reply != IDYES) { fclose(fin);return 0;}
		}
		NeutrInX = xd1; 
		NeutrOutX = xd2;
		NeutrXmax = xd2 + 0.01;
		NeutrXmin = Max(0.0, xd1-0.1);
	
		if (xd3 * xd4 < 1.e-6) {
			reply = AfxMessageBox("Warning:\n Zero X-limit for RID \n"+ quest, 3);
			if (reply != IDYES) {  fclose(fin);return 0;}
		}
		if (xd3 > xd4) {
			reply = AfxMessageBox("Warning:\nRID Entrance is set AFTER RID Exit!  \n"+ quest, 3);
			if (reply != IDYES) { fclose(fin);return 0;}
		}
		RIDInX = xd3; 
		RIDOutX = xd4;
		if (xd5 < 1.e-6) {
			reply = AfxMessageBox("Warning:\n Zero X for Scraper! \n"+ quest, 3);
			if (reply != IDYES) {  fclose(fin);return 0;}
		}
		PreDuctX = xd5; 
		if (xd6 * xd7 < 1.e-6) {
			reply = AfxMessageBox("Warning:\n Zero X-limit for Liner! \n"+ quest, 3);
			if (reply != IDYES) {  fclose(fin);return 0;}
		}
		DDLinerInX = xd6; 
		DDLinerOutX = xd7; 
		if (xd8 * xd9 * xd10 * xd11 * xd12 * xd13 * xd14 * xd15 < 1.e-6) {
			reply = AfxMessageBox("Warning:\n Zero position X for one of Duct sections! \n"+ quest, 3);
			if (reply != IDYES) {  fclose(fin);return 0;}
		}
		Duct1X = xd8; Duct2X = xd9; Duct3X = xd10;// 10 - used
		Duct4X = xd11; Duct5X = xd12; Duct6X = xd13; 
		Duct7X = xd14; Duct8X = xd15;
		AreaLong = xd15;
		
		ReionXmin = NeutrXmax;
		ReionXmax = AreaLong;

//"5th line - Wd(id), mm - horiz.width of diaphragms"
		double wd1, wd2, wd3, wd4, wd5, wd6, wd7, wd8, wd9, wd10, wd11, wd12, wd13, wd14, wd15;
		if (fgets(buf,1024,fin) == NULL) { fclose(fin); return 0; }
		result = sscanf(buf, " %le %le %le %le %le %le %le %le %le %le %le %le %le %le %le", 
				&wd1, &wd2, &wd3, &wd4, &wd5, &wd6, &wd7, &wd8, &wd9, &wd10, &wd11, &wd12, &wd13, &wd14, &wd15);
		if (result != 15) {
			reply = AfxMessageBox("Warning:\n Problem with line 5! Not all data found.\n"+ quest, 3);
			if (reply != IDYES) { fclose(fin);return 0;}
		}
		if (wd1 * wd2 < 1.e-6) {
			reply = AfxMessageBox("Warning:\n Zero Neutralizer Width! \n"+ quest, 3);
			if (reply != IDYES) { fclose(fin);return 0;}
		}
		NeutrInW = wd1 * 0.001; 
		NeutrOutW = wd2 * 0.001;
		if (wd3 * wd4 < 1.e-6) {
			reply = AfxMessageBox(" Zero RID Width! \n"+ quest, 3);
			if (reply != IDYES) { fclose(fin);return 0;}
		}
		RIDInW = wd3 * 0.001; 
		RIDOutW = wd4 * 0.001;
		if (wd5 < 1.e-6) {
			reply = AfxMessageBox(" Zero Scraper Width!\n"+ quest, 3);
			if (reply != IDYES) { fclose(fin);return 0;}
		}
		PreDuctW = wd5 * 0.001; 
		if (wd6 * wd7 < 1.e-6) {
			reply = AfxMessageBox(" Zero Liner Width! \n"+ quest, 3);
			if (reply != IDYES) { fclose(fin);return 0;}
		}
		DDLinerInW = wd6 * 0.001; 
		DDLinerOutW = wd7 * 0.001; 
		if (wd8 * wd9 * wd10 * wd11 * wd12 * wd13 * wd14 * wd15 < 1.e-6) {
			reply = AfxMessageBox(" Zero width of Duct section! \n"+ quest, 3);
			if (reply != IDYES) { fclose(fin);return 0;}
		}
		Duct1W = wd8 * 0.001;  Duct2W = wd9 * 0.001;  Duct3W = wd10 * 0.001;// 10 - used now
		Duct4W = wd11 * 0.001; Duct5W = wd12 * 0.001; Duct6W = wd13 * 0.001; 
		Duct7W = wd14 * 0.001; Duct8W = wd15 * 0.001;
		//AreaLong = wd14;
//AfxMessageBox("ReadPDPiput line 5 complete");
	
//"6th line - Hd(id), m -vert.hight of diaphragms"
		double hd1, hd2, hd3, hd4, hd5, hd6, hd7, hd8, hd9, hd10, hd11, hd12, hd13, hd14, hd15;
		if (fgets(buf,1024,fin) == NULL) { fclose(fin); return 0; }
		result = sscanf(buf, " %le %le %le %le %le %le %le %le %le %le %le %le %le %le %le", 
				&hd1, &hd2, &hd3, &hd4, &hd5, &hd6, &hd7, &hd8, &hd9, 
				&hd10, &hd11, &hd12, &hd13, &hd14, &hd15);
		if (result != 15) {
			reply = AfxMessageBox("Problem with line 6! Not all data found.\n"+ quest, 3);
			if (reply != IDYES) { fclose(fin);return 0;}
		}
		if (hd1 * hd2 < 1.e-6) {
			reply = AfxMessageBox("Zero Neutralizer Height! \n "+ quest, 3);
			if (reply != IDYES) { fclose(fin);return 0;}
		}
		NeutrH = hd1; 
		//NeutrOutH = hd2;
		if (hd3 * hd4 < 1.e-6) {
			reply = AfxMessageBox("Zero RID Height! \n"+ quest, 3);
			if (reply != IDYES) { fclose(fin);return 0;}
		}
		RIDH = hd3; 
	//	RIDOutH = hd4;
		if (hd5 < 1.e-6) {
			reply = AfxMessageBox("Zero Scraper Height! \n"+ quest, 3);
			if (reply != IDYES) { fclose(fin);return 0;}
		}
		PreDuctH = hd5; 
		if (hd6 * hd7 < 1.e-6) {
			reply = AfxMessageBox("Zero height for liner \n"+ quest, 3);
			if (reply != IDYES) {fclose(fin);return 0;}
		}
		DDLinerInH = hd6; 
		DDLinerOutH = hd7; 
		if (hd8 * hd9 * hd10 * hd11 * hd12 * hd13 * hd14 * hd15< 1.e-6) {
			reply = AfxMessageBox("Zero X-position of Duct section! \n"+ quest, 3);
			if (reply != IDYES) { fclose(fin);return 0;}
		}
		Duct1H = hd8; Duct2H = hd9; Duct3H = hd10;// 10 -  used
		Duct4H = hd11; Duct5H = hd12; Duct6H = hd13;
		Duct7H = hd14; Duct8H = hd15;
	//	AreaLong = hd14;
//AfxMessageBox("ReadPDPiput line 6 complete");

//"7th line -  DPLY(id), mm -horiz.displacement of diaphragms axes (misalignement)"	
	//	if (fgets(buf,1024,fin) == NULL) { fclose(fin); return 0; }
	//	double hd1, hd2, hd3, hd4, hd5, hd6, hd7, hd8, hd9, hd10, hd11, hd12, hd13, hd14;
		if (fgets(buf,1024,fin) == NULL) { fclose(fin); return 0; }
		result = sscanf(buf, " %le %le %le %le %le %le %le %le %le %le %le %le %le %le %le", 
				&hd1, &hd2, &hd3, &hd4, &hd5, &hd6, &hd7, &hd8, &hd9, &hd10,
				&hd11, &hd12, &hd13, &hd14, &hd15);
		if (result != 15) {
			reply = AfxMessageBox("Problem with line 7! Not all data found.\n"+ quest, 3);
			if (reply != IDYES) { fclose(fin);return 0;}
		}
		
		NeutrBiasInHor = hd1 * 0.001; NeutrBiasOutHor = hd2 * 0.001; 
		RIDBiasInHor = hd3 * 0.001; RIDBiasOutHor = hd4 * 0.001; 
	
		DiaBiasHor = hd5 * 0.001; 
	
		LinerBiasInHor = hd6 * 0.001; LinerBiasOutHor = hd7 * 0.001; 
		 
		Duct1BiasHor = hd8 * 0.001; Duct2BiasHor = hd9 * 0.001; 
		Duct3BiasHor = hd10 * 0.001;// 10 -  used
		Duct4BiasHor = hd11 * 0.001; Duct5BiasHor = hd12 * 0.001; 
		Duct6BiasHor = hd13 * 0.001; Duct7BiasHor = hd14 * 0.001;
		Duct8BiasHor = hd15 * 0.001;
//AfxMessageBox("ReadPDPiput line 7 complete");

//"8th line DPLZ(id), mm -vertic.displacement of diaphragms axes (misalignement)"
	//	if (fgets(buf,1024,fin) == NULL) { fclose(fin); return 0; }

		if (fgets(buf,1024,fin) == NULL) { fclose(fin); return 0; }
		result = sscanf(buf, " %le %le %le %le %le %le %le %le %le %le %le %le %le %le %le", 
				&hd1, &hd2, &hd3, &hd4, &hd5, &hd6, &hd7, &hd8, &hd9, &hd10, 
				&hd11, &hd12, &hd13, &hd14, &hd15);
		if (result != 15) {
			reply = AfxMessageBox("Problem with line 8! Not all data found.\n"+ quest, 3);
			if (reply != IDYES) { fclose(fin);return 0;}
		}
		
		NeutrBiasInVert = hd1 * 0.001; NeutrBiasOutVert = hd2 * 0.001; 
		RIDBiasInVert = hd3; RIDBiasOutVert = hd4; 
	
		DiaBiasVert = hd5 * 0.001; 
	
		LinerBiasInVert = hd6 * 0.001; LinerBiasOutVert = hd7 * 0.001; 
		 
		Duct1BiasVert = hd8 * 0.001; Duct2BiasVert = hd9 * 0.001; 
		Duct3BiasVert = hd10 * 0.001;// 10 -  used
		Duct4BiasVert = hd11 * 0.001; Duct5BiasVert = hd12 * 0.001; 
		Duct6BiasVert = hd13 * 0.001; Duct7BiasVert = hd14 * 0.001;
		Duct8BiasVert = hd15 * 0.001;

//AfxMessageBox("ReadPDPiput line 8 complete");

		double AYmis, AZmis, XHaim, Wrpl, Reion, Fr, Ainc, Atil;
//if (OldInput == 0) {// New PDP-11
/*"9th line - "NBL" : AYmis,AZmis,XHaim,(Reion),Fr,Ainc,Atil"													
"       AYmis=Horiz.beam angular misalignment, mrad"													
"       AZmis=Vertical beam angular misalignment, mrad"													
        XHaim=Distance to channels horiz.aiming, m
"       /// Wrpl=Thickness of the RID plates, mm"	- removed 26.03.07												
"       ///Reion=Reionization, " removed from PDP-11													
        Fr=Neutralisation efficiency													
"       Ainc=Beam (Duct) average axis inclination angle, mrad"														
"       Atil= beam vertical tilting angle, mrad"
*/		

		if (fgets(buf,1024,fin) == NULL) { fclose(fin); return 0; }
		result = sscanf(buf, " %lf %lf %lf %lf %lf %lf", 
				&AYmis, &AZmis, &XHaim, &Fr, &Ainc, &Atil);
		if (result != 6) {
			reply = AfxMessageBox("Problem with line 9! Not all data found. \n"+ quest, 3);
			if (reply != IDYES) { fclose(fin);return 0;}
		}
		BeamMisHor = AYmis * 0.001;
		BeamMisVert = AZmis * 0.001;
		BeamAimHor = XHaim;
		//RIDTh = Wrpl * 0.001;
		
	/*	if (Reion > 10) {
			reply = AfxMessageBox("Reionization is > 10%! \n Old PDP format?\n"+ quest, 3);
			if (reply != IDYES) { fclose(fin);return 0;}
		}
		ReionPercent = Reion;*/

		NeutrPart = Fr;
		VertInclin = Ainc * 0.001;
		BeamVertTilt = Atil * 0.001;


	// removed by PAA request
		/*	if (NeutrChannels != 4 || RIDChannels != 4) {
			reply = AfxMessageBox("Recalculate Channels Width for Neutralizer and RID?", 3);
			if (reply == IDYES)  SetChannelWidth();
		}*/ 
//}// NEW PDP-11 input
/*	
	else {//OLD input PDP-10

	"9th line - "NBL" : AYmis,AZmis,XHaim,Wrpl,Reion,Fr,Ainc,Atil"													
"       AYmis=Horiz.beam angular misalignment, mrad"													
"       AZmis=Vertical beam angular misalignment, mrad"													
        XHaim=Distance to channels horiz.aiming, m
"       ///Wrpl=Thickness of the RID plates, mm"	- removed 26.03.07 (before PDP-10)												
"       Reion=Reionization, "													
        Fr=Neutralisation efficiency													
"       Ainc=Beam (Duct) average axis inclination angle, mrad"														
"       Atil= beam vertical tilting angle, mrad"
	
		if (fgets(buf,1024,fin) == NULL) { fclose(fin); return 0; }
		result = sscanf(buf, " %le %le %le %le %le %le %le", 
				&AYmis, &AZmis, &XHaim, &Reion, &Fr, &Ainc, &Atil);
		if (result != 7) {
			reply = AfxMessageBox("Problem with line 9! Not all data found. \n"+ quest, 3);
			if (reply != IDYES) { fclose(fin);return 0;}
		}
		BeamMisHor = AYmis * 0.001;
		BeamMisVert = AZmis * 0.001;
		BeamAimHor = XHaim;
	//	RIDTh = Wrpl * 0.001; // before PDP-10
		
		if (Reion > 10) {
			reply = AfxMessageBox("Reionization value is > 10%! \n Too Old PDP format maybe?\n"+ quest, 3);
			if (reply != IDYES) { fclose(fin);return 0;}
		}
		ReionPercent = Reion;
		NeutrPart = Fr;
		VertInclin  = Ainc * 0.001;
		BeamVertTilt = Atil * 0.001;

	}//OLD input PDP-10*/	
//AfxMessageBox("ReadPDPiput line 9 complete");

double Th1=0, Th2=0, Th3=0, XBHaim = 0, XVaim = 0;
if (OldInput == 0) {// New PDP
	/*10th line - For information only: Th1, Th2, Th3, XBHaim, XVaim 
	Th1=Thickness of the neutraliser entry plates, mm
	Th2=Thickness of the neutraliser exit plates, mm
"   Th3=Thickness of the RID plates, mm"
	Xbh aim=Horizontal aiming of beamlets in the channel														
    Xvaim=Vertical aiming of segments	*/
		
		if (fgets(buf,1024,fin) == NULL) { fclose(fin); return 0; }
		result = sscanf(buf, " %lf %lf %lf %lf %lf", &Th1, &Th2, &Th3, &XBHaim, &XVaim);
		if (result != 5) {
			reply = AfxMessageBox("Problem with line 10!\n Old PDP format? \n" + quest, 3);
			if (reply != IDYES) { fclose(fin);return 0;}
		}

		if (Th1 > 1.e-3) NeutrInTh = Th1 * 0.001; 
		if (Th2 > 1.e-3) NeutrOutTh = Th2 * 0.001;
		if (Th3 > 1.e-3) RIDTh = Th3 * 0.001;
		AppertAimHor = XBHaim;
		BeamAimVert = XVaim;
}

else { // Old PDP
/*10th line - For information only: Th1, Th2, Th3 - this line isded from PDP-10 version
	Th1=Thickness of the neutraliser entry plates, mm
	Th2=Thickness of the neutraliser exit plates, mm
"      Th3=Thickness of the RID plates, mm"*/
		
		if (fgets(buf,1024,fin) == NULL) { fclose(fin); return 0; }
		result = sscanf(buf, " %le %le %le", &Th1, &Th2, &Th3);
		if (result != 3) {
			reply = AfxMessageBox("Problem with line 10!\n Old PDP format? \n"+ quest, 3);
			if (reply != IDYES) { fclose(fin);return 0;}
		}

		if (Th1 > 1.e-3) NeutrInTh = Th1 * 0.001; 
		if (Th2 > 1.e-3) NeutrOutTh = Th2 * 0.001;
		if (Th3 > 1.e-3) RIDTh = Th3 * 0.001;
} // old PDP
//AfxMessageBox("ReadPDPiput line 10 complete");
	
	fclose(fin);
	
		char OpenDirName[1024];
		::GetCurrentDirectory(1024, OpenDirName);
		CurrentDirName = OpenDirName; 
		
		PDPgeomFileName  = name;
		SetTitle(PDPgeomFileName);
	
	//	AfxMessageBox("ReadPDPiput complete");
		return 1;
}

int CBTRDoc::ReadPDPinput(char * name)
{
	char buf[1024];
	FILE * fin;
	CString quest = "Read next data anyhow?";

	int result, reply;
	fin = fopen(name, "r");
	
/*1st line : "Source" : Eo, Iacc, Acy, Acz, Ah, Ch, dYa, dZa
		Eo - Beam energy, MeV,
		Iacc - Beam Current, A
		Acy - Beam core horizontal divergence angle, mrad,
		Acz - Beam core vertical divergence angle, mrad,
		Ah - Beam "halo" angle, mrad
		Ch - Part of beam current in "halo"
		dYa - horizontal step between aperture axes in beam group, mm
		dZa - vertical step between aperture rows in beam group, mm*/
	double Eo, Iacc, Acy, Acz, Ah, Ch, dYa, dZa;
	if (fgets(buf, 1024, fin) == NULL) { AfxMessageBox("Problem 0 with file"); fclose(fin); return 0; }
	result = sscanf(buf, "%lf %lf %lf %lf %lf %lf %lf %lf", 
						&Eo, &Iacc, &Acy, &Acz, &Ah, &Ch, &dYa, &dZa);
	if (result < 8) { AfxMessageBox("Problem in line 1"); fclose(fin); return 0; }
	IonBeamEnergy = Eo;
	IonBeamCurrent = Iacc;
	BeamCoreDivY = Acy * 0.001;
	BeamCoreDivZ = Acz * 0.001;
	BeamHaloDiverg = Ah * 0.001;
	BeamHaloPart = Ch;
	AppertStepHor = dYa * 0.001;
	AppertStepVert = dZa * 0.001;
		

/*2nd line : "Numbers" : Lch, Kbg, Nach, Mach, Nrch, Idm
		Lch - number of neutraliser channels
		Kbg - number of beam groups in column
		Na - number of apertures columns in a beam group
		Ma - number of apertures rows in a beam group
		Nrch - number of RID channels(equal Lch or 1)
		Idm - total number of diaphragms(not more 20)*/
	double Lch, Kbg, Nach, Mach, Nrch, Idm;
	if (fgets(buf, 1024, fin) == NULL) { AfxMessageBox("Problem 1 with file"); fclose(fin); return 0; }
	result = sscanf(buf, "%lf %lf %lf %lf %lf %lf", &Lch, &Kbg, &Nach, &Mach, &Nrch, &Idm);
	if (result < 6) { AfxMessageBox("Problem in line 2"); fclose(fin); return 0; }
	NeutrChannels = Lch;
	NofChannelsHor = Lch;
	NofChannelsVert = Kbg;
	
	NofBeamletsHor = Nach;
	NofBeamletsVert = Mach;
	RIDChannels = Nrch;

//3th line : YSo(l), mm - horiz.coordinate of beam group column axis at X = 0
	double ys0, ys1, ys2, ys3;
	if (fgets(buf, 1024, fin) == NULL) { fclose(fin); return 0; }
	result = sscanf(buf, " %le %le %le %le", &ys0, &ys1, &ys2, &ys3);
	if (result < 4) result = sscanf(buf, " %le %le", &ys0, &ys1);
	if (result !=4 && result !=2) { AfxMessageBox("Problem in line 3"); fclose(fin); return 0; }
	SegmStepHor = (ys1 - ys0) * 0.001;

//4nd line : ZSo(k), mm - vertic.coordinate of beam group axis at X = 0
	double zs0, zs1, zs2, zs3;
	if (fgets(buf, 1024, fin) == NULL) { fclose(fin); return 0; }
	result = sscanf(buf, " %le %le %le %le", &zs0, &zs1, &zs2, &zs3);
	if (result != 4) { AfxMessageBox("Problem in line 4"); fclose(fin); return 0; }
	SegmStepVert = (zs1 - zs0) * 0.001;
	

//5th line : Xd(id), m - distance to an "id" diaphragm
//	(id = 1...2 - neutralizer;  id = 3...4 - RID; id = 5...6 - scraper; id = 7...Idm - Duct)
	double xd1, xd2, xd3, xd4, xd5, xd6, xd7, xd8, xd9, xd10, xd11, xd12, xd13, xd14, xd15;
	if (fgets(buf, 1024, fin) == NULL) { fclose(fin); return 0; }
	result = sscanf(buf, " %le %le %le %le %le %le %le %le %le %le %le %le %le %le %le",
		&xd1, &xd2, &xd3, &xd4, &xd5, &xd6, &xd7, &xd8, &xd9,
		&xd10, &xd11, &xd12, &xd13, &xd14, &xd15);

	NeutrInX = xd1;
	NeutrOutX = xd2;
	NeutrXmax = xd2 + 0.01;
	NeutrXmin = Max(0.0, xd1 - 0.1);
	RIDInX = xd3;
	RIDOutX = xd4;
	PreDuctX = xd5;
	DDLinerInX = xd6;
	DDLinerOutX = xd7;
	Duct1X = xd8; Duct2X = xd9; Duct3X = xd10;// 10 - used
	Duct4X = xd11; Duct5X = xd12; Duct6X = xd13;
	Duct7X = xd14; Duct8X = xd15;
	AreaLong = xd15;
	ReionXmin = NeutrXmax;
	ReionXmax = AreaLong;

//6th line : Wd(id), mm - horiz.width of diaphragms
/*	if (fgets(buf, 1024, fin) == NULL) { fclose(fin); return 0; }
	result = sscanf(buf, " %le %le %le %le %le %le %le %le %le %le %le %le %le %le %le",
		&xd1, &xd2, &xd3, &xd4, &xd5, &xd6, &xd7, &xd8, &xd9,
		&xd10, &xd11, &xd12, &xd13, &xd14, &xd15);*/
	double wd1, wd2, wd3, wd4, wd5, wd6, wd7, wd8, wd9, wd10, wd11, wd12, wd13, wd14, wd15;
	if (fgets(buf, 1024, fin) == NULL) { fclose(fin); return 0; }
	result = sscanf(buf, " %le %le %le %le %le %le %le %le %le %le %le %le %le %le %le",
		&wd1, &wd2, &wd3, &wd4, &wd5, &wd6, &wd7, &wd8, &wd9, &wd10, &wd11, &wd12, &wd13, &wd14, &wd15);
	
	NeutrInW = wd1 * 0.001;
	NeutrOutW = wd2 * 0.001;
	RIDInW = wd3 * 0.001;
	RIDOutW = wd4 * 0.001;
	PreDuctW = wd5 * 0.001;
	DDLinerInW = wd6 * 0.001;
	DDLinerOutW = wd7 * 0.001;
	Duct1W = wd8 * 0.001;  Duct2W = wd9 * 0.001;  Duct3W = wd10 * 0.001;// 10 - used now
	Duct4W = wd11 * 0.001; Duct5W = wd12 * 0.001; Duct6W = wd13 * 0.001;
	Duct7W = wd14 * 0.001; Duct8W = wd15 * 0.001;

//7th line : Hd(id), m - vert.hight of diaphragms
	/*if (fgets(buf, 1024, fin) == NULL) { fclose(fin); return 0; }
	result = sscanf(buf, " %le %le %le %le %le %le %le %le %le %le %le %le %le %le %le",
		&xd1, &xd2, &xd3, &xd4, &xd5, &xd6, &xd7, &xd8, &xd9,
		&xd10, &xd11, &xd12, &xd13, &xd14, &xd15);*/
	double hd1, hd2, hd3, hd4, hd5, hd6, hd7, hd8, hd9, hd10, hd11, hd12, hd13, hd14, hd15;
	if (fgets(buf, 1024, fin) == NULL) { fclose(fin); return 0; }
	result = sscanf(buf, " %le %le %le %le %le %le %le %le %le %le %le %le %le %le %le",
		&hd1, &hd2, &hd3, &hd4, &hd5, &hd6, &hd7, &hd8, &hd9,
		&hd10, &hd11, &hd12, &hd13, &hd14, &hd15);
	
	NeutrH = hd1;
	//NeutrOutH = hd2;
	RIDH = hd3;
	//	RIDOutH = hd4;
	PreDuctH = hd5;
	DDLinerInH = hd6;
	DDLinerOutH = hd7;
	Duct1H = hd8; Duct2H = hd9; Duct3H = hd10;// 10 -  used
	Duct4H = hd11; Duct5H = hd12; Duct6H = hd13;
	Duct7H = hd14; Duct8H = hd15;

//8th line : DPLY(id), mm - horiz.displacement of diaphragms axes
	if (fgets(buf, 1024, fin) == NULL) { fclose(fin); return 0; }
	result = sscanf(buf, " %le %le %le %le %le %le %le %le %le %le %le %le %le %le %le",
		&xd1, &xd2, &xd3, &xd4, &xd5, &xd6, &xd7, &xd8, &xd9,
		&xd10, &xd11, &xd12, &xd13, &xd14, &xd15);

//9th line : DPLZ(id), mm - vertic.displacement of diaphragms axes
	if (fgets(buf, 1024, fin) == NULL) { fclose(fin); return 0; }
	result = sscanf(buf, " %le %le %le %le %le %le %le %le %le %le %le %le %le %le %le",
		&xd1, &xd2, &xd3, &xd4, &xd5, &xd6, &xd7, &xd8, &xd9,
		&xd10, &xd11, &xd12, &xd13, &xd14, &xd15);

/*10th line : AYmis, AZmis, XYaim, XZaim, XBYaim, XBZaim, Trpl, Ainc, Atil, Fr
		AYmis - Horizontal beam angular misalignment, mrad
		AZmis - Vertical beam angular misalignment, mrad
		XYaim - Distance to channels horizontal aiming, m
		XZaim - Distance to segments vertical aiming, m
		XBYaim - Distance to beamlets horizontal aiming, m
		XBZaim - Distance to beamlets vertical aiming, m
		Trpl - Thickness of the RID plates, mm
		Ainc - Beam(Duct) nominal axis inclination angle, mrad
		Atil - Beam vertical tilting angle, mrad
		Fr - Neutralisation efficiency*/
	double AYmis, AZmis, XYaim, XZaim, XBYaim, XBZaim, Trpl, Ainc, Atil, Fr;
	if (fgets(buf, 1024, fin) == NULL) { AfxMessageBox("Problem 10 with file"); fclose(fin); return 0; }
	result = sscanf(buf, "%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf", 
		&AYmis, &AZmis, &XYaim, &XZaim, &XBYaim, &XBZaim, &Trpl, &Ainc, &Atil, &Fr);
	if (result < 10) { AfxMessageBox("Problem in line 10"); fclose(fin); return 0; }

	BeamMisHor = AYmis * 0.001;
	BeamMisVert = AZmis * 0.001;
	BeamAimHor = XYaim;
	BeamAimVert = XZaim;
	AppertAimHor = XBYaim;
	AppertAimVert = XBZaim;
	RIDTh = Trpl * 0.001;
	VertInclin = Ainc * 0.001;
	BeamVertTilt = Atil * 0.001;
	NeutrPart = Fr;

	fclose(fin);

	char OpenDirName[1024];
	::GetCurrentDirectory(1024, OpenDirName);
	CurrentDirName = OpenDirName;

	PDPgeomFileName = name;
	SetTitle(PDPgeomFileName);
	
/*	if ((int)BeamSplitType == 0) SetPolarBeamletCommon(); //SetBeamletAt(0);
	else SetGaussBeamlet(BeamCoreDivY, BeamCoreDivZ);
	SetBeam(); // Aimings, IS positions
			   //SetSINGAP()
	SetStatus();
	SetPlates();*/
	return 1;
}


void CBTRDoc:: ShowPlatePoints(bool draw)
{
	MSG message;
	CString S;
	CPlate * plate = pMarkedPlate;
	if (plate == NULL) return;
	CLoadView * pLV = (CLoadView *) pLoadView;
	CDC * pDC = pLV->GetDC();

	CArray <minATTR> & falls = plate->Falls;
	int Nfalls = falls.GetSize();
	//vector <minATTR> & arr = m_GlobalVector;// m_AttrVector[MaxThreadNumber - 1];
	//pSetView->Load = NULL;
	int x, y;
	
	//pSetView->InvalidateRect(NULL, TRUE); // clear old profiles
	
	pLV->STOP = FALSE;
	//pLV->InvalidateRect(NULL, TRUE);
	
	C3Point Pgl, Ploc;
	double power;
		
	double left = plate->leftX;
	double bot = plate->botY;
	
	int Npos = 0, Nneg = 0, Natom = 0;
	double Ppos = 0, Pneg = 0, Patom = 0;
	
	//CRect rect;
	//pLV->GetClientRect(rect);
	//pDC->Rectangle(rect);
	//pLV->UpdateScales(rect, plate->Xmax, plate->Ymax); // NEW
	//pLV->ShowPlateBound(pDC);// done before
	
	COLORREF color;
	CPen * pen = &pMainView->BlackPen;
	CPen * pOldPen = pDC->SelectObject(pen);
	CFont * pOldFont = pDC->SelectObject(&(pLV->smallfont));
	//for (int k = 0; k < ThreadNumber; k++) {
		//arr = m_AttrVector[k];
		for (int i = 0; i < Nfalls; i++) {
			minATTR & tattr = falls[i];
			if (tattr.Nfall == plate->Number) {
				if (!OptAtomPower && tattr.Charge == 0) continue;
				if (!OptNegIonPower && tattr.Charge < 0 ) continue;
				if (!OptPosIonPower && tattr.Charge > 0) continue;
				power = tattr.PowerW;
				Ploc.X = tattr.Xmm * 0.001;// plate->GetLocal(Pgl);
				Ploc.Y = tattr.Ymm * 0.001;
				Ploc.Z = 0;
				x = pLV->OrigX + (int)((Ploc.X - left) * pLV->ScaleX);
				y = pLV->OrigY - (int)((Ploc.Y - bot) * pLV->ScaleY);
				switch (tattr.Charge) {
					case -1: //pen = &pMainView->GreenPen; 
						color = RGB(0,255,0); 
						Nneg++;	Pneg += tattr.PowerW;
						break;
					case 0: //pen = &pMainView->BluePen; 
						color = RGB(0,0, 255);
						Natom++;	Patom += tattr.PowerW;
						break;
					case 1: //pen = &pMainView->RosePen; 
						color = RGB(255,0, 0);
						Npos++; Ppos += tattr.PowerW;
						break;
					default : //pen = &pMainView->BlackPen; 
						color = RGB(0,0,0);
				}
				//if (!pen.CreatePen(PS_SOLID, 1, color)) return;
				//pDC->SelectObject(pen);
				if (draw) { // show points
					pDC->SetPixel(x, y, color);
					pDC->SetPixel(x-1, y-1, color);
					pDC->SetPixel(x-1, y+1, color);
					pDC->SetPixel(x+1, y-1, color);
					pDC->SetPixel(x+1, y+1, color);
				}
				//pDC->Ellipse(x-1, y-1, x+1, y+1);
				//plate->Load->Distribute(Ploc.X, Ploc.Y, power);
				
			} // equal numbers
			if (::PeekMessage(&message, NULL, 0,0, PM_REMOVE)) {
				::TranslateMessage(&message);
				::DispatchMessage(&message);
			}
			if (pLV->STOP) break;
		} // i
		//if (pLV->STOP) break;
	//} //k

	if (Nfalls < 1) { // show power
		//plate->Load->SetSumMax();
		S.Format("ATOMS %10.3g,  NEG %10.3g,  POS %10.3g [W]     ", 
			plate->AtomPower, plate->NegPower, plate->PosPower);
		x = pLV->OrigX + 10;
		y = pLV->OrigY + 20; 
		pDC->TextOutA(x, y, S);
		S.Format("Total power = %g W", plate->Load->Sum);
		y = pLV->OrigY + 40; //- (int)((plate->topY - bot) * pLV->ScaleY) + 10;
		pDC->TextOutA(x, y, S);
		S.Format("Max PD = %g W/m2", plate->Load->MaxVal);
		y = pLV->OrigY + 60; //- (int)((plate->topY - bot) * pLV->ScaleY) + 30;
		pDC->TextOutA(x, y, S);
	}
	else {// Nfalls > 0
		S.Format("ATOMS  %d / %gW      ", Natom, Patom);
		x = pLV->OrigX + 20;
		y = pLV->OrigY + 20; 
		pDC->TextOutA(x, y, S);
		S.Format("NEG IONS %d / %gW       ", Nneg, Pneg);
		y = pLV->OrigY + 40; 
		pDC->TextOutA(x, y, S);
		S.Format("POS IONS %d / %gW      ", Npos, Ppos);
		y = pLV->OrigY + 60; 
		pDC->TextOutA(x, y, S);
	}
	

	pDC->SelectObject(pOldPen);
	pDC->SelectObject(pOldFont);
	pLV->ReleaseDC(pDC);
	pMainView->ShowNBLine();
	pMainView->ShowCoord();

}

void  CBTRDoc:: SetLoadArray(CPlate* plate, bool flag)
{
	CPlate* pl;
	int i, N;
	if (plate == NULL && !flag)  { // Unselect all plates
		Load_Arr.RemoveAll();
		LoadSelected = 0;
		return;
	}
	if (plate == NULL && flag)  {
		for (i = 0; i <LoadSelected; i++) { // calculate Max or all selected plates
			pl = Load_Arr[i];
			//pl->Load->SetSumMax();
		}
		SortLoadArrayAlongX();
		return;
	}
	if (plate != NULL && flag)  { // Add the plate to Selected List
		//if (plate->Number > 0 && plate->Number <= LoadSelected) return; 
		
		Load_Arr.Add(plate); LoadSelected++;
		//plate->Number = LoadSelected;
		for (int i = 0; i < LoadSelected; i++) {
			pl = Load_Arr[i];
			/*if (pl->Number == plate->Number) {
				Load_Arr.RemoveAt(i, 1);
				LoadSelected--;
				break;
			}*/
		}
		SortLoadArrayAlongX();
		
		//N = plate->Number;
		//int ind = 0; // pos to insert
		/*if (LoadSelected == 0) Load_Arr.Add(plate);
		else if (Load_Arr[LoadSelected-1]->Number < plate->Number) Load_Arr.Add(plate);
		else {
		for (int i = 0; i < LoadSelected; i++) {
			pl = Load_Arr[i];
			if (pl->Number > plate->Number) {
				Load_Arr.InsertAt(i, plate, 1);
				break;
			}
		}
		}*/
		


		// sort plates on x
	/*	for (i = N-1; i >0; i--) {
			pl = Load_Arr[i-1];//.GetAt(i-1);
			if (plate->Orig.X < pl->Orig.X) 
			{ 
				plate->Number = i; 
				pl->Number = i+1; 
				Load_Arr[i-1] = plate;
				Load_Arr[i] = pl;
			} 
		}*/
		return;
	}

	if (plate != NULL && !flag)  {  // Remove the plate from the Selected List
		if (LoadSelected == 0) return;
		for (int i = 0; i < LoadSelected; i++) {
			pl = Load_Arr[i];
			if (pl->Number == plate->Number) {
				Load_Arr.RemoveAt(i, 1);
				LoadSelected--;
				break;
			}
		}
		SortLoadArrayAlongX();
		
	//	Load_Arr.RemoveAt(plate->Number - 1);
	/*	N = plate->Number; // remove plate
		pl =  Load_Arr[N-1];//.GetAt(N-1);
		//pl->Number = 0;
		pl->filename = "";
		for (i = N-1; i < LoadSelected-1; i++) { // shift numbers after
			pl = Load_Arr[i+1];//.GetAt(i+1);
			//Number--;
			Load_Arr[i] = pl;//.SetAt(i, pl);
		}
		Load_Arr.SetSize(--LoadSelected);*/
		
		//SortLoadArrayAlongX();
		return;
	}
}

void  CBTRDoc::SortLoadArrayAlongX()
{
	if (Sorted_Load_Ind.GetSize() > 1) // sorted already
	{
		Sorted_Load_Ind.RemoveAll(); // unsort
		//OnShow();
		//return;
	}

	int maxind = Load_Arr.GetUpperBound();
	if (maxind < 0) return;
	int ind = 0;
	CPlate * pl, * pl0;
	double x0, x;
	BOOL found;
	Sorted_Load_Ind.Add(ind);
	pl = Load_Arr[0];
	int n, sortn;
	
	for (int k = 1; k <= maxind; k++) {
		ind = k;
		pl = Load_Arr[k];
		n = pl->Number;
		found = FALSE;
		while (!found && ind > 0) {
			ind--;
			sortn = Load_Arr[Sorted_Load_Ind[ind]]->Number;
			//if (pl->Orig.X >= Load_Arr[Sorted_Load_Ind[ind]]->Orig.X) {
			if (n >= sortn) {
				found = TRUE;
				if (n > sortn) Sorted_Load_Ind.InsertAt(ind+1, k, 1);
				else { Load_Arr.RemoveAt(k, 1); LoadSelected--; } // n == sortn
			}
		}
		if (!found) Sorted_Load_Ind.InsertAt(0, k, 1);
	}
	//OnShow();

}

void CBTRDoc:: DetachLoadFiles()
{
	CPlate * plate;
	for (int i = 0; i < LoadSelected; i++) {
			plate = Load_Arr[i];
			plate->filename = "";
		}
}

void CBTRDoc::SaveLoads(bool free) // SINGLE
{
//	CWaitCursor wait; 
	CPlate * plate;

	LPSECURITY_ATTRIBUTES  sec = NULL;//SEC_ATTRS;

	::SetCurrentDirectory(CurrentDirName);
	CTime t = StopTime; // CTime::GetCurrentTime();
	CString DirName;
	DirName.Format("BTR%02d%02d%02d", t.GetHour(), t.GetMinute(), t.GetSecond()); 
		::CreateDirectory(DirName, sec);
		::SetCurrentDirectory(DirName);

	FILE * fout;
	CString name;
	CString  sn;
	// Save Loads --------------------------------
	if (free) {
		POSITION pos = PlatesList.GetHeadPosition();
		while (pos != NULL) {
			plate = PlatesList.GetNext(pos);
			sn.Format("%d",plate->Number);
			name = "load" + sn + ".txt"; 
			fout = fopen(name, "w");
			plate->WriteLoadAdd(fout); // free
			plate->filename = name;
			fclose(fout);
		}
	} // free
	else { //standard
	for (int i = 0; i < LoadSelected; i++) {
		plate = Load_Arr[i];
		if (plate->Touched == FALSE) continue; // skip ZERO maps
		//CString  sn;
		sn.Format("%d",plate->Number);
		name = "load" + sn + ".txt"; 
		fout = fopen(name, "w");
		//if (free) plate->WriteLoadAdd(fout);
		plate->WriteLoad(fout);
		plate->filename = name;
		fclose(fout);
	}
	}// standard

	// Save Config File --------------------------------
		CurrentDirName =  DirName;
		FormDataText(TRUE);// include internal names of parameters
	
		name = "Config.txt";
		fout = fopen(name, "w");
		fprintf(fout, m_Text);
		fclose(fout);

		//WriteSumReiPowerX("Sum_ReiX.txt", -1, 100);
		//WriteSumAtomPowerX("Sum_AtomX.txt", -1, 100);

		//WriteSumPowerAngle("Sum_Angle.dat");
			
		// WriteExitVector();
		//WriteFallVector();
	
		::SetCurrentDirectory(CurrentDirName);//("..\\");
		SetTitle(DirName);// +" - " + TaskName);
		OnDataActive();
	
		if (free) AfxMessageBox("FREE geometry and PD results are stored in Folder " + DirName, 
					MB_ICONINFORMATION | MB_OK);
		else AfxMessageBox("STANDARD geometry and PD results are stored in Folder " + DirName, 
					MB_ICONINFORMATION | MB_OK);
}

void CBTRDoc:: WriteSumReiPowerX(char * name, float xmin, float xmax)
{
	FILE * fout;
	fout= fopen(name, "w");
	if (fout == NULL) {
			AfxMessageBox("failed to create file", MB_ICONSTOP | MB_OK);
			return;
	}
	fprintf(fout, "ReIon Power deposition via X-coordinate \n");
	fprintf(fout, " X \t Power, W \n");

	double x, power, Sum = 0, SumN = 0, SumRID = 0, SumCal = 0, SumDuct = 0;
	for (int k = 0; k < SumReiPowerX.GetSize(); k++) {
		x = SumPowerStepX * (k + 0.5);
		power = SumReiPowerX[k];
		if (x >= xmin && x <= xmax)
			fprintf(fout, " %g \t %g \n", x, power);
		Sum += power;
		if (OptFree) continue; // skip sums
		if (x >= NeutrInX && x <= NeutrOutX) SumN += power; 
		if (x >= RIDInX && x <= RIDOutX) SumRID += power; 
		if (x >= CalInX && x <= CalOutX) SumCal += power; 
		if (x >= PreDuctX && x <= Duct8X) SumDuct += power; 
	}
	fprintf(fout, "Total reionized power = %e W   (X = 0 - %g m)\n", Sum, AreaLong);
	if (!OptFree) fprintf(fout, "Neutralizer = %e W   (X = %g - %g m)\n", SumN, NeutrInX, NeutrOutX);
	if (!OptFree) fprintf(fout, "RID  = %e W     (X = %g - %g m)\n", SumRID, RIDInX, RIDOutX);
	if (!OptFree) fprintf(fout, "Calorimeter = %e W  (X = %g - %g m)\n", SumCal, CalInX, CalOutX);
	if (!OptFree) fprintf(fout, "DUCT = %e W     (X = %g - %g m)\n", SumDuct, PreDuctX, Duct8X);

	fclose(fout);

}

void CBTRDoc:: WriteSumAtomPowerX(char * name, float xmin, float xmax)
{
	FILE * fout;
	fout= fopen(name, "w");
	if (fout == NULL) {
			AfxMessageBox("failed to create file", MB_ICONSTOP | MB_OK);
			return;
	}
	fprintf(fout, "Atom Power deposition via X-coordinate \n");
	fprintf(fout, " X \t Power, W \n");

	double x, power, Sum = 0, SumN = 0, SumRID = 0, SumCal = 0, SumDuct = 0;
	for (int k = 0; k < SumAtomPowerX.GetSize(); k++) {
		x = SumPowerStepX * (k + 0.5);
		power = SumAtomPowerX[k];
		if (x >= xmin && x <= xmax)
			fprintf(fout, " %g \t %g \n", x, power);
		Sum += power;
		if (OptFree) continue; // skip sums
		if (x >= NeutrInX && x <= NeutrOutX) SumN += power; 
		if (x >= RIDInX && x <= RIDOutX) SumRID += power; 
		if (x >= CalInX && x <= CalOutX) SumCal += power; 
		if (x >= PreDuctX && x <= Duct8X) SumDuct += power; 
	}
	fprintf(fout, "Total deposited atom power = %e W   (X = 0 - %g m)\n", Sum, AreaLong);
	if (!OptFree) fprintf(fout, "Neutralizer = %e W   (X = %g - %g m)\n", SumN, NeutrInX, NeutrOutX);
	if (!OptFree) fprintf(fout, "RID  = %e W     (X = %g - %g m)\n", SumRID, RIDInX, RIDOutX);
	if (!OptFree) fprintf(fout, "Calorimeter = %e W  (X = %g - %g m)\n", SumCal, CalInX, CalOutX);
	if (!OptFree) fprintf(fout, "DUCT = %e W     (X = %g - %g m)\n", SumDuct, PreDuctX, Duct8X);

	fclose(fout);

}

void CBTRDoc:: WriteSumPowerAngle(char * name)
{
	FILE * fout;
	fout= fopen(name, "w");
	if (fout == NULL) {
			AfxMessageBox("failed to create file", MB_ICONSTOP | MB_OK);
			return;
	}
	fprintf(fout, "Power deposition via falling angle  \n");
	fprintf(fout, " Angle, grad  \t Power, W \n");

	double angle, power, Sum = 0;
	//double x, power, Sum = 0, SumN = 0, SumRID = 0, SumCal = 0, SumDuct = 0;
	for (int l = 0; l < SumPowerAngle.GetSize(); l++) {
		angle = SumPowerAngleStep * (l + 1);
		power = SumPowerAngle[l];
		fprintf(fout, " %g \t %g \n", angle, power);
		Sum += power;
	}
	fprintf(fout, "Total power = %g W   (0 - 90 grad)\n", Sum);
	
	fclose(fout);

}

void CBTRDoc::DeleteContents() 
{
	SegmCentreHor.RemoveAll();
	SegmCentreVert.RemoveAll();
	ActiveCh.RemoveAll();
	ActiveRow.RemoveAll();
	RectIS.RemoveAll();
	RectISrow.RemoveAll();
	BeamletAngleHor.RemoveAll();
	BeamletAngleVert.RemoveAll();
	BeamletPosHor.RemoveAll();
	BeamletPosVert.RemoveAll();

//	MFdata.RemoveAll();
//	MFXdata.RemoveAll();
	FWdata.RemoveAll();
	DecayArray.RemoveAll();
	DecayPathArray.RemoveAll();

	DataComment.RemoveAll();
	SkipSurfClass.RemoveAll(); // list of substrings to find in plate Comment
	ExceptSurf.RemoveAll(); // exceptions in skip
	DataName.RemoveAll();
	DataType.RemoveAll();
	DataValue.RemoveAll();

	Attr_Array.RemoveAll();
	Attr_Array_Reion.RemoveAll();
	Attr_Array_Resid.RemoveAll();
	BeamEntry_Array.RemoveAll();
	PolarAngle.RemoveAll();
	PolarCurrent.RemoveAll();
	
	ReionArray.RemoveAll(); //ReionArrayX.RemoveAll();	ReionArrayCurr.RemoveAll();
	NeutrArray.RemoveAll();
	PlasmaProfileNTZ.RemoveAll();
	PlasmaProfilePSI.RemoveAll();
	Load_Arr.RemoveAll();
	
	while (!PlatesList.IsEmpty()) {
		delete (PlatesList.RemoveHead());
	}

	while (!AddPlatesList.IsEmpty()) {
		delete (AddPlatesList.RemoveHead());
	}

	Singap_Array.RemoveAll();
	Exit_Array.RemoveAll();
	VolumeVector.clear();
	ExitArray.RemoveAll();
	PlasmaImpurA.RemoveAll();
	PlasmaImpurW.RemoveAll();
		
	ClearArrays();//skip ClearScenLoadTracks(); 
	if (MAXSCEN > 0) {
		for (int i = 0; i <= MAXSCEN; i++)	ScenData[i].RemoveAll();
		ClearScenLoadTracks(); 
		for (int i = 0; i < MAXSCENLIMIT; i++) delete[] ScenLoadSummary[i];
		delete[] ScenLoadSummary;
	}
	PowSolid.RemoveAll();
	PowNeutral.RemoveAll();
	PowInjected.RemoveAll();

	Reflectors.RemoveAll();
	ReflectorNums.RemoveAll();
		
	//if (ParticlesFile != NULL) fclose(ParticlesFile);
	CDocument::DeleteContents();
}

void  CBTRDoc:: SetVShifts()
{
	if (OptFree) return;
	double Xc;
	Xc = 0.5 * (NeutrInX + NeutrOutX);
	VShiftNeutr = Xc * tan(VertInclin);
	Xc = 0.5 * (RIDInX + RIDOutX);
	VShiftRID = Xc * tan(VertInclin);
	Xc = 0.5 * (CalInX + CalOutX);
	VShiftCal = Xc * tan(VertInclin);
	Xc = PreDuctX;// Scraper1
	VShiftDia = Xc * tan(VertInclin);
	Xc = DDLinerInX; // Scraper2
	VShiftLinerIn = Xc * tan(VertInclin);
	Xc = DDLinerOutX; // Duct0
	VShiftLinerOut = Xc * tan(VertInclin);
	Xc = Duct1X;
	VShiftDuct1 = Xc * tan(VertInclin);
	Xc = Duct2X;
	VShiftDuct2 = Xc * tan(VertInclin);
	Xc = Duct3X;
	VShiftDuct3 = Xc * tan(VertInclin);
	Xc = Duct4X;
	VShiftDuct4 = Xc * tan(VertInclin);
	Xc = Duct5X;
	VShiftDuct5 = Xc * tan(VertInclin);
	Xc = Duct6X;
	VShiftDuct6 = Xc * tan(VertInclin);
	Xc = Duct7X;
	VShiftDuct7 = Xc * tan(VertInclin);
	Xc = Duct8X;
	VShiftDuct8 = Xc * tan(VertInclin);

}

////-------------  BEAM --------------------------
void  CBTRDoc:: SetSINGAPfromMAMUG()
{
	BeamEntry_Array.RemoveAll();
	BEAMLET_ENTRY be;

	int i, j, ii, jj;
	int Ny = (int)NofBeamletsHor, Nz = (int)NofBeamletsVert; // MAMuG
	int NSy = (int)NofChannelsHor, NSz = (int)NofChannelsVert; // MAMuG
	int	Ntotal = Ny*Nz*NSy*NSz;
	double SumPower = 0;
	
	// double BeamPower = IonBeamPower; // MW

	
	for (i = 0; i<NSy;  i++) { ///NofChannelsHor;
		if (!ActiveCh[i]) continue;
			
	for (j = 0; j<NSz; j++) {  ///NofChannelsVert;
	if (!ActiveRow[j]) continue;	
	
	for (ii = 0; ii<Ny; ii++)  // NofBeamletsHor
	for (jj = 0; jj<Nz; jj++) // NofBeamletsVert
	{			
		be.Active = TRUE;// initially
		be.PosY = BeamletPosHor[i*Ny + ii]; // MAMuG
		be.PosZ = BeamletPosVert[j*Nz + jj]; // MAMuG
		be.AlfY = BeamletAngleHor[i*Ny + ii];
		be.AlfZ = BeamletAngleVert[j*Nz + jj];
		be.DivY = BeamCoreDivY;
		be.DivZ = BeamCoreDivZ;
		be.Fraction = 1000000 * IonBeamPower / Ntotal;
		be.i = ii; be.j = jj;
		SumPower += be.Fraction;
		BeamEntry_Array.Add(be);
		
	} // ii,jj
	} //j
	} // i
	NofBeamletsTotal = Ntotal;
	NofBeamlets = BeamEntry_Array.GetSize();// active
	logout << "SINGAP beam is set from MAMUG\n";
	logout << "Total beam power, W " << SumPower << "\n";
}

void  CBTRDoc:: SetSINGAP()
{
//	if (!OptSINGAP) return;

	CString S;
	FILE * fold;
	FILE * fnew;
	int res, reply;
	char oldname[1024], newname[1024];

	if (!SINGAPLoaded) {
		strcpy(oldname, "Beamlets.txt"); // old name
		strcpy(newname, BeamFileName); // new BEAM_BERT.txt

		if ((fold = fopen(oldname, "r")) == NULL) { // old file not found
			// try new name 
			if ((fnew = fopen(BeamFileName, "r")) == NULL) {
				S.Format("Define file to attach the Beam\n <CANCEL> will activate MAMuG default settings");
				reply = AfxMessageBox(S, 3);
				if (reply != IDYES ) { SINGAPLoaded = FALSE; OptSINGAP = FALSE; return;}
				else ReadSINGAP();
			} // new file not found also
			else { // new file found
				fclose(fnew);
				res = ReadBeamletsOld(newname);
				if (res >=1) { SINGAPLoaded = TRUE; BeamFileName = newname; }
			}// new file found
		} // file not found
		else { // old file found
			fclose(fold);
			res = ReadBeamletsOld(oldname);
			if (res >=1) { SINGAPLoaded = TRUE; BeamFileName = oldname;}
		}
	} // !SINGAPLoaded
	
	BeamEntry_Array.RemoveAll();

	BEAMLET_ENTRY be;

	int N = Singap_Array.GetSize();
	if (N <1) return; 
	
	if (!SINGAPLoaded) {
		return;
	}

	// account of misalignment, tilting, inclination

	for (int k = 0; k < N; k++) {
		be = Singap_Array.GetAt(k);
		be.AlfY += BeamMisHor; //mrad -> rad
		be.AlfZ += BeamMisVert + BeamVertTilt;
		//if  (!OptFree) 
		be.AlfZ += VertInclin; // NBI standard
	
	/*	int i, j;// = (int)((be.PosY + 0.32) / 0.16); // channel number

		/// set active chanels	
		iseg = 0;
		double Dmin = fabs(be.PosY - SegmCentreHor[0]);
		for (i = 1; i < (int)NofChannelsHor; i++) {// find iseg by closest centre
			//HorCentre = SegmCentreHor[i];
			//if (SegmStepHor > 1.e-6) maxhor = SegmStepHor * 0.5;
			if (fabs(be.PosY - SegmCentreHor[i]) < Dmin ) {
				Dmin = fabs(be.PosY - SegmCentreHor[i]);
				iseg = i; 
			}
		}
		//// set active rows
		jseg = 0;
		Dmin = fabs(be.PosZ - SegmCentreVert[0]);
		for (j = 1; j <(int)NofChannelsVert; j++) {
			//VertCentre = SegmCentreVert[j];
		//	if (fabs(be.PosZ - VertCentre) < SegmStepVert * 0.5) {
			if (fabs(be.PosZ - SegmCentreVert[j]) < Dmin ) {
				Dmin = fabs(be.PosZ - SegmCentreVert[j]);
				jseg = j; 
			}
		}
	
		be.Active = ActiveCh[iseg] * ActiveRow[jseg];
	*/
		be.Active = TRUE; // for SINGAP all beamlets are active!!!
		BeamEntry_Array.Add(be);// with fraction account
	}
	NofBeamletsTotal = BeamEntry_Array.GetSize();
	NofBeamlets = NofBeamletsTotal;
//	OptSINGAP = TRUE;
}

void CBTRDoc:: ReadSINGAP()
{
	char name[1024];
	int res;

//	SINGAPLoaded = FALSE;
	
	CFileDialog dlg(TRUE, "dat; txt | * ", "", OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"Beamlets data (*.txt);(*.dat)  | *.txt; *.TXT; *.dat; *.DAT | All Files (*.*) | *.*||", NULL);
	if (dlg.DoModal() == IDOK) {
			strcpy(name, dlg.GetPathName());
		
			CPreviewDlg pdlg;
			pdlg.m_Filename = name;
			if (pdlg.DoModal() == IDCANCEL) {
			//	OptSINGAP = FALSE;
			//	SINGAPLoaded = FALSE;
				AfxMessageBox("No Beam accepted", MB_ICONINFORMATION | MB_OK);
				return;
			}

			if (!IsBeamletsFile(name)) { 
			//	OptSINGAP = FALSE;
			//	SINGAPLoaded = FALSE; 
				AfxMessageBox("No Beam accepted", MB_ICONINFORMATION | MB_OK);
				return; 
			}
			
			if (OldBeamletsFormat(name) == TRUE) 
				res = ReadBeamletsOld(name);// BERT's beam 10 col
			else 
				res = ReadBeamletsBTR(name); // my format for PLV 09/2021
				//ReadBeamletsNew(name);// AIK's beam 9 col
			
			if (res < 1)	{
			//	OptSINGAP = FALSE;
			//	SINGAPLoaded = FALSE;
				AfxMessageBox("Invalid data format",	MB_ICONSTOP | MB_OK); 
				
			}
			else  {
				SINGAPLoaded = TRUE;
				SetTitle(name);
				BeamFileName = name;
				OptSINGAP = TRUE;
				TaskName = "SINGAP BEAM";
				SetSINGAP(); // form BEAMLET_ENTRY array (apply midalign, active chan)
							
				
			}
	} // IDOK
	else {
	//	OptSINGAP = FALSE;
	//	SINGAPLoaded = FALSE;
		AfxMessageBox("No Beam accepted", MB_ICONINFORMATION | MB_OK);
		
	}
	return;

}

bool CBTRDoc::IsBeamletsFile(char * name)
{
	CString S;
	CString line;
	char buf[1024];
	FILE * fin;
	int reply, values, length;
	if ((fin= fopen(name, "r")) == NULL) {
		AfxMessageBox("Failed to open file", MB_ICONINFORMATION | MB_OK);
		return FALSE;
	}
	if (fgets(buf, 1024, fin) == NULL) { 
		AfxMessageBox("Invalid data format", MB_ICONSTOP | MB_OK); 
		fclose(fin); 
		return FALSE;
	}

//	char *buff, *nptr, *endptr;
	double v1, v2, v3, v4, v5, v6, v7;

/*	int length = text.GetLength()*2;
	buff = (char *)calloc(length, sizeof(char));
	strcpy(buff, text);
	
	nptr = buff;

	for (j = 0; j <= Ny; j++)
	for (i = 0; i <= Nx; i++) {
		val = strtod(nptr,&endptr);
		plate0->Load->Val[i][j] = val;
		if (nptr == endptr) nptr++; //catch special case
		else nptr = endptr;
	}
*/	
	line = "";
	length = 0;
	BOOL datagot = FALSE;
	while (!feof(fin) && !datagot) {

		if (fgets(buf, 1024, fin) == NULL) break; // read  line
		if (sscanf(buf, "%le", &v1) < 1) {
			datagot = FALSE; 
			continue;
		}
		
		if (sscanf(buf, "%le %le", &v1, &v2) < 2) {
			datagot = TRUE; 
			values = 1;
		}
		if (sscanf(buf, "%le %le %le", &v1, &v2, &v3) < 3) {
			datagot = TRUE; 
			values = 2;
		}
		if (sscanf(buf, "%le %le %le %le", &v1, &v2, &v3, &v4) < 4) {
			datagot = TRUE; 
			values = 3;
		}
		if (sscanf(buf, "%le %le %le %le %le", &v1, &v2, &v3, &v4, &v5) < 5) {
			datagot = TRUE; 
			values = 4;
		}
		if (sscanf(buf, "%le %le %le %le %le %le", &v1, &v2, &v3, &v4, &v5, &v6) < 6) {
			datagot = TRUE; 
			values = 5;
		}
		if (sscanf(buf, "%le %le %le %le %le %le %le", &v1, &v2, &v3, &v4, &v5, &v6, &v7) < 7) {
			datagot = TRUE; 
			values = 6;
		}
		if (sscanf(buf, "%le %le %le %le %le %le %le", &v1, &v2, &v3, &v4, &v5, &v6, &v7) == 7) {
			datagot = TRUE; 
			values = 7;
		}
		line = buf;
	}
	fclose(fin);

	if (values < 7) {
		 S.Format("Are you sure this is Beamlets array file? \n 1st data string found:\n %s", line);
		 reply =  AfxMessageBox(S, 3);
		 if (reply == IDYES) return TRUE;
		 else return FALSE;
	 }
	
	return TRUE;
			
}

bool CBTRDoc:: OldBeamletsFormat(char * name) // check if this is Bert's file - 10 columns
{
	CString line;
	char buf[1024];
	FILE * fin;
	if ((fin= fopen(name, "r")) == NULL) {
		AfxMessageBox("Failed to open file", MB_ICONINFORMATION | MB_OK);
		return FALSE;
	}
	if (fgets(buf, 1024, fin) == NULL) { 
		AfxMessageBox("Invalid data format", MB_ICONSTOP | MB_OK); 
		fclose(fin); 
		return FALSE;
	}
	line = buf;
	line.MakeUpper();
	int pos1 = line.Find("START");
	int pos2 = line.Find("POS-X");
	int pos3 = line.Find("WID-X");
	int pos4 = line.Find("ALF-X");

	if (pos1 >= 0 && pos2 >= 0 && pos3 >= 0 && pos4 >= 0) { fclose(fin);	return TRUE;}
	
	if (fgets(buf, 1024, fin) == NULL) { 
		AfxMessageBox("Invalid line 2", MB_ICONSTOP | MB_OK); 
		fclose(fin);
		return FALSE;
	}
	double a,b,c,d,e,f,g,h,i,j;
	int res  = sscanf(buf, "%le %le %le %le %le %le %le %le %le %le", &a, &b, &c, &d, &e, &f, &g, &h, &i, &j);
	if (res < 10) {fclose(fin); return FALSE;}
	if (a < 1.e-6) { fclose(fin); return TRUE;}

	
	fclose(fin);
	return FALSE;

}

int CBTRDoc:: ReadBeamletsOld(char * name) // BERT de ESHE file (old)
// START  POS-X  WID-X  ALF-X  DIV-X  POS-Y  WID-Y  ALF-Y  DIV-Y   FRACTION
{
	Singap_Array.RemoveAll();
	BEAMLET_ENTRY be;
	char buf[1024];
	double st, px, wx, ax, dx, py, wy, ay, dy, fr;
	int total, result;
	CString S;
	double SumFract = 0;

	FILE * fin = fopen(name, "r");
	fgets(buf, 1024, fin); // read  line
	total = 0;
	while (!feof(fin)) {
		result = sscanf(buf, "%le %le %le %le %le %le %le %le %le %le", &st, &px, &wx, &ax, &dx, &py, &wy, &ay, &dy, &fr);
	//	result = fscanf(fin, "%le %le %le %le %le %le %le %le %le %le", &st, &px, &wx, &ax, &dx, &py, &wy, &ay, &dy, &fr);
		if (result == 10) {
			be.PosY = px * 0.001; //mm -> m
			be.PosZ = py * 0.001;
			be.AlfY = ax * 0.001;// mrad -> rad
			be.AlfZ = ay * 0.001;// 
			be.DivY = dx * 0.001;
			be.DivZ = dy * 0.001;
			be.Fraction = fr; // =1
			if (total == 0) { // set common 
				BeamCoreDivY = dx * 0.001;
				BeamCoreDivZ = dy * 0.001;
			}
			//int i  = (int)((be.PosY + 0.32) / 0.16); // channel number
			be.Active = TRUE;
			Singap_Array.Add(be);
			total++;
			SumFract += fr;
			fgets(buf, 1024, fin); // read next line
		} // scanned 10 fields
		else {
			fgets(buf, 1024, fin); // read next line
		}
	} // eof
	fclose(fin);

	if (total > 0) {
		OptSINGAP = TRUE;
		S.Format("%d beamlets accepted \n file %s", total, name);
		AfxMessageBox(S);
		logout << S << " <<<<<<<<<<<< \n";
	}
	
	if (total > 0) {
		logout << " Sum = " << SumFract << "\n";
		logout << " Each bml current will be normalized to IonBeamCurrent " << IonBeamCurrent << " A \n";
		for (int i = 0; i < total; i++)
		{
			fr = Singap_Array[i].Fraction; // can be zero current!
			Singap_Array[i].Fraction = fr * IonBeamCurrent / total;
		}

	}

	return total;
	
}

int CBTRDoc::ReadBeamletsBTR(char * name) // similar to BERT format (10 columns)
//******  Beam array data created by BTR 5 ***********
//N     POS - Y   WID - Y    ALF - Y       DIV - Y      POS - Z     WID - Z     ALF - Z     DIV - Z     Rel FRACT
{
	Singap_Array.RemoveAll();
	BEAMLET_ENTRY be;
	char buf[1024];
	double n, posy, widy, ay, divy, posz, widz, az, divz, fr;
	int bml, result;
	CString S;
	double SumFract = 0;

	FILE * fin = fopen(name, "r");
	fgets(buf, 1024, fin); // read  line
	bml = 0;
	while (!feof(fin)) {
		result = sscanf(buf, "%le %le %le %le %le %le %le %le %le %le", &n, &posy, &widy, &ay, &divy, &posz, &widz, &az, &divz, &fr);

		if (result == 10) {
			be.PosY = posy * 0.001; //mm -> m
			be.PosZ = posz * 0.001;
			be.AlfY = ay * 0.001;// mrad -> rad
			be.AlfZ = az * 0.001;// 
			be.DivY = divy * 0.001;
			be.DivZ = divz * 0.001;
			be.Fraction = fr; // =1
			if (bml == 0) { // set divs common from 1st bml 
				BeamCoreDivY = divy * 0.001;
				BeamCoreDivZ = divz * 0.001;
			}
			//int i  = (int)((be.PosY + 0.32) / 0.16); // channel number
			be.Active = TRUE;
			Singap_Array.Add(be);
			bml++;
			SumFract += fr;
			fgets(buf, 1024, fin); // read next line
		} // scanned 10 fields
		else {
			fgets(buf, 1024, fin); // read next line
		}
	} // eof
	fclose(fin);

	if (bml > 0) {
		OptSINGAP = TRUE;
		S.Format(" %d beamlets accepted from file %s", bml, name);
		AfxMessageBox(S);
		logout << S << " <<<<<<<<<<<< \n";
	}

	if (bml > 0) {
		//double AverFract = SumFract / bml; // normally should be 1
		//if (AverFract < 1.e-12) AverFract = 1;
		// NormFract = be.Fraction / AverFract 
		logout << " Sum = " << SumFract << "\n";
		logout << " Each bml current is normalized to IonBeamPower " << IonBeamPower << " MW \n";

		if (SumFract < 1.e-6) SumFract = 1;
		for (int i = 0; i < bml; i++)
		{
			fr = Singap_Array[i].Fraction; // can be zero current!
			Singap_Array[i].Fraction = fr * IonBeamPower * 1000000 / SumFract;
		}
	}

	return bml;
}

int CBTRDoc:: ReadBeamletsNew(char * name) // AIK file (new) - 9 columns
//"Yo"	"AY"	"AcY"	"Zo"	"AZ"	"AcZ"	rI	iY		iZ
// Yo	Ay	    Acy	    Zo	    Az  	 Acz   FrI	column	row
{
	Singap_Array.RemoveAll();
	BEAMLET_ENTRY be;
	char buf[1024];
	double Y0, Ay, Acy, Z0, Az, Acz, FrI;
	int iy, jz;
	int valid, total, comment, result, maxIy, maxJz;
	CString S;
	double SumFract = 0;

	FILE * fin = fopen(name, "r");
	if (fin == NULL) { 
		S.Format("Beam array %s not found!", name);
		AfxMessageBox(S);
		return 0;
	}

	fgets(buf, 1024, fin); // read  line

	total = 0;	valid = 0;	comment = 0;
	maxIy = 0; maxJz = 0;

	while (!feof(fin)) {
		result = sscanf(buf,"%le %le %le %le %le %le %le %d %d", 
						&Y0, &Ay, &Acy, &Z0, &Az, &Acz, &FrI, &iy, &jz );

						/*	result = fscanf(fin, "%le %le %le %le %le %le %le ", 
							&Y0, &Ay, &Acy, &Z0, &Az, &Acz, &FrI);*/

		if (result == 9) { 
			if (FrI <= 1.e-6) { // check-up for zero beamlet
				total++; 
				fgets(buf, 1024, fin);  
				continue;
			}
			be.PosY = Y0 * 0.001; //mm -> m
			be.PosZ = Z0 * 0.001;
			be.AlfY = Ay * 0.001;// mrad -> rad
			be.AlfZ = Az * 0.001;// 
			be.DivY = Acy * 0.001;
			be.DivZ = Acz * 0.001;
			be.Fraction = FrI; // =1
			
			if (iy < 0) iy = 0;
			if (jz < 0) jz = 0; 
			be.i = iy; be.j = jz;
			if (valid == 0) { // set common 
				BeamCoreDivY = Acy * 0.001;
				BeamCoreDivZ = Acz * 0.001;
			}
			//int i  = (int)((be.PosY + 0.32) / 0.16); // channel number
			be.Active = TRUE;
			Singap_Array.Add(be);
			valid++;
			total++;
			SumFract += FrI;
			maxIy = Max(maxIy, iy);
			maxJz = Max(maxJz, jz);
			fgets(buf, 1024, fin); // read  line
		} // scanned 9 fields
	
		else { // comments line
			fgets(buf, 1024, fin); // read  line
			comment++;
			total++;
		}
			
	} // eof
	fclose(fin);


	S.Format(" Non-zero beamlets - %d \n"
			"Max Horiz. index - %d\n"
			"Max Vert. index - %d\n\n"
			"Total lines in file - %d\n"
			"Comments or dubious lines - %d\t\n", 
			valid, maxIy, maxJz, total, comment);
	AfxMessageBox(S);

	if (valid < 1) return 0;

	double AverFract = SumFract / valid; // normally should be 1
	if (AverFract < 1.e-12) AverFract = 1;
	// NormFract = be.Fraction / AverFract 
	for (int i = 0; i < valid; i++) 
		Singap_Array[i].Fraction /= AverFract;

/*	BOOL isRegular = CheckRegularity(maxIy, maxJz); // SINGAP or MAMuG ?
	if (isRegular) { 
		OptSINGAP = FALSE;
//		ActiveCh.SetSize(10);
//		ActiveRow.SetSize(10);
//		SetBeam();
//		SetStatus();
		S.Format("Beam array is Regular and Symmetrical!\nactive option - MAMuG\n\nDetails:\n\n"
			"  Segments %d  Hor.Step %g  Focus %g\t\n"
			"  Groups   %d  Vert.Step %g Focus %g\n\n"
			"  beamlets %d x %d\n"
			"  steps hor/vert  %g x %g\n"
			"  focus hor/vert  %g x %g\n", 
			(int)NofChannelsHor, SegmStepHor, BeamAimHor, 
			(int)NofChannelsVert, SegmStepVert, BeamAimVert,
			(int)NofBeamletsHor, (int)NofBeamletsVert, AppertStepHor, AppertStepVert,
			  AppertAimHor, AppertAimVert); 
		AfxMessageBox(S);
	}
	else { */
		OptSINGAP = TRUE;
		SINGAPLoaded = TRUE;
		BeamFileName = name;
	//}

	return valid; //total;
}

double Round(double inval, int digits) // total non-zero digits
{
	double aval = fabs(inval);
	double val, outval;
	int ival;
	double imax = pow(10.0, digits);
	double imin = pow(10.0, digits-1); 
	int k;
	double ord;
	
	if (aval >= imin && aval <= imax) {
		ival = (int)inval; 
		if (ival - inval >= 0.5) ival--; 
		if (inval - ival >= 0.5) ival++; 
		outval = (double)ival;
	}
	
	else if (aval < imin) {
		k = 0;
		val = inval;
		while (val < imin) { val *= 10; k++; }
		ival = (int)val;
		if (ival - val >= 0.5) ival--;
		if (val - ival >= 0.5) ival++;
		ord = pow(10.0, k);
		outval = (double)ival / ord;
	} // aval < imin

	else {//aval > imax
		k = 0;
		val = inval;
		while (val > imax) {val *= 0.1; k++; }
		ival = (int)val;
		if (ival - val >= 0.5) ival--;
		if (val - ival >= 0.5) ival++;
		ord = pow(10.0, k);
		outval = (double)ival * ord;
	}

	//outval = (double)ival;
	return outval;

}

bool CBTRDoc:: CheckRegularity(int Iy, int Jz) // SINGAP or MAMuG ? - not called now
{
	CString S;

	CArray <double, double> posY;
	CArray <double, double> posZ;
	CArray <double, double> aimY;
	CArray <double, double> aimZ;
	double hy, hz, Hy, Hz, distAy, distAz, DistAy, DistAz; 
	int ny, nz, Ny, Nz;
	double y, z, ay, az; //positions / aimings of scrutinized beamlet
	BEAMLET_ENTRY be;
	double dPos = 0.001, dAim = 0.001; // precisions 
	double maxPos, maxAim; // precisions base 

	if (Iy*Jz == 1) { // 1 beamlet
		be = Singap_Array.GetAt(0);
		if (fabs(be.PosY) < 0.001*dPos && fabs(be.PosZ) < 0.001*dPos 
			&& fabs(be.AlfY) < 0.001*dAim && fabs(be.AlfZ) < 0.001*dAim) 
		{
			NofChannelsHor = 1;	NofChannelsVert = 1;
			NofBeamletsHor = 1;	NofBeamletsVert = 1;
			SegmStepHor = 0;		SegmStepVert = 0;
			AppertStepHor = 0;		AppertStepVert = 0;
			BeamAimHor = AreaLong;	BeamAimVert = AreaLong;
			AppertAimHor = 999;	AppertAimVert = 999;
		//	S.Format("Single Regular beamlet found\n active option - MAMuG"); 
		//	AfxMessageBox(S);
			return 1;
		} // regular beamlet
		else 
		{
			S.Format("Single Beamlet found\n with Non-Zero Position and/or Angle \n\n active option - SINGAP"); 
			AfxMessageBox(S);
			return 0;
		} // non-regular beamlet
	}
//////// check if Total Beamlets = Iy * Jz
	int total = Singap_Array.GetSize();
	if (total != Iy * Jz) {
		S.Format(" Total Number of beamlets (%d) does not match Max Indexes (%d x %d)!\n active option - SINGAP", 
			total, Iy, Jz);
		AfxMessageBox(S);
		return (0); // non-regular
	}
	
////////// check positions regularity 
	int k, indI, indJ;
	maxPos = 0; maxAim = 0;
	for (k = 0; k < total; k++) { // fill the base arrays - all beamlets will be compared with it
		be = Singap_Array.GetAt(k);
		y = be.PosY; z = be.PosZ;
		ay = be.AlfY; az = be.AlfZ;
		maxPos = Max(maxPos, fabs(y)); maxPos = Max(maxPos, fabs(z));
		maxAim = Max(maxAim, fabs(ay)); maxAim = Max(maxAim, fabs(az));
		if (be.i > posY.GetSize()) { posY.Add(y); aimY.Add(ay);}
		if (be.j > posZ.GetSize()) { posZ.Add(z); aimZ.Add(az);}
	}
	if (posY.GetSize() != Iy || posZ.GetSize() != Jz) {
		S.Format(" Horiz. or Vert. Index mismatch!\n active option - SINGAP"); 
		AfxMessageBox(S);
		return (0); // non-regular
	}
//	dPos = 0.001 * maxPos;
//	dAim = 0.001 * maxAim;

	for (k = 0; k < total; k++) { // compare beamlet pos/aim with base values
		be = Singap_Array.GetAt(k);
		y = be.PosY; z = be.PosZ;
		ay = be.AlfY; az = be.AlfZ;
		indI = be.i-1; indJ = be.j-1;
		if (fabs(y - posY[indI]) > dPos * maxPos || fabs(z - posZ[indJ]) > dPos * maxPos) {
			S.Format(" Non-regular Beamlets positions!\n active option - SINGAP\n\n i = %d, j = %d", 
						indI+1, indJ+1); 
			AfxMessageBox(S);
			return (0); // non-regular positions
		}
		if (fabs(ay - aimY[indI]) > dAim * maxAim || fabs(az - aimZ[indJ]) > dAim * maxAim) {
			S.Format(" Non-regular Beamlets aimings!\n active option - SINGAP\n\n i = %d, j = %d",
						indI+1, indJ+1); 
			AfxMessageBox(S);
			return (0); // non-regular aimings
		}
	} // k

	double y0, y1, z0, z1;
	double dymin, dymax, dzmin, dzmax;
	int n0 = 0;

////////// horizontal check -> Ny, ny, Hy, hy
	dymin = Max(maxPos, 1000.);	dymax = 0; 
	y0 = posY[0]; 
	for (k = 1; k < Iy; k++) { // define steps Y 
		y1 = posY[k];
		dymin = Min(dymin, fabs(y1-y0));
		dymax = Max(dymax, fabs(y1-y0));
		y0 = y1;
	}

	if (Iy == 1) { dymin = 0; dymax = 0; }

	if (fabs(dymax - dymin) < dPos * maxPos) { // segment = 1 (vert. channel) 
		Ny = 1; ny = Iy;
		Hy = 0; hy = dymin;
		DistAy = AreaLong; dymax = 0;
		//distAy = - posY[0] / atan(aimY); 
	} // segment = 1
	
	else { // segments >1
		ny = 1; Ny = 1;
		y0 = posY[0]; 
		
		for (k = 1; k < Iy; k++) {
			y1 = posY[k];
			//d1 = - posY[k] / atan(aimY[k]);
			if (fabs(fabs(y1-y0) - dymin) < dPos * maxPos) ny++; // next aperture within segment
			else if (fabs(fabs(y1-y0) - dymax) < dPos * maxPos) { // next segment
				if (Ny == 1) n0 = ny;
				else if (ny != n0) {
					S.Format(" Beamlets horiz. number within segment is not Const!\n active option - SINGAP"); 
					AfxMessageBox(S);
					return (0); // beamlet horizontal number not const
				}
				ny = 1; Ny++; 
			} // next segment
			else { 
				S.Format(" Beamlets horizontal steps mismatch!\n active option - SINGAP\n\n k = %d", k+1); 
				AfxMessageBox(S);
				return (0); // non-regular horizontal steps
			}
			y0 = y1;
		} // k
		hy = dymin; 
		Hy = dymax + dymin * (ny - 1);

	} // segments >1



///// Aimings Y (Segments, Beamlets) //////////////////////////
	int kseg;
	double Pos0; // segment centre //ay = - (posY[0] - Y0) / atan(aimY[0]); 
	double Dist0, Dist1; // segment axis aiming dist
	double dist0, dist1; // beamlet aiming dist

	Pos0 = (posY[0] + posY[ny - 1]) * 0.5; // 1st segment
	if (fabs(aimY[0] - aimY[ny-1]) < dAim * maxAim) dist0 = 999;
	else	dist0 = (posY[ny-1] - posY[0]) / (aimY[0] - aimY[ny-1]);
	if (fabs(posY[0] - Pos0 + tan(aimY[0]) * dist0) < dPos * maxPos) Dist0 = 999;
	else  Dist0 = -Pos0 * dist0 / (posY[0] - Pos0 + tan(aimY[0]) * dist0); 

	for (kseg = 1; kseg < Ny; kseg++) {
		if (Odd(Ny) && kseg == (Ny-1)/2) continue; // central column - zero aiming angle!
		Pos0 = (posY[kseg*ny] + posY[kseg*ny + ny - 1]) * 0.5; 
		if (fabs(aimY[kseg*ny] - aimY[kseg*ny + ny-1]) < dAim * maxAim) dist1 = 999;
		else	dist1 = (posY[kseg*ny + ny-1] - posY[kseg*ny]) / (aimY[kseg*ny] - aimY[kseg*ny + ny-1]);
		if (fabs(posY[kseg*ny] - Pos0 + tan(aimY[kseg*ny]) * dist1) < dPos * maxPos) Dist1 = 999;
		else  Dist1 = -Pos0 * dist1 / (posY[kseg*ny] - Pos0 + tan(aimY[kseg*ny]) * dist1); 
		
		if (fabs(Dist1 - Dist0) > dPos * Dist0) {
			S.Format(" Segments horizontal aimings mismatch!\n active option - SINGAP\n\n k = %d"
				" D0 = %g D1 = %g", kseg+1, Dist0, Dist1); 
			AfxMessageBox(S);
			return (0); // non-regular aimings
		}
		if (fabs(dist1 - dist0) > dPos * dist0) {
			S.Format(" Beamlets horizontal aimings mismatch!\n active option - SINGAP\n\n k = %d"
				" d0 = %g d1 = %g", kseg+1, dist0, dist1); 
			AfxMessageBox(S);
			return (0); // non-regular aimings
		}
	}// kseg

	DistAy = Dist0;
	distAy = dist0;


////////// vertical check -> Nz, nz, Hz, hz
	dzmin = Max(maxPos, 1000.);	dzmax = 0;
	z0 = posZ[0];
	for (k = 1; k < Jz; k++) { // define steps Z 
		z1 = posZ[k];
		dzmin = Min(dzmin, fabs(z1-z0));
		dzmax = Max(dzmax, fabs(z1-z0));
		z0 = z1;
	}

	if (Jz == 1) { dzmin = 0; dzmax = 0; }

	if (fabs(dzmax - dzmin) < dPos * maxPos) { // Single group (horizontal row) 
		Nz = 1; nz = Jz;
		Hz = 0; hz = dzmin;
		DistAz = 999; dzmax = 0; 
	//	distAz = - posZ[0] / atan(aimZ); 
	} // groups = 1

	else { //  groups >1
		nz = 1; Nz = 1;
		z0 = posZ[0]; 
		for (k = 1; k < Jz; k++) {
			z1 = posZ[k];
			if (fabs(fabs(z1-z0) - dzmin) < dPos * maxPos) nz++; // next aperture within segment
			else if (fabs(fabs(z1-z0) - dzmax) < dPos * maxPos) { // next segment
				if (Nz == 1) n0 = nz;
				else if (nz != n0) {
					S.Format(" Beamlets vert. number within segment is not Const!\n active option - SINGAP"); 
					AfxMessageBox(S);
					return (0); // beamlet horizontal number not const
				}
				nz = 1; Nz++; 
			} // next segment
			else { 
				S.Format(" Beamlets vertical steps mismatch!\n active option - SINGAP"); 
				AfxMessageBox(S);
				return (0); // non-regular vert. steps
			}
			z0 = z1;
		} // k

		hz = dzmin; 
		Hz = dzmax + dzmin * (nz - 1);
	}


///// Aimings Z  (Segments, Beamlets) /////////////////

/*	int kseg;
	double Pos0; // segment centre //ay = - (posY[0] - Y0) / atan(aimY[0]); 
	double Dist0, Dist1; // segment axis aiming dist
	double dist0, dist1; // beamlet aiming dist
*/
	Pos0 = (posZ[0] + posZ[nz-1]) * 0.5; // 1st segment
	if (fabs(aimZ[0] - aimZ[nz-1]) < dAim * maxAim) dist0 = 999;
	else	dist0 = (posZ[nz-1] - posZ[0]) / (aimZ[0] - aimZ[nz-1]);
	if (fabs(posZ[0] - Pos0 + tan(aimZ[0]) * dist0) < dPos * maxPos) Dist0 = 999;
	else  Dist0 = -Pos0 * dist0 / (posZ[0] - Pos0 + tan(aimZ[0]) * dist0); 

	for (kseg = 1; kseg < Nz; kseg++) {
		if (Odd(Nz) && kseg == (Nz-1)/2) continue; // central row - zero aiming angle!
		Pos0 = (posZ[kseg*nz] + posZ[kseg*nz + nz-1]) * 0.5; 
		if (fabs(aimZ[kseg*nz] - aimZ[kseg*nz + nz-1]) < dAim * maxAim) dist1 = 999;
		else	dist1 = (posZ[kseg*nz + nz-1] - posZ[kseg*nz]) / (aimZ[kseg*nz] - aimZ[kseg*nz + nz-1]);
		if (fabs(posZ[kseg*nz] - Pos0 + tan(aimZ[kseg*nz]) * dist1) < dPos * maxPos) Dist1 = 999;
		else  Dist1 = -Pos0 * dist1 / (posZ[kseg*nz] - Pos0 + tan(aimZ[kseg*nz]) * dist1); 
	
		if (fabs(Dist1 - Dist0) > dPos * Dist0) {
			S.Format(" Groups vertical aimings mismatch!\n active option - SINGAP\n\n k = %d"
				" D0 = %g  D1 = %g", kseg+1, Dist0, Dist1); 
			AfxMessageBox(S);
			return (0); // non-regular aimings
		}
		if (fabs(dist1 - dist0) > dPos * dist0) {
			S.Format(" Beamlets vertical aimings mismatch!\n active option - SINGAP\n\n k = %d"
				" d0 = %g  d1 = %g", kseg+1, dist0, dist1); 
			AfxMessageBox(S);
			return (0); // non-regular aimings
		}
	}// kseg

	DistAz = Dist0;
	distAz = dist0;


//////// if regular - set 12 parameters of regular beam 
	int dig = 3; // rounding precision
	NofChannelsHor = Ny;	NofChannelsVert = Nz;
	NofBeamletsHor = ny;	NofBeamletsVert = nz;
	SegmStepHor = Hy;		SegmStepVert = Hz;
	AppertStepHor = hy;	AppertStepVert = hz;
	BeamAimHor = Round(DistAy, dig);	
	BeamAimVert = Round(DistAz, dig);
	AppertAimHor = Round(distAy, dig);
	AppertAimVert = Round(distAz, dig);

	posY.RemoveAll(); posZ.RemoveAll(); 
	aimY.RemoveAll(); aimZ.RemoveAll(); 
	return (1); // regular
}

void CBTRDoc:: SetGaussBeamlet(double DivY, double DivZ) // split type = 1
{
//	CWaitCursor wait;
	CArray<double, double> SumCoreY;
	CArray<double, double> SumCoreZ;
	CArray<double, double> SumHaloY;
	CArray<double, double> SumHaloZ;
	CArray<double, double> AlfaY;
	CArray<double, double> AlfaZ;
	ATTRIBUTES Attr;
	
	Attr_Array.RemoveAll();

	if (BeamHaloDiverg < 1.e-16) BeamHaloPart = 0;
	if (DivY * DivZ < 1.e-16) BeamHaloPart = 1;

	int i, j;
	double core, halo, core0, halo0;
	double alfaY, alfaZ, alfaEff;
	double y, z, hy, hz;
	
	double divY = DivY;
	double divZ = DivZ;
	if (fabs(divY) < 1.e-16) divY = 0.00001;
	if (fabs(divZ) < 1.e-16) divZ = 0.00001;

	double AvCoreDiv = sqrt(divY * divZ);
	if (fabs(AvCoreDiv) < 1.e-16) AvCoreDiv = 0.00001; 
	double HaloDivRel = BeamHaloDiverg / AvCoreDiv;
	double HaloDivY = HaloDivRel * divY;
	double HaloDivZ = HaloDivRel * divZ;
	if (fabs(HaloDivY) < 1.e-16) HaloDivY = 0.00001;
	if (fabs(HaloDivZ) < 1.e-16) HaloDivZ = 0.00001;
		
	double IntLimitY;// = 3 * Max(divY, HaloDivY);
	double IntLimitZ;// = 3 * Max(divZ, HaloDivZ);
	if (BeamHaloPart < 1.e-3) {
		IntLimitY = SumGauss(1 - CutOffCurrent, divY);
		IntLimitZ = SumGauss(1 - CutOffCurrent, divZ);
	}
	else {
		IntLimitY = SumGauss(1 - CutOffCurrent, divY, HaloDivY, BeamHaloPart); // reduce
		IntLimitZ = SumGauss(1 - CutOffCurrent, divZ, HaloDivZ, BeamHaloPart); // reduce
	}
	double TotalCore = 1;// IntegralErr(IntLimitY, divY) * IntegralErr(IntLimitZ, divZ);
	double TotalHalo = 1;//IntegralErr(IntLimitY, BeamHaloDiverg) * IntegralErr(IntLimitZ, BeamHaloDiverg);
//	double Total =	(1-BeamHaloPart) * TotCore + BeamHaloPart * TotHalo; // 0..+inf
	int Ny = (int)BeamSplitNumberY;
	int Nz = (int)BeamSplitNumberZ;

	
	if (Ny <1) Ny = 1; 
	if (Nz <1) Nz = 1; 

	hy = 2 * IntLimitY / Ny;
	hz = 2 * IntLimitZ / Nz;

	PartDimHor = 0; PartDimVert = 0;
	if (Ny > 0) PartDimHor =   hy;// * 0.5; 
	if (Nz > 0) PartDimVert =  hz; // * 0.5;

	if (Ny == 1) { 
		SumCoreY.Add(1); SumHaloY.Add(1); AlfaY.Add(0); 
		SumCoreY.Add(0); SumHaloY.Add(0); AlfaY.Add(2*hy);
	} // i = 0
	if (Nz == 1) {
		SumCoreZ.Add(1); SumHaloZ.Add(1); AlfaZ.Add(0); 
		SumCoreZ.Add(0); SumHaloZ.Add(0); AlfaZ.Add(2*hz); 
	} // j = 0

	if (Ny > 1 && Odd(Ny)) { // 3,5,7...
	
		y = 0;
		core = exp(0.0);
		halo = exp(0.0);
		SumCoreY.Add(2*core); 
		SumHaloY.Add(2*halo); // i = 0
		AlfaY.Add(0); 
		
		for (i = 1; i <= (Ny - 1)/2 +1 ; i++) {
			y = hy * i;
			core = exp(-y*y / divY / divY);
			halo = exp(-y*y / HaloDivY / HaloDivY);
			SumCoreY.Add(core); 
			SumHaloY.Add(halo);
			AlfaY.Add(y); 
		//	core0 = core; halo0 = halo;
		} // i <= (Ny - 1)/2
	}
	if (!Odd(Ny)) { // 2,4,6...
	
		for (i = 0; i < Ny/2 + 1; i++) {
			y = hy * (i+0.5);
			core = exp(-y*y / divY / divY);
			halo = exp(-y*y / HaloDivY / HaloDivY);
			SumCoreY.Add(core); 
			SumHaloY.Add(halo);
			AlfaY.Add(y); 
		} // i < Ny/2
	}

	if (Nz > 1 && Odd(Nz)) { // 3,5,7...
		z = 0;
		core = exp(0.0);
		halo = exp(0.0);
		SumCoreZ.Add(2*core); 
		SumHaloZ.Add(2*halo); // i = 0
		AlfaZ.Add(0); 

		for (i = 1; i <= (Nz - 1)/2 + 1; i++) {
			z = hz * i;
			core = exp(-z*z / divZ / divZ);
			halo = exp(-z*z / HaloDivZ / HaloDivZ);
			SumCoreZ.Add(core); 
			SumHaloZ.Add(halo);
			AlfaZ.Add(z); 
		} // i <= (Nz - 1)/2
	}
	if (!Odd(Nz)) { // 2,4,6...

		for (i = 0; i < Nz/2 + 1; i++) {
			z = hz * (i+0.5);
			core = exp(-z*z / divZ / divZ);
			halo = exp(-z*z / HaloDivZ / HaloDivZ);
			SumCoreZ.Add(core); 
			SumHaloZ.Add(halo);
			AlfaZ.Add(z); 
		} // i < Nz/2
	}
	
	double SumCurrent = 0;
	double core1, halo1, core2, halo2;
	double curr0, curr1, curr2;

	for (i = 0; i < AlfaY.GetUpperBound(); i++) 
		for (j = 0; j < AlfaZ.GetUpperBound(); j++) {
			alfaY = AlfaY[i];// * divY;//
			alfaZ = AlfaZ[j];// * divZ;
			alfaEff = sqrt(alfaZ*alfaZ + alfaY*alfaY + 1.0);
			Attr.Vx = 1.0 / alfaEff;
			Attr.Vy = alfaY / alfaEff;
			Attr.Vz =  alfaZ / alfaEff;
			// V = 1; //automatically

			core0 = SumCoreY[i] * SumCoreZ[j] / TotalCore;
			halo0 = SumHaloY[i] * SumHaloZ[j] / TotalHalo;
			curr0 = core0 *(1 - BeamHaloPart) + halo0 * (BeamHaloPart); 
		
//			if (curr0 <= CutOffCurrent) continue;		
			
			Attr.Current = curr0;
			Attr.dCurrY = 0;
			Attr.dCurrZ = 0;

				core2 = SumCoreY[i+1] * SumCoreZ[j] / TotalCore;
				halo2 = SumHaloY[i+1] * SumHaloZ[j] / TotalHalo;
				curr2 = core2 *(1 - BeamHaloPart) + halo2 * (BeamHaloPart);
			
				if (i < 1) {// && Odd(Ny)) {	
					Attr.dCurrY = (curr2 - curr0) / curr0;
					//Attr.Current += 1 * (curr2 - curr0);
				}
				else {
						core1 = SumCoreY[i-1] * SumCoreZ[j] / TotalCore;
						halo1 = SumHaloY[i-1] * SumHaloZ[j] / TotalHalo;
						curr1 = core1 *(1 - BeamHaloPart) + halo1 * (BeamHaloPart); 
						Attr.dCurrY =  (curr2 - curr1) / curr0;
						if (fabs(Attr.dCurrY) > 2)  
							Attr.dCurrY = 2 * (curr2 - curr1)/fabs(curr2 - curr1);
				}
				
						
				core2 = SumCoreY[i] * SumCoreZ[j+1] / TotalCore;
				halo2 = SumHaloY[i] * SumHaloZ[j+1] / TotalHalo;
				curr2 = core2 *(1 - BeamHaloPart) + halo2 * (BeamHaloPart); 
				if (j < 1) { // && Odd(Nz)) { 
					Attr.dCurrZ = (curr2 - curr0) / curr0;
					//Attr.Current += 1 * (curr2 - curr0);
				}
				else  {
						core1 = SumCoreY[i] * SumCoreZ[j-1] / TotalCore;
						halo1 = SumHaloY[i] * SumHaloZ[j-1] / TotalHalo;
						curr1 = core1 *(1 - BeamHaloPart) + halo1 * (BeamHaloPart); 
						Attr.dCurrZ = (curr2 - curr1) / curr0;
						if (fabs(Attr.dCurrZ) > 1)  
							Attr.dCurrZ = 2 * (curr2 - curr1)/fabs(curr2 - curr1);
				}
				
						
			Attr.StartCurrent = Attr.Current;
			Attr.State = MOVING;
			Attr.X = 0;	Attr.Y = 0;	Attr.Z = 0;	Attr.L = 0;
				
			Attr_Array.Add(Attr);
			SumCurrent += Attr.Current;
			
			if (i > 0 || !Odd(Ny)) {
				Attr.Vy = - Attr.Vy;
				Attr.dCurrY = - Attr.dCurrY;
				Attr_Array.Add(Attr);
				SumCurrent += Attr.Current;
			}
			else if (j == 0 && Odd(Nz)) continue;

			if (j > 0 || !Odd(Nz)) {
				Attr.Vz = - Attr.Vz;
				Attr.dCurrZ = - Attr.dCurrZ;  
				Attr_Array.Add(Attr);
				SumCurrent += Attr.Current;

				if (i == 0 && Odd(Ny)) continue;

				if (i > 0 || !Odd(Ny)) {
					Attr.Vy = - Attr.Vy;
					Attr.dCurrY = - Attr.dCurrY;
					Attr_Array.Add(Attr);
					SumCurrent += Attr.Current;
				}
				
			}
			else continue;
			
		} // i,j

		for (int k = 0; k <= Attr_Array.GetUpperBound(); k++) {
			Attr_Array[k].StartCurrent = Attr_Array[k].Current / SumCurrent;
			Attr_Array[k].Current = Attr_Array[k].StartCurrent;
		}

	

/*	SumCurrent = 0;
	FILE *fout = fopen("attr.dat", "w");
	fprintf(fout, "Attributes Ny = %d Nz = %d \n", (int) BeamSplitNumberY, (int)BeamSplitNumberZ);
	fprintf(fout, " i		 X			Y		  Z		 Vx		Vy		Vz	Current  \n");
	for (k=0; k<=Attr_Array.GetUpperBound(); k++) {
		fprintf(fout, "%d  |  %12le   %12le   %12le  |  %12le   %12le   %12le | %12le  \n",
			k,  Attr_Array[k].X, Attr_Array[k].Y, Attr_Array[k].Z,
			Attr_Array[k].Vx, Attr_Array[k].Vy, Attr_Array[k].Vz,  Attr_Array[k].Current ); 
		SumCurrent += Attr_Array[k].Current;
	}
	fprintf(fout, "SumCurrent =  %g\n", SumCurrent);
	fclose(fout);
*/
	SumCoreY.RemoveAll();
	SumCoreZ.RemoveAll();
	SumHaloY.RemoveAll();
	SumHaloZ.RemoveAll();
	AlfaY.RemoveAll();
	AlfaZ.RemoveAll();

}


void CBTRDoc::SetPolarArrayBase(double DivY, double DivZ) // (Based AvCoreDiv = 1; Called for changed: Npolar, Nazim, CoreDiv/HaloDiv, HaloPart
/// Equal Angle Division: dAlfa = Const; Current = Var
{
	PolarAngle.RemoveAll();
	PolarCurrent.RemoveAll();

	double StepAngle;
	double SumCurrent;
	double SumAngle;
	double Curr;
	int i;
	
	if ((int)AzimNumber < 4) PolarNumber = 0;

	double AvCoreDiv = sqrt(DivY * DivZ);
	if (AvCoreDiv < 1.e-16) AvCoreDiv = 0.000001;
	double HaloDivRel = BeamHaloDiverg / AvCoreDiv;
//	double AlfaMax = 3 * Max(1.0, HaloDivRel); // 
	double AlfaMax;
	BOOL parbeam =  FALSE;

	if (BeamRadius < 1.e-4 || fabs(BeamFocusX) < 1.e-4) {
		// zero beamlet size at GG -> max angle limited by CutOff
		if (BeamHaloPart < 1.e-3) AlfaMax = SumGauss(1 - CutOffCurrent, 1);
		else AlfaMax = SumGauss(1 - CutOffCurrent, 1, HaloDivRel, BeamHaloPart); // reduce
	}
	else { // finite beamlet size GG -> max angle limited by apperture
		double minDiv = Min(BeamCoreDivY, BeamCoreDivZ);
		AlfaMax = atan(BeamRadius / fabs(BeamFocusX)) / minDiv;
		if (fabs(BeamFocusX) > 100) { 
			AlfaMax = SumGauss(1 - CutOffCurrent, 1);
			parbeam = TRUE;
		}
	}
	
	if  ((int)PolarNumber >= 1) StepAngle = AlfaMax /(int) PolarNumber;
	else StepAngle = 0.000001;

	PartDimHor = StepAngle * AvCoreDiv; // rectangular model
	PartDimVert = 2*PI*AvCoreDiv / (int)AzimNumber;// rectangular model
	
	SumCurrent = 0;
	if ((int)PolarNumber <1) {
		PolarAngle.Add(0); 
		PolarCurrent.Add(1);
		return;
	}
	else { // NPolar >= 1
		PolarAngle.Add(0); PolarCurrent.Add(0);
		SumCurrent = 0;

		for (int i=1; i<=(int)PolarNumber; i++) {
			SumAngle = i * StepAngle;//
			Curr = Gauss(SumAngle, 1, HaloDivRel, BeamHaloPart) - SumCurrent;
		//	if (Curr < CutOffCurrent) continue;
			PolarCurrent.Add(Curr);
			//if (parbeam) PolarAngle.Add(0);
			//else 
				PolarAngle.Add((i-0.5) * StepAngle);//
			SumCurrent += Curr;
		} // i = PolarNumber
	} //  NPolar >= 1
		
	for (i=0; i <= PolarCurrent.GetUpperBound(); i++)  {
		PolarCurrent[i] /= SumCurrent;
	}

//}
	CString S;
	int N = PolarAngle.GetSize();
	S.Format("Polar angles size %d", N);
	//AfxMessageBox(S);

}

void CBTRDoc::	SetPolarBeamletIndividual(double DivY, double DivZ)	
											// set attributes for (Npolar*Nazim)BP in Beamlet CS 
											// MAMuG; SINGAP with DivX, DivY != const for beamlets
											// Beam Misalignment = 0!
{
	ATTRIBUTES Attr;
	double AzimAngle, alfaY, alfaZ, alfaEff;
	Attr_Array.RemoveAll();

	SetPolarArrayBase(DivY, DivZ);

	if ((int)AzimNumber * (int)(PolarNumber) < 1) {// Axis Particle ONLY
			Attr.Current = PolarCurrent[0];
			Attr.StartCurrent = Attr.Current;
			Attr.Vx = 1.0;	Attr.Vy = 0; Attr.Vz = 0;
			Attr.X = 0;	Attr.Y = 0;	Attr.Z = 0;	Attr.L = 0;
			Attr_Array.Add(Attr);	
			return;
	}

	// ----- AzimNumber > 0, PolarNumber > 0 -------------------------

	for (int k = 0; k <= (int)PolarNumber; k++) {

//		if (k > 0 && PolarCurrent[k] < CutOffCurrent *(AzimNumber)) continue; // AzimNumber != 0!

		if  (k == 0) {// Axis particle
			Attr.Current = PolarCurrent[0]; //Polar group =0
			Attr.StartCurrent = Attr.Current;
			Attr.Vx = 1.0;	Attr.Vy = 0; Attr.Vz = 0;
			Attr.X = 0;	Attr.Y = 0;	Attr.Z = 0;	Attr.L = 0;
			Attr_Array.Add(Attr);
		}
		else //  Polar group !=0
		for (int n = 0; n < AzimNumber; n++)
		{
			AzimAngle = n* 2*PI /(int) AzimNumber;
			Attr.Current = PolarCurrent[k] / AzimNumber; //Polar group !=0
			Attr.StartCurrent = Attr.Current;
			// alfaX = Vx/Vx; alfaY = Vy/Vx; alfaZ = Vz/Vx
			alfaZ = DivZ * PolarAngle[k] * sin(AzimAngle);//
			alfaY = DivY * PolarAngle[k] * cos(AzimAngle);//
			alfaEff = sqrt(alfaZ*alfaZ + alfaY*alfaY + 1.0);
			Attr.Vx = 1.0 / alfaEff;
			Attr.Vy = alfaY / alfaEff;
			Attr.Vz = alfaZ / alfaEff;
			// V = 1; //automatically
			Attr.X = 0;	Attr.Y = 0;	Attr.Z = 0;	Attr.L = 0;
			Attr_Array.Add(Attr);			
		} // n = AzimNumber - 1
	} // k = PolarNumber

}


void CBTRDoc::SetPolarBeamletReion()
{
	ATTRIBUTES Attr;
	double AzimAngle, alfaY, alfaZ, alfaEff;

	// keep main values
	//double OldPolarNumber = PolarNumber;
	//double OldAzimNumber = AzimNumber;

	// set new values for reion task
	PolarNumber = PolarNumberReion; // 
	AzimNumber = AzimNumberReion; // 

	Attr_Array_Reion.RemoveAll(); // new array

	if (BeamHaloDiverg < 1.e-16) BeamHaloPart = 0;
	if ((BeamCoreDivY)* (BeamCoreDivZ) < 1.e-16) BeamHaloPart = 1;

	//AfxMessageBox("Set ReionAttr ");
	SetPolarArrayBase(BeamCoreDivY, BeamCoreDivZ);

	// ----- AzimNumber > 0, PolarNumber > 0 -------------------------

	double SumCurrent = 0;
	for (int k = 0; k <= (int)PolarNumber; k++) {

		if (k == 0) {// Axis particle
			Attr.Current = PolarCurrent[0]; // Axis particle for Plot Beamlet Foot 
			Attr.StartCurrent = Attr.Current;
			SumCurrent += Attr.Current;
			Attr.Vx = 1.0;	Attr.Vy = 0; Attr.Vz = 0;
			Attr.X = 0;	Attr.Y = 0;	Attr.Z = 0;	Attr.L = 0;
			Attr_Array_Reion.Add(Attr);// new
		}
		else //  Polar group !=0 
			for (int n = 0; n < (int)AzimNumber; n++)
			{
				double val = 0; //(double)rand() / (double) RAND_MAX;
				AzimAngle = (n + val) * 2 * PI / (int)AzimNumber;

				// alfaX = Vx/Vx; alfaY = Vy/Vx; alfaZ = Vz/Vx
				alfaZ = BeamCoreDivZ * PolarAngle[k] * sin(AzimAngle);//
				alfaY = BeamCoreDivY * PolarAngle[k] * cos(AzimAngle);//
				alfaEff = sqrt(alfaZ*alfaZ + alfaY*alfaY + 1.0);
				Attr.Vx = 1.0 / alfaEff;
				Attr.Vy = alfaY / alfaEff;
				Attr.Vz = alfaZ / alfaEff;
				// V = 1; //automatically
				Attr.X = 0;	Attr.Y = 0;	Attr.Z = 0;	Attr.L = 0;
				Attr.Current = PolarCurrent[k] / (int)AzimNumber; //Polar group !=0
				if (BeamRadius > 1.e-4) {
					if (fabs(BeamFocusX) > 1.e-4 && fabs(BeamFocusX) < 100) {
						if (sqrt(alfaY*alfaY + alfaZ*alfaZ) > BeamRadius / fabs(BeamFocusX))
							Attr.Current = 0;
					}
					else if (fabs(BeamFocusX) >= 100) { //parallel rays
						Attr.X = 0;
						Attr.Y = BeamRadius * (k / (PolarNumber)) * cos(AzimAngle);
						Attr.Z = BeamRadius * (k / (PolarNumber)) * sin(AzimAngle);
						Attr.Vx = 1; Attr.Vy = 0; Attr.Vz = 0;
					}
				}
				Attr.StartCurrent = Attr.Current;
				SumCurrent += Attr.Current;
				Attr_Array_Reion.Add(Attr);
			} // n = AzimNumber - 1
	} // k = PolarNumber

	for (int l = 0; l <= Attr_Array_Reion.GetUpperBound(); l++) {
		Attr_Array_Reion[l].StartCurrent = Attr_Array_Reion[l].Current / SumCurrent;
		Attr_Array_Reion[l].Current = Attr_Array_Reion[l].StartCurrent;
	}

	// restore defaults!!!!
	
	PolarNumber = PolarNumberAtom; //OldPolarNumber;
	AzimNumber = AzimNumberAtom;//OldAzimNumber;

	CString S;
	int Nattr = Attr_Array_Reion.GetSize();
	S.Format("Reion Attr size %d", Nattr);
	//AfxMessageBox(S);
}

void CBTRDoc::SetPolarBeamletResidual()
{
	ATTRIBUTES Attr;
	double AzimAngle, alfaY, alfaZ, alfaEff;
	
	// keep main values
	//double OldPolarNumber = PolarNumber;
	//double OldAzimNumber = AzimNumber;
	
	// set new values for residual
	PolarNumber = PolarNumberResid;
	AzimNumber = AzimNumberResid;

	Attr_Array_Resid.RemoveAll(); // new array

	if (BeamHaloDiverg < 1.e-16) BeamHaloPart = 0;
	if ((BeamCoreDivY)* (BeamCoreDivZ) < 1.e-16) BeamHaloPart = 1;

	//AfxMessageBox("Set Resid Attr 10x12");
	SetPolarArrayBase(BeamCoreDivY, BeamCoreDivZ);

	// ----- AzimNumber > 0, PolarNumber > 0 -------------------------

	double SumCurrent = 0;
	for (int k = 0; k <= (int)PolarNumber; k++) {
		
		if (k == 0) {// Axis particle
			Attr.Current = PolarCurrent[0]; // Axis particle for Plot Beamlet Foot 
			Attr.StartCurrent = Attr.Current;
			SumCurrent += Attr.Current;
			Attr.Vx = 1.0;	Attr.Vy = 0; Attr.Vz = 0;
			Attr.X = 0;	Attr.Y = 0;	Attr.Z = 0;	Attr.L = 0;
			Attr_Array_Resid.Add(Attr);// new
		}
		else //  Polar group !=0 
			for (int n = 0; n < (int)AzimNumber; n++)
			{
				double val = 0; //(double)rand() / (double) RAND_MAX;
				AzimAngle = (n + val) * 2 * PI / (int)AzimNumber;

				// alfaX = Vx/Vx; alfaY = Vy/Vx; alfaZ = Vz/Vx
				alfaZ = BeamCoreDivZ * PolarAngle[k] * sin(AzimAngle);//
				alfaY = BeamCoreDivY * PolarAngle[k] * cos(AzimAngle);//
				alfaEff = sqrt(alfaZ*alfaZ + alfaY*alfaY + 1.0);
				Attr.Vx = 1.0 / alfaEff;
				Attr.Vy = alfaY / alfaEff;
				Attr.Vz = alfaZ / alfaEff;
				// V = 1; //automatically
				Attr.X = 0;	Attr.Y = 0;	Attr.Z = 0;	Attr.L = 0;
				Attr.Current = PolarCurrent[k] / (int)AzimNumber; //Polar group !=0
				if (BeamRadius > 1.e-4) {
					if (fabs(BeamFocusX) > 1.e-4 && fabs(BeamFocusX) < 100) {
						if (sqrt(alfaY*alfaY + alfaZ*alfaZ) > BeamRadius / fabs(BeamFocusX))
							Attr.Current = 0;
					}
					else if (fabs(BeamFocusX) >= 100) { //parallel rays
						Attr.X = 0;
						Attr.Y = BeamRadius * (k / (PolarNumber)) * cos(AzimAngle);
						Attr.Z = BeamRadius * (k / (PolarNumber)) * sin(AzimAngle);
						Attr.Vx = 1; Attr.Vy = 0; Attr.Vz = 0;
					}
				}
				Attr.StartCurrent = Attr.Current;
				SumCurrent += Attr.Current;
				Attr_Array_Resid.Add(Attr);
			} // n = AzimNumber - 1
	} // k = PolarNumber

	for (int l = 0; l <= Attr_Array_Resid.GetUpperBound(); l++) {
		Attr_Array_Resid[l].StartCurrent = Attr_Array_Resid[l].Current / SumCurrent;
		Attr_Array_Resid[l].Current = Attr_Array_Resid[l].StartCurrent;
	}

	// restore defaults!!!!
	PolarNumber = PolarNumberAtom; //OldPolarNumber;
	AzimNumber = AzimNumberAtom;//OldAzimNumber;
	
	CString S;
	int Nattr = Attr_Array_Resid.GetSize();
	S.Format("Resid Attr size %d", Nattr);
	//AfxMessageBox(S);

}

void CBTRDoc::	SetPolarBeamletCommon()		// set attributes for (Npolar*Nazim)BP in Beamlet CS 
											// write "attr.dat" X Y Z Vx Vy Vz
											// MAMuG; SINGAP with DivX = DivY = const along all beamlets
											// Beam Misalignment = 0!
{
//	::SetCurrentDirectory(CurrentDirName);
//	CWaitCursor wait;
//	double AvCoreDiv = sqrt(BeamCoreDivY * (BeamCoreDivZ));
//	double HaloDivRel = BeamHaloDiverg / AvCoreDiv;

	ATTRIBUTES Attr;
	double AzimAngle, alfaY, alfaZ, alfaEff;
	// keep main values
	//double OldPolarNumber = PolarNumber;
	//double OldAzimNumber = AzimNumber;

	// set new values for residual
	PolarNumber = PolarNumberAtom;
	AzimNumber = AzimNumberAtom;

	Attr_Array.RemoveAll();

	if (BeamHaloDiverg < 1.e-16) BeamHaloPart = 0;
	if ((BeamCoreDivY) * (BeamCoreDivZ) < 1.e-16) BeamHaloPart = 1;

	CString S;
	S.Format("%d x %d", (int)PolarNumber, (int)AzimNumber);
	//AfxMessageBox("Set  Atom Attr " + S);
	SetPolarArrayBase(BeamCoreDivY, BeamCoreDivZ);

	if ((int)AzimNumber * (int)PolarNumber < 1) {// Axis Particle ONLY
			Attr.Current = PolarCurrent[0];
			Attr.StartCurrent = Attr.Current;
			Attr.Vx = 1.0;	Attr.Vy = 0; Attr.Vz = 0;
			Attr.X = 0;	Attr.Y = 0;	Attr.Z = 0;	Attr.L = 0;
			Attr_Array.Add(Attr);	
			return;
	}

// ----- AzimNumber > 0, PolarNumber > 0 -------------------------

	double SumCurrent = 0;
	for (int k = 0; k <= (int)PolarNumber; k++) {
		
//		if (k > 0 && PolarCurrent[k] < CutOffCurrent *(AzimNumber)) 	continue; // AzimNumber != 0!
	
		if  (k == 0) {// Axis particle
			Attr.Current = PolarCurrent[0]; // Axis particle for Plot Beamlet Foot 
			Attr.StartCurrent = Attr.Current;
			SumCurrent += Attr.Current;
			Attr.Vx = 1.0;	Attr.Vy = 0; Attr.Vz = 0;
			Attr.X = 0;	Attr.Y = 0;	Attr.Z = 0;	Attr.L = 0;
			Attr_Array.Add(Attr);
		}
		else //  Polar group !=0 
		for (int n = 0; n < (int)AzimNumber; n++)
		{
			double val = 0; //(double)rand() / (double) RAND_MAX;
			AzimAngle = (n+val)* 2*PI /(int) AzimNumber;
			
			// alfaX = Vx/Vx; alfaY = Vy/Vx; alfaZ = Vz/Vx
			alfaZ = BeamCoreDivZ * PolarAngle[k] * sin(AzimAngle);//
			alfaY = BeamCoreDivY * PolarAngle[k] * cos(AzimAngle);//
			alfaEff = sqrt(alfaZ*alfaZ + alfaY*alfaY + 1.0);
			Attr.Vx = 1.0 / alfaEff;
			Attr.Vy = alfaY / alfaEff;
			Attr.Vz = alfaZ / alfaEff;
			// V = 1; //automatically
			Attr.X = 0;	Attr.Y = 0;	Attr.Z = 0;	Attr.L = 0;
			Attr.Current = PolarCurrent[k] /(int) AzimNumber; //Polar group !=0
			if (BeamRadius > 1.e-4) {
				if (fabs(BeamFocusX) > 1.e-4 && fabs(BeamFocusX) < 100) {
					if (sqrt(alfaY*alfaY + alfaZ*alfaZ) > BeamRadius/fabs(BeamFocusX))
						Attr.Current = 0;
				}
				else if (fabs(BeamFocusX) >= 100) { //parallel rays
					Attr.X = 0; 
					Attr.Y = BeamRadius * (k/(PolarNumber)) * cos(AzimAngle);
					Attr.Z = BeamRadius * (k/(PolarNumber)) * sin(AzimAngle);
					Attr.Vx = 1; Attr.Vy = 0; Attr.Vz = 0;
				}
			}
			Attr.StartCurrent = Attr.Current;
			SumCurrent += Attr.Current;
			Attr_Array.Add(Attr);			
		} // n = AzimNumber - 1
	} // k = PolarNumber

		for (int l = 0; l <= Attr_Array.GetUpperBound(); l++) {
			Attr_Array[l].StartCurrent = Attr_Array[l].Current / SumCurrent;
			Attr_Array[l].Current = Attr_Array[l].StartCurrent;
		}
			//	::SetCurrentDirectory(CurrentDirName);
		
		// restore defaults!!!!
		
		PolarNumber = PolarNumberAtom; //OldPolarNumber;
		AzimNumber = AzimNumberAtom;//OldAzimNumber;
		
		int Nattr = Attr_Array.GetSize();
		S.Format("Atoms Attr size %d", Nattr);
		//AfxMessageBox(S);
}

void CBTRDoc:: SetBeam()// Beam Power; 
						// beamlets hor/vert positions/aimings for MAMuG 
						// tot number of beamlets 
{
	IonBeamPower = IonBeamCurrent * (IonBeamEnergy); 
	NeutrPower = IonBeamPower*(NeutrPart);

//	if (OptSINGAP) return;

	int i, j;//, NBhor, NBvert;
	int imax = (int)NofChannelsHor; 
	int jmax = (int)NofChannelsVert; 
	int iimax = (int)NofBeamletsHor;
	int jjmax =  (int)NofBeamletsVert;	

/*	SegmCentreHor.RemoveAll();	SegmCentreVert.RemoveAll();
	ActiveCh.RemoveAll();	ActiveRow.RemoveAll();
	RectIS.RemoveAll();	RectISrow.RemoveAll();
	BeamletAngleHor.RemoveAll();	BeamletAngleVert.RemoveAll();
	BeamletPosHor.RemoveAll();	BeamletPosVert.RemoveAll();*/

	SegmCentreHor.SetSize(100);//imax);
	SegmCentreVert.SetSize(100);//jmax);
	ActiveCh.SetSize(100);//imax);
	ActiveRow.SetSize(100);//jmax);
	RectIS.SetSize(100);//imax);
	RectISrow.SetSize(100);//jmax);
	BeamletAngleHor.SetSize(10000);//imax * iimax);
	BeamletAngleVert.SetSize(10000);//jmax * jjmax);
	BeamletPosHor.SetSize(10000);//imax * iimax);
	BeamletPosVert.SetSize(10000);//jmax * jjmax);


	for (i = 0; i < imax; i++)
		SegmCentreHor[i] =  SegmStepHor * ( -(imax -1) * 0.5 + i);
	for (j = 0; j < jmax; j++)
		SegmCentreVert[j] =  SegmStepVert * ( -(jmax -1) * 0.5 + j);

	//NBhor = (int)NofBeamletsHor;
	if (iimax > 1) SegmSizeHor = AppertStepHor * (iimax-1);
	else SegmSizeHor = 0.01;
	//NBvert = (int)NofBeamletsVert;
	if (jjmax > 1) SegmSizeVert = AppertStepVert * (jjmax-1);
	else SegmSizeVert = 0.01;
	

	double SegmAimHor, SegmAimVert;
	double dY, dZ;
	
	for (i = 0; i < imax; i++) {
		if (fabs(BeamAimHor) < 1.e-6) SegmAimHor = 0;
		else SegmAimHor = - atan(SegmCentreHor[i] / BeamAimHor);
		for (int ii = 0; ii < iimax; ii++) {
			 dY =  AppertStepHor * (-(iimax-1) * 0.5 + ii);
			 if (fabs(AppertAimHor) < 1.e-6) BeamletAngleHor[i * iimax + ii] = 0;
			 else BeamletAngleHor[i * iimax + ii] = 
			//	 atan((AppertAimHor * sin(SegmAimHor) - dY) / AppertAimHor * cos(SegmAimHor)) 
				 atan((-dY + AppertAimHor * tan(SegmAimHor)) / AppertAimHor) 
				 + BeamMisHor;
			 BeamletPosHor[i * iimax + ii] = SegmCentreHor[i] + dY;
		}
	}

	double VInclin = 0; // Free option -> no NBL inclination
	if (!OptFree) VInclin = VertInclin; // standard NBI
	/*else if (fabs(VertInclin) > 1.e-6) { // OptFree
		if (AfxMessageBox("Config: Beamline is inclined. Apply the inclination to the Beam Axis?", MB_YESNO) == IDYES)
		{
			BeamMisVert += BeamVertTilt;
			//BeamVertTilt += VertInclin;
			BeamVertTilt = VertInclin;
			VertInclin = 0;
		} // Yes
	}*/
	
	for (j = 0; j < jmax; j++){
		if (fabs(BeamAimVert) < 1.e-6) SegmAimVert = 0;
		else  SegmAimVert = - atan(SegmCentreVert[j] / BeamAimVert);
		 for (int jj = 0; jj < jjmax; jj++) {
			 dZ =  AppertStepVert * (-(jjmax-1) * 0.5 + jj);
			 if (fabs(AppertAimVert) < 1.e-6) BeamletAngleVert[j * jjmax + jj] = 0;
			 else BeamletAngleVert[j * jjmax + jj] = 
				 //atan((AppertAimVert * sin(SegmAimVert) - dZ) / AppertAimVert * cos(SegmAimVert)) 
				   atan((-dZ + AppertAimVert * tan(SegmAimVert)) / AppertAimVert) 
				 //-atan(SegmCentreVert[j] / BeamAimVert) 
				 + BeamMisVert + VInclin + BeamVertTilt;
			 BeamletPosVert[j * jjmax + jj] = SegmCentreVert[j] + dZ;
		 }
	}

	NofBeamletsTotal =  (int)NofBeamletsHor * (int)NofBeamletsVert * (int)NofChannelsHor * (int)NofChannelsVert;
	logout << "Set Beam - done\n";

}

int CBTRDoc:: EmitBeam() // account of beamlet hor/vert aimings - Rotate procedures
						// TOBEDONE: Misalignments not included! 
{
	DINSELECT = FALSE;
	NeedSave = FALSE;
	Progress = 0;
	NofCalculated = 0;
	Exit_Array.RemoveAll();
//	ShowStatus();
	int res;

	if (NofBeamlets <1) { 
		AfxMessageBox("At least one Channel/Row should be Switched-ON.", 
			MB_ICONSTOP | MB_OK);
		return (0);
	}

	if (LoadSelected < 1) {
		DINSELECT = TRUE;
		res = AfxMessageBox("No Active Surfaces. \n Create Automatically?",
			MB_ICONQUESTION | MB_YESNOCANCEL);
		if (res == IDNO) DINSELECT = FALSE;
		if (res == IDCANCEL)  return(0);
	}

/*	if (LoadSelected > 0 || DINSELECT) {
		 res =  AfxMessageBox("Save all Loads after calculation?",3);   
		if (res == IDYES) NeedSave = TRUE;
		if (res == IDCANCEL) return(0);
	}
*/
	StartTime = CTime::GetCurrentTime();
	
	pSetView->InvalidateRect(NULL, TRUE);

	ShowStatus();
	

	if (OptSINGAP) 
		res = EmitSINGAP(); // 0 - if stopped; 1 - if finished
	else 
		res = EmitMAMuG(); // 0 - if stopped; 1 - if finished

	//	ShowStatus();
	StopTime = CTime::GetCurrentTime();
	pSetView->InvalidateRect(NULL, TRUE);
	
	//Beep(500, 300); 
	if (LoadSelected > 0 && res == 1) {
		 if  (AfxMessageBox("Save results to disk?", MB_ICONQUESTION | MB_YESNO) == IDYES) NeedSave = TRUE;
	}

	if (NeedSave) SaveLoads(FALSE); //if calculation finished
	else DetachLoadFiles();
	
	//Beep(250, 200); Beep(125, 500);
	return(res);

}

int CBTRDoc::EmitElectrons(bool optSINGAP) // same as EmitSINGAP() - with very few changes!!! 
{
	double EMeV = IonBeamEnergy; //MeV - diff from EmitSingap
	// double BeamCurr = IonBeamCurrent; //A
	double BeamPower = IonBeamPower; // MW
	// double Xstart = 0;//EmitterCentre[0].X;
	double TStep = TraceTimeStep;
	double power; // BeamletPower * Attr.Current;
	ATTRIBUTES Attr;
	BEAMLET_ENTRY be;
	CParticle * Particle = new CParticle();
	Particle->SetPartSort(Electron);
	Particle->Mass = Me * RelMass(EMeV); /// ELECTRONS (diff from EmitSingap)
	C3Point Start; //in local channel (IS) coord 
	C3Point Local; // IS centre in Local CS
	C3Point Global; // IS centre in Global CS
	C3Point P;
	double DivY, DivZ;
	double fraction;

	Start = 0;  //pDoc->EmitterCentre[0]; 
	STOP = FALSE;

	double V = RelV(EMeV); //Particle->GetV(EkeV); //sqrt(2.* EkeV*1000 * Qe/mass); // {m/c}
	//	diff from EmitSingap
	
	int	Ntotal = NofBeamletsTotal; //BeamEntry_Array.GetSize();
	
	double  BeamletPower = BeamPower *1000000. / Ntotal ; //W per beamlet 

/////////////////////////////////////////////////////////////////////////////
if (optSINGAP) {

	for (int i = 0; i< Ntotal;  i++) { //
		if (STOP) return(0);
		// int result = 0;
		be = BeamEntry_Array[i];
		if (be.Active == FALSE) continue;
		Start.X = 0; 
		Start.Y = be.PosY; 
		Start.Z = be.PosZ; 
		DivY = be.DivY;
		DivZ = be.DivZ;
		fraction = be.Fraction;
		if (OptIndBeamlets) {// individual Div for each beamlet (core); average Halo diverg/part = const for all
			if ((int) BeamSplitType ==0)	SetPolarBeamletIndividual(DivY, DivZ); // polar splitting
			else SetGaussBeamlet(DivY, DivZ); // cartesian splitting
		}

	int Nmax = Attr_Array.GetUpperBound(); // BP per beamlet

	for (int n = 0; n <= Nmax; n++ ) { // Attr_Array UpperBound
		if (STOP) return(0);
		Attr = Attr_Array[n];
		Attr.Current *= fraction; Attr.StartCurrent = Attr.Current; 
		power = BeamletPower * Attr.Current; // W
		RotateVert(be.AlfZ, Attr.Vx, Attr.Vy, Attr.Vz); // SINGAP
		Attr.X = Start.X; Attr.Y = Start.Y; Attr.Z = Start.Z; Attr.L = 0;
		Attr.Vx *= V;  Attr.Vy *= V; Attr.Vz *= V;
		RotateHor(be.AlfY, Attr.Vx, Attr.Vy, Attr.Vz); // SINGAP
		Attr.Model = SLOW;	Attr.State = MOVING;
		Attr.Power = power; Attr.StartPower = power;
		Particle->SetPartAttr(Attr, TStep);
		if (n==0) Particle->Central = TRUE;
		else Particle->Central = FALSE;
		
		Particle->TraceElectron(); // 
	
	} // n  = Attr_Arr.Nmax
	NofCalculated++;
	Progress = (int) (NofCalculated*10000/NofBeamletsTotal);
	ShowStatus();

	} // Ntotal (beamlets)

}// SINGAP

///////////////////////////////////////////////////////////////////////
else { ///MAMuG

	int i, j, ii, jj;
	int Ny = (int)NofBeamletsHor, Nz = (int)NofBeamletsVert; // MAMuG
	int NSy = (int)NofChannelsHor, NSz = (int)NofChannelsVert; // MAMuG
	//int	Ntotal = Ny*Nz*NSy*NSz;

//	int N = 0; // number of beamlet == NofCalculated

for (i = 0; i<NSy;  i++) { ///NofChannelsHor;
		if (!ActiveCh[i]) continue;
			
for (j = 0; j<NSz; j++) {  ///NofChannelsVert;
	if (!ActiveRow[j]) continue;	
	
	for (ii = 0; ii<Ny; ii++)  // NofBeamletsHor
	for (jj = 0; jj<Nz; jj++) // NofBeamletsVert
	{
/// Upper Segments -----------------------------------------------
		if (STOP) return(0);
		// int result = 0;
		Start.X = 0; 
		Start.Y = BeamletPosHor[i*Ny + ii]; // MAMuG
		Start.Z = BeamletPosVert[j*Nz + jj]; // MAMuG

//		N++;
//		fprintf(fout, "%d    %d   %d   %g    %g    %g    %g\n", N, i*Ny + ii, j*Nz + jj, 
//			Start.Y, Start.Z, BeamletAngleHor[i*Ny + ii], BeamletAngleVert[j*Nz + jj]);
		
		int Nmax = Attr_Array.GetUpperBound();

	for (int n = 0; n <= Nmax; n++ ) { // Attr_Array UpperBound
		if (STOP) return(0);
		Attr = Attr_Array[n];
		//if (Attr.Current < 1.e-16) continue;
		power = BeamletPower * Attr.Current; // W
		RotateVert(BeamletAngleVert[j*Nz + jj], Attr.Vx, Attr.Vy, Attr.Vz); // MAMuG
		Attr.X = Start.X; Attr.Y = Start.Y; Attr.Z = Start.Z; Attr.L = 0;
		Attr.Vx *= V;  Attr.Vy *= V; Attr.Vz *= V;
		RotateHor(BeamletAngleHor[i*Ny + ii], Attr.Vx, Attr.Vy, Attr.Vz); // MAMuG
		Attr.Model = SLOW;	Attr.State = MOVING;
		Attr.Power = power; Attr.StartPower = power;
		Particle->SetPartAttr(Attr, TStep);
			
		if (n==0) Particle->Central = TRUE;
		else Particle->Central = FALSE;

		Particle->TraceElectron(); // 
	
	} // n  = Attr_Arr.Nmax
	NofCalculated++;
	Progress = (int) (NofCalculated *10000/NofBeamlets);
	ShowStatus();

	} // ii, ij 
	} // j
	} // i

}////MAMuG
	delete Particle;
	return (1);
}

int CBTRDoc:: EmitSINGAP()
{
	if (TracePartType == 0) return (EmitElectrons(TRUE));

	BEAMLET_ENTRY be;
	STOP = FALSE;
	int	Ntotal = BeamEntry_Array.GetSize();//NofBeamletsTotal;
	for (int i = 0; i< Ntotal;  i++) { //
		if (STOP) return(0);
		// int result = 0;
		be = BeamEntry_Array[i];
		EmitBeamlet(be);
		if (STOP) return(0);
		
		NofCalculated++;
		Progress = (int) (NofCalculated*10000/NofBeamlets);
		ShowStatus();
	} // Ntotal (beamlets)

	return (1);
}


int CBTRDoc:: EmitMAMuG() // account of beamlet hor/vert aimings - Rotate procedures
						// Misalignments are included in SetBeam() 
{
	SetSINGAPfromMAMUG();
	return (EmitSINGAP());

/*	if (TracePartType == 0) return(EmitElectrons(FALSE));
	BEAMLET_ENTRY be;
	STOP = FALSE;

	int i, j, ii, jj;
	int Ny = (int)NofBeamletsHor, Nz = (int)NofBeamletsVert; // MAMuG
	int NSy = (int)NofChannelsHor, NSz = (int)NofChannelsVert; // MAMuG
	int	Ntotal = Ny*Nz*NSy*NSz;
	
	double BeamPower = IonBeamPower; // MW
	double BeamletPower = BeamPower *1000000. / Ntotal ; //W per beamlet 

	int N = 0; // number of beamlet == NofCalculated

for (i = 0; i<NSy;  i++) { ///NofChannelsHor;
		if (!ActiveCh[i]) continue;
			
for (j = 0; j<NSz; j++) {  ///NofChannelsVert;
	if (!ActiveRow[j]) continue;	
	
	for (ii = 0; ii<Ny; ii++)  // NofBeamletsHor
	for (jj = 0; jj<Nz; jj++) // NofBeamletsVert
	{
		if (STOP) return(0);
		int result = 0;
		
		be.Active = TRUE;
		be.PosY = BeamletPosHor[i*Ny + ii]; // MAMuG
		be.PosZ = BeamletPosVert[j*Nz + jj]; // MAMuG
		be.AlfY = BeamletAngleHor[i*Ny + ii];
		be.AlfZ = BeamletAngleVert[j*Nz + jj];
		be.DivY = BeamCoreDivY;
		be.DivZ = BeamCoreDivZ;
		be.Fraction = 1;
		be.i = i; be.j = j;
	
		EmitBeamlet(be);
		N++;

		if (STOP) return(0);*/

/* //----------- old	
		Start.X = 0; 
		Start.Y = BeamletPosHor[i*Ny + ii]; // MAMuG
		Start.Z = BeamletPosVert[j*Nz + jj]; // MAMuG
		
//		fprintf(fout, "%d    %d   %d   %g    %g    %g    %g\n", N, i*Ny + ii, j*Nz + jj, 
//			Start.Y, Start.Z, BeamletAngleHor[i*Ny + ii], BeamletAngleVert[j*Nz + jj]);
		
		int Nmax = Attr_Array.GetUpperBound();

	for (int n = 0; n <= Nmax; n++ ) { // Attr_Array UpperBound
		if (STOP) return(0);
		Attr = Attr_Array[n];
		Attr.Current *= fraction; Attr.StartCurrent = Attr.Current; // MAMUG: fraction=const 
		power = BeamletPower * Attr.Current; // W
		RotateVert(BeamletAngleVert[j*Nz + jj], Attr.Vx, Attr.Vy, Attr.Vz); // MAMuG
		Attr.X = Start.X; Attr.Y = Start.Y; Attr.Z = Start.Z; Attr.L = 0;
		Attr.Vx *= V;  Attr.Vy *= V; Attr.Vz *= V;
		RotateHor(BeamletAngleHor[i*Ny + ii], Attr.Vx, Attr.Vy, Attr.Vz); // MAMuG
		Attr.Model = SLOW;	Attr.State = MOVING;
		Attr.Power = power; Attr.StartPower = power;
		Particle->SetPartAttr(Attr, TStep);
		
		if (n==0) Particle->Central = TRUE;
		else Particle->Central = FALSE;
		
	//	Particle->TraceParticle(power); // (Particle, power);

		EmitSourceParticle(Particle);
/*
		if (!OptThickNeutralization) {// THIN neutralization
		
			Particle->TraceSourceIon(NeutrXmax); // trace without Neutralization (until NeutrXmax)
			
			if (Particle->Attr.State != STOPPED) 
			{
				if (!OptNeutrStop)	Particle->EmitPosIon(-1);
				if (OptTraceAtoms) Particle->EmitAtom(-1);

				Particle->Attr.Current *= 1 - PosIonPart - NeutrPart;
				Particle->Attr.Power *= 1 - PosIonPart - NeutrPart;
				if (Particle->Attr.Current < 1.e-10) continue;

				if (!OptNeutrStop) { 
					Particle->TimeStep = IonStepL / V;
					Particle->TraceIon(); // source negative after Neutralizer
				} // trace primary ion 
			} // not stopped by solid surface
		} // THIN neutralization

		else  Particle->TraceSourceIonWithNeutralization();
	
	} // n  = Attr_Arr.Nmax
*/

/*	NofCalculated++;
	Progress = (int) (NofCalculated *10000/NofBeamlets);
	ShowStatus();

	} // ii, ij 
	} // j
	} // i
*/
//	delete Particle;
//	return (1); 
}


void CBTRDoc:: EmitSourceParticle(CParticle * Particle)
{
if (!OptThickNeutralization) {// THIN neutralization
		
			Particle->TraceSourceIon(NeutrXmax); // trace without Neutralization (until NeutrXmax)
			
			if (Particle->Attr.State != STOPPED) 
			{
				if (!OptNeutrStop) Particle->EmitPosIon(-1);
				if (OptTraceAtoms) Particle->EmitAtom(-1);

				Particle->Attr.Current *= 1 - PosIonPart - NeutrPart;
				Particle->Attr.Power *= 1 - PosIonPart - NeutrPart;
				if (Particle->Attr.Current < 1.e-10) return;//continue;

				if (!OptNeutrStop) { 
					Particle->TimeStep = IonStepL / Particle->Vstart;
					Particle->TraceIon(); // source negative after Neutralizer
				} // trace primary ion 
			} // not stopped by solid surface*/
		} // THIN neutralization

		else  Particle->TraceSourceIonWithNeutralization();

		return;
}

void CBTRDoc:: EmitBeamlet(BEAMLET_ENTRY be)
{
	if (be.Active == FALSE) return; //continue;

	ATTRIBUTES Attr;
	CParticle * Particle = new CParticle();
	Particle->SetPartType(TracePartType);
	double EkeV = IonBeamEnergy * 1000; //keV
	double V = Particle->SetV(EkeV); //sqrt(2.* EkeV*1000 * Qe/mass); // {m/c}
	double power;
	double BeamPower = IonBeamPower; // MW
	int	Ntotal = NofBeamletsTotal; //BeamEntry_Array.GetSize();
	double BeamletPower = BeamPower *1000000. / Ntotal ; //W per beamlet 
				
	if (OptSINGAP && OptIndBeamlets) {// individual Div for each beamlet (core); average Halo diverg/part = const for all
		if ((int)BeamSplitType == 0)	
			SetPolarBeamletIndividual(be.DivY, be.DivZ); // polar splitting
		else SetGaussBeamlet(be.DivY, be.DivZ); // cartesian splitting
	}
	// for MAMuG - SetPolarBeamletCommon() or SetGaussBeamlet(BeamCoreDivY,BeamCoreDivZ)
		
	int Nmax = Attr_Array.GetUpperBound(); // BP per beamlet
	
	for (int n = 0; n <= Nmax; n++ ) { // Attr_Array UpperBound
		if (STOP) return;
		Attr = Attr_Array[n];
		Attr.Current *= be.Fraction; Attr.StartCurrent = Attr.Current;//SINGAP fraction!=const 
		power = BeamletPower * Attr.Current; // W
		RotateVert(be.AlfZ, Attr.Vx, Attr.Vy, Attr.Vz); 
		RotateHor(be.AlfY, Attr.Vx, Attr.Vy, Attr.Vz); 
		Attr.Vx *= V;  Attr.Vy *= V; Attr.Vz *= V;

		//if (IonBeamStartX >= 0) {	Attr.X = IonBeamStartX; Attr.Y = be.PosY; Attr.Z = be.PosZ;}
		Attr.X = 0; Attr.Y = be.PosY; Attr.Z = be.PosZ;
		//Attr.Y = be.PosY - (IonBeamStartX) * Attr.Vy / Attr.Vx;
		//Attr.Z = be.PosZ - (IonBeamStartX) * Attr.Vz / Attr.Vx;
		
		Attr.L = 0;
		Attr.Model = SLOW;	Attr.State = MOVING;
		Attr.Power = power; Attr.StartPower = power;
		Particle->SetPartAttr(Attr, TraceTimeStep);
		
		if (n==0) Particle->Central = TRUE;
		else Particle->Central = FALSE;
		
		EmitSourceParticle(Particle);

	} // n  = Attr_Arr.Nmax
	if (!OptDrawPart) ShowBeamlet(be);

	delete Particle;
}

void CBTRDoc:: ShowBeamlet(BEAMLET_ENTRY be) //called from _ThreadFunc
{
	//CMainView* pMainView = (CMainView*)m_pViewWnd; 
	CDC* pDC = pMainView->GetDC();
	int x1, y1, z1, x2, y2, z2;
	double y0 = be.PosY;
	double z0 = be.PosZ;
	double	x = AreaLong;
	double	y = y0 + x * tan(be.AlfY); 
	double	z = z0 + x * tan(be.AlfZ); 
	
	x1 = pMainView->OrigX;
	y1 = pMainView->OrigY - (int)(y0 * pMainView->ScaleY);
	z1 = pMainView->OrigZ - (int)(z0 * pMainView->ScaleZ);
	x2 = pMainView->OrigX + (int)(x * pMainView->ScaleX);
	y2 = pMainView->OrigY - (int)(y * pMainView->ScaleY);
	z2 = pMainView->OrigZ - (int)(z * pMainView->ScaleZ);
	pMainView->ReleaseDC(pDC);
	
	//HDC DC = pMainView->GetDC()->GetSafeHdc();
	//cs.Lock();
	CPen * pOldPen = pDC->SelectObject(&pMainView->AtomPen);//DotPen);
	pDC->MoveTo(x1, y1); pDC->LineTo(x2, y2);
	pDC->MoveTo(x1, z1); pDC->LineTo(x2, z2);
	pDC->SelectObject(pOldPen);
	GdiFlush(); // temp rem
	pMainView->ReleaseDC(pDC);
	//cs.Unlock();
}

void CBTRDoc:: RandAttr(ATTRIBUTES & Attr, double step)
{
//	srand( (unsigned)time( NULL ) );   /* Display 10 numbers. */
//  for( i = 0;   i < 10;i++ )      printf( "  %6d\n", rand() );
//	srand(1);
	double val = (double)rand() / (double) RAND_MAX;
	double V = sqrt(Attr.Vx*Attr.Vx + Attr.Vy*Attr.Vy + Attr.Vz*Attr.Vz);
	double dx = step*val*Attr.Vx / V; 
	double dy = step*val*Attr.Vy / V; 
	double dz = step*val*Attr.Vz / V; 
	Attr.X += dx; Attr.Y += dy; Attr.Z += dz;
}


void CBTRDoc::OnOptionsAccountofreionization() 
{
	//OptReionAccount = !OptReionAccount;
	
	int res;
	if (OptReionAccount && !PressureLoaded) {
		res = AfxMessageBox(" The account of re-ionization needs Gas profile to be set.\n\t  Continue?",
			MB_ICONQUESTION | MB_OKCANCEL); 
		if (res == IDOK) { 
			OnEditGas(); 
			if (!PressureLoaded) OptReionAccount = FALSE;
		}
		else OptReionAccount = FALSE;
	}

	if (OptReionAccount) { // calculate re-ionized percent
		SetReionPercent();
	}

	OnShow(); //OnDataActive();	
}

void CBTRDoc:: ClearPressure(int add)
{
	if (add != 0) { AddGField = new CGasField(); return; } // clear additional profile

	switch (PressureDim) {
		case 0: break; // const everywhere
		case 1: GField.reset(new CGasField()); break;
		case 2: GFieldXZ.reset(new CGasFieldXZ()); break;
	}
	PressureLoaded = FALSE;
	GasFileName = "";
	GasCoeff = 0;
}


double CBTRDoc:: GetPressure(C3Point P)
{
	if (!PressureLoaded || GasCoeff < 1.e-6) return 0;
	double Dens = 0;

	switch (PressureDim) {
		case 0: Dens = 0; break; // const everywhere
		case 1: Dens = GField->GetPressure(P.X); break;
		case 2: Dens = GFieldXZ->GetDensity(P.X, P.Z); break;
	}

	return Dens * GasCoeff;
}

void CBTRDoc:: ClearB()
{
	switch (MagFieldDim) {
		case 0: break; // const everywhere
		case 1: BField.reset(new CMagField()); break;
		case 2: break; // not used
		case 3: BField3D.reset(new CMagField3D()); break;
	}

	FieldLoaded = FALSE;
	MagFieldFileName = "";
	MFcoeff = 0;
}

C3Point CBTRDoc:: GetB(C3Point P)
{
	C3Point B(0,0,0);
	if (!FieldLoaded) return B;
	
	switch (MagFieldDim) {
		case 0: break; // const everywhere
		case 1: B = BField->GetB(P.X); break;
		case 2: break; // not used
		case 3: B = BField3D->GetB(P); break;
	}
	return B * MFcoeff;
}

C3Point CBTRDoc:: GetB(double x, double y, double z)
{
	return GetB(C3Point(x,y,z));
}

void CBTRDoc:: SetReionPercent()
{
	if (!PressureLoaded) return;
	//|| GasCoeff < 1.e-6) return;

	ReionArray.RemoveAll();

	double Summa = 0; // atom current drop
	double Curr = 0; //  = NeutrDrop = PosCurr
	double NeutrCurr = 1;
	double x = ReionXmin;
	double dL;// = ReionStepL; //0.01;
	C3Point P;
	double Dens;

	
	while (x < ReionXmax) {
		if (x >= RXspec0 && x <= RXspec1) dL = ReionStepLspec;//special area for AP
		else dL = ReionStepL;
		Dens = GetPressure(C3Point(x, 0, 0));
		Curr = Dens * dL * (ReionSigma);// = dj/j = NeutrDrop = PosCurr 
		if (OptAddPressure) { 
			double ADens = AddGField->GetPressure(x);
			Curr += dL * ADens * AddReionSigma;
		}

		Summa += Curr;
		NeutrCurr = NeutrCurr * (1 - Curr); 
		P.X = x + dL; 
		P.Y = Curr * NeutrCurr; // H+ released at dL
		P.Z = exp(-Summa);// H0 total at X
		ReionArray.Add(P);
		x += dL;
	}
//	fclose(fout);

	ReionPercent = 100 * (1 - P.Z);
	logout << "Reionization loss  " << ReionPercent << "%\n";
}

void CBTRDoc::SetNeutrCurrents_Neg() // for THICK, negative IS
{
	logout << "SetNeutrCurrents for Negative Ions\n";
	NeutrArray.RemoveAll();
	C3Point P;
	CString s;

	double NL = 0; // Target Thickness
	double x = NeutrXmin;
	double dL = NeutrStepL; //;
	//double NegDrop, PosRate; // = dj/j = NegDrop = NeutrCurr + PosCurr
	double NegCurr, PosCurr; //j-, j+
	double A, B, C, D;

	// H-/D-
	A = ReionSigma + PosExchSigma; // H0->H+, H+->H0 
	B = TwoStripSigma - ReionSigma; // H-->H+, H0->H+
	C = ReionSigma; // H0->H+
	D = NeutrSigma + TwoStripSigma;// H-->H0
	s.Format("  Sigmas: 0+1 - %g +10 - %g  -1+1 - %g  -10 - %g\n", 
			ReionSigma, PosExchSigma, TwoStripSigma, NeutrSigma);
	logout << s;
		
		NegCurr = 1; PosCurr = 0;
		P.X = x; P.Y = NegCurr; P.Z = PosCurr; // H-/D-
		NeutrArray.Add(P);
		x += dL;
		while (x <= NeutrXmax) {
			NL += GetPressure(C3Point(x, 0, 0)) * dL;
			if (NL < 1.e-30 || fabs(D) < 1.e-30) NegCurr = 1;
			else  NegCurr = exp(-D*NL);
			if (NL < 1.e-30 || fabs(A) < 1.e-30 || fabs(D) < 1.e-30 || fabs(A - D) < 1.e-30) PosCurr = 0;
			else  PosCurr = C / A - (C / A + B / (A - D)) * exp(-A*NL) + B / (A - D) * exp(-D*NL);
			P.X = x; P.Y = NegCurr; P.Z = PosCurr;
			NeutrArray.Add(P);
			x += dL;
		}
		
		double NegIonPart = NegCurr; //exp(-SummaNeg);
		PosIonPart = PosCurr; // (1 - ((NeutrSigma)*exp(-SummaPos) - (ReionSigma)*exp(-SummaNeg)) / (NeutrSigma - ReionSigma));
		NeutrPart = 1 - NegIonPart - PosIonPart;
		NeutrPower = IonBeamPower * (NeutrPart);

		logout << "Neutral fraction         " << NeutrPart << "\n";
		logout << "Positive ions fraction   " << PosIonPart << "\n";
		logout << "Negative ions fraction   " << NegIonPart << "\n";
			
}

void CBTRDoc::SetNeutrCurrents_Pos() // for THICK, positive IS
{
	logout << "SetNeutrCurrents for Positive Ions\n";
	NeutrArray.RemoveAll();
	C3Point P;
	CString s;
	
	double NL = 0; // Target Thickness
	double x = NeutrXmin;
	double dL = NeutrStepL; //;
	double PosCurr; // = dj+/j+ = PosDrop = NeutrCurr 
	double NeutrCurr; // j0
	double A, B, C, D;
	A = PosExchSigma; // H+ -> H0
	B = ReionSigma; // H0 -> H+
	C = A + B;
	s.Format(" Sigmas: 0+1 - %g +10 - %g  Sum - %g\n", ReionSigma, PosExchSigma, C);
	logout << s;

	NeutrCurr = 0; PosCurr = 1;
	P.X = x; P.Y = NeutrCurr; P.Z = PosCurr;// 
	NeutrArray.Add(P);
	x += dL;
	while (x <= NeutrXmax) {
		NL += GetPressure(C3Point(x, 0, 0)) * dL; // dL = NeutrStepL
		if (NL < 1.e-30 || fabs(C) < 1.e-30) PosCurr = 1;
		else  PosCurr = (1 - B / C) * exp(-C*NL) + B / C; //
		P.X = x; P.Y = NeutrCurr; P.Z = PosCurr;
		NeutrArray.Add(P);
		x += dL;
	}
	
	PosIonPart = PosCurr; // (1 - ((NeutrSigma)*exp(-SummaPos) - (ReionSigma)*exp(-SummaNeg)) / (NeutrSigma - ReionSigma));
	NeutrPart = 1 - PosIonPart;
	NeutrPower = IonBeamPower * (NeutrPart);

	logout << "Neutral fraction         " << NeutrPart << "\n";
	logout << "Positive ions fraction   " << PosIonPart << "\n";
	
}

void CBTRDoc::SetNeutrYield_Neg() // for THIN, negative IS
{
	logout << "SetNeutrYield for Negative Ions\n";

	BOOL CalcYield = FALSE; // user defined yields 
	if (PressureLoaded && GasCoeff > 1.e-6) CalcYield = TRUE;

	//if (!PressureLoaded || NeutrPart > 1.e-3) CalcYield = FALSE;
	//if (GasCoeff < 1.e-6) CalcYield = FALSE;// Set Yields directly

	double x = NeutrXmin;
	double dL = NeutrStepL; //;
	double NegCurr, PosCurr; //j-, j+
	C3Point P;

	if (CalcYield) // use gas prof
	{
		SetNeutrCurrents_Neg(); // similarly for THICK
		//PosIonPart = PosCurr; // (1 - ((NeutrSigma)*exp(-SummaPos) - (ReionSigma)*exp(-SummaNeg)) / (NeutrSigma - ReionSigma));
		//NeutrPart = 1 - NegIonPart - PosIonPart;
	}

	NeutrArray.RemoveAll(); // clear array
	// for THIN - add 4 points only - step 
	NegCurr = 1; PosCurr = 0;
	P.X = x; P.Y = NegCurr; P.Z = PosCurr;
	NeutrArray.Add(P);
	x = NeutrXmax; P.X = x;
	NeutrArray.Add(P);

	x = NeutrXmax + dL;
	NegCurr = 1 - NeutrPart - PosIonPart;
	PosCurr = PosIonPart;
	P.X = x; P.Y = NegCurr; P.Z = PosCurr; // after NeutrXmax
	NeutrArray.Add(P);
	x += dL; P.X = x;
	NeutrArray.Add(P);

	if (!CalcYield) {
		NeutrPower = IonBeamPower * (NeutrPart);
		double NegIonPart = 1 - NeutrPart - PosIonPart; //exp(-SummaNeg);
		logout << "Neutral fraction         " << NeutrPart << "\n";
		logout << "Positive ions fraction   " << PosIonPart << "\n";
		logout << "Negative ions fraction   " << NegIonPart << "\n";
	}
}

void CBTRDoc::SetNeutrYield_Pos() // for THIN, positive IS
{
	logout << "SetNeutrYield for Positive Ions\n";
	
	BOOL CalcYield = FALSE; // user defined yields 
	if (PressureLoaded && GasCoeff > 1.e-6) 	CalcYield = TRUE;

	//if (!PressureLoaded || NeutrPart > 1.e-3) CalcYield = FALSE;
	//if (GasCoeff < 1.e-6) CalcYield = FALSE;// Set Yields directly
	
	if (CalcYield) // use gas prof
	{
		SetNeutrCurrents_Pos(); // similarly for THICK
		//PosIonPart = PosCurr; // (1 - ((NeutrSigma)*exp(-SummaPos) - (ReionSigma)*exp(-SummaNeg)) / (NeutrSigma - ReionSigma));
		//NeutrPart = 1 - PosIonPart;
		//NeutrPower = IonBeamPower * (NeutrPart);
	}
	NeutrArray.RemoveAll(); // clear array
	
// for THIN - add 4 points only - step 
	double x = NeutrXmin;
	double dL = NeutrStepL; //;
	double NeutrCurr, PosCurr; //j0, j+
	C3Point P;

	NeutrCurr = 0;	PosCurr = 1;
	P.X = x; P.Y = NeutrCurr; P.Z = PosCurr;
	NeutrArray.Add(P);
	x = NeutrXmax; P.X = x;
	NeutrArray.Add(P);

	x = NeutrXmax + dL;
	NeutrCurr = 1 - PosIonPart;
	PosCurr = PosIonPart;
	P.X = x; P.Y = NeutrCurr; P.Z = PosCurr; // after NeutrXmax
	NeutrArray.Add(P);
	x += dL; P.X = x;
	NeutrArray.Add(P);

	if (!CalcYield) {
		logout << "Neutral fraction         " << NeutrPart << "\n";
		logout << "Positive ions fraction   " << PosIonPart << "\n";
	}
}

void CBTRDoc::SetNeutrCurrents()
{
	if (!PressureLoaded || GasCoeff < 1.e-6) // set THIN mode
		OptThickNeutralization = FALSE;

	if (TracePartQ > 0) {// H+/D+
		if (OptThickNeutralization)
			SetNeutrCurrents_Pos();
		else SetNeutrYield_Pos(); // with or without gas prof!
	}
	else { // H-/D-
		if (OptThickNeutralization)
			SetNeutrCurrents_Neg();
		else SetNeutrYield_Neg();// with or without gas prof!
	}
}
	
/*	// Old CBTRDoc::SetNeutrCurrents()
	NeutrArray.RemoveAll();
	C3Point P;
	bool OptThick = OptThickNeutralization;
	//if (!OptThickNeutralization) return;

	// double SummaNeg = 0; // source current total drop
	// double SummaPos = 0; // Total Pos current 
	double NL = 0; // Target Thickness
	double x = NeutrXmin;
	double dL = NeutrStepL; //;
	double NegDrop, PosRate; // = dj/j = NegDrop = NeutrCurr + PosCurr
	double NegCurr, PosCurr; //j-, j+
	double A, B, C, D;
	
	if (TracePartQ > 0) {// H+/D+
		A = PosExchSigma; // H+ -> H0
		B = ReionSigma; // H0 -> H+
		C = A + B;
		NegCurr = 0; PosCurr = 1;
		P.X = x; P.Y = NegCurr; P.Z = PosCurr;// 
		NeutrArray.Add(P);
		x += dL;
		while (x <= NeutrXmax) {
			NL += GetPressure(C3Point(x, 0, 0)) * dL; // dL = NeutrStepL
			if (NL < 1.e-30 || fabs(C) < 1.e-30) PosCurr = 1; 
			else  PosCurr = (1 - B/C) * exp(-C*NL) +  B / C; //
			P.X = x; P.Y = NegCurr; P.Z = PosCurr;
			if (!OptThick) {// THIN
				NegCurr = 0; PosCurr = 1;
				P.X = x; P.Y = NegCurr; P.Z = PosCurr;// 
			}
			NeutrArray.Add(P);
			x += dL;
		}
		if (!OptThick) {// THIN - add 2 points
				P.X = x; P.Y = 0; P.Z = PosIonPart;// after NeutrXmax
				NeutrArray.Add(P);
				x += dL;
				P.X = x;
				NeutrArray.Add(P);
		}
	} // positive source ions

	else { // H-/D-
		A = ReionSigma + PosExchSigma; // H-/D-
		B = TwoStripSigma - ReionSigma; // H-/D-
		C = ReionSigma; // H-/D-
		D = NeutrSigma + TwoStripSigma;// H-/D-
		NegCurr = 1; PosCurr = 0;
		P.X = x; P.Y = NegCurr; P.Z = PosCurr;// // H-/D-
		NeutrArray.Add(P);
		x += dL;
		while (x <= NeutrXmax) {
			NL += GetPressure(C3Point(x, 0, 0)) * dL; 
			if (NL < 1.e-30 || fabs(D) < 1.e-30) NegCurr = 1; 
			else  NegCurr = exp(-D*NL);
			if (NL < 1.e-30 || fabs(A) < 1.e-30 || fabs(D) < 1.e-30 || fabs(A-D) < 1.e-30) PosCurr = 0;
			else  PosCurr = C/A - (C/A + B/(A-D)) * exp(-A*NL) + B/(A-D) * exp(-D*NL);
			P.X = x; P.Y = NegCurr; P.Z = PosCurr;
			if (!OptThick) {// THIN
				NegCurr = 1; PosCurr = 0;
				P.X = x; P.Y = NegCurr; P.Z = PosCurr; //PosIonPart at NeutrXmax
			}
			NeutrArray.Add(P);
			x += dL;
		}
		if (!OptThick) {// THIN - add 2 points
			NegCurr = 1 - NeutrPart - PosIonPart; PosCurr = PosIonPart;
			P.X = x; P.Y = NegCurr; P.Z = PosCurr; // after NeutrXmax
			NeutrArray.Add(P);
			x += dL; P.X = x;
			NeutrArray.Add(P);
		}
			
	} // negative source ions
	
	double NegIonPart = NegCurr; //exp(-SummaNeg);
	PosIonPart = PosCurr; // (1 - ((NeutrSigma)*exp(-SummaPos) - (ReionSigma)*exp(-SummaNeg)) / (NeutrSigma - ReionSigma));
	NeutrPart = 1 - NegIonPart - PosIonPart;
	NeutrPower = IonBeamPower * (NeutrPart);

	logout << "Neutral fraction         " << NeutrPart << "\n";
	logout << "Positive ions fraction   " << PosIonPart << "\n";
	logout << "Negative ions fraction   " << NegIonPart << "\n";
	*/


C3Point CBTRDoc:: GetCurrentRate(double x, double NL, bool getRate) // 0 - currents, 1 - rates
// source ions H-/D- !
{
	C3Point P(0,1,0);
	if (!PressureLoaded || GasCoeff < 1.e-6) return P;
	C3Point Pos(x,0,0);
	double Dens = GetPressure(Pos);
	double dL = NeutrStepL;
	double NegDrop, PosRate, NeutrRate; // = dj/j = NegDrop = NeutrCurr + PosCurr
	double NegCurr, PosCurr, NeutrCurr; //j-, j+, j0
	double A, B, C, D;
	double S01, S10, S_10, S_11;

	S01 = ReionSigma;
	S10 = PosExchSigma;
	S_10 = NeutrSigma;
	S_11 = TwoStripSigma;
	A = S01 + S10; // H-/D-
	B = S_11 - S01; // H-/D-
	C = S01; // H-/D-
	D = S_10 + S_11;// H-/D-
	
	if (NL < 1.e-30 || fabs(D) < 1.e-30) NegCurr = 1; 
	else  NegCurr = exp(-D*NL);
	if (NL < 1.e-30 || fabs(A) < 1.e-30 || fabs(D) < 1.e-30 || fabs(A-D) < 1.e-30) PosCurr = 0;
	else  PosCurr = C/A - (C/A + B/(A-D)) * exp(-A*NL) + B/(A-D) * exp(-D*NL);
	if (PosCurr < 0 ) PosCurr = 0;
	NeutrCurr = 1 - NegCurr - PosCurr;
	
	if (getRate == 0) {
		P.X = PosCurr;
		P.Y = NegCurr;
		P.Z = NeutrCurr;
		return P;
	}

	NegDrop = NegCurr * D * dL * Dens; // Drop >0
	PosRate = (NeutrCurr * S01 - PosCurr * S10 + NegCurr * S_11) * dL * Dens; // (dj+/dl)*dL
//	NeutrRate = (NegCurr * S_10 - NeutrCurr * S01 + PosCurr * S10) * dL * Dens;  // (djo/dl)*dL
	NeutrRate = (NegCurr * S_10 + PosCurr * S10) * dL * Dens; // without reionisation
	if (NeutrRate < 0) NeutrRate = 0;
	P.X = PosRate;
	P.Y = NegDrop;
	P.Z = NeutrRate;
	return P;
}

double CBTRDoc:: GetNL(double xmin, double xmax)
{
	double NL = 0;
	if (!PressureLoaded || GasCoeff < 1.e-6) return NL;
	double x = xmin;
	C3Point Pos(0,0,0); // axis X!!!
	Pos.X = xmin;
	double Dens0 = GetPressure(Pos);
	double Dens1, Dens;
	double dL = NeutrStepL;
	while (x < xmax) {
		x += dL;
		Pos.X = x;
		Dens1 = GetPressure(Pos);
		Dens = (Dens0 + Dens1) * 0.5;
		NL += Dens * dL;
		Dens0 = Dens1;
	}
	return NL;
}

void CBTRDoc::OnUpdateOptionsAccountofreionization(CCmdUI* pCmdUI) 
{
	
	//pCmdUI->SetCheck(OptReionAccount);
	if (!OptReionAccount) pCmdUI->SetText("Allow for Re-ionization");
	else pCmdUI->SetText("Skip Re-ionization");
}

void CBTRDoc::OnOptionsStopionsafterneutraliser() 
{
	OptNeutrStop = !OptNeutrStop;
	
	OnShow(); //OnDataActive();
}

void CBTRDoc::OnUpdateOptionsStopionsafterneutraliser(CCmdUI* pCmdUI) 
{
	//pCmdUI->SetCheck(OptNeutrStop);
	if (!OptNeutrStop) pCmdUI->SetText("Stop Residual Ions");
	else pCmdUI->SetText("Trace Residual Ions");
}

void CBTRDoc::OnOptionsStopreionizedparticles() 
{
//	OptReionStop = !OptReionStop;
	if (!OptReionStop && !OptReionAccount) OnOptionsAccountofreionization(); 

	if (!OptReionAccount) { 
		OptReionStop = TRUE; 
		OnShow(); //OnDataActive(); 
		return; 
	}

	// !OptReionStop
	int res;
	if (!OptReionStop && !FieldLoaded) {
		res = AfxMessageBox(" Ions Tracing needs Magnetic Field to be set.\n\t  Continue?",
			MB_ICONQUESTION | MB_OKCANCEL); 
		if (res == IDOK) { 
			OnEditMagfield_4columns(); 
			if (!FieldLoaded) OnEditMagfield_7columns(); 
			OptReionStop = !FieldLoaded;
		}
		else  OptReionStop = TRUE;
	}
	OnShow(); //OnDataActive();
}

void CBTRDoc::OnUpdateOptionsStopreionizedparticles(CCmdUI* pCmdUI) 
{
	//pCmdUI->SetCheck(OptReionStop);
	if (!OptReionStop) pCmdUI->SetText("Stop Re-ionized");
	else pCmdUI->SetText("Trace Re-ionized");
}

void CBTRDoc::OnOptionsStrayfieldonoff() 
{
//	OptStrayField =!OptStrayField;
	pMainView->InvalidateRect(NULL, TRUE);
	OnShow(); //OnDataActive();
}

void CBTRDoc::OnUpdateOptionsStrayfieldonoff(CCmdUI* /* pCmdUI */) 
{
	//pCmdUI->SetCheck(OptStrayField);
	//if (!OptStrayField) pCmdUI->SetText("Switch-ON MagField");
	//else pCmdUI->SetText("Switch-OFF MagField");
}

void CBTRDoc::OnOptionsOpencalorimeter() 
{
	OptCalOpen =!OptCalOpen;

	SetPlates();
	OnShow(); //OnDataActive();
	pMainView->InvalidateRect(NULL, TRUE);

}

void CBTRDoc::OnUpdateOptionsOpencalorimeter(CCmdUI* pCmdUI) 
{
	//pCmdUI->SetCheck(OptCalOpen);
	if (!OptCalOpen) pCmdUI->SetText("Open Calorimeter");
	else pCmdUI->SetText("Close Calorimeter");

}

void CBTRDoc::OnTasksMagshield() // disabled now
{
	CRemnDlg dlg;
	dlg.m_B1 = Bremnant1;
	dlg.m_B2 = Bremnant2;
	dlg.m_B3 = Bremnant3;
	dlg.m_X1 = Xremnant1;
	dlg.m_X2 = Xremnant2;
	dlg.m_X3 = Xremnant3;
	if (dlg.DoModal() == IDOK) {
		Bremnant1 = dlg.m_B1;
		Bremnant2 = dlg.m_B2;
		Bremnant3 = dlg.m_B3;
		Xremnant1 = dlg.m_X1;
		Xremnant2 = dlg.m_X2;
		Xremnant3 = dlg.m_X3;
		if (Xremnant2 < Xremnant1) Xremnant2 = Xremnant1;
		if (Xremnant3 < Xremnant2) Xremnant3 = Xremnant2;
		TaskRemnantField = TRUE;
		ShowTestField();
	}
	else TaskRemnantField = FALSE;
}

void CBTRDoc::OnUpdateTasksMagshield(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(FALSE);
//	pCmdUI->SetCheck(TaskRemnantField);
}

void CBTRDoc::OnTasksReionization() 
{
/*	CReionDlg dlg;

	switch (TracePartType) {
	case 0:	dlg.m_Caption = "e -> e+"; break;
	case 1:
	case -1:
	case 10:
			dlg.m_Caption = "Ho -> H+"; break;
	case 2:
	case -2:
	case 20:
			dlg.m_Caption = "Do -> D+"; break;
	default: dlg.m_Caption = "Neutral atom Zo -> Ion Z+"; break; 
	}
	dlg.m_IonStepL = IonStepL;
	dlg.m_Lstep = ReionStepL;
	dlg.m_Percent =	ReionPercent;
	dlg.m_Sigma = ReionSigma;
	dlg.m_Xmin = ReionXmin;
	dlg.m_Xmax = ReionXmax;
	dlg.m_Polar = PolarNumberReion;
	dlg.m_Azim = AzimNumberReion;

	if (dlg.DoModal() == IDOK) {
		ReionSigma = dlg.m_Sigma;
		ReionXmin = dlg.m_Xmin;
		ReionXmax = dlg.m_Xmax;
		ReionStepL = dlg.m_Lstep;
		IonStepL = dlg.m_IonStepL;
		PolarNumberReion = dlg.m_Polar;
		AzimNumberReion = dlg.m_Azim;
	
		SetReionPercent();
		SetPolarBeamletReion();

		CString S;
		if (PressureLoaded)	S.Format("Re-ionization loss = %3.1f %%", ReionPercent);
		else S.Format("Pressure profile not defined. \n Re-ionized current = 0");
		AfxMessageBox(S);
	}
*/	
	SetReionization();
///////////////////
//	Progress = 0;
//	CurrentDirName = TopDirName;
//	::SetCurrentDirectory(CurrentDirName);
//	SetTitle(CurrentDirName);

//	TaskRID = FALSE;
	TaskReionization = TRUE; //!TaskReionization; 
//	if (LoadSelected > 0) SelectAllPlates(); // empty the plates list
//	InitTaskReionization();
	
	TaskName = " RE-IONISED POWER";
	OnShow();

}

void CBTRDoc::OnUpdateTasksReionization(CCmdUI* pCmdUI) 
{
//	pCmdUI->Enable(FALSE);
	pCmdUI->SetCheck(TaskReionization);
}


void CBTRDoc::OnTasksNeutralisation() //
{
//	if (PressureLoaded)	SetNeutrCurrents();
//	OnShow();

/* 	CNeutrDlg dlg;
	dlg.m_Thin = OptThickNeutralization;

	if (dlg.DoModal() == IDOK) {
				OptThickNeutralization = dlg.m_Thin;
			}
	else return; // ThickNeutralization = FALSE;
	
	if (!PressureLoaded && OptThickNeutralization) {
		AfxMessageBox("Please, define gas profile before choosing THICK model \n THIN model is active ");
		OptThickNeutralization = FALSE;
	}

	if (!OptThickNeutralization) {
		CThinDlg dlg;
		dlg.m_Xlim = NeutrXmax;
		dlg.m_Neff = NeutrPart * 100;
		dlg.m_PosYield = PosIonPart * 100;
		if (dlg.DoModal() == IDOK) {
			NeutrXmax = dlg.m_Xlim;
			NeutrPart = dlg.m_Neff * 0.01;
			PosIonPart = dlg.m_PosYield * 0.01;
			CheckData();
		}
	}

	else {
		CThickDlg dlg;
		dlg.m_Xmin = NeutrXmin;
		dlg.m_Xmax = NeutrXmax;
		dlg.m_Step = NeutrStepL;
		dlg.m_SigmaNeutr = NeutrSigma * 10000; // -> cm2
		dlg.m_SigmaIon = ReionSigma * 10000;
		dlg.m_Sigma2Strip = TwoStripSigma * 10000;
		dlg.m_SigmaExch = PosExchSigma * 10000;
		if (dlg.DoModal() == IDOK) {
			NeutrXmin = dlg.m_Xmin;
			NeutrXmax = dlg.m_Xmax;
			NeutrStepL = dlg.m_Step;
			NeutrSigma = dlg.m_SigmaNeutr * 0.0001; // -> m2
			ReionSigma = dlg.m_SigmaIon * 0.0001;
			TwoStripSigma = dlg.m_Sigma2Strip * 0.0001;
			PosExchSigma = dlg.m_SigmaExch * 0.0001;
			SetNeutrCurrents();
		}
	}*/
	SetNeutralisation();
	OnShow();
}

void CBTRDoc::OnUpdateTasksNeutralisation(CCmdUI* pCmdUI) 
{
//	pCmdUI->Enable(FALSE);
	pCmdUI->SetCheck(PressureLoaded);
}

void CBTRDoc::OnResultsRead() 
{
	CLoadView * pLV = (CLoadView *) pLoadView;

	CLoad * load = NULL;
	char name[1024];
//	char buf[1024];
//	CFileDialog dlg(TRUE, "dat", "*.dat");
	CFileDialog dlg(TRUE, "dat; txt | * ", "", OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"3-column load data file (*.txt); (*.dat) | *.txt; *.TXT; *.dat; *.DAT | All Files (*.*) | *.*||", NULL);
	if (dlg.DoModal() == IDOK) {
	
		strcpy(name, dlg.GetPathName());
		CWaitCursor wait;
		CPreviewDlg dlg;
		dlg.m_Filename = name;
		if (dlg.DoModal() == IDCANCEL) return;
		SetTitle(name);// + TaskName);
	
//	if (LoadSelected > 0) SelectAllPlates();
//	InitAddPlates();

		
	load = ReadLoad(name);
	if (load != NULL && load->MaxVal >1.e-10) {
		pLV->ShowLoad = TRUE;
		pLV->SetLoad_Plate(load, pMarkedPlate);
		pSetView->SetLoad_Plate(load, pMarkedPlate);
		ShowProfiles = TRUE;
		OptCombiPlot = 1; 
	} // load != NULL
	else {
		AfxMessageBox("OnResultsRead : load = NULL");
 		pLV->ShowLoad = FALSE;
		pSetView->Load = NULL;
	}

	//ModifyArea(FALSE);// beam not fixed, account beamfoot change
    pMainView->InvalidateRect(NULL, TRUE);
	pLV->InvalidateRect(NULL, TRUE);
	pSetView->InvalidateRect(NULL, TRUE);
	ShowFileText(name);
	} // if IDOK
	OptParallel = FALSE;// ! for CalculateCombi(0)
	
}

CLoad* CBTRDoc::ReadLoad(char * name)
{
	//if (InvUser) return NULL;
	//SetLoadArray(NULL, FALSE);//TRUE);
	CPlate * plate = NULL; 
	CPlate * plate0 = new CPlate();
	CLoad * load = new CLoad();
	plate0->Load = load;
	int Number = -1;
	int Nx = 0, Ny = 0, SmoothDegree = 0;
	double StepX = 0, StepY = 0, Xmax = 0, Ymax = 0;
	C3Point Orig(-1,0,0), OrtX(0,1,0), OrtY(0,0,1);
	CString Comment = "No comment";
	C3Point p0, p1, p2, p3, mass;
	CString S;

//	ReadLoadInfo(m_Text, Nx, Ny, StepX, StepY);// standard old
	BOOL Free;
	Free = ReadPlateInfo(name, plate0);
	Nx = plate0->Load->Nx;
	Ny = plate0->Load->Ny;
	StepX = plate0->Load->StepX;
	StepY = plate0->Load->StepY;
	Xmax = Nx*StepX;
	Ymax = Ny*StepY;
	Orig = plate0->Orig;
	OrtX = plate0->OrtX; OrtY = plate0->OrtY;
	SmoothDegree = plate0->SmoothDegree;
	Number = plate0->Number;
	Comment = plate0->Comment;
	
	//mass = (p0 + p1 + p2 + p3) * 0.25;

if (Xmax * Ymax > 1.e-6) {
	load = new CLoad(Xmax, Ymax, StepX, StepY);
	int res = ReadLoadArray(name, load, Free);
	if (res > 0) load->SetSumMax();
}
	bool found = FALSE;

	POSITION pos = PlatesList.GetHeadPosition();
	while (pos != NULL && !found) {// find plate0 in the existing plates list 
		plate = PlatesList.GetNext(pos);
		//if (GetDistBetween(plate->MassCentre, mass) < 1.e-3) { found = TRUE; break; }
		/*C3Point Dist = (plate->Orig) - Orig;
		C3Point Vsum1 = VectSum(plate->OrtX, OrtX, 1, -1);
		C3Point Vsum2 = VectSum(plate->OrtY, OrtY, 1, -1);
		if (ModVect(Dist) < 1.e-3 && ModVect(Vsum1) < 0.1 && ModVect(Vsum2) < 0.1) {*/
		//if (GetDistBetween(plate->Corn[1], plate0->Corn[1]) < 1.e-6 && GetDistBetween(plate->Corn[2], plate0->Corn[2]) < 1.e-6) { found = TRUE; break; } 
		if (!OptFree && Comment.Find(plate->Comment, 0) > -1) { found = TRUE; break; }
		if (plate->Number == Number && GetDistBetween(plate->Orig, Orig) < 1.e-3) {
			found = TRUE; break; }
		if (GetDistBetween(plate->Vmax, plate0->Vmax) < 1.e-3 
			&& GetDistBetween(plate->Vmin, plate0->Vmin) < 1.e-3) {
			found = TRUE; break; }
		
	}//pos
	
	if (found) {
			plate->SetLocals(plate->Corn[0], plate->Corn[1], plate->Corn[3], plate->Corn[2]);
			plate->filename = name;

		if (plate->Loaded && Xmax * Ymax > 1.e-6) { // delete existing load
			plate->Load->Clear();
			plate->Load->~CLoad();
		}
			plate->Load = load; 
			plate->Loaded = TRUE;
			plate->Xmax = Xmax; plate->Ymax = Ymax;
			load->Comment = plate->Comment;
			//plate->SmLoad = new CLoad(plate->Xmax, plate->Ymax, StepX, StepY);
			//plate->SmLoad->Comment = load->Comment;
			plate->SmoothDegree = SmoothDegree;
			SetLoadArray(plate, TRUE); // 
			S.Format("Plate %d %s \n is identified\n", Number, Comment);
			//AfxMessageBox(S);
			logout << S;
			pMarkedPlate = plate;
			return load;
	} // found in list

	else {// (!found) or  free surf
		S.Format("Plate %d %s \n not identified\n", Number, Comment);
		//AfxMessageBox(S);
		logout << S;
		return NULL; // not found
	}

	// --- free mode ---
	{ // (!found) or  free surf
		p0 = plate0->Corn[0]; //p0 = Orig; 
		p1 = plate0->Corn[1]; //p1 = Orig + OrtX*Xmax;
		p2 = plate0->Corn[3]; //p2 = Orig + OrtY*Ymax;
		p3 = plate0->Corn[2]; //p3 = p1 + OrtY*Ymax;
		
		if (GetDistBetween(p0, p3) < 1.e-6 && GetDistBetween(p1, p2) < 1.e-6) {
			p0 = Orig; 
			p1 = Orig + OrtX*Xmax;
			p2 = Orig + OrtY*Ymax;
			p3 = p1 + OrtY*Ymax;
		}
		plate0->SetLocals(p0, p1, p2, p3);
		plate0->Fixed = -1;
		plate0->filename = name;
		if (Xmax * Ymax > 1.e-6) {
			plate0->Loaded = TRUE;
			plate0->Load = load;
			load->Comment = plate0->Comment;
			//plate0->SmLoad = new CLoad(Xmax, Ymax, StepX, StepY);
			//plate0->SmLoad->Comment = load->Comment;
			plate0->SmoothDegree = SmoothDegree;
		}
		else  { // load not found
			plate0->Loaded = FALSE;
			plate0->Load = NULL;
			load = NULL;
		}
		
		CString com = plate0->Comment.MakeUpper();
		if (com.Find("SOLID") >= 0)  plate0->Solid = TRUE; 
		//else  plate0->Solid = FALSE;
		plate0->Visible = plate0->Solid;
	
		PlatesList.AddTail(plate0);
		if (load != NULL) 
			SetLoadArray(plate0, TRUE); // changes plate->Number!
		pMarkedPlate = plate0;
		//AddPlatesNumber++;

	} // not found in the PlatesList

	return load;
//} //Nx*Ny>0

//else return NULL;

}

void CBTRDoc:: ReadLoadInfo(CString & Text, int & Nx, int & Ny, double & StepX, double & StepY)
{
//	m_Text.Empty();// = '\0';
	CString text = Text; 
	CString line, name, valstr;
	double Value;
	BOOL found = FALSE; 
	int pos, pos1; // current position of symbol

	CString info = "";
	pos = text.Find("PARAM", 0);
	if (pos > 0) {
		name = text.Left(pos);
		//name.TrimRight();
		text.Delete(0, pos + 1);
	}
	if (name.GetLength() > 1) 
		info = name;
	else info = "Unknown";
	//Text = info;
	//return;
	Nx = -1;
	Ny = -1;
	StepX = -1;
	StepY = -1;

	
	while (text.GetLength() > 0 && !found) {
		//pos =  text.FindOneOf("\n\0");// text.Find("\n", n);
		pos = text.Find("=", 0);
		if (pos < 2) break;
		name = text.Left(pos);
		valstr = text.Mid(pos+1);
	    Value = atof(valstr);
		pos1 = name.Find("Nx", 0);
		if (pos1 >0) {
			Nx = (int)Value;
			text.Delete(0, pos+1);
			continue;
		}
		pos1 = name.Find("Ny", 0);
		if (pos1 >0) {
			Ny = (int)Value;
			text.Delete(0, pos+1);
			continue;
		}
		pos1 = name.Find("StepX", 0);
		if (pos1 >0) {
			StepX = Value;
			text.Delete(0, pos+1);
			continue;
		}
		pos1 = name.Find("StepY", 0);
		if (pos1 >0) {
			StepY = Value;
			text.Delete(0, pos+1);
			found = TRUE; // stop search
			continue;
		}
	} // while !found && textLength >0

	Text = info;
	
}

BOOL CBTRDoc:: ReadPlateInfo(char* name, CPlate * Plate)
{
	FILE * fin;
	char buf[1024];
	CString Line;
	if ( (fin= fopen(name, "r")) == NULL) {
		CString S ="Can't find/open ";
		S+=  name;
		logout << S << "\n";
		//AfxMessageBox(S, MB_ICONSTOP | MB_OK);
		return 0;
	}
	//AfxMessageBox("Enter ReadPlateInfo");
	int  pos0, pos, res; //Name, "="
	CString valstr;
	double Value, x, y, z;
	//C3Point P;
	Plate->Corn[0] = C3Point(0,0,0);//Orig; 
	Plate->Corn[1] = C3Point(0,0,0); //p1 = Orig + OrtX*Xmax;
	Plate->Corn[3] = C3Point(0,0,0); //p2 = Orig + OrtY*Ymax;
	Plate->Corn[2] = C3Point(0,0,0); //p3 = p1 + OrtY*Ymax;
	Plate->Number = -1;
	Plate->Comment = "No comment";
	Plate->Solid = FALSE;
	BOOL foundFree = FALSE;

	//while (!feof(fin)) {
		Line = fgets(buf, 1024, fin);
		pos0 = -1;
		pos = -1;
		Line.MakeUpper();
		//AfxMessageBox(Line);

		if (Line.Find("#")>-1 || Line.Find("FREE") > -1)  
		{
			foundFree = TRUE; 
			if ((Line.Find("SOLID")) > -1) Plate->Solid = TRUE;
			else if ((Line.Find("TRANSPAR")) > -1) Plate->Solid = FALSE;
		} //found Free

		if (foundFree) {
			AfxMessageBox("ReadPlateInfo found FREE surf");
			for (int i = 0; i < 4; i++) { // read corners
				if (fgets(buf, 1024, fin) == NULL) continue;
				res = sscanf(buf, "%le %le %le", &x, &y, &z);
			
				if (res != 3) {
					CString S;
					S.Format("%s - Corner %d not defined", name, i);
					AfxMessageBox(S); 
					//fgets(buf, 1024, fin); // read  line
					break;
				} //if

				Plate->Corn[i].X = x; Plate->Corn[i].Y = y; Plate->Corn[i].Z = z; 
			} //read corners
			//Line = fgets(buf, 1024, fin); // read neaxt line - Comments 
		} // # found Free surf

		//else OptFree = FALSE;// Standard surf found -> switch to Standard geometry
 
 // Next 2 lines shoud be read in Free mode!!! (uncommented)
	//	Line = fgets(buf, 1024, fin); // read neaxt line - Comments 
	//	Line.MakeUpper();
	
		if ((pos0 = Line.Find("PARAM",0)) >=0) {
			if ((Line.Find("SOLID")) > -1) Plate->Solid = TRUE;
			else if ((Line.Find("TRANSPAR")) > -1) Plate->Solid = FALSE;
			else Plate->Solid = TRUE;
			Plate->Comment = Line.Left(pos0-3);
			pos = Line.MakeUpper().Find("PLATE", 0);
			valstr = Line.Mid(pos+5);
			Value = atof(valstr);
			Plate->Number = (int)Value;
		} // 
	

	while (!feof(fin)) {
		Line = fgets(buf, 1024, fin);
		//AfxMessageBox(Line);
		if ((pos0 = Line.Find("Nx",0)) >=0) {
			pos = Line.Find("=", 0);
			valstr = Line.Mid(pos+1);
			Value = atof(valstr);
			Plate->Load->Nx = (int)Value;
		}
		if ((pos0 = Line.Find("Ny",0)) >=0) {
			pos = Line.Find("=", pos0);
			valstr = Line.Mid(pos+1);
			Value = atof(valstr);
			Plate->Load->Ny = (int)Value;
			continue;
		}
		if ((pos0 = Line.Find("StepX",0)) >=0) {
			pos = Line.Find("=", 0);
			valstr = Line.Mid(pos+1);
			Value = atof(valstr);
			Plate->Load->StepX = Value;
			Plate->Load->Xmax = Plate->Load->Nx * Plate->Load->StepX;
			Plate->Xmax = Plate->Load->Xmax;
		}
		if ((pos0 = Line.Find("StepY",0)) >=0) {
			pos = Line.Find("=",pos0);
			valstr = Line.Mid(pos+1);
			Value = atof(valstr);
			Plate->Load->StepY = Value;
			Plate->Load->Ymax = Plate->Load->Ny * Plate->Load->StepY;
			Plate->Ymax = Plate->Load->Ymax;
			continue;
		}
		if ((pos0 = Line.Find("Origin",0)) >=0) {
			pos = Line.Find("=", 0); //X =
			valstr = Line.Mid(pos+1);
			Value = atof(valstr);
			Plate->Orig.X = Value;
			Line.Delete(0, pos+1);
			pos = Line.Find("=", 0); //Y =
			valstr = Line.Mid(pos+1);
			Value = atof(valstr);
			Plate->Orig.Y = Value;
			Line.Delete(0, pos+1);
			pos = Line.Find("=",0); //Z = 
			valstr = Line.Mid(pos+1);
			Value = atof(valstr);
			Plate->Orig.Z = Value;
			continue;
		}
		if ((pos0 = Line.Find("OrtX",0)) >=0) {
			pos = Line.Find("=", 0); //X =
			valstr = Line.Mid(pos+1);
			Value = atof(valstr);
			Plate->OrtX.X = Value;
			Line.Delete(0, pos+1);
			pos = Line.Find("=", 0); //Y =
			valstr = Line.Mid(pos+1);
			Value = atof(valstr);
			Plate->OrtX.Y = Value;
			Line.Delete(0, pos+1);
			pos = Line.Find("=",0); //Z = 
			valstr = Line.Mid(pos+1);
			Value = atof(valstr);
			Plate->OrtX.Z = Value;
			continue;
		}
		if ((pos0 = Line.Find("OrtY",0)) >=0) {
			pos = Line.Find("=", 0); //X =
			valstr = Line.Mid(pos+1);
			Value = atof(valstr);
			Plate->OrtY.X = Value;
			Line.Delete(0, pos+1);
			pos = Line.Find("=", 0); //Y =
			valstr = Line.Mid(pos+1);
			Value = atof(valstr);
			Plate->OrtY.Y = Value;
			Line.Delete(0, pos+1);
			pos = Line.Find("=",0); //Z = 
			valstr = Line.Mid(pos+1);
			Value = atof(valstr);
			Plate->OrtY.Z = Value;
			continue;
		}
		if (Line.Find("Degree",0)>=0){
			pos = Line.Find("=", 0);
			valstr = Line.Mid(pos+1);
			Value = atof(valstr);
			Plate->SmoothDegree = (int)Value;
		}
		//if (Line.Find("DEPOSIT",0)>=0) break; //endInfo = TRUE;

	} // feof
	fclose(fin);

	if (Plate->Number < 0) {
		Line = CString(name);
		pos = Line.Find("load", 0);
		if (pos < 0) {
			Plate->Number = 1000; return 0;}
		Line.Delete(pos, 4);
		pos = Line.Find("txt", 0);
		Line.Delete(pos-1, 4);
		valstr = Line;
		Value = atof(valstr);
		Plate->Number = (int)Value;
	}

	if (foundFree) return TRUE;// OptFree = TRUE;
	else return FALSE;
}



int CBTRDoc:: ReadLoadArray(char* name, CLoad* Load, bool isfree)
{
	if (Load == NULL) return 0;
	//if (Load->Nx * Load->Ny == 0) return;
	//if (Load->StepX * Load->StepY < 1.e-8) return;
	//AfxMessageBox("Enter ReadLoadArray");

	FILE * fin;
	if ( (fin = fopen(name, "r")) == NULL) {
		CString S = "Can't find/open ";
		S +=  name;
		logout << S << "\n";
		//AfxMessageBox(S, MB_ICONSTOP | MB_OK);
		return 0;
	}
	char buf[1024];
	double x, y, load;
	double stepX = Load->StepX;
	double stepY = Load->StepY;
	int i, j;
	if (isfree) {
		for (int k = 0; k < 5; k ++) fgets(buf, 1024, fin);
	}

	while (!feof(fin)) {
		fgets(buf, 1024, fin);
		int result = sscanf(buf, "%lf %lf %lf",  &x, &y, &load);
		if (result < 3) continue;
				
		else {
			i = (int) (x / stepX);
			if ( i*stepX - x > 0.5*stepX) i--; 
			if ( i*stepX - x < -0.5*stepX) i++; 
			j = (int) (y / stepY);
			if ( j*stepY - y > 0.5*stepY) j--; 
			if ( j*stepY - y < -0.5*stepY) j++; 
			if (i <= Load->Nx  && j <= Load->Ny)
			Load->Val[i][j] = load;
		}
		//else	fgets(buf, 1024, fin);
	}
	fclose(fin);
	return 1;
}

void CBTRDoc:: ReadIonPowerX()
{
	//WriteSumReiPowerX("Sum_ReiX.dat");
	//WriteSumAtomPowerX("Sum_AtomX.dat");
	//CString name = "Sum_X.dat";
	FILE * fin;
	CString S;
	fin = fopen("Sum_X.dat", "r");
	if (fin == NULL) fin = fopen("Sum_ReiX.dat", "r");
	if (fin == NULL) fin = fopen("Sum_ReiX.txt", "r");
	if (fin == NULL)  {
		S = "Failed to read IonPower along X";
		//AfxMessageBox(S, MB_ICONSTOP | MB_OK);
		return;
	}
	SumReiPowerX.RemoveAll();

	char buf[1024];
	double x, y;
	int i, j=0;
	for (int k = 0; k <2; k ++) fgets(buf, 1024, fin);
	while (!feof(fin)) {
		int result = fscanf(fin, " %le  %le", &x, &y); 
		//	fprintf(fout, "\t%-7.3f \t %-7.3f \t %-12.3le \n", x,y,load);
		if (result != 2) {
				fgets(buf, 1024, fin);
				continue;
			}
		else {
			i = (int) (x / SumPowerStepX);
			SumReiPowerX.Add(y);
			j++;// data line
		}
		//else	fgets(buf, 1024, fin);
	}
	fclose(fin);
		//S.Format("%d lines accepted, array size - %d", j, SumPowerX.GetSize()); 
		//AfxMessageBox(S);
		
}

void CBTRDoc:: ReadAtomPowerX()
{
	//WriteSumReiPowerX("Sum_ReiX.dat");
	//WriteSumAtomPowerX("Sum_AtomX.dat");
	//CString name = "Sum_X.dat";
	FILE * fin;
	CString S;
	fin = fopen("Sum_AtomX.dat", "r");
	if (fin == NULL) fin = fopen("Sum_AtomX.txt", "r");
	if (fin == NULL)  {
		S = "Failed to read AtomPower along X";
		//AfxMessageBox(S, MB_ICONSTOP | MB_OK);
		return;
	}

	SumAtomPowerX.RemoveAll();

	char buf[1024];
	double x, y;
	int i, j=0;
	for (int k = 0; k <2; k ++) fgets(buf, 1024, fin);
	while (!feof(fin)) {
		int result = fscanf(fin, " %le  %le", &x, &y); 
		//	fprintf(fout, "\t%-7.3f \t %-7.3f \t %-12.3le \n", x,y,load);
		if (result != 2) {
				fgets(buf, 1024, fin);
				continue;
			}
		else {
			i = (int) (x / SumPowerStepX);
			SumAtomPowerX.Add(y);
			j++;// data line
		}
		//else	fgets(buf, 1024, fin);
	}
		fclose(fin);
		//S.Format("%d lines accepted, array size - %d", j, SumPowerX.GetSize()); 
		//AfxMessageBox(S);
		
}

void CBTRDoc::OnResultsSaveall() 
{
	SaveLoads(FALSE);
//	NeedSave = TRUE;
/*	int res;
	if (!OptFree) {// Standard Geom
		res = AfxMessageBox("Standard NBI geometry can be also stored as Free Surfaces.\n Do you want to Save it Free?",
			MB_YESNOCANCEL); 
		if (res == IDCANCEL) return;
		else if (res == IDYES) { // Save Standard As Free
			OptFree = TRUE;
			SaveLoads(TRUE); // + save SumPower included
			return;
		}
		else SaveLoads(FALSE); // Save Standard
		return;
	} // OptFree = FALSE
	
	else 
	SaveLoads(TRUE); // OptFree = TRUE -> Save Free
	*/
}

void CBTRDoc::OnResultsSaveList()
{
	CFileDialog dlg(FALSE, "dat;txt | * ", "AllLoads.txt", OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"Loads Summary file (*.dat); (*.txt) | *.dat; *.DAT; *.txt; *.TXT | All Files (*.*) | *.*||", NULL);
	
	if (dlg.DoModal() == IDOK) {
		FILE * fout;
		char name[1024];
		
		strcpy(name, dlg.GetPathName());
		fout = fopen(name, "w");
		//fprintf(fout, Text);
		
	//FILE * fout = fopen("LoadsList.txt", "w");
		fprintf(fout, "Power Loads Summary \n");
		fprintf(fout, " Num      Total, W     Max, W/m2     Steps,m   <species>  Comment\n");

		CPlate * plate;
		CString S;
		//double percent;
		int k;
		for (int i = 0; i < LoadSelected; i++) {
			k = i;
			if (Sorted_Load_Ind.GetSize() > 1) // sorted 
				k = Sorted_Load_Ind[i]; 
			plate = Load_Arr[k];
	/*	percent = (plate->Load->Sum) / (pDoc->NeutrPower)/10000;
		S.Format(" %d    %10.4e    %10.4e    %0.4f", plate->Number, plate->Load->Sum,  plate->Load->MaxVal,  percent);*/
			S.Format(" %3d    %10.3e    %10.3e   %g / %g  <%s> ", 
				plate->Number, plate->Load->Sum,  plate->Load->MaxVal,
				plate->Load->StepX, plate->Load->StepY, plate->Load->Particles);
			
			S += plate->Comment;
			fprintf(fout, S + "\n");
		}
		fclose(fout);
	} 
}
void CBTRDoc::SaveRUNSummary(CString name)
{
	FILE * fout;
	fout = fopen(name, "w");
		
	CTime tm = CTime::GetCurrentTime();
	//StartTime = tm;	StopTime = tm;
	//CString CurrDate, CurrTime;
	CString Date, Time;
	Date.Format("%02d-%02d-%04d", tm.GetDay(), tm.GetMonth(), tm.GetYear());
	Time.Format("%02d:%02d:%02d", tm.GetHour(), tm.GetMinute(), tm.GetSecond());

	CString S, Scapt, Sscen, Sopt, Srun;
	Scapt.Format("Power Loads Summary  %s  %s\n", Date, Time);
	Sscen.Format("SCEN #%d  {'Version 5 TEST' - to modify}\n", SCEN);
	Sopt = "";
	if (OptTraceAtoms) Sopt += " + ATOMS";
	if (!OptNeutrStop) Sopt += " + Residual Ions";
	if (!OptReionStop) Sopt += " + Reions";
	Srun.Format("RUN  #%d  Options: %s\n", RUN, Sopt);

	//FILE * fout = fopen("LoadsList.txt", "w");
	fprintf(fout, Scapt);//1
	fprintf(fout, Sscen);//2
	fprintf(fout, Srun);//3

	// write TRACKED POWER param
	double Pn = GetNeutrPower();// at Neutr Exit plane
	double Pinj = GetInjectedPowerW();
	double Psolid = GetTotSolidPower();// Area Exit excluded

	if (RUN == 1) {
		PowNeutral[SCEN] = Pn;   // taken from run 1
		PowInjected[SCEN] = Pinj; // from run 1
		PowSolid[SCEN].X = Psolid; // X - run 1
	}
	if (RUN == 2) PowSolid[SCEN].Y = Psolid;// Y - run 2
	if (RUN == 3) PowSolid[SCEN].Z = Psolid;// Z - run 3

	S.Format("--- SCEN %d RUN %d Pneutr=%10.2g  Pinj=%10.2g  Psolid = %10.2g\n", 
					SCEN, RUN, Pn, Pinj, Psolid);  
	logout << S;

	fprintf(fout, "Neutral power, W  = %10.2g\n", Pn);//4
	fprintf(fout, "Injected power, W = %10.2g\n", Pinj);//5
	fprintf(fout, "Solid power, W    = %10.2g\n", Psolid);//6
	
	fprintf(fout, "Surf      Total, W     Max, W/m2     GridSteps,m\n");//7
	CPlate * plate;
		
	int k;
	for (int i = 0; i < LoadSelected; i++) {
		k = i;
		if (Sorted_Load_Ind.GetSize() > 1) // sorted 
		k = Sorted_Load_Ind[i];
		plate = Load_Arr[k];
		//	percent = (plate->Load->Sum) / (pDoc->NeutrPower) /10000;
		S.Format("%3d   %10le    %10le   %5g  %5g\n",
				plate->Number, plate->Load->Sum, plate->Load->MaxVal,
				plate->Load->StepX, plate->Load->StepY);//, plate->Load->Particles);
		//S += plate->Comment;
		fprintf(fout, S);
	}
	fclose(fout);
	S.Format("\nWrite Scen %d Run %d SUMMARY >>> %s \n", SCEN, RUN, name);
	logout << S;
}

void CBTRDoc::OnEditPlasmaTeNe()
{
	//if (!OptBeamInPlasma) return;
	char name[1024];
//	char buf[1024];
//	CFileDialog dlg(TRUE, "dat", "*.dat");
	CFileDialog dlg(TRUE, "dat; txt | * ", "", OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"psi Te Ne Ti Zeff columns  (*.txt);(*.dat)  | *.txt; *.TXT; *.dat; *.DAT | All Files (*.*) | *.*||", NULL);
	
	if (dlg.DoModal() == IDOK) {
		strcpy(name, dlg.GetPathName());
		//CWaitCursor wait;
		CPreviewDlg pdlg;
		pdlg.m_Filename = name;
		if (pdlg.DoModal() == IDCANCEL) {
			pPlasma->ProfilesLoaded = FALSE; // use parabolic
			return;
		}
		//SetTitle(name);// + TaskName);
		//BOOL res = 
		
		//pPlasma->ProfilesLoaded = pPlasma->ReadProfilesWithSigmas(name); 
		// same as ReadProfiles + 4 colums (SigmaCX, II, EI, Delta)
		
		pPlasma->ProfilesLoaded = pPlasma->ReadProfiles(name);
		MaxPlasmaDensity = pPlasma->Nemax;
		MaxPlasmaTe = pPlasma->Temax;
				//pPlasma->ProfilesLoaded = FALSE -> use parabolic
	}
	else {
		pPlasma->ProfilesLoaded = FALSE; // use parabolic
		pPlasma->SetParProfiles(); // parabolic
		//return;
	}
	
	PlotPSIArrays(); // profiles along PSI
	

	//SetDecayInPlasma(); // fill decay array

}

void CBTRDoc:: ReadPlasmaProfiles(char* name) //moved to plasma obj
{	
	FILE * fin;
	CString S;
	if ( (fin= fopen(name, "r")) == NULL) {
		S ="Can't find/open ";
		S+= name;
		AfxMessageBox(S, MB_ICONSTOP | MB_OK);
		return;
	}

	double MaxDensity = 0;
	double MaxTe = 0;
	PlasmaProfileNTZ.RemoveAll();
	PlasmaProfilePSI.RemoveAll();

	char buf[1024];
	double psi, Te, Ne, Ti, Zeff;
	C3Point P;
	int i, j=0;
	//for (int k = 0; k <2; k ++) 
	fgets(buf, 1024, fin);
	while (!feof(fin)) {
		int result = fscanf(fin, " %le  %le  %le  %le  %le", &psi, &Te, &Ne, &Ti, &Zeff); //Te,keV	Ne,10^19/m3
		//	fprintf(fout, "\t%-7.3f \t %-7.3f \t %-12.3le \n", x,y,load);
		if (result != 5) {
				fgets(buf, 1024, fin);
				continue;
			}
		else {
			fgets(buf, 1024, fin);
			//i = (int) (x / SumPowerStepX);
			P.X = Ne; P.Y = Te; P.Z = Zeff; //Te,keV	Ne,10^19/m3
			PlasmaProfileNTZ.Add(P);
			PlasmaProfilePSI.Add(psi);
			MaxDensity = Max(MaxDensity, Ne);
			MaxTe = Max(MaxTe, Te);
			j++;// data line
		}
		//else	fgets(buf, 1024, fin);
	}
		fclose(fin);

		if (MaxDensity > 1.e-3) MaxPlasmaDensity = MaxDensity;
		if (MaxTe > 1.e-3) MaxPlasmaTe = MaxTe;


		if (j < 1) {
			AfxMessageBox("invalid number of columns (must be 5)");
			ProfLoaded = FALSE;
			PlasmaLoaded = FALSE;
			return;
		}

		if (MaxDensity * MaxTe > 1.e-6) ProfLoaded = TRUE;
		else ProfLoaded = FALSE;
		
		if (ProfLoaded && PSILoaded) PlasmaLoaded = TRUE;
		else PlasmaLoaded = FALSE;

		//MaxPlasmaDensity *= 1.e19;
		//S.Format("%d lines accepted, array size - %d", j, PlasmaProfileNTZ.GetSize()); 
		//AfxMessageBox(S);
		
}




void CBTRDoc::OnUpdateOptionsRealionsourcestructure(CCmdUI* /* pCmdUI */) 
{
/*	pCmdUI->SetCheck((int)*ISourceType); */
}

void CBTRDoc::OnDataSave() 
{
	if (!STOP) OnStop();
	/*{
		AfxMessageBox("Stop calculations before storing data! \n (Red cross on the Toolbar)");
		return ;
	} */
	//STOP = TRUE;
	
	//CWaitCursor wait; 
	SaveData();
	OnShow();

}

void CBTRDoc::ReadResultsSingle()
{
	char DirName[1024];
	CString OldDirName = CurrentDirName; // old path
	logout << "Current Config path " << OldDirName << "\n"; 
		
	int nread = ReadData(); // Read text-> to m_Text 
	CString NewDirName = CurrentDirName; // new - SCEN folder
	//::GetCurrentDirectory(1024, NewDirName);// SCEN Config
	logout << "Current Config path " << NewDirName << "\n"; 
	if (nread == 0) return; // failed
	
	// RETURN BACK HOME  
	CurrentDirName = OldDirName;
	::SetCurrentDirectory(CurrentDirName);// go up("..\\"); - above scen
	logout << "Current folder " << CurrentDirName << "\n"; 
	
	InitOptions(); //MAXSCEN = 1  empty Fields, AddSurfName, Set default tracing options
	//InitTrackArrays();// clear 
	UpdateDataArray(); // m_Text -> SetData SetOption CheckData 
	SetFields(); // GAS + MF from files if found in Config
	CheckData();
	SetBeam(); // Aimings, IS positions
	InitPlasma(); // called in OnDataGet
	InitAddPlates(); // Remove AddPlatesList, set AddPlatesNumber = 0;
	SetPlates();//SetPlasma is called from SetPlatesNBI
	SetAddPlates();//ReadAddPlates(AddSurfName);
	if (AddPlatesNumber > 0)	AppendAddPlates();
	SetStatus();

	CurrentDirName = NewDirName;
	::SetCurrentDirectory(CurrentDirName);// SCEN folder
	logout << "Current folder " << CurrentDirName << "\n"; 
	 
/*	CString stot = "Loads_total";
	CString path = stot + "\\" + "3-col" + "\\";  
	logout << path << "\n";*/

	SetLoadArray(NULL, FALSE);//clear
		
	CString path = ""; // empty
	int count = ReadLoads3_AtPath(path);// count non-zero loads

	CString S;
	S.Format(" %d NON_ZERO loads found \n %d total Loads listed\n", count, LoadSelected);
	logout << S;
	OptCombiPlot = -1;//-1 - no load, 0 - map is calculated, >0 - selected but not calculated 
	//ModifyArea(FALSE);
	OnPlateClearSelect();
	Progress = 0;
	//OptParallel = FALSE; // CalculateCombi(0)
	OnShow();
}


void CBTRDoc::OnResultsReadall() 
{
	ReadResultsSingle();
	return;

	//////////// OLD /////////////////
	OptFree = FALSE; // set STANDARD config - by default
	DINSELECT = FALSE;	
	
	if (OnDataGet() == 0) return;// read config failed
	
	SetLoadArray(NULL, FALSE);//clear
	
	BeginWaitCursor(); //OnShow();
	WIN32_FIND_DATA FindFileData; //fData;
	HANDLE hFind;
	// LPTSTR DirSpec = (LPTSTR)"*.dat";
	char name[1024];
	CString  sn;

	char OpenDirName[1024];
	::GetCurrentDirectory(1024, OpenDirName);
	//::SetCurrentDirectory(OpenDirName);
	SetTitle(OpenDirName);// + TaskName);

  // Find the first file in the directory.
   hFind = FindFirstFile("*.txt", &FindFileData);

   if (hFind == INVALID_HANDLE_VALUE) AfxMessageBox("Invalid file handle");
 
   else  {
	   sn = (CString) (FindFileData.cFileName);
	   if (sn.Find("load", 0) >= 0) {
		strcpy(name, sn);
		ReadLoad(name);
	   }
   //}
  
      while (FindNextFile(hFind, &FindFileData) != 0) 
      {
         //_tprintf (TEXT("Next file name is: %s\n"),  FindFileData.cFileName);
		  sn = (CString) (FindFileData.cFileName);
		  if (sn.Find("load", 0) >= 0) {
			  strcpy(name, sn);
			  ReadLoad(name);
		  } // if
      } // while
   } //valid handle
     // dwError = GetLastError();
    FindClose(hFind);
	EndWaitCursor();

	OptCombiPlot = -1;//-1 - no load, 0 - map is calculated, >0 - selected but not calculated 
	//ModifyArea(FALSE);
	OnPlateClearSelect();
	Progress = 0;
	//OptParallel = FALSE; // CalculateCombi(0)
	OnShow();
}

bool CBTRDoc:: CorrectPDPInput()
{
	WIN32_FIND_DATA fData;
	HANDLE h;
//	char name[1024];
	char * fname;
	CString  S = "";
	int pos, pos1, reply = -1;
	bool found = false;
 
	::SetCurrentDirectory(CurrentDirName);

	// .TXT 
	h = ::FindFirstFile("*.txt", &fData); 
	do {// (h != NULL) {
		//		if (h == (HANDLE) 0xFFFFFFFF) break;
		if (h == INVALID_HANDLE_VALUE) { 
			found = false;
			break;
		}
		fname = fData.cFileName;
		S = fname;
		S.MakeUpper();
		pos = S.Find("INP");
		pos1 = S.Find("BEAM");
		if (pos >= 0 && pos1 < 0) { // not beamlets file!
			CorrectFile(fname);
			found = true;

			CPreviewDlg dlg;
			dlg.m_Filename = fname;
			if (dlg.DoModal() == IDCANCEL)
				found = false; //return 0;
			else
				return true;
		}
	} while (::FindNextFile(h, &fData));

	// .DAT
	h = ::FindFirstFile("*.dat", &fData); 
	do {// (h != NULL) {
		//		if (h == (HANDLE) 0xFFFFFFFF) break;
		if (h == INVALID_HANDLE_VALUE) { 
			found = false;
			break;
		}
		fname = fData.cFileName;
		S = fname;
		S.MakeUpper();
		pos = S.Find("INP");
		pos1 = S.Find("BEAM");
		if (pos >= 0 && pos1 < 0) { // not beamlets file!
			CorrectFile(fname);
			found = true;
			
			CPreviewDlg dlg;
			dlg.m_Filename = fname;
			if (dlg.DoModal() == IDCANCEL)
				found = false;
			else
				return true;
		}
	} while (::FindNextFile(h, &fData));

	if (!found)
		reply = AfxMessageBox("INPUT file not found or rejected \n Continue anyway?", 3); 
	return (reply == IDYES);
}

void CBTRDoc::CorrectFile(char * name)
{
	FILE * fin;
	FILE * fout;

	fin = fopen(name, "r");
	if (fin==NULL){
		AfxMessageBox("input file not found", MB_ICONSTOP | MB_OK);
		return;
	}

	char buf[1024];
	CString text; 
	
	int result;

	double Eo, Iacc;
	if (fgets(buf,1024,fin) == NULL) { AfxMessageBox("Problem 0 with file"); fclose(fin); return; }
		
	result = sscanf(buf, "%lf %lf", &Eo, &Iacc); 
		
	if (result < 2) {
			text ="";
			while (!feof(fin) && result < 2) {
				fgets(buf, 1024, fin);
				result = sscanf(buf, "%lf %lf", &Eo, &Iacc);
			}
			if (feof(fin)) { AfxMessageBox("Data not found"); fclose(fin); return;}
			text += buf;
		
			while (!feof(fin)) {
				fgets(buf, 1024, fin);
				if (!feof(fin)) text += buf;
			}
		
			fclose(fin); 
	
		fout = fopen(name, "w");
		if (fout==NULL){
			AfxMessageBox("problem opening data file", MB_ICONSTOP | MB_OK);
			return;
		}
		fprintf(fout, text);
		fclose(fout);

	} // result < 2

}

void CBTRDoc::OnOptionsSingap() 
{
	OptSINGAP = !OptSINGAP;
	if (OptSINGAP && !SINGAPLoaded) {
		ReadSINGAP();
		SetSINGAP();
	}
	pMainView->InvalidateRect(NULL, TRUE);
	if (OptSINGAP) TaskName = "SINGAP BEAM";
	else TaskName = "MAMuG BEAM";

	Progress = 0;
	OnShow(); //OnDataActive();

}

void CBTRDoc::OnUpdateOptionsSingap(CCmdUI* pCmdUI) 
{
	
//	pCmdUI->SetCheck(OptSINGAP);
	if (OptSINGAP) pCmdUI->SetText("Choose MAMuG");
	else pCmdUI->SetText("Choose SINGAP");
}

void CBTRDoc::OnTasksRid() 
{
/*
	Progress = 0;
	CurrentDirName = TopDirName;
	::SetCurrentDirectory(CurrentDirName);
	SetTitle(CurrentDirName);
	
	TaskReionization = FALSE;
	TaskRID = TRUE;//!TaskRID;
	if (LoadSelected > 0) SelectAllPlates(); // empty the plates list
	InitTaskRID();
	SetPlates(); */
	
	CurrentDirName = TopDirName;
	::SetCurrentDirectory(CurrentDirName);

	CRIDDlg dlg;
	dlg.SetDocument(this);
	if (dlg.DoModal() == IDOK) {// APPLY button pushed
		RIDField->Copy(dlg.Nx, dlg.Ny, dlg.StepX, dlg.StepY, dlg.X0, dlg.NodeU);
		RIDFieldLoaded = TRUE;
		TaskName = "RID model applied";

	}

	//TaskName = " ION POWER in RID";

	OnShow();
}

void CBTRDoc::OnUpdateTasksRid(CCmdUI* pCmdUI) 
{
	//pCmdUI->Enable(FALSE);
	pCmdUI->Enable(!OptFree);
	pCmdUI->SetCheck(TaskRID);
	
}


/*
UINT ThreadProc(LPVOID pParam) /// not used
{
	CCriticalSection cs;
	//CDC * pDC = (CDC*) pParam;
	//if (pDC == NULL || !pDC->IsKindOf(RUNTIME_CLASS(CDC))) return -1;
	CBTRDoc * pDoc = (CBTRDoc*) pParam;
	if (pDoc == NULL || !pDoc->IsKindOf(RUNTIME_CLASS(CBTRDoc))) return -1;
	if (pDoc->STOP) return 0;

/*	cs.Lock();
	HDC DC = pMainView->GetDC()->GetSafeHdc();
	CString s;
	s.Format("%d", pDoc->ThreadNumber);

	int k = pDoc->ThreadNumber * 20;
	Rectangle(DC, k, k, k+100, k+100);
	TextOut(DC, k+1, k+1, s, s.GetLength());
	GdiFlush();
	cs.Unlock();
*/
/*	//pDoc->EmitBeam(OptSINGAP);
	int res;
	if (pDoc->OptSINGAP) 
		res = pDoc->EmitSINGAP(); // 0 - if stopped; 1 - if finished
	else 
		res = pDoc->EmitMAMuG(); // 0 - if stopped; 1 - if finished

	//Sleep(1000);
	return res;
}

void CBTRDoc::OnParallel() /// not used
{
	CWinThread * pThread[2000];// max - 16
	//int ThreadNumber;
	int NofThreads = 30;
	int res;
	if (pDataView->m_rich.GetModify()) {
		res = AfxMessageBox("Data is modified but NOT UPDATED! \n Continue?", 3);
		if (res != IDYES) return;
	}
	if (LoadSelected > 0) {
		res = AfxMessageBox("Keep Surfaces selection as is?",3);
		
		if (res == IDYES) ClearAllPlates();
		else if (res == IDNO) OnOptionsSurfacesEnabledisableall();
		else return;
	}

	CString s;
	//Beep(100,100);
	Progress = 0;
	STOP = FALSE;
	
	pMainView->STOP = FALSE;
	//pMainView->InvalidateRect(NULL, TRUE);
		
//	int finished = EmitBeam(OptSINGAP);// run beam in series

// begin parallel beam ---------------------------------------------------
	OptDrawPart = FALSE;
	OptParallel = TRUE;
	//int prior = THREAD_PRIORITY_LOWEST; // _LOWEST, _BELOW_NORMAL, _NORMAL, _ABOVE_NORMAL, _HIGHEST
	int i;
	int finished = 0;
	//CDC* pDC = pMainView->GetDC();
	/*for (i = 0; i < NofThreads; i++)
	{
		if (pThread[i] != NULL) {
			WaitForSingleObject(pThread[i]->m_hThread, INFINITE);
			delete pThread[i];
		}
	}*/

/*	for (i = 0; i < NofThreads; i++)
	{
		/*pThread[i] = new CWinThread(ThreadProc, pDC->GetSafeHdc());
		if (pThread[i] == NULL) break;
		pThread[i] ->m_bAutoDelete = FALSE;
		if (!pThread[i]->CreateThread(CREATE_SUSPENDED))
		{
			delete pThread[i];
			pThread[i] = NULL;
			break;
		}
		VERIFY(pThread[i]->SetThreadPriority(prior));
		pThread[i]->ResumeThread();*/

		//ThreadNumber = i;
	/*	pThread[i] = ::AfxBeginThread(ThreadProc, this);//, prior);
		if (STOP) {
			finished = 0;
			break;
		}
		finished = 1;
		//Sleep(6000); // ms
	
	}
// end parallel beam ---

	if (finished == 1) s.Format(" FINISHED    "); //Progress = 100;
	else s.Format(" Beam STOPPED  ");//Progress = 50;
	STOP = TRUE;
	Progress = 100;
	SetLoadArray(NULL, TRUE);
	if (finished == 0) DetachLoadFiles();
	AfxMessageBox(s, MB_ICONEXCLAMATION | MB_OK);
	ShowStatus();
	Progress = 0;
//	pMainView->InvalidateRect(NULL, FALSE);
}
// <--- not used
*/

void CBTRDoc::OnStop() // stop (or suspend - commented) threads
{
	int i;
	pMainView->STOP = TRUE;
	pLoadView->STOP = TRUE;
	
	StopTime = CTime::GetCurrentTime();
	
	CString S;
	if (OptParallel && !STOP) { //NofCalculated > 0) {// started = TRUE
	//if (STOP) { //Paused
		
		for (i = 0; i < ThreadNumber; i++) {
			m_Tracer[i].SetContinue(FALSE);
			//m_Tracer[i].DumpArrays();
		}
		//SuspendAll(TRUE);// works bad
		for (i = 0; i < ThreadNumber; i++) 
			WaitForSingleObject(m_pTraceThread[i]->m_hThread, 1000); //INFINITE);
			
		//NofCalculated = 0;// to check if everything works!!!
		//SetTitle("STOPPED");

	//} // if paused
/*	else { //not paused yet
		SuspendAll(FALSE);// run = FALSE
		TbeginSuspend = CTime::GetCurrentTime();
		pMainView->ShowNBLine(); pMainView->ShowCoord();
		s.Format("Beam Paused (%d)", ThreadNumber);
		AfxMessageBox(s);
		SetTitle("PAUSED");
		
	}*/
	
	} //OptParallel && NofCalculated > 0)
	
	STOP = TRUE;
	
	//SetTitle("STOPPED"); // in all cases!
	//ShowStatus();
	
	S.Format(" -->[STOP]<--\n\n", BTRVersion);
	//SetTitle(S);
	logout << S;
		
	//OnShow();
	//SwapMouseButton(TRUE);
	
}
void CBTRDoc::CollectRUNSummary(CString SUMname, CStringArray & names) 
// merge load list results
{
	FILE * fin;
	FILE * fout;
	CString S, Scomm;
	CPlate * plate;
	char name[1024];
	int n, line, nmax = 0;
	double pow, PDmax, stepX, stepY;
	char * text;// string part not used: incl steps, sort, definition
	char buf[1024];
	
	int Num[1000];// surf number
	double Pow_A[1000];// atoms
	double Pow_Resid[1000];// resid
	double Pow_Reion[1000];// reion
	double SumPow[1000]; // total added
	double Max_A[1000];// atoms
	double Max_Resid[1000]; // resid
	double Max_Reion[1000]; // reion
	for (int i = 0; i < 1000; i++) {
		Num[i] = -1;
		Pow_A[i] = 0;
		Pow_Resid[i] = 0;
		Pow_Reion[i] = 0;
		SumPow[i] = 0;
		Max_A[i] = 0;
		Max_Resid[i] = 0;
		Max_Reion[i] = 0;
	}
	BOOL optA, optResid, optReion;// each file options
	BOOL SUMoptA = FALSE, SUMoptResid = FALSE, SUMoptReion = FALSE; // SUMMARY options
////// READ LOAD-FILES //////////////// 	
	int kmax = names.GetSize();
	for (int k = 0; k < kmax; k++)
	{
		strcpy(name, names[k]);
		fin = fopen(name, "r");
		if (fin == NULL) {
			S = " - Failed to read " + names[0];
			logout << S << "\n";
			//AfxMessageBox(S, MB_ICONSTOP | MB_OK);
			return;
		}
		optA = FALSE;
		optResid = FALSE;
		optReion = FALSE;

		for (int i = 0; i < 2; i++) {// skip caption "Scen loads Summary"
			fgets(buf, 1024, fin);//1st 2 lines
		}
		
		fgets(buf, 1024, fin); // 3rd line - options
		S.Format("%s", buf);
		if (S.Find("ATOM", 0) >= 0) {
			optA = TRUE; SUMoptA = TRUE;
		}
		if (S.Find("Resid", 0) >= 0) {
			optResid = TRUE; SUMoptResid = TRUE;
		}
		if (S.Find("Reion", 0) >= 0) {
			optReion = TRUE; SUMoptReion = TRUE;
		}

		// Read TRACKED power - 3 lines: 4-6
		int pos;
		double Pn, Pinj, Psolid;
		CString vs;
		for (int i = 0; i < 3; i++) { 
			fgets(buf, 1024, fin); //4,5,6
			S.Format("%s", buf);
			if ((pos = S.Find("=", 0)) > 1) {
				vs = S.Mid(pos+1);
				if (S.Find("Neutr", 0) > -1) Pn = atof(vs);
				else if (S.Find("Inject", 0) > -1) Pinj = atof(vs);
				else Psolid = atof(vs);
			} // "=" found
		}// i

		fgets(buf, 1024, fin); // 7th line - column names
		S.Format("%s", buf);

		//READ load sum data --------------------
		
		line = 0;
		while (!feof(fin)) {
			int result = fscanf(fin, "%d  %le  %le  %le  %le", &n, &pow, &PDmax, &stepX, &stepY);
			//	fprintf(fout, "\t%-7.3f \t %-7.3f \t %-12.3le \n", x,y,load);
			if (result != 5) {
				fgets(buf, 1024, fin);	//S.Format("%s", buf); std::cout << S;
				continue;
			}
			else {
				Num[n] = n;
				nmax = max(n, nmax);
				if (optA) {
					Pow_A[n] = pow;
					Max_A[n] = PDmax;
				}
				if (optResid) {
					Pow_Resid[n] = pow;
					Max_Resid[n] = PDmax;
				}
				if (optReion) {
					Pow_Reion[n] = pow;
					Max_Reion[n] = PDmax;
				}
				SumPow[n] += pow;
				line++;// data line
			}
		}
		fclose(fin);
		S.Format("+ %s: read %d lines with Surf results\n", name, line);
		logout << S;
		::DeleteFile(name);
		S.Format("  >>>> DELETE %s\n", name);	
		logout << S;

	} // kmax total list-files ////////////////////// 

	//ofstream csvf; // EXCEL format
	//csvf.open(SUMname + ".csv");

	strcpy(name, SUMname + ".txt"); // create SUMMARY file
	fout = fopen(name, "w");
	fprintf(fout, "Scen Loads Summary \n");
	S = "Summary Options: ";
	if (SUMoptA) S += " + ATOMS";
	if (SUMoptResid) S += " + Residual ions (Neg/Pos)";
	if (SUMoptReion) S += " + Reions (Pos)";
	S += "\n";
	fprintf(fout, S);

	/*S.Format("Neutral Power (RUN 1), W = %g\n", PowNeutral[SCEN]);
	fprintf(fout, S);
	S.Format("Injected Power (RUN 1), W = %g\n", PowInjected[SCEN]);
	fprintf(fout, S);
	S.Format("Deposited Power Atoms / Resid / Reion, W = %g / %g / %g \n", 
			PowSolid[SCEN].X, PowSolid[SCEN].Y, PowSolid[SCEN].Z);
	fprintf(fout, S); */

	fprintf(fout, " Num   A_Power  Resid_Power  Reion_Power     SUM[W]     A_Max[W/m2]  Resid_Max  Reion_Max    Comment\n");
	
	//csvf << " Num ; A_Power ; Resid_Power ; Reion_Power ; SUM[W] ; A_Max[W/m2] ; Resid_Max ; Reion_Max ; Comment" << endl;
	for (int i = 0; i <= nmax; i++) {
		if (Num[i] > 0) {
			n = Num[i];
			plate = GetPlateByNumber(Num[i]);
			Scomm = plate->Comment;
			if (SumPow[i] > 1.e-6) {// write NON-ZERO!!
				fprintf(fout, "%3d  %0.3le  %0.3le  %0.3le    %0.3le    %0.3le  %0.3le  %0.3le \t%s\n",
					Num[i], Pow_A[i], Pow_Resid[i], Pow_Reion[i], SumPow[i], 
					Max_A[i], Max_Resid[i], Max_Reion[i], Scomm);
				//csvf << Num[i] << ";" << Pow_A[i] << ";" << Pow_Resid[i] << ";"
					//<< Pow_Reion[i] << ";" << SumPow[i] << ";" << Max_A[i] << ";"
					//<< Max_Resid[i] << ";" << Max_Reion[i] << ";" << Scomm << endl;
				//ScenLoadSummary[i][n];// 1st index - scen
				ScenLoadSummary[SCEN][n] = SumPow[i];// 1st index - scen, 2nd - Surf
			}
		}
	}
	fclose(fout);
	//csvf.close();
}

int  CBTRDoc::GetPrefFiles(CString pref, CStringArray & names) 
{
	int size = 0; // files found
	CString S;// = "\t Files found";
	WIN32_FIND_DATA FindFileData; //fData;
	HANDLE hFind;
	CString  sname;

	char OpenDirName[1024];
	CString DirName;
	::GetCurrentDirectory(1024, OpenDirName);
	S.Format("\n  %s List <%s>:\n", pref, OpenDirName);
	logout << S;// << std::endl;*///////////
	// Find the first file in the directory.
	hFind = FindFirstFile("*.txt", &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE) AfxMessageBox("GetPrefFiles: Invalid file handle");

	else {
		sname = (CString)(FindFileData.cFileName);
		if (sname.Find(pref, 0) >= 0 && sname.Find("SUM", 0) <= 0) {
			names.Add(sname);
		}
	}

	while (FindNextFile(hFind, &FindFileData) != 0)
	{
		sname = (CString)(FindFileData.cFileName);
		if (sname.Find(pref, 0) >= 0 && sname.Find("SUM", 0) <= 0) {
			//strcpy(name, sn);	//ReadLoad(name);
			names.Add(sname);
		}
	}
	// dwError = GetLastError();
	FindClose(hFind);
	//EndWaitCursor();
	size = names.GetSize();
	return size;
}

int CBTRDoc::GetFilesSubstr(CString substr, CStringArray & fnames) 
{
	fnames.RemoveAll();
	int size = 0; // files found
	CString S;// = "\t Files found";
	WIN32_FIND_DATA FindFileData; //fData;
	HANDLE hFind;
	CString  sname;

	char OpenDirName[1024];
	CString DirName;
	
	::GetCurrentDirectory(1024, OpenDirName);
	S.Format("\n  %s File List <%s>:\n", substr, OpenDirName);
	logout << S;// << std::endl;*///////////
	// Find the first file in the directory.
	hFind = FindFirstFile("*.TXT", &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE) AfxMessageBox("GetFilesSubstr: Invalid file handle");

	else {
		sname = (CString)(FindFileData.cFileName);
		if (sname.Find(substr, 0) >= 0) {
			fnames.Add(sname);
		}
	}

	while (FindNextFile(hFind, &FindFileData) != 0)
	{
		sname = (CString)(FindFileData.cFileName);
		if (sname.Find(substr, 0) >= 0) {
			fnames.Add(sname);
		}
	}
	// dwError = GetLastError();
	FindClose(hFind);
	//EndWaitCursor();
	size = fnames.GetSize();
	return size;
}

void CBTRDoc::SaveRUNLoads() // create RUN Loads-storage in current SCEN dir
{
	//CWaitCursor wait; 
	CPlate * plate;
	LPSECURITY_ATTRIBUTES  sec = NULL;//SEC_ATTRS;
	//::SetCurrentDirectory(CurrentDirName);
	//CTime t = StopTime; // 
	
	CString LoadsDirName;// loads storage
	LoadsDirName.Format("Loads%d_run%d", SCEN, RUN);
	::CreateDirectory(LoadsDirName, sec);
	::SetCurrentDirectory(LoadsDirName);

	double Coeff=1, Coeff2=1, Coeff3=1;
	if (AtomPower2 > 1e-3) Coeff2 = AtomPower1 / AtomPower2;
	if (AtomPower3 > 1e-3) Coeff3 = AtomPower1 / AtomPower3;

	if (RUN == 1) Coeff = 1;
	else if (RUN == 2) Coeff = Coeff2;
	else if (RUN == 3) Coeff = Coeff3;

	CString S;
	S.Format(">>>> SAVE Scen %d Run %d Loads to %s (created) ....\n", SCEN, RUN, LoadsDirName);
	logout << S;// << std::endl;
	S.Format("AtomPower1 = %g, AtomPower2 = %g, AtomPower3 = %g\n", 
			 AtomPower1, AtomPower2, AtomPower3);
	logout << S;
	S.Format("\t RUN Power multiplication COEFFICIENT = %g\n", Coeff);
	logout << S;
	
	FILE * fout;
	CString name;
	CString  sn;
	long totfilesize = 0;
	// Save Loads --------------------------------
		for (int i = 0; i < LoadSelected; i++) {
			plate = Load_Arr[i];
			if (plate->Touched == FALSE) continue; // skip zero load
			
			plate->CorrectLoad(Coeff);

			sn.Format("%d", plate->Number);
			name = "load_" + sn + ".txt";
			fout = fopen(name, "w");
			//if (free) plate->WriteLoadAdd(fout);
			plate->WriteLoad(fout);
			//plate->filename = name;
			fclose(fout);
			
			// define size
			FILE *pFile = fopen(name, "rb");
			long nFileLen = 0;
			if (pFile)
			{
				fseek(pFile, 0, SEEK_END);
				nFileLen = ftell(pFile);
				fclose(pFile);
			}
			//long fsize = filelength((name); //file(name); // ("file.dat);
									   //printf("Ðàçìåð ôàéëà file.dat ðàâåí %d\n", size);
			totfilesize += nFileLen;

		}
	//}// standard
		
	// Save RUN-Config File --------------------------------
/*	FormDataText(TRUE);// include internal names of parameters
	name.Format("Scen%d_Run%d_Config.txt", SCEN, RUN);
	fout = fopen(name, "w");
	fprintf(fout, m_Text);
	fclose(fout);
*/
	//WriteSumReiPowerX("Sum_ReiX.dat", -1, 100);
	//WriteSumAtomPowerX("Sum_AtomX.dat", -1, 100);
	//WriteSumPowerAngle("Sum_Angle.dat");
	//WriteExitVector();
	//WriteFallVector();

	S.Format(">>>>> Scen %d Run %d >> %d Loads are stored \n", SCEN, RUN, LoadSelected);
	logout << S;// << std::endl;

	long config_kB = 16; 
	S.Format("\n >>>>> SCEN %d Run %d RESULTS ~ %d kB <<<<<<<<<< \n\n", SCEN, RUN, totfilesize / 1024 + config_kB);
	logout << S;
	
	//::SetCurrentDirectory(CurrentDirName);//("..\\");
}

void RecursiveDelete(CString szPath)
{
	CFileFind ff;
	CString path = szPath;

	if (path.Right(1) != "\\")
		path += "\\";

	path += "*.*";

	BOOL res = ff.FindFile(path);

	while (res)
	{
		res = ff.FindNextFile();
		if (!ff.IsDots() && !ff.IsDirectory())
			DeleteFile(ff.GetFilePath());
		else if (ff.IsDirectory())
		{
			path = ff.GetFilePath();
			RecursiveDelete(path);
			RemoveDirectory(path);
		}
	}
}
void CBTRDoc::CollectRUNLoads() //called by CompleteScen
// READ loads for each surf -> WRITE FOLDER with merged loads
{
	logout << "\n ------ Merging Load-files from all RUNs ----------\n";
	char buf[1024];
	::GetCurrentDirectory(1024, buf); // SCEN folder
	CString ScenDir = CString(buf);

	CString run[3]; // subfolders in SCEN folder
	run[0].Format("Loads%d_run1\\", SCEN); // SCEN1_RUN1... SCEN1_RUN2... 
	run[1].Format("Loads%d_run2\\", SCEN);
	run[2].Format("Loads%d_run3\\", SCEN);//"Loads%d_run3\\"

	FILE * fout = fopen("loads.txt", "w"); // loads statistic

	FILE * fout1; // 6-col load
	FILE * fout2; // 3-col load
	FILE * fin;
	char name[1024];
	char * line;
	int loads = 0; // non-zero
	int found; // files for each load
	CString S, textinfo;
	int nx, ny;
	double stepx, stepy;
	int headline = 13; // header
	int maxdata = 0; // maxsize among loads to find
	int ndata = 0; // curr load data size
	CStringArray names; // loads found
	CArray <int> sizes; // load data size store
	CArray <double> Scells; // load cell square
	
	for (int iload = 0; iload < 1000; iload++) { // look for existing files
		S.Format("load_%d.txt", iload);
		found = 0;
		textinfo = ""; // info-lines
		for (int k = 0; k < 3; k++) { // look for load_i in all runs
			strcpy(name, run[k] + S);
			if ((fin = fopen(name, "r")) != NULL) {
				//if (fopen(name, "r") != NULL) {
				found++;
				if (found == 1) { // read 1st file only!
					for (int i = 0; i < headline; i++) { // read info-lines
						line = fgets(buf, 1024, fin);
						textinfo = textinfo + CString(line);
					}
					ReadLoadInfo(textinfo, nx, ny, stepx, stepy);
					names.Add(S);
					Scells.Add(stepx*stepy); // cell square

					// count data lines for this load
					ndata = 0;
					while (!feof(fin)) {
						line = fgets(buf, 1024, fin);
						ndata++;
					}
					sizes.Add(ndata);// gata length for this load
				} // file found for the 1st time

				fclose(fin);
						
				 
			} // file exists
		} // all 3 run folders

		if (found > 0) { // load found at least once
						 //fprintf(fout, " %s :: %d files - %s\n", S, found, textinfo);
			fprintf(fout, " %12s :: %d files   Nx = %4d Ny = %4d  StepX = %5g StepY = %5g  ndata = %d ->> %s\n",
					S, found, nx, ny, stepx, stepy, ndata, textinfo);
			loads++;
		}
		maxdata = Max(maxdata, ndata);
	} // iload = 1000
	fclose(fout);// info

	S.Format("%d total loads found,   max datasize - %d\n", loads, maxdata);
	logout << S;
	//std::cout << loads << " total loads found ----- max datasize - " << maxdata << "\n";

	CPlate * plate;
	LPSECURITY_ATTRIBUTES  sec = NULL;
									  
	CString LoadsDirName; // merged loads storage
	LoadsDirName.Format("Loads_total"); // from all RUNs ("Loads%d_total", SCEN)-old
	::CreateDirectory(LoadsDirName, sec);
	::SetCurrentDirectory(LoadsDirName);
	S.Format("Created Scen %d TOTAL Loads FOLDER >>>>> %s .....\n", SCEN, LoadsDirName);
	logout << S;//std::cout << S;// << std::endl;

	::CreateDirectory("6-col", sec);// LoadsDirname \ 6-col
	::CreateDirectory("3-col", sec);// LoadsDirname \ 3-col

	CArray<C3Point> Loads;// X - A, Y - Resid, Z - Reion 
	int lmax = names.GetSize();

	double x, y, z;
	CArray<double> X;
	CArray<double> Y;
	int datacount;
	CString loadfiles;
	CString loadINFO;
	long totfilesize = 0; // total output size
	char name1[1024];// 6-col load name
	char name2[1024];// 3-col load name
	
	for (int i = 0; i < lmax; i++) { // look for load-files - for i-load
		S = names[i];//"load_%d.txt", i
		ndata = sizes[i];
		Loads.RemoveAll(); 
		X.RemoveAll(); 
		Y.RemoveAll();

		for (int n = 0; n < ndata; n++) {
			Loads.Add(C3Point(0, 0, 0)); // init loads
			X.Add(0);
			Y.Add(0);
		}

		loadINFO = "";
		loadfiles = "";// list of source loads
		BOOL done = FALSE;
		double SumAtom=0, SumResid=0, SumReion=0; // total PD (to be x Scell) 
		double MaxAtom =0, MaxResid=0, MaxReion=0, Total=0, TotMax=0; // max PD

		found = 0;
		for (int k = 0; k < 3; k++) { // look for load_i in all runs, fill arrays
			strcpy(name, "..\\" + run[k] + S);
			fin = fopen(name, "r");
			if (fin == NULL) continue;

			loadfiles += name;
			loadfiles += "  "; // add to source list
			found++;
			if (found == 1) {
				for (int l = 0; l < headline; l++) {
					line = fgets(buf, 1024, fin); // read info-lines
					//if (done) continue; // dont copy to output
					if (l < 3) {
						loadINFO += line;
						continue;
					}
					if (CString(line).Find("rigin") >= 0 || CString(line).Find("Ort") >= 0) {
						CString(line).TrimLeft();
						loadINFO += CString(line);
						//done = TRUE; // copied to output
					}
				}// l = 13
			} // use 1st file for info

			datacount = 0;
			int n = 0;
			while (!feof(fin)) {
			//for (int n = 0; n < ndata; n++) { //while (!feof(fin))
				fgets(buf, 1024, fin);
				int res = sscanf(buf, "%le %le %le", &x, &y, &z);
				//line = fgets(buf, 1024, fin);
				//int result = fscanf(fin, "%d  %le  %le  %le  %le", &n, &pow, &PDmax, &stepX, &stepY);
				//int res = fscanf(fin, "%le  %le  %le", &x, &y, &z);
				if (res != 3) continue;
				if (n >= ndata) break;
				X[n] = x; 
				Y[n] = y;
				if (z < 1.e-9) z = 0;

				switch (k) {
				case 0: Loads[n].X = z; // k = 0
					SumAtom += z;
					MaxAtom = Max(MaxAtom, z);
					break;
				case 1: Loads[n].Y = z;
					SumResid += z;
					MaxResid = Max(MaxResid, z);
					break;
				case 2: Loads[n].Z = z;
					SumReion += z;
					MaxReion = Max(MaxReion, z);
					break;
				default: break;
				} // switch k
				Total = Loads[n].X + Loads[n].Y + Loads[n].Z; // sum PD in a cell
				TotMax = Max(TotMax, Total); // max PD in a cell
				datacount++;
				n++;
				
			}// eof
			fclose(fin); // file for current load

			::DeleteFile(name); // REMOVE SOURCE LOAD FILES//////////////////////////////

		} // k load-files collected
		SumAtom = SumAtom  * Scells[i];
		SumResid = SumResid  * Scells[i];
		SumReion = SumReion  * Scells[i];

		// make merged load-file
		S = "6-col\\" + names[i]; // .Format("load_%d.txt", iload);
		strcpy(name1, S);
		S = "3-col\\" + names[i]; // .Format("load_%d.txt", iload);
		strcpy(name2, S);
		fout1 = fopen(name1, "w"); // in Loads_Total folder!
		fout2 = fopen(name2, "w"); // in Loads_Total folder!
		
		fprintf(fout1, "---- Mixed Load for SCEN %d ----\n", SCEN);
		//fprintf(fout1, "source files: %s \n", loadfiles);
		//fprintf(fout1, "ndata = %d  (last = %d)\n", ndata, datacount-1);
		
		fprintf(fout2, "---- Total Load for SCEN %d ----\n", SCEN);
		//fprintf(fout2, "source files: %s \n", loadfiles);
		//fprintf(fout2, "ndata = %d  (last = %d)\n", ndata, datacount - 1);
		
		S = "SCEN Options: "; //
		if (OptTraceAtoms) S += " + ATOMS";
		if (!OptNeutrStop) S += " + Residual ions (Neg/Pos)";
		if (!OptReionStop) S += " + Reions (Pos)";
		S += "\n";
		
		fprintf(fout1, S); 
		fprintf(fout1, "%s ", loadINFO);
		fprintf(fout1, "Sum Power [W]: Atoms = %le  Resid = %le  Reions = %le \n", SumAtom, SumResid, SumReion);
		fprintf(fout1, " Max PD [W/m2]: Atoms = %le  Resid = %le  Reions = %le \n", MaxAtom, MaxResid, MaxReion);
		
		fprintf(fout2, S);
		fprintf(fout2, "%s ", loadINFO);
		fprintf(fout2, "Sum Power [W]: Atoms + Resid + Reions = %le \n", (SumAtom + SumResid + SumReion));
		fprintf(fout2, " Max PD [W/m2]:  %le \n", TotMax);

		fprintf(fout1, "\n \t\t POWER DEPOSITION on the Plate [W/m2] \n");
		fprintf(fout1, "  X,m \t  Y,m \t      DI  \t  Resid  \t Rei     \t Total \n"); // 6 columns

		fprintf(fout2, "\n \t\t POWER DEPOSITION on the Plate [W/m2] \n");
		fprintf(fout2, "  X,m \t  Y,m \t\t Total \n");// 3 columns

		for (int n = 0; n < ndata; n++) {
			//plate = GetPlateByNumber(Num[i]);
			//Scomm = plate->Comment;
			//fprintf(fout, "%3d  %0.3le  %0.3le  %0.3le    %0.3le    %0.3le  %0.3le  %0.3le \t%s\n",
				//Num[i], Pow_A[i], Pow_Resid[i], Pow_Reion[i], SumPow[i],Max_A[i], Max_Resid[i], Max_Reion[i], Scomm);

			fprintf(fout1, " %-8.5f %-8.5f %-12.3le %-12.3le %-12.3le %-12.3le \n",
					X[n], Y[n], Loads[n].X, Loads[n].Y, Loads[n].Z, (Loads[n].X + Loads[n].Y + Loads[n].Z));
			fprintf(fout2, " %-8.5f %-8.5f %-12.3le \n",
				X[n], Y[n], (Loads[n].X + Loads[n].Y + Loads[n].Z));
		}
		fclose(fout1); fclose(fout2);

		// define output file size
		FILE *pFile = fopen(name1, "rb");
		long nFileLen = 0;
		if (pFile)
		{
			fseek(pFile, 0, SEEK_END);
			nFileLen = ftell(pFile);
			fclose(pFile);
		}
		totfilesize += nFileLen;

		pFile = fopen(name2, "rb");
		nFileLen = 0;
		if (pFile)
		{
			fseek(pFile, 0, SEEK_END);
			nFileLen = ftell(pFile);
			fclose(pFile);
		}
		totfilesize += nFileLen;

	} // all loads
	
	S.Format("-------- %d loads merged\n", loads);
	logout << S;//std::cout << loads << " <<<<< loads merged\n";

	::SetCurrentDirectory(ScenDir); // return from LoadDir
	S.Format("_____BACK to SCEN folder____ %s\n", ScenDir);
	logout << S;//std::cout << "BACK to SCEN folder !!!" << ScenDir << "\n";

	//::DeleteFile(run[0]);
	//RecursiveDelete(run[0]);//RemoveDirectory(path);
	
	RemoveDirectory(run[0]);// Loads%_Run1
	S.Format("  >>>> REMOVE FOLDER %s\n", run[0]);	
	logout << S; //std::cout << S;
		
	//RecursiveDelete(run[1]);//
	RemoveDirectory(run[1]);// Loads%_Run1
	S.Format("  >>>> REMOVE FOLDER %s\n", run[1]);	
	logout << S;//std::cout << S;
	
	//RecursiveDelete(run[2]);//
	RemoveDirectory(run[2]);// Loads%_Run1
	S.Format("  >>>> REMOVE FOLDER %s\n", run[2]);	
	logout << S;//std::cout << S;
	
	//::MoveFile("loads.txt", LoadsDirName + "\\loads-info.txt");
	//std::cout << "MOVE loads-info FILE to " << ScenDir << std::endl;

	long info_kB = 5; // "loads-info" file
	S.Format("\n >>>>>>> SCEN %d ALL-RUNS RESULTS {6+3} take ~ %d kB <<<<<<<<<< \n\n", 
		SCEN, totfilesize / 1024 + info_kB);
	logout << S;//std::cout << S;
}

void CBTRDoc:: AddAllScenLoads()//called by CompleteScen
// add power from runs 1,2,3 to each plate
{
	logout << "\n ------ Adding Loads from RUNs (Normalized) ----------\n";
	char buf[1024];
	::GetCurrentDirectory(1024, buf); // SCEN folder
	CString ScenDir = CString(buf);

	CString Srun[3]; // subfolders in SCEN folder
	Srun[0].Format("Loads%d_run1\\", SCEN); // SCEN1_RUN1... SCEN1_RUN2... 
	Srun[1].Format("Loads%d_run2\\", SCEN);
	Srun[2].Format("Loads%d_run3\\", SCEN);//"Loads%d_run3\\"
//	FILE * fout = fopen("loads.txt", "w"); // loads statistic

	SetNullLoads();// init all maps (incl NOMAP) with default steps
	
	double xmax, ymax, stepx, stepy;
	double TotSum; // Load Sum from all Runs
	CString S, Sname, Sfull;
	char name[1024];
	CPlate * plate;
	int res; // result of reading load array
	int tot = 0; // plates found
	/*	
	Sfull = Srun[0] + Sname;// run 1
	Sfull = Srun[1] + Sname;// run 2
	Sfull = Srun[2] + Sname;// run 3*/
		
	CLoad * Load1;// = new CLoad(xmax, ymax, stepx, stepy);
	CLoad * Load2;// = new CLoad(xmax, ymax, stepx, stepy);
	CLoad * Load3;// = new CLoad(xmax, ymax, stepx, stepy);
	
/////// add ALL RUNs loads (normalized already in SaveRUN loads)
	POSITION pos = PlatesList.GetHeadPosition();
	while (pos != NULL) {
		plate = PlatesList.GetNext(pos);
		plate->Touched = FALSE;	// plate->Loaded = TRUE 
		Sname.Format("load_%d.txt", plate->Number);
		xmax = plate->Load->Xmax;
		ymax = plate->Load->Ymax;
		stepx = plate->Load->StepX;
		stepy = plate->Load->StepY;
	
		Sfull = Srun[0] + Sname;// run 1
		strcpy(name, Sfull);
		Load1 = new CLoad(xmax, ymax, stepx, stepy);
		res = ReadLoadArray(name, Load1, FALSE); // not free
		if (res > 0) Load1->SetSumMax();
				
		Sfull = Srun[1] + Sname; // run 2
		strcpy(name, Sfull);
		Load2 = new CLoad(xmax, ymax, stepx, stepy);
		res = ReadLoadArray(name, Load2, FALSE); // not free
		if (res > 0) Load2->SetSumMax();
				
		Sfull = Srun[2] + Sname;
		strcpy(name, Sfull);
		Load3 = new CLoad(xmax, ymax, stepx, stepy);
		res = ReadLoadArray(name,Load3, FALSE); // not free
		if (res > 0) Load3->SetSumMax();
		
		TotSum = Load1->Sum + Load2->Sum + Load3->Sum;

		if (TotSum > 1.e-3) {
			tot++;
			plate->Touched = TRUE;// checked in SaveLoad() !!!
			plate->AddLoads(Load1, Load2, Load3);// + SetSumMax called
			plate->AtomPower = Load1->Sum;
			plate->PosPower = Load2->Sum + Load3->Sum;
			plate->NegPower = 0;
		}
	}

	delete Load1; delete Load2; delete Load3; 

	S.Format("SCEN %d LOADS (%d) SUCCESSFULLY ADDED!\n", SCEN, tot);
	logout << S;
	::SetCurrentDirectory(ScenDir); // return from LoadDir
	S.Format("_____BACK to SCEN folder____ %s\n", ScenDir);
	logout << S;
}

int CBTRDoc:: ReadLoads3_AtPath(CString path) //return NONZERO loads count
{
	int count = 0;// count loads
	CString info; // 13 lines in load file 
	CString Sname;
	FILE * fin;
	int infolines = 13;
	char buf[1024];
	double x, y, load;
	double stepx, stepy, xmax, ymax;// = Load->StepX;
	int nx, ny;
	CLoad * Load;
	CPlate * plate;
	char name[1024];
	int res;
	CString S;
	BeginWaitCursor(); //OnShow();

	POSITION pos = PlatesList.GetHeadPosition();
	while (pos != NULL) {
		plate = PlatesList.GetNext(pos);
		plate->Touched = FALSE;	// plate->Loaded = TRUE 
		Sname.Format(path + "load_%d.txt", plate->Number);
		strcpy(name, Sname);
		
		if ((fin = fopen(name, "r")) == NULL) {
			Sname.Format(path + "load%d.txt", plate->Number);
			strcpy(name, Sname);
			if ((fin = fopen(name, "r")) == NULL)	continue;// not exists - next
		}
		
		//else {
			info = "";
			for (int l = 0; l < infolines; l ++) {
				fgets(buf, 1024, fin);
				info += CString(buf);
			}
			fclose(fin);
			
			ReadLoadInfo(info, nx, ny, stepx, stepy);// fetch plate params from text
			xmax = stepx * nx;
			ymax = stepy * ny;
			Load = new CLoad(xmax, ymax, stepx, stepy);
			//Load->Comment = info;
			res = ReadLoadArray(name, Load, FALSE); // open file and read 3-col data
			if (res > 0) Load->SetSumMax();
			else {
				S.Format("%s - LoadArray not read \n", name);
				logout << S;
			}
					
			if (Load->Sum > 1.e-3) {
				plate->Touched = TRUE;
				plate->Load = Load;
				plate->Loaded = TRUE;
				SetLoadArray(plate, TRUE);// Add the plate to Selected Plates List
				count++;
			}
			else {
				S.Format("Surf %d  - ZERO power read! \n", plate->Number);
				logout << S;
				S.Format("\tNx = %d, Ny = %d, stepX = %5f, stepY = %5f, Xmax = %5f, Ymax = %5f\n",
					Load->Nx, Load->Ny, Load->StepX, Load->StepY, Load->Xmax, Load->Ymax);
				logout << S;
			}
		
		/*	if (TotSum > 1.e-3) {
			plate->Touched = TRUE;// checked in SaveLoad() !!!
			plate->AddLoads(Load1, Load2, Load3);// + SetSumMax called
			plate->AtomPower = Load1->Sum;
			plate->PosPower = Load2->Sum + Load3->Sum;
			plate->NegPower = 0;
		}*/
		//} // file exists
	} // pos

	EndWaitCursor();
	return count;
}

void CBTRDoc:: ReadScenLoads3col() // read SCEN results from Load_total (3-col)
{
	// Loads reading from SCEN folder (CurrentDir)
	//CString LoadsDirName = ConfigFileName + "\\" + stot + "\\" + "3-col" ;
	CString stot = "Loads_total";
	CString path = stot + "\\" + "3-col" + "\\";  
	logout << path << "\n";

	SetLoadArray(NULL, FALSE);//clear
	
	int count = ReadLoads3_AtPath(path);// count non-zero loads
		
	CString S;
	S.Format(" %d NON_ZERO loads found \n %d total Loads listed\n", count, LoadSelected);
	logout << S;
}

void CBTRDoc::CompleteRun() // calculate, save loads, re-init arrays
{
	//CalculateAllLoads(); // only touched plates are calculated
	CString sn, name, S;
	char buf[1024];
	::GetCurrentDirectory(1024, buf); // SCEN folder
	CString OldDir = CString(buf);
	
	CTime t0 = CTime::GetCurrentTime();
	S.Format("%d:%d:%d   Current SCEN folder is %s \n", t0.GetHour(), t0.GetMinute(), t0.GetSecond(), OldDir);
	logout << S; //std::cout << S;
		
	sn.Format("%d_run%d_", SCEN, RUN);
	name = "SCEN" + sn + "loads.txt";
			
	SaveRUNLoads(); //create RUN-folder in current SCEN-folder and Save RUN net Loads in it
	// the loads are normalized after runs 2,3 

	::SetCurrentDirectory(OldDir); // back to SCEN
	S.Format("--- back to %s \n", OldDir);
	logout << S; //std::cout << S;

	SaveRUNSummary(name);// + write Pn, Pinj, Psolid for this Run
	// write RUN summary to current SCEN folder (Surf   Total, W  Max, W/m2  GridSteps,m)

	CTime t1 = CTime::GetCurrentTime();
	CTimeSpan dt = t1 - t0; //GetElapse(t0, t1);
	DataSaveTime += dt;

	ResetLogFile();
			
}

void CBTRDoc::CorrectRunPower()//not called - included to SaveRUNloads
//to fit balance with RUN 1 - before merging RUN-Loads 
{
	double Coeff, power;
	double Coeff2 = 1;
	if (AtomPower2 > 1e-3) Coeff2 = AtomPower1 / AtomPower2;
	double Coeff3 = 1;
	if (AtomPower3 > 1e-3) Coeff3 = AtomPower1 / AtomPower3;
}

void CBTRDoc::CompleteScen() // Scen consists of 1-3 runs 
//merge runs SUM-loads
//add all run loads with corrected coeff
//write 3/6col loads
//write Reports
//back to home dir, resume config
{
	STOP = TRUE; // stop threads, STOP >> TRUE
	int n; // count files found
	CString S, SUMname, pref;
	CStringArray SRCnames;// source files with prefix 
	CTime t0 = CTime::GetCurrentTime();
	
	// MERGE LOADS SUMMARY (LoadList)
	pref.Format("SCEN%d", SCEN); // SCEN1_RUN1... SCEN1_RUN2... 
	n = GetPrefFiles(pref, SRCnames);// find files in current SCEN name	//n = names.GetSize();
	S = "";
	for (int i = 0; i < n; i++) S += "  " + SRCnames[i] + "\n";
	logout << S; //std::cout << S;// << std::endl;//////////
	
	SUMname.Format("SCEN%d_SUMloads", SCEN);
	CollectRUNSummary(SUMname, SRCnames); // merge load list SUMMARY results + delete RUN summary files

	S.Format(">>>> MERGE Scen %d Results SUMMARY: >>>> %s <<<< \n", SCEN, SUMname);
	logout << S; //std::cout << S;// << std::endl;
	 
	// Add Loads from All RUNs - with normalization of run2, run3
	AddAllScenLoads();
	
	// MERGE each SURF load  folders: Loads1_run1, Loads1_run2 \ load_7.txt
	CollectRUNLoads(); // READ load for each surf -> WRITE FOLDER with merged loads(3/6)
	
	// SHOW TRACKED INFO - SINGLE //
	WriteReport("TXT");
	WriteReport("CSV");

	CTime t1 = CTime::GetCurrentTime();
	CTimeSpan dt = t1 - t0; //GetElapse(t0, t1);
	DataSaveTime += dt;

	// RETURN BACK HOME  
	::SetCurrentDirectory(CurrentDirName);// go up("..\\"); - after completeing scen
	if (SCEN >= MAXSCEN) return; // exception - last run!
	
	logout << "\n GO back HOME ->> " << CurrentDirName << "\n";
	ResumeData(); // back to initial config
}

void CBTRDoc::InitScenOpt(int & optA, int & optRes, int & optRei) // 0 - stop, 1 - trace
{	// set new options if found (other than for basic config)
	// dont change basic scen opts!!!
	bool found = 0; // found opts
	CString S;
	int maxdata = ScenData[SCEN].GetSize();
	
	for (int idata = 1; idata < maxdata; idata++) {
		S.Format(ScenData[SCEN][idata]);
		if (S.Find("OptTraceAtoms") >= 0) {// -> change defaults
			found = TRUE; 
			optA = 0;  
			OptTraceAtoms = FALSE; //stop
			int pos = -1;
			if ((pos = S.Find("#")) > 0) S.Truncate(pos);
			//if (S.Find("ON") > 0 || S.Find("TRUE") > 0 || S.Find("1") > 0) {
			if (S.Find("1") > 0) {
				optA = 1; 
				OptTraceAtoms = TRUE;// trace
			}
		} // atoms

		if (S.Find("OptNeutrStop") >= 0) {// -> change defaults
			found = TRUE;
			optRes = 0; 
			OptNeutrStop = TRUE; //stop
			int pos = -1;
			if ((pos = S.Find("#")) > 0) S.Truncate(pos);
			//if (S.Find("OFF") > 0 || S.Find("FALSE") > 0 || S.Find("0") > 0) {
			if (S.Find("0") > 0) {
				optRes = 1;
				OptNeutrStop = FALSE; // trace
			}//+
		} // resid
		if (S.Find("OptReionStop") >= 0) {// -> change defaults
			found = TRUE;
			optRei = 0; 
			OptReionStop = TRUE; //stop
			int pos = -1;
			if ((pos = S.Find("#")) > 0) S.Truncate(pos);
			//if (S.Find("OFF") > 0  || S.Find("FALSE") > 0 || S.Find("0") > 0)
			if (S.Find("0") > 0) 
				optRei = 1; //OptReionStop = FALSE; //+
			if (!PressureLoaded || !FieldLoaded) optRei = 0;  // OptReionStop = TRUE;
			if (optRei == 1) OptReionStop = FALSE; // trace
		} // reion
	} // for

	if (found) {
		S.Format(" SCEN %d OPTIONS: \n\t ATOMS = %d\n\t Resid ions = %d\n\t Reions = %d\n", 
						SCEN, optA, optRes, optRei);
		logout << S; //std::cout << S;// std::endl;
	}
}

CString CBTRDoc::GetScenName(int scen)
{
	if (scen > MAXSCEN) return "UNDEF";
	int maxdata = ScenData[scen].GetSize();
	int pos;
	CString S, SCEN_NAME;
	SCEN_NAME.Format("SCEN_%03d", scen); // default name
	
	if (maxdata > 1) { // find scen name
		S = ScenData[scen][1];
		if (S.Find("NAME", 0) >= 0) {
			pos = S.Find("=");
			SCEN_NAME = S.Mid(pos + 1);
			SCEN_NAME.Trim();
		}
		else SCEN_NAME.Format("SCEN_%03d", scen); // default name
	}
	return SCEN_NAME;
}
							
void CBTRDoc::CreateScenFolder() // create folder, copy input files, write scen options - set it as current dir!!!
{
	LPSECURITY_ATTRIBUTES  sec = NULL;//SEC_ATTRS;
	::SetCurrentDirectory(CurrentDirName);
	CTime t = CTime::GetCurrentTime();
	
	FILE * fout;
	CString name;
	CString S;
	int pos;
	int maxdata = ScenData[SCEN].GetSize();
	CString ScenDirName = GetScenName(SCEN);// SCEN_NAME; 	//set  ScenDirName
	// Create SCEN Folder
	::CreateDirectory(ScenDirName, sec);
	::SetCurrentDirectory(ScenDirName); //dont change CurrentDirName!

	S.Format("****** %02d:%02d:%02d ******** \n", t.GetHour(), t.GetMinute(), t.GetSecond());
	logout << S; //std::cout << S; 
	S.Format("_______Create Folder _______ %s\n", ScenDirName);
	logout << S; //std::cout << "- create folder ..... " << ScenDirName << "\n";

	char OpenDirName[1024];
	CString DirName;
	::GetCurrentDirectory(1024, OpenDirName); // check
	S.Format("---- Current Folder is %s\n", OpenDirName);
	logout << S;//std::cout << "---- Current Folder is " << OpenDirName << "\n";///////////
	
	// Save Config File --------------------------------
	FormDataText(TRUE);// include internal names of parameters
	name.Format("Scen%d_Config.txt", SCEN);
	fout = fopen(name, "w");
	fprintf(fout, m_Text);
	fclose(fout);

	// Write Scen param ----------------
	
	name.Format("Scen%d_Param.txt", SCEN);
	fout = fopen(name, "w");
	//sn.Format("%d", SCEN);
	S.Format(" Basic settings : " + ScenData[0][1]); // SCEN0 [1] -> config file 
	fprintf(fout, S);
	S.Format(" Scenario PARAM: " + ScenData[SCEN][0]);
	fprintf(fout, S);
	for (int idata = 1; idata < maxdata; idata++) {
		S.Format(ScenData[SCEN][idata]);
		fprintf(fout, S);
	}
	fclose(fout);
	S.Format(">>> write Scen config/params to %s\n", ScenDirName);
	logout << S;//std::cout << ">>> write Scen config & params to " << ScenDirName << "\n";
	//::SetCurrentDirectory(CurrentDirName);// go up("..\\"); - after completeing scen!!!
}

void CBTRDoc::InitScen() // set data + opt for current SCEN
{
	int maxdata = ScenData[SCEN].GetSize();
	CString S;
	S.Format("  Basic settings : " + ScenData[0][1]); // SCEN0 [1] -> config file 
	logout << S;//
	S.Format("  Current Scenario PARAM: " + ScenData[SCEN][0]);
	logout << S;// "  Set parameters for  " << S;// std::endl;
	
	int data = 0; // found in data list
	for (int idata = 1; idata < maxdata; idata++) {
		S.Format(ScenData[SCEN][idata]);
		data += SetData(S, 0);// update data
		logout << S;// std::endl;
	}
	logout << data << " Data lines + " << maxdata-data << " Opt lines)\n";

	SetBeam(); // MAMuG Channels/Beamlets, Aimings/Positions
	// update BML Attr arrays for all(3) SCEN Runs
	SetPolarBeamletCommon();
	SetPolarBeamletResidual();
	SetPolarBeamletReion();

	SetSINGAPfromMAMUG();
	
	CreateScenFolder();
}

void CBTRDoc::InitRun(int run)// set options which are used in TraceALL!! (MULTI)
// only one Trace Option must be set per each RUN (Atoms or Resid or REion)
{
	StartTime = CTime::GetCurrentTime();// RUN start
	CString S = "    RUN trace opt : ";
	
	switch (run) { // 0,1,2
	case 0: // RUN 1 - only atoms traced and deposed + count AtomPower1
		OptTraceAtoms = TRUE;// trace atoms
		OptNeutrStop = TRUE; // stop residuals
		OptReionStop = TRUE; // stop reions
		
		OptAtomPower = TRUE; // depose atoms
		OptNegIonPower = FALSE; // depose negions
		OptPosIonPower = FALSE; // depose posions
		
		PolarNumber = PolarNumberAtom;
		AzimNumber = AzimNumberAtom;
		
		S += "Atoms with/without ionization (for gas+plasma On/Off)";
		break;
	case 1: //RUN 2 - trace Residuls, depose Ions, + count AtomPower2 
		OptTraceAtoms = FALSE;// stop
		OptNeutrStop = FALSE; // trace residuals
		OptReionStop = TRUE; // stop reions

		OptAtomPower = FALSE; // depose atoms - OFF, (count AtomPower)
		OptNegIonPower = TRUE; // depose negions
		OptPosIonPower = TRUE; // depose posions

		PolarNumber = PolarNumberResid;
		AzimNumber = AzimNumberResid;
		
		S += "Residual ions after Neutraliser(+/-)";
		break;
	case 2: //RUN 3 trace Reions, depose Ions, + count AtomPower3
		OptTraceAtoms = FALSE;// stop 
		OptNeutrStop = TRUE; // stop residuals
		OptReionStop = FALSE;// trace reions

		OptAtomPower = FALSE; // depose atoms - OFF
		OptNegIonPower = FALSE; // depose negions
		OptPosIonPower = TRUE; // depose posions

		PolarNumber = PolarNumberReion;
		AzimNumber = AzimNumberReion;
		
		S += " Reionised particles (for gas + Mag Field preset)";
		break;

	default: //= case DI
		OptTraceAtoms = TRUE;
		OptNeutrStop = TRUE;
		OptReionStop = TRUE;
		
		OptAtomPower = TRUE; // depose atoms
		OptNegIonPower = TRUE; // depose negions
		OptPosIonPower = TRUE; // depose posions

		PolarNumber = PolarNumberAtom;
		AzimNumber = AzimNumberAtom;

		break;
	}
	S += "\n";
	logout << S; //std::cout << S << "\n";
}

void CBTRDoc::WriteScenTracks()
{
	CString S;
	int i;
	logout << "----- TRACKED INFO (for debug)--------\n"; 
	for (i = 1; i <= MAXSCEN; i++) {
		// Area Exit excluded from Solid loads!!!
		double Pn = PowNeutral[i];// run 1
		double Pinj = PowInjected[i]; // run 1
		double Psum = PowSolid[i].X + PowSolid[i].Y + PowSolid[i].Z;
			
		S.Format("SCEN %d\n Neutralized %g[W]\n Injected %g[W]\n Total Deposited %g[W]: \n", 
							i, Pn,  Pinj,  Psum);// - Pinj);// Area Exit excluded
		logout << S;
		S.Format("\t Atoms %g[W]\n\t Residuals %g[W]\n\t Reions %g[W]\n", 
						PowSolid[i].X, PowSolid[i].Y, PowSolid[i].Z);
		logout << S;
	} // i
	
	ofstream csvf; // EXCEL format
	csvf.open("_ALL_SCEN_LOADS.CSV");
	ofstream txtf;
	txtf.open("_ALL_SCEN_LOADS.TXT");
	logout << "\nWriting ALL_SCEN_LOADS.csv at HOME\n";

	//logout << "\n Surf";
	csvf << " Surf"; 
	txtf << "Surf";
	CString ScenName, SurfName;
	for (i = 1; i <= MAXSCEN; i++)  {
		ScenName = GetScenName(i);
		logout << "  " << ScenName;
		csvf << " , " << ScenName;//csvf << " ; " << ScenName;
		txtf << " \t " << ScenName;
	}
	logout << "\t Comment .......... \n"; 
	csvf << " , Comment \n";//csvf << " ; Comment \n";
	txtf << " \t Comment \n";

//fprintf(fout, "%d   %-4.3f   %-4.3f   %-4.3f     %-4.3f    %-10.3le  %-10.3le  %-10.3le   %-10.3le  %-10.3le  %d\n",
//  i,  Path[i].X, Path[i].Y, Path[i].Z, psi[i], dens[i], sigma[i], sumthick[i], decay[i], rate[i], locstate);

	for (int n = 1; n <= PlateCounter; n++) {
		//logout << "  " << n;
		S.Format("%5d", n);
		csvf << " " << n;
		txtf << S; //" " << n;
		SurfName = GetPlateByNumber(n)->Comment;
		for (i = 1; i <= MAXSCEN; i++) {
			S.Format("%-10.2le", ScenLoadSummary[i][n]);
			//logout << "\t" << ScenLoadSummary[i][n];// 1st index - scen
			//csvf << " ; " << ScenLoadSummary[i][n];// 1st index - scen
			csvf << " , " << ScenLoadSummary[i][n];// 1st index - scen
			txtf << "\t" << S; //ScenLoadSummary[i][n];// 1st index - scen
		}
		//logout << "\t" << SurfName << "\n";
		csvf << " , " << SurfName << "\n";//csvf << " ; " << SurfName << "\n";
		txtf << "\t" << SurfName << "\n";
	}
	csvf.close();
	txtf.close();
}

void CBTRDoc:: WriteReport(CString ext)
{
	//logout << "----- TRACKED PARAMETERS -------\n"; 
	CStringArray Keys;
	CString S;
	ofstream f;
	
	CString sep;// separator
	S = ext.MakeUpper();
	if (S.Find("CSV", 0) > -1) sep = ",";// ";";
	else if (S.Find("TXT", 0) > -1) sep = "\t";
	
	if (MAXSCEN == 1) S = "_00."; //SINGLE
	else S.Format("_SCEN%2d.", SCEN);
	CString name = "REPORT" + S + ext;

	f.open(name);
	f << "<<<<< Components REPORT >>>>>>\n";
	logout << "\nWriting REPORT " << ext << " at HOME\n";

	double Sum = -1, MaxPD = -1;
	int n;
	double Pentry = -1, Pexit = -1;
	double Pn = GetNeutrPower();//  AtomPower1
	double Presid;
	if (NeutrPart > 1.e-3) Presid = Pn * (1.- NeutrPart) / NeutrPart;
	else Presid  = 1; // Pn = 0
	double Preion = Pn * ReionPercent * 0.01;
	double Pinj = GetInjectedPowerW(); //Plasma Emitter AtomPower
	double SolidPower = GetTotSolidPower();// Atom + Ion Power at solids excl Area Exit	
	CString sf = "%-30s"; // comment field format
	//CString vf = "\t%-12.3le\n";// txt
	CString vf = sep + " %-12.3le \n"; // data field format
	CString sdf = "%-25s (%d)";// comment field with surf count (n)
	CString sff = "%-22s (%4.3f)";// comment field with fraction

	S.Format(sff, "Neutralized , W", NeutrPart);  f << S;
	S.Format(vf, Pn); f << S;

	S.Format(sff, "Resid Ions, W", (1.- NeutrPart)); f << S;
	S.Format(vf, Presid); f << S;

	S.Format(sff, "Reionized, W", ReionPercent * 0.01); f << S;
	S.Format(vf, Preion); f << S;

	S.Format(sf, "Injected, W"); f << S;
	if (Pinj < 1e-6) f << sep + "  NAN\n";
	else { S.Format(vf, Pinj); f << S; }

	S.Format(sf, "Solid Surf, W"); f << S;
	S.Format(vf, SolidPower); f << S;

	S.Format(sf, "Injected + Solid, W"); f << S;
	S.Format(vf, (Pinj + SolidPower)); f << S;
	
	f << "\n__CUT-OFF_LIMITS__\n"; // before Neutr
	Keys.RemoveAll(); Keys.Add("CUT"); 
	Sum = -1, MaxPD = -1;
	n = GetCompSolid(Keys, Sum, MaxPD);
	S.Format(sdf, "Total Power, W", n); f << S;
	S.Format(vf, Sum);  f << S;

	f << "\n___NEUTRALISER____\n";	
	Keys.RemoveAll(); Keys.Add("NEUTR"); //Keys.Add("WALL");
	Sum = -1; MaxPD = -1;	
	n = GetCompSolid(Keys, Sum, MaxPD);
	S.Format(sdf, "Total Power, W", n); f << S;
	S.Format(vf, Sum);  f << S;
	
	Keys.RemoveAll(); Keys.Add("NEUTR"); Keys.Add("WALL");
	Sum = -1; MaxPD = -1;	
	n = GetCompSolid(Keys, Sum, MaxPD);
	S.Format(sdf,"Walls Power, W", n); f << S;
	S.Format(vf, Sum);  f << S;
	S.Format(sf, "Walls MaxPD, W/m2");  f << S;
	S.Format(vf, MaxPD); f << S;
	
	Keys.RemoveAll(); Keys.Add("NEUTR"); Keys.Add("LEAD");
	Sum = -1; MaxPD = -1;	
	n = GetCompSolid(Keys, Sum, MaxPD);
	S.Format(sdf, "Faces Power, W", n); f << S;
	S.Format(vf, Sum);  f << S;
	S.Format(sf, "Faces MaxPD, W/m2");  f << S;
	S.Format(vf, MaxPD); f << S;

	Keys.RemoveAll(); Keys.Add("NEUTR");
	Pentry = -1; Pexit = -1;
	GetCompTransparent(Keys, Pentry, Pexit);
	S.Format(sf,"  Start Power, W");  f << S;
	S.Format(vf, Pentry);  f << S;
	S.Format(sf,"  Exit Power, W");   f << S;
	S.Format(vf, Pexit);   f << S;
		
	f << "\n______RID_______\n"; 
	Keys.RemoveAll(); Keys.Add("RID"); 
	Sum = -1, MaxPD = -1;
	n = GetCompSolid(Keys, Sum, MaxPD);
	S.Format(sdf, "Total Power, W", n); f << S;
	S.Format(vf, Sum);  f << S;
	
	Keys.RemoveAll(); Keys.Add("RID"); Keys.Add("WALL");
	Sum = -1; MaxPD = -1;	
	n = GetCompSolid(Keys, Sum, MaxPD);
	S.Format(sdf,"Walls Power, W", n); f << S;
	S.Format(vf, Sum);  f << S;
	S.Format(sf, "Walls MaxPD, W/m2");  f << S;
	S.Format(vf, MaxPD); f << S;
	
	Keys.RemoveAll(); Keys.Add("RID"); Keys.Add("LEAD");
	Sum = -1; MaxPD = -1;	
	n = GetCompSolid(Keys, Sum, MaxPD);
	S.Format(sdf, "Faces Power, W", n); f << S;
	S.Format(vf, Sum);  f << S;
	S.Format(sf, "Faces MaxPD, W/m2");  f << S;
	S.Format(vf, MaxPD); f << S;

	Keys.RemoveAll(); Keys.Add("RID");
	Pentry = -1; Pexit = -1;
	GetCompTransparent(Keys, Pentry, Pexit);
	S.Format(sf,"  Exit Power, W");   f << S;
	S.Format(vf, Pexit);   f << S;

	f << "\n__CALORIMETER___\n"; 
	Keys.RemoveAll(); Keys.Add("CALORIM"); 
	Sum = -1, MaxPD = -1;
	n = GetCompSolid(Keys, Sum, MaxPD);
	S.Format(sdf, "Total Power, W", n); f << S;
	S.Format(vf, Sum);  f << S;
	S.Format(sf, "MaxPD, W/m2");  f << S;
	S.Format(vf, MaxPD); f << S;
		
	Keys.RemoveAll(); Keys.Add("CALORIM");
	Pentry = -1; Pexit = -1;
	GetCompTransparent(Keys, Pentry, Pexit);
	S.Format(sf,"  Exit Power, W");   f << S;
	S.Format(vf, Pexit);   f << S;

	f << "\n____SCRAPER_____\n"; 
	Keys.RemoveAll(); Keys.Add("SCRAP"); 
	Sum = -1, MaxPD = -1;
	n = GetCompSolid(Keys, Sum, MaxPD);
	S.Format(sdf, "Total Power, W", n); f << S;
	S.Format(vf, Sum);  f << S;
	S.Format(sf, "MaxPD, W/m2");  f << S;
	S.Format(vf, MaxPD); f << S;

	Keys.RemoveAll(); Keys.Add("SCRAP"); Keys.Add("WALL");
	Sum = -1; MaxPD = -1;
	n = GetCompSolid(Keys, Sum, MaxPD);
	S.Format(sdf, "Walls Power, W", n); f << S;
	S.Format(vf, Sum);  f << S;
	S.Format(sf, "Walls MaxPD, W/m2");  f << S;
	S.Format(vf, MaxPD); f << S;

	Keys.RemoveAll(); Keys.Add("SCRAP"); Keys.Add("WING");
	Sum = -1; MaxPD = -1;
	n = GetCompSolid(Keys, Sum, MaxPD);
	S.Format(sdf, "Faces Power, W", n); f << S;
	S.Format(vf, Sum);  f << S;
	S.Format(sf, "Faces MaxPD, W/m2");  f << S;
	S.Format(vf, MaxPD); f << S;

	/*
	f << "\n____SHUTTER_____\n"; 
	Keys.RemoveAll(); Keys.Add("SHUT"); 
	Sum = -1, MaxPD = -1;
	n = GetCompSolid(Keys, Sum, MaxPD);
	S.Format(sdf, "Total Power, W", n); f << S;
	S.Format(vf, Sum);  f << S;
	S.Format(sf, "MaxPD, W/m2");  f << S;
	S.Format(vf, MaxPD); f << S;

	f << "\n____ABS VALVE____\n"; 
	Keys.RemoveAll(); Keys.Add("ABSOL"); 
	Sum = -1, MaxPD = -1;
	n = GetCompSolid(Keys, Sum, MaxPD);
	S.Format(sdf, "Total Power, W", n); f << S;
	S.Format(vf, Sum);  f << S;
	S.Format(sf, "MaxPD, W/m2");  f << S;
	S.Format(vf, MaxPD); f << S;
	*/
	f << "\n_____D-LINER_____\n"; 
	Keys.RemoveAll(); Keys.Add("LINER"); 
	Sum = -1, MaxPD = -1;
	n = GetCompSolid(Keys, Sum, MaxPD);
	S.Format(sdf, "Total Power, W", n); f << S;
	S.Format(vf, Sum);  f << S;
	S.Format(sf, "MaxPD, W/m2");  f << S;
	S.Format(vf, MaxPD); f << S;

	f << "\n____BLANKET_____\n"; 
	Keys.RemoveAll(); Keys.Add("BLANK"); 
	Sum = -1, MaxPD = -1;
	n = GetCompSolid(Keys, Sum, MaxPD);
	S.Format(sdf, "Total Power, W", n); f << S;
	S.Format(vf, Sum);  f << S;
	S.Format(sf, "MaxPD, W/m2");  f << S;
	S.Format(vf, MaxPD); f << S;

	f << "\n____ALL DUCT >_____\n"; 
	Keys.RemoveAll(); Keys.Add("DUCT"); 
	Sum = -1, MaxPD = -1;
	n = GetCompSolid(Keys, Sum, MaxPD);
	S.Format(sdf, "Total Power, W", n); f << S;
	S.Format(vf, Sum);  f << S;
	S.Format(sf, "MaxPD, W/m2");  f << S;
	S.Format(vf, MaxPD); f << S;
	Pentry = -1; Pexit = -1;
	GetCompTransparent(Keys, Pentry, Pexit);
	S.Format(sf,"  Exit Power, W");   f << S;
	S.Format(vf, Pexit);  f << S;

	f << "\n__ADDIT_(solid)___\n"; 
	Keys.RemoveAll(); Keys.Add("ADD"); 
	Sum = -1, MaxPD = -1;
	n = GetCompSolid(Keys, Sum, MaxPD);
	S.Format(sdf, "Total Power, W", n); f << S;
	S.Format(vf, Sum);  f << S;
	S.Format(sf, "MaxPD, W/m2");  f << S;
	S.Format(vf, MaxPD); f << S;
	Pentry = -1; Pexit = -1;
	GetCompTransparent(Keys, Pentry, Pexit);
	S.Format(sf,"  Exit Power, W");   f << S;
	S.Format(vf, Pexit);  f << S;

	f << "\n___AREA_BOUNDS____\n"; 
	Keys.RemoveAll(); Keys.Add("AREA"); 
	Sum = -1, MaxPD = -1;
	n = GetCompSolid(Keys, Sum, MaxPD);
	S.Format(sdf, "Total Power, W", n); f << S;
	S.Format(vf, Sum);  f << S;
	Pentry = -1; Pexit = -1;
	GetCompTransparent(Keys, Pentry, Pexit);
	S.Format(sf,"  Exit Power, W");   f << S;
	S.Format(vf, Pexit);  f << S;
		
	f.close();
}

void CBTRDoc::RunScen(int iopt[3]) // - current scenario with default run-options 
								   // called by OnStartParallel
{
	CString S;
	//int opt[3] = { iopt[0], iopt[1], iopt[2] };// { ATOMS, Resid, Reions }
										  // now can be changed by each scenario!!
	logout << "\n GO back HOME ->> " << CurrentDirName << "\n";
	ResumeData(); // back to initial config

	STOP = FALSE;//started = TRUE
	int optA = iopt[0]; //default
	int optRes = iopt[1];//default
	int optRei = iopt[2];//default
	InitScenOpt(optA, optRes, optRei); // reset default options if spec in SCEN_PARAM 
	InitScen();// SetData, SetBeam.. for current SCEN, create SCEN Folder
	
	int scopt[3] = { optA, optRes, optRei };//final SCEN run opts (DEFAULT are not changed)
	//MAXRUN = runs[0] + runs[1] + runs[2];
	CString Date, Time;
	///////Set New StartTime!!!
	StartTime = CTime::GetCurrentTime();
	CTime tm = StartTime;
	Date.Format("%02d-%02d-%04d", tm.GetDay(), tm.GetMonth(), tm.GetYear());
	int success = 1;

	ArrSize = 0;
	RUN = 0;// count actual runs 1..3 
	
	// reset global counters before each SCEN - for MULTI mode
	// for SINGLE - before each BML
	AtomPower1 = 0;//  MULTI
	AtomPower2 = 0;//  MULTI
	AtomPower3 = 0;//  MULTI

	for (int irun = 0; irun < 3; irun++) { //0,1,2
		
		RUN++;	//::MessageBox(NULL, S, "BTR 5 RunScen", 0);//AfxMessageBox(S, MB_ICONEXCLAMATION | MB_OK);
		
		if (scopt[irun] == 0) //scopt[3] - final SCEN run opts = {optA, optRes, optRei} 
			continue; // this trace option - OFF
		else 
			InitRun(irun); // set single trace for this run: Atoms or Resid or Reions + Power deposited and saved! 
				
		Time.Format("%02d:%02d:%02d", tm.GetHour(), tm.GetMinute(), tm.GetSecond());
		S.Format("  --- Scen %d RUN %d ---- %s --- %s --- \n",  SCEN, RUN, Date, Time);
		//SetTitle(S);	
		logout << S;//std::cout << S;// << std::endl;//////////

		NofCalculated = 0; // beamlets done - set 0 
		OptParallel = TRUE;
		ClearAllPlates();	//OptCombiPlot = -1; // no load 
		SetPlasmaTarget(); //init plasma object, Geometry, Nray = -1, clear arrays
		//ShowStatus();//show active data on views

		success = Run_BTR_Fast();
		// calls ClearArrays, Starts&Deletes NEW Threads, InitTracers 

		CompleteRun(); // save loads list
		if (success == 0) break; // CompleteScen
	}
	InitScenOpt(optA, optRes, optRei); // GET SCEN options - needed for CompleteScen merging!
	
	//CorrectLoadsPower(); //to fit balance with RUN 1 - before merging RUN-Loads 
	//CorrectFallsPower(); //called for SINGLE after each BML before Loads Calculations
	
	CompleteScen(); //merge runs SUM-loads
					//add all run loads with corrected coeff
					//write 3/6col loads
					//write Reports
					//back to home dir, Resume Config after SCEN
	OnStop();//	STOP = TRUE;
	
	SCEN++;	//S.Format("  Scen %d set \n", SCEN); std::cout << S;

	if (SCEN > MAXSCEN) {
		tm = CTime::GetCurrentTime();
		StopTime = tm;
		//CString Date, Time;
		//Date.Format("%02d-%02d-%04d", tm.GetDay(), tm.GetMonth(), tm.GetYear());
		Time.Format("%02d:%02d:%02d", tm.GetHour(), tm.GetMinute(), tm.GetSecond());
		S.Format(" +++++ Date %s  Time %s +++++ \n", Date, Time);
		logout << "BTR" << BTRVersion << "--- ALL is DONE (last SCEN shown)----\n";
		logout << S << "\n";

		//----- TRACKED INFO -------- 
		WriteScenTracks();// Pinj, Psolid, ALL_SCEN_LOADS
		
		CTime t0 = StartTime0; // from global start!!!
		CTime t1 = StopTime;
		CTimeSpan dt = t1 - t0;
		Time.Format("%02d:%02d:%02d", dt.GetHours(), dt.GetMinutes(), dt.GetSeconds());
		S.Format("\n+++ ALL SCENs Running Time %s \n", Time);
		logout << S;
		
		CTimeSpan dst = DataSaveTime;
		Time.Format("%02d:%02d:%02d", dst.GetHours(), dst.GetMinutes(), dst.GetSeconds());
		S.Format("+++ ALL SCENs Writing Time %s \n", Time);
		logout << S;

		//Beep(100, 200);
		ShowStatus();
		OnShow();
		SetTitle("DONE");
		AfxMessageBox("BTR 5 multi is DONE!\n  LAST SCEN is shown");
	}
		
	//for (int i = 0; i <= MAXSCEN; i++)	ScenData[i].RemoveAll();
	//delete ScenData;

	return;

	//begin/resume/suspend thread with Tracer obj and ThreadFunc
	//SuspendAll(TRUE); // run = TRUE 
   //for (k = 0; k < ThreadNumber; k++) 	WaitForSingleObject(m_pTraceThread[k]->m_hThread, INFINITE);
		
}
	
	

void CBTRDoc:: OnStartParallel() // launches MULTI or Single SCEN 
								 // calls InitTracers
								 // start or resume threads //older - enabled BTR-Fast)
{
	CString S, s1, s2;
	int res;
	if (!STOP) {
		OnStop(); // only stop threads, keep NofCalculated
		//S.Format("Stopped (x%d)", ThreadNumber);
		//SetTitle(S);
		return;
	}
	OptParallel = TRUE;
	NofCalculated = 0; //new: NO restart after pause!!
	logout.SetLogSave(OptLogSave);

	if (BTRVersion  < 5) { //old version
		AfxMessageBox("Multi-run is supported from Version 5.0");
		return;
	}
	//OnShow();
	//	::MessageBox(NULL, "SCEN >= MAXSCEN", "OnStartPar", 0);

	CWnd * pMW = theApp.m_pMainWnd;// for collapsing in MULTI-SCEN
	pMW->FlashWindow(TRUE);

	int runs[3] = { 1, 0, 0 }; // { ATOMS, Resid, Reions } - basic options for each scen
	if (OptTraceAtoms == 0) runs[0] = 0;
	if (OptNeutrStop == 0) runs[1] = 1;
	if (OptReionStop == 0) runs[2] = 1;
	MAXRUN = runs[0] + runs[1] + runs[2];
		
	if (!FieldLoaded && MAXSCEN > 1 && runs[2] > 0) { // NO MF + REION task
		AfxMessageBox("MF is not loaded");
		return;
	}

	SCEN = 1;
	
	SuspendedSpan = 0;
	DataSaveTime = 0;
		
	StartTime0 = CTime::GetCurrentTime();// Global start
	StartTime = StartTime0; // //Reset time for current RUN start
	// RESET LOG for append
	ResetLogFile();
	
	//ShowStatus();//show active data on views
	CTime tm = StartTime;//CTime::GetCurrentTime();
	CString Date, Time;
	Date.Format("%02d-%02d-%04d", tm.GetDay(), tm.GetMonth(), tm.GetYear());
	Time.Format("%02d:%02d:%02d", tm.GetHour(), tm.GetMinute(), tm.GetSecond());
	S.Format("BTR %g -------- Date %s  Time %s ------------\n", BTRVersion, Date, Time);
	logout << S;// << std::endl;////////////
	
	//if (SCEN > 1 && MAXSCEN > 1) ResumeData(); // back to initial config
	
	if (!SetDefaultMesh()) // set surf resolution - before all runs
		return;// cancel run

	SetNullLoads();
	S.Format("----- Init Null Loads for ALL surfaces --------\n");
	logout << S;

	ShowStatus();//show active data on views

	if (MAXSCEN > 1) { // multi-run, v5.0 /////////////////////////
		//if (SetDefaultMesh()) {  // set surf resolution - before all runs = TRUE
			
		S.Format("\n\t *********** Start MULTI-RUN ***********\n");
		SetTitle(S);
		logout << S; // << std::endl;///////////////

		//CBTRApp theApp;///COLLAPSE MAIN VIEW on start /////////////////////////////
		pMW->ShowWindow(SW_SHOWMINIMIZED);
		pMW->UpdateWindow();///////////////////////////////////////////////////

		while (SCEN <= MAXSCEN) 
			RunScen(runs); // default runs are set in current Config
								// each SCEN can reset them in SCEN_PARAM
							// SCEN is +1 after each run-set is complete -> CompleteScen()
		
		//pMW->ShowWindow(SW_SHOWMAXIMIZED);
		pMW->FlashWindow(FALSE);//>UpdateWindow();
		
		ResetLogFile();//if (OptLogSave) CloseLogFile(); // after ALL SCENS
		
		return;
	} // if  MAXSCEN > 1 -> run Multi//////////////////////////////////////
	
//  else -> RUN SINGLE run old version (allows manual stop)/////////////////////////
		
/*	if (!STOP) {
		OnStop(); // only stop threads, keep NofCalculated
		s.Format("Stopped (%d threads)", ThreadNumber);
		SetTitle(s);
		return;	
	}*/
		
	//ClearAllPlates();// similar to SetNullLoads	//OptCombiPlot = -1; // no load 
	
	if (!SINGAPLoaded) SetSINGAPfromMAMUG();// called before by SetStatus (?)
	SetPlasmaTarget(); //init plasma object, Geometry, Nray = -1, clear arrays
		
/*	if (NofCalculated > 0 && NofCalculated < NofBeamlets) { // if paused
		s.Format("Beam is currently paused. Do you want to continue? \n (Press NO to start a new beam)");
			res = AfxMessageBox(s, MB_ICONQUESTION | MB_YESNO); 
			if (res	== IDNO) {
				OnStop();
			}
	}*/

	if (STOP) {  //MUST BE already STOPPED by OnStop() // start new run, NofCalculated = 0
		NofCalculated = 0;
		//OptDrawPart = TRUE;
	/*	if (OptDrawPart){
			s.Format("Tracks show will slow down the calculation!"
				"\n Do you want to hide the tracks during the run? \n");
			if (AfxMessageBox(s, MB_ICONQUESTION | MB_YESNO) == IDYES) OptDrawPart = FALSE;
		}*/
		
		RUN = 13;
		S.Format(".... Start SINGLE-RUN .....\n", BTRVersion, ThreadNumber);
		SetTitle(S);
		logout << S;// << std::endl;////////////////////

		//ThreadNumber = num_proc;
		for (int k = 0; k < ThreadNumber; k++) m_pTraceThread[k] = NULL;
			InitTracers();
			SuspendedSpan = 0;
			StartTime = CTime::GetCurrentTime();
			//OnShow();
	} // start NEW run

	else {// (Resume from pause) -> this is already checked by if (!STOP) see the top
		S.Format("Continued");	
		AfxMessageBox(S); 
		TendSuspend = CTime::GetCurrentTime();
		SuspendedSpan += TendSuspend - TbeginSuspend; 
	}
		
	STOP = FALSE;//started = TRUE !!!
	for (int i = 0; i < ThreadNumber; i++)	
		m_Tracer[i].SetContinue(TRUE);
	SuspendAll(TRUE); // run = TRUE
	::GdiFlush();
	//CloseLogFile();// after SINGLE RUN
	//OnShow();
	//ShowStatus();
	//if (NofCalculated < NofBeamlets) ShowLogFile(0);// all

	/*CalculateTracks();
	pBeamHorPlane->Load->SetSumMax(); // created in SetPlatesNBI
	pBeamVertPlane->Load->SetSumMax();// created in SetPlatesNBI*/
	
}

void CBTRDoc:: GetMemState()
{
	int DIV = 1024;

	MEMORYSTATUSEX statex;
	statex.dwLength = sizeof (statex);
	GlobalMemoryStatusEx (&statex);

//  printf ("There is  %*ld percent of memory in use.\n",WIDTH, statex.dwMemoryLoad);
//  printf ("There are %*I64d total Kbytes of physical memory.\n", WIDTH, statex.ullTotalPhys/DIV);//DIV = 1024
//  printf ("There are %*I64d free Kbytes of physical memory.\n", WIDTH, statex.ullAvailPhys/DIV);//DIV = 1024

	MemFree = (long long) (statex.ullAvailPhys /DIV); // available on the system
	
	HANDLE hProcess = GetCurrentProcess();
	PROCESS_MEMORY_COUNTERS pmc;
	//DWORD processID = GetCurrentProcessId(); 
	//hProcess = OpenProcess(  PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID );
	if (NULL == hProcess)  return;

	if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc)) )
		MemUsed = pmc.WorkingSetSize /DIV; // currently used by BTR
		//MemUsed = pmc.PrivateUsage / DIV;
		//CloseHandle( hProcess );
	//long Falls = 0;// total fall-points
	MemFalls = 0;// m_AttrVector[0].size() * ThreadNumber * sizeof(SATTR) / DIV;
	//for (int i = 0; i < ThreadNumber; i++) Falls += m_AttrVector[i].size();// falls within thread
	//ArrSize = m_GlobalVector.size();//ArrSize = Falls;
	//Falls = m_GlobalDeque.size();

}

int CBTRDoc:: Run_BTR_Fast() // 
//example 
//https://www.codeproject.com/Articles/18067/Synchronization-in-Multithreaded-Applications-with
{	
	m_ThreadArray.RemoveAll();
	//CWinThread * pThread[16];// max - 16
	//HANDLE hThread[16];
	int NofThreads = min(ThreadNumber, MaxThreadNumber);//=  num_proc; //(21)
	//int ThreadNumber;
	//BEAMLET_ENTRY b;
	//CWaitCursor wait;
	int numb = NofBeamlets / NofThreads; // beamlets per thread
	NofCalculated = 0; // init global beamlets counter
	int Nmin, Nmax;
	CPoint bml;// min/max bml limits
	CTracer * pTracer;
	
//CWinThread *pThread = AfxBeginThread(ThreadDraw,(PVOID)pInfo, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
	
	InitTracers();//calls ClearArrays, init start BML
		
	for (int k = 0; k < NofThreads; k++) 
	{
		//THREADINFO *pInfo = new THREADINFO;
		//pInfo->hWnd = GetSafeHwnd();
		//pInfo->point = point;
		//pInfo->m_ID = k + 1;
		//pInfo->pDoc = this;
		//pInfo->pView = pMainView;
		//Nmin = k * numb; Nmax = (k + 1)*numb - 1;
		//bml.x = Nmin; bml.y = Nmax;
		//pInfo->BML = bml;
		//pTracer = &m_Tracer[k]; //created in CBTRDoc, must be static  //new CTracer();
		//pTracer->SetLimits(Nmin, Nmax);
		//InitTracer(k, pTracer); // similar to InitTracers 
		//pInfo->pTracer = pTracer;

		//CWinThread *pThread = AfxBeginThread(_Threadfunc,&bml, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);//works!
		//CWinThread *pThread = AfxBeginThread(_Threadfunc,(PVOID)pInfo, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
		//CWinThread *pThread = AfxBeginThread(ThreadFunc, &m_Tracer[iIndex]);
		
		CWinThread *pThread = AfxBeginThread(ThreadFunc, &m_Tracer[k], THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
		pThread->m_bAutoDelete = FALSE;
		pThread->ResumeThread();
		m_ThreadArray.Add(pThread);

		//pThread[k] = ::AfxBeginThread(_Threadfunc, &BML); 
		//hThread[k] = pThread[k]->m_hThread;
		
	} 

	int nSize = m_ThreadArray.GetSize();
	HANDLE *h = new HANDLE[nSize];

	for (int j = 0; j < nSize; j++)
	{
		h[j] = m_ThreadArray[j]->m_hThread;
	}

	// Wait for all threads to terminate
   	::WaitForMultipleObjects(NofThreads, h, TRUE, INFINITE);
    for (int j = 0; j < nSize; j++)
	{
		delete m_ThreadArray[j];
	}
	delete[] h;

	
	// alternative way to destroy
	//g_eventEnd.SetEvent();
	/*for (int j = 0; j < m_ThreadArray.GetSize(); j++){
		::WaitForSingleObject(m_ThreadArray[j]->m_hThread, INFINITE);
		delete m_ThreadArray[j];
	}*/

	//CString s;
	//s.Format("Scen %d DONE\n %d threads \n total %d beamlets", SCEN, nSize, NofCalculated);
	//::MessageBox(NULL, s, "Run_BTR_FAST", 0);
	
	//OnShow();
	//::GdiFlush();

	//cs.Unlock();
	if (NofCalculated == NofBeamletsTotal) //finished 
		 return 1;  
	else return 0; // stopped
}

UINT CBTRDoc:: _ThreadFunc(LPVOID param)// not active now (earlier called by BTR_FAST) 
	// uses new InitTracer
	//calls TraceAll 5.0 (or Draw 4.5)
	
{	
	//CBTRDoc * pDoc = (CBTRDoc*)pMainView->GetDocument();//
	BEAMLET_ENTRY be;// = pDoc->BeamEntry_Array.GetAt(m_iSource);
	
	//static int snCount = 0;
	//snCount++;
	//TRACE("- ThreadDraw %d: started...\n", snCount);
	//THREADINFO *pInfo = reinterpret_cast<threadinfo /> (pParam);//???
	//CWnd *pWnd = CWnd::FromHandle(pInfo->hWnd);
	//CClientDC dc(pWnd);

/*	THREADINFO *pInfo = (THREADINFO *)param;
	int m_ID = pInfo->m_ID;
	int min = pInfo->BML.x;
	int max = pInfo->BML.y;
	CBTRDoc* pDoc = (CBTRDoc*)pInfo->pDoc;*/
	
	CTracer * pTrace = (CTracer*)param;
	//pMainView is already defined
	//CMainView * pView = (CMainView*)pInfo->pView;
	//CPoint * lim = (CPoint*) param;
	CCriticalSection cs;
	cs.Lock();
	int m_ID = pTrace->Get_ID();
	int min = pTrace->GetBMLmin(); // from BML (start)
	int max = pTrace->GetBMLmax(); // upto (include)
	cs.Unlock();

	CString S;
	//S.Format("Tracer %d: min %d  max %d", m_ID, min, max);
	//::MessageBox(NULL, S, "BTR 5 ThreadFunc", 0);
	
	if (min < 0 || max < 0) { 
		S.Format("Invalid min or max BML in thread %d", pTrace->Get_ID());
		::MessageBox(NULL, S, "BTR 5 ThreadFunc", 0); 
		return 1;
	}
	

	///// old 4.5/5 ThreadFunc //////////////
	
/*	for (int is = min; is <= max; is++) {
		//bool done = TRUE;
		//done = pTrace->TraceBeamletAtoms(is); // atoms with decay
		pTrace->ShowBeamlet(is);// locked by tracer's CS!!

		cs.Lock();
		NofCalculated++;
		cs.Unlock();
	}*/

	while (pTrace->GetContinue()) //(TRUE)//

		if (BTRVersion - 4.5 < 1.e-6)
			pTrace->Draw(); // 4.5
		else
			pTrace->TraceAll(); // new - 5.0*/
			//pTrace->ClearArrays();	pTrace->TestMem();
	
	//S.Format("Thread %d done \n %d - %d bml", m_ID, min + 1, max + 1);
	//::MessageBox(NULL, S, "BTR 5 ThreadFunc", 0);
	return 0;

	////////////////////////////////////////////
/*
	//HDC DC = pMainView->GetDC()->GetSafeHdc();
	
	CDC* pDC = pMainView->GetDC(); //pMainView->GetDC();
	CPen * pOldPen = pDC->SelectObject(&pMainView->AtomPen);//DotPen);
	
	for (int is = min; is <= max; is++) // < NofBeamlets; k++)
	{
		//be = pDoc->BeamEntry_Array[is];//be = SourceArr.GetAt(k);
		//pDoc->ShowBeamlet(be); -- not working directly
		// CTracer:: ShowBeamlet(is) - copy //////////////////////////
		be = BeamEntry_Array.GetAt(is);// [is];//.GetAt(k);
		int x1, y1, z1, x2, y2, z2;
		double y0 = be.PosY;
		double z0 = be.PosZ;
		double	x = Max(PlasmaXmax, AreaLong);
		double	y = y0 + x * tan(be.AlfY);
		double	z = z0 + x * tan(be.AlfZ);

		x1 = pMainView->OrigX;
		y1 = pMainView->OrigY - (int)(y0 * pMainView->ScaleY);
		z1 = pMainView->OrigZ - (int)(z0 * pMainView->ScaleZ);
		x2 = pMainView->OrigX + (int)(x * pMainView->ScaleX);
		y2 = pMainView->OrigY - (int)(y * pMainView->ScaleY);
		z2 = pMainView->OrigZ - (int)(z * pMainView->ScaleZ);
			
		pDC->MoveTo(x1, y1); pDC->LineTo(x2, y2);
		pDC->MoveTo(x1, z1); pDC->LineTo(x2, z2);
//////////////////////////////////////////////////////				
		cs.Lock();
		
		NofCalculated++;

		cs.Unlock();				
/////////////////////////////////////////////////////
				
	}
	pDC->SelectObject(pOldPen);
	pMainView->ReleaseDC(pDC);
	GdiFlush();
*/		
	S.Format("Thread %d done \n %d - %d bml", pTrace->Get_ID(), min+1, max+1);
	::MessageBox(NULL, S, "BTR 5 ThreadFunc", 0);
	//Sleep(1000);
	return 0;
}


void CBTRDoc:: F_CalcRIDsimple()  //simplified form
{
	CPlate * plate = pMarkedPlate;//(CPlate *) param;
	if (plate == NULL) return;// -1;
	CLoad * L = plate->Load;
	C3Point Ploc, Ploc0, Ploc1, Ps, Pgl, P0, V;
	double  Ay, Az, Dy, Dz, DyH, DzH, dens, SumDens;
	double totdist, Cos, Path0, Y0, Z0, X0 = RIDXmin, distX, distZ;
	// double TotSum = 0;
	
	BEAMLET_ENTRY b;
	double divH = BeamHaloDiverg; //SourceArr[0].DivY; //*pDoc->BeamHaloDiverg;
	double H = BeamHaloPart; //SourceArr[0].Fraction; //*pDoc->BeamHaloPart;			
	double BeamletPower = IonBeamPower * 1000000 /NofBeamletsTotal;//(SourceArr[0].PosY) * (SourceArr[0].PosZ); 
	//pDoc->IonBeamPower *1000000/ pDoc->NofBeamletsTotal * NeutrPart; //W per beamlet 

	//CCriticalSection cs;
		
	int MaxSource = SourceArr.GetUpperBound();// Element 0 is for halo!
	if (MaxSource < 1) return;// 0; // Element 0 is for halo!

	int x1, y1, z1, x2, y2, z2;
	double y0, z0, x, y, z, dx, dy, dz;
	BOOL positive;
	double IonFraction;
	int sign;
	double dt = 1.0e-9, alfaEff, vx, vy, vz;

	CDC* pDC = pMainView->GetDC();
	//HDC DC = pMainView->GetDC()->GetSafeHdc();
	CPen * pOldPen;
	CPen pen;

	if (RID_A * plate->OrtZ.Y < 0) {
		positive = TRUE;
		pen.CreatePen(PS_SOLID, 1, RGB(200,0,0));
		pOldPen = pDC->SelectObject(&pen);	
		sign = 1;
		IonFraction = PosIonPart;
	}
	else {
		positive = FALSE;
		pen.CreatePen(PS_SOLID, 1, RGB(0,200,0));
		pOldPen = pDC->SelectObject(&pen);	
		sign = -1;
		IonFraction = 1.0 - NeutrPart - PosIonPart;
	}
	
	for (int k = 1; k <= MaxSource; k++) { // NofBeamlets or NofCalculated
			b = SourceArr.GetAt(k); 
			y0 = b.PosY;
			z0 = b.PosZ;
			// beamlet position at RID entry
			x = X0;// RIDXmin
			y = y0 + x * tan(b.AlfY); 
			z = z0 + x * tan(b.AlfZ); 
	
			if (fabs(y - plate->Orig.Y) > RIDInW) continue;// out of channel
			if ((y - plate->Orig.Y) * plate->OrtZ.Y < 0) continue;// non-face plate side
					
			x1 = pMainView->OrigX + (int)(x * pMainView->ScaleX);
			y1 = pMainView->OrigY - (int)(y * pMainView->ScaleY);
			z1 = pMainView->OrigZ - (int)(z * pMainView->ScaleZ);
							
			BOOL stopped = FALSE;
			Y0 = fabs(y - plate->Orig.Y); 
			Z0 = z;
			Path0 = 0;
						
			alfaEff = sqrt(b.AlfY * b.AlfY  + b.AlfZ * b.AlfZ  + 1.0);
			vx = Part_V / alfaEff;
			vy = Part_V * tan(b.AlfY) / alfaEff;
			vz = Part_V * tan(b.AlfZ) / alfaEff;
			
			while (!stopped) { // tracing axis particle
				dx = vx * dt;
				dz = vz * dt;
				x += dx; z += dz; 
				vy += sign * RID_A * dt; 
				dy = vy * dt;
				y += dy;
				Path0 += sqrt(dx*dx + dy*dy + dz*dz);
				x2 = pMainView->OrigX + (int)(x * pMainView->ScaleX);
				y2 = pMainView->OrigY - (int)(y * pMainView->ScaleY);
				z2 = pMainView->OrigZ - (int)(z * pMainView->ScaleZ);
				pDC->MoveTo(x1, y1); pDC->LineTo(x2, y2); 
				pDC->MoveTo(x1, z1); pDC->LineTo(x2, z2);

				P0 = C3Point(x,y,z);
				Ploc0 = plate->GetLocal(P0); // axis point

				if (Ploc0.Z < 0 || Ploc0.Z > RIDchannelWidth)  stopped = TRUE;
				x1 = x2; y1 = y2; z1 = z2;
			}
			//GdiFlush();
			//cs.Unlock();

			for (int i = 0; i <= L->Nx; i++) {
			Ploc.X = i * L->StepX;
			Ploc.Z = 0;
			for (int j = 0; j <= L->Ny; j++) {
				Ploc.Y = j * L->StepY;
				Pgl = plate->GetGlobalPoint(Ploc);
				distX = Pgl.X - X0; // dist to entry point
				distZ = fabs(Pgl.Z - Z0);
				y = distX * distX /(P0.X-X0)/(P0.X-X0) * Y0;

				Ay = atan((y-Y0)/X0) + b.AlfY;// horizontal angle
				Az = atan((Pgl.Z - z0)/Pgl.X);
				totdist = X0 + Path0 * distX  /(P0.X - X0);// very approx! 

				Ploc1.X = 0;	Ploc1.Y = 0;	Ploc1.Z = y; //point image
				if (Ploc1.Z > RIDInW) continue; // out of channel
				Pgl = plate->GetGlobalPoint(Ploc1); 
				Pgl.Z = z0 + X0 * tan(Az); 
				Ps = C3Point(0, y0, z0);// source point
				if (!F_GotThrough(Ps, Pgl)) continue; // intercepted before RID
			
				//SumDens = 0;
				V.X = vx; V.Z = vx * Az;
				if (distX > 1.e-6)	V.Y = sign * vx * 2 * y * RID_A / fabs(RID_A) / distX;
				else V.Y = 0;
				Cos = -ScalProd(V, plate->OrtZ) / ModVect(V);
				if (Cos < 1.e-6) continue; // no load
			
				Dy = GAUSS(fabs(Ay - b.AlfY), b.DivY);
				Dz = GAUSS(fabs(Az - b.AlfZ), b.DivZ);
				DyH = GAUSS(fabs(Ay - b.AlfY), divH);
				DzH = GAUSS(fabs(Az - b.AlfZ), divH);
				dens = (1-H) * Dy * Dz + H * DyH * DzH; 
				//SumDens += BeamletPower * dens * Cos * b.Fraction / (dist*dist * PI);
				L->Val[i][j] += IonFraction * BeamletPower * dens * Cos * b.Fraction / (totdist*totdist * PI);
			} // j
			} // i
			

			//ShowProfiles = TRUE;
			//plate->ShowLoadState();			

	}// k = MaxSource
	pDC->SelectObject(pOldPen);
	pMainView->ReleaseDC(pDC);
	
	return;// 1;
}

void CBTRDoc:: F_CalcRID()  //exact form
{
	CPlate * plate = pMarkedPlate;//(CPlate *) param;
	if (plate == NULL) return;// -1;
	CLoad * L = plate->Load;
	C3Point Ploc, Ploc0, Ploc1, Ps, Pgl, P0, V;
	double Ay, Az, Dy, Dz, DyH, DzH, dens, SumDens;
	double TotSum = 0;
	
	BEAMLET_ENTRY b;
	double divH = BeamHaloDiverg; //SourceArr[0].DivY; //*pDoc->BeamHaloDiverg;
	double H = BeamHaloPart; //SourceArr[0].Fraction; //*pDoc->BeamHaloPart;			
	double BeamletPower = IonBeamPower * 1000000 /NofBeamletsTotal;//(SourceArr[0].PosY) * (SourceArr[0].PosZ); 
	
	int MaxSource = SourceArr.GetUpperBound();// Element 0 is for halo!
	if (MaxSource < 1) return;// 0; // Element 0 is for halo!
	
	BOOL positive;
	double IonFraction;
	int sign;

	CDC* pDC = pMainView->GetDC();
	//HDC DC = pMainView->GetDC()->GetSafeHdc();
	CPen * pOldPen;
	CPen pen;

	if (RID_A * plate->OrtZ.Y < 0) {
		positive = TRUE;
		pen.CreatePen(PS_SOLID, 1, RGB(200,0,0));
		pOldPen = pDC->SelectObject(&pen);	
		sign = 1;
		IonFraction = PosIonPart;
	}
	else {
		positive = FALSE;
		pen.CreatePen(PS_SOLID, 1, RGB(0,200,0));
		pOldPen = pDC->SelectObject(&pen);	
		sign = -1;
		IonFraction = 1.0 - NeutrPart - PosIonPart;
	}

	int x1, y1, z1, x2, y2, z2;
	double Ys, Zs, X0 = RIDXmin+0.01, Y0, Z0, y0, x, y, z, dx, dy, dz;
	double dt = 1.0e-9, alfaEff, V0, VV0, Vx0, Vy0, Vz0, Vx, Vy, Vz, Vtan, Vabs, t0, t;
	double Dist0, Dist, TotDist, Len, TotLen, Cos;
	double A, B, C, D;
	
	for (int k = 1; k <= MaxSource; k++) { // NofBeamlets or NofCalculated
			b = SourceArr.GetAt(k); 
			Ys = b.PosY;
			Zs = b.PosZ;
			Ps = C3Point(0, Ys, Zs);
			// global position at RID entry - X0, Y0, Z0
			Y0 = Ys + X0 * tan(b.AlfY);
			Z0 = Zs + X0 * tan(b.AlfZ);
			x = X0;	y = Y0; z = Z0; 
			
	
			if (fabs(y - plate->Orig.Y) > RIDInW) continue;// out of channel
			if ((y - plate->Orig.Y) * plate->OrtZ.Y < 0) continue;// non-face plate side
					
			x1 = pMainView->OrigX + (int)(x * pMainView->ScaleX);
			y1 = pMainView->OrigY - (int)(y * pMainView->ScaleY);
			z1 = pMainView->OrigZ - (int)(z * pMainView->ScaleZ);
							
			BOOL stopped = FALSE;
			y0 = fabs(y - plate->Orig.Y); 
									
			V0 = Part_V;
			VV0 = V0 * V0;
			/*alfaEff = sqrt(b.AlfY * b.AlfY  + b.AlfZ * b.AlfZ  + 1.0);
			Vx0 = V0 / alfaEff;
			Vy0 = V0 * tan(b.AlfY) / alfaEff;
			Vz0 = V0 * tan(b.AlfZ) / alfaEff;*/
			Vx0 = V0 * cos(b.AlfY)* cos(b.AlfZ);
			Vy0 = V0 * sin(b.AlfY);
			Vz0 = V0 * cos(b.AlfY)* sin(b.AlfZ);
	
			Vx = Vx0; Vy = Vy0; Vz = Vz0;
			t0 = 0;
			Dist = 0;
			Len = 0;
			
			while (!stopped) { // tracing axis particle - not for load calculation!
				dx = Vx * dt;
				dz = Vz * dt;
				x += dx; z += dz; 
				Vy += sign * RID_A * dt; 
				dy = Vy * dt;
				y += dy;
				Len += sqrt(dx*dx + dy*dy + dz*dz);
				t0 += dt;
				Dist += sqrt(dx*dx + dz*dz);
				x2 = pMainView->OrigX + (int)(x * pMainView->ScaleX);
				y2 = pMainView->OrigY - (int)(y * pMainView->ScaleY);
				z2 = pMainView->OrigZ - (int)(z * pMainView->ScaleZ);
				pDC->MoveTo(x1, y1); pDC->LineTo(x2, y2); 
				pDC->MoveTo(x1, z1); pDC->LineTo(x2, z2);

				P0 = C3Point(x,y,z);
				Ploc0 = plate->GetLocal(P0); // axis point

				if (Ploc0.Z < 0 || Ploc0.Z > RIDchannelWidth)  stopped = TRUE;
				x1 = x2; y1 = y2; z1 = z2;
			}
			t0 = Dist / sqrt(Vx0*Vx0 + Vz0*Vz0);
			
			for (int i = 0; i <= L->Nx; i++) {
			Ploc.X = i * L->StepX;
			Ploc.Z = 0;
			for (int j = 0; j <= L->Ny; j++) {
				Ploc.Y = j * L->StepY;
				Pgl = plate->GetGlobalPoint(Ploc);
				//distX = Pgl.X - X0; // dist to RID entry point
				//distZ = Pgl.Z - Zs;
				//y = distX * distX /(P0.X-X0)/(P0.X-X0) * Y0;
				//Ay = atan((y-Y0)/X0) + b.AlfY;// horizontal angle
				TotDist = sqrt(Pgl.X * Pgl.X + (Pgl.Z - Zs)*(Pgl.Z - Zs));// GetDistBetween(Ps, Pgl); // from beam source to calc.point
				Az = atan((Pgl.Z - Zs)/Pgl.X);
				Dist0 = TotDist * X0 / Pgl.X; // from source to RID
				Dist = TotDist - Dist0; // in RID
				if (Dist < 1.e-6) continue;
				//totdist = X0 + Path0 * distX  /(P0.X - X0);// very approx! 

				A = 1;
				B = - 2 * VV0 * Dist0 * (1 + Dist0/Dist) / (Dist * fabs(RID_A));// * sign);
				C = Dist0 * Dist0;
				D = B * B - 4 * A * C;
				if (D < 0) continue;

				y = (-B - sqrt(D)) * 0.5;
				if (y <= 1.e-6) {
					y = (-B + sqrt(D)) * 0.5;
				}
				if (y > RIDInW) continue; // out of channel
				
				Ploc1.X = 0;	 Ploc1.Y = 0;	Ploc1.Z = y; //point image
				//if (Ploc1.Z > RIDInW) continue; // out of channel
				Pgl = plate->GetGlobalPoint(Ploc1); 
				Pgl.X = X0;
				Pgl.Z = Zs + X0 * tan(Az);
				//Pgl.Y = plate->Orig.Y + y * plate->OrtZ.Y;//y0 = fabs(y - plate->Orig.Y); 
				
				
				//Ps = C3Point(0, Ys, Zs);// source point
				if (!F_GotThrough(Ps, Pgl)) continue; // intercepted before RID
				Ay = atan((Pgl.Y - Ys)/Pgl.X);
			
				//SumDens = 0;
				Vtan = V0 * cos(Ay);
				t = Dist / Vtan;
				Vy0 = V0 * sin(Ay);
				V.Y = Vy0 + sign * RID_A * t;
				V.X = Vtan * cos(Az); 
				V.Z = Vtan * sin(Az);
				//if (Dist > 1.e-6)	V.Y = sign * vx * 2 * y * RID_A / fabs(RID_A) / distX;
				//else V.Y = 0;
				Vabs = ModVect(V);
				Cos = -ScalProd(V, plate->OrtZ) / Vabs;
				if (Cos < 1.e-6) continue; // no load

				//Len = t * sqrt(VV0 - 2 * sign * RID_A * y);
				/*Len = 0.5 / fabs(RID_A) * 
					( fabs(V.Y) * Vabs - fabs(Vy0) * V0 + 
					Vtan*Vtan * log((fabs(V.Y) + Vabs)/(fabs(Vy0) + V0)) ); */
				//Vy0 = 0;
				Len = 0.6 / fabs(RID_A) * 
					(fabs(V.Y) * Vabs - Vy0 * V0 + 
					Vtan*Vtan * log((fabs(V.Y) + Vabs)/fabs(Vy0 + V0)) ); 

				TotLen = Dist0/cos(Ay) + Len;// * t / t0;
				
				Dy = GAUSS(fabs(Ay - b.AlfY), b.DivY);
				Dz = GAUSS(fabs(Az - b.AlfZ), b.DivZ);
				DyH = GAUSS(fabs(Ay - b.AlfY), divH);
				DzH = GAUSS(fabs(Az - b.AlfZ), divH);
				dens = (1-H) * Dy * Dz + H * DyH * DzH; 
				SumDens = IonFraction * BeamletPower * dens * Cos * b.Fraction / (TotLen*TotLen * PI);
				L->Val[i][j] += SumDens; //IonFraction * BeamletPower * dens * Cos * b.Fraction / (TotLen*TotLen * PI);
				TotSum += SumDens;
		
			} // j
			} // i
			//ShowProfiles = TRUE;
			//plate->ShowLoadState();
	}// k = MaxSource
	
	pDC->SelectObject(pOldPen);
	pMainView->ReleaseDC(pDC);
	
	return;// 1;
}
void CBTRDoc:: F_CalcDirect() //UINT _ThreadCalcLoad(LPVOID param)
{
//	CBTRDoc * pDoc = (CBTRDoc*) param;// pointer type cast
//	if (pDoc == NULL || !pDoc->IsKindOf(RUNTIME_CLASS(CBTRDoc))) return -1;
	CPlate * plate = pMarkedPlate;//(CPlate *) param;// pDoc->pMarkedPlate;
	if (plate == NULL) return;// -1;

	CLoad * L = plate->Load;
	C3Point Ploc, Pgl, P0, V;
	double  Ay, Az, Dy, Dz, DyH, DzH, dens, SumDens;
	double dist, Cos, CosX, CosY, VortX, VortY, VortZ;
	// double TotSum = 0;
	
	BEAMLET_ENTRY b;
	double divH = BeamHaloDiverg; //SourceArr[0].DivY; //*pDoc->BeamHaloDiverg;
	double H = BeamHaloPart; //SourceArr[0].Fraction; //*pDoc->BeamHaloPart;			
	double BeamletPower = IonBeamPower *1000000/NofBeamletsTotal; //* NeutrPart;//(SourceArr[0].PosY) * (SourceArr[0].PosZ); 
	//pDoc->IonBeamPower *1000000/ pDoc->NofBeamletsTotal * NeutrPart; //W per beamlet 
	if (plate->Orig.X > NeutrOutX) BeamletPower *= NeutrPart;
	
	//CCriticalSection cs;
	int x1, y1, z1;
	
	//int i = SourceArr[0].i; // row number (pDoc->IofCalculated);
	//if (i < 0) return 0;

	int MaxSource = SourceArr.GetUpperBound();
	if (MaxSource < 1) return;// 0; // Element 0 is for halo!

/*	for (i = 0; i <= L->Nx; i++) {
	Ploc.X = i * L->StepX;
	Ploc.Z = 0;
	for (int j = 0; j <= L->Ny; j++) {
		Ploc.Y = j * L->StepY;
		Pgl = plate->GetGlobalPoint(Ploc); 
		
		SumDens = 0;*/
		
		for (int k = 1; k <= MaxSource; k++) { // NofBeamlets or NofCalculated
			b = SourceArr.GetAt(k); //pDoc->BeamEntry_Array.GetAt(k);
			P0 = C3Point(0, b.PosY, b.PosZ); // ion source position
			
			for (int i = 0; i <= L->Nx; i++) {
				Ploc.X = i * L->StepX;
				Ploc.Z = 0;
			for (int j = 0; j <= L->Ny; j++) {
				Ploc.Y = j * L->StepY;
				Pgl = plate->GetGlobalPoint(Ploc); 
			
			if (!F_GotThrough(P0, Pgl)) continue; // 
			
			V = VectSum(Pgl, P0, 1, -1);
			dist  = ModVect(V);
			Ay = atan(V.Y/V.X);
			Az = atan(V.Z/V.X);
			
			//Normal = - plate->OrtZ;
			Cos = -ScalProd(V, plate->OrtZ) / dist;
			if (Cos < 1.e-6) continue; // no load
						
			Dy = GAUSS(fabs(Ay - b.AlfY), b.DivY);
			Dz = GAUSS(fabs(Az - b.AlfZ), b.DivZ);
			DyH = GAUSS(fabs(Ay - b.AlfY), divH);
			DzH = GAUSS(fabs(Az - b.AlfZ), divH);
			dens = (1-H) * Dy * Dz + H * DyH * DzH; 
			//SumDens += BeamletPower * dens * Cos * b.Fraction / (dist*dist * PI);
			L->Val[i][j] += BeamletPower * dens * Cos * b.Fraction / (dist*dist * PI);
			} // j
			} // i
			//OnPlotMaxprofiles();
			//ShowProfiles = TRUE;
			//pSetView->InvalidateRect(NULL, TRUE);
			//plate->ShowLoadState(); // works well for many beamlets
		} // k

		//L->Val[i][j] = SumDens;
		
		//TotSum += SumDens;

	//} // i
	//ShowProfiles = TRUE;
	//plate->ShowLoadState(); // works well for many beamlets
	//} // i
	//dc.Detach();

	/*cs.Lock();
	SourceArr[0].j = i;	//(pDoc->ThreadNumber)++;
	cs.Unlock();*/
	
	return;// 1;
}
/*double CBTRDoc:: GAUSS(double arg, double delta) // inline
{	
	return exp(-arg*arg / (delta*delta)) / delta;
}*/

BOOL F_GotThrough(C3Point P1, C3Point P2)
{
	int PassedNumber = 0;
	int Channel = 0;
	RECT_DIA dia;
	double xdia, y, z, ymin, ymax, zmin, zmax;
	C3Point V = VectSum(P2, P1, 1, -1);
	int NdiaMax = Dia_Array.GetUpperBound();
			for (int ndia = 0; ndia <= NdiaMax; ndia++) {
				dia = Dia_Array.GetAt(ndia);
				
				if (dia.Number - PassedNumber > 1) // stopped at prev layer!
					return 0;
				if (dia.Number > 1 && dia.Number < 5 
					&& Channel != 0 && dia.Channel != Channel) // wrong channel
					continue;

				xdia  = dia.Corn[0].X;
				if ((xdia - P1.X) * (xdia - P2.X) > -1.e-12 ) { //both points at one side
					PassedNumber = dia.Number; 
					Channel = 0;
					continue; // go next dia
				}
				ymin = dia.Corn[0].Y; ymax = dia.Corn[2].Y;
				zmin = dia.Corn[0].Z; zmax = dia.Corn[2].Z;
				y = P1.Y + V.Y * xdia / V.X;
				z = P1.Z + V.Z * xdia / V.X;
				if ((y-ymin) * (y - ymax) > -1.e-12) continue; // crossed outside
				if ((z-zmin) * (z - zmax) > -1.e-12) continue; // crossed outside

				PassedNumber = dia.Number;
				Channel = dia.Channel; 
				// success
				
			} // ndia

			if (PassedNumber >= 15) return 1; //passed all
			else return 0; // stopped 
}

bool CBTRDoc:: F_CrossedSolid(C3Point P1, C3Point P2)
{
	//PtrList & PlatesList = PlatesList;
	POSITION pos = PlatesList.GetHeadPosition();
	CPlate * plate;
	C3Point Pend, Ploc;
	C3Point Last = P2;
	double dist;//, dist0 = GetDistBetween(P1, P2);
	
	while (pos != NULL) {
			plate = PlatesList.GetNext(pos); 
			if (plate->Solid == FALSE) continue;

			if (plate->IsCrossed(P1, Last)) { // crossed solid
				Ploc = plate->FindLocalCross(P1, Last);
				Pend = plate->GetGlobalPoint(Ploc);
				dist = GetDistBetween(P2, Pend);
				if (dist < 1.e-3) return 0;
								
				if (Ploc.X < plate->Xmax && Ploc.X > 0 
					&& Ploc.Y < plate->Ymax && Ploc.Y > 0) { // within plate
						return TRUE;
					
					/*dist = GetDistBetween(P1, Pend);
					 if (dist < dist0) { // check if plate is closer than previous
						//dist0 = dist;
						//Last = Pend;
						return 1; // YES
					}*/
				} // within plate
			} // crossed solid
			
		} // while pos
	return 0; // NO
}
	
void CBTRDoc:: F_CalculateLoad(CPlate * plate)
{
	if (OptCalOpen == FALSE && plate->Orig.X > CalOutX) return; 
	CWinThread * pThread[1000];// max 
	//HANDLE hThread[1000];

	int NofThreads;
	//int ThreadNumber;

	CWaitCursor wait;
	CCriticalSection cs;
	STOP = FALSE;
	plate->Loaded = TRUE;
	CLoad * L = plate->Load;
	pSetView->Load = L;
	C3Point Ploc, Pgl, P0, V, Vxy, Vxz, Normal;
	double dist, Cos, Ay, Az, Dy, Dz, dens, SumDens;
	int i;
	//double BeamPower = IonBeamPower; // MW
	//double BeamletPower = BeamPower *1000000. / NofBeamletsTotal ; //W per beamlet 
	SourceArr[0].PosZ = 1;
	if (plate->Orig.X > NeutrXmax) SourceArr[0].PosZ = NeutrPart;
	
	//ThreadNumber = 0;// thread counter, which is increased on _Threadfunc call
	IofCalculated = 0;// rows counter
	
	NofThreads = 1;//L->Nx + 1;

	BOOL RIDwall = FALSE;
	if (plate->Comment.Find("RID", 0) >= 0 && plate->Comment.Find("wall", 0) >= 0)
		RIDwall = TRUE;
		
	if (!OptNeutrStop && RIDwall) { // RID wall
		RIDXmin = RIDInX;
		RIDchannelWidth = RIDInW;
		C3Point p = plate->MassCentre;
		//p.Y += 0.1 * plate->OrtZ.Y;
		C3Point E = RIDField->GetE(p.X, p.Y, p.Z);
		RID_A = Qe * E.Y / TracePartMass;
		double EkeV = IonBeamEnergy * 1000; //keV
	    Part_V = sqrt(2.* EkeV * 1000 * Qe / TracePartMass); // {m/c}
		
		F_CalcRID();
		
	/*for (i = 0; i < NofThreads; i++) {
		pThread[i] = ::AfxBeginThread(_ThreadCalcRID, plate);
		
	} // i,j */
	//Sleep(1000);
		STOP = TRUE;
		return;
	}

	else {

		F_CalcDirect();

	/*for (i = 0; i < NofThreads; i++) {
		cs.Lock();
		SourceArr[0].i = i;
		cs.Unlock();
		//Sleep(100);
		pThread[i] = ::AfxBeginThread(_ThreadCalcLoad, plate);
		//hThread[i] = pThread[i]->m_hThread;
		//ThreadNumber++;
		IofCalculated = SourceArr[0].j;
	} // i,j*/
	

 // Wait for all threads to terminate
  	/* WaitForMultipleObjects(NofThreads, hThread, TRUE, INFINITE);
    // Close thread (and mutex) handles
	for( i=0; i < NofThreads; i++ ){
		CloseHandle(hThread[i]);
		//delete pThread[i];
	} */
	/*for (i = 0; i < NofThreads; i++)
	{
		if (pThread[i] != NULL) {
		//	while (1)
			DWORD res = WaitForSingleObject(pThread[i]->m_hThread, INFINITE);
			//delete pThread[i];// error
		}
	}*/
		STOP = TRUE;
		return;
	} // not RID

	
	//plate->ShowLoadState();
}

void CBTRDoc::OnStart() // BTR-slow
{
	//if (!STOP) OnStop();
	if (!STOP) {
		AfxMessageBox("Stop the beam before restart! \n (push stop-button on the Toolbar)");
		return;
	}
	int res, i;
	if (pDataView->m_rich.GetModify()) {
		res = AfxMessageBox("Data is modified but NOT UPDATED! \n Continue?", 3);
		if (res != IDYES) return;
	}
//	else AfxMessageBox("Data is not modified");

	CString s;

	Progress = 0;

	OptParallel = FALSE;
/*	for (i = 0; i < ThreadNumber; i++)	m_Tracer[i].SetContinue(FALSE);
	SuspendAll(FALSE);
	for (i = 0; i < ThreadNumber; i++) {
		WaitForSingleObject(m_pTraceThread[i]->m_hThread, INFINITE);
	}*/
	InitTracers(); // clear arrays
	ClearAllPlates();
	//pMarkedPlate = NULL;
	OptCombiPlot = -1;
	SetTitle("Started Non-parallel");
		
	pMainView->STOP = FALSE;
	//OnShow();
	
	if (LoadSelected > 0) {
		res = AfxMessageBox("Keep Surfaces selection as is?",3);
		
		//if (res == IDYES) ClearAllPlates();
		if (res == IDNO) OnOptionsSurfacesEnabledisableall(); // SelectAllPlates()
		else if (res == IDCANCEL) return;
	 
	}
	SuspendedSpan = 0;
	int finished = EmitBeam();

 	//if (finished == 1) s.Format(" FINISHED    "); //Progress = 100;
	if (finished == 0) { s.Format(" Beam STOPPED  "); AfxMessageBox(s, MB_ICONEXCLAMATION | MB_OK); }//Progress = 50;
	STOP = TRUE;
	Progress = 100;
	SetLoadArray(NULL, TRUE);
	if (finished == 0) DetachLoadFiles();
	
	ShowStatus();
	Progress = 0;
	if (Sorted_Load_Ind.GetSize() > 1) // sorted already
		SortLoadArrayAlongX();//unsort
	//pMainView->InvalidateRect(NULL, FALSE);
	
	OnShow();
}

void CBTRDoc::OnViewFullarea() 
{
	AreaLong = AreaLongMax;
	OnShow();
}

void CBTRDoc::OnViewNeutraliser() 
{
	AreaLong = NeutrOutX + 0.5;
	OnShow();
}

void CBTRDoc::OnViewRid() 
{
	AreaLong = RIDOutX + 0.5;
	OnShow();
	
}
void CBTRDoc::OnTasksOther() 
{
	Progress = 0;
	CurrentDirName = TopDirName;
	::SetCurrentDirectory(CurrentDirName);
	SetTitle(CurrentDirName);
	
	TaskRID = FALSE;
	TaskReionization = FALSE;
	
	if (LoadSelected > 0) SelectAllPlates(); // empty the plates list
	InitTaskTransport();
	//SetPlates();
	TaskName = "New task"; //Beam Transport";
	TaskComment = "To be defined"; 
	OnShow();
}

void CBTRDoc::OnUpdateTasksOther(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(FALSE);
	pCmdUI->SetCheck(!TaskRID && !TaskReionization && !TaskRID);
	pCmdUI->SetText("Transport");
}

void CBTRDoc::OnViewFitonoff() 
{
	pMainView->OnViewFitonoff();
	
}

void CBTRDoc::OnUpdateViewFitonoff(CCmdUI* pCmdUI) 
{
	pMainView->OnUpdateViewFitonoff(pCmdUI);
}

void CBTRDoc::OnViewBeam() 
{
	pMainView->OnViewBeam();
}

void CBTRDoc::OnUpdateViewBeam(CCmdUI* pCmdUI) 
{
	pMainView->OnUpdateViewBeam(pCmdUI);
	
}

void CBTRDoc::OnViewFields() 
{
	pMainView->OnViewFields(); 
	
}

void CBTRDoc::OnUpdateViewFields(CCmdUI* pCmdUI) 
{
	pMainView->OnUpdateViewFields(pCmdUI);
	
}

void CBTRDoc::OnUpdateViewFullarea(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(!Progress); 
	pCmdUI->SetCheck(AreaLong == AreaLongMax);
	
}

void CBTRDoc::OnUpdateViewNeutraliser(CCmdUI* pCmdUI) 
{
	//pCmdUI->Enable(!Progress);
	pCmdUI->Enable(!OptFree);
	//pCmdUI->SetCheck(AreaLong == NeutrOutX + 0.5);
}

void CBTRDoc::OnViewNumbering() 
{
	pMainView->OnViewNumbering();
	
}

void CBTRDoc::OnUpdateViewNumbering(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(!OptFree);
	pMainView->OnUpdateViewNumbering(pCmdUI); 
}

void CBTRDoc::OnUpdateViewRid(CCmdUI* pCmdUI) 
{
	//pCmdUI->Enable(!Progress); 
	pCmdUI->Enable(!OptFree);
	//pCmdUI->SetCheck(AreaLong == RIDOutX + 0.5);
}

void CBTRDoc::OnEditComments() 
{
	CCommentsDlg dlg;
	dlg.m_Comment = TaskComment;
	if (dlg.DoModal() == IDOK) {
		TaskComment = dlg.m_Comment;
	}
	OnShow(); //OnDataActive();
}

void CBTRDoc::OnEditTitle() 
{
	CTaskDlg dlg;
	dlg.m_Task = TaskName;
	if (dlg.DoModal() == IDOK) {
		TaskName = ' ' + dlg.m_Task;
	}
	//else TaskName = "No tytle";
	OnShow();//OnDataActive();
}

void CBTRDoc::OnFileNew() 
{
	InitAddPlates();
//	OnTasksOther();

}

void CBTRDoc::OnFileOpen() 
{
	//OnDataGet(); // finally calls BTRDoc::ReadData()

	char name[1024];
	char buf[1024];
//	CFileDialog dlg(TRUE, "dat;txt", "Configuration file");
	CFileDialog dlg(TRUE, "dat;txt | * ", "", OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"Text file (*.dat);(*.txt) | *.dat; *.DAT; *.txt; *.TXT | All Files (*.*) | *.*||", NULL);
	if (dlg.DoModal() == IDOK) {
		strcpy(name, dlg.GetPathName());

		//::GetCurrentDirectory(1024, CurrentDirName);
	
	/*CPreviewDlg dlg;
		dlg.m_Filename = name;
		if (dlg.DoModal() == IDCANCEL) return 0;*/

		FILE * fin;
		fin = fopen(name, "r");
		m_Text.Empty();// = "";
		while (!feof(fin)) {
			fgets(buf, 1024, fin);
			if (!feof(fin))	m_Text += buf;
		}
		fclose(fin);

		SetTitle(name);
		pDataView->m_rich.SetFont(&pDataView->font, TRUE);
		pDataView->m_rich.SetBackgroundColor(FALSE, RGB(250,230,180));
		pDataView->m_rich.SetWindowText(m_Text);
		pDataView->m_rich.SetModify(FALSE);
		
	}
}

void CBTRDoc::OnFileSave() 
{
	//OnDataSave();
	CString S;
	CString text;
	pDataView->m_rich.GetWindowText(S);
	int pos = S.Find("****\n");
	if (pos >= 0) text = S.Mid(pos + 5);
	else text = S;

	CFileDialog dlg(FALSE, "dat;txt | * ", "", OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"Text file (*.dat); (*.txt) | *.dat; *.DAT; *.txt; *.TXT | All Files (*.*) | *.*||", NULL);
		if (dlg.DoModal() == IDOK) {
			FILE * fout;
			char name[1024];
			strcpy(name, dlg.GetPathName());
			fout = fopen(name, "w");
			fprintf(fout, text);
			fclose(fout);
		}
		OnShow(); //OnDataActive();
}


void CBTRDoc::OnFileSaveAs() 
{
	OnFileSave();
}



void CBTRDoc::OnViewParticles() 
{
	OptDrawPart = !OptDrawPart;
	//if (!OptDrawPart) 
	pMainView->InvalidateRect(NULL, TRUE); 
	//OnShow();
}

void CBTRDoc::OnUpdateViewParticles(CCmdUI* pCmdUI) 
{
//	if (!OptDrawPart) pCmdUI->(SetText("Show Particles on the screen");
//	else pCmdUI->SetText("Hide Particles");
	pCmdUI->SetCheck(OptDrawPart);
}

void CBTRDoc::OnEditGas() 
{
}

void CBTRDoc::OnGasProf_X()
{
	char s[1024];
	CGasDlg dlg;
	dlg.m_Head1 = "X,m"; 
	dlg.m_Head2 = "Density, 1/m3"; 
	dlg.m_Filename = GasFileName;
	dlg.m_X.Empty();
	dlg.m_Y.Empty();
	dlg.Arr = &(GField->GasArray);
	dlg.m_Caption = "Gas Profile along X";

	double x, p, xmax = 0, pmax = 0, xmin = 1000;
	int i;
	for (i = 0; i <= GField->GasArray.GetUpperBound(); i++) {
		x = GField->GasArray[i].X;
		p = GField->GasArray[i].Y;
		sprintf(s,"%f\r\n",x); dlg.m_X += s;
		sprintf(s,"%le\r\n",p); dlg.m_Y += s;
	}

	if(dlg.DoModal()==IDCANCEL) return;

	for (i = 0; i <= dlg.Arr->GetUpperBound(); i++) {
		x = dlg.Arr->GetAt(i).X;
		p = dlg.Arr->GetAt(i).Y;
		xmax = Max(xmax, x);
		pmax = Max(pmax, p);
		xmin = Min(xmin, x);
	}

 	GField->Pmax = pmax;
	GField->Xmax = xmax;
	GField->Xmin = xmin;
	GField->Nx = dlg.Arr->GetSize();

	GasFileName = dlg.m_Filename;
	PressureLoaded = TRUE;
	PressureDim = 1;
	GasCoeff = 1;
	OptReionAccount = TRUE;//automatic when gas loaded
	//OptThickNeutralization = TRUE; // not automatic 
	SetReionPercent();
	SetNeutrCurrents();

	OnShow();
}

void CBTRDoc::OnGasProf_XZ()
{
	CFileDialog *fname_dia;
	CString infile;
	char s[1024];

//	CFileDialog dlg(TRUE, "dat; txt | * ", "", OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
//		"E-field data (*.txt);(*.dat)  | *.txt; *.TXT; *.dat; *.DAT | All Files (*.*) | *.*||", NULL);
	
	fname_dia=new CFileDialog(TRUE, "dat; txt  | * ",	infile,	OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"Gas density file (*.txt);(*.dat) | *.txt; *.TXT; *.dat; *.DAT | All Files (*.*) | *.*||",
		NULL);

	if(fname_dia->DoModal()==IDCANCEL){
		delete[] fname_dia;
		return;
	}
	infile = fname_dia->GetPathName();
	delete[] fname_dia;

	CPreviewDlg dlg;
	dlg.m_Filename = infile;
	if (dlg.DoModal() == IDCANCEL) return;

	int n =0;
	FILE * fp = fopen(infile,"rt");
	if (fp==NULL){
		AfxMessageBox("problem opening data file", MB_ICONSTOP | MB_OK);
		return;
	}
	fclose(fp);

	char name[256];
	strcpy(name, infile);
		
	n = GFieldXZ->ReadData(name);//(fp);

	/*CString S;
	S.Format("read %d data lines\n from file %s", n, name);
	AfxMessageBox(S);*/

	if (n<0)  {
		sprintf(s,"Invalid format of Gas Matrix data", MB_ICONSTOP | MB_OK);
		AfxMessageBox(s,MB_ICONINFORMATION);
		//MF_7 = FALSE;
	}
	else {
		//sprintf(s,"MF data is Read (%d lines)", n);
		//MF_7 = TRUE;
		//OptStrayField = TRUE;
		PressureLoaded = TRUE;
		GasFileName = dlg.m_Filename;
		PressureDim = 2;
		GasCoeff = 1;
		OptReionAccount = TRUE;//automatic when gas loaded
		//OptThickNeutralization = TRUE;
		SetReionPercent();
		SetNeutrCurrents();

	}

	OnShow();
}

void CBTRDoc::OnEditFW2D()
{
	if (!OptBeamInPlasma) return;
	char s[1024];
	CGasDlg dlg;
	dlg.m_Head1 = "R,m"; 
	dlg.m_Head2 = "Z,m"; 
	dlg.m_Filename = FW2DFileName;
	dlg.m_X.Empty();
	dlg.m_Y.Empty();
	dlg.Arr = &FWdata;
	dlg.m_Caption = "First Wall 2D contour";

	double r, z, rmin = 1000, rmax = -1000, zmin = 1000, zmax = -1000;
	int i;
	for (i = 0; i <= FWdata.GetUpperBound(); i++) {
		r = FWdata[i].X;
		z = FWdata[i].Y;
		sprintf(s,"%f\r\n",r); dlg.m_X += s;
		sprintf(s,"%le\r\n",z); dlg.m_Y += s;
		
	}
		

	if(dlg.DoModal()==IDCANCEL) return;

	for (i = 0; i <= FWdata.GetUpperBound(); i++) {
		r = FWdata[i].X; // data cm->m 
		z = FWdata[i].Y; // data cm->m
		//FWdata[i].X = r;
		//FWdata[i].Y = z;
	
		rmax = Max(rmax, r);
		rmin = Min(rmin, r);
		zmax = Max(zmax, z);
		zmin = Min(zmin, z);
	}

	FWRmin = rmin;
	FWRmax = rmax;
	FWZmin = zmin;
	FWZmax = zmax;

	FW2DFileName = dlg.m_Filename;
	FWdataLoaded = TRUE;
	
	//SetPlates();
	//SetPlasmaGeom(); // called from SetPlatesNBI
	//SetDecayInPlasma();
	OnShow();
}


void CBTRDoc::OnEditMagfield() 
{
}

void CBTRDoc::OnEditMagfield_7columns() 
{
	CFileDialog *fname_dia;
	CString infile;
	char s[1024];

//	CFileDialog dlg(TRUE, "dat; txt | * ", "", OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
//		"E-field data (*.txt);(*.dat)  | *.txt; *.TXT; *.dat; *.DAT | All Files (*.*) | *.*||", NULL);
	
	fname_dia=new CFileDialog(TRUE, "dat; txt  | * ",	infile,	OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"MF data file (*.txt);(*.dat) | *.txt; *.TXT; *.dat; *.DAT | All Files (*.*) | *.*||",
		NULL);

	if(fname_dia->DoModal()==IDCANCEL){
		delete[] fname_dia;
		return;
	}
	infile = fname_dia->GetPathName();
	delete[] fname_dia;

	CPreviewDlg dlg;
	dlg.m_Filename = infile;
	if (dlg.DoModal() == IDCANCEL) return;

	int n =0;
	FILE * fp=fopen(infile,"rt");
	if (fp==NULL){
		AfxMessageBox("problem opening data file", MB_ICONSTOP | MB_OK);
		return;
	}
	fclose(fp);

	char name[256];
	strcpy(name, infile);
	
	//BField3D->Init(name);
	//return;

	n = BField3D->ReadData(name);//(fp);

	/*CString S;
	S.Format("read %d data lines\n from file %s", n, name);
	AfxMessageBox(S);*/

	if (n<0)  {
		sprintf(s,"Invalid format of Mag. Field data", MB_ICONSTOP | MB_OK);
		AfxMessageBox(s,MB_ICONINFORMATION);
		//MF_7 = FALSE;
	}
	else {
		//sprintf(s,"MF data is Read (%d lines)", n);
		//MF_7 = TRUE;
		//OptStrayField = TRUE;
		MagFieldDim = 3;
		FieldLoaded = TRUE;
		MagFieldFileName = dlg.m_Filename;
	}
	
	//MFcoeff = 1.;
//	pMainView->SHOW_FIELDS = TRUE;
//	pMainView->SHOW_BEAM = FALSE;
	OnShow();
}

void CBTRDoc::OnUpdateEditMagfield_7columns(CCmdUI* /* pCmdUI */) 
{
//	pCmdUI->SetCheck(MF_7);

}

void CBTRDoc::OnEditMagfield_4columns() 
{
	char s[1024];
	CMFDlg dlg;
	if (MagFieldDim == 1) dlg.m_Filename = MagFieldFileName; //"Manually Set MF Data";
	else dlg.m_Filename = "";
	dlg.m_COLUMNS = "  X,m \t  Bx,T \t  By,T \t  Bz,T";
	dlg.m_Bdata.Empty();
	dlg.XArr = &(BField->ArrX);//&MFXdata;
	dlg.BArr = &(BField->ArrB);//&MFdata;
	int i;
	double x, bx, by, bz;
	C3Point P;
	CString S;

	for (i = 0; i < BField->Nx; i++) {
		x = BField->ArrX[i];
		bx = BField->ArrB[i].X; by = BField->ArrB[i].Y; bz = BField->ArrB[i].Z;
		sprintf(s,"%g\t %g\t %g\t %g\r\n ", x,bx,by,bz); 
		dlg.m_Bdata += s;
	}

	if(dlg.DoModal() == IDCANCEL) return;

	double xmax = 0, bmax = 0, xmin = 1000;
	for (i = 0; i <= dlg.XArr->GetUpperBound(); i++) {
		x = dlg.XArr->GetAt(i);
		P = dlg.BArr->GetAt(i);
		xmax = Max(xmax, x);
		bmax = Max(bmax, P.X); bmax = Max(bmax, P.Y); bmax = Max(bmax, P.Z);
		xmin = Min(xmin, x);
	}

 	BField->Bmax = bmax;
	BField->Xmax = xmax;
	BField->Xmin = xmin;
	BField->Nx = dlg.XArr->GetSize();
	MagFieldFileName = dlg.m_Filename;
	//MF_7 = FALSE;
	MagFieldDim = 1;
	FieldLoaded = TRUE;
	if (MFcoeff == 0) MFcoeff = 1.;
	
	S.Format("%d", BField->Nx);	//AfxMessageBox(S);
	//if (BTRVersion >= 5.0) AfxMessageBox("V5.0:\n MF is not applied\n -> all Atoms start from GG");
	
	OnShow();
}

void CBTRDoc::OnUpdateEditMagfield_4columns(CCmdUI*) 
{
//	pCmdUI->SetCheck(!MF_7);
}

/*C3Point CBTRDoc:: GetManMF(double x)
 {
	 C3Point P(0,0,0);
	 double x0, x1;
	 C3Point P0, P1;
	 int Nmax = MFXdata.GetUpperBound();
	 double xmax = max(MFXdata[0], MFXdata[Nmax]);
	 double xmin = min(MFXdata[0], MFXdata[Nmax]);
	 if (x > xmax || x < xmin) return P;
	 for (int i = 0; i < Nmax; i++) {
		 if (((x - MFXdata[i]) * (x - MFXdata[i+1])) <= 0) {
			  x0 = MFXdata[i];  x1 = MFXdata[i+1];
			  P0 = MFdata[i];  P1 = MFdata[i+1];
			  P = P0 + (P1-P0) * (x-x0) / (x1-x0);
			  return P;
		 } //if
	 } // for
	 return P;
 }*/

C3Point CBTRDoc:: GetFW_Z(double x, double y, int nz) //  called by ShowTor
// if nz > 0 -> Z > 0
{
	// double Z = 1000;
	double dx = x - TorCentre.X;
	double dy = y - TorCentre.Y;
	double R = sqrt(dx * dx + dy * dy); // from torus centre
	double z;
	C3Point P(R,1000,1000);
 
/*	if (!FWdataLoaded) // default 
	{
		double dr = fabs(r - PlasmaMajorR);
		if (fabs(dr) > PlasmaMinorR) return P;
		z = sqrt(PlasmaMinorR * PlasmaMinorR - dr*dr);
		if (nz < 0) z = -z;
		P.Y = z + TorCentre.Z; P.Z = z + TorCentre.Z;
		return P;
	}*/

	int Nmax = FWdata.GetUpperBound();
	double  z0, z1, r0, r1;
	for (int i = 0; i < Nmax; i++) { //increase
		r0 = FWdata[i].X; r1 = FWdata[i+1].X;
		if ((r0 - R)*(r1 - R) > 0) continue;
		z0 = FWdata[i].Y; z1 = FWdata[i+1].Y;
		z = z0 + (R - r0) / (r1 - r0) * (z1 - z0);
		if (z * nz < 0) continue;
		P.Y = z + TorCentre.Z;
		break;
		//return (z + TorCentre.Z);
	}
	for (int i = Nmax; i > 0; i--) { //decrease
		r0 = FWdata[i].X; r1 = FWdata[i-1].X;
		if ((r0 - R)*(r1 - R) > 0) continue;
		z0 = FWdata[i].Y; z1 = FWdata[i-1].Y;
		z = z0 + (R - r0) / (r1 - r0) * (z1 - z0);
		if (z * nz < 0) continue;
		P.Z = z + TorCentre.Z;
		break;
		//return (z + TorCentre.Z);
	}

	return P; // 1000 if not found
}
C3Point CBTRDoc:: GetBeamFootLimits(double Xtarget, double & Ymin, double & Ymax, double & Zmin, double & Zmax)
{
	double y0, z0, x, y, z;
	double ymin = 1000, ymax = -1000, zmin = 1000, zmax = -1000; 
	if (!OptSINGAP || !SINGAPLoaded) { // Mamug
	for (int is = 0; is < (int)NofChannelsHor;  is++)  ///NofChannelsHor;
		for (int js = 0; js < (int)NofChannelsVert; js++)   ///NofChannelsVert;
			for (int i = 0; i < (int)NofBeamletsHor; i++) 
				for (int j = 0; j < (int)NofBeamletsVert; j++)
	
		{
			y0 = BeamletPosHor[is * (int)NofBeamletsHor + i]; 
			z0 = BeamletPosVert[js * (int)NofBeamletsVert + j]; 
			x = Xtarget;
			y = y0 + x * tan(BeamletAngleHor[is * (int)NofBeamletsHor + i]);
			z = z0 + x * tan(BeamletAngleVert[js * (int)NofBeamletsVert + j]);
			ymin = Min(ymin, y); ymax = Max(ymax, y);
			zmin = Min(zmin, z); zmax = Max(zmax, z);
		}
				
	}// Mamug
	
	else if (OptSINGAP){ // Singap
		BEAMLET_ENTRY be;
		int N = BeamEntry_Array.GetSize();
		for (int k = 0; k < N; k++) {
			be = BeamEntry_Array.GetAt(k);
			y0 = be.PosY;
			z0 = be.PosZ;
			x = Xtarget;
			y = y0 + x * tan(be.AlfY); 
			z = z0 + x * tan(be.AlfZ); 
			ymin = Min(ymin, y); ymax = Max(ymax, y);
			zmin = Min(zmin, z); zmax = Max(zmax, z);
		}
	} // singap
	Ymin = ymin; Ymax = ymax;
	Zmin = zmin; Zmax = zmax;
	C3Point P = C3Point(Xtarget, 0.5 * (Ymin + Ymax), 0.5 * (Zmin + Zmax));
	return P;
}

void CBTRDoc::OnPlotSingappos() 
{
	int N = BeamEntry_Array.GetSize();
	if (N <1) return; 

	int i;
	double x, y;
	CArray<double, double> PosY;
	CArray<double, double> PosZ;
	BEAMLET_ENTRY be;
	PLOTINFO SingapPlot;
	SingapPlot.Caption = "SINGAP beamlets Positions,  X = 0";
	SingapPlot.LabelX = "X, m";
	SingapPlot.LabelY = "Y, m";
	SingapPlot.LimitX = 0;//0.3;
	SingapPlot.LimitY = 0;//0.8;
	SingapPlot.CrossX = 0.1;
	SingapPlot.CrossY = 0.2;
	SingapPlot.Line = 0;
	SingapPlot.Interp = 1;
//	SingapPlot.N1 = BeamEntry_Array.GetSize(); //Singap_Array.GetSize();
//	SingapPlot.N2 = SingapPlot.N1;
	SingapPlot.PosNegX = TRUE;
	SingapPlot.PosNegY = TRUE;
//	SingapPlot.DataX = &PolarAngle;
//	SingapPlot.DataY = &PolarCurrent;
	for (i = 0; i < BeamEntry_Array.GetSize(); i++) {
		be = BeamEntry_Array.GetAt(i);
		if (be.Active == FALSE) continue;
		x = be.PosY;
		y = be.PosZ;
		PosY.Add(x);
		PosZ.Add(y);
	}
	SingapPlot.N1 = PosY.GetSize();

	int j =0;
	if (OptSINGAP)
		for (i = 0; i < NofBeamlets, j < NofCalculated; i++) {
			be = BeamEntry_Array.GetAt(i);
			if (be.Active == FALSE) continue;
			x = be.PosY;
			y = be.PosZ;
			PosY.Add(x);
			PosZ.Add(y);
			j++;
			
		} //i < NofBeamlets

	SingapPlot.N = PosY.GetSize();
	SingapPlot.N2 = SingapPlot.N1;
	

	SingapPlot.DataX = &PosY;
	SingapPlot.DataY = &PosZ;
	
	CPlotDlg dlg;
	dlg.Plot = &SingapPlot;
	dlg.InitPlot();
	dlg.DoModal();

	PosY.RemoveAll();
	PosZ.RemoveAll();
}

void CBTRDoc::OnUpdatePlotSingappos(CCmdUI* pCmdUI) 
{
	//pCmdUI->Enable(SINGAPLoaded);
	pCmdUI->SetCheck(OptSINGAP);
}

void CBTRDoc::OnPlotGasprofile() 
{
	CArray<double, double>  ArrX;
	CArray<double, double>  ArrY;
	double Xmax;
	if (PressureDim == 1) Xmax = GField->Xmax;
	else Xmax = GFieldXZ->Xmax;

	PLOTINFO GasPlot;
	GasPlot.Caption = "Gas Density Distribution along NBL";
	GasPlot.LabelX = "X, m";
	GasPlot.LabelY = "Gas Density [1/m3], NL [1/m2]";
	if (OptAddPressure) GasPlot.LabelY = "Basic Profile [1/m3], NL [1/m2], Additional Profile";
	GasPlot.LimitX = Xmax;//ReionXmax;
	GasPlot.LimitY = 0;//1.e20;
	GasPlot.CrossX = 1;
	GasPlot.CrossY = 1.e19;
	GasPlot.Line = 1;
	GasPlot.Interp = 1;
	//GasPlot.N = 100; //GField->GasArray.GetSize();
	//GasPlot.N1 = GasPlot.N +1;
	//GasPlot.N2 = GasPlot.N +1;
	GasPlot.PosNegY = FALSE;
	GasPlot.PosNegX = FALSE;
	double x, y, xmin = 0;
	double xmax = ReionXmax;
	double dx = NeutrStepL; 
	x = xmin;
	while (x <= xmax) {
		//x = GField->GasArray[i].X;
		y = GetPressure(C3Point(x, 0, 0)); //GField->GasArray[i].Y;
		ArrX.Add(x); ArrY.Add(y);
		x += dx;
	}
	GasPlot.N1 = ArrX.GetSize();

	x = xmin;
	double sumNL = 0;
	while (x <= xmax) {
		//x = GField->GasArray[i].X;
		sumNL += dx * GetPressure(C3Point(x, 0, 0));// NL
		y = sumNL; 
		ArrX.Add(x); ArrY.Add(y);
		x += dx;
	}
	
if (OptAddPressure) {
	GasPlot.N2 = ArrX.GetSize();

	x = xmin;
	while (x <= xmax) {
		//x = GField->GasArray[i].X;
		y = AddGField->GetPressure(x); //GField->GasArray[i].Y;
		ArrX.Add(x); ArrY.Add(y);
		x += dx;
	}
} // optaddpressure
	else GasPlot.N2 = ArrX.GetSize() +1;

	GasPlot.N = ArrX.GetSize();
	
	GasPlot.DataX = &ArrX;
	GasPlot.DataY = &ArrY;
	
	CPlotDlg dlg;
	dlg.Plot = &GasPlot;
	dlg.InitPlot();
	dlg.DoModal();

	ArrX.RemoveAll();
	ArrY.RemoveAll();
	
}

void CBTRDoc::OnUpdatePlotGasprofile(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(PressureLoaded);
	
}

void CBTRDoc::OnPlotMagneticfield() 
{
	CArray<double, double>  ArrX;
	CArray<double, double>  ArrY;
	double Xmax;

	PLOTINFO MFPlot;
	MFPlot.Caption = "Magnetic Field Distribution along NBL (Scaling = 1)";
	MFPlot.LabelX = "X, m";
	MFPlot.LabelY = "Bx, By, Bz (Tesla)";
	//MFPlot.LabelY = "Bz(z=-0.7), Bz(z=0), Bz(z=0.7) Tesla";
	if (MagFieldDim == 1) Xmax = BField->Xmax;
	else Xmax = BField3D->Xmax;
	MFPlot.LimitX = 0;//Xmax;//AreaLong;
	MFPlot.LimitY = 0; //5;
	MFPlot.CrossX = 0;
	MFPlot.CrossY = 1;
	MFPlot.Line = 1;
	MFPlot.Interp = 1;	
	MFPlot.PosNegY = TRUE;
	MFPlot.PosNegX = FALSE;
	
	int Nmax;
	if (MagFieldDim == 1) Nmax = BField->Nx; // 4-col
	else Nmax = BField3D->Nx; 

	MFPlot.N = 3 * Nmax;
	MFPlot.N1 = Nmax;
	MFPlot.N2 = 2 * Nmax; 

	double x, y, z;
	int i, j, k;
		
	for (i = 0; i < Nmax; i++) {
		if (MagFieldDim == 1) { // 4 col
			x = BField->ArrX[i];
			y = BField->ArrB[i].X;
		} // 4 col
		else {// 6-col
			x = BField3D->ArrX[i]; //i * BField3D->StepX;
			y = BField3D->GetB(x, 0, 0).X;//ValB[i][j][k].X;
			//y = BField3D->GetB(x, 0, -0.7).Z;
		}
		ArrX.Add(x); ArrY.Add(y);
	}
	for (i = 0; i < Nmax; i++) {
		if (MagFieldDim == 1) { // 4 col
			x = BField->ArrX[i];
			y = BField->ArrB[i].Y;
		} // 4 col
		else {// 6-col
			x =  BField3D->ArrX[i]; //i * BField3D->StepX;
			y = BField3D->GetB(x, 0, 0).Y;//BField3D->ValB[i][j][k].Y;
			//y = BField3D->GetB(x, 0, 0).Z;
		}
		ArrX.Add(x); ArrY.Add(y);
	}
	for (i = 0; i < Nmax; i++) {
		if (MagFieldDim == 1) { // 4 col
			x = BField->ArrX[i];
			y = BField->ArrB[i].Z;
		} // 4 col
		else {// 6-col
			x = BField3D->ArrX[i]; //i * BField3D->StepX;
			y = BField3D->GetB(x, 0, 0).Z; //BField3D->ValB[i][j][k].Z; 
			
		}
		ArrX.Add(x); ArrY.Add(y);
	}

	MFPlot.DataX = &ArrX;
	MFPlot.DataY = &ArrY;
	
	CPlotDlg dlg;
	dlg.Plot = &MFPlot;
	dlg.InitPlot();
	dlg.DoModal();

	ArrX.RemoveAll();
	ArrY.RemoveAll();
	
}

void CBTRDoc::OnUpdatePlotMagneticfield(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(FieldLoaded);
		
}

void CBTRDoc::OnPlotBeamletcurr() 
{
	/*int res;
	if (pDataView->m_rich.GetModify()) {
		res = AfxMessageBox("Data is modified but NOT UPDATED! \n Continue?", 3);
		if (res != IDYES) return;
	}*/
	PlotBeamletCurrent();
}

void CBTRDoc:: PlotBeamletCurrent()
{
	CWaitCursor wait;
	double angle, y;
	CArray<double, double> PosX;
	CArray<double, double> PosY;
	PLOTINFO DecilPlot;
	CString S;

	if ((int)BeamSplitType ==0) { 
		DecilPlot.Caption = "BML polar prof ";
//		S.Format(" (Polar = %d)", (int)PolarNumber);
		S.Format(" Core %g mrad, Halo %g mrad, Halo = %g",
			BeamCoreDivY*1000, BeamHaloDiverg*1000, BeamHaloPart);
		DecilPlot.Caption += S;
		DecilPlot.LabelX = "mrad";
		DecilPlot.LabelY = "Splitting, Polar Profile";
		DecilPlot.Line = 1;
		DecilPlot.Interp = 0; // stairs
		
	}
	else {
		DecilPlot.Caption = "Beamlet Current vs Horizontal Direction ";
		S.Format(" (Core DivY = %g mrad, Halo Div = %g mrad, Halo Fraction = %g)", 
			BeamCoreDivY*1000, BeamHaloDiverg*1000, BeamHaloPart);
		DecilPlot.Caption += S;
		DecilPlot.LabelX = "mrad";
		DecilPlot.LabelY = "Core+Halo, Core, Halo";
		DecilPlot.Line = 1;
		DecilPlot.Interp = 1; //
		
	}
	DecilPlot.LimitX = 90;//0
	DecilPlot.LimitY = 1;
	DecilPlot.CrossX = 20;
	DecilPlot.CrossY = 0.2;
	
	DecilPlot.PosNegX = FALSE;
	DecilPlot.PosNegY = FALSE;

	double divY = BeamCoreDivY;
	if (fabs(divY) < 1.e-16) divY = 0.00001;
	double divZ = BeamCoreDivZ;
	if (fabs(divZ) < 1.e-16) divZ = 0.00001;

	if ((int)BeamSplitType == 0) SetPolarBeamletCommon(); //SetBeamletAt(0);
	else SetGaussBeamlet(BeamCoreDivY, BeamCoreDivZ);

	if ((int)BeamSplitType ==0) { //  Polar Splitting --------------------------
		PosX.Add(PolarAngle[0] * divY);
		PosY.Add(PolarCurrent[0]);
		for (int i = 1; i <= PolarAngle.GetUpperBound(); i++) {
			PosX.Add(0.5 * 1000 *(PolarAngle[i-1] + PolarAngle[i])* divY);
			PosY.Add(PolarCurrent[i]);
		}
		DecilPlot.N1 = PosX.GetSize();

		angle = 0;
		double amax = SumGauss(0.99999, BeamCoreDivY, BeamHaloDiverg, BeamHaloPart);
		while (angle < amax) {
			PosX.Add(angle * 1000);
		//	y = GaussDensity(angle, BeamCoreDiverg, BeamHaloDiverg, BeamHaloPart);
			y = 1. - Gauss(angle, BeamCoreDivY, BeamHaloDiverg, BeamHaloPart);
			PosY.Add(y);
			angle += 0.00001;
		}
	
		DecilPlot.N = PosX.GetSize();
		DecilPlot.N2 = DecilPlot.N +1;
	}//  Polar Splitting 

	else { // Cartesian splitting ---------------------------------------
//	int i;
	double x, core0, halo0, core, halo;
	// int Ny = (int) BeamSplitNumberY;
//	double DivY = BeamCoreDivY;
	double dx = 0.0001; //2 * 3 * Max(BeamHaloDiverg, DivY) / Ny;
	double MaxVal = 1;
	
	x = 0;
	core0 = 0; halo0 = 0; core = 0; halo = 0; 
	if (BeamCoreDivY > 1.e-16)	core =  IntegralErr(x+0.5*dx, divY) * (1 - BeamHaloPart);
	if (BeamHaloDiverg > 1.e-16) halo = IntegralErr(x+0.5*dx, BeamHaloDiverg) * (BeamHaloPart);
	MaxVal = 2*(core+halo);
			PosY.Add(2*(core+halo)/MaxVal);
			PosX.Add(0); //(x - 0.5*dx);
			core0 = core; halo0 = halo;
			x+=dx;
		//for (i = 0; i < Ny/2; i++) {
	while (x <= 2 * Max(divY, BeamHaloDiverg)) { //DecilPlot.LimitX) 
			if (BeamCoreDivY > 1.e-16) core =  IntegralErr(x+0.5*dx, divY) * (1-BeamHaloPart);
			if (BeamHaloDiverg > 1.e-16) halo =  IntegralErr(x+0.5*dx, BeamHaloDiverg) * (BeamHaloPart);
			PosY.Add((core - core0 + halo - halo0)/MaxVal);
			PosX.Add(1000*x);//(x - 0.5*dx);
			core0 = core; halo0 = halo;
			x+=dx;
		} // 
	DecilPlot.N1 = PosX.GetSize();


	core0 = 0; x = 0; core = 0;
	if (BeamCoreDivY > 1.e-16)	core = IntegralErr(x+0.5*dx, divY) * (1 - BeamHaloPart);
			PosY.Add(2*core/MaxVal);
			PosX.Add(0); //(x - 0.5*dx);
			core0 = core;
			x+=dx;
		//for (i = 0; i < Ny/2; i++) {
	while (x <= 2 * Max(divY, BeamHaloDiverg)) { //DecilPlot.LimitX) 
			if (BeamCoreDivY > 1.e-16)	core =  IntegralErr(x+0.5*dx, divY) * (1-BeamHaloPart);
			PosY.Add((core - core0)/MaxVal);
			PosX.Add(1000*x);//(x - 0.5*dx);
			core0 = core;
			x+=dx;
		} // 
	DecilPlot.N2 = PosX.GetSize();

	halo0 = 0; x = 0; halo = 0;
	if (BeamHaloDiverg > 1.e-16) halo = IntegralErr(x+0.5*dx, BeamHaloDiverg) * (BeamHaloPart);
			PosY.Add(2*halo/MaxVal);
			PosX.Add(0); //(x - 0.5*dx);
			halo0 = halo;
			x+=dx;
		//for (i = 0; i < Ny/2; i++) {
	while (x <= 2 * Max(divY, BeamHaloDiverg)) { // DecilPlot.LimitX) 
			if (BeamHaloDiverg > 1.e-16) halo =  IntegralErr(x+0.5*dx, BeamHaloDiverg) * (BeamHaloPart);
			PosY.Add((halo - halo0)/MaxVal);
			PosX.Add(1000*x); //(x - 0.5*dx);
			halo0 = halo;
			x+=dx;
		} // 
	DecilPlot.N = PosX.GetSize();



	} // cartesian

	DecilPlot.DataX = &PosX; //PolarAngle;
	DecilPlot.DataY = &PosY; //PolarCurrent;

	CPlotDlg dlg;
	dlg.Plot = &DecilPlot;
	dlg.InitPlot();
	dlg.DoModal();
	PosX.RemoveAll();
	PosY.RemoveAll();

}

void CBTRDoc::OnPlotMamugpositions() 
{
//	int is, js, i, j;
	int res;
	if (pDataView->m_rich.GetModify()) {
		res = AfxMessageBox("Data is modified but NOT UPDATED! \n Continue?", 3);
		if (res != IDYES) return;
	}
	double x, y;
	CArray<double, double> PosY;
	CArray<double, double> PosZ;
	
	PLOTINFO MamugPlot;
	MamugPlot.Caption = "MAMuG beamlets Positions,  X = 0";
	MamugPlot.LabelX = "X, m";
	MamugPlot.LabelY = "Y, m";
	MamugPlot.LimitX = 0;//0.3;
	MamugPlot.LimitY = 0; //0.8;
	MamugPlot.CrossX = 0.1;
	MamugPlot.CrossY = 0.2;
	MamugPlot.Line = 0;
	MamugPlot.Interp = 1;
//	MamugPlot.N1 = NofChannelsHorNofChannelsVertNofBeamletsHorNofBeamletsVert;
//	MamugPlot.N2 = MamugPlot.N1; 
	MamugPlot.PosNegX = TRUE;
	MamugPlot.PosNegY = TRUE;
//	MamugPlot.DataX = &PolarAngle;
//	MamugPlot.DataY = &PolarCurrent;
	for (int is = 0; is < (int)NofChannelsHor;  is++)  ///NofChannelsHor;
		for (int js = 0; js < (int)NofChannelsVert; js++)   ///NofChannelsVert;
			for (int i = 0; i < (int)NofBeamletsHor; i++) 
				for (int j = 0; j < (int)NofBeamletsVert; j++)
	
	{
		if (ActiveCh[is] == FALSE || ActiveRow[js] == FALSE) continue;
		x = BeamletPosHor[is * (int)NofBeamletsHor + i]; 
		y = BeamletPosVert[js * (int)NofBeamletsVert + j]; 
		PosY.Add(x);
		PosZ.Add(y);
	}
	MamugPlot.N1 = PosY.GetSize();

int j = 0;
if (!OptSINGAP) 
	for (int k = 0; k < NofBeamlets, j < NofCalculated; k++) {
		x = PosY.GetAt(k);
		y = PosZ.GetAt(k);
		PosY.Add(x);
		PosZ.Add(y);
		j++;
	}

	MamugPlot.N = PosY.GetSize();
	MamugPlot.N2 = MamugPlot.N1; 

	MamugPlot.DataX = &PosY;
	MamugPlot.DataY = &PosZ;
	
	CPlotDlg dlg;
	dlg.Plot = &MamugPlot;
	dlg.InitPlot();
	dlg.DoModal();

	PosY.RemoveAll();
	PosZ.RemoveAll();
	
}

void CBTRDoc::OnUpdatePlotMamugpositions(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(!OptSINGAP);
}

void CBTRDoc::OnPlot3dload() 
{
	// temporary removed
//	pMarkedPlate = pBeamHorPlane; //pMarkedPlate;
	
	ShowProfiles = TRUE;
	OnPlotMaxprofiles();
	OnPlotLoadmap();
	return;

	if (OptCombiPlot == -1) return; //pMarkedPlate == NULL) 
	if (!(pMarkedPlate->Loaded)) return;
	
	ShowProfiles = FALSE;
	pSetView->InvalidateRect(NULL, TRUE);
}

void CBTRDoc::OnUpdatePlot3dload(CCmdUI* pCmdUI) 
{
	//pCmdUI->Enable(OptCombiPlot != -1 && pMarkedPlate->Loaded);
	
}

void CBTRDoc::OnPlotLoadmap() 
{
	if (pMarkedPlate == NULL) return;
	if (!(pMarkedPlate->Loaded)) return;
	
	CPlate * plate = pMarkedPlate;
	ShowProfiles = TRUE;
	int Sdegree = plate->SmoothDegree;
	CLoad * OldLoad = plate->Load;
	CLoad * NewLoad = OldLoad->Smoothed(Sdegree);
	pMarkedPlate->Load = NewLoad;

	pLoadView->SetLoad_Plate(NewLoad, pMarkedPlate);
	pLoadView->Contours = FALSE;
	pLoadView->Cross.X = NewLoad->iProf * NewLoad->StepX;
	pLoadView->Cross.Y = NewLoad->jProf * NewLoad->StepY;
	pMarkedPlate->ShowLoad();// pLoadView->SetPlate(pMarkedPlate);

	pSetView->SetLoad_Plate(NewLoad, pMarkedPlate);
	pSetView->ShowProfiles();//->InvalidateRect(NULL, TRUE);
	
	pMarkedPlate->Load = OldLoad;
	delete (NewLoad);// created in Smoothed()
	//plate->SmoothDegree = 0;
	return;

/////////////////NOT CALLED //////////////////////////
	//if (OptCombiPlot == -1) return;//(pMarkedPlate == NULL) return;
	if (!(pMarkedPlate->Loaded)) {
		AfxMessageBox("no load"); return;
	}
	//if (!(pMarkedPlate->Selected)) return;
	//pMarkedPlate->Load->SetSumMax(); // max profiles
	pLoadView->Contours = FALSE;
	pLoadView->Cross.X = pMarkedPlate->Load->iProf * pMarkedPlate->Load->StepX;
	pLoadView->Cross.Y = pMarkedPlate->Load->jProf * pMarkedPlate->Load->StepY;
	//pMarkedPlate->ShowLoadState();
	pMarkedPlate->ShowLoad();// pLoadView->SetPlate(pMarkedPlate);
	
}

void CBTRDoc::OnUpdatePlotLoadmap(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(OptCombiPlot != -1 && pMarkedPlate->Loaded);
}

void CBTRDoc::OnPlotContours() 
{
	//if (InvUser) return;
	if (OptCombiPlot == -1) return;//;(pMarkedPlate == NULL) return;
	//if (!(pMarkedPlate->Selected)) return;
	if (!(pMarkedPlate->Loaded)) return;
	//pMarkedPlate->Load->SetSumMax(); // max profiles
	pLoadView->Contours = TRUE;
	pLoadView->Cross.X = pMarkedPlate->Load->iProf * pMarkedPlate->Load->StepX;
	pLoadView->Cross.Y = pMarkedPlate->Load->jProf * pMarkedPlate->Load->StepY;
	//pMarkedPlate->ShowLoadState();
	CLevelsDlg dlg;
	dlg.m_Min = 0;
	dlg.m_Max = pMarkedPlate->Load->MaxVal * 1.e-6; // -> MW/m2
	dlg.m_Step = Min(LevelStep, dlg.m_Max * 0.1);
	dlg.m_Write = LevelWrite;
	if (dlg.DoModal() == IDOK) {
		LevelMin = dlg.m_Min;
		LevelMax = dlg.m_Max;
		LevelStep = dlg.m_Step;
		LevelWrite = dlg.m_Write;
		pMarkedPlate->ShowLoad();
	}
	
}

void CBTRDoc::OnUpdatePlotContours(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(OptCombiPlot != -1 && pMarkedPlate->Loaded);
	
}


void CBTRDoc::OnPlotMaxprofiles() 
{
	//if (OptCombiPlot == -1) return;//(pMarkedPlate == NULL) return;
	
	if (pMarkedPlate == NULL) return;
	if (!(pMarkedPlate->Loaded)) return;
	
	CPlate * plate = pMarkedPlate;
	ShowProfiles = TRUE;
	int Sdegree = plate->SmoothDegree;
	CLoad * OldLoad = plate->Load;
	CLoad * NewLoad = OldLoad->Smoothed(Sdegree);
	pMarkedPlate->Load = NewLoad;

	pLoadView->Cross.X = NewLoad->iProf * NewLoad->StepX;
	pLoadView->Cross.Y = NewLoad->jProf * NewLoad->StepY;

	pSetView->SetLoad_Plate(NewLoad, pMarkedPlate);//(pMarkedPlate->Load, pMarkedPlate);
	pSetView->ShowProfiles();
	//pSetView->InvalidateRect(NULL, TRUE);

	pMarkedPlate->Load = OldLoad;
	delete (NewLoad);// created in Smoothed()
	//plate->SmoothDegree = 0;
	
}

void CBTRDoc::OnUpdatePlotMaxprofiles(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(OptCombiPlot != -1 && pMarkedPlate->Loaded);
	
}

void CBTRDoc:: GetCombiPlotLimits(C3Point & lb, C3Point & rt)
{
	double Xmin = 100, Xmax = 0;//AreaLong;
	double Ymin = 100, Ymax = 0;//AreaHorMax;
	double Zmin = 100, Zmax = 0;//AreaVertMax;
	CPlate * plate;
	POSITION pos = PlatesList.GetHeadPosition();
		while (pos != NULL) {
			plate = PlatesList.GetNext(pos);
			if (plate->Selected) {
				for (int k = 0; k < 4; k++) {
					Xmin = Min(Xmin, plate->Corn[k].X); 
					Xmax = Max(Xmax, plate->Corn[k].X); 
					Ymin = Min(Ymin, plate->Corn[k].Y); 
					Ymax = Max(Ymax, plate->Corn[k].Y); 
					Zmin = Min(Zmin, plate->Corn[k].Z); 
					Zmax = Max(Zmax, plate->Corn[k].Z); 
				}// k
			}//if selected
		}//pos

	lb.X = Xmin; lb.Y = Ymin; lb.Z = Zmin;
	rt.X = Xmax; rt.Y = Ymax; rt.Z = Zmax;
	return;
}
void CBTRDoc:: GetCombiPlotOrts(C3Point & OrtX, C3Point & OrtY)
{
	CPlate * plate;
	//BOOL Target = TRUE;
	POSITION pos = PlatesList.GetHeadPosition();
		while (pos != NULL) {
			plate = PlatesList.GetNext(pos);
			if (plate->Selected) {
				OrtX = plate->OrtX;
				OrtY = plate->OrtY;
				break;
			}//if selected
		}//pos
		
	return;
}

void CBTRDoc:: CreateBaseSingle() // creates a new plate - pMarkedPlate projection
{
//	if (OptCombiPlot < 1) return;
	C3Point P, Ploc, OrtX, OrtY, OrtZ, Orig, p0, p1, p2, p3;
	CPlate * plate = new CPlate();

	OrtZ = pMarkedPlate->OrtZ;// / (double)k;
	Orig = pMarkedPlate->Orig;
	int DirZ = 0;
	if (fabs(OrtZ.X) > fabs(OrtZ.Y) && fabs(OrtZ.X) > fabs(OrtZ.Z)) DirZ = 1;
	if (fabs(OrtZ.Y) > fabs(OrtZ.X) && fabs(OrtZ.Y) > fabs(OrtZ.Z)) DirZ = 2;
	if (fabs(OrtZ.Z) > fabs(OrtZ.X) && fabs(OrtZ.Z) > fabs(OrtZ.Y)) DirZ = 3;
	
	switch (DirZ) {
		case 1: OrtX = C3Point(0,1,0); OrtY = C3Point(0,0,1); OrtZ = C3Point(1,0,0); break;
		case 2: OrtX = C3Point(1,0,0); OrtY = C3Point(0,0,1); OrtZ = C3Point(0,1,0); break;
		case 3: OrtX = C3Point(1,0,0); OrtY = C3Point(0,1,0); OrtZ = C3Point(0,0,1); break;
		default: OrtX = C3Point(0,1,0); OrtY = C3Point(0,0,1); break;
	}
	//OrtZ = VectProd(OrtX, OrtY);

	plate->Orig = Orig;
	plate->OrtX = OrtX; plate->OrtY = OrtY;  plate->OrtZ = OrtZ;
	
	double xmin = 1000, xmax = -1000;//baseplate->Xmax;
	double ymin = 1000, ymax = -1000;//baseplate->Ymax;
	double stepX = 0, stepY = 0;
	
	for (int k = 0; k < 4; k++) {
		P = pMarkedPlate->Corn[k];
		Ploc = plate->GetLocal(P);
		xmin = Min(xmin, Ploc.X);
		xmax = Max(xmax, Ploc.X);
		ymin = Min(ymin, Ploc.Y);
		ymax = Max(ymax, Ploc.Y);
	} //k

		if (pMarkedPlate->Loaded) {
				stepX = Max(stepX, pMarkedPlate->Load->StepX);
				stepY = Max(stepY, pMarkedPlate->Load->StepY);
		}

				
		Ploc = C3Point(xmin, ymin, 0);
		p0 = plate->GetGlobalPoint(Ploc);
		Ploc = C3Point(xmax, ymin, 0);
		p1 = plate->GetGlobalPoint(Ploc);
		Ploc = C3Point(xmin, ymax, 0);
		p2 = plate->GetGlobalPoint(Ploc);
		Ploc = C3Point(xmax, ymax, 0);
		p3 = plate->GetGlobalPoint(Ploc);

	//plate = new CPlate(); // combi-load
	plate->SetLocals(p0, p1, p2, p3);
	plate->Solid = FALSE;
	plate->Visible = FALSE;
	plate->Comment = pMarkedPlate->Comment + " /proj";
	plate->Number = pMarkedPlate->Number;
	 //PlatesList.AddTail(pPlate);
	
	plate->ApplyLoad(TRUE, stepX, stepY); // optimize grid 
	plate->Selected = TRUE;

	pMarkedPlate = plate;

	OptCombiPlot = 0;
	P_CalculateCombi(1); // calculate maps + proect them on the combi-plate
	plate->ShowLoadState();	
	OnPlotMaxprofiles();

	//pMarkedPlate = plate;
	//return plate;

}

void CBTRDoc::OnPlotCombi() // creates a new plate on which selected maps will be projected
//  NOT USED now ----------------------------
{
	//if (InvUser) return;
	OptCombiPlot = 0;//!OptCombiPlot;
	// CLoadView * pLV = (CLoadView *) pLoadView;
	C3Point P, Ploc, OrtX, OrtY, OrtZ, Orig, p0, p1, p2, p3;
	CPlate * plate;
	CPlate * baseplate = new CPlate(); //  combi-load base
	C3Point SumOrtZ = C3Point(0,0,0);
	CString Comment = "No plate selected";
	int k = 0;
	POSITION pos = PlatesList.GetHeadPosition();
		while (pos != NULL) {
			plate = PlatesList.GetNext(pos);
			if (plate->Selected) {
				k++;
				SumOrtZ = SumOrtZ + plate->OrtZ;
				Orig = plate->Orig;
				Comment = plate->Comment;
				//break;
			}//if selected
		}//pos
		OptCombiPlot = k;
		CString S;
		S.Format("%d", k);
		if (k >= 1) {
			Comment = Comment + " - Combi-" +S;
		}
		if (k < 1) { AfxMessageBox("No plate selected"); return; }

	OrtZ = SumOrtZ;// / (double)k;
	int DirZ = 0;
	if (fabs(OrtZ.X) > fabs(OrtZ.Y) && fabs(OrtZ.X) > fabs(OrtZ.Z)) DirZ = 1;
	if (fabs(OrtZ.Y) > fabs(OrtZ.X) && fabs(OrtZ.Y) > fabs(OrtZ.Z)) DirZ = 2;
	if (fabs(OrtZ.Z) > fabs(OrtZ.X) && fabs(OrtZ.Z) > fabs(OrtZ.Y)) DirZ = 3;
	
	switch (DirZ) {
		case 1: OrtX = C3Point(0,1,0); OrtY = C3Point(0,0,1); break;
		case 2: OrtX = C3Point(1,0,0); OrtY = C3Point(0,0,1); break;
		case 3: OrtX = C3Point(1,0,0); OrtY = C3Point(0,1,0); break;
		default: OrtX = C3Point(0,1,0); OrtY = C3Point(0,0,1); break;
	}
	OrtZ = VectProd(OrtX, OrtY);

	baseplate->Orig = Orig;
	baseplate->OrtX = OrtX; baseplate->OrtY = OrtY;  baseplate->OrtZ = OrtZ;
	
	double xmin = 1000, xmax = -1000;//baseplate->Xmax;
	double ymin = 1000, ymax = -1000;//baseplate->Ymax;
	double stepX = 0, stepY = 0; // grid steps
	
	pos = PlatesList.GetHeadPosition();
		while (pos != NULL) {
			plate = PlatesList.GetNext(pos);
			if (plate->Selected && plate->Solid) {
				for (int k = 0; k < 4; k++) {
					P = plate->Corn[k];
					Ploc = baseplate->GetLocal(P);
					xmin = Min(xmin, Ploc.X);
					xmax = Max(xmax, Ploc.X);
					ymin = Min(ymin, Ploc.Y);
					ymax = Max(ymax, Ploc.Y);
				}// k
				if (plate->Loaded) {
					stepX = Max(stepX, plate->Load->StepX);
					stepY = Max(stepY, plate->Load->StepY);
				}
			}//if selected
		}//pos
		Ploc = C3Point(xmin, ymin, 0);
		p0 = baseplate->GetGlobalPoint(Ploc);
		Ploc = C3Point(xmax, ymin, 0);
		p1 = baseplate->GetGlobalPoint(Ploc);
		Ploc = C3Point(xmin, ymax, 0);
		p2 = baseplate->GetGlobalPoint(Ploc);
		Ploc = C3Point(xmax, ymax, 0);
		p3 = baseplate->GetGlobalPoint(Ploc);

	plate = new CPlate(); // combi-load
	plate->SetLocals(p0, p1, p2, p3);
	 plate->Solid = FALSE;
	 plate->Visible = FALSE;
	 plate->Comment = Comment;
	 //PlatesList.AddTail(pPlate);
	plate->ApplyLoad(TRUE, stepX, stepY); // optimize grid 
	plate->Selected = TRUE;

	pMarkedPlate = plate;

	P_CalculateCombi(OptParallel); // calculate maps + proect them on the combi-plate
	plate->ShowLoadState();	
	OnPlotMaxprofiles();
	OptCombiPlot = 0;

	baseplate->~CPlate();
}

void CBTRDoc::OnPlotBeamletfoot() 
{
	int res;
	if (pDataView->m_rich.GetModify()) {
		res = AfxMessageBox("Data is modified but NOT UPDATED! \n Continue?", 3);
		if (res != IDYES) return;
	}
	PlotBeamletFoot();
}

void CBTRDoc:: PlotBeamletFoot()
{
	CString S;
	double Vx, Vy, Vz, y, z, Vymin, Vzmin, Curr, MaxCurr, MaxY, MaxZ;
	CArray<double, double> PosY;
	CArray<double, double> PosZ;
	PLOTINFO BletPlot;
	
	BletPlot.LabelX = "Y, m";
	BletPlot.LabelY = "Z, m";
	BletPlot.LimitX = 0;//1.4;
	BletPlot.LimitY = 0;//1.4;
	BletPlot.CrossX = 0.2;
	BletPlot.CrossY = 0.2;
	BletPlot.Line = 0;
	BletPlot.Interp = 1;
	
	BletPlot.PosNegX = TRUE;
	BletPlot.PosNegY = TRUE;
	BletPlot.Caption = "Beamlet Image";
	S.Format(" X = %g m ", AreaLong);
	BletPlot.Caption += S;
	if ((int)BeamSplitType == 0) 
		S.Format(" (Polar = %d Azim = %d Cut-Off = %g)", (int)PolarNumber, (int)AzimNumber, CutOffCurrent);
	else 
		S.Format(" (Horiz. = %d  Vert. = %d  Cut-Off = %g)", (int)BeamSplitNumberY, (int)BeamSplitNumberZ, CutOffCurrent);
	BletPlot.Caption += S;

	int i;
	Vymin = 1.e100; Vzmin = 1.e100; 
	MaxCurr = 0; MaxY = 0; MaxZ = 0;
	for (i = 0; i < Attr_Array.GetSize(); i++) {
		Vx =  Attr_Array.GetAt(i).Vx;
		Vy = Attr_Array.GetAt(i).Vy;
		
		Vymin = Min(Vymin, fabs(Vy));
		Vz = Attr_Array.GetAt(i).Vz;
		
		Vzmin = Min(Vzmin, fabs(Vz));
		MaxCurr = Max(MaxCurr, Attr_Array.GetAt(i).Current);
		y = AreaLong * Vy / Vx; 
		if (fabs(y) < 1.e-6) y = Attr_Array.GetAt(i).Y;
		MaxY = Max(MaxY, fabs(y));
		z = AreaLong * Vz / Vx;
		if (fabs(z) < 1.e-6) z = Attr_Array.GetAt(i).Z;
		MaxZ = Max(MaxZ, fabs(z));
		PosY.Add(y);
		PosZ.Add(z);
	}

	BletPlot.N1 = Attr_Array.GetSize();
	BletPlot.N2 = BletPlot.N1;

/*	if ((int)BeamSplitType !=0) {// && Attr_Array.GetSize() >= 400) {// Cartesian
	for (i = 0; i < Attr_Array.GetSize(); i++) {
		Vx = Attr_Array.GetAt(i).Vx;
		Vy = Attr_Array.GetAt(i).Vy;
		Vz = Attr_Array.GetAt(i).Vz;
		Curr  =  Attr_Array.GetAt(i).Current;
		
			if (fabs(Vz) <= Vzmin + 1.e-16) {
				y = AreaLong * Vy / Vx;
				z = Curr / MaxCurr * MaxZ;
				PosY.Add(y);PosZ.Add(z);
			}
			if (fabs(Vy) <=  Vymin + 1.e-16) {
				y = Curr / MaxCurr * MaxY;
				z = AreaLong * Vz / Vx;
				PosY.Add(y);PosZ.Add(z);
			}
		
	}
	} // Cartesian
*/
	BletPlot.N = PosY.GetSize();

	BletPlot.DataX = &PosY;
	BletPlot.DataY = &PosZ;
	
	CPlotDlg dlg;
	dlg.Plot = &BletPlot;
	dlg.InitPlot();
	dlg.DoModal();

	PosY.RemoveAll();
	PosZ.RemoveAll();

	
}

void CBTRDoc::OnPlotBeamfoot() 
{
	int res;
	if (pDataView->m_rich.GetModify()) {
		res = AfxMessageBox("Data is modified but NOT UPDATED! \n Continue?", 3);
		if (res != IDYES) return;
	}

	double x, y0, z0, y, z;
	CArray<double, double> PosY;
	CArray<double, double> PosZ;
	CString S;
	int N = 0, N1 = 0, N2 = 0;
	C3Point P;
	
	PLOTINFO FootPlot;
	
	FootPlot.LabelX = "Y, m";
	FootPlot.LabelY = "Z, m";
	FootPlot.LimitX = 0;//0.3;
	FootPlot.LimitY = 0;//0.8;
	FootPlot.CrossX = 0.1;
	FootPlot.CrossY = 0.2;
	FootPlot.Line = 0;
	FootPlot.Interp = 1;
	FootPlot.N = 0;
	FootPlot.PosNegX = TRUE;
	FootPlot.PosNegY = TRUE;
	// Set Caption
	if (OptSINGAP) S.Format("SINGAP ");
	else S.Format("MAMuG ");
	FootPlot.Caption = S + " Beam Exit";
	if (Progress == 0)	S.Format(" Target Image");
	else S.Format(" Footprint of %d beamlets", Exit_Array.GetUpperBound());
	FootPlot.Caption += S;
	S.Format("   X = %gm ", AreaLong);
	FootPlot.Caption += S;

if (Progress == 0) {// Calculation OFF - create target image

	if (!OptSINGAP || !SINGAPLoaded) { // Mamug
	for (int is = 0; is < (int)NofChannelsHor;  is++)  ///NofChannelsHor;
		for (int js = 0; js < (int)NofChannelsVert; js++)   ///NofChannelsVert;
			for (int i = 0; i < (int)NofBeamletsHor; i++) 
				for (int j = 0; j < (int)NofBeamletsVert; j++)
	
		{
			y0 = BeamletPosHor[is * (int)NofBeamletsHor + i]; 
			z0 = BeamletPosVert[js * (int)NofBeamletsVert + j]; 
			x = AreaLong;
			y = y0 + x * tan(BeamletAngleHor[is * (int)NofBeamletsHor + i]);
			z = z0 + x * tan(BeamletAngleVert[js * (int)NofBeamletsVert + j]);
			PosY.Add(y);
			PosZ.Add(z);
		}
		N = NofBeamlets;
		
	}// Mamug
	
	else if (OptSINGAP){ // Singap
		BEAMLET_ENTRY be;
		N = BeamEntry_Array.GetSize();
		for (int k = 0; k < N; k++) {
			be = BeamEntry_Array.GetAt(k);
			y0 = be.PosY;
			z0 = be.PosZ;
			x = AreaLong;
			y = y0 + x * tan(be.AlfY); 
			z = z0 + x * tan(be.AlfZ); 
			PosY.Add(y);
			PosZ.Add(z);
		}
	} // singap
} // Calculation OFF (Progress = 0)

else { // calculation in progress 
	N = Exit_Array.GetSize();
		for (int k = 0; k < N; k++) {
			P = Exit_Array.GetAt(k);
			y = P.Y;
			z = P.Z;
			PosY.Add(y);
			PosZ.Add(z);
		}
} // in progress
	
	FootPlot.N = N;
	FootPlot.N1 = N1;
	FootPlot.N2 = N2;
	FootPlot.DataX = &PosY;
	FootPlot.DataY = &PosZ;

	CPlotDlg dlg;
	dlg.Plot = &FootPlot;
	dlg.InitPlot();
	dlg.DoModal();

	PosY.RemoveAll();
	PosZ.RemoveAll();
	
}

void CBTRDoc::OnAsk() 
{
	AskDlg->Create();
	AskDlg->ShowWindow(SW_RESTORE);

//	EnableAsk = FALSE;
}

void CBTRDoc::OnUpdateAsk(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(EnableAsk);	
}

////////////// from BTR-K ///////////////////////////////////////////////////////
///////////////// Beam + Plasma ///////////////////////
/*
void CBTRDoc:: SetSigmaExchangeDefault()
{
	SigmaExchEeV.RemoveAll();
	SigmaExchValcm2.RemoveAll();

	SigmaExchEeV.Add(1);	SigmaExchValcm2.Add(7e-15);
	SigmaExchEeV.Add(4);	SigmaExchValcm2.Add(6e-15);
	SigmaExchEeV.Add(10);	SigmaExchValcm2.Add(5e-15);
	SigmaExchEeV.Add(40);	SigmaExchValcm2.Add(4e-15);
	SigmaExchEeV.Add(100);	SigmaExchValcm2.Add(3.5e-15);
	SigmaExchEeV.Add(1000); SigmaExchValcm2.Add(2e-15);
	SigmaExchEeV.Add(2000); SigmaExchValcm2.Add(1.5e-15);
	SigmaExchEeV.Add(4000); SigmaExchValcm2.Add(1.2e-15);
	SigmaExchEeV.Add(6000); SigmaExchValcm2.Add(1.1e-15);
	SigmaExchEeV.Add(10000); SigmaExchValcm2.Add(1e-15);
	SigmaExchEeV.Add(20000); SigmaExchValcm2.Add(6e-16);
	SigmaExchEeV.Add(30000); SigmaExchValcm2.Add(4e-16);
	SigmaExchEeV.Add(40000); SigmaExchValcm2.Add(2e-16);
	SigmaExchEeV.Add(50000); SigmaExchValcm2.Add(1e-16);
	SigmaExchEeV.Add(60000); SigmaExchValcm2.Add(6e-17);
	SigmaExchEeV.Add(70000); SigmaExchValcm2.Add(3e-17);
	SigmaExchEeV.Add(80000); SigmaExchValcm2.Add(2e-17);
	SigmaExchEeV.Add(100000); SigmaExchValcm2.Add(1e-17);
}

double 	CBTRDoc:: GetSigmaExchange(int A, double EkeV)//
{
	CArray<double, double> * Xdata = &SigmaExchEeV; // E, eV 
	CArray<double, double> * Sdata = &SigmaExchValcm2; // cm2

	double S = 0;
	double x0, x1;
	double s0, s1;
	double x = EkeV * 1000 / A; // eV  (Energy div 2 for D)
//	double v = 1.e6 * sqrt(2*EkeV*1000 / A); // cm/s
	int Nmax = Xdata->GetUpperBound();
	 if (x > Xdata->GetAt(Nmax) || x < Xdata->GetAt(0)) return 0;
	 for (int i = 0; i < Nmax; i++) {
		 if (x >= Xdata->GetAt(i) && x < Xdata->GetAt(i+1)) {
			  x0 = Xdata->GetAt(i);  x1 = Xdata->GetAt(i+1);
			  s0 = Sdata->GetAt(i);  s1 = Sdata->GetAt(i+1);
			  S = s0 + (s1-s0) * (x-x0) / (x1-x0);
			  return (S * 1e-4); // m2
		 } //if
	 } // for
	return 0;

}

void CBTRDoc:: SetSigmaIonDefault()
{
	SigmaIonEeV.RemoveAll();
	SigmaIonValcm2.RemoveAll();

	SigmaIonEeV.Add(1000); SigmaIonValcm2.Add(0);
	SigmaIonEeV.Add(2000); SigmaIonValcm2.Add(4e-18);
	SigmaIonEeV.Add(4000); SigmaIonValcm2.Add(4e-17);
	SigmaIonEeV.Add(6000); SigmaIonValcm2.Add(8e-17);
	SigmaIonEeV.Add(8000); SigmaIonValcm2.Add(1e-16);
	SigmaIonEeV.Add(10000); SigmaIonValcm2.Add(1.2e-16);
	SigmaIonEeV.Add(20000); SigmaIonValcm2.Add(1.5e-16);
	SigmaIonEeV.Add(40000); SigmaIonValcm2.Add(1.5e-16);
	SigmaIonEeV.Add(60000); SigmaIonValcm2.Add(1.4e-16);
	SigmaIonEeV.Add(100000); SigmaIonValcm2.Add(1.2e-16);
	SigmaIonEeV.Add(200000); SigmaIonValcm2.Add(8e-17);
	SigmaIonEeV.Add(400000); SigmaIonValcm2.Add(5e-17);
	SigmaIonEeV.Add(600000); SigmaIonValcm2.Add(3e-17);
	SigmaIonEeV.Add(800000); SigmaIonValcm2.Add(2.5e-17);
	SigmaIonEeV.Add(1000000); SigmaIonValcm2.Add(2e-17);
	SigmaIonEeV.Add(2000000); SigmaIonValcm2.Add(1e-17);
	SigmaIonEeV.Add(4000000); SigmaIonValcm2.Add(6e-18);
	SigmaIonEeV.Add(6000000); SigmaIonValcm2.Add(4e-18);
	SigmaIonEeV.Add(10000000); SigmaIonValcm2.Add(3e-18);
}

double 	CBTRDoc:: GetSigmaIon(int A, double EkeV)// proton ionisation
{
	CArray<double, double> * Xdata = &SigmaIonEeV; // E, eV 
	CArray<double, double> * Sdata = &SigmaIonValcm2; // cm2
	
	double S = 0;
	double x0, x1;
	double s0, s1;
	double x = EkeV * 1000 / A; // eV  (Energy div 2 for D)
//	double v = 1.e6 * sqrt(2*EkeV*1000 / A); // cm/s
	int Nmax = Xdata->GetUpperBound();
	 if (x > Xdata->GetAt(Nmax) || x < Xdata->GetAt(0)) return 0;
	 for (int i = 0; i < Nmax; i++) {
		 if (x >= Xdata->GetAt(i) && x < Xdata->GetAt(i)) {
			  x0 = Xdata->GetAt(i);  x1 = Xdata->GetAt(i+1);
			  s0 = Sdata->GetAt(i);  s1 = Sdata->GetAt(i+1);
			  S = s0 + (s1-s0) * (x-x0) / (x1-x0);
			  return (S * 1e-4); // m2
		 } //if
	 } // for
	return 0;
}

void CBTRDoc:: SetRateElectronDefault()
{
	RateElectronTeV.RemoveAll();
	RateElectronValcm3s.RemoveAll();

	RateElectronTeV.Add(2); RateElectronValcm3s.Add(0);
	RateElectronTeV.Add(3); RateElectronValcm3s.Add(1e-10);
	RateElectronTeV.Add(4); RateElectronValcm3s.Add(5e-10);	
	RateElectronTeV.Add(6); RateElectronValcm3s.Add(2e-9);
	RateElectronTeV.Add(8); RateElectronValcm3s.Add(4e-9);
	RateElectronTeV.Add(10); RateElectronValcm3s.Add(6e-9);
	RateElectronTeV.Add(20); RateElectronValcm3s.Add(1.5e-8);
	RateElectronTeV.Add(40); RateElectronValcm3s.Add(3e-8);
	RateElectronTeV.Add(60); RateElectronValcm3s.Add(4e-8);
	RateElectronTeV.Add(80); RateElectronValcm3s.Add(4e-8);
	RateElectronTeV.Add(100); RateElectronValcm3s.Add(4e-8);
	RateElectronTeV.Add(400); RateElectronValcm3s.Add(3e-8);
	RateElectronTeV.Add(1000); RateElectronValcm3s.Add(2e-8);
	RateElectronTeV.Add(4000); RateElectronValcm3s.Add(1.5e-8);
	RateElectronTeV.Add(10000); RateElectronValcm3s.Add(1e-8);
	RateElectronTeV.Add(20000); RateElectronValcm3s.Add(8e-9);
	RateElectronTeV.Add(40000); RateElectronValcm3s.Add(6e-9);
	RateElectronTeV.Add(100000); RateElectronValcm3s.Add(4e-9);
}

double 	CBTRDoc:: GetSigmaElectron(int A, double EkeV, double TkeV)
{
	CArray<double, double> * Xdata = &RateElectronTeV; // Te, eV 
	CArray<double, double> * SVdata = &RateElectronValcm3s; // rate cm3/c
	
	double SV = 0;
	double x0, x1;
	double s0, s1;
	double x = TkeV * 1000 / A; // Te, eV  (Energy div 2 for D ??????)
	double v = 1.e6 * sqrt(2*EkeV*1000 / A); // cm/s
	int Nmax = Xdata->GetUpperBound();
	 if (x > Xdata->GetAt(Nmax) || x < Xdata->GetAt(0)) return 0;
	 for (int i = 0; i < Nmax; i++) {
		 if (x >= Xdata->GetAt(i) && x < Xdata->GetAt(i+1)) {
			  x0 = Xdata->GetAt(i);  x1 = Xdata->GetAt(i+1);
			  s0 = SVdata->GetAt(i);  s1 = SVdata->GetAt(i+1);
			  SV = s0 + (s1-s0) * (x-x0) / (x1-x0);
			  return (SV/v * 1e-4); // m2
		 } //if
	 } // for
	 return 0;
}
*/	
/////////end ///////BTR-K ///////////////////////////////////////

void CBTRDoc::OnTasksBeamplasma() 
{
	//OnPlotPenetration();
	PlotBeamDecay(0, 0);// set plasma and calculate decay
}

void CBTRDoc::OnPlotPenetration() 
{
	//if (!OptBeamInPlasma) return;
	OptBeamInPlasma = TRUE; //!OptBeamInPlasma;
	if (OptBeamInPlasma) { // calculate expected (axial) beam decay
		PlotStopArray();
		//PlotBeamDecay(0, 0);
	}
}

void CBTRDoc::OnOptionsTraceneutralsinplasma() 
{
	//OnPlotPenetration();
	PlotBeamDecay(0, 0);// set plasma and calculate decay
	
}

double CBTRDoc:: GetPlasmaDensityParabolic(double r, int order)
{
	double Dens = 1;
	if (fabs(r) > PlasmaMinorR) return 0;
	double rn = r / PlasmaMinorR;
	switch (order) {
	case 0: Dens = 1;break;
	case 1: Dens = 1 - rn; break;
	case 2: Dens = 1 - rn*rn; break;
	case 4: Dens = 1 - rn*rn*rn*rn; break;
	case 8: Dens = 1 - rn*rn*rn*rn*rn*rn*rn*rn;	break;
	default: Dens = 1;
	}
	return Dens;
}

double CBTRDoc:: GetPlasmaTeParabolic(double r, int order)
{
	double Te = 1;
	if (fabs(r) > PlasmaMinorR) return 0;
	double rn = r / PlasmaMinorR;
	switch (order) {
	case 0: Te = 1; break;
	case 1: Te = 1 - rn; break;
	case 2: Te = 1 - rn*rn; break;
	case 4: Te = 1 - rn*rn*rn*rn; break;
	case 8: Te = 1 - rn*rn*rn*rn*rn*rn*rn*rn; break;
	default: Te = 1;
	}
	return Te;
}

double CBTRDoc:: GetR(double X, double Y, double Z) // poloidal radius of plasma
{
	double x = X - TorCentre.X;
	double y = Y - TorCentre.Y;
	double z = Z - TorCentre.Z;
	double R, r;
	R = sqrt(x*x + y*y); // toroidal radius at z=0 
	r = R - PlasmaMajorR; //poloidal radius at z=0
	return sqrt(r*r + z*z);
}

C3Point CBTRDoc:: GetTorRZ(double X, double Y, double Z) // get R/Z in tokamak frame
{
	C3Point P(0,0,0);
	double x = X - TorCentreX;
	double y = Y - TorCentreY;
	double z = Z - TorCentreZ;
	double R = sqrt(x*x + y*y); // toroidal radius at z=0 
	P.X = R; P.Y = R; P.Z = z;
	return P;
}
double CBTRDoc:: GetPSI(double R, double Z) // get poloidal flux value (normalized to 1 on separatrix)
{
	double PSI = -1;
	C3Point Pgl, Ploc;
	if (R < PlasmaMajorR - PlasmaMinorR || R > PlasmaMajorR + PlasmaMinorR) return -1;
	Pgl.X = 1000; Pgl.Y = R; Pgl.Z = Z; // R, Z - in tor frame!
	if (pTorCrossPlate != NULL) {
		Ploc = pTorCrossPlate->GetLocal(Pgl);
		PSI = pTorCrossPlate->Load->GetVal(Ploc.X, Ploc.Y);
	}

	return PSI;
}
double CBTRDoc::GetPSIrdr(double psi) // averaged PSI-volume r*dr
{
	return StepPSI;
}

void CBTRDoc:: SetNeutralisation()
{
	CNeutrDlg dlg;
	dlg.m_Thin = OptThickNeutralization ? 1 : 0;

	if (dlg.DoModal() == IDOK) {
		OptThickNeutralization = (dlg.m_Thin != 0);
	}
	else
		return; // ThickNeutralization = FALSE;
	
	if (!PressureLoaded && OptThickNeutralization) {
		AfxMessageBox("gas profile is needed for THICK model \n THIN model is active ");
		OptThickNeutralization = FALSE;
	}
/*	if (BTRVersion >= 5.0 && OptThickNeutralization) {
		AfxMessageBox("Version 5: not supports THICK model yet \n THIN model is active ");
		OptThickNeutralization = FALSE;
	} */

	if (!OptThickNeutralization) {
		CThinDlg dlg;
		dlg.m_Xlim = NeutrXmax;
		dlg.m_Neff = NeutrPart * 100;
		dlg.m_PosYield = PosIonPart * 100;
		if (dlg.DoModal() == IDOK) {
			NeutrXmax = dlg.m_Xlim;
			NeutrPart = dlg.m_Neff * 0.01;
			PosIonPart = dlg.m_PosYield * 0.01;
			CheckData();
		}
	}

	else {
		CThickDlg dlg;
		dlg.m_Xmin = NeutrXmin;
		dlg.m_Xmax = NeutrXmax;
		dlg.m_Step = NeutrStepL;
		dlg.m_SigmaNeutr = NeutrSigma * 10000; // -> cm2
		dlg.m_SigmaIon = ReionSigma * 10000;
		dlg.m_Sigma2Strip = TwoStripSigma * 10000;
		dlg.m_SigmaExch = PosExchSigma * 10000;
		if (dlg.DoModal() == IDOK) {
			NeutrXmin = dlg.m_Xmin;
			NeutrXmax = dlg.m_Xmax;
			NeutrStepL = dlg.m_Step;
			NeutrSigma = dlg.m_SigmaNeutr * 0.0001; // -> m2
			ReionSigma = dlg.m_SigmaIon * 0.0001;
			TwoStripSigma = dlg.m_Sigma2Strip * 0.0001;
			PosExchSigma = dlg.m_SigmaExch * 0.0001;
			SetNeutrCurrents();
		}
	}
}

void CBTRDoc:: SetReionization()
{
	CReionDlg dlg;

	switch (TracePartType) {
	case 0:	dlg.m_Caption = "e -> e+"; break;
	case 1:
	case -1:
	case 10:
			dlg.m_Caption = "Ho -> H+"; break;
	case 2:
	case -2:
	case 20:
			dlg.m_Caption = "Do -> D+"; break;
	default: dlg.m_Caption = "Neutral atom Zo -> Ion Z+"; break; 
	}
	dlg.m_IonStepL = IonStepL;
	dlg.m_Lstep = ReionStepL;
	dlg.m_Percent =	ReionPercent;
	dlg.m_Sigma = ReionSigma;
	dlg.m_Xmin = ReionXmin;
	dlg.m_Xmax = ReionXmax;
	dlg.m_StepSpec = IonStepLspec;
	dlg.m_Xspec0 = Xspec0;
	dlg.m_Xspec1 = Xspec1;
	dlg.m_StepSpecR = ReionStepLspec;
	dlg.m_XRspec0 = RXspec0;
	dlg.m_XRspec1 = RXspec1;
	dlg.m_Polar = (int)PolarNumberReion;
	dlg.m_Azim = (int)AzimNumberReion;

	if (dlg.DoModal() == IDOK) {
		ReionSigma = dlg.m_Sigma;
		ReionXmin = dlg.m_Xmin;
		ReionXmax = dlg.m_Xmax;
		ReionStepL = dlg.m_Lstep;
		IonStepL = dlg.m_IonStepL;
		IonStepLspec = dlg.m_StepSpec;
		Xspec0 = dlg.m_Xspec0;
		Xspec1 = dlg.m_Xspec1;
		ReionStepLspec = dlg.m_StepSpecR;
		RXspec0 = dlg.m_XRspec0;
		RXspec1 = dlg.m_XRspec1;
		PolarNumberReion = (double) dlg.m_Polar;
		AzimNumberReion = (double) dlg.m_Azim;
	
		SetReionPercent();
		CString S;
		if (PressureLoaded && GasCoeff > 1.e-6)	S.Format("Re-ionization loss = %3.1f %%", ReionPercent);
		else S.Format("Pressure profile not defined. \n Re-ionized current = 0");
		AfxMessageBox(S);

		SetPolarBeamletReion();
		
		int Nattr = Attr_Array_Reion.GetSize();
		S.Format("Reion Attr size %d \n works in BTR-5.0 \n (Current version is BTR %g)", Nattr, BTRVersion);
		AfxMessageBox(S);
	}
}

double CBTRDoc:: GetReionDecay(C3Point P)
{
	if (!OptReionAccount) return 1;
	double power = 1, power0, power1;
	//C3Point P0, P1;
	double X0, X1;
	int Nmax = ReionArray.GetUpperBound();
	if (Nmax < 1) return 1; 
	if (P.X > ReionArray[Nmax].X) return (ReionArray[Nmax].Z);
	if (P.X < ReionArray[0].X) return 1;
	
	for (int i = 0; i < Nmax; i++) {
		 X0 = ReionArray[i].X;  X1 = ReionArray[i+1].X;
		 if (P.X >= X0 && P.X < X1) {
			 power0 = ReionArray[i].Z; power1 = ReionArray[i+1].Z;
			 power = power0 + (power1 - power0) * (P.X - X0) / (X1 - X0);
			 
			 return power;
		 } // if
	 } // for
	
	return power;
}

double CBTRDoc:: GetNeutrDecay(C3Point P1, C3Point P2)//used by 4.5
// called by Tracer->GetAtomSolidFall, GetAtomFallsBetween
{
	//if (P1.X > pDoc->NeutrXmax) return 1; // atom start after NeutrXmax
	
	if (P1.X >= P2.X) return 1;
	double Pend = Min(P2.X, NeutrXmax);
	double NL = GetNL(P1.X, Pend);//along axis X (Y = Z = 0)
	double S01 = ReionSigma;
	double decay = exp(- S01 * NL); 
	return decay;
}

void CBTRDoc:: SetPlasmaGeom() // Tor geometry
{
	CString S;
	//if (!OptBeamInPlasma) return;
	//InjectAimR = -TorCentreY; //5.31; //tangency rad of Inject Point from Tokamak Center
	//InjectAimZ = 0;//-0.131; // InjectPoint shift from Tokamak Center line
	TorCentre.X = TorCentreX; //31.952; // horiz distance from GG centre to InjectPoint
	TorCentre.Y = TorCentreY; //- InjectAimR;
	TorCentre.Z = TorCentreZ; //-1.443;// vert distance from GG centre to Tokamak Center line
	//S.Format("SetPlasmaGeom\n TorCentre: %g, %g, %g", TorCentreX,TorCentreY,TorCentreZ);
	//AfxMessageBox(S);
	
	if (!FWdataLoaded) {
		InitFWarray();
		//PlasmaMajorR = (FWRmin + FWRmax) * 0.5;
		//PlasmaMinorR = (FWRmax - FWRmin) * 0.5;
		/*FWRmin = PlasmaMajorR - PlasmaMinorR;
		FWRmax = PlasmaMajorR + PlasmaMinorR;
		FWZmin = TorCentreZ - PlasmaMinorR;
		FWZmax = TorCentreZ + PlasmaMinorR;*/
	}
	else { // FW loaded
		PlasmaMajorR = (FWRmin + FWRmax) * 0.5;
		PlasmaMinorR = (FWRmax - FWRmin) * 0.5;
	}
	double dx = 0;
/*	if (FWRmax > fabs(TorCentreY)) 
		dx = sqrt(FWRmax*FWRmax - TorCentreY * TorCentreY);
	else {
		//S.Format("SetPlasmaGeom\n TorCentreY(%g)is beyond FW Rmax(%g)", TorCentreY, FWRmax);
		//AfxMessageBox(S);
	}*/
	dx = FWRmax; // changed !!! 
	PlasmaXmax = TorCentreX + dx;
	PlasmaXmin = TorCentreX - dx;
	/*if (FWRmin > fabs(TorCentreY)) {
		double dx1 = sqrt(FWRmin*FWRmin - TorCentreY * TorCentreY);
		PlasmaXmax = TorCentreX - dx1;
	}*/
		
	//S.Format("SetPlasmaGeom\n Plasma Xmin / Xmax : %g / %g", PlasmaXmin, PlasmaXmax);
	//AfxMessageBox(S);

	
// NEXT ARRAYS TO BE REMOVED! -> CPlasma -----------------------

	MagSurfDim = 3;
	StepPSI = 1.0 / MagSurfDim;
	DecayArray.RemoveAll();
	DecayPathArray.RemoveAll();
	SumPSIArray.RemoveAll();
	//PlasmaLoaded = FALSE;

	if (!PSILoaded) {
		PSIvolume.RemoveAll();
		for (int k = 0; k < MagSurfDim; k++) PSIvolume.Add((k + 1));// *(k + 1));// v = 2PI*r
	
	}
	
//	pBeamHorPlane->Load->Clear(); // created in SetPlatesNBI
//	pBeamVertPlane->Load->Clear();// created in SetPlatesNBI

	int Kpsi = MagSurfDim;// (int)ceil(1. / StepPSI);
	for (int k = 0; k < Kpsi; k++) {
		//double vol = PSIvolume[k];
		SumPSIArray.Add(C3Point(0, 0, 0)); // sum IonPower, L, Npower for each PSI
	}

// create path - to be moved to CPlasma!! (target)
	double Xstart = PlasmaXmin - 1;
	double Xfin = PlasmaXmax + 1;
	double LpathX = Xfin - Xstart;//GetDistBetween(P0, P1); //P1.X - P0.X;
	int Kpath = 1000;//Kpsi * 20;// (int)ceil(Lpath / StepPath);
	double StepX = LpathX / Kpath;// 0.0002 * Lpath;// PathArray
								  //C3Point vectL = (P1 - P0) / Lpath * StepPath;

	for (int k = 0; k <= Kpath; k++)
		DecayPathArray.Add(C3Point(Xstart + StepX * k, 0, 0)); // Xpoint Density Sigma along path (Kpath + 1)

	for (int k = 0; k <= Kpath; k++)
		DecayArray.Add(C3Point(0, 0, 0)); // thickness, psi, Npower  - along path
	
}

void CBTRDoc:: SetPlasmaTarget() //init plasma object
{
	// global inject geom
	SetPlasmaGeom();// set global beam-tor geometry, plasma Rminor/Rmajor, Xmin/Xmax
	// psi, profiles, cross-sections
	int Knucl = TracePartNucl;
	double E = IonBeamEnergy * 1000 / Knucl; //keV AtomEnergy;
	double Sigma0 = SigmaBase;// GetSigmaEkeV(E);
	double SigMult = (1. + SigmaEnhancement);// Delta = 1 + SigmaEnh, defined like Janev
	//double SigmaTot = Sigma0 * SigMult ; 
	//pPlasma->SetPlasmaParam(this, SigmaTot);
	double Nemax = MaxPlasmaDensity;
	double Temax = MaxPlasmaTe;
	C3Point TorC;// = TorCentre;
	TorC.X = TorCentreX;
	TorC.Y = TorCentreY;
	TorC.Z = TorCentreZ;
	double a = PlasmaMinorR;
	double R = PlasmaMajorR;
	double Ell = PlasmaEllipticity;
	
	pPlasma->SetPlasmaParam(TorC, a, R, Ell, Nemax, Temax, Sigma0);// ->Rmin, Rmax, Zmin, Zmax
	pPlasma->SetSigMult(SigMult);
	double Xstart = PlasmaXmin;
	double Xfin = PlasmaXmax;
	
	//C3Point P0 = GetBeamFootLimits(Xstart, Ymin, Ymax, Zmin, Zmax);
	//C3Point P1 = GetBeamFootLimits(PlasmaXmax, Ymin, Ymax, Zmin, Zmax);
	double Ymin, Ymax, Zmin, Zmax;
	double Y0min, Y0max, Z0min, Z0max, Y1min, Y1max, Z1min, Z1max;
	C3Point P0, P1;
	P0 = GetBeamFootLimits(Xstart, Y0min, Y0max, Z0min, Z0max); // axis ray start
	P1 = GetBeamFootLimits(Xfin, Y1min, Y1max, Z1min, Z1max); // axis ray finish

	Ymin = TorCentreY; //AreaHorMin
	Ymax = TorCentreY + FWRmax; //AreaHorMax;
	Zmin = FWZmin; //AreaVertMin;
	Zmax = FWZmax;
/*
	Ymin = pPlasma->Rmin + TorCentreY; // Min (Y0min, Y1min);
	Ymax = pPlasma->Rmax + TorCentreY; //Max(Y0max, Y1max);
	Zmin = pPlasma->Zmin;// +TorCentreZ; //Min(Z0min, Z1min); 
	Zmax = pPlasma->Zmax;// +TorCentreZ; //Max(Z0max, Z1max);
*/	
	double dX = Xfin - Xstart;
	double dY = Ymax - Ymin;
	double dZ = Zmax - Zmin;
	double gapY = 0;// dY * 0.2;
	double gapZ = 0;// dZ * 0.2;
	C3Point Pmin = C3Point(Xstart, Ymin-gapY, Zmin-gapZ); //C3Point(Xstart, P0.Y - 0.25, P0.Z - 0.5);// 
	C3Point Pmax = C3Point(Xfin, Ymax+gapY, Zmax+gapZ); //C3Point(Xfin, P0.Y + 0.25, P0.Z + 0.5); //
	//pPlasma->SetMesh(Pmin, Pmax, 50, 50, 50);// SetOriginDim->SetCutArrays
	
	double stepX = dX / 50;
	double stepY = dY / 50;
	double stepZ = dZ / 50;
	//double step = 0.01;
	pPlasma->SetMeshSteps(Pmin, Pmax, stepX, stepY, stepZ);//Set 3 ortogonal cuts
	pPlasma->StepZ = (pPlasma->Zmax - pPlasma->Zmin) / pPlasma->Nz;// Step.Z;
	pPlasma->StepR = 2 * a / pPlasma->Npsi;
	pPlasma->SetCutRZ(); // 2 poloidal cross-sections: R-Z Psi-Z - both [Npsi, Nz] 

	C3Point Vn = P1 - P0; // axis ray init
	BEAM_RAY ray = BEAM_RAY(IonBeamEnergy * 1.e6, TracePartNucl, P0, Vn);
	pPlasma->SetBeamRay(ray);

	double stepL = 0.01;
	SetStopArrays(P0, P1, stepL);// - for PlotStopArray, GetBeamDecay

	//std::cout << "Set Plasma Target - done" << std::endl;
	
}

void CBTRDoc::CalculateThickFocused() // run real set of axis-rays
{
	//double decay;// = GetDecayBetween(P0, P1); //fill arrays, return neutral power / power0
	CWaitCursor wait;
	double y0, z0, Ystart, Zstart, Yfin, Zfin; // bml axis
	double Xstart = PlasmaXmin - 0.5;
	double Xfin = PlasmaXmax;
	double LpathX = Xfin - Xstart;//GetDistBetween(P0, P1); //P1.X - P0.X;
	int Kpath = 3000;//Kpsi * 20;// (int)ceil(Lpath / StepPath);
	double StepL = LpathX / Kpath;
	double Ymin, Ymax, Zmin, Zmax;// tot beam size
	C3Point P0, P1, Pstart, Pfin;
	P0 = GetBeamFootLimits(Xstart, Ymin, Ymax, Zmin, Zmax);
	P1 = GetBeamFootLimits(PlasmaXmax, Ymin, Ymax, Zmin, Zmax);
	pPlasma->SetPath(P0, P1, StepL);
	pPlasma->AddDecayArrays(); // set Nrays = 0 initialize sumarrays for axis path
 // actual beam axes
	for (int is = 0; is < (int)NofChannelsHor; is++)  ///NofChannelsHor;
		for (int js = 0; js < (int)NofChannelsVert; js++)   ///NofChannelsVert;
			for (int i = 0; i < (int)NofBeamletsHor; i++)
				for (int j = 0; j < (int)NofBeamletsVert; j++)
				{
					y0 = BeamletPosHor[is * (int)NofBeamletsHor + i];
					z0 = BeamletPosVert[js * (int)NofBeamletsVert + j];

					Ystart = y0 + Xstart * tan(BeamletAngleHor[is * (int)NofBeamletsHor + i]);
					Zstart = z0 + Xstart * tan(BeamletAngleVert[js * (int)NofBeamletsVert + j]);
					Yfin = y0 + Xfin * tan(BeamletAngleHor[is * (int)NofBeamletsHor + i]);
					Zfin = z0 + Xfin * tan(BeamletAngleVert[js * (int)NofBeamletsVert + j]);

					Pstart = C3Point(Xstart, Ystart, Zstart);
					Pfin = C3Point(Xfin, Yfin, Zfin);
					pPlasma->SetPath(Pstart, Pfin, StepL);
					pPlasma->SetDecayArrays();// thin ray
					pPlasma->AddDecayArrays();// Nrays++
				
				}

}
void CBTRDoc::CalculateThickParallel() // run parallel set of rays
{
	// thick parallel beam along common axis
	CWaitCursor wait;
	double Xstart = PlasmaXmin - 0.5;
	double Xfin = PlasmaXmax;
	double LpathX = Xfin - Xstart;//GetDistBetween(P0, P1); //P1.X - P0.X;
	int Kpath = 3000;//Kpsi * 20;// (int)ceil(Lpath / StepPath);
	double StepL = LpathX / Kpath;
	
	double Ymin, Ymax, Zmin, Zmax; // tot beam 
	double Ystart, Zstart, Yfin, Zfin;  // bml axis
	C3Point P0, P1, Pstart, Pfin;
	P0 = GetBeamFootLimits(Xstart, Ymin, Ymax, Zmin, Zmax);
	P1 = GetBeamFootLimits(PlasmaXmax, Ymin, Ymax, Zmin, Zmax);
	C3Point Vn = P1 - P0; // CONST
	BEAM_RAY ray;// = BEAM_RAY(IonBeamEnergy * 1.e6, TracePartNucl, P0, Vn);
	BeamArray1D rays;
	double th = 1.0;// 1m
	double dy = 0.4 * th; // beam port width
	double dz = 0.8 * th; // beam port height

	int ky = 2, kz = 2; // to write set of rays 3x3
/*	for (int i = 0; i <= ky; i++)
		for (int j = 0; j <= kz; j++) {
			Ystart = P0.Y - dy * 0.5 + dy / ky * i;
			Zstart = P0.Z - dz * 0.5 + dz / kz * j;
			Pstart = C3Point(P0.X, Ystart, Zstart);
			//Vn = P1 - P0; // = const for parallel
			ray = BEAM_RAY(IonBeamEnergy * 1.e6, TracePartNucl, Pstart, Vn);
			rays.Add(ray);
		} // i,j
	*/		
//	pPlasma->WritePathSet(rays, pPlasma);

// calculate thick beam
	pPlasma->SetPath(P0, P1, StepL);//axis ray
	pPlasma->AddDecayArrays(); // set Nrays = 0 initialize sumarrays for axis path
	ky = 19; // to calculate 20x20
	kz = 39;
	for (int i = 0; i <= ky; i++)
		for (int j = 0; j <= kz; j++) {
			Ystart = P0.Y - dy * 0.5 + dy / ky * i;
			Zstart = P0.Z - dz * 0.5 + dz / kz * j;
			Yfin = P1.Y - dy * 0.5 + dy / ky * i;
			Zfin = P1.Z - dz * 0.5 + dz / kz * j;
			Pstart = C3Point(P0.X, Ystart, Zstart);
			Pfin = C3Point(P1.X, Yfin, Zfin);
			pPlasma->SetPath(Pstart, Pfin, StepL);
			pPlasma->SetDecayArrays();// thin ray
			pPlasma->AddDecayArrays();// Nrays++
									  //decay = GetFillDecayBetween(Pstart, Pfin, 1); // fill arrays
		}

}

void CBTRDoc:: CalculateThinDecay() //run axis beam
{
	double Xstart = PlasmaXmin;
	double Xfin = PlasmaXmax;
	double LpathX = Xfin - Xstart;//GetDistBetween(P0, P1); //P1.X - P0.X;
	int Kpath = 1000;//Kpsi * 20;// (int)ceil(Lpath / StepPath);
	double StepL = LpathX / Kpath;// 0.0002 * Lpath;// PathArray
	//C3Point vectL = (P1 - P0) / Lpath * StepPath;
	//C3Point P0(Xstart, 0, 0);
	//C3Point P1(Xfin, 0, 0);
	double Ymin, Ymax, Zmin, Zmax;
	C3Point P0, P1;
	P0 = GetBeamFootLimits(PlasmaXmin, Ymin, Ymax, Zmin, Zmax);
	P1 = GetBeamFootLimits(PlasmaXmax, Ymin, Ymax, Zmin, Zmax);
	C3Point Vn = P1 - P0;
	//pPlasma->SetOriginDim(P0, 100, 0, 0); // target start, nx,ny,nz
	
	//pPlasma->SetPath(P0, P1, 0.1);// calc path for output
	//pPlasma->WritePathThin(); // old output (no Sigmas)
	BEAM_RAY ray = BEAM_RAY(IonBeamEnergy * 1.e6, TracePartNucl, P0, Vn);
	//pPlasma->SetPathFrom(P0, Vn, 0.01);
	//pPlasma->WritePathSingle(ray, pPlasma);
	
	pPlasma->Nrays = -1;
	pPlasma->SetPath(P0, P1, StepL);// calc path points
	pPlasma->AddDecayArrays();// Init decay arrays, Nrays++ -1 -> 0

	pPlasma->SetDecayArrays();// calculate decay arrays
	pPlasma->AddDecayArrays();// add to sumarrays + convert to flux
	
	pPlasma->WritePathSingle(ray, pPlasma);
	/*
	for (int k = 0; k < MagSurfDim; k++) {
	SumPSIArray[k].X = SumPSIArray[k].X / PSIvolume[k]; // IonPower/ V
	SumPSIArray[k].Y = SumPSIArray[k].Z / SumPSIArray[k].Y; //Npower / L
	SumPSIArray[k].Z = SumPSIArray[k].Z / PSIvolume[k]; // Npower
	}
	*/
}

/*double GetSumHDT_Janev(int A, double E, double Density, double Te)
{
	double Sum = 0;
	return Sum;
}*/

void CBTRDoc::FillArrayA_Suzuki(int A, double * CoeffA) // A = 1/2/3 for H/D/T plasma
{
	CoeffA[0] = A; // atom number H/D/T
	switch (A) {
		case 1: //H
			CoeffA[1] = 12.7;
			CoeffA[2] = 1.25;
			CoeffA[3] = 0.452;
			CoeffA[4] = 0.0105;
			CoeffA[5] = 0.547;
			CoeffA[6] = -0.102;
			CoeffA[7] = 0.36;
			CoeffA[8] = -0.0298;
			CoeffA[9] = -0.0959;
			CoeffA[10] = 4.21e-3;
			break;
		case 2: //D
			CoeffA[1] = 14.1;
			CoeffA[2] = 1.11;
			CoeffA[3] = 0.408;
			CoeffA[4] = 0.0105;
			CoeffA[5] = 0.547;
			CoeffA[6] = -0.0403;
			CoeffA[7] = 0.345;
			CoeffA[8] = -0.0288;
			CoeffA[9] = -0.0971;
			CoeffA[10] = 4.74e-3;
			break;
		case 3: // T
			CoeffA[1] = 12.7;
			CoeffA[2] = 1.26;
			CoeffA[3] = 0.449;
			CoeffA[4] = 0.0105;
			CoeffA[5] = 0.547;
			CoeffA[6] = -5.77e-3;
			CoeffA[7] = 0.336;
			CoeffA[8] = -0.0282;
			CoeffA[9] = -0.0974;
			CoeffA[10] = 4.87e-3;
			break;
	}

}

void CBTRDoc::FillArrayB_Suzuki(int A, double * CoeffB) // 
{
	CoeffB[0] = A; // atom number 
	switch (A) {
		case 2: // He (alfa)
			CoeffB[1] = -1.05; // B111
			CoeffB[2] = 0.141; // B112
			CoeffB[3] = -0.375; // B121
			CoeffB[4] = -0.0155; // B122
			CoeffB[5] = 0.531; // B211
			CoeffB[6] = -0.0309; // B212
			CoeffB[7] = 0.105; // B221
			CoeffB[8] = 5.03e-3; // B222
			CoeffB[9] = -0.0417; // B311
			CoeffB[10] = 2.58e-3; // B312
			CoeffB[11] = -7.02e-3; // B321
			CoeffB[12] = -3.47e-3; // B322
			break;
		case 3: // Li
			CoeffB[1] = -1.27; // B111
			CoeffB[2] = -1.41e-3; // B112
			CoeffB[3] = -0.284; // B121
			CoeffB[4] = -0.0184; // B122
			CoeffB[5] = 0.552; // B211
			CoeffB[6] = 0.0102; // B212
			CoeffB[7] = 0.0781; // B221
			CoeffB[8] = 5.63e-3; // B222
			CoeffB[9] = -0.0408; // B311
			CoeffB[10] = -2.95e-4; // B312
			CoeffB[11] = -5.09e-3; // B321
			CoeffB[12] = -3.75e-4; // B322
			break;
		case 4: // Be
			CoeffB[1] = -1.28; // B111
			CoeffB[2] = -0.0439; // B112
			CoeffB[3] = -0.255; // B121
			CoeffB[4] = -0.0176; // B122
			CoeffB[5] = 0.529; // B211
			CoeffB[6] = 0.0209; // B212
			CoeffB[7] = 0.0682; // B221
			CoeffB[8] = 5.2e-3; // B222
			CoeffB[9] = -0.0377; // B311
			CoeffB[10] = -9.73e-4; // B312
			CoeffB[11] = -4.27e-3; // B321
			CoeffB[12] = -3.27e-4; // B322
			break;
		case 5: // B
			CoeffB[1] = -1.32; // B111
			CoeffB[2] = -0.06; // B112
			CoeffB[3] = -0.225; // B121
			CoeffB[4] = -0.0185; // B122
			CoeffB[5] = 0.519; // B211
			CoeffB[6] = 0.0242; // B212
			CoeffB[7] = 0.594; // B221
			CoeffB[8] = 5.32e-3; // B222
			CoeffB[9] = -0.0359; // B311
			CoeffB[10] = -1.14e-3; // B312
			CoeffB[11] = -3.67e-3; // B321
			CoeffB[12] = -3.29e-4; // B322
			break;
		case 6: // C
			CoeffB[1] = -1.54; // B111
			CoeffB[2] = -0.0868; // B112
			CoeffB[3] = -0.1830; // B121
			CoeffB[4] = -0.0153; // B122
			CoeffB[5] = 0.567; // B211
			CoeffB[6] = 0.0313; // B212
			CoeffB[7] = 0.046; // B221
			CoeffB[8] = 4.21e-3; // B222
			CoeffB[9] = -0.0386; // B311
			CoeffB[10] = -1.6e-3; // B312
			CoeffB[11] = -2.68e-3; // B321
			CoeffB[12] = -2.41e-4; // B322
			break;
		case 7: // N
			CoeffB[1] = -1.5; // B111
			CoeffB[2] = -0.0883; // B112
			CoeffB[3] = -0.147; // B121
			CoeffB[4] = -0.0119; // B122
			CoeffB[5] = 0.537; // B211
			CoeffB[6] = 0.03; // B212
			CoeffB[7] = 0.0355; // B221
			CoeffB[8] = 3.12e-3; // B222
			CoeffB[9] = -0.0354; // B311
			CoeffB[10] = -1.41e-3; // B312
			CoeffB[11] = -1.97e-3; // B321
			CoeffB[12] = -1.61e-4; // B322
			break;
		case 8: // 0
			CoeffB[1] = -1.46; // B111
			CoeffB[2] = -0.085; // B112
			CoeffB[3] = -0.105; // B121
			CoeffB[4] = -8.88e-3; // B122
			CoeffB[5] = 0.51; // B211
			CoeffB[6] = 0.0278; // B212
			CoeffB[7] = 0.0237; // B221
			CoeffB[8] = 2.21e-3; // B222
			CoeffB[9] = -0.0329; // B311
			CoeffB[10] = -1.2e-3; // B312
			CoeffB[11] = -1.18e-3; // B321
			CoeffB[12] = -9.79e-5; // B322
			break;
		case 26: // Fe
			CoeffB[1] = -0.427; // B111
			CoeffB[2] = 0.0439; // B112
			CoeffB[3] = 0.103; // B121
			CoeffB[4] = 0.0124; // B122
			CoeffB[5] = 0.0827; // B211
			CoeffB[6] = 0.0224; // B212
			CoeffB[7] = 0.0378; // B221
			CoeffB[8] = 4.72e-3; // B222
			CoeffB[9] = 2.84e-3; // B311
			CoeffB[10] = 2.91e-3; // B312
			CoeffB[11] = 3.19e-3; // B321
			CoeffB[12] = 4.32e-4; // B322
			break;
		default: break;
		}
}


double CBTRDoc::GetSumHDT_Suzuki(int A, double E, double Density, double Te)
{
	double Sum = 0;
	double * CoeffA = new double[12];
	FillArrayA_Suzuki(A, CoeffA);
	double e = log(E);
	double N = Density / (1.e19);
	double U = log(Te);

	double S1 = 1 + CoeffA[2] * e + CoeffA[3] * e * e;
	double S2 = 1 - exp(-CoeffA[4] * N); 
	double S3 = CoeffA[6] + CoeffA[7] * e + CoeffA[8] * e * e;
	double S4 = 1 + CoeffA[9] * U + CoeffA[10] * U * U;

	Sum = CoeffA[1] * S1 * (1 + pow(S2, CoeffA[5]) * S3) * S4; 
	return Sum;
}

double CBTRDoc::GetSumSz(int A, double E, double Density, double Te)
{
	double Sum = 0;
	double * CoeffB = new double[13];
	FillArrayB_Suzuki(A, CoeffB);
	double e = log(E);
	double N = Density / 1.e19;
	double U = log(Te);
	double lnN = log(N);

	double S1, S2, S3, S4, S5, S6, S7, S8, S9, S10, S11, S12;
	int i, j, k;
	i = 1; j = 1; k = 1;
	S1 = CoeffB[1] * pow(e, i-1) * pow(lnN, j-1) * pow(U, k-1);
	i = 1; j = 1; k = 2;
	S2 = CoeffB[2] * pow(e, i-1) * pow(lnN, j-1) * pow(U, k-1);
	i = 1; j = 2; k = 1;
	S3 = CoeffB[3] * pow(e, i-1) * pow(lnN, j-1) * pow(U, k-1);
	i = 1; j = 2; k = 2;
	S4 = CoeffB[4] * pow(e, i-1) * pow(lnN, j-1) * pow(U, k-1);
	i = 2; j = 1; k = 1;
	S5 = CoeffB[5] * pow(e, i-1) * pow(lnN, j-1) * pow(U, k-1);
	i = 2; j = 1; k = 2;
	S6 = CoeffB[6] * pow(e, i-1) * pow(lnN, j-1) * pow(U, k-1);
	i = 2; j = 2; k = 1;
	S7 = CoeffB[7] * pow(e, i-1) * pow(lnN, j-1) * pow(U, k-1);
	i = 2; j = 2; k = 2;
	S8 = CoeffB[8] * pow(e, i-1) * pow(lnN, j-1) * pow(U, k-1);
	i = 3; j = 1; k = 1;
	S9 = CoeffB[9] * pow(e, i-1) * pow(lnN, j-1) * pow(U, k-1);
	i = 3; j = 1; k = 2;
	S10 = CoeffB[10] * pow(e, i-1) * pow(lnN, j-1) * pow(U, k-1);
	i = 3; j = 2; k = 1;
	S11 = CoeffB[11] * pow(e, i-1) * pow(lnN, j-1) * pow(U, k-1);
	i = 3; j = 2; k = 2;
	S12 = CoeffB[12] * pow(e, i-1) * pow(lnN, j-1) * pow(U, k-1);
	
	Sum = S1 + S2 + S3 + S4 + S5 + S6 + S7 + S8 + S9 + S10 + S11 + S12;

	return Sum;
}

void CBTRDoc::SetImpurNumber()
{
	NofPlasmaImpur = 0;
	double Wimp;
	for (int i = 0; i < NofPlasmaImpurMax; i++) {
				Wimp = PlasmaImpurW[i];
				if (Wimp > 1.e-6) NofPlasmaImpur++;
	} //i
	CString S;
	S.Format("Number of impurities - %d", NofPlasmaImpur);
	AfxMessageBox(S);
}
int CBTRDoc::GetFirstImp()
{
	int A = 0;
	int Aimp;
	double Wimp;
	for (int i = 0; i < NofPlasmaImpurMax; i++) {
				Aimp = PlasmaImpurA[i];
				Wimp = PlasmaImpurW[i];
				if (Wimp > 1.e-6) { 
					A = Aimp;
					break;
				}
	}
	return A;
}

void CBTRDoc::SetDecayInPlasma() 
//not called (fills array of neutral power / ion density / SumPSIArray)
{
	//if (!PlasmaLoaded)  - use simplified model
/*	TorCentre.X = TorCentreX; //31.952; // horiz distance from GG centre to InjectPoint
	TorCentre.Y = TorCentreY; //- InjectAimR;
	TorCentre.Z = TorCentreZ; //-1.443;// vert distance from GG centre to Tokamak Center line
	return;
*/
	FILE * fout = fopen("Decay_in_pl.txt", "w");
	
	DecayArray.RemoveAll();
	DecayPathArray.RemoveAll();
	SumPSIArray.RemoveAll();
	StepPSI = 0.05;
	int Kpsi = (int)ceil(1. / StepPSI);
	for (int k = 0; k < Kpsi; k++) SumPSIArray.Add(C3Point(0,0,0));

	if (AreaLong < PlasmaXmin) return;
	double Ymin, Ymax, Zmin, Zmax;
	C3Point P0, P1, P, Power;

	P0 = GetBeamFootLimits(PlasmaXmin, Ymin, Ymax, Zmin, Zmax);
	P1 = GetBeamFootLimits(PlasmaXmax, Ymin, Ymax, Zmin, Zmax);

	double power = 1;
	TorCentre.X = TorCentreX; //31.952; // horiz distance from GG centre to InjectPoint
	TorCentre.Y = TorCentreY; //- InjectAimR;
	TorCentre.Z = TorCentreZ; //-1.443;// vert distance from GG centre to Tokamak Center line
	C3Point D = P1 - TorCentre;
	double R = sqrt(D.X*D.X + D.Y*D.Y); // from tor vert axis
	double DR = R - PlasmaMajorR;
	double r = sqrt(DR*DR + D.Z*D.Z); // from toroidal axis (from Major radius)
	double Density, Te, Zeff, LinIonDensity; 
	double E = IonBeamEnergy *1000; //keV AtomEnergy;
	
	int A = abs(TracePartType); //BeamType; // mass number
	int AccountEl = 1;
	int AccountIon = 1;
	int AccountEx = 1;
	double SigmaEx, SigmaIon, SigmaEl, TotalSigma = 0;
	//double SigmaEnhancement = 0;
	
	double PSI = -1;
	double EkeVamu, Wh, Wd, Wt, Wimp, TotalW;
	double SigmaH =0, SigmaD, SigmaT, Simp, TotalImpurSum;
	int Aimp, Zimp;
	double Rtor, Ztor;

	double SumThickness = 0.;
	double L = 0, dL = 0.001;
	C3Point vect = P1 - P0;
	double TotDist = ModVect(vect);
	C3Point unitvect = vect / TotDist;
	dL = Min(TotDist * 0.02, dL);
	C3Point Ptor;
	int ipsi;// index of magsurf (psi=const)
	double dth; // local thickness


	//fprintf(fout, " X	PSI	\t Ne\t	Te \t Zeff \t SigmaH \t Stotal \t NSL \t Power \t LinIonDens\n\n");	
	fprintf(fout, "\t X \t  Y \t  Z \t R \t\t      PSI 	Density  TotalSigma  Power  ipsi\n");
	P = P0;
	while (L < TotDist) {
	
		if (!PlasmaLoaded) {// plasma profiles not set -> get them from simplified model
			r = GetR(P.X, P.Y, P.Z);// poloidal radius of plasma (double)
			ipsi = (int)floor(r / PlasmaMinorR / StepPSI); // index of magsurf (psi=const)
			Density = MaxPlasmaDensity * GetPlasmaDensityParabolic(r, 2);
			Te = MaxPlasmaTe * GetPlasmaTeParabolic(r, 2);
			Zeff = 1.7;
			Ptor = GetTorRZ(P.X, P.Y, P.Z);// 2D-point in tor frame C3Point(R,R,Z)
			Rtor = Ptor.X;
			//L += dL;
			//P = P0 + unitvect * L;

			// Sigmas <- old model of beam stopping --->
			SigmaEx = AccountEx * GetSigmaExchange(A, E);//
			SigmaIon = AccountIon * GetSigmaIon(A, E);//
			SigmaEl = AccountEl * GetSigmaElectron(A, E, Te);//
			TotalSigma = (SigmaEx + SigmaIon + SigmaEl) * (1. + SigmaEnhancement);// 
		}
	
		else
		{	// plasma profiles + PSI are loaded -> use new model
			Ptor = GetTorRZ(P.X, P.Y, P.Z);// 2D-point in tor frame C3Point(R,R,Z)
			Rtor = Ptor.X;
			Ztor = Ptor.Z;
			PSI = GetPSI(Rtor, Ztor); // poloidal flux 0...1
			ipsi = (int)(PSI / StepPSI);

			if (PSI < 1.e-6 || PSI > 1) {// out of limits
				Density = 0;
				Power.X = 0;// liniondensity
				Power.Y = 0; // density
				Power.Z = power; // last value of neutral power
				TotalSigma = 0;
				ipsi = -1;
				
			}
			else { // 0 < PSI < 1
				C3Point PP = GetPlasmaTeNe(PSI); //  Ne,Te, Zeff
				Density = PP.X * (1.e19); //m-3
				Te = PP.Y; //keV
				Zeff = PP.Z; /////// PSI-profiles
				if (Density < 1.e-6) {
					AfxMessageBox("Density = 0");
					break;
				}

				// if loaded -> use Janev-Suzuki model
				EkeVamu = E / A; // beam energy per beam nucleo
				Wh = PlasmaWeightH; // hydrogen
				Wd = PlasmaWeightD; // deuterium
				Wt = PlasmaWeightT; // tritium
				TotalW = Wh + Wd + Wt;

				SigmaH = GetSumHDT_Suzuki(1, EkeVamu, Density, Te) * (1.e-16) / EkeVamu;
				SigmaD = GetSumHDT_Suzuki(2, EkeVamu, Density, Te) * (1.e-16) / EkeVamu;
				SigmaT = GetSumHDT_Suzuki(3, EkeVamu, Density, Te) * (1.e-16) / EkeVamu;
				TotalSigma = 0.0001 * (Wh * SigmaH + Wd * SigmaD + Wt * SigmaT) / TotalW;//cm2 -> m2


				/*	if (NofPlasmaImpur > 0) {// Single or many impurities plasma, Suzuki model

				if (NofPlasmaImpur == 1) {
					Aimp = GetFirstImp(); // 1st = single
					Simp = GetSumSz(Aimp, EkeVamu, Density, Te);
					TotalSigma = TotalSigma * (1 + (Zeff - 1) * Simp);  //m2
				} // single impur
				else { // > 1 impur
					TotalImpurSum = 0;
					for (int i = 0; i < NofPlasmaImpurMax; i++) {
						Aimp = PlasmaImpurA[i];
						Wimp = PlasmaImpurW[i];
						TotalW += Wimp;
						Simp = GetSumSz(Aimp, EkeVamu, Density, Te);
						Zimp = Aimp;
						TotalImpurSum += Wimp / TotalW * (Zimp - 1) * Simp;
					} // i
					TotalSigma = TotalSigma * (1 + TotalImpurSum); //(Zeff - 1) * Sz);  //m2
				}// > 1 impur
				} // plasma with impurities
			*/
			} // 0 < PSI < 1
		}

/*
		if (!PlasmaLoaded) {// old model of beam stopping --->

			SigmaEx = AccountEx * GetSigmaExchange(A, E);//
			SigmaIon = AccountIon * GetSigmaIon(A, E);//
			SigmaEl = AccountEl * GetSigmaElectron(A, E, Te);//
			TotalSigma = (SigmaEx + SigmaIon + SigmaEl) * (1. + SigmaEnhancement);// 
		}

		else { // if loaded -> use Janev-Suzuki model

// ------------ Suzuki model of stopping
			//double EkeVamu, Wh, Wd, Wt, Wimp, TotalW;
			//double SigmaH, SigmaD, SigmaT, TotalSigma, Simp, TotalImpurSum;
			//int Aimp, Zimp;
			EkeVamu = E / A; // beam energy per beam nucleo
			Wh = PlasmaWeightH; // hydrogen
			Wd = PlasmaWeightD; // deuterium
			Wt = PlasmaWeightT; // tritium
			TotalW = Wh + Wd + Wt;

			SigmaH = GetSumHDT_Suzuki(1, EkeVamu, Density, Te) * (1.e-16) / EkeVamu;
			SigmaD = GetSumHDT_Suzuki(2, EkeVamu, Density, Te) * (1.e-16) / EkeVamu;
			SigmaT = GetSumHDT_Suzuki(3, EkeVamu, Density, Te) * (1.e-16) / EkeVamu;
			TotalSigma = 0.0001 * (Wh * SigmaH + Wd * SigmaD + Wt * SigmaT) / TotalW;//cm2 -> m2

	/*	if (NofPlasmaImpur > 0) {// Single or many impurities plasma, Suzuki model

			if (NofPlasmaImpur == 1) {
				Aimp = GetFirstImp(); // 1st = single
				Simp = GetSumSz(Aimp, EkeVamu, Density, Te);
				TotalSigma = TotalSigma * (1 + (Zeff - 1) * Simp);  //m2
			} // single impur
			else { // > 1 impur
				TotalImpurSum = 0;
				for (int i = 0; i < NofPlasmaImpurMax; i++) {
					Aimp = PlasmaImpurA[i];
					Wimp = PlasmaImpurW[i];
					TotalW += Wimp;
					Simp = GetSumSz(Aimp, EkeVamu, Density, Te);
					Zimp = Aimp;
					TotalImpurSum += Wimp / TotalW * (Zimp - 1) * Simp;
				} // i
				TotalSigma = TotalSigma * (1 + TotalImpurSum); //(Zeff - 1) * Sz);  //m2
			}// > 1 impur
		} // plasma with impurities
	*/
//		} // PSI + Profiles loaded -> use Janev-Suzuki model
	
// <---------Suzuki
		dth = Density * TotalSigma * dL;
		SumThickness += dth; // integral x0...x
		power = exp(-SumThickness); // power0 = 1
		LinIonDensity = Density * TotalSigma * power;
		
		Power.X = LinIonDensity; 
		Power.Y = Density; // already multiplied by * (1.e19); //m-3
		Power.Z = power;
		DecayArray.Add(Power);
		DecayPathArray.Add(P);
		if (ipsi >= 0 && ipsi < Kpsi) {
			SumPSIArray[ipsi].X += dL;
			SumPSIArray[ipsi].Y += dth;
			SumPSIArray[ipsi].Z += LinIonDensity;
		}

	//	fprintf(fout, " %8g	%8g	%12g %12g %6g	%12g	%12g		%8g	  %8g   %8g\n", 
		//		P.X, PSI, Density, Te, Zeff,   SigmaH, TotalSigma, SumThickness, power, LinIonDensity);	
		fprintf(fout, " %2g	%2g	%2g %4g     %12g %12g %12g %8g  \t %3d \n",
						P.X, P.Y, P.Z, Rtor, PSI, Density, TotalSigma, power, ipsi);
		L += dL;
		P = P0 + unitvect * L;
	} // while (L < TotDist) 

	fclose(fout);
	//power = exp(-SumThickness); // power0 = 1
	//return endpower;
}

void CBTRDoc::SetBeamRayDecay(C3Point P0, C3Point P1) 
// not called more (in past - by PlotBeamDecay)
// clears arrays -> calls GetFillDecayBetween to fill - fixed step along L
{
	if (AreaLong < PlasmaXmin) return;
	//FILE * fout = fopen("Ray_in_pl.txt", "w");

	DecayArray.RemoveAll();
	DecayPathArray.RemoveAll();
	SumPSIArray.RemoveAll();
	
	StepPSI = 0.02;
	int Kpsi = (int)ceil(1. / StepPSI);
	for (int k = 0; k <= Kpsi; k++)
		SumPSIArray.Add(C3Point(0, 0, 0)); // accumulate IonPower, Density*Sigma, Npower

	double Lpath = GetDistBetween(P0, P1); //P1.X - P0.X;
	StepPath = 0.0002 * Lpath;// PathArray
	C3Point vectL = (P1 - P0) / Lpath * StepPath;
	
	int Kpath = (int)ceil(Lpath / StepPath);
	for (int k = 0; k <= Kpath; k++) 
		DecayPathArray.Add(C3Point(P0 + vectL * k)); // set points along path
	for (int k = 0; k <= Kpath; k++)
		DecayArray.Add(C3Point(0, 0, 0)); // set rnorm, psi, power along path

	double decay = GetFillDecayBetween(P0, P1, 1); //fill arrays, return neutral power / power0

	//fclose(fout);
}
void  CBTRDoc:: SetStopArrays(C3Point P0, C3Point P1, double dL) 
// X, Neutr, PosIon - similar to Neutr/Reion
{
	StopArray.RemoveAll();
	CArray<double, double> * pArr = pPlasma->GetPowerDecayArray(P0,P1,dL); 
	// return power along created path
	
	double PathLen = ModVect(P1 - P0);
	int N = (int)ceil(PathLen / dL);
	C3Point PathVect = (P1 - P0) / PathLen;
	C3Point Pgl;
	double Npower0, Npower1, IonPower;
	int Nmax = pArr->GetUpperBound();
	if (Nmax < 2) {
		StopArray.Add(C3Point(P0.X, 1, 0)); return;
	}
	for (int i = 0; i < Nmax; i++) {
		Pgl = P0 + PathVect * (i * dL);
		Npower0 = pArr->GetAt(i);
		Npower1 = pArr->GetAt(i+1);
		IonPower = (Npower0 - Npower1) / dL;
		StopArray.Add(C3Point(Pgl.X, Npower0, IonPower));
	}
	// last point (Nmax)
	Pgl = P1; //P0 + PathVect * (Nmax * dL);
	Npower0 = pArr->GetAt(Nmax);
	IonPower = 0;
	StopArray.Add(C3Point(Pgl.X, Npower0, IonPower));
}

double CBTRDoc:: GetDecay(C3Point P0, C3Point P1, double dL) //each ray decay
// set start point always at Plasma start - PlasmaXmin
{
	double decay = 1;
	if (P1.X < PlasmaXmin || P0.X > PlasmaXmax) return decay;
	if (!OptBeamInPlasma) return decay;//1
	//// double CPlasma::GetPowerDecay(C3Point P0, C3Point P1, double dL)
	//decay = exp(-SumThickness); // total neutrals decay along the path
	// set P0 at PlasmaXmin plane
	C3Point dP = P1 - P0;
	C3Point V = dP / ModVect(dP); 
	double dX2 = P1.X - PlasmaXmin;
	double dt = dX2 / V.X;
	C3Point P0new = P1 - V * dt;
	decay = pPlasma->GetPowerDecay(P0new, P1, dL);
	return decay;
}

double CBTRDoc:: GetDecay(C3Point P) // get atoms Decay along axis X
{
	double decay = 1;
	if (P.X < PlasmaXmin) return decay;
	if (!OptBeamInPlasma) return decay;//1

	//if (GetDistBetween(P0, P1) < StepPath) return decay;
	//decay = GetDecayBetween(P0, P1); // not called any more
	/*double PathLen = ModVect(P1 - P0);
	int Npath = 1000;
	double StepL = PathLen / Npath;
	//pPlasma->SetPath(P0, P1, StepL);
	//pPlasma->SetDecayArrays();// thin ray
	//pPlasma->AddDecayArrays();// BAD - Tracers will overlap!! (Nrays++)*/
//test
	//double L0 = 5;
	//double L = GetDistBetween(P0, P1);
	//decay = exp(-L / L0);
	// decay = pPlasma->GetPowerDecay(P0, P1, 0.01);
	//return decay;

//------ use StopArray (X, Neutr, PosCurr)-- similar to Reion/Neutr arrays
	double power = 1, power0, power1;
	double X0, X1;
	int Nmax = StopArray.GetUpperBound();
	if (Nmax < 1) return decay; //1
	if (P.X > StopArray[Nmax].X) return (StopArray[Nmax].Y); // final Npower
	if (P.X < StopArray[0].X) return decay;//1
	
	for (int i = 0; i < Nmax; i++) {
		 X0 = StopArray[i].X;  X1 = StopArray[i+1].X;
		 if (P.X >= X0 && P.X < X1) {
			 power0 = StopArray[i].Y;  power1 = StopArray[i+1].Y;// Neutr
			 power = power0 + (power1 - power0) * (P.X - X0) / (X1 - X0);
			 return power;
		 } // if
	 } // for
	return power;


}

double CBTRDoc::GetDecayBetween(C3Point P0, C3Point P1) // old
// calculate decay - not filling arrays
														
{
	int Kpsi = MagSurfDim;// SumPSIArray.GetSize(); // (int)ceil(1. / StepPSI);

	double Lpath = GetDistBetween(P0, P1); 
	int Kpath = DecayPathArray.GetSize() - 1;// (int)ceil(Lpath / StepPath);

	double decay = 0;
	double Npower0 = 1;
	double IonPower = 0;
	double dth, SumThickness = 0;
	//double dNpower, dIonPower;// 1/m3 - volume power densities

	int ipsi, ipath;
	double r, rnorm = 1, Rtor, Ztor;
	C3Point Ptor;
	double Vsurf; // PSI-volume (within StepPSI)
	double Density, Te;
	int AccountEl = 1;
	int AccountIon = 1;
	int AccountEx = 1;
	double SigmaEx, SigmaIon, SigmaEl, TotalSigma = 0;
	//double SigmaEnhancement = 0;
	int A = abs(TracePartType); //BeamType; // mass number
	int Knucl = TracePartNucl;
	double E = IonBeamEnergy * 1000/ Knucl; //keV AtomEnergy;
	CString S;
	C3Point P = P0;
	double L = 0;
	double Lmax = Lpath;// GetDistBetween(P0, P1);
	double dL = Lpath / Kpath;// *0.01;// *PlasmaMinorR;//Lmax;
	double PSI = -1;
	double Npower = Npower0;
	double Xloc, Yloc; // local coord at hor/vert planes
	SigmaEx = AccountEx * GetSigmaExchange(A, E);//
	SigmaIon = AccountIon * GetSigmaIon(A, E);//
	//SigmaEl = AccountEl * GetSigmaElectron(A, E, Te);//
	 //TotalSigma = 1.e-20; // CONST!!!!
	double Sigma0 = GetSigmaEkeV(E); //Min = 1.e-20;
	double dSigma = SigmaEnhancement;
	double mult = 1.0 + dSigma;
	int k = 0;
	DecayArray[0].X += IonPower;// rnorm;
	DecayArray[0].Y += 1;
	DecayArray[0].Z += Npower;// k = 0

	while (k < Kpath) { //(L < Lmax) {
		k++;
		L = k * dL;
		P = P0 + (P1 - P0) * L / Lmax;
		
		if (!PlasmaLoaded) { // profiles+psi
			//r = GetR(P.X, P.Y, P.Z);// poloidal radius of plasma (double)
			//rnorm = r / PlasmaMinorR;

			PSI = pPlasma->GetPSI(P);
			if (PSI > 1) continue;
			
			//ipsi = (int)floor(r / PlasmaMinorR / StepPSI); // index of magsurf (psi=const)
			Density = MaxPlasmaDensity * pPlasma->GetPlasmaDensityParabolic(PSI, 2);
			Te = MaxPlasmaTe * pPlasma->GetPlasmaTeParabolic(PSI, 2);
			//Zeff = 1.7;
			// Sigmas <- old model of beam stopping --->
			//SigmaEx = AccountEx * GetSigmaExchange(A, E);//
			//SigmaIon = AccountIon * GetSigmaIon(A, E);//
			//SigmaEl = AccountEl * GetSigmaElectron(A, E, Te);//
			//TotalSigma = (SigmaEx + SigmaIon + SigmaEl) * (1. + SigmaEnhancement);//
			//PSI = rnorm*rnorm;
			TotalSigma = Sigma0 * mult;// *(1 - PSI));
			//Vsurf = rnorm;// *StepPSI; // r*dr;
		}
		else { // Plasma Loaded
			Ptor = pPlasma->GetTorRZ(P.X, P.Y, P.Z);// 2D-point in tor frame C3Point(R,R,Z)
			Rtor = Ptor.X;
			Ztor = Ptor.Z;
			PSI = pPlasma->GetPSI(Rtor, Ztor); // poloidal flux 0...1
			if (PSI >= 0) rnorm = sqrt(PSI);
			else continue;
			//Vsurf = StepPSI;// GetPSIrdr(PSI);
			Density =  MaxPlasmaDensity * pPlasma->GetPlasmaNePSI(PSI);// .X * (1.e19); //m-3; //  Ne,Te, Zeff
			Te = MaxPlasmaTe * pPlasma->GetPlasmaTePSI(PSI);
			//TotalSigma = GetStopSigma(PSI);// Suzuki - ???
			//SigmaEl = AccountEl * GetSigmaElectron(A, E, Te);//
			//TotalSigma = (SigmaEx + SigmaIon + SigmaEl) * (1. + SigmaEnhancement);//old
			//TotalSigma = 6.e-21 * (1.+ SigmaEnhancement); // m2 - sigma(0,1) from PAA table 
			TotalSigma = Sigma0 * mult;// *(1 - PSI));
		}
		dth = Density * TotalSigma * dL;
		SumThickness += dth;
		decay = exp(-SumThickness);
		IonPower = Npower * dth;//linear density
		Npower = Npower - IonPower; // Npower0 * decay;// Npower0 = 1
	}// (L < Lmax)
	
	return decay;
}

void CBTRDoc:: DistributeTrack(CArray<C3Point> & Pos, CArray<double> & Pow)
//DrawPartTrack(CArray <C3Point> &Pos, int charge, COLORREF color)
{
	//if (charge != 0 ) return; // only atoms now
	// temporary removed
/*	int n = Pos.GetSize();
	if (n < 1) return;
	C3Point Orig1 = pBeamHorPlane->Orig;
	C3Point Orig2 = pBeamVertPlane->Orig;
	if (n < 2) {//single point
		pBeamHorPlane->Load->Distribute(Pos[0].X - Orig1.X, Pos[0].Y - Orig1.Y, Pow[0]);
		pBeamVertPlane->Load->Distribute(Pos[0].X - Orig2.X, Pos[0].Z - Orig2.Z, Pow[0]);
		return;
	}
	for (int i = 0; i < n; i++) { // (P <= Pend) {
		pBeamHorPlane->Load->Distribute(Pos[i].X - Orig1.X, Pos[i].Y - Orig1.Y, Pow[i]);
		pBeamVertPlane->Load->Distribute(Pos[i].X - Orig2.X, Pos[i].Z - Orig2.Z, Pow[i]);
	}
	*/
}

double CBTRDoc::GetFillDecayBetween(C3Point P0, C3Point P1, double Power)//fill BeamPlanes maps
// not called (used by older versions)
//-> all is done in CPlasma object
// fill arrays - along path and along PSI
// PSI, SumThick, decay
{
	//double StepPath = 0.1 * PlasmaMinorR;// for DecayPathArray
	int Kpsi = MagSurfDim;// SumPSIArray.GetSize(); // (int)ceil(1. / StepPSI);
	
	double Lpath = GetDistBetween(P0, P1); //P1.X - P0.X;
	//StepPath = 0.02 * Lpath;// PathArray
	//C3Point vectL = (P1 - P0) / Lpath * StepPath;
	int Kpath = DecayPathArray.GetSize() - 1;// (int)ceil(Lpath / StepPath);

	double decay = 0;
	double Npower0 = Power;
	double IonPower = 0;
	double dth, SumThickness = 0;
	//double dNpower, dIonPower;// 1/m3 - volume power densities
	
	int ipsi, ipath;
	double r, rnorm = 1, Rtor, Ztor;
	C3Point Ptor;
	double Vsurf; // PSI-volume (within StepPSI)
	double Density, Te;
	int AccountEl = 1;
	int AccountIon = 1;
	int AccountEx = 1;
	//double SigmaEx, SigmaIon, SigmaEl;
	double TotalSigma = 0;
	//double SigmaEnhancement = 1;
	int A = abs(TracePartType); //BeamType; // mass number
	int Knucl = TracePartNucl;
	double E = IonBeamEnergy * 1000 /Knucl; //keV AtomEnergy;
	CString S;
	C3Point P = P0;
	double L = 0;
	double Lmax = Lpath;// GetDistBetween(P0, P1);
	double dL = Lpath / Kpath;// *0.01;// *PlasmaMinorR;//Lmax;
	double PSI = -1;
	double Npower = Npower0;
	double Xloc, Yloc; // local coord at hor/vert planes
	//SigmaEx = AccountEx * GetSigmaExchange(A, E);//
	//SigmaIon = AccountIon * GetSigmaIon(A, E);//
	//SigmaEl = AccountEl * GetSigmaElectron(A, E, Te);//
	//TotalSigma = 1.e-20; // CONST!!!!
	double Sigma0 = GetSigmaEkeV(E);// 1.e-20;
	double dSigma = SigmaEnhancement;
	//if (fabs(dSigma) < 1.e-6) dSigma = 1.e-6;

	int k = 0;
	DecayArray[0].X += IonPower;// rnorm;
	DecayArray[0].Y += 1;
	DecayArray[0].Z += Npower;// k = 0

	while (k < Kpath) { //(L < Lmax) {
		k++;
		L = k * dL;
		P = P0 + (P1 - P0) * L / Lmax;
	/*	if (L > Lmax) { // stop on P1
			dL = GetDistBetween(P, P1);// decrease step at final point
			P = P1;
		} */

		if (!PlasmaLoaded) {
			r = GetR(P.X, P.Y, P.Z);// poloidal radius of plasma (double)
			rnorm = r / PlasmaMinorR;
			if (rnorm > 1) {
				DecayArray[k].X += 0;// rnorm;
				DecayArray[k].Y += 1;
				DecayArray[k].Z += Npower;
				continue;
			}
			//ipsi = (int)floor(r / PlasmaMinorR / StepPSI); // index of magsurf (psi=const)
			Density = MaxPlasmaDensity * GetPlasmaDensityParabolic(r, 2);
			Te = MaxPlasmaTe * GetPlasmaTeParabolic(r, 2);
			//Zeff = 1.7;
			// Sigmas <- old model of beam stopping --->
			//SigmaEx = AccountEx * GetSigmaExchange(A, E);//
			//SigmaIon = AccountIon * GetSigmaIon(A, E);//
			//SigmaEl = AccountEl * GetSigmaElectron(A, E, Te);//
			//TotalSigma = (SigmaEx + SigmaIon + SigmaEl) * (1. + SigmaEnhancement);//
			PSI = rnorm;// *rnorm;
			TotalSigma = Sigma0 * (1 + dSigma);// *(1 - PSI));
			//Vsurf = rnorm;// *StepPSI; // r*dr;
		}
		else { // Plasma Loaded
			Ptor = GetTorRZ(P.X, P.Y, P.Z);// 2D-point in tor frame C3Point(R,R,Z)
			Rtor = Ptor.X;
			Ztor = Ptor.Z;
			PSI = GetPSI(Rtor, Ztor); // poloidal flux 0...1
			if (PSI >= 0) rnorm = sqrt(PSI);
			else {
				DecayArray[k].X += 0;// IonPower or thickness
				DecayArray[k].Y += 1;
				DecayArray[k].Z += Npower;
				continue;
			}
			
			//Vsurf = StepPSI;// GetPSIrdr(PSI);
			Density = GetPlasmaTeNe(PSI).X * (1.e19); //m-3; //  Ne,Te, Zeff
			Te = GetPlasmaTeNe(PSI).Y;
			//TotalSigma = GetStopSigma(PSI);// Suzuki - ???
			//SigmaEl = AccountEl * GetSigmaElectron(A, E, Te);//
			//TotalSigma = (SigmaEx + SigmaIon + SigmaEl) * (1. + SigmaEnhancement);//old
			
			//TotalSigma = 6.e-21 * (1. + SigmaEnhancement); // m2 - sigma(0,1) from PAA table 
			TotalSigma = Sigma0 * (1 + dSigma);// *(1 - PSI));
			
			/*if (Density < 1.e-6 || TotalSigma < 1.e-18 && PSI < 1) {
				S.Format("Density = %g Sigma = %g PSI = %g", Density, TotalSigma, PSI);
				AfxMessageBox(S);break;
			}*/
		}
		
		dth = Density * TotalSigma * dL;
		SumThickness += dth; 
		decay = exp(-SumThickness);
		IonPower = Npower * dth;//linear density
		Npower =  Npower - IonPower; // Npower0 * decay;// Npower0 = 1
		
		DecayArray[k].X += dth;// IonPower;// rnorm;
		DecayArray[k].Y += PSI;
		DecayArray[k].Z += Npower;
		DecayPathArray[k].Y += Density;
		DecayPathArray[k].Z += TotalSigma;
		// temporary removed
		/*Xloc = P.X - pBeamHorPlane->Orig.X;
		Yloc = P.Y - pBeamHorPlane->Orig.Y;
		pBeamHorPlane->Load->Distribute(Xloc, Yloc, IonPower);

		Xloc = P.X - pBeamVertPlane->Orig.X;
		Yloc = P.Z - pBeamVertPlane->Orig.Z;
		pBeamVertPlane->Load->Distribute(Xloc, Yloc, IonPower);
		*/		
		ipsi = (int)floor(PSI / StepPSI);
		if (PSI > 0 && ipsi > 0 && ipsi < Kpsi) {
			SumPSIArray[ipsi].X += IonPower;
			SumPSIArray[ipsi].Y += dL;// PSIvolume[ipsi];//dth; // SumPath within StepPSI
			SumPSIArray[ipsi].Z += Npower;
		}
			
	}// (L < Lmax)
			
	return decay;
}

double CBTRDoc::GetSigmaEkeV(double EkeV)
{
	double CS = 0;
	double x = EkeV;
	double x0, x1, cs0, cs1;
	int n = CSarray.GetSize();
	if (n == 0) return CS;
	if (n == 1) return CSarray[0].Y * (1.e-4);
	else {
	for (int i = 0; i < n-1; i++) {
		 if (x >= CSarray[i].X && x <= CSarray[i+1].X) {
			 x0 = CSarray[i].X;  x1 = CSarray[i+1].X;
			 cs0 = CSarray[i].Y; cs1 = CSarray[i+1].Y;
			 CS = cs0 + (cs1-cs0) * (x-x0) / (x1-x0);
			 return (CS *1.e-4);
		 } //if
	 } // for
	}
	 return (CS * 1.e-4);
}

double CBTRDoc::GetStopSigma(double PSI)
{
	double TotalSigma = 0;
	if (PSI < 0 || PSI > 1) {// out of limits
		return TotalSigma; // 0
	}
	else { // 0 < PSI < 1
		C3Point PP = GetPlasmaTeNe(PSI); //  Ne,Te, Zeff
		double Density = PP.X * (1.e19); //m-3
		double Te = PP.Y; //keV
		double Zeff = PP.Z; /////// PSI-profiles

		// if loaded -> use Janev-Suzuki model
		int A = abs(TracePartType); //BeamType; // mass number
		double E = IonBeamEnergy * 1000; //keV AtomEnergy;
		double EkeVamu = E / A; // beam energy per beam nucleo

		int AccountEl = 1;
		int AccountIon = 1;
		int AccountEx = 1;
		//double SigmaEx, SigmaIon, SigmaEl, 
		double TotalSigma = 0;
		double SigmaEnh = 0;
		double Wh, Wd, Wt, Wimp, TotalW;
		double SigmaH = 0, SigmaD, SigmaT, Simp, TotalImpurSum;
		Wh = PlasmaWeightH; // hydrogen
		Wd = PlasmaWeightD; // deuterium
		Wt = PlasmaWeightT; // tritium
		TotalW = Wh + Wd + Wt;

		SigmaH = GetSumHDT_Suzuki(1, EkeVamu, Density, Te) * (1.e-16) / EkeVamu;//cm2
		SigmaD = GetSumHDT_Suzuki(2, EkeVamu, Density, Te) * (1.e-16) / EkeVamu;
		SigmaT = GetSumHDT_Suzuki(3, EkeVamu, Density, Te) * (1.e-16) / EkeVamu;
		TotalSigma = 0.0001 * (Wh * SigmaH + Wd * SigmaD + Wt * SigmaT) / TotalW;//cm2 -> m2
	}

	return TotalSigma;
}

C3Point CBTRDoc:: GetPlasmaTeNe(double PSI) // ref to poloidal flux
{
	C3Point P0, P1, P(0,0,0);
	double psi0, psi1;
	int Kmax = PlasmaProfilePSI.GetUpperBound();
	if (PSI < 0) return P;
	if (Kmax < 1) return P;
	if ((PSI - PlasmaProfilePSI[Kmax]) * (PSI - PlasmaProfilePSI[0]) > 0) return P;//PlasmaProfile[Kmax]; // Te/Ne at separatrix
	//if (PSI <= PlasmaProfilePSI[0]) return P;// PlasmaProfile[0];
	
	for (int i = 0; i < Kmax; i++) {
		 psi0 = PlasmaProfilePSI[i];  psi1 = PlasmaProfilePSI[i+1];
		 if ((PSI - psi0) * (PSI - psi1)<= 1.e-12) {
			 P0 = PlasmaProfileNTZ[i]; P1 = PlasmaProfileNTZ[i+1];
			 P = P0 + (P1 - P0) * (PSI - psi0) / (psi1 - psi0);
			 return P;
		 } //if
	 } // for
	
	return P;
}

void CBTRDoc:: SetBeamDecay() //previously called by PlotBeamDecay, CalculateTracks
{ // calls GetFillDecayBetween
	double decay;// = GetDecayBetween(P0, P1); //fill arrays, return neutral power / power0
	CWaitCursor wait;
	double y0, z0, Ystart, Zstart, Yfin, Zfin;
	C3Point Pstart, Pfin;
	double Xstart = DecayPathArray[0].X;
	int Kmax = DecayPathArray.GetUpperBound();
	double Xfin = DecayPathArray[Kmax].X;//PlasmaXmax + 1;
	
	// actual beam axes
	for (int is = 0; is < (int)NofChannelsHor; is++)  ///NofChannelsHor;
		for (int js = 0; js < (int)NofChannelsVert; js++)   ///NofChannelsVert;
			for (int i = 0; i < (int)NofBeamletsHor; i++)
				for (int j = 0; j < (int)NofBeamletsVert; j++)

				{
					y0 = BeamletPosHor[is * (int)NofBeamletsHor + i];
					z0 = BeamletPosVert[js * (int)NofBeamletsVert + j];

					Ystart = y0 + Xstart * tan(BeamletAngleHor[is * (int)NofBeamletsHor + i]);
					Zstart = z0 + Xstart * tan(BeamletAngleVert[js * (int)NofBeamletsVert + j]);
					Yfin = y0 + Xfin * tan(BeamletAngleHor[is * (int)NofBeamletsHor + i]);
					Zfin = z0 + Xfin * tan(BeamletAngleVert[js * (int)NofBeamletsVert + j]);

					Pstart = C3Point(Xstart, Ystart, Zstart);
					Pfin = C3Point(Xfin, Yfin, Zfin);
					//SetBeamRayDecay(Pstart, Pfin);
					decay = GetFillDecayBetween(Pstart, Pfin, 1); //fill arrays, return neutral power / power0
				}


	// thin/thick beam = axis ray + parallel rays
/*	double Ymin, Ymax, Zmin, Zmax;
	C3Point P0 = GetBeamFootLimits(Xstart, Ymin, Ymax, Zmin, Zmax);
	C3Point P1 = GetBeamFootLimits(PlasmaXmax, Ymin, Ymax, Zmin, Zmax);
	double th = 1.0;// 1 cm
	double dy =  0.2 * th; // beam port width
	double dz =  0.4 * th; // beam port height
	double y, z;
	int ky = 10, kz = 10;
	for (int i = 0; i <= ky; i++)
	for (int j = 0; j <= kz; j++) {
	y = P0.Y - dy * 0.5 + dy / ky * i;
	z = P0.Z - dz * 0.5 + dz / kz * j;
	Pstart = C3Point(P0.X, y, z);
	y = P1.Y - dy * 0.5 + dy / ky * i;
	z = P1.Z - dz * 0.5 + dz / kz * j;
	Pfin = C3Point(P1.X, y, z);
	decay = GetFillDecayBetween(Pstart, Pfin, 1); // fill arrays
	}
	*/

		/*	SumPSIArray[ipsi].X += IonPower;
			SumPSIArray[ipsi].Y += dL;// PSIvolume[ipsi];//dth; // SumPath within StepPSI
			SumPSIArray[ipsi].Z += Npower;*/
	for (int k = 0; k < MagSurfDim; k++) {
		SumPSIArray[k].X = SumPSIArray[k].X / PSIvolume[k]; // IonPower/ V
		SumPSIArray[k].Y = SumPSIArray[k].Z / SumPSIArray[k].Y; //Npower / L
		SumPSIArray[k].Z = SumPSIArray[k].Z / PSIvolume[k]; // Npower 
	}
} 

void CBTRDoc::CalculateBeamPlasma() // 0 - thin, 1 -  parallel, 2 - focused
{
	SetPlasmaTarget();
	int bopt = pPlasma->BeamOpt;
	if (bopt == 0) CalculateThinDecay(); //run ray beam along axis
	else if (bopt == 1) CalculateThickParallel(); // parallel set 11x11
	else CalculateThickFocused(); // beamopt =2 real bml axes
}

void CBTRDoc::PlotBeamDecay(int bopt, int popt) // calculate and plot the decay along set of beam rays
{
	int AccountEl = 1;
	int AccountIon = 1;
	int AccountEx = 1;
	
	double AtomEnergy = IonBeamEnergy * 1000; //keV
	int BeamType = abs(TracePartType);
	int beamopt = bopt; // beam
	int pathopt = popt; // path
	//pPlasma->SetCalcOpt(pathopt);

	//double SigmaEnhancement = 0;
	//SetPlasmaTarget(); //init plasma object + geom
	//CalculateThinDecay();
		
	CInjectDlg dlg;
	dlg.doc = this;
	CString S = "plasma: ";

	//while (idres != IDCANCEL) {
	if (pPlasma->PSIloaded) S = S + "PSI ";
	if (pPlasma->ProfilesLoaded) S = S + "prof";
	if (pPlasma->SigmaLoaded) S = S + " sigmas";
	//AfxMessageBox(S);
	dlg.m_Sort = BeamType - 1;
	dlg.m_Energy1 = AtomEnergy;
	dlg.m_Density = MaxPlasmaDensity;
	dlg.m_Te = MaxPlasmaTe;
	dlg.m_Rmajor = PlasmaMajorR;
	dlg.m_Rminor = PlasmaMinorR;
	dlg.m_Sigma0 = SigmaBase;
	dlg.m_Enhance = SigmaEnhancement;
	dlg.m_TorCentreX = TorCentreX;
	dlg.m_TorCentreY = TorCentreY;
	dlg.m_TorCentreZ = TorCentreZ;
	//dlg.m_AimR = -TorCentre.Y;//InjectAimR;
	dlg.m_OptEl = AccountEl;
	dlg.m_OptIon = AccountIon;
	dlg.m_OptExch = AccountEx;
	dlg.m_Rays = beamopt;
	dlg.m_Path = pathopt;
	
	int idres = dlg.DoModal();
	
	if (idres == IDOK) { // || idres == IDCANCEL){ // APPLY or CLOSE
		//dlg.UpdateData(TRUE);
		BeamType = (dlg.m_Sort) + 1;
		TracePartType = -BeamType; // Neg Ions
		IonBeamEnergy = dlg.m_Energy1 * 0.001;
		//TorCentre.Y = - dlg.m_AimR;
		TorCentreX = dlg.m_TorCentreX;
		TorCentreY = dlg.m_TorCentreY;
		TorCentreZ = dlg.m_TorCentreZ;
		TorCentre.X = TorCentreX;
		TorCentre.Y = TorCentreY;
		TorCentre.Z = TorCentreZ;

		AtomEnergy = dlg.m_Energy1;
		MaxPlasmaDensity = dlg.m_Density;
		MaxPlasmaTe = dlg.m_Te;
		PlasmaMajorR = dlg.m_Rmajor;
		PlasmaMinorR = dlg.m_Rminor;
		SigmaBase = dlg.m_Sigma0;
		SigmaEnhancement = dlg.m_Enhance; // 
		//AccountEl = dlg.m_OptEl; // not used !!
		//AccountIon = dlg.m_OptIon; // not used  !!
		//AccountEx = dlg.m_OptExch; // not used  !!
		beamopt = dlg.m_Rays;
		pathopt = dlg.m_Path;
				
		SetPlasmaTarget(); //init plasma object, Nray = -1, plasma->ClearArrays(), set beam Ray (axial)
		// SetPlasmaGeom();// set beam-tor geometry, plasma Rminor/Rmajor, Xmin/Xmax
		pPlasma->SetBeamCalcOpt(beamopt, pathopt);// option CalcPlot is used only for plot, nowhere else
		
		OnShow();
		if (fabs(TorCentreY) > FWRmax) 	
			AfxMessageBox("Beam injects beyond tokamak (FWRmax)");
	
		PlotBeamDecay(beamopt, pathopt); // self call - return back to dlg

	}// OK or CANCEL

	else { // trace beam
		//CalculateBeamPlasma();
		//if (beamopt == 0) CalculateThinDecay(); //run ray beam along axis
		//else if (beamopt == 1) CalculateThickParallel(); // parallel set 11x11
		//else CalculateThickFocused(); // beamopt =2 real bml axes
		//pPlasma->SetCalcOpt(pathopt);// option CalcPlot is used only for plot, nowhere else
		//	pBeamHorPlane->Load->Clear(); // created in SetPlatesNBI
	    //	pBeamVertPlane->Load->Clear();// created in SetPlatesNBI
		//SetBeamDecay(); // calls GetFillDecayBetween - to replace!
		//pBeamHorPlane->Load->SetSumMax();
		//pBeamVertPlane->Load->SetSumMax();
		//TracksCalculated = 1;// otherwise will recalculate tracks!!!
	}// Apply (OK) 

}

void CBTRDoc::PlotPSIArrays() // profiles along PSI
{
	CArray<double, double> PosX;
	CArray<double, double> PosY;
	//CArray<double, double> NCurr;
	PLOTINFO DecayPlot;
	int idres = IDOK;
	
	DecayPlot.Caption = "Plasma parameters across norm. poloidal flux";   //"Beam deposition across magnetic surfaces" ;
	DecayPlot.LabelX = "PSI norm";
	DecayPlot.LabelY = "Te, Ne, Vol   "; 
	DecayPlot.LimitX = 1;
	DecayPlot.LimitY = 1;

	DecayPlot.CrossX = 0.1;
	DecayPlot.CrossY = 0.1;
	DecayPlot.Line = 1;
	DecayPlot.Interp = 1;
	DecayPlot.PosNegX = FALSE;
	DecayPlot.PosNegY = FALSE;

	//C3Point P;
	double psi, Te, Ne, Vol, Tmax, Nmax;
	double V1max = 0, V2max = 0, V3max = 0;
	double fStep = pPlasma->StepFlux;//pDoc->StepPSI
	int Kmax = pPlasma->Nprof;//(int)ceil(1.0 / fStep); 
	//if (pPlasma->ProfilesLoaded) Kmax = pPlasma->Nprof;
	Tmax = pPlasma->Temax;
	Nmax = pPlasma->Nemax;
	for (int i = 0; i < Kmax; i++) { 
		//P = SumPSIArray[i];
		psi = pPlasma->PSIprof[i];// *fStep;
		PosX.Add(psi);// (StepPSI * i);
		Te = pPlasma->GetPlasmaTePSI(psi);//pPlasma->TeNorm[i];
		PosY.Add(Te);
		V1max = max(V1max, Te);
	}
	DecayPlot.N1 = PosX.GetSize();

	for (int i = 0; i < Kmax; i++) { // Npower div PSI volume
		//P = SumPSIArray[i];
		psi = pPlasma->PSIprof[i];// i * fStep;
		PosX.Add(psi);// (StepPSI * i);
		Ne = pPlasma->GetPlasmaNePSI(psi); //pPlasma->NeNorm[i]; 
		PosY.Add(Ne);
		V2max = max(V2max, Ne);
	}
	DecayPlot.N2 = PosX.GetSize();

	for (int i = 0; i < Kmax; i++) { // plot Npower		
		//P = SumPSIArray[i];
		psi = pPlasma->PSIprof[i];// i * fStep;
		PosX.Add(psi);// (StepPSI * i);
		Vol = pPlasma->GetPlasmaVolPSI(psi);
		PosY.Add(Vol);
		V3max = max(V3max, Vol);
	}
	DecayPlot.N = PosX.GetSize();

/*	FILE * fout;
	fout = fopen("OutProf.txt", "w");
	fprintf(fout, "Plasma Profiles PSI  Ne  Te\n");
	for (int i = 0; i < Kmax; i++) {
		CString Sf;
		Sf.Format("%f    %g    %g\n", pPlasma->PSIprof[i], pPlasma->NeNorm[i], pPlasma->TeNorm[i]);
		fprintf(fout, Sf);
	}
	fclose(fout);*/

	// Normalize
	int k;
	for (k = 0; k < DecayPlot.N1; k++) PosY[k] = PosY[k] / V1max;
	for (k = DecayPlot.N1; k < DecayPlot.N2; k++) PosY[k] = PosY[k] / V2max;
	for (k = DecayPlot.N2; k < DecayPlot.N; k++) PosY[k] = PosY[k] / V3max;
	
	DecayPlot.DataX = &PosX;
	DecayPlot.DataY = &PosY;

	//DecayPlot.Caption += S;
	CString S, Sopt;
	if (pPlasma->ProfilesLoaded) Sopt = "ON";
	else Sopt = "OFF";
	S.Format(" real %s Tmax = %g, Nmax = %g, VMax = %g ", Sopt, Tmax, Nmax, V3max);
	DecayPlot.Caption += S;

	CPlotDlg pdlg;
	pdlg.Plot = &DecayPlot;
	pdlg.InitPlot();
	if (pdlg.DoModal() == IDCANCEL) idres = IDCANCEL;
	PosX.RemoveAll();
	PosY.RemoveAll();

}

void CBTRDoc::PlotDecayArray(int opt)// = beamopt 0 - thin, 1 - par, 2 - real
{ // plots Decay arrays, calculated in CalculateBeamPlasma
	// StopArrays are set in SetStopArrays and actually used for beam tracing
	CArray<double, double> PosX;
	CArray<double, double> PosY;
	CArray<C3Point, C3Point> * path = &(pPlasma->AverPath);
	CArray<double, double> * Power = &(pPlasma->SumPower);
	CArray<double, double> * NL = &(pPlasma->SumNL);
	CArray<double, double> * IonRate = &(pPlasma->SumIonRate);
	CArray<int, int> * Npart = &(pPlasma->SumPart);
	int Nrays = pPlasma->Nrays;
	int Kmax = path->GetSize()-1;// AverPath
	CArray<C3Point, C3Point> fPath;// create for R/flux option
	CArray<int, int> Np; // create for 1 ray along path
	
	C3Point P;
	int CalcOpt = pPlasma->CalcOpt;// 0 -path 1 - radius
	//double Npower0, Npower1;
	double df = pPlasma->StepFlux;
	int N;// , N0, N1;
	
	double Percent = pPlasma->RayDecay * 100; //thin
	if (opt > 0) Percent = pPlasma->SumDecay * 100;// thick
		
	if (CalcOpt == 1) {//along radius -> generate "path" - array along R/flux
		Kmax = pPlasma->Npsi;
		for (int i = 0; i <= Kmax; i++) { // generate "points" - with const stepFlux along R/flux
			P = C3Point(i * df, 0, 0);
			fPath.Add(P);
		}
		path = &fPath;
		Power = &(pPlasma->fPower);
		NL = &(pPlasma->fNL);
		IonRate = &(pPlasma->fIonRate);
		Npart = &(pPlasma->fSumPart);
	} //  opt  R/flux

	else if (opt == 0 && CalcOpt == 0 )  { //i.e. CalcOpt != 1 along path, 1 ray -> create Npart=1
		path = &(pPlasma->Path);
		Power = &(pPlasma->Power);
		NL = &(pPlasma->NL);
		IonRate = &(pPlasma->IonRate);
		Kmax = pPlasma->Npath;
		
		for (int i = 0; i <= Kmax; i++) {// create Npart = 1 everywhere
			Np.Add(1);
		}
		Npart = &Np;
		
	}
	else {} // leave default AverPath, Sumarrays
			
	//CArray<double, double> NCurr;
	PLOTINFO DecayPlot;
	int idres = IDOK;
	int BeamType = abs(TracePartType);
	CString S;
	CString Sopt;
	if (pPlasma->ProfilesLoaded) Sopt = "EQDSK";
	else Sopt = "R2";
	if (BeamType == 2) S.Format("D beam in %s plasma:  ", Sopt);
	else S.Format("H Beam in %s plasma:", Sopt);
	DecayPlot.Caption = S;
	DecayPlot.LabelX = "Xb-Xo, m";
	if (CalcOpt == 1) DecayPlot.LabelX = "psi";
	//DecayPlot.Caption = "Beam deposition across magnetic surfaces";
	//DecayPlot.LabelX = "PSI norm";
	DecayPlot.LabelY = "IRate, PSI/Volume, NPower"; //"NSdL, PSI, Npower   "; 
		//+IonPower, +SumThickness, +Npower
	
	if (pPlasma->CalcOpt == 0) DecayPlot.LimitX = floor(PlasmaXmax - PlasmaXmin) + 1;
	else DecayPlot.LimitX = 1;	
	DecayPlot.LimitY = 1;

	if (pPlasma->CalcOpt == 0) DecayPlot.CrossX = 1;
	else DecayPlot.CrossX = 0.1;

	DecayPlot.CrossY = 0.1;
	DecayPlot.Line = 1;
	DecayPlot.Interp = 1;
	DecayPlot.PosNegX = FALSE;
	DecayPlot.PosNegY = FALSE;

	double x, val, psi;
	P = path->GetAt(0);
	double Xstart = P.X;// DecayPathArray[0].X;
	double Vol = 1.0; // default psi Vol
	
	double V1max = 0, V2max = 0, V3max = 0;
	//int Kmax = pPlasma->Npath;//DecayPathArray.GetSize(); // (int)ceil((PlasmaXmax - PlasmaXmin) / StepPath);
	for (int i = 0; i < Kmax-1; i++) { // rn
		P = path->GetAt(i);
		x = P.X - Xstart;
		N =  Npart->GetAt(i);
		if (N < 1) N = 1;
		if (CalcOpt == 1) Vol = pPlasma->GetPlasmaVolPSI(x + 0.5*df);// fVolume[i];//;
		else Vol = 1.0;
		val = (IonRate->GetAt(i)) / Vol;// P = DecayArray[i];
		PosX.Add(x);// (StepPath * (i + 0.5));
		PosY.Add(val);//(P.X);
		V1max = max(V1max, val);
	}
	DecayPlot.N1 = PosX.GetSize();

	for (int i = 0; i < Kmax-1; i++) { // psi
		P = path->GetAt(i);
		x = P.X - Xstart;
		N = Npart->GetAt(i);
		if (N < 1) N = 1;
		if (CalcOpt == 1) val = pPlasma->GetPlasmaVolPSI(x); //fVolume[i]; // show PSI volume
		else val = NL->GetAt(i) / N;// show <Ne>
		PosX.Add(x);// (StepPath * (i + 0.5));
		PosY.Add(val);//(P.Y);
		V2max = max(V2max, val);
	}
	DecayPlot.N2 = PosX.GetSize();

		
	for (int i = 0; i < Kmax-1; i++) { // Npower
		P = path->GetAt(i);
		x = P.X - Xstart;
		N = Npart->GetAt(i);
		if (N < 1) N = 1;
		if (CalcOpt == 1) Vol = pPlasma->GetPlasmaVolPSI(x + 0.5 * df); //fVolume[i];
		else Vol = 1.0;
		val = (Power->GetAt(i)) / Vol;
		PosX.Add(x);// (StepPath * (i + 0.5));
		PosY.Add(val);
		V3max = max(V3max, val);
	}
	
	DecayPlot.N = PosX.GetSize();

	// Normalize
	int k;
	for (k = 0; k < DecayPlot.N1; k++) PosY[k] = PosY[k] / V1max;
	for (k = DecayPlot.N1; k < DecayPlot.N2; k++) PosY[k] = PosY[k] / V2max;
	for (k = DecayPlot.N2; k < DecayPlot.N; k++) PosY[k] = PosY[k] / V3max;

	DecayPlot.DataX = &PosX;
	DecayPlot.DataY = &PosY;

	double sigma = pPlasma->Sigma;
	double mult = pPlasma->SigMult;
	double lost = 100 - Percent;//power onto FW (not captured)
	double Vbeam = pPlasma->Ray.Vmod;
	
	S.Format("%d rays, Ytor = %6.2f, Ztor= %6.2f, X = %5.2f->%5.2f, v = %6.2e, Sig = %4.2f lost = %5.3f%% (%dpts)",
		Nrays, TorCentreY, TorCentreZ, PlasmaXmin, PlasmaXmax, Vbeam, mult, lost, Kmax);  //Percent,
	DecayPlot.Caption += S;

	CPlotDlg pdlg;
	pdlg.Plot = &DecayPlot;
	pdlg.InitPlot();
	//pdlg.SetWindowTextA("PlotDecayArray");//error
	if (pdlg.DoModal() == IDCANCEL) idres = IDCANCEL;
	PosX.RemoveAll();
	PosY.RemoveAll();

}

void CBTRDoc::PlotStopArray()
// StopArrays are set in SetStopArrays and actually used for beam decay calc for falls
{
	int res;
	if (pDataView->m_rich.GetModify()) {
		res = AfxMessageBox("Data is modified but NOT UPDATED! \n Continue?", 3);
		if (res != IDYES) return;
	}
	SetPlasmaTarget();

	CArray<double, double>  ArrX;
	CArray<double, double>  ArrY;
	CString S, spart;
	char name[1024];

	PLOTINFO Plot;
	Plot.Caption = "Decay/Ionization in Plasma" ;
	Plot.LabelX = "X, m";
	Plot.LabelY = "Ions, Atoms";
	Plot.LimitX = PlasmaXmax - PlasmaXmin;
	Plot.LimitY = 1;
	Plot.CrossX = 1;
	Plot.CrossY = 0.1;
	Plot.Line = 1;
	Plot.Interp = 1;
	Plot.PosNegY = FALSE;
	Plot.PosNegX = FALSE;

	int i;
	double x;
	// double dx = ReionStepL;
	double Curr = 0;
	double CurrMax = 0;
	double NCurrMax = 0;
	int Nmax = StopArray.GetUpperBound(); 
	for (i = 0; i <= Nmax; i++) {
		x = StopArray[i].X - PlasmaXmin;
		Curr = StopArray[i].Z; //+= StopArray.GetAt(i).Y;
		CurrMax = Max(Curr, CurrMax);
		ArrX.Add(x);
		ArrY.Add(Curr); // H+
	}
	Plot.N1 = ArrX.GetSize();
	for (i = 0; i <= Nmax; i++) ArrY[i] = ArrY[i] / CurrMax;

	double NCurr = 0;
	for (i = 0; i <= Nmax; i++) {
		x = StopArray[i].X - PlasmaXmin;
		NCurr = StopArray.GetAt(i).Y;
		NCurrMax = Max(NCurr, NCurrMax);
		ArrX.Add(x);
		ArrY.Add(NCurr); // H0
	}
	Plot.N2 = ArrX.GetSize();
	double lost = NCurr;

	Plot.N = ArrX.GetSize();
	
	Plot.DataX = &ArrX;
	Plot.DataY = &ArrY;
	
	double sigma = pPlasma->Sigma;
	double mult = pPlasma->SigMult;
	//double lost = 100 - Percent;//power onto FW (not captured)
	double Vbeam = pPlasma->Ray.Vmod;
	
	S.Format("\n Yt=%g,Zt=%g,X=%5.2f->%5.2f,v =%6.2e,S=%6.2e,lost=%7.5f",
		 TorCentreY, TorCentreZ, PlasmaXmin, PlasmaXmax, Vbeam, sigma*mult, lost);
	//Plot.Caption += S;

	CPlotDlg dlg;
	dlg.Plot = &Plot;
	//dlg.SetWindowTextA("PlotStopArray");//error
	dlg.InitPlot();
	if (dlg.DoModal() == IDOK) {
		CFileDialog dlg(FALSE, "txt | * ", "Stopping_data.txt", OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"Text file (*.txt) | *.TXT | All Files (*.*) | *.*||", NULL);
		if (dlg.DoModal() == IDOK) {
			FILE * fout;
			strcpy(name, dlg.GetPathName());
			fout = fopen(name, "w");
			//fprintf(fout, m_Text);
			//FILE * fout = fopen("Reion_data.txt", "w");
			fprintf(fout, " Beam stopping along X-axis \n");
			fprintf(fout, " Xmin = %g  Xmax = %g shine-through = %f\n", 
				"Stopping.txt", PlasmaXmin, PlasmaXmax, lost);
			fprintf(fout, " X   \t\t  J+   \t\t  J0 \n");
			//Curr = 0;
			for (i = 0; i <= Nmax; i++) {
				x = StopArray[i].X;
				Curr = StopArray.GetAt(i).Z;//H+
				NCurr = StopArray.GetAt(i).Y;//H0
				fprintf(fout, "  %f \t %f \t %f \n", x, Curr, NCurr); 
			}
			fclose(fout);
			delete dlg;
		} // filedialog OK
	} // Plot.DoModal
	
	ArrX.RemoveAll();
	ArrY.RemoveAll();

}


/*void CBTRDoc::PlotDecayArray()// along beam path
{	
	double AtomEnergy = IonBeamEnergy * 1000; //keV
	int BeamType = abs(TracePartType);
	int idres = IDOK;
	double SigmaEx, SigmaIon, SigmaEl;
	double TotalSigma;// = (SigmaEx + SigmaIon + SigmaEl)*1.0;//0.8e-20
	double x = PlasmaXmin;
	// double dL = 0.2;
	double SumThickness = 0.; // Summa N*Sigma*L
	double NeutrCurr0 = 0, NeutrCurr = 0, LinIonDensity, PSI = 0;
	double r;
	// int A = BeamType; // mass number
	double E; // beam energy
	double Density, Te; 
	int i;
	CString S;

	double MaxCurr = 1;
//----------------------------------------
	CArray<double, double> PosX;
	CArray<double, double> PosY;
	//CArray<double, double> NCurr;
	PLOTINFO DecayPlot;

	if (BeamType == 2) S.Format("DEUTERIUM Beam ");
	else S.Format("HYDROGEN Beam ");
	DecayPlot.Caption = S;
	DecayPlot.LabelX = "Xb-Xo, m";
	//DecayPlot.LabelY = "Ion Density, Plasma Density, Neutral Beam Current   ";
	DecayPlot.LabelY = "Poloidal flux, Thickness, Neutral Beam Current   ";
	DecayPlot.LimitX = floor(PlasmaXmax - PlasmaXmin)+1;
	DecayPlot.LimitY = MaxCurr;
		
	DecayPlot.CrossX = 1;
	DecayPlot.CrossY = 0.1;
	DecayPlot.Line = 1;
	DecayPlot.Interp = 1;
	DecayPlot.PosNegX = FALSE;
	DecayPlot.PosNegY = FALSE;

	E = AtomEnergy;//Energy keV;
	NeutrCurr0 = MaxCurr;
	x = PlasmaXmin; // AreaLongMax
	SumThickness = 0.;
	C3Point P;

	// plot  PSI //old - Ion current, generated by neutral beam
	for (int i = 0; i <= DecayArray.GetUpperBound(); i++) {
	//while (x < PlasmaXmax) {
	
		
			P = DecayPathArray[i];
			//LinIonDensity = DecayArray[i].X;
			PSI = DecayArray[i].X;
			PosX.Add(P.X - PlasmaXmin);
			PosY.Add(PSI);
			//NCurr.Add(P.Y);
			//x += dL;
		
	
	}
	DecayPlot.N1 = PosX.GetSize();

	
	// plot Thickness // old - Density
	int Nmax = DecayArray.GetUpperBound();
	double MaxVal = DecayArray[Nmax].Y;// last value
	for (int i = 0; i <= DecayArray.GetUpperBound(); i++) {
		P = DecayPathArray[i];
		SumThickness = DecayArray[i].Y;
		PosX.Add(P.X - PlasmaXmin);
		PosY.Add(SumThickness / MaxVal);
		//double dens = Density / MaxPlasmaDensity;
		//if (PlasmaLoaded) dens = dens / 1.0E+19;
		//PosY.Add(dens); // (Density / (MaxPlasmaDensity * 1.0E+19));
	}
	DecayPlot.N2 = PosX.GetSize();

	
	// plot Neutral current
	for (int i = 0; i <= DecayArray.GetUpperBound(); i++) {
		P = DecayPathArray[i];
		NeutrCurr = DecayArray[i].Z;
		PosX.Add(P.X - PlasmaXmin);
		PosY.Add(NeutrCurr);
	}
	DecayPlot.N = PosX.GetSize();

	DecayPlot.DataX = &PosX; 
	DecayPlot.DataY = &PosY; 

	//NeutrCurr = DecayArray[DecayArray.GetUpperBound()].Z;
	double Percent = 100 * (1 - NeutrCurr / NeutrCurr0);
	//S.Format("(Ne = %gx10^19 m-3, Te = %g keV)  Absorbed power = %4.2f %%",
				//MaxPlasmaDensity, MaxPlasmaTe, Percent);
	S.Format("Plasma(%d), Ytor = %g, Ztor= %g,  X = %f -> %f,  Absorbed power = %4.2f %%",
		PlasmaLoaded, TorCentreY, TorCentreZ, PlasmaXmin, PlasmaXmax, Percent);
	DecayPlot.Caption += S;

	CPlotDlg pdlg;
	pdlg.Plot = &DecayPlot;
	pdlg.InitPlot();
	if (pdlg.DoModal() == IDCANCEL) idres = IDCANCEL;
	PosX.RemoveAll();
	PosY.RemoveAll();
	//NCurr.RemoveAll();
	
//	} // while !idres = IDCANCEL

	
}*/



void CBTRDoc::OnUpdateOptionsTraceneutralsinplasma(CCmdUI*) 
{
//	if (!OptBeamInPlasma) pCmdUI->SetText("Trace Beam in Plasma");
//	else pCmdUI->SetText("Stop Beam before Plasma");
	
}

double 	CBTRDoc:: GetSigmaExchange(int A, double EkeV)//
{
	CArray<double, double> Xdata; // Te, eV 
	CArray<double, double> Sdata; // rate cm3/c

	Xdata.Add(1); Sdata.Add(7e-15);
	Xdata.Add(4); Sdata.Add(6e-15);
	Xdata.Add(10); Sdata.Add(5e-15);
	Xdata.Add(40); Sdata.Add(4e-15);
	Xdata.Add(100); Sdata.Add(3.5e-15);
	Xdata.Add(1000); Sdata.Add(2e-15);
	Xdata.Add(2000); Sdata.Add(1.5e-15);
	Xdata.Add(4000); Sdata.Add(1.2e-15);
	Xdata.Add(6000); Sdata.Add(1.1e-15);
	Xdata.Add(10000); Sdata.Add(1e-15);
	Xdata.Add(20000); Sdata.Add(6e-16);
	Xdata.Add(30000); Sdata.Add(4e-16);
	Xdata.Add(40000); Sdata.Add(2e-16);
	Xdata.Add(50000); Sdata.Add(1e-16);
	Xdata.Add(60000); Sdata.Add(6e-17);
	Xdata.Add(70000); Sdata.Add(3e-17);
	Xdata.Add(80000); Sdata.Add(2e-17);
	Xdata.Add(100000); Sdata.Add(1e-17);

	double S = 0;
	double x0, x1;
	double s0, s1;
	double x = EkeV * 1000 / A; // eV  (Energy div 2 for D)
//	double v = 1.e6 * sqrt(2*EkeV*1000 / A); // cm/s
	int Nmax = Xdata.GetUpperBound();
	 if (x > Xdata[Nmax] || x < Xdata[0]) return 0;
	 for (int i = 0; i < Nmax; i++) {
		 if (x >= Xdata[i] && x < Xdata[i+1]) {
			  x0 = Xdata[i];  x1 = Xdata[i+1];
			  s0 = Sdata[i];  s1 = Sdata[i+1];
			  S = s0 + (s1-s0) * (x-x0) / (x1-x0);
			  return (S * 1e-4); // m2
		 } //if
	 } // for
	return 0;
}

double 	CBTRDoc:: GetSigmaIon(int A, double EkeV)// proton ionisation
{
	CArray<double, double> Xdata; // Te, eV 
	CArray<double, double> Sdata; // rate cm3/c

	Xdata.Add(1000); Sdata.Add(0);
	Xdata.Add(2000); Sdata.Add(4e-18);
	Xdata.Add(4000); Sdata.Add(4e-17);
	Xdata.Add(6000); Sdata.Add(8e-17);
	Xdata.Add(8000); Sdata.Add(1e-16);
	Xdata.Add(10000); Sdata.Add(1.2e-16);
	Xdata.Add(20000); Sdata.Add(1.5e-16);
	Xdata.Add(40000); Sdata.Add(1.5e-16);
	Xdata.Add(60000); Sdata.Add(1.4e-16);
	Xdata.Add(100000); Sdata.Add(1.2e-16);
	Xdata.Add(200000); Sdata.Add(8e-17);
	Xdata.Add(400000); Sdata.Add(5e-17);
	Xdata.Add(600000); Sdata.Add(3e-17);
	Xdata.Add(800000); Sdata.Add(2.5e-17);
	Xdata.Add(1000000); Sdata.Add(2e-17);
	Xdata.Add(2000000); Sdata.Add(1e-17);
	Xdata.Add(4000000); Sdata.Add(6e-18);
	Xdata.Add(6000000); Sdata.Add(4e-18);
	Xdata.Add(10000000); Sdata.Add(3e-18);

	double S = 0;
	double x0, x1;
	double s0, s1;
	double x = EkeV * 1000 / A; // eV  (Energy div 2 for D)
//	double v = 1.e6 * sqrt(2*EkeV*1000 / A); // cm/s
	int Nmax = Xdata.GetUpperBound();
	 if (x > Xdata[Nmax] || x < Xdata[0]) return 0;
	 for (int i = 0; i < Nmax; i++) {
		 if (x >= Xdata[i] && x < Xdata[i+1]) {
			  x0 = Xdata[i];  x1 = Xdata[i+1];
			  s0 = Sdata[i];  s1 = Sdata[i+1];
			  S = s0 + (s1-s0) * (x-x0) / (x1-x0);
			  return (S * 1e-4); // m2
		 } //if
	 } // for
	return 0;
}

double 	CBTRDoc:: GetSigmaElectron(int A, double EkeV, double TkeV)
{
	CArray<double, double> Xdata; // Te, eV 
	CArray<double, double> SVdata; // rate cm3/c

	Xdata.Add(10); SVdata.Add(6e-9);
	Xdata.Add(20); SVdata.Add(1.5e-8);
	Xdata.Add(40); SVdata.Add(3e-8);
	Xdata.Add(60); SVdata.Add(4e-8);
	Xdata.Add(80); SVdata.Add(4e-8);
	Xdata.Add(100); SVdata.Add(4e-8);
	Xdata.Add(400); SVdata.Add(3e-8);
	Xdata.Add(1000); SVdata.Add(2e-8);
	Xdata.Add(4000); SVdata.Add(1.5e-8);
	Xdata.Add(10000); SVdata.Add(1e-8);
	Xdata.Add(20000); SVdata.Add(8e-9);
	Xdata.Add(40000); SVdata.Add(6e-9);
	Xdata.Add(100000); SVdata.Add(4e-9);

	double SV = 0;
	double x0, x1;
	double s0, s1;
	double x = TkeV * 1000;// / A; // Te, eV  (Energy div 2 for D ??????)
	double v = 1.e6 * sqrt(2*EkeV*1000 / A); // cm/s
	int Nmax = Xdata.GetUpperBound();
	 if (x > Xdata[Nmax] || x < Xdata[0]) return 0;
	 for (int i = 0; i < Nmax; i++) {
		 if (x >= Xdata[i] && x < Xdata[i+1]) {
			  x0 = Xdata[i];  x1 = Xdata[i+1];
			  s0 = SVdata[i];  s1 = SVdata[i+1];
			  SV = s0 + (s1-s0) * (x-x0) / (x1-x0);
			  return (SV/v * 1e-4); // m2
		 } //if
	 } // for
	 return 0;
}
	

void CBTRDoc::OnPlotSigmas() // not called
{
	CArray<double, double>  ArrX;
	CArray<double, double>  ArrY;

	PLOTINFO SigmaPlot;
	SigmaPlot.Caption = "Hydrogen Atom Stopping Sigmas vs Energy";
	SigmaPlot.LabelX = "H Energy, keV";
	SigmaPlot.LabelY = "Sigma, cm2: proton, electron, Charge Exchange  ";
	SigmaPlot.LimitX = 150;
	SigmaPlot.LimitY = 1.e-15;
	SigmaPlot.CrossX = 10;
	SigmaPlot.CrossY = 0.1e-15;
	SigmaPlot.Line = 1;
	SigmaPlot.Interp = 1;
//	SigmaPlot.N = GField->GasArray.GetSize();
//	
//	SigmaPlot.N2 = SigmaPlot.N +1;
	SigmaPlot.PosNegY = FALSE;
	SigmaPlot.PosNegX = FALSE;
	double dx = 1;
	double x = 1;
	double sigma;
	while (x < SigmaPlot.LimitX) {
		sigma = GetSigmaIon(1, x)*10000;
		ArrX.Add(x); ArrY.Add(sigma);
		x += dx;
	}
	SigmaPlot.N1 = ArrX.GetSize();

	x = 1;
	while (x < SigmaPlot.LimitX) {
		sigma = GetSigmaElectron(1, x, MaxPlasmaTe)*10000;
		ArrX.Add(x); ArrY.Add(sigma);
		x += dx;
	}
	SigmaPlot.N2 = ArrX.GetSize();

	x = 10;
	while (x < 100) {
		sigma = GetSigmaExchange(1, x)*10000;
		ArrX.Add(x); ArrY.Add(sigma);
		x += dx;
	}
	SigmaPlot.N = ArrX.GetSize();
	
	SigmaPlot.DataX = &ArrX;
	SigmaPlot.DataY = &ArrY;
	
	CPlotDlg dlg;
	dlg.Plot = &SigmaPlot;
	dlg.InitPlot();
	dlg.DoModal();

	ArrX.RemoveAll();
	ArrY.RemoveAll();
	
}

double 	CBTRDoc:: GetNeutrOutput(int A, int K , double EkeV)// inf neutralization target - NOT USED
{
	CArray<double, double> Xdata; // E, eV 
	CArray<double, double> Fdata; // 

	Xdata.Add(10); Fdata.Add(0.9);
	Xdata.Add(20); Fdata.Add(0.81);
	Xdata.Add(30); Fdata.Add(0.72);
	Xdata.Add(40); Fdata.Add(0.63);
	Xdata.Add(50); Fdata.Add(0.53);
	Xdata.Add(60); Fdata.Add(0.43);
	Xdata.Add(70); Fdata.Add(0.35);
	Xdata.Add(80); Fdata.Add(0.3);
	Xdata.Add(90); Fdata.Add(0.25);
	Xdata.Add(100); Fdata.Add(0.21);
	Xdata.Add(120); Fdata.Add(0.14);
	Xdata.Add(140); Fdata.Add(0.11);
	Xdata.Add(150); Fdata.Add(0.1);
	Xdata.Add(200); Fdata.Add(0.05);

	double F = 0;
	double x0, x1;
	double f0, f1;
	double x = EkeV / A; // eV  (Energy div 2 for D)
//	double v = 1.e6 * sqrt(2*EkeV*1000 / A); // cm/s
	int Nmax = Xdata.GetUpperBound();
	 if (x > Xdata[Nmax] || x < Xdata[0]) return 0;
	 for (int i = 0; i < Nmax; i++) {
		 if (x >= Xdata[i] && x < Xdata[i+1]) {
			  x0 = Xdata[i];  x1 = Xdata[i+1];
			  f0 = Fdata[i];  f1 = Fdata[i+1];
			  F = f0 + (f1-f0) * (x-x0) / (x1-x0);
			  return (F * K); // account of molecule dissociation to atoms
		 } //if
	 } // for
	return 0;
}

void CBTRDoc::OnPlotReionization()
{
	int res;
	if (pDataView->m_rich.GetModify()) {
		res = AfxMessageBox("Data is modified but NOT UPDATED! \n Continue?", 3);
		if (res != IDYES) return;
	}
	CArray<double, double>  ArrX;
	CArray<double, double>  ArrY;
	CString S, spart;
	char name[1024];

	PLOTINFO ReiPlot;
	ReiPlot.Caption = "Beam Re-ionization loss" ;
	ReiPlot.LabelX = "X, m";
	ReiPlot.LabelY = "j+, J0";
	ReiPlot.LimitX = ReionXmax + 0.5;
	ReiPlot.LimitY = 1;
	ReiPlot.CrossX = 5;
	ReiPlot.CrossY = 0.1;
	ReiPlot.Line = 1;
	ReiPlot.Interp = 1;
	ReiPlot.PosNegY = FALSE;
	ReiPlot.PosNegX = FALSE;

		//Summa += Curr;
		//P.X = x; P.Y = Curr; P.Z = exp(-Summa);
	int i;
	double x;
	// double dx = ReionStepL;
	double Curr = 0;
	int Nmax = ReionArray.GetUpperBound(); 
	for (i = 0; i <= Nmax; i++) {
		x = ReionArray[i].X;
		Curr += ReionArray.GetAt(i).Y;
		ArrX.Add(x);
		ArrY.Add(Curr); // H+
	}
	ReiPlot.N1 = ArrX.GetSize();

	double NCurr = 0;
	for (i = 0; i <= Nmax; i++) {
		x = ReionArray[i].X;
		NCurr = ReionArray.GetAt(i).Z;
		ArrX.Add(x);
		ArrY.Add(NCurr); // H0
	}
	ReiPlot.N2 = ArrX.GetSize();

	ReiPlot.N = ArrX.GetSize();
	
	ReiPlot.DataX = &ArrX;
	ReiPlot.DataY = &ArrY;
	
	CPlotDlg dlg;
	dlg.Plot = &ReiPlot;
	dlg.InitPlot();
	if (dlg.DoModal() == IDOK) {
		CFileDialog dlg(FALSE, "txt | * ", "Reion_data.txt", OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"Text file (*.txt) | *.TXT | All Files (*.*) | *.*||", NULL);
		if (dlg.DoModal() == IDOK) {
			FILE * fout;
			strcpy(name, dlg.GetPathName());
			fout = fopen(name, "w");
			//fprintf(fout, m_Text);
			//FILE * fout = fopen("Reion_data.txt", "w");
			fprintf(fout, " Reionization along X-axis \n");
			fprintf(fout, " Gas file <%s> \n Xmin = %g  Xmax = %g ReionPercent = %f\n", 
				GasFileName, ReionXmin, ReionXmax, ReionPercent);
			fprintf(fout, " X   \t\t  J+   \t\t  J0 \n");
			Curr = 0;
			for (i = 0; i <= Nmax; i++) {
				x = ReionArray[i].X;
				Curr += ReionArray.GetAt(i).Y;//H+
				NCurr = ReionArray.GetAt(i).Z;//H0
				fprintf(fout, "  %f \t %f \t %f \n", x, Curr, NCurr); 
			}
			fclose(fout);
			delete dlg;
		} // filedialog OK
	} // Plot.DoModal
	
	ArrX.RemoveAll();
	ArrY.RemoveAll();

}

void CBTRDoc::OnUpdatePlotReion(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(PressureLoaded);
}

void CBTRDoc:: PlotCurrentRates(bool rate) // Neutralization currents / rates
{
	CArray<double, double>  ArrX;
	CArray<double, double>  ArrY;
	double Xmax = NeutrXmax;
	CString S;
	
	PLOTINFO CurrPlot;
	
	int Knucl = TracePartNucl;
	if (Knucl == 0) Knucl = 1;
	S.Format("Neutralization on loaded GAS profile:  Energy = %g keV/nucl",  
		IonBeamEnergy*1000/Knucl,  NeutrXmin);

	if (rate) CurrPlot.Caption = "Neutralization rates";
	else CurrPlot.Caption = S;
	CurrPlot.LabelX = "X,m";
	if (TracePartType == -1) CurrPlot.LabelY = "H+,  H-,  Ho   ";
	else CurrPlot.LabelY = "D+,  D-,  Do   ";
		
	CurrPlot.LimitX = Xmax;//ReionXmax;
	CurrPlot.LimitY = 0;//1.e20;
	CurrPlot.CrossX = 0.5;
	CurrPlot.CrossY = 0;
	CurrPlot.Line = 1;
	CurrPlot.Interp = 1;
	//GasPlot.N = 100; //GField->GasArray.GetSize();
	//GasPlot.N1 = GasPlot.N +1;
	//GasPlot.N2 = GasPlot.N +1;
	CurrPlot.PosNegY = FALSE;
	CurrPlot.PosNegX = FALSE;
	double x, y;
	
	double dx = NeutrStepL; 
	double NL = 0;
	x = NeutrXmin;
	while (x <= Xmax) {//NeutrXmax
		NL += dx * GetPressure(C3Point(x, 0, 0));// NL
		y = GetCurrentRate(x, NL, rate).X; //PosRate
		if (y > 1.e-20) {
			ArrX.Add(x); ArrY.Add(y);
		}
		x += dx;
		
	}
	CurrPlot.N1 = ArrX.GetSize();

	x = NeutrXmin;
	NL = 0;
	while (x <= Xmax) {
		NL += dx * GetPressure(C3Point(x, 0, 0));// NL
		y = GetCurrentRate(x, NL, rate).Y; //NegRate
		if (y > 1.e-20) {
			ArrX.Add(x); ArrY.Add(y);
		}
		x += dx;
		
	}
	
	CurrPlot.N2 = ArrX.GetSize();

	x = NeutrXmin;
	NL = 0;
	while (x <= Xmax) {
		NL += dx * GetPressure(C3Point(x, 0, 0));// NL
		y = GetCurrentRate(x, NL, rate).Z; //NeutrRate
		if (y > 1.e-20) {
			ArrX.Add(x); ArrY.Add(y);
		}
		x += dx;
		
	}

	CurrPlot.N = ArrX.GetSize();
	
	CurrPlot.DataX = &ArrX;
	CurrPlot.DataY = &ArrY;
	
	CPlotDlg dlg;
	dlg.Plot = &CurrPlot;
	dlg.InitPlot();
	//dlg.DoModal();
	if (dlg.DoModal() == IDOK) {
		char name[1024];
		CFileDialog dlg(FALSE, "txt | * ", "Neutr_data.txt", OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"Text file (*.txt) | *.TXT | All Files (*.*) | *.*||", NULL);
		if (dlg.DoModal() == IDOK) {
			FILE * fout;
			strcpy(name, dlg.GetPathName());
			fout = fopen(name, "w");
			
			if (rate) fprintf(fout, " Rates in Neutralization area along X-axis \n");
			else fprintf(fout, " Currents in Neutralization area along X-axis \n");
			fprintf(fout, " Gas file <%s> \n Xmin = %g  Xmax = %g\t  J0 = %f \t J- = %f \t J+ = %f\n", 
				GasFileName, NeutrXmin, NeutrXmax, NeutrPart, (1. - NeutrPart - PosIonPart), PosIonPart);
			fprintf(fout, "  X  \t\t  J+  \t\t   J- \t\t  J0 \n");
			x = NeutrXmin;
			NL = 0;
			C3Point P;
			while (x <= Xmax) {
				NL += dx * GetPressure(C3Point(x, 0, 0));// NL
				P = GetCurrentRate(x, NL, rate); //NegRate
				fprintf(fout, "  %g \t %g \t %g  \t %g \n", x, P.X, P.Y, P.Z);
				x += dx;
			}
			
			fclose(fout);
			delete dlg;//filedlg
		} // filedialog OK
	} // Plot.DoModal

	ArrX.RemoveAll();
	ArrY.RemoveAll();
	
}
void CBTRDoc::OnPlotNeutrefficiency() // -> OnNeutralization -> PlotCurrentRates
{
	//PlotCurrentRates(1);	return;

	int res;
	if (pDataView->m_rich.GetModify()) {
		res = AfxMessageBox("Data is modified but NOT UPDATED! \n Continue?", 3);
		if (res != IDYES) return;
	}
	CArray<double, double>  ArrX;
	CArray<double, double>  ArrY;
	CString S, spart;
	char name[1024];
	switch (TracePartType) {
	case 0: spart = "(e)"; break;
	case -1: spart = "H-"; break;
	case -2: spart = "D-"; break;
	case 1: spart = "H+"; break;
	case 2: spart = "D+"; break;
	case 10: spart = "Ho"; break;
	case 20: spart = "Do"; break;
	default: spart = ""; break;
	}

	int Knucl = TracePartNucl;
	if (Knucl == 0) Knucl = 1;

	S.Format("THICK Neutralization:  %s  Energy = %g keV/nucl", spart,
				 IonBeamEnergy*1000/Knucl,  NeutrXmin);

	PLOTINFO NeffPlot;
	NeffPlot.Caption = S;
	NeffPlot.LabelX = "X,m";
	if (TracePartType == -1) NeffPlot.LabelY = "H+,  H-,  Ho   ";
	else NeffPlot.LabelY = "D+,  D-,  Do   ";
	NeffPlot.LimitX = NeutrXmax + 0.5;
	NeffPlot.LimitY = 1;
	NeffPlot.CrossX = 0.5;
	NeffPlot.CrossY = 0.1;
	NeffPlot.Line = 1;
	NeffPlot.Interp = 1;
	NeffPlot.PosNegY = FALSE;
	NeffPlot.PosNegX = FALSE;

	int i;
	double x;
	C3Point P;
	// double dx = NeutrStepL;
	double PosEff = 0;
	int Nmax = NeutrArray.GetUpperBound(); 
	for (i = 0; i <= Nmax; i++) {
		x = NeutrArray[i].X;
		PosEff = NeutrArray.GetAt(i).Z;
		ArrX.Add(x);
		ArrY.Add(PosEff); // H+
	}
	NeffPlot.N1 = ArrX.GetSize();

	double NegEff = 0;
	for (i = 0; i <= Nmax; i++) {
		x = NeutrArray[i].X;
		NegEff = NeutrArray.GetAt(i).Y;
		ArrX.Add(x);
		ArrY.Add(NegEff); // H-
	}
	NeffPlot.N2 = ArrX.GetSize();

	double AtomEff = 0;
	for (i = 0; i <= Nmax; i++) {
		x = NeutrArray[i].X;
		PosEff = NeutrArray.GetAt(i).Z;
		NegEff = NeutrArray.GetAt(i).Y;
		ArrX.Add(x);
		ArrY.Add(1 - NegEff - PosEff); // Ho
	}

	NeffPlot.N = ArrX.GetSize();
	
	NeffPlot.DataX = &ArrX;
	NeffPlot.DataY = &ArrY;
		
	CPlotDlg dlg;
	dlg.Plot = &NeffPlot;
	dlg.InitPlot();
	if (dlg.DoModal() == IDOK) {
		CFileDialog dlg(FALSE, "txt | * ", "Neutr_data.txt", OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"Text file (*.txt) | *.TXT | All Files (*.*) | *.*||", NULL);
		if (dlg.DoModal() == IDOK) {
			FILE * fout;
			strcpy(name, dlg.GetPathName());
			fout = fopen(name, "w");
			
			fprintf(fout, " Neutralization along X-axis \n");
			fprintf(fout, " Gas file <%s> \n Xmin = %g  Xmax = %g\t  J0 = %f \t J- = %f \t J+ = %f\n", 
				GasFileName, NeutrXmin, NeutrXmax, NeutrPart, (1. - NeutrPart - PosIonPart), PosIonPart);
			fprintf(fout, "  X  \t\t  J+  \t\t   J- \t\t  J0 \n");
			
			for (i = 0; i <= Nmax; i++) {
				x = NeutrArray[i].X;
				PosEff = NeutrArray.GetAt(i).Z;//H+
				NegEff = NeutrArray.GetAt(i).Y;//H-
				AtomEff = 1 - NegEff - PosEff; // Ho
				fprintf(fout, "  %f \t %f \t %f  \t %f \n", x, PosEff, NegEff, AtomEff); 
			}
			fclose(fout);
			delete dlg;
		} // filedialog OK
	} // Plot.DoModal

	ArrX.RemoveAll();
	ArrY.RemoveAll();
	
}


void CBTRDoc::OnUpdatePlotNeutrefficiency(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(PressureLoaded);
	
}

void CBTRDoc::OnUpdatePlotSigmas(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(FALSE);
	
}

void CBTRDoc::OnCloseDocument() 
{
	CDocument::OnCloseDocument();
}


void CBTRDoc::OnUpdateAppExit(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(STOP);
	
}

BOOL CBTRDoc::CanCloseFrame(CFrameWnd* pFrame) 
{
	//int reply = AfxMessageBox(" Report on BTR Exit ", MB_YESNOCANCEL);
	int reply = AfxMessageBox(" Exit BTR? ", MB_OKCANCEL);
	if (reply == IDCANCEL) return FALSE;

	STOP = TRUE;	OnStop();
	logout << "------------ EXIT BTR 5 session -----------------\n";
	CloseLogFile();
		
	//SendReport(FALSE); // don't include input	//for Valid User returns back:

	return CDocument::CanCloseFrame(pFrame);
}


void CBTRDoc::SendReport(bool includetext)
{
/*	unsigned long Size1 = MAX_COMPUTERNAME_LENGTH + 1;
	unsigned long Size2 = MAX_COMPUTERNAME_LENGTH + 1;
	char user[MAX_COMPUTERNAME_LENGTH + 1];
	char machine[MAX_COMPUTERNAME_LENGTH + 1];*/
	CString CompName = GetMachine();
	CString UserName = GetUser();
	CString Body;

	//if (!includetext && !InvUser && UserName.Find("NAS",0) <= 0) return;

	char * server = "smtp.gmail.com";//"nfi.kiae.ru"; ////
	char * from = "INFO@BTR.com"; //"edlougach@nfi.kiae.ru";//"edlougach@gmail.com";//
	char * to = "edlougach@gmail.com";
	//char * to = "kuyanov@nfi.kiae.ru";
	//char * to = "edlougach@gmail.com";// gmail does not accept exe-attachments!
	char subject[1024];
	strcpy(subject, CompName);//machine);
	strcat(subject, " / "); strcat(subject, UserName);

	char body[65555];
	
	if (includetext) {
		strcat(subject, " -> BTR input");
		FormDataText(TRUE);
		strcpy(body, m_Text);
	}
	else { // empty
		strcat(subject, " -> exit");
		CString CurrDate, CurrTime;
		CString Date, Time, version = " - \n 4 beta - unlimited since Jan 2012 \n";
		CTime tm = CTime::GetCurrentTime();// tm.GetDay() GetMonth()
		CurrDate.Format(" date %02d:%02d:%04d", tm.GetDay(), tm.GetMonth(), tm.GetYear());
		CurrTime.Format("  time %02d:%02d:%02d", tm.GetHour(), tm.GetMinute(), tm.GetSecond());
		strcpy(body,("sent by BTR " + version + CurrDate + CurrTime ));
	}

	char * attach = 0;//"BTR.exe";

	sendMail(server, from, to, subject, body, attach);
}

void CBTRDoc::OnSendBtrInput()
{
	//CSMTPConnection smtp;
	//CIMapi mail;
	MapiMessage msg;
	memset(&msg, 0, sizeof(MapiMessage));//size = strTitle.GetLength() + 1;
	msg.lpszSubject = "test subject";
	msg.lpszNoteText = "Test message from OnSendBTRInput";
	msg.lpszMessageType = NULL;
		
	MapiRecipDesc recipient;
	memset(&recipient, 0, sizeof(MapiRecipDesc));
	recipient.ulEIDSize = 0;
	recipient.ulRecipClass = MAPI_TO;
	CString cstrAddress = "edlougach@gmail.com"; //strAddress;
	recipient.lpszAddress = (char*)(const char*)cstrAddress;
	recipient.lpszName = (char*)(const char*)cstrAddress;
	msg.lpRecips = &recipient;
	msg.nRecipCount = 1;
	LPMAPISENDMAIL lpfnMAPISendMail = NULL;
	HINSTANCE hlibMAPI = LoadLibrary("MAPI32.DLL");//Mapi32.dll
	lpfnMAPISendMail = (LPMAPISENDMAIL)GetProcAddress(hlibMAPI, "MAPISendMail");
	//return (*lpfnMAPISendMail)(0, (ULONG)hWnd, &msg, 0, 0);
	//MAPISendMail(0, 0, &msg, 0, 0);
	HWND hWndTop;
	CWnd * pParentWnd = CWnd::GetSafeOwner(NULL, &hWndTop);
	// send mail
	FLAGS flags = MAPI_LOGON_UI;
	//BitSetIf(flags, MAPI_DIALOG, bShowDialog);
	ULONG nError = (*lpfnMAPISendMail)(0, (ULONG)pParentWnd->GetSafeHwnd(), &msg, flags, 0);

	if (nError != SUCCESS_SUCCESS &&
		nError != MAPI_USER_ABORT &&
		nError != MAPI_E_LOGIN_FAILURE)
	{
		AfxMessageBox(AFX_IDP_FAILED_MAPI_SEND);
	}
	//CMailAPI32   m_api;              // MAPI interface
	//m_api.SendMail();
	
	//SendReport(TRUE);
	AfxMessageBox("Message is sent.\n  Thank you!");
}

void CBTRDoc::OnSendOther()
{
	char * server = "smtp.gmail.com";//"nfi.kiae.ru"; ////
	char * from = "BTR@mail.com"; //"edlougach@nfi.kiae.ru";//"edlougach@gmail.com";//
	char * to = "edlougach@gmail.com";//nfi.kiae.ru";
	//char * to = "kuyanov@nfi.kiae.ru";
	//char * to = "edlougach@gmail.com";// gmail does not accept exe-attachments!
	char subject[1024];
	strcpy(subject, "BTR version");
	
	char body[65555];
	CString CurrDate, CurrTime;
	CString Date, Time;
	CString version = " - \n 3.3 May 03 2012 - unlimited since Jan 2012 \n";
	CTime tm = CTime::GetCurrentTime();// tm.GetDay() GetMonth()
	CurrDate.Format(" date %02d:%02d:%04d", tm.GetDay(), tm.GetMonth(), tm.GetYear());
	CurrTime.Format("  time %02d:%02d:%02d", tm.GetHour(), tm.GetMinute(), tm.GetSecond());
	strcpy(body,("This is sent by BTR via gmail \n\n" + CurrDate + CurrTime + version));
	
	char * attach = 0;// "BTR.exe";

	sendMail(server, from, to, subject, body, attach);

}

void CBTRDoc::CreateCopyMessage(CString server)
{/*
   MailAddress^ from = gcnew MailAddress( L"edlougach@nfi.kiae.ru",L"DED" );
   MailAddress^ to = gcnew MailAddress( L"edlougach@nfi.kiae.ru",L"Jane" );
   MailMessage^ message = gcnew MailMessage( from,to );

   // message.Subject = "Using the SmtpClient class.";
   message->Subject = L"BTR message";
   message->Body = L"Using this feature, you can send an e-mail message from an application very easily.";

   // Add a carbon copy recipient.
   MailAddress^ copy = gcnew MailAddress( L"edlougach@gmail.com" );
   message->CC->Add( copy );
   SmtpClient^ client = gcnew SmtpClient( server );

   // Include credentials if the server requires them.
   client->Credentials = CredentialCache::DefaultNetworkCredentials;
   AfxMessageBox(L"Sending an e-mail message by using the SMTP ");
   Console::WriteLine( L"Sending an e-mail message to {0} by using the SMTP host {1}.", to->Address, client->Host );
   client->Send( message );
   client->~SmtpClient();*/
}



void CBTRDoc::OnOptionsSurfacesAdd() 
{
	C3Point P0, P1, P2, P3;
	//BOOL solid = FALSE;
	CPlate * plate = pMarkedPlate; // last selected
	CString S;
	CSurfDlg dlg;
	
	if (plate != NULL) {
		dlg.m_X1 = plate->Corn[0].X; dlg.m_Y1 = plate->Corn[0].Y; dlg.m_Z1 = plate->Corn[0].Z;
		dlg.m_X2 = plate->Corn[1].X; dlg.m_Y2 = plate->Corn[1].Y; dlg.m_Z2 = plate->Corn[1].Z;
		dlg.m_X3 = plate->Corn[2].X; dlg.m_Y3 = plate->Corn[2].Y; dlg.m_Z3 = plate->Corn[2].Z;
		dlg.m_X4 = plate->Corn[3].X; dlg.m_Y4 = plate->Corn[3].Y; dlg.m_Z4 = plate->Corn[3].Z;
		dlg.m_Solid = !plate->Solid;
		dlg.m_N = plate->Number;
		dlg.m_Comment = "free";//+ plate->Comment;
		dlg.m_N = PlateCounter + 1;//AddPlatesNumber;
	}
	else { // plate == NULL
		dlg.m_N = PlateCounter + 1;//AddPlatesNumber;
		dlg.m_Comment = "Type Surf Name here";
		dlg.m_Solid = 0;
	}

	int res = dlg.DoModal();
	
	if (res == IDCANCEL) {//
		if (!dlg.OptRead) return;
		int added = ReadAddPlates();
		AppendAddPlates();
		//ModifyArea(FALSE);
		//AddPlatesNumber += added;
		S.Format("Total addit %d (+ %d)\n %d tot plates ", 
					AddPlatesNumber, added, PlateCounter);
		//AfxMessageBox(S);
		OnShow();
		return;
	}

	if (res == IDOK) {
		P0.X = dlg.m_X1; P0.Y = dlg.m_Y1; P0.Z = dlg.m_Z1;
		P1.X = dlg.m_X2; P1.Y = dlg.m_Y2; P1.Z = dlg.m_Z2;
		P2.X = dlg.m_X4; P2.Y = dlg.m_Y4; P2.Z = dlg.m_Z4;
		P3.X = dlg.m_X3; P3.Y = dlg.m_Y3; P3.Z = dlg.m_Z3;
		
		plate = new CPlate();//(P1, P2, P3, P4);
		plate->SetLocals(P0, P1, P2, P3); //SetArbitrary(P1, P2, P3, P4);
		plate->Solid = !(dlg.m_Solid);
		plate->Comment = dlg.m_Comment;
		plate->Number = dlg.m_N;
		if (plate->Solid && plate->Comment.Find("solid") < 0) plate->Comment += " (solid)";
		//if (plate->Comment.Find("free") < 0 && plate->Comment.Find("add") < 0) plate->Comment += " added";
		//else plate->Comment += " (transpar)";
						
		plate->Fixed = -1; // to define
		plate->Visible = plate->Solid;
		AddPlate(plate); // (solid, P1, P2, P3, P4);//AddPlatesNumber++;
		AppendAddPlates();//PlateCounter++;
		PlatesList.AddTail(plate);
	}

	OnShow();

}

void CBTRDoc:: WriteAddPlates(char * name)
{
	FILE * fout;
	fout= fopen(name, "w");
	if (fout == NULL) {
			AfxMessageBox("failed to create file", MB_ICONSTOP | MB_OK);
			return;
	}
	fprintf(fout, "Free Surfaces List \n");
	fprintf(fout, "Each Surface is defined by 4 Corner Points  (X  Y  Z) \n");
	
	CPlate * plate;
	CString com;
	CString sol;
	C3Point P;
	POSITION pos = PlatesList.GetHeadPosition();
	while (pos != NULL) {// find Plate0 in the existing plates list 
		plate = PlatesList.GetNext(pos);
		com = plate->Comment;
		com.MakeUpper();
		if (com.Find("ADDIT") >= 0 || com.Find("FREE") >= 0) {
			if (plate->Solid) sol = " (solid)";
			else sol = " (transpar)";
			fprintf(fout, "# %s\n", com);
			for (int i = 0; i < 4; i++) {
				P = plate->Corn[i];
				fprintf(fout, "%g    %g    %g\n", P.X, P.Y, P.Z);
			}
		} // additional
	} // list
	fclose(fout);

}

int CBTRDoc:: ReadAddPlatesDir(char * dirname) // read all surf files from folder
{
	int totfiles = 0;
	CString S;
	CString FullPath = dirname; // CurrentDirName + "\\" + dirname;

	::SetCurrentDirectory(FullPath);
	//logout << "Set Folder " << FullPath << "\n";

	DWORD error = ::GetLastError();
	//if (error != 0) { AfxMessageBox("AddSurf Dir not found"); return 0;}

	S.Format(">>>> Reading AddSurf Folder:\n %s ...\n", FullPath);
	logout << S;

	BeginWaitCursor(); 
	WIN32_FIND_DATA FindFileData; //fData;
	HANDLE hFind;
	
	char name[1024];
	CString  sn;
	FILE * fin;
	char buf[1024];
	CString line;
	int pos;
	int n, added = 0; // plates

/*	char OpenDirName[1024];
	::GetCurrentDirectory(1024, OpenDirName);
	//::SetCurrentDirectory(OpenDirName);
	SetTitle(OpenDirName);// + TaskName);*/

  // Find the first file in the directory.
   hFind = FindFirstFile("*.txt", &FindFileData);

   if (hFind == INVALID_HANDLE_VALUE)//   AfxMessageBox("Invalid surf-file handle");
	   { AfxMessageBox("NO txt-file found"); return 0;}
   else  { // valid handle
	   sn = (CString) (FindFileData.cFileName);
	  /* if (sn.Find("load", 0) >= 0) {
		strcpy(name, sn);
		ReadLoad(name);}*/
	   
		strcpy(name, sn);
		S.Format(" - opening file %s\n", name);
		logout << S;
		fin = fopen(name, "r");
		if (fin == NULL) { AfxMessageBox("AddSurf file error"); return 0;}
		if (fgets(buf, 1024, fin) == NULL)	return 0;
		line = buf;
		fclose(fin);
		line.MakeUpper();
		pos = line.Find("SURF");
		if (pos >= 0) {
			n = ReadAddPlates(name);
			added += n;
			totfiles++;// file read 
			S.Format("\t + %d surfaces, total %d \n", n, added);
			logout << S;
		}
  
      while (FindNextFile(hFind, &FindFileData) != 0) 
      {
          sn = (CString) (FindFileData.cFileName);
		 /* if (sn.Find("load", 0) >= 0) {
			  strcpy(name, sn);
			  ReadLoad(name);} // if*/
		  
		  strcpy(name, sn);
		  S.Format(" - opening file %s\n", name);
		  logout << S;
		  fin = fopen(name, "r");
		  if (fin == NULL) continue; // { AfxMessageBox("AddSurf file error"); return totfiles;}
		  if (fgets(buf, 1024, fin) == NULL) continue;	//return totfiles;
		  line = buf;
		  fclose(fin);
		  line.MakeUpper();
		  pos = line.Find("SURF");
		  if (pos >= 0) {
			  n = ReadAddPlates(name);
			  added += n;
			  totfiles++;// file read 
			  S.Format("\t + %d surfaces, total %d \n", n, added);
			  logout << S;
		  }
      } // while next txt-file found
   } //valid handle
     // dwError = GetLastError();
    FindClose(hFind);
	EndWaitCursor();

	S.Format(" - %d Surf Files found with %d surfaces\n", totfiles, added);
	logout << S;// << std::endl;
	S.Format(">> result AddPlatesNumber - %d\n", AddPlatesNumber);// added in Addplate
	logout << S;

	::SetCurrentDirectory(CurrentDirName);
	return totfiles;
}

int CBTRDoc:: ReadAddPlates(char * name)
{
	char buf[1024];
	CString line = "";
	int i, pos, num, res;
	char *endptr;
	CPlate * plate;
	double x, y, z;
	C3Point P[4];
	int total = 0;
	bool solid;
	CString S;
		 
	FILE * fin;
	fin = fopen(name, "r");
		
		if (fin == NULL) {
			S.Format("%s AddSurf-file error, not found at Home\n ..\\ %s \n", 
				name, CurrentDirName);
			AfxMessageBox(S, MB_ICONSTOP | MB_OK);
			logout << S;// << std::endl;
			return 0;
		}

		S.Format("\t - reading file: %s\n",name);
		logout << S;

		total = 0;  
		while (!feof(fin)) {
			if (fgets(buf, 1024, fin) == NULL)
				break;
			line = buf;
			pos = line.Find("#");
			if (pos < 0) continue;
		
			line = line.Mid(pos+1);
			strcpy(buf, line);
			num = strtol(buf, &endptr, 10);
			if (line.Find("solid") > -1)
				solid = true;
			else
				solid = false;
			pos = line.Find("\n");
			if (pos > 0) line = line.Left(pos);
			
			for (i = 0; i < 4; i++) {
				if (fgets(buf, 1024, fin) == NULL) break;
				res = sscanf(buf, "%le %le %le", &x, &y, &z);
				P[i].X = x; P[i].Y = y; P[i].Z = z; 
			}
			plate = new CPlate();//(P[0], P[1], P[2], P[3]);
			plate->Comment = line;// +" Additional";
			if (plate->CheckPoints(P[0], P[1], P[2], P[3]) > 0) {
				AfxMessageBox(line + "\n - ERROR corners");
				continue;
			}
			plate->SetLocals(P[0], P[1], P[2], P[3]);//plate->SetArbitrary(P[0], P[1], P[2], P[3]);
			plate->Solid = solid;
			plate->Visible = solid;
			plate->Fixed = -1; // to define
			plate->Number = -1; // to define when adding to PlatesList
			AddPlate(plate);		
			total++;// addded	
			//if (total > 27) break;
		}// feof
		fclose(fin);
	S.Format(" - Scanned %d Surf \n",	total);
	//logout << S;// << std::endl;
	S.Format(">>>> result AddPlatesNumber - %d\n", AddPlatesNumber);
	//logout << S; // "\t result AddPlatesNumber - " << AddPlatesNumber << std::endl; 
	
	//ShowFileText(name);
	//ModifyArea(FALSE);
	return total;

}

int CBTRDoc:: ReadAddPlates()// read from selected file
{
	char name[1024];
	CString Name;
	int added = 0;
	CFileDialog dlg(TRUE, "txt; dat | * ", "", OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"Addit. Surf. list for BTR (*.txt); (*.dat) | *.txt; *.TXT; *.dat; *.DAT | All Files (*.*) | *.*||", NULL);
	if (dlg.DoModal() == IDOK) {
		Name = dlg.GetPathName();
		strcpy(name, Name);

		logout << "READ Add Surfaces from " + Name + "\n";// << dlg.GetPathName() << std::endl;

		CPreviewDlg dlg;
		dlg.m_Filename = CString(name); //dlg.GetPathName();
		if (dlg.DoModal() == IDCANCEL) {
			logout << " NO Surf added\n";
			return 0;
		}
		//CString Name = CString(name);
		added = ReadAddPlates(name);
		if (added > 0) {
			AddSurfName = Name;
		}
		
	} // OK
	return added;
}

void CBTRDoc::OnOptionsBeam() 
{
	CBeamDlg dlg;
	dlg.doc = this;
	dlg.m_Gauss = (int)BeamSplitType;
	dlg.m_OptSINGAP = !OptSINGAP;
	
	switch (TracePartType) {
		case 0: dlg.m_D = 2; break;
		case 1:
		case -1:
		case 10:dlg.m_D = 1; break;
		case 2: 
		case -2:
		case 20:dlg.m_D = 0; break;
		
		default: dlg.m_D = 0; break;
	}
	
	dlg.m_OptThick = OptThickNeutralization;
	dlg.m_OptRID = !OptNeutrStop;
	dlg.m_Atoms = OptTraceAtoms;
	dlg.m_OptReion = !OptReionStop;
	dlg.m_OptPlasma = OptBeamInPlasma;
	dlg.m_AtomPower = OptAtomPower;
	dlg.m_NegIon_Power = OptNegIonPower;
	dlg.m_PosIon_Power = OptPosIonPower;
	dlg.m_IonPower = OptKeepFalls;
	dlg.m_Reflect = OptReflectRID;
	

	if (dlg.DoModal() == IDOK) {
			OptSINGAP = !(dlg.m_OptSINGAP);
			BeamSplitType = dlg.m_Gauss;
			OptThickNeutralization = (dlg.m_OptThick != 0);
			OptNeutrStop = !dlg.m_OptRID;
			OptTraceAtoms = (dlg.m_Atoms != 0);
			OptReionStop = !dlg.m_OptReion;
			OptAtomPower = (dlg.m_AtomPower != 0);
			OptNegIonPower = (dlg.m_NegIon_Power != 0);
			OptPosIonPower = (dlg.m_PosIon_Power != 0);
			OptBeamInPlasma = TRUE; // (dlg.m_OptPlasma != 0);
			OptReflectRID = (dlg.m_Reflect != 0);
			OptKeepFalls = (dlg.m_IonPower != 0);

		//	TracePartType = 2 - dlg.m_D;
			CheckData();
		
		
			if (OptSINGAP) {
				TaskName = "SINGAP BEAM";
				if (!SINGAPLoaded) SetSINGAP(); // form BEAMLET_ENTRY array (apply midalign, active chan)
				
			}
			else { 
				TaskName = "MAMuG BEAM";
				SetBeam();// Mamug Aimings, IS positions, BeamPower
				for (int i = 0; i < (int)NofChannelsHor; i++) ActiveCh[i] = TRUE;
				for (int j = 0; j < (int)NofChannelsVert; j++) ActiveRow[j] = TRUE;
				//SetStatus();// NofBeamlets, Nofactive channels/rows/totbeamlets	
			}
				
			if ((int)BeamSplitType ==0) SetPolarBeamletCommon(); //SetBeamletAt(0);
			else SetGaussBeamlet(BeamCoreDivY, BeamCoreDivZ); 
			//SetPolarBeamletReion();
			//SetPolarBeamletResidual();

			if (TracePartType == 0) { // trace electrons
				TraceStepL = Min(TraceStepL, 0.01);
				TraceTimeStep = Min(TraceTimeStep, 1.e-10);
			}
	
			SetTraceParticle(TracePartNucl, TracePartQ);
			logout << "Edit Beam Options - DONE \n"; 

	} // IDOK
//	RIDField->Set();

//	SetBeam();  
	if (STOP) //(NofCalculated == 0)
	SetStatus(); // NofBeamlets, Nofactive channels/rows/totbeamlets	
	
	OnShow();
}

void CBTRDoc::OnOptionsFields() 
{
	CFieldsDlg dlg;
	dlg.doc = this;
	dlg.m_MFON = !FieldLoaded; //MFON = 0 if MF is ON 
	dlg.m_MFcoeff = MFcoeff;
	dlg.m_Volt = RIDU;
	dlg.m_GasON = !PressureLoaded;
	dlg.m_GasCoeff = GasCoeff;
	dlg.m_MFfilename = MagFieldFileName;
	dlg.m_Gasfilename = GasFileName;
	
	if (dlg.DoModal() == IDOK) {
		MFcoeff = dlg.m_MFcoeff;
		GasCoeff = dlg.m_GasCoeff;
		if (dlg.m_MFON == 1) MFcoeff = 0; // switched off
		if (dlg.m_GasON == 1) GasCoeff = 0; // switched off

		RIDU = dlg.m_Volt;

		if (dlg.m_MFON == 0 && !FieldLoaded) {// MF is ON but not read
			AfxMessageBox("MF is not read!");
		}
		if (dlg.m_GasON == 0 && !PressureLoaded) {// Gas is ON but not read
			AfxMessageBox("Gas is not read!");
		}
	}

//	RIDField->Set();
	CheckData();
	SetNeutrCurrents();
	SetReionPercent();
	
	/*if (PressureLoaded && GasCoeff > 1.e-6) {
		//if (GasCoeff == 0) GasCoeff = 1;
		OptReionAccount = TRUE;//automatic when gas loaded
		OptThickNeutralization = TRUE;
		SetNeutrCurrents();
		SetReionPercent();
	}
	else {
		OptReionAccount = FALSE;//automatic when gas loaded
		OptReionStop = TRUE;
		OptThickNeutralization = FALSE;
		SetNeutrCurrents();
		SetReionPercent();
	}*/
	OnShow();//	OnDataActive();
}

void CBTRDoc::OnOptionsNBIconfig() 
{
	if (OptFree) return;
	CNBIconfDlg dlg;
	dlg.doc = this;
	dlg.m_CalculWidth = 0;
	dlg.m_AddDuct = OptAddDuct;
	dlg.m_CalOpen = !OptCalOpen;
	dlg.m_NeutrChannels = (int)NeutrChannels;
	dlg.m_RIDChannels = (int)RIDChannels;
	dlg.m_NeutrGapIn = NeutrInW *1000;
	dlg.m_NeutrGapOut = NeutrOutW *1000;
	dlg.m_NeutrThickIn = NeutrInTh *1000;
	dlg.m_NeutrThickOut = NeutrOutTh *1000;
	dlg.m_RIDGapIn = RIDInW *1000;
	dlg.m_RIDGapOut = RIDOutW * 1000;
	dlg.m_RIDThick = RIDTh * 1000;
	dlg.m_Volt = RIDU;

	if (dlg.DoModal() == IDOK) {
		NeutrChannels = dlg.m_NeutrChannels;
		RIDChannels = dlg.m_RIDChannels;
		
		NeutrInW =	dlg.m_NeutrGapIn * 0.001;
		NeutrOutW = dlg.m_NeutrGapOut * 0.001;
		NeutrInTh = dlg.m_NeutrThickIn * 0.001;
		NeutrOutTh = dlg.m_NeutrThickOut * 0.001;
		RIDInW = dlg.m_RIDGapIn * 0.001;
		RIDOutW = dlg.m_RIDGapOut * 0.001;
		RIDTh = dlg.m_RIDThick * 0.001;
		RIDU = dlg.m_Volt;
		OptCalOpen = !(dlg.m_CalOpen);
		//	RIDField->Set();
		if (!OptCalOpen) CalOutW = 0.0;
		else CalOutW = CalInW;
		OptAddDuct = dlg.m_AddDuct;
	
		CheckData();
		//InitPlasma();
		//SetPlates();

		ClearAllPlates();
		//InitPlasma();
		if (!OptFree) 
			SetPlates(); // create again//SetPlasma() is called from SetPlatesNBI;
		else ModifyArea(TRUE);
		
		OnShow();//	OnDataActive();
	}
	

}

void CBTRDoc::OnOptionsSurfacesEnabledisableall() 
{
	// SelectAllPlates();
	
	if (OptParallel) { //parallel
		//CalculateAllLoads(); 
		ShowStatus();
	}//parallel

	else //!(pDoc->OptParallel
		SelectAllPlates();

}

BOOL CBTRDoc::SetDefaultMesh() // mesh + part opt
{
	//CNeutrDlg dlg;
	//dlg.m_Thin = OptThickNeutralization ? 1 : 0;
	DefLoadDlg dlg;
	dlg.m_OptN = DefLoadOptN ? 0 : 1; // TRUE - appr size / False - fixed step
	dlg.NX = DefLoadNX;
	dlg.NY = DefLoadNY;
	dlg.Round = DefLoadStepRound; // round step if optN == TRUE (size)
	dlg.m_StepX = DefLoadStepX;
	dlg.m_StepY = DefLoadStepY;

	dlg.m_OptAtom = (BOOL) OptAtomPower;
	dlg.m_OptPos = (BOOL) OptPosIonPower;
	dlg.m_OptNeg = (BOOL) OptNegIonPower;
	
	if (dlg.DoModal() == IDOK) {
		DefLoadOptN = (dlg.m_OptN == 0);
		DefLoadNX = dlg.NX;
		DefLoadNY = dlg.NY;
		DefLoadStepRound = dlg.Round;
		DefLoadStepX = dlg.m_StepX;
		DefLoadStepY = dlg.m_StepY;

		OptAtomPower = dlg.m_OptAtom;
		OptPosIonPower = dlg.m_OptPos;
		OptNegIonPower = dlg.m_OptNeg;

		logout << "  Set Default Mesh on surfaces\n";
		return TRUE;
	}
	else {
		CString S = " Run Cancelled\n ";
		AfxMessageBox(S);
		logout << S;
		//OnStop();// STOP = TRUE
		return FALSE;
	}
}
void CBTRDoc::SetNullLoads() // init maps for all "interesting" plates
{
	logout << "Set NULL Loads on ALL Surf\n";
	PtrList & List = PlatesList;
	CPlate * plate;
	//SetDefaultMesh();// removed for multi-run 
	ClearAllPlates();// delete all loads
	SetLoadArray(NULL, FALSE); // Load_Arr.RemoveAll();	LoadSelected = 0;
	POSITION pos = List.GetHeadPosition();
	OptCombiPlot = -1; //no map selected
	//CWaitCursor wait;
	while (pos != NULL) {
		plate = List.GetNext(pos);
		plate->Falls.RemoveAll();

		if (plate->MAP) { // only for interesting (MAP)
			SetNullLoad(plate); // create zero load with default mesh option
			//P_CalculateLoad(plate); // distribute m_GlobalVector
			SetLoadArray(plate, TRUE); // Add the plate to Loads List
		}
		else { //all the rest - single cell for Sum power
			// not included to Loads list
			plate->ApplyLoad(TRUE, 1000, 1000); //-> Nx = Ny = 0 Loaded->TRUE
			plate->Load->Sum = 0;
			plate->Load->MaxVal = 0;
		}
		plate->AtomPower = 0;
		plate->NegPower = 0; 
		plate->PosPower = 0;
	}
	// temporary removed
	//pBeamHorPlane->Load->Clear();
	//pBeamVertPlane->Load->Clear();
	
} 

void CBTRDoc::AddFallsToLoads(int tid, int isrc,  std::vector<minATTR> * tattr) 
//bool CBTRDoc::AddFalls
// replace CalculateAllLoads()
// called after Each BML
// Distribute falls among all maps, Add to Sum - for SKIPPED  
{
	//if (STOP) return; // FALSE;
	//vector<minATTR>::iterator it;
	PtrList & List = PlatesList;
	CPlate * plate;
	//SetDefaultMesh();// removed for multi-run 
	//SetLoadArray(NULL, FALSE); // Load_Arr.RemoveAll();	LoadSelected = 0;
	POSITION pos = List.GetHeadPosition();
	OptCombiPlot = -1; //no map selected
	//CWaitCursor wait;
	while (pos != NULL) {
		plate = List.GetNext(pos);
		P_CalculateLoad(plate, tattr); // distribute m_GlobalVector
	}
	
	// temporary removed !!! to restore later if needed
	//pBeamHorPlane->Load->SetSumMax();
	//pBeamVertPlane->Load->SetSumMax();
}

void CBTRDoc::CalculateAllLoads() // called after each RUN or on User request
// Set Zero loads
// calls Falls Distribution
// creates Loads List
{
	PtrList & List = PlatesList;
	CPlate * plate;
	//SetDefaultMesh();// removed for multi-run 
	SetLoadArray(NULL, FALSE); // Load_Arr.RemoveAll();	LoadSelected = 0;
	POSITION pos = List.GetHeadPosition();
	OptCombiPlot = -1; //no map selected
	
	CWaitCursor wait;
	while (pos != NULL) {
		plate = List.GetNext(pos);
		if (plate->Touched == TRUE) { // only non-zero loads 
			SetNullLoad(plate); // create zero load with default mesh option
			P_CalculateLoad(plate); // distribute m_GlobalVector
			SetLoadArray(plate, TRUE); // Add the plate to Loads List
		}
	}
//int res = AfxMessageBox(S, MB_ICONQUESTION | MB_YESNOCANCEL); // question removed
//int res = AfxMessageBox("Calculate All non-zero maps", MB_ICONEXCLAMATION | MB_OKCANCEL); // question removed!
}

double CBTRDoc::GetInjectedPowerW() // calculate Atom power at Duct Exit
{
	double Pinj = 0;
	CString S;
	int n = PlasmaEmitter;
	CPlate * plate = GetPlateByNumber(n);
	if (plate != NULL) Pinj = plate->AtomPower;
	//else AfxMessageBox("PlasmaEmitter = NULL");

	if (Pinj < 1.e-6) { // no data
		n = DuctExit; // try another
		plate = GetPlateByNumber(n);
		if (plate != NULL) { 
			Pinj = plate->AtomPower;
			PlasmaEmitter = DuctExit;
		}
		if (Pinj < 1.e-6) {
			S.Format("Scen %d Run %d \n- failed to get Pinj\n", SCEN, RUN);
			//AfxMessageBox(S);
			logout << "!!!  " << S;
		}
	}
	return Pinj;
}

double CBTRDoc::GetNeutrPower() // calculate neutral power at Neutralizer Exit
{
	double Pn = 0;
	CString S;
	int n = 7; //Neutr exit
	CPlate * plate = GetPlateByNumber(n);
	if (plate != NULL) Pn = plate->AtomPower;
	
	if (Pn < 1.e-6) { // no data
		S.Format("Scen %d Run %d \n- failed to get Neutralized power\n", SCEN, RUN);
		//AfxMessageBox(S);
			logout << "!!!  " << S;
		}
	return Pn;
}

double CBTRDoc::GetTotSolidPower() // sum power falled on solids
{
	double Ptot = 0;
	PtrList & List = PlatesList;
	CPlate * plate;
	double Pow = 0;
	POSITION pos = List.GetHeadPosition();
	//CWaitCursor wait;
	while (pos != NULL) {
		plate = List.GetNext(pos);
		if (plate->Number == 1 || plate->Number == PlasmaEmitter) continue; 
		// skip AreaExit from solids
		//if (plate->Loaded == FALSE) continue;
		if (plate->Touched && plate->Solid) { // only non-zero loads on solids
			//Ptot += plate->Load->Sum;
			Pow = plate->AtomPower + plate->NegPower + plate->PosPower;
			Ptot += Pow;
		}
	}
	return Ptot;
}

int CBTRDoc::GetCompSolid(CStringArray & keys, double & SumPower, double & MaxPD)
{
	SumPower = 0;
	MaxPD = 0;
	double Pow = 0;
	double Ptot = 0;
	double PDmax = 0;
	
	PtrList & List = PlatesList;
	CPlate * plate;
	CString S;
		
	POSITION pos = List.GetHeadPosition();
	int kmax = keys.GetSize();
	int found = -1;
	int count = 0; // surf found
	
	while (pos != NULL) {
		plate = List.GetNext(pos);
		S = plate->Comment.MakeUpper();
		for (int k = 0; k < kmax; k++) {
			found = S.Find(keys[k],0);
			if (found < 0) break; //for
		}
		if (found < 0) continue; // while pos
		// else - scan solid surf

		if (plate->Touched && plate->Solid) { // only non-zero loads on solids
			//Ptot += plate->Load->Sum;// for MAP only
			Pow = plate->AtomPower + plate->NegPower + plate->PosPower;//include NOMAP surf
			Ptot += Pow;
			if (plate->MAP) PDmax = max(PDmax, plate->Load->MaxVal);
			count++;
		}
	}// while
	
	SumPower = Ptot;
	MaxPD = PDmax;
	return count;
}

void CBTRDoc::GetCompTransparent(CStringArray & keys, double & Pentry, double & Pexit)
{
	Pentry = 0;
	Pexit = 0;

	double Pow = 0;
	PtrList & List = PlatesList;
	CPlate * plate;
	POSITION pos = List.GetHeadPosition();
	int kmax = keys.GetSize();
	int found = -1;
	CString S;
	
	while (pos != NULL) {
		plate = List.GetNext(pos);
		S = plate->Comment.MakeUpper();
		for (int k = 0; k < kmax; k++) {
			found = S.Find(keys[k],0);
			if (found < 0) break; //for
		}
		if (found < 0) continue; // while pos
		// else - scan transp surf

		if (plate->Touched && !(plate->Solid)) { // only transparent
			//Ptot += plate->Load->Sum;// for MAP only!
			Pow = plate->AtomPower + plate->NegPower + plate->PosPower;//include NOMAP surf
			if (S.Find("ENTR",0) > -1) Pentry = Pow;
			else if (S.Find("EXIT",0) > -1) Pexit = Pow;
		}
	}// while
}

////////////////// PDP /////////////////////////////////////////
int OldPDPexe(char * name)
{
	CString S;
	int size;
	unsigned long sizeH[1024];
//	FILE * f = fopen(name, "r");
	size = GetCompressedFileSize(name, sizeH);
//	size = GetFileSize(f, sizeH);
//	fclose(f);
	
	if (size != 278528){//PDP10 - 279040
		S.Format("BTR warning:\nFile Size (%d byte) \n not matching the last-known PDP version!", size);
		AfxMessageBox(S);
		return 1; // OLD
	}
	else return 0;// NEW ?
}

void CBTRDoc::OnTasksPDP() 
{
	char OpenDirName[1024];
	char name[1024];// exe
	char iname[1024];
	char bname[1024];
	CString S;

//	CFileDialog dlg(TRUE, "exe; exe", "PDP executable");
	CFileDialog dlg(TRUE, "exe | * ", "", OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"PDP executable (*.exe) | *.exe; *.EXE | All Files (*.*) | *.*||", NULL);
	
	if (dlg.DoModal() == IDOK) {
		strcpy(name, dlg.GetPathName());
		::GetCurrentDirectory(1024, OpenDirName);
		CurrentDirName = OpenDirName; 
//		SetTitle(OpenDirName);
//		SetTitle(name);
		strcpy(iname, "_Input_PDP_BTR.txt"); //PDPgeomFileName); // default name!
			
		BOOL OldExec = OldPDPexe(name);
		BOOL OldInput = OldPDPinput(iname);

		if (OldInput < 0) {
			AfxMessageBox("Error: PDP-Input failed!"); return;
		}

	/*	if (OldExec && OldInput)  S.Format("OLD versions of PDP-code and PDP-input file!\n\n Continue?");
		if (OldExec && !OldInput) S.Format("Versions mismatch!\n OLD pdp.exe\n and NEW pdp-Input file!\n\n Continue?");
		if (!OldExec && OldInput) S.Format("Versions mismatch!\n NEW pdp.exe\n and OLD pdp-Input file!\n\n Continue?");
*/
		if (OldExec || OldInput) {
			S.Format("BTR warning:\n Versions of PDP.exe and PDP-input\n may not fit each other!\n\n Run PDP Anyhow?");  
			int res = AfxMessageBox(S, 3);
			if (res != IDYES && res != IDOK) return;
		}

	/*	CPreviewDlg pdlg;
		pdlg.m_Filename = iname;//PDPgeomFileName;
		if (pdlg.DoModal() == IDCANCEL) return;
	*/
		PDPgeomLoaded = FALSE;
	
		if (ReadPDPinput(iname) == 0) {
				AfxMessageBox("PDP-input cancelled"); return; 
		} //fail
	
		else { // data loaded
			PDPgeomLoaded = TRUE; // scanned
			CheckData();
			if (!OptFree) { AdjustAreaLimits();	SetPlates();}
			SetReionPercent();
			OnShow();
			ShowFileText(iname); 	
		}


		OptSINGAP = TRUE; // 
		
	    int res = 0;
		strcpy(bname, "_Beamlet_Regular_Array.txt");
		
		res = ReadBeamletsNew(bname);
		if (res != 0) {
			SINGAPLoaded = TRUE;  
		//	ShowFileText(bname);	
		}
		//ReadSINGAP(); //->SINGAPLoaded + Regularity Check
		SetSINGAP();
	if (!OptSINGAP) { // MAMuG
		SetBeam(); 
		for (int i = 0; i < (int)NofChannelsHor; i++) ActiveCh[i] = TRUE;
		for (int j = 0; j < (int)NofChannelsVert; j++) ActiveRow[j] = TRUE;
		SetStatus(); 
	}

	if ((int)BeamSplitType ==0) SetPolarBeamletCommon(); //SetBeamletAt(0);
		else SetGaussBeamlet(BeamCoreDivY, BeamCoreDivZ);
				
		OnShow();

		OptPDP = TRUE;
	/*	int res = CorrectPDPInput();
		if (res)*/
		CorrectFile(iname);//remove comments from PDP-input
		RunPDP(name); // PDP.exe
		OptPDP = FALSE;
	
	}
}

int CBTRDoc:: RunPDP(char * name)
///	BOOL RUN_SELECTED_APP(int n) from APPEAL.CPP
{
	STARTUPINFO st;
	PROCESS_INFORMATION pi;
	// const int MY_MESS = 1000;
	HANDLE HActiveProcess = NULL;
	//HICON Icon;

	BOOL i = 0;
	st.cb = 68;
	st.lpReserved = 0;
	st.lpDesktop = 0;
	st.lpTitle = 0; // 0- for windows app, name - for console app
	st.dwFlags = STARTF_USESHOWWINDOW | STARTF_USEPOSITION | STARTF_USESIZE | STARTF_USEFILLATTRIBUTE;
	st.dwX = 50;
	st.dwY = 100;
	st.dwXSize = 800;
	st.dwYSize = 800;
	st.cbReserved2 = 0;
	st.lpReserved2 = 0;
	st.wShowWindow = SW_SHOWDEFAULT;//SW_SHOWNORMAL;
	st.dwFillAttribute = FOREGROUND_INTENSITY |// FOREGROUND_BLUE | FOREGROUND_RED | 
		BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY;//red text on white bk

	if (HActiveProcess == NULL) { // No process started	

			/*	i = CreateProcess(NULL, "D:\\Users\\Jane\\BTR ITER\\Release\\BTR.exe", 0,0,0,0,0,
								"D:\\Users\\Jane\\BTR ITER\\Release", &st, &pi);*/
		/*	i = CreateProcess(NULL, name, 0,0,0,0,0, CurrentDirName, &st, &pi);
			HActiveProcess = pi.hProcess;*/
			
		/*	i = CreateProcess(NULL, "A_pdp_LR.exe", 0,0,0,
				CREATE_NEW_CONSOLE,0, NULL, &st, &pi);*/
			i = CreateProcess(NULL, name, 0,0,0, CREATE_NEW_CONSOLE,0, NULL, &st, &pi);
			HActiveProcess = pi.hProcess;
		
			WaitMessage();
		
			OptPDP = TRUE;


	/*	case 1: 
			i = CreateProcess(NULL, "C:\\Program Files\\Far\\Far.exe", 0,0,0, CREATE_NEW_CONSOLE,0,
								"C:\\Program Files\\Far", &st, &pi);
			HActiveProcess = pi.hProcess;
			break;*/
//	}
	}

	else { // HActiveProcess != NULL Process Exists 
	//	GetExitCodeProcess(HActiveProcess, ExitCode);
	//	if (*ExitCode != 0) 
		
		TerminateProcess(HActiveProcess, 0);
		
		HActiveProcess = NULL;
//		OptPDP = FALSE;
		
	}
	
	BOOL finished = FALSE;
	DWORD lpEC;
	MSG message; 

//	WaitForSingleObject(HActiveProcess, INFINITE); // suspends parent process!
	while (!finished) {
	//	WaitMessage();
		::PeekMessage(&message, NULL, 0,0, PM_REMOVE); 
		::TranslateMessage(&message);
		::DispatchMessage(&message);
		GetExitCodeProcess(HActiveProcess, &lpEC);
  				//HANDLE hProcess,     // handle to the process
				//LPDWORD lpExitCode   // address to receive termination status
		if (lpEC != STILL_ACTIVE) finished = TRUE; 
	}
		
	
	ShowFileText("1_Hnbl_dat.txt"); // Show PDP Output file "1_Hnbl_dat.txt"

	
	return i;
	
}

void CBTRDoc::StartAtomsFromEmitter()// trace atoms in plasma
{
	pPlasma->SetZeroCuts();// !!!!

	int n = PlasmaEmitter;
	if (n < 0) { 
		AfxMessageBox("Emitter is not defined"); return;
	}
	CPlate * plate = GetPlateByNumber(n);
	CString S;
	//vector<minATTR> & arr = m_GlobalVector;// m_AttrVector[MaxThreadNumber - 1];
	CArray<minATTR> & falls = plate->Falls;
	int Nfalls = falls.GetSize();
	if (Nfalls < 1) {
		S.Format("No start-points (falls) on Emitter Surf %d", n);
		AfxMessageBox(S); logout << S;
		return;
	}

	C3Point Pgl, Ploc, Vat, Vgl;
	double power;
	double stepL = 0;// (pPlasma->Rmax - pPlasma->Rmin) / pPlasma->Nx / 2;
	if (stepL < 1.e-6) stepL = 0.01;
	int found = 0;// count atoms
	//double Mass = pDoc->TracePartMass;
	//double Vstart = sqrt(2.* EkeV*1000 * Qe / Mass);

		for (int i = 0; i < Nfalls; i++) {
			minATTR & tattr = falls[i];
			//if (tattr.Nfall == n) { //plate->Number - always
				if (tattr.Charge != 0) continue; // trace only atoms!!!

				found++;
				power = tattr.PowerW;
				Ploc.X = tattr.Xmm * 0.001;// plate->GetLocal(Pgl);
				Ploc.Y = tattr.Ymm * 0.001;
				Ploc.Z = 0;
				Vat.X = tattr.AXmrad * 0.001;
				Vat.Y = tattr.AYmrad * 0.001;
				Vat.Z = 1;// not used
				Pgl = plate->GetGlobalPoint(Ploc);
				Vgl = plate->GetGlobalVector(Vat.X, Vat.Y); // * Vstart
				
				pPlasma->SetPathFrom(Pgl, Vgl, stepL); // Path[]
				pPlasma->SetDecayArrays(); // fill Power, PSI, IonRate normalised 
				//pPlasma->AddPowerToCutPlanes(power);// account for start power!!!
				pPlasma->AddIonRateToCutPlanes(power);// account for start power!!!
				
				//Pstart = C3Point(Xstart, Ystart, Zstart);
				//Pfin = C3Point(Xfin, Yfin, Zfin);
				//pPlasma->SetPath(Pstart, Pfin, StepL);
				//pPlasma->SetDecayArrays();// thin ray
				//pPlasma->AddDecayArrays();// Nrays++
					
			//} // Nfall == plate Num
		} // i - scan falls attr 

		S.Format("%d atoms traced from Emitter plane %d\n", found, PlasmaEmitter); 
		AfxMessageBox(S);
		logout << S;

		TracksCalculated = 1;

}

void CBTRDoc::ShowBPdata(int n, int lines, int pos, int angle, int power) // show selected options
{
	m_Text.Empty();// = "";
	CString S, s, Ssort;
	COLORREF color;
	CPlate * plate; // = pMarkedPlate;
	int optN;
	//CPlate * plateN = pMarkedPlate;// if selected (n > 0)
	//CPlate * plate;// not selected -> all
	
	CArray<minATTR, const minATTR &> arr;// collect falls from plates tp temp arr

	if (n < 0) { // not selected
		optN = PlasmaEmitter; // 
	}
	else optN = n;
	plate = GetPlateByNumber(optN);// >=0
	S.Format("Show Falls on Emitter Surf %d\n", optN);
	AfxMessageBox(S); logout << S;

	//if (n >= 0) { // single plate
		//vector <minATTR> & arr = plate->Falls; // m_GlobalVector;// m_AttrVector[MaxThreadNumber - 1];
	
	arr.Append(plate->Falls);
	int Nfalls = arr.GetSize();
	//long long Nfalls = plate->Falls.GetSize(); //m_GlobalVector.size();
	
	if (Nfalls < 1) {
		S.Format("Zero start-points (Falls) on Emitter Surf %d !!!\n", optN);
		AfxMessageBox(S); logout << S;
		return;
	}

	C3Point Pgl, Ploc, Vat;
	double PowerW;
	double SumA = 0; 
	double Axy, minA = 100, maxA = 0;

	Ssort = " ";
	if (OptAtomPower) Ssort += "A";
	if (OptNegIonPower) Ssort += "/N";
	if (OptPosIonPower) Ssort += "/P";
	if (n >= 0) {
		S.Format("Particles data on Surf %d (in local CS) (%s) - %d lines\n", n, Ssort, lines);
		m_Text += S;
		C3Point ortX = plate->OrtX;
		C3Point ortY = plate->OrtY;
		C3Point ortZ = plate->OrtZ;
		S.Format("Surf loc vectors:  X (%g, %g, %g), Y(%g, %g, %g), Normal (%g, %g, %g)\n",
			ortX.X, ortX.Y, ortX.Z, ortY.X, ortY.Y, ortY.Z, ortZ.X, ortZ.Y, ortZ.Z);
		m_Text += S;
	}
	else {
		S.Format("Particles data (in local CS) across ALL surfs (%s) - %d lines \n", Ssort, lines);
		//if (n >= 0)	S.Format("Particles data on Surf %d (in local CS) - %d lines\n", n, lines);
		//else S.Format("Particles data (in local CS) across ALL surfs  - %d lines \n", lines);
		m_Text += S;
	}
	S.Format("Xlocal, Ylocal are in [m]\t  AXfall = VXlocal / Vabs, AYfall = VYlocal / Vabs \n\n");
	m_Text += S;
		
	//Pgl = plate->GetGlobalPoint(Ploc);
	//S.Format("Num  Sort\t Xlocal  Ylocal\t AXfall  AYfall\t Power,W\t #surf\n");
	S = " Num   Sort  ";
	if (pos == 1 || pos == 2) S += " Xlocal  Ylocal  "; // 1 - local 3 - global 
	if (pos == 2 || pos == 3) S += " Xglob  Yglob  Zglob  "; // 2 - local + global pos
	//if (angle == 1) S += "Polar Angle\t";
	if (angle > 0) S += "  AXfall  AYfall   ";
	if (power > 0) S += "  Power,W\t ";
	S += "  #surf\n";
	m_Text += S;

	int arrsize = Nfalls; // (int)arr.size();
	int lim = min(lines, arrsize);
	if (lim < 1) {
		S.Format("\n ----------- No data found ---------------\n");
		m_Text += S;
	}

	int i = 0; // written lines
	int k = -1; // array index
	while (i < lim && k < arrsize - 1) {
		k++;

		minATTR & mattr = arr[k]; // collected from all

		//if (n >= 0 && mattr.Nfall != n) {// single surf selected
		if (mattr.Nfall != optN) // wrong fall N
			continue; //pass without i++
		
		else { // -1 or correct fall number -> write
			if (!OptAtomPower   && mattr.Charge == 0) continue;
			if (!OptNegIonPower && mattr.Charge < 0) continue;
			if (!OptPosIonPower && mattr.Charge > 0) continue;
			
			PowerW = mattr.PowerW;
			Ploc.X = mattr.Xmm * 0.001;// plate->GetLocal(Pgl);
			Ploc.Y = mattr.Ymm * 0.001;
			Ploc.Z = 0;
			Vat.X = mattr.AXmrad * 0.001; //AXmrad = (short)(ax * 1000);
			Vat.Y = mattr.AYmrad * 0.001; //AYmrad = (short)(ay * 1000);
			Vat.Z = 1;

			Axy = sqrt(Vat.X * Vat.X + Vat.Y * Vat.Y);
			minA = min(minA, Axy);
			maxA = max(maxA, Axy);
			SumA += Axy;
			
			//if (n >=0) 
			//Pgl = plate->GetGlobalPoint(Ploc);// one
			//else { // all plates
			//plate = GetPlateByNumber(mattr.Nfall);//plate = GetPlateByNumber(optN);// >=0
			if (plate == NULL) Pgl = C3Point(-1, -1, 0); // undefined
			else Pgl = plate->GetGlobalPoint(Ploc);
			//}

			switch (mattr.Charge) {
			case -1: color = RGB(0, 255, 0); Ssort = "N"; break;
			case  0: color = RGB(0, 0, 255); Ssort = "A"; break;
			case  1: color = RGB(255, 0, 0); Ssort = "P"; break;
			default: color = RGB(0, 0, 0); Ssort = "U";
			}

			S = " ";
			//S.Format("%5d  %s\t %6.3f %6.3f\t %6.3f %6.3f\t %9.2e\t %3d\n",
				//i, Ssort, Ploc.X, Ploc.Y, Vat.X, Vat.Y, PowerW, tattr.Nfall);
			s.Format("%5d  %s   ", i, Ssort); // "Num   Sort\t";
			S += s;
			if (pos == 1 || pos == 2) {
				s.Format("%6.3f %6.3f\t", Ploc.X, Ploc.Y); // " Xlocal   Ylocal\t "; // 1 - local pos
				S += s;   
			}
			if (pos == 2 || pos == 3) {
				s.Format("%6.3f %6.3f %6.3f\t", Pgl.X, Pgl.Y, Pgl.Z); // " Xglob   Yglob\t "; // 1 - local pos
				S += s;   
			}
			if (angle > 0) {
				s.Format("%6.3f %6.3f\t", Vat.X, Vat.Y); // " AXfall  AYfall\t ";
				S += s;
			}
			if (power > 0) {
				s.Format("   %9.2e\t", PowerW); //" Power,W\t ";
				S += s;
			}
			s.Format("%  3d\n", mattr.Nfall); // " #surf\n";
			S += s;
			m_Text += S;
			
			i++; // line written

		} // nfall = correct plate number
				 
	} // Falls size 
	double AverA = SumA / i; // div by lines written
	S.Format("Average Vt/Vabs (<sin> angle from normal) %g\n", AverA); 
	m_Text += S;

	double Arad = asin(AverA);
	double Adeg = Arad * 180 / PI;
	double Amin = asin(minA) * 180 / PI;
	double Amax = asin(maxA) * 180 / PI;
	S.Format("Average angle from normal: %g rad = %g deg\n", Arad, Adeg);
	m_Text += S;
	S.Format("Min/Max: %g / %g, deg\n", Amin, Amax);
	m_Text += S;

	arr.RemoveAll();

	pDataView->m_rich.SetFont(&pDataView->font, TRUE);
	pDataView->m_rich.SetBackgroundColor(FALSE, RGB(255, 187, 130)); // (250, 250, 180)); // G230
	pDataView->m_rich.SetWindowText(m_Text);
	pDataView->m_rich.SetModify(FALSE);
}


void CBTRDoc::ShowBPdata(int n, int lines) // show statistics on Marked Surf in orange panel
// not called now
{
	m_Text.Empty();// = "";
	CString S, Ssort;
	COLORREF color;
	
	vector <minATTR> & arr = m_GlobalVector;// m_AttrVector[MaxThreadNumber - 1];
		
	C3Point Pgl, Ploc, Vat;
	double power;
	//CPlate * plate = pMarkedPlate;
	//if (plate == NULL) return;
	Ssort = " ";
	if (OptAtomPower) Ssort += "A";
	if (OptNegIonPower) Ssort += "/N";
	if (OptPosIonPower) Ssort += "/P";
	if (n >= 0)	S.Format("Particles data on Surf %d (in local CS) (%s) - %d lines\n", n, Ssort, lines);
	else S.Format("Particles data (in local CS) across ALL surfs (%s) - %d lines \n", Ssort, lines);
	m_Text += S;
	S.Format("Xlocal, Ylocal are in [m]\t  AXfall = VXloc / modV, AYfall = VYloc / modV \n\n");
	m_Text += S;
	S.Format("Num\t Sort\t Xlocal\tYlocal\tAXfall\tAYfall\t Power,W\t #surf\n");
	m_Text += S;
	int arrsize = (int)arr.size();
	int lim = min(lines, arrsize);
	if (lim < 1) {
		S.Format("\n ----------- No data found ---------------\n");
		m_Text += S;
	
	}
		
	//for (int i = 0; i < lim; i++) {
	int i = 0; // written lines
	int k = -1; // array index
	while (i < lim && k < arrsize-1) {
		k++;
		minATTR &tattr = arr[k];
		if (n >= 0 && tattr.Nfall != n) {// single surf selected
			continue;
		}
		else { // all plates or true number
			if (!OptAtomPower && tattr.Charge == 0) {
				continue;
			}
			if (!OptNegIonPower && tattr.Charge < 0) {
				continue;
			}
			if (!OptPosIonPower && tattr.Charge > 0) {
				continue;
			}
			power = tattr.PowerW;
			Ploc.X = tattr.Xmm * 0.001;// plate->GetLocal(Pgl);
			Ploc.Y = tattr.Ymm * 0.001;
			Ploc.Z = 0;
			Vat.X = tattr.AXmrad * 0.001;
			Vat.Y = tattr.AYmrad * 0.001;
			Vat.Z = 1;

			switch (tattr.Charge) {
			case -1: color = RGB(0, 255, 0); Ssort = "NEG "; break;
			case  0: color = RGB(0, 0, 255); Ssort = "ATOM"; break;
			case  1: color = RGB(255, 0, 0); Ssort = "POS "; break;
			default: color = RGB(0, 0, 0); Ssort = "UNDEFINED PARTICLE";
			}

			S.Format("%5d\t %s\t%6.3f\t%6.3f\t %6.3f\t%6.3f\t %9.2e\t %3d\n",
				i, Ssort, Ploc.X, Ploc.Y, Vat.X, Vat.Y, power, tattr.Nfall);
			m_Text += S;
			i++; // line written

		} // nfall = plate number

		/*if (::PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) {
			::TranslateMessage(&message);
			::DispatchMessage(&message);
		}*/
	} // Global vector size 
	  
	pDataView->m_rich.SetFont(&pDataView->font, TRUE);
	pDataView->m_rich.SetBackgroundColor(FALSE, RGB(255, 187, 130)); // (250, 250, 180)); // G230
	pDataView->m_rich.SetWindowText(m_Text);
	pDataView->m_rich.SetModify(FALSE);
}
void CBTRDoc:: ShowReport()
{
	m_Text.Empty();// = "";
	CString S;  //m_Text += "--- press F2 to return to BTR Input list! ---\n\n";
	char buf[1024];
	char * name;

	::GetCurrentDirectory(1024, buf);
	S.Format("Current Folder  - %s\n", buf);
	m_Text += S;
	
	CStringArray foundnames;
	int found = GetFilesSubstr("REPORT", foundnames);
	if (found == 0)
		m_Text += " NO REPORT file found \n";
		
	else {
		for (int i = 0; i < found; i++) {
			S = foundnames[i];
			m_Text += S; m_Text += "\n";
		}
		
		S = foundnames[0];// SHOW the 1st REPORT found
		m_Text += S; m_Text += " is SHOWN --------------------------\n";
		
		name = strcpy(buf, S);
		FILE * fin = fopen(name, "r");
		while (!feof(fin)) {
			fgets(buf, 1024, fin);
			if (!feof(fin))	m_Text += buf;
		}
		fclose(fin);
	}
	pDataView->m_rich.SetFont(&pDataView->font, TRUE);
	pDataView->m_rich.SetBackgroundColor(FALSE, RGB(255, 187, 130)); 
	pDataView->m_rich.SetWindowText(m_Text);
	pDataView->m_rich.SetModify(FALSE);
}

void CBTRDoc::ShowLogFile(int lines) // show log part. if lines == 0 -> show all
{
	m_Text.Empty();// = "";
	CString S;  //m_Text += "--- press F2 to return to BTR Input list! ---\n\n";
	m_Text += "LOG-file current state...\n";
	if (!OptLogSave) m_Text += "Log-file output is switched OFF!\n";

	else {
		S = LogFilePath + "\\" + LogFileName;
		char * name;
		char buf[1024];
		name = strcpy(buf, S);
		FILE * fin = fopen(name, "r");
		while (!feof(fin)) {
			fgets(buf, 1024, fin);
			if (!feof(fin))	m_Text += buf;
		}
		fclose(fin);
	}
	pDataView->m_rich.SetFont(&pDataView->font, TRUE);
	pDataView->m_rich.SetBackgroundColor(FALSE, RGB(250, 250, 180)); // G230
	pDataView->m_rich.SetWindowText(m_Text);
	pDataView->m_rich.SetModify(FALSE);
}

void CBTRDoc:: ShowFileText(char * fname)
{
	FILE * f;
	//	char * fname = "1_Hnbl_dat.txt";
		char buf[1024];
		f = fopen(fname, "r");
		if (f == NULL) return;
		m_Text.Empty();// = "";
		m_Text += "...... press F2 to return to BTR Input list! .......\n\n";
		m_Text += "***** ";
		m_Text += fname;
		m_Text += " **********\n";
		while (!feof(f)) {
			fgets(buf, 1024, f);
			if (!feof(f))	m_Text += buf;
		}
		fclose(f);
		pDataView->m_rich.SetFont(&pDataView->font, TRUE);
		pDataView->m_rich.SetBackgroundColor(FALSE, RGB(255, 187, 130)); //RGB(250,180,180));
		pDataView->m_rich.SetWindowText(m_Text);
		pDataView->m_rich.SetModify(FALSE);
		SetTitle(fname);
}

void CBTRDoc::ReadPlasmaPSI()
{
	char name[1024];
	CString S;
	CFileDialog dlg(TRUE, "txt; dat | * ", "", OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"EQDSK format (*.txt);(*.dat) | *.txt; *.TXT; *.dat; *.DAT| All Files (*.*) | *.*||", NULL);

	if (dlg.DoModal() == IDOK) {
		strcpy(name, dlg.GetPathName());
		CWaitCursor wait;
		CPreviewDlg pdlg;
		pdlg.m_Filename = name;
		if (pdlg.DoModal() == IDCANCEL) return;
		SetTitle(name);// + TaskName);

		BOOL isread = pPlasma->ReadPSI(name);
		if (!isread) {
			AfxMessageBox("PSI not read \n Default model is active");
			pPlasma->PSIloaded = FALSE; // use simple model R/Rnorm
			//return;
		}

		OnPlotPSI(); // anyway
		PSILoaded = pPlasma->PSIloaded;
		return;

		//CPlate * plate0 = new CPlate();
/*		double stepR = pPlasma->StepR;
		double stepZ = pPlasma->StepZ;
		double DR = pPlasma->Rmax - pPlasma->Rmin;
		double DZ = pPlasma->Zmax - pPlasma->Zmin;
		//S.Format("R/Z steps %g /%g   DR/DZ  %g /%g", stepR, stepZ, DR, DZ);	AfxMessageBox(S);
		CLoad * load = new CLoad(DR, DZ, stepR, stepZ);
		//S.Format("load limits Nx %d Ny %d", load->Nx, load->Ny);	AfxMessageBox(S);

		load->Val = pPlasma->PSI;
		load->SetSumMax();
		if (load->MaxVal < 1.e-10) {
			AfxMessageBox("Zero load (PSI array)");
			return;
		}
		//S.Format("PSImax = %g", load->MaxVal);	AfxMessageBox(S);
		load->iProf = (int)(0.5 * load->iMaxVal);
		load->jProf = (int)(0.5 * load->jMaxVal);
		pMarkedPlate = new CPlate();
		C3Point LB(1000, pPlasma->Rmin, pPlasma->Zmin);
		C3Point RT(1000, pPlasma->Rmax, pPlasma->Zmax);
		pMarkedPlate->SetFromLimits(LB, RT);
		pMarkedPlate->Load = load;
		//pMarkedPlate->Load->SetSumMax();
		pMarkedPlate->Number = 1000;
		pMarkedPlate->Selected = TRUE;
		pMarkedPlate->Loaded = TRUE;
		pMarkedPlate->SmLoad = new CLoad(DR, DZ, stepR, stepZ);
		pMarkedPlate->SmoothDegree = SmoothDegree;
		pMarkedPlate->filename = name;
		pMarkedPlate->Comment = "PSI(R,Z)";
		OptParallel = 0;
		OptCombiPlot = 0;
		pLoadView->Cross.X = load->iProf * load->StepX;
		pLoadView->Cross.Y = load->jProf * load->StepY;
		ShowFileText(name);
		//load.DrawLoad();
		ShowProfiles = TRUE;
		pMarkedPlate->ShowLoadState();
		//	if (pLoadView->ShowLoad == TRUE) pLoadView->InvalidateRect(NULL, TRUE);
		pSetView->InvalidateRect(NULL, TRUE);//	OnPlotMaxprofiles();
		*/
	} // dlg OK

	else {
		AfxMessageBox("PSI read cancelled\n Default model is active");
		pPlasma->PSIloaded = FALSE; // use simple model R/Rnorm
		OnPlotPSI();
		PSILoaded = pPlasma->PSIloaded;
		return;
	}
}

void CBTRDoc::OnUpdateTasksPDP(CCmdUI*) 
{
//	pCmdUI->SetCheck(OptPDP);
}

void CBTRDoc::OnResultsPdpoutputTable() 
{
	CLoadView * pLV = (CLoadView *) pLoadView;
	CLoad * load = NULL;
	char name[1024];
//	CFileDialog dlg(TRUE, "txt; *txt", "PDP load table");
	CFileDialog dlg(TRUE, "txt; dat | * ", "", OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"Tabular data (*.txt);(*.dat) | *.txt; *.TXT; *.dat; *.DAT| All Files (*.*) | *.*||", NULL);
	
	if (dlg.DoModal() == IDOK) {
	
		strcpy(name, dlg.GetPathName());
		CWaitCursor wait;
		CPreviewDlg dlg;
		dlg.m_Filename = name;
		if (dlg.DoModal() == IDCANCEL) return;
		SetTitle(name);// + TaskName);
		OptPDP = TRUE;
	
		ReadEQDSKtable(name); // test
		//return;
		//load = ReadPDPLoadTable(name);
		//load = pTorCrossPlate->Load;
		load = pMarkedPlate->Load;
		if (load != NULL && load->MaxVal >1.e-10) {
		//	pLV->ShowLoad = TRUE;
		//	pLV->SetLoad_Plate(load, pMarkedPlate);
		//	pLV->Contours = FALSE;
			pLV->Cross.X = pMarkedPlate->Load->iProf * pMarkedPlate->Load->StepX;
			pLV->Cross.Y = pMarkedPlate->Load->jProf * pMarkedPlate->Load->StepY;
		//	ShowProfiles = TRUE;
			pMarkedPlate->ShowLoadState();
			//pMarkedPlate->ShowLoad();
		} // load != NULL
		else {
			pLV->ShowLoad = FALSE;
			pSetView->Load = NULL;
		}

		pMainView->InvalidateRect(NULL, TRUE);
		//pLV->InvalidateRect(NULL, TRUE);
		//pSetView->InvalidateRect(NULL, TRUE);
		ShowFileText(name);
	} // if IDOK

	OptParallel = FALSE;//  - to be found 

}

CLoad * CBTRDoc:: ReadPDPLoadTable(char * name)
{
//	CPlate * plate; 
	CPlate * plate0 = new CPlate();
	CLoad * load;// = new CLoad();
	
	int Nx = 0, Ny = 0, SmoothDegree = 0;
	double StepH = 0, StepV = 0, Hmin, Vmin, Hmax, Vmax, Zmax, Scell;
	C3Point Orig, OrtX, OrtY;

	FILE * fin;
	if ( (fin= fopen(name, "r")) == NULL) {
		CString S ="Can't find/open ";
		S+=  name;
		AfxMessageBox(S, MB_ICONSTOP | MB_OK);
		return NULL;
	}

	char buf[1024];
	int i, j, result;
	C3Point lb(1000,1000,1000), rt(1000,1000,1000);
	CString line;
	CString axisX, axisY;

	if (fgets(buf,1024,fin) == NULL) { AfxMessageBox("Problem with line 1"); fclose(fin); return NULL; }
		result = sscanf(buf, "%d %d", &Nx, &Ny); 
		if (result != 2) {
			while (!feof(fin) && result != 2) {
				fgets(buf, 1024, fin);
				result = sscanf(buf, "%d %d", &Nx, &Ny);
			}
			if (feof(fin)) { AfxMessageBox("Invalid Format of 1-st line", MB_ICONSTOP | MB_OK); fclose(fin); return NULL;}
			if (Nx*Ny <= 1.e-6) {AfxMessageBox("Invalid numbers Nx, Ny", MB_ICONSTOP | MB_OK); fclose(fin); return NULL;}
		}
		line = buf;
		int pos = line.Find("#");
		if (pos < 0) {
			plate0->Comment = "Enjoy the Picture!";// "NO COMMENT";
		}

		else {
			if (pos >= 0) line = line.Mid(pos+1);
			pos = line.Find("\n");
			if (pos > 0) line = line.Left(pos);
			plate0->Comment += line;
		}

		if (fgets(buf,1024,fin) == NULL) { AfxMessageBox("Problem with line 2"); fclose(fin); return NULL; }
		result = sscanf(buf, "%le %le", &Hmin, &StepH); 
		if (result != 2) {AfxMessageBox("Invalid line 2"); fclose(fin); return NULL;}
		if (StepH < 1.e-6) {AfxMessageBox("Hor. Step = 0!",MB_ICONSTOP | MB_OK); fclose(fin); return NULL;}
		axisX = buf;

		if (fgets(buf,1024,fin) == NULL) { AfxMessageBox("Problem with line 3"); fclose(fin); return NULL; }
		result = sscanf(buf, "%le %le", &Vmin, &StepV); 
		if (result != 2) {AfxMessageBox("Invalid line 3"); fclose(fin); return NULL;}
		if (StepV < 1.e-6) {AfxMessageBox("Vert. StepY = 0!",MB_ICONSTOP | MB_OK); fclose(fin); return NULL;}
		axisY = buf;
		
		if (fgets(buf,1024,fin) == NULL) { AfxMessageBox("Problem with line 4"); fclose(fin); return NULL; }
		result = sscanf(buf, "%le %le", &Scell, &Zmax); 
		if (result != 2) {AfxMessageBox("Invalid line 4"); fclose(fin); return NULL;}
		if (Zmax < 1.e-12) {AfxMessageBox("Zmax < 1.e-12",MB_ICONSTOP | MB_OK); fclose(fin); return NULL;}
	
	Hmax = Hmin + Nx*StepH;
	Vmax = Vmin + Ny*StepV;
	
	pos = axisX.Find("#");
	if (pos < 0) { 
		AfxMessageBox("Horizontal Axis not defined\n (# not found)");
		OrtX = C3Point(0,1,0);
		lb = C3Point(1000,Hmin,Vmin); 
		rt = C3Point(1000,Hmax, Vmax);
	}
	else { 
		axisX = axisX.Mid(pos+1);
		if (axisX.Find("X") >= 0) {
			OrtX = C3Point(1,0,0);
			lb.X = Hmin;
			rt.X = Hmax;
		}
		if (axisX.Find("Y") >= 0) {
			OrtX = C3Point(0,1,0); 
			lb.Y = Hmin; 
			rt.Y = Hmax;
		}
		if (axisX.Find("Z") >= 0) { 
			OrtX = C3Point(0,0,1); 
			lb.Z = Hmin; 
			rt.Z = Hmax;
		}
	}// # found

	pos = axisY.Find("#");
	if (pos < 0) {
		AfxMessageBox("Vertical Axis not defined\n (# not found)");
		OrtY = C3Point(0,0,1);
		lb = C3Point(1000,Hmin,Vmin); 
		rt = C3Point(1000,Hmax, Vmax);
	}
	else {
		axisY = axisY.Mid(pos+1);
		if (axisY.Find("X") >= 0) { 
			OrtY = C3Point(1,0,0); 
			lb.X = Vmin; 
			rt.X = Vmax; 
		}
		if (axisY.Find("Y") >= 0) { 
			OrtY = C3Point(0,1,0); 
			lb.Y = Vmin; 
			rt.Y = Vmax; 
		}
		if (axisY.Find("Z") >= 0) {
			OrtY = C3Point(0,0,1);
			lb.Z = Vmin; 
			rt.Z = Vmax; 
		}
	} // # found
	
	plate0->SetFromLimits(lb, rt);//SetOrts();
	//plate0->OrtX = OrtX; plate0->OrtY = OrtY; plate0->OrtZ = VectProd(OrtX, OrtY); 
	
	load = new CLoad(Hmax-Hmin, Vmax-Vmin, StepH, StepV);
	
	plate0->Load = load;
	load->Comment = plate0->Comment;
	int scenario = 0;
	if (plate0->Comment.Find("Poloidal") >= 0) {
		if (plate0->Comment.Find("cenario 4") >= 0)	scenario = 4;
		if (plate0->Comment.Find("cenario 2") >= 0)	scenario = 2;
	}

	
	CString text = "";

	while (!feof(fin)) {
			fgets(buf, 1024, fin);
			if (!feof(fin))	text += buf;
		}

	char *buff, *nptr, *endptr;
	double val, newval;

	double PSIc, PSIx;
	switch (scenario) { // scenario > 0  -> PSI table
		case 2: PSIc = 12.0375224; PSIx = -0.406686479; break; // scenario 2
		case 4: PSIc = 8.9614899;  PSIx = 3.238912; break; // scenario 4
		default : break; // PDP table
	}

	int length = text.GetLength()*2;
	buff = (char *)calloc(length, sizeof(char));
	strcpy(buff, text);
	
	nptr = buff;

	for (j = 0; j <= Ny; j++)
	for (i = 0; i <= Nx; i++) {
		val = strtod(nptr,&endptr);
		
		if (scenario > 0) { // PSI table
			val = (val - PSIc) / (PSIx - PSIc); // EQDSK
			if (val > 1 || val < 0) val = -0.01;
			//newval = GetPlasmaTeNe(val).Z; // Ne
		} // PSI

		// else -> PDP table

		plate0->Load->Val[i][j] = val;
		if (nptr == endptr) nptr++; //catch special case
		else nptr = endptr;
	}
	//SetDecayInPlasma();
	fclose(fin);
	
	load->SetSumMax();
	if (Scell > 1.e-12)	load->Sum = load->Sum / StepH / StepV * Scell;// PAA email 25.12.09

		plate0->Selected = TRUE;
		plate0->Loaded = TRUE;
		plate0->Load = load;
		plate0->SmLoad = new CLoad(Hmax-Hmin, Vmax-Vmin, StepH, StepV);
		plate0->SmoothDegree = SmoothDegree;
		plate0->filename = name;
		plate0->Number = 1000;
	//	SetLoadArray(plate0, TRUE); // changes plate->Number!
	//	AddPlate(plate0);
		pMarkedPlate = plate0;
		if (scenario >0 ) pTorCrossPlate = plate0;
		else pTorCrossPlate = NULL;
		//OptParallel = FALSE;// not parallel calculation
		OptCombiPlot = 0; 

 return (load);

}

void CBTRDoc::ReadEQDSKtable(char * name)
/*// format is taken from A.Dnestrovsky message 08/11/2017
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

! Ïðèâåäåíèå ê íóëþ íà ìàãíèòíîé îñè
DO i2 = 1 , Ny
DO i1 = 1 , Nx
psirz(i1,i2) = psirz(i1,i2) - PSIc
END DO
END DO
PSIx = PSIx - PSIc // íà ñåïàðàòðèñå
*/
//val = (val - PSIc) / (PSIx - PSIc); //-> psi = psirz / PSIx

{
	PSIvolume.RemoveAll();
	for (int k = 0; k < MagSurfDim; k++) PSIvolume.Add(0.001);

	FILE * fin;
	CString S;
	if ((fin = fopen(name, "r")) == NULL) {
		S = "Can't find/open ";
		S += name;
		AfxMessageBox(S, MB_ICONSTOP | MB_OK);
		return;
	}

	char buf[1024];
	int idum, Nx, Ny;
	char s[255];

	// line 1 : geqdsk_lbl,idum, nh(Nx), nv(Ny)
	if (fgets(buf, 1024, fin) == NULL) { 
		AfxMessageBox("Problem with line 1"); 
		fclose(fin); 
		return;
	}

	int res = sscanf(buf, "%s %d %d %d", s, &idum, &Nx, &Ny);
	if (res != 4) {
		AfxMessageBox("Invalid format in line 1", MB_ICONSTOP | MB_OK);
		fclose(fin);
		return;
	}
	if (Nx*Ny <= 1.e-6) {
		AfxMessageBox("Invalid numbers Nx, Ny", MB_ICONSTOP | MB_OK); 
		fclose(fin); 
		return;
	}

//	S.Format("Nx = %d Ny = %d", Nx, Ny);
//	AfxMessageBox(S);
	double val;
		
	// line 2 : rdim(DR),zdim(DZ),rcentr,rleft(Rmin),zmid(Zmid)
	double DR, DZ, Rmin, Zmid; //read
	double Rmax, Zmin, Zmax;   // calculated
	if (fgets(buf, 1024, fin) == NULL) {
		AfxMessageBox("Problem with line 2");
		fclose(fin);
		return;
	}
	
	res = sscanf(buf, "%le %le %le %le %le", &DR, &DZ, &val, &Rmin, &Zmid);
	if (res != 5) {
		AfxMessageBox("Invalid format in line 2", MB_ICONSTOP | MB_OK);
		fclose(fin);
		return;
	}
	if (DR*DZ <= 1.e-6) {
		AfxMessageBox("Invalid dimensions DR, DZ", MB_ICONSTOP | MB_OK);
		fclose(fin);
		return;
	}
	//S.Format("Rmin = %le Zmid = %le", Rmin, Zmid);
	//AfxMessageBox(S);
	Rmax = Rmin + DR;
	Zmin = Zmid - 0.5*DZ;
	Zmax = Zmid + 0.5*DZ;
	
	// line 3 : rmaxis,zmaxis,zpsimag(PSIc),zpsibdy(PSIx),bcentr
	double v1, v2; // read but not used
	double PSIc, PSIx;
	if (fgets(buf, 1024, fin) == NULL) {
		AfxMessageBox("Problem with line 3");
		fclose(fin);
		return;
	}

	res = sscanf(buf, "%le %le %le %le %le", &v1, &v2, &PSIc, &PSIx, &val);
	if (res != 5) {
		AfxMessageBox("Invalid format in line 3", MB_ICONSTOP | MB_OK);
		fclose(fin);
		return;
	}
	if (fabs(PSIc*PSIx) <= 1.e-6) {
		AfxMessageBox("PSIc or PSIx too small", MB_ICONSTOP | MB_OK);
		
	}
	S.Format("PSIc = %le PSIx = %le", PSIc, PSIx);
	AfxMessageBox(S);

	// line 4 : read but not used ( pcur,zpsimag(PSIc),xdum,rmaxis,xdum )
	if (fgets(buf, 1024, fin) == NULL) {
		AfxMessageBox("Problem with line 4");
		fclose(fin);
		return;
	}
	// line 5 : read but not used  ( zmaxis,xdum,zpsibdy(PSIx),xdum,xdum )
	if (fgets(buf, 1024, fin) == NULL) {
		AfxMessageBox("Problem with line 5");
		fclose(fin);
		return;
	}

	// read arrays
	char *buff, *nptr, *endptr;
	CString text = "";

	while (!feof(fin)) { // read total text in file
		fgets(buf, 1024, fin);
		if (!feof(fin))	text += buf;
	}
	
	int size = text.GetLength();
	
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
		for (int i = 0; i < Nx; i++) {
			val = strtod(nptr, &endptr);
			if (nptr == endptr) nptr++; //catch special case
			else nptr = endptr;
		}
	}

	double PSI;
	int kpsi;
	double PSIxn = PSIx - PSIc; // íà ñåïàðàòðèñå
	CPlate * plate0 = new CPlate();
	CLoad * load;
	double stepR = DR / (Nx - 1);
	double stepZ = DZ / (Ny - 1);
	C3Point lb = C3Point(1000, Rmin, Zmin);
	C3Point rt = C3Point(1000, Rmax, Zmax);
	plate0->SetFromLimits(lb, rt);//SetOrts();
								  //plate0->OrtX = OrtX; plate0->OrtY = OrtY; plate0->OrtZ = VectProd(OrtX, OrtY); 

	load = new CLoad(DR, DZ, stepR, stepZ);
	
// read pol flux array [Nx, Ny]	
	int count = 0;
	for (int j = 0; j < Ny; j++) {
		for (int i = 0; i < Nx; i++) {
			val = strtod(nptr, &endptr);
			count++;
			//val = (val - PSIc) / (PSIx - PSIc); //-> psi = psirz / PSIx
			PSI = (val - PSIc) / PSIxn; // normalised
			load->Val[i][j] = PSI; //-> psi = psirz / PSIxval;
			if (nptr == endptr) nptr++; //catch special case
			else nptr = endptr;
			kpsi = (int)floor(PSI / StepPSI);
			if (kpsi >= 0 && kpsi < MagSurfDim)	PSIvolume[kpsi] += 1;// stepR*stepZ
		}
	}
	// next lines - not used 
	fclose(fin);

	S.Format("%d psi values read /n Nx %d Ny %d", count, Nx, Ny);
	AfxMessageBox(S);

	load->SetSumMax();
	load->iProf = (int)(0.5 * load->iMaxVal);
	load->jProf = (int)(0.5 * load->jMaxVal);
	plate0->Selected = TRUE;
	plate0->Loaded = TRUE;
	plate0->Load = load;
	plate0->SmLoad = new CLoad(DR, DZ, stepR, stepZ);
	plate0->SmoothDegree = 0;// SmoothDegree;
	plate0->filename = name;
	plate0->Number = 1000;
	//	SetLoadArray(plate0, TRUE); // changes plate->Number!
	//	AddPlate(plate0);
	pMarkedPlate = plate0;
	pMarkedPlate->Load = load;
	pTorCrossPlate = plate0;
	pTorCrossPlate->Load = load;

	PSILoaded = TRUE;
	if (ProfLoaded) PlasmaLoaded = TRUE;
	else PlasmaLoaded = FALSE;

	//if (scenario >0) pTorCrossPlate = plate0; else pTorCrossPlate = NULL;
	//OptParallel = FALSE;// not parallel calculation
	OptCombiPlot = 0;
}


void CBTRDoc::OnDataImportSingapBeam() 
{
	OnPdpBeamlets();
	
}

void CBTRDoc::OnDataImportMamugBeam() 
{

	OnPdpBeamlets();
}

void CBTRDoc::OnPdpBeamlets() 
{
	if (!STOP) {
		AfxMessageBox("Stop calculations before reading new data! \n (RED CROSS stop button on the Toolbar)");
		return ;
	}
	OptSINGAP = TRUE; // 
	ReadSINGAP(); //->SINGAPLoaded + Regularity Check
	SetSINGAP();
	if (!OptSINGAP) { // MAMuG
		SetBeam(); 
		for (int i = 0; i < (int)NofChannelsHor; i++) ActiveCh[i] = TRUE;
		for (int j = 0; j < (int)NofChannelsVert; j++) ActiveRow[j] = TRUE;
		//SetStatus(); 
	}
	SetStatus(); // for SINGAP as well
	if ((int)BeamSplitType ==0) SetPolarBeamletCommon(); //SetBeamletAt(0);
		else SetGaussBeamlet(BeamCoreDivY, BeamCoreDivZ);
		
	//OnShow();
}

void CBTRDoc::OnDataExportPDPgeom() 
{
	//AfxMessageBox("Sorry, not available now\n Can be switched ON by request");
	//return;

	/*char name[1024];
	FormDataText(TRUE);// include internal names of parameters
	CFileDialog dlg(FALSE, "dat | * ", "", OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"BTR Output file (*.dat) | *.dat; *.DAT | All Files (*.*) | *.*||", NULL);
		if (dlg.DoModal() == IDOK) {
			FILE * fout;
			strcpy(name, dlg.GetPathName());
			fout = fopen(name, "w");
			fprintf(fout, m_Text);
			fclose(fout);
			SetTitle(name);// + TaskName);
		}
		delete dlg;
*/
	char name[1024];// = "_Input_PDP_BTR.txt";
	strcpy(name,"_Input_PDP_BTR.txt");
//	char title[1024];
//	char newname[1024];
	FILE * fin;
	CString S;

	CFileDialog dlg(FALSE, "txt; bak | * ", name, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"PDP INPUT file (*.txt); (*.bak) | *.txt; *.TXT; *.bak; *.BAK | All Files (*.*) | *.*||", NULL);
	if (dlg.DoModal() == IDOK) {
			strcpy(name, dlg.GetPathName());
		//	strcpy(title, dlg.GetFileTitle());
		//	strcpy(newname, title);
		//	strcat(newname, ".bak");
			

	/*	if ((fin = fopen(name, "r")) != 0) {// already exists
			fclose(fin);
			DeleteFile(newname);
			MoveFile(name, newname);
			S.Format("Old data are kept in file %s", newname); 
			AfxMessageBox(S);
		}*/
	
		WritePDPinput(name);
		
		/* CPreviewDlg pdlg;
		pdlg.m_Filename = name;
		if (pdlg.DoModal() == IDCANCEL) { // return back
			DeleteFile(name);
			MoveFile(newname, name);
			S.Format("Old data are kept in file %s", name); 
			AfxMessageBox(S);
		}*/

	} // file dlg OK
	
}

void CBTRDoc::OnDataExportRegularBeam() 
{
	//char * name = "_Beamlet_Regular_Array.txt";
	char name[1024];// = "_Input_PDP_BTR.txt";
	strcpy(name,"Beamlets_BERT.txt");
//	char title[1024];
//	char newname[1024];
	FILE * fin;
	CString S;

	CFileDialog dlg(FALSE, "txt; bak | * ", name, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"Beamlets file (*.txt); (*.bak) | *.txt; *.TXT; *.bak; *.BAK | All Files (*.*) | *.*||", NULL);
	if (dlg.DoModal() == IDOK) {
			strcpy(name, dlg.GetPathName());
		//	strcpy(title, dlg.GetFileTitle());
		//	strcpy(newname, title);
		//	strcat(newname, ".bak");

	/*	if ((fin = fopen(name, "r")) != 0) {// already exists
			fclose(fin);
			DeleteFile(newname);
			MoveFile(name, newname);
			S.Format("Old data are kept in file %s", newname); 
			AfxMessageBox(S);
		} */
	
		//WriteRegularBeam(name);
		WriteSingapBeam(name);
		
	/*	CPreviewDlg pdlg;
		pdlg.m_Filename = name;
		if (pdlg.DoModal() == IDCANCEL) { // return back
			DeleteFile(name);
			MoveFile(newname, name);
			S.Format("Old data are kept in file %s", name); 
			AfxMessageBox(S);
		}
*/
	} // file dlg OK
	
	
}

void CBTRDoc::WriteSingapBeam(char * name)
{
	FILE * fout;
	CString S;
	BEAMLET_ENTRY be;
	int kmax = BeamEntry_Array.GetUpperBound();

	fout = fopen(name, "w"); 
	fprintf(fout, " ******  Beam array data created by BTR 5 ***********\n");

	fprintf(fout, " N     POS-Y   WID-Y    ALF-Y       DIV-Y      POS-Z     WID-Z     ALF-Z     DIV-Z     RelFRACT\n");

	//fprintf(fout, "  Y,mm\t AlfY,mrad DivY,mrad\t\t Z,mm\t Alfz,mrad DivZ,mrad\t\t Fract\t column\t row\n"); // AK format
	int N = 0;
	double WID = 0; // not used
	double POSY, ALFY, DIVY, POSZ, ALFZ, DIVZ, Fr;

	for (int k = 0; k <= kmax; k++) {
		be = BeamEntry_Array[k];
		N++;

		be.AlfY -= BeamMisHor; //mrad -> rad
		be.AlfZ -= BeamMisVert + BeamVertTilt;
		be.AlfZ -= VertInclin; // NBI standard
		//fprintf(fout, " %g\t %g\t %g\t\t\t %g\t %g\t %g\t\t %g\t %d\t %d\n", 
		//1000 * be.PosY, 1000 * be.AlfY, 1000 * be.DivY, 1000 * be.PosZ, 1000 * be.AlfZ, 1000 * be.DivZ, 
		//be.Fraction, be.i, be.j);

		POSY = 1000 * be.PosY; POSZ = 1000 * be.PosZ;
		ALFY = 1000 * be.AlfY; ALFZ = 1000 * be.AlfZ;
		DIVY = 1000 * be.DivY; DIVZ = 1000 * be.DivZ;
		Fr = be.Fraction;

		fprintf(fout, " %d\t %5g\t %3g\t %7g\t %3g\t %5g\t %3g\t %7g\t %3g\t %7g\n",
						N,  POSY,  WID,  ALFY, DIVY, POSZ,  WID,  ALFZ,  DIVZ, Fr);
	}
	fprintf(fout, " N    POS-Y   WID-Y    ALF-Y       DIV-Y      POS-Z     WID-Z     ALF-Z     DIV-Z     Rel FRACT\n\n");

	fprintf(fout, "BEAM SOURCE GG data ~ Bert's format (SINGAP accelerator exit) \n");
	fprintf(fout, "Dimensions are mm and mrad. Y is horizontal, Z is vertical \n");
	fprintf(fout, "POS-Y/POS-Z - Horizontal / Vertical position of beamlet in mm \n");
	fprintf(fout, "WID-Y/WID-Z - Horizontal / Vertical width(= radius) of beamlet in mm (not used) \n");
	fprintf(fout, "ALF-Y/ALF-Z - Horizontal / Vertical angle in mrad. Positive angle is pointing left/upwards \n");
	fprintf(fout, "DIV-Y/DIV-Z - Horizontal / Vertical beamlet divergence in mrad \n");
	fprintf(fout, "FRACT -	Reference power fraction for this beamlet. BTR will normalize it by the total source power \n");
	/*  be.Active = TRUE;
		be.PosY = BeamletPosHor[i*Ny + ii]; // MAMuG
		be.PosZ = BeamletPosVert[j*Nz + jj]; // MAMuG
		be.AlfY = BeamletAngleHor[i*Ny + ii];
		be.AlfZ = BeamletAngleVert[j*Nz + jj];
		be.DivY = BeamCoreDivY;
		be.DivZ = BeamCoreDivZ;
		be.Fraction = 1;
		be.i = i; be.j = j;*/
		
	fclose(fout);

}

void CBTRDoc:: WriteRegularBeam(char * name) // not called
{
	FILE * fout;
	CString S;

	fout = fopen(name, "w"); //("PDP_input.dat", "w");
	fprintf(fout, "  Regular beam array (for PDP-code), created by BTR 1.7\n");

	fprintf(fout, "  Yo,mm  Ay,mrad  Acy,mrad    Zo,mm  Az,mrad  Acz,mrad   FrI   column (I)   row (J) \n");
	double Y0, Ay, Acy, Z0, Az, Acz, FrI;
	int iy, jz;
	int NtotalHor = (int)(NofChannelsHor * (NofBeamletsHor));
	int NtotalVert = (int)(NofChannelsVert * (NofBeamletsVert));

/*	double *	NofChannelsHor;// Beam Source
	double *	NofChannelsVert;
	double *	NofBeamletsHor;
	double *	NofBeamletsVert; 
	double *	SegmStepHor;
	double *	SegmStepVert;
	double *	BeamAimHor; 
	double *	BeamAimVert;
	double *	AppertStepHor;
	double *	AppertStepVert;
	double *	AppertAimHor; 
	double *	AppertAimVert;*/
		
	Acy = BeamCoreDivY * 1000; // rad -> mrad
	Acz = BeamCoreDivZ * 1000;
	FrI = 1;

	for (iy = 1; iy <= NtotalHor; iy++) {

		Y0 = BeamletPosHor[iy-1] * 1000; // m -> mm
		Ay = 1000 * (BeamletAngleHor[iy-1] - BeamMisHor); // rad -> mrad

		for (jz = 1; jz <= NtotalVert; jz++) {

			double VInclin = 0;
			if (!OptFree) VInclin = VertInclin;
			Z0 = BeamletPosVert[jz-1] * 1000; // m -> mm
			Az = 1000 *(BeamletAngleVert[jz-1] - (BeamMisVert + VInclin + BeamVertTilt)); // rad -> mrad

//			fprintf(fout, " %g   %g   %g         %g   %g   %g        %g     %d   %d\n", Y0, Ay, Acy, Z0, Az, Acz, FrI, iy, jz);
			fprintf(fout, " %g   %4.4f   %g         %g   %4.4f   %g        %g     %d   %d\n", Y0, Ay, Acy, Z0, Az, Acz, FrI, iy, jz);
	
		} // jz

	} // iy


	fclose(fout);
}

void CBTRDoc::OnPlotReionPower()
{
	CMultiPlotDlg dlg;
	//dlg.pDoc = this;
	dlg.InitPlot(this);

	if (dlg.DoModal() == IDOK) return;
}

void CBTRDoc::OnSurfacesSort()
{
	// TODO: Add your command handler code here
	SortLoadArrayAlongX();
}

void CBTRDoc::OnUpdatePlotReionPower(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	double Sum = 0;
	for (int k = 0; k < SumReiPowerX.GetSize(); k++) Sum += SumReiPowerX[k];
	pCmdUI->Enable(Sum > 1.e-12);
}

void CBTRDoc::ClearArrays() //called by DeleteContents <- OnNewDocument!!!
{
	for (int i = 0; i < ThreadNumber; i++) {
		m_AttrVector[i].clear();
	//	m_AttrVector[i].shrink_to_fit();
		m_ExitVector[i].clear();
	//	m_ExitVector[i].shrink_to_fit();
		m_FallVector[i].clear();
	//	m_FallVector[i].shrink_to_fit();
		m_Log[i].clear();
		//m_Log[i].shrink_to_fit();
	}
	m_GlobalVector.clear();// size -> 0
	//m_GlobalVector.shrink_to_fit();
	//m_GlobalDeque.clear();// size -> 0
	//m_GlobalDeque.shrink_to_fit();/// not valid
	m_GlobalLog.clear();
	//m_GlobalLog.shrink_to_fit();
	MemFallReserved = FALSE;
	Exit_Array.RemoveAll();
	SetLoadArray(NULL, FALSE);

	//ClearScenLoadTracks(); // not works when called by NewDocument!!!
}


bool CBTRDoc::AddLog(std::vector<CString> * log)
{
	vector<CString>::iterator it = m_GlobalLog.end();
	TRY{
		m_GlobalLog.insert(it, log->begin(), log->end());
	} CATCH(CMemoryException, e) {
		return FALSE;
	} END_CATCH;

	//m_GlobalLog.shrink_to_fit();
	return TRUE;

}

bool CBTRDoc::AddFallsToFalls(int tid, int isrc, std::vector<minATTR> * tattr)

// append vector tattr to plate.Falls array
// currently used for n = PlasmaEmitter (few surf can be added) -  Single run
// called after each BML in SINGLE run for limited split
// + can be called after each bml in MULTI
{
	
	if (RUN != 13) return TRUE; // keep falls in SINGLE run ONLY

	//if (Attr_Array.GetSize() > 13000) return true; // hard limit for falls - 24.5e+6
	
	//return TRUE; // switched OFF now !!!!!!!!!!!!
////////////////////////////////////////////////////
	if (STOP) return FALSE;

	CPlate * plate;
	CString S, Slog;
	int n, nrefl;
		
	long long tsize = tattr->size();
	long long OldArrSize = ArrSize; // plate->Falls.GetSize(); //m_GlobalVector.size();

	/* // for multiple reflectors
	CString name;
	CString Names[10];
	int Nrefl = Reflectors.GetSize();
	for (int i = 0; i < Nrefl; i++)
		Names[i] = Reflectors.GetAt(i);
		
	int Nlen = 1;// Nrefl+1; // sizeof(Names);
	
	int n0 = PlasmaEmitter;
	int NumS[] = { n0 , -1, -1, -1, -1, -1, -1, -1, -1, -1 }; // max = 10

	int i = 1; // counter for NumS > 0
	POSITION pos = PlatesList.GetHeadPosition();
	while (pos != NULL) {
		plate = PlatesList.GetNext(pos);
		CString Com = plate->Comment;
		for (n = 0; n < Nlen; n++) {
			name = Names[n];
			if (Com.Find(name, 0) > 0) {
				NumS[i] = plate->Number;
				i++; // max index
				if (i > sizeof(NumS)) {
					//AfxMessageBox("Surf Emitter out of bounds (10) ");
					i--;
					break;
				}
			}
		} // scan names
	} // scan plates

	int NumLen = 1;// i;// sizeof(NumS);  // <= 10 !!!

	for (int ind = 0; ind < NumLen; ind++) {
		int np = NumS[ind];
		plate = GetPlateByNumber(np);
		OldArrSize += plate->Falls.GetSize();
	}
	*/ // for multiple reflectors

	//bool exists = std::find(std::begin(a), std::end(a), x) != std::end(a);
	
	//int n0 = PlasmaEmitter;// single!!
	//plate = GetPlateByNumber(n0);
	//OldArrSize += plate->Falls.GetSize();
	//ArrSize = plate->Falls.GetSize(); //OldArrSize; // new size to be calculated
	//OldArrSize = ArrSize;
	
	nrefl = ReflectorNums.GetSize();
	if (nrefl < 1) return FALSE;

	bool found = FALSE; // n - in reflectors list

	for (int i = 0; i < tsize; i++) { // falls
		minATTR fall(tattr->at(i));
		n = fall.Nfall;	//if (fall.Nfall != n) continue;
		found = FALSE;
		for (int j = 0; j < nrefl; j++) {
			if (ReflectorNums[j] == n) {
				found = TRUE;
				break;
			}
		}
		if (!found) continue; // go to next fall
		
		//if (n != n0) // && fall.Charge != 0) continue; // keep atoms only for plasma Emitter
		//bool exists = std::find(std::begin(NumS), std::end(NumS), n) != std::end(NumS);// multiple reflectors

		plate = GetPlateByNumber(n); // found in the list
		TRY {
			//fall = tattr->at(i);
			plate->Falls.Add(fall);
		} CATCH (CMemoryException, e) {
			S.Format("\n ******* FAILED to ADD Falls to Emitter! *******\n");
			logout << S;
			//ArrSize = m_GlobalVector.size();
			logout << "----- Last Falls Size --- " << OldArrSize << "\n";
			//STOP = TRUE;
			return FALSE;
		} END_CATCH;
		ArrSize++; // added
	} // i falls

	//ArrSize = plate->Falls.GetSize();
	return TRUE;
}

bool CBTRDoc::AddFalls(int tid, int isrc,  std::vector<minATTR> * tattr) // not called
// not called now  - only for tests (DoNextStep, TestMem)
// bool AddFalls(int tid, int isrc, std::vector<minATTR> & attr);
{
	if (STOP) return FALSE;
	vector<minATTR>::iterator it;//= m_GlobalVector.end();
	//deque<SATTR>::iterator it= m_GlobalDeque.end();
	 
	CString S, Slog;
	minATTR a;
	long tsize = tattr->size();
	long OldArrSize = m_GlobalVector.size();
	long NewArrSize = OldArrSize + tsize;
	//logout << "OLD Falls Size " << OldArrSize << "\n";
	//logout << "NEW Falls Size " << NewArrSize << "\n";
	
	TRY {
		//tattr->reserve(sz);
		m_GlobalVector.reserve(NewArrSize);
		it = m_GlobalVector.end();
		m_GlobalVector.insert(it, tattr->begin(), tattr->end());//append Bml to global vect
		//m_GlobalDeque.insert(it, tattr->begin(), tattr->end());
	} CATCH (CMemoryException, e) {
		//S.Format("Thread %d\n FAILED to add Falls! \n");// Last bml -  %d\n",tid, NofCalculated);
		//AfxMessageBox(S);
		S.Format("\n ******* FAILED to add Falls! (out of MEMORY) *******\n");
		logout << S;
		ArrSize = m_GlobalVector.size();
		logout << "----- Last Falls Size --- " << ArrSize << "\n";
		STOP = TRUE;
		//OnStop();
		return FALSE;
	} END_CATCH;

	ArrSize = m_GlobalVector.size();
	//logout << "Falls Size After Add " << ArrSize << "\n";

	return TRUE;
	
}

void CBTRDoc::InitTracer(int thread, CTracer * pTracer)// not called
{
	int i = thread;
	m_AttrVector[i].clear();//	m_AttrVector[i].shrink_to_fit();
	m_ExitVector[i].clear();//	m_ExitVector[i].shrink_to_fit();
	m_FallVector[i].clear();//	m_FallVector[i].shrink_to_fit();
	m_Log[i].clear();
	//m_GlobalVector.clear();// size -> 0 - called in ClearArrays
	//m_GlobalLog.clear();// size -> 0 - called in ClearArrays
	pTracer->SetID(i + 1);
	pTracer->SetDraw(OptDrawPart);
	pTracer->SetDocument(this);
	pTracer->SetAttrArray(&m_AttrVector[i]);
	
//	pTracer->SetLogArray(&m_Log[i]);
	pTracer->SetViewWnd(pMainView);
	pTracer->SetStatWnd(pSetView);
	pTracer->SetContinue(FALSE);// continue = FALSE
	//Nmin = i*numb; Nmax = (i + 1)*numb - 1;
	//if (i == ThreadNumber - 1) Nmax = Max(Nmax, NofBeamlets - 1); // last thread takes all remained
	//m_Tracer[i].SetLimits(Nmin, Nmax);
	//m_Tracer[i].SetStartPoint(Nmin);
	pTracer->SetRaysAttr(&Attr_Array);
	pTracer->SetStartVP(0);
	pTracer->ClearArrays();//tattr, logarr
	//m_pTraceThread[i] = NULL;
	//pTracer->SetContinue(TRUE);
}

void CBTRDoc:: InitTracers() // called by OnStartParallel
{
	CString S;
	if (NofBeamlets < 1) { AfxMessageBox("N of beamlets = 0?"); return; }
	
	if (NofBeamlets < ThreadNumber) { 
		ThreadNumber = 1; //NofBeamlets;
	}
	
	int numb = NofBeamlets / ThreadNumber; // beamlets per thread
	int Nattr = Attr_Array.GetSize();
	S.Format(" InitTracers = %d\n  Attr Array size %d\n", ThreadNumber, Nattr);	
	logout << S; //AfxMessageBox(S);
	int Nmin, Nmax;// BML limits for each tracer
	
	ClearArrays();//Global arrays size -> 0
	
	SetNullLoads();//clear loads, remove falls

	NofCalculated = 0;

	for (int i = 0; i < ThreadNumber; i++)
		{
			m_Tracer[i].SetID(i + 1);
			m_Tracer[i].SetDocument(this);
			m_Tracer[i].SetDraw(OptDrawPart);
			m_Tracer[i].SetAttrArray(&m_AttrVector[i]);
		//	m_Tracer[i].SetExitArray(&m_ExitVector[i]);
		//	m_Tracer[i].SetFallArray(&m_FallVector[i]);
		//	m_Tracer[i].SetLogArray(&m_Log[i]);
			m_Tracer[i].SetViewWnd(pMainView);
			m_Tracer[i].SetStatWnd(pSetView);
			m_Tracer[i].SetContinue(FALSE);// continue = FALSE
			Nmin = i*numb; Nmax = (i+1)*numb - 1;
			if (i == ThreadNumber-1) Nmax = Max(Nmax, NofBeamlets - 1); // last thread takes all remained
			m_Tracer[i].SetLimits(Nmin, Nmax);
			m_Tracer[i].SetStartPoint(Nmin);
			m_Tracer[i].SetSrcIonState();
			m_Tracer[i].SetRaysAttr(&Attr_Array);
			m_Tracer[i].SetStartVP(0);
			m_Tracer[i].ClearArrays();
			m_pTraceThread[i] = NULL;
			m_Tracer[i].SetContinue(TRUE);
		}
	//MemFallReserved = FALSE;
	//Exit_Array.RemoveAll();
		
}


UINT CBTRDoc:: ThreadFunc(LPVOID pParam) // RUN (called from single or multi-scen)
{
	CTracer * pTrace =(CTracer*) pParam;
	//MSG message;
	while (pTrace->GetContinue()) //(TRUE)//
	{
	/*	if (::PeekMessage(&message, NULL, 0,0, PM_REMOVE)) { // not working
				::TranslateMessage(&message);
				::DispatchMessage(&message);
			} 	if (STOP) return 0;	*/	
		if (BTRVersion - 4.5 < 1.e-6)
			pTrace->Draw(); // 4.5
		else 
			pTrace->TraceAll(); // new - 5.0
			//pTrace->ClearArrays();	pTrace->TestMem();
	}
	
	/*CString S;
	CCriticalSection cs;
	cs.Lock();
	int m_ID = pTrace->Get_ID();
	int min = pTrace->GetBMLmin(); // from BML (start)
	int max = pTrace->GetBMLmax(); // upto (include)
	cs.Unlock();*/
	//S.Format("Thread %d done \n %d - %d bml", m_ID, min + 1, max + 1);
	//::MessageBox(NULL, S, "BTR 5 ThreadFunc", 0);
	
	return 0;
}

void CBTRDoc::SuspendTracer(int iIndex, bool bRun)
{
	if (!bRun) //stop
	{
		if (m_pTraceThread[iIndex])
		{
			CTracer::cs.Lock();
			m_pTraceThread[iIndex]->SuspendThread();
			CTracer::cs.Unlock();
		}
	}

	else //run
	{
		m_Tracer[iIndex].SetDraw(OptDrawPart);
		
		if (m_pTraceThread[iIndex])
		{
			m_pTraceThread[iIndex]->ResumeThread();
		}
		else m_pTraceThread[iIndex] = AfxBeginThread(ThreadFunc, &m_Tracer[iIndex]);

		//if (thread != NULL)
		//m_pTraceThread[iIndex]->SetThreadPriority(-1);
	

	}
}

void CBTRDoc::SuspendAll(bool bRun)
{
	for (int i = 0; i < ThreadNumber; i++) SuspendTracer(i, bRun);
	
}

void CBTRDoc:: P_CalculateAngularProf(CPlate * plate) // not called- replaced by ShowBPdata
{
	if (plate == NULL) return;

	CLimitsDlg dlg; 
	dlg.m_StrMin = "Min, mrad";
	dlg.m_StrMax = "Max, mrad";
	dlg.m_Sstep = "Step, mrad";
	dlg.m_Xmin = plate->MinAngle; 
	dlg.m_Xmax = plate->MaxAngle;
	dlg.m_Step = plate->StepAngle;
	if (dlg.DoModal() == IDOK) {
		plate->MinAngle = (double) dlg.m_Xmin;
		plate->MaxAngle = (double) dlg.m_Xmax;
		plate->StepAngle = (double) dlg.m_Step;
		//Xmax = (double) dlg.m_Xmax;
	} 

	double power;
	CString S1, S2, S3;
	float AXmrad, AYmrad, amrad;
	vector<minATTR> & arr = m_GlobalVector;// m_AttrVector[MaxThreadNumber - 1];

	plate->InitAngleArray(plate->MinAngle, plate->MaxAngle, plate->StepAngle);

	//for (int k = 0; k < ThreadNumber; k++) {
		//arr = m_AttrVector[k];
		if (arr.size() < 1) return;
		for (int i = 0; i < (int)arr.size(); i++) {
			minATTR &tattr = arr[i];
			if (tattr.Nfall == plate->Number) {
				if (!OptAtomPower && tattr.Charge == 0) continue;
				if (!OptNegIonPower && tattr.Charge < 0) continue;
				if (!OptPosIonPower && tattr.Charge > 0) continue;
				power = (double) tattr.PowerW;
				AXmrad = tattr.AXmrad;
				AYmrad = tattr.AYmrad;
				amrad = sqrt(AXmrad * AXmrad + AYmrad * AYmrad);
				plate->DistributeAngle(amrad, power);
			} // equal numbers
		} // i
	//} //k
	S1 = ""; S2 = ""; S3 = "";
	if (OptAtomPower) S1 = "A";
	if (OptNegIonPower) S2 = "N";
	if (OptPosIonPower) S3 = "P";
	if (plate->Load != NULL) plate->Load->Particles = S1 + S2 + S3;
	return;
}

void CBTRDoc::SetNullLoad(CPlate * plate) // create zero load with default mesh options
{ 
	int gridNX = DefLoadNX;
	int gridNY = DefLoadNY;
	double Nres;
	C3Point H;
	double hx, hy;
	if (DefLoadOptN) { // fixed mesh size
		hx = plate->Xmax / gridNX;
		hy = plate->Ymax / gridNY;

		if (DefLoadStepRound) { // round gridsteps
			H = GetMantOrder(plate->Xmax / gridNX);
			hx = H.X * H.Z;
			Nres = plate->Xmax / hx;
			if (Nres / gridNX < 0.8) hx = hx * 0.5; // refine if Nres < requested too much
			
			H = GetMantOrder(plate->Ymax / gridNY);
			hy = H.X * H.Z;
			Nres = plate->Ymax / hy;
			if (Nres / gridNY < 0.8) hy = hy * 0.5; // refine if Nres < requested too much
		}
	}
	else { // fixed step
		hx = DefLoadStepX;
		hy = DefLoadStepY;
	}

	// set 1mm limit to grid step - limited by minAttr pos X,Y precision
	if (hx < 0.001) hx = 0.001;
	if (hy < 0.001) hy = 0.001;
	plate->ApplyLoad(TRUE, hx, hy); //delete old + create new array (Loaded >> TRUE)
	plate->Loaded = TRUE;
}

void CBTRDoc:: P_CalculateLoad(CPlate * plate)
{
	vector<minATTR> * parr = &m_GlobalVector;
	P_CalculateLoad(plate, parr);
}

void CBTRDoc:: P_CalculateLoad(CPlate * plate, vector<minATTR> * parr)
{
	if (plate == NULL) return;// -1;
	//if (plate->Loaded == FALSE) return;
	//if (plate->Touched == FALSE) return;
	CLoad * L = plate->Load;
	pSetView->Load = L;
	//vector<minATTR> & arr = m_GlobalVector;// m_AttrVector[MaxThreadNumber - 1];
	C3Point Pgl, Ploc;
	double power;
	int run; 
	CString S1, S2, S3;
	
		for (int i = 0; i < (int)parr->size(); i++) {
			minATTR &tattr = parr->at(i);
			power = (double)tattr.PowerW;
			run = (unsigned char)tattr.run;

			if (tattr.Nfall == plate->Number) {
				//count
				if (MAXSCEN == 1) { // SINGLE mode: keep Apow from run1 only! 
					if (tattr.Charge == 0 && run == 1) plate->AtomPower += power;//
					if (tattr.Charge == 0 && run !=1 )  continue; // no Map
					if (tattr.Charge < 0) plate->NegPower += power;
					if (tattr.Charge > 0) plate->PosPower += power;
				}
				else { // MULTI: keep Apow for each RUN - to normalize to Pn
					if (tattr.Charge == 0) plate->AtomPower += power;//
					if (tattr.Charge == 0 && run !=1 ) continue; // no Map
					if (tattr.Charge < 0) plate->NegPower += power;
					if (tattr.Charge > 0) plate->PosPower += power;
				}
				//if (tattr.Charge == 0 && !OptAtomPower)  continue; // not deposit
				//if (tattr.Charge < 0 && !OptNegIonPower) continue; // not deposit
				//if (tattr.Charge > 0 && !OptPosIonPower) continue; // not deposit
								
				//rest - Deposit power
				Ploc.X = tattr.Xmm * 0.001;// plate->GetLocal(Pgl);
				Ploc.Y = tattr.Ymm * 0.001;
				Ploc.Z = 0;
							//if (plate->WithinPoly(Ploc)) 
				plate->Load->Distribute(Ploc.X, Ploc.Y, power);
				
			} // Nfall == plate Num
		} // i
	
	if (plate->MAP) plate->Load->SetSumMax();//Loaded !!
	else { 
		plate->Load->Sum = plate->AtomPower + plate->NegPower + plate->PosPower;
		plate->Load->MaxVal = 0;
	}
	S1 = ""; S2 = ""; S3 = "";
	if (OptAtomPower) S1 = "A";
	if (OptNegIonPower) S2 = "N";
	if (OptPosIonPower) S3 = "P";
	plate->Load->Particles = S1 + S2 + S3;
	
}

void CBTRDoc:: P_CalculateCombi(bool update) // not active now - must be rewritten
// calculate maps + proect them on the marked (combi-)plate
{
	CWaitCursor wait;
	CPlate * Plate = pMarkedPlate;//
	if (Plate == NULL) return;// -1;
	if (OptCombiPlot > 1) Plate->Load->Clear();
	
	//CCriticalSection cs;
	//STOP = FALSE;
	CPlate * plate;
	double CombiSum = 0, CombiMax = 0;
	C3Point local, Local, Global;
	double power;
	double stepX;// = Plate->Load->StepX;
	double stepY;// = Plate->Load->StepY;
	POSITION pos = PlatesList.GetHeadPosition();
		while (pos != NULL) {// calc load on selected plates
			plate = PlatesList.GetNext(pos);
			if (!plate->Selected) continue;
			//if (OptCombiPlot > 1 && !plate->Solid) continue;

			if (plate->Loaded == TRUE) { // LOAD already exists
				stepX = Plate->Load->StepX;
				stepY = Plate->Load->StepY;
				if (OptParallel && update) { // recalculate load
					plate->Load->Clear();
					plate->ApplyLoad(TRUE, stepX, stepY); 
					P_CalculateLoad(plate); 
				}
				//else keep the existing load
			}
			
			else //(plate->Loaded == FALSE) { // not calculated yet
			{ 
					
					stepX = 0; stepY = 0;
					plate->ApplyLoad(TRUE, stepX, stepY); // optimize grid 
					P_CalculateLoad(plate); // with new grid steps
					SetLoadArray(plate, TRUE);
					
			}
			
			stepX = Plate->Load->StepX;
			stepY = Plate->Load->StepY;
		
			//plate->ApplyLoad(TRUE, Plate->Load->StepX, Plate->Load->StepY); // optimize grid 
			//P_CalculateLoad(plate); // with new grid steps

			CombiSum += plate->Load->Sum;
			CombiMax = Max(CombiMax, plate->Load->MaxVal);
			C3Point p0, p1, p2, p3, P1, P2;
			C3Point Size = plate->ProectCorners(Plate, p0, p1, p2, p3);
			if (Size.X * Size.Y < 1.e-6) continue;

			double Xmin = Min(Min(p0.X, p1.X), Min(p2.X, p3.X));
			double Xmax = Max(Max(p0.X, p1.X), Max(p2.X, p3.X));
			double Ymin = Min(Min(p0.Y, p1.Y), Min(p2.Y, p3.Y));
			double Ymax = Max(Max(p0.Y, p1.Y), Max(p2.Y, p3.Y));
			// double Zmax = Max(Max(fabs(p0.Z), fabs(p1.Z)), Max(fabs(p2.Z), fabs(p3.Z)));

			//Xmin = Max(0, Xmin); Xmax = Min(Xmax, Plate->Xmax);
			//Ymin = Max(0, Ymin); Ymax = Min(Ymax, Plate->Ymax);

			for (int i = 0; i <= Plate->Load->Nx; i++) {
				Local.X = i * stepX;
				if (Local.X < Xmin || Local.X > Xmax + 0.5 * stepX) continue;
				for (int j = 0; j <= Plate->Load->Ny; j++) {
				Local.Y = j * stepY;
				if (Local.Y < Ymin || Local.Y > Ymax + 0.5 * stepY) continue;
				Local.Z = -10; P1 = Plate->GetGlobalPoint(Local);
				Local.Z = 10; P2 = Plate->GetGlobalPoint(Local);
				local = plate->FindLocalCross(P1, P2);
				if (plate->WithinPoly(local))
				Plate->Load->Val[i][j] = plate->Load->GetVal(local.X, local.Y); 
				} // j
			} //i

		} // calc load on selected plates and distribute on combi

	ShowProfiles = TRUE;
	Plate->Loaded = TRUE;
	Plate->Load->SetSumMax();
	Plate->Load->Sum = CombiSum;
	Plate->Load->MaxVal = CombiMax;
	pSetView->Load = Plate->Load;
	CString S1, S2, S3;
	S1 = ""; S2 = ""; S3 = "";
	if (OptAtomPower) S1 = "A";
	if (OptNegIonPower) S2 = "N";
	if (OptPosIonPower) S3 = "P";
	Plate->Load->Particles = S1 + S2 + S3;

	pLoadView->SetLoad_Plate(Plate->Load, Plate);
//	pLoadView->ShowLoad = TRUE;
	
}

void CBTRDoc::OnUpdatePlateSelect(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	//pMainView->OnUpdatePlateSelect(pCmdUI);
	if (OptParallel) pCmdUI->SetText("Calculate");
	else pCmdUI->SetText("Select / Unselect");
}

void CBTRDoc::OnUpdateSurfacesSortincr(CCmdUI *pCmdUI) // disable
{
	pCmdUI->Enable(FALSE);
}

void CBTRDoc::OnUpdateOptionsSurfacesAdd(CCmdUI *pCmdUI) // disable
{
	pCmdUI->Enable(TRUE);
}



void CBTRDoc::OnPlateClearSelect()
{
	//if (OptCombiPlot > 0) return;// = FALSE;
	CPlate * plate;
	POSITION pos = PlatesList.GetHeadPosition();
		while (pos != NULL) {
			plate = PlatesList.GetNext(pos);
			plate->Selected = FALSE;
		}
		
		
		if (pMarkedPlate != NULL) {
			pMarkedPlate->Selected = TRUE;
			OptCombiPlot = 0;
		}
		else 
			OptCombiPlot = -1; ///-1 - no load, 0 - map is calculated, >0 - selected but not calculated
	//	OnShow();
}


CString  GetMachine()
{
	unsigned long Size1 = MAX_COMPUTERNAME_LENGTH + 1;
	char machine[MAX_COMPUTERNAME_LENGTH + 1];
	CString CompName = "";
	if (::GetComputerName(machine, &Size1)!= 0) {
		CompName = machine; CompName.MakeUpper();
	}
	return CompName;
}

CString  GetUser()
{
	unsigned long Size2 = MAX_COMPUTERNAME_LENGTH + 1;
	char user[MAX_COMPUTERNAME_LENGTH + 1];
	CString UserName = "";
	if (::GetUserName(user, &Size2)!= 0) {
		UserName = user; UserName.MakeUpper();
	}
	return UserName;
}

void CBTRDoc::ShowPlasmaCut(int icut) // 0- horiz along X, 1- vert along X, 2- YZ, 3 - poloidal (RZ)
{
	//CPlate * plate0 = new CPlate();
	C3Point Step = pPlasma->Step;//orthogonal
	double  StepPsi = pPlasma->StepFlux;//poloidal
	double  StepR = pPlasma->StepR;//poloidal
	double  StepZ = pPlasma->StepZ;//poloidal
	C3Point Orig = pPlasma->Orig;
	double Zmin = pPlasma->Zmin;
	double Zmax = pPlasma->Zmax;
	//C3Point OrigRZ(1000, pPlasma->Rmin, pPlasma->Zmin);

	int Nx = pPlasma->Nx;
	int Ny = pPlasma->Ny;
	int Nz = pPlasma->Nz;// Nzpol = Nz;
	int Nr = pPlasma->Npsi;
		
	double dX = Nx * Step.X;
	double dY = Ny * Step.Y;
	double dZ = Nz * Step.Z;
	double dZpol = Nz * StepZ;//pPlasma->Zmax - pPlasma->Zmin;
	double dR = Nr * StepR;
	double dPsi = Nr * StepPsi;
	//double dS = dY; // default - horizontal step
	//double stepS = Step.Y;
	double Dhor, Dvert;
	double Shor, Svert;
	CString S;
	
	switch (icut) {
	case 0: Dhor = dX; Dvert = dY; Shor = Step.X; Svert = Step.Y;  break;
	case 1: Dhor = dX; Dvert = dZ; Shor = Step.X; Svert = Step.Z;  break;
	case 2: Dhor = dY; Dvert = dZ; Shor = Step.Y; Svert = Step.Z;  break;
	case 3: Dhor = dR;   Dvert = dZpol; Shor = StepR;    Svert = StepZ;  break;
	case 4: Dhor = dPsi; Dvert = dZpol; Shor = StepPsi;  Svert = StepZ;  break;
	default: Dhor = dY; Dvert = dZ; Shor = Step.Y; Svert = Step.Z; break;
	}

	C3Point LB = Orig; // left-bottom
	C3Point dP = C3Point(Dhor, Dvert, 0);//area size
	C3Point RT = LB + dP;// right-top   +C3Point(dX, 0, 0);  
	if (icut == 2) { // poloidal
		LB = C3Point(1000, Orig.Y, Orig.Z);
		RT = C3Point(1000, Orig.Y + dY, Orig.Z + dZ);
	}

	if (icut == 3) { // poloidal R-Z
		LB = C3Point(1000, -0.5 * dR , Zmin);
		RT = C3Point(1000, 0.5 * dR, Zmax);
	}
	if (icut == 4) { // poloidal Psi-Z
		LB = C3Point(1000, 0, Zmin);
		RT = C3Point(1000, 1, Zmax);
	}
	pMarkedPlate = new CPlate();
	pMarkedPlate->SetFromLimits(LB, RT);
	//S.Format("R/Z steps %g /%g   DR/DZ  %g /%g", stepR, stepZ, DR, DZ);	AfxMessageBox(S);
	pMarkedPlate->Load = new CLoad(Dhor, Dvert, Shor, Svert);
		
	switch (icut) {
		case 0: pMarkedPlate->Load->SetValArray(pPlasma->CutHor, Nx, Ny);  
			pMarkedPlate->Comment = "Horizontal "; break;
		case 1: pMarkedPlate->Load->SetValArray(pPlasma->CutVert, Nx, Nz); 
			pMarkedPlate->Comment = "Vertical "; break;
		case 2: pMarkedPlate->Load->SetValArray(pPlasma->CutCS, Ny, Nz);   
			pMarkedPlate->Comment = "Cross "; break;
		case 3: pMarkedPlate->Load->SetValArray(pPlasma->CutRZ, Nr, Nz);   
			pMarkedPlate->Comment = "Poloidal R-Z "; break;
		case 4: pMarkedPlate->Load->SetValArray(pPlasma->CutPsiZ, Nr, Nz); 
			pMarkedPlate->Comment = "Poloidal Psi-Z "; break;
		default: break;
	}
	//if (icut == 0) pMarkedPlate->Load->SetValArray(pPlasma->CutHor, Nx, Ny); //load->Val = pPlasma->CutHor; //PSI;
	//else pMarkedPlate->Load->SetValArray(pPlasma->CutVert, Nx, Nz);
	
	pMarkedPlate->Load->SetSumMax();
	if (pMarkedPlate->Load->MaxVal < 1.e-10) {
		AfxMessageBox("Zero power along cut plane");
		//return;
	}
	pMarkedPlate->Load->iProf = (int)(1.0 * pMarkedPlate->Load->iMaxVal);
	pMarkedPlate->Load->jProf = (int)(1.0 * pMarkedPlate->Load->jMaxVal);
	pMarkedPlate->Number = 2000;
	pMarkedPlate->Selected = TRUE;
	pMarkedPlate->Loaded = TRUE;
	pMarkedPlate->SmoothDegree = SmoothDegree;
	pMarkedPlate->Comment += "Beam Cut ";
	//if (icut == 1) pMarkedPlate->Comment += " - Vertical";
	//else pMarkedPlate->Comment += " - Horizontal";
	OptParallel = 0;
	OptCombiPlot = 0;
	pLoadView->Cross.X = pMarkedPlate->Load->iProf * pMarkedPlate->Load->StepX;
	pLoadView->Cross.Y = pMarkedPlate->Load->jProf * pMarkedPlate->Load->StepY;

	ShowProfiles = TRUE;
	pMarkedPlate->ShowLoadState();
	pSetView->InvalidateRect(NULL, TRUE); //	OnPlotMaxprofiles();
}

void CBTRDoc::ShowPlasmaCutDiagonal() // not called   
{
	//CPlate * plate0 = new CPlate();
	double StepX = pPlasma->CutStepX;
	double StepY = pPlasma->CutStepY;
	C3Point Orig = pPlasma->CutOrig;
	C3Point Dest = pPlasma->CutDest;
	int Nx = pPlasma->CutNx;
	int Ny = pPlasma->CutNy;
	
	double dX = Nx * StepX;
	double dY = Ny * StepY;
	
	CString S;
		
	C3Point LB = Orig;
	C3Point RT = Dest; //Orig + C3Point(dX, 0, 0) + dP * 0.5;
	pMarkedPlate = new CPlate();
	pMarkedPlate->SetFromLimits(LB, RT);
	//S.Format("R/Z steps %g /%g   DR/DZ  %g /%g", stepR, stepZ, DR, DZ);	AfxMessageBox(S);
	pMarkedPlate->Load = new CLoad(dX, dY, StepX, StepY);

	pMarkedPlate->Load->SetValArray(pPlasma->CutCS, Nx, Ny); //load->Val = pPlasma->CutHor; //PSI;
	pMarkedPlate->Load->SetSumMax();
	if (pMarkedPlate->Load->MaxVal < 1.e-10) {
		AfxMessageBox("Zero power along diag cut plane");
		return;
	}
	pMarkedPlate->Load->iProf = (int)(1.0 * pMarkedPlate->Load->iMaxVal);
	pMarkedPlate->Load->jProf = (int)(1.0 * pMarkedPlate->Load->jMaxVal);
	pMarkedPlate->Number = 2000;
	pMarkedPlate->Selected = TRUE;
	pMarkedPlate->Loaded = TRUE;
	pMarkedPlate->SmoothDegree = SmoothDegree;
	pMarkedPlate->Comment = "Plasma Cut";
	pMarkedPlate->Comment += " - Diagonal";
	
	OptParallel = 0;
	OptCombiPlot = 0;
	pLoadView->Cross.X = pMarkedPlate->Load->iProf * pMarkedPlate->Load->StepX;
	pLoadView->Cross.Y = pMarkedPlate->Load->jProf * pMarkedPlate->Load->StepY;

	ShowProfiles = TRUE;
	pMarkedPlate->ShowLoadState();
	pSetView->InvalidateRect(NULL, TRUE);//	OnPlotMaxprofiles();
}

void CBTRDoc::OnBeaminplasmaVerticalplane()
{
	OnShow();
	if (pPlasma->Nrays < 1 && TracksCalculated == 0) StartAtomsFromEmitter();// trace atoms in plasma
			//CalculateTracks();//fill Plasma Cut-planes
			//ShowPlasmaCut(1);// vertical
	ShowPlasmaCut(1); //  0- horiz along X, 1- vert along X, 2 - YZ, 3 - RZ, 4 - psi-Z
}

void CBTRDoc::OnBeaminplasmaHorizontalplane()
{
	OnShow();
	if (pPlasma->Nrays < 1 && TracksCalculated == 0) StartAtomsFromEmitter();// trace atoms in plasma
	ShowPlasmaCut(0); //  0- horiz along X, 1- vert along X, 2 - YZ, 3 - RZ, 4 - psi-Z
}


void CBTRDoc::OnBeaminplasmaPoloidalRz()
{
	OnShow();
	if (pPlasma->Nrays < 1 && TracksCalculated == 0) StartAtomsFromEmitter();// trace atoms in plasma
	ShowPlasmaCut(3); //  0- horiz along X, 1- vert along X, 2 - YZ, 3 - RZ, 4 - psi-Z
}


void CBTRDoc::OnBeaminplasmaPoloidalPsiZ()
{
	OnShow();
	if (pPlasma->Nrays < 1 && TracksCalculated == 0) StartAtomsFromEmitter();// trace atoms in plasma
	ShowPlasmaCut(4); //  0- horiz along X, 1- vert along X, 2 - YZ, 3 - RZ, 4 - psi-Z
}


void CBTRDoc::OnPlotPSI()
{ // ReadPlasmaPSI
	int Nr = 50; //pPlasma->psiNr;
	int Nz = 50; // pPlasma->psiNz;
	double DR = pPlasma->Rmax - pPlasma->Rmin; //FWRmin
	double DZ = pPlasma->Zmax - pPlasma->Zmin; //FWZmax
	double stepR = DR / Nr; // pPlasma->StepR;
	double stepZ = DZ / Nz; // pPlasma->StepZ;
	//S.Format("R/Z steps %g /%g   DR/DZ  %g /%g", stepR, stepZ, DR, DZ);	AfxMessageBox(S);
	CLoad * load = new CLoad(DR, DZ, stepR, stepZ);
	//S.Format("load limits Nx %d Ny %d", load->Nx, load->Ny);	AfxMessageBox(S);
	//if (pPlasma->PSIloaded) load->Val = pPlasma->PSI;
	//else {
	double R, Z, Z0, psi, dens;
	if (pPlasma->PSIloaded) Z0 = pPlasma->Zmin; // Zmin is already in TOR ref
	else Z0 = - (pPlasma->PlasmaMinR) * (pPlasma->PlasmaEll);//FWZmin
	for (int i = 0; i < Nr; i++) {
		R = pPlasma->Rmin + i * stepR;
		for (int k = 0; k < Nz; k++) {
			Z = Z0 + k * stepZ;
			psi = pPlasma->GetPSI(R, Z); // R,Z must be in TOROIDAL ref !!! GetTorRZ is usually called before
			dens = pPlasma->GetPlasmaNePSI(psi);
			load->Val[i][k] = dens;// psi;
		}
	}

	//} // PSI not loaded

	load->SetSumMax();
	if (load->MaxVal < 1.e-10) {
		AfxMessageBox("Zero load (PSI array)");
		return;
	}
	//S.Format("PSImax = %g", load->MaxVal);	AfxMessageBox(S);
	load->iProf = (int)(0.5 * load->Nx);// iMaxVal);
	load->jProf = (int)(0.5 * load->Ny);// jMaxVal);
	if (pPlasma->PSIloaded) {
		load->iProf = (int)(0.5 * load->iMaxVal);
		load->jProf = (int)(0.5 * load->jMaxVal);
	}
	pMarkedPlate = new CPlate();
	C3Point LB(pPlasma->Rmin, Z0, 0); //pPlasma->Zmin);
	C3Point RT(pPlasma->Rmax, Z0 + DZ, 0); //pPlasma->Zmax);
	pMarkedPlate->SetFromLimits(LB, RT);
	pMarkedPlate->Load = load;
	//pMarkedPlate->Load->SetSumMax();
	pMarkedPlate->Number = 1000;
	pMarkedPlate->Selected = TRUE;
	pMarkedPlate->Loaded = TRUE;
	//pMarkedPlate->SmLoad = new CLoad(DR, DZ, stepR, stepZ);
	//pMarkedPlate->SmoothDegree = SmoothDegree;
	//pMarkedPlate->filename = name;
	pMarkedPlate->Comment = "DENSITY = f(PSI(R,Z))";
	OptParallel = 0;
	OptCombiPlot = 0;
	//pLoadView->Cross.X = load->iProf * load->StepX;
	//pLoadView->Cross.Y = load->jProf * load->StepY;
		
	ShowProfiles = TRUE;
	//pMarkedPlate->ShowLoadState();
	pMarkedPlate->ShowLoad();
	//	if (pLoadView->ShowLoad == TRUE) pLoadView->InvalidateRect(NULL, TRUE);
	pSetView->InvalidateRect(NULL, TRUE);//	OnPlotMaxprofiles();

/*	// OnPlotLoadmap
	if (OptCombiPlot == -1) return;//(pMarkedPlate == NULL) return;
	if (!(pMarkedPlate->Loaded)) {
		AfxMessageBox("no load"); return;
	}
	//if (!(pMarkedPlate->Selected)) return;
	//pMarkedPlate->Load->SetSumMax(); // max profiles
	pLoadView->Contours = FALSE;
	pLoadView->Cross.X = pMarkedPlate->Load->iProf * pMarkedPlate->Load->StepX;
	pLoadView->Cross.Y = pMarkedPlate->Load->jProf * pMarkedPlate->Load->StepY;
	pMarkedPlate->ShowLoad();// pLoadView->SetPlate(pMarkedPlate);
	// OnPlotLoadmap */
}


void CBTRDoc:: WriteExitVector()
{
	CString name = "RID_ions.txt"; 
	C3Point P0, P1, V;
	EXIT_RAY ray;
	double t, power, vel;
	double Sum = 0;
	long count = 0;
	CWaitCursor wait;
	FILE * fout;
	fout = fopen(name, "w");
	if (fout == NULL) {
			AfxMessageBox("failed to create file \n" + name, MB_ICONSTOP | MB_OK);
			return;
	}
	fprintf(fout, "Dumped Ions position, velocity & Power (RID plate)\n");
	fprintf(fout, " N \t\t X\t\tY\t\tZ\t\tVx/V\t\tVy/V\t\tVz/V\t\t Power, W \n");

	vector <EXIT_RAY> & vect = m_ExitVector[MaxThreadNumber-1];
			
	for (int k = 0; k < ThreadNumber; k++) {
		vect = m_ExitVector[k];
		for (int i = 0; i < (long)vect.size(); i++) {
			EXIT_RAY &ray = vect[i];
			P0 = ray.Position;
			V = ray.Velocity;
			vel = ModVect(V);
			power = ray.Power; 
			count++;
			Sum += power;
			if (power > 1.e-12)	fprintf(fout, " %ld \t %le %le %le\t %le %le %le\t %le \n", 
											count, P0.X, P0.Y, P0.Z, V.X/vel, V.Y/vel, V.Z/vel, power);
		
		}//i
	}//k

	fprintf(fout, "\n  Totally dumped  %le W \n %d rays ", Sum, count);
		
	fclose(fout);
} 

void CBTRDoc:: CalculateTracks() // NOT called (by OnBeaminplasmaVerticalplane()
// replaced by CalculateTracks from DuctExit
{
	CalculateThickFocused();// actual beam axes
	TracksCalculated = 1;
	//SetDecayInPlasma();
/*	C3Point P0, P1, V;
	EXIT_RAY ray;
	double t, power, vel;
	CWaitCursor wait;
	
	pBeamHorPlane->Load->Clear();
	pBeamVertPlane->Load->Clear();*/

	//vector <EXIT_RAY> & vect = m_ExitVector[MaxThreadNumber-1];
				
/*	for (int k = 0; k < ThreadNumber; k++) {
		vector <EXIT_RAY> * vect = m_Tracer[k].exitarr;
		//vect = m_ExitVector[k];
		if (vect->size() < 1) continue;
		for (int i = 0; i < (long)vect->size(); i++) {
			EXIT_RAY ray = vect->at(i);
			P0 = ray.Position;
			V = ray.Velocity;
			vel = fabs(V.X); // ModVect(V);
			t =  fabs(PlasmaXmax - P0.X) / V.X;
			P1.X = PlasmaXmax;
			P1.Y = P0.Y + V.Y * t;
			P1.Z = P0.Z + V.Z * t;
			power = ray.Power; 
			
			GetFillDecayBetween(P0, P1, power);
			//CalculatePowerTrack(P0, P1, power, vel);
		}
	}*/

	//SetBeamDecay(); // calls GetFillDecayBetween
	
/*	double dL = 0.1;
	CArray<double, double> * Parr = pPlasma->GetPowerDecayArray(P0, P1, dL);
	int Nmax = Parr->GetSize();
	//decay = Parr->GetAt(Nmax - 1);

	//distribute Power to Hor/Vert Planes
	double Xloc, Yloc, Power;
	C3Point P;
	double PathLen = ModVect(P1 - P0);
	int N = (int)ceil(PathLen / dL);
	C3Point PathVect = (P1 - P0) / PathLen;
	for (int i = 0; i < Nmax; i++) {
		P = P0 + PathVect * (i * dL);
		Power = Parr->GetAt(i);
		Xloc = P.X - pBeamHorPlane->Orig.X;
		Yloc = P.Y - pBeamHorPlane->Orig.Y;
		pBeamHorPlane->Load->Distribute(Xloc, Yloc, Power);
		Xloc = P.X - pBeamVertPlane->Orig.X;
		Yloc = P.Z - pBeamVertPlane->Orig.Z;
		pBeamVertPlane->Load->Distribute(Xloc, Yloc, Power);
	}*/
	//pBeamHorPlane->Load->SetSumMax();
	//pBeamVertPlane->Load->SetSumMax();
}

void CBTRDoc:: CalculatePowerTrack(C3Point Start, C3Point Finish, double StartPower, double Vel) 
// project power track on plane
{
	if (OptCombiPlot == -1) return; //(pMarkedPlate == NULL) return;
	CPlate * plate = pMarkedPlate; // vert or hor plane in plasma
	double stepZ = 0.1; // the layer thickness
	double stepX = plate->Load->StepX;
	double stepY = plate->Load->StepY;
	double Vol = stepX * stepY * stepZ; // cell volume
	double stepL = stepX;// along X   // 0.5 * sqrt(stepX*stepX + stepY*stepY);//Min(stepX, stepY) * 0.5; // track step
	double dt = stepL / Vel; //both along X  
	double l = 0, lmax = (Finish - Start).X;// along X    // GetDistBetween(Start, Finish);
	C3Point dL = (Finish - Start) * stepL / lmax;//along track
	double power;
	C3Point Ploc;
	C3Point P = Start;

/*	while (l <= lmax) {
		power = StartPower * GetDecay(P);
		Ploc = plate->GetLocal(P);
		if (fabs(Ploc.Z) < 0.5*stepZ)	plate->Load->Distribute(Ploc.X, Ploc.Y, power * dt / Vol);
		l += stepL;//along track
		P = P + dL;
	}
*/
}
void CBTRDoc::OnDataSaveaddsurf()
{
	char name[1024];
	if (AddPlatesNumber > 0) {
			int reply = AfxMessageBox("Save the List of Additional Surfaces?", 3);
			if (reply != IDYES) return;

		CFileDialog dlg1(FALSE, "txt; dat | * ", "", OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"Additional Surfaces data file (*.txt);(*.dat) | *.txt; *.TXT; *.dat; *.DAT | All Files (*.*) | *.*||", NULL);
		if (dlg1.DoModal() == IDOK) {
			strcpy(name, dlg1.GetPathName());
			WriteAddPlates(name);
		}
	}
}

void CBTRDoc::OnDataReadaddsurf()
{
	//ReadAddPlates();
	//CPlate::AbsYmin = 1000; CPlate::AbsYmax = -1000;
	//CPlate::AbsZmin = 1000; CPlate::AbsZmax = -1000;
	int added = ReadAddPlates();//AddPlatesNumber += added;
	AppendAddPlates();
	//ModifyArea(FALSE);
	
	//CString S;
	//S.Format("Total addSurf %d (added %d) ", AddPlatesNumber, added);
	//AfxMessageBox(S);
	ClearAllPlates();
	OnShow();
}

void CBTRDoc::OnUpdateDataSaveaddsurf(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(AddPlatesNumber > 0);
}

void CBTRDoc::OnPlasmaPsi()
{
	//if (!OptBeamInPlasma) return;
	ReadPlasmaPSI();//v 4.5
	//OnResultsPdpoutputTable(); // in v4.0

}

void CBTRDoc::OnUpdateTasksBeamplasma(CCmdUI *pCmdUI)
{
	/*if (MaxPlasmaDensity * MaxPlasmaTe < 1.e-6) //|| pTorCrossPlate == NULL) 
		pCmdUI->Enable(FALSE);
	else */pCmdUI->Enable(TRUE);
}

void CBTRDoc::OnUpdatePlotPenetration(CCmdUI *pCmdUI)
{
	/*if (MaxPlasmaDensity * MaxPlasmaTe < 1.e-6)// || pTorCrossPlate == NULL) 
		pCmdUI->Enable(FALSE);
	else*/ pCmdUI->Enable(TRUE);
}

void CBTRDoc::OnTraceBigAtom()
{		
	CAtomDlg dlg;
	dlg.m_Option = "get reionised";
	dlg.m_PosX = SingleBPpos.X;
	dlg.m_PosY = SingleBPpos.Y;
	dlg.m_PosZ = SingleBPpos.Z;
	dlg.m_Vx = SingleBPvel.X;
	dlg.m_Vy = SingleBPvel.Y;
	dlg.m_Vz = SingleBPvel.Z;
	dlg.m_Step = SingleBPstep;
	dlg.m_OptReion = !OptReionStop;
	dlg.m_Energy = SingleBPenergy;

	if (dlg.DoModal() == IDOK) {
		OptReionStop = !dlg.m_OptReion;
		C3Point Pos = C3Point(dlg.m_PosX, dlg.m_PosY, dlg.m_PosZ);
		C3Point V = C3Point(dlg.m_Vx, dlg.m_Vy, dlg.m_Vz);
		TraceSingleParticle(ATOM, Pos, V, dlg.m_Step, dlg.m_Energy);
		OptTraceSingle = TRUE;
		SingleBP = ATOM;
		SingleBPpos = Pos;
		SingleBPvel = V;
		SingleBPstep = dlg.m_Step;
		SingleBPenergy = dlg.m_Energy;
	}
}

void CBTRDoc::OnTraceBigIon()
{
	CAtomDlg dlg;
	dlg.m_Option = " no option ";
	dlg.m_PosX = SingleBPpos.X;
	dlg.m_PosY = SingleBPpos.Y;
	dlg.m_PosZ = SingleBPpos.Z;
	dlg.m_Vx = SingleBPvel.X;
	dlg.m_Vy = SingleBPvel.Y;
	dlg.m_Vz = SingleBPvel.Z;
	dlg.m_Step = SingleBPstep;
	dlg.m_Energy = SingleBPenergy;

	if (dlg.DoModal() == IDOK) {
		C3Point Pos = C3Point(dlg.m_PosX, dlg.m_PosY, dlg.m_PosZ);
		C3Point V = C3Point(dlg.m_Vx, dlg.m_Vy, dlg.m_Vz);
		TraceSingleParticle(POSION, Pos, V, dlg.m_Step, dlg.m_Energy);	
		OptTraceSingle = TRUE;
		SingleBP = POSION;
		SingleBPpos = Pos;
		SingleBPvel = V;
		SingleBPstep = dlg.m_Step;
		SingleBPenergy = dlg.m_Energy;
	}
}

void CBTRDoc::OnTraceBigElectron()
{
	CAtomDlg dlg;
	dlg.m_Option = "relativistic";
	dlg.m_PosX = SingleBPpos.X;
	dlg.m_PosY = SingleBPpos.Y;
	dlg.m_PosZ = SingleBPpos.Z;
	dlg.m_Vx = SingleBPvel.X;
	dlg.m_Vy = SingleBPvel.Y;
	dlg.m_Vz = SingleBPvel.Z;
	dlg.m_Step = SingleBPstep;
	dlg.m_Energy = SingleBPenergy;
	if (SingleBP == RELECTRON) dlg.m_OptReion = TRUE;
	else dlg.m_OptReion = FALSE;

	if (dlg.DoModal() == IDOK) {
		
		C3Point Pos = C3Point(dlg.m_PosX, dlg.m_PosY, dlg.m_PosZ);
		C3Point V = C3Point(dlg.m_Vx, dlg.m_Vy, dlg.m_Vz);
		if (dlg.m_OptReion) TraceSingleParticle(RELECTRON, Pos, V, dlg.m_Step,dlg.m_Energy);	
		else TraceSingleParticle(ELECTRON, Pos, V, dlg.m_Step, dlg.m_Energy);	
		OptTraceSingle = TRUE;
		SingleBP = ELECTRON;
		if (dlg.m_OptReion) SingleBP = RELECTRON;
		SingleBPpos = Pos;
		SingleBPvel = V;
		SingleBPstep = dlg.m_Step;
		SingleBPenergy = dlg.m_Energy;
	}
}

void CBTRDoc:: TraceSingleAgain(double X, double Y)
{
	SingleBPpos.X = X;
	SingleBPpos.Y = Y;
	TraceSingleParticle(SingleBP, SingleBPpos, SingleBPvel, SingleBPstep, SingleBPenergy);

}

void CBTRDoc:: TraceSingleParticle(int state, C3Point pos, C3Point v, double step, double energy)
{
	C3Point Pos, V;
	double Step = step;
	double Energy = energy; // eV
	Pos = pos; //C3Point(part.Attr.X, part.Attr.Y, part.Attr.Z);
	V = v; //C3Point(part.Attr.Vx, part.Attr.Vy, part.Attr.Vz);
	OptParallel = TRUE;
	OptCalcNeutralizer = TRUE;
	OptCalcRID = TRUE;
	OptCalcDuct = TRUE;
	CalcLimitXmin = 0;
	CalcLimitXmax = AreaLong + 1;
	
	if (!STOP) OnStop();
	for (int k = 0; k < ThreadNumber; k++) m_pTraceThread[k] = NULL;
	SetTitle("Single BP");
	//pMarkedPlate = NULL;
	OptCombiPlot = -1;
	ClearAllPlates();
	//ShowStatus();
	if (LoadSelected > 0)  SelectAllPlates();	
	//pMarkedPlate->Selected = FALSE; //OnShow();}
	
	NofBeamlets = 1;
	OptDrawPart = TRUE;
	InitTracers();//ThreadNumber = NofBeamlets;
	for (int i = 0; i < ThreadNumber; i++) {
	
	//m_Tracer[i].SetSingleAtom(Pos, V);
	m_Tracer[i].SetSingleParticle(state, Pos, V, Step, Energy);
	m_Tracer[i].SetLimits(0,-1);
	m_Tracer[i].SetDraw(TRUE);
		
	}

	STOP = FALSE;
	for (int i = 0; i < ThreadNumber; i++) m_Tracer[i].SetContinue(TRUE);
	SuspendAll(TRUE); // run = TRUE

	//OnStop();
	//for (int i = 0; i < ThreadNumber; i++)	m_Tracer[i].SetContinue(FALSE);
	//	SuspendAll(TRUE);
	//	WaitForSingleObject(m_pTraceThread[i]->m_hThread, 1000); //INFINITE);
	//	m_pTraceThread[i] = NULL;

	//	NofCalculated = 0;
		SetTitle("STOPPED");

}

double CBTRDoc:: GetIonStep(double x)
{
	if (x < NeutrXmin) return TraceStepL; // source ions
	else if (x >= NeutrXmin && x <= NeutrXmax) {
		if (OptThickNeutralization) return NeutrStepL; // THICK neutralization region
		else return TraceStepL; // source ions
	}
	else { //if (x > NeutrXmax) {
		if (x >= Xspec0 && x <= Xspec1) return IonStepLspec;
		else return IonStepL; // reion / resid ions
	}
}

void CBTRDoc::OnOptionsPlasma()
{
	//OnPlotPenetration();
	PlotBeamDecay(0, 0);// set plasma and calculate decay
	//AfxMessageBox("Under construction");
	return;

	if (!OptBeamInPlasma) return;
	CPlasmaDlg dlg;
	dlg.doc = this;
	dlg.m_Hw = PlasmaWeightH; dlg.m_H = (dlg.m_Hw > 1.e-6);
	dlg.m_Dw = PlasmaWeightD; dlg.m_D = (dlg.m_Dw > 1.e-6);
	dlg.m_Tw = PlasmaWeightT; dlg.m_T = (dlg.m_Tw > 1.e-6);

	dlg.m_HeW = PlasmaImpurW[0]; dlg.m_He = (dlg.m_HeW > 1.e-6);
	dlg.m_LiW = PlasmaImpurW[1]; dlg.m_Li = (dlg.m_LiW > 1.e-6);
	dlg.m_BeW = PlasmaImpurW[2]; dlg.m_Be = (dlg.m_BeW > 1.e-6);
	dlg.m_Bw = PlasmaImpurW[3];  dlg.m_B = (dlg.m_Bw > 1.e-6);
	dlg.m_Cw = PlasmaImpurW[4];  dlg.m_C = (dlg.m_Cw > 1.e-6);
	dlg.m_Nw = PlasmaImpurW[5];  dlg.m_N = (dlg.m_Nw > 1.e-6);
	dlg.m_Ow = PlasmaImpurW[6];  dlg.m_O = (dlg.m_Ow > 1.e-6);
	dlg.m_FeW = PlasmaImpurW[7]; dlg.m_Fe = (dlg.m_FeW > 1.e-6);
	
	if (dlg.DoModal() == IDOK) {
		 PlasmaWeightH = dlg.m_Hw;
		 PlasmaWeightD = dlg.m_Dw;
		 PlasmaWeightT = dlg.m_Tw;
		 PlasmaImpurW[0] = dlg.m_HeW;
		 PlasmaImpurW[1] = dlg.m_LiW;
		 PlasmaImpurW[2] = dlg.m_BeW;
		 PlasmaImpurW[3] = dlg.m_Bw;
		 PlasmaImpurW[4] = dlg.m_Cw;
		 PlasmaImpurW[5] = dlg.m_Nw;
		 PlasmaImpurW[6] = dlg.m_Ow;
		 PlasmaImpurW[7] = dlg.m_FeW;
		 SetImpurNumber();
	
	}
}

void CBTRDoc::OnResultsSavesummary()
{
	// TODO: Add your command handler code here
	OnResultsSaveList();

}

void CBTRDoc:: WriteFallVector()
{
	CString name = "Falls.txt"; 
	C3Point P;
	FALL_ATTR fall;
	int q, n;
	double alfa, power, alfdeg;
	double Sum = 0;
	long count = 0;

	CWaitCursor wait;
	FILE * fout;
	fout = fopen(name, "w");
	if (fout == NULL) {
			AfxMessageBox("failed to create file \n" + name, MB_ICONSTOP | MB_OK);
			return;
	}

	fprintf(fout, "N   Q    X,m   Y,m    Zm \t   Power,W     Alfa,rad \t  Alfa, deg\n" );
	
	vector <FALL_ATTR> & vect = m_FallVector[MaxThreadNumber-1];
			
	for (int k = 0; k < ThreadNumber; k++) {
		vect = m_FallVector[k];
		for (int i = 0; i < (long)vect.size(); i++) {
			FALL_ATTR &fall = vect[i];
			n = fall.Nfall;
			q = fall.Charge;
			P = fall.Position;
			alfa = fall.Angle;
			alfdeg = alfa / PI * 180;
			power = fall.Power; 
			count++;
			Sum += power;
			fprintf(fout, "%d  %d   %g  %g  %g       %g    %g    %f\n",  n, q, P.X, P.Y, P.Z, power, alfa, alfdeg);
					
		}//i
	}//k

	fprintf(fout, "\n  Total %g W, count %d \n", Sum, count);
		
	fclose(fout);
} 

void CBTRDoc::OnUpdateOptionsNbiconfig(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(!OptFree);
}

void CBTRDoc::OnInputFree()
{
	//OptFree = TRUE; // OLD
	//Version = 5.0;//BTRVersion = Version;

	OnShow();
	CString s;
	s.Format("Version %g is active!", BTRVersion);
	SetTitle(s);
	
	SetStatus();
	OnShow();
	//ClearData();
	//InitData();
	
}

void CBTRDoc::OnInputStandard()
{
	AfxMessageBox("Sorry! Version 4.5 is not available \n Test release");
	//OptFree = FALSE; // OLD
/*	Version = 4.5;
	BTRVersion = Version;
	
	OnShow();
	CString s;
	s.Format("Version %g is Set", BTRVersion);
	SetTitle(s);
		
	SetStatus();
	OnShow();
	//ClearData();
	//InitData();
	*/
}

void CBTRDoc::OnUpdateInputFree(CCmdUI *pCmdUI)// 5.0 
{
	/*bool opt = 0;
	if (BTRVersion > 4.6) opt = 1;
	pCmdUI->SetCheck(opt); //OLD (OptFree); */
	pCmdUI->SetCheck(1);//checked
}

void CBTRDoc::OnUpdateInputStandard(CCmdUI *pCmdUI) // 4.5
{
/*	bool opt = 1;
	if (BTRVersion > 4.6) opt = 0;
	pCmdUI->SetCheck(opt); //OLD (!OptFree);*/
	pCmdUI->SetCheck(0); // unchecked
}

void CBTRDoc::OnSurfacesRead()
{
	OnDataReadaddsurf();
}

void CBTRDoc::OnOptionInput()
{	
	CString s;
	s.Format("Version %g is active", BTRVersion);
	SetTitle(s);
	SetStatus();
	OnShow();
}


void CBTRDoc::OnPlateSelect() // add /remove to load list
{
	/*if (OptCombiPlot == -1) { //|| !(pDoc->pMarkedPlate->Loaded)) {
		ShowStatus(); return;
	}*/
	CPlate * plate = pMarkedPlate;
	if (plate == NULL) {
		AfxMessageBox("Plate not selected");
		return;
	}
	plate->Selected = TRUE;

	if (plate->Loaded == FALSE) { // not calculated yet
		SetNullLoad(plate);//plate->ApplyLoad(TRUE, 0, 0); // optimize grid //CreateBaseSingle();
						   // set new pMarkedPlate!!! OptCombiPlot >> 0
		SetLoadArray(plate, TRUE);
		OptCombiPlot = 0;
		P_CalculateLoad(plate);
	}

	//if (plate->Loaded) { //(OptCombiPlot == 0) { // base plate already exists
	//OnPlotMaxprofiles();
	//plate->ShowLoadState();
	
	OnPlateClearSelect();// unselect all excepting this plate
		
	//OnShow();

}

void CBTRDoc::OnPlateModify()
{
	CSurfDlg dlg;
	CPlate * plate0 = pMarkedPlate;
	if (plate0 == NULL) return;

	C3Point P0, P1, P2, P3;
		dlg.m_Solid = 0;
		if (plate0->Solid == FALSE) dlg.m_Solid = 1;
		dlg.m_X1 = plate0->Corn[0].X; dlg.m_Y1 = plate0->Corn[0].Y; dlg.m_Z1 = plate0->Corn[0].Z;
		dlg.m_X2 = plate0->Corn[1].X; dlg.m_Y2 = plate0->Corn[1].Y; dlg.m_Z2 = plate0->Corn[1].Z;
		dlg.m_X3 = plate0->Corn[2].X; dlg.m_Y3 = plate0->Corn[2].Y; dlg.m_Z3 = plate0->Corn[2].Z;
		dlg.m_X4 = plate0->Corn[3].X; dlg.m_Y4 = plate0->Corn[3].Y; dlg.m_Z4 = plate0->Corn[3].Z;
		dlg.m_N = plate0->Number;
		dlg.m_Comment = plate0->Comment;

		int res = dlg.DoModal();
		if (res == IDOK) {
			plate0->Solid = FALSE;
			if (dlg.m_Solid == 0) plate0->Solid = TRUE;
			plate0->Visible = plate0->Solid;
			P0.X = dlg.m_X1; P0.Y = dlg.m_Y1; P0.Z = dlg.m_Z1;
			P1.X = dlg.m_X2; P1.Y = dlg.m_Y2; P1.Z = dlg.m_Z2;
			P2.X = dlg.m_X4; P2.Y = dlg.m_Y4; P2.Z = dlg.m_Z4;
			P3.X = dlg.m_X3; P3.Y = dlg.m_Y3; P3.Z = dlg.m_Z3;
			plate0->SetLocals(P0, P1, P2, P3); //SetArbitrary(P1, P2, P3, P4);
			plate0->Number = dlg.m_N;
			plate0->Comment = dlg.m_Comment;
			OnShow();
			return;
		} // OK
		
		else if (res == IDCANCEL) {//
			if (!dlg.OptRead) return;
			int added = ReadAddPlates();//	AddPlatesNumber += added;
			AppendAddPlates();
			//ModifyArea(FALSE);
			CString S;
			//S.Format("Total addSurf %d (added %d) ", AddPlatesNumber, added);
			//AfxMessageBox(S);
			
			if (AfxMessageBox("Replace the selected Surf?", MB_ICONQUESTION | MB_YESNOCANCEL) == IDYES)
				pMainView->OnPlateDelete(); // delete pMarkedPlate

			OnShow();
			return;
		}
}

void CBTRDoc::OnPlateDeleteAllFree()
{
	CString S;
	S.Format("Delete ALL free surfaces?");
	if (AfxMessageBox(S, MB_ICONQUESTION | MB_YESNO) == IDYES) 
		InitAddPlates(); // clear the add list, set AddPlatesNumber = 0
	SetPlates();
	OnShow();
	//ShowStatus();
}

void CBTRDoc::OnPlateShowFile()
{
	FILE * fin;
	char name[1024];
	char buf [1024];
	if (pMarkedPlate == NULL) {
		AfxMessageBox("Load not specified"); return; }
	if (pMarkedPlate->filename == "") {
		AfxMessageBox("File not found"); return; }
	strcpy(name, pMarkedPlate->filename);
	ShowFileText(name);

/*	fin = fopen(name, "r");
	m_Text.Empty();// = "";
	while (!feof(fin)) {
		fgets(buf, 1024, fin);
		if (!feof(fin))	m_Text += buf;
	}
	fclose(fin);

		SetTitle(name);
		pDataView->m_rich.SetFont(&pDataView->font, TRUE);
		pDataView->m_rich.SetBackgroundColor(FALSE, RGB(250,230,180));
		pDataView->m_rich.SetWindowText(m_Text);
		pDataView->m_rich.SetModify(FALSE); */
}




void CBTRDoc::OnOptionsThreads()
{
	// here we detect number of processors present in the system (for unknown purposes)
	SYSTEM_INFO si;
	::GetSystemInfo(&si);
	int num_proc = si.dwNumberOfProcessors;
	CThreadsDlg dlg;

		dlg.Nproc = num_proc;
		dlg.Nthreads = ThreadNumber;
		dlg.m_MaxScen = MAXSCEN;
		dlg.doc = this;
		dlg.m_Scenfilename = ScenFileName;

		if (dlg.DoModal() == IDOK) {
			
			ThreadNumber = dlg.Nthreads;
			//MaxThreadNumber		21
			if (ThreadNumber > 20) {
				ThreadNumber = 20;
				AfxMessageBox("Number of threads is set = 20 (max)");
			}
			
			//dlg.m_MaxScen = MAXSCEN;
			
			if (SkipSurfClass.GetSize() > 0) SetPlates();
					
			ShowStatus();
		}
		else {
			for (int i = 0; i <= MAXSCEN; i++)	ScenData[i].RemoveAll();
			ScenLoaded = FALSE;// INIT default scenarios
			MAXSCEN = 1;
			logout << " SCEN READ Canceled -> SINGLE scenario is active!\n ";
			AfxMessageBox("SINGLE SCENARIO is active \n SCEN READ Canceled");
			return; // cancel 
		}
}

void CBTRDoc::OnPlotSumReionX()
{
	char name[1024];
	CMultiPlotDlg dlg;
	dlg.DataY = &SumReiPowerX;
	
	dlg.InitPlot(this);
	dlg.plotcolor = RGB(200,0,0);

	if (dlg.DoModal() == IDOK) { //return;
		CFileDialog fdlg(FALSE, "txt | * ", "Sum_ReiX.txt", OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"Text file (*.txt) | *.TXT | All Files (*.*) | *.*||", NULL);
		if (fdlg.DoModal() == IDOK) {
			strcpy(name, fdlg.GetPathName());
			WriteSumReiPowerX(name, (float) dlg.Xmin, (float) dlg.Xmax);//"Sum_ReiX.dat");
			//AfxMessageBox("Data is written to file \n Sum_ReiX.dat");
			delete fdlg;
		} // filedialog OK
	} // OK
}

void CBTRDoc::OnPlotSumAtomX()
{
	char name[1024];
	CMultiPlotDlg dlg;
	dlg.DataY = &SumAtomPowerX;
	
	dlg.InitPlot(this);
	dlg.plotcolor = RGB(0,0,200);

	if (dlg.DoModal() == IDOK) { //return;
		CFileDialog fdlg(FALSE, "txt | * ", "Sum_AtomX.txt", OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"Text file (*.txt) | *.TXT | All Files (*.*) | *.*||", NULL);
		if (fdlg.DoModal() == IDOK) {
			strcpy(name, fdlg.GetPathName());
			WriteSumAtomPowerX(name, (float) dlg.Xmin, (float) dlg.Xmax);//"Sum_AtomX.dat");
			//AfxMessageBox("Data is written to file \n Sum_AtomX.dat");
			delete fdlg;
		} // filedialog OK
	} // OK
}

	//WriteSumReiPowerX("Sum_ReiX.dat");
	//WriteSumAtomPowerX("Sum_AtomX.dat");

void CBTRDoc::OnPlotAngulardistr() // calls statistics
{
	//if (OptCombiPlot == -1  || pMarkedPlate == NULL) return;
/*	int n = -1; // all falls
	int lines = 10000;
	if (pMarkedPlate != NULL) n = pMarkedPlate->Number;
	CStatOutDlg dlg;
	dlg.m_Lines = lines;
	if (dlg.DoModal() == IDOK) {
		lines = dlg.m_Lines;
	}
	ShowBPdata(n, lines);*/

	OnStatisticsSet();

/*	P_CalculateAngularProf(pMarkedPlate); //set limits, step

	int kmax = pMarkedPlate->AngularProfile.GetSize();
	double step = pMarkedPlate->StepAngle;
	double Amin = pMarkedPlate->MinAngle;
	double Amax = pMarkedPlate->MaxAngle;

	CArray<double, double>  ArrX; 
	CArray<double, double>  ArrY;
	CString S = pMarkedPlate->Comment;
	if (pMarkedPlate->Load != NULL) S += " " + pMarkedPlate->Load->Particles;
		
	PLOTINFO AnglePlot;
	AnglePlot.Caption =
		"Power vs Angle (from Normal) at "+ S; //pMarkedPlate->Comment;
	
	AnglePlot.LabelX = "Angle, mrad";
	AnglePlot.LabelY = "Power Density [W/mrad]";
	AnglePlot.LimitX = 0;//90;//ReionXmax;
	AnglePlot.LimitY = 0;//1.e20;
	AnglePlot.CrossX = 0;//100;
	AnglePlot.CrossY = 0;
	AnglePlot.Line = 1;
	AnglePlot.Interp = 1;
	AnglePlot.PosNegY = FALSE;
	AnglePlot.PosNegX = FALSE;

	double x, y;
	// double amin = 0;
	// double amax = 90;
	char name[1024];
	int k;
	 
	for (k = 0; k < kmax; k++) {
		x = Amin + step * (k+0.5);
		ArrX.Add(x);
		y = pMarkedPlate->AngularProfile[k];
		ArrY.Add(y);
		
	}

	AnglePlot.N = ArrX.GetSize();
	
	AnglePlot.DataX = &ArrX;
	AnglePlot.DataY = &ArrY;
	
	CPlotDlg dlg;
	dlg.Plot = &AnglePlot;

	dlg.InitPlot(Amin, Amax, step); 

	if (dlg.DoModal() == IDOK) {
		CFileDialog fdlg(FALSE, "txt | * ", "Angular_data.txt", OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"Text file (*.txt) | *.TXT | All Files (*.*) | *.*||", NULL);
		if (fdlg.DoModal() == IDOK) {
			FILE * fout;
			strcpy(name, fdlg.GetPathName());
			fout = fopen(name, "w");
			
			fprintf(fout, "Angular Distribution of Power \n at " + S);
			
			fprintf(fout, "\n Aver Angle, mrad    Power, W/mrad \n");
			for (k = 0; k < kmax-1; k++) {
				x = ArrX[k];
				y = ArrY[k];
				fprintf(fout, "\t  %g \t\t %g \n", x, y); 
			}
			fclose(fout);
			delete fdlg;
		} // filedialog OK
	} // Plot.DoModal

	ArrX.RemoveAll();
	ArrY.RemoveAll();
	*/
}

void CBTRDoc::OnUpdatePlotAngulardistr(CCmdUI *pCmdUI)
{
	//pCmdUI->Enable(OptCombiPlot != -1 && pMarkedPlate != NULL);
}

void CBTRDoc::OnNeutralizationCurrents()
{
	OnPlotNeutrefficiency();
	//PlotCurrentRates(0); 
}

void CBTRDoc::OnNeutralizationRates()
{
	PlotCurrentRates(1); 
}


void CBTRDoc::OnLogView()
{
	ShowLogFile(0);
}


/*void CBTRDoc::OnLogSave()
{
	ShowLogFile(10);
	AfxMessageBox("Log settings will be added here");
}*/

void CBTRDoc::OnLogSave()
{
	OptLogSave = !OptLogSave;
	logout.SetLogSave(OptLogSave);
	if (OptLogSave) AfxMessageBox("Log-file is switched ON");
	else AfxMessageBox("Log-file is switched OFF");
}

void CBTRDoc::OnUpdateLogSave(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(OptLogSave);
	if (OptLogSave) pCmdUI->SetText("ON"); // currently ON
	else pCmdUI->SetText("OFF");// currently ON
}

void CBTRDoc::OnStatisticsView() //= Show ALL (all stat options -> ON)
{
	int n = -1; // all surf if not Selected
	int lines = 100000;
	
	//if (pMarkedPlate != NULL && pMarkedPlate->Selected && OptCombiPlot > -1) 
		//n = pMarkedPlate->Number;

	if (pMarkedPlate != NULL) 
		n = pMarkedPlate->Number;
	else n = -1; // show all falls
	ShowBPdata(n, lines); // all data on selected surf (or ALL)*/

	//ShowReport();
}


void CBTRDoc::OnStatisticsSet()
{
	//AfxMessageBox("Not active now");
	//return;

	int n = -1; // all falls
	int lines = 100; // to show
	int power, posloc, posglob, angle, pos; // options for output
	if (pMarkedPlate != NULL) // && pMarkedPlate->Selected && OptCombiPlot > -1)
		n = pMarkedPlate->Number;
	
	CStatOutDlg dlg;
	dlg.m_Lines = lines;
	dlg.m_Atoms = OptAtomPower;
	dlg.m_PosIons = OptPosIonPower;
	dlg.m_NegIons = OptNegIonPower;
	dlg.m_Power = 1; //  1 - [W]
	dlg.m_PosLoc = 1; // 1 - local
	dlg.m_PosGlob = 1; // 1 - global
	dlg.m_Afall = 2; // 2 - both angles
	if (dlg.DoModal() == IDOK) {
		lines = dlg.m_Lines;
		power = dlg.m_Power;
		posloc = dlg.m_PosLoc;
		posglob = dlg.m_PosGlob;
		angle = dlg.m_Afall;
		OptAtomPower = dlg.m_Atoms;
		OptPosIonPower = dlg.m_PosIons;
		OptNegIonPower = dlg.m_NegIons;
		pos = 0;
		if (posloc > 0 && posglob == 0) pos = 1;
		if (posloc > 0 && posglob > 0) pos = 2;
		if (posloc == 0 && posglob > 0) pos = 3;
		ShowBPdata(n, lines, pos, angle, power); // show selected options
		//ShowBPdata(n, lines); // show all
	}
	
}


void CBTRDoc::OnSurfacesMesh() // set global opt from main menu 
{
	SetDefaultMesh();
}


void CBTRDoc::OnUpdatePlateFile(CCmdUI *pCmdUI)
{
	//BOOL found = TRUE;
	//if (pMarkedPlate == NULL) found = FALSE; 
	//else if (pMarkedPlate->filename == "") found = FALSE;
	pCmdUI->Enable(pMarkedPlate != NULL && pMarkedPlate->filename != "");
}


void CBTRDoc::OnPlateLoadOptRecalc() // set plate opt in pop-up menu
{
	AfxMessageBox("Not active since 5.1");// static loads
	return;

	//// old  - worked for Falls (dynamic loads)
	int n = -1;
	if (pMarkedPlate != NULL && pMarkedPlate->Selected && OptCombiPlot > -1)
		n = pMarkedPlate->Number;
	if (n < 0) {
		SetDefaultMesh();
		AfxMessageBox("No plate selected\n Settings will be applied next to all recalculated loads");
		return;
	}
	CPlate * plate = pMarkedPlate;
	if (plate->Touched == FALSE) { 		// non-zero loads 
		AfxMessageBox("Not loaded");
		return;
	}

	if (plate->Loaded == FALSE) { // touched but not calculated yet
		SetNullLoad(plate); // create zero load with default mesh options
		//P_CalculateLoad(plate);
		//SetLoadArray(plate, TRUE);
	}

	DefLoadDlg dlg; // set new opt
	dlg.m_OptN = 1; // fixed step
	dlg.NX = DefLoadNX;
	dlg.NY = DefLoadNY;
	dlg.Round = DefLoadStepRound; // round step if optN == TRUE (size)
	dlg.m_StepX = plate->Load->StepX;// DefLoadStepX;
	dlg.m_StepY = plate->Load->StepY;// DefLoadStepY;

		dlg.m_OptAtom = OptAtomPower;
		dlg.m_OptPos = OptPosIonPower;
		dlg.m_OptNeg = OptNegIonPower;

		if (dlg.DoModal() == IDOK) {
			DefLoadOptN = (dlg.m_OptN == 0);
			DefLoadNX = dlg.NX;
			DefLoadNY = dlg.NY;
			DefLoadStepRound = dlg.Round;//if OptN == TRUE -> round step
			DefLoadStepX = dlg.m_StepX;
			DefLoadStepY = dlg.m_StepY;

			OptAtomPower = dlg.m_OptAtom;
			OptPosIonPower = dlg.m_OptPos;
			OptNegIonPower = dlg.m_OptNeg;
		}

		SetNullLoad(plate); // create zero load with default mesh options
		P_CalculateLoad(plate);
		//SetLoadArray(plate, TRUE);
		ShowProfiles = TRUE;
		OnPlotMaxprofiles();
		plate->ShowLoadState(); // show summary (info)	
	
}


void CBTRDoc::OnEditCrossSect()
{
	if (!OptBeamInPlasma) return;
	
	char s[1024];
	CGasDlg dlg;
	dlg.m_Head1 = "Energy, keV/amu";
	dlg.m_Head2 = "StopCS, cm2"; // CS(10keV) = 10, CS(1000) = 0.1-0.2 (Suzuki)
//	dlg.m_Filename = CSFileName;
	dlg.m_X.Empty();
	dlg.m_Y.Empty();
	dlg.Arr = &CSarray;//
	dlg.m_Caption = "Stopping Cross-section data";

	double x, p, xmax = 0, pmax = 0, xmin = 1000;
	int i;
	for (i = 0; i <= CSarray.GetUpperBound(); i++) {
		x = CSarray[i].X; //E [keV/nu] 
		p = CSarray[i].Y;//SigmaStop [cm2]
		sprintf(s, "%f\r\n", x); dlg.m_X += s;
		sprintf(s, "%le\r\n", p); dlg.m_Y += s;
	}

	if (dlg.DoModal() == IDCANCEL) return;

	for (i = 0; i <= dlg.Arr->GetUpperBound(); i++) {
		x = dlg.Arr->GetAt(i).X;
		p = dlg.Arr->GetAt(i).Y;
		xmax = Max(xmax, x);
		pmax = Max(pmax, p);
		xmin = Min(xmin, x);
	}
/*
	GField->Pmax = pmax;
	GField->Xmax = xmax;
	GField->Xmin = xmin;
	GField->Nx = dlg.Arr->GetSize();

	GasFileName = dlg.m_Filename;
	PressureLoaded = TRUE;
	PressureDim = 1;
	GasCoeff = 1;
	OptReionAccount = TRUE;//automatic when gas loaded
	OptThickNeutralization = TRUE;
	SetReionPercent();
	SetNeutrCurrents(); */
	//OnShow();
}


void CBTRDoc::OnPlateClearall()
{
	ClearAllPlates();
	ShowStatus();
}


void CBTRDoc::OnPlateScale()// moved from MainView method
{
	CPlate * plate = pMarkedPlate;
	if (plate == NULL) { ShowStatus(); return; }
	if (plate->Loaded == FALSE) return;

	C3Point p0, p1, p2, p3;
	double stepX, stepY;// , left, right, bot, top;
	double xmin, xmax, ymin, ymax, zmin, zmax;
	CString S;
		
	//plate->SetViewDim(); // - set max limits 
	//plate->ShowLoad(); //OnPlotLoadmap();

/*	plate->ShowEmptyLoad(); // show total rect, pLV->SetPlate(this);
	plate->DrawLoadRect(); //can be zero - calculate and Show Scale (local) for current limits!!
	plate->DrawPlateBound(); // plate polygon - scale not recalculated
	ShowPlatePoints(OptDrawPart); // particle spots - scale not recalculated*/

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
			
		stepX = plate->Load->StepX; //dlg.m_StepX; // to add later
		stepY = plate->Load->StepY; //dlg.m_StepY; // to add later
		xmin =  dlg.m_Xmin; 
		xmax =  dlg.m_Xmax;
		ymin =  dlg.m_Ymin;
		ymax =  dlg.m_Ymax;
		zmin =  dlg.m_Zmin;
		zmax =  dlg.m_Zmax;
		
		S.Format(" steps: %g / %g\n  Dims: X: %g - %g  Y: %g - %g ", stepX, stepY, xmin, xmax, ymin, ymax);
		AfxMessageBox(S);
				
		//plate->ApplyLoad(TRUE, stepX, stepY); // individual Loaded->TRUE
				//SetNullLoad(plate); // create zero load with default mesh options
		//pDoc->P_CalculateLoad(plate);
		
	
		if (xmax - xmin > stepX && ymax - ymin > stepY)
			plate->SetViewDim(xmin, xmax, ymin, ymax); // - set new limits 

		//plate->ShowLoad(); //OnPlotLoadmap();
		
		/*plate->ShowEmptyLoad(); // show total rect, pLV->SetPlate(this);
		plate->DrawLoadRect(); //can be zero - calculate and Show Scale (local) for current limits!!
		plate->DrawPlateBound(); // plate polygon - scale not recalculated
		//pDoc->ShowPlatePoints(); // particle spots - scale not recalculated*/

		if (plate->Number == 2000) {
			//pDoc->CalculateTracks(); // beam planes in plasma
		}
		
	} //dlg IDOK
	//else return;// (dlg.DoModal() == IDCANCEL) 

	ShowProfiles = TRUE;
	OnPlotMaxprofiles();	
	plate->ShowLoadState(); // show summary (info)
	//OnPlotLoadmap();

	/////plate->ShowLoad(); //OnPlotLoadmap();
}

void CBTRDoc::OnResultsReadScen()
{
	char DirName[1024];
	CString OldDirName = CurrentDirName; // old path
	logout << "Current Config path " << OldDirName << "\n"; 
		
	int nread = ReadData(); // Read text-> to m_Text 
	// Set new ConfigFileName, CurrDirName!!! 
	CString NewDirName = CurrentDirName; // new - SCEN folder
	//::GetCurrentDirectory(1024, NewDirName);// SCEN Config
	logout << "Current Config path " << NewDirName << "\n"; 
	if (nread == 0) return; // failed
	
	// RETURN BACK HOME  
	CurrentDirName = OldDirName;
	::SetCurrentDirectory(CurrentDirName);// go up("..\\"); - above scen
	logout << "Current folder " << CurrentDirName << "\n"; 
	
	InitOptions(); //MAXSCEN = 1  empty Fields, AddSurfName, Set default tracing options
	//InitTrackArrays();// clear 
	UpdateDataArray(); // m_Text -> SetData SetOption CheckData 
	SetFields(); // GAS + MF from files if found in Config
	CheckData();
	SetBeam(); // Aimings, IS positions
	InitPlasma(); // called in OnDataGet
	InitAddPlates(); // Remove AddPlatesList, set AddPlatesNumber = 0;
	SetPlates();//SetPlasma is called from SetPlatesNBI
	SetAddPlates();//ReadAddPlates(AddSurfName);
	if (AddPlatesNumber > 0)	AppendAddPlates();
	SetStatus();

	CurrentDirName = NewDirName;
	::SetCurrentDirectory(CurrentDirName);// SCEN folder
	logout << "Current folder " << CurrentDirName << "\n"; 
	 
	// SCEN folder is set as CurrDir!
	//ScenData[0].SetAt(1, "CONFIG = " + CurrentDirName + "\\ " + ConfigFileName + "\n");
	
	/*CString sn;
	char name[1024];
		int pos1 = -1, pos2 = -1;
	CString NAME  = ConfigFileName.MakeUpper();
	pos1 = NAME.Find("SCEN", 0);
	pos2 = NAME.Find("_", 0);
	if (pos1 >=0 && pos2 - pos1 > 4) {
		sn = NAME.Mid(pos1+4, pos2-pos1-4);  
		SCEN = atoi(sn);
		AfxMessageBox("SCEN NUMBER = " + sn); 
	}
	else {
		AfxMessageBox("SCEN NUMBER = 0"); 
		SCEN = 0;
		return;
	}*/

	ReadScenLoads3col(); ////////// Loads reading from SCEN folder////////////////
	OptCombiPlot = -1;//-1 - no load, 0 - map is calculated, >0 - selected but not calculated 
	//ModifyArea(FALSE);
	//OnPlateClearSelect();
	Progress = 0;
	OptParallel = FALSE; // CalculateCombi(0)
	OnShow();
}
