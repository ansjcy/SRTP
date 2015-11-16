#pragma once
#include "ChartClass\ChartCtrl.h"
#include "ChartClass\ChartLineSerie.h"
#include "ChartClass\ChartAxis.h"

// CProp1 对话框

class CProp1 : public CPropertyPage
{
	DECLARE_DYNAMIC(CProp1)

public:
	CProp1();
	virtual ~CProp1();

// 对话框数据
	enum { IDD = IDD_PROPPAGE_LARGE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	virtual BOOL OnInitDialog();
	//add...
	CMFCTabCtrl m_tab;
	CChartCtrl m_HSChartCtrl;
	CChartLineSerie* m_pLineSerie;
	double randf(double min, double max);
	long m_nPointNum;
private:
	void drawMoving();
	double m_HightSpeedChartArray[2096];
	double m_X[2096];
	unsigned int m_count;
	const size_t m_c_arrayLength;
	void LeftMoveArray(double* ptr, size_t length, double data);
	void RandArray(double* ptr, size_t length);
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
