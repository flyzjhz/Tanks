#include <client/tank.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <core/level/level.h>
#include <core/common/net.h>

#define COMMAND_LEFT    0x0
#define COMMAND_DOWN    0x1
#define COMMAND_UP      0x2
#define COMMAND_RIGHT   0x3

#define COMMAND_TYPE_MOVE   0x8
#define COMMAND_TYPE_FIRE   0x16


struct packet send_packet;
tk_point_t *p_point;
scene_t scene;
int socket_id = -1;
struct sockaddr_in srv;
SDL_Surface *screen;
SDL_Rect tmp_src_rect;      //refresh()
SDL_Rect tmp_dst_rect;      //refresh()
int tmp_index;              //refresh(),sock_handler();
char *datas;                //sock_handler();
uint16_t id;

int init_socket(char *,uint16_t);
void quit(int);
void send_command(uint16_t type,uint32_t param);

void request_all_object(){
    send_command(PACKET_ALL_OBJECT_REQUEST, 0);
}

int main(int argc, char *argv[])
{
    char *ip, *port;
    if(argc < 3){
        port = "8887";
    }else{
        port = argv[2];
    }
    if(argc < 2){
        ip = "127.0.0.1";
    }else{
        ip = argv[1];
    }
    init_socket(ip,atoi(port));
    memset(&scene, 0, sizeof(scene));
    init_objects();
    join_game();
    if(fork() != 0){
        /*main process*/
        recv_packet();
    }
    request_all_object();
}

int join_game()
{
    printf("join_gaming...\n");
    send_command(PACKET_JOIN_GAME, 0);
    socklen_t len = sizeof(srv);
    struct packet *recv_packet = malloc(PACKET_MAX_SIZE);
    int ret = recvfrom(socket_id, recv_packet, PACKET_MAX_SIZE, 0, (struct sockaddr *)&srv, &len);
    ERRP(-1 == ret, goto ERR0, 2, "join_game,recvfrom");
    printf("recv_packet->type = %d\n",recv_packet->type);
    ERRP(PACKET_AGREE_GAME != recv_packet->type, goto ERR0, 1, "join game failed!\n");
    id = recv_packet->object_id;
    printf("join game successful!\n");
    return 0;
ERR0:
    exit(-1);
}

int init_windows()
{
    int ret;
    ret = SDL_Init(SDL_INIT_VIDEO);
    ERRP(-1 == ret, goto ERR0, 1,"init:SDL_init!\n");

    screen = SDL_SetVideoMode(scene.cur_scene.p_level->size.width, scene.cur_scene.p_level->size.height, 32, SDL_SWSURFACE);

    return 0;
ERR0:
    return -1;
}

int refresh()
{
    tk_object_t *object_list = scene.cur_scene.object_item->head;
    SDL_FillRect(screen, NULL, 0);
    //画里面所有的物体
    while(object_list)
    {
        get_object_rect(scene,object_list, &tmp_dst_rect);
        tmp_src_rect.x = 0;
        tmp_src_rect.y = 0;
        tmp_src_rect.w = tmp_dst_rect.w;
        tmp_src_rect.h = tmp_dst_rect.h;
        SDL_BlitSurface(object_list->pic, &tmp_src_rect, screen, &tmp_dst_rect);
    }
    SDL_Flip(screen);
    return 0;
}

