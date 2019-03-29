#include"stdafx.h"
#include"FurTempOptThread.h"
#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<math.h>
#include<string.h>

#include"VariableDef.h"

#include<Windows.h>
#include<time.h>


#define POPSIZE 500			//最大种群个数
#define CodeL 8				//编码长度
#define ParaC 3				//参数个数
#define chromlength CodeL*ParaC			//染色体编码长度
#define maxgeneration 50	//最大进化代数
int popsize = 20;			//种群数
double pc = 1.0;			//交叉率
//double pm = 0.095;			//变异率
double pm = 0.1;			//变异率

//定义个体，包含染色体、适应度、实际值
struct individual
{
	char chrom[chromlength + 1];		//染色体编码
	double fitness;					//适应度
	double targetfunction;			//目标函数值
	double P;						//惩罚项
	//	double dechrom[ParaC];			//解码后的三段温度

};

int generation;					//进化代数
int best_index;					//最佳个体索引号
int worst_index;				//最差个体索引号
//int worst_index[9];				//9个最差个体的索引号
//double tempPara[ParaC];			//临时存储三段炉温(计算目标函数时)
double besttempPara[ParaC];		//存储当前代最佳炉温
double previousbesttempPara[ParaC];	//存储前一代最佳炉温
double SecondbesttempPara[ParaC];	//存储第二优个体炉温

double errbest[ParaC] = { 5.0, 5.0, 5.0 };			//当前代与上一代最优值(三段)的差值

struct individual bestindividual;		//最佳个体
struct individual worstindividual;		//最差个体
struct individual secondindividual;		//第二个体

//struct individual worstindividual[9];	//找出最差的9个个体(其中8个用预测值替换，1个用前一代最佳值替换)
struct individual previousbest;			//前一代最佳个体
//struct individual currentbest;			//当前代及之前中的最佳个体
struct individual population[POPSIZE];	//POPSIZE个个体
//struct individual forcastpopulation[8];	//预测的8个最佳个体
struct individual forcastpopulation;	//预测的1个个体

double penalty[POPSIZE];

float uplimit[ParaC] = { 1100, 1350, 1330 };	//预热段、加热段、均热段温度上限
float downlimit[ParaC] = { 850, 1150, 1130 };	//预热段、加热段、均热段温度下限
float JDpreheating = (float)((uplimit[0] - downlimit[0]) / (pow(2.0, CodeL) - 1));	//预热段编码精度
float JDheating = (float)((uplimit[1] - downlimit[1]) / (pow(2.0, CodeL) - 1));		//加热段编码精度
float JDsoking = (float)((uplimit[2] - downlimit[2]) / (pow(2.0, CodeL) - 1));		//均热段编码精度

double restrictSection[3] = { 20, 10, 10 };	//预测出炉断面温差约束条件
double restrictSurface[3] = { 20, 10, 10 };		//预测出炉表面温度约束条件
double restrictActually[3] = { 20, 10, 10 };	//实测余预测

double optv;		//钢坯在炉内运行速度


double AtuallyTemp = 1120;	//实测钢坯出炉温度

///主内核定时器
HANDLE hMainTimerOpt = NULL;


/////////////////////////////////////////////调试用//////////////////////////////////////////////
extern bool optFurTemp;
FILE *fpWrite;
FILE *fpBestvidualofTemp;
FILE *fpBestofObjectfunction;
FILE *fpBestofFitness;
FILE *fpPreBtOutTemp;
FILE *fpIndividual;
FILE *fpTime;
FILE *fpTime2;
FILE *fpTime3;

FILE *fpCalTime;
FILE *fpOtherTime;
FILE *fpCycleTime;
FILE *fpInvidSum;
FILE *fpforcastTemp;
FILE *fpTemp;
FILE *fpP;

clock_t begin4, end4;
double cost4;

clock_t begin5, end5;
double cost5;

clock_t begin6, end6;
double cost6;
/////////////////////////////////////////////////////////////////////////////////////////////////


