#if !defined(AFX_TRVIEW_H__EF11C38B_1F7A_11D5_9A4F_006097D3F37D__INCLUDED_)
#define AFX_TRVIEW_H__EF11C38B_1F7A_11D5_9A4F_006097D3F37D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TrView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTrView view

class CTrView : public CTreeView
{
protected:
	CTrView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CTrView)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTrView)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CTrView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CTrView)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TRVIEW_H__EF11C38B_1F7A_11D5_9A4F_006097D3F37D__INCLUDED_)
