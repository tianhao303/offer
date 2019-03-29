// RemoteOPCClientDlg.cpp : 实现文件

#include<stdlib.h>
#include<stdio.h>
#include<math.h>
#include<time.h>
#include <math.h>

#include "stdafx.h"
#include "RemoteOPCClient.h"
#include "RemoteOPCClientDlg.h"
#include "afxdialogex.h"
#include"BtTempCalThread.h"
#include"FurTempOptThread.h"
#include"VariableDef.h"

#include "CSeries.h"   
#include "CAxis.h"  
#include "CAxes.h"  
#include "CLegend.h"
#include "CBarSeries.h"

#include "CEnvironment.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//自定义消息
#define WM_MYMESSAGE		 WM_USER+100	//OnMyMessage
#define WM_MYCLOSEMSG		 WM_USER+101	//opc服务器关闭
#define WM_MYDISCONNECTMSG	 WM_USER+102	//opc连接问题
#define WM_MYLOGOUTMSG		 WM_USER+103	//客户端登出
#define WM_MYCONNECTMSG		 WM_USER+104	//尝试登陆


/////////////////////////调试用变量/////////////////////////////////////

float readData[14];	//存储从文件中读取的数据
bool optFurTemp = false; //优化完成状态

////////////// 较正模型参数
int saveOutData = 0;
////////////////

FILE *fpSaveData = fopen("SaveData.txt","w");
FILE *fpMethanceBt = fopen("btparameter.txt","r");

FILE *fpFurnaceSize = fopen("furnacesize.txt", "r");

QUEUE qTtempIn;
QUEUE qTtempOut;
//float furnaceSize[6];
///////////////////////////////////////////////////////////////////////

////////////////////////控制台输出函数////////////////////////////
#include "io.h"  
#include "fcntl.h" 
void InitConsoleWindow()
{
	AllocConsole();
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	int hCrt = _open_osfhandle((long)handle, _O_TEXT);
	FILE * hf = _fdopen(hCrt, "w");
	*stdout = *hf;
}

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CRemoteOPCClientDlg 对话框

CRemoteOPCClientDlg::CRemoteOPCClientDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CRemoteOPCClientDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CRemoteOPCClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_KVTCPIPCLIENTOCXCTRL1, m_myKingview);
	DDX_Control(pDX, IDC_TCHART1, m_Chart_Preheat);
	DDX_Control(pDX, IDC_TCHART2, bar_chart);
	DDX_Control(pDX, IDC_BUTTON2, m_ButtonBtThread);
	DDX_Control(pDX, IDC_BUTTON3, m_ButtonOptThread);
}

BEGIN_MESSAGE_MAP(CRemoteOPCClientDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_LOGIN, &CRemoteOPCClientDlg::OnBnClickedButtonLogin)
	ON_BN_CLICKED(IDC_BUTTON_LOGOUT, &CRemoteOPCClientDlg::OnBnClickedButtonLogout)
	ON_WM_TIMER()

	//自定义消息映射
	ON_MESSAGE(WM_MYMESSAGE, &CRemoteOPCClientDlg::OnMyMessage)
	ON_MESSAGE(WM_MYGETMSG, &CRemoteOPCClientDlg::OnGetOPCData)
	ON_MESSAGE(WM_MYSETMSG, &CRemoteOPCClientDlg::OnSetOPCData)
	ON_MESSAGE(WM_MYTHREADMSG, &CRemoteOPCClientDlg::OnSetThreadState)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BUTTON1, &CRemoteOPCClientDlg::OnBnClickedButton1)
END_MESSAGE_MAP()


// CRemoteOPCClientDlg 消息处理程序




BOOL CRemoteOPCClientDlg::OnInitDialog()
{
	//登陆界

	CDialogEx::OnInitDialog();
	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	

	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO:  在此添加额外的初始化代码

	//设置画刷颜色
	m_brush.CreateSolidBrush(RGB(179, 179, 179));

	//全屏
	ModifyStyle(WS_CAPTION, 0, 0);
	SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE);

	//// 设置计算线程和优化线程颜色显示 ////
	CWnd *cSeBkBtnMy = GetDlgItem(1059);
	cSeBkBtnMy->EnableWindow(false);

	CWnd *cSeBkBtnMy1 = GetDlgItem(1061);
	cSeBkBtnMy1->EnableWindow(false);

	m_ButtonBtThread.m_bTransparent = FALSE;
	m_ButtonBtThread.m_bDontUseWinXPTheme = TRUE; //两个参数那设置为这中情况，才能显示
	m_ButtonBtThread.SetFaceColor(RGB(255, 0, 0), true);


	m_ButtonOptThread.m_bTransparent = FALSE;
	m_ButtonOptThread.m_bDontUseWinXPTheme = TRUE; //两个参数那设置为这中情况，才能显示
	m_ButtonOptThread.SetFaceColor(RGB(255, 0, 0), true);

	////////////////////////////////调用控制台函数///////////////////////////////////
	InitConsoleWindow();  // add  
	///////////////////////////////关键段初始化//////////////////////////////////////
	InitializeCriticalSection(&crtSec);


	///////////////////////////////炉子信息初始化//////////////////////////////////////
	InitFurnaceInfo();

	init(&qTtempIn);	//初始化对列入炉温度
	init(&qTtempOut);	//初始化对列出炉温度
	//用于显示系统时间
	SetTimer(IDC_TIMER_DISPTIME, 1000, NULL);         //启动定时器
	
	//设置复选框

	//((CButton*)GetDlgItem(1057))->SetCheck(TRUE);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CRemoteOPCClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

BEGIN_EVENTSINK_MAP(CRemoteOPCClientDlg, CDialogEx)
	ON_EVENT(CRemoteOPCClientDlg, IDC_KVTCPIPCLIENTOCXCTRL1, 4, CRemoteOPCClientDlg::OnServercloseKvtcpipclientocxctrl1, VTS_NONE)
	ON_EVENT(CRemoteOPCClientDlg, IDC_KVTCPIPCLIENTOCXCTRL1, 7, CRemoteOPCClientDlg::OnLoginserverokKvtcpipclientocxctrl1, VTS_UI4)
	ON_EVENT(CRemoteOPCClientDlg, IDC_KVTCPIPCLIENTOCXCTRL1, 8, CRemoteOPCClientDlg::OnLogoutserverokKvtcpipclientocxctrl1, VTS_NONE)
	ON_EVENT(CRemoteOPCClientDlg, IDC_KVTCPIPCLIENTOCXCTRL1, 5, CRemoteOPCClientDlg::OnServerdisconnectKvtcpipclientocxctrl1, VTS_NONE)
	ON_EVENT(CRemoteOPCClientDlg, IDC_KVTCPIPCLIENTOCXCTRL1, 1, CRemoteOPCClientDlg::OnVariablevaluechangedKvtcpipclientocxctrl1, VTS_UI2 VTS_UI4 VTS_I2 VTS_BSTR VTS_UI4 VTS_UI4 VTS_UI2)
	ON_EVENT(CRemoteOPCClientDlg, IDC_TCHART2, 7, CRemoteOPCClientDlg::OnClickSeriesTchart2, VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4)
