/**
* @file				TempPresetThread.h
* @brief			炉温预设温度计算线程头文件，定义了实测温度个数，预设温度个数
* @author			wmshun
* @date			2017年3月15日
* @version			初稿
* @par				版权所有(C), 2013 - 2023, 北京科技大学
* @par History :
*1.日    期 : 2017年3月15日
* 作    者 : wmshun
*   修改内容 : 创建文件
*/

// #include"VariableDef.h"

#ifndef _BtTraceThread_H_
#define _BtTraceThread_H__

unsigned long BtTempCal(LPVOID lpParam);

//第一块钢坯入炉，创建链表
struct btListNode * Create();

//新钢坯入炉后，在链表尾部插入新的节点，并将新节点内的内容初始化
void Insert(struct btListNode * pHead);

//当有钢坯出炉时，删除第一个节点，并释放第一个节点空间以及第一个节点内钢坯温度分布空间
struct btListNode* Delete(struct btListNode * pHead);

//计算钢坯温度分布
void calculation(struct btInfo * btinfotemp);

//根据炉内工况，更新钢坯位置、温度分布等
void update(struct btListNode *pHead);

void SendTemp(struct btListNode *pHead);

#endif 