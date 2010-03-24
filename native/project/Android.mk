LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(call all-subdir-java-files)

LOCAL_JAVA_RESOURCE_DIRS := resources

LOCAL_JAVA_LIBRARIES := core framework

LOCAL_MODULE:= ttsfliteengine

LOCAL_DX_FLAGS := --core-library

include $(BUILD_JAVA_LIBRARY)

# Include subdirectory makefiles
# ============================================================

ifneq ($(SDK_ONLY),true)
  include $(call first-makefiles-under,$(LOCAL_PATH))
endif
