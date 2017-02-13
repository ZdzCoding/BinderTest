
#include "TestBinderService.h"
#include <binder/IPCThreadState.h>
#include <media/mediarecorder.h>
#include <ui/DisplayInfo.h>
#include <gui/Surface.h>
#include <gui/SurfaceComposerClient.h>
#include <gui/ISurfaceComposer.h>

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <utils/String8.h>

#include <binder/IServiceManager.h>

#define BINDER_TESTSERVICE "TestBinderService"

namespace android
{
    
    TestBinderService* TestBinderService::gScrService = NULL;
    
    int TestBinderService::Instance()
    {
        if(!gScrService)
        {
            gScrService = new TestBinderService();
            int ret = defaultServiceManager()->addService(String16(BINDER_TESTSERVICE), gScrService);
            return ret;
        }
        return 0;
    }
    
    TestBinderService::TestBinderService()
    {
        // printf("TestBinderService::TestBinderService");
    }


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
//*/
    
    TestBinderService::~TestBinderService()
    {
        
    }
}


