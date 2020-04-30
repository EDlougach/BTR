#pragma once


// CPlasmaDlg dialog

class CPlasmaDlg : public CDialog
{
	DECLARE_DYNAMIC(CPlasmaDlg)

public:
	CPlasmaDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CPlasmaDlg();

// Dialog Data
	enum { IDD = IDD_Plasma_Dlg };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CDocument * doc;
	// Hydrogen ions Weight
	double m_Hw;
	// Deuterium ions Weight
	double m_Dw;
	// Tritium ions Weight
	double m_Tw;
	// Helium ions Weight
	double m_HeW;
	// Litium ions Weight
	double m_LiW;
	// Berillium ions Weight
	double m_BeW;
	// Bor ions Weight
	double m_Bw;
	// Carbon ions Weight
	double m_Cw;
	// Nitrogen ions Weight
	double m_Nw;
	// Oxigen ions Weight
	double m_Ow;
	// Ferrum ions Weight
	double m_FeW;
	afx_msg void OnBnClickedPsi();
	afx_msg void OnBnClickedNtprofiles();
	BOOL m_H;
	BOOL m_D;
	BOOL m_T;
	BOOL m_He;
	BOOL m_Li;
	BOOL m_Be;
	BOOL m_B;
	BOOL m_C;
	BOOL m_N;
	BOOL m_O;
	BOOL m_Fe;
};