END_EVENTSINK_MAP()

/*
*@brief:		CRemoteOPCClientDlg.OnServercloseKvtcpipclientocxctrl1
*@details:		组态王OPC服务器关闭，触发此事件
*@param[in]		无
*@param[out]	无
*@retval:		void
*/
void CRemoteOPCClientDlg::OnServercloseKvtcpipclientocxctrl1()
{
	// TODO:  在此处添加消息处理程序代码

	//关闭显示定时器IDC_TIMER_DISPLAY
	KillTimer(IDC_TIMER_DISPLAY);
	KillTimer(IDC_TIMER_DISPTIME);

	//关闭计算线程
	if (NULL != this->calThread)
	{
		::TerminateThread(calThread, 0);		//结束计算线程
		::CloseHandle(hMainTimerCal);			//关闭计算线程内核定时器
		calThread = NULL;
		//		GetDlgItem(IDC_COMSTAT)->SetWindowText("CLOSED!");
	}
	if (NULL != this->optThread)
	{
		::TerminateThread(optThread, 0);		//结束优化线程
		optThread = NULL;
	}
	//logout the opc server
	if (TRUE == this->loginFlag)
	{
		//		this->m_KingViewCtrl.Method_LogoutServer();
		this->loginFlag = FALSE;
	}

	ReleaseSemaphore(smphPrint, 1, NULL);

	//发送组态王服务器关闭的消息
	PostMessage(WM_MYMESSAGE, WM_MYCLOSEMSG, NULL);
}


/**
*@brief:		CRemoteOPCClientDlg.OnServercloseKvtcpipclientocxctrl1
*@details:		成功登陆组态王OPC服务器，触发此事件
*@param[in]		无
*@param[out]	无
*@retval:		void
*/
void CRemoteOPCClientDlg::OnLoginserverokKvtcpipclientocxctrl1(unsigned long client_id)
{
	// TODO:  在此处添加消息处理程序代码
}


/**
*@brief:		CRemoteOPCClientDlg.OnServercloseKvtcpipclientocxctrl1
*@details:		登出组态王OPC服务器，触发此事件
*@param[in]		无
*@param[out]	无
*@retval:		void
*/
void CRemoteOPCClientDlg::OnLogoutserverokKvtcpipclientocxctrl1()
{
	// TODO:  在此处添加消息处理程序代码

	//关闭显示定时器IDC_TIMER_DISPLAY
	KillTimer(IDC_TIMER_DISPLAY);
	//系统时间显示定时器
	KillTimer(IDC_TIMER_DISPTIME);
//	SetTimer(IDC_TIMER_LOGIN, 1000, NULL);
	//关闭计算线程
	if (NULL != this->calThread)
	{
		::TerminateThread(calThread, 0);		//结束计算线程
		::CloseHandle(hMainTimerCal);			//关闭计算线程内核定时器
		calThread = NULL;
//		GetDlgItem(IDC_COMSTAT)->SetWindowText("CLOSED!");
	}
	if (NULL != this->optThread)
	{
		::TerminateThread(optThread, 0);		//结束优化线程
		optThread = NULL;
	}

	//update the connect flag
	if (loginFlag == true)
	{
		loginFlag = FALSE;

		//发送登出组态王服务器的消息
		PostMessage(WM_MYMESSAGE, WM_MYLOGOUTMSG, NULL);
	}

	ReleaseSemaphore(smphPrint, 1, NULL);
	
}

/**
*@brief:		CRemoteOPCClientDlg.OnServercloseKvtcpipclientocxctrl1
*@details:		与组态王OPC服务器连接出现问题，触发此事件
*@param[in]		无
*@param[out]	无
*@retval:		void
*/
void CRemoteOPCClientDlg::OnServerdisconnectKvtcpipclientocxctrl1()
{
	// TODO:  在此处添加消息处理程序代码

	//关闭显示定时器IDC_TIMER_DISPLAY
	KillTimer(IDC_TIMER_DISPLAY);

	//关闭计算线程
	if (NULL != this->calThread)
	{
		//freeOptSize(pHead);						//释放计算线程申请的空间
		//MessageBox(_T("释放线程资源"));
		::TerminateThread(calThread, 0);		//结束计算线程
		::CloseHandle(hMainTimerCal);			//关闭计算线程内核定时器
		calThread = NULL;
		//		GetDlgItem(IDC_COMSTAT)->SetWindowText("CLOSED!");
	}
	if (NULL != this->optThread)
	{
												//判读优化线程内存是否需要释放
		::TerminateThread(optThread, 0);		//结束优化线程
		optThread = NULL;
	}
	//logout the opc server
	if (TRUE == this->loginFlag)
	{
//		this->m_KingViewCtrl.Method_LogoutServer();
		this->loginFlag = FALSE;
	}
	//发送与组态王连接断开的消息
	PostMessage(WM_MYMESSAGE, WM_MYDISCONNECTMSG, NULL);

	ReleaseSemaphore(smphPrint, 1, NULL);
}


