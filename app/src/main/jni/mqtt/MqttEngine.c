//
// Created by kason_zhang on 11/23/2016.
//
#include "mqttPush_MqttEngine.h"
#include "MQTTAsync.h"
#include "MQTTClientPersistence.h"
#include "MQTTClient.h"

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
// Android log head file
#include <android/log.h>
// log name
#define TAG "MqttEngineJNI"
// info message
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,TAG,__VA_ARGS__)
// debug message
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
// error message
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG,__VA_ARGS__)
//apoint : -1 :fail 1: successfully
static JavaVM *g_jvm = NULL;
static jobject g_obj = NULL;
static volatile int finished = 0;
static int subscribed = 0;
static int disconnected = 0;
static char *topic = "trend/kason/zhang";

void cfinish(int sig) {
    LOGE("SIGINT,ERROR start to disconnect");
    signal(SIGINT, NULL);
    finished = 1;
}

struct {
    char *clientid;
    int nodelimiter;
    char delimiter;
    int qos;
    char *username;
    char *password;
    char *host;
    char *port;
    int showtopics;
    int keepalive;
} opts = {
        "stdout-subscriber-async", 1, '\n', 2, NULL, NULL, "10.64.24.53", "8883", 0, 1800
};

int messageArrived(void *context, char *topicName, int topicLen, MQTTAsync_message *message) {
    if (opts.showtopics) {
        LOGI("%s\t", topicName);
        printf("%s\t", topicName);
    }
    //callJavaMethod((jint)1000);
    LOGI("message length is %d",message->payloadlen);

    if (opts.nodelimiter) {
        LOGI("nodelimiter%s", (char *)message->payload);
        //callJavaMethod2(message->payload);
        printf("%.*s", message->payloadlen, (char *) message->payload);
    } else {
        LOGI("no_nodelimiter%s", (char *)message->payload);

        //callJavaMethod2(message->payload);
        printf("%.*s%c", message->payloadlen, (char *) message->payload, opts.delimiter);
    }
    fflush(stdout);
    MQTTAsync_freeMessage(&message);
    MQTTAsync_free(topicName);
    return 1;
}

//disconnect callback
void onDisconnect(void *context, MQTTAsync_successData *response) {
    LOGI("onDisconnect has triggered");
    disconnected = 1;
}

//subscribe callback
void onSubscribe(void *context, MQTTAsync_successData *response) {
    LOGI("onSubscribe");
    subscribed = 1;
}

//subscribe fail callback
void onSubscribeFailure(void *context, MQTTAsync_failureData *response) {
    printf("Subscribe failed start to disconnect, rc %d\n", response->code);
    finished = 1;
}

//connect fail callback
void onConnectFailure(void *context, MQTTAsync_failureData *response) {
    // printf("Connect failed, rc %d\n", response->code);//don't print this,it will cause
    //null point exception when no network
    LOGE("onConnectFailure start to disconnect");
    //LOGI("Connect failed, rc %d\n", response->code);
    finished = 1;
}

//start to subscribe
void onConnect(void *context, MQTTAsync_successData *response) {
    MQTTAsync client = (MQTTAsync) context;
    MQTTAsync_responseOptions ropts = MQTTAsync_responseOptions_initializer;
    MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
    int rc;

    if (opts.showtopics)
        printf("Subscribing to topic %s with client %s at QoS %d\n", topic, opts.clientid,
               opts.qos);

    ropts.onSuccess = onSubscribe;
    ropts.onFailure = onSubscribeFailure;
    ropts.context = client;
    if ((rc = MQTTAsync_subscribe(client, topic, opts.qos, &ropts)) != MQTTASYNC_SUCCESS) {
        printf("Failed to start subscribe, return code %d\n", rc);
        LOGE("Failed to start subscribe start to disconnect, return code %d\n", rc);
        finished = 1;
    }
}

static int published = 0;
void onSendFailure(void *context, MQTTAsync_failureData *response) {
    //printf("Publish failed, rc %d\n", response ? -1 : response->code);
    LOGE("Publish fail, onPublishFailure called");
    published = -1;
}

void onSend(void *context, MQTTAsync_successData *response) {
    MQTTAsync client = (MQTTAsync) context;
    published = 1;
}