unsigned long FurTempOpt(LPVOID lpParam)
{ 

	//定时器周期
	int calPrd = 4000;
	//设置定时器
	LARGE_INTEGER li;
	li.QuadPart = -1;
	//建立内核定时器
	hMainTimerOpt = CreateWaitableTimer(NULL, 0, NULL);
	//开启内核定时器
	SetWaitableTimer(hMainTimerOpt, &li, calPrd, NULL, NULL, 0);

	srand((unsigned) time(NULL));

//////////////////////////////////////////调试用//////////////////////////////////////////////////////
	clock_t begin1, end1;
	double cost1;

	clock_t begin2, end2;
	double cost2;

	clock_t begin3, end3;
	double cost3;


	fpWrite = fopen("doptFurTemp.txt", "w");	//优化出的最有炉温设定值
	fpBestvidualofTemp = fopen("dbestIndividulaFurTemp.txt", "w");	//每一代最佳炉温
	fpBestofObjectfunction = fopen("dBestofObjectfunction.txt", "w");	//最佳个体的目标函数值
	fpBestofFitness = fopen("dBestofFitness.txt", "w");					//最佳的适应度值
	fpPreBtOutTemp = fopen("dPreBtOutTemp.txt", "w");					//每个个体下的炉内所的钢坯的出炉温度
	fpIndividual = fopen("dfpIndividual.txt", "w");						//每个个体的适应度
	fpTime = fopen("dtime.txt", "w");									//优化一次的时间
	fpTime2 = fopen("dtime2.txt", "w");									//优化总时间，从等待到结束
	fpTime3 = fopen("dtime3.txt", "w");									//优化等待时间


	fpCalTime = fopen("dcalTime.txt", "w");								//评价个体花费时间
	fpOtherTime = fopen("dotherTime.txt", "w");							//生成种群时间
	fpCycleTime = fopen("dcyleTime.txt", "w");							//迭代一次需要的时间

	fpInvidSum = fopen("fpInvidSum.txt", "w");

	fpforcastTemp = fopen("fpforcastTemp.txt", "w");		//预测温度
	fpTemp = fopen("fpTemp.txt", "w");						//每一代的温度
	fpP = fopen("fpP.txt", "w");							// 预惩罚项
	WaitForSingleObject(hMainTimerOpt, INFINITE);

	begin3 = clock();

	/*待测试程序段*/
	while (1)
	{
//		fpWrite = fopen("data.txt", "a");		

		if (initterminalFlag == true)
		{
	
			begin2 = clock();
			/*待测试程序段*/
		
			printf("\n\n\n########################优化线程等待资源#################\n\n\n");
			WaitForSingleObject(smphPrint, INFINITE); // 等待对共享资源请求被通过，等于P操作

			begin1 = clock();
			/*待测一次优化所用时间*/

			end3 = clock();
			cost3 = ((double)(end3 - begin3));
			//		cost = ((double)(end - begin)) / CLOCKS_PER_SEC;
			printf("constant CLOCKS_PER_SEC is: %ld, time cost is: %lf secs\n", CLOCKS_PER_SEC, cost3);
			fprintf(fpTime3, "%lf\n", cost3);

			begin3 = clock();
			/*测试两次优化之间时间间隔*/

			NewbtList = listCopy(pHead);

			// 复制完成，空间没有释放
			optRelease = TRUE;
			optv = v;
			printf("\n\n\n########################复制完成#################\n\n\n");
			

			ReleaseSemaphore(smphPrint, 1, NULL); // 释放共享资源 等于 V操作

			
			printf("开始优化--------------**************************************\n");

//			begin1 = clock();
//			/*待测试程序段*/
			
			ga(NewbtList);
			//发送计算完成消息
			PostMessage(AfxGetApp()->m_pMainWnd->m_hWnd, WM_MYSETMSG, NULL, NULL);
			printf("----------------优化完成****，最优炉温为:\n%f\t%f\t%f\n", furTempSet[0], furTempSet[1], furTempSet[2]);

			fprintf(fpWrite, "%f ", furTempSet[0]);
			fprintf(fpWrite, "%f ", furTempSet[1]);
			fprintf(fpWrite, "%f\n", furTempSet[2]);
			fflush(stdin);
//			fclose(fpWrite);
//			Sleep(100);

			if (optFurTemp == false)
			{
				optFurTemp = true;
			}

			// freeOptSize(NewbtList);

			// 释放复制的钢坯温度内存
			struct btListNode *freepTemp;

			while (NewbtList != NULL)
			{
				freepTemp = NewbtList;
				NewbtList = NewbtList->pNext;
				free(freepTemp->btinfo.btTempDistri);
				free(freepTemp);
			}
			// 内存释放完成
			optRelease = FALSE;

			end1 = clock();
			cost1 = ((double)(end1 - begin1));
			//		cost = ((double)(end - begin)) / CLOCKS_PER_SEC;
			printf("优化一次时间constant CLOCKS_PER_SEC is: %ld, time cost is: %lf secs\n", CLOCKS_PER_SEC, cost1);
			fprintf(fpTime, "%lf\n", cost1);
//			Sleep(10000);
//			PostMessage(AfxGetApp()->m_pMainWnd->m_hWnd, WM_MYTHREADMSG, WM_MYTHREADSUSPEND, NULL);
			end2 = clock();
			cost2 = ((double)(end2 - begin2));

			printf("优化总时间constant CLOCKS_PER_SEC is: %ld, time cost is: %lf secs\n", CLOCKS_PER_SEC, cost2);
			fprintf(fpTime2, "%lf\n", cost2);
			printf("\n\n\n***************************优化完成**************************\n\n\n");
			//WaitForSingleObject(hMainTimerOpt, INFINITE);
		}
	}
	return 0;
}

