package mqttPush;

import android.app.PendingIntent;
import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.support.v4.app.NotificationCompat;
import android.util.Log;

/**
 * 设置前台Service保持PushService不被杀死
 */
public class KeepAliveService extends Service {
    private static final String TAG = "KeepAliveService";
    public KeepAliveService() {
    }

    @Override
    public void onCreate() {
        Log.d(TAG, "KeepAliveService onCreated, thread ID: " + Thread.currentThread().getId());
        Intent fromKeepAliveServiceToPushServiceIntent = new Intent(this,MqttPushService.class);
        PendingIntent pendingIntent = PendingIntent.getActivity(this,0,fromKeepAliveServiceToPushServiceIntent
        ,PendingIntent.FLAG_UPDATE_CURRENT);

        NotificationCompat.Builder mBuilder = new NotificationCompat
                .Builder(this)
                .setSmallIcon(android.R.mipmap.sym_def_app_icon)
                .setContentTitle("")
                .setContentText("ForKeepingMQTTPushService")
                .setOngoing(true);
        mBuilder.setContentIntent(pendingIntent);
        int mNotificationId = 1;
        startForeground(mNotificationId,mBuilder.build());
        super.onCreate();
    }

    @Override
    public IBinder onBind(Intent intent) {
        // TODO: Return the communication channel to the service.
        //throw new UnsupportedOperationException("Not yet implemented");
        return null;
    }

    @Override
    public void onDestroy() {
        Log.d(TAG, "onDestroy in.");
        super.onDestroy();
    }

    @Override
    public boolean onUnbind(Intent intent) {
        Log.d(TAG, "onUnbind in.");
        return super.onUnbind(intent);
    }
}
