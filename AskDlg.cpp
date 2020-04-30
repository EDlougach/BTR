// AskDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BTR.h"
#include "AskDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAskDlg dialog


CAskDlg::CAskDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAskDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAskDlg)
	m_Answer = _T("");
	//}}AFX_DATA_INIT
}


void CAskDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAskDlg)
	DDX_Control(pDX, IDC_LIST_Quest, m_ListQuest);
	DDX_Text(pDX, IDC_EDIT_ANSWER, m_Answer);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAskDlg, CDialog)
	//{{AFX_MSG_MAP(CAskDlg)
	ON_WM_CREATE()
	ON_LBN_SELCHANGE(IDC_LIST_Quest, OnSelchangeLISTQuest)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAskDlg message handlers

int CAskDlg::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	
	// TODO: Add your specialized creation code here
	
	return 0;
}

BOOL CAskDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	FillQuestList();
	FillAnsList();
		
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAskDlg::FillQuestList()
{
	m_ListQuest.ResetContent();

	m_ListQuest.AddString("Invoke Pop-up Menu");
	m_ListQuest.AddString("Set Title / Comments");
	m_ListQuest.AddString("Modify NBI Configuration / Options / Settings");
	m_ListQuest.AddString("Save NBI config / Options / Settings");
	m_ListQuest.AddString("Read NBI config / Options / Settings");

	m_ListQuest.AddString("Import geometry from PDP Input");
	m_ListQuest.AddString("Export geometry to PDP");
	m_ListQuest.AddString("Import Beam from external file");
	m_ListQuest.AddString("Export Beam to PDP");

	m_ListQuest.AddString("Add new Surfaces to NBI");
	m_ListQuest.AddString("Set Source Beam Geometry");
	m_ListQuest.AddString("Set Beamlets Optics");
	m_ListQuest.AddString("Set Active Beam Segments");
	m_ListQuest.AddString("Set Active Surfaces");
	m_ListQuest.AddString("Set grid-steps for Active Surface");

	m_ListQuest.AddString("Set Gas Density");
	m_ListQuest.AddString("Set Magnetic Field");
	
	m_ListQuest.AddString("Start/Stop Beam Tracing");
	m_ListQuest.AddString("Write Output files");
	m_ListQuest.AddString("Read Results");
	m_ListQuest.AddString("View Results");
	m_ListQuest.AddString("Zoom View");

	m_Answer = "\r\n Click on the Topic \r\n\r\n - see the Answer here ";
	
	UpdateData(FALSE);

}

