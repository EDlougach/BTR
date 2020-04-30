#if !defined(AFX_MULTIPLOTDLG_H__5A6B8BE1_6CDC_4E77_8A86_A89707393D05__INCLUDED_)
#define AFX_MULTIPLOTDLG_H__5A6B8BE1_6CDC_4E77_8A86_A89707393D05__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MultiPlotDlg.h : header file
//
#include "PlotDlg.h"
//class CPlotDlg;
/////////////////////////////////////////////////////////////////////////////
// CMultiPlotDlg dialog

class CMultiPlotDlg : public CPlotDlg
{
// Construction
public:
	CMultiPlotDlg(CWnd* pParent = NULL);   // standard constructor
	//CMultiPlotDlg(CDocument * doc);
	virtual void InitPlot(CDocument * pDoc);
	virtual void PlotLines(CDC* pDC);
	virtual void DrawPlot(CDC* pDC, CRect * Bound);

// Dialog Data
	//{{AFX_DATA(CMultiPlotDlg)
	enum { IDD = IDD_MPLOT_Dlg };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA
	//CDocument * pDoc;
	double Xmin, Xmax, Ymin, Ymax, Zmax;
	double LimX, LimY;
	double CrossX, CrossY;
	double * Xdia;
	double * PowerAtDia;
	double ScaleZ;
	CArray<double, double> * DataY;
	COLORREF plotcolor;
	BOOL free;


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMultiPlotDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	
	// Generated message map functions
	//{{AFX_MSG(CMultiPlotDlg)
		// NOTE: the ClassWizard will add member functions here
	afx_msg void OnPaint();
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLeft();
	afx_msg void OnRight();
	afx_msg void OnUp();
	afx_msg void OnDown();
	afx_msg void OnFit();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MULTIPLOTDLG_H__5A6B8BE1_6CDC_4E77_8A86_A89707393D05__INCLUDED_)
#pragma once

