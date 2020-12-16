#ifndef SOCKETSERVER_H
#define SOCKETSERVER_H

#include "socketglobal.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include<pthread.h>

using namespace std;

class ISocketServerCallBack{
public:
    virtual void socketReceiverData(const char *data) = 0;
    virtual void socketConnectStatus(const int flag) = 0;
};

class SocketServer
{
public:
    static SocketServer *getInstance();
    bool closeSocket();
	bool closeSocket(int fd);
    void setSocketCallback(ISocketServerCallBack *callback);
    bool createSocketServer(int port);
	bool sendDataToClient(char *sendMess);
	int mSocketFD;
	int mCurrentClientFD = -1;
	bool mIsLoopToClient = true;
	bool mIsLoopToServer = true;
	ISocketServerCallBack *mp_socketServerCallback = nullptr;
private:
    SocketServer();
    ~SocketServer();
};
// SOCKETSERVER_H
#endif 
