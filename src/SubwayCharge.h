/******************************************************************************

                  ��Ȩ���� (C), 2010-2011, ��Ϊ�������޹�˾

 ******************************************************************************
  �� �� ��   : SubwayCharge.h
  �� �� ��   : v1.0
  ��������   : SubwayCharge.cpp ��ͷ�ļ�
  �����б�   :

******************************************************************************/

#ifndef __SUBWAYCHARGE_H__
#define __SUBWAYCHARGE_H__

#include "api.h"

/*�궨�忼��������Ҫ����*/
#define MAX_CARD_NUMBERS (10)
#define MAX_LOG_ITEMS (10)

#define FILENAME ("SubwayCharge.txt")


/*�ṹ���忼��������Ҫ����*/


/*****************************************************************************
 �� �� ��  : opResetProc
 ��������  : ������Ҫʵ�ֵĽӿ�
             ��ɳ����ʼ��,������ܸ�λ����
 �������  : ��
 �������  : ��
 �� �� ֵ  : ��
*****************************************************************************/
void opResetProc(void);

/*****************************************************************************
 �� �� ��  : opChargeProc
 ��������  : ������Ҫʵ�ֵĽӿ�
             �������۷ѵĹ���(���������˵��)
 �������  : pstTravelInfo  ���γ˳���¼��Ϣ
 �������  : ��
 �� �� ֵ  : ��
*****************************************************************************/
void opChargeProc(TravelInfo_ST* pstTravelInfo);

/*****************************************************************************
 �� �� ��  : opQueryLogProc
 ��������  : ������Ҫʵ�ֵĽӿ�
             ��ɲ�ѯ�˳���¼��־�Ĺ���(���������˵��)
 �������  : pstQueryCond  ��־��ѯ����
 �������  : ��
 �� �� ֵ  : ��
*****************************************************************************/
void opQueryLogProc(QueryCond_ST* pstQueryCond);

/*****************************************************************************
 �� �� ��  : opQueryHistoryChargeListProc
 ��������  : ������Ҫʵ�ֵĽӿ�
             ��ɲ�ѯָ�����ŵ�Ʊ��������ʷ��¼����(���������˵��)
 �������  : iCardNo  ����ѯ��Ʊ������
 �������  : ��
 �� �� ֵ  : ��
*****************************************************************************/
void opQueryHistoryChargeListProc(int iCardNo);

/*****************************************************************************
 �� �� ��  : opDestroyCardProc
 ��������  : ������Ҫʵ�ֵĽӿ�
             ���ע��ָ�����ŵ�Ʊ��������ʷ��¼����(���������˵��)
 �������  : iCardNo  ��ע����Ʊ������
 �������  : ��
 �� �� ֵ  : ��
*****************************************************************************/
void opDestroyCardProc(int iCardNo);


/*�������������������ݹ�����Ҫ����*/

//�������Ʊ��
int ComputeBasePrice(int distance);

//����ͬվ���������
int GetSameStationChargePrice(TravelInfo_ST* pstTravelInfo);

//����۷�Ʊ��
int ComputeChargePrice(int baseprice, TravelInfo_ST* pstTravelInfo);

//�۷ѹ���
int ChargeProcess(int nChargePrice, TravelInfo_ST* pstTravelInfo);

//���۷Ѽ�¼��ӵ�����β����
int AddHistoryItemOnListTail(int nChargePrice, TravelInfo_ST* pstTravelInfo );

//�ṹ�彻��
void Swap(LogItem_ST &logItemA, LogItem_ST &logItemB);

//����ð������Կ��Ž�������
void SortByCardID(LogItem_ST logItems[], int nItems);

//��վʱ���Ƿ���ڵ��ڲ�ѯ��ʼʱ��С�ڵ��ڲ�ѯ����ʱ��
int IsCheckTimeValid(QueryCond_ST* pstQueryCond, LogItem_ST *logAddr);

#endif /* __SUBWAYCHARGE_H__ */

