
// RemoteOPCClientDlg.h : 头文件
//

#pragma once
#include "CDKvTcpipClientOcx.h"
#include "tchart1.h"
#include "VariableDef.h"
#include "InformationDlg.h"
#include "afxwin.h"

// CRemoteOPCClientDlg 对话框
class CRemoteOPCClientDlg : public CDialogEx
{
// 构造
public:
	CRemoteOPCClientDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_REMOTEOPCCLIENT_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
public:
	
	afx_msg void OnBnClickedButtonLogin();
	CDKvTcpipClientOcx m_myKingview;
	afx_msg void OnBnClickedButtonLogout();
	DECLARE_EVENTSINK_MAP()
	void OnServercloseKvtcpipclientocxctrl1();
	void OnLoginserverokKvtcpipclientocxctrl1(unsigned long client_id);
	void OnLogoutserverokKvtcpipclientocxctrl1();
	void OnServerdisconnectKvtcpipclientocxctrl1();
	void OnVariablevaluechangedKvtcpipclientocxctrl1(unsigned short station_id, unsigned long variable_id, short variable_value_type, LPCTSTR variable_value_string, unsigned long stamp_time_hi, unsigned long stamp_time_lo,
		unsigned short stamp_quality);
	bool ReadVariableValue(int variableID, void * outVariableValue);
	bool WritevariableValue(int variableID, void * inVariableValue);
	bool SubscribeVariableValue(int variableID);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	
	//自定义消息
protected:
	afx_msg LRESULT OnMyMessage(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnGetOPCData(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSetOPCData(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSetThreadState(WPARAM wParam, LPARAM lParam);

	//自定义变量
public:
	//标志位
	bool loginFlag = false;			//登录状态标志位
	bool calculateComplete = false;	//二级计算完成标志位
	bool inputFlag = 0;				//二级投入标志位
	int countTime = 0;				//步进梁计数

	clock_t begin;
	clock_t end;


	//实测炉温（0-8）（预热上炉温，加热二段上东、加热二段上西，加热二段下东、加热二段下西；加热一段上东、加热一段上西；加热一段下东、加热一段下西；均热段上东、均热段上西；均热段下东、均热段下西)
	float furTempMeasured[TEMPCNT];

	//线程句柄
	HANDLE calThread = NULL;	//炉温分布计算线程句柄
	HANDLE optThread = NULL;	//炉温优化线程句柄

	//TeeChart
public:
	
	afx_msg void OnEnChangeEdit12();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	CBrush m_brush;
	CTchart1 m_Chart_Preheat;
	void ClearAllSeries();
	void Draw();
	void Show();
	void System_time();


	// //自动登录组态王服务器
	InformationDlg *m_information;
	CTchart1 bar_chart;
	void OnClickSeriesTchart2(long SeriesIndex, long ValueIndex, long Button, long Shift, long X, long Y);
	afx_msg void OnBnClickedButton1();
	CCheckListBox m_CheckList;
	CMFCButton m_ButtonBtThread;
	CMFCButton m_ButtonOptThread;
};
