// InformationDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "RemoteOPCClient.h"
#include "InformationDlg.h"
#include "afxdialogex.h"


// InformationDlg �Ի���

IMPLEMENT_DYNAMIC(InformationDlg, CDialogEx)

InformationDlg::InformationDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(InformationDlg::IDD, pParent)
{

}

InformationDlg::~InformationDlg()
{
}

void InformationDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TCHART1, m_infor);
}


BEGIN_MESSAGE_MAP(InformationDlg, CDialogEx)
END_MESSAGE_MAP()

