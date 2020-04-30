#if !defined(AFX_DATAVIEW_H__EF11C388_1F7A_11D5_9A4F_006097D3F37D__INCLUDED_)
#define AFX_DATAVIEW_H__EF11C388_1F7A_11D5_9A4F_006097D3F37D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DataView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDataView view

class CDataView : public CView
{
protected:
	CDataView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CDataView)

// Attributes
public:
	CBTRDoc* GetDocument();
	CRichEditCtrl   m_rich;
	CFont font;

// Operations
public:
	afx_msg void OnDataActive();
//	void OnDataGet();
//	void OnDataStore();
//	afx_msg void OnUpdateDataStore(CCmdUI* pCmdUI);
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDataView)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL


// Implementation
protected:
	virtual ~CDataView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CDataView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DATAVIEW_H__EF11C388_1F7A_11D5_9A4F_006097D3F37D__INCLUDED_)
