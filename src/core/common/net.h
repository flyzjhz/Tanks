#ifndef _NET_H_
#define _NET_H_

#include "common.h"

#define PACKET_OBJECT_MOVE            1
#define PACKET_OBJECT_FIRE            2

#define PACKET_ALL_OBJECT_REQUEST   3
#define PACKET_ALL_OBJECT_CONTINUE  4
#define PACKET_ALL_OBJECT_OVER      5

#define PACKET_ALL_LEVEL_REQUEST    6
#define PACKET_ALL_LEVEL_CONTINUE   7
#define PACKET_ALL_LEVEL_OVER       8

#define PACKET_JOIN_GAME            9
#define PACKET_AGREE_GAME           10
#define PACKET_REJECT_GAME          11

#define PACKET_GAME_START           12

#define PACKET_TYPE_FEEDBACK        0xFFFF

#define PACKET_MAX_SIZE     1024
#define PACKET_STRUCT_SIZE  (sizeof(struct packet))
#define SERVER_PORT         8887

struct packet
{
    uint16_t type;                  //数据包的类型
    uint8_t id:7;                   //表示包的id
    uint8_t feedback:1;             //是否需要回包
    uint16_t object_id;             //操作的id
    uint16_t length;
    uint32_t param;
    char data[0];
};

#endif
