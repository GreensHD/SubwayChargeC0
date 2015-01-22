#include "stdio.h"
#include "string.h"
#include "stdlib.h"

#include "api.h"
#include "SubwayCharge.h"
#include "SubwayList.h"


/*其他全局变量定义考生根据需要补充*/
HistoryInfoNode *g_historyInfoNodeHead = NULL; 
CardStat_EN g_CardStatusInfo[MAX_CARD_NUMBERS] = {CARD_VALID};


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

	if(NULL != g_historyInfoNodeHead)
	{
		if(RET_OK != RemoveList(g_historyInfoNodeHead))
		{
			return;
		}
		g_historyInfoNodeHead = NULL;
	}
	g_historyInfoNodeHead = CreateList(); //创建链表头指针

	if(NULL == g_historyInfoNodeHead)
	{
		return;
	}
	

	FILE *fp = NULL;
	fp = fopen(FILENAME, "wa+"); 
	if(NULL == fp)
	{
		apiPrintErrInfo(E99);
		return;
	}
	fclose(fp);

	for(i = 0; i < MAX_CARD_NUMBERS; i++)
	{
		g_CardStatusInfo[i] = CARD_VALID; //卡号为0-9的可用
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
	if(CARD_UNVAILD == g_CardStatusInfo[pstTravelInfo->nCardNo])
	{
		apiPrintErrInfo(E22);
		return;
	}
	//判断出站时间是否大于等于入站时间
	if(apiTimeDiff(pstTravelInfo->nInHour,pstTravelInfo->nInMinute,pstTravelInfo->nOutHour,pstTravelInfo->nOutMinute) > 0)
	{

		apiPrintErrInfo(E02);
		apiWriteLog(0, pstTravelInfo, RET_ERROR);
		return;

	}

	int nDistance = 0;
	int flag = 0;
	//计算两个站点之间的距离
	flag = apiGetDistanceBetweenTwoStation(pstTravelInfo->sInStation ,pstTravelInfo->sOutStation, &nDistance);
	if(RET_ERROR == flag)
	{

		apiPrintOpStatusInfo(I10,pstTravelInfo->nCardNo, pstTravelInfo->nCardMoney);
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

//计算基本票价
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

//计算同站进出的情况
int GetSameStationChargePrice(TravelInfo_ST* pstTravelInfo)
{
//	if(NULL == pstTravelInfo)
	int nMoney = pstTravelInfo->nCardMoney;   //卡金额
	CardType_EN nCardType= pstTravelInfo->enCardType;
	int nTimeDiff = apiTimeDiff(pstTravelInfo->nOutHour, pstTravelInfo->nOutMinute, pstTravelInfo->nInHour, pstTravelInfo->nInMinute);
	if(nTimeDiff <= 30)
	{
		if(CARDTYPE_A == nCardType)  //单程票
			return nMoney;
		else
			return 0;
	}
	else
	{
		if(CARDTYPE_A == nCardType)  //单程票
		{
			return (nMoney>3 ? nMoney:3);
		}
		else
		{
			return 3;
		}
	}
	
//	return 0;
}


//计算扣费票价
int ComputeChargePrice(int baseprice, TravelInfo_ST* pstTravelInfo)
{
	//同站进出
	if(0 == strcmp(pstTravelInfo->sInStation, pstTravelInfo->sOutStation))
	{
		return GetSameStationChargePrice(pstTravelInfo);
	}
	//非同站进出
	else
	{
		//票卡为单程票
		if(CARDTYPE_A == pstTravelInfo->enCardType)
		{
			return (pstTravelInfo->nCardMoney > baseprice) ? pstTravelInfo->nCardMoney : baseprice;  
		}
		//票卡为非单程票
		else
		{
			int nInHour = pstTravelInfo->nInHour;
			int nInMin = pstTravelInfo->nInMinute;
			int nCmpWith07_00 = apiTimeDiff(nInHour, nInMin, 7, 0);  
			int nCmpWith09_00 = apiTimeDiff(nInHour, nInMin, 9, 0);  
			int nCmpWith16_30 = apiTimeDiff(nInHour, nInMin, 16, 30);  
			int nCmpWith18_30 = apiTimeDiff(nInHour, nInMin, 18, 30);  
			if (((0 <= nCmpWith07_00) && (0 > nCmpWith09_00)) || ((0 <= nCmpWith16_30) && (0 > nCmpWith18_30)))//处于非优惠时段
			{
				return baseprice;
			}
			
			else
			{
				int nCmpWith10_00 = apiTimeDiff(nInHour, nInMin, 10, 0);  
				int nCmpWith11_00 = apiTimeDiff(nInHour, nInMin, 11, 0);  
				int nCmpWith15_00 = apiTimeDiff(nInHour, nInMin, 15, 0);                 
				int nCmpWith16_00 = apiTimeDiff(nInHour, nInMin, 16, 0);  
				if (((0 <= nCmpWith10_00) && (0 > nCmpWith11_00))|| ((0 <= nCmpWith15_00) && (0 > nCmpWith16_00)))   // 处于优惠时段
				{
					return baseprice/2;
				}
				else
				{
					if(CARDTYPE_C == pstTravelInfo->enCardType) 
					{
						return baseprice;  
					}
					else if(CARDTYPE_B == pstTravelInfo->enCardType) 
					{
						return (baseprice*9)/10;  
					}
				}
			}
		}
	}
	return 0;

}

int ChargeProcess(int nChargePrice, TravelInfo_ST* pstTravelInfo) 
{

	int nCardID = pstTravelInfo->nCardNo;
	int nLogItems = apiGetLogNum();
	//判断扣费票价是否大于卡上余额
	if(nChargePrice > pstTravelInfo->nCardMoney)
	{
		apiPrintOpStatusInfo(I13,nCardID, pstTravelInfo->nCardMoney);
		
		apiWriteLog(0,pstTravelInfo,RET_ERROR);
		
		return RET_ERROR;
	}
	
	pstTravelInfo->nCardMoney = pstTravelInfo->nCardMoney - nChargePrice;
	if(CARDTYPE_A == pstTravelInfo->enCardType)
	{
		apiPrintOpStatusInfo(I11, nCardID, pstTravelInfo->nCardMoney);
	}
	else
	{
		if(pstTravelInfo->nCardMoney >= 20)
		{
			apiPrintOpStatusInfo(I11, pstTravelInfo->nCardNo, pstTravelInfo->nCardMoney); 
		}
		else
		{
			apiPrintOpStatusInfo(I12, pstTravelInfo->nCardNo, pstTravelInfo->nCardMoney); 
		}
	}

	//写日志
	apiWriteLog(nChargePrice, pstTravelInfo, RET_OK);  
	
	//将扣费记录添加到链表尾
	if(RET_OK == AddHistoryItemOnListTail(nChargePrice,pstTravelInfo))
	{
		return RET_OK;
	}
	else
	{
		return RET_ERROR;
	}
		
}


//将扣费记录添加到链表尾函数
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

void Swap(LogItem_ST &logItemA, LogItem_ST &logItemB)
{
	LogItem_ST tmp;
	memcpy(&tmp, &logItemA, sizeof(LogItem_ST));
	memcpy(&logItemA, &logItemB, sizeof(LogItem_ST));
	memcpy(&logItemB, &tmp, sizeof(LogItem_ST));
}


//利用冒泡排序对卡号进行排序
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




