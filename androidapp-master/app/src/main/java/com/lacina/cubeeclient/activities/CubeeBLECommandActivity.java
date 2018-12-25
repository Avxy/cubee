package com.lacina.cubeeclient.activities;

import android.app.Activity;
import android.app.ProgressDialog;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGattCharacteristic;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.support.annotation.Nullable;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.View;
import android.widget.Button;

import com.lacina.cubeeclient.R;
import com.lacina.cubeeclient.bleConnection.BleConstants;
import com.lacina.cubeeclient.bleConnection.bleService.BluetoothLeService;
import com.lacina.cubeeclient.controllers.BLEConnectionController;
import com.lacina.cubeeclient.fragments.ScanCubeesTabFragment;
import com.lacina.cubeeclient.model.InstantCommandBluetooth;
import com.lacina.cubeeclient.utils.ActivityUtils;
import com.lacina.cubeeclient.utils.SensorOrientationChangeNotifier;

import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

import java.util.Timer;

import butterknife.BindView;
import butterknife.ButterKnife;
import uk.co.alt236.bluetoothlelib.util.ByteUtils;

import static com.lacina.cubeeclient.bleConnection.bleService.BluetoothLeService.EXTRA_UUID_CHAR;

/**
 * Activity used to control CUBEE via BLE.
 * Receive a CUBEE address to start.
 * Needs Scan first: {@link ScanCubeesTabFragment}
 * The activity where Scan is implemented needs to implement
 * {@link com.lacina.cubeeclient.interfaces.BleScanChangeable}
 **/
public class CubeeBLECommandActivity extends AppCompatActivity implements SensorOrientationChangeNotifier.Listener{

    private static final String TAG = "CubeeCommandActv";

    /**
     * Received device from scan.
     * Set at onCreate.
     */
    private BluetoothDevice mDevice;

    /**
     * Service to control BLE. Used to Write and Read.
     */
    private BluetoothLeService mBluetoothLeService;

    /**
     * BLE Connection controller for some BLE operations.
     */
    private BLEConnectionController bleConnectionController;

    /**
     * Reference for this Activity.
     */
    private Activity activity;

    /**
     * Characteristic to send commands to CUBEE.
     */
    private BluetoothGattCharacteristic sendCommandsChar;

    /**
     * Characteristic to read in loop to receive commands from CUBEE.
     */
    private BluetoothGattCharacteristic receiveCommandChar;

    /**
     * Monitor Characteristic, to retrieve cubee relayStatus.
     */
    private BluetoothGattCharacteristic monitorChar;

    /**
     * Status button. Indicates the signal sent by CUBEE.
     */
    private boolean buttonStatus = false;

    /**
     * Relay status. Indicates if the CUBEE has power ON or OFF;
     */
    private Boolean relayStatus = false;

    /**
     * Button to show Signal from CUBEE status.
     */
    @BindView(R.id.btn_cubee_button)
    public  Button btnCubeeButton;

    /**
     * Button to send LED command to CUBEE.
     */
    @BindView(R.id.btn_led)
    public  Button btnLed;

    /**
     * Disconnect button.
     */
    @BindView(R.id.btn_disconnect)
    public  Button btnDisconnect;

    /**
     * Button to show relay status.
     */
    @BindView(R.id.btn_relay_onoff)
    public  Button btnRealyOnOff;

    /**
     * Timer used to check if CUBEE is sending a Signal.
     */
    private Timer receiveCommandTimer;

    /**
     * Timer used to check RELAY status.
     */
    private Timer monitorCharTimer;

    /**
     * Progress dialog that appears when you turn ON/OFF a cubee.
     */
    private ProgressDialog progressDialogBLEActivated;

    private android.support.v7.app.AlertDialog.Builder builder;

    private android.support.v7.app.AlertDialog myDialog;

