#include "stdio.h"
#include "string.h"
#include "stdlib.h"

#include "api.h"
#include "SubwayCharge.h"
#include "SubwayList.h"


/*����ȫ�ֱ������忼��������Ҫ����*/
HistoryInfoNode *g_historyInfoNodeHead = NULL; 
CardStat_EN g_CardStatusInfo[MAX_CARD_NUMBERS] = {CARD_VALID};


/*****************************************************************************
 �� �� ��  : main
 ��������  : ����ڲ���(�����������)
 �������  : argc  ��������ʱ�Ĳ�������
             argv  ��������ʱ�Ĳ���
 �������  : ��
 �� �� ֵ  : ��
*****************************************************************************/
void main(int argc, char* argv[])
{
    /*����Socket��������5555�˿�(apiServerStart������lib����ʵ��)*/
    apiServerStart(argc, argv);
    return;
}

/*****************************************************************************
 �� �� ��  : opResetProc
 ��������  : ������Ҫʵ�ֵĽӿ�
             ��ɳ����ʼ��,������ܸ�λ����
             ���������Զ����øú���,r/R����ʱ�Զ����øú���
 �������  : ��
 �������  : ��
 �� �� ֵ  : ��
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
	g_historyInfoNodeHead = CreateList(); //��������ͷָ��

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
		g_CardStatusInfo[i] = CARD_VALID; //����Ϊ0-9�Ŀ���
	}
	

	return;
	
}

/*****************************************************************************
 �� �� ��  : opChargeProc
 ��������  : ������Ҫʵ�ֵĽӿ�
             �������۷ѵĹ���(���������˵��)
             c/C����ʱ�Զ����øú���
 �������  : pstTravelInfo  ���γ˳���¼��Ϣ
 �������  : ��
 �� �� ֵ  : ��
*****************************************************************************/
void opChargeProc(TravelInfo_ST* pstTravelInfo)
{

	//�����жϿ��Ƿ����
	if(CARD_UNVAILD == g_CardStatusInfo[pstTravelInfo->nCardNo])
	{
		apiPrintErrInfo(E22);
		return;
	}
	//�жϳ�վʱ���Ƿ���ڵ�����վʱ��
	if(apiTimeDiff(pstTravelInfo->nInHour,pstTravelInfo->nInMinute,pstTravelInfo->nOutHour,pstTravelInfo->nOutMinute) > 0)
	{

		apiPrintErrInfo(E02);
		apiWriteLog(0, pstTravelInfo, RET_ERROR);
		return;

	}

	int nDistance = 0;
	int flag = 0;
	//��������վ��֮��ľ���
	flag = apiGetDistanceBetweenTwoStation(pstTravelInfo->sInStation ,pstTravelInfo->sOutStation, &nDistance);
	if(RET_ERROR == flag)
	{

		apiPrintOpStatusInfo(I10,pstTravelInfo->nCardNo, pstTravelInfo->nCardMoney);
		apiWriteLog(0, pstTravelInfo, RET_ERROR);
		return;
		
	}
	//�������Ʊ��
	int nBasePrice = ComputeBasePrice(nDistance);
	//����۷�Ʊ��
	int nChargePrice = ComputeChargePrice(nBasePrice,pstTravelInfo);
	//���п۷ѣ������۷Ѽ�¼д������β
	ChargeProcess(nChargePrice, pstTravelInfo);
	return ;
}

