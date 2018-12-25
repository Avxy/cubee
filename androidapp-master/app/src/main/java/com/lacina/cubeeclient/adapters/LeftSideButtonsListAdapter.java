package com.lacina.cubeeclient.adapters;


import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.TextView;

import com.lacina.cubeeclient.R;
import com.lacina.cubeeclient.model.Sector;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

/**
 * Side buttons list adapter. Treat all like sectors,
 * Set view for all items, on click comportaments setted at activity
 * USED IN: {@link com.lacina.cubeeclient.activities.MainActivity}
 */
@SuppressWarnings("ALL")
public class LeftSideButtonsListAdapter extends BaseAdapter {
    private final List<Sector> items;

    /**
     * Reference to caller context
     */
    private final Context context;

    /**
     * Selected item
     */
    private int selectedIndex;

    /**
     * Fixed items.
     */
    private final List<Sector> staticButtons = Arrays.asList(new Sector("Add Setor", "add"), new Sector("Todos os CUBEEs", "all_cubees"), new Sector("CUBEEs sem Setor", "no_sector"));

    /**
     *
     * @param context caller activity context
     * @param items items to show in a list
     */
    public LeftSideButtonsListAdapter(Context context, List<Sector> items) {
        super();
        this.context = context;
        this.items = new ArrayList<>();
        this.items.addAll(staticButtons);
        this.items.addAll(items);
    }

    @Override
    public int getCount() {
        return items.size();
    }

    @Override
    public Object getItem(int position) {
        return items.get(position);
    }

    @SuppressWarnings("unused")
    public Sector getSector(int position) {
        return items.get(position);
    }


    public String getItem_id(int position) {
        return items.get(position).get_id();
    }

    @Override
    public long getItemId(int position) {
        return position;
    }

    @SuppressWarnings("unused")
    public Boolean addItem(Sector sector) {
        Boolean result = items.add(sector);
        notifyDataSetChanged();
        return result;
    }

    @SuppressWarnings("unused")
    public Boolean addAll(ArrayList<Sector> sectorList) {
        return items.addAll(sectorList);

    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        String nomeSector = items.get(position).getName();
        View view = LayoutInflater.from(context).inflate(R.layout.adapter_sector, parent, false);
        ImageView image = (ImageView) view.findViewById(R.id.img_sector);
        TextView t = (TextView) view.findViewById(R.id.nome_sector);


        if (position == 0 && position == selectedIndex) {
            image.setImageResource(R.mipmap.add_sector_selected);
        } else if (position == 0) {
            image.setImageResource(R.mipmap.add_sector);
        } else if (position == 1 && position == selectedIndex) {
            image.setImageResource(R.mipmap.all_cubees_clicked);
        } else if (position == 1) {
            image.setImageResource(R.mipmap.all_cubees_unclicked);
        } else if (position == 2 && position == selectedIndex) {
            image.setImageResource(R.mipmap.no_sector_selected);
        } else if (position == 2) {
            image.setImageResource(R.mipmap.no_sector_unselected);
        } else if (position == selectedIndex) {
            image.setImageResource(R.mipmap.setor_selected);
        } else {
            image.setImageResource(R.mipmap.sector_unselected);
        }

        t.setText(nomeSector);

        return view;
    }

    public void setSelectedIndex(int selectedIndex) {
        this.selectedIndex = selectedIndex;
    }


}