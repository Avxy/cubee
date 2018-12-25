package com.lacina.cubeeclient.adapters;


import android.content.Context;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.TextView;

import com.lacina.cubeeclient.R;
import com.lacina.cubeeclient.fragments.CubeeGridFragment;
import com.lacina.cubeeclient.model.Cubee;

import java.util.ArrayList;
import java.util.List;

/**
 * Adapter for list of cubees at CubeeGridFragment {@link CubeeGridFragment}
 **/
public class CubeeGridAdapter extends BaseAdapter {
    /**
     * CUBEEs list
     */
    private final ArrayList<Cubee> items;

    /**
     * Reference to the caller context
     */
    private final Context context;

    /**
     * List of selected cubees, used for register a event or rule
     */
    private ArrayList<Cubee> listSelectedCubees = new ArrayList<Cubee>();

    public CubeeGridAdapter(Context context, ArrayList<Cubee> items) {
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
        //SET ITEM VIEW
        String nomeCubee = items.get(position).getName();
        View view = LayoutInflater.from(context).inflate(R.layout.adapter_cubee, parent, false);
        TextView tvNomeCubee = (TextView) view.findViewById(R.id.nome_cubee);
        TextView tvEventOrder = (TextView) view.findViewById(R.id.tv_event_order);
        ImageView v = (ImageView) view.findViewById(R.id.img_cubee);

        //SET VIEW WITH CORRESPONDING ITEM INFORMATION
        tvNomeCubee.setText(nomeCubee);
        Cubee cubee = items.get(position);

        //CHANGE VIEW WHEN A ITEM IS ADDED TO SELECTED LIST
        //PUT CARDINAL ORDER
        try {
            if (cubee.getCubeeBtn()) {
                if (listSelectedCubees.contains(cubee)) {
                    tvEventOrder.setVisibility(View.VISIBLE);
                    v.setBackgroundResource(R.mipmap.img_cubee_event_signal_selected);

                    String positionSelected = 1 + listSelectedCubees.indexOf(cubee) + "º";
                    tvEventOrder.setText(positionSelected);


                } else {
                    tvEventOrder.setVisibility(View.GONE);
                    v.setBackgroundResource(R.mipmap.img_cubee_signal_on);
                }

            } else {
                if (listSelectedCubees.contains(cubee)) {
                    tvEventOrder.setVisibility(View.VISIBLE);
                    v.setBackgroundResource(R.mipmap.img_cubee_event_selected);

                    String positionSelected = 1 + listSelectedCubees.indexOf(cubee) + "º";
                    tvEventOrder.setText(positionSelected);


                } else {
                    tvEventOrder.setVisibility(View.GONE);
                }
            }
        } catch (Exception e) {
            Log.d("Home", e.getMessage());
        }
        return view;
    }


    public void setListSelectedCubees(ArrayList<Cubee> listSelectedCubees) {
        this.listSelectedCubees = listSelectedCubees;
    }

    public List<Cubee> getSelectedCubees(){
        return listSelectedCubees;
    }

    public ArrayList<Cubee> getItems() {
        return items;
    }

    public Context getContext() {
        return context;
    }

    public ArrayList<Cubee> getListSelectedCubees() {
        return listSelectedCubees;
    }
}