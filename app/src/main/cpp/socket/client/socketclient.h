#ifndef SOCKETCLIENT_H
#define SOCKETCLIENT_H

#include "socketglobal.h"
#include <sys/types.h>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include <unistd.h>
#include <time.h>
#include <thread>
#include <errno.h>
#include <iostream>
#include<pthread.h>
#include <unistd.h>
#include "qdebug.h"

using namespace std;

class ISocketCallBack{
public:
    virtual void socketReceiverData(const char *data) = 0;
    virtual void socketConnectStatus(const int flag) = 0;
};

class SocketClient
{
public:
    static SocketClient *getInstance();
    void closeSocket();
    void sendData(char *datas = NULL,char *dataType = NULL);
    //char *recvData();
    void setSocketCallback(ISocketCallBack *callback = NULL);
    bool createSocketClient(const char *ipAddress, int port);
    SOCKET mCurSocket = NULL;
    ISocketCallBack *mp_socketCallback = NULL;
    char *mp_resp = NULL;
    bool mIsLoop = true;
private:
    SocketClient();
    ~SocketClient();
};

#endif // SOCKETCLIENT_H
