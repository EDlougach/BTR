#pragma once


// CThreadsDlg dialog

class CThreadsDlg : public CDialog
{
	DECLARE_DYNAMIC(CThreadsDlg)

public:
	CThreadsDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CThreadsDlg();

// Dialog Data
	enum { IDD = IDD_Threads_Dlg };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CDocument * doc;
	int Nproc;
	// number of active worker threads
	int Nthreads;
//	BOOL m_CalcNeutr;
//	BOOL m_CalcRID;
//	BOOL m_CalcDuct;
//	double m_Xmin;
//	double m_Xmax;
	int m_MaxScen;
	CString m_Scenfilename;
	afx_msg void OnBnClickedButtonParam();
};
