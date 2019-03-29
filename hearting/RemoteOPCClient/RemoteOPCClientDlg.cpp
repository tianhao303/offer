// RemoteOPCClientDlg.cpp : ʵ���ļ�

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

//�Զ�����Ϣ
#define WM_MYMESSAGE		 WM_USER+100	//OnMyMessage
#define WM_MYCLOSEMSG		 WM_USER+101	//opc�������ر�
#define WM_MYDISCONNECTMSG	 WM_USER+102	//opc��������
#define WM_MYLOGOUTMSG		 WM_USER+103	//�ͻ��˵ǳ�
#define WM_MYCONNECTMSG		 WM_USER+104	//���Ե�½


/////////////////////////�����ñ���/////////////////////////////////////

float readData[14];	//�洢���ļ��ж�ȡ������
bool optFurTemp = false; //�Ż����״̬

////////////// ����ģ�Ͳ���
int saveOutData = 0;
////////////////

FILE *fpSaveData = fopen("SaveData.txt","w");
FILE *fpMethanceBt = fopen("btparameter.txt","r");

FILE *fpFurnaceSize = fopen("furnacesize.txt", "r");

QUEUE qTtempIn;
QUEUE qTtempOut;
//float furnaceSize[6];
///////////////////////////////////////////////////////////////////////

////////////////////////����̨�������////////////////////////////
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

// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
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


// CRemoteOPCClientDlg �Ի���

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

	//�Զ�����Ϣӳ��
	ON_MESSAGE(WM_MYMESSAGE, &CRemoteOPCClientDlg::OnMyMessage)
	ON_MESSAGE(WM_MYGETMSG, &CRemoteOPCClientDlg::OnGetOPCData)
	ON_MESSAGE(WM_MYSETMSG, &CRemoteOPCClientDlg::OnSetOPCData)
	ON_MESSAGE(WM_MYTHREADMSG, &CRemoteOPCClientDlg::OnSetThreadState)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BUTTON1, &CRemoteOPCClientDlg::OnBnClickedButton1)
END_MESSAGE_MAP()


// CRemoteOPCClientDlg ��Ϣ�������




BOOL CRemoteOPCClientDlg::OnInitDialog()
{
	//��½��

	CDialogEx::OnInitDialog();
	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
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

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO:  �ڴ���Ӷ���ĳ�ʼ������

	//���û�ˢ��ɫ
	m_brush.CreateSolidBrush(RGB(179, 179, 179));

	//ȫ��
	ModifyStyle(WS_CAPTION, 0, 0);
	SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE);

	//// ���ü����̺߳��Ż��߳���ɫ��ʾ ////
	CWnd *cSeBkBtnMy = GetDlgItem(1059);
	cSeBkBtnMy->EnableWindow(false);

	CWnd *cSeBkBtnMy1 = GetDlgItem(1061);
	cSeBkBtnMy1->EnableWindow(false);

	m_ButtonBtThread.m_bTransparent = FALSE;
	m_ButtonBtThread.m_bDontUseWinXPTheme = TRUE; //��������������Ϊ���������������ʾ
	m_ButtonBtThread.SetFaceColor(RGB(255, 0, 0), true);


	m_ButtonOptThread.m_bTransparent = FALSE;
	m_ButtonOptThread.m_bDontUseWinXPTheme = TRUE; //��������������Ϊ���������������ʾ
	m_ButtonOptThread.SetFaceColor(RGB(255, 0, 0), true);

	////////////////////////////////���ÿ���̨����///////////////////////////////////
	InitConsoleWindow();  // add  
	///////////////////////////////�ؼ��γ�ʼ��//////////////////////////////////////
	InitializeCriticalSection(&crtSec);


	///////////////////////////////¯����Ϣ��ʼ��//////////////////////////////////////
	InitFurnaceInfo();

	init(&qTtempIn);	//��ʼ��������¯�¶�
	init(&qTtempOut);	//��ʼ�����г�¯�¶�
	//������ʾϵͳʱ��
	SetTimer(IDC_TIMER_DISPTIME, 1000, NULL);         //������ʱ��
	
	//���ø�ѡ��

	//((CButton*)GetDlgItem(1057))->SetCheck(TRUE);

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
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
*@details:		��̬��OPC�������رգ��������¼�
*@param[in]		��
*@param[out]	��
*@retval:		void
*/
void CRemoteOPCClientDlg::OnServercloseKvtcpipclientocxctrl1()
{
	// TODO:  �ڴ˴������Ϣ����������

	//�ر���ʾ��ʱ��IDC_TIMER_DISPLAY
	KillTimer(IDC_TIMER_DISPLAY);
	KillTimer(IDC_TIMER_DISPTIME);

	//�رռ����߳�
	if (NULL != this->calThread)
	{
		::TerminateThread(calThread, 0);		//���������߳�
		::CloseHandle(hMainTimerCal);			//�رռ����߳��ں˶�ʱ��
		calThread = NULL;
		//		GetDlgItem(IDC_COMSTAT)->SetWindowText("CLOSED!");
	}
	if (NULL != this->optThread)
	{
		::TerminateThread(optThread, 0);		//�����Ż��߳�
		optThread = NULL;
	}
	//logout the opc server
	if (TRUE == this->loginFlag)
	{
		//		this->m_KingViewCtrl.Method_LogoutServer();
		this->loginFlag = FALSE;
	}

	ReleaseSemaphore(smphPrint, 1, NULL);

	//������̬���������رյ���Ϣ
	PostMessage(WM_MYMESSAGE, WM_MYCLOSEMSG, NULL);
}


