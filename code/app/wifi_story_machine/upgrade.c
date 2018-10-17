/*************************************************************************
	> File Name: app/wifi_story_machine/upgrade.c
	> Author:
	> Mail:
	> Created Time: Fri 26 May 2017 03:45:26 PM HKT
 ************************************************************************/
#include "system/includes.h"
#include "app_config.h"
#include "storage_device.h"
#include "server/upgrade_server.h"

/*
 * wifi 升级配置流程：
 *
 * 1.打开upgrade_server 服务，可以配置参数struct sys_upgrade_param
 *   param.buf = 申请固定长度的buffer 4K对齐
 *   param.buf_size = 长度
 *   param.dev_name = "spiflash"
 * 2.请求 UPGRADE_REQ_CHECK_FILE *进行数据校验*
 * 3.请求 UPGRADE_REQ_CHECK_SYSTEM  *进行系统升级检测*
 * 4.从offset为0开始进行一块一块数据升级请求 UPGRADE_REQ_CORE_START
 * 5.升级完成关闭服务，复位重启
 */

#define  RECV_BLOCK_SIZE    (4 * 1024)
/*
 * flash升级文件测试代码，wifi可以参照这个写文件
 */
int wifi_file_upgrade_demo(void)
{
    void *sys_ufd = NULL;
    struct server *upgrade_ser;
    union sys_upgrade_req req = {0};
    u8 *buf = NULL;
    int err = 0;
    int block_size = 0;
    int size = 0;
    int offset = 0;

    server_load(upgrade_server);
    upgrade_ser = server_open("upgrade_server", NULL);
    if (!upgrade_ser) {
        log_e("open upgrade server error\n");
        return -EINVAL;
    }

    FILE *fsdc = fopen(CONFIG_ROOT_PATH"ota_ver3.bfu", "r");
    if (!fsdc) {
        log_e("open test ota_ver2.bfu error\n");
        return -ENOENT;
    }

    buf = malloc(RECV_BLOCK_SIZE);
    if (!buf) {
        log_e("malloc recv buffer error\n");
        goto __ota_exit;
    }
    block_size = RECV_BLOCK_SIZE;
    fread(fsdc, buf, block_size);
    /*
     * 校验文件，必须从0开始校验
     * 如果传入的长度 < 文件长度，则只校验头的部分
     */
    req.info.type = UPGRADE_TYPE_BUF;
    req.info.input.data.buf = buf;
    req.info.input.data.size = block_size;
    req.info.offset = 0;
    err = server_request(upgrade_ser, UPGRADE_REQ_CHECK_FILE, &req);
    if (err) {
        log_e("upgrade file err : 0x%x\n", err);
        return err;
    }

    /*
     * 升级信息校验，确定系统是否需要升级
     */
    req.info.type = UPGRADE_TYPE_BUF;
    req.info.input.data.buf = buf;
    req.info.input.data.size = block_size;
    req.info.offset = 0;
    err = server_request(upgrade_ser, UPGRADE_REQ_CHECK_SYSTEM, &req);
    if (err) {
        log_e("system not match to file : 0x%x\n", err);
        return err;
    }
    /*
     * 这里已经确定可以升级
     * 可以在这里进行升级确认，以及插电提醒
     */

    /*
     * 开始升级
     */
    size = flen(fsdc);
    do {
        req.core.type = UPGRADE_TYPE_BUF;
        req.core.input.data.buf = buf;
        req.core.input.data.size = block_size;
        req.core.offset = offset;
        err = server_request(upgrade_ser, UPGRADE_REQ_CORE_START, &req);
        if (err) {
            log_e("upgrade core run err : 0x%x\n", err);
        }

        offset += block_size;
        size -= block_size;
        block_size = size > RECV_BLOCK_SIZE ? RECV_BLOCK_SIZE : size;
        fread(fsdc, buf, block_size);
    } while (block_size);


    server_close(upgrade_ser);
    cpu_reset();
__ota_exit:
    if (fsdc) {
        fclose(fsdc);
    }

    if (buf) {
        free(buf);
    }

    return 0;
}
