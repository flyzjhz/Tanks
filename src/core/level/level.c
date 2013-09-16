#include <core/level/level.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

int add_level(char *filepath,int index,struct level *level)
{
    int ret;
    /*ERRP(index > scene->level_count + 1, goto ERR0, 1, "insert index error!\n");*/
    int f_id = open(filepath, O_RDWR | O_CREAT);
    ERRP(-1 == f_id, goto ERR0, 2, "add_level,open");
    struct stat stat;
    fstat(f_id, &stat);
    ERRP(-1 == f_id, goto ERR1, 2, "init_level,fstat");
    int offset;
    if(stat.st_size)
    {
        offset = get_level_index_of_file(f_id, index);
        ERRP(-1 == offset, goto ERR1, 0);
    }
    else
    {
        offset = 0;
    }
    printf("offset = %d\n",offset);
    char *buf;
    print_one_level(level);
    if(offset > 0)
    {
        int size = stat.st_size - offset;
        buf = malloc(size);
        char *old_data = buf;
        ret = lseek(f_id, offset,SEEK_SET);
        ERRP(-1 == ret, goto ERR1, 2, "add_level,lseek");
        while((ret = read(f_id,buf,1024)) > 0)
        {
            buf += ret;
        }
        ERRP(-1 == ret, goto ERR1, 2, "add_level,read");
        ret = lseek(f_id, offset, SEEK_SET);
        ERRP(-1 == ret, goto ERR1, 2, "add_level,lseek2");
        ret = write(f_id, level, LEVEL_STRUCT_SIZE_);
        if(level->objects_count > 0)
        {
            ret = write(f_id, level->objects, level->objects_count * sizeof(struct level_objects));
            ERRP(-1 == ret, goto ERR1, 2, "add_level,write");
            printf("wriete %d char!\n",ret);
        }
        ret = write(f_id, old_data, size);
        ERRP(-1 == ret, goto ERR1, 2, "add_level,write1");
    }
    else
    {
        ret = lseek(f_id, 0, SEEK_END);
        ERRP(-1 == ret, goto ERR1, 2, "add_level,lseek1");
        ret = write(f_id, level, LEVEL_STRUCT_SIZE_);
        ERRP(-1 == ret, goto ERR1, 2, "add_level,write");
        if(level->objects_count > 0)
        {
            ret = write(f_id, level->objects, level->objects_count * sizeof(struct level_objects));
            ERRP(-1 == ret, goto ERR1, 2, "add_level,write");
            printf("wriete %d char!\n",ret);
        }
    }
    close(f_id);
    return 0;
ERR1:
    close(f_id);
ERR0:
    return -1;
}

int get_level_size(struct level *level)
{
    return level->objects_count * sizeof(struct level_objects) + LEVEL_STRUCT_SIZE_;
}

int get_level_index_of_file(int f_id,int index)
{
    int ret;
    /*由于需要读取struct level_objects,需要多分配一些内存空间*/
    struct level *level = (struct level *)malloc(20 * sizeof(struct level));
    ERRP(!level, goto ERR0, 1, "get_level_index_of_file,malloc\n");
    int offset = 0,i=0,count,length;
    while(1)
    {
        ERRP(i == index, return offset, 0);
        ret = read(f_id, level, LEVEL_STRUCT_SIZE_);
        ERRP(ret == 0, return -2, 1, "get_level_index_of_file,cannot find %d level!\n", index);
        ERRP(ret != LEVEL_STRUCT_SIZE_, goto ERR1, 1, "get_level_index_of_file,read record[%d] failed[%d != %lu],file format error!\n",i+1,ret,LEVEL_STRUCT_SIZE_);
        offset += ret;
        length = level->objects_count * sizeof(struct level_objects);
        while((ret = read(f_id, level, length)) > 0 && length)
        {
            offset += ret;
            length -= ret;
        }
        printf("get_level_index_of_file:offset = %d\n",offset);
        ERRP(ret == 0 && length != 0, goto ERR1, 1, "file format error!\n");
        ERRP(-1 == ret,goto ERR1, 2,"get_level_index_of_file,read1");
        offset += ret;
        ++i;
    }
    free(level);
    return 0;
ERR1:
    free(level);
ERR0:
    return -1;
}