/**
*@brief:		CRemoteOPCClientDlg.OnServercloseKvtcpipclientocxctrl1
*@details:		订阅组态王变量值发生变化时，触发此事件
*@param[in]		无
*@param[out]	无
*@retval:		void
*/
void CRemoteOPCClientDlg::OnVariablevaluechangedKvtcpipclientocxctrl1(unsigned short station_id, unsigned long variable_id, short variable_value_type, LPCTSTR variable_value_string, unsigned long stamp_time_hi, unsigned long stamp_time_lo,
	unsigned short stamp_quality)
{
	// TODO:  在此处添加消息处理程序代码
	printf("%d ", (int)(variable_id));
	/*****************************二级投入标志位*************************************
	//若二级投入标志位发生改变
	//读取二级投入标志位
	//若二级投入标志位1，则
	// 38为步进梁信号   31为入钢信号   122为切换信号 32出炉
	******************************************************************************/
	if ((int)(variable_id) == 122)
	{
		ReadVariableValue(123, &inputFlag);		//读回二级投入标志位
	
		if (inputFlag == 1)
		{
			
			//开启炉温优化线程
			if (NULL == this->optThread)
			{
				calculateComplete = 0;		//二级计算完成标志位置0
				this->optThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)FurTempOpt, this->m_hWnd, 0, NULL);
				optState = true;
				SetThreadPriority(optThread, THREAD_PRIORITY_LOWEST);
			}

		}
		else
		{

			calculateComplete = 0;
			Sleep(4000);
			WritevariableValue(126, &calculateComplete);		//写回二级投入标志位
			m_ButtonOptThread.SetFaceColor(RGB(255, 0, 0), true);

			if (NULL != this->optThread)
			{
				if (optRelease)
				{
					freeOptSize(NewbtList);
				}
				::TerminateThread(optThread, 0);		//结束优化线程
				optThread = NULL;
				optState = false;
			}

		}
	}

	//出炉信号
	if ((int)(variable_id) == 32)
	{
		bool tempOutSignal = false;
		ReadVariableValue(33, &tempOutSignal);
		CString c;
		c.Format(_T("%d"), tempOutSignal);
		GetDlgItem(1033)->SetWindowTextW(c);

		if (tempOutSignal == true)
		{
			btOutSignal = true;
		}

	}

	/*****************************入炉信号标志位*************************************
	//若入炉信号标志位发生改变
	//读取入炉信号标志位
	//需不需要置零
	//需不需要写回
	******************************************************************************/
	if ((int)(variable_id) == 31)
	{
		bool tempInSignal = false;
		ReadVariableValue(32, &tempInSignal);

		CString b;
		b.Format(_T("%d"), tempInSignal);
		GetDlgItem(1024)->SetWindowTextW(b);

		if (tempInSignal == true)
		{
			btInSignal = true;
			
		}
	}
	
	// 步进梁步进信号
	
	if ((int)(variable_id) == 38)
	{

		bool workingBeamSignal = false;
		ReadVariableValue(39, &workingBeamSignal);

		CString worinIng;
		worinIng.Format(_T("%d"), workingBeamSignal);
		GetDlgItem(1033)->SetWindowTextW(worinIng);

		/*****************************出炉信号标志位、步进梁信号*************************************
		//若出炉信号标志位发生改变
		//读取出炉信号标志位

		******************************************************************************/
		///读取步进梁步进	
		if (workingBeamSignal == true)
		{

			if (countTime == 0)
			{
				begin = clock();
				btOutSignal = true;

				countTime += 1;
			}
			else
			{
				clock_t end = clock();
				double time = double((end - begin) / CLOCKS_PER_SEC);
				stepcycle = (int)time;
				printf("哈啊哈哈哈哈哈哈%d\n", stepcycle);
				countTime = 0;
			}
		}
	}

}

/**
*@brief:		CRemoteOPCClientDlg.OnBnClickedButtonLogin
*@details:		登录按钮,连接组态王OPC服务器,开启通讯定时器以及钢坯预测线程和炉温优化线程
*@param[in]		无
*@param[out]	无
*@retval:		void
*/
void CRemoteOPCClientDlg::OnBnClickedButtonLogin()
{
	//if (loginFlag == false)
	//{
	//	loginFlag = true;
	//	//开启炉温预设定计算线程
	//	if (NULL == this->calThread)
	//	{
	//		this->calThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)BtTempCal, this->m_hWnd, 0, NULL);
	//		SetThreadPriority(calThread, THREAD_PRIORITY_BELOW_NORMAL);
	//	}
	//
	//	//开启炉温优化线程
	//	/*if (NULL == this->optThread)
	//	{
	//		this->optThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)FurTempOpt, this->m_hWnd, 0, NULL);
	//		SetThreadPriority(optThread, THREAD_PRIORITY_LOWEST);
	//		optState = true;
	//	}*/

	//	//定时更新显示数据
	//	SetTimer(IDC_TIMER_DISPLAY, 3000, NULL);
	//}
	//else
	//{
	//	AfxMessageBox(_T("已与组态王建立连接！"));
	//}

	if (false == loginFlag)
	{
		//登录组态王
		short result = m_myKingview.Method_LoginServer(_T("192.168.43.208"), 41190, _T("admin"), _T("123456"));
		::Sleep(1000);
		if (result == 2 || result == 4)
		{
			loginFlag = true;			//登录状态标志位置1

			//开启显示定时器
			SetTimer(IDC_TIMER_DISPLAY, 3000, NULL);

			//开启炉温预设定计算线程
			if (NULL == this->calThread)
			{
				this->calThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)BtTempCal, this->m_hWnd, 0, NULL);
				SetThreadPriority(calThread, THREAD_PRIORITY_BELOW_NORMAL);
				m_ButtonBtThread.SetFaceColor(RGB(0, 255, 0), true);
			}
			
			//订阅变量
			SubscribeVariableValue(30); //订阅二级投入标志位inputFlag
			SubscribeVariableValue(39); //#步进梁信号
			SubscribeVariableValue(32); //订阅入炉信号btInSignal
			SubscribeVariableValue(33); //订阅出炉信号btInSignal
			

			//查看此时是否有二级投入
			ReadVariableValue(123, &inputFlag);
	
			if (inputFlag == 1)
			{
				calculateComplete = 0;		//则从下一次优化计算完成开始发送炉温设定值	
				//开启炉温优化线程
				if (NULL == this->optThread)
				{
					this->optThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)FurTempOpt, this->m_hWnd, 0, NULL);
					SetThreadPriority(optThread, THREAD_PRIORITY_LOWEST);
				}
			}
		}
		else
		{
			PostMessage(WM_MYMESSAGE, WM_MYCONNECTMSG, NULL);
		}
	}
	else
	{
		AfxMessageBox(_T("已与组态王建立连接"));
	}
}


/**
*@brief:		CRemoteOPCClientDlg.OnBnClickedButtonLogout
*@details:		登出按钮,断开组态王OPC服务器,关闭通讯定时器以及钢坯预测线程和炉温优化线程
*@param[in]		无
*@param[out]	无
*@retval:		void
*/
void CRemoteOPCClientDlg::OnBnClickedButtonLogout()
{
	// TODO:  在此添加控件通知处理程序代码
	if (loginFlag == true)
	{
		m_myKingview.Method_LogoutServer();
		//m_ButtonBtThread.SetFaceColor(RGB(255, 0, 0), true);//1月14日
	}
	else
	{
		AfxMessageBox(_T("未登录组态王服务器！"));
	}

}
////////////////用于显示系统时间  1s 1次
///////////////////12月22日修改

void CRemoteOPCClientDlg::System_time()
{
	CString strTime;
	CTime tm;
	tm = CTime::GetCurrentTime();
	strTime = tm.Format("%Y年%m月%d日 %H:%M:%S");
	GetDlgItem(1032)->SetWindowTextW(strTime); 
	
	//CString opt_stata;
	//CString cal_State;

	//opt_stata.Format(_T("%d"), bool(optState));
	//GetDlgItem(1027)->SetWindowTextW(opt_stata);

	//cal_State.Format(_T("%d"), bool(calThread));
	//GetDlgItem(1026)->SetWindowTextW(cal_State);


}

void CRemoteOPCClientDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值
	switch (nIDEvent)
	{
	case IDC_TIMER_DISPLAY:
		Draw();
		break;
	case IDC_TIMER_DISPTIME:
		System_time();
		break;
	default:
		break;
	}
	CDialogEx::OnTimer(nIDEvent);
}
/**
*@brief: 		CRemoteOPCClientDlg.ReadVariableValue
*@details:		通过变量ID号，读取组态王OPC服务器中对应变量的值
*@param[in]		int variableID: variable的标识号，组态王中数据词典内的ID
*@param[out]	void *outVariableValue  :读取到的对应variable的值
*@retval:		bool
*/
bool CRemoteOPCClientDlg::ReadVariableValue(int variableID, void * outVariableValue)
{
	unsigned short stationNumber = 0;	//站点数
	short variableType = -1;		//变量类型：3：整型；4：实型；11：离散型；16400：字符串型
	BSTR variableStr = ::SysAllocStringLen(NULL, 50);	//给变量申请空间
	//获取站点数量
	m_myKingview.Method_GetStationNumber(&stationNumber);
	//读取ID号为variableID的数据类型和数据值相对应的字符串
	m_myKingview.Method_GetVariableValueByVariableId(stationNumber - 1, variableID, &variableType, &variableStr);

	CString variableTemp;	//临时变量字符串
	variableTemp.Empty();
	//将BSTR型字符串付给CString型
	variableTemp = variableStr;

	//根据读取的变量类型，将变量转换为相应的数值
	int iTemp = -1;
	long lTemp = -1;
	float fTemp = -1;
	switch (variableType)
	{
	case 11:
		iTemp  = _ttoi(variableTemp);
		*((bool*)outVariableValue) = (bool)iTemp;
		break;
	case 3:
		lTemp = _ttol(variableTemp);
		*((long*)outVariableValue) = lTemp;
		break;
	case 4:
		fTemp = _ttof(variableTemp);
		*((float*)outVariableValue) = fTemp;
		break;
	case 16400:
		char * rTemp = _com_util::ConvertBSTRToString(variableStr);
		strcpy((char *)outVariableValue, rTemp);
		delete[] rTemp;
		break;
	}

	//释放BSTR分配的空间
	SysFreeString(variableStr);
	
	return false;
}

/**
*@brief: 		CRemoteOPCClientDlg.WritevariableValue
*@details:		将变量的值根据变量的ID号写回组态王OPC服务器
*@param[in]		int variableID: variable的标识号，组态王中数据词典内的ID
				void *inVariableValue: the data pointer that you wanna write
*@param[out]	无
*@retval:		bool
*/
bool CRemoteOPCClientDlg::WritevariableValue(int variableID, void * inVariableValue)
{
	unsigned short stationNumber = 0;	//站点数
	short variableType = -1;		//变量类型：3：整型；4：实型；11：离散型；16400：字符串型
	BSTR variableStr = ::SysAllocStringLen(NULL, 50);	//给变量申请空间
	//获取站点数量
	m_myKingview.Method_GetStationNumber(&stationNumber);
	//读取ID号为variableID的数据类型
	m_myKingview.Method_GetVariableValueByVariableId(stationNumber - 1, variableID, &variableType, &variableStr);

	CString variableTemp;
	variableTemp.Empty();

	//根据读取的数据类型，转换成字符串型
	switch (variableType)
	{
	case 11:
		variableTemp.Format(_T("%i"), *(bool*)inVariableValue);
		break;
	case 3:
		variableTemp.Format(_T("%i"), *(int *)inVariableValue);
		break;
	case 4:
		variableTemp.Format(_T("%f"), *(float *)inVariableValue);
		break;
	case 16400:
		variableTemp.Format(_T("%s"), (unsigned char *)inVariableValue);
		break;
	}
	//根据变量ID号向组态王写回变量值
	m_myKingview.Method_SetVariableValueByVariableId(stationNumber - 1, variableID, variableTemp);
	//释放申请的空间
	SysFreeString(variableStr);
	
	return false;
}


/**
*@brief: 		CRemoteOPCClientDlg.OnMyMessage
*@details:		针对不同的与组态王的连接状况，作出相应的报警
*@param[in]		WPARAM wParam: 消息参数，根据此参数判断进行相应的报警
				LPARAM lParam: 消息参数，未用
*@param[out]	无
*@retval:		LRESULT
*/
afx_msg LRESULT CRemoteOPCClientDlg::OnMyMessage(WPARAM wParam, LPARAM lParam)
{
	// 一级主动切出时，二级如何动作， 
	switch (wParam)
	{
	case WM_MYCLOSEMSG:
		AfxMessageBox(_T("OPC服务器已关闭"));
		break;
	case WM_MYDISCONNECTMSG:
		AfxMessageBox(_T("与OPC服务器连接出现问题，请重新连接"));
		break;
	case WM_MYLOGOUTMSG:
		AfxMessageBox(_T("OPC客户端已登出"));
		break;
	case WM_MYCONNECTMSG:	
		if (0 == connectResponse)
		{
			connectResponse++;
			AfxMessageBox(_T("尝试与组态王建立连接失败"));
			connectResponse = 0;
		}	
		break;
	default:
		break;
	}

	return 0;
}

//修改 2018-11-29/////
// th	添加对列
void ReadInitTemp(float btInitTemp)
{
	//需添加代码，判断钢坯是冷坯还是热坯，从哪里读数据，读红外成像的，还是冷坯检测的，读到的大于300℃为热坯，小于50摄氏度为冷坯。
	//现在不需要添加
	float ppp;

	if (full_quene(&qTtempIn) != 0)	//如果是对列没满则向对列中加数据
	{
		en_quene(&qTtempIn, btInitTemp);	//入队列,初始化对列
	}
		else
		{
			out_quene(&qTtempIn, &ppp);	//出对列
			en_quene(&qTtempIn, btInitTemp);	//入队列

			//////////20待修改///////////////////
			if (abs(qTtempIn.TEMP[(qTtempIn.rear + NUM - 1) % NUM] - qTtempIn.TEMP[(qTtempIn.rear + NUM - 2) % NUM]) > 20)
			{
				btInfoInit.btTempInit = qTtempIn.TEMP[(qTtempIn.rear + NUM - 2) % NUM];
				printf("采用上一时刻入炉温度%.2f\n", qTtempIn.TEMP[(qTtempIn.rear + NUM - 2) % NUM]);
			}
			else
			{
				btInfoInit.btTempInit = (qTtempIn.TEMP[(qTtempIn.rear + NUM - 1) % NUM] + qTtempIn.TEMP[(qTtempIn.rear + NUM - 2) % NUM]) / 2;

			}
			printf("读到数据%.2f\n", btInitTemp);
			printf("入炉温度为%.2f\n", btInfoInit.btTempInit);
		}
		bianli_queue(&qTtempIn);
		printf("最新的%.2f\n", qTtempIn.TEMP[(qTtempIn.rear + NUM - 1) % NUM]);

}

