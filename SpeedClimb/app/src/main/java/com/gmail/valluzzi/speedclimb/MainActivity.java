package com.gmail.valluzzi.speedclimb;

import android.content.pm.ActivityInfo;
import android.graphics.Color;
import android.media.AudioManager;
import android.media.ToneGenerator;
import android.os.StrictMode;
import androidx.appcompat.app.AppCompatActivity;
import androidx.dynamicanimation.animation.DynamicAnimation;
import androidx.dynamicanimation.animation.FlingAnimation;

import android.os.Bundle;
import android.util.Log;
import android.view.GestureDetector;
import android.view.MotionEvent;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.ProgressBar;
import android.widget.TextView;

import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.util.Date;
import java.util.Random;
import java.util.Timer;
import java.util.TimerTask;

public class MainActivity extends AppCompatActivity {

    private static final int MAX_UDP_DATAGRAM_LEN = 64;
    private static Chronometer clock;
    private static TextView status;
    private static ImageView arrowDown;
    private static ProgressBar loadPosition;
    private static DatagramSocket socket = null;
    private static Timer timer1 = new Timer();
    private static long SECONDS_FROM_LAST_MESSAGE = 0;
    private static ToneGenerator BEEPER = new ToneGenerator(AudioManager.STREAM_MUSIC, ToneGenerator.MAX_VOLUME);
    private GestureDetector gestureDetector;
    private DatagramReceiver receiver = null;
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
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    loadPosition.setVisibility(View.GONE);
                }
            });

            String[] words = receiver.lastMessage.split(":");
            if (words.length>1 && words[0].equals( ""+Utils.getID() ) ) {  // ID ==> CurrentID
                String message = words[1];
                if (message.startsWith("kill")){
                    return;
                }

                Log.e(Utils.TAG,message);

                if (message.equals("IDLE")) {

                    clock.stop();
                    clock.setTextColor(Color.YELLOW);
                    status.setTextColor(Color.YELLOW);
                    status.setText(getResources().getString(R.string.footboard)+
                            Utils.getPositionName()+"CONNECTED...");

                }else if (message.equals("PREARMED")) {

                    clock.stop();
                    clock.reset();
                    clock.setTextColor(Color.MAGENTA);
                    status.setTextColor(Color.MAGENTA);
                    status.setText(getResources().getString(R.string.footboard)+
                            Utils.getPositionName()+"GET READY...");
                }
                else if (message.equals("ARMED")) {

                    clock.stop();
                    clock.reset();
                    clock.setTextColor(Color.RED);
                    status.setTextColor(Color.RED);
                    status.setText(getResources().getString(R.string.footboard)+
                            Utils.getPositionName()+"READY");
                    BEEPER.startTone(ToneGenerator.TONE_CDMA_PIP,150);
                }
                else if (message.equals("RUNNING")) {

                    if (words.length > 2) {
                        long ms = Long.parseLong(words[2]);
                        clock.start(ms);
                    } else {
                        clock.start();
                    }
                    clock.setTextColor(Color.RED);
                    status.setTextColor(Color.RED);
                    status.setText(getResources().getString(R.string.footboard)+
                            Utils.getPositionName()+"CLIMBING...");

                    //BEEPER.startTone(ToneGenerator.	TONE_CDMA_PIP,400);
                }
                else if (message.equals("STOP")) {

                    clock.stop();
                    if (words.length > 2) {
                        long ms = Long.parseLong(words[2]);
                        clock.setText(ms);
                    }
                    clock.setTextColor(Color.YELLOW);
                    status.setTextColor(Color.YELLOW);
                    status.setText(getResources().getString(R.string.footboard)+
                            Utils.getPositionName()+"STOPPED");

                    //BEEPER.startTone(ToneGenerator.	TONE_CDMA_ONE_MIN_BEEP,200);
                }else{

                    //Se ricevi un qualunque messaggio significa che sei connesso

                }

            } else {
                Utils.addNewPosition(Integer.valueOf(words[0]));

                if (Utils.getNrOfPositions() > 1)
                    arrowDown.setVisibility(View.VISIBLE);
            }
        }//end run
    };//end Runnable

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        //Get saved Settings
        //SharedPreferences settings = getSharedPreferences("SETTINGS", MODE_PRIVATE);
        //ID = settings.getString("ID", "0");

        //Avoid the title bar and FULL SCREEN mode
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,WindowManager.LayoutParams.FLAG_FULLSCREEN);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);

        setContentView(R.layout.activity_main);
        clock = this.findViewById(R.id.editText);
        status = this.findViewById(R.id.weight);
        arrowDown = this.findViewById(R.id.arrowDown);
        loadPosition = this.findViewById(R.id.loadPosition);

        arrowDown.setVisibility(View.GONE);

        loadPosition.animate();


        //IDLE TIMER
        timer1.scheduleAtFixedRate(new TimerTask() {
            @Override
            public void run() {
                if ( SECONDS_FROM_LAST_MESSAGE>10) {

                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            loadPosition.setVisibility(View.GONE);
                        }
                    });

                    //DISCONNECTED;
                    clock.setTextColor(Color.GRAY);
                    status.setTextColor(Color.GRAY);
                    status.setText(getResources().getString(R.string.footboard)+Utils.getPositionName()+"NOT CONNECTED...");
                }

                SECONDS_FROM_LAST_MESSAGE+=5;
            }
        }, new Date(), 5000);


        // Hack Prevent crash (sending should be done using an async task)
        StrictMode.ThreadPolicy policy = new   StrictMode.ThreadPolicy.Builder().permitAll().build();
        StrictMode.setThreadPolicy(policy);

        gestureDetector = new GestureDetector(this, new GestureListener());

        FlingAnimation fling = new FlingAnimation(findViewById(android.R.id.content).getRootView(),
                DynamicAnimation.SCROLL_Y);

    }

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

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        gestureDetector.onTouchEvent(event);
        return super.onTouchEvent(event);
    }

    protected void onPause() {
        Log.e(Utils.TAG, "onPause:");
        if (receiver !=null) {
            receiver.kill();
        }
        super.onPause();
    }

    private class GestureListener
            extends GestureDetector.SimpleOnGestureListener {

        private static final String GEST_TAG = "FlingGesture";

        @Override
        public boolean onFling(MotionEvent e1, MotionEvent e2,
                               float velocityX, float velocityY) {
            if (Utils.getID() > -1) {
                Utils.nextID();
                loadPosition.setVisibility(View.VISIBLE);
            }
            return super.onFling(e1, e2, velocityX, velocityY);
        }
    }

    private class DatagramReceiver extends Thread {
        public String lastMessage= "";
        private boolean running = true;

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

                socket= new DatagramSocket(Utils.LOCAL_PORT);
                socket.setTrafficClass( 0x04 ); //IPTOS_RELIABILITY

                while(running) {
                    socket.receive(packet);
                    message = new String(buffer, 0, packet.getLength());
                    runOnUiThread(parseMessage);

                    lastMessage = message;
                }
            } catch (Throwable e) {

                Log.e(Utils.TAG,"Throwable Exception:"+e);

            }
            if (socket != null) {
                socket.close();
            }
            running = true;
        }

        public void kill() {
            running = false;
            //!important --> to unlock socket.receive() send it anything
            Utils.broadcast("kill thread");
        }


    }


}//end class




