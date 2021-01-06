/*
 * File Name: lpwmn.cpp
 * Author: ram krishnan (rkris@wisense.in)
 * Created: Jan/25/2018
 *
 *
 * Copyright (c) <2018>, <ram krishnan>
 * All rights reserved.
 *
 * File cannot be copied and/or distributed without the express
 * permission of the author.
 */
 
 
#include <string.h>
#include <stdlib.h>
#include "lpwmn.h"



LPWMN_nwkNodeInfo_s LPWMN_nwkNodeList[LPWMN_NWK_NODE_LIST_MAX_ENTRY_CNT];


extern unsigned char GW_serTxPyldBuff[];

   

/*
 ********************************************************************
 *
 *
 *
 ********************************************************************
 */
LPWMN_nwkNodeInfo_s *LPWMN_lookUpNode(const unsigned char *extAddr_p, const int offset)
{
    int idx, found = 0;
    
    
    if (offset < 0 || offset >= LPWMN_MAC_EXT_ADDR_LEN)
        return NULL;
    
    for (idx=0; idx<LPWMN_NWK_NODE_LIST_MAX_ENTRY_CNT; idx++)
    {
         if (LPWMN_nwkNodeList[idx].shortAddr != LPWMN_BROADCAST_SHORT_ADDR)
         {
             if (memcmp(LPWMN_nwkNodeList[idx].extAddr + offset, extAddr_p + offset, LPWMN_MAC_EXT_ADDR_LEN - offset) == 0)
             {
                 found = 1;
                 break;
             }
         }
    }
    
    return found ? LPWMN_nwkNodeList + idx : NULL;
}



/*
 ********************************************************************
 *
 *
 *
 ********************************************************************
 */
LPWMN_nwkNodeInfo_s *LPWMN_getFreeNodeEntry(void)
{
    int idx, found = 0;
    
    for (idx=0; idx<LPWMN_NWK_NODE_LIST_MAX_ENTRY_CNT; idx++)
    {
        if (LPWMN_nwkNodeList[idx].shortAddr == LPWMN_BROADCAST_SHORT_ADDR)
        {
            found = 1;
            break;
        }
    }
    
    return found ? LPWMN_nwkNodeList + idx : NULL;
}


/*
 ********************************************************************
 *
 *
 *
 ********************************************************************
 */
int LPWMN_updateNodeList(const unsigned int shortAddr, 
                         const unsigned char *extAddr_p,
                         const char addDelFlag)
{
    if (addDelFlag == 1)
    {
        LPWMN_nwkNodeInfo_s *node_p = LPWMN_lookUpNode(extAddr_p, 0);
        
        // Add node

        if (node_p == NULL)
        {
            node_p = LPWMN_getFreeNodeEntry();
            if (node_p != NULL)
                memcpy(node_p->extAddr, extAddr_p, LPWMN_MAC_EXT_ADDR_LEN);
        }
        
        if (node_p != NULL)
        {
            node_p->shortAddr = shortAddr;
            node_p->msgCnt ++;
            node_p->timeSinceLastMsg = 0;
        }    
    }
    else
    {
        // Remove node
        
        LPWMN_nwkNodeInfo_s *node_p = LPWMN_lookUpNode(extAddr_p, 0);
        if (node_p != NULL)
        {
            node_p->shortAddr = LPWMN_BROADCAST_SHORT_ADDR;
        }
    }
    
}


/*
 ********************************************************************
 *
 *
 *
 ********************************************************************
 */
void LPWMN_initNodeList(void)
{
    int idx;
    
    for (idx=0; idx<LPWMN_NWK_NODE_LIST_MAX_ENTRY_CNT; idx++)
    {
         LPWMN_nwkNodeList[idx].shortAddr = LPWMN_BROADCAST_SHORT_ADDR;
         LPWMN_nwkNodeList[idx].msgCnt = 0;
         LPWMN_nwkNodeList[idx].timeSinceLastMsg = 0;
         memset(LPWMN_nwkNodeList[idx].extAddr, 0xff, LPWMN_MAC_EXT_ADDR_LEN);
    }
}