void ReadOutTemp(float btTempOut)
{
	float ppp;

		if (full_quene(&qTtempOut) != 0)	//如果是对列没满则向对列中加数据
		{
			en_quene(&qTtempOut, btTempOut);	//入队列,初始化对列
		}
		else
		{
			out_quene(&qTtempOut, &ppp);	//出对列
			en_quene(&qTtempOut, btTempOut);	//入队列

			//////////20待修改///////////////////
			if (abs(qTtempOut.TEMP[(qTtempOut.rear + NUM - 1) % NUM] - qTtempOut.TEMP[(qTtempOut.rear + NUM - 2) % NUM]) > 20)
			{
				btInfoInit.btTempOut = qTtempOut.TEMP[(qTtempOut.rear + NUM - 2) % NUM];
				printf("采用上一时刻出炉温度%.2f\n", qTtempOut.TEMP[(qTtempOut.rear + NUM - 2) % NUM]);
			}
			else
			{
				btInfoInit.btTempOut = (qTtempOut.TEMP[(qTtempOut.rear + NUM - 1) % NUM] + qTtempOut.TEMP[(qTtempOut.rear + NUM - 2) % NUM]) / 2 ;

			}
			printf("读到数据%.2f\n", btTempOut);
			printf("出炉温度为%.2f\n", btInfoInit.btTempOut);
		}
		bianli_queue(&qTtempOut);
		printf("最新的出炉%.2f\n", qTtempOut.TEMP[(qTtempOut.rear + NUM - 1) % NUM]);

}
void SaveData()
{
	//保存数据2、入炉温度3、出炉温度 1、实测炉温4、步进梁周期5、步进梁步距


	// 有入炉写入入炉温度
	if (btInSignal == 1)
	{
		fprintf(fpSaveData, "%d\t", btInfoInit.btTempInit);

	}
	else
	{
		fprintf(fpSaveData, "%d\t", 0);
	}
	// 有出炉写入出炉温度
	if (btOutSignal == 1)
	{
		fprintf(fpSaveData, "%d\t", btInfoInit.btTempOut);
		saveOutData++;
	}
	else
	{
		fprintf(fpSaveData, "%d\t", 0);
	}

	for (int i = 0; i < 13; i++)
	{
		fprintf(fpSaveData, "%.2f\t", readData[i]);
	}
	

	fprintf(fpSaveData, "%d\t", stepcycle);
	fprintf(fpSaveData, "%.2f\t", steplength);
	fprintf(fpSaveData, "\n");
}

/**
*@brief: 		CRemoteOPCClientDlg.OnGetOPCData
*@details:		通过控件读取加热炉实时数据
*@param[in]		WPARAM wParam: 消息参数，未用
				LPARAM lParam: 消息参数，未用
*@param[out]	无
*@retval:		LRESULT
*/
afx_msg LRESULT CRemoteOPCClientDlg::OnGetOPCData(WPARAM wParam, LPARAM lParam)
{

	//读取炉温实测值,钢坯入炉温度、钢坯出炉温度、钢坯种类信息
	//修改日期2019年-2-18日
	//钢种信息在入炉时读取文件，需在线读通过组态王
	//读取钢坯的加热制度信息
	EnterCriticalSection(&crtSec);
	printf("\n\n-------------------主线程进入临界区--------------------\n\n");
	//线程同步，进入关键段


	ReadVariableValue(29, &furTempMeasured[0]);//预热短炉温实测

	ReadVariableValue(27, &furTempMeasured[1]);//加二上东炉温实测
	ReadVariableValue(27, &furTempMeasured[2]);//加二上西炉温实测

	ReadVariableValue(28, &furTempMeasured[3]);//加二下东炉温实测
	ReadVariableValue(28, &furTempMeasured[4]);//加二下西炉温实测

	ReadVariableValue(25, &furTempMeasured[5]);//加一上东炉温实测
	ReadVariableValue(25, &furTempMeasured[6]);//加一上西炉温实测

	ReadVariableValue(26, &furTempMeasured[7]);//加一下东炉温实测
	ReadVariableValue(26, &furTempMeasured[8]);//加一下西炉温实测

	ReadVariableValue(21, &furTempMeasured[9]);//均热上东炉温实测
	ReadVariableValue(22, &furTempMeasured[10]);//均热上西炉温实测
	ReadVariableValue(23, &furTempMeasured[11]);//均热下东炉温实测
	ReadVariableValue(24, &furTempMeasured[12]);//均热下西炉温实测

	for (int i = 0; i < TEMPCNT; i++)
	{
		//ReadVariableValue(21+i, &furTempMeasured[i]);
		CString tempMeasured;
		tempMeasured.Format(_T("%.2f"), furTempMeasured[i]);

		GetDlgItem(1036+i)->SetWindowTextW(tempMeasured);
	}

	//预热短炉温
	furTemp[0] = furTempMeasured[0];
	//加热一段炉温用加热一段上东西炉温均值
	furTemp[1] = (furTempMeasured[1] + furTempMeasured[2]) / 2;
	//加热二段炉温用加热二段上东西炉温均值
	furTemp[2] = (furTempMeasured[5] + furTempMeasured[6]) / 2;
	//均热段炉温用均热段上东西炉温均值
	furTemp[3] = (furTempMeasured[9] + furTempMeasured[10]) / 2;

	//预热段下
	botTemp[0] = furTempMeasured[0] ;
	//加热二段下
	botTemp[1] = (furTempMeasured[3] + furTempMeasured[4])/2;
	//加热一段下
	botTemp[2] = (furTempMeasured[7] + furTempMeasured[8])/2;
	
	// 均热段下
	botTemp[3] = (furTempMeasured[11] + furTempMeasured[12]) / 2;

	//读取入出炉温度
	float btInitTemp;
	float btTempOut;

	ReadVariableValue(37, &btInitTemp);	//入炉温度
	ReadInitTemp(btInitTemp);	//添加对列-2018-2-20修改读取入炉温度实测
	ReadVariableValue(38, &btTempOut);	//出炉温度
	ReadOutTemp(btTempOut);	//添加对列-2018-2-20修改读取出炉温度实测
	
	ReadVariableValue(36, &btInfoInit.btpysicalinfo.btType);	//钢坯种类

	btInfoInit.bttarget.btTargetTemp = 1100;

	//显示钢种
	//::SetDlgItemTextA(this->m_hWnd, IDC_EDIT13, btInfoInit.btpysicalinfo.btType);

	CString Cyclea;
	Cyclea.Format(_T("%d"), stepcycle);
	GetDlgItem(1003)->SetWindowTextW(Cyclea);

	//初始化完成才能开始矫正模型并且温差小于50℃时，矫正模型

	if (initterminalFlag == true  && (pHead->btinfo.btTempDistri[0] - btInfoInit.btTempOut) > 50)
	{
		if ( saveOutData < 3)
		{
			SaveData();			//保存数据
		}
		else if (saveOutData == 3)
		{
			fprintf(fpSaveData, "--------------------------------");
			saveOutData = 0;
		}
	}
	LeaveCriticalSection(&crtSec);


	return 0;
}

