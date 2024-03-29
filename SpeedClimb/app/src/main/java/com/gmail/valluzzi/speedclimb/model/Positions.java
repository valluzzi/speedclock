package com.gmail.valluzzi.speedclimb.model;

import java.util.LinkedList;
import java.util.List;

public class Positions {
    private List<Integer> positions;
    private int currentPosition;

    public Positions() {
        this.positions = new LinkedList<Integer>();
        currentPosition = 1;
    }

    public void addPosition(int ID) {
        if(!positions.contains(ID))
            this.positions.add(ID);
    }

    public int getNextID() {
        if (positions.size() > 0) {
            currentPosition = ++currentPosition % positions.size();
            return positions.get(currentPosition);
        } else {
            return -1;
        }

    }

    public int getNrOfPositions() {
        return positions.size();
    }
}
