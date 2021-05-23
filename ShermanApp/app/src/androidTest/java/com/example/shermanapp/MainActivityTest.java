package com.example.shermanapp;

import android.content.Context;

import androidx.test.platform.app.InstrumentationRegistry;

import junit.framework.TestCase;

import org.junit.Test;

import static org.junit.Assert.assertEquals;

public class MainActivityTest extends TestCase {

    public void testGenerateConfig() {

        MainActivity testObject = new MainActivity();
        testObject.humidity_switch.setChecked(true);
        testObject.watering_interval_bar.setProgress(9);
        testObject.watering_time_bar.setProgress(7);

        assertEquals(testObject.generateConfig(), "7097");
    }
}