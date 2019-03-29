
// RemoteOPCClientDlg.h : ͷ�ļ�
//

#pragma once
#include "CDKvTcpipClientOcx.h"
#include "tchart1.h"
#include "VariableDef.h"
#include "InformationDlg.h"
#include "afxwin.h"

// CRemoteOPCClientDlg �Ի���
class CRemoteOPCClientDlg : public CDialogEx
{
// ����
public:
	CRemoteOPCClientDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_REMOTEOPCCLIENT_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
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
	
	//�Զ�����Ϣ
protected:
	afx_msg LRESULT OnMyMessage(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnGetOPCData(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSetOPCData(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSetThreadState(WPARAM wParam, LPARAM lParam);

	//�Զ������
public:
	//��־λ
	bool loginFlag = false;			//��¼״̬��־λ
	bool calculateComplete = false;	//����������ɱ�־λ
	bool inputFlag = 0;				//����Ͷ���־λ
	int countTime = 0;				//����������

	clock_t begin;
	clock_t end;


	//ʵ��¯�£�0-8����Ԥ����¯�£����ȶ����϶������ȶ������������ȶ����¶������ȶ�������������һ���϶�������һ������������һ���¶�������һ�����������ȶ��϶������ȶ����������ȶ��¶������ȶ�����)
	float furTempMeasured[TEMPCNT];

	//�߳̾��
	HANDLE calThread = NULL;	//¯�·ֲ������߳̾��
	HANDLE optThread = NULL;	//¯���Ż��߳̾��

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


	// //�Զ���¼��̬��������
	InformationDlg *m_information;
	CTchart1 bar_chart;
	void OnClickSeriesTchart2(long SeriesIndex, long ValueIndex, long Button, long Shift, long X, long Y);
	afx_msg void OnBnClickedButton1();
	CCheckListBox m_CheckList;
	CMFCButton m_ButtonBtThread;
	CMFCButton m_ButtonOptThread;
};
