package com.lacina.cubeeclient.activities;

import android.app.Activity;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Build;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.v7.app.AlertDialog;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ListView;
import android.widget.TextView;

import com.android.volley.VolleyError;
import com.google.android.gms.tasks.OnCompleteListener;
import com.google.firebase.auth.FirebaseAuth;
import com.google.firebase.auth.FirebaseUser;
import com.google.firebase.auth.GetTokenResult;
import com.google.gson.Gson;
import com.google.gson.JsonArray;
import com.google.gson.JsonObject;
import com.google.gson.reflect.TypeToken;
import com.lacina.cubeeclient.R;
import com.lacina.cubeeclient.adapters.Db9ListAdapter;
import com.lacina.cubeeclient.fragments.LoginTabFragment;
import com.lacina.cubeeclient.model.Cubee;
import com.lacina.cubeeclient.model.Db9Rule;
import com.lacina.cubeeclient.serverConnection.AppConfig;
import com.lacina.cubeeclient.serverConnection.RequestQueueSingleton;
import com.lacina.cubeeclient.serverConnection.callbacks.OnGetJsonArrayCallback;
import com.lacina.cubeeclient.serverConnection.callbacks.OnPostJsonObjectCallback;
import com.lacina.cubeeclient.serverConnection.requests.GetJsonArrayRequest;
import com.lacina.cubeeclient.serverConnection.requests.PostJsonObjectRequest;
import com.lacina.cubeeclient.utils.ActivityUtils;
import com.lacina.cubeeclient.utils.VolleyErrorUtils;

import java.lang.reflect.Type;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import butterknife.BindView;
import butterknife.ButterKnife;

/**
 * Activity to list DB9Rules
 * Shows all rules to cubee.
 */
@SuppressWarnings("ALL")
public class ListDb9RulesActivity extends AppCompatActivity {

    private final String TAG = "DB9_LIST";

    /**
     * List that will have DB9 Rules.
     */
    @SuppressWarnings({"WeakerAccess", "CanBeFinal", "unused"})
    @BindView(R.id.lv_db9)
    public ListView lvRules;

    /**
     * TextView with a message if there is no DB9 Rules.
     */
    @SuppressWarnings({"WeakerAccess", "CanBeFinal", "unused"})
    @BindView(R.id.tv_msg_no_db9)
    public TextView tvMsgNoEvents;

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

    /**
     * idToken used in request to autenticate with backend
     */
    private String idToken;

    /**
     * Adapter to DB9 Rules list
     */
    private Db9ListAdapter ruleListAdapter;

    /**
     * Current cubee selected
     */
    private Cubee cubeeSelected;

