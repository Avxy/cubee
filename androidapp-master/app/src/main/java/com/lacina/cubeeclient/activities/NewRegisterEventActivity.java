package com.lacina.cubeeclient.activities;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.v7.app.AppCompatActivity;
import android.text.TextUtils;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ListView;

import com.android.volley.VolleyError;
import com.google.android.gms.tasks.OnCompleteListener;
import com.google.firebase.auth.FirebaseAuth;
import com.google.firebase.auth.FirebaseUser;
import com.google.firebase.auth.GetTokenResult;
import com.google.gson.Gson;
import com.google.gson.GsonBuilder;
import com.google.gson.JsonObject;
import com.lacina.cubeeclient.R;
import com.lacina.cubeeclient.adapters.TaskEditListAdapter;
import com.lacina.cubeeclient.interfaces.OnDatePicked;
import com.lacina.cubeeclient.interfaces.OnOffChange;
import com.lacina.cubeeclient.model.Cubee;
import com.lacina.cubeeclient.model.Event;
import com.lacina.cubeeclient.model.Task;
import com.lacina.cubeeclient.serverConnection.AppConfig;
import com.lacina.cubeeclient.serverConnection.RequestQueueSingleton;
import com.lacina.cubeeclient.serverConnection.callbacks.OnPostJsonObjectCallback;
import com.lacina.cubeeclient.serverConnection.requests.PostJsonObjectRequest;
import com.lacina.cubeeclient.utils.ActivityUtils;
import com.lacina.cubeeclient.utils.VolleyErrorUtils;

import java.util.ArrayList;
import java.util.Date;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import butterknife.BindView;
import butterknife.ButterKnife;

/**
 * This activity turn place when is needed to register a new event.
 * An event has a time and a command. When event time has come the backend server tries to make the command performs.
 * To Register an event you firt need to select cubees, and define the hour and command to each one
 * The time of a CUBEE B cannot be posterior to the time of his posterioir cubee in order, A.
 */
@SuppressWarnings("ALL")
public class NewRegisterEventActivity extends AppCompatActivity implements OnDatePicked, OnOffChange {

    private static final String TAG = "RegisterEventA";

    /**
     * View for event list
     */
    @SuppressWarnings("unused")
    @BindView(R.id.lv_cubees_events)
    public ListView lvEventList;

    /**
     * View for cancel button.
     */
    @SuppressWarnings("unused")
    @BindView(R.id.btn_cancel)
    public Button btnCancel;

    /**
     * View for register event button
     */
    @SuppressWarnings("unused")
    @BindView(R.id.btn_register_event)
    public Button btnRegisterEvent;

    /**
     * View for the text input for the name of the event.
     */
    @SuppressWarnings("unused")
    @BindView(R.id.edt_event_name)
    public EditText edtEventName;

    /**
     * Cubee list that come from the caller activity, passed via Intent.
     */
    private ArrayList<Cubee> cubeeList;

    /**
     * Adapter to control the list view.
     */
    private TaskEditListAdapter eventAdapter;

    /**
     * List of tasks, each task has a cubee, date and command
     * see {@link Task}
     */
    private List<Task> taskList;

    /**
     * Firebase user profile information object.
     * Contain helper methods to o change or retrieve profile information,
     * as well as to manage that user's authentication state.
     */
    private FirebaseUser firebaseUser;

