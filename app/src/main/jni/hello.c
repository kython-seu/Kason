//
// Created by kason_zhang on 11/7/2016.
//
#include "com_example_android_mqttjnidemo2_Hello.h"
#include "MQTTAsync.h"
#include "MQTTClientPersistence.h"

#include <stdio.h>
#include <signal.h>
#include <memory.h>


#if defined(WIN32)
#include <windows.h>
#define sleep Sleep
#else
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>
#endif
// 引入log头文件
#include <android/log.h>
// log标签
#define TAG "hello_load"
// 定义info信息
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,TAG,__VA_ARGS__)
// 定义debug信息
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
// 定义error信息
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG,__VA_ARGS__)


volatile int finished = 0;
char* topic = "kason/topic";
int subscribed = 0;
int disconnected = 0;
JavaVM *g_jvm = NULL;
jobject g_obj = NULL;



void cfinish(int sig)
{
    LOGE("SIGINT,ERROR");
    signal(SIGINT, NULL);
    finished = 1;
}


struct
{
    char* clientid;
    int nodelimiter;
    char delimiter;
    int qos;
    char* username;
    char* password;
    char* host;
    char* port;
    int showtopics;
    int keepalive;
} opts =
        {
                "stdout-subscriber-async", 1, '\n', 2, "admin", "password", "10.64.24.138", "61613", 0, 10
        };
void callJavaMethod(jint i){
    LOGI("START to call callJavaMethod");
    //Attach主线程
    JNIEnv *env;
    jclass cls;
    jmethodID mid;
    if((*g_jvm)->AttachCurrentThread(g_jvm, &env, NULL) != JNI_OK)
    {
        LOGE("AttachCurrentThread() failed");
        return;
    }
    LOGI("HERE");
    //找到对应的类
    cls = (*env)->GetObjectClass(env,g_obj);
    if(cls == NULL)
    {
        LOGE("FindClass() Error.....");
        goto error;
    }
    LOGI("get class instance");
    //再找类中的静态方法
    mid = (*env)->GetStaticMethodID(env, cls, "hehe", "(I)V");
    if (mid == NULL)
    {
        LOGE("GetMethodID() Error.....");
        goto error;
    }

    //回掉Java中的静态方法;
    (*env)->CallStaticVoidMethod(env,cls,mid,i);
    //再找类中的非静态方法
    jmethodID mid2 = (*env)->GetMethodID(env, cls, "nativeCallJni", "(I)V");
    if(mid2 == NULL){
        LOGE("GetMethodID() Error.....");
        goto error;
    }
    //回掉Java中的普通方法;
    (*env)->CallVoidMethod(env,g_obj,mid2,i);
    error:
    //Detach主线程
    if((*g_jvm)->DetachCurrentThread(g_jvm) != JNI_OK)
    {
        LOGE("DetachCurrentThread() failed");
    }
}
void callJavaMethod2(char* str){
    LOGI("START to call callJavaMethod");

    //Attach主线程
    JNIEnv *env;
    jclass cls;
    jmethodID mid;
    if((*g_jvm)->AttachCurrentThread(g_jvm, &env, NULL) != JNI_OK)
    {
        LOGE("AttachCurrentThread() failed");
        return;
    }
    LOGI("HERE");
    str = "123456789kkkklll";
    jstring str_ = (*env)->NewStringUTF(env,str);
    LOGI("STR IS %s",str);
            //找到对应的类
    cls = (*env)->GetObjectClass(env,g_obj);
    if(cls == NULL)
    {
        LOGE("FindClass() Error.....");
        goto error;
    }
    LOGI("get class instance");

    //再找类中的非静态方法
    jmethodID mid2 = (*env)->GetMethodID(env, cls, "nativeCallJni2", "(Ljava/lang/String;)V");
    if(mid2 == NULL){
        LOGE("GetMethodID() Error.....");
        goto error;
    }
    //回掉Java中的普通方法;
    (*env)->CallVoidMethod(env,g_obj,mid2,str_);
    error:
    //Detach主线程
    if((*g_jvm)->DetachCurrentThread(g_jvm) != JNI_OK)
    {
        LOGE("DetachCurrentThread() failed");
    }
}
int messageArrived(void *context, char *topicName, int topicLen, MQTTAsync_message *message)
{
    if (opts.showtopics) {
        LOGI("%s\t", topicName);
        printf("%s\t", topicName);
    }
    callJavaMethod((jint)1000);
    if (opts.nodelimiter) {
        LOGI("%s",(char *) message->payload);
        callJavaMethod2(message->payload);
        printf("%.*s", message->payloadlen, (char *) message->payload);
    } else {
        LOGI("%s",(char *) message->payload);

        callJavaMethod2(message->payload);
        printf("%.*s%c", message->payloadlen, (char *) message->payload, opts.delimiter);
    }
    fflush(stdout);
    MQTTAsync_freeMessage(&message);
    MQTTAsync_free(topicName);
    return 1;
}


void onDisconnect(void* context, MQTTAsync_successData* response)
{
    LOGI("onDisconnect has triggered");
    disconnected = 1;
}


void onSubscribe(void* context, MQTTAsync_successData* response)
{
    subscribed = 1;
}


