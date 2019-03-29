#include"stdafx.h"
#include"BtTempCalThread.h"
#include"VariableDef.h"
#include<math.h>
#include<stdlib.h>

#include<Windows.h>
#include<time.h>
//#include<iostream>
using namespace std;


/////////////////////////////调试用////////////////////////////////////
extern float readData[14];
extern int iTime;		//记录步进时间
clock_t begin, end;
double cost;
//开始记录
extern bool optFurTemp;

FILE *fpbtOutTemp = fopen("dbtOutTemp.txt", "w");

////////////////////////////////////////////////////////////////////////

/*
* 函数介绍：钢坯温度分布计算线程
* 输入参数：LPVOID lpParam
* 输出参数：
* 返回值  ：
*/
unsigned long BtTempCal(LPVOID lpParam)
{
	/*待测试程序段*/

	int calPrd = 2000;		//定时器周期
	LARGE_INTEGER li;
	li.QuadPart = -1;
	//创建一个timer对象
	hMainTimerCal = CreateWaitableTimer(NULL, 0, NULL);

	smphPrint = CreateSemaphore(NULL, 1, 1, L"PrintSemaphore"); // 创建信号量时即要同时指出允许的最大资源计数和当前可用资源计数

	WaitForSingleObject(smphPrint, INFINITE); // 等待对共享资源请求被通过

	SetWaitableTimer(hMainTimerCal, &li, calPrd, NULL, NULL, 0);



	//读取数据，直到入炉信号为true，则跳出此循环，开始入炉过程
	while (1)
	{
		WaitForSingleObject(hMainTimerCal, INFINITE);

		//发送读取温度消息
		PostMessage(AfxGetApp()->m_pMainWnd->m_hWnd, WM_MYGETMSG, NULL, NULL);
		Sleep(50);


		//关键区代码////////////确保读取完数据再进行操作
		EnterCriticalSection(&crtSec);
		printf("\n\n-------------------计算线程进入临界区--------------------\n\n");
		LeaveCriticalSection(&crtSec);


		if (btInSignal == true)
		{


			pHead = Create();

			btInSignal = false;
			//开启内核定时器

			break;
		}
	}
	

	//根据第一块入炉钢坯位置和出炉信号判断是否入炉结束

	while (false == initterminalFlag)
	{
		/*待测试程序段*/
		WaitForSingleObject(hMainTimerCal, INFINITE);//////	初始化时较快，不用开定时器
//		Sleep(100);

		//发送读取温度消息
		PostMessage(AfxGetApp()->m_pMainWnd->m_hWnd, WM_MYGETMSG, NULL, NULL);
		Sleep(50);

		//关键区代码
		EnterCriticalSection(&crtSec);
		//读取温度数据
		printf("\n\n-------------------计算线程进入临界区--------------------\n\n");

		LeaveCriticalSection(&crtSec);

		
		if (true == btInSignal)
		{
			update(pHead);
			Insert(pHead);
			btInSignal = false;
			
		}
		else
		{
			update(pHead);
		}
		
		if (pHead->btinfo.site > 23.36 && btOutSignal == true)
		{
			initterminalFlag = true;
			pHead = Delete(pHead);
			btOutSignal = false;
		}

		//SendTemp(pHead);
	}


	ReleaseSemaphore(smphPrint, 1, NULL); // 释放共享资源 等于 V操作

	printf("\n\n\n++++++++++++++++++++++++++++释放资源完成++++++++++++++++++++++++++\n\n\n");
	WaitForSingleObject(hMainTimerCal, INFINITE);

	while (1)
	{

		if (optState == true)
		{
			WaitForSingleObject(hMainTimerCal, INFINITE);
			
			WaitForSingleObject(smphPrint, INFINITE); // 等待对共享资源请求被通过，等于P操作

			//发送读取温度消息
			PostMessage(AfxGetApp()->m_pMainWnd->m_hWnd, WM_MYGETMSG, NULL, NULL);
			Sleep(100);

			//关键区代码
			EnterCriticalSection(&crtSec);

			LeaveCriticalSection(&crtSec);

			if (true == btInSignal)
			{
				update(pHead);
				Insert(pHead);
				btInSignal = false;
			}
			else
			{
				update(pHead);
			}

			if (true == btOutSignal && pHead->btinfo.site > 23.3)
			{
				fprintf(fpbtOutTemp, "%f ", pHead->btinfo.btTempDistri[0]);
				fprintf(fpbtOutTemp, "%f\n", pHead->btinfo.btTempDistri[13]);
				pHead = Delete(pHead);
				btOutSignal = false;
			}

			ReleaseSemaphore(smphPrint, 1, NULL); // 释放共享资源 等于 V操作
			printf("\n\n\n++++++++++++++++++++++++++++释放资源完成++++++++++++++++++++++++++\n\n\n");
		}

		else
		{
			WaitForSingleObject(hMainTimerCal, INFINITE);
			//发送读取温度消息
			PostMessage(AfxGetApp()->m_pMainWnd->m_hWnd, WM_MYGETMSG, NULL, NULL);
			Sleep(100);

			//关键区代码
			EnterCriticalSection(&crtSec);
			//读取温度数据

			LeaveCriticalSection(&crtSec);

			if (true == btInSignal)
			{
				
				update(pHead);
				Insert(pHead);
				btInSignal = false;
			}
			else
			{
				update(pHead);
			}

			if (true == btOutSignal  && pHead->btinfo.site > 23.3)
			{
				fprintf(fpbtOutTemp, "%f ", pHead->btinfo.btTempDistri[0]);
				fprintf(fpbtOutTemp, "%f\n", pHead->btinfo.btTempDistri[13]);

				pHead = Delete(pHead);
				btOutSignal = false;
				printf("更新后钢坯位置---------------%f\n", pHead->btinfo.site);
				//			Sleep(500);
			}
		}
	}
	return 0;
}