void ga(struct btListNode *newBtList)
{

	generation = 0;
	generateinitialpopulation();

	begin4 = clock();
	/*待测一次优化所用时间*/

	evaluatepopulation(newBtList);

	end4 = clock();
	cost4 = ((double)(end4 - begin4));
	//		cost = ((double)(end - begin)) / CLOCKS_PER_SEC;
	printf("评价个体花费时间：constant CLOCKS_PER_SEC is: %ld, time cost is: %lf secs\n", CLOCKS_PER_SEC, cost4);
	fprintf(fpCalTime, "%lf\n", cost4);

	//	outputtextreport();
	// 大于迭代次数并且惩罚项 == 0时退出
	while (generation < maxgeneration)
	{
		printf("第******************%d******************次迭代", generation);
		printf("***************************************************\n");
		generation++;

		begin6 = clock();

		begin5 = clock();

		generatenextpopulation();	//生成下一代种群

		end5 = clock();
		cost5 = ((double)(end5 - begin5));
		//		cost = ((double)(end - begin)) / CLOCKS_PER_SEC;
		printf("生成下一代种群时间：constant CLOCKS_PER_SEC is: %ld, time cost is: %lf secs\n", CLOCKS_PER_SEC, cost5);
		fprintf(fpOtherTime, "%lf\n", cost5);


		begin4 = clock();

		evaluatepopulation(newBtList);		//评价个体

		end4 = clock();
		cost4 = ((double)(end4 - begin4));
		//		cost = ((double)(end - begin)) / CLOCKS_PER_SEC;
		printf("评价个体花费时间：constant CLOCKS_PER_SEC is: %ld, time cost is: %lf secs\n", CLOCKS_PER_SEC, cost4);
		fprintf(fpCalTime, "%lf\n", cost4);


		end6 = clock();
		cost6 = ((double)(end6 - begin6));
		//		cost = ((double)(end - begin)) / CLOCKS_PER_SEC;
		printf("循环一次时间constant CLOCKS_PER_SEC is: %ld, time cost is: %lf secs\n", CLOCKS_PER_SEC, cost6);
		fprintf(fpCycleTime, "%lf\n", cost6);

		fprintf(fpInvidSum, "----------------------\n");

		fprintf(fpTemp, "---------------------- \n");
		fprintf(fpInvidSum, "----------------------\n");
		//		outputtextreport();
	}
	fprintf(fpP, "----------------\n");
	fprintf(fpBestofObjectfunction, "------------------------------------\n");


	furTempSet[0] = (float)besttempPara[0];
	furTempSet[1] = (float)besttempPara[1];
	furTempSet[2] = (float)besttempPara[2];

	//fprintf(fpWrite, "%f ", furTempSet[0]);
	//fprintf(fpWrite, "%f ", furTempSet[1]);
	//fprintf(fpWrite, "%f\n", furTempSet[2]);

}



//十进制转二进制
void dec2bin(int decimal, char binary[9])
{
	
	int count = 0;	
	int i;
	while (0 != (decimal / 2))
	{
		if (0 == (decimal % 2))
		{
			binary[count] = '0';
		}
		else
		{
			binary[count] = '1';
		}
		count++;
		decimal /= 2;
	}
	binary[count] = '1';
	if ((count + 1) < 8)
	{
		for (i = count + 1; i < 8; i++)
			binary[i] = '0';
	}

	binary[8] = '\0';

}

//随机生成初始种群
void generateinitialpopulation()
{

	int i, j;
	for (i = 0; i < popsize; i++)
	{
		for (j = 0; j < chromlength; j++)
		{
			population[i].chrom[j] = (rand() % 10 < 5) ? '0' : '1';
			//printf("%c\t", population[i].chrom[j]);
		}
		population[i].chrom[chromlength] = '\0';
	}
}

