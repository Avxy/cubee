package com.lacina.cubeeclient.activities;

import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGattCharacteristic;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.support.annotation.NonNull;
import android.support.v4.app.FragmentActivity;
import android.support.v7.app.AppCompatActivity;
import android.text.TextUtils;
import android.util.Log;
import android.view.View;
import android.widget.AutoCompleteTextView;
import android.widget.Button;

import com.android.volley.VolleyError;
import com.google.android.gms.tasks.OnCompleteListener;
import com.google.android.gms.tasks.Task;
import com.google.firebase.auth.FirebaseAuth;
import com.google.firebase.auth.FirebaseUser;
import com.google.firebase.auth.GetTokenResult;
import com.google.gson.Gson;
import com.google.gson.JsonObject;
import com.lacina.cubeeclient.R;
import com.lacina.cubeeclient.bleConnection.BleConstants;
import com.lacina.cubeeclient.bleConnection.bleService.BluetoothLeService;
import com.lacina.cubeeclient.controllers.BLEConnectionController;
import com.lacina.cubeeclient.controllers.CubeeController;
import com.lacina.cubeeclient.fragments.LoginTabFragment;
import com.lacina.cubeeclient.fragments.ScanCubeesTabFragment;
import com.lacina.cubeeclient.localDB.LocalDB;
import com.lacina.cubeeclient.model.Cubee;
import com.lacina.cubeeclient.serverConnection.AppConfig;
import com.lacina.cubeeclient.serverConnection.RequestQueueSingleton;
import com.lacina.cubeeclient.serverConnection.callbacks.OnGetJsonObjectCallback;
import com.lacina.cubeeclient.serverConnection.callbacks.old.OnGetStringRequest;
import com.lacina.cubeeclient.serverConnection.requests.GetJsonObjectRequest;
import com.lacina.cubeeclient.serverConnection.requests.GetStringRequest;
import com.lacina.cubeeclient.utils.ActivityUtils;
import com.lacina.cubeeclient.utils.VolleyErrorUtils;

import java.util.HashMap;
import java.util.Map;

import butterknife.BindView;
import butterknife.ButterKnife;
import butterknife.OnClick;

import static com.lacina.cubeeclient.bleConnection.bleService.BluetoothLeService.EXTRA_UUID_CHAR;

/**
 * The goal with this activity is to setup a new CUBEE with wifi SSID and Password
 * so this CUBEE can connect with the server via wifi.
 * After write the configuration in cubee it will try to connect with server via wifi and the
 * aplication will constantly check with server if the procedure was successful
 * Pseudocode:
 * CONNECT TO CUBEE WITH BLE
 * GET FROM USER SSID WIFI AND PASSWORD
 * GET FROM SERVER A NEW ID
 * WRITE CONFIGURATION INFORMATIONS AT CONFIG BLE CHARACTERISTIC
 * WRITE A CONFIGURATION COMMAND AT CONFIGURARION BLE CHARACTERISTIC
 * CHECK WITH SERVER IF THE CUBEE HAS CONNECTED
 */
@SuppressWarnings("ALL")
public class SetUpWifiCubeeActivity extends AppCompatActivity {
    private static final String TAG = "SetUpWifiCubee";

    /**
     * Input Text cubee name view
     */
    @SuppressWarnings("unused")
    @BindView(R.id.actv_name_cubee)
    public AutoCompleteTextView edtNameCubee;

    /**
     * Input text wifi ssd view
     */
    @SuppressWarnings("unused")
    @BindView(R.id.actv_ssd_wifi)
    public AutoCompleteTextView edtSsdWifi;

    /**
     * Input text wifi pass
     */
    @SuppressWarnings("unused")
    @BindView(R.id.actv_wifi_pass)
    public AutoCompleteTextView edtWifiPass;

    /**
     * Register button
     */
    @SuppressWarnings("unused")
    @BindView(R.id.btn_register_cubee)
    public Button btnRegisterCubee;

    /**
     * SSID name, to send to cubee so it can connect with server via wifi
     */
    private String SSIDName;

    /**
     * SSid password retrieved from local db
     */
    private String localDBSSIDPassword;

    /**
     * ID of the cubee
     * First we retrieve this id from the server and send it to the cubee
     */
    private String idCubee;

    /**
     * ID of the cubee owner
     */
    private String idOwner;

    /**
     * Bluetooth controller reference for common CUBEE related methods
     */
    private CubeeController cubeeController;

    /**
     * Bluetooth controller reference for common bluetooth related methods
     */
    private BLEConnectionController bleConnectionController;

    /**
     * Reference for this activity
     */
    private FragmentActivity activity;

