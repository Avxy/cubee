package com.lacina.cubeeclient.fragments;


import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.le.BluetoothLeScanner;
import android.bluetooth.le.ScanCallback;
import android.bluetooth.le.ScanResult;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Build;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v4.app.Fragment;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ImageButton;
import android.widget.ListView;
import android.widget.ProgressBar;
import android.widget.TextView;

import com.lacina.cubeeclient.R;
import com.lacina.cubeeclient.activities.CubeeBLECommandActivity;
import com.lacina.cubeeclient.adapters.BLEListAdapter;
import com.lacina.cubeeclient.bleConnection.bleService.BluetoothLeService;
import com.lacina.cubeeclient.controllers.BLEConnectionController;
import com.lacina.cubeeclient.interfaces.BleScanChangeable;
import com.lacina.cubeeclient.interfaces.ChangeCenterFragmentInterface;
import com.lacina.cubeeclient.utils.ActivityUtils;
import com.lacina.cubeeclient.utils.BluetoothUtils;

import java.util.ArrayList;

import butterknife.BindView;
import butterknife.ButterKnife;


/**
 * Fragment for scan cubees with BLE.
 **/
@SuppressWarnings("ALL")
public class ScanCubeesTabFragment extends BackableFragment {
    public static final String EXTRA_DEVICE = "ExtraDevice";

    /**
     * Tag to auxiliate the application log
     */
    private static final String TAG = "ScanFrag";

    private static final String PARAM_GO_WHEN_CONNECT = "activityToGoWhenConnect";

    /**
     * Text view to show scan state
     */
    public static TextView tvBluetoothConnection;

    /**
     * Reference to the current activity
     */
    public static Activity activity;

    /**
     * A broadcast receiver to listener for ddevice connection status.
     * Change to CubeeBleFragment when services ares discovered.
     **/

    public static ProgressBar progressBar;

    /**
     * List view of cubees founded
     */
    @SuppressWarnings("unused")
    @BindView(R.id.listview)
    public ListView lvCubeesList;

    /**
     * Button to start/stop scan
     */
    @SuppressWarnings("unused")
    @BindView(R.id.btn_bluetooth_conection)
    public ImageButton btnBluetoothConnection;

    /**
     * Array List of cubees fouded
     */
    @SuppressWarnings("unused")
    private ArrayList cubees;
    /*
      callback for whrn a device is found.
     */
    //private BluetoothAdapter.LeScanCallback mLeScanCallback;

    /**
     * Adapter to control Listview {@link #lvCubeesList}
     */
    private BLEListAdapter adapter;

    /**
     * Fragment manages reference to do fragment transactions
     */
    @SuppressWarnings("unused")
    private android.support.v4.app.FragmentManager fragmentManager;

    /**
     * Ble controlles to use for some operations with BLE connection.
     */
    private BLEConnectionController bleConnectionController;

    @SuppressWarnings("unused")
    private int MY_PERMISSIONS_CODE = 1;

    @SuppressWarnings("unused")
    private String activityToGoWhenConnect;

    private BluetoothAdapter mBluetoothAdapter;

    private BluetoothLeScanner mBluetoothLeScanner;

    private boolean scanning = false;

    private BluetoothUtils bluetoothUtils;

    private ScanCallback mLeScanCallback;

    @SuppressWarnings("unused")
    public ScanCubeesTabFragment() {
        // Required empty public constructor
    }

    public static Fragment newInstance(String fragmentToGoWhenConnect) {
        ScanCubeesTabFragment f = new ScanCubeesTabFragment();
        // Supply index input as an argument.
        Bundle args = new Bundle();
        args.putString(PARAM_GO_WHEN_CONNECT, fragmentToGoWhenConnect);
        f.setArguments(args);
        return f;
    }

    //AlertsFilter the broadcast Receiver
    @SuppressWarnings("unused")
    private static IntentFilter makeGattUpdateIntentFilter() {
        final IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(BluetoothLeService.ACTION_GATT_CONNECTED);
        intentFilter.addAction(BluetoothLeService.ACTION_GATT_DISCONNECTED);
        intentFilter.addAction(BluetoothLeService.ACTION_GATT_CONNECTING);
        intentFilter.addAction(BluetoothLeService.ACTION_GATT_SERVICES_DISCOVERED);
        return intentFilter;
    }

    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        bleConnectionController = BLEConnectionController.getInstance();
        bleConnectionController.setActivity(getActivity());
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            mLeScanCallback = new ScanCallback() {

                @Override
                public void onScanResult(int callbackType, final ScanResult result) {
                    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
                        result.getDevice().fetchUuidsWithSdp();
                        if (result.getDevice().getName() != null && result.getDevice().getName().contains("CUBEE")) {
                            adapter.addDevice(result.getDevice());
                            adapter.notifyDataSetChanged();
                        }
                    }

                }

