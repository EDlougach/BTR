//#include "partfld.h"
#pragma once


// CRIDDlg dialog
#include "config.h"
#include "BTRDoc.h"

class CRIDDlg : public CDialog
{
	DECLARE_DYNAMIC(CRIDDlg)

public:
	CRIDDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CRIDDlg();
	void DeleteFields();
	void SetDocument(CBTRDoc * doc);
	void SetGeometry(); // size, steps, Eps, Coeff, states...
	int GetState(double x, double y); // in/out AREA, boundary
	int GetState(C3Point P);
	void Init(); // before calculation
	C3Point ConvertToGlobal(C3Point Plocal);
	C3Point ConvertToLocal(C3Point Pglobal);
	double SetU(double u0, double u1, double u2, double u3, double u4, double f0); // new potential in node
	void CalculateNode(int i, int j);
	void Calculate();
	void Read(char * name, double coeff, int mode);
	void Write();
	void WriteTab(char * name);
	void UpdateU();
	C3Point GetEloc(double x, double y);
	C3Point GetEloc(C3Point Ploc);
	double GetUnode(int i, int j, int type);
	void Compensate();
	void Clear_Q();
	void Distribute_Q(double Q, C3Point oldPos, C3Point pos);
	void EmitBeam(int charge);
	void Trace(C3Point Start, C3Point Vstart, int icharge, double Mass, double dT, double PartQ);
	BOOL IsStopped(C3Point Pos);

// Dialog Data
	enum { IDD = IDD_RID_Dlg };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CBTRDoc * pDoc;
	//CRIDField * Field;
	// calculate until
	double m_EpsLimit;
	//  maximum iterations number
	long m_IterLimit;
	long m_Iter;
	float m_Eps;

	int Nx, Ny;
	double StepX, StepY;
	
	int    ** NodeState; // state of point 0 - inner, 1 - RID plate, 2 - boundary 
	double ** NodeU; // normalized potential 
	double ** NodeUnew; //
	double ** NodeQ; // net charge density = Q/StepX/StepY
	
	double Xmax, Ymax; // local dimensions
	double Yground; // U=0 plate Y
	double X0,Y0; // for convertion  in global CS  
	double Xin, Xout, Thickness;
	double Eps, EpsLimit; // accuracy
	int Iter, IterMax; // iteration number
	double Coeff; //RID potential, Ureal = U * Coeff
	double delta;
	double Wrelax; // relaxation
	BOOL STOP; // stop calculation
	BOOL CONTINUE;
		
	char * Fieldname;
	afx_msg void OnBnClickedCalculate();
	afx_msg void OnBnClickedStop();
	afx_msg void OnBnClickedWrite();
	float m_StepX;
	float m_StepY;
	int m_Nx;
	int m_Ny;
	BOOL m_account;
};
