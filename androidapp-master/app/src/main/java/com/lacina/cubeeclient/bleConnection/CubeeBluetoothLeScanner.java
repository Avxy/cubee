package com.lacina.cubeeclient.bleConnection;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.le.ScanCallback;
import android.bluetooth.le.ScanResult;
import android.os.Build;
import android.os.Handler;
import android.util.Log;

import com.lacina.cubeeclient.utils.BluetoothUtils;

/**
 * A BLE Scanner Composition. Set callback, start scan and stop scan.
 * Compatible with Versions above and below LOLLIPOP.
 */
@SuppressWarnings("ALL")
public class CubeeBluetoothLeScanner {
    private final Handler mHandler;

    private final BluetoothUtils mBluetoothUtils;

    @SuppressWarnings("unused")
    private ScanCallback mNewLeScanCallback = null;

    private BluetoothAdapter.LeScanCallback mLeScanCallback;

    private boolean mScanning;

    @SuppressWarnings("unused")
    private String TAG = "mLeScanner";

    @SuppressWarnings("unused")
    public CubeeBluetoothLeScanner(final BluetoothAdapter.LeScanCallback leScanCallback, final BluetoothUtils bluetoothUtils) {
        mHandler = new Handler();
        mLeScanCallback = leScanCallback;
        mBluetoothUtils = bluetoothUtils;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            createNewLeCallback();

        }
    }

    private void createNewLeCallback() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            mNewLeScanCallback = new ScanCallback() {
                @Override
                public void onScanResult(int callbackType, ScanResult result) {
                    super.onScanResult(callbackType, result);
                    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
                        if (result != null && result.getScanRecord() != null) {
                            mLeScanCallback.onLeScan(result.getDevice(), result.getRssi(),
                                    result.getScanRecord().getBytes());
                        }
                    }
                }


                @Override
                public void onScanFailed(int errorCode) {
                    super.onScanFailed(errorCode);
                    Log.d("erro scan", String.valueOf(errorCode));
                }
            };
        }
    }

    public boolean isScanning() {
        return mScanning;
    }

    public void scanLeDevice(final int duration, final boolean enable) {
        if (enable) {
            if (mScanning) {
                return;
            }
            Log.d("TAG", "~ Starting Scan");
            // Stops scanning after a pre-defined scan period.
            if (duration > 0) {
                mHandler.postDelayed(new Runnable() {
                    @Override
                    public void run() {
                        Log.d("TAG", "~ Stopping Scan (timeout)");
                        mScanning = false;
                        mBluetoothUtils.getBluetoothAdapter().stopLeScan(mLeScanCallback);
                    }
                }, duration);
            }
            mScanning = true;
            mBluetoothUtils.getBluetoothAdapter().startLeScan(mLeScanCallback);
        } else {
            Log.d("TAG", "~ Stopping Scan");
            mScanning = false;
            mBluetoothUtils.getBluetoothAdapter().stopLeScan(mLeScanCallback);
        }
//        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.LOLLIPOP) {
//
//            final android.bluetooth.le.BluetoothLeScanner myBluetoothLeScanner = mBluetoothUtils.getBluetoothAdapter().getBluetoothLeScanner();
//            if (enable) {
//                if (!mScanning) {
//                    Log.d(TAG, "~ Starting Scan");
//                    mScanning = true;
//                    ScanSettings.Builder builder = new ScanSettings.Builder();
//                    builder.setScanMode(ScanSettings.SCAN_MODE_BALANCED);
//                    if(mNewLeScanCallback != null){
//                        createNewLeCallback();
//                    }
//                    myBluetoothLeScanner.startScan(null, builder.build(), mNewLeScanCallback);
//                }
//
//            } else {
//                Log.d("TAG", "~ Stopping Scan");
//                mScanning = false;
//                if(myBluetoothLeScanner != null){
//                    myBluetoothLeScanner.stopScan(mNewLeScanCallback);
//                }
//                ActivityUtils.cancelProgressDialog();
//            }
//        } else {
//            if (enable) {
//                if (!mScanning) {
//                    mScanning = true;
//                    Log.d("TAG", "~ Starting Scan");
//                    mBluetoothUtils.getBluetoothAdapter().startLeScan(mLeScanCallback);
//                }
//            } else {
//                if(mScanning){
//                    Log.d("TAG", "~ Stopping Scan");
//                    mScanning = false;
//                    mBluetoothUtils.getBluetoothAdapter().stopLeScan(mLeScanCallback);
//                    ActivityUtils.cancelProgressDialog();
//                }
//            }
//
//        }

    }


    public void setCallback(BluetoothAdapter.LeScanCallback callback) {
        this.mLeScanCallback = callback;
    }
}