/**
*@brief: 		CRemoteOPCClientDlg.OnSetOPCData
*@details:		写回计算后的炉温预设值
*@param[in]		WPARAM wParam: 消息参数，未用
				LPARAM lParam: 消息参数，未用
*@param[out]	无
*@retval:		LRESULT
*/
afx_msg LRESULT CRemoteOPCClientDlg::OnSetOPCData(WPARAM wParam, LPARAM lParam)
{
	//////////////////////////////////////////调试用////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////
	CString secondTemp;
	CString firstTemp;;
	CString preatingTemp;

	//WritevariableValue(126, &inputFlag);
	
	printf("llllllllllllllllllllllllll\n");
	furTempSet[0] = 1100;
	furTempSet[1] = 1200;
	furTempSet[2] = 1230;
	secondTemp.Format(_T("%.2f"), furTempSet[0]);
	firstTemp.Format(_T("%.2f"), furTempSet[1]);
	preatingTemp.Format(_T("%.2f"), furTempSet[2]);
	
	if (calculateComplete == 0)
	{
		WritevariableValue(129, &furTempSet[0]);
		WritevariableValue(128, &furTempSet[1]);
		WritevariableValue(130, &furTempSet[2]);

		GetDlgItem(1016)->SetWindowTextW(secondTemp);
		GetDlgItem(1015)->SetWindowTextW(firstTemp);
		GetDlgItem(1030)->SetWindowTextW(preatingTemp);
		calculateComplete = 1;
		WritevariableValue(126, &calculateComplete);
		m_ButtonOptThread.SetFaceColor(RGB(0, 255, 0), true);
		
	}
	else
	{
		//
		WritevariableValue(129, &furTempSet[0]);
		WritevariableValue(128, &furTempSet[1]);
		WritevariableValue(130, &furTempSet[2]);

		GetDlgItem(1016)->SetWindowTextW(secondTemp);
		GetDlgItem(1015)->SetWindowTextW(firstTemp);
		GetDlgItem(1030)->SetWindowTextW(preatingTemp);

		//WritevariableValue(126, &calculateComplete);
		//m_ButtonOptThread.SetFaceColor(RGB(0, 255, 0), true);
	}
	
	printf("炉温设定值为： %f\t%f\n", furTempSet[0], furTempSet[1]);
	
	return 0;
}


/**
*@brief: 		CRemoteOPCClientDlg.SubscribeVariableValue
*@details:		通过变量ID号订阅组态王中变量的值变化，以使用变量值改变通知事件
*@param[in]		int variableID：variable的标识号，组态王中数据词典内的ID
*@param[out]	无
*@retval:		bool
*/
bool CRemoteOPCClientDlg::SubscribeVariableValue(int variableID)
{
	unsigned short stationNumber = 0;	//站点数
	printf("ddd %d\n", m_myKingview.Method_GetStationNumber(&stationNumber));	//获取站点数

	printf("aaa %d\n",m_myKingview.Method_SubscibeVariable(stationNumber - 1, variableID, 1));		//订阅variableID变量的值变化

	return false;
}



afx_msg LRESULT CRemoteOPCClientDlg::OnSetThreadState(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case WM_MYTHREADSUSPEND:
		SuspendThread(optThread);
		break;
	case WM_MYTHREADRESUME:
		ResumeThread(optThread);
		break;
	default:
		break;
	}

	return 0;
}

HBRUSH CRemoteOPCClientDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);
	CFont all_font;
	all_font.CreatePointFont(130, L"华文行楷");

	// TODO:  在此更改 DC 的任何特性
	//在这加一条是否为对话框的判断语句  
	if (nCtlColor == CTLCOLOR_DLG )
	{
//		pDC->SetBkMode(TRANSPARENT);
//		pDC->SetTextColor(RGB(0, 255, 0));
		return   m_brush;   //返加红色刷子 
	}
	else if (nCtlColor == CTLCOLOR_STATIC)  //更改静态文本
	{
		pDC->SetTextColor(RGB(9, 101, 250));
		pDC->SetBkColor(RGB(179, 179, 179));
		pDC->SelectObject(&all_font);

		HBRUSH b = CreateSolidBrush(RGB(179, 179, 179));
		CFont m_font;
		if (pWnd->GetDlgCtrlID() == IDC_STATIC1)
		{
			m_font.CreatePointFont(220, L"华文行楷");
			pDC->SelectObject(&m_font);
			
		}
		return b;
	}
	else if (nCtlColor == CTLCOLOR_EDIT)  //更改文本框文本
	{
		pDC->SelectObject(&all_font);
		return hbr;
	}	 

	//TODO:  如果默认的不是所需画笔，则返回另一个画笔
	return hbr;
}

