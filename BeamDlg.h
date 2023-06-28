#if !defined(AFX_BEAMDLG_H__6B51CDE9_49AF_432A_BA51_E8EA40E3D70E__INCLUDED_)
#define AFX_BEAMDLG_H__6B51CDE9_49AF_432A_BA51_E8EA40E3D70E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BeamDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CBeamDlg dialog

class CBeamDlg : public CDialog
{
// Construction
public:
	CBeamDlg(CWnd* pParent = NULL);   // standard constructor
	CDocument * doc;
	void SetButtons();

// Dialog Data
	//{{AFX_DATA(CBeamDlg)
	enum { IDD = IDD_Beam_Dlg };
	CButton	m_Beamlet;
	CButton	m_Particles;
	CButton	m_Reionisation;
	CButton	m_Neutralisation;
	CButton	m_BeamPlasma;
	int		m_D;
	int		m_OptPlasma;
	int		m_OptReion;
	int		m_OptRID;
	int		m_OptSINGAP;
	int		m_OptThick;
	int		m_Gauss;
	int		m_Atoms;
	int		m_IonPower;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBeamDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CBeamDlg)
	afx_msg void OnNeutralisation();
	afx_msg void OnBeamPlasma();
	afx_msg void OnReionisation();
	afx_msg void OnBeamlet();
	afx_msg void OnParticles();
	afx_msg void OnOptReion();
	afx_msg void OnE();
	afx_msg void OnOptD();
	afx_msg void OnOptH();
	afx_msg void OnPaint();
	afx_msg void OnOptThickNeutr();
	afx_msg void OnPlasma();
	afx_msg void OnFile();
	afx_msg void OnIonPower();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	int m_Reflect;
	int m_AtomPower;
	int m_NegIon_Power;
	int m_PosIon_Power;
	afx_msg void OnClickedRefl();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BEAMDLG_H__6B51CDE9_49AF_432A_BA51_E8EA40E3D70E__INCLUDED_)