    /**
     * Firebase user profile information object.
     * Contain helper methods to o change or retrieve profile information,
     * as well as to manage that user's authentication state.
     */
    private FirebaseUser firebaseUser;

    /**
     * Received device from scan.
     */
    private BluetoothDevice mDevice;

    /**
     * Service to control BLE. Used to Write and Read.
     */
    private BluetoothLeService mBluetoothLeService;


    /**
     * Characteristic to write configuration into cubee, with wifi ssd and password.
     */
    private BluetoothGattCharacteristic configCharacteristic;

    /**
     * Characteristic do send commands to CUBEE.
     */
    private BluetoothGattCharacteristic sendCommandsChar;

    /**
     * Broadcast receiver to communicate this fragment with the BLEConnection Service.
     * Receive Actions and for each action call a different method.
     */
    private final BroadcastReceiver mGattUpdateReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(final Context context, final Intent intent) {
            final String action = intent.getAction();
            String charUuid = intent.getStringExtra(EXTRA_UUID_CHAR);
            //IF READ ACTION
            if (BluetoothLeService.ACTION_GATT_SERVICES_DISCOVERED.equals(action)) {
                onServidesDiscovered();
            } else if (BluetoothLeService.ACTION_GATT_DISCONNECTED.equals(action)) {
                onGattDisconnected();
            } else if (BluetoothLeService.ACTION_DATA_WRITE.equals(action)) {
                onDataWrite(charUuid);
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
            mBluetoothLeService.connect(mDevice.getAddress());
        }

        @Override
        public void onServiceDisconnected(final ComponentName componentName) {
            mBluetoothLeService = null;
        }
    };

    /**
     * Number of tries
     */
    private int tries;

    /**
     * A handler to set a runnable and post delay to make us read a characteristic in loop
     */
    @SuppressWarnings("CanBeFinal")
    private Handler handler = new Handler();

    /**
     * This interface is designed to provide a common protocol for objects that wish to
     * execute code while they are active.
     */
    private Runnable runnable;

