package mqttPush;


import android.app.Service;
import android.content.Intent;
import android.os.Binder;
import android.os.IBinder;
import android.support.annotation.Nullable;
import android.util.Log;

/**
 * Created by kason_zhang on 11/24/2016.
 * Team:TrendMicro VMI
 */

public class KeepingPushAliveService extends Service {
    private static final String TAG = "KeepingPushAliveService";
    public class KeepBinder extends Binder{

        public KeepingPushAliveService getService(){
            return KeepingPushAliveService.this;
        }
    }
    @Nullable
    @Override
    public IBinder onBind(Intent intent) {
        return new KeepBinder();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        Log.i(TAG, "onDestroy: KeepingPushAliveService destroy");
    }
}
