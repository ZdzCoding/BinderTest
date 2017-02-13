
#include "TestBinderClient.h"

#include "TestBinderClient.h"
#include <utils/String8.h>

#include <binder/IServiceManager.h>
#include <binder/IPCThreadState.h>
#include "TestBinderClient.h"

// #define  K_java_class_name  "com/example/bindertest/ScrNative"

#define  NDK_LOG(...)  __android_log_print(ANDROID_LOG_ERROR,"binder",__VA_ARGS__)

#define BINDER_TESTSERVICE "TestBinderService"

typedef enum binderMsgId
{
    BINDER_send = 100,
    BINDER_get
}binderMsgId;

namespace android
{
    class TestBinderClient
    {
    public:
        int sendMsg();
        int getMsg();
        
    public:
        TestBinderClient();
        ~TestBinderClient();
        
    private:
        static void getScrService();
    };
    
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
        // printf("TestBinderClient::%s\n", __FUNCTION__);
        sp<IServiceManager> sm = defaultServiceManager();
        binder = sm->getService(String16(BINDER_TESTSERVICE));
        if(binder == 0)
        {
            printf("getScrService failed\n");
            return;
        }
    }
    
//--------------------------------------------
    
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
}

using namespace android;

int createBinder()
{
    TestBinderClient* client = new TestBinderClient();
    client->sendMsg();
    // printf("收到 = %d\n", aa);
    client->getMsg();
    // printf("收到 = %d\n", bb);
    return (int)client;
}



