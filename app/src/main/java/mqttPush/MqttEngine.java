package mqttPush;

import android.os.Handler;

/**
 * Created by kason_zhang on 11/23/2016.
 * Team:TrendMicro VMI
 */

public class MqttEngine {

    
    private Handler mHandler;

    public MqttEngine(Handler mHandler) {
        this.mHandler = mHandler;
    }

    //打开jni环境
    public native void jniInit();
    //关闭jni环境
    public native void jniDeInit();
}
