#include "stdio.h"
#include "string.h"
#include "stdlib.h"

#include "api.h"
#include "SubwayCharge.h"
#include "SubwayList.h"


/*其他全局变量定义考生根据需要补充*/
HistoryInfoNode *g_historyInfoNodeHead = NULL; //历史消费记录链表头结点
CardStat_EN g_CardStatusInfo[MAX_CARD_NUMBERS] = {CARD_VALID}; //将所有的票设为激活状态


/*****************************************************************************
 函 数 名  : main
 功能描述  : 主入口参数(考生无需更改)
 输入参数  : argc  程序启动时的参数个数
             argv  程序启动时的参数
 输出参数  : 无
 返 回 值  : 无
*****************************************************************************/
void main(int argc, char* argv[])
{
    /*启动Socket服务侦听5555端口(apiServerStart函数在lib库已实现)*/
    apiServerStart(argc, argv);
    return;
}

/*****************************************************************************
 函 数 名  : opResetProc
 功能描述  : 考生需要实现的接口
             完成程序初始化,或程序功能复位操作
             程序启动自动调用该函数,r/R命令时自动调用该函数
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 无
*****************************************************************************/
void opResetProc(void)
{
	int i = 0;

	if(g_historyInfoNodeHead != NULL) //历史消费记录链表不为空
	{
		if(RemoveList(g_historyInfoNodeHead) != RET_OK) //删除历史消费记录链表
		{
			return;
		}
		g_historyInfoNodeHead = NULL;
	}
	g_historyInfoNodeHead = CreateList(); //创建历史消费记录链表

	if(g_historyInfoNodeHead == NULL)
	{
		return;
	}
	

	FILE *fp = NULL;
	fp = fopen(FILENAME, "wa+"); 
	if(fp == NULL)
	{
		apiPrintErrInfo(E99); //系统内部错误
		return;
	}
	fclose(fp);

	for(i = 0; i < MAX_CARD_NUMBERS; i++)
	{
		g_CardStatusInfo[i] = CARD_VALID; //将0~9的卡号设置为可用
	}
	
	return;
	
}

/*****************************************************************************
 函 数 名  : opChargeProc
 功能描述  : 考生需要实现的接口
             完成请求扣费的功能(详见试题规格说明)
             c/C命令时自动调用该函数
 输入参数  : pstTravelInfo  单次乘车记录信息
 输出参数  : 无
 返 回 值  : 无
*****************************************************************************/
void opChargeProc(TravelInfo_ST* pstTravelInfo)
{

	//首先判断卡是否可用
	if(g_CardStatusInfo[pstTravelInfo->nCardNo] == CARD_UNVAILD)
	{
		apiPrintErrInfo(E22); //操作失败，此票卡已经注销
		return;
	}
	//判断出站时间是否大于等于入站时间
	if(apiTimeDiff(pstTravelInfo->nInHour,pstTravelInfo->nInMinute,pstTravelInfo->nOutHour,pstTravelInfo->nOutMinute) > 0)
	{

		apiPrintErrInfo(E02); //参数错误(时间关系错误)
		apiWriteLog(0, pstTravelInfo, RET_ERROR);
		return;

	}

	int nDistance = 0;
	int flag = 0;
	//计算两个站点之间的距离
	flag = apiGetDistanceBetweenTwoStation(pstTravelInfo->sInStation ,pstTravelInfo->sOutStation, &nDistance);
	if(flag == RET_ERROR)
	{

		apiPrintOpStatusInfo(I10,pstTravelInfo->nCardNo, pstTravelInfo->nCardMoney); //输出操作状态函数,I10:扣费失败(无效路线)
		apiWriteLog(0, pstTravelInfo, RET_ERROR);
		return;
		
	}
	//计算基本票价
	int nBasePrice = ComputeBasePrice(nDistance);
	//计算扣费票价
	int nChargePrice = ComputeChargePrice(nBasePrice,pstTravelInfo);
	//进行扣费，并将扣费记录写入链表尾
	ChargeProcess(nChargePrice, pstTravelInfo);
	return ;
}

