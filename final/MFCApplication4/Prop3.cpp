// Prop3.cpp : 实现文件
//

#include "stdafx.h"
#include "MFCApplication4.h"
#include "Prop3.h"
#include "afxdialogex.h"


// CProp3 对话框

IMPLEMENT_DYNAMIC(CProp3, CPropertyPage)

CProp3::CProp3()
	: CPropertyPage(CProp3::IDD)
{

}

CProp3::~CProp3()
{
}

void CProp3::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CProp3, CPropertyPage)
END_MESSAGE_MAP()


// CProp3 消息处理程序