//对种群进行评价，包含三个函数(求解目标函数值，计算适应度，求最佳和最差个体)
void evaluatepopulation(struct btListNode *pHead)
{
	calculateobjectvalue(pHead);		//解码并计算函数值
	for (int ll = 0; ll < 5; ll++)
	{
		printf("函数值计算完成-----------------------\n");
	}
	calculatefitnessvalue();	//计算适应度
	for (int ll = 0; ll < 5; ll++)
	{
		printf("计算适应度值-----------------------\n");
	}
	findbestandworstindividual();	//求解最佳和最差个体
	for (int ll = 0; ll < 5; ll++)
	{
		printf("求解最佳个体-----------------------\n");
	}
}

//染色体解码，二进制转十进制
long decodechrom(char *string, int point, int length)	//解码染色体(编码字符串、解码初始位置、码长度)
{
	int i;
	long decimal = 0;
	char *pointer;
	for (i = 0, pointer = string + point; i < length; i++, pointer++)
	{
		if (*pointer - '0')
		{
			decimal += (long)pow(2.0, i);
		}
	}
	return decimal;
}

//计算目标函数值
void calculateobjectvalue(struct btListNode *pHead)
{

	int i, j, k, n;
	struct btListNode *pTemp;

	long decimal;
	double furnaceTemp[3];	//存储个体由二进制转化为十进制所代表的三段炉温
	double para[3];				//拟合炉温二次函数系数
	int leftTime;			//剩余在炉时间
	double site;			//钢坯位置
	int N;					//钢坯分层数
	double errSection;		//存储钢坯出炉表面温度与中心温度差
	double errSurface;		//存储钢坯出炉表面温度与目标温度差
	double errActually;    //存储钢温实测与预测的温差
	double penaltySection;	//根据预测断面温差和约束条件，计算惩罚项(1-7:20度;8-16:25度;17-22:35度)
	double penaltySurface;	//根据预测的钢坯表面温度与目标温差和约束条件，计算惩罚项(1-7:20度;8-16:25度;17-22:30度)
	double penaltyActually; //钢坯实测与钢坯出炉表面温度之间的实测
	double penaltyheat;		//根据预测钢坯表面温度与在加热段的目标值约束，计算在加热段的惩罚项
	//double sum;				//存储22块钢坯表面温度和
	double sumErr;			//存储22块钢坯温差和
//	double sumAtual_pred;
//	struct btListNode *NewbtListCopy;

	for (i = 0; i < popsize; i++)
	{
		pTemp = pHead;
		sumErr = 0;
		penaltySection = 0;	
		penaltySurface = 0;
		penaltyActually = 0;
		penaltyheat = 0;


		for (j = 0; j < 3; j++)
		{
			decimal = decodechrom(population[i].chrom, j*CodeL, CodeL);	
			furnaceTemp[j] = decimal * (uplimit[j] - downlimit[j]) / (pow(2.0, CodeL) - 1) + downlimit[j];	
		}


		polyfit(sizeof(thermocouple) / sizeof(thermocouple[0]), thermocouple, furnaceTemp, 2, para);

		double gas_sum = 0.0;

		//炉温积分
		for (double bt_site = 0; bt_site < furnaceSize[0]; bt_site += 0.1)
		{
			gas_sum += (((para[0] * (bt_site + 1)*(bt_site + 1) + para[1] * (bt_site + 1) + para[2]) + (para[0] * (bt_site)*(bt_site)+para[1] * (bt_site)+para[2]))*0.1/2);
		}
		fprintf(fpTemp, "第%d个 ", i);
		fprintf(fpTemp, "%f ", furnaceTemp[0]);
		fprintf(fpTemp, "%f ", furnaceTemp[1]);
		fprintf(fpTemp, "%f\n", furnaceTemp[2]);

		fprintf(fpPreBtOutTemp, "%f ", furnaceTemp[0]);
		fprintf(fpPreBtOutTemp, "%f ", furnaceTemp[1]);
		fprintf(fpPreBtOutTemp, "%f\n", furnaceTemp[2]);
		int num = 0;
		while (pTemp != NULL)
		{
			num++;
			site = pTemp->btinfo.site;
			leftTime = (int)((furnaceSize[0] - site) / optv / 2);
			// printf("距离出炉时间间隔%d个体为%d:\n", leftTime, i);
			N = pTemp->btinfo.dzCount;
			double *btTemp = (double*)malloc(sizeof(double) * N);
			double *prebtTemp = (double*)malloc(sizeof(double) * N);


			for (k = 0; k < N; k++)	
			{
				btTemp[k] = pTemp->btinfo.btTempDistri[k];
				prebtTemp[k] = pTemp->btinfo.btTempDistri[k];

			}
	

			if (leftTime < 1)
			{
				continue;
			}
			else
			{
				for (n = 1; n < leftTime; n++)
				{

					site += optv * 2;

					double temp = para[0] * site * site + para[1] * site + para[2];	
					double eco2 = 7.17 * pow((0.13*5.56), (1 / 3.0)) / sqrt(temp);
					double eh2o = 7.17 * 0.11 * 5.56 * pow(5.56, 0.6) / temp;
					double airBlackness = eco2 + 1.05 * eh2o;



					double flux_up;	//上部热流密度
					double flux_down; // 下部热流

					// 上部热流
					flux_up = (Stefan_Boltzmann*pTemp->btinfo.btpysicalinfo.btBlackness*airBlackness*(1 + pTemp->btinfo.btpysicalinfo.btAngle *(1 - airBlackness))*(pow((temp + 273), 4) - pow((prebtTemp[0] + 273), 4))) /
						(airBlackness + pTemp->btinfo.btpysicalinfo.btAngle * (1 - airBlackness)*(pTemp->btinfo.btpysicalinfo.btBlackness + (1 - pTemp->btinfo.btpysicalinfo.btBlackness) * airBlackness));
					// 下部热流
					flux_down = (Stefan_Boltzmann*pTemp->btinfo.btpysicalinfo.btBlackness*airBlackness*(1 + pTemp->btinfo.btpysicalinfo.btAngle *(1 - airBlackness))*(pow((temp + 273), 4) - pow((prebtTemp[N - 1] + 273), 4))) /
						(airBlackness + pTemp->btinfo.btpysicalinfo.btAngle  * (1 - airBlackness)*(pTemp->btinfo.btpysicalinfo.btBlackness + (1 - pTemp->btinfo.btpysicalinfo.btBlackness) * airBlackness));
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
					//钢温积分
					//sum += (btTemp[0] + prebtTemp[0]) * optv * dt / 2;
					

					for (int l = 0; l < N; l++)
					{
						prebtTemp[l] = btTemp[l];
					}
					
				}
				//sum += 500 * penaltyheat;
				//penaltyheat = 0;
			}

//			sum += pTemp->btinfo.sum;



			//约束条件
			if (N % 2 == 0)
			{
				errSection = fabs(btTemp[0] - (btTemp[N / 2] + btTemp[N / 2 - 1]) / 2);
				errSurface = fabs(btTemp[0] - pTemp->btinfo.bttarget.btTargetTemp);
				errActually = fabs(btTemp[0] - AtuallyTemp);
			}
			else
			{
				errSection = fabs(btTemp[0] - btTemp[(N - 1) / 2]);
				errSurface = fabs(btTemp[0] - pTemp->btinfo.bttarget.btTargetTemp);
				errActually = fabs(btTemp[0] - AtuallyTemp);
			}
			//	断面、表面、实测
			if (pTemp->btinfo.site <= 1 / 3.0 * furnaceSize[0])
			{
				penaltySection += (errSection < restrictSection[0]) ? 0 : (errSection - restrictSection[0]);
				penaltySurface += (errSurface < restrictSurface[0]) ? 0 : (errSurface - restrictSurface[0]);
				penaltyActually += (errActually < restrictActually[0]) ? 0 : (errActually - restrictActually[0]);
			}
			else if (pTemp->btinfo.site <= 2 / 3.0 * furnaceSize[0])
			{
				penaltySection += (errSection < restrictSection[1]) ? 0 : (errSection - restrictSection[1]);
				penaltySurface += (errSurface < restrictSurface[1]) ? 0 : (errSurface - restrictSurface[1]);
				penaltyActually += (errActually < restrictActually[1]) ? 0 : (errActually - restrictActually[1]);
			}
			else
			{
				penaltySection += (errSection < restrictSection[2]) ? 0 : (errSection - restrictSection[2]);
				penaltySurface += (errSurface < restrictSurface[2]) ? 0 : (errSurface - restrictSurface[2]);
				penaltyActually += (errActually < restrictActually[2]) ? 0 : (errActually - restrictActually[2]);
			}

			sumErr += errSection + errSurface + errActually;

			fprintf(fpPreBtOutTemp, "%f ", btTemp[0]);
			fprintf(fpPreBtOutTemp, "%f\n", btTemp[13]);
			
			free(btTemp);
			free(prebtTemp);
			pTemp = pTemp->pNext;

			
		}
		//Sleep(1000);
		fprintf(fpPreBtOutTemp, "\n");
		fprintf(fpInvidSum, "第几个%d\n ",i);		//写入炉温积分
		fprintf(fpInvidSum, "gas_sum: %lf ", gas_sum);		//写入炉温积分
		fprintf(fpInvidSum, "sumErr: %lf ", sumErr);
		fprintf(fpInvidSum, "penaltySection: %lf ", penaltySection);
		fprintf(fpInvidSum, "penaltySurface: %lf ", penaltySurface);
		fprintf(fpInvidSum, "penaltyActually: %lf ", penaltyActually);
		fprintf(fpInvidSum, "\n");

		population[i].targetfunction = gas_sum;
		int factor1 = 100;
		int factor2 = 500;
		int factor3 = 300;

		if (penaltySection < 10)
		{
			factor1 = 10000;
		}
		if (penaltyActually < 10)
		{
			factor2 = 10000;
		}
		if (penaltySurface < 10)
		{
			factor3 = 10000;
		}

		penalty[i] =  factor1*penaltySection +  factor2*penaltySurface +  factor3*penaltyActually;

		population[i].P = penalty[i];
		population[i].targetfunction = population[i].targetfunction + penalty[i];
		//population[i].targetfunction = population[i].targetfunction;

	}

	fprintf(fpPreBtOutTemp, "\n");
	fprintf(fpPreBtOutTemp, "\n");
	fprintf(fpPreBtOutTemp, "\n");
}

