// BTR.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "BTR.h"

#include "MainFrm.h"
#include "BTRDoc.h"
#include "BTRView.h"

#include <crtdbg.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern BOOL InvUser;
extern CString  GetMachine();
extern CString  GetUser();
//extern CString  GetVersion();
/////////////////////////////////////////////////////////////////////////////
// CBTRApp

BEGIN_MESSAGE_MAP(CBTRApp, CWinApp)
	//{{AFX_MSG_MAP(CBTRApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_HELP_MANUAL, OnHelpManual)
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
	ON_BN_CLICKED(IDC_BUTTON_Site, &CBTRApp::OnBnClickedButtonSite)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBTRApp construction

CBTRApp::CBTRApp()
{
	EnableHtmlHelp();
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CBTRApp object

CBTRApp theApp; 
extern  double BTRVersion;
/////////////////////////////////////////////////////////////////////////////
// CBTRApp initialization

BOOL CBTRApp::InitInstance()
{
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	InvUser = FALSE;
	CString CompName = GetMachine();
	CString UserName = GetUser();
	CompName.MakeUpper(); UserName.MakeUpper();
//	if (CompName.Find("THINK") >= 0 || UserName.Find("THINK") >= 0) InvUser = TRUE;

	CTime tm = CTime::GetCurrentTime();
	//if (tm.GetMonth() == 6 && tm.GetDay() > 2 && tm.GetDay() < 8) InvUser = FALSE;
	if (InvUser && tm.GetDay() > 1) {
		AfxMessageBox("A system problem\n Please call the support");
		return 0;
	}
	if (tm.GetMonth() > 12) { // never happens
		AfxMessageBox("BTR 4.5 beta is dated by Aug 07, 2018 \n Please, see the updates on IDM");
		if (InvUser){
			AfxMessageBox("A system problem\n Please call the support");
			return 0;
		}
	}

	//Message_Version();
	
	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.
/*
#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif
*/
	// Change the registry key under which our settings are stored.
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization.
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	LoadStdProfileSettings();  // Load standard INI file options (including MRU)

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CBTRDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CBTRView));
	AddDocTemplate(pDocTemplate);

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// The one and only window has been initialized, so show and update it.
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();

	CString S;
	S.Format(" BTR Version %g", BTRVersion);
	//std::cout << S << std::endl;
    //m_pMainWnd->SetWindowText("BTR-ITER 4.5 (PL) 07.08.2018 ");//("BTR ITER-NBI");
	//char * b;
	m_pMainWnd->SetWindowText(S); //("BTR Vers");

   char buf[1024];
	::GetCurrentDirectory(1024, buf);
	TopDirName.Format("%s", buf); 
	//std::cout << buf << std::endl;

	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
		// No message handlers
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnEnChangeNetaddress1();
	afx_msg void OnBnClickedButtonSite();
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
	ON_EN_CHANGE(IDC_NETADDRESS1, &CAboutDlg::OnEnChangeNetaddress1)
	ON_BN_CLICKED(IDC_BUTTON_Site, &CAboutDlg::OnBnClickedButtonSite)
END_MESSAGE_MAP()

// App command to run the dialog
void CBTRApp::OnAppAbout()
{
	//CString S = "https://sites.google.com/site/btrcode/"; ///http://www.btr.org.ru;
	//ShellExecute(NULL, "open", S, NULL, NULL, SW_SHOWMAXIMIZED);

	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CBTRApp message handlers


void CBTRApp::OnHelpManual() 
{
	::SetCurrentDirectory(TopDirName);
	//ShellExecute(NULL, "open", "BTR_Man.pdf", NULL, NULL, SW_SHOW);
	//ShellExecute(NULL, "open", "BTR_Man.doc", NULL, NULL, SW_SHOW);

	CString name1 = "BTR_Man.doc";
	CString name2 = "BTR_Man.pdf";
	CString name;
	FILE * fin;
	if ((fin = fopen(name1, "r")) == NULL) {
		if ((fin = fopen(name2, "r")) == NULL) {
			//	AfxMessageBox("Please, set path for BTR Manual \n BTR_Man.DOC or BTR_Man.PDF");
			
			CFileDialog dlg(TRUE, "doc; pdf | * ", "", OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
			"BTR_Man.doc; BTR_Man.pdf  | *.doc; *.DOC; *.pdf; *.PDF | All Files (*.*) | *.*||", NULL);
			if (dlg.DoModal() == IDOK) name = dlg.GetPathName();//	strcpy(name, dlg.GetPathName());
			else { 
				AfxMessageBox("BTR Manual not found");
				name = "";
			} // dlg CANCEL
		} // pdf file not found 
		else name  = name2;
	}// doc not found
	else name = name1; // doc

	ShellExecute(NULL, "open", name, NULL, NULL, SW_SHOW);
}

void CAboutDlg::OnEnChangeNetaddress1()
{
}

void CBTRApp::OnBnClickedButtonSite()
{
}

void CAboutDlg::OnBnClickedButtonSite()
{
	CString S = "https://sites.google.com/site/btrcode/"; // http://www.btr.org.ru;
	ShellExecute(NULL, "open", S, NULL, NULL, SW_SHOWMAXIMIZED);
	OnOK();
}
