#ifndef _FurTempOptThread_H_
#define _FurTempOptThread_H_

unsigned long FurTempOpt(LPVOID lpParam);

//遗传算法当前链表头指针
void ga(struct btListNode *pHeda);		

//十进制转二进制
void dec2bin(int decimal, char binary[9]);		

//产生初始种群
void generateinitialpopulation();				

//解码染色体(编码字符串、解码初始位置、码长度)
long decodechrom(char *string, int point, int length);	

//对种群进行评价，包含三个函数(求解目标函数值，计算适应度，求最佳和最差个体)
void evaluatepopulation(struct btListNode *pHead);		

//解码并计算目标函数
void calculateobjectvalue(struct btListNode *pHead);					

//计算适应度
void calculatefitnessvalue();	

//求解最佳和最差个体
void findbestandworstindividual();	

//通过选择，交叉，编译以及预测替换产生下一代种群
void generatenextpopulation();	

//选择操作
void selectopeator();	

//交叉操作
void crossoveroperator();	

//变异操作
void mutationoperator();		

//根据产生的最优值，预测下一代的最优值，并将最优值替换掉第9代中的最差的8个个体
void forecastbest();

//输出当前代数，最优值
void outputtextreport();


#endif 