//发送钢坯温度信息
void SendTemp(struct btListNode *pHead)
{
	struct btListNode* pTemp = pHead;
	double temp[90] = {0};
	int i = 90;
	double interval = furnaceSize[0] / 107;	//107代表炉内钢坯总数

	while (pTemp->pNext)
	{
		i = (int)floor(pTemp->btinfo.site / interval);
		temp[i] = pTemp->btinfo.btTempDistri[0];
		pTemp = pTemp->pNext;
	}
}
/*
* 函数介绍：计算热流密度
* 输入参数：struct btInfo * btinfotemp
* 输出参数：
* 返回值  ：
*/
void cal_flux(struct btInfo * btinfotemp)
{
	double para_up[3];				//上二次函数系数
	double  para_down[3];			//下炉温

	//拟合炉温，求二次函数系数
	polyfit(sizeof(thermocouple) / sizeof(thermocouple[0]), thermocouple, furTemp, 2, para_up);
	polyfit(sizeof(thermocouple) / sizeof(thermocouple[0]), thermocouple, botTemp, 2, para_down);

	for (int i = 0; i < 3; i++)
	{
		printf("系数%.2f\n", para_up[i]);
	}
	for (int i = 0; i < 4; i++)
	{
		printf("炉温%.2f\n", furTemp[i]);
	}

	double temp_up = para_up[0] * btinfotemp->site * btinfotemp->site + para_up[1] * btinfotemp->site + para_up[2];		//当前位置上炉温
	double temp_down = para_down[0] * btinfotemp->site * btinfotemp->site + para_down[1] * btinfotemp->site + para_down[2];		//当前位置下炉温

	//应该先更新钢坯物理信息和炉气黑度，在求热流密度，计算钢坯温度分布
	double eco2_up = 7.17 * pow((0.13*5.56), (1.0 / 3)) / sqrt(temp_up);
	double eh2o_up = 7.17 * 0.11 * 5.56 * pow(5.56, 0.6) / temp_up;
	double airBlackness_up = eco2_up + 1.05 * eh2o_up;	//上部炉气黑度

	double eco2_down = 7.17 * pow((0.13*5.56), (1.0 / 3)) / sqrt(temp_down);
	double eh2o_down = 7.17 * 0.11 * 5.56 * pow(5.56, 0.6) / temp_down;
	double airBlackness_down = eco2_down + 1.05 * eh2o_down;//下部炉气黑度


	double flux_up;		//上部热流密度
	double flux_down;

	//计算上部热流密度，在温度处加上273K
	flux_up = (Stefan_Boltzmann*btinfotemp->btpysicalinfo.btBlackness*airBlackness_up*(1 + btinfotemp->btpysicalinfo.btAngle*(1 - airBlackness_up))*(pow((temp_up + 273), 4) - pow((btinfotemp->btTempDistri[0] + 273), 4))) /
		(airBlackness_up + btinfotemp->btpysicalinfo.btAngle * (1 - airBlackness_up)*(btinfotemp->btpysicalinfo.btBlackness + (1 - btinfotemp->btpysicalinfo.btBlackness) * airBlackness_up));

	//计算下部热流密度
	flux_down = (Stefan_Boltzmann*btinfotemp->btpysicalinfo.btBlackness*airBlackness_down*(1 + btinfotemp->btpysicalinfo.btAngle*(1 - airBlackness_down))*(pow((temp_down + 273), 4) - pow((btinfotemp->btTempDistri[btinfotemp->dzCount - 1] + 273), 4))) /
		(airBlackness_down + btinfotemp->btpysicalinfo.btAngle * (1 - airBlackness_down)*(btinfotemp->btpysicalinfo.btBlackness + (1 - btinfotemp->btpysicalinfo.btBlackness) * airBlackness_down));

	btinfotemp->flux_up = flux_up;
	btinfotemp->flux_down = flux_down;

}
/*
* 函数介绍：初始化钢坯温度分布，不均匀分布 1月3日修改
* 输入参数：struct btInfo * bt_infoinit
* 输出参数：
* 返回值  ：
*/
void init_unequal(struct btInfo * bt_infoinit)
{
	// 表面温度得到中心温度

	float centorTemp = -6.23484848e-04 * pow(bt_infoinit->btTempInit, 2) + 0.609121212 * bt_infoinit->btTempInit - 2.11518939;
	double para[3] = {};
	
	// 偶数层

	if (bt_infoinit->dzCount % 2 == 0)
	{
		double N1 = bt_infoinit->dzCount / 2;
		double N2 = bt_infoinit->dzCount / 2 - 1;

		double height[4] = { 0, centorTemp, centorTemp, 0 };
		double site[4] = { 0, N2, N1, bt_infoinit->dzCount-1 };

		polyfit(4, site, height, 2, para);
	}
	else
	{
		double N = (bt_infoinit->dzCount - 1) / 2;

		double height[3] = { 0, centorTemp, 0 };
		double site[3] = { 0, N, bt_infoinit->dzCount-1 };

		// 由中心温度和层数拟合二次曲线
		polyfit(3, site, height, 2, para);
	}


	for (int i = 0; i < bt_infoinit->dzCount; i++)
	{
		bt_infoinit->btTempDistri[i] = bt_infoinit->btTempInit + para[0] * i*i + para[1] * i + para[2];
	}
}
/*
* 函数介绍：读取钢坯物理信息
* 输入参数：struct btPysicalInfo *btPyInfo
* 输出参数：
* 返回值  ：
*/
void initBtInSite(struct btPysicalInfo *btPyInfo)
{
	//根据钢种从文件中读取钢坯的密度，比热，导热系数
	FILE *fpBtPara = fopen("btparameter.txt", "r");
	char tempType[50];
	if (fpBtPara != NULL)
	{
		fseek(fpBtPara, 0L, SEEK_SET);
		while (!feof(fpBtPara))
		{// 钢坯种类 钢坯厚度 钢坯长度 钢坯密度 钢坯比热 钢坯导热系数 钢坯黑度 钢坯出炉目标 目标最大温差 钢坯断面温差
			fscanf(fpBtPara, "%s %lf %lf %lf %lf %lf %lf", &tempType, &btPyInfo->btHeight, &btPyInfo->btLength,
				&btPyInfo->btDensity, &btPyInfo->btSpecificHeat, &btPyInfo->btConductivity,&btPyInfo->btBlackness);
			//根据不同钢种选择不同的钢坯参数
			if (!strcmp(tempType, btInfoInit.btpysicalinfo.btType))
			break;
		}
	}
	strcpy(btPyInfo->btType, tempType);

	btPyInfo->btAngle = btPyInfo->btLength / (2 * 1000 * (furnaceSize[1] + furnaceSize[2]) - btPyInfo->btLength);

	fclose(fpBtPara);
		
}
/*
* 函数介绍：初始化钢坯信息
* 输入参数：struct btPysicalInfo *btPyInfo
* 输出参数：
* 返回值  ：
*/
void init_bt_info(struct btInfo * bt_infoinit)
{
	
	initBtInSite(&bt_infoinit->btpysicalinfo);		//读取钢坯物理信息

	bt_infoinit->btTempInit = btInfoInit.btTempInit;//读取钢坯的入炉温度

	bt_infoinit->bttarget = btInfoInit.bttarget;		//读取钢坯的工艺指标
	
	bt_infoinit->sum = 0;
	bt_infoinit->site = 0;							//将钢坯的初始位置设为0

	bt_infoinit->dzCount = (int)floor(bt_infoinit->btpysicalinfo.btHeight / dz);		//根据钢坯厚度和分层间隔，划分钢坯层数
	
	

	bt_infoinit->btTempDistri = (double*)malloc(bt_infoinit->dzCount * sizeof(double));	//为钢坯温度分布分配空间

	init_unequal(bt_infoinit);
	for (int i = 0; i < bt_infoinit->dzCount; i++)
	{

		printf("钢坯入炉温度%.2f\n", bt_infoinit->btTempDistri[i]);
	}
}
/*
* 函数介绍：第一块钢坯入炉，创建链表
* 输入参数：
* 输出参数：
* 返回值  ：创建成功的节点struct btListNode *()
*/

