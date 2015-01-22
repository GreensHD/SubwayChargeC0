
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "SubwayList.h"
#include "malloc.h"
#include <stdarg.h>

/*************************************************
Function      : CreateList
Description   : 创建链表头节点
Return        : 链表的头指针
*************************************************/
HistoryInfoNode* CreateList(void)
{
    HistoryInfoNode *pHead = NULL;

    pHead = (HistoryInfoNode *)malloc(sizeof(HistoryInfoNode));
    if (NULL == pHead)
    {
        apiPrintErrInfo(E99);
        return NULL;
    }

    pHead->data.nCardNo = 0;
    pHead->pNext = NULL;

    return pHead;
}

/*************************************************
Function      : FindNodeByCardNo
Description   : 根据卡号，查找某个节点
Input         : pHead 链表的头节点指针
                要查找的卡号
Return        : 正确:返回指定节点的指针
                失败:返回空指针
*************************************************/
HistoryInfoNode* FindNodeByCardNo(HistoryInfoNode *pHead, int iCradNo)
{
    HistoryInfoNode *pNode = NULL;

    if ((NULL == pHead) || (iCradNo < 0))
    {
        apiPrintErrInfo(E99);
        return NULL;
    }

    pNode = pHead->pNext;
    while ((NULL != pNode))
    {
        if (pNode->data.nCardNo == iCradNo)
        {
            break;
        }
        pNode = pNode->pNext;
    }

    return pNode;
}

/*************************************************
Function      : PushBackNode
Description   : 向链表中尾部插入某个节点
Input         : pHead        链表的头节点指针
                pCardInfo    消费记录信息
Output        : 无
Return        : 正确:返回头节点指针
                失败:返回空指针
*************************************************/
HistoryInfoNode* PushBackNode(HistoryInfoNode *pHead, HistoryItem *pCardInfo)
{
    HistoryInfoNode* pNode      = NULL;
    HistoryInfoNode* pNewNode   = NULL;

    if ((NULL == pHead) || (NULL == pCardInfo))
    {
        apiPrintErrInfo(E99);
        return NULL;
    }

    pNode = pHead;
    while (pNode->pNext != NULL)
    {
        pNode = pNode->pNext;
    }

    pNewNode = (HistoryInfoNode *)malloc(sizeof(HistoryInfoNode));
    if (NULL == pNewNode)
    {
        apiPrintErrInfo(E99);
        return NULL;
    }

    pNode->pNext     = pNewNode;
    pNewNode->pNext = NULL;

    memcpy(&(pNewNode->data), pCardInfo, sizeof(LogItem_ST));

    return pHead;
}

/*************************************************
Function      : RemoveNodeByCardNo
Description   : 从链表中删除指定卡号的记录
Input         : pHead       链表的头节点指针
                iCradNo     待删除的节点的卡号
Return        : 正确:返回链表头节点的指针
                失败:返回空指针
*************************************************/
HistoryInfoNode* RemoveNodeByCardNo(HistoryInfoNode *pHead, int iCradNo)
{
    HistoryInfoNode* pNode      = NULL;
    HistoryInfoNode* pDelNode   = NULL;

    if ((NULL == pHead) || (iCradNo < 0))
    {
        apiPrintErrInfo(E99);
        return NULL;
    }

    pNode = pHead;
    while (NULL != pNode->pNext)
    {
        if (pNode->pNext->data.nCardNo == iCradNo)
        {
            break;
        }

        pNode = pNode->pNext;
    }

    pDelNode = pNode->pNext;
    if (NULL == pDelNode)
    {
        apiPrintErrInfo(E99);
        return NULL;
    }

    pNode->pNext = pDelNode->pNext;
    free(pDelNode);

    pDelNode = NULL;

    return pHead;
}


/*************************************************
Function      : RemoveList
Description   : 删除整个链表
Input         : pHead 链表的头节点指针
Return        : 正确:RET_OK
                失败:RET_ERROR
*************************************************/
int RemoveList(HistoryInfoNode *pHead)
{
    HistoryInfoNode *pNode  = NULL;
    HistoryInfoNode *pb     = NULL;

    if (NULL == pHead)
    {
        apiPrintErrInfo(E99);
        return RET_ERROR;
    }

    pNode = pHead;

    pb = pNode->pNext;
    if (NULL == pb)
    {
        free(pNode);
    }
    else
    {
        while (NULL != pb)
        {
            free(pNode);
            pNode = pb;
            pb = pb->pNext;
        }

        free(pNode);
    }

    pNode = NULL;

    return RET_OK;
}

