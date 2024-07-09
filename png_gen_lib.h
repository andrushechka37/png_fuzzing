#pragma once

#include <sys/_types/_size_t.h>

struct chunk {
    unsigned long length;
    char type[4];
    unsigned char * data; 
};

struct png_buffer {
    unsigned char * data;
    size_t len;
};

void make_png(struct png_buffer * buffer);