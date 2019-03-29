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


#define POPSIZE 500			//�����Ⱥ����
#define CodeL 8				//���볤��
#define ParaC 3				//��������
#define chromlength CodeL*ParaC			//Ⱦɫ����볤��
#define maxgeneration 50	//����������
int popsize = 20;			//��Ⱥ��
double pc = 1.0;			//������
//double pm = 0.095;			//������
double pm = 0.1;			//������

//������壬����Ⱦɫ�塢��Ӧ�ȡ�ʵ��ֵ
struct individual
{
	char chrom[chromlength + 1];		//Ⱦɫ�����
	double fitness;					//��Ӧ��
	double targetfunction;			//Ŀ�꺯��ֵ
	double P;						//�ͷ���
	//	double dechrom[ParaC];			//�����������¶�

};

int generation;					//��������
int best_index;					//��Ѹ���������
int worst_index;				//������������
//int worst_index[9];				//9���������������
//double tempPara[ParaC];			//��ʱ�洢����¯��(����Ŀ�꺯��ʱ)
double besttempPara[ParaC];		//�洢��ǰ�����¯��
double previousbesttempPara[ParaC];	//�洢ǰһ�����¯��
double SecondbesttempPara[ParaC];	//�洢�ڶ��Ÿ���¯��

double errbest[ParaC] = { 5.0, 5.0, 5.0 };			//��ǰ������һ������ֵ(����)�Ĳ�ֵ

struct individual bestindividual;		//��Ѹ���
struct individual worstindividual;		//������
struct individual secondindividual;		//�ڶ�����

//struct individual worstindividual[9];	//�ҳ�����9������(����8����Ԥ��ֵ�滻��1����ǰһ�����ֵ�滻)
struct individual previousbest;			//ǰһ����Ѹ���
//struct individual currentbest;			//��ǰ����֮ǰ�е���Ѹ���
struct individual population[POPSIZE];	//POPSIZE������
//struct individual forcastpopulation[8];	//Ԥ���8����Ѹ���
struct individual forcastpopulation;	//Ԥ���1������

double penalty[POPSIZE];

float uplimit[ParaC] = { 1100, 1350, 1330 };	//Ԥ�ȶΡ����ȶΡ����ȶ��¶�����
float downlimit[ParaC] = { 850, 1150, 1130 };	//Ԥ�ȶΡ����ȶΡ����ȶ��¶�����
float JDpreheating = (float)((uplimit[0] - downlimit[0]) / (pow(2.0, CodeL) - 1));	//Ԥ�ȶα��뾫��
float JDheating = (float)((uplimit[1] - downlimit[1]) / (pow(2.0, CodeL) - 1));		//���ȶα��뾫��
float JDsoking = (float)((uplimit[2] - downlimit[2]) / (pow(2.0, CodeL) - 1));		//���ȶα��뾫��

double restrictSection[3] = { 20, 10, 10 };	//Ԥ���¯�����²�Լ������
double restrictSurface[3] = { 20, 10, 10 };		//Ԥ���¯�����¶�Լ������
double restrictActually[3] = { 20, 10, 10 };	//ʵ����Ԥ��

double optv;		//������¯�������ٶ�


double AtuallyTemp = 1120;	//ʵ�������¯�¶�

///���ں˶�ʱ��
HANDLE hMainTimerOpt = NULL;


/////////////////////////////////////////////������//////////////////////////////////////////////
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

	//��ʱ������
	int calPrd = 4000;
	//���ö�ʱ��
	LARGE_INTEGER li;
	li.QuadPart = -1;
	//�����ں˶�ʱ��
	hMainTimerOpt = CreateWaitableTimer(NULL, 0, NULL);
	//�����ں˶�ʱ��
	SetWaitableTimer(hMainTimerOpt, &li, calPrd, NULL, NULL, 0);

	srand((unsigned) time(NULL));

