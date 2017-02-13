// #include <stdio.h>
// #include <stdlib.h>
// #include <unistd.h>
#include <stdio.h>
#include <dlfcn.h>
#include <android/log.h>
#include <errno.h>
#include <sys/file.h>
#include <unistd.h>
#include <jni.h>

#include <binder/IPCThreadState.h>
#include "binder/IPCThreadState.h"
#include "binder/IServiceManager.h"
#include "TestBinderClient.h"

using namespace android;

int main()
{
    printf("-------客户端启动---------\n");
    createBinder();
    return 0;
}



