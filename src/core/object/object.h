#ifndef _OBJECT_H_
#define _OBJECT_H_

#define MAX_TEAM    16 
#define AI1         0x00000100
#define OBJECT_START_ID     16

#include <core/common/common.h>
#include <SDL/SDL.h>
/*坐标*/
typedef struct tk_point_t
{
    uint16_t x;
    uint16_t y;
}tk_point_t;
/*大小*/
typedef struct tk_size
{
    uint16_t width;
    uint16_t height;
}tk_size_t;
/*战场上的坦克、子弹、阻挡物*/
struct tk_object_table
{
    uint16_t id;
    int8_t blood;                     //一次生命拥有多少血
    uint16_t pic_direction[0];
    uint16_t pic_dir:1;             //是否包含方向图片,如果包含有四个
    uint16_t pic_fire:1;            //是否包含开火图片
    uint16_t pic_harm:1;            //是否包含受伤图片
    uint16_t pic_harm_count:5;      //受伤图片的个数
    uint16_t width:3;             //图片占用几个最小精度 width*最小精度=图片宽度
    uint16_t height:3;
    uint16_t speed:2;               //需要乘以最小精度
    uint8_t power_id;
    uint16_t team;
    uint16_t friends;
    uint8_t is_full:1;              //是否为一次性灭亡
    uint8_t param;
    uint8_t addition:1;
    uint8_t z:2;
};

struct power
{
    uint8_t id;
    uint8_t types;
    uint16_t param1;
    uint16_t param2;
    uint16_t param3;
};

typedef struct power_link
{
    uint16_t id;
    struct power power;
    struct power_link *next;
}power_t;

/* power_type取值 */
#define POWER_TYPE_DEAD     0x1         //死亡伤害,只有死亡时才会造成伤害
#define POWER_TYPE_AROUND   0x2         //靠近时造成伤害
#define POWER_TYPE_FRIENDS  0x4         //盟友造成伤害
#define POWER_TYPE_ENEMY    0x8         //非盟友造成伤害
#define POWER_TYPE_SELF     0x16        //对自己也造成伤害
#define POWER_TYPE_SLOW     0x32        //周期性伤害


/*用连表可查找战场上所有的物体*/
typedef struct tk_object_link
{
    //object的ID
    uint16_t id;
    //object_table的数组下标
    uint16_t object_table_index;
    tk_point_t old_point;               //出生坐标
    tk_point_t point;
    SDL_Surface *pic;
    uint8_t power_index;                    //所拥有的武器
    struct tk_object_link *next;
    uint8_t direction:2;
    uint8_t status:2;
    uint8_t life:4;                     //生命数
    int8_t blood;
    uint8_t speed:2;
    uint8_t fire:2;                     //是否可以进行攻击
    uint16_t team;
    uint16_t friends;
}tk_object_t;

#define OBJECT_STATUS_MOVE      0x1     //
#define OBJECT_STATUS_WUDI      0x8     //无敌

/*所有坦克、子弹、阻挡物(stl)*/
struct object_item
{
    uint16_t max_size;                  //当前已分配空间的最大条数
    uint16_t size;                      //当前已使用的条数
    uint8_t inc;
    tk_object_t *head;
};

#define PRODUCE_TYPE_DEAD    0x1          //死亡时生产下一个object
#define PRODUCE_TYPE_TIME    0x2          //隔多少秒出现下一个
#define PRODUCE_TYPE_RAND    0x3
#define PRODUCE_TYPE_REPEAT  0x80         //重复，必须和其它标志位一起使用  
//uint16_t type;

/*物体生产器*/
struct object_creator
{
    uint16_t id;
    uint16_t type;          //object生产器的类型
    uint16_t object_index;  //object_table数组中的坐标
    uint16_t param1;        //根据生产器类型的不同，代表不同的意义
    uint16_t param2;
    uint32_t param3;
};

typedef struct object_creator_link
{
    uint16_t id;
    uint16_t count;
    struct object_creator data[0];
}object_creator_t;

/*坦克和子弹队伍HASH表*/
struct hash_team
{
    uint16_t team;
    uint16_t object_table_index;
    struct tk_object_t *next;
};

/*盟友HASH表*/
struct hash_friends
{
    uint16_t friends;
    uint16_t object_table_index;
    struct hash_friends *next;
};

int load_all_object(const char *filepath,struct tk_object_table **object);
#endif
