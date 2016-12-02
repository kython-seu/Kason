package mqttPush;

import android.os.Handler;
import android.os.Message;
import android.util.Log;

import java.io.UnsupportedEncodingException;

/**
 * Created by kason_zhang on 11/23/2016.
 * Team:TrendMicro VMI
 */

public class MqttEngine {

    private static final String TAG = "MqttEngine";
    static {
        System.loadLibrary("mqttjni");
    }
    private Handler mHandler;

    public MqttEngine(Handler mHandler) {
        this.mHandler = mHandler;
    }

    //open jni environment
    public native boolean jniInit();

    public native boolean jniDeInit();
    //connect mqtt and subscribe topic
    public native int connect();

    public native boolean disconnect();


    public native void sendMessage(byte[] bytes);

    public native int getGlobal();
    //start connect callback
    public void onStartConnect(){
        Log.i(TAG, "MQTT onStartConnect");
        Message message = mHandler.obtainMessage(MdmConstant.MQTT_STATE_START_CONNECT);
        mHandler.sendMessage(message);
    }

    public void onFinishedConnect(){
        Log.i(TAG, "MQTT onFinishedConnect connect successfully");
        Message message = mHandler.obtainMessage(MdmConstant.MQTT_STATE_FINISHED_CONNECT);
        mHandler.sendMessage(message);
    }
    //receive command
    public void getCommand(byte[] bytes){
        String response = "";
        try {
            response = new String(bytes,"UTF-8");
        } catch (UnsupportedEncodingException e) {
            e.printStackTrace();
        }
        Log.i(TAG, "getCommand: command is "+response);

    }
}
