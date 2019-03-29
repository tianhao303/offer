#include"stdafx.h"
#include"VariableDef.h"



//定义全局变量

double furnaceSize[3];	//加热炉尺寸，长宽高
double thermocouple[4];	//热电偶位置，用于炉温拟合
const double Stefan_Boltzmann = 5.67 * pow(10.0, -8);	//斯忒藩-玻尔兹曼常数,计算钢温分布时用
double steplength = 0.25;		//步进梁步进距离
int  stepcycle = 29;		//步进周期
double v = 0;	//钢坯运行速度

double  furTempSet[3];		//炉温预设定值
int iCount = 0;				////记录链表中节点的个数,即钢坯块数

struct btInfo btInfoInit;	//定义一个钢坯信息的结构体对象，存储入炉钢坯信息
struct btListNode *pHead = NULL;	//链表头
struct btListNode *NewbtList = NULL;	//优化拷贝指针

CRITICAL_SECTION crtSec;	//临界区对象

//RemoteOPCClientDlg.cpp自用
int connectResponse = 0;

//FurTempOptThread.cpp与BtTempCalThread.cpp公用变量
HANDLE smphPrint; // 信号量,表示是否已经达到允许的最大线程数


//RemoteOPCClientDlg.cpp与BtTempCalThread.cpp公用变量
bool btInSignal = false;		//钢坯入炉信号
bool btOutSignal = false;		//钢坯出炉信号
bool initterminalFlag = false;		//入炉初始化结束标志位

double furTemp[4];	//13个实测炉温，一段只用一个，转换为4段炉温
double botTemp[4];	//加热炉下部炉温分布
char btTypeChoose[40] = "APHC";	//持续读写的钢坯种类

HANDLE hMainTimerCal = NULL;	//钢温计算线程定时器对象

//RemoteOPCClientDlg.cpp与BtTempOptThread.cpp公用变量

bool optState = false;		//优化线程状态
bool optRelease = false;	// 优化线程申请空间是否释放标志




//声明公用函数


/*============================================================*/
////    高斯消元法计算得到   n 次多项式的系数  
////    n: 系数的个数  
////    ata: 线性矩阵  
////    sumxy: 线性方程组的Y值  
////    p: 返回拟合的结果  
/*============================================================*/
void gauss_solve(int n, double A[], double x[], double b[])
{
	int i, j, k, r;
	double max;
	for (k = 0; k<n - 1; k++)
	{
		max = fabs(A[k*n + k]);                 // find maxmum   
		r = k;
		for (i = k + 1; i<n - 1; i++)
		{
			if (max<fabs(A[i*n + i]))
			{
				max = fabs(A[i*n + i]);
				r = i;
			}
		}
		if (r != k)
		{
			for (i = 0; i<n; i++)        //change array:A[k]&A[r]  
			{
				max = A[k*n + i];
				A[k*n + i] = A[r*n + i];
				A[r*n + i] = max;
			}

			max = b[k];                    //change array:b[k]&b[r]  
			b[k] = b[r];
			b[r] = max;
		}

		for (i = k + 1; i<n; i++)
		{
			for (j = k + 1; j<n; j++)
				A[i*n + j] -= A[i*n + k] * A[k*n + j] / A[k*n + k];
			b[i] -= A[i*n + k] * b[k] / A[k*n + k];
		}
	}

	for (i = n - 1; i >= 0; x[i] /= A[i*n + i], i--)
	{
		for (j = i + 1, x[i] = b[i]; j<n; j++)
			x[i] -= A[i*n + j] * x[j];
	}

	double temp;
	for (int j = 0; j < n / 2; j++)
	{
		temp = x[j];
		x[j] = x[n - j - 1];
		x[n - j - 1] = temp;
	}

}
//拟合函数(n:x,y数组长度，x[],热电偶位置，y[]：炉温，poly_n为拟合次数，p：拟合后的系数（降序排列））
void polyfit(int n, double x[], double y[], int poly_n, double p[])
{
	int i, j;
	double *tempx, *tempy, *sumxx, *sumxy, *ata;

	tempx = (double *)calloc(n, sizeof(double));
	sumxx = (double *)calloc((poly_n * 2 + 1), sizeof(double));
	tempy = (double *)calloc(n, sizeof(double));
	sumxy = (double *)calloc((poly_n + 1), sizeof(double));
	ata = (double *)calloc((poly_n + 1)*(poly_n + 1), sizeof(double));
	for (i = 0; i<n; i++)
	{
		tempx[i] = 1;
		tempy[i] = y[i];
	}
	for (i = 0; i<2 * poly_n + 1; i++)
	{
		for (sumxx[i] = 0, j = 0; j<n; j++)
		{
			sumxx[i] += tempx[j];
			tempx[j] *= x[j];
		}
	}

	for (i = 0; i<poly_n + 1; i++)
	{
		for (sumxy[i] = 0, j = 0; j<n; j++)
		{
			sumxy[i] += tempy[j];
			tempy[j] *= x[j];
		}
	}

	for (i = 0; i<poly_n + 1; i++)
	{
		for (j = 0; j<poly_n + 1; j++)
		{
			ata[i*(poly_n + 1) + j] = sumxx[i + j];
		}
	}
	gauss_solve(poly_n + 1, ata, p, sumxy);

	free(tempx);
	free(sumxx);
	free(tempy);
	free(sumxy);
	free(ata);
}