    /**
     * Reference to this activity.
     */
    private Activity activity;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        //Setup activity variables
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_resgister_event);
        ButterKnife.bind(this);
        firebaseUser = FirebaseAuth.getInstance().getCurrentUser();
        activity = this;


        //Get cubee list from intent, caller activity has to send this list.
        Bundle extras = getIntent().getExtras();
        if (extras != null) {
            //noinspection unchecked
            cubeeList = (ArrayList) extras.get("ListOfCubees");
        }


        //Set up a task list with the given cubee list
        //noinspection Convert2Diamond
        taskList = new ArrayList<Task>();
        for (int i = 0; i < cubeeList.size(); i++) {
            Date date = new Date();
            date.setMinutes(0);
            date.setHours(0);
            date.setSeconds(0);

            String previoutTask = "";
            Task task = new Task(null, "", cubeeList.get(i).get_id(), cubeeList.get(i).getName(), previoutTask, 1, date);
            taskList.add(task);
        }

        //Setup view with build taskl list
        eventAdapter = new TaskEditListAdapter(this, cubeeList, taskList, this, this);
        lvEventList.setAdapter(eventAdapter);
        eventAdapter.notifyDataSetChanged();
        initButtons();
    }

    private void initButtons() {
        btnCancel.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                finish();
            }
        });

        btnRegisterEvent.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                attemptRegisterEvent();
            }
        });

    }

    /**
     * Called when the submit button is pressed
     * Validate if the event name is not empty
     */
    private void attemptRegisterEvent() {
        edtEventName.setError(null);

        // Store values at the time of the login attempt.
        String name = edtEventName.getText().toString().trim();

        boolean cancel = false;
        View focusView = null;


        // Check for a valid email address.
        if (TextUtils.isEmpty(name)) {
            edtEventName.setError(getString(R.string.error_field_required));
            focusView = edtEventName;
            cancel = true;
        }


        if (cancel) {
            focusView.requestFocus();
        } else {
            ActivityUtils.showProgressDialog(activity, "Cadastrando evento");
            createEventAndSendToServer(name);
        }
    }

    /**
     * Requet to send event to server
     * Callback: {@link #getPostEventRegisterCallback()}
     *
     * @param name name of the event
     */
    private void createEventAndSendToServer(final String name) {
        if (firebaseUser != null) {
            firebaseUser.getIdToken(true)
                    .addOnCompleteListener(new OnCompleteListener<GetTokenResult>() {
                        public void onComplete(@NonNull com.google.android.gms.tasks.Task<GetTokenResult> task) {
                            if (task.isSuccessful()) {
                                String idToken = task.getResult().getToken();

                                //Creates a model of a event with build tasklist
                                Event event = new Event("", name, firebaseUser.getUid(), true, taskList);

                                //Reflect event model to Json, to sent it to server
                                Map<String, String> params = new HashMap<>();
                                GsonBuilder gsonBuilder = new GsonBuilder();
                                gsonBuilder.setDateFormat("yyyy-MM-dd'T'HH:mm:ss.SSSZ");
                                Gson gson = gsonBuilder.create();
                                String jsonEvent = gson.toJson(event);
                                params.put("event", jsonEvent);
                                params.put("idToken", idToken);
                                PostJsonObjectRequest postEventRegisterRequest = new PostJsonObjectRequest(getPostEventRegisterCallback(), AppConfig.getInstance().postEvent());
                                RequestQueueSingleton.getInstance(activity).addToRequestQueue(postEventRegisterRequest.getRequest(params));
                            } else {
                                logout();
                                //Log if error
                                //noinspection ThrowableResultOfMethodCallIgnored,ThrowableResultOfMethodCallIgnored
                                Log.e(TAG, "error on Get Token:" + (task.getException() == null ? "null exception" : task.getException().getMessage()));
                            }


                        }
                    });
        }
    }

    /**
     * Logout method. Return to the first activity.
     */
    private void logout() {
        FirebaseAuth.getInstance().signOut();
        Intent intent = new Intent(activity, LoginAndBLETabsActivity.class);
        activity.startActivity(intent);
        activity.finish();
    }

    private OnPostJsonObjectCallback getPostEventRegisterCallback() {
        return new OnPostJsonObjectCallback() {
            @Override
            public void onPostJsonObjectCallbackSuccess(JsonObject jsonObject) {
                //If the event is sucefully registered inform it to user and finish this activity
                ActivityUtils.showToast(activity, "Evento cadastrado com sucesso.");
                ActivityUtils.cancelProgressDialog();
                finish();
            }

            @Override
            public void onPostJsonObjectCallbackError(VolleyError volleyError) {
                //If error, show to user and return to caller activity
                String myMessage = "Erro ao cadastrar o evento. Tente novamente.";
                ActivityUtils.showToast(activity, VolleyErrorUtils.decideRightMessage(volleyError, myMessage, activity, false));
                finish();
            }
        };
    }

    /**
     * Called when a date is chosed
     *
     * @param cubee          Cubee form the task that had his data changed
     * @param dateFromPicker Chosed date
     * @param position       Position of the task in the TaskList
     */
    @Override
    public void onDatePicked(Cubee cubee, Date dateFromPicker, int position) {
        //Pseudocode:
        //IF IS VALID
        //  CHANGE Date
        //  FOR EVERY TASK TO THE END
        //      CHANGE Date
        //ELSE
        //  ERROR PROMPT

        Date date = new Date();
        date.setTime(taskList.get(position).getDateTask().getTime());
        date.setYear(dateFromPicker.getYear());
        date.setMonth(dateFromPicker.getMonth());
        date.setDate(dateFromPicker.getDate());


        //IF VALID
        if (position == 0 || atPositionIsBeforeOrEqualsDate(position - 1, date)) {
            //CHANGE Date
            Date dateToEdit = taskList.get(position).getDateTask();
            dateToEdit.setYear(date.getYear());
            dateToEdit.setMonth(date.getMonth());
            dateToEdit.setDate(date.getDate());
            taskList.get(position).setDateTask(dateToEdit);

            Date afterDate;
            //FOR EVERY TASK TO THE END
            for (int i = position + 1; i < taskList.size(); i++) {
                //CHANGE Date
                afterDate = taskList.get(i).getDateTask();
                if (afterDate.before(dateToEdit)) {
                    afterDate.setYear(date.getYear());
                    afterDate.setMonth(date.getMonth());
                    afterDate.setDate(date.getDate());
                    taskList.get(i).setDateTask(afterDate);

                }
            }

            //update view
            eventAdapter.notifyDataSetChanged();

        } else {
            //Error prompt if its not a valida date
            ActivityUtils.alertDialogSimple(this, "Data inserida inválida.", "A data dever ser posterior à data de ativação do cubee anterior");
        }

    }

    @Override
    public void onHourPicked(Cubee cubee, Date dateFromPicker, int position) {
        //Pseudocode:
        //IF IS VALID
        //  CHANGE Hour
        //  FOR EVERY TASK TO THE END
        //      CHANGE Hour
        //ELSE
        //  ERROR PROMPT

        Date date = new Date();
        date.setTime(taskList.get(position).getDateTask().getTime());
        date.setHours(dateFromPicker.getHours());
        date.setMinutes(dateFromPicker.getMinutes());

        //IF IS VALID
        if (position == 0 || atPositionIsBeforeOrEqualsDate(position - 1, date)) {
            //CHANGE Hour
            Date dateToEdit = taskList.get(position).getDateTask();
            dateToEdit.setHours(date.getHours());
            dateToEdit.setMinutes(date.getMinutes());
            taskList.get(position).setDateTask(dateToEdit);
            Date afterDate;
            //  FOR EVERY TASK TO THE END
            for (int i = position; i < taskList.size(); i++) {
                //      CHANGE Hour
                afterDate = taskList.get(i).getDateTask();
                if (afterDate.before(dateToEdit)) {
                    afterDate.setHours(date.getHours());
                    afterDate.setMinutes(date.getMinutes());
                    taskList.get(i).setDateTask(afterDate);
                }
            }
            //update view
            eventAdapter.notifyDataSetChanged();

        } else {
            //ELSE
            //  ERROR PROMPT
            ActivityUtils.alertDialogSimple(this, "Hora inserida inválida.", "A data e hora desta tarefa deve ser maior ou igual a data e hora da tarefa anterior.");
        }
    }

    /**
     * Validade if a date on the tasklist is before a given date.
     * Used so you cant put a task to occur before a previous task
     *
     * @param position Position in the taskList
     * @param date     date to verify
     * @return Return true if the date at the position passed is equals or before a date passed
     */
    private boolean atPositionIsBeforeOrEqualsDate(int position, Date date) {
        Boolean isBeforeorEquals = false;
        if (position > -1) {
            isBeforeorEquals = taskList.get(position).getDateTask().equals(date) || taskList.get(position).getDateTask().before(date);
        }
        return isBeforeorEquals;
    }

    /**
     * Update the task command at a position to a given command
     *
     * @param position Position in the taskList
     * @param command  Command to update
     */
    @Override
    public void changeOnOffCommand(int position, int command) {
        taskList.get(position).setAppcommand(command);


    }
}

