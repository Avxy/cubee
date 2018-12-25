package com.lacina.cubeeclient.utils;

import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothManager;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;

/**
 * Class with some utility methods
 **/
@SuppressWarnings("ALL")
public final class BluetoothUtils {
    private final static int REQUEST_ENABLE_BT = 2001;
    private final Activity mActivity;
    private final BluetoothAdapter mBluetoothAdapter;

    public BluetoothUtils(final Activity activity) {
        mActivity = activity;
        final BluetoothManager btManager = (BluetoothManager) mActivity.getSystemService(Context.BLUETOOTH_SERVICE);
        mBluetoothAdapter = btManager.getAdapter();
    }

    /**
     * check if BLE Supported device
     */
    @SuppressWarnings("unused")
    public static boolean isBLESupported(Context context) {
        return context.getPackageManager().hasSystemFeature(PackageManager.FEATURE_BLUETOOTH_LE);
    }

    /**
     * get BluetoothManager
     */
    @SuppressWarnings("unused")
    public static BluetoothManager getManager(Context context) {
        return (BluetoothManager) context.getSystemService(Context.BLUETOOTH_SERVICE);
    }

    public void askUserToEnableBluetoothIfNeeded() {
        if (isBluetoothLeSupported() && (mBluetoothAdapter == null || !mBluetoothAdapter.isEnabled())) {
            final Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            mActivity.startActivityForResult(enableBtIntent, REQUEST_ENABLE_BT);
        }
    }

    public BluetoothAdapter getBluetoothAdapter() {
        return mBluetoothAdapter;
    }

    public boolean isBluetoothLeSupported() {
        return mActivity.getPackageManager().hasSystemFeature(PackageManager.FEATURE_BLUETOOTH_LE);
    }

    public boolean isBluetoothOn() {
        return mBluetoothAdapter != null && mBluetoothAdapter.isEnabled();
    }
}
