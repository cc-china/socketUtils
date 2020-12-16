#include <jni.h>
#include <string>
#include <android/log.h>
#include <fcntl.h>
#include "include/socketglobal.h"
#include "include/socketserver.h"
#include <stdio.h>
#include <stdlib.h>
#define  LOG_TAG    "JNI_socket_utils"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
typedef struct {
    JNIEnv *pEnv;
    jobject pJobject;
    jmethodID socketConnectStatus;
    jmethodID socketReceiverData;
} IMPLPAR;
IMPLPAR *implParm = NULL;
JavaVM *java_vm;
class SocketServerCallbackImpl : public ISocketServerCallBack {
public:
    IMPLPAR *mp_implPar;

    explicit SocketServerCallbackImpl(IMPLPAR *implPar) {
        mp_implPar = implPar;
    }

    void socketReceiverData(const char *data) override {
        LOGE("socketReceiverData111 : %s", data);
        java_vm->AttachCurrentThread(&mp_implPar->pEnv, NULL);
        jstring jdata = mp_implPar->pEnv->NewStringUTF(data);
        mp_implPar->pEnv->CallVoidMethod(mp_implPar->pJobject, mp_implPar->socketReceiverData,
                                         jdata);
        LOGE("socketReceiverData : %s", data);
        //mp_implPar->pEnv->ReleaseStringChars(jdata, reinterpret_cast<const jchar *>(data));
    }

    void socketConnectStatus(const int flag) override {
        java_vm->AttachCurrentThread(&mp_implPar->pEnv, NULL);
        if (mp_implPar !=NULL) {
            mp_implPar->pEnv->CallVoidMethod(mp_implPar->pJobject, mp_implPar->socketConnectStatus,
                                             flag);
        } else {
            LOGE("socketConnectStatus : %d", flag);
        }
    }
};

bool jni_closeServer(JNIEnv *env, jclass clazz) {
    (void) clazz;
    (void) env;
    SocketServer *socketServer = SocketServer::getInstance();
    return socketServer->closeSocket();
}

SocketServerCallbackImpl *mp_socketServerCallbackImpl = nullptr;
const char *mp_callbackClassName = "com/android/socket/ISocketCallBack";

bool jni_startServer(JNIEnv *env, jclass clazz, jint port, jobject callback) {

    (void) clazz;
    jclass clazzCallBack = env->FindClass(mp_callbackClassName);
    if (clazzCallBack == nullptr) {
        LOGE("cannot get class : %s", mp_callbackClassName);
        return false;
    }
    jmethodID socketConnectStatus = env->GetMethodID(clazzCallBack, "socketConnectStatus", "(I)V");
    jmethodID socketReceiverData = env->GetMethodID(clazzCallBack, "socketReceiverData",
                                                    "(Ljava/lang/String;)V");
    if (socketConnectStatus == nullptr || socketReceiverData == nullptr) {
        LOGE("cannot get Method ");
        return false;
    }
    if (implParm == NULL) {
        implParm = new IMPLPAR;
    }
    //implParm->pEnv = env;
    implParm->pJobject = env->NewGlobalRef(callback);
    implParm->socketReceiverData = socketReceiverData;
    implParm->socketConnectStatus = socketConnectStatus;
    SocketServer *socketServer = SocketServer::getInstance();
    if (mp_socketServerCallbackImpl == nullptr) {
        mp_socketServerCallbackImpl = new SocketServerCallbackImpl(implParm);
    }
    socketServer->setSocketCallback(mp_socketServerCallbackImpl);
    socketServer->createSocketServer(port == 0 ? fixed_port : port);
    return socketServer != nullptr;
}

bool jni_sendData(JNIEnv *env, jclass clazz, jstring sendData){
    SocketServer *socketServer = SocketServer::getInstance();
    if (socketServer == nullptr){
        return false;
    }
    const char * data = env->GetStringUTFChars(sendData, JNI_FALSE);
    return socketServer->sendDataToClient(const_cast<char *>(data));
}

const char *jniClassName = "com/android/socket/SocketUtils";
const JNINativeMethod getMethods[] = {{"jni_startServer", "(ILcom/hyn/tp_updatefw/socket/ISocketCallBack;)Z", (void *) jni_startServer},
                                      {"jni_closeServer", "()Z",                                              (void *) jni_closeServer},
                                      {"jni_sendData",    "(Ljava/lang/String;)Z",                            (void *) jni_sendData}
};

jint RegisterNative(JNIEnv *env) {
    //获取映射的Java类
    jclass javaClass = env->FindClass(jniClassName);
    if (javaClass == nullptr) {
        LOGE("cannot get class : %s", jniClassName);
        return -1;
    }
    return env->RegisterNatives(javaClass, getMethods, sizeof(getMethods) / sizeof(getMethods[0]));
}

//jni函数入口，类似main函数
jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    LOGI("jni_onLoad_start");
    (void) reserved;
    //注册时在JNIEnv中实现，所以必须首先获取它
    JNIEnv *env = nullptr;
    jint result = -1;
    java_vm = vm;
    //从JavaVM中获取JNIEnv，一般使用1.4的版本
    if (vm->GetEnv((void **) &env, JNI_VERSION_1_4) != JNI_OK) {
        return result;
    }
    result = RegisterNative(env);
    LOGI("RegisterNatives result:%d", result);
    LOGI("jni_onLoad_end");
    return JNI_VERSION_1_4;//必须返回版本，否则加载会失败
}