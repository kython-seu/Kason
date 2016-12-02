package mqttPush;

import android.app.Notification;
import android.app.Service;
import android.content.ComponentName;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Build;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.util.Log;

public class MqttPushService extends Service {

    private static final String TAG = "MqttPushService";
    private MqttHandler mqttHandler;
    private MqttEngine mqttEngine;
    private KeepingPushAliveService keepingPushAliveService;
    public static final int KEEP_ALIVE_NOTIFICATION_ID = 11;
    private ServiceConnection conn = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            keepingPushAliveService = ((KeepingPushAliveService.KeepBinder) service).getService();
            MqttPushService.this.startForeground(KEEP_ALIVE_NOTIFICATION_ID,new Notification());
            keepingPushAliveService.startForeground(KEEP_ALIVE_NOTIFICATION_ID,new Notification());
            keepingPushAliveService.stopForeground(true);
            MqttPushService.this.unbindService(conn);
            conn = null;
        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            conn = null;
        }
    };
    public MqttPushService() {
    }

    @Override
    public void onCreate() {
        Log.i(TAG, "MQTTPushService onCreate");
        super.onCreate();

        HandlerThread mHandlerThread = new HandlerThread("MQTTPushService");
        mHandlerThread.start();
        mqttHandler = new MqttHandler(mHandlerThread.getLooper());
        mqttEngine = new MqttEngine(mqttHandler);
        mqttEngine.jniInit();
        //Keep Alive
        //Intent intent = new Intent(this,KeepAliveService.class);
        //startService(intent);
        setForeground();

    }

    private void setForeground() {
        if(Build.VERSION.SDK_INT<18){
            startForeground(KEEP_ALIVE_NOTIFICATION_ID,new Notification());
        }else {
            Intent intent = new Intent(MqttPushService.this, KeepingPushAliveService.class);
            MqttPushService.this.bindService(intent, conn, Service.BIND_AUTO_CREATE);
        }

    }

    @Override
    public IBinder onBind(Intent intent) {
        // TODO: Return the communication channel to the service.
        //throw new UnsupportedOperationException("Not yet implemented");
        return null;
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        if(intent == null){
            return START_STICKY;
        }
        Log.i(TAG, "MQTTPushService onStartCommand");
        String action = intent.getAction();
        Log.i(TAG, "action: "+action);
        if(MdmConstant.ACTION_CONNECT_MQTT.equals(action)){
            //Log.i(TAG, "onStartCommand: a global "+mqttEngine.getGlobal());
            new Thread(new Runnable() {
                @Override
                public void run() {
                    mqttEngine.connect();
                    //Log.i(TAG, "onStartCommand: a global "+mqttEngine.getGlobal());
                }
            }).start();
        }else if(MdmConstant.ACTION_MQTT_STATE_DISCONNECT.equals(action)){
            mqttEngine.disconnect();

        }else if("sendmessage".equals(action)){
            String responseToServer = intent.getStringExtra("responseToServer");
            mqttEngine.sendMessage(responseToServer.getBytes());
        }
        else{
            Log.i(TAG, "onStartCommand: NO ACTION");
        }
        return START_STICKY;
    }

    @Override
    public void onDestroy() {
        Log.i(TAG, "MQTTPushService onDestroy");
        if(mqttEngine!=null){
            //do some other works such as disconnect mqtt;
            mqttEngine.jniDeInit();
            mqttEngine = null;
        }
        super.onDestroy();
    }

    private class MqttHandler extends Handler{
        public MqttHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what){
                case MdmConstant.MQTT_STATE_START_CONNECT:
                    Log.i(TAG, "handleMessage: Start Connect");
                    break;
                case MdmConstant.MQTT_STATE_FINISHED_CONNECT:
                    Log.i(TAG, "handleMessage: Finished Connection");
                default:
                    super.handleMessage(msg);
            }

        }
    }
}
