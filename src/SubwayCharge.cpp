#include "stdio.h"
#include "string.h"
#include "stdlib.h"

#include "api.h"
#include "SubwayCharge.h"
#include "SubwayList.h"


/*����ȫ�ֱ������忼��������Ҫ����*/
HistoryInfoNode *g_historyInfoNodeHead = NULL; //��ʷ���Ѽ�¼����ͷ���
CardStat_EN g_CardStatusInfo[MAX_CARD_NUMBERS] = {CARD_VALID}; //�����е�Ʊ��Ϊ����״̬


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

	if(g_historyInfoNodeHead != NULL) //��ʷ���Ѽ�¼����Ϊ��
	{
		if(RemoveList(g_historyInfoNodeHead) != RET_OK) //ɾ����ʷ���Ѽ�¼����
		{
			return;
		}
		g_historyInfoNodeHead = NULL;
	}
	g_historyInfoNodeHead = CreateList(); //������ʷ���Ѽ�¼����

	if(g_historyInfoNodeHead == NULL)
	{
		return;
	}
	

	FILE *fp = NULL;
	fp = fopen(FILENAME, "wa+"); 
	if(fp == NULL)
	{
		apiPrintErrInfo(E99); //ϵͳ�ڲ�����
		return;
	}
	fclose(fp);

	for(i = 0; i < MAX_CARD_NUMBERS; i++)
	{
		g_CardStatusInfo[i] = CARD_VALID; //��0~9�Ŀ�������Ϊ����
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
	if(g_CardStatusInfo[pstTravelInfo->nCardNo] == CARD_UNVAILD)
	{
		apiPrintErrInfo(E22); //����ʧ�ܣ���Ʊ���Ѿ�ע��
		return;
	}
	//�жϳ�վʱ���Ƿ���ڵ�����վʱ��
	if(apiTimeDiff(pstTravelInfo->nInHour,pstTravelInfo->nInMinute,pstTravelInfo->nOutHour,pstTravelInfo->nOutMinute) > 0)
	{

		apiPrintErrInfo(E02); //��������(ʱ���ϵ����)
		apiWriteLog(0, pstTravelInfo, RET_ERROR);
		return;

	}

	int nDistance = 0;
	int flag = 0;
	//��������վ��֮��ľ���
	flag = apiGetDistanceBetweenTwoStation(pstTravelInfo->sInStation ,pstTravelInfo->sOutStation, &nDistance);
	if(flag == RET_ERROR)
	{

		apiPrintOpStatusInfo(I10,pstTravelInfo->nCardNo, pstTravelInfo->nCardMoney); //�������״̬����,I10:�۷�ʧ��(��Ч·��)
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


/*****************************************************************************
 �� �� ��  : ComputeBasePrice
 ��������  : �������Ʊ��
 �������  : distance  ���վ���յ�վ֮��ľ���
 �������  : ��
 �� �� ֵ  : �����ɾ��������Ļ���Ʊ��
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
 �� �� ��  : GetSameStationChargePrice
 ��������  : �������վ���յ�վ�غ�ʱ��Ʊ��
 �������  : pstTravelInfo  �ÿͽ���վ��Ϣ
 �������  : ��
 �� �� ֵ  : ���վ���յ�վ�غ�ʱ��Ʊ��
*****************************************************************************/
int GetSameStationChargePrice(TravelInfo_ST* pstTravelInfo)
{
	if(NULL == pstTravelInfo) //�Դ�����������ж�
	{
		return -1;
	}
	int nMoney = pstTravelInfo->nCardMoney;   //�����
	CardType_EN nCardType= pstTravelInfo->enCardType;
	int nTimeDiff = apiTimeDiff(pstTravelInfo->nOutHour, pstTravelInfo->nOutMinute, pstTravelInfo->nInHour, pstTravelInfo->nInMinute);
	if(nTimeDiff <= 30)
	{
		if(nCardType == CARDTYPE_A)  //����Ʊ
			return nMoney;
		else //��ͨƱ��������Ʊ
			return 0;
	}
	else
	{
		if(CARDTYPE_A == nCardType)  //����Ʊ
		{
			return (nMoney>3 ? nMoney:3);
		}
		else //��ͨƱ��������Ʊ
		{
			return 3;
		}
	}
}

/*****************************************************************************
 �� �� ��  : ComputeChargePrice
 ��������  : ����۷�Ʊ��
 �������  : baseprice  ����Ʊ��
           : pstTravelInfo  �ÿͽ���վ��Ϣ
 �������  : ��
 �� �� ֵ  : �۷�Ʊ��
*****************************************************************************/
int ComputeChargePrice(int baseprice, TravelInfo_ST* pstTravelInfo)
{
	if(strcmp(pstTravelInfo->sInStation, pstTravelInfo->sOutStation) == 0) //ͬվ����
	{
		return GetSameStationChargePrice(pstTravelInfo);
	}
	else //��ͬվ����
	{
		if(pstTravelInfo->enCardType == CARDTYPE_A) //Ʊ��Ϊ����Ʊ
		{
			return (pstTravelInfo->nCardMoney > baseprice) ? pstTravelInfo->nCardMoney : baseprice;  
		}
		else //Ʊ��Ϊ�ǵ���Ʊ
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

			if (((CompSeven >= 0) && (CompNine < 0)) || ((CompSixteenThirty >= 0) && (CompEighteenThirty < 0))) //������Ż�ʱ��
			{
				return baseprice;
			}
			else if (((CompTen >= 0) && (CompEleven <0))|| ((CompFifteen >= 0) && (CompSixteen < 0))) //�����Ż�ʱ��
			{
				return (int)(baseprice*0.5); //����С��������ȡ��
			}
			else //������ʱ��
			{
				if(CARDTYPE_C == pstTravelInfo->enCardType) //��ͨ��
				{
					return baseprice;  
				}
				else if(CARDTYPE_B == pstTravelInfo->enCardType) //���꿨
				{
					return (int)(baseprice*0.9); //����С��������ȡ��
				}
				else
				{
					return -1; //���ִ���
				}
			}
		}
	}
}

/*****************************************************************************
 �� �� ��  : ChargeProcess
 ��������  : ���ÿͽ��п۷Ѵ�����ӡ״̬��Ϣ��д����־��Ϣ
 �������  : nChargePrice  �ÿ�Ӧ��Ʊ��
           : pstTravelInfo  �ÿͽ���վ��Ϣ
 �������  : ��
 �� �� ֵ  : �۷ѳɹ�: RET_OK
           : �۷�ʧ��: RET_ERROR
*****************************************************************************/
int ChargeProcess(int nChargePrice, TravelInfo_ST* pstTravelInfo) 
{

	int nCardID = pstTravelInfo->nCardNo;
	int nLogItems = apiGetLogNum();
	
	if(nChargePrice > pstTravelInfo->nCardMoney) //�жϿ۷�Ʊ���Ƿ���ڿ������
	{
		apiPrintOpStatusInfo(I13,nCardID, pstTravelInfo->nCardMoney); //�������״̬����,I13:�۷�ʧ��(����)
		apiWriteLog(0,pstTravelInfo,RET_ERROR);
		return RET_ERROR;
	}
	
	pstTravelInfo->nCardMoney = pstTravelInfo->nCardMoney - nChargePrice;
	if(pstTravelInfo->enCardType == CARDTYPE_A) //����Ʊ
	{
		apiPrintOpStatusInfo(I11, nCardID, pstTravelInfo->nCardMoney); //�������״̬����,I11:�۷ѳɹ�
	}
	else //�ǵ���Ʊ
	{
		if(pstTravelInfo->nCardMoney >= 20)
		{
			apiPrintOpStatusInfo(I11, pstTravelInfo->nCardNo, pstTravelInfo->nCardMoney); //�������״̬����,I11:�۷ѳɹ�
		}
		else
		{
			apiPrintOpStatusInfo(I12, pstTravelInfo->nCardNo, pstTravelInfo->nCardMoney); //�������״̬����,I11:�۷ѳɹ�(������)
		}
	}
	
	apiWriteLog(nChargePrice, pstTravelInfo, RET_OK); //д��־
	
	if(AddHistoryItemOnListTail(nChargePrice,pstTravelInfo) == RET_OK) //���۷Ѽ�¼��ӵ�����β
	{
		return RET_OK;
	}
	else
	{
		return RET_ERROR;
	}
		
}

/*****************************************************************************
 �� �� ��  : AddHistoryItemOnListTail
 ��������  : ���۷Ѽ�¼��ӵ�����β����
 �������  : nChargePrice  �ÿ�Ӧ��Ʊ��
           : pstTravelInfo  �ÿͽ���վ��Ϣ
 �������  : ��
 �� �� ֵ  : д��ɹ�: RET_OK
           : д��ʧ��: RET_ERROR
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
 �� �� ��  : Swap
 ��������  : ����������־��Ϣ
 �������  : logItemA  ��־��Ϣ��ĿA
           : logItemB  ��־��Ϣ��ĿA
 �������  : �����������ͬ
 �� �� ֵ  : ��
*****************************************************************************/
void Swap(LogItem_ST &logItemA, LogItem_ST &logItemB)
{
	LogItem_ST tmp;
	memcpy(&tmp, &logItemA, sizeof(LogItem_ST));
	memcpy(&logItemA, &logItemB, sizeof(LogItem_ST));
	memcpy(&logItemB, &tmp, sizeof(LogItem_ST));
}

/*****************************************************************************
 �� �� ��  : SortByCardID
 ��������  : ����ð������Կ��Ž�������
 �������  : logItems  ��־��Ϣ��Ŀ����
           : nItems  ��־��Ϣ��Ŀ����
 �������  : logItems  ��־��Ϣ��Ŀ����
 �� �� ֵ  : ��
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
 �� �� ��  : IsCheckTimeValid
 ��������  : ���ߺ���������ʱ����Ч�ԣ�����opQueryLogProc����
 �������  : pstQueryCond  ��ѯ�۷���־����������� �ṹ
           : logAddr  �˳���¼��Ϣ ��־��¼�ṹ
 �������  : ��
 �� �� ֵ  : д��ɹ�: RET_OK
           : д��ʧ��: RET_ERROR
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




