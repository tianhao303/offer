#ifndef _FurTempOptThread_H_
#define _FurTempOptThread_H_

unsigned long FurTempOpt(LPVOID lpParam);

//�Ŵ��㷨��ǰ����ͷָ��
void ga(struct btListNode *pHeda);		

//ʮ����ת������
void dec2bin(int decimal, char binary[9]);		

//������ʼ��Ⱥ
void generateinitialpopulation();				

//����Ⱦɫ��(�����ַ����������ʼλ�á��볤��)
long decodechrom(char *string, int point, int length);	

//����Ⱥ�������ۣ�������������(���Ŀ�꺯��ֵ��������Ӧ�ȣ�����Ѻ�������)
void evaluatepopulation(struct btListNode *pHead);		

//���벢����Ŀ�꺯��
void calculateobjectvalue(struct btListNode *pHead);					

//������Ӧ��
void calculatefitnessvalue();	

//�����Ѻ�������
void findbestandworstindividual();	

//ͨ��ѡ�񣬽��棬�����Լ�Ԥ���滻������һ����Ⱥ
void generatenextpopulation();	

//ѡ�����
void selectopeator();	

//�������
void crossoveroperator();	

//�������
void mutationoperator();		

//���ݲ���������ֵ��Ԥ����һ��������ֵ����������ֵ�滻����9���е�����8������
void forecastbest();

//�����ǰ����������ֵ
void outputtextreport();


#endif 