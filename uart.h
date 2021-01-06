/*
 * File Name: uart.h
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

#ifndef __UART_H__
#define __UART_H__

#define UART_MSG_TYPE_ACK   0x0

// Bit definitions of the flags field in the message header
#define UART_HDR_ACK_BM  (1 << 7)
#define UART_PYLD_ACK_BM  (~(1 << 7))

#define UART_ACK_STS_OK_BM               (1 << 0)
#define UART_ACK_STS_OOM_BM              (1 << 1)
#define UART_ACK_STS_FRAME_TOO_LONG_BM   (1 << 2)
#define UART_ACK_STS_INV_CRC             (1 << 3)
#define UART_ACK_STS_RELAY_IN_PROGRESS   (1 << 4)
#define UART_ACK_STS_HDR_BYTES_MISSING   (1 << 5)
#define UART_ACK_STS_PYLD_BYTES_MISSING  (1 << 6)




#define UART_FRAME_HDR_MSG_TYPE_FIELD_LEN   2
#define UART_FRAME_HDR_FLAGS_FIELD_LEN      1
#define UART_FRAME_HDR_SEQ_NR_FIELD_LEN     1
#define UART_FRAME_HDR_PYLD_LEN_FIELD_LEN   2
#define UART_FRAME_HDR_CRC_FIELD_LEN        2

#define UART_FRAME_HDR_HDR_CRC_FIELD_LEN   UART_FRAME_HDR_CRC_FIELD_LEN
#define UART_FRAME_HDR_PYLD_CRC_FIELD_LEN  UART_FRAME_HDR_CRC_FIELD_LEN

#define UART_FRAME_HDR_MSG_TYPE_FIELD_OFF   0

#define UART_FRAME_HDR_FLAGS_FIELD_OFF \
        UART_FRAME_HDR_MSG_TYPE_FIELD_LEN

#define UART_FRAME_HDR_SEQ_NR_FIELD_OFF \
        (UART_FRAME_HDR_FLAGS_FIELD_OFF + UART_FRAME_HDR_FLAGS_FIELD_LEN)

#define UART_FRAME_HDR_PYLD_LEN_FIELD_OFF \
        (UART_FRAME_HDR_SEQ_NR_FIELD_OFF + UART_FRAME_HDR_SEQ_NR_FIELD_LEN)

#define UART_FRAME_HDR_HDR_CRC_FIELD_OFF  \
        (UART_FRAME_HDR_PYLD_LEN_FIELD_OFF + UART_FRAME_HDR_PYLD_LEN_FIELD_LEN)

#define UART_FRAME_HDR_PYLD_CRC_FIELD_OFF \
        (UART_FRAME_HDR_HDR_CRC_FIELD_OFF + UART_FRAME_HDR_HDR_CRC_FIELD_LEN)

#define UART_FRAME_MAX_PYLD_LEN  128

#define UART_FRAME_HDR_LEN  (UART_FRAME_HDR_MSG_TYPE_FIELD_LEN \
                             + UART_FRAME_HDR_FLAGS_FIELD_LEN \
                             + UART_FRAME_HDR_SEQ_NR_FIELD_LEN \
                             + UART_FRAME_HDR_PYLD_LEN_FIELD_LEN \
                             + UART_FRAME_HDR_HDR_CRC_FIELD_LEN \
                             + UART_FRAME_HDR_PYLD_CRC_FIELD_LEN)

#endif
