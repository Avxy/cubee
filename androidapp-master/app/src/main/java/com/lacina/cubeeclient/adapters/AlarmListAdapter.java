package com.lacina.cubeeclient.adapters;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.TextView;

import com.lacina.cubeeclient.R;
import com.lacina.cubeeclient.model.Alarm;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Locale;

/**
 * Adapter for alarms.
 * Used in {@link com.lacina.cubeeclient.fragments.AlertListFragment}
 */
@SuppressWarnings("ALL")
public class AlarmListAdapter extends BaseAdapter {

    /**
     * LIst of alarms, setted in contructor
     */
    @SuppressWarnings("CanBeFinal")
    private ArrayList<Alarm> items;

    /**
     * Reference to the caller context;
     */
    private final Context context;

    public AlarmListAdapter(Context context, ArrayList<Alarm> items) {
        this.context = context;
        this.items = items;
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

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        //LIST ITEM VIEW
        View view = LayoutInflater.from(context).inflate(R.layout.adapter_alarm_item, parent, false);
        TextView title = (TextView) view.findViewById(R.id.title);
        TextView body = (TextView) view.findViewById(R.id.body);
        TextView data = (TextView) view.findViewById(R.id.data);
        TextView hour = (TextView) view.findViewById(R.id.hour);
        ImageView icon = (ImageView) view.findViewById(R.id.alert_icon);

        //ITEM INFORMATIONS
        Alarm alarm = (Alarm) getItem(position);
        title.setText(alarm.getTitle());
        body.setText(alarm.getBody());

        //SET VIEW WITH INFORMATION
        SimpleDateFormat dateHourFormat = new SimpleDateFormat("HH:mm:ss", Locale.ENGLISH);
        String hourString = dateHourFormat.format(alarm.getDate());
        SimpleDateFormat dateDateFormat = new SimpleDateFormat("dd/MM/yyyy", Locale.ENGLISH);
        String dateString = dateDateFormat.format(alarm.getDate());
        data.setText(dateString);
        hour.setText(hourString);

        //DIFFERENTIATE VIEW WITH ALARM LEVEL
        switch (alarm.getLevel()) {
            case "LOW":
                icon.setBackgroundResource(R.mipmap.icon_alert_low);
                break;
            case "MEDIUM":
                icon.setBackgroundResource(R.mipmap.icon_alert_medium);
                break;
            default:
                icon.setBackgroundResource(R.mipmap.icon_alert_high);
                break;
        }

        return view;
    }

    @SuppressWarnings("unused")
    public Alarm getItemEvent(int position) {
        return items.get(position);
    }
}

