package com.lacina.cubeeclient.adapters;


import android.bluetooth.BluetoothDevice;
import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.TextView;

import com.lacina.cubeeclient.R;
import com.lacina.cubeeclient.fragments.ScanCubeesTabFragment;

import java.util.ArrayList;

/**
 * Adapter for BLE devices list. Seted up at {@link ScanCubeesTabFragment}
 **/
@SuppressWarnings("ALL")
public class BLEListAdapter extends BaseAdapter {
    /**
     * List of devices
     */
    private ArrayList<BluetoothDevice> items;

    /**
     * Reference to caller context
     */
    private final Context context;

    public BLEListAdapter(Context context) {
        super();
        this.context = context;
        this.items = new ArrayList<>();
    }

    @Override
    public int getCount() {
        return items.size();
    }

    @Override
    public Object getItem(int position) {
        return items.get(position);
    }

    @Override
    public long getItemId(int position) {
        return position;
    }

    /**
     * Add a device in the list if its not there yet
     * @param device device to be added
     */
    public void addDevice(BluetoothDevice device) {
        if (!items.contains(device)) {
            items.add(device);
        }
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        //VIEW
        LayoutInflater vi = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        convertView = vi.inflate(R.layout.adapter_cubee, parent, false);
        TextView nomeCubee = (TextView) convertView.findViewById(R.id.nome_cubee);
        convertView.setTag(nomeCubee);

        //SET VIEW WITH INTEN INFORMATION
        nomeCubee.setText(items.get(position).getName());

        return convertView;
    }

    /**
     * Return a bluetooth device given a position
     * @param position position in {@link #items}
     * @return return a bluetooth device
     */
    public BluetoothDevice getDevice(int position) {
        return items.get(position);

    }

    /**
     * Clear intens and refresh UI
     */
    public void flushDevices() {
        this.items = new ArrayList<>();
        this.notifyDataSetChanged();
    }

    private static class ViewHolder {
        @SuppressWarnings("unused")
        private TextView nomeCubee;
    }
}