/**
*@brief:		CRemoteOPCClientDlg.OnServercloseKvtcpipclientocxctrl1
*@details:		�ɹ���½��̬��OPC���������������¼�
*@param[in]		��
*@param[out]	��
*@retval:		void
*/
void CRemoteOPCClientDlg::OnLoginserverokKvtcpipclientocxctrl1(unsigned long client_id)
{
	// TODO:  �ڴ˴������Ϣ����������
}


/**
*@brief:		CRemoteOPCClientDlg.OnServercloseKvtcpipclientocxctrl1
*@details:		�ǳ���̬��OPC���������������¼�
*@param[in]		��
*@param[out]	��
*@retval:		void
*/
void CRemoteOPCClientDlg::OnLogoutserverokKvtcpipclientocxctrl1()
{
	// TODO:  �ڴ˴������Ϣ����������

	//�ر���ʾ��ʱ��IDC_TIMER_DISPLAY
	KillTimer(IDC_TIMER_DISPLAY);
	//ϵͳʱ����ʾ��ʱ��
	KillTimer(IDC_TIMER_DISPTIME);
//	SetTimer(IDC_TIMER_LOGIN, 1000, NULL);
	//�رռ����߳�
	if (NULL != this->calThread)
	{
		::TerminateThread(calThread, 0);		//���������߳�
		::CloseHandle(hMainTimerCal);			//�رռ����߳��ں˶�ʱ��
		calThread = NULL;
//		GetDlgItem(IDC_COMSTAT)->SetWindowText("CLOSED!");
	}
	if (NULL != this->optThread)
	{
		::TerminateThread(optThread, 0);		//�����Ż��߳�
		optThread = NULL;
	}

	//update the connect flag
	if (loginFlag == true)
	{
		loginFlag = FALSE;

		//���͵ǳ���̬������������Ϣ
		PostMessage(WM_MYMESSAGE, WM_MYLOGOUTMSG, NULL);
	}

	ReleaseSemaphore(smphPrint, 1, NULL);
	
}

/**
*@brief:		CRemoteOPCClientDlg.OnServercloseKvtcpipclientocxctrl1
*@details:		����̬��OPC���������ӳ������⣬�������¼�
*@param[in]		��
*@param[out]	��
*@retval:		void
*/
void CRemoteOPCClientDlg::OnServerdisconnectKvtcpipclientocxctrl1()
{
	// TODO:  �ڴ˴������Ϣ����������

	//�ر���ʾ��ʱ��IDC_TIMER_DISPLAY
	KillTimer(IDC_TIMER_DISPLAY);

	//�رռ����߳�
	if (NULL != this->calThread)
	{
		//freeOptSize(pHead);						//�ͷż����߳�����Ŀռ�
		//MessageBox(_T("�ͷ��߳���Դ"));
		::TerminateThread(calThread, 0);		//���������߳�
		::CloseHandle(hMainTimerCal);			//�رռ����߳��ں˶�ʱ��
		calThread = NULL;
		//		GetDlgItem(IDC_COMSTAT)->SetWindowText("CLOSED!");
	}
	if (NULL != this->optThread)
	{
												//�ж��Ż��߳��ڴ��Ƿ���Ҫ�ͷ�
		::TerminateThread(optThread, 0);		//�����Ż��߳�
		optThread = NULL;
	}
	//logout the opc server
	if (TRUE == this->loginFlag)
	{
//		this->m_KingViewCtrl.Method_LogoutServer();
		this->loginFlag = FALSE;
	}
	//��������̬�����ӶϿ�����Ϣ
	PostMessage(WM_MYMESSAGE, WM_MYDISCONNECTMSG, NULL);

	ReleaseSemaphore(smphPrint, 1, NULL);
}


