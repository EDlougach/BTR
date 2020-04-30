#pragma once


// CAtomDlg dialog

class CAtomDlg : public CDialog
{
	DECLARE_DYNAMIC(CAtomDlg)

public:
	CAtomDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAtomDlg();

// Dialog Data
	enum { IDD = IDD_ATOM_Dlg };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	double m_PosX;
	double m_PosY;
	double m_PosZ;
	double m_Vx;
	double m_Vy;
	double m_Vz;
	BOOL m_OptReion;
	CString m_Option;
	double m_Step;
	double m_Energy;
};
