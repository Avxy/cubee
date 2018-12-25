package com.lacina.cubeeclient.adapters;

import android.app.DatePickerDialog;
import android.app.TimePickerDialog;
import android.content.Context;
import android.support.annotation.IdRes;
import android.support.annotation.NonNull;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.DatePicker;
import android.widget.RadioGroup;
import android.widget.TextView;
import android.widget.TimePicker;

import com.lacina.cubeeclient.R;
import com.lacina.cubeeclient.interfaces.OnDatePicked;
import com.lacina.cubeeclient.interfaces.OnOffChange;
import com.lacina.cubeeclient.model.Cubee;
import com.lacina.cubeeclient.model.Task;

import java.util.Calendar;
import java.util.Date;
import java.util.List;


public class TaskEditListAdapter extends BaseAdapter {

    private final OnDatePicked onDatePicked;

    @SuppressWarnings("CanBeFinal")
    private List<Cubee> items;

    /**
     * Reference to caller context
     */
    private final Context context;

    private int startYear;

    private int starthMonth;

    private int startDay;

    @SuppressWarnings("CanBeFinal")
    private List<Task> taskList;

    @SuppressWarnings("CanBeFinal")
    private OnOffChange onOffChange;

    private int startMinute;

    private int startHour;

    /**
     *
     * @param context caller context
     * @param cubeeList list of cubees related with the task list
     * @param taskList task list to show
     * @param onDatePicked interface to communicate the date picked to the caller activity
     * @param onOffChange interface to communicate the command picked to the caller acticity
     */
    public TaskEditListAdapter(Context context, List<Cubee> cubeeList, List<Task> taskList, OnDatePicked onDatePicked, OnOffChange onOffChange) {
        super();
        this.context = context;
        this.items = cubeeList;
        this.onDatePicked = onDatePicked;
        this.taskList = taskList;
        this.onOffChange = onOffChange;
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
     * Set each list item view.
     **/
    @Override
    public View getView(final int position, View convertView, ViewGroup parent) {
        //SET VIEW
        LayoutInflater vi = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        convertView = vi.inflate(R.layout.adapter_task_date, parent, false);
        TextView nomeCubee = (TextView) convertView.findViewById(R.id.nome_cubee);
        Button dateButton = (Button) convertView.findViewById(R.id.btn_date);
        Button hourButton = (Button) convertView.findViewById(R.id.btn_hour);
        TextView tvEventOrder = (TextView) convertView.findViewById(R.id.tv_event_order);
        TextView tvDate = (TextView) convertView.findViewById(R.id.tv_date);
        TextView tvHour = (TextView) convertView.findViewById(R.id.tv_hour);
        RadioGroup rgOnOff = (RadioGroup) convertView.findViewById(R.id.rg_on_off);
        convertView.setTag(nomeCubee);


        //SET VIEW INFORMATION
        final Cubee cubee = items.get(position);
        Task task = taskList.get(position);
        final String dateString = task.getDateTaskDataString();
        final String hourString = task.getDateTaskHourString();
        final Calendar calendar = Calendar.getInstance();
        String positionSelected = 1 + items.indexOf(cubee) + "º";
        tvEventOrder.setText(positionSelected);
        nomeCubee.setText(items.get(position).getName());
        tvDate.setText(dateString);
        tvHour.setText(hourString);
        int initialChecked = task.getAppcommand();
        if (initialChecked == 1) {
            rgOnOff.check(R.id.rbtn_on);
        } else if (initialChecked == 2) {
            rgOnOff.check(R.id.rbtn_off);
        }
        calendar.setTime(task.getDateTask());
        startYear = calendar.get(Calendar.YEAR);
        starthMonth = calendar.get(Calendar.MONTH);
        startDay = calendar.get(Calendar.DATE);
        startHour = calendar.get(Calendar.HOUR_OF_DAY);
        startMinute = calendar.get(Calendar.MINUTE);

        //IF CHANGE COMMAND
        rgOnOff.setOnCheckedChangeListener(new RadioGroup.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(RadioGroup group, @IdRes int checkedId) {
                int command = 1;
                if (checkedId == R.id.rbtn_on) {
                    command = 1;
                } else if (checkedId == R.id.rbtn_off) {
                    command = 2;
                }
                onOffChange.changeOnOffCommand(position, command);

            }
        });

        //IF CHANGE DATE
        dateButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                DatePickerDialog datePickerDialog = getDatePickerDialog(cubee, position, calendar);
                datePickerDialog.show();
            }
        });


        //IF CHANGE HOUR
        hourButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                TimePickerDialog mTimePicker;
                mTimePicker = getTimePickerDialog(cubee, position, calendar);
                mTimePicker.show();
            }
        });

        return convertView;
    }

    @NonNull
    private TimePickerDialog getTimePickerDialog(final Cubee cubee, final int position, final Calendar calendarPicked) {
        return new TimePickerDialog(context, new TimePickerDialog.OnTimeSetListener() {
            @Override
            public void onTimeSet(TimePicker view, int selectedHour, int selectedMinute) {
                calendarPicked.set(Calendar.HOUR_OF_DAY, selectedHour);
                calendarPicked.set(Calendar.MINUTE, selectedMinute);
                Date date = calendarPicked.getTime();
                onDatePicked.onHourPicked(cubee, date, position);
            }
        }, startHour, startMinute, true);
    }

    @NonNull
    private DatePickerDialog getDatePickerDialog(final Cubee cubee, final int position, final Calendar calendarPicked) {
        return new DatePickerDialog(
                context, new DatePickerDialog.OnDateSetListener() {
            @Override
            public void onDateSet(DatePicker view, int year, int month, int dayOfMonth) {
                calendarPicked.set(Calendar.YEAR, year);
                calendarPicked.set(Calendar.MONTH, month);
                calendarPicked.set(Calendar.DAY_OF_MONTH, dayOfMonth);
                Date date = calendarPicked.getTime();
                onDatePicked.onDatePicked(cubee, date, position);
            }
        }, startYear, starthMonth, startDay);
    }


}

