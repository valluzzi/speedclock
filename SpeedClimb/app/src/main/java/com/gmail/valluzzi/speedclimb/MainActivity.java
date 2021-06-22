package com.gmail.valluzzi.speedclimb;

import android.content.SharedPreferences;
import android.content.pm.ActivityInfo;
import android.graphics.Color;
import android.media.AudioManager;
import android.media.ToneGenerator;
import android.os.StrictMode;
import androidx.appcompat.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.TextView;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.util.Date;
import java.util.Timer;
import java.util.TimerTask;

import static com.gmail.valluzzi.speedclimb.Utils.sleep;


public class MainActivity extends AppCompatActivity {

    private static String ID;

    private enum state{
        BEGIN,
        IDLE,
        PREARMED,
        ARMED,
        RUNNING,
        STOP,
        DISCONNECTED
    };

    private static state STATE=state.DISCONNECTED;
    private static final String TAG  = "SpeedClimb";
    private static final int LOCAL_PORT  = 11000;
    private static final int REMOTE_PORT = 10999;
    private static final int MAX_UDP_DATAGRAM_LEN = 64;
    private static Chronometer clock;
    private static TextView status;
    private static Button reset;
    private static DatagramSocket socket = null;
    private static Timer timer1 = new Timer();
    private static long SECONDS_FROM_LAST_MESSAGE = 0;

    private static ToneGenerator BEEPER = new ToneGenerator(AudioManager.STREAM_MUSIC, ToneGenerator.MAX_VOLUME);

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        //Get saved Settings
        SharedPreferences settings = getSharedPreferences("SETTINGS", MODE_PRIVATE);
        ID = settings.getString("ID", "0");

