#include "socketserver.h"
#include <jni.h>
#include <string>
#include <android/log.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include <thread>
#include <errno.h>
#include <iostream>

#define  LOG_TAG    "socket_server"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

SocketServer::SocketServer() {
    LOGI("SocketServer(");
}

bool SocketServer::closeSocket(int fd) {
    int ret = shutdown(fd, SHUT_RDWR);
    ret |= close(fd);
    cout << "close server fd success:" << ret;
    if (ret > 0 && this->mp_socketServerCallback) {
        this->mp_socketServerCallback->socketConnectStatus(0);
    }
    return ret;
}

bool SocketServer::closeSocket() {
    mIsLoopToServer = false;
    return closeSocket(mSocketFD);
}

SocketServer::~SocketServer() {
    closeSocket(mSocketFD);
    mIsLoopToServer = false;
}

SocketServer *SocketServer::getInstance() {
    LOGI("SocketServer::getInstance");
    static SocketServer instance;
    return &instance;
}

void *waitData(void *p) {
    SocketServer *pSocketServer = (SocketServer*)p;
    if (pSocketServer == NULL){
        LOGE("SocketServer is NULL");
        return NULL;
    }
    char resp[DATA_LENGTH];
    int ret;
    while (pSocketServer->mIsLoopToClient){
        ret = recv(pSocketServer->mCurrentClientFD, (char *) resp, DATA_LENGTH, 0);
        if (ret <= 0) {
            pSocketServer->closeSocket(pSocketServer->mCurrentClientFD);
            pSocketServer->mIsLoopToClient = false;
        } else if (pSocketServer->mp_socketServerCallback) {
            LOGI("::wait_data setSocketCallback,resp:%s", resp);
            pSocketServer->mp_socketServerCallback->socketReceiverData(reinterpret_cast<const char *>(resp));
        }
    }
}

void *serverLoop(void *p) {
    SocketServer *pSocketServer = (SocketServer*)p;
    if (pSocketServer == NULL){
        LOGE("SocketServer is NULL");
        return NULL;
    }
    sockaddr_in caddr;
    socklen_t size = sizeof(sockaddr_in);
    int cfd;
    while (pSocketServer->mIsLoopToServer) {
        cfd = accept(pSocketServer->mSocketFD, (sockaddr*)&caddr, &size);
        if (cfd >= 0) {
            if (pSocketServer->mCurrentClientFD > 0) {
                pSocketServer->closeSocket(pSocketServer->mCurrentClientFD);
                pSocketServer->mIsLoopToClient = false;
                pSocketServer->mCurrentClientFD = -1;
            }
            pSocketServer->mCurrentClientFD = cfd;
            if (pSocketServer->mp_socketServerCallback) {
                pSocketServer->mp_socketServerCallback->socketConnectStatus(1);
            }
            pthread_t recv_t;
            pthread_create(&recv_t,NULL,waitData,pSocketServer);
        }
    }
    return NULL;
}

bool SocketServer::createSocketServer(int port) {
    LOGI("createSocketServer,%d", port);
    mSocketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (mSocketFD == -1) {
        cout << "socket start error! " << errno;
        return false;
    }
    sockaddr_in addr;
    memset(&addr, 0, sizeof(sockaddr_in));
    addr.sin_family = AF_INET;
    int opt = 1;
    setsockopt(mSocketFD, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    addr.sin_port = htons(port == 0 ? fixed_port : port);
    if (::bind(mSocketFD, (sockaddr*)&addr, sizeof(addr)) == -1) {
        LOGE("bind port error:%d",errno);
        cout << "bind port error!";
        return false;
    }
    if (listen(mSocketFD, 1) == -1) {
        LOGE("listen port error:%d",errno);
        return false;
    }
    pthread_t accept_t;
    pthread_create(&accept_t,NULL,serverLoop,this);
    return true;
}

void SocketServer::setSocketCallback(ISocketServerCallBack *callback) {
    LOGI("setSocketCallback");
    this->mp_socketServerCallback = callback;
    LOGI("setSocketCallback,%d", this->mp_socketServerCallback);

}

bool SocketServer::sendDataToClient(char *sendMess) {
    int ret = 0;
    if (mCurrentClientFD == -1){
        LOGE("ERROR: please start server");
        return false;
    }
    ret = send(mCurrentClientFD, sendMess, sizeof(sendMess), 0);
    if ( ret <= 0 ){
        closeSocket(mCurrentClientFD);
        mIsLoopToClient = false;
        return false;
    }
    LOGI("client_send_data:%s ---", sendMess);
    return true;
}

