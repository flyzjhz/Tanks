#include "level.h"

int main(void)
{
    int i;
    scene_t scene;
    init_level_file(LEVEL_LIST, &scene);
    printf("total %d level,please input:\n", scene.level_count);
    scanf("%d",&i);
    print_level(&scene,i);
}
