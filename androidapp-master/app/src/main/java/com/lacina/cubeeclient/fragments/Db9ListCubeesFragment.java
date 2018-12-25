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
import com.lacina.cubeeclient.activities.MainActivity;
import com.lacina.cubeeclient.adapters.CubeeDb9ListAdapter;
import com.lacina.cubeeclient.interfaces.ChangeCenterFragmentInterface;
import com.lacina.cubeeclient.interfaces.OnActivateDeactivateDB9Rule;
import com.lacina.cubeeclient.model.Cubee;
import com.lacina.cubeeclient.model.Db9Rule;
import com.lacina.cubeeclient.serverConnection.AppConfig;
import com.lacina.cubeeclient.serverConnection.RequestQueueSingleton;
import com.lacina.cubeeclient.serverConnection.callbacks.OnGetJsonArrayCallback;
import com.lacina.cubeeclient.serverConnection.callbacks.OnGetJsonObjectCallback;
import com.lacina.cubeeclient.serverConnection.callbacks.OnPostJsonObjectCallback;
import com.lacina.cubeeclient.serverConnection.requests.GetJsonArrayRequest;
import com.lacina.cubeeclient.serverConnection.requests.GetJsonObjectRequest;
import com.lacina.cubeeclient.serverConnection.requests.PostJsonObjectRequest;
import com.lacina.cubeeclient.utils.ActivityUtils;
import com.lacina.cubeeclient.utils.VolleyErrorUtils;

import java.lang.reflect.Type;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Timer;
import java.util.TimerTask;

import butterknife.BindView;
import butterknife.ButterKnife;

/**
 * List Cubees associated with a DB9 Rule
 */
@SuppressWarnings("ALL")
public class Db9ListCubeesFragment extends BackableFragment implements OnActivateDeactivateDB9Rule {

    private final String TAG = "RulesListCubee";

    /**
     * List of cubees associateds to a rule
     */
    @SuppressWarnings({"WeakerAccess", "unused"})
    @BindView(R.id.lv_cubees_db9_list)
    public ListView lvCubees;


    /**
     * Message if there is no cubee associated to a rule
     */
    @SuppressWarnings({"WeakerAccess", "unused"})
    @BindView(R.id.no_db9_tv)
    public TextView tvMsgNoRules;

    /**
     * Current selected rule.
     */
    private static Db9Rule currentRule;

    /**
     * Interface to callback the informations.
     */
    private OnActivateDeactivateDB9Rule onActivateDeactivateDB9RuleImplementation;

    /**
     * Field to reference this activity
     */
    private Activity activity;

    /**
     * Flag to identify if it's necessary to change the command.
     */
    private Boolean changeCommandFlag;

    /**
     * Firebase user profile information object.
     * Contain helper methods to o change or retrieve profile information,
     * as well as to manage that user's authentication state.
     */
    private FirebaseUser firebaseUser;

    /**
     * idToken of firebase to authenticate with server.
     */
    private String idToken;

    /**
     * Adapter to cubee list.
     */
    private CubeeDb9ListAdapter cubeeListAdapter;

    private View fragmentView;

    /**
     * Current selected cubee.
     */
    private Cubee cubeeAtual;

    /**
     * Timer while state of cubee is changing.
     */
    private int contTime = 0;

    private Timer timer;

    @SuppressWarnings("unused")
    public Db9ListCubeesFragment() {
        // Required empty public constructor
    }

    public static Db9ListCubeesFragment newInstance(Db9Rule rule) {

        Bundle args = new Bundle();
        currentRule = rule;
        Db9ListCubeesFragment fragment = new Db9ListCubeesFragment();
        fragment.setArguments(args);
        return fragment;
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        onActivateDeactivateDB9RuleImplementation = this;

    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        // Inflate the layout for this fragment
        fragmentView = inflater.inflate(R.layout.fragment_db9_list_cubees, container, false);
        ButterKnife.bind(this, fragmentView);
        activity = getActivity();
        firebaseUser = FirebaseAuth.getInstance().getCurrentUser();
        init();
        return fragmentView;
    }

    /**
     * Do firebase authentication and get all user's DB9 rules.
     * <p>
     * Callback: {@link #getCubeesCallback()} ()} ()}
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
                                headers.put("ruleId", currentRule.get_id());
                                headers.put("idToken", idToken);
                                GetJsonArrayRequest getRules = new GetJsonArrayRequest(getCubeesCallback(), AppConfig.getInstance().getCubeesByDB9());
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
    private OnGetJsonArrayCallback getCubeesCallback() {
        return new OnGetJsonArrayCallback() {
            @Override
            public void onGetJsonArrayCallbackSuccess(JsonArray jsonArray) {
                Gson gson = new Gson();
                Type type = new TypeToken<List<Cubee>>() {
                }.getType();
                ArrayList<Cubee> cubeeList = gson.fromJson(jsonArray, type);
                cubeeListAdapter = new CubeeDb9ListAdapter(activity, cubeeList, onActivateDeactivateDB9RuleImplementation);
                if (cubeeList.size() == 0) {
                    tvMsgNoRules.setVisibility(View.VISIBLE);
                } else {
                    tvMsgNoRules.setVisibility(View.GONE);
                }

                lvCubees.setAdapter(cubeeListAdapter);
            }

            @Override
            public void onGetJsonArrayCallbackError(VolleyError volleyError) {
                String myMessage = "Um Erro ao recuperar CUBEEs ocorreu. Por favor, tente novamente.";
                ActivityUtils.showToast(activity, VolleyErrorUtils.decideRightMessage(volleyError, myMessage, activity, true));
            }
        };

    }

    @Override
    public void onBackButtonPressed() {
        if (activity instanceof ChangeCenterFragmentInterface) {
            ((ChangeCenterFragmentInterface) activity).changeCenterFragment(new Db9ListFragment());
        }
    }

    /**
     * Active DB9 Rule in a cubee
     * @param cubee Current cubee
     */
    @Override
    public void activateDB9Rule(Cubee cubee) {
        contTime = 0;
        cubeeAtual = cubee;
        changeCommandFlag = true;
        initLoop();
        ActivityUtils.showProgressDialog(activity, "Ativando CUBEE");
        sendRequestCommandCubee(1);

    }