/**
*@brief:		CRemoteOPCClientDlg.OnServercloseKvtcpipclientocxctrl1
*@details:		������̬������ֵ�����仯ʱ���������¼�
*@param[in]		��
*@param[out]	��
*@retval:		void
*/
void CRemoteOPCClientDlg::OnVariablevaluechangedKvtcpipclientocxctrl1(unsigned short station_id, unsigned long variable_id, short variable_value_type, LPCTSTR variable_value_string, unsigned long stamp_time_hi, unsigned long stamp_time_lo,
	unsigned short stamp_quality)
{
	// TODO:  �ڴ˴������Ϣ����������
	printf("%d ", (int)(variable_id));
	/*****************************����Ͷ���־λ*************************************
	//������Ͷ���־λ�����ı�
	//��ȡ����Ͷ���־λ
	//������Ͷ���־λ1����
	// 38Ϊ�������ź�   31Ϊ����ź�   122Ϊ�л��ź� 32��¯
	******************************************************************************/
	if ((int)(variable_id) == 122)
	{
		ReadVariableValue(123, &inputFlag);		//���ض���Ͷ���־λ
	
		if (inputFlag == 1)
		{
			
			//����¯���Ż��߳�
			if (NULL == this->optThread)
			{
				calculateComplete = 0;		//����������ɱ�־λ��0
				this->optThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)FurTempOpt, this->m_hWnd, 0, NULL);
				optState = true;
				SetThreadPriority(optThread, THREAD_PRIORITY_LOWEST);
			}

		}
		else
		{

			calculateComplete = 0;
			Sleep(4000);
			WritevariableValue(126, &calculateComplete);		//д�ض���Ͷ���־λ
			m_ButtonOptThread.SetFaceColor(RGB(255, 0, 0), true);

			if (NULL != this->optThread)
			{
				if (optRelease)
				{
					freeOptSize(NewbtList);
				}
				::TerminateThread(optThread, 0);		//�����Ż��߳�
				optThread = NULL;
				optState = false;
			}

		}
	}

	//��¯�ź�
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

	/*****************************��¯�źű�־λ*************************************
	//����¯�źű�־λ�����ı�
	//��ȡ��¯�źű�־λ
	//�費��Ҫ����
	//�費��Ҫд��
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
	
	// �����������ź�
	
	if ((int)(variable_id) == 38)
	{

		bool workingBeamSignal = false;
		ReadVariableValue(39, &workingBeamSignal);

		CString worinIng;
		worinIng.Format(_T("%d"), workingBeamSignal);
		GetDlgItem(1033)->SetWindowTextW(worinIng);

		/*****************************��¯�źű�־λ���������ź�*************************************
		//����¯�źű�־λ�����ı�
		//��ȡ��¯�źű�־λ

		******************************************************************************/
		///��ȡ����������	
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
				printf("����������������%d\n", stepcycle);
				countTime = 0;
			}
		}
	}

}

/**
*@brief:		CRemoteOPCClientDlg.OnBnClickedButtonLogin
*@details:		��¼��ť,������̬��OPC������,����ͨѶ��ʱ���Լ�����Ԥ���̺߳�¯���Ż��߳�
*@param[in]		��
*@param[out]	��
*@retval:		void
*/
void CRemoteOPCClientDlg::OnBnClickedButtonLogin()
{
	//if (loginFlag == false)
	//{
	//	loginFlag = true;
	//	//����¯��Ԥ�趨�����߳�
	//	if (NULL == this->calThread)
	//	{
	//		this->calThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)BtTempCal, this->m_hWnd, 0, NULL);
	//		SetThreadPriority(calThread, THREAD_PRIORITY_BELOW_NORMAL);
	//	}
	//
	//	//����¯���Ż��߳�
	//	/*if (NULL == this->optThread)
	//	{
	//		this->optThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)FurTempOpt, this->m_hWnd, 0, NULL);
	//		SetThreadPriority(optThread, THREAD_PRIORITY_LOWEST);
	//		optState = true;
	//	}*/

	//	//��ʱ������ʾ����
	//	SetTimer(IDC_TIMER_DISPLAY, 3000, NULL);
	//}
	//else
	//{
	//	AfxMessageBox(_T("������̬���������ӣ�"));
	//}

	if (false == loginFlag)
	{
		//��¼��̬��
		short result = m_myKingview.Method_LoginServer(_T("192.168.43.208"), 41190, _T("admin"), _T("123456"));
		::Sleep(1000);
		if (result == 2 || result == 4)
		{
			loginFlag = true;			//��¼״̬��־λ��1

			//������ʾ��ʱ��
			SetTimer(IDC_TIMER_DISPLAY, 3000, NULL);

			//����¯��Ԥ�趨�����߳�
			if (NULL == this->calThread)
			{
				this->calThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)BtTempCal, this->m_hWnd, 0, NULL);
				SetThreadPriority(calThread, THREAD_PRIORITY_BELOW_NORMAL);
				m_ButtonBtThread.SetFaceColor(RGB(0, 255, 0), true);
			}
			
			//���ı���
			SubscribeVariableValue(30); //���Ķ���Ͷ���־λinputFlag
			SubscribeVariableValue(39); //#�������ź�
			SubscribeVariableValue(32); //������¯�ź�btInSignal
			SubscribeVariableValue(33); //���ĳ�¯�ź�btInSignal
			

			//�鿴��ʱ�Ƿ��ж���Ͷ��
			ReadVariableValue(123, &inputFlag);
	
			if (inputFlag == 1)
			{
				calculateComplete = 0;		//�����һ���Ż�������ɿ�ʼ����¯���趨ֵ	
				//����¯���Ż��߳�
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
		AfxMessageBox(_T("������̬����������"));
	}
}