void calculatefitnessvalue()	//计算适应度
{
	int i;
	double max;
	double min;
	double mean = 0;
	double sum = 0;

	max = population[0].targetfunction;
	min = population[0].targetfunction;

	for (i = 1; i < popsize; i++)
	{
		if (population[i].targetfunction > max)
		{
			max = population[i].targetfunction;
		}
	}

	for (i = 1; i < popsize; i++)
	{
		if (population[i].targetfunction < min)
		{
			min = population[i].targetfunction;
		}
	}

	for (i = 0; i < popsize; i++)
	{
		sum += population[i].targetfunction;
	}
	mean = sum / popsize;

//	fprintf(fpBestofObjectfunction, "%f ", max);
	

	//计算个体适应度
	for (i = 0; i < popsize; i++)
	{
		// population[i].fitness = max - population[i].targetfunction - penalty[i];
		population[i].fitness = max - population[i].targetfunction;
		//population[i].fitness = 2*exp(-sqrt(population[i].targetfunction/mean))/(1+exp(-sqrt(population[i].targetfunction/mean)));
		//population[i].fitness = population[i].targetfunction -  1/(population[i].P +1);
		// population[i].fitness = (pow(10.0, 9)) / population[i].targetfunction;
		fprintf(fpIndividual, "个体%d ", i);
		fprintf(fpIndividual, "%f ", max);
		fprintf(fpIndividual, "%f ", population[i].targetfunction);
		fprintf(fpIndividual, "%f ", penalty[i]);
		fprintf(fpIndividual, "%f\n", population[i].fitness);

	}
	//fprintf(fpIndividual, "\n");


	int x = 0;
	double y = population[0].fitness;
	for (i = 1; i < popsize; i++)
	{
		if (population[i].fitness > y)
		{
			x = i;
			y = population[i].fitness;
		}
	}
	fprintf(fpIndividual, "当代最优个体为第%d个\n", x);
	printf("------------当代最优个体为第%d个------------\n", x);
	// Sleep(100);

}