/*****************************************************************************
 函 数 名  : opQueryLogProc
 功能描述  : 考生需要实现的接口
             完成查询乘车记录日志的功能(详见试题规格说明)
             q/Q命令时自动调用该函数
 输入参数  : pstQueryCond  日志查询条件
 输出参数  : 无
 返 回 值  : 无
*****************************************************************************/
void opQueryLogProc(QueryCond_ST* pstQueryCond)
{
	if(NULL == pstQueryCond)
	{
		apiPrintErrInfo(E99);
		return;
	}
	int nTimeDiff = apiTimeDiff(pstQueryCond->nStartHour,pstQueryCond->nStartMinute,pstQueryCond->nEndHour,pstQueryCond->nEndMinute);
	//出站时间是否大于等于进站时间
	if(nTimeDiff > 0)
	{
		apiPrintErrInfo(E02);
		return;
	}

	LogItem_ST tmpLogItem[MAX_LOG_ITEMS];
	memset(tmpLogItem, 0, sizeof(LogItem_ST) * MAX_LOG_ITEMS);
	int i = 0, j = 0;
	int nLogItemCnt = 0; //记录条目数
	LogItem_ST *pLogAddr = apiGetLogAddr();
	int nItems = apiGetLogNum();
	//卡号为0
	if(0 == pstQueryCond->nCardNo)
	{
		for(i = 1; i < MAX_CARD_NUMBERS; i++)
		{
			if(CARD_UNVAILD == g_CardStatusInfo[i])
			{
				for(j = 0; j < nItems; j++)
				{
					if((pstQueryCond->nCardNo == pLogAddr[j].nCardNo) && (RET_OK == IsCheckTimeValid(pstQueryCond,pLogAddr + j)))
					{
						memcpy(&(tmpLogItem[nLogItemCnt]),&(pLogAddr[j]),sizeof(LogItem_ST));
						nLogItemCnt++;
					}
				}
			}
		}
	}
	//卡号不为0
	else 
	{
		//判断卡是否有效
		if(CARD_UNVAILD == g_CardStatusInfo[pstQueryCond->nCardNo])
		{
			apiPrintErrInfo(E22);  
			return;
		}
		//卡有效
		else
		{
			for(j = 0; j < nItems; j++)
			{
				if((pstQueryCond->nCardNo == pLogAddr[j].nCardNo) && (RET_OK == IsCheckTimeValid(pstQueryCond,pLogAddr + j)))
				   
				{
					memcpy(&(tmpLogItem[nLogItemCnt]), &(pLogAddr[j]), sizeof(LogItem_ST));
					nLogItemCnt++;
				}
			}
		}
	}

	if(nLogItemCnt <= 0)
	{
		apiPrintErrInfo(E21);
		return;
	}
	else
	{
		//sort
		SortByCardID(tmpLogItem, nLogItemCnt);

		apiPrintLog(tmpLogItem, nLogItemCnt);
	}
	return;
}


/*****************************************************************************
 函 数 名  : opQueryHistoryChargeListProc
 功能描述  : 考生需要实现的接口
             完成查询指定卡号的票卡消费历史记录功能(详见试题规格说明)
 输入参数  : iCardNo  待查询的票卡卡号
 输出参数  : 无
 返 回 值  : 无
*****************************************************************************/
void opQueryHistoryChargeListProc(int iCardNo)
{


}

/*****************************************************************************
 函 数 名  : opDestroyCardProc
 功能描述  : 考生需要实现的接口
             完成注销指定卡号的票卡消费历史记录功能(详见试题规格说明)
 输入参数  : iCardNo  待注销的票卡卡号
 输出参数  : 无
 返 回 值  : 无
*****************************************************************************/
void opDestroyCardProc(int iCardNo)
{

}


/*其他函数定义考生根据功能需要补充*/


/*****************************************************************************
 函 数 名  : ComputeBasePrice
 功能描述  : 计算基本票价
 输入参数  : distance  起点站和终点站之间的距离
 输出参数  : 无
 返 回 值  : 返回由距离计算出的基本票价
*****************************************************************************/
int ComputeBasePrice(int distance)
{

	if (0 >= distance)  
	{
		return 0; 
	}
	else if ((0 < distance) && (3 >= distance))  
	{
		 return 2; 
	}
	else if ((3 < distance) && (5 >= distance))  
	{
		 return 3; 
	}
	else if ((5 < distance) && (10 >= distance))
	{
		return 4; 
	}
	else
	{
		return 5;
	}

}

/*****************************************************************************
 函 数 名  : GetSameStationChargePrice
 功能描述  : 计算起点站和终点站重合时的票价
 输入参数  : pstTravelInfo  旅客进出站信息
 输出参数  : 无
 返 回 值  : 起点站和终点站重合时的票价
*****************************************************************************/
int GetSameStationChargePrice(TravelInfo_ST* pstTravelInfo)
{
	if(NULL == pstTravelInfo) //对传入参数进行判断
	{
		return -1;
	}
	int nMoney = pstTravelInfo->nCardMoney;   //卡金额
	CardType_EN nCardType= pstTravelInfo->enCardType;
	int nTimeDiff = apiTimeDiff(pstTravelInfo->nOutHour, pstTravelInfo->nOutMinute, pstTravelInfo->nInHour, pstTravelInfo->nInMinute);
	if(nTimeDiff <= 30)
	{
		if(nCardType == CARDTYPE_A)  //单程票
			return nMoney;
		else //普通票或者老年票
			return 0;
	}
	else
	{
		if(CARDTYPE_A == nCardType)  //单程票
		{
			return (nMoney>3 ? nMoney:3);
		}
		else //普通票或者老年票
		{
			return 3;
		}
	}
}

