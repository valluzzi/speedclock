package com.gmail.valluzzi.speedclimb;


import android.util.Log;

import com.gmail.valluzzi.speedclimb.model.Positions;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;

public class Utils {

    public static final int LOCAL_PORT  = 11000;

    public static final int REMOTE_PORT = 10999;

    public static final String TAG = "SpeedClimb";

    private static int ID = -1;

    private static Positions positions;

    public static void sleep(int mills)
    {
        try{Thread.sleep(mills);}catch(Exception ex){}
    }

    public static void sendPacket(String message){
        try {
            //Open a random port to send the package
            //add ID to the message string
            message = ""+ ID+":"+message;

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


    public static void broadcast(String message, int times) {
        try {
            //Open a random port to send the package
            //add ID to the message string
            message = ""+ ID+":"+message;

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

    public static void broadcast(String message) {
        broadcast(message,1);
    }

    public static int getID() {
        return ID;
    }

    public static String getPositionName() {
        if(ID > -1)
            return " " + String.valueOf(ID) + " ";
        else
            return " ";
    }

    public static void nextID() {
        if (positions != null)
            ID = positions.getNextID();
    }

    public static void addNewPosition(int newPosition) {

        if(positions == null)
           positions  = new Positions();

        positions.addPosition(newPosition);
        if(ID == -1)
            ID = newPosition;
    }

    public static int getNrOfPositions() {
        return positions.getNrOfPositions();
    }
}//end class