int init_level_file(char *filepath , scene_t *scene)
{
    int f_id = open(filepath, O_RDONLY);
    ERRP(-1 == f_id, goto ERR0, 2, "init_level_file,open");
    int ret;
    struct stat stat;
    fstat(f_id, &stat);
    ERRP(-1 == f_id, goto ERR1, 2, "init_level_file,fstat");
    ERRP(0 == stat.st_size, goto ERR1, 1, "init_level_file,file is zero!\n");
    scene->levels = malloc(stat.st_size);
    ERRP(NULL == scene->levels, goto ERR1, 2, "init_level_file,malloc");
    char *buf = (char *)scene->levels;

    level_t *level_link;
    level_link = malloc(sizeof(level_t));
    ERRP(NULL == level_link, goto ERR2, 2, "init_level_file,malloc");
    scene->level_link = level_link;
    int offset = 0,i=0,length;
    //该循环内部跳出可能存在内存泄露，以后修复
    while(1)
    {
        ret = read(f_id, buf, LEVEL_STRUCT_SIZE_);
        if(0 == ret)
            break;
        ERRP(-1 == ret,goto ERR3, 2,"init_level_file,read");
        ERRP(ret != LEVEL_STRUCT_SIZE_, goto ERR3, 1, "init_level_file,read record[%d] failed,file format error!\n",i+1);
        level_link->level = (struct level *)buf;
        buf += ret;
        printf("i = %d,level = %p, objects = %p\n",i,level_link->level,level_link->level->objects);

        length = level_link->level->objects_count * sizeof(struct level_objects);
        ret = 0;
        while(length != ret && (ret = read(f_id, buf,  length)) > 0)
        {
            buf += ret;
            length -= ret;
        }
        ERRP(-1 == ret,goto ERR3, 2,"init_level_file,read1");
        ++i;
        level_link->next = malloc(sizeof(level_t));
        level_link = level_link->next;
    }
    printf("i = %d\n",i);
    scene->level_count = i;
    scene->level_size = stat.st_size;
    level_link->next = NULL;
    close(f_id);
    return 0;
ERR3:
    free(level_link);
ERR2:
    free(scene->levels);
ERR1:
    close(f_id);
ERR0:
    return -1;
}

//程序中所有的关数都从0开始
int print_level(scene_t *scene, int index)
{
    level_t *level_link;
    level_link = scene->level_link;
    for(;index > 0; --index)
    {
        level_link = level_link->next;
        printf("index = %d,level_link=%p\n", index,level_link);
    }
    struct level *level = level_link->level;
    print_one_level(level);
    return index;
}

int print_one_level(struct level *level)
{
    int j;
    printf("level address = %p\n", level);
    printf("level.id = %d\n" , level->id);
    printf("level.background = %s\n",level->background);
    printf("level.count = %d\n", level->count);
    printf("level.score = %d\n", level->score);
    printf("level.skip_type = %d\n",level->skip_type);
    printf("level.objects_count = %d,objects = %p\n",level->objects_count,level->objects);
    for(j = 0; j < level->objects_count; ++j)
    {
        printf("level_objects[%d].object_id = %d\n", j, level->objects[j].object_id);
        printf("level_objects[%d].type = %d\n", j, level->objects[j].type);
        printf("level_objects[%d].object_index = %d\n", j,level->objects[j].object_index);
        printf("level_objects[%d].point.x = %d\n", j,level->objects[j].point.x);
        printf("level_objects[%d].point.y = %d\n",j, level->objects[j].point.y);
    }
    return 0;
}


