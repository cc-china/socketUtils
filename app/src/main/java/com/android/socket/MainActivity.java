package com.android.socket;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.widget.TextView;

public class MainActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Example of a call to a native method
        TextView tv = findViewById(R.id.sample_text);
        tv.setText(stringFromJNI());
        startSocketServer();
    }
}

    private void startSocketServer() {
        new Thread(new Runnable() {
            @Override
            public void run() {
                SocketUtils.startServer(0, new ISocketCallBack() {
                    @Override
                    public void socketConnectStatus(int flag) {
                        Log.e("JNI_socket_utils11111", flag + "");
                    }

                    @Override
                    public void socketReceiverData(String data) {
                        Log.e("JNI_socket_utils22222", data);
                        SocketUtils.jni_sendData("JNI_socket_utils22222");
                    }
                });
            }
        }).start();
    }