
LOCAL_PATH:=$(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := main.cpp \
	TestBinderService.cpp
	
LOCAL_SHARED_LIBRARIES := \
    libutils \
    libcutils \
    libbinder 

LOCAL_MODULE := binderserver
LOCAL_MODULE_TAGS := optional

include $(BUILD_EXECUTABLE)


