#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <linux/types.h>
#include<stdbool.h>
#include <string.h>

#include <private/android_filesystem_config.h>

#include "binder4Server.h"

#define LOG_TAG "BinderServer"
#include <log/log.h>

#define HELLO_SVR_CMD_SAYHELLO     1
#define HELLO_SVR_CMD_SAYHELLO_TO  2

int svcmgr_publish(struct binder_state *bs, uint32_t target, const char *name, void *ptr)
{
    int status;
    unsigned iodata[512/4];
    struct binder_io msg, reply;

    bio_init(&msg, iodata, sizeof(iodata), 4);
    bio_put_uint32(&msg, 0);  // strict mode header
    bio_put_uint32(&msg, 0);
    bio_put_string16_x(&msg, SVC_MGR_NAME);
    bio_put_string16_x(&msg, name);
    bio_put_obj(&msg, ptr);
    bio_put_uint32(&msg, 0);
    bio_put_uint32(&msg, 0);

    if (binder_call(bs, &msg, &reply, target, SVC_MGR_ADD_SERVICE)) {
        fprintf(stderr, "svcmgr_public 远程调用失败\n");
        return -1;
    }
   
    status = bio_get_uint32(&reply); //调用成功返回0
    binder_done(bs, &msg, &reply);

    return status;
}

void sayhello(void)
{
	static int cnt = 0;
	fprintf(stderr, "say hello : %d\n", ++cnt);
}


int sayhello_to(char *name)
{
	static int cnt = 0;
	fprintf(stderr, "say hello to %s : %d\n", name, ++cnt);
	return cnt;
}



int hello_service_handler(struct binder_state *bs,
                   struct binder_transaction_data_secctx *txn_secctx,
                   struct binder_io *msg,
                   struct binder_io *reply)
{
    struct binder_transaction_data *txn = &txn_secctx->transaction_data;

	/* 根据txn->code知道要调用哪一个函数
	 * 如果需要参数, 可以从msg取出
	 * 如果要返回结果, 可以把结果放入reply
	 */

	/* sayhello
	 * sayhello_to
	 */
	
    uint16_t *s;
	char name[512];
    size_t len;
    //uint32_t handle;
    uint32_t strict_policy;
	int i;


    // Equivalent to Parcel::enforceInterface(), reading the RPC
    // header with the strict mode policy mask and the interface name.
    // Note that we ignore the strict_policy and don't propagate it
    // further (since we do no outbound RPCs anyway).
    strict_policy = bio_get_uint32(msg);

    switch(txn->code) {
    case HELLO_SVR_CMD_SAYHELLO:
		sayhello();
		bio_put_uint32(reply, 0); /* no exception */
        return 0;

    case HELLO_SVR_CMD_SAYHELLO_TO:
		/* 从msg里取出字符串 */
		s = bio_get_string16(msg, &len);  //"IHelloService"
		s = bio_get_string16(msg, &len);  // name
		if (s == NULL) {
			return -1;
		}
		for (i = 0; i < len; i++)
			name[i] = s[i];
		name[i] = '\0';

		/* 处理 */
		i = sayhello_to(name);

		/* 把结果放入reply */
		bio_put_uint32(reply, 0); /* no exception */
		bio_put_uint32(reply, i);
		
        break;

    default:
        fprintf(stderr, "unknown code %d\n", txn->code);
        return -1;
    }

    return 0;
}

int test_server_handler(struct binder_state *bs,
                struct binder_transaction_data_secctx *txn_secctx,
                struct binder_io *msg,
                struct binder_io *reply)
{
    struct binder_transaction_data *txn = &txn_secctx->transaction_data;
	
    int (*handler)(struct binder_state *bs,
                   struct binder_transaction_data *txn,
                   struct binder_io *msg,
                   struct binder_io *reply);

	handler = (int (*)(struct binder_state *bs,
                   struct binder_transaction_data *txn,
                   struct binder_io *msg,
                   struct binder_io *reply))txn->target.ptr;
	
	return handler(bs, txn, msg, reply);
}


int main(int argc, char **argv)
{
    struct binder_state *bs;
    uint32_t svcmgr = BINDER_SERVICE_MANAGER;
    uint32_t handle;
	int ret;

    ALOGW("BinderServer 开始启动");
    
    //打开驱动
    bs = binder_open("/dev/binder", 128*1024);
    if (!bs) {
        fprintf(stderr, "failed to open binder driver\n");
        return -1;
    }

	//添加服务
	ret = svcmgr_publish(bs, svcmgr, "hello", hello_service_handler);
    if (ret) {
        fprintf(stderr, "failed to publish hello service\n");
        return -1;
    } else {
        ALOGW("BinderServer 添加 hello service 成功");
    }
    
    ALOGW("服务器进入消息循环");
    binder_loop(bs, test_server_handler);
    ALOGW("服务器退出消息循环");
    return 0;
}