#ifndef _LEVEL_H_
#define _LEVEL_H_

#include "object.h"

#define LEVEL_OR        0x1         //分数和坦克数量任意一个达到数目后过关
#define LEVEL_AND       0x2         //分数和坦克数量都必须达到才能过关
#define LEVEL_SKIP      0x4         //直接过关


//struct level 结构题除指针外的大小
#define LEVEL_STRUCT_SIZE_          (sizeof(struct level) )
//struct level 结构体struct level_objects *指针在结构体中的相对位置
#define LEVEL_OBJECT_COUNT_INDEX    (sizeof(struct level) - sizeof((void *)) - sizeof(char))
//禁止使用sizeof(struct level),情使用宏LEVEL_SIZEOF(N)
#define LEVEL_SIZEOF(N)             (LEVEL_STRUCT_SIZE_+N*sizeof(struct level_objects))

typedef struct level_link
{
    struct level *level;
    struct level_link *next;
}level_t;

/* 物品类型取值 */
#define LEVEL_OBJECTS_ONE       0x1       //单个物品
#define LEVEL_OBJECTS_CREATOR   0x2       //物品生产起

struct level_objects
{
    uint16_t object_id;         //物品ID
    uint8_t type;               //物品类型
    uint16_t object_index;      //对应tk_object_table
    tk_point_t point;
};
//一关内的所有物品
struct level_objects_link
{
    struct level_objects object;
    struct level_objects_link *next;
}level_objects_t;

//包括每关的信息
struct level
{
    uint8_t id;                 //相当于关卡等级
    char background[21];
    uint16_t count;             //过关需要攻击的坦克数量
    uint16_t score;
    uint8_t skip_type;          //过关类型
    uint8_t objects_count;
    tk_size_t size;
    /*尺寸的最小精度*/
    uint8_t dd;
    //level_objects_t objects;
    struct level_objects objects[0];
};

/*当前场景*/
typedef struct cur_scene
{
    /*所有坦克\子弹\阻挡物*/
    struct object_item *object_item;
    /*下一个object_item的位置*/
    tk_object_t *next_object;
    /*根据队伍保存的hash表*/
    struct hash_team    *hash_team;
    /*根据盟友保存的hash表*/
    struct hash_friends *hash_friends;
    /*下一次object_item的id*/
    uint16_t next_id;
    /*AI的队伍*/
    uint16_t ai_team;
    /*根据队伍计算生产坦克数量*/
    uint16_t team_tank_count;
    /*当前level*/
    uint8_t int8_level;
    struct level *p_level;
    /*当前游戏的状态*/
    uint8_t status;
    /*屏幕上所有坐标对应object的指针*/
    tk_object_t **point_object;
}cur_scene_t;

#define SCENE_STATUS_PAUSE          1
#define SCENE_STATUS_CONTINUE       2
#define SCENE_STATUS_OVER           3
#define SCENE_STATUS_DEAD           4
#define SCENE_STATUS_WAIT           5
#define SCENE_STATUS_START          6
#define SCENE_STATUS_NEXT           7
#define SCENE_STATUS_RUNNING        8

typedef struct scene
{
    uint8_t level_count;    //关卡总数
    uint32_t level_size;    //关卡数据占用的总字节数
    uint16_t object_count;  //所有物品
    uint8_t object_creator_count;
    uint8_t power_count;    //武器的数量
    cur_scene_t cur_scene;
    struct tk_object_table *objects;
    object_creator_t *object_creators;
    struct level *levels;   //存放关卡的全部数据
    struct power *powers;    //存放所有的武器
    level_t *level_link;    //用它来查找某个关卡
}scene_t;
#endif