    /**
     * Builder to create dialogs
     */
    private AlertDialog.Builder builder;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_list_db9_rules);

        activity = this;
        ButterKnife.bind(this);
        firebaseUser = FirebaseAuth.getInstance().getCurrentUser();

        //Get cubee from intent, caller activity has to send this list.
        Bundle extras = getIntent().getExtras();
        if (extras != null) {
            cubeeSelected = (Cubee) extras.get("cubeeSelected");
        }

        setupDialogs();
    }

    @Override
    protected void onResume() {
        init();
        super.onResume();
    }

    /**
     * Instantiate AlertDialog Builder.
     */
    private void setupDialogs() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            builder = new AlertDialog.Builder(activity, android.R.style.Theme_Material_Dialog_Alert);
        } else {
            builder = new AlertDialog.Builder(activity);
        }
    }

    /**
     * Do firebase authentication and get all user's DB9 rules.
     * <p>
     * Callback: {@link #getRulesCallback()} ()}
     */
    private void init() {
        firebaseUser = FirebaseAuth.getInstance().getCurrentUser();
        if (firebaseUser != null) {
            firebaseUser.getIdToken(true)
                    .addOnCompleteListener(new OnCompleteListener<GetTokenResult>() {
                        public void onComplete(@NonNull com.google.android.gms.tasks.Task<GetTokenResult> task) {
                            if (task.isSuccessful()) {
                                idToken = task.getResult().getToken();
                                Map<String, String> headers = new HashMap<>();
                                headers.put("idToken", idToken);
                                GetJsonArrayRequest getRules = new GetJsonArrayRequest(getRulesCallback(), AppConfig.getInstance().getDB9ByUser());
                                RequestQueueSingleton.getInstance(activity).addToRequestQueue(getRules.getRequest(headers));

                            } else {
                                //Log Erro
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

    /**
     * Callback from getting all user's DB9 rules.
     * Here, the adapter and it's clicks listeners are also setted.
     *
     * @return OnGetJsonArrayCallback
     */
    private OnGetJsonArrayCallback getRulesCallback() {
        return new OnGetJsonArrayCallback() {
            @Override
            public void onGetJsonArrayCallbackSuccess(JsonArray jsonArray) {
                Gson gson = new Gson();
                Type type = new TypeToken<List<Db9Rule>>() {
                }.getType();
                ArrayList<Db9Rule> ruleList = gson.fromJson(jsonArray, type);
                ruleListAdapter = new Db9ListAdapter(activity, ruleList);
                if (ruleList.size() == 0) {
                    tvMsgNoEvents.setVisibility(View.VISIBLE);
                } else {
                    tvMsgNoEvents.setVisibility(View.GONE);
                }

                lvRules.setAdapter(ruleListAdapter);
                lvRules.setOnItemClickListener(new AdapterView.OnItemClickListener() {
                    @Override
                    public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                        try {
                            showAddRuleDialog((Db9Rule) lvRules.getItemAtPosition(position));

                        } catch (ClassCastException cce) {
                            cce.printStackTrace();
                        }
                    }
                });
            }

            @Override
            public void onGetJsonArrayCallbackError(VolleyError volleyError) {
                Log.d(TAG, "Generic" + VolleyErrorUtils.getGenericMessage(volleyError, activity));
                Log.d(TAG, "Server" + VolleyErrorUtils.getMessageFromServer(volleyError));
                ActivityUtils.showToast(activity, "Um Erro desconhecido ocorreu. Por favor, tente novamente.");
            }
        };

    }

    /**
     * Show add rule dialog
     *
     * @param rule A DB9 Rule to be added to a Cubee.
     */
    private void showAddRuleDialog(final Db9Rule rule) {
        builder.setTitle("Adicionar regra")
                .setMessage("Deseja associar a regra '" + rule.getName() + "' ao Cubee '"
                        + cubeeSelected.getName() + "'?")
                .setPositiveButton(android.R.string.yes, new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int which) {
                        addRuleToCubee(rule);
                    }
                })
                .setNegativeButton(android.R.string.no, new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int which) {
                        // do nothing
                    }
                })
                .setIcon(android.R.drawable.ic_dialog_alert)
                .show();
    }

    /**
     * Add a DB9 Rule to a Cubee.
     *
     * @param rule A DB9 Rule to be added to a Cubee.
     *             Callback: {@link #onSetDB9Rule()}
     */
    private void addRuleToCubee(Db9Rule rule) {
        Map<String, String> params = new HashMap<>();

        params.put("ruleId", rule.get_id());
        params.put("idToken", idToken);
        params.put("cubeeId", cubeeSelected.get_id());

        PostJsonObjectRequest postJsonObjectRequest = new PostJsonObjectRequest(onSetDB9Rule(), AppConfig.getInstance().setDB9ToCubee());
        RequestQueueSingleton.getInstance(activity).addToRequestQueue(postJsonObjectRequest.getRequest(params));
    }

    /**
     * Callback of adding a DB9 Rule to a Cubee.
     *
     * @return OnPostJsonObjectCallback
     */
    private OnPostJsonObjectCallback onSetDB9Rule() {
        return new OnPostJsonObjectCallback() {
            @Override
            public void onPostJsonObjectCallbackSuccess(JsonObject jsonObject) {
                ActivityUtils.showToast(activity, "Regra cadastrada com sucesso.");
                ActivityUtils.cancelProgressDialog();
                finish();
            }

            @Override
            public void onPostJsonObjectCallbackError(VolleyError volleyError) {
                String myMessage = "Erro ao cadastrar a regra. ";
                ActivityUtils.showToast(activity, VolleyErrorUtils.decideRightMessage(volleyError, myMessage, activity, false));
            }
        };
    }

}
