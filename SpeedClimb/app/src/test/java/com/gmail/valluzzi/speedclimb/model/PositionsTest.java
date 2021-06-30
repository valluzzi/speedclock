package com.gmail.valluzzi.speedclimb.model;

import junit.framework.TestCase;

public class PositionsTest extends TestCase {

    private Positions positions;

    public void setUp() throws Exception {
        positions = new Positions();
        super.setUp();
    }

    public void testAddPosition() {
        assertEquals(-1, positions.getNextID());
        assertEquals(0, positions.getNrOfPositions());
        positions.addPosition(4);
        assertEquals(1, positions.getNrOfPositions());
        assertEquals(4, positions.getNextID());
        positions.addPosition(4);
        assertEquals(1, positions.getNrOfPositions());
        assertEquals(4, positions.getNextID());
        positions.addPosition(5);
        assertEquals(2, positions.getNrOfPositions());
        assertEquals(5, positions.getNextID());
    }

}