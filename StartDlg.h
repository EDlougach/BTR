#pragma once


// CStartDlg dialog

class CStartDlg : public CDialog
{
	DECLARE_DYNAMIC(CStartDlg)

public:
	CStartDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CStartDlg();

// Dialog Data
	enum { IDD = IDD_START_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	// Start option - New task / Results review / Demo
	int m_Start;
};