void CRemoteOPCClientDlg::Draw()
{
	//2019年2月17日在界面上添加优化设定曲线。
	const UINT N = 200;

	double para_up[3];
	double para_down[3];
	double para_opt[3];

	polyfit(sizeof(thermocouple) / sizeof(thermocouple[0]), thermocouple, furTemp, 2, para_up);//上部炉温
	polyfit(sizeof(thermocouple) / sizeof(thermocouple[0]), thermocouple, botTemp, 2, para_down);//下部炉温
	polyfit(sizeof(thermocouple) / sizeof(thermocouple[0]), thermocouple, furTempSet, 2, para_opt);//预设优化炉温

	double dDataX_up[N];
	double dDataY_up[N];

	double dDataX_down[N];
	double dDataY_down[N];

	double dDataX_Opt[N];
	double dDataY_Opt[N];


	for (int i = 0; i<N; i++)
	{
		dDataX_up[i] = i * 23.36 / N;
		dDataX_down[i] = i * 23.36 / N;
		dDataX_Opt[i] = i * 23.36 / N;
	}
	for (int i = 0; i<N; i++)
	{
		dDataY_up[i] = para_up[0] * dDataX_up[i] * dDataX_up[i] + para_up[1] * dDataX_up[i] + para_up[2];
		dDataY_down[i] = para_down[0] * dDataX_down[i] * dDataX_down[i] + para_down[1] * dDataX_down[i] + para_down[2];
		dDataY_Opt[i] = para_opt[0] * dDataX_Opt[i] * dDataX_Opt[i] + para_opt[1] * dDataX_Opt[1] + para_opt[2];
	}
	CSeries lineSeries = (CSeries)m_Chart_Preheat.Series(0);//上部炉温实测
	CSeries lineSeries2 = (CSeries)m_Chart_Preheat.Series(1);//下部炉温实测
	CSeries lineSeries3 = (CSeries)m_Chart_Preheat.Series(2);// 预设定炉温曲线

	//画面不随滚轮的变化而变化
	CEnvironment enviro = m_Chart_Preheat.get_Environment();
	enviro.put_MouseWheelScroll(FALSE);

	lineSeries.Clear();//在最前面加上ClearAllSeries(ClearAllSeries是自己写的函数)就不用了  
	lineSeries2.Clear();
	lineSeries3.Clear();

	for (int i = 0; i<N; i++)
	{
		lineSeries.AddXY(dDataX_up[i], dDataY_up[i], NULL, RGB(0, 0, 255));
		lineSeries2.AddXY(dDataX_down[i], dDataY_down[i], NULL, RGB(34, 139, 34));
		lineSeries3.AddXY(dDataX_Opt[i], dDataY_Opt[i], NULL, RGB(34, 139, 34));
	}

	
	double dX[N];
	double dY[N];
	

	int count = 0;
	struct btListNode * btListCopy = pHead;

	if (iCount != 0)
	{
		while (btListCopy != NULL)
		{

			dX[count] = btListCopy->btinfo.site;
			dY[count] = btListCopy->btinfo.btTempDistri[0];
			count++;
			btListCopy = btListCopy->pNext;

		}
		CString bt_num;
		bt_num.Format(_T("%d"), iCount);
		GetDlgItem(1031)->SetWindowTextW(bt_num);

		//CString optthread;
		//optthread.Format(_T("%d"), optState);
		//GetDlgItem(1027)->SetWindowTextW(optthread);

		//CString site;
		//site.Format(_T("%lf"),dY[0]);
		//GetDlgItem(1033)->SetWindowTextW(site);



		CSeries barSeries = (CSeries)bar_chart.Series(0);
		CEnvironment enviro = bar_chart.get_Environment();
		//画面不随滚轮的变化而变化
		enviro.put_MouseWheelScroll(FALSE);
		((CBarSeries)(barSeries.get_asBar())).put_BarWidth(7);
		barSeries.Clear();//在最前面加上ClearAllSeries(ClearAllSeries是自己写的函数)就不用了

		for (int i = 0; i<count; i++)
		{
			if (dY[i] < 800)
			{
				barSeries.AddXY(dX[i], dY[i], NULL, RGB(8, 46, 84));
			}
			else if (dY[i] < 850)
			{
				barSeries.AddXY(dX[i], dY[i], NULL, RGB(273, 23, 73));
			}
			else if (dY[i] < 900)
			{
				barSeries.AddXY(dX[i], dY[i], NULL, RGB(255, 153, 18));
			}
			else if (dY[i] < 1000)
			{
				barSeries.AddXY(dX[i], dY[i], NULL, RGB(255, 128, 0));
			}
			else if (dY[i] < 1100)
			{
				barSeries.AddXY(dX[i], dY[i], NULL, RGB(255, 0, 0));
			}
			else
			{
				barSeries.AddXY(dX[i], dY[i], NULL, RGB(255, 0, 0));
			}
		
		}

	}

} 


btListNode *find(btListNode *phead, int ValueIndex)
{
	btListNode *p1 = phead;
	btListNode *p2 = phead;

	for (int i = 0; i < ValueIndex; ++i)
	{
		if (p1->pNext == NULL)
			return NULL;
		p1 = p1->pNext;
	}
	while (p1->pNext != NULL)
	{
		p1 = p1->pNext;
		p2 = p2->pNext;
	}
	return p2;

}


double ShowOutTemp(struct btListNode *pTemp, double *btTemp, double *prebtTemp)
{
	double site;
	int leftTime;
	double optv;
	int N;
	double para[3];				//拟合炉温二次函数系数

	optv = steplength / stepcycle;
	site = pTemp->btinfo.site;
	leftTime = (int)((furnaceSize[0] - site) / optv / 2);

	N = pTemp->btinfo.dzCount;


	for (int k = 0; k < N; k++)
	{
		btTemp[k] = pTemp->btinfo.btTempDistri[k];
		prebtTemp[k] = pTemp->btinfo.btTempDistri[k];
	}

	polyfit(sizeof(thermocouple) / sizeof(thermocouple[0]), thermocouple, furTemp, 2, para);//拟合系数

	if (leftTime < 1)
	{
		return btTemp[0];
	}
	else
	{

		for (int n = 1; n < leftTime; n++)
		{
			
			site += optv * 2;

			double temp = para[0] * site * site + para[1] * site + para[2];
			double eco2 = 7.17 * pow((0.13*5.56), (1 / 3.0)) / sqrt(temp);
			double eh2o = 7.17 * 0.11 * 5.56 * pow(5.56, 0.6) / temp;
			double airBlackness = eco2 + 1.05 * eh2o;


			double flux_up;	//上部热流密度
			double flux_down; // 下部热流

			// 上部热流
			flux_up = (Stefan_Boltzmann * pTemp->btinfo.btpysicalinfo.btBlackness * airBlackness*(1 + pTemp->btinfo.btpysicalinfo.btAngle*(1 - airBlackness))*(pow((temp + 273), 4) - pow((prebtTemp[0] + 273), 4))) /
				(airBlackness + pTemp->btinfo.btpysicalinfo.btAngle* (1 - airBlackness)*(pTemp->btinfo.btpysicalinfo.btBlackness + (1 - pTemp->btinfo.btpysicalinfo.btBlackness) * airBlackness));
			// 下部热流
			flux_down = (Stefan_Boltzmann*pTemp->btinfo.btpysicalinfo.btBlackness*airBlackness*(1 + pTemp->btinfo.btpysicalinfo.btAngle*(1 - airBlackness))*(pow((temp + 273), 4) - pow((prebtTemp[N - 1] + 273), 4))) /
				(airBlackness + pTemp->btinfo.btpysicalinfo.btAngle * (1 - airBlackness)*(pTemp->btinfo.btpysicalinfo.btBlackness + (1 - pTemp->btinfo.btpysicalinfo.btBlackness) * airBlackness));
			// 上表面温度
			btTemp[0] = prebtTemp[0] + dt * flux_up / (pTemp->btinfo.btpysicalinfo.btDensity*pTemp->btinfo.btpysicalinfo.btSpecificHeat * (dz / 1000.0)) +
				pTemp->btinfo.btpysicalinfo.btConductivity * dt * (prebtTemp[1] - prebtTemp[0]) / (pTemp->btinfo.btpysicalinfo.btDensity*pTemp->btinfo.btpysicalinfo.btSpecificHeat * (dz / 1000.0) * (dz / 1000.0));
			// 下表面温度
			btTemp[N - 1] = prebtTemp[N - 1] + dt * flux_down / (pTemp->btinfo.btpysicalinfo.btDensity*pTemp->btinfo.btpysicalinfo.btSpecificHeat * (dz / 1000.0))
				+ pTemp->btinfo.btpysicalinfo.btConductivity * dt *(prebtTemp[N - 2] - prebtTemp[N - 1]) / (pTemp->btinfo.btpysicalinfo.btDensity * pTemp->btinfo.btpysicalinfo.btSpecificHeat*(dz / 1000.0)*(dz / 1000.0));

			// 内部温度分布
			for (int m = 1; m < N - 1; m++)
			{
				btTemp[m] = prebtTemp[m] + pTemp->btinfo.btpysicalinfo.btConductivity * dt *(prebtTemp[m - 1] + prebtTemp[m + 1] - 2 * prebtTemp[m]) /
					(pTemp->btinfo.btpysicalinfo.btDensity*pTemp->btinfo.btpysicalinfo.btSpecificHeat*(dz / 1000.0)*(dz / 1000.0));
			}

			for (int l = 0; l < N; l++)
			{
				prebtTemp[l] = btTemp[l];
			}
		}

		return btTemp[0];
	}

}

