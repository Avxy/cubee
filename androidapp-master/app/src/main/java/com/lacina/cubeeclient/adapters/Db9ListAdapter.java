package com.lacina.cubeeclient.adapters;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.TextView;

import com.lacina.cubeeclient.R;
import com.lacina.cubeeclient.model.Db9Rule;

import java.util.ArrayList;

public class Db9ListAdapter extends BaseAdapter {
    private final ArrayList<Db9Rule> items;

    /**
     * Reference to caller context
     */
    private final Context context;

    public Db9ListAdapter(Context context, ArrayList<Db9Rule> items) {
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
        View view = LayoutInflater.from(context).inflate(R.layout.adapter_db9_list, parent, false);
        TextView eventName = (TextView) view.findViewById(R.id.name_rule_db9);
        Db9Rule rule = (Db9Rule) getItem(position);
        eventName.setText(rule.getName());
        return view;
    }

    public Db9Rule getItemEvent(int position) {
        return items.get(position);
    }
}
