// DataView.cpp : implementation file
//

#include "stdafx.h"
#include "BTR.h"

#include "BTRDoc.h"
#include "DataView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CDataView * pDataView;

/////////////////////////////////////////////////////////////////////////////
// CDataView

IMPLEMENT_DYNCREATE(CDataView, CView)

CDataView::CDataView()
{
	
}

CDataView::~CDataView()
{
}


BEGIN_MESSAGE_MAP(CDataView, CView)
	//{{AFX_MSG_MAP(CDataView)
	ON_WM_CREATE()
	ON_WM_SIZE()
//	ON_COMMAND(ID_DATA_GET, OnDataGet)
//	ON_COMMAND(ID_DATA_STORE, OnDataStore)
//	ON_UPDATE_COMMAND_UI(ID_DATA_STORE, OnUpdateDataStore)
	ON_COMMAND(ID_DATA_ACTIVE, OnDataActive)
	//}}AFX_MSG_MAP
	ON_WM_RBUTTONDOWN()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDataView drawing

void CDataView::OnDraw(CDC* /* pDC */)
{
	//AfxMessageBox("CDataView OnDraw");
	pDataView = this;
	CBTRDoc* pDoc = (CBTRDoc*)GetDocument();
//	OnDataGet();
	m_rich.SetFont(&font, TRUE);
//	m_rich.SetBackgroundColor(FALSE, RGB(210,250,200));
	m_rich.SetWindowText(pDoc->m_Text);
	m_rich.SetModify(FALSE);
}

/////////////////////////////////////////////////////////////////////////////
// CDataView diagnostics

#ifdef _DEBUG
void CDataView::AssertValid() const
{
	CView::AssertValid();
}

void CDataView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

/*CBTRDoc* CDataView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CBTRDoc)));
	return (CBTRDoc*)m_pDocument;
}*/
#endif //_DEBUG

CBTRDoc* CDataView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CBTRDoc)));
	return (CBTRDoc*)m_pDocument;
}

/////////////////////////////////////////////////////////////////////////////
// CDataView message handlers

/*void CDataView::OnDataGet() // load data from config file
{
	CBTRDoc* pDoc = GetDocument();
	if (pDoc->ReadData() ==0) return; // file -> m_Text 
	CWaitCursor wait;
	CString S = pDoc->m_Text;
	pDoc->UpdateDataArray(S); // SetData
	m_rich.SetWindowText(pDoc->m_Text);
	m_rich.SetModify(FALSE);
}*/
/*
void CDataView::OnDataStore() 
{
	CBTRDoc* pDoc = GetDocument();
	CString S;
	m_rich.GetWindowText(S); //pDoc->m_Text);
	pDoc->m_Text.Empty();
	pDoc->m_Text = S; // S -> m_Text;
	pDoc->UpdateDataArray(pDoc->m_Text);
//	pDoc->SaveData();
	m_rich.SetWindowText(pDoc->m_Text);
	m_rich.SetModify(FALSE);
}*/

void CDataView::OnDataActive() 
{
		CBTRDoc* pDoc = GetDocument();
		if (pDoc == NULL) return;
		pDoc->FormDataText(FALSE); //without internal names
		m_rich.SetFont(&font, TRUE);
		m_rich.SetBackgroundColor(FALSE, RGB(210,250,200));
		m_rich.SetWindowText(pDoc->m_Text);
		m_rich.SetModify(FALSE);
		InvalidateRect(NULL, TRUE);
		
	
}

/*void CDataView::OnUpdateDataStore(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(TRUE);//(m_rich.GetModify());
}*/

int CDataView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	CRect rect(0,0,0,0);
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;
	// TODO: Add your specialized creation code here
	m_rich.Create(ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_MULTILINE | // ES_WANTRETURN |
							WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL ,  rect, this, 1);
	
/*	BOOL CreateFont( int nHeight, int nWidth, int nEscapement, int nOrientation, 
					int nWeight, BYTE bItalic, BYTE bUnderline, BYTE cStrikeOut, 
					BYTE nCharSet, BYTE nOutPrecision, BYTE nClipPrecision, BYTE nQuality, 
					BYTE nPitchAndFamily, LPCTSTR lpszFacename );
*/
	//CFont font;
	font.CreateFont(-18, 0, 0, 0, 800, FALSE, FALSE, 0,
		0, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
		FIXED_PITCH | FF_MODERN, "Courier");//);"Terminal"

/*	font.CreateFont(-12, 0, 0, 0, 400, FALSE, FALSE, 0,
		0, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
		FF_MODERN, "Arial");//"Courier New");
/*	font.CreateFont(-12, 0, 0, 0, 400, FALSE, FALSE, 0,
		ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
		DEFAULT_PITCH | FF_ROMAN, "Times New Roman Cyr");*/

	//CFont* pOldFont = (CFont*)(pDC->SelectObject(&font));
	
	m_rich.SetFont(&font, TRUE);
	m_rich.SetBackgroundColor(FALSE, RGB(210,250,200));
	
//	CDocument* pDoc = GetDocument();
//	pDoc->AddView((CView*)this);
	return 0;
}

void CDataView::OnSize(UINT nType, int cx, int cy) 
{
	CRect rect;
	CView::OnSize(nType, cx, cy);
	// TODO: Add your message handler code here
	GetClientRect(rect);
	m_rich.SetWindowPos(&wndTop, rect.top+5, rect.left+5, 
							rect.right - rect.left-10,  rect.bottom - rect.top - 5,  SWP_SHOWWINDOW);
}




void CDataView::OnRButtonDown(UINT nFlags, CPoint point)
{
	//CBTRDoc * pDoc = GetDocument();
	//pDoc->OnShow();
	//pDoc->ShowLogFile(0);
	
	CView::OnRButtonDown(nFlags, point);
}
