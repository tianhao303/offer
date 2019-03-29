#include"stdafx.h"
#include"BtTempCalThread.h"
#include"VariableDef.h"
#include<math.h>
#include<stdlib.h>

#include<Windows.h>
#include<time.h>
//#include<iostream>
using namespace std;


/////////////////////////////������////////////////////////////////////
extern float readData[14];
extern int iTime;		//��¼����ʱ��
clock_t begin, end;
double cost;
//��ʼ��¼
extern bool optFurTemp;

FILE *fpbtOutTemp = fopen("dbtOutTemp.txt", "w");

////////////////////////////////////////////////////////////////////////

/*
* �������ܣ������¶ȷֲ������߳�
* ���������LPVOID lpParam
* ���������
* ����ֵ  ��
*/
unsigned long BtTempCal(LPVOID lpParam)
{
	/*�����Գ����*/

	int calPrd = 2000;		//��ʱ������
	LARGE_INTEGER li;
	li.QuadPart = -1;
	//����һ��timer����
	hMainTimerCal = CreateWaitableTimer(NULL, 0, NULL);

	smphPrint = CreateSemaphore(NULL, 1, 1, L"PrintSemaphore"); // �����ź���ʱ��Ҫͬʱָ������������Դ�����͵�ǰ������Դ����

	WaitForSingleObject(smphPrint, INFINITE); // �ȴ��Թ�����Դ����ͨ��

	SetWaitableTimer(hMainTimerCal, &li, calPrd, NULL, NULL, 0);



	//��ȡ���ݣ�ֱ����¯�ź�Ϊtrue����������ѭ������ʼ��¯����
	while (1)
	{
		WaitForSingleObject(hMainTimerCal, INFINITE);

		//���Ͷ�ȡ�¶���Ϣ
		PostMessage(AfxGetApp()->m_pMainWnd->m_hWnd, WM_MYGETMSG, NULL, NULL);
		Sleep(50);


		//�ؼ�������////////////ȷ����ȡ�������ٽ��в���
		EnterCriticalSection(&crtSec);
		printf("\n\n-------------------�����߳̽����ٽ���--------------------\n\n");
		LeaveCriticalSection(&crtSec);


		if (btInSignal == true)
		{


			pHead = Create();

			btInSignal = false;
			//�����ں˶�ʱ��

			break;
		}
	}
	

	//���ݵ�һ����¯����λ�úͳ�¯�ź��ж��Ƿ���¯����

	while (false == initterminalFlag)
	{
		/*�����Գ����*/
		WaitForSingleObject(hMainTimerCal, INFINITE);//////	��ʼ��ʱ�Ͽ죬���ÿ���ʱ��
//		Sleep(100);

		//���Ͷ�ȡ�¶���Ϣ
		PostMessage(AfxGetApp()->m_pMainWnd->m_hWnd, WM_MYGETMSG, NULL, NULL);
		Sleep(50);

		//�ؼ�������
		EnterCriticalSection(&crtSec);
		//��ȡ�¶�����
		printf("\n\n-------------------�����߳̽����ٽ���--------------------\n\n");

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


	ReleaseSemaphore(smphPrint, 1, NULL); // �ͷŹ�����Դ ���� V����

	printf("\n\n\n++++++++++++++++++++++++++++�ͷ���Դ���++++++++++++++++++++++++++\n\n\n");
	WaitForSingleObject(hMainTimerCal, INFINITE);

	while (1)
	{

		if (optState == true)
		{
			WaitForSingleObject(hMainTimerCal, INFINITE);
			
			WaitForSingleObject(smphPrint, INFINITE); // �ȴ��Թ�����Դ����ͨ��������P����

			//���Ͷ�ȡ�¶���Ϣ
			PostMessage(AfxGetApp()->m_pMainWnd->m_hWnd, WM_MYGETMSG, NULL, NULL);
			Sleep(100);

			//�ؼ�������
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

			ReleaseSemaphore(smphPrint, 1, NULL); // �ͷŹ�����Դ ���� V����
			printf("\n\n\n++++++++++++++++++++++++++++�ͷ���Դ���++++++++++++++++++++++++++\n\n\n");
		}

		else
		{
			WaitForSingleObject(hMainTimerCal, INFINITE);
			//���Ͷ�ȡ�¶���Ϣ
			PostMessage(AfxGetApp()->m_pMainWnd->m_hWnd, WM_MYGETMSG, NULL, NULL);
			Sleep(100);

			//�ؼ�������
			EnterCriticalSection(&crtSec);
			//��ȡ�¶�����

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
				printf("���º����λ��---------------%f\n", pHead->btinfo.site);
				//			Sleep(500);
			}
		}
	}
	return 0;
}



