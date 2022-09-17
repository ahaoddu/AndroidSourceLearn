#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <linux/types.h>
#include <stdbool.h>
#include <string.h>

#include <private/android_filesystem_config.h>

#include "binder4Client.h"

#define LOG_TAG "BinderClient"
#include <log/log.h>

#define HELLO_SVR_CMD_SAYHELLO     1
#define HELLO_SVR_CMD_SAYHELLO_TO  2

int g_handle = 0;
struct binder_state *g_bs;

uint32_t svcmgr_lookup(struct binder_state *bs, uint32_t target, const char *name)
{
    uint32_t handle;
    unsigned iodata[512/4];
    struct binder_io msg, reply;

    bio_init(&msg, iodata, sizeof(iodata), 4);
    bio_put_uint32(&msg, 0);  // strict mode header
    bio_put_uint32(&msg, 0);
    bio_put_string16_x(&msg, SVC_MGR_NAME);
    bio_put_string16_x(&msg, name);

    if (binder_call(bs, &msg, &reply, target, SVC_MGR_CHECK_SERVICE)) {
        ALOGW("binder client 查找服务 %s 失败", name);
        return 0;
    }

    handle = bio_get_ref(&reply);
    

    if (handle)
        binder_acquire(bs, handle);

    binder_done(bs, &msg, &reply);

    return handle;
}

void sayhello(void)
{
    unsigned iodata[512/4];
    struct binder_io msg, reply;

	/* 构造binder_io */
    bio_init(&msg, iodata, sizeof(iodata), 4);
   

	/* 放入参数 */
    bio_put_uint32(&msg, 0);  // strict mode header
    bio_put_string16_x(&msg, "IHelloService");

	/* 调用binder_call */
    if (binder_call(g_bs, &msg, &reply, g_handle, HELLO_SVR_CMD_SAYHELLO))
        return ;
	
	/* 从reply中解析出返回值 */
    binder_done(g_bs, &msg, &reply);
	
}

int main(int argc, char **argv)
{
    int fd;
    struct binder_state *bs;
    uint32_t svcmgr = BINDER_SERVICE_MANAGER;
	int ret;

    bs = binder_open("/dev/binder", 128*1024);
    if (!bs) {
        fprintf(stderr, "failed to open binder driver\n");
        return -1;
    }

    g_bs = bs;

	/* get service */
	g_handle = svcmgr_lookup(bs, svcmgr, "hello");
	if (!g_handle) {
        ALOGW("binder client 查找服务 hello 失败");
        return -1;
	} else {
        ALOGW("binder client 查找服务成功 handle = %d", g_handle);
    }

    //调用服务
    sayhello();

}