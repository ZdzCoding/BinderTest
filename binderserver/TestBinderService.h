

#ifndef __SCRSERVICE_H__
#define __SCRSERVICE_H__

#include <utils/RefBase.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>

typedef enum binderMsgId
{
    BINDER_send = 100,
    BINDER_get
}binderMsgId;

namespace android
{
    class TestBinderService : public BBinder
    {
        
    public:
        static int Instance();
        
        static TestBinderService* gScrService;
        
        TestBinderService();
        
        virtual ~TestBinderService();
        
        virtual status_t onTransact(uint32_t, const Parcel&, Parcel*, uint32_t);
    };
}

#endif //__SCRSERVICE_H__


