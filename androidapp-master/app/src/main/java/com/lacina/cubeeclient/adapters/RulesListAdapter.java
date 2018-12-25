package com.lacina.cubeeclient.adapters;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.TextView;

import com.lacina.cubeeclient.R;
import com.lacina.cubeeclient.model.Rules;

import java.util.ArrayList;

public class RulesListAdapter extends BaseAdapter {
    @SuppressWarnings("CanBeFinal")
    private ArrayList<Rules> items;


    /**
     * Reference to caller context
     */
    private final Context context;

    public RulesListAdapter(Context context, ArrayList<Rules> items) {
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
        //SET UP VIEW
        View view = LayoutInflater.from(context).inflate(R.layout.adapter_rule_item, parent, false);
        TextView eventName = (TextView) view.findViewById(R.id.name_rule);
        Rules rule = (Rules) getItem(position);

        //VIEW INFORMATION
        eventName.setText(rule.getName());
        return view;
    }

    public Rules getItemEvent(int position) {
        return items.get(position);
    }
}