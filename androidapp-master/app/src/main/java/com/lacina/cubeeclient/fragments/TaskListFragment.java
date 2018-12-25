package com.lacina.cubeeclient.fragments;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ListView;

import com.android.volley.VolleyError;
import com.google.android.gms.tasks.OnCompleteListener;
import com.google.firebase.auth.FirebaseAuth;
import com.google.firebase.auth.FirebaseUser;
import com.google.firebase.auth.GetTokenResult;
import com.google.gson.Gson;
import com.google.gson.JsonArray;
import com.lacina.cubeeclient.R;
import com.lacina.cubeeclient.adapters.TaskShowListAdapter;
import com.lacina.cubeeclient.interfaces.ChangeCenterFragmentInterface;
import com.lacina.cubeeclient.model.Task;
import com.lacina.cubeeclient.serverConnection.AppConfig;
import com.lacina.cubeeclient.serverConnection.RequestQueueSingleton;
import com.lacina.cubeeclient.serverConnection.callbacks.OnGetJsonArrayCallback;
import com.lacina.cubeeclient.serverConnection.requests.GetJsonArrayRequest;
import com.lacina.cubeeclient.utils.ActivityUtils;
import com.lacina.cubeeclient.utils.VolleyErrorUtils;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;

import butterknife.BindView;
import butterknife.ButterKnife;

import static com.facebook.GraphRequest.TAG;

/**
 * Created by matheus on 19/07/17.
 */

@SuppressWarnings({"ALL", "DefaultFileTemplate"})
public class TaskListFragment extends BackableFragment {


    @SuppressWarnings("unused")
    @BindView(R.id.lv_event_and_rules)
    public ListView lvEvent;

    /**
     * Field to reference this activity
     */
    private Activity activity;

    private String idEvent;

    /**
     * idToken used in request to autenticate with backend
     */
    private String idToken;

    /**
     * Firebase user profile information object.
     * Contain helper methods to o change or retrieve profile information,
     * as well as to manage that user's authentication state.
     */
    private FirebaseUser firebaseUser;

    @SuppressWarnings("unused")
    public TaskListFragment() {
        // Required empty public constructor
    }


    public static TaskListFragment newInstance(String id) {
        Bundle args = new Bundle();
        args.putString("idEvent", id);
        TaskListFragment fragment = new TaskListFragment();
        fragment.setArguments(args);
        return fragment;
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        // Inflate the layout for this fragment
        View fragmentView = inflater.inflate(R.layout.fragment_task_list, container, false);
        ButterKnife.bind(this, fragmentView);
        activity = getActivity();
        firebaseUser = FirebaseAuth.getInstance().getCurrentUser();
        idEvent = getArguments().getString("idEvent");
        init();
        return fragmentView;
    }

    private void init() {
        firebaseUser = FirebaseAuth.getInstance().getCurrentUser();
        if (firebaseUser != null) {
            firebaseUser.getIdToken(true)
                    .addOnCompleteListener(new OnCompleteListener<GetTokenResult>() {
                        public void onComplete(@NonNull com.google.android.gms.tasks.Task<GetTokenResult> task) {
                            if (task.isSuccessful()) {
                                idToken = task.getResult().getToken();
                                Map<String, String> params = new HashMap<>();
                                params.put("idToken", idToken);
                                params.put("idEvent", idEvent);
                                GetJsonArrayRequest getTaskByEventRequest = new GetJsonArrayRequest(getTaskByEventCallback(), AppConfig.getInstance().getTaskByEvent());
                                RequestQueueSingleton.getInstance(activity).addToRequestQueue(getTaskByEventRequest.getRequest(params));
                            } else {
                                //noinspection ThrowableResultOfMethodCallIgnored,ThrowableResultOfMethodCallIgnored
                                Log.e(TAG, "Error Get token" + (task.getException() == null ? "null exception" : task.getException().getMessage()));
                                FirebaseAuth.getInstance().signOut();
                                Intent intent = new Intent(activity, LoginTabFragment.class);
                                startActivity(intent);
                                activity.finish();
                            }
                        }
                    });
        }


    }

    private OnGetJsonArrayCallback getTaskByEventCallback() {
        return new OnGetJsonArrayCallback() {
            @Override
            public void onGetJsonArrayCallbackSuccess(JsonArray jsonArray) {
                Gson gson = new Gson();
                ArrayList taskList = new ArrayList();
                for (int i = 0; i < jsonArray.size(); i++) {
                    //noinspection unchecked
                    taskList.add(gson.fromJson(jsonArray.get(i), Task.class));
                }
                //noinspection unchecked
                lvEvent.setAdapter(new TaskShowListAdapter(activity, taskList));

            }

            @Override
            public void onGetJsonArrayCallbackError(VolleyError volleyError) {
                String myMessage = "Erro ao recuperar lista de tarefas. ";
                ActivityUtils.showToast(activity, VolleyErrorUtils.decideRightMessage(volleyError, myMessage, activity, false));
            }

        };
    }

    @Override
    public void onBackButtonPressed() {
        if (activity instanceof ChangeCenterFragmentInterface) {
            ((ChangeCenterFragmentInterface) activity).changeCenterFragmentGridFragmentToSelected();
        }
    }
}
