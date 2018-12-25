package com.lacina.cubeeclient.bleConnection;

/**
 * Constants with meaning to communicate with device
 */
@SuppressWarnings("ALL")
public class BleConstants {
    public static final String CONTROL_SERVICE = "00000030-0000-1000-8000-00805f9b34fb"; //Service control UUID

    public static final String SEND_CONTROL_CHAR = "00000031-0000-1000-8000-00805f9b34fb"; // Send controls characteristic UUID

    public static final String RECIVE_CONTROL_CHAR = "00000032-0000-1000-8000-00805f9b34fb"; //Recive control service UUID

    @SuppressWarnings("unused")
    public static final String MONITOR_SERVICE = "00000040-0000-1000-8000-00805f9b34fb"; //Monitor Service UUID

    public static final String MONITOR_CHAR = "00000041-0000-1000-8000-00805f9b34fb"; //Minitor Characteristc UUID

    @SuppressWarnings("unused")
    public static final String NAME_SERVICE = "00001800-0000-1000-8000-00805f9b34fb"; //Device Name Service UUID

    @SuppressWarnings("unused")
    public static final String NAME_CHAR = "00002a24-0000-1000-8000-00805f9b34fb"; //Device Name Characteristc UUID

    @SuppressWarnings("unused")
    public static final String CONFIG_SERVICE = "00000050-0000-1000-8000-00805f9b34fb";

    public static final String CONFIG_CHAR = "00000051-0000-1000-8000-00805f9b34fb";

    public static final String DB9_CHAR = "00000061-0000-1000-8000-00805f9b34fb";

    public static final int SCAN_TIMEOUT = -1; //Infinite scan time

    public static final String RELAY_ON = "[01]";

    public static final String RELAY_OFF = "[02]";

    public static final String BUTTON_PRESS = "[01]"; //Command to read when button is pressed

    public static final int TURN_ON = 0x01; //Command to send to Turn on

    public static final int TURN_OFF = 0x02; //Comand to send to Turn off

    @SuppressWarnings("unused")
    public static final int TIMEOUT_WRITE_AFTER_READ = 500; //Timeout interval between a write and a read

    @SuppressWarnings("unused")
    public static final int TIMEOUT_READ_AFTER_WRITE = 3500;

    public static final int NONE = 0x00; //Command to write when no command is left to processes

    public static final int LED_ON = 0x03; //Command to send to change LED status.

    public static final int TIMEOUT_LOOP_RECEIVE = 1000;

    public static final int TIMEOUT_LOOP_MONITOR = 3000;

    @SuppressWarnings("unused")
    public static final int CONFIG = 0x03;

    public static final int JSON_CONFIG = 0x04;
    public static final int DB9_COMMAND = 0x06;
}

