package com.gmail.valluzzi.speedclimb;

import junit.framework.TestCase;

public class UtilsTest extends TestCase {

    public void setUp() throws Exception {
        Utils.addNewPosition(0);
        super.setUp();
    }

    public void testNextID() {
        assertEquals(0, Utils.getID());
        Utils.nextID();
        assertEquals(0, Utils.getID());
        Utils.addNewPosition(1);
        assertEquals(0, Utils.getID());
        Utils.nextID();
        assertEquals(1, Utils.getID());
        Utils.addNewPosition(2);
        Utils.nextID();
        assertEquals(2, Utils.getID());
        Utils.nextID();
        assertEquals(0, Utils.getID());
    }
}