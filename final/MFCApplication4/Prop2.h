#pragma once


// CProp2 对话框

class CProp2 : public CPropertyPage
{
	DECLARE_DYNAMIC(CProp2)

public:
	CProp2();
	virtual ~CProp2();

// 对话框数据
	enum { IDD = IDD_PROPPAGE_LARGE1 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
};
