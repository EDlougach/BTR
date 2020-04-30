#if !defined(AFX_LOADVIEW_H__EF11C38A_1F7A_11D5_9A4F_006097D3F37D__INCLUDED_)
#define AFX_LOADVIEW_H__EF11C38A_1F7A_11D5_9A4F_006097D3F37D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LoadView.h : header file
//
#include "config.h"

/////////////////////////////////////////////////////////////////////////////
// CLoadView view

class CLoadView : public CScrollView
{
protected:
	CLoadView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CLoadView)

// Attributes
public:
	CFont  font, smallfont, bigfont;
	BOOL ShowLoad;
	BOOL Contours;
	double Xmax, Ymax, Xmin, Ymin;
	double ScaleX, ScaleY;
	int OrigX, OrigY;
	CLoad * Load;
	CPlate * Plate;
	CRect LoadRect;
	BOOL STOP;
	CArray<C3Point, C3Point> LabelArray;
	C3Point Cross;

// Operations
public:
	void  UpdateScales(CRect & rect); // active
	void  UpdateScales(CRect & rect, double DX, double DY); // not used
	void  SetLoad_Plate(CLoad * load, CPlate * plate);
	void  SetPlate(CPlate * plate);
	void  ShowLabels(CDC* pDC);
	void  ShowGlobalScale(CDC* pDC);
	void  ShowLocalScale(CDC* pDC);
	void  ShowPlateBound(CDC* pDC);
	void  ShowFWdata();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLoadView)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	void DrawBMP(CDC * pDC);
	virtual void OnInitialUpdate();     // first time after construct
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CLoadView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CLoadView)
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LOADVIEW_H__EF11C38A_1F7A_11D5_9A4F_006097D3F37D__INCLUDED_)
