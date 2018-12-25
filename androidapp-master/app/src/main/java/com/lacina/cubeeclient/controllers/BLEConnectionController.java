package com.lacina.cubeeclient.controllers;

import android.Manifest;
import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattService;
import android.content.ServiceConnection;
import android.os.Build;
import android.util.Log;
import android.view.View;
import android.widget.Toast;

import com.anthonycr.grant.PermissionsManager;
import com.anthonycr.grant.PermissionsResultAction;
import com.lacina.cubeeclient.R;
import com.lacina.cubeeclient.activities.CubeeBLECommandActivity;
import com.lacina.cubeeclient.bleConnection.BleConstants;
import com.lacina.cubeeclient.bleConnection.CubeeBluetoothLeScanner;
import com.lacina.cubeeclient.bleConnection.bleService.BluetoothLeService;
import com.lacina.cubeeclient.fragments.ScanCubeesTabFragment;
import com.lacina.cubeeclient.utils.ActivityUtils;
import com.lacina.cubeeclient.utils.BluetoothUtils;

import java.util.List;
import java.util.Timer;
import java.util.TimerTask;

/**
 * A Singleton controller for all BLE connection operations.
 **/
@SuppressWarnings("ALL")
public class BLEConnectionController {
    private static final String TAG = "BLEController";

    private static BLEConnectionController myInstance;

    private CubeeBluetoothLeScanner mScanner;

    private BluetoothUtils mBluetoothUtils;

    private BluetoothLeService mBluetoothLeService;

    @SuppressWarnings({"CanBeFinal", "unused"})
    private ServiceConnection mServiceConnection;
    /**
     * Field to reference this activity
     */
    private Activity activity;


    public static synchronized BLEConnectionController getInstance() {
        if (myInstance == null) {
            myInstance = new BLEConnectionController();
        }
        return myInstance;
    }


    /**
     * Method for set activity and define context.
     **/
    public void setActivity(Activity activity) {
        this.activity = activity;
        mBluetoothUtils = new BluetoothUtils(activity);
    }


    /**
     * A method for verify BLE On and BLE present at device and start scan.
     * Called by startScanPrepare
     **/
    private void startScan() {
        //ActivityUtils.showProgressDialog(activity, "Procurando Dispositivos.");
        final boolean isBluetoothOn = mBluetoothUtils.isBluetoothOn();
        final boolean isBluetoothLePresent = mBluetoothUtils.isBluetoothLeSupported();
        mBluetoothUtils.askUserToEnableBluetoothIfNeeded();
        if (isBluetoothOn && isBluetoothLePresent) {
            if (mScanner != null) {
                mScanner.scanLeDevice(BleConstants.SCAN_TIMEOUT, true);
                ScanCubeesTabFragment.tvBluetoothConnection.setText(R.string.stop);
                ScanCubeesTabFragment.progressBar.setVisibility(View.VISIBLE);
            }
        }
    }

