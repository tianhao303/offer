/**
* @file				TempPresetThread.h
* @brief			¯��Ԥ���¶ȼ����߳�ͷ�ļ���������ʵ���¶ȸ�����Ԥ���¶ȸ���
* @author			wmshun
* @date			2017��3��15��
* @version			����
* @par				��Ȩ����(C), 2013 - 2023, �����Ƽ���ѧ
* @par History :
*1.��    �� : 2017��3��15��
* ��    �� : wmshun
*   �޸����� : �����ļ�
*/

// #include"VariableDef.h"

#ifndef _BtTraceThread_H_
#define _BtTraceThread_H__

unsigned long BtTempCal(LPVOID lpParam);

//��һ�������¯����������
struct btListNode * Create();

//�¸�����¯��������β�������µĽڵ㣬�����½ڵ��ڵ����ݳ�ʼ��
void Insert(struct btListNode * pHead);

//���и�����¯ʱ��ɾ����һ���ڵ㣬���ͷŵ�һ���ڵ�ռ��Լ���һ���ڵ��ڸ����¶ȷֲ��ռ�
struct btListNode* Delete(struct btListNode * pHead);

//��������¶ȷֲ�
void calculation(struct btInfo * btinfotemp);

//����¯�ڹ��������¸���λ�á��¶ȷֲ���
void update(struct btListNode *pHead);

void SendTemp(struct btListNode *pHead);

#endif 