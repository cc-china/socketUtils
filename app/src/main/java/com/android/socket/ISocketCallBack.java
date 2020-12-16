package com.android.socket;

public interface ISocketCallBack {
    void socketConnectStatus(int flag);
    void socketReceiverData(String data);
}