    /**
     * Method to prepare scan BLE function. Request Permission if necessary.
     *
     * @param mLeScanCallback Callback when a device is found or when a error occur .
     **/
    @SuppressWarnings("unused")
    public void startScanPrepare(BluetoothAdapter.LeScanCallback mLeScanCallback) {
        if (mScanner == null) {
            mScanner = new CubeeBluetoothLeScanner(mLeScanCallback, mBluetoothUtils);
        } else {
            mScanner.setCallback(mLeScanCallback);
        }

        // The COARSE_LOCATION permission is only needed after API 23 to do a BTLE scan
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            PermissionsManager.getInstance().requestPermissionsIfNecessaryForResult(activity,
                    new String[]{Manifest.permission.ACCESS_COARSE_LOCATION}, new PermissionsResultAction() {

                        @Override
                        public void onGranted() {
                            startScan();
                        }

                        @Override
                        public void onDenied(String permission) {
                            Toast.makeText(activity,
                                    R.string.permission_localization,
                                    Toast.LENGTH_LONG)
                                    .show();
                        }
                    });
        } else {
            startScan();
        }


    }


    /**
     * Return the service that controls BLE connection.
     * Can return null
     */
    public BluetoothLeService getmBluetoothLeService() {
        return mBluetoothLeService;
    }

    public void setmBluetoothLeService(BluetoothLeService mBluetoothLeService) {
        this.mBluetoothLeService = mBluetoothLeService;
    }

    public void unBindService() {
        if (mServiceConnection != null) {
            ScanCubeesTabFragment.activity.unbindService(mServiceConnection);
        }
    }


    /**
     * Use the connection service and try to return the Send control characteristic.
     * Used by {@link CubeeBLECommandActivity}
     */
    public BluetoothGattCharacteristic getControlCharacteristic() {
        List<BluetoothGattService> services = mBluetoothLeService.getSupportedGattServices();
        BluetoothGattCharacteristic characteristicToFind = null;
        if (services != null) {
            for (BluetoothGattService service : services) {
                if (service.getUuid().toString().equals(BleConstants.CONTROL_SERVICE)) {
                    List<BluetoothGattCharacteristic> chars = service.getCharacteristics();
                    for (BluetoothGattCharacteristic characteristic : chars) {
                        if (characteristic.getUuid().toString().equals(BleConstants.SEND_CONTROL_CHAR)) {
                            characteristicToFind = characteristic;
                        }
                    }

                }
            }
        }

        if (characteristicToFind != null) {
            return characteristicToFind;
        } else {
            Log.d(TAG, "Unable to find characteristic");
            return null;
        }

    }

    /**
     * Use the connection service and try to return the recive control characteristic.
     * Used by {@link CubeeBLECommandActivity}
     */
    public BluetoothGattCharacteristic getCommandFromCubeeCharacteristic() {
        List<BluetoothGattService> services = mBluetoothLeService.getSupportedGattServices();
        BluetoothGattCharacteristic characteristicToFind = null;
        if (services != null) {
            for (BluetoothGattService service : services) {
                if (service.getUuid().toString().equals(BleConstants.CONTROL_SERVICE)) {
                    List<BluetoothGattCharacteristic> chars = service.getCharacteristics();
                    for (BluetoothGattCharacteristic characteristic : chars) {
                        if (characteristic.getUuid().toString().equals(BleConstants.RECIVE_CONTROL_CHAR)) {
                            characteristicToFind = characteristic;
                        }
                    }

                }
            }
        }

        if (characteristicToFind != null) {
            return characteristicToFind;
        } else {
            Log.d(TAG, "Unable to find characteristic");
            return null;
        }

    }

    /**
     * Use the connection service and try to return the monitor control characteristic.
     * Used by {@link CubeeBLECommandActivity}
     */
    public BluetoothGattCharacteristic getMonitorCharacteristic() {
        List<BluetoothGattService> services = mBluetoothLeService.getSupportedGattServices();
        BluetoothGattCharacteristic characteristicToFind = null;
        if (services != null) {
            for (BluetoothGattService service : services) {
                if (service.getUuid().toString().equals(BleConstants.CONTROL_SERVICE)) {
                    List<BluetoothGattCharacteristic> chars = service.getCharacteristics();
                    for (BluetoothGattCharacteristic characteristic : chars) {
                        if (characteristic.getUuid().toString().equals(BleConstants.MONITOR_CHAR)) {
                            characteristicToFind = characteristic;
                        }
                    }

                }
            }
        }

        if (characteristicToFind != null) {
            return characteristicToFind;
        } else {
            Log.d(TAG, "Unable to find characteristic");
            return null;
        }

    }


    public BluetoothGattCharacteristic getBLECharacteristic(String serviceUUID, String characteristicUUID){
        List<BluetoothGattService> services = mBluetoothLeService.getSupportedGattServices();
        BluetoothGattCharacteristic characteristicToFind = null;
        if (services != null) {
            for (BluetoothGattService service : services) {
                if (service.getUuid().toString().equals(serviceUUID)) {
                    List<BluetoothGattCharacteristic> chars = service.getCharacteristics();
                    for (BluetoothGattCharacteristic characteristic : chars) {
                        if (characteristic.getUuid().toString().equals(characteristicUUID)) {
                            characteristicToFind = characteristic;
                        }
                    }

                }
            }
        }

        if (characteristicToFind != null) {
            return characteristicToFind;
        } else {
            Log.d(TAG, "Unable to find characteristic");
            return null;
        }

    }

    /**
     * Method for read a characteristic
     *
     * @param characteristic characterist to be read. Use the BLE service.
     **/
    public void readCharacteristic(BluetoothGattCharacteristic characteristic) {
        if (characteristic != null) {
            mBluetoothLeService.readCharacteristic(characteristic);
        }
    }

    /**
     * Method for write a characteristic
     *
     * @param characteristic characteristic to be read
     * @param data           data to write
     **/
    public void writeCharacteristic(BluetoothGattCharacteristic characteristic, byte[] data) {
        characteristic.setValue(data);
        mBluetoothLeService.writeCharacteristic(characteristic);
    }

    /**
     * Method for write a characteristic
     *
     * @param characteristic characteristic to be read
     * @param data           data to write
     **/
    public void writeCharacteristicString(BluetoothGattCharacteristic characteristic, String data) {

        if (characteristic != null && data != null) {
            characteristic.setValue(data);
            mBluetoothLeService.writeCharacteristic(characteristic);

        } else {
            Log.d(TAG, "characteristica é null ou data é null");
            ActivityUtils.showToast(activity, "Erro ao executar operação. Tente novamente, por favor.");
            ActivityUtils.cancelProgressDialog();

        }


    }


    @SuppressWarnings("unused")
    public void logCharacteristics() {
        List<BluetoothGattService> services = mBluetoothLeService.getSupportedGattServices();
        if (services != null) {
            for (BluetoothGattService service : services) {
                List<BluetoothGattCharacteristic> chars = service.getCharacteristics();
                for (BluetoothGattCharacteristic characteristic : chars) {
                    Log.d("UUID", service.getUuid().toString() + "<Service Char>" + characteristic.getUuid().toString());

                }

            }
        }

    }

    /**
     * Pause scan
     * Called by {@link ScanCubeesTabFragment}
     */
    public void pauseScan() {
        if (mScanner != null) {
            mScanner.scanLeDevice(-1, false);
        }
    }

    /**
     * Read acharacteris with a loop for monitor a change.
     **/
    public Timer readCharacteristicInLoop(final BluetoothGattCharacteristic characteristic, int timeout, final int v) {
        Timer timer = new Timer();
        timer.schedule(new TimerTask() {
            @Override
            public void run() {
                Log.d(TAG, v + "");
                readCharacteristic(characteristic);
            }
        }, 0, timeout);

        return timer;
    }


    public BluetoothGattCharacteristic getConfigCharacteristic() {
        List<BluetoothGattService> services = mBluetoothLeService.getSupportedGattServices();
        BluetoothGattCharacteristic characteristicToFind = null;
        if (services != null) {
            for (BluetoothGattService service : services) {
                if (service.getUuid().toString().equals(BleConstants.CONTROL_SERVICE)) {
                    List<BluetoothGattCharacteristic> chars = service.getCharacteristics();
                    for (BluetoothGattCharacteristic characteristic : chars) {
                        if (characteristic.getUuid().toString().equals(BleConstants.CONFIG_CHAR)) {
                            characteristicToFind = characteristic;
                        }
                    }

                }
            }
        }

        if (characteristicToFind != null) {
            return characteristicToFind;
        } else {
            Log.d(TAG, "Unable to find characteristic");
            //TODO não encontrou a característica o que fazer ???
            return null;
        }

    }

    @SuppressWarnings("unused")
    public boolean isScanning() {
        return mScanner != null && mScanner.isScanning();

    }

    /**
     * Get DB9 command bluetooth characteristic
     * @return BluetoothGattCharacteristic
     */
    public BluetoothGattCharacteristic getDB9CommandChar() {
        List<BluetoothGattService> services = mBluetoothLeService.getSupportedGattServices();
        BluetoothGattCharacteristic characteristicToFind = null;
        if (services != null) {
            for (BluetoothGattService service : services) {
                if (service.getUuid().toString().equals(BleConstants.CONTROL_SERVICE)) {
                    List<BluetoothGattCharacteristic> chars = service.getCharacteristics();
                    for (BluetoothGattCharacteristic characteristic : chars) {
                        if (characteristic.getUuid().toString().equals(BleConstants.DB9_CHAR)) {
                            characteristicToFind = characteristic;
                        }
                    }

                }
            }
        }

        if (characteristicToFind != null) {
            return characteristicToFind;
        } else {
            Log.d(TAG, "Unable to find characteristic");
            //TODO não encontrou a característica o que fazer ???
            return null;
        }

    }
}