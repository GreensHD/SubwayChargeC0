/******************************************************************************

                  版权所有 (C), 2010-2011, 华为技术有限公司

 ******************************************************************************
  文 件 名   : SubwayCharge.h
  版 本 号   : v1.0
  功能描述   : SubwayCharge.cpp 的头文件
  函数列表   :

******************************************************************************/

#ifndef __SUBWAYCHARGE_H__
#define __SUBWAYCHARGE_H__

#include "api.h"

/*宏定义考生根据需要补充*/
#define MAX_CARD_NUMBERS (10)
#define MAX_LOG_ITEMS (10)

#define FILENAME ("SubwayCharge.txt")


/*结构定义考生根据需要补充*/


/*****************************************************************************
 函 数 名  : opResetProc
 功能描述  : 考生需要实现的接口
             完成程序初始化,或程序功能复位操作
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 无
*****************************************************************************/
void opResetProc(void);

/*****************************************************************************
 函 数 名  : opChargeProc
 功能描述  : 考生需要实现的接口
             完成请求扣费的功能(详见试题规格说明)
 输入参数  : pstTravelInfo  单次乘车记录信息
 输出参数  : 无
 返 回 值  : 无
*****************************************************************************/
void opChargeProc(TravelInfo_ST* pstTravelInfo);

/*****************************************************************************
 函 数 名  : opQueryLogProc
 功能描述  : 考生需要实现的接口
             完成查询乘车记录日志的功能(详见试题规格说明)
 输入参数  : pstQueryCond  日志查询条件
 输出参数  : 无
 返 回 值  : 无
*****************************************************************************/
void opQueryLogProc(QueryCond_ST* pstQueryCond);

/*****************************************************************************
 函 数 名  : opQueryHistoryChargeListProc
 功能描述  : 考生需要实现的接口
             完成查询指定卡号的票卡消费历史记录功能(详见试题规格说明)
 输入参数  : iCardNo  待查询的票卡卡号
 输出参数  : 无
 返 回 值  : 无
*****************************************************************************/
void opQueryHistoryChargeListProc(int iCardNo);

/*****************************************************************************
 函 数 名  : opDestroyCardProc
 功能描述  : 考生需要实现的接口
             完成注销指定卡号的票卡消费历史记录功能(详见试题规格说明)
 输入参数  : iCardNo  待注销的票卡卡号
 输出参数  : 无
 返 回 值  : 无
*****************************************************************************/
void opDestroyCardProc(int iCardNo);


/*其他函数声明考生根据功能需要补充*/

//计算基本票价
int ComputeBasePrice(int distance);

//计算同站进出的情况
int GetSameStationChargePrice(TravelInfo_ST* pstTravelInfo);

//计算扣费票价
int ComputeChargePrice(int baseprice, TravelInfo_ST* pstTravelInfo);

//扣费过程
int ChargeProcess(int nChargePrice, TravelInfo_ST* pstTravelInfo);

//将扣费记录添加到链表尾函数
int AddHistoryItemOnListTail(int nChargePrice, TravelInfo_ST* pstTravelInfo );

//结构体交换
void Swap(LogItem_ST &logItemA, LogItem_ST &logItemB);

//利用冒泡排序对卡号进行排序
void SortByCardID(LogItem_ST logItems[], int nItems);

//出站时间是否大于等于查询起始时间小于等于查询结束时间
int IsCheckTimeValid(QueryCond_ST* pstQueryCond, LogItem_ST *logAddr);

#endif /* __SUBWAYCHARGE_H__ */

