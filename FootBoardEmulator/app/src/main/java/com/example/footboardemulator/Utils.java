package com.example.footboardemulator;


import android.util.Log;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;

public class Utils {

    public static final int LOCAL_PORT  = 10999;

    public static final int REMOTE_PORT = 11000;

    public static final String TAG = "FootBoardEmulator";

    public static int ID = -1;


    public static void sleep(int mills)
    {
        try{
            Thread.sleep(mills);}catch(Exception ex){}
    }

    public static void broadcast(String message) {
        try {
            //Open a random port to send the package
            //add ID to the message string
            //message = ""+ ID+":"+message;

            InetAddress address = InetAddress.getByName("255.255.255.255");
            DatagramSocket socket = new DatagramSocket();
            socket.setBroadcast(true);
            byte[] sendData = message.getBytes();
            DatagramPacket sendPacket = new DatagramPacket(sendData, sendData.length,address, REMOTE_PORT);
            socket.send(sendPacket);

        } catch (IOException e) {
            Log.e(TAG, "IOException: " + e.getMessage());
        }
    }

}//end class