//////////////////////////////////////////������//////////////////////////////////////////////////////
	clock_t begin1, end1;
	double cost1;

	clock_t begin2, end2;
	double cost2;

	clock_t begin3, end3;
	double cost3;


	fpWrite = fopen("doptFurTemp.txt", "w");	//�Ż���������¯���趨ֵ
	fpBestvidualofTemp = fopen("dbestIndividulaFurTemp.txt", "w");	//ÿһ�����¯��
	fpBestofObjectfunction = fopen("dBestofObjectfunction.txt", "w");	//��Ѹ����Ŀ�꺯��ֵ
	fpBestofFitness = fopen("dBestofFitness.txt", "w");					//��ѵ���Ӧ��ֵ
	fpPreBtOutTemp = fopen("dPreBtOutTemp.txt", "w");					//ÿ�������µ�¯�����ĸ����ĳ�¯�¶�
	fpIndividual = fopen("dfpIndividual.txt", "w");						//ÿ���������Ӧ��
	fpTime = fopen("dtime.txt", "w");									//�Ż�һ�ε�ʱ��
	fpTime2 = fopen("dtime2.txt", "w");									//�Ż���ʱ�䣬�ӵȴ�������
	fpTime3 = fopen("dtime3.txt", "w");									//�Ż��ȴ�ʱ��


	fpCalTime = fopen("dcalTime.txt", "w");								//���۸��廨��ʱ��
	fpOtherTime = fopen("dotherTime.txt", "w");							//������Ⱥʱ��
	fpCycleTime = fopen("dcyleTime.txt", "w");							//����һ����Ҫ��ʱ��

	fpInvidSum = fopen("fpInvidSum.txt", "w");

	fpforcastTemp = fopen("fpforcastTemp.txt", "w");		//Ԥ���¶�
	fpTemp = fopen("fpTemp.txt", "w");						//ÿһ�����¶�
	fpP = fopen("fpP.txt", "w");							// Ԥ�ͷ���
	WaitForSingleObject(hMainTimerOpt, INFINITE);

	begin3 = clock();

	/*�����Գ����*/
	while (1)
	{
//		fpWrite = fopen("data.txt", "a");		

		if (initterminalFlag == true)
		{
	
			begin2 = clock();
			/*�����Գ����*/
		
			printf("\n\n\n########################�Ż��̵߳ȴ���Դ#################\n\n\n");
			WaitForSingleObject(smphPrint, INFINITE); // �ȴ��Թ�����Դ����ͨ��������P����

			begin1 = clock();
			/*����һ���Ż�����ʱ��*/

			end3 = clock();
			cost3 = ((double)(end3 - begin3));
			//		cost = ((double)(end - begin)) / CLOCKS_PER_SEC;
			printf("constant CLOCKS_PER_SEC is: %ld, time cost is: %lf secs\n", CLOCKS_PER_SEC, cost3);
			fprintf(fpTime3, "%lf\n", cost3);

			begin3 = clock();
			/*���������Ż�֮��ʱ����*/

			NewbtList = listCopy(pHead);

			// ������ɣ��ռ�û���ͷ�
			optRelease = TRUE;
			optv = v;
			printf("\n\n\n########################�������#################\n\n\n");
			

			ReleaseSemaphore(smphPrint, 1, NULL); // �ͷŹ�����Դ ���� V����

			
			printf("��ʼ�Ż�--------------**************************************\n");

//			begin1 = clock();
//			/*�����Գ����*/
			
			ga(NewbtList);
			//���ͼ��������Ϣ
			PostMessage(AfxGetApp()->m_pMainWnd->m_hWnd, WM_MYSETMSG, NULL, NULL);
			printf("----------------�Ż����****������¯��Ϊ:\n%f\t%f\t%f\n", furTempSet[0], furTempSet[1], furTempSet[2]);

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

			// �ͷŸ��Ƶĸ����¶��ڴ�
			struct btListNode *freepTemp;

			while (NewbtList != NULL)
			{
				freepTemp = NewbtList;
				NewbtList = NewbtList->pNext;
				free(freepTemp->btinfo.btTempDistri);
				free(freepTemp);
			}
			// �ڴ��ͷ����
			optRelease = FALSE;

			end1 = clock();
			cost1 = ((double)(end1 - begin1));
			//		cost = ((double)(end - begin)) / CLOCKS_PER_SEC;
			printf("�Ż�һ��ʱ��constant CLOCKS_PER_SEC is: %ld, time cost is: %lf secs\n", CLOCKS_PER_SEC, cost1);
			fprintf(fpTime, "%lf\n", cost1);
//			Sleep(10000);
//			PostMessage(AfxGetApp()->m_pMainWnd->m_hWnd, WM_MYTHREADMSG, WM_MYTHREADSUSPEND, NULL);
			end2 = clock();
			cost2 = ((double)(end2 - begin2));

			printf("�Ż���ʱ��constant CLOCKS_PER_SEC is: %ld, time cost is: %lf secs\n", CLOCKS_PER_SEC, cost2);
			fprintf(fpTime2, "%lf\n", cost2);
			printf("\n\n\n***************************�Ż����**************************\n\n\n");
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
	/*����һ���Ż�����ʱ��*/

	evaluatepopulation(newBtList);

	end4 = clock();
	cost4 = ((double)(end4 - begin4));
	//		cost = ((double)(end - begin)) / CLOCKS_PER_SEC;
	printf("���۸��廨��ʱ�䣺constant CLOCKS_PER_SEC is: %ld, time cost is: %lf secs\n", CLOCKS_PER_SEC, cost4);
	fprintf(fpCalTime, "%lf\n", cost4);

	//	outputtextreport();
	// ���ڵ����������ҳͷ��� == 0ʱ�˳�
	while (generation < maxgeneration)
	{
		printf("��******************%d******************�ε���", generation);
		printf("***************************************************\n");
		generation++;

		begin6 = clock();

		begin5 = clock();

		generatenextpopulation();	//������һ����Ⱥ

		end5 = clock();
		cost5 = ((double)(end5 - begin5));
		//		cost = ((double)(end - begin)) / CLOCKS_PER_SEC;
		printf("������һ����Ⱥʱ�䣺constant CLOCKS_PER_SEC is: %ld, time cost is: %lf secs\n", CLOCKS_PER_SEC, cost5);
		fprintf(fpOtherTime, "%lf\n", cost5);


		begin4 = clock();

		evaluatepopulation(newBtList);		//���۸���

		end4 = clock();
		cost4 = ((double)(end4 - begin4));
		//		cost = ((double)(end - begin)) / CLOCKS_PER_SEC;
		printf("���۸��廨��ʱ�䣺constant CLOCKS_PER_SEC is: %ld, time cost is: %lf secs\n", CLOCKS_PER_SEC, cost4);
		fprintf(fpCalTime, "%lf\n", cost4);


		end6 = clock();
		cost6 = ((double)(end6 - begin6));
		//		cost = ((double)(end - begin)) / CLOCKS_PER_SEC;
		printf("ѭ��һ��ʱ��constant CLOCKS_PER_SEC is: %ld, time cost is: %lf secs\n", CLOCKS_PER_SEC, cost6);
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



//ʮ����ת������
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

//������ɳ�ʼ��Ⱥ
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

//����Ⱥ�������ۣ�������������(���Ŀ�꺯��ֵ��������Ӧ�ȣ�����Ѻ�������)
void evaluatepopulation(struct btListNode *pHead)
{
	calculateobjectvalue(pHead);		//���벢���㺯��ֵ
	for (int ll = 0; ll < 5; ll++)
	{
		printf("����ֵ�������-----------------------\n");
	}
	calculatefitnessvalue();	//������Ӧ��
	for (int ll = 0; ll < 5; ll++)
	{
		printf("������Ӧ��ֵ-----------------------\n");
	}
	findbestandworstindividual();	//�����Ѻ�������
	for (int ll = 0; ll < 5; ll++)
	{
		printf("�����Ѹ���-----------------------\n");
	}
}

//Ⱦɫ����룬������תʮ����
long decodechrom(char *string, int point, int length)	//����Ⱦɫ��(�����ַ����������ʼλ�á��볤��)
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

//����Ŀ�꺯��ֵ
void calculateobjectvalue(struct btListNode *pHead)
{

	int i, j, k, n;
	struct btListNode *pTemp;

	long decimal;
	double furnaceTemp[3];	//�洢�����ɶ�����ת��Ϊʮ���������������¯��
	double para[3];				//���¯�¶��κ���ϵ��
	int leftTime;			//ʣ����¯ʱ��
	double site;			//����λ��
	int N;					//�����ֲ���
	double errSection;		//�洢������¯�����¶��������¶Ȳ�
	double errSurface;		//�洢������¯�����¶���Ŀ���¶Ȳ�
	double errActually;    //�洢����ʵ����Ԥ����²�
	double penaltySection;	//����Ԥ������²��Լ������������ͷ���(1-7:20��;8-16:25��;17-22:35��)
	double penaltySurface;	//����Ԥ��ĸ��������¶���Ŀ���²��Լ������������ͷ���(1-7:20��;8-16:25��;17-22:30��)
	double penaltyActually; //����ʵ���������¯�����¶�֮���ʵ��
	double penaltyheat;		//����Ԥ����������¶����ڼ��ȶε�Ŀ��ֵԼ���������ڼ��ȶεĳͷ���
	//double sum;				//�洢22����������¶Ⱥ�
	double sumErr;			//�洢22������²��
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

		//¯�»���
		for (double bt_site = 0; bt_site < furnaceSize[0]; bt_site += 0.1)
		{
			gas_sum += (((para[0] * (bt_site + 1)*(bt_site + 1) + para[1] * (bt_site + 1) + para[2]) + (para[0] * (bt_site)*(bt_site)+para[1] * (bt_site)+para[2]))*0.1/2);
		}
		fprintf(fpTemp, "��%d�� ", i);
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
			// printf("�����¯ʱ����%d����Ϊ%d:\n", leftTime, i);
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



					double flux_up;	//�ϲ������ܶ�
					double flux_down; // �²�����

					// �ϲ�����
					flux_up = (Stefan_Boltzmann*pTemp->btinfo.btpysicalinfo.btBlackness*airBlackness*(1 + pTemp->btinfo.btpysicalinfo.btAngle *(1 - airBlackness))*(pow((temp + 273), 4) - pow((prebtTemp[0] + 273), 4))) /
						(airBlackness + pTemp->btinfo.btpysicalinfo.btAngle * (1 - airBlackness)*(pTemp->btinfo.btpysicalinfo.btBlackness + (1 - pTemp->btinfo.btpysicalinfo.btBlackness) * airBlackness));
					// �²�����
					flux_down = (Stefan_Boltzmann*pTemp->btinfo.btpysicalinfo.btBlackness*airBlackness*(1 + pTemp->btinfo.btpysicalinfo.btAngle *(1 - airBlackness))*(pow((temp + 273), 4) - pow((prebtTemp[N - 1] + 273), 4))) /
						(airBlackness + pTemp->btinfo.btpysicalinfo.btAngle  * (1 - airBlackness)*(pTemp->btinfo.btpysicalinfo.btBlackness + (1 - pTemp->btinfo.btpysicalinfo.btBlackness) * airBlackness));
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
					//���»���
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



			//Լ������
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
			//	���桢���桢ʵ��
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
		fprintf(fpInvidSum, "�ڼ���%d\n ",i);		//д��¯�»���
		fprintf(fpInvidSum, "gas_sum: %lf ", gas_sum);		//д��¯�»���
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

void calculatefitnessvalue()	//������Ӧ��
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
	

	//���������Ӧ��
	for (i = 0; i < popsize; i++)
	{
		// population[i].fitness = max - population[i].targetfunction - penalty[i];
		population[i].fitness = max - population[i].targetfunction;
		//population[i].fitness = 2*exp(-sqrt(population[i].targetfunction/mean))/(1+exp(-sqrt(population[i].targetfunction/mean)));
		//population[i].fitness = population[i].targetfunction -  1/(population[i].P +1);
		// population[i].fitness = (pow(10.0, 9)) / population[i].targetfunction;
		fprintf(fpIndividual, "����%d ", i);
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
	fprintf(fpIndividual, "�������Ÿ���Ϊ��%d��\n", x);
	printf("------------�������Ÿ���Ϊ��%d��------------\n", x);
	// Sleep(100);

}


void findbestandworstindividual()	//�����Ѻ�������
{
	int i, j;
	long decimal;

	best_index = 0;

	worst_index = 0;
	//�Ը��尴����Ӧ�ȴ�С�����������ѡ����Ѹ����������

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
			secondindividual = population[j];		//���Ÿ���
			break;
		}
	}

	printf("s����%f\n ", secondindividual.fitness);
	printf("����%f \n", bestindividual.fitness);
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


	//����ǰ����Ѹ���ת����ʮ����

	for (j = 0; j < 3; j++)
	{
		decimal = decodechrom(bestindividual.chrom, j*CodeL, CodeL);	//����
		besttempPara[j] = decimal * double(uplimit[j] - downlimit[j]) / (pow(2.0, CodeL) - 1) + downlimit[j];	//��¯��
	}

	///////////////////////////////////////////////////////////////////
	//////////////////////������///////////////////////////////////////

	fprintf(fpBestvidualofTemp, "%f ", besttempPara[0]);
	fprintf(fpBestvidualofTemp, "%f ", besttempPara[1]);
	fprintf(fpBestvidualofTemp, "%f\n", besttempPara[2]);
	fprintf(fpBestofObjectfunction, "%f\n", bestindividual.targetfunction);
	fprintf(fpBestofFitness, "%f\n", bestindividual.fitness);
	fprintf(fpP, "%f\n", bestindividual.P);
	printf("����ֵΪ%lf\t%lf\t%lf\n", besttempPara[0], besttempPara[1], besttempPara[2]);

	///////////////////////////////////////////////////////////////////
	//////////////////////������///////////////////////////////////////

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
	selectopeator();		//ѡ�����
	crossoveroperator();	//�������
	mutationoperator();		//�������
	if (generation > 1)
	{
		forecastbest();		//Ԥ���滻
	}

	population[0] = bestindividual;

}

void selectopeator()	//ѡ�����(����ѡ��)
{
	int i, j;
	int count = 2;
	double sum = 0.0;
	int pfitness;		//ѡ�����
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
	
	//�������ɵ���Ⱥ����ԭ��Ⱥ
	for (i = 2; i < popsize; i++)
	{
		population[i] = newpopulation[i-2];
	}


}

void crossoveroperator()	//�������
{
	int i, j, k;
	int point, temp;
	int len, end;
	int index[POPSIZE];
	double p;	//�������
	//	char ch;
	struct individual newpopulation[POPSIZE];
	for (i = 0; i < popsize; i++)
	{
		index[i] = i;
	}
	//�������������,δ���������׼��
	for (i = 0; i < popsize; i++)
	{
		point = rand() % (popsize - i);	//�������0 ~(popsize - i)������
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
			//��ͬ��֮��ֱ𽻲�
			for (k = 0; k < ParaC; k++)
			{
				point = rand() % CodeL;				//ÿ�εĽ������(�������0-7)
				len = rand() % (CodeL - point);		//���泤��
				end = point + len + k * CodeL;		//ÿ�ν����յ�
				for (j = point + k * CodeL; j <= end; j++)
				{
					population[i].chrom[j] = newpopulation[index[i]].chrom[j];
				}
			}
		}
	}
}
void mutationoperator()		//�������
{
	int i, j;
	double p;	//����������������������Ƚϣ���С��pm�������
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

//�ӵ�9����ʼ������ǰ8������������ֵ��Ԥ����һ��������ֵ����������ֵ�滻����9���е�����8������
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
		second_dec = decodechrom(secondindividual.chrom, j*CodeL, CodeL);	//����
		SecondbesttempPara[j] = second_dec * double(uplimit[j] - downlimit[j]) / (pow(2.0, CodeL) - 1) + downlimit[j];	//��¯��
	}


	for (j = 0; j < 3; j++)
	{
		forcastpreheatDetla[j] = besttempPara[j] + (rand() % 1000 / 1000.0)*(besttempPara[j] - SecondbesttempPara[j]);
	}

	//����,С��������

	//���ȶ���
	if (forcastpreheatDetla[0] > uplimit[0])
		{
			forcastpreheatDetla[0] = uplimit[0];
		}
	else if (forcastpreheatDetla[0] < downlimit[0])
		{
			forcastpreheatDetla[0] = downlimit[0];
		}
	forcastpreheat[0] = round((forcastpreheatDetla[0] - downlimit[0]) / JDpreheating);
	// ���ȶ�
	if (forcastpreheatDetla[1] > uplimit[1])
	{
		forcastpreheatDetla[1] = uplimit[1];
	}
	else if (forcastpreheatDetla[1] < downlimit[1])
	{
		forcastpreheatDetla[1] = downlimit[1];
	}
	forcastpreheat[1] = round((forcastpreheatDetla[1] - downlimit[1]) / JDheating);
	//����
	if (forcastpreheatDetla[2] > uplimit[2])
	{
		forcastpreheatDetla[2] = uplimit[2];
	}
	else if (forcastpreheatDetla[2] < downlimit[2])
	{
		forcastpreheatDetla[2] = downlimit[2];
	}
	forcastpreheat[2] = round((forcastpreheatDetla[2] - downlimit[2]) / JDsoking);


	fprintf(fpforcastTemp, "��ǰ������");
	for (int i = 0; i < 3; i++)
	{
		fprintf(fpforcastTemp, " %f ", besttempPara[i]);
		
	}

	fprintf(fpforcastTemp, " ����");
	for (int i = 0; i < 3; i++)
	{
	fprintf(fpforcastTemp, " %f ", SecondbesttempPara[i]);
	}
	fprintf(fpforcastTemp, " Ԥ��");
	for (int i = 0; i < 3; i++)
	{
		fprintf(fpforcastTemp, " %f ", forcastpreheatDetla[i]);
	}
	fprintf(fpforcastTemp, "\n");
	

	//ǰ8λ����(Ԥ�ȶ�)

	dec2bin(forcastpreheat[0], binary);
	for (j = 0; j < 8; j++)
	{
		forcastpopulation.chrom[j] = binary[j];
	}

	//�м�8λ����(���ȶΣ�
	dec2bin(forcastpreheat[1], binary);
	for (j = 8; j < 16; j++)
	{
		forcastpopulation.chrom[j] = binary[j - 8];
	}
	//��8λ����(���ȶ�)
	dec2bin(forcastpreheat[1], binary);
	for (j = 16; j < 24; j++)
	{
		forcastpopulation.chrom[j] = binary[j - 16];
	}
	//���һλ��ֵ'\0'����Ϊ���ַ���
	forcastpopulation.chrom[chromlength] = '\0';
	population[1] = forcastpopulation;


}

//�����ǰ����������ֵ
void outputtextreport()
{
	printf("��ǰ��=%d\n��ǰ����������¯��Ϊ:%lf\t%lf\t%lf\t%lf\n",
		generation, besttempPara[0], besttempPara[1], besttempPara[2], bestindividual.fitness);
}

