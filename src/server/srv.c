#include <server/srv.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <math.h>
#include <core/common/net.h>
#include <core/object/object.h>

int srv_fd;
struct sockaddr_in srv;
scene_t scene;

int create_object()
{
    /*遍历当前关卡的所有物体，并初始化*/
    static struct level *level = NULL;
    level = scene.cur_scene.p_level;
    static int i;
    for(i = 0; i < level->objects_count; ++i)
    {
        switch(level->objects[i].type)
        {
            case LEVEL_OBJECTS_ONE:
            {
                add_one_object(scene
                    , scene.objects[level->objects[i].object_index].team
                    , scene.objects[level->objects[i].object_index].friends
                    , scene.objects[level->objects[i].object_index].id
                    , level->objects[i].point,2,1);
                break;
            }
            case LEVEL_OBJECTS_CREATOR:
            {
                //start_object_creator(level->objects[]);
                break;
            }
        }
    }
}

int clear()
{

}
int destroy_object()
{

}

inline int is_deam(tk_object_t *obj)
{
    if(obj->blood < 0)
    {
        --obj->life;
    }
    if(obj->life < 0)
    {
        clear(obj);
    }
    else
    {
        if(scene.objects[obj->object_table_index].is_full)
        {
            obj->blood = scene.objects[obj->object_table_index].blood;
        }
    }
}

/* obj1对obj2造成伤害 */
inline int produce_object_fire(tk_object_t *obj1, tk_object_t *obj2)
{
    if(obj1->fire)
    {
        if(scene.powers[obj1->power_index].types & POWER_TYPE_AROUND)
        {
            if(obj1->friends & obj2->team)
            {
                //obj2是obj1的盟友
                if(scene.powers[obj1->power_index].types & POWER_TYPE_FRIENDS)
                {
                    obj2->blood -= scene.powers[obj1->power_index].param1;
                }
            }
            else
            {
                if(scene.powers[obj1->power_index].types & POWER_TYPE_ENEMY)
                {
                    obj2->blood -= scene.powers[obj1->power_index].param1;
                }
            }
            if(scene.powers[obj1->power_index].types & POWER_TYPE_SELF)
            {
                obj1->blood -= scene.powers[obj1->power_index].param1;
            }
        }
    }
}

int produce_one_object(tk_object_t *object,uint8_t dir)
{
    static i,j,x1,y1,x2,y2,h,w;
    static tk_object_t *p_obj;
    switch(dir)
    {
        case 0://UP
        {
            y1 = object->point.y - object->speed;
            ERRP(y1 < 0, return -1, 1, "object[%d] cannot up!\n",object->id);
            w = scene.objects[object->object_table_index].width;
            h = scene.objects[object->object_table_index].height;
            x1 = object->point.x;
            x2 = object->point.x + w;
            //y1为扫描矩形最大y坐标
            y2 = y1 + object->speed;
        }
    }
    for(i = x1; i < x2; ++i)
    {
        for(j = y1; j < y2; ++j)
        {
            if(scene.cur_scene.point_object[i * w + j])
            {
                //说明有碰撞
                produce_object_fire(object,scene.cur_scene.point_object[i * w + j]);
                produce_object_fire(scene.cur_scene.point_object[i * w + j],object);
            }
        }
    }
}

int produce_object()
{
    
}

int ai()
{

}

int main(void)
{
    init_objects();
    init_levels();
    init_scene();
    init_socket();
    while(1)
    {
        switch(scene.cur_scene.status)
        {
            case SCENE_STATUS_START:
            {
                create_object();
                //处理物体的移动和费血
                //produce_object();
                break;
            }
            case SCENE_STATUS_RUNNING:
            {
                //ai的移动和发射
                ai();
                //玩家移动和
                produce_object();
                break;
            }
            /*
            case SCENE_STATUS_START:
            {
                break;
            }
            case SCENE_STATUS_START:
            {
                break;
            }
            case SCENE_STATUS_START:
            {
                break;
            }
            case SCENE_STATUS_START:
            {
                break;
            }
            case SCENE_STATUS_START:
            {
                break;
            }*/
        }
    }
}

