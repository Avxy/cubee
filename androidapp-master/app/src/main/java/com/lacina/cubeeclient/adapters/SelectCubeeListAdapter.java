package com.lacina.cubeeclient.adapters;


import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.TextView;

import com.lacina.cubeeclient.R;
import com.lacina.cubeeclient.model.Cubee;

import java.util.ArrayList;
import java.util.List;
/**
 * Adapter for a generic list where you can select cubees
 * Used in many situations, like select cubees to add to a sector
 */
public class SelectCubeeListAdapter extends BaseAdapter {
    /**
     * Cubees list
     */
    @SuppressWarnings("CanBeFinal")
    private ArrayList<Cubee> items;

    /**
     * Caller activity context
     */
    private final Context context;

    /**
     * Selected cubees
     */
    private List<Cubee> itemSelected;


    public SelectCubeeListAdapter(Context context, ArrayList<Cubee> items) {
        super();
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
        //SET VIEW
        View view = LayoutInflater.from(context).inflate(R.layout.adapter_listview_add_remove_cubee_to_sector, parent, false);
        TextView tv_cubeeName = (TextView) view.findViewById(R.id.tv_cubee_name);
        ImageView img_cubee = (ImageView) view.findViewById(R.id.cubee_img);

        //SET INFORMATION
        String nomeCubee = items.get(position).getName();
        tv_cubeeName.setText(nomeCubee);
        if (itemSelected != null && itemSelected.contains(items.get(position))) {
            img_cubee.setBackgroundResource(R.mipmap.img_cubee_signal_off);
        } else {
            img_cubee.setBackgroundResource(R.mipmap.cubee_simple);
        }

        return view;
    }

    public void setItemSelected(List<Cubee> itemSelected) {
        this.itemSelected = itemSelected;
    }
}