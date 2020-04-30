// CommentsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BTR.h"
#include "CommentsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCommentsDlg dialog


CCommentsDlg::CCommentsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCommentsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCommentsDlg)
	m_Comment = _T("");
	//}}AFX_DATA_INIT
}


void CCommentsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCommentsDlg)
	DDX_Text(pDX, IDC_Comment, m_Comment);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCommentsDlg, CDialog)
	//{{AFX_MSG_MAP(CCommentsDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCommentsDlg message handlers