        //Avoid the title bar and FULL SCREEN mode
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,WindowManager.LayoutParams.FLAG_FULLSCREEN);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);

        setContentView(R.layout.activity_main);
        clock = this.findViewById(R.id.editText);
        status = this.findViewById(R.id.weight);


        reset = this.findViewById(R.id.stop);
        reset.setOnClickListener( new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                sendPacket("stop:0");
                status.setText("STOP!");
            }
        });

        //IDLE TIMER
        timer1.scheduleAtFixedRate(new TimerTask() {
            @Override
            public void run() {
                if ( (STATE!=state.RUNNING) && SECONDS_FROM_LAST_MESSAGE>10) {

                    STATE = state.DISCONNECTED;
                    clock.setTextColor(Color.GRAY);
                    status.setTextColor(Color.GRAY);
                    status.setText("NOT CONNECTED...");
                }

                SECONDS_FROM_LAST_MESSAGE+=5;
            }
        }, new Date(), 5000);


        // Hack Prevent crash (sending should be done using an async task)
        StrictMode.ThreadPolicy policy = new   StrictMode.ThreadPolicy.Builder().permitAll().build();
        StrictMode.setThreadPolicy(policy);
    }

    public void sendPacket(String message){
        try {
            //Open a random port to send the package
            //add ID to the message string
            message = ID+":"+message;

            InetAddress address = InetAddress.getByName("255.255.255.255");
            DatagramSocket socket = new DatagramSocket();
            socket.setBroadcast(true);
            byte[] sendData = message.getBytes();
            DatagramPacket packet = new DatagramPacket(sendData, sendData.length, address, REMOTE_PORT);
            socket.send(packet);

        } catch (IOException e) {
            Log.e(TAG, "IOException: " + e.getMessage());
        }
    }


    public void broadcast(String message, int times) {
        try {
            //Open a random port to send the package
            //add ID to the message string
            message = ID+":"+message;

            InetAddress address = InetAddress.getByName("255.255.255.255");
            DatagramSocket socket = new DatagramSocket();
            socket.setBroadcast(true);
            byte[] sendData = message.getBytes();
            DatagramPacket sendPacket = new DatagramPacket(sendData, sendData.length,address, LOCAL_PORT);
            for(int j=1;j<=times;j++) {
                socket.send(sendPacket);
                if (j>1)sleep(j*10);
            }

        } catch (IOException e) {
            Log.e(TAG, "IOException: " + e.getMessage());
        }
    }

    public void broadcast(String message) {
        broadcast(message,1);
    }

    private DatagramReceiver receiver = null;

    protected void onResume() {
        super.onResume();
        //Log.e(TAG, "onResume:");
        if (receiver ==null){
            receiver = new DatagramReceiver();
        }

        if (receiver !=null && !receiver.isAlive()){
            receiver.start();
        }
    }

    protected void onPause() {
        Log.e(TAG, "onPause:");
        if (receiver !=null) {
            receiver.kill();
        }
        super.onPause();
    }



    private class DatagramReceiver extends Thread {
        private boolean running = true;
        public String lastMessage= "";

        public DatagramReceiver(){
            //Log.e(TAG,"new MyDatagramReceiver()");

            this.setPriority(MAX_PRIORITY);

            running = true;

        }

        public void run() {
            String message;
            byte[] buffer = new byte[MAX_UDP_DATAGRAM_LEN];
            DatagramPacket packet = new DatagramPacket(buffer, buffer.length);
            DatagramSocket socket = null;
            try {

                socket= new DatagramSocket(LOCAL_PORT);
                socket.setTrafficClass( 0x04 ); //IPTOS_RELIABILITY

                while(running) {
                    socket.receive(packet);
                    message = new String(buffer, 0, packet.getLength());
                    runOnUiThread(parseMessage);

                    lastMessage = message;
                }
            } catch (Throwable e) {

                Log.e(TAG,"Throwable Exception:"+e);

            }
            if (socket != null) {
                socket.close();
            }
            running = true;
        }

        public void kill() {
            running = false;
            //!important --> to unlock socket.receive() send it anything
            broadcast("kill thread");
        }


    }

    /**
     *
     *  parseMessage - core logic
     *
     *
     */
    private Runnable parseMessage = new Runnable() {
        public void run() {
            if (receiver == null) return;
            SECONDS_FROM_LAST_MESSAGE =0;

            String[] words = receiver.lastMessage.split(":");
            if (words.length>1 && words[0].equals( ID ) ) {
                String message = words[1];
                if (message.startsWith("kill")){
                    return;
                }

                Log.e(TAG,message);

                if (message.equals("IDLE")) {

                    STATE = state.IDLE;
                    clock.stop();
                    clock.setTextColor(Color.YELLOW);
                    status.setTextColor(Color.YELLOW);
                    status.setText("IDLE...");

                }else if (message.equals("PREARMED")) {

                    STATE=state.PREARMED;

                    clock.stop();
                    clock.reset();
                    clock.setTextColor(Color.MAGENTA);
                    status.setTextColor(Color.MAGENTA);
                    status.setText("GET READY...");
                }
                else if (message.equals("ARMED")) {

                    STATE=state.ARMED;

                    clock.stop();
                    clock.reset();
                    clock.setTextColor(Color.RED);
                    status.setTextColor(Color.RED);
                    status.setText("READY");
                    BEEPER.startTone(ToneGenerator.TONE_CDMA_PIP,150);
                }
                else if (message.equals("RUNNING")) {

                    STATE=state.RUNNING;

                    if (words.length > 2) {
                        long ms = Long.parseLong(words[2]);
                        clock.start(ms);
                    } else {
                        clock.start();
                    }
                    clock.setTextColor(Color.RED);
                    status.setTextColor(Color.RED);
                    status.setText("RUNNING...");

                    //BEEPER.startTone(ToneGenerator.	TONE_CDMA_PIP,400);
                }
                else if (message.equals("STOP")) {
                    STATE=state.STOP;

                    clock.stop();
                    if (words.length > 2) {
                        long ms = Long.parseLong(words[2]);
                        clock.setText(ms);
                    }
                    clock.setTextColor(Color.YELLOW);
                    status.setTextColor(Color.YELLOW);
                    status.setText("STOPPED");

                    BEEPER.startTone(ToneGenerator.	TONE_CDMA_ONE_MIN_BEEP,200);
                }else{

                    //Se ricevi un qualunque messaggio significa che sei connesso

                }

            }
        }//end run
    };//end Runnable
}//end class




