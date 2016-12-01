LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libssl_static
LOCAL_SRC_FILES := thirdparty/extlibs/armeabi/libssl_static.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libcrypto_static
LOCAL_SRC_FILES := thirdparty/extlibs/armeabi/libcrypto_static.a
include $(PREBUILT_STATIC_LIBRARY)


include $(CLEAR_VARS)
LOCAL_CFLAGS := -llog -lpthread -DOPENSSL
LOCAL_LDLIBS := -L$(SYSROOT)/usr/lib -llog
LOCAL_LDLIBS += -pthread
LOCAL_MODULE := mqttjni
LOCAL_REQUIRED_MODULES := libssl_static
LOCAL_SRC_FILES := MQTTProtocolClient.c \
                Clients.c \
                utf-8.c \
                StackTrace.c \
                MQTTPacket.c \
                MQTTPacketOut.c \
                Messages.c \
                Tree.c \
                Socket.c \
                Log.c \
                MQTTPersistence.c \
                Thread.c \
                MQTTProtocolOut.c \
                MQTTPersistenceDefault.c \
                SocketBuffer.c \
                Heap.c \
                LinkedList.c \
                MQTTAsync.c \
                SSLSocket.c \
                MqttEngine.c

LOCAL_WHOLE_STATIC_LIBRARIES := libssl_static libcrypto_static
include $(BUILD_SHARED_LIBRARY)