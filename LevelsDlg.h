#pragma once


// CLevelsDlg dialog

class CLevelsDlg : public CDialog
{
	DECLARE_DYNAMIC(CLevelsDlg)

public:
	CLevelsDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CLevelsDlg();

// Dialog Data
	enum { IDD = IDD_Contours_Dlg };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	double m_Min;
	double m_Max;
	double m_Step;
	BOOL m_Write;
};