//connection lost callback
void connectionLost(void *context, char *cause) {
    MQTTAsync client = (MQTTAsync) context;
    MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
    int rc;
    //printf("connectionLost called\n");//don't print this,it will cause null point exception
    //when there is no network;
    LOGE("connection lost because of no network %s", cause);
    if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS) {
        //printf("Failed to start reconnect, return code %d\n", rc);
        LOGE("connection lost because of no network start to disconnect");
        //do something callback to Java and close the Connect
        finished = 1;
    }

}

//Connection State Callback to call Java
void onStartConnectCallback(JNIEnv *env) {
    //Attach to get
    jclass cls;
    jmethodID mid;
    cls = (*env)->GetObjectClass(env, g_obj);
    if (cls == NULL) {
        LOGE("onConnectCallBack method get class fail");
        return;
    }
    mid = (*env)->GetMethodID(env, cls, "onStartConnect", "()V");
    if (mid == NULL) {
        LOGE("onConnectCallBack method get method id fail");
        return;
    }
    if (g_obj == NULL) {
        LOGE("onConnectCallBack method Null global object");
        return;
    }
    (*env)->CallVoidMethod(env, g_obj, mid);
}

void onFinishedConnectCallback(JNIEnv *env) {
    //Attach to get
    jclass cls;
    jmethodID mid;
    cls = (*env)->GetObjectClass(env, g_obj);
    if (cls == NULL) {
        LOGE("onFinishedConnect method get class fail");
        return;
    }
    mid = (*env)->GetMethodID(env, cls, "onFinishedConnect", "()V");
    if (mid == NULL) {
        LOGE("onFinishedConnect method get method id fail");
        return;
    }
    if (g_obj == NULL) {
        LOGE("onFinishedConnect method Null global object");
        return;
    }
    (*env)->CallVoidMethod(env, g_obj, mid);

}

JNIEXPORT jboolean JNICALL Java_mqttPush_MqttEngine_jniInit
        (JNIEnv *env, jobject thiz) {
    //保存全局JVM以便在子线程中使用
    (*env)->GetJavaVM(env, &g_jvm);
    //不能直接赋值(g_obj = thiz)
    g_obj = (*env)->NewGlobalRef(env, thiz);
    LOGI("MQTT jni completed");
    return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL Java_mqttPush_MqttEngine_jniDeInit
        (JNIEnv *env, jobject thiz) {
    (*env)->DeleteGlobalRef(env, g_obj);
    g_jvm = NULL;
    LOGI("MQTT JNI deinit completed");
    return JNI_TRUE;
}
MQTTAsync client;
MQTTAsync_disconnectOptions disc_opts = MQTTAsync_disconnectOptions_initializer;
MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
MQTTAsync_willOptions wopts = MQTTAsync_willOptions_initializer;// will topic and will message
MQTTAsync_SSLOptions ssl_opts = MQTTAsync_SSLOptions_initializer;//SSL
JNIEXPORT jint JNICALL Java_mqttPush_MqttEngine_connect
        (JNIEnv *env, jobject thiz) {
    LOGI("open ssl, test it");
    int rc = 0;
    char url[100];
    opts.showtopics = 1;
    sprintf(url, "%s://%s:%s", "ssl",opts.host, opts.port);
    LOGI("Start to Create connection");
    LOGI("url is %s ", url);
    if (client == NULL) {
        LOGI("client == null");
    } else {
        LOGI("CLIENT != NULL");
    }
    rc = MQTTAsync_create(&client, url, opts.clientid, MQTTCLIENT_PERSISTENCE_NONE, NULL);

    MQTTAsync_setCallbacks(client, client, connectionLost, messageArrived, NULL);
    //LOGI("SET SIGINT SIGTERM SIGSEGV Exception CALLBACK");
    signal(SIGINT, cfinish);
    signal(SIGTERM, cfinish);
    signal(SIGSEGV, cfinish);
    conn_opts.keepAliveInterval = opts.keepalive;
    conn_opts.cleansession = 1;
    conn_opts.username = opts.username;
    conn_opts.password = opts.password;
    conn_opts.onSuccess = onConnect;
    conn_opts.onFailure = onConnectFailure;
    /**
     * set will configration
     */
    conn_opts.will = &wopts;
    conn_opts.will->message = "will message";
    conn_opts.will->qos = 1;
    conn_opts.will->retained = 0;
    conn_opts.will->topicName = "will topic";
    conn_opts.will = NULL;
    /**
     * ssl enable?
     */
    ssl_opts.enableServerCertAuth = 1;
    conn_opts.ssl = &ssl_opts;
    char *server_key_file = "/mnt/sdcard/mqttca/ca.pem";
    conn_opts.ssl->trustStore = server_key_file;
    char *client_key_file = "/mnt/sdcard/mqttca/client.pem";
    conn_opts.ssl->keyStore = client_key_file;
    conn_opts.ssl->privateKey = "/mnt/sdcard/mqttca/client.key";
    //conn_opts.ssl->privateKeyPassword = "cs";
    ssl_opts.enabledCipherSuites = "TLSv1";

    conn_opts.context = client;

    LOGI("GOING TO CONNECT");
    onStartConnectCallback(env);
    if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS) {
        LOGI("fAILED %d", rc);
        printf("Failed to start connect, return code %d\n", rc);
        //exit(EXIT_FAILURE);
        return (jint)-1;
    }else {
        LOGI("MQTT connect successfully");
        onFinishedConnectCallback(env);
    }
    LOGI("after MQTTAsync_connect rc = %d", rc);
    while (!subscribed)
#if defined(WIN32)
        Sleep(100);
#else
        usleep(10000L);
#endif
    LOGI("do sending");
    /*MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
    opts.onSuccess = onSend;
    opts.onFailure = onSendFailure;
    opts.context = client;
    MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
    pubmsg.payload = "hello";
    pubmsg.payloadlen = 6;
    pubmsg.qos = 2;
    pubmsg.retained = 0;
    if ((rc = MQTTAsync_sendMessage(client, topic, &pubmsg, &opts)) != MQTTASYNC_SUCCESS) {
        LOGE("fAILED TO START SEND mESSAGE rc = %d\n", rc);
    }*/
    return EXIT_SUCCESS;
}