void findbestandworstindividual()	//求解最佳和最差个体
{
	int i, j;
	long decimal;

	best_index = 0;

	worst_index = 0;
	//对个体按照适应度从小到大进行排序，选出最佳个体个最差个体

	for (i = 0; i < popsize - 1; i++)
	{
		for (j = 0; j < popsize - 1 - i; j++)
		{
			if (population[j].fitness > population[j + 1].fitness)
			{
				bestindividual = population[j];
				population[j] = population[j + 1];
				population[j + 1] = bestindividual;
			}
		}
	}

	bestindividual = population[popsize - 1];


	for (j = popsize - 2; j >= 0; j--)
	{
		if (bestindividual.fitness != population[j].fitness)
		{
			secondindividual = population[j];		//次优个体
			break;
		}
	}

	printf("s次优%f\n ", secondindividual.fitness);
	printf("最优%f \n", bestindividual.fitness);
	//Sleep(1000);


	//for (i = 0; i < popsize - 1; i++) 
	//{
	//	for (j = 0; j < popsize - 1 - i; j++)
	//	{
	//		if (population[j].fitness < population[j + 1].fitness)
	//		{
	//			bestindividual = population[j];
	//			population[j] = population[j + 1];
	//			population[j + 1] = bestindividual;
	//		}
	//	}
	//}

	//bestindividual = population[0];


	//将当前代最佳个体转换成十进制

	for (j = 0; j < 3; j++)
	{
		decimal = decodechrom(bestindividual.chrom, j*CodeL, CodeL);	//解码
		besttempPara[j] = decimal * double(uplimit[j] - downlimit[j]) / (pow(2.0, CodeL) - 1) + downlimit[j];	//求炉温
	}

	///////////////////////////////////////////////////////////////////
	//////////////////////调试用///////////////////////////////////////

	fprintf(fpBestvidualofTemp, "%f ", besttempPara[0]);
	fprintf(fpBestvidualofTemp, "%f ", besttempPara[1]);
	fprintf(fpBestvidualofTemp, "%f\n", besttempPara[2]);
	fprintf(fpBestofObjectfunction, "%f\n", bestindividual.targetfunction);
	fprintf(fpBestofFitness, "%f\n", bestindividual.fitness);
	fprintf(fpP, "%f\n", bestindividual.P);
	printf("最优值为%lf\t%lf\t%lf\n", besttempPara[0], besttempPara[1], besttempPara[2]);

	///////////////////////////////////////////////////////////////////
	//////////////////////调试用///////////////////////////////////////

	if (0 == generation)
	{
		previousbest = bestindividual;
		//for (j = 0; j < 3; j++)
		//{
		//	previousbesttempPara[j] = besttempPara[j];
		//}
	}
	else
	{

		if (previousbest.targetfunction != bestindividual.targetfunction)
		{
			secondindividual = previousbest;

		}
		/*for (j = 0; j < 3; j++)
		{
			if (0 != previousbesttempPara[j] - besttempPara[j])	
			{
				errbest[j] = fabs(previousbesttempPara[j] - besttempPara[j]);
			}
			else	
			{
				errbest[j] = errbest[j];
			}
		}*/
		previousbest = bestindividual;
		//for (j = 0; j < 3; j++)
		//{
		//	previousbesttempPara[j] = besttempPara[j];
		//}
	}
}