int cli_handler()
{
    int cli_fd,ret;
    struct sockaddr_in cli;
    socklen_t len = sizeof(srv);
    uint32_t send_packet_len = 0;
    struct packet *send_packet = malloc(PACKET_MAX_SIZE);
    struct packet *recv_packet = malloc(PACKET_MAX_SIZE);
    const uint32_t objects_total_length = scene.object_count * sizeof(struct tk_object_table);
    while(1)
    {
        cli_fd = accept(srv_fd,(struct sockaddr *) &cli, &len);
        send_packet->id = 0;    //包的ID
        while(1)
        {
            int ret = recvfrom(srv_fd, recv_packet, PACKET_MAX_SIZE, 0,(struct sockaddr *) &cli, &len);
            printf("recv_packet->type = %d\n",recv_packet->type);
            ERRP(-1 == ret, goto ERR0, 2, "cli_handler,recvfrom");
            switch(recv_packet->type)
            {
                case PACKET_JOIN_GAME:
                    printf("申请加入游戏！\n");
                    send_packet->type = PACKET_AGREE_GAME;
                    send_packet_len = PACKET_STRUCT_SIZE;
                    send_packet->feedback = 0;
                    send_packet->object_id = 1;
                    goto SEND;
                case PACKET_ALL_OBJECT_REQUEST:
                    send_packet->type = PACKET_ALL_OBJECT_REQUEST;
                    send_packet_len = PACKET_STRUCT_SIZE + sizeof(uint32_t);
                    send_packet->feedback = 0;
                    send_packet->param = objects_total_length;
                    //客户端接收后会发送PACKET_ALL_OBJECT_CONTINUE,可以不用反馈
                    goto SEND;
                case PACKET_ALL_OBJECT_CONTINUE:
                    if(recv_packet->param < objects_total_length)
                    {
                        send_packet->type = PACKET_ALL_OBJECT_CONTINUE;
                        //这里只是packet->data的长度,后面记得加上结构体的长度
                        send_packet_len = PACKET_STRUCT_SIZE + MIN(PACKET_MAX_SIZE - PACKET_STRUCT_SIZE,objects_total_length - recv_packet->param);
                        send_packet->feedback = 1;
                        memcpy(send_packet->data, scene.objects + recv_packet->param, send_packet_len);
                        send_packet_len += PACKET_STRUCT_SIZE;
                    }
                    else
                    {
                        send_packet->type = PACKET_ALL_OBJECT_OVER;
                        send_packet_len = PACKET_STRUCT_SIZE;
                    }
                    goto SEND;
                case PACKET_ALL_LEVEL_REQUEST:
                    send_packet->type = PACKET_ALL_LEVEL_REQUEST;
                    send_packet_len = PACKET_STRUCT_SIZE + sizeof(uint32_t);
                    send_packet->feedback = 0;
                    send_packet->param = scene.level_size;
                    //客户端接收后会发送PACKET_ALL_LEVEL_CONTINUE,可以不用反馈
                    goto SEND;
                case PACKET_ALL_LEVEL_CONTINUE:
                    if(recv_packet->param < scene.level_size)
                    {
                        send_packet->type = PACKET_ALL_LEVEL_CONTINUE;
                        //这里只是packet->data的长度,后面记得加上结构体的长度
                        send_packet_len = PACKET_STRUCT_SIZE + MIN(PACKET_MAX_SIZE - PACKET_STRUCT_SIZE,scene.level_size - recv_packet->param);
                        send_packet->feedback = 1;
                        memcpy(send_packet->data, scene.levels + recv_packet->param, send_packet_len);
                        send_packet_len += PACKET_STRUCT_SIZE;
                    }
                    else
                    {
                        send_packet->type = PACKET_ALL_LEVEL_OVER;
                        send_packet_len = PACKET_STRUCT_SIZE;
                    }
                    goto SEND;
                case PACKET_GAME_START:
                {
                    send_packet->type = PACKET_GAME_START;
                    send_packet_len = PACKET_STRUCT_SIZE;
                    goto SEND;
                }
            }
            SEND:
                sendto(srv_fd, send_packet, send_packet_len, 0,(struct sockaddr *) &cli, sizeof(srv));
                ++send_packet->id;
                continue;
            ERR0:
                break;
        }
    } 
}

int init_socket()
{
    srv_fd = socket(PF_INET, SOCK_DGRAM, 0);
    ERRP(-1 == srv_fd, goto ERR0, 2, "init_socket,socket");

    srv.sin_addr.s_addr = INADDR_ANY;
    srv.sin_port = htons(SERVER_PORT);
    srv.sin_family = PF_INET;

    int ret = bind(srv_fd, (struct sockaddr *)&srv, sizeof(srv));
    ERRP(-1 == ret, goto ERR1, 2, "init_socket,bind");
    /*
     *ret = listen(srv_fd, 5);
     *ERRP(-1 == ret, goto ERR1, 2, "init_socket,listen");
     */
    int i;
    /*
     *for(i =0; i < 10; ++i)
     *{
     *    if(0 == fork())
     *        cli_handler();
     *}
     */
    cli_handler();
    return 0;
ERR1:
    close(srv_fd);
ERR0:
    exit(-1);
}

int fire(tk_object_t *obj,tk_object_t *obj2)
{
    switch(scene.powers[obj->power_index].types)
    {
        case POWER_TYPE_DEAD:
        {
            break;
        }
        case POWER_TYPE_AROUND:
        {
            
            break;
        }
        /*
         *case POWER_TYPE_AROUND:
         *{
         *    break;
         *}
         *case POWER_TYPE_AROUND:
         *{
         *    break;
         *}
         *case POWER_TYPE_AROUND:
         *{
         *    break;
         *}
         */
    }
}


int init_objects()
{
    int ret = load_all_object(OBJECT_LIST, &scene.objects);
    ERRP(ret < 0, exit(-1), 1, "init_objects failed!\n");
    scene.object_count = ret;
    ret = load_all_object_creator(OBJECT_CREATOR_LIST, &scene.object_creators);
    ERRP(ret < 0, exit(-1), 1, "init_object_creators failed!\n");
    scene.object_creator_count = ret;
    return 0;
}

int init_levels()
{
   int ret = init_level_file(LEVEL_LIST, &scene); 
   ERRP(-1 == ret, exit(-1), 1, "init_levels,init_level_file failed!\n");
   return 0;
}

int init_scene()
{
   /*初始化cur_scene*/
   init_cur_scene(&scene.cur_scene, 10);
   scene.cur_scene.int8_level = 1;
   scene.cur_scene.p_level = scene.levels;
   scene.cur_scene.point_object = malloc(scene.levels->size.width * scene.levels->size.height * sizeof(void *));
   return 0;
}