//���͸����¶���Ϣ
void SendTemp(struct btListNode *pHead)
{
	struct btListNode* pTemp = pHead;
	double temp[90] = {0};
	int i = 90;
	double interval = furnaceSize[0] / 107;	//107����¯�ڸ�������

	while (pTemp->pNext)
	{
		i = (int)floor(pTemp->btinfo.site / interval);
		temp[i] = pTemp->btinfo.btTempDistri[0];
		pTemp = pTemp->pNext;
	}
}
/*
* �������ܣ����������ܶ�
* ���������struct btInfo * btinfotemp
* ���������
* ����ֵ  ��
*/
void cal_flux(struct btInfo * btinfotemp)
{
	double para_up[3];				//�϶��κ���ϵ��
	double  para_down[3];			//��¯��

	//���¯�£�����κ���ϵ��
	polyfit(sizeof(thermocouple) / sizeof(thermocouple[0]), thermocouple, furTemp, 2, para_up);
	polyfit(sizeof(thermocouple) / sizeof(thermocouple[0]), thermocouple, botTemp, 2, para_down);

	for (int i = 0; i < 3; i++)
	{
		printf("ϵ��%.2f\n", para_up[i]);
	}
	for (int i = 0; i < 4; i++)
	{
		printf("¯��%.2f\n", furTemp[i]);
	}

	double temp_up = para_up[0] * btinfotemp->site * btinfotemp->site + para_up[1] * btinfotemp->site + para_up[2];		//��ǰλ����¯��
	double temp_down = para_down[0] * btinfotemp->site * btinfotemp->site + para_down[1] * btinfotemp->site + para_down[2];		//��ǰλ����¯��

	//Ӧ���ȸ��¸���������Ϣ��¯���ڶȣ����������ܶȣ���������¶ȷֲ�
	double eco2_up = 7.17 * pow((0.13*5.56), (1.0 / 3)) / sqrt(temp_up);
	double eh2o_up = 7.17 * 0.11 * 5.56 * pow(5.56, 0.6) / temp_up;
	double airBlackness_up = eco2_up + 1.05 * eh2o_up;	//�ϲ�¯���ڶ�

	double eco2_down = 7.17 * pow((0.13*5.56), (1.0 / 3)) / sqrt(temp_down);
	double eh2o_down = 7.17 * 0.11 * 5.56 * pow(5.56, 0.6) / temp_down;
	double airBlackness_down = eco2_down + 1.05 * eh2o_down;//�²�¯���ڶ�


	double flux_up;		//�ϲ������ܶ�
	double flux_down;

	//�����ϲ������ܶȣ����¶ȴ�����273K
	flux_up = (Stefan_Boltzmann*btinfotemp->btpysicalinfo.btBlackness*airBlackness_up*(1 + btinfotemp->btpysicalinfo.btAngle*(1 - airBlackness_up))*(pow((temp_up + 273), 4) - pow((btinfotemp->btTempDistri[0] + 273), 4))) /
		(airBlackness_up + btinfotemp->btpysicalinfo.btAngle * (1 - airBlackness_up)*(btinfotemp->btpysicalinfo.btBlackness + (1 - btinfotemp->btpysicalinfo.btBlackness) * airBlackness_up));

	//�����²������ܶ�
	flux_down = (Stefan_Boltzmann*btinfotemp->btpysicalinfo.btBlackness*airBlackness_down*(1 + btinfotemp->btpysicalinfo.btAngle*(1 - airBlackness_down))*(pow((temp_down + 273), 4) - pow((btinfotemp->btTempDistri[btinfotemp->dzCount - 1] + 273), 4))) /
		(airBlackness_down + btinfotemp->btpysicalinfo.btAngle * (1 - airBlackness_down)*(btinfotemp->btpysicalinfo.btBlackness + (1 - btinfotemp->btpysicalinfo.btBlackness) * airBlackness_down));

	btinfotemp->flux_up = flux_up;
	btinfotemp->flux_down = flux_down;

}
/*
* �������ܣ���ʼ�������¶ȷֲ��������ȷֲ� 1��3���޸�
* ���������struct btInfo * bt_infoinit
* ���������
* ����ֵ  ��
*/
void init_unequal(struct btInfo * bt_infoinit)
{
	// �����¶ȵõ������¶�

	float centorTemp = -6.23484848e-04 * pow(bt_infoinit->btTempInit, 2) + 0.609121212 * bt_infoinit->btTempInit - 2.11518939;
	double para[3] = {};
	
	// ż����

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

		// �������¶ȺͲ�����϶�������
		polyfit(3, site, height, 2, para);
	}


	for (int i = 0; i < bt_infoinit->dzCount; i++)
	{
		bt_infoinit->btTempDistri[i] = bt_infoinit->btTempInit + para[0] * i*i + para[1] * i + para[2];
	}
}
/*
* �������ܣ���ȡ����������Ϣ
* ���������struct btPysicalInfo *btPyInfo
* ���������
* ����ֵ  ��
*/
void initBtInSite(struct btPysicalInfo *btPyInfo)
{
	//���ݸ��ִ��ļ��ж�ȡ�������ܶȣ����ȣ�����ϵ��
	FILE *fpBtPara = fopen("btparameter.txt", "r");
	char tempType[50];
	if (fpBtPara != NULL)
	{
		fseek(fpBtPara, 0L, SEEK_SET);
		while (!feof(fpBtPara))
		{// �������� ������� �������� �����ܶ� �������� ��������ϵ�� �����ڶ� ������¯Ŀ�� Ŀ������²� ���������²�
			fscanf(fpBtPara, "%s %lf %lf %lf %lf %lf %lf", &tempType, &btPyInfo->btHeight, &btPyInfo->btLength,
				&btPyInfo->btDensity, &btPyInfo->btSpecificHeat, &btPyInfo->btConductivity,&btPyInfo->btBlackness);
			//���ݲ�ͬ����ѡ��ͬ�ĸ�������
			if (!strcmp(tempType, btInfoInit.btpysicalinfo.btType))
			break;
		}
	}
	strcpy(btPyInfo->btType, tempType);

	btPyInfo->btAngle = btPyInfo->btLength / (2 * 1000 * (furnaceSize[1] + furnaceSize[2]) - btPyInfo->btLength);

	fclose(fpBtPara);
		
}
/*
* �������ܣ���ʼ��������Ϣ
* ���������struct btPysicalInfo *btPyInfo
* ���������
* ����ֵ  ��
*/
void init_bt_info(struct btInfo * bt_infoinit)
{
	
	initBtInSite(&bt_infoinit->btpysicalinfo);		//��ȡ����������Ϣ

	bt_infoinit->btTempInit = btInfoInit.btTempInit;//��ȡ��������¯�¶�

	bt_infoinit->bttarget = btInfoInit.bttarget;		//��ȡ�����Ĺ���ָ��
	
	bt_infoinit->sum = 0;
	bt_infoinit->site = 0;							//�������ĳ�ʼλ����Ϊ0

	bt_infoinit->dzCount = (int)floor(bt_infoinit->btpysicalinfo.btHeight / dz);		//���ݸ�����Ⱥͷֲ��������ָ�������
	
	

	bt_infoinit->btTempDistri = (double*)malloc(bt_infoinit->dzCount * sizeof(double));	//Ϊ�����¶ȷֲ�����ռ�

	init_unequal(bt_infoinit);
	for (int i = 0; i < bt_infoinit->dzCount; i++)
	{

		printf("������¯�¶�%.2f\n", bt_infoinit->btTempDistri[i]);
	}
}
/*
* �������ܣ���һ�������¯����������
* ���������
* ���������
* ����ֵ  �������ɹ��Ľڵ�struct btListNode *()
*/

