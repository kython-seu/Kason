package com.example.android.mqttjnidemo2;

import android.os.Handler;
import android.os.Message;
import android.util.Log;

/**
 * Created by kason_zhang on 11/7/2016.
 */

public class Hello {
    private static final String TAG = "Hello";
    private Handler handler;


    static {
        System.loadLibrary("data_op");
    }
    public native int add(int a,int b);
    public native int subscribe();
    //public native int publish(String topic,String msg);

    public Hello(Handler handler){
        this.handler = handler;
    }
    public void nativeCallJni(int i){
        Message msg = new Message();
        msg.what = 1;
        msg.obj = i;
        handler.sendMessage(msg);
        Log.i(TAG, "nativeCallJni: native C code call JNI");
    }
    public void nativeCallJni2(String i){
        Message msg = new Message();
        msg.what = 2;
        msg.obj = i;
        handler.sendMessage(msg);
        Log.i(TAG, "nativeCallJni: native C code call JNI");
    }

    public native void reconnect();
    public  static void hehe(int i){
        Log.i(TAG, "hehe: hhehe"+i);
    }
}
