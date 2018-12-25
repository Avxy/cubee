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
import com.google.gson.JsonObject;
import com.lacina.cubeeclient.R;
import com.lacina.cubeeclient.adapters.RuleTaskEditListAdapter;
import com.lacina.cubeeclient.interfaces.OnOffChange;
import com.lacina.cubeeclient.interfaces.OnTriggerChange;
import com.lacina.cubeeclient.model.Cubee;
import com.lacina.cubeeclient.model.RuleTask;
import com.lacina.cubeeclient.serverConnection.AppConfig;
import com.lacina.cubeeclient.serverConnection.RequestQueueSingleton;
import com.lacina.cubeeclient.serverConnection.callbacks.OnPostJsonObjectCallback;
import com.lacina.cubeeclient.serverConnection.requests.PostJsonObjectRequest;
import com.lacina.cubeeclient.utils.ActivityUtils;

import com.lacina.cubeeclient.model.Rules;
import com.lacina.cubeeclient.utils.VolleyErrorUtils;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import butterknife.BindView;
import butterknife.ButterKnife;


@SuppressWarnings("ALL")
public class NewRegisterRuleActivity extends AppCompatActivity implements OnOffChange, OnTriggerChange {

    private static final String TAG = "RegisterRuleA";

    /**
     * Rules list view reference
     */
    @SuppressWarnings("unused")
    @BindView(R.id.lv_cubees_for_rule)
    public ListView lvRuleList;

    /**
     * Cancel button view reference
     */
    @SuppressWarnings("unused")
    @BindView(R.id.btn_cancel)
    public Button btnCancel;

    /**
     * Register Rule Button view reference
     */
    @SuppressWarnings("unused")
    @BindView(R.id.btn_register_rule)
    public Button btnRegisterRule;

    /**
     * Edit Text Rule View Reference
     */
    @SuppressWarnings("unused")
    @BindView(R.id.edt_rule_name)
    public EditText edtRuleName;

    /**
     * Cubee list that come from the caller activity, passed via Intent.
     */
    private ArrayList<Cubee> cubeeList;

    /**
     * Adapter to control the list view.
     */
    private RuleTaskEditListAdapter rulesTaskAdapter;

    /**
     * List of tasks, each Ruletask has a cubee, typeAlert and command
     * see {@link RuleTask}
     */
    private List<RuleTask> ruleTaskList;

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
        setContentView(R.layout.activity_register_rule);
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
        ruleTaskList = new ArrayList<RuleTask>();
        for (int i = 0; i < cubeeList.size(); i++) {
            String previousIdCubee = null;
            Integer taskCommand = null;
            if(i != 0){
                previousIdCubee = cubeeList.get(i-1).get_id();
                taskCommand = 1;
            }
            RuleTask ruleTask = new RuleTask(null, cubeeList.get(i).get_id(), cubeeList.get(i).getName(), previousIdCubee,null, taskCommand);
            ruleTaskList.add(ruleTask);
        }

        //Setup view with build task list
        rulesTaskAdapter = new RuleTaskEditListAdapter(this, cubeeList, ruleTaskList, this, this);
        lvRuleList.setAdapter(rulesTaskAdapter);
        rulesTaskAdapter.notifyDataSetChanged();
        initButtons();
    }

    /**
     * OnClickListeners of buttons
     */
    private void initButtons() {
        btnCancel.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                finish();
            }
        });
        btnRegisterRule.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                attemptRegisterRule();
            }
        });

    }

    /**
     * Called when the submit button is pressed
     * Validate if the Rule name is not empty
     */
    private void attemptRegisterRule() {

        edtRuleName.setError(null);

        // Store values at the time of the login attempt.
        String name = edtRuleName.getText().toString().trim();

        boolean cancel = false;
        View focusView = null;


        // Check for a valid email address.
        if (TextUtils.isEmpty(name)) {
            edtRuleName.setError(getString(R.string.error_field_required));
            focusView = edtRuleName;
            cancel = true;
        }


        if (cancel) {
            focusView.requestFocus();
        } else {
            ActivityUtils.showProgressDialog(activity, "Cadastrando regra");
            createRulesAndSendToServer(name);
        }
    }

    /**
     * Requet to send rule to server
     * Callback: {@link #onPostRuleCallback()} ()}
     *
     * @param name name of the rule
     */
    private void createRulesAndSendToServer(final String name) {
        if (firebaseUser != null) {
            firebaseUser.getIdToken(true)
                    .addOnCompleteListener(new OnCompleteListener<GetTokenResult>() {
                        public void onComplete(@NonNull com.google.android.gms.tasks.Task<GetTokenResult> task) {
                            if (task.isSuccessful()) {
                                Rules rules = new Rules(null, name, firebaseUser.getUid(), ruleTaskList);
                                String idToken = task.getResult().getToken();
                                Map<String, String> params = new HashMap<>();

                                Gson gson = new Gson();
                                String jsonEvent = gson.toJson(rules);

                                params.put("jsonRule", jsonEvent);
                                params.put("idToken", idToken);

                                PostJsonObjectRequest postJsonObjectRequest = new PostJsonObjectRequest(onPostRuleCallback(), AppConfig.getInstance().registerRule());
                                RequestQueueSingleton.getInstance(activity).addToRequestQueue(postJsonObjectRequest.getRequest(params));
                            } else {
                                logout();
                                //noinspection ThrowableResultOfMethodCallIgnored,ThrowableResultOfMethodCallIgnored
                                Log.e(TAG, "error on Get Token:" + (task.getException() == null ? "null exception" : task.getException().getMessage()));
                            }


                        }
                    });
        }
    }

    private OnPostJsonObjectCallback onPostRuleCallback() {
        return new OnPostJsonObjectCallback() {
            @Override
            public void onPostJsonObjectCallbackSuccess(JsonObject jsonObject) {
                ActivityUtils.showToast(activity, "Regra cadastrada com sucesso.");
                ActivityUtils.cancelProgressDialog();
                finish();

            }

            @Override
            public void onPostJsonObjectCallbackError(VolleyError volleyError) {
                String myMessage = "Erro ao cadastrar a regra.";
                ActivityUtils.showToast(activity, VolleyErrorUtils.decideRightMessage(volleyError, myMessage, activity, false));

            }
        };
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

    /**
     * Called when user change on off option in any position
     * @param position position in {@link #ruleTaskList} of the rule task to change command
     * @param command the command to change
     */
    @Override
    public void changeOnOffCommand(int position, int command) {
        ruleTaskList.get(position).setTaskCommand(command);
    }

    /**
     * Called when user change trigger option in any position
     * @param position position in {@link #ruleTaskList} of the rule task to change trigger
     * @param trigger ther trigger to change
     */
    @Override
    public void onTriggerChanged(int position, String trigger) {
        ruleTaskList.get(position).setTypeAlert(trigger);
    }
}