    private void sendRequestCommandCubee(int command) {
        Map<String, String> params = new HashMap<>();
        params.put("idCubee", cubeeAtual.get_id());
        params.put("idToken", idToken);
        params.put("command", Integer.toString(command));
        PostJsonObjectRequest postCubeeActivate = new PostJsonObjectRequest(onPostCubeeCommandCallback(), AppConfig.getInstance().setCommandCubee());
        RequestQueueSingleton.getInstance(activity).addToRequestQueue(postCubeeActivate.getRequest(params));
    }

    /**
     * Callbacks to the request.
     * If Sucess: Change the CUBEE command and show a message to user
     * If Error:  Show message to user.
     **/
    private OnPostJsonObjectCallback onPostCubeeCommandCallback() {
        return new OnPostJsonObjectCallback() {
            @Override
            public void onPostJsonObjectCallbackSuccess(JsonObject jsonObject) {

            }

            @Override
            public void onPostJsonObjectCallbackError(VolleyError volleyError) {
                String myMessage = "Erro ao enviar comando para o CUBEE.";
                ActivityUtils.showToast(activity, VolleyErrorUtils.decideRightMessage(volleyError, myMessage, activity, false));

            }

        };
    }

    /**
     * Loop while trying to change cubee state
     */
    private void initLoop() {
        if (timer != null) {
            timer.cancel();
        }
        timer = new Timer();
        // This timer task will be executed every 1 sec.
        timer.schedule(new TimerTask() {
            @Override
            public void run() {
                if (idToken != null) {
                    refreshActualCubee();
                }
                if (changeCommandFlag) {
                    contTime++;
                    if (contTime == 10) {
                        ActivityUtils.cancelProgressDialog();
                        ActivityUtils.showToastOutActivity(activity, "Erro ao mudar estado do CUBEE");
                    }
                }

            }
        }, 0, 3000);
    }

    private void refreshActualCubee() {
        Map<String, String> headers = new HashMap<>();
        headers.put("idToken", idToken);
        headers.put("idCubee", cubeeAtual.get_id());
        GetJsonObjectRequest getCubeeByIdRequest = new GetJsonObjectRequest(onGetCubeeByIdCallback(), AppConfig.getInstance().getCubeeById());
        RequestQueueSingleton.getInstance(activity).addToRequestQueue(getCubeeByIdRequest.getRequest(headers));

    }

    private OnGetJsonObjectCallback onGetCubeeByIdCallback() {
        return new OnGetJsonObjectCallback() {
            @Override
            public void onGetJsonObjectCallbackSuccess(JsonObject jsonObject) {
                Gson gson = new Gson();
                Cubee cubee = gson.fromJson(jsonObject, Cubee.class);
                if (!cubeeAtual.getCubeeState().equals(cubee.getCubeeState())) {
                    cubeeAtual = cubee;
                    if (cubeeAtual.getCubeeState()) {
                        ActivityUtils.showToast(activity, "CUBEE ativado com sucesso.");
                    } else {
                        ActivityUtils.showToast(activity, "CUBEE desativado com sucesso.");
                    }
                    init();
                    ActivityUtils.cancelProgressDialog();
                    contTime = 0;
                    timer.cancel();
                }
            }

            @Override
            public void onGetJsonObjectCallbackError(VolleyError volleyError) {
                ActivityUtils.cancelProgressDialog();
                String myMessage = "Erro ao acessar CUBEE.";
                ActivityUtils.showToast(activity, VolleyErrorUtils.decideRightMessage(volleyError, myMessage, activity, false));
                MainActivity mainActivity = (MainActivity) activity;
                mainActivity.backToCurrentSectorOrGrupe();
            }
        };
    }

    /**
     * Deactive DB9 rule in a cubee.
     * @param cubee
     */
    @Override
    public void deactivateDB9Rule(Cubee cubee) {
        contTime = 0;
        cubeeAtual = cubee;
        changeCommandFlag = true;
        ActivityUtils.showProgressDialog(activity, "Desativando CUBEE");
        initLoop();
        sendRequestCommandCubee(2);

    }
}
