#pragma once


// диалоговое окно CStatOutDlg

class CStatOutDlg : public CDialog
{
	DECLARE_DYNAMIC(CStatOutDlg)

public:
	CStatOutDlg(CWnd* pParent = NULL);   // стандартный конструктор
	virtual ~CStatOutDlg();
	//CDocument * doc;
	//void SetButtons();
	

// Данные диалогового окна
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_StatOptDlg };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // поддержка DDX/DDV

	DECLARE_MESSAGE_MAP()
public:
	int m_Atoms;
	int m_NegIons;
	int m_PosIons;
	int m_PosLoc;
	int m_PosGlob;
	int m_Afall;
	int m_Power;
	int m_Lines;
};
