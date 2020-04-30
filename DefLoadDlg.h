#pragma once


// ���������� ���� DefLoadDlg

class DefLoadDlg : public CDialog
{
	DECLARE_DYNAMIC(DefLoadDlg)

public:
	DefLoadDlg(CWnd* pParent = NULL);   // ����������� �����������
	virtual ~DefLoadDlg();

// ������ ����������� ����
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DefLoad_Dlg };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // ��������� DDX/DDV

	DECLARE_MESSAGE_MAP()
public:
	int NX;
	int NY;
	BOOL Round;
	int m_OptN;
	double m_StepX;
	double m_StepY;
	BOOL m_OptAtom;
	BOOL m_OptNeg;
	BOOL m_OptPos;
};
