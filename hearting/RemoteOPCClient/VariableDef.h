#ifndef VARIABLEDEF_H_
#define VARIABLEDEF_H_

#define dz 5	//厚度划分以5mm为间隔
#define dt 2	//时间间隔为每2s计算一次
#define TEMPCNT 13	//实测炉温个数
#define round(x) ((int)(x+0.5))	//四舍五入
#define NUM 4
#define LAYERS 3

#define IDC_TIMER_DISPLAY 1		//显示定时器，用于显示数据，每隔3秒执行一次
#define IDC_TIMER_DISPTIME 2		//用于显示系统时间定时器，每隔1秒执行一次

//自定义消息，在RemoteOPCClientDlg.cpp和FurTempThrea.cpp中使用
#define WM_MYGETMSG			 WM_USER+105	//OnGetOPCData
#define WM_MYSETMSG			 WM_USER+106	//OnSetOPCData
#define WM_MYTHREADMSG		 WM_USER+107	//
#define WM_MYTHREADSUSPEND	 WM_USER+108
#define WM_MYTHREADRESUME	 WM_USER+109



//double  btTempInit;	//钢坯入炉温度
//钢坯工艺指标（钢坯出炉表面温度，钢坯出炉最大断面温差，钢坯出炉表面最大目标温差，加热温度上限，加热温度下限）
struct  btTarget
{
	double  btTargetTemp;	//钢坯出炉目标温度
	double  btSectionTemp;	//钢坯出炉最大断面温差
	double  btAcandTaTemp;	//钢坯出炉表面温度与目标温度最大温差
	double  tempUplimit[3];	//钢坯加热制度，炉温设定上限
	double  tempDownLimit[3];	//钢坯加热制度，炉温设定下限
};
//钢坯物理信息（钢坯长度、钢坯厚度、钢坯种类、钢坯密度、钢坯比热容、钢坯导热系数、角度系数、钢坯黑度）
struct  btPysicalInfo
{
	double  btLength;			//钢坯长度
	double  btWidth;			//钢坯宽度
	double  btHeight;			//钢坯厚度
	double  btDensity;			//钢坯密度
	double  btSpecificHeat;		//钢坯比热容
	double  btConductivity;		//钢坯导热系数
	double  btBlackness;		//钢坯黑度
	char  btType[50];			//钢坯种类
	double  btAngle;			//角度系数
};


//钢坯信息（钢坯入炉温度，钢坯工艺指标，钢坯物理信息，钢坯位置，钢坯温度分布）
struct btInfo
{
	double btTempInit;				//钢坯入炉温度
	double btTempOut;				//钢坯出炉温度
	struct btTarget bttarget;		//钢坯工艺指标
	struct btPysicalInfo btpysicalinfo;//钢坯物理信息

	double site;				//钢坯当前位置
	int dzCount;				//钢坯分层数
	double flux_down;		//上热流密度
	double flux_up;			//下热流密度
	double sum;				// 钢坯表面温度总和
	double *btTempDistri;//指向当前时刻温度分布数组的指针
};


//对列
typedef struct Queue{
	int front;
	int rear;
	float *TEMP;

}QUEUE;

//钢坯链表节点

struct  btListNode
{
	struct  btInfo btinfo;		//钢坯信息
	struct  btListNode  *pNext;		//指向链表下一节点
};

//定义全局变量，三线程公用变量

extern double furnaceSize[3];	//加热炉尺寸，长宽高
extern double thermocouple[4];	//热电偶位置，用于炉温拟合

extern const double Stefan_Boltzmann;	//斯忒藩-玻尔兹曼常数,计算钢温分布时用
extern double furTempSet[3];		//炉温预设定值
extern double steplength;	//步进梁步进距离
extern int  stepcycle;		//步进周期
extern double v;			//钢坯在炉内运行速度
extern int iCount;				//链表中节点个数，即钢坯块数

extern char btTypeChoose[40];	//持续读写的钢坯种类

extern struct btInfo btInfoInit;	//定义一个钢坯信息的结构体对象，存储入炉钢坯信息
extern struct btListNode *pHead;	//链表头
extern struct btListNode *NewbtList;	// 优化结构体

extern CRITICAL_SECTION crtSec;		//临界区对象

//FurTempOptThread.cpp与BtTempCalThread.cpp公用
extern HANDLE smphPrint; // 信号量,表示是否已经达到允许的最大线程数

//RemoteOPCClientDlg.cpp自用
extern int connectResponse;

//RemoteOPCClientDlg.cpp与BtTempCalThread.cpp公用
extern bool btInSignal;			//钢坯入炉信号
extern bool btOutSignal;		//钢坯出炉信号
extern bool initterminalFlag;	//入炉初始化结束标志位
extern double furTemp[4];		////12个实测炉温，一段只用一个，转换为三段炉温
extern double botTemp[4];	//加热炉下部炉温分布
extern HANDLE hMainTimerCal;	//定时器对象

extern bool optState;
extern bool optRelease;		// 优化线程申请空间是否释放标志

//声明公用函数

//拟合函数(n:x,y数组长度，x[],热电偶位置，y[]：炉温，poly_n为拟合次数，p：拟合后的系数（降序排列））
void polyfit(int n, double x[], double y[], int poly_n, double p[]);

//复制当前链表
struct btListNode* listCopy(struct btListNode * pHead);

void InitFurnaceInfo();

//队列消息函数
void init(QUEUE *Pq);
int full_quene(QUEUE *Pq);
int en_quene(QUEUE *Pq, float val);
void bianli_queue(QUEUE *Pq);
int empty_quene(QUEUE *Pq);
int out_quene(QUEUE *Pq, float *pal);

void freeOptSize(struct btListNode *btHead);	// 释放钢坯结构体申请的动态内存
#endif