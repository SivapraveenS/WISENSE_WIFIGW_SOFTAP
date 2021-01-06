/*
 * File Name: lpwmn.h
 * Author: ram krishnan (rkris@wisense.in)
 * Created: Nov/16/2017
 *
 *
 * Copyright (c) <2018>, <ram krishnan>
 * All rights reserved.
 *
 * File cannot be copied and/or distributed without the express
 * permission of the author.
 */
 
#ifndef __LPWMN_H__
#define __LPWMN_H__

#define LPWMN_MAC_SHORT_ADDR_LEN  2
#define LPWMN_MAC_EXT_ADDR_LEN  8
#define LPWMN_MAC_NODE_CAPABILITY_INFO_LEN  1

#define LPWMN_COORD_SHORT_ADDR  0x1
#define LPWMN_MAX_UNICAST_SHORT_ADDR  0x7fff
#define LPWMN_MIN_MULTICAST_SHORT_ADDR  0x8000
#define LPWMN_MAX_MULTICAST_SHORT_ADDR  0xfffe
#define LPWMN_BROADCAST_SHORT_ADDR  0xffff

#define LPWMN_MESH_LEVEL_BROADCAST_ADDR   0xfff0

#define LPWMN_NWK_NODE_LIST_MAX_ENTRY_CNT  32


typedef struct
{
   unsigned int shortAddr;
   unsigned int msgCnt;
   unsigned char extAddr[LPWMN_MAC_EXT_ADDR_LEN];
   unsigned int timeSinceLastMsg;
} LPWMN_nwkNodeInfo_s;


extern int LPWMN_updateNodeList(const unsigned int shortAddr, 
                                const unsigned char *extAddr_p,
                                const char addDelFlag);
                                
                                
extern void LPWMN_initNodeList(void);


extern LPWMN_nwkNodeInfo_s *LPWMN_lookUpNode(const unsigned char *extAddr_p, int offset);

#endif