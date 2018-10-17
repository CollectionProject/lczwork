#ifndef __JIELI_RESULT_H
#define __JIELI_RESULT_H

#include "typedef.h"

enum {
   JL_RESULT_ERROR_OK = 0, 
   JL_RESULT_ERROR_MALLOC, 
   JL_RESULT_ERROR_JSON, 
   JL_RESULT_ERROR_UNKOWN_CMD, 
};

enum {
    JL_CMD_NULL = 0,
    JL_CMD_TEXT,
    JL_CMD_VOICE,
    JL_CMD_IMAGE,

    JL_CMD_MEDIA_ID,
};

typedef int (*jl_result_func)(void *hdl, int cmd, void *buf, int parm);
int jieli_result(void *in, jl_result_func cb, void *cb_hdl);
int jieli_wechat_result(void *in, jl_result_func cb, void *cb_hdl);

char *jieli_result_post(char *cmd, char *parm);

#endif /* __JIELI_RESULT_H */