/*****************************************************************************
 �� �� ��  : opQueryLogProc
 ��������  : ������Ҫʵ�ֵĽӿ�
             ��ɲ�ѯ�˳���¼��־�Ĺ���(���������˵��)
             q/Q����ʱ�Զ����øú���
 �������  : pstQueryCond  ��־��ѯ����
 �������  : ��
 �� �� ֵ  : ��
*****************************************************************************/
void opQueryLogProc(QueryCond_ST* pstQueryCond)
{
	if(NULL == pstQueryCond)
	{
		apiPrintErrInfo(E99);
		return;
	}
	int nTimeDiff = apiTimeDiff(pstQueryCond->nStartHour,pstQueryCond->nStartMinute,pstQueryCond->nEndHour,pstQueryCond->nEndMinute);
	//��վʱ���Ƿ���ڵ��ڽ�վʱ��
	if(nTimeDiff > 0)
	{
		apiPrintErrInfo(E02);
		return;
	}

	LogItem_ST tmpLogItem[MAX_LOG_ITEMS];
	memset(tmpLogItem, 0, sizeof(LogItem_ST) * MAX_LOG_ITEMS);
	int i = 0, j = 0;
	int nLogItemCnt = 0; //��¼��Ŀ��
	LogItem_ST *pLogAddr = apiGetLogAddr();
	int nItems = apiGetLogNum();
	//����Ϊ0
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
	//���Ų�Ϊ0
	else 
	{
		//�жϿ��Ƿ���Ч
		if(CARD_UNVAILD == g_CardStatusInfo[pstQueryCond->nCardNo])
		{
			apiPrintErrInfo(E22);  
			return;
		}
		//����Ч
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
 �� �� ��  : opQueryHistoryChargeListProc
 ��������  : ������Ҫʵ�ֵĽӿ�
             ��ɲ�ѯָ�����ŵ�Ʊ��������ʷ��¼����(���������˵��)
 �������  : iCardNo  ����ѯ��Ʊ������
 �������  : ��
 �� �� ֵ  : ��
*****************************************************************************/
void opQueryHistoryChargeListProc(int iCardNo)
{


}

/*****************************************************************************
 �� �� ��  : opDestroyCardProc
 ��������  : ������Ҫʵ�ֵĽӿ�
             ���ע��ָ�����ŵ�Ʊ��������ʷ��¼����(���������˵��)
 �������  : iCardNo  ��ע����Ʊ������
 �������  : ��
 �� �� ֵ  : ��
*****************************************************************************/
void opDestroyCardProc(int iCardNo)
{

}


/*�����������忼�����ݹ�����Ҫ����*/

//�������Ʊ��
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

//����ͬվ���������
int GetSameStationChargePrice(TravelInfo_ST* pstTravelInfo)
{
//	if(NULL == pstTravelInfo)
	int nMoney = pstTravelInfo->nCardMoney;   //�����
	CardType_EN nCardType= pstTravelInfo->enCardType;
	int nTimeDiff = apiTimeDiff(pstTravelInfo->nOutHour, pstTravelInfo->nOutMinute, pstTravelInfo->nInHour, pstTravelInfo->nInMinute);
	if(nTimeDiff <= 30)
	{
		if(CARDTYPE_A == nCardType)  //����Ʊ
			return nMoney;
		else
			return 0;
	}
	else
	{
		if(CARDTYPE_A == nCardType)  //����Ʊ
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


//����۷�Ʊ��
int ComputeChargePrice(int baseprice, TravelInfo_ST* pstTravelInfo)
{
	//ͬվ����
	if(0 == strcmp(pstTravelInfo->sInStation, pstTravelInfo->sOutStation))
	{
		return GetSameStationChargePrice(pstTravelInfo);
	}
	//��ͬվ����
	else
	{
		//Ʊ��Ϊ����Ʊ
		if(CARDTYPE_A == pstTravelInfo->enCardType)
		{
			return (pstTravelInfo->nCardMoney > baseprice) ? pstTravelInfo->nCardMoney : baseprice;  
		}
		//Ʊ��Ϊ�ǵ���Ʊ
		else
		{
			int nInHour = pstTravelInfo->nInHour;
			int nInMin = pstTravelInfo->nInMinute;
			int nCmpWith07_00 = apiTimeDiff(nInHour, nInMin, 7, 0);  
			int nCmpWith09_00 = apiTimeDiff(nInHour, nInMin, 9, 0);  
			int nCmpWith16_30 = apiTimeDiff(nInHour, nInMin, 16, 30);  
			int nCmpWith18_30 = apiTimeDiff(nInHour, nInMin, 18, 30);  
			if (((0 <= nCmpWith07_00) && (0 > nCmpWith09_00)) || ((0 <= nCmpWith16_30) && (0 > nCmpWith18_30)))//���ڷ��Ż�ʱ��
			{
				return baseprice;
			}
			
			else
			{
				int nCmpWith10_00 = apiTimeDiff(nInHour, nInMin, 10, 0);  
				int nCmpWith11_00 = apiTimeDiff(nInHour, nInMin, 11, 0);  
				int nCmpWith15_00 = apiTimeDiff(nInHour, nInMin, 15, 0);                 
				int nCmpWith16_00 = apiTimeDiff(nInHour, nInMin, 16, 0);  
				if (((0 <= nCmpWith10_00) && (0 > nCmpWith11_00))|| ((0 <= nCmpWith15_00) && (0 > nCmpWith16_00)))   // �����Ż�ʱ��
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
	//�жϿ۷�Ʊ���Ƿ���ڿ������
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

	//д��־
	apiWriteLog(nChargePrice, pstTravelInfo, RET_OK);  
	
	//���۷Ѽ�¼��ӵ�����β
	if(RET_OK == AddHistoryItemOnListTail(nChargePrice,pstTravelInfo))
	{
		return RET_OK;
	}
	else
	{
		return RET_ERROR;
	}
		
}


//���۷Ѽ�¼��ӵ�����β����
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


//����ð������Կ��Ž�������
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




