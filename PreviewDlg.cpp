// PreviewDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BTR.h"
#include "PreviewDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPreviewDlg dialog


CPreviewDlg::CPreviewDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPreviewDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPreviewDlg)
	m_Filename = _T("");
	m_Text = _T("");
	//}}AFX_DATA_INIT
}


void CPreviewDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPreviewDlg)
	DDX_Text(pDX, IDC_FILENAME, m_Filename);
	DDX_Text(pDX, IDC_Text, m_Text);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPreviewDlg, CDialog)
	//{{AFX_MSG_MAP(CPreviewDlg)
	ON_WM_CREATE()
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPreviewDlg message handlers

int CPreviewDlg::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	char s[1024];
//	char buf[1024];
	CString Line;

	m_Text.Empty();
	int n = 0;
	int pos;
	FILE * fin = fopen(m_Filename, "r");
	if (!fin) {
		AfxMessageBox("Error opening file", MB_ICONSTOP | MB_OK);
		return 1;
	}
/*	while (!feof(fin)) {
		fgets(buf, 1024, fin);
		if (!feof(fin))	m_Text += buf;
		n++;
	}*/
	do{
		if(fgets(s,1024,fin)== NULL) break;
		Line.Empty();
		Line.Format("%s", s);
		pos = Line.FindOneOf("\n\0");
		if (pos >=0) Line.Delete(pos, 1);
		//sprintf(buf,"%s\r\n", s);
		m_Text += Line + "\r\n"; //buf;
		n++;
	} while(1>0);

	fclose(fin);

	if (n<1) {
		sprintf(s,"File is Empty  or  Invalid format");
		AfxMessageBox(s, MB_ICONSTOP | MB_OK);
	}
	else { 
		//sprintf(s,"Gas data is Read (%d lines)", n);
		SetDlgItemText(IDC_Text, m_Text);
		
		//	UpdateData(FALSE);
	}	
	return 0;
}

void CPreviewDlg::OnClose() 
{
	//	
	CDialog::OnClose();
}
