#ifndef __ASM_FBDEV_H
#define __ASM_FBDEV_H

#include "device/video.h"


#define FBDEV_FULL_SCREEN_FIRST 1
#define FBDEV_FULL_IMAGE_FIRST  2

struct video_fb_s_attr {
    u8  rotate;
    u8  imr_id;
    u16 width;
    u16 height;
};

void *video_fb_open(struct video_format *, int, int);
int video_fb_get_map(void *, struct fb_map_user *map);
int video_fb_put_map(void *, struct fb_map_user *map);
int video_fb_close(void *fb);
int video_fb_get_pingpong_buffer(void *_hdl, int id, struct fb_map_user *map);
int video_fb_get_s_attr(void *_hdl, struct video_fb_s_attr *attr);

int video_fb_set_event_handler(void *_hdl,
                               int (*handler)(void *, enum fb_event), void *priv);




#endif
