转载请注明出处：http://blog.csdn.net/zhaodai11?viewmode=contents

Binder是Android系统独有的一种IPC通信机制，贯穿在整个Android系统中。

Binder通信使用C/S架构，除了C/S架构所包括的Client端和Server端外，Android还有一个ServiceManager端，用来注册和查询服务。（注意这里的ServiceManager是指底层和驱动交互实现服务的注册和查询，并非Java类中的ServiceManager，这点很容易搞混）下面这张来自邓凡平老师博客的图片可以形象描绘出他们三者之间的关系。

![ Client、Server和ServiceManager三者之间的交互关系](http://img.blog.csdn.net/20150802155713567?watermark/2/text/aHR0cDovL2Jsb2cuY3Nkbi5uZXQv/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70/gravity/Center)

根据上面的图，可以看出：

 1. Server进程注册服务到ServiceManager，此时Server是ServiceManager的客户端，ServiceManager是服务端。
 2. Client进程使用服务，必须先要通过ServerManager获取相应的服务信息。此时Client是客户端，ServiceManager是服务端。
 3. Client根据得到的服务信息建立与服务所在的Server进程通信的通路，然后就可以直接与Service交互了，此时Client是客户端，Server是服务端。
 
 
上面提到的ServiceManager很容易被大家误以为是Java中ServiceManager，它真正的实现是在 source/android-6.0.1_r17/frameworks/native/cmds/servicemanager/service_manager.c中


```
//...

//svcmgr_handler是真正负责查找和添加服务信息的函数
int svcmgr_handler(struct binder_state *bs,
                   struct binder_transaction_data *txn,
                   struct binder_io *msg,
                   struct binder_io *reply)
{
    struct svcinfo *si;
    uint16_t *s;
    size_t len;
    uint32_t handle;
    uint32_t strict_policy;
    int allow_isolated;

    //ALOGI("target=%p code=%d pid=%d uid=%d\n",
    //      (void*) txn->target.ptr, txn->code, txn->sender_pid, txn->sender_euid);

    if (txn->target.ptr != BINDER_SERVICE_MANAGER)
        return -1;

    if (txn->code == PING_TRANSACTION)
        return 0;

    // Equivalent to Parcel::enforceInterface(), reading the RPC
    // header with the strict mode policy mask and the interface name.
    // Note that we ignore the strict_policy and don't propagate it
    // further (since we do no outbound RPCs anyway).
    strict_policy = bio_get_uint32(msg);
    s = bio_get_string16(msg, &len);
    if (s == NULL) {
        return -1;
    }

    if ((len != (sizeof(svcmgr_id) / 2)) ||
        memcmp(svcmgr_id, s, sizeof(svcmgr_id))) {
        fprintf(stderr,"invalid id %s\n", str8(s, len));
        return -1;
    }

    if (sehandle && selinux_status_updated() > 0) {
        struct selabel_handle *tmp_sehandle = selinux_android_service_context_handle();
        if (tmp_sehandle) {
            selabel_close(sehandle);
            sehandle = tmp_sehandle;
        }
    }

    switch(txn->code) {
    //获取某个Service信息
    case SVC_MGR_GET_SERVICE:
    case SVC_MGR_CHECK_SERVICE:
        s = bio_get_string16(msg, &len);
        if (s == NULL) {
            return -1;
        }
        handle = do_find_service(bs, s, len, txn->sender_euid, txn->sender_pid);
        if (!handle)
            break;
        bio_put_ref(reply, handle);
        return 0;
//添加service到servicemanager
    case SVC_MGR_ADD_SERVICE:
        s = bio_get_string16(msg, &len);
        if (s == NULL) {
            return -1;
        }
        handle = bio_get_ref(msg);
        allow_isolated = bio_get_uint32(msg) ? 1 : 0;
        if (do_add_service(bs, s, len, handle, txn->sender_euid,
            allow_isolated, txn->sender_pid))
            return -1;
        break;
//获取当前系统已经注册的所有service名称
    case SVC_MGR_LIST_SERVICES: {
        uint32_t n = bio_get_uint32(msg);

        if (!svc_can_list(txn->sender_pid)) {
            ALOGE("list_service() uid=%d - PERMISSION DENIED\n",
                    txn->sender_euid);
            return -1;
        }
        si = svclist;
        while ((n-- > 0) && si)
            si = si->next;
        if (si) {
            bio_put_string16(reply, si->name);
            return 0;
        }
        return -1;
    }
    default:
        ALOGE("unknown code %d\n", txn->code);
        return -1;
    }

    bio_put_uint32(reply, 0);
    return 0;
}
//.....

```

以上是简单介绍，详细原理，给大家推荐两个讲解Binder比较全面细致的博客：
邓凡平老师：http://blog.csdn.net/innost/article/details/47208049
罗升阳老师：http://blog.csdn.net/Luoshengyang/article/list/4

在Android系统源码中，使用Binder时用了很多的代理，包括在很多博客的示例中也一样，让人感觉眼花缭乱。其实弄懂原理不用写代理那些也能实现。

服务端：

```
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <binder/IPCThreadState.h>
#include "binder/IPCThreadState.h"
#include "binder/IServiceManager.h"
#include "TestBinderService.h"

using namespace android;

int main()
{
    printf("-------服务端启动---------\n");
    // 获得一个ProcessState实例
    sp<ProcessState> proc(ProcessState::self());
    //调用defaultServiceManager获取ServiceManger 
    sp<IServiceManager> sm = defaultServiceManager();
    
    //创建TestBinderService服务 将服务注册到ServiceManager中
    TestBinderService::Instance();
	//创建一个线程池
    ProcessState::self()->startThreadPool();
    //
    IPCThreadState::self()->joinThreadPool(true);
    return 0;
}

```

TestBinderService::Instance()实现

```

#define BINDER_TESTSERVICE "TestBinderService"

namespace android
{
    
    TestBinderService* TestBinderService::gScrService = NULL;
    
    int TestBinderService::Instance()
    {
        if(!gScrService)
        {
	        //创建服务
            gScrService = new TestBinderService();
            //将服务注册到serviceManager中
            int ret = defaultServiceManager()->addService(String16(BINDER_TESTSERVICE), gScrService);
            return ret;
        }
        return 0;
    }
    //.......
    //与客户端进行通信
    status_t TestBinderService::onTransact(uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
    {
        printf("命令码 code=%d\n",code);
        switch (code)
        {
            case BINDER_send:
            {
                int ivalue = data.readInt32();
                String8 s = data.readString8();
                // string str(s);
                const char *cptr = s; 
                printf("服务端接收信息：数字 = %d 字符串 = %s\n", ivalue,cptr);
                // reply->writeInt32(200);
                String8 msg("收到了 谢谢！！！");
                reply->writeString8(msg);
            }
                break;
                
            case BINDER_get:
            {
                // int ivalue = data.readInt32();
                String8 s = data.readString8();
                const char *cptr = s; 
                printf("服务端接收信息：信息 = %s\n",cptr);
                reply->writeInt32(600);
                String8 msg("给你发个信息");
                reply->writeString8(msg);

            }
                break;
                
            default:
                break;
        }
        return 0;
    }


}

```

客户端：

```
	//获取相关服务信息 
	TestBinderClient* client = new TestBinderClient();
	//与服务端通信
    client->sendMsg();
    client->getMsg();
    return (int)client;
```

TestBinderClient.cpp

```
	sp<IBinder> binder;
    
    TestBinderClient::TestBinderClient()
    {
        // printf("TestBinderClient::%s\n", __FUNCTION__);
        getScrService();
    }
    
    
    TestBinderClient::~TestBinderClient()
    {
        // printf("TestBinderClient::%s\n", __FUNCTION__);
        binder = 0;
    }
    
    void TestBinderClient::getScrService()
    {
        //获取serviceManager
        sp<IServiceManager> sm = defaultServiceManager();
        //查找相关服务信息
        binder = sm->getService(String16(BINDER_TESTSERVICE));
        if(binder == 0)
        {
            printf("getScrService failed\n");
            return;
        }
    }
    
    //使用服务进行进程间通信
    int TestBinderClient::sendMsg()
    {
        // printf("TestBinderClient::%s\n", __FUNCTION__);
        Parcel data, reply;
        data.writeInt32(100);
        String8 msg("给你发个信息");
        data.writeString8(msg);
        binder->transact(BINDER_send, data, &reply);
        // int ret = reply.readInt32();
        String8 s = reply.readString8();
        const char *cptr = s; 
        printf("客户端接收回复信息：信息 = %s\n",cptr);
        return 0;
    }

    int TestBinderClient::getMsg()
    {
        // printf("TestBinderClient::%s\n", __FUNCTION__);
        Parcel data, reply;
        // data.writeInt32(0);
        String8 msg("给我来个信息");
        data.writeString8(msg);
        binder->transact(BINDER_get, data, &reply);
        int ret = reply.readInt32();
        String8 s = reply.readString8();
        const char *cptr = s; 
        printf("客户端接收回复信息：数字 = %d 字符串 = %s\n",ret,cptr);
        return ret;
    }
```

运行效果图：
服务端：
![服务端启动](http://img.blog.csdn.net/20170213153807403?watermark/2/text/aHR0cDovL2Jsb2cuY3Nkbi5uZXQvemhhb2RhaTEx/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70/gravity/SouthEast)

客户端：
![客户端启动](http://img.blog.csdn.net/20170213153629449?watermark/2/text/aHR0cDovL2Jsb2cuY3Nkbi5uZXQvemhhb2RhaTEx/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70/gravity/SouthEast)

编译代码需要在源码环境下编译，将服务端和客户端编译成两个可执行文件，push到手机中执行。

当然也可以增加编写JNI接口，编译成静态库文件，在APP中进行调用。不过这样需要APP获取ROOT权限或者将APP变成系统应用。

相关代码已经传到github: https://github.com/zhaodaizheng/BinderTest