/*****************************************************************************
 函 数 名  : ComputeChargePrice
 功能描述  : 计算扣费票价
 输入参数  : baseprice  基本票价
           : pstTravelInfo  旅客进出站信息
 输出参数  : 无
 返 回 值  : 扣费票价
*****************************************************************************/
int ComputeChargePrice(int baseprice, TravelInfo_ST* pstTravelInfo)
{
	if(strcmp(pstTravelInfo->sInStation, pstTravelInfo->sOutStation) == 0) //同站进出
	{
		return GetSameStationChargePrice(pstTravelInfo);
	}
	else //非同站进出
	{
		if(pstTravelInfo->enCardType == CARDTYPE_A) //票卡为单程票
		{
			return (pstTravelInfo->nCardMoney > baseprice) ? pstTravelInfo->nCardMoney : baseprice;  
		}
		else //票卡为非单程票
		{
			int nInHour = pstTravelInfo->nInHour;
			int nInMin = pstTravelInfo->nInMinute;

			int CompSeven = apiTimeDiff(nInHour, nInMin, 7, 0);  
			int CompNine = apiTimeDiff(nInHour, nInMin, 9, 0);  
			int CompSixteenThirty = apiTimeDiff(nInHour, nInMin, 16, 30);  
			int CompEighteenThirty = apiTimeDiff(nInHour, nInMin, 18, 30);
			int CompTen= apiTimeDiff(nInHour, nInMin, 10, 0);  
			int CompEleven = apiTimeDiff(nInHour, nInMin, 11, 0);  
			int CompFifteen = apiTimeDiff(nInHour, nInMin, 15, 0);                 
			int CompSixteen = apiTimeDiff(nInHour, nInMin, 16, 0); 

			if (((CompSeven >= 0) && (CompNine < 0)) || ((CompSixteenThirty >= 0) && (CompEighteenThirty < 0))) //特殊非优惠时段
			{
				return baseprice;
			}
			else if (((CompTen >= 0) && (CompEleven <0))|| ((CompFifteen >= 0) && (CompSixteen < 0))) //特殊优惠时段
			{
				return (int)(baseprice*0.5); //出现小数，向下取整
			}
			else //非特殊时段
			{
				if(CARDTYPE_C == pstTravelInfo->enCardType) //普通卡
				{
					return baseprice;  
				}
				else if(CARDTYPE_B == pstTravelInfo->enCardType) //老年卡
				{
					return (int)(baseprice*0.9); //出现小数，向下取整
				}
				else
				{
					return -1; //出现错误
				}
			}
		}
	}
}

/*****************************************************************************
 函 数 名  : ChargeProcess
 功能描述  : 对旅客进行扣费处理，打印状态信息，写入日志信息
 输入参数  : nChargePrice  旅客应付票价
           : pstTravelInfo  旅客进出站信息
 输出参数  : 无
 返 回 值  : 扣费成功: RET_OK
           : 扣费失败: RET_ERROR
*****************************************************************************/
int ChargeProcess(int nChargePrice, TravelInfo_ST* pstTravelInfo) 
{

	int nCardID = pstTravelInfo->nCardNo;
	int nLogItems = apiGetLogNum();
	
	if(nChargePrice > pstTravelInfo->nCardMoney) //判断扣费票价是否大于卡上余额
	{
		apiPrintOpStatusInfo(I13,nCardID, pstTravelInfo->nCardMoney); //输出操作状态函数,I13:扣费失败(余额不足)
		apiWriteLog(0,pstTravelInfo,RET_ERROR);
		return RET_ERROR;
	}
	
	pstTravelInfo->nCardMoney = pstTravelInfo->nCardMoney - nChargePrice;
	if(pstTravelInfo->enCardType == CARDTYPE_A) //单程票
	{
		apiPrintOpStatusInfo(I11, nCardID, pstTravelInfo->nCardMoney); //输出操作状态函数,I11:扣费成功
	}
	else //非单程票
	{
		if(pstTravelInfo->nCardMoney >= 20)
		{
			apiPrintOpStatusInfo(I11, pstTravelInfo->nCardNo, pstTravelInfo->nCardMoney); //输出操作状态函数,I11:扣费成功
		}
		else
		{
			apiPrintOpStatusInfo(I12, pstTravelInfo->nCardNo, pstTravelInfo->nCardMoney); //输出操作状态函数,I11:扣费成功(余额过低)
		}
	}
	
	apiWriteLog(nChargePrice, pstTravelInfo, RET_OK); //写日志
	
	if(AddHistoryItemOnListTail(nChargePrice,pstTravelInfo) == RET_OK) //将扣费记录添加到链表尾
	{
		return RET_OK;
	}
	else
	{
		return RET_ERROR;
	}
		
}