    //Activity Methods ----------------------------------------------------------------------------
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.fragment_set_up_wifi_cubee);
        activity = this;

        ButterKnife.bind(activity);

        bleConnectionController = BLEConnectionController.getInstance();
        mBluetoothLeService = bleConnectionController.getmBluetoothLeService();

        final Intent intent = getIntent();
        mDevice = intent.getParcelableExtra(ScanCubeesTabFragment.EXTRA_DEVICE);

        final Intent gattServiceIntent = new Intent(this, BluetoothLeService.class);
        bindService(gattServiceIntent, mServiceConnection, BIND_AUTO_CREATE);


        firebaseUser = FirebaseAuth.getInstance().getCurrentUser();
        if (firebaseUser != null) {
            idOwner = firebaseUser.getUid();
            firebaseUser.getIdToken(true)
                    .addOnCompleteListener(new OnCompleteListener<GetTokenResult>() {
                        public void onComplete(@NonNull Task<GetTokenResult> task) {
                            if (!task.isSuccessful()){
                                //Log Erro
                                //noinspection ThrowableResultOfMethodCallIgnored,ThrowableResultOfMethodCallIgnored
                                Log.e(TAG, "Error Get token" + (task.getException() == null ? "null exception" : task.getException().getMessage()));
                                FirebaseAuth.getInstance().signOut();
                                Intent intent = new Intent(activity, LoginTabFragment.class);
                                startActivity(intent);
                                activity.finish();
                            }
                        }
                    });
        }
        cubeeController = CubeeController.getInstance();

        ActivityUtils.showProgressDialog(this, "Conectando.");


        SSIDName = ActivityUtils.getSSID(getApplicationContext());
        localDBSSIDPassword = LocalDB.getPasswordWifiIfSaved(activity, SSIDName);
        edtSsdWifi.setText(SSIDName);
        edtWifiPass.setText(localDBSSIDPassword);

        runnable = new Runnable() {
            @Override
            public void run() {
                verifyCubeeRegister();
            }
        };

    }

    /***
     * Register broadcast receiver at onResume
     * */
    @Override
    public void onResume() {
        super.onResume();
        registerReceiver(mGattUpdateReceiver, makeGattUpdateIntentFilter());
    }

    @Override
    public void onPause() {
        super.onPause();
        unregisterReceiver(mGattUpdateReceiver);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        disconnect();

    }

    @Override
    public void onStop() {
        super.onStop();
        finish();
    }

    @Override
    public void onBackPressed() {
        disconnectAndReturnToScan();
    }

    /**
     * Validate form and try to send Register Cubee Request
     **/
    @SuppressWarnings("unused")
    @OnClick(R.id.btn_register_cubee)
    public void attemptRegisterCubee() {
        // Reset errors.
        edtNameCubee.setError(null);
        edtSsdWifi.setError(null);
        edtWifiPass.setError(null);

        // Store values at the time of the login attempt.
        final String name = edtNameCubee.getText().toString().trim();
        final String wifiName = edtSsdWifi.getText().toString().trim();
        final String wifiPass = edtWifiPass.getText().toString().trim();

        boolean cancel = false;
        View focusView = null;


        // Check for a valid email address.
        if (TextUtils.isEmpty(name)) {
            edtNameCubee.setError(getString(R.string.error_field_required));
            focusView = edtNameCubee;
            cancel = true;
        }

        // Check for a valid name.
        if (TextUtils.isEmpty(wifiName)) {
            edtSsdWifi.setError(getString(R.string.error_field_required));
            focusView = edtSsdWifi;
            cancel = true;
        }

        // Check for a valid name.
        if (TextUtils.isEmpty(wifiPass)) {
            edtWifiPass.setError(getString(R.string.error_field_required));
            focusView = edtWifiPass;
            cancel = true;
        }

        if (cancel) {
            focusView.requestFocus();
        } else {
            if (!localDBSSIDPassword.equals(wifiPass)) {
                ActivityUtils.alertDialogSimpleDualButton(activity, "ALERTA",
                        "Você deseja salvar a senha digitada para esta rede, para utilizá-la em cadastros futuros?",
                        new DialogInterface.OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialog, int which) {
                                LocalDB.savePasswordWifi(activity, wifiName, wifiPass);
                                registerCubee(name, wifiName, wifiPass);
                            }
                        }, new DialogInterface.OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialog, int which) {
                                registerCubee(name, wifiName, wifiPass);
                            }
                        });

            } else {
                registerCubee(name, wifiName, wifiPass);
            }

        }
    }

    //BLE Service ---------------------------------------------------------------------------------

    /**
     * Starts everything used to make BLE services work
     */
    private void startBLEServices() {
        configCharacteristic = bleConnectionController.getBLECharacteristic(BleConstants.CONTROL_SERVICE, BleConstants.CONFIG_CHAR);
        sendCommandsChar = bleConnectionController.getBLECharacteristic(BleConstants.CONTROL_SERVICE, BleConstants.SEND_CONTROL_CHAR);
        if((sendCommandsChar == null) || (configCharacteristic == null)){
            disconnectAndReturnToScan();
        }
    }

    /**
     * This method request a IS to server and
     * at this request callback the program send the configuration
     * in json format to cubee
     * CALLBACK: {@link #onGetIdNewCubeeRequestCallback(String, String, String)}
     * @param name name of the new cubee
     * @param wifiSSID Wifi ssid to send to cubee
     * @param wifiPass Wifi password to send to cubee
     */
    private void registerCubee(String name, String wifiSSID, String wifiPass) {
        ActivityUtils.showProgressDialog(activity, "Registrando CUBEE");
        GetStringRequest newIdRequest = new GetStringRequest(onGetIdNewCubeeRequestCallback(name, wifiSSID, wifiPass), AppConfig.getInstance().getIdNewCubee());
        RequestQueueSingleton.getInstance(activity).addToRequestQueue(newIdRequest.getRequest());
    }

    private OnGetStringRequest onGetIdNewCubeeRequestCallback(final String name, final String wifiName, final String wifiPass) {
        return new OnGetStringRequest() {

            @Override
            public void onGetStringRequestSuccess(String id) {
                idCubee = id;
                JsonObject json = new JsonObject();
                json.addProperty("cubeeName", "CUBEE-" + name);
                json.addProperty("idCubee", id);
                json.addProperty("userId", idOwner);
                json.addProperty("wifiSSID", wifiName);
                json.addProperty("wifiPass", wifiPass);
                Log.d(TAG, "json enviado = " + json.toString());
                sendConfigToCubee(json.toString());
            }

            @Override
            public void onGetStringRequestError() {
                ActivityUtils.cancelProgressDialog();
                ActivityUtils.showToast(activity, "Erro ao cadastrar CUBEE");
            }
        };
    }

    /**
     * Send a configuration json to cubee via BLE
     * @param cubeeJson Cubee json with all informations
     */
    private void sendConfigToCubee(String cubeeJson) {
        bleConnectionController.writeCharacteristicString(configCharacteristic, cubeeJson + "\r\n\r\n");
        ActivityUtils.cancelProgressDialog();
        ActivityUtils.showProgressDialog(activity, "Registrando CUBEE");
        verifyCubeeRegister();
        tries = 0;
    }

    /**
     * Request to see if the CUBEE is already registered
     * CALLBACK: {@link #onGetCubeeByIdCallback}
     */
    private void verifyCubeeRegister() {
        Map<String, String> headers = new HashMap<>();
        headers.put("idCubee", idCubee);
        GetJsonObjectRequest getCubeeByIdRequest = new GetJsonObjectRequest(onGetCubeeByIdCallback(), AppConfig.getInstance().getCubeeById());
        RequestQueueSingleton.getInstance(activity).addToRequestQueue(getCubeeByIdRequest.getRequest(headers).setShouldCache(false));
    }

    private OnGetJsonObjectCallback onGetCubeeByIdCallback() {
        return new OnGetJsonObjectCallback() {
            @Override
            public void onGetJsonObjectCallbackSuccess(JsonObject jsonObject) {
                Gson gson = new Gson();
                Cubee cubee = gson.fromJson(jsonObject, Cubee.class);
                tries = 0;
                handler.removeCallbacks(runnable);
                ActivityUtils.showToast(activity, "Cubee cadastrado com sucesso");
                cubeeController.setCubeeRegistered(true);
                disconnectAndReturnToScan();
                Log.d(TAG, cubee.getName() + " Cadastrado");

            }

            @Override
            public void onGetJsonObjectCallbackError(VolleyError volleyError) {
                if (tries < 8) {
                    tries = tries + 1;
                    handler.postDelayed(runnable, 3000);
                } else {
                    tries = 0;
                    handler.removeCallbacks(runnable);
                    ActivityUtils.cancelProgressDialog();
                    String myMessage = "Erro ao cadastrar cubee";
                    ActivityUtils.showToast(activity, VolleyErrorUtils.decideRightMessage(volleyError, myMessage, activity, true));
                }
            }
        };
    }

    /**
     * Method for disconnect from device,
     * stop service and cancel timers.
     * Stop everthing done in startBleServices method.
     * With this implementation when the user get out of the activity and return he has to connect again
     */
    private void disconnect() {
        if (mBluetoothLeService != null) {
            mBluetoothLeService.disconnect();
        }
        try {
            unbindService(mServiceConnection);
        } catch (IllegalArgumentException e) {
            Log.d(TAG, "serviço já UNBINDED");
        }
        bleConnectionController.unBindService();
        mBluetoothLeService = null;
        //  cancelTimers();

        ActivityUtils.cancelProgressDialog();
//        if (progressDialogBLEActivated != null) {
//            progressDialogBLEActivated.cancel();
//        }
    }

    /**
     * Disconnect from the service, cancel any loop monitor and return to scan fragment.
     **/
    private void disconnectAndReturnToScan() {
        stopService(new Intent(this, BluetoothLeService.class));
        disconnect();
        //handler.removeCallbacks(runnable);
        finish();
    }


    // Broadcast Receiver Methods ------------------------------------------------------------------

    /**
     * Called when BLE services from device are discovered
     */
    private void onServidesDiscovered() {
        ActivityUtils.cancelProgressDialog();
        ActivityUtils.showToastLong(activity, "Por favor, pressione o botão do CUBEE.");
        bleConnectionController.setmBluetoothLeService(mBluetoothLeService);
        startBLEServices();
    }

    /**
     * Called When the program successfully write a data in the characteristic.
     * @param charUuid Intent that contains the data
     */
    private void onDataWrite(String charUuid) {
        Log.d(TAG, "Char writed");
        if (charUuid.equals(BleConstants.CONFIG_CHAR)) {
            bleConnectionController.writeCharacteristic(sendCommandsChar, new byte[]{(byte) BleConstants.JSON_CONFIG});

        } else if (charUuid.equals(BleConstants.RECIVE_CONTROL_CHAR)) {
            Log.d("CharControlWrited", "ChaControl");
            //Available buttons
        }
    }

    /**
     * When device has beean dsconnected
     */
    private void onGattDisconnected() {
        ActivityUtils.cancelProgressDialog();
        disconnectAndReturnToScan();
    }

    private IntentFilter makeGattUpdateIntentFilter() {
        //AlertsFilter for the broadcast receiver
        final IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(BluetoothLeService.ACTION_DATA_AVAILABLE);
        intentFilter.addAction(BluetoothLeService.ACTION_GATT_DISCONNECTED);
        intentFilter.addAction(BluetoothLeService.ACTION_DATA_WRITE);
        intentFilter.addAction(BluetoothLeService.ACTION_GATT_SERVICES_DISCOVERED);
        return intentFilter;
    }


}