struct btListNode *Create()
{
	struct btListNode *pHead = (struct btListNode*)malloc(sizeof(struct btListNode));//入炉过程开始，创建头结点

	pHead->pNext = NULL;	//指向下一节点的指针为空

	//初始钢坯种类	1月26日
	strcpy(btInfoInit.btpysicalinfo.btType, btTypeChoose);

	init_bt_info(&pHead->btinfo);	//初始化钢坯信息
	cal_flux(&pHead->btinfo);		//计算热流密度
	iCount++;		//链表节点数加1
	return pHead;
}
/*
* 函数介绍：新钢坯入炉后，在链表尾部插入新的节点，并将新节点内的内容初始化
* 输入参数：struct btListNode * pHead 头节点
* 输出参数：
* 返回值  ：创建成功的节点
*/
void Insert(struct btListNode * pHead)
{
	struct btListNode *pNew,*pTemp = pHead;	//指向新分配的空间
	pNew = (struct btListNode*)malloc(sizeof(struct btListNode));	//给新节点分配空间

	pNew->pNext = NULL;

	init_bt_info(&pNew->btinfo);				//初始化钢坯信息
	cal_flux(&pNew->btinfo);			//计算热流密度

	while (pTemp->pNext)						//找到最后一个节点
	{
		pTemp = pTemp->pNext;
	}
	pTemp->pNext = pNew;

	iCount++;		//链表节点数加1
}
/*
* 函数介绍：当有钢坯出炉时，删除第一个节点，并释放第一个节点空间以及第一个节点内钢坯温度分布空间
* 输入参数：struct btListNode * pHead 头节点
* 输出参数：
* 返回值  ：创建成功的节点
*/

