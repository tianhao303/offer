#pragma once
#include "tchart1.h"


// InformationDlg 对话框

class InformationDlg : public CDialogEx
{
	DECLARE_DYNAMIC(InformationDlg)

public:
	InformationDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~InformationDlg();

// 对话框数据
	enum { IDD = IDD_DIALOG1 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CTchart1 m_infor;
};
