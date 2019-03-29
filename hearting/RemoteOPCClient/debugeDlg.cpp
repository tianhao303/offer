// debugeDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "RemoteOPCClient.h"
#include "debugeDlg.h"
#include "afxdialogex.h"
#include "VariableDef.h"


// debugeDlg 对话框

IMPLEMENT_DYNAMIC(debugeDlg, CDialogEx)

debugeDlg::debugeDlg(CWnd* pParent /*=NULL*/)
{

}

debugeDlg::~debugeDlg()
{
}

void debugeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(debugeDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON1, &debugeDlg::OnBnClickedButton1)
END_MESSAGE_MAP()


// debugeDlg 消息处理程序


void debugeDlg::OnBnClickedButton1()
{
	// TODO:  在此添加控件通知处理程序代码
	CString m_cycle;
	double num = 0;

	GetDlgItemTextW(IDC_EDIT1, m_cycle);
	stepcycle = _ttoi(m_cycle);
	//SetDlgItemTextW(IDC_EDIT3, m_cycle);


	//m_debuger->SetDlgItemTextW(IDC_EDIT3, m_cycle);
}
