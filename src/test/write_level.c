#include "level.h"
#include <stdlib.h>

int main(void)
{
    scene_t scene;
    struct level_objects *level_objects;
    struct level *level = (struct level*)malloc(LEVEL_SIZEOF(20));
    printf("sizeof = %d,level = %p\n",LEVEL_SIZEOF(20),level);
    int i;
    for(i = 0; i < 1; ++i)
    {
        level->id = i;
        snprintf(level->background,21,"map%d.png",i+1);
        level->count = 100;
        level->score = 100;
        level->skip_type = 1;
        level->objects_count = 1;
        level->size.width = 36;
        level->size.height = 28;
        level->dd = 16;
        level_objects = level->objects;
        level_objects[i].object_id = 16;
        level_objects[i].type = 1;
        level_objects[i].object_index = 1;
        level_objects[i].point.x = 0;
        level_objects[i].point.y = 0;
        add_level(LEVEL_LIST, 1, level);
    }
}
