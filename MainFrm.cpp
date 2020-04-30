// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "BTR.h"

#include "MainFrm.h"
#include "MainView.h"
#include "BTRDoc.h"
#include "LoadView.h"
#include "DataView.h"
#include "SetView.h" 

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

HWND ProjectBar;
/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_COMMAND(ID_FILE_SEND_MAIL, OnFileSendMail)
	ON_UPDATE_COMMAND_UI(ID_FILE_SEND_MAIL, OnUpdateFileSendMail)
	//}}AFX_MSG_MAP
	// Global help commands
	ON_COMMAND(ID_HELP_FINDER, &CMDIFrameWnd::OnHelpFinder)
	ON_COMMAND(ID_HELP, &CFrameWnd::OnHelp)
	ON_COMMAND(ID_CONTEXT_HELP, &CMDIFrameWnd::OnContextHelp)
	ON_COMMAND(ID_DEFAULT_HELP, &CMDIFrameWnd::OnHelpFinder)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
	
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
/*	
	if (!m_wndProjectBar.Create(this, IDD_PROJECTBAR, 
		CBRS_TOP, // | CBRS_SIZE_DYNAMIC, 
		IDD_PROJECTBAR)) {
		TRACE("Failed to create Dialog bar \n");
		return -1;
	}	
 	ProjectBar= m_wndProjectBar.m_hWnd;
*/
  if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
	
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	// TODO: Delete these three lines if you don't want the toolbar to
	//  be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);
	m_bAutoMenuEnable = TRUE;


	return 0;
}

BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT /*lpcs*/,
	CCreateContext* pContext)
{

		//	return m_wndSplitter.Create(this,
//		2, 2,               // TODO: adjust the number of rows, columns
//		CSize(10, 10),      // TODO: adjust the minimum pane size
//		pContext);
	VERIFY(m_wndSplitter.CreateStatic(this, 2,2));
	VERIFY(m_wndSplitter.CreateView(0, 0, RUNTIME_CLASS(CMainView),
		CSize(1000, 500), pContext)); //680,390
	VERIFY(m_wndSplitter.CreateView(0, 1, RUNTIME_CLASS(CLoadView),
		CSize(1000, 500), pContext)); //680,390
	VERIFY(m_wndSplitter.CreateView(1, 0, RUNTIME_CLASS(CDataView),
		CSize(1000, 500), pContext)); //680,390
	VERIFY(m_wndSplitter.CreateView(1, 1, RUNTIME_CLASS(CSetView),
		CSize(1000, 500), pContext)); //680,390
 return TRUE;
	
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	
	int cx = GetSystemMetrics(SM_CXSCREEN);
	int cy = GetSystemMetrics(SM_CYSCREEN);
	cs.x = 1; cs.y = 1; cs.cx = cx - 1; cs.cy = cy - 30; 
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers



void CMainFrame::OnFileSendMail() 
{
	// TODO: Add your command handler code here

}

void CMainFrame::OnUpdateFileSendMail(CCmdUI*) 
{
	// TODO: Add your command update UI handler code here
	
}
