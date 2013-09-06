#ifndef __COMMON_H_
#define __COMMON_H_

#include <stdio.h>
#include <errno.h>

//typedef char int8_t;
typedef unsigned int   uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;

#define OBJECT_PALYER1_TEAM      0x1;
#define OBJECT_LIST         "object.l"
#define LEVEL_LIST          "level.l"
#define OBJECT_CREATOR_LIST "creator.l"
#define MIN(A,B)    (A>B?B:A)

#define DEBUG0(...)
#define DEBUG1(...)         fprintf(stderr,__VA_ARGS__);
#define DEBUG2(...)         perror(__VA_ARGS__);

#define ERRP(con, ret, flag, ...) do                \
            {                                       \
                if(con)                             \
                {                                   \
                    DEBUG##flag(__VA_ARGS__)        \
                    ret;                            \
                }                                   \
            }while(0)                               \

#endif
