#pragma once


// CProp3 对话框

class CProp3 : public CPropertyPage
{
	DECLARE_DYNAMIC(CProp3)

public:
	CProp3();
	virtual ~CProp3();

// 对话框数据
	enum { IDD = IDD_PROPPAGE_LARGE2 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
};
