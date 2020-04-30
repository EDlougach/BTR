// BTRView.cpp : implementation of the CBTRView class
//

#include "stdafx.h"
#include "BTR.h"

#include "BTRDoc.h"
#include "BTRView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBTRView

IMPLEMENT_DYNCREATE(CBTRView, CScrollView)

BEGIN_MESSAGE_MAP(CBTRView, CView)
	//{{AFX_MSG_MAP(CBTRView)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBTRView construction/destruction

CBTRView::CBTRView()
{
	// TODO: add construction code here

}

CBTRView::~CBTRView()
{
}

BOOL CBTRView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CBTRView drawing

void CBTRView::OnDraw(CDC* /* pDC */)
{
	CBTRDoc* pDoc = GetDocument();  
	ASSERT_VALID(pDoc);
	// TODO: add draw code for native data here
}

void CBTRView::OnInitialUpdate()
{
	CView::OnInitialUpdate();

//	CSize sizeTotal;
//	sizeTotal.cx = sizeTotal.cy = 100;
//	SetScrollSizes(MM_TEXT, sizeTotal);
}

/////////////////////////////////////////////////////////////////////////////
// CBTRView printing

BOOL CBTRView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CBTRView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CBTRView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

/////////////////////////////////////////////////////////////////////////////
// CBTRView diagnostics

#ifdef _DEBUG
void CBTRView::AssertValid() const
{
	CView::AssertValid();
}

void CBTRView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CBTRDoc* CBTRView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CBTRDoc)));
	return (CBTRDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CBTRView message handlers