struct btListNode* Delete(struct btListNode * pHead)
{
	struct btListNode * pTemp;
	pTemp = pHead;
	pHead = pHead->pNext;
	free(pTemp->btinfo.btTempDistri);
	free(pTemp);
	iCount--;
	return pHead;
}
/*
* 函数介绍：计算钢坯温度分布
* 输入参数：struct btInfo * btinfotemp
* 输出参数：
* 返回值  ：创建成功的节点
*/
void calculation(struct btInfo * btinfotemp)
{

	v = steplength / stepcycle;
	btinfotemp->site += 2 * v;		//更新位置

	double para_up[3];				//上二次函数系数
	double  para_down[3];			//下炉温

	//拟合炉温，求二次函数系数
	polyfit(sizeof(thermocouple) / sizeof(thermocouple[0]), thermocouple, furTemp, 2, para_up);
	polyfit(sizeof(thermocouple) / sizeof(thermocouple[0]), thermocouple, botTemp, 2, para_down);

	double temp_up = para_up[0] * btinfotemp->site * btinfotemp->site + para_up[1] * btinfotemp->site + para_up[2];		//当前位置上炉温
	double temp_down = para_down[0] * btinfotemp->site * btinfotemp->site + para_down[1] * btinfotemp->site + para_down[2];		//当前位置下炉温

	//应该先更新钢坯物理信息和炉气黑度，在求热流密度，计算钢坯温度分布
	double eco2_up = 7.17 * pow((0.13*5.56), (1.0 / 3)) / sqrt(temp_up);
	double eh2o_up = 7.17 * 0.11 * 5.56 * pow(5.56, 0.6) / temp_up;
	double airBlackness_up = eco2_up + 1.05 * eh2o_up;	//上部炉气黑度

	double eco2_down = 7.17 * pow((0.13*5.56), (1.0 / 3)) / sqrt(temp_down);
	double eh2o_down = 7.17 * 0.11 * 5.56 * pow(5.56, 0.6) / temp_down;
	double airBlackness_down = eco2_down + 1.05 * eh2o_down;//下部炉气黑度


	double flux_up;		//上部热流密度
	double flux_down;

	double *prebtTemp = (double *)malloc(btinfotemp->dzCount * sizeof(double));

	//将上一次钢坯温度分布复制下来
	for (int i = 0; i < btinfotemp->dzCount; i++)
	{
		prebtTemp[i] = btinfotemp->btTempDistri[i];
	}

	//计算上部热流密度，在温度处加上273K
	flux_up = (Stefan_Boltzmann*btinfotemp->btpysicalinfo.btBlackness*airBlackness_up*(1 + btinfotemp->btpysicalinfo.btAngle*(1 - airBlackness_up))*(pow((temp_up + 273), 4) - pow((btinfotemp->btTempDistri[0] + 273), 4))) /
		(airBlackness_up + btinfotemp->btpysicalinfo.btAngle * (1 - airBlackness_up)*(btinfotemp->btpysicalinfo.btBlackness + (1 - btinfotemp->btpysicalinfo.btBlackness) * airBlackness_up));

	//计算下部热流密度
	flux_down = (Stefan_Boltzmann*btinfotemp->btpysicalinfo.btBlackness*airBlackness_down*(1 + btinfotemp->btpysicalinfo.btAngle*(1 - airBlackness_down))*(pow((temp_down + 273), 4) - pow((btinfotemp->btTempDistri[btinfotemp->dzCount - 1] + 273), 4))) /
		(airBlackness_down + btinfotemp->btpysicalinfo.btAngle * (1 - airBlackness_down)*(btinfotemp->btpysicalinfo.btBlackness + (1 - btinfotemp->btpysicalinfo.btBlackness) * airBlackness_down));


	//计算当前时刻钢坯上表面温度
	btinfotemp->btTempDistri[0] = prebtTemp[0] + dt * flux_up / (btinfotemp->btpysicalinfo.btDensity*btinfotemp->btpysicalinfo.btSpecificHeat * (dz / 1000.0)) +
		btinfotemp->btpysicalinfo.btConductivity * dt * (prebtTemp[1] - prebtTemp[0]) / (btinfotemp->btpysicalinfo.btDensity*btinfotemp->btpysicalinfo.btSpecificHeat * (dz / 1000.0) * (dz / 1000.0));
	//计算当前时刻钢坯下表面温度
	btinfotemp->btTempDistri[btinfotemp->dzCount - 1] = prebtTemp[btinfotemp->dzCount - 1] + dt * flux_down / (btinfotemp->btpysicalinfo.btDensity*btinfotemp->btpysicalinfo.btSpecificHeat * (dz / 1000.0))
		+ btinfotemp->btpysicalinfo.btConductivity * dt *(prebtTemp[btinfotemp->dzCount - 2] - prebtTemp[btinfotemp->dzCount - 1]) / (btinfotemp->btpysicalinfo.btDensity * btinfotemp->btpysicalinfo.btSpecificHeat*(dz / 1000.0)*(dz / 1000.0));
	//计算钢坯内部各层温度
	for (int i = 1; i < btinfotemp->dzCount - 1; i++)
	{
		btinfotemp->btTempDistri[i] = prebtTemp[i] + btinfotemp->btpysicalinfo.btConductivity * dt *(prebtTemp[i - 1] + prebtTemp[i + 1] - 2 * prebtTemp[i]) / (btinfotemp->btpysicalinfo.btDensity*btinfotemp->btpysicalinfo.btSpecificHeat*(dz / 1000.0)*(dz / 1000.0));
	}

	btinfotemp->sum += (prebtTemp[0] + btinfotemp->btTempDistri[0]) * v * dt / 2;

	free(prebtTemp);
}

/*
* 函数介绍：根据炉内工况，更新钢坯位置、温度分布等
* 输入参数：struct btListNode* pHead
* 输出参数：
* 返回值  ：创建成功的节点
*/
void update(struct btListNode* pHead)
{
	struct btListNode* pTemp = pHead;
	while (pTemp)
	{
		calculation(&pTemp->btinfo);	//计算温度分布
		pTemp = pTemp->pNext;
	}

}