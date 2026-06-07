#include "plimits.h"

unsigned int screen_size[2] = {
    VIS_WIDTH,
    VIS_HEIGHT
};

void plimits_update_screen_size(unsigned int width, unsigned int height) {
    screen_size[0] = width;
    screen_size[1] = height;
}