void generatenextpopulation()	
{
	selectopeator();		//选择操作
	crossoveroperator();	//交叉操作
	mutationoperator();		//变异操作
	if (generation > 1)
	{
		forecastbest();		//预测替换
	}

	population[0] = bestindividual;

}

void selectopeator()	//选择操作(比例选择)
{
	int i, j;
	int count = 2;
	double sum = 0.0;
	int pfitness;		//选择个数
	struct individual newpopulation[POPSIZE];
	for (i = 2; i < popsize; i++)
	{
		sum += population[i].fitness;
	}

	for (i = 2; i < popsize; i++)
	{
		pfitness = (int)(floor(population[i].fitness / sum * popsize));
		for (j = 0; j < pfitness; j++)
		{
			newpopulation[count] = population[i];
			count++;
		}
	}

	while (count < popsize)
	{
		for (j = 0; j < chromlength; j++)
		{
			newpopulation[count].chrom[j] = (rand() % 10 < 5) ? '0' : '1';
		}
		newpopulation[count].chrom[chromlength] = '\0';
		count++;
	}
	
	//将新生成的种群赋给原种群
	for (i = 2; i < popsize; i++)
	{
		population[i] = newpopulation[i-2];
	}


}

void crossoveroperator()	//交叉操作
{
	int i, j, k;
	int point, temp;
	int len, end;
	int index[POPSIZE];
	double p;	//随机概率
	//	char ch;
	struct individual newpopulation[POPSIZE];
	for (i = 0; i < popsize; i++)
	{
		index[i] = i;
	}
	//随机打乱索引号,未随机交叉做准备
	for (i = 0; i < popsize; i++)
	{
		point = rand() % (popsize - i);	//随机产生0 ~(popsize - i)的数字
		temp = index[i];
		index[i] = index[point + i];
		index[point + i] = temp;
	}

	for (i = 0; i < popsize; i++)
	{
		newpopulation[i] = population[i];
	}
	for (i = 0; i < popsize; i++)
	{
		p = rand() % 1000 / 1000.0;
		if (p < pc)
		{
			//不同段之间分别交叉
			for (k = 0; k < ParaC; k++)
			{
				point = rand() % CodeL;				//每段的交叉起点(随机产生0-7)
				len = rand() % (CodeL - point);		//交叉长度
				end = point + len + k * CodeL;		//每段交叉终点
				for (j = point + k * CodeL; j <= end; j++)
				{
					population[i].chrom[j] = newpopulation[index[i]].chrom[j];
				}
			}
		}
	}
}
void mutationoperator()		//变异操作
{
	int i, j;
	double p;	//产生随机概率与变异概率作比较，若小于pm，则变异
	for (i = 0; i < popsize; i++)
	{
		for (j = 0; j < chromlength; j++)
		{
			p = rand() % 1000 / 1000.0;
			if (p < pm)
				population[i].chrom[j] = (population[i].chrom[j] == '0') ? '1' : '0';
		}
	}
}