                @Override
                public void onScanFailed(int errorCode) {
                    super.onScanFailed(errorCode);
                    Log.i("BLE", "error");
                }
            };
        }


    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        // Inflate the layout for this fragment
        View fragmentView = inflater.inflate(R.layout.fragment_scan_cubees, container, false);
        activity = getActivity();
        ButterKnife.bind(this, fragmentView);

        fragmentManager = getFragmentManager();
        tvBluetoothConnection = (TextView) fragmentView.findViewById(R.id.tv_bluetooth_connection);


        if (getArguments() != null) {
            activityToGoWhenConnect = getArguments().getString(PARAM_GO_WHEN_CONNECT, CubeeBLECommandActivity.class.getName());
        } else {
            activityToGoWhenConnect = CubeeBLECommandActivity.class.getName();
        }

        adapter = new BLEListAdapter(activity);
        lvCubeesList.setAdapter(adapter);
        progressBar = (ProgressBar) LayoutInflater.from(activity).inflate(R.layout.actionbar_progress_indeterminate, null);
        lvCubeesList.addFooterView(progressBar);

        createOnClickListeners();

        bluetoothUtils = new BluetoothUtils(activity);
        bluetoothUtils.askUserToEnableBluetoothIfNeeded();
        this.mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP && mBluetoothAdapter != null) {
            this.mBluetoothLeScanner = mBluetoothAdapter.getBluetoothLeScanner();
        }

        return fragmentView;
    }

    private void createOnClickListeners() {
        lvCubeesList.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                if (position < adapter.getCount()) {
                    Intent intent;
                    if (activity instanceof BleScanChangeable) {
                        intent = new Intent(activity, ((BleScanChangeable) activity).activityToGoTo());
                        intent.putExtra(EXTRA_DEVICE, adapter.getDevice(position));
                        bleConnectionController.pauseScan();
                        tvBluetoothConnection.setText(R.string.scan);
                        progressBar.setVisibility(View.INVISIBLE);
                        adapter.flushDevices();
                        startActivity(intent);
                    } else {
                        ActivityUtils.showToast(activity, "Erro ao escanear, tente novamente.");
                        Log.d(TAG, "Erro ao escanear, erro na interface");
                    }
                }
            }
        });


        btnBluetoothConnection.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Log.d(TAG, "Scan Button Click");

                if (scanning) {

                    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
                        if (mBluetoothLeScanner != null) {
                            mBluetoothLeScanner.stopScan(mLeScanCallback);

                        }
                    }
                    bleConnectionController.pauseScan();
                    tvBluetoothConnection.setText(R.string.scan);
                    progressBar.setVisibility(View.INVISIBLE);
                    Log.i("Scanning", "stop");
                    scanning = false;

                } else {

                    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
                        if (bluetoothUtils.isBluetoothOn() && mBluetoothLeScanner != null) {
                            Log.i("Scanning", "start");
                            adapter.flushDevices();
                            scanning = true;
                            tvBluetoothConnection.setText(R.string.stop);
                            progressBar.setVisibility(View.VISIBLE);
                            mBluetoothLeScanner.startScan(mLeScanCallback);
                        } else {
                            bluetoothUtils.askUserToEnableBluetoothIfNeeded();
                        }
                    }
                }


            }
        });


    }

    /*
    *         if (enable) {
            mScanning = true;
            Log.i("Scanning", "start");
            mBluetoothLeScanner.startScan(mLeScanCallback);
        } else {
            Log.i("Scanning", "stop");
            mScanning = false;
            mBluetoothLeScanner.stopScan(mLeScanCallback);
        }
    *
    * */


    private void resetScan() {
        tvBluetoothConnection.setText(R.string.scan);
        progressBar.setVisibility(View.INVISIBLE);
        bleConnectionController.pauseScan();
    }


    /**
     * Pause any scan and unregister the broadcast receiver
     **/
    @Override
    public void onPause() {
        super.onPause();
        bleConnectionController.pauseScan();
    }


    /**
     * Register broadcast receiver at onResume
     **/
    @Override
    public void onResume() {
        super.onResume();
        ActivityUtils.cancelProgressDialog();
        resetScan();
    }

    @SuppressWarnings("unused")
    private void updateConnectionState(final State state) {
        switch (state) {
            case CONNECTED:
                break;
            case DISCONNECTED:
                ActivityUtils.showToast(activity, "Erro ao conectar, tente novamente.");
                ActivityUtils.cancelProgressDialog();
//                FragmentTransaction fragmentTransactionRefresh = fragmentManager.beginTransaction();
//                fragmentTransactionRefresh.replace(R.id.fragment_ble_ln_inferior, ScanCubeesTabFragment.newInstance());
//                fragmentTransactionRefresh.commit();

                break;
            case CONNECTING:
                break;

            default:
                ActivityUtils.cancelProgressDialog();
                break;
        }

    }

    @Override
    public void onBackButtonPressed() {
        if (activity instanceof ChangeCenterFragmentInterface) {
            ((ChangeCenterFragmentInterface) activity).changeCenterFragmentGridFragmentToSelected();
        }
    }


    @SuppressWarnings("unused")
    private enum State {
        DISCONNECTED,
        CONNECTING,
        CONNECTED
    }


}