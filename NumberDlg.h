#pragma once


// CNumberDlg dialog

#ifndef __NumberDlg_h__
#define __NumberDlg_h__

class CNumberDlg : public CDialog
{
	DECLARE_DYNAMIC(CNumberDlg)

public:
	CNumberDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CNumberDlg();

// Dialog Data
	enum { IDD = IDD_Num_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	int SurfNumber;
	CString m_Text;
};

 #endif