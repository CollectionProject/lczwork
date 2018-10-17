#ifndef __JIELI_OTA_H
#define __JIELI_OTA_H

#include "typedef.h"

typedef void (*jl_ota_cb)(char *ver, char *url);
int jieli_ota_post(jl_ota_cb cb);

void jieli_ota_set_info(char *key, char *secret, char *cur_ver);

#endif /* __JIELI_OTA_H */