/*****************************************************************************
 函 数 名  : AddHistoryItemOnListTail
 功能描述  : 将扣费记录添加到链表尾函数
 输入参数  : nChargePrice  旅客应付票价
           : pstTravelInfo  旅客进出站信息
 输出参数  : 无
 返 回 值  : 写入成功: RET_OK
           : 写入失败: RET_ERROR
*****************************************************************************/
int AddHistoryItemOnListTail(int nChargePrice, TravelInfo_ST* pstTravelInfo )
{
	HistoryItem historyItem;
	memset(&historyItem, 0, sizeof(HistoryItem)); 

	historyItem.enCardType = pstTravelInfo->enCardType;
	historyItem.nCardNo = pstTravelInfo->nCardNo;
	historyItem.nInHour = pstTravelInfo->nInHour;
	historyItem.nInMin = pstTravelInfo->nInMinute;
	historyItem.nMoney = nChargePrice;
	historyItem.nOutHour = pstTravelInfo->nOutHour;
	historyItem.nOutMin = pstTravelInfo->nOutMinute;

	int nInStationLen = strlen(pstTravelInfo->sInStation);
	int nOutStationLen = strlen(pstTravelInfo->sOutStation);
	memcpy(historyItem.sInStation, pstTravelInfo->sInStation, nInStationLen);
	memcpy(historyItem.sOutStation, pstTravelInfo->sOutStation, nOutStationLen);

	
	if(NULL == PushBackNode(g_historyInfoNodeHead,&historyItem))
	{
		return RET_ERROR;
	}
	else
	{
		return RET_OK;
	}

}

/*****************************************************************************
 函 数 名  : Swap
 功能描述  : 交换两个日志信息
 输入参数  : logItemA  日志信息条目A
           : logItemB  日志信息条目A
 输出参数  : 与输入参数相同
 返 回 值  : 无
*****************************************************************************/
void Swap(LogItem_ST &logItemA, LogItem_ST &logItemB)
{
	LogItem_ST tmp;
	memcpy(&tmp, &logItemA, sizeof(LogItem_ST));
	memcpy(&logItemA, &logItemB, sizeof(LogItem_ST));
	memcpy(&logItemB, &tmp, sizeof(LogItem_ST));
}

/*****************************************************************************
 函 数 名  : SortByCardID
 功能描述  : 利用冒泡排序对卡号进行排序
 输入参数  : logItems  日志信息条目数组
           : nItems  日志信息条目数量
 输出参数  : logItems  日志信息条目数组
 返 回 值  : 无
*****************************************************************************/
void SortByCardID(LogItem_ST logItems[], int nItems)
{
	if(NULL == logItems || nItems <= 0)
	{
		return;
	}
	int i = 0, k = nItems;
	int flag = 1;
	while(flag)
	{
		flag = 0;
		for (i = 1; i < k; i++)
		{
			if (logItems[i - 1].nCardNo > logItems[i].nCardNo)
			{
				Swap(logItems[i - 1], logItems[i]);
				flag = 1;
			}
		}
		k--;
	}
}

/*****************************************************************************
 函 数 名  : IsCheckTimeValid
 功能描述  : 工具函数，检验时间有效性，用于opQueryLogProc函数
 输入参数  : pstQueryCond  查询扣费日志操作命令参数 结构
           : logAddr  乘车记录信息 日志记录结构
 输出参数  : 无
 返 回 值  : 写入成功: RET_OK
           : 写入失败: RET_ERROR
*****************************************************************************/
int IsCheckTimeValid(QueryCond_ST* pstQueryCond, LogItem_ST *logAddr)
{
	if(NULL == pstQueryCond || NULL == logAddr)
	{
		apiPrintErrInfo(E99);
		return RET_ERROR;
	}
	int nQueryStartHour = pstQueryCond->nStartHour;
	int nQueryStartMin = pstQueryCond->nStartMinute;
	int nQueryEndHour = pstQueryCond->nEndHour;
	int nQueryEndMin = pstQueryCond->nEndMinute;

	int nLogStartHour = logAddr->nInHour;
	int nLogStartMin = logAddr->nInMin;
	int nLogEndHour = logAddr->nOutHour;
	int nLogEndMin = logAddr->nOutMin;

	if((apiTimeDiff(nQueryStartHour, nQueryStartMin, nLogEndHour, nLogEndMin) <= 0)
		&& (apiTimeDiff(nLogEndHour,nLogEndMin,nQueryEndHour,nQueryEndMin) <= 0))
	{
		return RET_OK;
	}
	else
	{
		return RET_ERROR;
	}

}




