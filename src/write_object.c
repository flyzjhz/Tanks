#include "object.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

int main(void)
{
    int f_id = -1;
    f_id = open(OBJECT_LIST,O_RDWR | O_CREAT);
    if(-1 == f_id)
        perror("open");
    struct tk_object_table object;
    object.id = 16;
    object.blood = 1;
    object.pic_dir = 1;
    object.pic_fire = 0;
    object.pic_harm = 0;
    object.pic_harm_count = 0;
    object.width = 2;
    object.height = 2;
    object.speed = 1;
    object.power_id = 1;
    object.team = 1;
    object.friends = 0;
    object.is_full = 0;
    object.param = 0;
    object.addition = 0;
    object.z = 1;
    write_object(f_id,0,object);
}