void onSubscribeFailure(void* context, MQTTAsync_failureData* response)
{
    printf("Subscribe failed, rc %d\n", response->code);
    finished = 1;
}


//response null point?
void onConnectFailure(void* context, MQTTAsync_failureData* response)
{
//    printf("Connect failed, rc %d\n", response->code);

    LOGE("onConnectFailure");
    //LOGI("Connect failed, rc %d\n", response->code);
    finished = 1;
}


void onConnect(void* context, MQTTAsync_successData* response)
{
    MQTTAsync client = (MQTTAsync)context;
    MQTTAsync_responseOptions ropts = MQTTAsync_responseOptions_initializer;
    MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
    int rc;

    if (opts.showtopics)
        printf("Subscribing to topic %s with client %s at QoS %d\n", topic, opts.clientid, opts.qos);

    ropts.onSuccess = onSubscribe;
    ropts.onFailure = onSubscribeFailure;
    ropts.context = client;
    if ((rc = MQTTAsync_subscribe(client, topic, opts.qos, &ropts)) != MQTTASYNC_SUCCESS)
    {
        printf("Failed to start subscribe, return code %d\n", rc);
        finished = 1;
    }
}


MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;


void connectionLost(void *context, char *cause)
{
    MQTTAsync client = (MQTTAsync)context;
    int rc;

    //printf("connectionLost called\n");
    LOGE("connection lost because of no network %s",cause);
    if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS)
    {
        //printf("Failed to start reconnect, return code %d\n", rc);
        LOGE("connection lost because of no network");
        finished = 1;
    }
}

JNIEXPORT jint JNICALL Java_com_example_android_mqttjnidemo2_Hello_add
        (JNIEnv * env, jobject thiz, jint a, jint b){
    int re = a+b;
    return (jint)re;
}

JNIEXPORT jint JNICALL Java_com_example_android_mqttjnidemo2_Hello_subscribe
        (JNIEnv * env, jobject thiz){

    //保存全局JVM以便在子线程中使用
    (*env)->GetJavaVM(env,&g_jvm);
    //不能直接赋值(g_obj = obj)
    g_obj = (*env)->NewGlobalRef(env,thiz);

    MQTTAsync client;
    MQTTAsync_disconnectOptions disc_opts = MQTTAsync_disconnectOptions_initializer;
    int rc = 0;
    char url[100];

    //if (argc < 2)
    //	usage();

    //topic = argv[1];
    //topic = "kason/topic";
    opts.showtopics = 1;
    /*if (strchr(topic, '#') || strchr(topic, '+'))
        opts.showtopics = 1;
    if (opts.showtopics)
        printf("topic is %s\n", topic);*/

    //getopts(argc, argv);
    sprintf(url, "%s:%s", opts.host, opts.port);
    LOGI("Start Create");
    rc = MQTTAsync_create(&client, url, opts.clientid, MQTTCLIENT_PERSISTENCE_NONE, NULL);

    MQTTAsync_setCallbacks(client, client, connectionLost, messageArrived, NULL);
    LOGI("SET CALLBACK");
    signal(SIGINT, cfinish);
    signal(SIGTERM, cfinish);
    signal(SIGSEGV,cfinish);
    conn_opts.keepAliveInterval = opts.keepalive;
    conn_opts.cleansession = 1;
    conn_opts.username = opts.username;
    conn_opts.password = opts.password;
    conn_opts.onSuccess = onConnect;
    conn_opts.onFailure = onConnectFailure;
    conn_opts.context = client;
    LOGI("GOING TO CONNECT");
    if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS)
    {
        LOGI("fAILED %d",rc);
        printf("Failed to start connect, return code %d\n", rc);
        //exit(EXIT_FAILURE);
        return (jint)4999;
    }
    LOGI("AFTER MQTTAsync_connect %d",rc);
    while (!subscribed)
        #if defined(WIN32)
            Sleep(100);
        #else
            usleep(10000L);
        #endif

    if (finished) {
        LOGE("finished");
        goto exit;
    }

    while (!finished)
        #if defined(WIN32)
            Sleep(100);
        #else
            usleep(10000L);
        #endif

    disc_opts.onSuccess = onDisconnect;
    if ((rc = MQTTAsync_disconnect(client, &disc_opts)) != MQTTASYNC_SUCCESS)
    {
        printf("Failed to start disconnect, return code %d\n", rc);
        LOGI("going to dead");
        //exit(EXIT_FAILURE);//这句是退出程序
        finished = 0;
        MQTTAsync_destroy(&client);//推出前先销毁Client
        return (jint)1;
    }

    while	(!disconnected)
        #if defined(WIN32)
            Sleep(100);
        #else
            usleep(10000L);
        #endif
    exit:
    MQTTAsync_destroy(&client);

    return EXIT_SUCCESS;
}
JNIEXPORT void JNICALL Java_com_example_android_mqttjnidemo2_Hello_reconnect
        (JNIEnv *env, jobject thiz){

}
//当动态库被加载时这个函数被系统调用
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved)
{
    JNIEnv* env = NULL;
    jint result = -1;

    //获取JNI版本
    if ((*vm)->GetEnv(vm, (void**)&env, JNI_VERSION_1_6) != JNI_OK)
    {
        LOGE("GetEnv failed!");
        return result;
    }

    return JNI_VERSION_1_6;
}
