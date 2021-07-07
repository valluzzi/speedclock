package com.example.footboardemulator;

import androidx.appcompat.app.AppCompatActivity;

import android.os.AsyncTask;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;

import java.util.concurrent.Executors;


public class MainActivity extends AppCompatActivity {

    private static long t1=System.currentTimeMillis();
    private static long t2=t1;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        Button button2 = (Button) findViewById(R.id.button2);
        button2.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                Executors.newSingleThreadExecutor().submit(() -> {
                    t1 = System.currentTimeMillis();
                    String msg = "1:RUNNING:0";
                    Log.e(Utils.TAG,msg);
                    Utils.broadcast(msg);
                });
            }
        });

        Button button3 = (Button) findViewById(R.id.button3);
        button3.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                Executors.newSingleThreadExecutor().submit(() -> {
                    long deltaT = System.currentTimeMillis()-t1;
                    String msg = "1:STOP:"+deltaT;
                    Log.e(Utils.TAG,msg);
                    Utils.broadcast(msg);
                });
            }
        });
        Button button4 = (Button) findViewById(R.id.button4);
        button4.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                Executors.newSingleThreadExecutor().submit(() -> {
                    t2 = System.currentTimeMillis();
                    String msg = "2:RUNNING:0";
                    Log.e(Utils.TAG,msg);
                    Utils.broadcast(msg);
                });
            }
        });

        Button button5 = (Button) findViewById(R.id.button5);
        button5.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                Executors.newSingleThreadExecutor().submit(() -> {
                    long deltaT = System.currentTimeMillis()-t2;
                    String msg = "2:STOP:"+deltaT;
                    Log.e(Utils.TAG,msg);
                    Utils.broadcast(msg);
                });
            }
        });



    }

    private Runnable start = new Runnable() {
        @Override
        public void run() {
            Utils.broadcast("1:RUNNING:0");
            Log.e(Utils.TAG,"1:RUNNING:0");
        }
    };
}//end class