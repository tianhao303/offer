#pragma once
#include "tchart1.h"


// InformationDlg �Ի���

class InformationDlg : public CDialogEx
{
	DECLARE_DYNAMIC(InformationDlg)

public:
	InformationDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~InformationDlg();

// �Ի�������
	enum { IDD = IDD_DIALOG1 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	CTchart1 m_infor;
};
