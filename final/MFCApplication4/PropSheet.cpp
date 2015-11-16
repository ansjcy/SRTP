// PropSheet.cpp : 实现文件
//

#include "stdafx.h"
#include "MFCApplication4.h"
#include "PropSheet.h"


// CPropSheet

IMPLEMENT_DYNAMIC(CPropSheet, CPropertySheet)

CPropSheet::CPropSheet(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{

}

CPropSheet::CPropSheet(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
	this->m_psh.dwFlags |= PSH_NOAPPLYNOW; //get rid of "apply" button..

	this->m_psh.dwFlags &= ~(PSH_HASHELP);
	m_prop1.m_psp.dwFlags &= ~(PSP_HASHELP);
	m_prop2.m_psp.dwFlags &= ~(PSP_HASHELP);
	m_prop3.m_psp.dwFlags &= ~(PSP_HASHELP); //get rid of "help" button
	AddPage(&m_prop1);
	AddPage(&m_prop2);
	AddPage(&m_prop3);
}

CPropSheet::~CPropSheet()
{
}


BEGIN_MESSAGE_MAP(CPropSheet, CPropertySheet)
END_MESSAGE_MAP()


// CPropSheet 消息处理程序


BOOL CPropSheet::OnInitDialog()
{
	BOOL bResult = CPropertySheet::OnInitDialog();

	// TODO:  在此添加您的专用代码
	CWnd *pWnd = GetDlgItem(IDOK);
	if (pWnd && pWnd->GetSafeHwnd())
	{
		pWnd->ShowWindow(SW_HIDE);
	}
	pWnd = GetDlgItem(IDCANCEL);
	if (pWnd && pWnd->GetSafeHwnd()){
		pWnd->ShowWindow(SW_HIDE);
	}

	GetDlgItem(IDOK)->ShowWindow(SW_HIDE);
	GetDlgItem(IDHELP)->ShowWindow(SW_HIDE);
	GetDlgItem(IDCANCEL)->ShowWindow(SW_HIDE);
	GetDlgItem(ID_APPLY_NOW)->ShowWindow(SW_HIDE);  //获取窗体尺寸 
	CRect btnRect;
	GetDlgItem(IDCANCEL)->GetWindowRect(&btnRect);
	CRect wdnRect;
	GetWindowRect(&wdnRect);  //调整窗体大小  
	::SetWindowPos(this->m_hWnd, HWND_TOP, 0, 0, wdnRect.Width(), wdnRect.Height() - btnRect.Height(), SWP_NOMOVE | SWP_NOZORDER);
	return bResult;
}
