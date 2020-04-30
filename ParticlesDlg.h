#if !defined(AFX_PARTICLESDLG_H__8AD70406_387E_414F_AA93_2B6688EC4AE2__INCLUDED_)
#define AFX_PARTICLESDLG_H__8AD70406_387E_414F_AA93_2B6688EC4AE2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ParticlesDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CParticlesDlg dialog

class CParticlesDlg : public CDialog
{
// Construction
public:
	CParticlesDlg(CWnd* pParent = NULL);   // standard constructor
	void SetButtons();

// Dialog Data
	//{{AFX_DATA(CParticlesDlg)
	enum { IDD = IDD_Particles_Dlg };
	double	m_EkeV;
	double	m_Lstep;
	double	m_Mass;
	int		m_MaxPoints;
	int		m_OptTime;
	CString	m_Caption;
	int		m_Nucl;
	int		m_Q;
	double	m_Current;
	float	m_Tstep;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CParticlesDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CParticlesDlg)
	afx_msg void OnOptTime();
	afx_msg void OnOptLength();
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PARTICLESDLG_H__8AD70406_387E_414F_AA93_2B6688EC4AE2__INCLUDED_)