//复制当前链表
struct btListNode* listCopy(struct btListNode * pHead)
{
	struct btListNode *pNew, *pNewHead, *pNewEnd = NULL;
	struct btListNode *pTemp = pHead;
	pNewHead = NULL;

	int iNode = 0;

	while (pTemp != NULL)
	{
		iNode++;

		pNew = (struct btListNode *)malloc(sizeof(struct btListNode));

		pNew->btinfo.btpysicalinfo = pTemp->btinfo.btpysicalinfo;
		pNew->btinfo.bttarget = pTemp->btinfo.bttarget;
		pNew->btinfo.btTempInit = pTemp->btinfo.btTempInit;
		pNew->btinfo.dzCount = pTemp->btinfo.dzCount;
		pNew->btinfo.site = pTemp->btinfo.site;
		pNew->btinfo.sum = pTemp->btinfo.sum;
		//pNew->btinfo.btAngle = pTemp->btinfo.btAngle;
		pNew->btinfo.btTempDistri = (double*)malloc(pNew->btinfo.dzCount * sizeof(double));	//为钢坯温度分布分配空间
		for (int i = 0; i < pNew->btinfo.dzCount; i++)
		{
			pNew->btinfo.btTempDistri[i] = pTemp->btinfo.btTempDistri[i];
		}
		if (iNode == 1)
		{
			pNew->pNext = NULL;
			pNewEnd = pNew;
			pNewHead = pNew;
		}
		else
		{
			pNew->pNext = NULL;
			pNewEnd->pNext = pNew;
			pNewEnd = pNew;
		}
		pTemp = pTemp->pNext;
	}

	return pNewHead;
}

void InitFurnaceInfo()
{
	char furnaceFile[] = "furnacesize.txt";
	FILE *fpFurnace = fopen(furnaceFile, "r");

	if (fpFurnace != NULL)
	{ 
		fseek(fpFurnace, 0L, SEEK_SET);
		for (int i = 0; i < 3; i++)
		{
			fscanf(fpFurnace, "%lf", &furnaceSize[i]);
		}
		for (int i = 0; i < 4; i++)
		{
			fscanf(fpFurnace, "%lf", &thermocouple[i]);
		}
	}

	fclose(fpFurnace);
}
//////////对列////////////////////

void init(QUEUE *Pq)
{	//初始化对列
	Pq->front = 0;
	Pq->rear = 0;
	Pq->TEMP = (float *)malloc(sizeof(float)*NUM);
}

int full_quene(QUEUE *Pq)
{
	if ((Pq->rear + 1) % NUM == Pq->front)
	{
		return 0;
	}
	else{
		return 1;
	}
}

int en_quene(QUEUE *Pq, float val)
{
	if (full_quene(Pq) == 0)
		return 0;
	else
	{
		Pq->TEMP[Pq->rear] = val;
		Pq->rear = (Pq->rear + 1) % NUM;
	}

}

void bianli_queue(QUEUE *Pq)
{

	int i = Pq->front;
	while (i != Pq->rear)
	{
		printf("%.2f ", Pq->TEMP[i]);
		i = (i + 1) % NUM;
	}
}

int empty_quene(QUEUE *Pq)
{
	if (Pq->front == Pq->rear)
		return 0;
	else
	{
		return 1;
	}
}

int out_quene(QUEUE *Pq, float *pal)
{
	if (empty_quene(Pq) == 0)
		return 1;
	else
	{
		*pal = Pq->TEMP[Pq->front];
		Pq->front = (Pq->front + 1) % NUM;
		return 0;
	}
}

/*
* 函数介绍：释放钢坯结构体所申请的动态空间
* 输入参数：struct btListNode *pTemp
* 输出参数：
* 返回值  ：
*/
void freeOptSize(struct btListNode *btHead)
{
	struct btListNode *pTemp;

	while (btHead != NULL)
	{
		pTemp = btHead;
		btHead = btHead->pNext;
		free(pTemp->btinfo.btTempDistri);
		free(pTemp);
	}
}