/**
*@brief:		CRemoteOPCClientDlg.OnBnClickedButtonLogout
*@details:		�ǳ���ť,�Ͽ���̬��OPC������,�ر�ͨѶ��ʱ���Լ�����Ԥ���̺߳�¯���Ż��߳�
*@param[in]		��
*@param[out]	��
*@retval:		void
*/
void CRemoteOPCClientDlg::OnBnClickedButtonLogout()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	if (loginFlag == true)
	{
		m_myKingview.Method_LogoutServer();
		//m_ButtonBtThread.SetFaceColor(RGB(255, 0, 0), true);//1��14��
	}
	else
	{
		AfxMessageBox(_T("δ��¼��̬����������"));
	}

}
////////////////������ʾϵͳʱ��  1s 1��
///////////////////12��22���޸�

void CRemoteOPCClientDlg::System_time()
{
	CString strTime;
	CTime tm;
	tm = CTime::GetCurrentTime();
	strTime = tm.Format("%Y��%m��%d�� %H:%M:%S");
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
	// TODO:  �ڴ������Ϣ�����������/�����Ĭ��ֵ
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
*@details:		ͨ������ID�ţ���ȡ��̬��OPC�������ж�Ӧ������ֵ
*@param[in]		int variableID: variable�ı�ʶ�ţ���̬�������ݴʵ��ڵ�ID
*@param[out]	void *outVariableValue  :��ȡ���Ķ�Ӧvariable��ֵ
*@retval:		bool
*/
bool CRemoteOPCClientDlg::ReadVariableValue(int variableID, void * outVariableValue)
{
	unsigned short stationNumber = 0;	//վ����
	short variableType = -1;		//�������ͣ�3�����ͣ�4��ʵ�ͣ�11����ɢ�ͣ�16400���ַ�����
	BSTR variableStr = ::SysAllocStringLen(NULL, 50);	//����������ռ�
	//��ȡվ������
	m_myKingview.Method_GetStationNumber(&stationNumber);
	//��ȡID��ΪvariableID���������ͺ�����ֵ���Ӧ���ַ���
	m_myKingview.Method_GetVariableValueByVariableId(stationNumber - 1, variableID, &variableType, &variableStr);

	CString variableTemp;	//��ʱ�����ַ���
	variableTemp.Empty();
	//��BSTR���ַ�������CString��
	variableTemp = variableStr;

	//���ݶ�ȡ�ı������ͣ�������ת��Ϊ��Ӧ����ֵ
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

	//�ͷ�BSTR����Ŀռ�
	SysFreeString(variableStr);
	
	return false;
}

/**
*@brief: 		CRemoteOPCClientDlg.WritevariableValue
*@details:		��������ֵ���ݱ�����ID��д����̬��OPC������
*@param[in]		int variableID: variable�ı�ʶ�ţ���̬�������ݴʵ��ڵ�ID
				void *inVariableValue: the data pointer that you wanna write
*@param[out]	��
*@retval:		bool
*/
bool CRemoteOPCClientDlg::WritevariableValue(int variableID, void * inVariableValue)
{
	unsigned short stationNumber = 0;	//վ����
	short variableType = -1;		//�������ͣ�3�����ͣ�4��ʵ�ͣ�11����ɢ�ͣ�16400���ַ�����
	BSTR variableStr = ::SysAllocStringLen(NULL, 50);	//����������ռ�
	//��ȡվ������
	m_myKingview.Method_GetStationNumber(&stationNumber);
	//��ȡID��ΪvariableID����������
	m_myKingview.Method_GetVariableValueByVariableId(stationNumber - 1, variableID, &variableType, &variableStr);

	CString variableTemp;
	variableTemp.Empty();

	//���ݶ�ȡ���������ͣ�ת�����ַ�����
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
	//���ݱ���ID������̬��д�ر���ֵ
	m_myKingview.Method_SetVariableValueByVariableId(stationNumber - 1, variableID, variableTemp);
	//�ͷ�����Ŀռ�
	SysFreeString(variableStr);
	
	return false;
}


/**
*@brief: 		CRemoteOPCClientDlg.OnMyMessage
*@details:		��Բ�ͬ������̬��������״����������Ӧ�ı���
*@param[in]		WPARAM wParam: ��Ϣ���������ݴ˲����жϽ�����Ӧ�ı���
				LPARAM lParam: ��Ϣ������δ��
*@param[out]	��
*@retval:		LRESULT
*/
afx_msg LRESULT CRemoteOPCClientDlg::OnMyMessage(WPARAM wParam, LPARAM lParam)
{
	// һ�������г�ʱ��������ζ����� 
	switch (wParam)
	{
	case WM_MYCLOSEMSG:
		AfxMessageBox(_T("OPC�������ѹر�"));
		break;
	case WM_MYDISCONNECTMSG:
		AfxMessageBox(_T("��OPC���������ӳ������⣬����������"));
		break;
	case WM_MYLOGOUTMSG:
		AfxMessageBox(_T("OPC�ͻ����ѵǳ�"));
		break;
	case WM_MYCONNECTMSG:	
		if (0 == connectResponse)
		{
			connectResponse++;
			AfxMessageBox(_T("��������̬����������ʧ��"));
			connectResponse = 0;
		}	
		break;
	default:
		break;
	}

	return 0;
}

//�޸� 2018-11-29/////
// th	��Ӷ���
void ReadInitTemp(float btInitTemp)
{
	//����Ӵ��룬�жϸ�����������������������������ݣ����������ģ������������ģ������Ĵ���300��Ϊ������С��50���϶�Ϊ������
	//���ڲ���Ҫ���
	float ppp;

	if (full_quene(&qTtempIn) != 0)	//����Ƕ���û����������м�����
	{
		en_quene(&qTtempIn, btInitTemp);	//�����,��ʼ������
	}
		else
		{
			out_quene(&qTtempIn, &ppp);	//������
			en_quene(&qTtempIn, btInitTemp);	//�����

			//////////20���޸�///////////////////
			if (abs(qTtempIn.TEMP[(qTtempIn.rear + NUM - 1) % NUM] - qTtempIn.TEMP[(qTtempIn.rear + NUM - 2) % NUM]) > 20)
			{
				btInfoInit.btTempInit = qTtempIn.TEMP[(qTtempIn.rear + NUM - 2) % NUM];
				printf("������һʱ����¯�¶�%.2f\n", qTtempIn.TEMP[(qTtempIn.rear + NUM - 2) % NUM]);
			}
			else
			{
				btInfoInit.btTempInit = (qTtempIn.TEMP[(qTtempIn.rear + NUM - 1) % NUM] + qTtempIn.TEMP[(qTtempIn.rear + NUM - 2) % NUM]) / 2;

			}
			printf("��������%.2f\n", btInitTemp);
			printf("��¯�¶�Ϊ%.2f\n", btInfoInit.btTempInit);
		}
		bianli_queue(&qTtempIn);
		printf("���µ�%.2f\n", qTtempIn.TEMP[(qTtempIn.rear + NUM - 1) % NUM]);

}

void ReadOutTemp(float btTempOut)
{
	float ppp;

		if (full_quene(&qTtempOut) != 0)	//����Ƕ���û����������м�����
		{
			en_quene(&qTtempOut, btTempOut);	//�����,��ʼ������
		}
		else
		{
			out_quene(&qTtempOut, &ppp);	//������
			en_quene(&qTtempOut, btTempOut);	//�����

			//////////20���޸�///////////////////
			if (abs(qTtempOut.TEMP[(qTtempOut.rear + NUM - 1) % NUM] - qTtempOut.TEMP[(qTtempOut.rear + NUM - 2) % NUM]) > 20)
			{
				btInfoInit.btTempOut = qTtempOut.TEMP[(qTtempOut.rear + NUM - 2) % NUM];
				printf("������һʱ�̳�¯�¶�%.2f\n", qTtempOut.TEMP[(qTtempOut.rear + NUM - 2) % NUM]);
			}
			else
			{
				btInfoInit.btTempOut = (qTtempOut.TEMP[(qTtempOut.rear + NUM - 1) % NUM] + qTtempOut.TEMP[(qTtempOut.rear + NUM - 2) % NUM]) / 2 ;

			}
			printf("��������%.2f\n", btTempOut);
			printf("��¯�¶�Ϊ%.2f\n", btInfoInit.btTempOut);
		}
		bianli_queue(&qTtempOut);
		printf("���µĳ�¯%.2f\n", qTtempOut.TEMP[(qTtempOut.rear + NUM - 1) % NUM]);

}
void SaveData()
{
	//��������2����¯�¶�3����¯�¶� 1��ʵ��¯��4������������5������������


	// ����¯д����¯�¶�
	if (btInSignal == 1)
	{
		fprintf(fpSaveData, "%d\t", btInfoInit.btTempInit);

	}
	else
	{
		fprintf(fpSaveData, "%d\t", 0);
	}
	// �г�¯д���¯�¶�
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
*@details:		ͨ���ؼ���ȡ����¯ʵʱ����
*@param[in]		WPARAM wParam: ��Ϣ������δ��
				LPARAM lParam: ��Ϣ������δ��
*@param[out]	��
*@retval:		LRESULT
*/
afx_msg LRESULT CRemoteOPCClientDlg::OnGetOPCData(WPARAM wParam, LPARAM lParam)
{

	//��ȡ¯��ʵ��ֵ,������¯�¶ȡ�������¯�¶ȡ�����������Ϣ
	//�޸�����2019��-2-18��
	//������Ϣ����¯ʱ��ȡ�ļ��������߶�ͨ����̬��
	//��ȡ�����ļ����ƶ���Ϣ
	EnterCriticalSection(&crtSec);
	printf("\n\n-------------------���߳̽����ٽ���--------------------\n\n");
	//�߳�ͬ��������ؼ���


	ReadVariableValue(29, &furTempMeasured[0]);//Ԥ�ȶ�¯��ʵ��

	ReadVariableValue(27, &furTempMeasured[1]);//�Ӷ��϶�¯��ʵ��
	ReadVariableValue(27, &furTempMeasured[2]);//�Ӷ�����¯��ʵ��

	ReadVariableValue(28, &furTempMeasured[3]);//�Ӷ��¶�¯��ʵ��
	ReadVariableValue(28, &furTempMeasured[4]);//�Ӷ�����¯��ʵ��

	ReadVariableValue(25, &furTempMeasured[5]);//��һ�϶�¯��ʵ��
	ReadVariableValue(25, &furTempMeasured[6]);//��һ����¯��ʵ��

	ReadVariableValue(26, &furTempMeasured[7]);//��һ�¶�¯��ʵ��
	ReadVariableValue(26, &furTempMeasured[8]);//��һ����¯��ʵ��

	ReadVariableValue(21, &furTempMeasured[9]);//�����϶�¯��ʵ��
	ReadVariableValue(22, &furTempMeasured[10]);//��������¯��ʵ��
	ReadVariableValue(23, &furTempMeasured[11]);//�����¶�¯��ʵ��
	ReadVariableValue(24, &furTempMeasured[12]);//��������¯��ʵ��

	for (int i = 0; i < TEMPCNT; i++)
	{
		//ReadVariableValue(21+i, &furTempMeasured[i]);
		CString tempMeasured;
		tempMeasured.Format(_T("%.2f"), furTempMeasured[i]);

		GetDlgItem(1036+i)->SetWindowTextW(tempMeasured);
	}

	//Ԥ�ȶ�¯��
	furTemp[0] = furTempMeasured[0];
	//����һ��¯���ü���һ���϶���¯�¾�ֵ
	furTemp[1] = (furTempMeasured[1] + furTempMeasured[2]) / 2;
	//���ȶ���¯���ü��ȶ����϶���¯�¾�ֵ
	furTemp[2] = (furTempMeasured[5] + furTempMeasured[6]) / 2;
	//���ȶ�¯���þ��ȶ��϶���¯�¾�ֵ
	furTemp[3] = (furTempMeasured[9] + furTempMeasured[10]) / 2;

	//Ԥ�ȶ���
	botTemp[0] = furTempMeasured[0] ;
	//���ȶ�����
	botTemp[1] = (furTempMeasured[3] + furTempMeasured[4])/2;
	//����һ����
	botTemp[2] = (furTempMeasured[7] + furTempMeasured[8])/2;
	
	// ���ȶ���
	botTemp[3] = (furTempMeasured[11] + furTempMeasured[12]) / 2;

	//��ȡ���¯�¶�
	float btInitTemp;
	float btTempOut;

	ReadVariableValue(37, &btInitTemp);	//��¯�¶�
	ReadInitTemp(btInitTemp);	//��Ӷ���-2018-2-20�޸Ķ�ȡ��¯�¶�ʵ��
	ReadVariableValue(38, &btTempOut);	//��¯�¶�
	ReadOutTemp(btTempOut);	//��Ӷ���-2018-2-20�޸Ķ�ȡ��¯�¶�ʵ��
	
	ReadVariableValue(36, &btInfoInit.btpysicalinfo.btType);	//��������

	btInfoInit.bttarget.btTargetTemp = 1100;

	//��ʾ����
	//::SetDlgItemTextA(this->m_hWnd, IDC_EDIT13, btInfoInit.btpysicalinfo.btType);

	CString Cyclea;
	Cyclea.Format(_T("%d"), stepcycle);
	GetDlgItem(1003)->SetWindowTextW(Cyclea);

	//��ʼ����ɲ��ܿ�ʼ����ģ�Ͳ����²�С��50��ʱ������ģ��

	if (initterminalFlag == true  && (pHead->btinfo.btTempDistri[0] - btInfoInit.btTempOut) > 50)
	{
		if ( saveOutData < 3)
		{
			SaveData();			//��������
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
*@details:		д�ؼ�����¯��Ԥ��ֵ
*@param[in]		WPARAM wParam: ��Ϣ������δ��
				LPARAM lParam: ��Ϣ������δ��
*@param[out]	��
*@retval:		LRESULT
*/
afx_msg LRESULT CRemoteOPCClientDlg::OnSetOPCData(WPARAM wParam, LPARAM lParam)
{
	//////////////////////////////////////////������////////////////////////////////////////
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
	
	printf("¯���趨ֵΪ�� %f\t%f\n", furTempSet[0], furTempSet[1]);
	
	return 0;
}


/**
*@brief: 		CRemoteOPCClientDlg.SubscribeVariableValue
*@details:		ͨ������ID�Ŷ�����̬���б�����ֵ�仯����ʹ�ñ���ֵ�ı�֪ͨ�¼�
*@param[in]		int variableID��variable�ı�ʶ�ţ���̬�������ݴʵ��ڵ�ID
*@param[out]	��
*@retval:		bool
*/
bool CRemoteOPCClientDlg::SubscribeVariableValue(int variableID)
{
	unsigned short stationNumber = 0;	//վ����
	printf("ddd %d\n", m_myKingview.Method_GetStationNumber(&stationNumber));	//��ȡվ����

	printf("aaa %d\n",m_myKingview.Method_SubscibeVariable(stationNumber - 1, variableID, 1));		//����variableID������ֵ�仯

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
	all_font.CreatePointFont(130, L"�����п�");

	// TODO:  �ڴ˸��� DC ���κ�����
	//�����һ���Ƿ�Ϊ�Ի�����ж����  
	if (nCtlColor == CTLCOLOR_DLG )
	{
//		pDC->SetBkMode(TRANSPARENT);
//		pDC->SetTextColor(RGB(0, 255, 0));
		return   m_brush;   //���Ӻ�ɫˢ�� 
	}
	else if (nCtlColor == CTLCOLOR_STATIC)  //���ľ�̬�ı�
	{
		pDC->SetTextColor(RGB(9, 101, 250));
		pDC->SetBkColor(RGB(179, 179, 179));
		pDC->SelectObject(&all_font);

		HBRUSH b = CreateSolidBrush(RGB(179, 179, 179));
		CFont m_font;
		if (pWnd->GetDlgCtrlID() == IDC_STATIC1)
		{
			m_font.CreatePointFont(220, L"�����п�");
			pDC->SelectObject(&m_font);
			
		}
		return b;
	}
	else if (nCtlColor == CTLCOLOR_EDIT)  //�����ı����ı�
	{
		pDC->SelectObject(&all_font);
		return hbr;
	}	 

	//TODO:  ���Ĭ�ϵĲ������軭�ʣ��򷵻���һ������
	return hbr;
}

void CRemoteOPCClientDlg::Draw()
{
	//2019��2��17���ڽ���������Ż��趨���ߡ�
	const UINT N = 200;

	double para_up[3];
	double para_down[3];
	double para_opt[3];

	polyfit(sizeof(thermocouple) / sizeof(thermocouple[0]), thermocouple, furTemp, 2, para_up);//�ϲ�¯��
	polyfit(sizeof(thermocouple) / sizeof(thermocouple[0]), thermocouple, botTemp, 2, para_down);//�²�¯��
	polyfit(sizeof(thermocouple) / sizeof(thermocouple[0]), thermocouple, furTempSet, 2, para_opt);//Ԥ���Ż�¯��

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
	CSeries lineSeries = (CSeries)m_Chart_Preheat.Series(0);//�ϲ�¯��ʵ��
	CSeries lineSeries2 = (CSeries)m_Chart_Preheat.Series(1);//�²�¯��ʵ��
	CSeries lineSeries3 = (CSeries)m_Chart_Preheat.Series(2);// Ԥ�趨¯������

	//���治����ֵı仯���仯
	CEnvironment enviro = m_Chart_Preheat.get_Environment();
	enviro.put_MouseWheelScroll(FALSE);

	lineSeries.Clear();//����ǰ�����ClearAllSeries(ClearAllSeries���Լ�д�ĺ���)�Ͳ�����  
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
		//���治����ֵı仯���仯
		enviro.put_MouseWheelScroll(FALSE);
		((CBarSeries)(barSeries.get_asBar())).put_BarWidth(7);
		barSeries.Clear();//����ǰ�����ClearAllSeries(ClearAllSeries���Լ�д�ĺ���)�Ͳ�����

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
	double para[3];				//���¯�¶��κ���ϵ��

	optv = steplength / stepcycle;
	site = pTemp->btinfo.site;
	leftTime = (int)((furnaceSize[0] - site) / optv / 2);

	N = pTemp->btinfo.dzCount;


	for (int k = 0; k < N; k++)
	{
		btTemp[k] = pTemp->btinfo.btTempDistri[k];
		prebtTemp[k] = pTemp->btinfo.btTempDistri[k];
	}

	polyfit(sizeof(thermocouple) / sizeof(thermocouple[0]), thermocouple, furTemp, 2, para);//���ϵ��

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


			double flux_up;	//�ϲ������ܶ�
			double flux_down; // �²�����

			// �ϲ�����
			flux_up = (Stefan_Boltzmann * pTemp->btinfo.btpysicalinfo.btBlackness * airBlackness*(1 + pTemp->btinfo.btpysicalinfo.btAngle*(1 - airBlackness))*(pow((temp + 273), 4) - pow((prebtTemp[0] + 273), 4))) /
				(airBlackness + pTemp->btinfo.btpysicalinfo.btAngle* (1 - airBlackness)*(pTemp->btinfo.btpysicalinfo.btBlackness + (1 - pTemp->btinfo.btpysicalinfo.btBlackness) * airBlackness));
			// �²�����
			flux_down = (Stefan_Boltzmann*pTemp->btinfo.btpysicalinfo.btBlackness*airBlackness*(1 + pTemp->btinfo.btpysicalinfo.btAngle*(1 - airBlackness))*(pow((temp + 273), 4) - pow((prebtTemp[N - 1] + 273), 4))) /
				(airBlackness + pTemp->btinfo.btpysicalinfo.btAngle * (1 - airBlackness)*(pTemp->btinfo.btpysicalinfo.btBlackness + (1 - pTemp->btinfo.btpysicalinfo.btBlackness) * airBlackness));
			// �ϱ����¶�
			btTemp[0] = prebtTemp[0] + dt * flux_up / (pTemp->btinfo.btpysicalinfo.btDensity*pTemp->btinfo.btpysicalinfo.btSpecificHeat * (dz / 1000.0)) +
				pTemp->btinfo.btpysicalinfo.btConductivity * dt * (prebtTemp[1] - prebtTemp[0]) / (pTemp->btinfo.btpysicalinfo.btDensity*pTemp->btinfo.btpysicalinfo.btSpecificHeat * (dz / 1000.0) * (dz / 1000.0));
			// �±����¶�
			btTemp[N - 1] = prebtTemp[N - 1] + dt * flux_down / (pTemp->btinfo.btpysicalinfo.btDensity*pTemp->btinfo.btpysicalinfo.btSpecificHeat * (dz / 1000.0))
				+ pTemp->btinfo.btpysicalinfo.btConductivity * dt *(prebtTemp[N - 2] - prebtTemp[N - 1]) / (pTemp->btinfo.btpysicalinfo.btDensity * pTemp->btinfo.btpysicalinfo.btSpecificHeat*(dz / 1000.0)*(dz / 1000.0));

			// �ڲ��¶ȷֲ�
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
	// TODO:  �ڴ˴������Ϣ����������
	// TODO:  �ڴ˴������Ϣ����������
	CSeries barSeries1 = (CSeries)m_Chart_Preheat.Series(1);

	double outTemp = 0;		//��¯�¶�
	LPDISPATCH  Xvalue = barSeries1.get_YValues();

	int i = barSeries1.Clicked(X, Y);
	if (Button == 1)
	{
		struct btListNode * btListCopy = pHead;
		struct btListNode * btinfomation = NULL;
		struct btListNode * showBtinfomation = NULL;


		//�ؼ������� ���ƽڵ��¶�
		EnterCriticalSection(&crtSec);
		showBtinfomation = find(pHead, ValueIndex);
		
		//malloc��Ҫ�ͷ�

		int N = showBtinfomation->btinfo.dzCount;

		double *btTemp = (double*)malloc(sizeof(double) * N);
		double *prebtTemp = (double*)malloc(sizeof(double) * N);

		btinfomation = listCopy(showBtinfomation);

		LeaveCriticalSection(&crtSec);

		//��ó�¯�¶�
		outTemp = ShowOutTemp(btinfomation, btTemp, prebtTemp);

		free(btTemp);
		free(prebtTemp);

		if (m_information == NULL){//�ж��Ի����Ƿ�����ָ��

			m_information = new InformationDlg();//ָ��һ����ģ�Ի���ʾ��

			m_information->Create(IDD_DIALOG1, this);//����

		}
		m_information->ShowWindow(SW_SHOW);//��ʾ


		CString bt_inittemp;	// ������¯�¶�
		CString bt_downtemp;	// �����±����¶�
		CString bt_uptemp;		// �����ϱ����¶�
		CString bt_site;		// ������ǰλ��
		CString bt_finallytemp;	// ������¯Ŀ���¶�
		CString bt_class;		// ��������
		CString bt_hight;		// ������Ԥ���
		CString bt_outTemp;		//��¯�¶�
		

		// ������¯�¶�
		bt_inittemp.Format(_T("%.2lf"), btinfomation->btinfo.btTempInit);
		m_information->SetDlgItemTextW(IDC_EDIT2, bt_inittemp);
		//Ԥ���¯�¶�
		bt_outTemp.Format(_T("%.2lf"), outTemp);
		m_information->SetDlgItemTextW(IDC_EDIT8, bt_outTemp);


		//�����ϱ����¶�
		bt_uptemp.Format(_T("%.2lf"), btinfomation->btinfo.btTempDistri[0]);
		m_information->SetDlgItemTextW(IDC_EDIT1, bt_uptemp);
		//�����±����¶�
		bt_downtemp.Format(_T("%.2lf"), btinfomation->btinfo.btTempDistri[btinfomation->btinfo.dzCount - 1]);
		m_information->SetDlgItemTextW(IDC_EDIT7, bt_downtemp);
		//������ǰλ��
		bt_site.Format(_T("%.2lf"), btinfomation->btinfo.site);
		m_information->SetDlgItemTextW(IDC_EDIT4, bt_site);
		////������¯Ŀ���¶�
		bt_finallytemp.Format(_T("%.2lf"), btinfomation->btinfo.bttarget);
		m_information->SetDlgItemTextW(IDC_EDIT3, bt_finallytemp);
		//��������	
		//��ʾ����
		HWND aa = ::GetActiveWindow();
		::SetDlgItemTextA(aa, IDC_EDIT5, btInfoInit.btpysicalinfo.btType);
		//�������
		bt_hight.Format(_T("%.2lf"), btinfomation->btinfo.btpysicalinfo.btHeight);
		m_information->SetDlgItemTextW(IDC_EDIT6, bt_hight);

		double *AY = new double[N];
		double *AX = new double[N];

		CSeries lineSeries = (CSeries)m_information->m_infor.Series(0);
		lineSeries.Clear();//����ǰ�����ClearAllSeries(ClearAllSeries���Լ�д�ĺ���)�Ͳ�����

		for (int i = 0; i<N; i++)
		{
			AX[i] = i * btinfomation->btinfo.btpysicalinfo.btHeight / N;
			AY[i] = btinfomation->btinfo.btTempDistri[i];
		}
		for (int j = 0; j < N; j++)
		{
			lineSeries.AddXY(AX[j], AY[j], NULL, 0);
		}

		freeOptSize(btinfomation);	//�ͷ��ڴ�
		free(AX);
		free(AY);
		
	}
}


void CRemoteOPCClientDlg::OnBnClickedButton1()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ�������
	exit(0);
}