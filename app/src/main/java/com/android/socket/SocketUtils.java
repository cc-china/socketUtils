package com.android.socket;

public class SocketUtils {
    static {
        System.loadLibrary("socket_jni");
    }
    private static native boolean jni_startServer(int port, ISocketCallBack callBack);
    public static native boolean jni_closeServer();
    public static native boolean jni_sendData(String sendData);
    /* It must be used in the worker thread and must not be opened in the UI thread*/
    public static void startServer(int port, ISocketCallBack callBack){
        jni_startServer(port,callBack);
    }
}
