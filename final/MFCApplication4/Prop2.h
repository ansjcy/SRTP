#pragma once


// CProp2 �Ի���

class CProp2 : public CPropertyPage
{
	DECLARE_DYNAMIC(CProp2)

public:
	CProp2();
	virtual ~CProp2();

// �Ի�������
	enum { IDD = IDD_PROPPAGE_LARGE1 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
};
