#pragma once


// CAddGasDlg dialog

class CAddGasDlg : public CDialog
{
	DECLARE_DYNAMIC(CAddGasDlg)

public:
	CAddGasDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAddGasDlg();
	CDocument * doc;

// Dialog Data
	enum { IDD = IDD_AddGas_Dlg };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	double m_Sigma;
	afx_msg void OnBnClickedAddFilebutton();
};
