#if !defined(AFX_PLOTDLG_H__A6589D34_52C4_4CDA_BE6E_0D261E4704E6__INCLUDED_)
#define AFX_PLOTDLG_H__A6589D34_52C4_4CDA_BE6E_0D261E4704E6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PlotDlg.h : header file
//

struct PLOTINFO // information for Plot screen
{
	CString Caption;
	CString LabelX, LabelY;
	double LimitX, LimitY;
	double CrossX, CrossY;
	BOOL PosNegY; // DataY can change sign 
	BOOL PosNegX;
	byte Line; // 0 - separate points; 1 - solid line; 2 - dotted line
	byte Interp; // 0 - const between 2 points, 1 - linear interp, 2 - spline, ...
	int N; // arrays length, mb equal
	CArray<double, double> * DataX;
	CArray<double, double> * DataY;
	int N1, N2; // begin data series 1,2 
};
/////////////////////////////////////////////////////////////////////////////
// CPlotDlg dialog

class C3Point;

class CPlotDlg : public CDialog
{
// Construction
public:
	PLOTINFO * Plot;
	double Xmin, Xmax, Ymin, Ymax;
	CPoint Orig;
	double ScaleX, ScaleY;
	double ZOOM;
	int SHIFT_X, SHIFT_Y;
	CString	m_Caption;
	HICON icon_UP;

	CPlotDlg(CWnd* pParent = NULL);   // standard constructor
	void InitPlot();
	void InitPlot(double xmin, double xmax, double step); // new - Xmin/Xmax is selected
	void PlotLines(CDC* pDC);
	void DrawPlot(CDC* pDC, CRect * Bound);

// Dialog Data
	//{{AFX_DATA(CPlotDlg)
	enum { IDD = IDD_PLOT_Dlg };
	
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPlotDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPlotDlg)
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

#endif // !defined(AFX_PLOTDLG_H__A6589D34_52C4_4CDA_BE6E_0D261E4704E6__INCLUDED_)