int sock_handler(struct packet *recv_packet,int length)
{
    int is_draw = 0;
    printf("recv_packet->type = %d\n", recv_packet->type);
    switch(recv_packet->type)
    {
        case PACKET_OBJECT_MOVE:
        {
            move_object(recv_packet->object_id,recv_packet->data);
            is_draw = 1;
            break;
        }
        case PACKET_ALL_OBJECT_REQUEST:
        {
            printf("准备接收objects_table!\n");
            if(!scene.objects)
                free(scene.objects);
            scene.objects = malloc(recv_packet->param);
            datas = (char *)scene.objects;
            send_command(PACKET_ALL_OBJECT_CONTINUE,0);
            break;
        }
        case PACKET_ALL_OBJECT_CONTINUE:
        {
            printf("开始接收objects_table!\n");
            length -= PACKET_STRUCT_SIZE;
            memcpy(datas,recv_packet->data,length);
            datas+=length;
            break;
        }
        case PACKET_ALL_OBJECT_OVER:
        {
            printf("objects_table接收完毕，开始验证!\n");
            if(datas - (char *)scene.objects != tmp_index)
                send_command(PACKET_ALL_OBJECT_REQUEST,0);
            else
                ERRP(1, continue, 1, "objects_tables验证失败!\n");
            int f_id = open(OBJECT_LIST, O_RDWR | O_CREAT);
            ERRP(-1 == f_id, goto ERR0, 2, "sock_handler,open");
            int ret = write(f_id, scene.objects, tmp_index);
            if(-1 == ret)
            {
                perror("sock_handler,write!\n");
                close(f_id);
                return -1;
            }

            break;
        }
        case PACKET_ALL_LEVEL_REQUEST:
        {
            printf("准备接收levels_table!\n");
            if(!scene.objects)
                free(scene.objects);
            scene_t *tmp_scene = (scene_t *)recv_packet->data;
            scene.level_count = tmp_scene->level_count;
            tmp_index = *((uint32_t *)(recv_packet->data + 2));
            scene.levels = malloc(tmp_index);
            datas = (char *)scene.levels;
            send_command(PACKET_ALL_OBJECT_CONTINUE,0);
            break;
        }
        case PACKET_ALL_LEVEL_CONTINUE:
        {
            printf("开始接收levels_table!\n");
            memcpy(datas,recv_packet->data,length);
            datas +=length;
            break;
        }
        case PACKET_ALL_LEVEL_OVER:
        {
            printf("levels_table接收完毕，开始验证!\n");
            if(datas - (char *)scene.objects != tmp_index)
                send_command(PACKET_ALL_OBJECT_REQUEST,0);
            else
                ERRP(1, goto ERR0, 1, "levels_tables验证失败!\n");
            int f_id = open(LEVEL_LIST, O_RDWR | O_CREAT);
            ERRP(-1 == f_id, goto ERR0, 2, "sock_handler,open");
            int ret = write(f_id, scene.levels, tmp_index);
            if(-1 == ret)
            {
                perror("sock_handler,write!\n");
                close(f_id);
                return -1;
            }
            /*scene->object_count = tmp_index/sizeof(struct tk_object_table);*/
            break;
        }
        case PACKET_GAME_START:
        {
            init_windows();
        }
    }
    if(recv_packet->feedback)
        send_command(PACKET_TYPE_FEEDBACK, recv_packet->id);
    if(is_draw)
        refresh();
    return 0;
ERR0:
    return -1;
}

int init_socket(char *ip,uint16_t port)
{
    socket_id = socket(PF_INET,SOCK_DGRAM,0);
    ERRP(-1 == socket_id, goto ERR0, 2, "connect,socket");
    
    srv.sin_addr.s_addr = inet_addr(ip);
    srv.sin_port = htons(port);
    srv.sin_family = PF_INET;
    return 0;
ERR0:
    return -1;
}

int recv_packet()
{
    uint16_t size;
    socklen_t len = sizeof(srv);
    int ret;
    char *buf = malloc(PACKET_MAX_SIZE);
    ERRP(NULL == buf, goto ERR0, 2, "recv_packet,malloc");
    while(1)
    {
        /*while((ret = recvfrom(socket_id, buf, PACKET_MAX_SIZE,0, (struct sockaddr *)&srv, &len)) > 0)
        {
            printf("%d------------\n", ret);
            buf += ret;
        }*/
        ret = recvfrom(socket_id, buf, PACKET_MAX_SIZE,0, (struct sockaddr *)&srv, &len);
        ERRP(-1 == ret, goto ERR1, 2, "connect,recvfrom");
        sock_handler((struct packet *)buf, ret);
    }
    return 0;
ERR1:
    free(buf);
ERR0:
    return 1;
}

void keyboard_handler()
{
    SDL_Event event;
    
    while(1)
    {
        if(SDL_WaitEvent(&event))
        {
            if( event.type == SDL_QUIT)
            {
                quit(0);
            }
            else if(event.type == SDL_MOUSEMOTION)
            {
            
            }
            else if(event.type == SDL_MOUSEBUTTONDOWN)
            {
                if(event.button.button == SDL_BUTTON_LEFT)
                {}
            }
            else if(event.type == SDL_KEYDOWN)
            {
                if(event.key.keysym.sym == SDLK_ESCAPE)
                {
                    quit(0);
                }
                else if(SDLK_a == event.key.keysym.sym)
                {
                    send_command(id,COMMAND_LEFT);
                }
                else if(SDLK_w == event.key.keysym.sym)
                {
                    send_command(id,COMMAND_UP);
                }
                else if(SDLK_s == event.key.keysym.sym)
                {
                    send_command(id,COMMAND_DOWN);
                }
                else if(SDLK_d == event.key.keysym.sym)
                {
                    send_command(id,COMMAND_RIGHT);
                }
            }
        }
    }
}

void quit(int code)
{
    exit(code);
}

void send_command(uint16_t type,uint32_t param)
{
    send_packet.type = type;
    send_packet.param = param;
    sendto(socket_id, &send_packet, sizeof(send_packet), 0,(struct sockaddr *) &srv, sizeof(srv));
    return;
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