JNIEXPORT jboolean JNICALL Java_mqttPush_MqttEngine_disconnect
        (JNIEnv *env, jobject thiz){
    //finished = 1;
    LOGI("java call disconnect");
    if (client == NULL) {
        LOGI("It seems that you haven't establish creation beetween Android Client and Broker");
        return 0;
    }
    int rc;
    disc_opts.onSuccess = onDisconnect;
    if ((rc = MQTTAsync_disconnect(client, &disc_opts)) != MQTTASYNC_SUCCESS) {
        printf("Failed to start disconnect, return code %d\n", rc);
        LOGI("Java_mqttPush_MqttEngine_disconnect java call disconnect");
        //exit(EXIT_FAILURE);//这句是退出程序
        finished = 0;
        MQTTAsync_destroy(&client);//推出前先销毁Client
        LOGI("destroy client successfully");
        return (jint) 1;
    }
    LOGI("Destroy client");
    MQTTAsync_destroy(&client);//推出前先销毁Client
    LOGI("Destroy over");
    return 1;
}
//Send Message to Server
JNIEXPORT void JNICALL Java_mqttPush_MqttEngine_sendMessage
        (JNIEnv *env, jobject thiz,jstring responseToServer){
    if (client == NULL) {
        LOGI("It seems that you haven't establish creation beetween Android Client and Broker");
        return;
    }
    //get the data
    //char *c_responseToServer = jstringTostring(env,responseToServer);
    char *c_responseToServer = (char*)(*env)->GetStringUTFChars(env,responseToServer,NULL);

    LOGI("jstring to char* %s",c_responseToServer);
    (*env)->ReleaseStringUTFChars(env,responseToServer,c_responseToServer);
    int rc;
    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
    opts.onSuccess = onSend;
    opts.onFailure = onSendFailure;
    opts.context = client;
    MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
    pubmsg.payload = c_responseToServer;
    //pubmsg.payload = c_responseToServer;
    pubmsg.payloadlen = strlen(c_responseToServer);//message length
    pubmsg.qos = 2;
    pubmsg.retained = 0;

    if ((rc = MQTTAsync_sendMessage(client, topic, &pubmsg, &opts)) != MQTTASYNC_SUCCESS) {
        LOGE("FAILED TO START SEND MESSAGE rc = %d\n", rc);
        return;
    }
    LOGI("Push Message Successfully");
}
//当动态库被加载时这个函数被系统调用
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *env = NULL;
    jint result = -1;
    //获取JNI版本
    if ((*vm)->GetEnv(vm, (void **) &env, JNI_VERSION_1_6) != JNI_OK) {
        LOGE("GetEnv failed!");
        return result;
    }
    return JNI_VERSION_1_6;
}

