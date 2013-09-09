#include "object.h"
#include "level.h"
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

/*
 * 根据id获取其在object_index中的位置
 */
int find_object_id(const scene_t *scene, uint16_t id)
{
    int i = -1;
    while(i < scene->object_count && id != scene->objects[++i].id);
    ERRP(i == scene->object_count, return -1, 1, "find_object_id:cannot find object_id[%d]\n",id);
    return i;
}

/*
 * 添加一个物体
 */
int add_one_object(scene_t *scene,uint16_t team,uint16_t friends, uint16_t object_id, tk_point_t point, uint8_t direction,uint8_t life)
{
    int index = find_object_id(scene, object_id);
    ERRP(-1 == index, return -1, 1,"add_one_object:cannot find object_id[%d]\n",object_id);
    ERRP(point.x + scene->cur_scene.p_level->dd * scene->objects[index].width > scene->cur_scene.p_level->size.width || point.y + scene->cur_scene.p_level->dd * scene->objects[index].height > scene->cur_scene.p_level->size.height, return -1, 1, "add_one_object:point above max_size");

    /*增加存放所有物体的连表 tk_object_t,并设置其属性*/
    if(scene->cur_scene.object_item->size++ == scene->cur_scene.object_item->max_size)
    {
        tk_object_t *list = malloc(scene->cur_scene.object_item->inc * sizeof(tk_object_t));
        ERRP(NULL == list, return -1, 2, "add_one_object:malloc");
        scene->cur_scene.next_object = list;
        int i;
        for(i = 0; i < scene->cur_scene.object_item->inc - 1; ++i)
        {
            list->id = 0;
            list->next = (list + 1);
            list = list->next;
        }
        list->id = 0;
        list->next = NULL;
        scene->cur_scene.object_item->max_size += scene->cur_scene.object_item->inc;
    }
    scene->cur_scene.next_object->team = team;
    scene->cur_scene.next_object->friends = friends;
    scene->cur_scene.next_object->id = scene->cur_scene.next_id++;
    scene->cur_scene.next_object->object_table_index = index;
    memcpy(&scene->cur_scene.next_object->point, &point, sizeof(tk_point_t));
    memcpy(&scene->cur_scene.next_object->old_point, &point, sizeof(tk_point_t));
    scene->cur_scene.next_object->direction = direction;
    scene->cur_scene.next_object->status = 0;
    scene->cur_scene.next_object->life = life;
    scene->cur_scene.next_object->blood = scene->objects[index].blood;
    /*修改物体点阵*/
    int i,j;
    for(i = 0;i < scene->objects[index].width; ++i)
    {
        for(j = 0; j < scene->objects[index].height; ++j)
        {
            scene->cur_scene.point_object[(point.x + i) * (point.y + j)] = scene->cur_scene.next_object;
        }
    }
    scene->cur_scene.next_object = scene->cur_scene.next_object->next;
}

/*
 * 初始化当前场景
 */
int init_cur_scene( cur_scene_t * cur_scene, uint8_t inc)
{
    cur_scene->object_item = (struct object_item *)malloc(sizeof(struct object_item));
    ERRP(!cur_scene->object_item, goto ERR0, 2,"malloc init_scene->object_item");
    cur_scene->hash_team = (struct hash_team *)malloc(MAX_TEAM * sizeof(struct hash_team));
    ERRP(!cur_scene->hash_team, goto ERR1, 2,"malloc init_scene->hash_team");
    cur_scene->hash_friends = (struct hash_friends *)malloc(MAX_TEAM * sizeof(struct hash_friends));
    ERRP(!cur_scene->hash_friends, goto ERR2, 2,"malloc init_scene->hash_frineds");

    cur_scene->object_item->head = (tk_object_t *)malloc(inc * sizeof(tk_object_t));
    ERRP(!cur_scene->object_item->head, goto ERR3, 1, "init_scene:malloc scene->object_item->head %d*%lu bytes failed!\n",inc,sizeof(tk_object_t));
    int i;
    tk_object_t *list = cur_scene->object_item->head;
    cur_scene->next_object = list;
    for(i = 0; i < inc - 1; ++i)
    {
        list->id = 0;
        list->next = (list + 1);
        list = list->next;
    }
    list->next = NULL;
    
    cur_scene->object_item->max_size = inc;
    cur_scene->object_item->inc = inc;
    cur_scene->object_item->size = 0;
    cur_scene->next_id = OBJECT_START_ID;   //十一下的坦克是系统预定，6个玩家坦克从90度开始逆时针旋转分别为1-6
    cur_scene->ai_team = AI1;

    //将当前关数设置为1
    cur_scene->int8_level = 1;
    //当前关数指针暂时无法初始化

    cur_scene->status = 0;
    

    return 0;
ERR3:
    free(cur_scene->hash_friends);
ERR2:
    free(cur_scene->hash_team);
ERR1:
    free(cur_scene->object_item);
ERR0:
    return -1;
}

