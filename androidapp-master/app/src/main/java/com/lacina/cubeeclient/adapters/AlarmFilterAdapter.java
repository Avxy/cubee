package com.lacina.cubeeclient.adapters;

import android.content.Context;
import android.graphics.Color;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.TextView;

import com.lacina.cubeeclient.R;

import java.util.ArrayList;

public class AlarmFilterAdapter extends BaseAdapter{
    private final ArrayList<String> items;

    /**
     * Reference to caller context
     */
    private final Context context;

    /**
     * Selected item
     */
    private ArrayList<Integer> selectedFilters;

    /**
     * Fixed items.
     */
    private final ArrayList<String> staticButtons = new ArrayList<String>();

    private static final int CUBEE_FILTER = 0;
    private static final int HIGH_CRITICALITY = 1;
    private static final int MEDIUM_CRITICALITY = 2;
    private static final int LOW_CRITICALITY = 3;

    /**
     *
     * @param context caller activity context
     *
     */
    public AlarmFilterAdapter(Context context) {
        super();
        addStaticButtons();
        this.context = context;
        this.items = new ArrayList<>();
        this.items.addAll(staticButtons);
        selectedFilters = new ArrayList<>();
    }

    private void addStaticButtons() {
        staticButtons.add("Filtrar por CUBEE");
        staticButtons.add("Alta criticidade");
        staticButtons.add("Media criticidade");
        staticButtons.add("Baixa criticidade");
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

    @SuppressWarnings("unused")
    public Boolean addAll(ArrayList<String> filters) {
        return items.addAll(filters);

    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        String filterName = items.get(position);
        View view = LayoutInflater.from(context).inflate(R.layout.adapter_alarm_filters, parent, false);
        ImageView image = (ImageView) view.findViewById(R.id.img_filter);
        TextView t = (TextView) view.findViewById(R.id.name_filter);

        if(position == HIGH_CRITICALITY && selectedFilters.contains(HIGH_CRITICALITY)){
            image.setImageResource(R.mipmap.icon_alert_high);
            image.setBackgroundColor(Color.GRAY);
        }else if(position == HIGH_CRITICALITY){
            image.setImageResource(R.mipmap.icon_alert_high);
            image.setBackgroundColor(Color.parseColor("#606060"));
        }
        if(position == MEDIUM_CRITICALITY && selectedFilters.contains(MEDIUM_CRITICALITY)){
            image.setImageResource(R.mipmap.icon_alert_medium);
            image.setBackgroundColor(Color.GRAY);
        }else if(position == MEDIUM_CRITICALITY){
            image.setImageResource(R.mipmap.icon_alert_medium);
            image.setBackgroundColor(Color.parseColor("#606060"));
        }
        if(position == LOW_CRITICALITY && selectedFilters.contains(LOW_CRITICALITY)){
            image.setImageResource(R.mipmap.icon_alert_low);
            image.setBackgroundColor(Color.GRAY);
        }else if(position == LOW_CRITICALITY){
            image.setImageResource(R.mipmap.icon_alert_low);
            image.setBackgroundColor(Color.parseColor("#606060"));
        }

        t.setText(filterName);

        return view;
    }

    public void setSelectedFilters(Integer selectedFilter) {
        if(!selectedFilters.contains(selectedFilter)){
            this.selectedFilters.add(selectedFilter);
        }else if(selectedFilters.contains(selectedFilter)){
            this.selectedFilters.remove(selectedFilter);
        }

    }

    public ArrayList<Integer> getFilters(){
        return selectedFilters;
    }

}
