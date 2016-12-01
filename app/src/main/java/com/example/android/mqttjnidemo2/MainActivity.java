package com.example.android.mqttjnidemo2;

import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.os.SystemClock;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStreamReader;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Locale;
import java.util.Timer;
import java.util.TimerTask;

import mqttPush.MdmConstant;
import mqttPush.MqttPushService;

public class MainActivity extends AppCompatActivity {

    private static final String TAG = "MainActivity";
    private TextView textView;
    private HandlerThread mHandlerThread;
    MyHandler handler;
    Button sub;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        textView = (TextView)findViewById(R.id.textView);
        sub = (Button)findViewById(R.id.button);
        mHandlerThread = new HandlerThread("MQTT");
        mHandlerThread.start();
        Log.i(TAG, "onCreate: Main:"+Thread.currentThread().getName()+"id:"+Thread.currentThread().getId());
        Log.i(TAG, "mHandlerThread onCreate: "+mHandlerThread.getName()+"id: "+mHandlerThread.getId());
        final Looper looper = mHandlerThread.getLooper();
        handler = new MyHandler(looper);
        final Hello hello = new Hello(handler);
        sub.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                /*new Thread(new Runnable() {
                    @Override
                    public void run() {
                        //Hello hello = new Hello(handler);
                        Hello hello = new Hello(handler);

                        Log.i(TAG, "run: "+hello.add(2,8));
                        hello.subscribe();
                        sub.setEnabled(false);
                        sub.setClickable(false);

                    }
                }).start();*/



                Log.i(TAG, "run: "+hello.add(2,8));
                hello.subscribe();
                //sub.setEnabled(false);
                //sub.setClickable(false);
            }
        });

        findViewById(R.id.mqttPushStart).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent(MainActivity.this, MqttPushService.class);
                intent.setAction(MdmConstant.ACTION_CONNECT_MQTT);
                startService(intent);
            }
        });
        findViewById(R.id.mqttPushStop).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent(MainActivity.this,MqttPushService.class);
                intent.setAction(MdmConstant.ACTION_MQTT_STATE_DISCONNECT);
                startService(intent);
            }
        });
        findViewById(R.id.pressText).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                new Thread(new Runnable() {
                    @Override
                    public void run() {
                        //压力测试开启，关闭连接；
                        for(int i=0;i<500;i++){
                            Intent intent = new Intent(MainActivity.this, MqttPushService.class);
                            intent.setAction(MdmConstant.ACTION_CONNECT_MQTT);
                            startService(intent);

                            SystemClock.sleep(1000*30);
                            //disconnect
                            intent = new Intent(MainActivity.this,MqttPushService.class);
                            intent.setAction(MdmConstant.ACTION_MQTT_STATE_DISCONNECT);
                            startService(intent);
                            SystemClock.sleep(1000*30);
                        }
                    }
                }).start();
            }
        });

        findViewById(R.id.sendMessage).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                final Intent intent = new Intent(MainActivity.this,MqttPushService.class);
                TimerTask timerTask = new TimerTask() {
                    @Override
                    public void run() {
                        Log.i(TAG, "run: start to send message");
                        intent.setAction("sendmessage");
                        Calendar calendar = Calendar.getInstance();
                        SimpleDateFormat simpleDateFormat =
                                new SimpleDateFormat("yyyy-MM-dd HH:mm:SS", Locale.ENGLISH);
                        String time = simpleDateFormat.format(calendar.getTime());
                        String responseToServer = time + ": Java Receives the Result," +
                                "Response to you"+"你好";
                        intent.putExtra("responseToServer",responseToServer);
                        startService(intent);
                    }
                };
                Timer timer = new Timer();
                timer.schedule(timerTask,1000,1000*2);

            }
        });
        String sdCardPath = null;

        sdCardPath = Environment.getExternalStorageDirectory().getPath();
        //sdCardPath = "/mnt/sdcard/";
        //Environment.get
        Context context = MainActivity.this;
        //sdCardPath = context.getExternalFilesDir("mqttca");
        Log.i(TAG, "sdCardPath :"+sdCardPath);
        File file = new File("/mnt/sdcard/mqttca/ca.crt");
        //BufferedInputStream bufferedInputStream = null;
        BufferedReader bufferedReader = null;
        try {
            bufferedReader = new BufferedReader(new InputStreamReader(new FileInputStream(file)));
            try {
                String s = null;
                while((s = bufferedReader.readLine())!=null){
                    Log.i(TAG, "content: "+s);
                }
            } catch (IOException e) {
                e.printStackTrace();
            }
            bufferedReader.close();
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private class MyHandler extends Handler{
        public MyHandler() {
        }

        public MyHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what){
                case 1:
                    int number = (Integer)msg.obj;
                    //textView.setText("result is"+number);
                    Log.i(TAG, "handleMessage: result is"+number);
                    break;
                case 2:
                    String s = (String)msg.obj;
                    //textView.setText("HEHE"+s);
                    Log.i(TAG, "handleMessage: HEHE"+s);
                    break;
                default:
                    super.handleMessage(msg);
            }
        }
    }
}
