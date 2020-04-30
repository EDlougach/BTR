#if !defined(AFX_BEAMLETDLG_H__4C91F7A2_AEC9_4C35_BB4F_40E9D9E97233__INCLUDED_)
#define AFX_BEAMLETDLG_H__4C91F7A2_AEC9_4C35_BB4F_40E9D9E97233__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BeamletDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// BeamletDlg dialog
class BeamletDlg : public CDialog
{
// Construction
public:
	BeamletDlg(CWnd* pParent = NULL);   // standard constructor
	CDocument * doc;
	void SetButtons();
	void UpdateValues();

	
// Dialog Data
	//{{AFX_DATA(BeamletDlg)
	enum { IDD = IDD_Beamlet };
	int		m_Nazim;
	int		m_Npolar;
	double	m_BeamletCurr;
	int		m_Nbeamlets;
	CString	m_AzimNStr;
	double	m_CoreDivHor;
	double	m_CoreDivVert;
	double	m_HaloDiv;
	CString	m_PolarNStr;
	double	m_HaloPart;
	int		m_Common;
	float	m_CutOffCurr;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(BeamletDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(BeamletDlg)
	afx_msg void OnCommon();
	afx_msg void OnIndividual();
//	afx_msg void OnPlotBeamletfoot();
//	afx_msg void OnPlotBeamletCurr();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BEAMLETDLG_H__4C91F7A2_AEC9_4C35_BB4F_40E9D9E97233__INCLUDED_)
