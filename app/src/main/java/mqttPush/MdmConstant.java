package mqttPush;

/**
 * Created by kason_zhang on 11/23/2016.
 * Team:TrendMicro VMI
 */

public class MdmConstant {
    public static final int MQTT_STATE_START_CONNECT = 1;
    public static final int MQTT_STATE_FINISHED_CONNECT = 2;
    public static final int MQTT_STATE_CONNECTION_LOST = 3;
    public static final int MQTT_STATE_DISCONNECT = 4;
    public static final String ACTION_CONNECT_MQTT = "com.trendmicro.virdroid.action.CONNECT_MQTT";
    public static final String ACTION_MQTT_STATE_DISCONNECT = "com.trendmicro.virdroid.action.ACTION_MQTT_STATE_DISCONNECT";
}
