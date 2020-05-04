#if !defined(AFX_MAINVIEW_H__EF11C387_1F7A_11D5_9A4F_006097D3F37D__INCLUDED_)
#define AFX_MAINVIEW_H__EF11C387_1F7A_11D5_9A4F_006097D3F37D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MainView.h : header file
//
//#include "partfld.h"
#include "MenuPop.h"
#include "config.h"


/////////////////////////////////////////////////////////////////////////////
// CMainView view

extern enum SORT;
class C3Point;

class CMainView : public CScrollView
{
protected:
	CMainView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CMainView)

// Attributes
public:
//		CBTRDoc*  GetDocument();
	
		BOOL MODE_FIT;
		BOOL STOP;
//		BOOL MODE_SELECT;
//		BOOL MODE_BOX;
//		BOOL MODE_ZOOM;
//		BOOL ZOOM_FINISHED;
		BOOL SHOW_FIELDS;
		BOOL SHOW_NUMBERS;
		BOOL SHOW_BEAM;
		BOOL ZOOM_IN;
		
		double Xmin, Xmax, Ymin, Ymax, Zmin, Zmax;
		double ScaleX, ScaleY, ScaleZ;
		int OrigX, OrigY, OrigZ;
		CPen BlackPen;
		CPen ThinPen;
		CPen ThickPen;
		CPen DotPen;
		CPen MarkPen;
		CPen RedPen;
		CPen RosePen;
		CPen MamugPen;
		CPen SingapPen;
		CPen YellowPen;
		CPen GreenPen;
		CPen BluePen;
		CPen AtomPen;
//		CBrush YellowBrush;
		CFont smallfont;
		CFont font;
		CFont midfont;
		CMenuPop MenuPop;
		CPoint LastPoint;
		CRect  BoxRect, RectPlan, RectSide;
		C3Point ZoomPoint;
		double ZoomCoeffX, ZoomCoeffY, ZoomCoeffZ;
	
// Operations
public:
	
		void  UpdateScales(CRect Plan, CRect Side);
	//	void ZoomView();
		void ShowCoord();
		void ShowNBLine();
	//	void ShowIonSources(CDC* pDC);
		void ShowSINGAP(); // Ion Source
		void ShowMAMuG(); // Ion Source
		void ShowTraced();
		void ShowParticlePos(C3Point Pos, COLORREF color);
		void DrawPartTrack(CArray <C3Point>& Pos, int charge, COLORREF color);
		void ShowComments();
		void ShowTor();
		void ShowProgress(int percent);
		//void DrawManMF(CDC*pDC);
		void OnViewFitonoff();
		void OnUpdateViewFitonoff(CCmdUI* pCmdUI);
		void OnViewBeam();
		void OnUpdateViewBeam(CCmdUI* pCmdUI);
		void OnViewFields();
		void OnUpdateViewFields(CCmdUI* pCmdUI);
		void OnViewNumbering();
		void OnUpdateViewNumbering(CCmdUI* pCmdUI);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainView)
	public:
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnInitialUpdate();     // first time after construct
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CMainView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CMainView)
	afx_msg void OnViewSize();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnPlateLoad();
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnPlateEmit();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	//afx_msg void OnPlateSelect();
	//afx_msg void OnStop();
	afx_msg void OnPlateProperties();
	afx_msg void OnPlateClear();
	afx_msg void OnPlateScale();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	
	afx_msg void OnPlateSave();
	afx_msg void OnPlateSmooth();
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnPlateSelectallplates();
	afx_msg void OnPlate3dplot();
	afx_msg void OnPlateMaxprofiles();
	afx_msg void OnPlateZoomin();
	afx_msg void OnPlateZoomout();
	afx_msg void OnPlateContours();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPlateDelete();
	afx_msg void OnUpdatePlateSelect(CCmdUI *pCmdUI);
//	afx_msg void OnStop();
	afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnSize(UINT nType, int cx, int cy);
//	afx_msg void OnLogSave();
//	afx_msg void OnUpdateLogSave(CCmdUI *pCmdUI);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINVIEW_H__EF11C387_1F7A_11D5_9A4F_006097D3F37D__INCLUDED_)
