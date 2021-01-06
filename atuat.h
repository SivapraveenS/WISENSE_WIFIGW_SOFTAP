/*
 * File Name: atuat.h
 * Author: ram krishnan (rkris@wisense.in)
 * Created: Nov/24/2017
 *
 *
 * Copyright (c) <2018>, <ram krishnan>
 * All rights reserved.
 *
 * File cannot be copied and/or distributed without the express
 * permission of the author.
 */

#ifndef __ATUAT_H__
#define __ATUAT_H__


#define ATUAT_TAG_BCN_PDU_LEN  8 
#define ATUAT_TAG_BCN_PDU_TAG_ID_FIELD_LEN  3
#define ATUAT_TAG_BCN_PDU_CNTR_FIELD_LEN  2
#define ATUAT_TAG_BCN_PDU_VCC_FIELD_LEN  2
#define ATUAT_TAG_BCN_PDU_RSSI_FIELD_LEN  1

#define ATUAT_TAG_BCN_PDU_TAG_ID_FIELD_OFF  0
#define ATUAT_TAG_BCN_PDU_CNTR_FIELD_OFF  (ATUAT_TAG_BCN_PDU_TAG_ID_FIELD_OFF \
                                           + ATUAT_TAG_BCN_PDU_TAG_ID_FIELD_LEN)
#define ATUAT_TAG_BCN_PDU_VCC_FIELD_OFF (ATUAT_TAG_BCN_PDU_CNTR_FIELD_OFF \
                                         + ATUAT_TAG_BCN_PDU_CNTR_FIELD_LEN)

#define ATUAT_TAG_BCN_PDU_RSSI_FIELD_OFF (ATUAT_TAG_BCN_PDU_VCC_FIELD_OFF \
                                          + ATUAT_TAG_BCN_PDU_VCC_FIELD_LEN)


#endif