/*生成所有的object全局表*/
int load_all_object(const char *filepath,struct tk_object_table **object)
{
    int f_id = open(filepath,O_RDONLY);
    ERRP(!f_id, goto ERR0, 2, "open");
    struct stat stat;
    fstat(f_id, &stat);

    ERRP(!stat.st_size%sizeof(struct tk_object_table), goto ERR1, 1, "load_all_ai:the %s file format error",filepath);
    (*object) = malloc(stat.st_size);
    char *buf = (char *)(*object);
    int ret;
    while((ret = read(f_id, buf, stat.st_size)) > 0)
    {
        buf += ret;
    }
    ERRP(ret < 0, goto ERR1, 2, "read");
    close(f_id);
    return stat.st_size/sizeof(struct tk_object_table);
ERR1:
    close(f_id);
ERR0:
    return -1;
}

/*
 * 移动一个物体
 */
int move_object(const scene_t *scene, uint16_t id, tk_point_t *p_point)
{
    tk_object_t *object_list = scene->cur_scene.object_item->head;
    ERRP(!object_list, return -1, 1, "move_object:object_item->head is null!\n");
    while(object_list->id != id)
    {
        object_list = object_list->next;
        ERRP(!object_list, return -1, 1,"move_object:cannot find object[%d]\n",id);
    }
    object_list->point.x = p_point->x;
    object_list->point.y = p_point->y;
    return 0;
}
/*
 * 移动一个物体
 */
int move_object_of_dir(const scene_t *scene, uint16_t id, uint8_t dir)
{
    tk_object_t *object_list = scene->cur_scene.object_item->head;
    ERRP(!object_list, return -1, 1, "move_object:object_item->head is null!\n");
    while(object_list->id != id)
    {
        object_list = object_list->next;
        ERRP(!object_list, return -1, 1,"move_object:cannot find object[%d]\n",id);
    }
    static int h;
    static int w;
    static tk_point_t point;
    static int i;
    memcpy(&point, &object_list->point, sizeof(tk_point_t));
    if(dir == 0)
    {
        if(object_list->point.y - object_list->speed > 0)
        {
            object_list->point.y -= object_list->speed;
        }
        else if (object_list->point.y > 0)
        {
            object_list->point.y = 0;
        }
        else
        {
            return -1;
        }
    }
    else if(dir == 1)
    {
        h = scene->cur_scene.p_level->size.height;
        if(object_list->point.y + object_list->speed + scene->objects[object_list->object_table_index].height <= h)
        {
            object_list->point.y += scene->objects[object_list->object_table_index].height;
        }
        else if(object_list->point.y + scene->objects[object_list->object_table_index].height < h)
        {
            object_list->point.y = h - scene->objects[object_list->object_table_index].height;
        }
        else
        {
            return -1;
        }
    }
    else if(dir == 2)
    {
        if(object_list->point.x - object_list->speed > 0)
        {
            object_list->point.x -= scene->objects[object_list->object_table_index].width;
        }
        else if (object_list->point.x > 0)
        {
            object_list->point.x = 0;
        }
        else
        {
            return -1;
        }
    }
    else if(dir == 2)
    {
        w = scene->cur_scene.p_level->size.height;
        if(object_list->point.x + object_list->speed + scene->objects[object_list->object_table_index].width < w)
        {
            object_list->point.x += object_list->speed;
        }
        else if(object_list->point.x +scene->objects[object_list->object_table_index].width < w)
        {
            object_list->point.x = w;
        }
        else
        {
            return -1;
        }
    }
    return 0;
}

