#include"stdafx.h"
#include"VariableDef.h"



//����ȫ�ֱ���

double furnaceSize[3];	//����¯�ߴ磬�����
double thermocouple[4];	//�ȵ�żλ�ã�����¯�����
const double Stefan_Boltzmann = 5.67 * pow(10.0, -8);	//˹߯��-������������,������·ֲ�ʱ��
double steplength = 0.25;		//��������������
int  stepcycle = 29;		//��������
double v = 0;	//���������ٶ�

double  furTempSet[3];		//¯��Ԥ�趨ֵ
int iCount = 0;				////��¼�����нڵ�ĸ���,����������

struct btInfo btInfoInit;	//����һ��������Ϣ�Ľṹ����󣬴洢��¯������Ϣ
struct btListNode *pHead = NULL;	//����ͷ
struct btListNode *NewbtList = NULL;	//�Ż�����ָ��

CRITICAL_SECTION crtSec;	//�ٽ�������

//RemoteOPCClientDlg.cpp����
int connectResponse = 0;

//FurTempOptThread.cpp��BtTempCalThread.cpp���ñ���
HANDLE smphPrint; // �ź���,��ʾ�Ƿ��Ѿ��ﵽ���������߳���


//RemoteOPCClientDlg.cpp��BtTempCalThread.cpp���ñ���
bool btInSignal = false;		//������¯�ź�
bool btOutSignal = false;		//������¯�ź�
bool initterminalFlag = false;		//��¯��ʼ��������־λ

double furTemp[4];	//13��ʵ��¯�£�һ��ֻ��һ����ת��Ϊ4��¯��
double botTemp[4];	//����¯�²�¯�·ֲ�
char btTypeChoose[40] = "APHC";	//������д�ĸ�������

HANDLE hMainTimerCal = NULL;	//���¼����̶߳�ʱ������

//RemoteOPCClientDlg.cpp��BtTempOptThread.cpp���ñ���

bool optState = false;		//�Ż��߳�״̬
bool optRelease = false;	// �Ż��߳�����ռ��Ƿ��ͷű�־




//�������ú���


/*============================================================*/
////    ��˹��Ԫ������õ�   n �ζ���ʽ��ϵ��  
////    n: ϵ���ĸ���  
////    ata: ���Ծ���  
////    sumxy: ���Է������Yֵ  
////    p: ������ϵĽ��  
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
//��Ϻ���(n:x,y���鳤�ȣ�x[],�ȵ�żλ�ã�y[]��¯�£�poly_nΪ��ϴ�����p����Ϻ��ϵ�����������У���
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

//���Ƶ�ǰ����
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
		pNew->btinfo.btTempDistri = (double*)malloc(pNew->btinfo.dzCount * sizeof(double));	//Ϊ�����¶ȷֲ�����ռ�
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
//////////����////////////////////

void init(QUEUE *Pq)
{	//��ʼ������
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
* �������ܣ��ͷŸ����ṹ��������Ķ�̬�ռ�
* ���������struct btListNode *pTemp
* ���������
* ����ֵ  ��
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
