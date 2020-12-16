#include "socketclient.h"

SocketClient::SocketClient()
{
    //createSocketClient(ipAddress, port);
}

SocketClient::~SocketClient()
{
    closeSocket();
    if(mp_resp != NULL){
        delete mp_resp;
        mp_resp = NULL;
    }
}

SocketClient *SocketClient::getInstance()
{
    static SocketClient instance;
    return &instance;
}

void *recvData(void *p){
    SocketClient *pSocketClient = (SocketClient*)p;
    if(pSocketClient == NULL){
        qDebug() << "recvData thread init error";
        return NULL;
    }
    if(pSocketClient->mp_resp == NULL){
        pSocketClient->mp_resp = new char[DATA_LENGTH]();
    }
    while(pSocketClient->mIsLoop){
        memset(pSocketClient->mp_resp,0,DATA_LENGTH);
        int ret = recv(pSocketClient->mCurSocket, (char*)pSocketClient->mp_resp, DATA_LENGTH, 0);
        char* buf = strstr( pSocketClient->mp_resp, SEPARATOR);
        while( buf != NULL )
        {
            buf[0]='\0';
            printf( "%s\n ", pSocketClient->mp_resp);
            pSocketClient->mp_resp = buf + strlen(SEPARATOR);
            /* Get next token: */
            buf = strstr( pSocketClient->mp_resp, SEPARATOR);
        }
        qDebug() << "recvData :ret " <<ret;
        if(ret<=0){
            pSocketClient->closeSocket();
            return NULL;
        }
        if(pSocketClient->mp_socketCallback != NULL){
            pSocketClient->mp_socketCallback->socketReceiverData(reinterpret_cast<const char *>(pSocketClient->mp_resp));
        }
        qDebug() << "ret:"<<ret<< ",recvData"<<pSocketClient->mp_resp;
    }
    return NULL;
}
bool SocketClient::createSocketClient(const char *ipAddress, int port)
{
    // init and create socket
    WORD sock_ver = MAKEWORD(2, 2);
    WSADATA wsa_data;
    if (WSAStartup(sock_ver, &wsa_data) != 0) {
        //printf("init error\n");
        qDebug() << "401 init error";
        return false;
    }
    SOCKET sock_client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    //int nNetTimeout=5000;
    //setsockopt(sock_client,SOL_SOCKET,SO_RCVTIMEO, (char *)&nNetTimeout,sizeof(int));
    qDebug() << "create socket ipAddress"<< ipAddress;

    if (sock_client == INVALID_SOCKET) {
        qDebug() << "create socket error"<< errno;
        return false;
    }
    mCurSocket = sock_client;
    // connect
    sockaddr_in addr_server;
    addr_server.sin_family = AF_INET;
    addr_server.sin_port = htons(port);
    addr_server.sin_addr.s_addr = inet_addr(ipAddress);
    int code = connect(mCurSocket, (sockaddr *)&addr_server, sizeof(sockaddr_in));
    qDebug() << "connect"<<code;
    if ( code == SOCKET_ERROR) {
        closeSocket();
        return false;
    }
    if(this->mp_socketCallback!=NULL){
        this->mp_socketCallback->socketConnectStatus(1);
    }
    mIsLoop = true;
    pthread_t recv_t;
    pthread_create(&recv_t, NULL, &recvData, this);
    return true;
}

void SocketClient::sendData(char *datas,char *dataType)
{
    int ret = 0;
    char *dataLen = NULL;
    sprintf(dataLen,"%d",strlen(datas));
    //package send data
    //char *packet = SOCKET_PACKET_STRAT + SEPARATOR + dataType + SEPARATOR+ dataLen +
    //                SEPARATOR + datas + SEPARATOR + SOCKET_PACKET_END;
    char *tempSendDatas = new char[strlen(SOCKET_PACKET_STRAT)+
            strlen(SEPARATOR)+
            strlen(dataType)+
            strlen(SEPARATOR)+
            strlen(dataLen)+
            strlen(SEPARATOR)+
            strlen(datas)+
            strlen(SEPARATOR)+
            strlen(SOCKET_PACKET_END)+1];
    strcpy(tempSendDatas,SOCKET_PACKET_STRAT);
    strcat(tempSendDatas,SEPARATOR);
    strcat(tempSendDatas,dataType);
    strcat(tempSendDatas,SEPARATOR);
    strcat(tempSendDatas,dataLen);
    strcat(tempSendDatas,SEPARATOR);
    strcat(tempSendDatas,datas);
    strcat(tempSendDatas,SEPARATOR);
    strcat(tempSendDatas,SOCKET_PACKET_END);
    ret = send(mCurSocket, tempSendDatas, strlen(tempSendDatas), 0);
    if(ret<=0){
        closeSocket();
    }
    qDebug() << "ret:"<<ret<< ",sendData"<<datas;
}

void SocketClient::closeSocket(){
    if(mCurSocket!=NULL){
        closesocket(mCurSocket);
        WSACleanup();
        if(this->mp_socketCallback!=NULL){
            this->mp_socketCallback->socketConnectStatus(0);
        }
        mIsLoop = false;
        if(mp_resp != NULL){
            delete mp_resp;
            mp_resp = NULL;
        }
    }
}
void SocketClient::setSocketCallback(ISocketCallBack *callback) {
    this->mp_socketCallback = callback;
    qDebug() << "callback:"<<callback<< ",this->mp_socketCallback"<<this->mp_socketCallback;
}
