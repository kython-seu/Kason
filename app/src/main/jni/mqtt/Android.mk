LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_CFLAGS := -llog -lpthread
LOCAL_LDLIBS := -L$(SYSROOT)/usr/lib -llog
LOCAL_LDLIBS += -pthread
LOCAL_MODULE := mqttjni
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
include $(BUILD_SHARED_LIBRARY)