struct btListNode *Create()
{
	struct btListNode *pHead = (struct btListNode*)malloc(sizeof(struct btListNode));//��¯���̿�ʼ������ͷ���

	pHead->pNext = NULL;	//ָ����һ�ڵ��ָ��Ϊ��

	//��ʼ��������	1��26��
	strcpy(btInfoInit.btpysicalinfo.btType, btTypeChoose);

	init_bt_info(&pHead->btinfo);	//��ʼ��������Ϣ
	cal_flux(&pHead->btinfo);		//���������ܶ�
	iCount++;		//����ڵ�����1
	return pHead;
}
/*
* �������ܣ��¸�����¯��������β�������µĽڵ㣬�����½ڵ��ڵ����ݳ�ʼ��
* ���������struct btListNode * pHead ͷ�ڵ�
* ���������
* ����ֵ  �������ɹ��Ľڵ�
*/
void Insert(struct btListNode * pHead)
{
	struct btListNode *pNew,*pTemp = pHead;	//ָ���·���Ŀռ�
	pNew = (struct btListNode*)malloc(sizeof(struct btListNode));	//���½ڵ����ռ�

	pNew->pNext = NULL;

	init_bt_info(&pNew->btinfo);				//��ʼ��������Ϣ
	cal_flux(&pNew->btinfo);			//���������ܶ�

	while (pTemp->pNext)						//�ҵ����һ���ڵ�
	{
		pTemp = pTemp->pNext;
	}
	pTemp->pNext = pNew;

	iCount++;		//����ڵ�����1
}
/*
* �������ܣ����и�����¯ʱ��ɾ����һ���ڵ㣬���ͷŵ�һ���ڵ�ռ��Լ���һ���ڵ��ڸ����¶ȷֲ��ռ�
* ���������struct btListNode * pHead ͷ�ڵ�
* ���������
* ����ֵ  �������ɹ��Ľڵ�
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
* �������ܣ���������¶ȷֲ�
* ���������struct btInfo * btinfotemp
* ���������
* ����ֵ  �������ɹ��Ľڵ�
*/
void calculation(struct btInfo * btinfotemp)
{

	v = steplength / stepcycle;
	btinfotemp->site += 2 * v;		//����λ��

	double para_up[3];				//�϶��κ���ϵ��
	double  para_down[3];			//��¯��

	//���¯�£�����κ���ϵ��
	polyfit(sizeof(thermocouple) / sizeof(thermocouple[0]), thermocouple, furTemp, 2, para_up);
	polyfit(sizeof(thermocouple) / sizeof(thermocouple[0]), thermocouple, botTemp, 2, para_down);

	double temp_up = para_up[0] * btinfotemp->site * btinfotemp->site + para_up[1] * btinfotemp->site + para_up[2];		//��ǰλ����¯��
	double temp_down = para_down[0] * btinfotemp->site * btinfotemp->site + para_down[1] * btinfotemp->site + para_down[2];		//��ǰλ����¯��

	//Ӧ���ȸ��¸���������Ϣ��¯���ڶȣ����������ܶȣ���������¶ȷֲ�
	double eco2_up = 7.17 * pow((0.13*5.56), (1.0 / 3)) / sqrt(temp_up);
	double eh2o_up = 7.17 * 0.11 * 5.56 * pow(5.56, 0.6) / temp_up;
	double airBlackness_up = eco2_up + 1.05 * eh2o_up;	//�ϲ�¯���ڶ�

	double eco2_down = 7.17 * pow((0.13*5.56), (1.0 / 3)) / sqrt(temp_down);
	double eh2o_down = 7.17 * 0.11 * 5.56 * pow(5.56, 0.6) / temp_down;
	double airBlackness_down = eco2_down + 1.05 * eh2o_down;//�²�¯���ڶ�


	double flux_up;		//�ϲ������ܶ�
	double flux_down;

	double *prebtTemp = (double *)malloc(btinfotemp->dzCount * sizeof(double));

	//����һ�θ����¶ȷֲ���������
	for (int i = 0; i < btinfotemp->dzCount; i++)
	{
		prebtTemp[i] = btinfotemp->btTempDistri[i];
	}

	//�����ϲ������ܶȣ����¶ȴ�����273K
	flux_up = (Stefan_Boltzmann*btinfotemp->btpysicalinfo.btBlackness*airBlackness_up*(1 + btinfotemp->btpysicalinfo.btAngle*(1 - airBlackness_up))*(pow((temp_up + 273), 4) - pow((btinfotemp->btTempDistri[0] + 273), 4))) /
		(airBlackness_up + btinfotemp->btpysicalinfo.btAngle * (1 - airBlackness_up)*(btinfotemp->btpysicalinfo.btBlackness + (1 - btinfotemp->btpysicalinfo.btBlackness) * airBlackness_up));

	//�����²������ܶ�
	flux_down = (Stefan_Boltzmann*btinfotemp->btpysicalinfo.btBlackness*airBlackness_down*(1 + btinfotemp->btpysicalinfo.btAngle*(1 - airBlackness_down))*(pow((temp_down + 273), 4) - pow((btinfotemp->btTempDistri[btinfotemp->dzCount - 1] + 273), 4))) /
		(airBlackness_down + btinfotemp->btpysicalinfo.btAngle * (1 - airBlackness_down)*(btinfotemp->btpysicalinfo.btBlackness + (1 - btinfotemp->btpysicalinfo.btBlackness) * airBlackness_down));


	//���㵱ǰʱ�̸����ϱ����¶�
	btinfotemp->btTempDistri[0] = prebtTemp[0] + dt * flux_up / (btinfotemp->btpysicalinfo.btDensity*btinfotemp->btpysicalinfo.btSpecificHeat * (dz / 1000.0)) +
		btinfotemp->btpysicalinfo.btConductivity * dt * (prebtTemp[1] - prebtTemp[0]) / (btinfotemp->btpysicalinfo.btDensity*btinfotemp->btpysicalinfo.btSpecificHeat * (dz / 1000.0) * (dz / 1000.0));
	//���㵱ǰʱ�̸����±����¶�
	btinfotemp->btTempDistri[btinfotemp->dzCount - 1] = prebtTemp[btinfotemp->dzCount - 1] + dt * flux_down / (btinfotemp->btpysicalinfo.btDensity*btinfotemp->btpysicalinfo.btSpecificHeat * (dz / 1000.0))
		+ btinfotemp->btpysicalinfo.btConductivity * dt *(prebtTemp[btinfotemp->dzCount - 2] - prebtTemp[btinfotemp->dzCount - 1]) / (btinfotemp->btpysicalinfo.btDensity * btinfotemp->btpysicalinfo.btSpecificHeat*(dz / 1000.0)*(dz / 1000.0));
	//��������ڲ������¶�
	for (int i = 1; i < btinfotemp->dzCount - 1; i++)
	{
		btinfotemp->btTempDistri[i] = prebtTemp[i] + btinfotemp->btpysicalinfo.btConductivity * dt *(prebtTemp[i - 1] + prebtTemp[i + 1] - 2 * prebtTemp[i]) / (btinfotemp->btpysicalinfo.btDensity*btinfotemp->btpysicalinfo.btSpecificHeat*(dz / 1000.0)*(dz / 1000.0));
	}

	btinfotemp->sum += (prebtTemp[0] + btinfotemp->btTempDistri[0]) * v * dt / 2;

	free(prebtTemp);
}

/*
* �������ܣ�����¯�ڹ��������¸���λ�á��¶ȷֲ���
* ���������struct btListNode* pHead
* ���������
* ����ֵ  �������ɹ��Ľڵ�
*/
void update(struct btListNode* pHead)
{
	struct btListNode* pTemp = pHead;
	while (pTemp)
	{
		calculation(&pTemp->btinfo);	//�����¶ȷֲ�
		pTemp = pTemp->pNext;
	}

}