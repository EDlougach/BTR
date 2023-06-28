#if !defined(AFX_SETVIEW_H__EF11C38C_1F7A_11D5_9A4F_006097D3F37D__INCLUDED_)
#define AFX_SETVIEW_H__EF11C38C_1F7A_11D5_9A4F_006097D3F37D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SetView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSetView view
class C3Point;
class CLoad;
class CPlate;

class CSetView : public CScrollView
{
protected:
	CSetView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CSetView)

// Attributes
public:
	CFont  smallfont, font;
//	BOOL ShowLoad;
	double Xmax, Ymax, Zmax;
	double ScaleX, ScaleY, ScaleZ;
	int OrigX, OrigY;
	int ymin1, ymin2;
	int Width, Height;
	BOOL STOP;
	double SinFi, CosFi, SinTeta, CosTeta;
	CLoad * Load;
	CPlate * Plate;
	CRect LoadRect;
	CArray<CRect, CRect> RectArrayX;
	CArray<CRect, CRect> RectArrayY;
	bool ShowProf;

// Operations
public:
	void  UpdateScales(CRect & rect);
	double GetZmax(CLoad* load);
	//void  SetLoad(CLoad * load);
	void  SetLoad_Plate(CLoad * load, CPlate * plate);
	void ShowCoord(CDC* pDC);
	CPoint ScreenPoint(C3Point P);
	void ShowLoad(CDC* pDC);
	void Show3DLoad(CDC* pDC);
	void ShowProfiles();
	void DrawLoadProfiles(CDC* pDC); // in local coord
	void ShowLoadProfiles(CDC* pDC); // in global coord
	void ShowIntegralProfiles(CDC* pDC);
	void ShowGlobalScale(CDC* pDC);
	void DrawLineElement(CDC* pDC, COLORREF color, CPoint  P1, CPoint  P2);
	void DrawTriangle(CDC* pDC, C3Point p1, C3Point p2, C3Point  p3);
	void DrawFlatSurfaceElement(CDC* pDC,  	COLORREF color,
							   CPoint  P1, CPoint P2, CPoint P3, CPoint P4);
	void ShowStatus();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSetView)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnInitialUpdate();     // first time after construct
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CSetView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CSetView)
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SETVIEW_H__EF11C38C_1F7A_11D5_9A4F_006097D3F37D__INCLUDED_)
