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
    sp<ProcessState> proc(ProcessState::self());
    sp<IServiceManager> sm = defaultServiceManager();
    TestBinderService::Instance();
    ProcessState::self()->startThreadPool();
    IPCThreadState::self()->joinThreadPool(true);
    return 0;
}



