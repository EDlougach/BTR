// OptView.cpp : implementation file
//

#include "stdafx.h"
#include "BTR.h"
#include "OptView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptView

IMPLEMENT_DYNAMIC(COptView, CPropertySheet)

COptView::COptView(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{
}

COptView::COptView(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
}

COptView::~COptView()
{
}


BEGIN_MESSAGE_MAP(COptView, CPropertySheet)
	//{{AFX_MSG_MAP(COptView)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptView message handlers
