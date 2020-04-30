#pragma once


// CLimitsDlg dialog

class CLimitsDlg : public CDialog
{
	DECLARE_DYNAMIC(CLimitsDlg)

public:
	CLimitsDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CLimitsDlg();

// Dialog Data
	enum { IDD = IDD_Limits_Dlg };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	// min value = 0
	double m_Xmin;
	// max value = area Xmax
	double m_Xmax;
	CString m_StrMin;
	CString m_StrMax;
	CString m_Sstep;
	double m_Step;
};