//从第9代开始，根据前8代产生的最优值，预测下一代的最优值，并将最优值替换掉第9代中的最差的8个个体
void forecastbest()
{
	
	double forcastpreheatDetla[3];

	int forcastpreheat[3];

	// bestindividual
	// secondindividual
	int j;
	long second_dec;
	char binary[9];

	for (j = 0; j < 3; j++)
	{
		second_dec = decodechrom(secondindividual.chrom, j*CodeL, CodeL);	//解码
		SecondbesttempPara[j] = second_dec * double(uplimit[j] - downlimit[j]) / (pow(2.0, CodeL) - 1) + downlimit[j];	//求炉温
	}


	for (j = 0; j < 3; j++)
	{
		forcastpreheatDetla[j] = besttempPara[j] + (rand() % 1000 / 1000.0)*(besttempPara[j] - SecondbesttempPara[j]);
	}

	//编码,小于上下线

	//与热度那
	if (forcastpreheatDetla[0] > uplimit[0])
		{
			forcastpreheatDetla[0] = uplimit[0];
		}
	else if (forcastpreheatDetla[0] < downlimit[0])
		{
			forcastpreheatDetla[0] = downlimit[0];
		}
	forcastpreheat[0] = round((forcastpreheatDetla[0] - downlimit[0]) / JDpreheating);
	// 加热短
	if (forcastpreheatDetla[1] > uplimit[1])
	{
		forcastpreheatDetla[1] = uplimit[1];
	}
	else if (forcastpreheatDetla[1] < downlimit[1])
	{
		forcastpreheatDetla[1] = downlimit[1];
	}
	forcastpreheat[1] = round((forcastpreheatDetla[1] - downlimit[1]) / JDheating);
	//均热
	if (forcastpreheatDetla[2] > uplimit[2])
	{
		forcastpreheatDetla[2] = uplimit[2];
	}
	else if (forcastpreheatDetla[2] < downlimit[2])
	{
		forcastpreheatDetla[2] = downlimit[2];
	}
	forcastpreheat[2] = round((forcastpreheatDetla[2] - downlimit[2]) / JDsoking);


	fprintf(fpforcastTemp, "当前带最优");
	for (int i = 0; i < 3; i++)
	{
		fprintf(fpforcastTemp, " %f ", besttempPara[i]);
		
	}

	fprintf(fpforcastTemp, " 次优");
	for (int i = 0; i < 3; i++)
	{
	fprintf(fpforcastTemp, " %f ", SecondbesttempPara[i]);
	}
	fprintf(fpforcastTemp, " 预测");
	for (int i = 0; i < 3; i++)
	{
		fprintf(fpforcastTemp, " %f ", forcastpreheatDetla[i]);
	}
	fprintf(fpforcastTemp, "\n");
	

	//前8位编码(预热段)

	dec2bin(forcastpreheat[0], binary);
	for (j = 0; j < 8; j++)
	{
		forcastpopulation.chrom[j] = binary[j];
	}

	//中间8位编码(加热段）
	dec2bin(forcastpreheat[1], binary);
	for (j = 8; j < 16; j++)
	{
		forcastpopulation.chrom[j] = binary[j - 8];
	}
	//后8位编码(均热段)
	dec2bin(forcastpreheat[1], binary);
	for (j = 16; j < 24; j++)
	{
		forcastpopulation.chrom[j] = binary[j - 16];
	}
	//最后一位赋值'\0'，因为是字符串
	forcastpopulation.chrom[chromlength] = '\0';
	population[1] = forcastpopulation;


}

//输出当前代数，最优值
void outputtextreport()
{
	printf("当前代=%d\n当前代最优三段炉温为:%lf\t%lf\t%lf\t%lf\n",
		generation, besttempPara[0], besttempPara[1], besttempPara[2], bestindividual.fitness);
}

