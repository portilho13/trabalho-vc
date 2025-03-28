#ifndef VIDEO_H
#define VIDEO_H

typedef struct Video {
    int width, height;
    int ntotalframes;
    int fps;
    int nframe;
} Video;

#endif