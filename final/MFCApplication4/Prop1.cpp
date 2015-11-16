// Prop1.cpp : 实现文件
//

#include "stdafx.h"
#include "MFCApplication4.h"
#include "Prop1.h"
#include "afxdialogex.h"


// CProp1 对话框

IMPLEMENT_DYNAMIC(CProp1, CPropertyPage)

CProp1::CProp1()
	: CPropertyPage(CProp1::IDD)
	, m_nPointNum(20)
	, m_c_arrayLength(20)
{

}

CProp1::~CProp1()
{
}

void CProp1::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CProp1, CPropertyPage)
	ON_BN_CLICKED(IDC_BUTTON1, &CProp1::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CProp1::OnBnClickedButton2)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CProp1 消息处理程序


void CProp1::OnBnClickedButton1()
{
	// TODO:  在此添加控件通知处理程序代码
	KillTimer(2);
	ZeroMemory(&m_HightSpeedChartArray, sizeof(double)*m_c_arrayLength);
	for (size_t i = 0; i<m_c_arrayLength; ++i)
	{
		m_X[i] = i;
	}
	m_count = m_c_arrayLength;
	m_pLineSerie->ClearSerie();
	SetTimer(2, 1000, NULL);
}


void CProp1::OnBnClickedButton2()
{
	// TODO:  在此添加控件通知处理程序代码
	KillTimer(2);
}


BOOL CProp1::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	// TODO:  在此添加额外的初始化
	CRect rect;
	GetDlgItem(IDC_STATIC_Tab)->GetWindowRect(rect);
	GetDlgItem(IDC_STATIC_Tab)->ShowWindow(SW_HIDE);
	ScreenToClient(rect);
	m_tab.Create(CMFCTabCtrl::STYLE_3D_ONENOTE,//控件样式，如最上面所演示 
		rect,//控件区域
		this,//控件的父窗口指针
		1,//控件ID
		CMFCTabCtrl::LOCATION_TOP);//标签位置	
	m_tab.AutoSizeWindow(TRUE);//可以让对话框随tab显示区域而缩放，同时出发对话框的OnSize消息


	m_HSChartCtrl.Create(&m_tab, rect, 2);
	CChartAxis *pAxis = NULL;
	//初始化坐标
	pAxis = m_HSChartCtrl.CreateStandardAxis(CChartCtrl::BottomAxis);
	//pAxis->SetMinMax(30, 50);
	pAxis->SetAutomatic(true);
	//pAxis->SetVisible(false);
	pAxis = m_HSChartCtrl.CreateStandardAxis(CChartCtrl::LeftAxis);
	pAxis->SetMinMax(-1, 10);
	//pAxis->SetAutomatic(true);

	m_pLineSerie = m_HSChartCtrl.CreateLineSerie();
	UpdateData(FALSE);
	m_tab.AddTab(&m_HSChartCtrl, _T("HightSpeedCtrl"));
	m_tab.SetActiveTab(0);//设置显示第一页；     
	m_tab.ShowWindow(SW_SHOWNORMAL);
	CRect TabBRect, TabTRect;
	m_tab.GetWindowRect(rect);
	ScreenToClient(rect);
	m_tab.GetTabArea(TabTRect, TabBRect);
	rect.top += TabTRect.Height();
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常:  OCX 属性页应返回 FALSE
}


void CProp1::OnTimer(UINT_PTR nIDEvent)
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值
	if (0 == nIDEvent)
	{
		++m_count;
		drawMoving();
	}
	if (1 == nIDEvent)
	{
		++m_count;
		drawMoving();
	}
	if (2 == nIDEvent)
	{
		++m_count;
		m_pLineSerie->ClearSerie();
		LeftMoveArray(m_HightSpeedChartArray, m_c_arrayLength, randf(0, 10));
		LeftMoveArray(m_X, m_c_arrayLength, m_count);
		m_pLineSerie->AddPoints(m_X, m_HightSpeedChartArray, m_c_arrayLength);
	}
	CPropertyPage::OnTimer(nIDEvent);
}
double CProp1::randf(double min, double max)
{
	int minInteger = (int)(min * 10000);
	int maxInteger = (int)(max * 10000);
	int randInteger = rand()*rand();
	int diffInteger = maxInteger - minInteger;
	int resultInteger = randInteger % diffInteger + minInteger;
	return resultInteger / 10000.0;
}
void CProp1::drawMoving()
{
	m_pLineSerie->ClearSerie();
	RandArray(m_HightSpeedChartArray, m_c_arrayLength);
	LeftMoveArray(m_X, m_c_arrayLength, m_count);
	m_pLineSerie->AddPoints(m_X, m_HightSpeedChartArray, m_c_arrayLength);
}
void CProp1::LeftMoveArray(double* ptr, size_t length, double data)
{
	for (size_t i = 1; i<length; ++i)
	{
		ptr[i - 1] = ptr[i];
	}
	ptr[length - 1] = data;
}

void CProp1::RandArray(double* ptr, size_t length)
{
	for (size_t i = 0; i<length; ++i)
	{
		ptr[i] = randf(-1, 1);
	}
}