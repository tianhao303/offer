#ifndef VARIABLEDEF_H_
#define VARIABLEDEF_H_

#define dz 5	//��Ȼ�����5mmΪ���
#define dt 2	//ʱ����Ϊÿ2s����һ��
#define TEMPCNT 13	//ʵ��¯�¸���
#define round(x) ((int)(x+0.5))	//��������
#define NUM 4
#define LAYERS 3

#define IDC_TIMER_DISPLAY 1		//��ʾ��ʱ����������ʾ���ݣ�ÿ��3��ִ��һ��
#define IDC_TIMER_DISPTIME 2		//������ʾϵͳʱ�䶨ʱ����ÿ��1��ִ��һ��

//�Զ�����Ϣ����RemoteOPCClientDlg.cpp��FurTempThrea.cpp��ʹ��
#define WM_MYGETMSG			 WM_USER+105	//OnGetOPCData
#define WM_MYSETMSG			 WM_USER+106	//OnSetOPCData
#define WM_MYTHREADMSG		 WM_USER+107	//
#define WM_MYTHREADSUSPEND	 WM_USER+108
#define WM_MYTHREADRESUME	 WM_USER+109



//double  btTempInit;	//������¯�¶�
//��������ָ�꣨������¯�����¶ȣ�������¯�������²������¯�������Ŀ���²�����¶����ޣ������¶����ޣ�
struct  btTarget
{
	double  btTargetTemp;	//������¯Ŀ���¶�
	double  btSectionTemp;	//������¯�������²�
	double  btAcandTaTemp;	//������¯�����¶���Ŀ���¶�����²�
	double  tempUplimit[3];	//���������ƶȣ�¯���趨����
	double  tempDownLimit[3];	//���������ƶȣ�¯���趨����
};
//����������Ϣ���������ȡ�������ȡ��������ࡢ�����ܶȡ����������ݡ���������ϵ�����Ƕ�ϵ���������ڶȣ�
struct  btPysicalInfo
{
	double  btLength;			//��������
	double  btWidth;			//�������
	double  btHeight;			//�������
	double  btDensity;			//�����ܶ�
	double  btSpecificHeat;		//����������
	double  btConductivity;		//��������ϵ��
	double  btBlackness;		//�����ڶ�
	char  btType[50];			//��������
	double  btAngle;			//�Ƕ�ϵ��
};


//������Ϣ��������¯�¶ȣ���������ָ�꣬����������Ϣ������λ�ã������¶ȷֲ���
struct btInfo
{
	double btTempInit;				//������¯�¶�
	double btTempOut;				//������¯�¶�
	struct btTarget bttarget;		//��������ָ��
	struct btPysicalInfo btpysicalinfo;//����������Ϣ

	double site;				//������ǰλ��
	int dzCount;				//�����ֲ���
	double flux_down;		//�������ܶ�
	double flux_up;			//�������ܶ�
	double sum;				// ���������¶��ܺ�
	double *btTempDistri;//ָ��ǰʱ���¶ȷֲ������ָ��
};


//����
typedef struct Queue{
	int front;
	int rear;
	float *TEMP;

}QUEUE;

//��������ڵ�

struct  btListNode
{
	struct  btInfo btinfo;		//������Ϣ
	struct  btListNode  *pNext;		//ָ��������һ�ڵ�
};

//����ȫ�ֱ��������̹߳��ñ���

extern double furnaceSize[3];	//����¯�ߴ磬�����
extern double thermocouple[4];	//�ȵ�żλ�ã�����¯�����

extern const double Stefan_Boltzmann;	//˹߯��-������������,������·ֲ�ʱ��
extern double furTempSet[3];		//¯��Ԥ�趨ֵ
extern double steplength;	//��������������
extern int  stepcycle;		//��������
extern double v;			//������¯�������ٶ�
extern int iCount;				//�����нڵ����������������

extern char btTypeChoose[40];	//������д�ĸ�������

extern struct btInfo btInfoInit;	//����һ��������Ϣ�Ľṹ����󣬴洢��¯������Ϣ
extern struct btListNode *pHead;	//����ͷ
extern struct btListNode *NewbtList;	// �Ż��ṹ��

extern CRITICAL_SECTION crtSec;		//�ٽ�������

//FurTempOptThread.cpp��BtTempCalThread.cpp����
extern HANDLE smphPrint; // �ź���,��ʾ�Ƿ��Ѿ��ﵽ���������߳���

//RemoteOPCClientDlg.cpp����
extern int connectResponse;

//RemoteOPCClientDlg.cpp��BtTempCalThread.cpp����
extern bool btInSignal;			//������¯�ź�
extern bool btOutSignal;		//������¯�ź�
extern bool initterminalFlag;	//��¯��ʼ��������־λ
extern double furTemp[4];		////12��ʵ��¯�£�һ��ֻ��һ����ת��Ϊ����¯��
extern double botTemp[4];	//����¯�²�¯�·ֲ�
extern HANDLE hMainTimerCal;	//��ʱ������

extern bool optState;
extern bool optRelease;		// �Ż��߳�����ռ��Ƿ��ͷű�־

//�������ú���

//��Ϻ���(n:x,y���鳤�ȣ�x[],�ȵ�żλ�ã�y[]��¯�£�poly_nΪ��ϴ�����p����Ϻ��ϵ�����������У���
void polyfit(int n, double x[], double y[], int poly_n, double p[]);

//���Ƶ�ǰ����
struct btListNode* listCopy(struct btListNode * pHead);

void InitFurnaceInfo();

//������Ϣ����
void init(QUEUE *Pq);
int full_quene(QUEUE *Pq);
int en_quene(QUEUE *Pq, float val);
void bianli_queue(QUEUE *Pq);
int empty_quene(QUEUE *Pq);
int out_quene(QUEUE *Pq, float *pal);

void freeOptSize(struct btListNode *btHead);	// �ͷŸ����ṹ������Ķ�̬�ڴ�
#endif