void CAskDlg::FillAnsList()
{
	AnsArr.RemoveAll();

	AnsArr.Add("To Invoke the Pop-up Menu:"
		"\r\n\r\n - Right Mouse Button click on the Configuration View");
	AnsArr.Add("To Set Title / Comments for the Task:"
		"\r\n\r\n - Main Menu -> EDIT -> Title/Comments"
		"\r\n\r\n NOTE: Please, don't use equals sign <=> and return sign <CR> in the text");
	AnsArr.Add("To Modify the data in the LIST:"
		"\r\n\r\n - Use Data-Editor to set new parameters values"
		"\r\n - (IMPORTANT!) Update the LIST by choosing the command: Main Menu -> Data -> Update ");
	AnsArr.Add("To Save the data LIST (NBI config / Options / Settings):"
		"\r\n\r\n - goto Main Menu -> Data -> Save"
		"\r\n - set the name for I/O text file"
		"\r\n\r\n NOTE: This file will be written in standard BTR I/O format");
	AnsArr.Add("To Read the data LIST (NBI config / Options / Settings):"
		"\r\n\r\n - goto Main Menu -> Data -> Read"
		"\r\n - open the text file"
		"\r\n\r\n NOTE: This file must be in standard BTR I/O format");

	AnsArr.Add("To Import geometry from PDP Input file:"
		"\r\n\r\n - goto Main Menu -> Data -> Import -> NBL geometry (PDP)"
		"\r\n - open PDP-input text file"
		"\r\n\r\n NOTE: This file must be in PDP Input format");
	AnsArr.Add("To Export geometry to PDP:"
		"\r\n\r\n - goto Main Menu -> Data -> Export -> NBL geometry"
		"\r\n - set the name for the file"
		"\r\n\r\n NOTE: This file will be written in PDP Input format");
	AnsArr.Add("To Import Beam from external file:"
		"\r\n\r\n - goto Main Menu -> Data -> Import -> Beam"
		"\r\n - open the beamlets file");
	AnsArr.Add("To Export Beam to PDP:"
		"\r\n\r\n - goto Main Menu -> Data -> Export -> Regular Beam"
		"\r\n - set the name for the beamlets file"
		"\r\n\r\n NOTE: This file will be compatible with PDP");

	AnsArr.Add("To Add new Surfaces to NBI configuration:"
		"\r\n\r\n - goto Main Menu -> Options -> Surfaces -> Add new"
		"\r\n - Set the Surface corner-points"
		"\r\n - or choose READ to open the input file with additional surfaces");
	AnsArr.Add("To set Source Beam Geometry:"
		"\r\n\r\n - goto Main Menu -> Options -> Beam"
		"\r\n - set SINGAP or MAMuG"
		"\r\n - for SINGAP option choose the input File"
		"\r\n - for MAMuG option set the regular array via Data-Editor or choose File");
	AnsArr.Add("To set Beamlets Optics:"
		"\r\n\r\n - goto Main Menu -> Options -> Beam "
		"\r\n - choose DETAILS button in the Beamlets Model section"
		"\r\n - toggle between Common/Individual beamlet optics");   
	AnsArr.Add("To set Active Beam Segments / Rows (in MAMuG!):"
		"\r\n\r\n - Left Mouse Button Click on a Beam Group 'Emitter' \r\n (vertically elongated rectangle at X = 0) \r\n - in the Plan View (to set Channels) \r\n - or in the Side View (to set Rows) ");
	AnsArr.Add("To set Active Surfaces:"
		"\r\n\r\n - Mark a Surface: Left Mouse Button click on its 'hot point'"
		"\r\n - Pop-up Menu -> Select / Unselect");
	AnsArr.Add("To Set grid-steps for Active Surface:"
		"\r\n\r\n - Mark a Surface"
		"\r\n - Pop-up Menu -> PD steps"
		"\r\n - Set new steps in the dialog");

	AnsArr.Add("To Set Gas Density:"
		"\r\n\r\n - goto Main Menu -> Edit -> Gas");
	AnsArr.Add("To Set Magnetic Field:"
		"\r\n\r\n - goto Main Menu -> Edit -> Mag Field");

	AnsArr.Add("To Start/Stop Beam Tracing:"
		"\r\n\r\n - choose Toolbar -> GO / STOP button"
		"\r\n - or goto Pop-up Menu -> GO / STOP command");
	AnsArr.Add("To Write Results in Output files:"
		"\r\n\r\n - goto Main Menu -> Results -> Save All");
	AnsArr.Add("To Read Results from disk:"
		"\r\n\r\n - goto Main Menu -> Results -> Read All"
		"\r\n - Open the folder with BTR results"
		"\r\n - Open file 'CONFIG.DAT'in this folder");
	AnsArr.Add("To View Results:"
		"\r\n\r\n - Mark a Surface: Left Mouse Button click on its 'hot point'"
		"\r\n - goto Main Menu -> Plot"
		"\r\n\t -> Multi-colour Map OR"
		"\r\n\t -> Contour Map OR"
		"\r\n\t ->3D-VIEW OR"
		"\r\n\t -> Max Profiles");
	AnsArr.Add("To Zoom / Unzoom the Configuration View:"
		"\r\n\r\n - Left Mouse Button Click on the Configuration View"
		"\r\n - choose Pop-up -> ZOOM IN/OUT command");

}

void CAskDlg::OnSelchangeLISTQuest() 
{
	UpdateData(TRUE);
	SetAnswer();

}

void CAskDlg::SetAnswer()
{
	int i = m_ListQuest.GetCurSel(); //.GetCount();
	if (i <= AnsArr.GetUpperBound()) 
	m_Answer = AnsArr[i] + "\r\n\r\n\t Please, refer to the User's Manual for details";
	else m_Answer = "\r\n\t Please, refer to the User's Manual for details";
	UpdateData(FALSE);

}



BOOL CAskDlg::Create()
{
	return CDialog::Create(CAskDlg::IDD);

}

void CAskDlg::OnCancel() 
{
	//CDialog::OnCancel();
	DestroyWindow();
	//ShowWindow(FALSE);
}

void CAskDlg::OnOK() 
{
	DestroyWindow();
//	CDialog::OnOK();
}
