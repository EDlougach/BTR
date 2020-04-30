// TrView.cpp : implementation file
//

#include "stdafx.h"
#include "BTR.h"
#include "TrView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTrView

IMPLEMENT_DYNCREATE(CTrView, CTreeView)

CTrView::CTrView()
{
}

CTrView::~CTrView()
{
}


BEGIN_MESSAGE_MAP(CTrView, CTreeView)
	//{{AFX_MSG_MAP(CTrView)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTrView drawing

void CTrView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// CTrView diagnostics

#ifdef _DEBUG
void CTrView::AssertValid() const
{
	CTreeView::AssertValid();
}

void CTrView::Dump(CDumpContext& dc) const
{
	CTreeView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CTrView message handlers