void CRemoteOPCClientDlg::OnClickSeriesTchart2(long SeriesIndex, long ValueIndex, long Button, long Shift, long X, long Y)
{
	// TODO:  在此处添加消息处理程序代码
	// TODO:  在此处添加消息处理程序代码
	CSeries barSeries1 = (CSeries)m_Chart_Preheat.Series(1);

	double outTemp = 0;		//出炉温度
	LPDISPATCH  Xvalue = barSeries1.get_YValues();

	int i = barSeries1.Clicked(X, Y);
	if (Button == 1)
	{
		struct btListNode * btListCopy = pHead;
		struct btListNode * btinfomation = NULL;
		struct btListNode * showBtinfomation = NULL;


		//关键区代码 复制节点温度
		EnterCriticalSection(&crtSec);
		showBtinfomation = find(pHead, ValueIndex);
		
		//malloc需要释放

		int N = showBtinfomation->btinfo.dzCount;

		double *btTemp = (double*)malloc(sizeof(double) * N);
		double *prebtTemp = (double*)malloc(sizeof(double) * N);

		btinfomation = listCopy(showBtinfomation);

		LeaveCriticalSection(&crtSec);

		//获得出炉温度
		outTemp = ShowOutTemp(btinfomation, btTemp, prebtTemp);

		free(btTemp);
		free(prebtTemp);

		if (m_information == NULL){//判定对话框是否有所指向

			m_information = new InformationDlg();//指向一个非模对话框示例

			m_information->Create(IDD_DIALOG1, this);//创建

		}
		m_information->ShowWindow(SW_SHOW);//显示


		CString bt_inittemp;	// 钢坯入炉温度
		CString bt_downtemp;	// 钢坯下表面温度
		CString bt_uptemp;		// 钢坯上表面温度
		CString bt_site;		// 钢坯当前位置
		CString bt_finallytemp;	// 钢坯出炉目标温度
		CString bt_class;		// 钢坯种类
		CString bt_hight;		// 钢坯厚预测度
		CString bt_outTemp;		//出炉温度
		

		// 钢坯入炉温度
		bt_inittemp.Format(_T("%.2lf"), btinfomation->btinfo.btTempInit);
		m_information->SetDlgItemTextW(IDC_EDIT2, bt_inittemp);
		//预测出炉温度
		bt_outTemp.Format(_T("%.2lf"), outTemp);
		m_information->SetDlgItemTextW(IDC_EDIT8, bt_outTemp);


		//钢坯上表面温度
		bt_uptemp.Format(_T("%.2lf"), btinfomation->btinfo.btTempDistri[0]);
		m_information->SetDlgItemTextW(IDC_EDIT1, bt_uptemp);
		//钢坯下表面温度
		bt_downtemp.Format(_T("%.2lf"), btinfomation->btinfo.btTempDistri[btinfomation->btinfo.dzCount - 1]);
		m_information->SetDlgItemTextW(IDC_EDIT7, bt_downtemp);
		//钢坯当前位置
		bt_site.Format(_T("%.2lf"), btinfomation->btinfo.site);
		m_information->SetDlgItemTextW(IDC_EDIT4, bt_site);
		////钢坯出炉目标温度
		bt_finallytemp.Format(_T("%.2lf"), btinfomation->btinfo.bttarget);
		m_information->SetDlgItemTextW(IDC_EDIT3, bt_finallytemp);
		//钢坯种类	
		//显示钢种
		HWND aa = ::GetActiveWindow();
		::SetDlgItemTextA(aa, IDC_EDIT5, btInfoInit.btpysicalinfo.btType);
		//钢坯厚度
		bt_hight.Format(_T("%.2lf"), btinfomation->btinfo.btpysicalinfo.btHeight);
		m_information->SetDlgItemTextW(IDC_EDIT6, bt_hight);

		double *AY = new double[N];
		double *AX = new double[N];

		CSeries lineSeries = (CSeries)m_information->m_infor.Series(0);
		lineSeries.Clear();//在最前面加上ClearAllSeries(ClearAllSeries是自己写的函数)就不用了

		for (int i = 0; i<N; i++)
		{
			AX[i] = i * btinfomation->btinfo.btpysicalinfo.btHeight / N;
			AY[i] = btinfomation->btinfo.btTempDistri[i];
		}
		for (int j = 0; j < N; j++)
		{
			lineSeries.AddXY(AX[j], AY[j], NULL, 0);
		}

		freeOptSize(btinfomation);	//释放内存
		free(AX);
		free(AY);
		
	}
}


void CRemoteOPCClientDlg::OnBnClickedButton1()
{
	// TODO:  在此添加控件通知处理程序
	exit(0);
}