/* 此函数没有经过测试 */
int load_all_object_creator(char *filepath, struct object_creator **data)
{
    int f_id = open(filepath,O_RDONLY);
    ERRP(!f_id, goto ERR0, 2, "open");
    struct stat stat;
    fstat(f_id, &stat);

    *data = malloc(stat.st_size);
    char *buf = (char *)(*data);
    int ret;
    while((ret = read(f_id, buf, stat.st_size)) > 0)
    {
        buf +=ret;
    }
    ERRP(ret < 0, goto ERR1, 2, "read");
    close(f_id);
    return stat.st_size/sizeof(struct object_creator);
ERR1:
    close(f_id);
ERR0:
    return -1;
}

int write_object(const int f_id,const int index,const struct tk_object_table object)
{
    struct stat stat;
    fstat(f_id, &stat);
    /*printf("stat.st_size=%d,sizeof = %d\n",stat.st_size,sizeof(struct tk_object_table));*/
    ERRP(stat.st_size%sizeof(struct tk_object_table), goto ERR0, 1, "load_all_object:file format error");
    int size = stat.st_size - index * sizeof(struct tk_object_table);
    char *buf = NULL;
    if(size > 0)
    {
        buf = malloc(size);
        char *old_buf = buf;

        int ret;
        ret = lseek(f_id, index * sizeof(struct tk_object_table), SEEK_SET);
        ERRP(-1 == ret, goto ERR1, 2, "write_object, lseek");
        while((ret = read(f_id, buf,1024)) > 0)
        {
            printf("read = %d,ret = %d\n",(*buf),ret);
            buf+=ret;
        }
        ERRP(-1 == ret, goto ERR1, 2, "write_object,read");
        ret = lseek(f_id, index * sizeof(struct tk_object_table), SEEK_SET);
        ERRP(-1 == ret, goto ERR1, 2, "write_object, lseek1");
        ret = write(f_id, &object, sizeof(struct tk_object_table));
        ERRP(-1 == ret, goto ERR1, 2, "write_ai,write");
        //ret = lseek(f_id, (index+1) * sizeof(struct tk_object_table), SEEK_SET);
        //ERRP(-1 == ret, goto ERR1, 2, "write_object, lseek2");
        ret = write(f_id, old_buf, size);
        ERRP(-1 == ret, goto ERR1, 2, "write_ai,write2");
    }
    else
    {
        int ret;
        ret = lseek(f_id, 0, SEEK_END);
        ERRP(-1 == ret, goto ERR0, 2, "write_ai, lseek");
        ret = write(f_id, &object, sizeof(struct tk_object_table));
        ERRP(-1 == ret, goto ERR0, 2, "write_ai,write");
    }
    return 0;
    ERR1:
        free(buf);
    ERR0:
        return -1;
}

int print_all_object(const char *filepath)
{
    struct tk_object_table *objects;
    int count = load_all_object(filepath,&objects);
    int i;
    for(i = 0; i < count; ++i)
    {
        printf("%d:id[%d],size[%d,%d],blood[%d],team[%d],fri[%d],p_id[%d]\n",i,
                                 objects[i].id,
                                 objects[i].width,
                                 objects[i].height,
                                 objects[i].blood,
                                 objects[i].team,
                                 objects[i].friends,
                                 objects[i].power_id
                                 );
    }
}

int get_object_rect(const scene_t *scene,const tk_object_t *o,SDL_Rect *r)
{
    r->w = (scene->objects[o->object_table_index]).width;
    r->h = (scene->objects[o->object_table_index]).height;
    r->x = o->point.x;
    r->y = o->point.y;
}

/*
 *int init_object(struct object_item *object,uint16_t max_size,uint8_t inc)
 *{
 *    ERRP(inc > max_size, return -1, 1, "init_object:inc > max_size\n");
 *    
 *    object->head = (struct object_item*)malloc(inc * sizeof(struct tk_object_t));
 *    ERRP(!object->head, return -2, 1, "init_object:malloc tk_object_t %d*%d bytes failed!\n",inc,sizeof(struct tk_object_t));
 *
 *    int i;
 *    struct tk_object_t *list = object->head;
 *    for(i = 0; i < inc - 1; ++i)
 *    {
 *        list->id = 0;
 *        list->next = (list + 1);
 *        list = list->next;
 *    }
 *    list->next = NULL;
 *}
 */