    /**
     * Broadcast receiver to communicate this fragment with the BLEConnection Service.
     * Receive Actions and for each action call a different method.
     **/
    private final BroadcastReceiver mGattUpdateReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(final Context context, final Intent intent) {
            final String action = intent.getAction();
            if (BluetoothLeService.ACTION_GATT_SERVICES_DISCOVERED.equals(action)) {
                //After connect the service tries to discover the services.
                onServicesDiscovered();
            } else if (BluetoothLeService.ACTION_DATA_AVAILABLE.equals(action)) {
                // When the service successfully read a characteristic.
                onDataAvailable(intent);
            } else if (BluetoothLeService.ACTION_GATT_DISCONNECTED.equals(action)) {
                //Received when for any reason the device got disconnected.
                onGattDisconnected();
            } else if (BluetoothLeService.ACTION_DATA_WRITE.equals(action)) {
                //Received then successfully wrote on a activity.
                onDataWrite(intent);
            }
        }
    };

    /**
     * Communication interface to communicate with BLE service.
     * Receive messages when you bind the activity with the service
     * and when you unbind the service.
     * bind service start a service if needed
     * to bind you have to use bindService(intent, serviceConnection, flags)
     */
    private final ServiceConnection mServiceConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(final ComponentName componentName, final IBinder service) {
            mBluetoothLeService = ((BluetoothLeService.LocalBinder) service).getService();
            if (!mBluetoothLeService.initialize()) {
                Log.e(TAG, "Unable to initialize Bluetooth");
                finish();
            }
            // Automatically connects to the device upon successful start-up initialization.
            // If the service is successful srated connect with device.
            mBluetoothLeService.connect(mDevice.getAddress());

        }

        @Override
        public void onServiceDisconnected(final ComponentName componentName) {
            mBluetoothLeService = null;
        }
    };

    /**
     * Reference to the intent used to bind(and start if needed) the BLE service
     */
    private Intent gattServiceIntent;

    /**
     * Characteristic to send commands db9 to cubee
     */
    private BluetoothGattCharacteristic db9CommandChar;

    //Activity Methods ----------------------------------------------------------------------------
    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        //Set Up activity and ui.
        super.onCreate(savedInstanceState);
        setContentView(R.layout.fragment_cubee_ble);
        ButterKnife.bind(this);
        activity = this;

        //Get device from intent.
        final Intent intent = getIntent();
        mDevice = intent.getParcelableExtra(ScanCubeesTabFragment.EXTRA_DEVICE);

        //Create the intent to start the service and stop it when needed.
        gattServiceIntent = new Intent(this, BluetoothLeService.class);

        //Start the service and establish the serviceConnection interface.
        bindService(gattServiceIntent, mServiceConnection, BIND_AUTO_CREATE);

        //Shows a progress dialog to block user from control the cubee while services are not discovered yet.
        ActivityUtils.showProgressDialog(this, getString(R.string.connecting));

        //Set up BLE controller
        bleConnectionController = BLEConnectionController.getInstance();
        bleConnectionController.setActivity(this);
    }

    @Override
    public void onBackPressed() {
        disconnectAndReturnToScan();
    }

    private void createOnClickListeners() {
        //DISCONNECT
        btnDisconnect.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                disconnectAndReturnToScan();
            }
        });

        //LED
        btnLed.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                ActivityUtils.showToast(getApplicationContext(), getString(R.string.sending_signal));
                bleConnectionController.writeCharacteristic(sendCommandsChar, new byte[]{(byte) BleConstants.LED_ON});
            }
        });

        //RELAY
        btnRealyOnOff.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                String message;
                if (relayStatus) {
                    bleConnectionController.writeCharacteristic(sendCommandsChar, new byte[]{(byte) BleConstants.TURN_OFF});
                    message = getString(R.string.desactivating);

                } else {
                    bleConnectionController.writeCharacteristic(sendCommandsChar, new byte[]{(byte) BleConstants.TURN_ON});
                    message = getString(R.string.activating);
                }

                progressDialogBLEActivated = new ProgressDialog(CubeeBLECommandActivity.this);
                progressDialogBLEActivated.setMessage(message);
                progressDialogBLEActivated.setCanceledOnTouchOutside(false);
                progressDialogBLEActivated.show();


                Handler delay = new Handler();
                delay.postDelayed(new Runnable() {
                    @Override
                    public void run() {
                        if (progressDialogBLEActivated != null) {
                            progressDialogBLEActivated.cancel();
                        }
                    }
                }, 30000);

            }
        });


    }

    /***
     * Register broadcast receiver at onResume
     */
    @Override
    public void onResume() {
        super.onResume();
        SensorOrientationChangeNotifier.getInstance(this).addListener(this);
        registerReceiver(mGattUpdateReceiver, makeGattUpdateIntentFilter());
        if(!EventBus.getDefault().isRegistered(this)) {
            EventBus.getDefault().register(this);
        }
    }

    @Override
    public void onPause() {
        Log.d("Onstop", "pause");
        super.onPause();
        SensorOrientationChangeNotifier.getInstance(this).remove(this);
    }

    @Override
    public void onStop() {
        Log.d("Onstop", "stop");
        super.onStop();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        EventBus.getDefault().unregister(this);
        unregisterReceiver(mGattUpdateReceiver);
        disconnect();
    }

    //BLE Services ---------------------------------------------------------------------------------

    /**
     * Method to disconnect from device,
     * stop service and cancel timers.
     * Stop everything done in startBleServices method.
     * With this implementation when the user get out of the activity and return he has to connect again
     */
    private void disconnect() {
        //Send Disconnect signal to cubee
        if (mBluetoothLeService != null) {
            mBluetoothLeService.disconnect();
        }

        //Stop the service.
        try {
            stopService(gattServiceIntent);
            unbindService(mServiceConnection);
        } catch (IllegalArgumentException e) {
            Log.d(TAG, "Service already Unbound");
        }
        bleConnectionController.unBindService();
        mBluetoothLeService = null;

        //Cancel timers and progress Dialogs
        cancelTimers();
        ActivityUtils.cancelProgressDialog();
        if (progressDialogBLEActivated != null) {
            progressDialogBLEActivated.cancel();
        }
    }

    /**
     * Starts everything used to make BLE services work
     */
    private void startBLEServices() {
        //GET characteristics
        monitorChar = bleConnectionController.getBLECharacteristic(BleConstants.CONTROL_SERVICE, BleConstants.MONITOR_CHAR);
        receiveCommandChar = bleConnectionController.getBLECharacteristic(BleConstants.CONTROL_SERVICE, BleConstants.RECIVE_CONTROL_CHAR);
        sendCommandsChar = bleConnectionController.getBLECharacteristic(BleConstants.CONTROL_SERVICE, BleConstants.SEND_CONTROL_CHAR);
        db9CommandChar = bleConnectionController.getBLECharacteristic(BleConstants.CONTROL_SERVICE, BleConstants.DB9_CHAR);

        if((monitorChar == null) || (receiveCommandChar == null) || (sendCommandsChar == null) || (db9CommandChar == null)){
            disconnectAndReturnToScan();
        }

        //Read once at the beginning
        bleConnectionController.readCharacteristic(monitorChar);

        //Read in Loop monitor and receive command characteristics
        monitorCharTimer = bleConnectionController.readCharacteristicInLoop(monitorChar, BleConstants.TIMEOUT_LOOP_MONITOR, 2);
        receiveCommandTimer = bleConnectionController.readCharacteristicInLoop(receiveCommandChar, BleConstants.TIMEOUT_LOOP_RECEIVE, 0);

        //After this free the buttons.
        createOnClickListeners();
    }

    /**
     * Disconnect from the service, cancel any loop monitor and return to scan fragment.
     **/
    private void disconnectAndReturnToScan() {
        disconnect();
        finish();
    }

    //Cancel loops.
    private void cancelTimers() {
        if (monitorCharTimer != null) {
            monitorCharTimer.cancel();
        }
        if (receiveCommandTimer != null) {
            receiveCommandTimer.cancel();
        }
    }

    // Broadcast Receiver Methods ------------------------------------------------------------------

    /**
     * Called when BLE services from device are discovered
     */
    private void onServicesDiscovered() {
        bleConnectionController.setmBluetoothLeService(mBluetoothLeService);
        startBLEServices();
        ActivityUtils.cancelProgressDialog();
        ActivityUtils.showToastLong(activity, "Por favor, pressione o botão do CUBEE.");
    }

    /**
     * Called when a data is available, after the program request a read.
     */
    private void onDataAvailable(Intent intent) {
        String charUuid = intent.getStringExtra(EXTRA_UUID_CHAR);
        //READ: MONITOR CHAR
        if (charUuid.equals(BleConstants.MONITOR_CHAR)) {
            onReceiveMonitorChar(intent);
        }//READ: RECEIVER CONTROL CHAR
        else if (charUuid.equals(BleConstants.RECIVE_CONTROL_CHAR)) {
            onReceiveControlChar(intent);

        }
    }

    /**
     * When the receive a data from Monitor Characteristic
     * @param intent intent that contains the data
     */
    private void onReceiveMonitorChar(Intent intent) {
        final byte[] dataArr = intent.getByteArrayExtra(BluetoothLeService.EXTRA_DATA_RAW);
        String value = ByteUtils.byteArrayToHexString(dataArr);
        String stringValue = "";
        Boolean response = null;

        //READ; MONITOR CHAR - RELAY OFF
        if (value.equals(BleConstants.RELAY_OFF)) {
            response = false;
            stringValue = getString(R.string.deactivating);

        }
        //READ: MONITOR CHAR - RELAY ON
        else if (value.equals(BleConstants.RELAY_ON)) {
            response = true;
            stringValue = getString(R.string.activated);
        }


        //COMPARES STATUS WITH RECEIVED VALUE
        Log.d("CharValueMonitor", value);
        if (response!= null && !relayStatus.equals(response)) {
            //WAS WAITING FOR THIS CHANGE?
            if (progressDialogBLEActivated != null) {
                //IF YES, CANCEL PROGRESS
                progressDialogBLEActivated.cancel();
            }
        }

        //CHANGE STATUS AND BUTTONS
        relayStatus = response;
        btnRealyOnOff.setText(stringValue);
    }

    /**
     * When the receive a data from Monitor Characteristic
     */
    private void onReceiveControlChar(Intent intent) {
        final byte[] dataArr = intent.getByteArrayExtra(BluetoothLeService.EXTRA_DATA_RAW);
        String value = ByteUtils.byteArrayToHexString(dataArr);
        Log.d("CharControlReceived", value);
        if (value.equals(BleConstants.BUTTON_PRESS)) {
            receiveCommandTimer.cancel();
            if (buttonStatus) {
                btnCubeeButton.setText(R.string.button_off);
                buttonStatus = false;
                bleConnectionController.writeCharacteristic(receiveCommandChar, new byte[]{(byte) BleConstants.NONE});
            } else {
                btnCubeeButton.setText(R.string.button_on);
                buttonStatus = true;
                bleConnectionController.writeCharacteristic(receiveCommandChar, new byte[]{(byte) BleConstants.NONE});
            }
        }
    }

    /**
     * Called When the program successfully write a data in the characteristic.
     * @param intent Intent that contains the data
     */
    private void onDataWrite(Intent intent) {
        String charUuid = intent.getStringExtra(EXTRA_UUID_CHAR);
        if (charUuid.equals(BleConstants.SEND_CONTROL_CHAR)) {
            Log.d("CharWrited", "Char writed");
        } else if (charUuid.equals(BleConstants.RECIVE_CONTROL_CHAR)) {
            Log.d("CharControlWrited", "ChaControl");
            //Available buttons
            receiveCommandTimer = bleConnectionController.readCharacteristicInLoop(receiveCommandChar, BleConstants.TIMEOUT_LOOP_RECEIVE, 0);
        }else if(charUuid.equals(BleConstants.DB9_CHAR)){
            Log.d("DB9ControlWrited", "BD9CHAR");
            bleConnectionController.writeCharacteristic(sendCommandsChar, new byte[]{(byte) BleConstants.DB9_COMMAND}); //ESCREVER 6 DEPOIS DE ESCREVER O VALOR

        }
    }

    /**
     * When device has been disconnected
     */
    private void onGattDisconnected() {
        ActivityUtils.showToast(activity, getString(R.string.you_disconnected));
        disconnectAndReturnToScan();
    }

    /**
     * Return a filter for broadcast receiver
     */
    private static IntentFilter makeGattUpdateIntentFilter() {
        final IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(BluetoothLeService.ACTION_DATA_AVAILABLE);
        intentFilter.addAction(BluetoothLeService.ACTION_GATT_DISCONNECTED);
        intentFilter.addAction(BluetoothLeService.ACTION_DATA_WRITE);
        intentFilter.addAction(BluetoothLeService.ACTION_GATT_SERVICES_DISCOVERED);
        return intentFilter;
    }

    /**
     * Listener to detect when the cellphone orientation change.
     *
     * @param orientation integer representing in degrees
     */
    @Override
    public void onOrientationChange(int orientation) {
        if (orientation == 90 || orientation == 270){
            // Nothing
        } else {
            if(!relayStatus){
                remoteControlDialog();
            }
        }
    }

    /**
     * Dialog to enter remote control mode by changing orientation.
     */
    private void remoteControlDialog() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            builder = new android.support.v7.app.AlertDialog.Builder(activity, android.R.style.Theme_Material_Dialog_Alert);
        } else {
            builder = new android.support.v7.app.AlertDialog.Builder(activity);
        }
        if(myDialog == null){
            showRemoteControlDialog();
        }else if(!myDialog.isShowing()){
            showRemoteControlDialog();
        }

    }

    private void showRemoteControlDialog(){
        builder.setTitle("Controle remoto")
                .setMessage("Deseja entrar no modo controle remoto?")
                .setPositiveButton(android.R.string.yes, new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int which) {
                        Intent i = new Intent(activity, RemoteControlBluetoothActivity.class);
                        startActivityForResult(i, 1);
                    }
                })
                .setNegativeButton(android.R.string.no, new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int which) {
                        // do nothing
                    }
                })
                .setIcon(android.R.drawable.ic_dialog_alert);
        myDialog = builder.create();
        myDialog.show();
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onEvent(final InstantCommandBluetooth command) {
        bleConnectionController.writeCharacteristic(db9CommandChar,  new byte[]{(byte) command.getCurrentOutput()}); //ESCREVER 6
    }

}
