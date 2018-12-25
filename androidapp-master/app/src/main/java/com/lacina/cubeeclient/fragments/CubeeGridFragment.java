package com.lacina.cubeeclient.fragments;


import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.graphics.Color;
import android.os.Bundle;
import android.os.Handler;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentTransaction;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.GridView;
import android.widget.ImageButton;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.android.volley.VolleyError;
import com.facebook.login.LoginManager;
import com.google.android.gms.tasks.OnCompleteListener;
import com.google.android.gms.tasks.Task;
import com.google.firebase.auth.FirebaseAuth;
import com.google.firebase.auth.FirebaseUser;
import com.google.firebase.auth.GetTokenResult;
import com.google.gson.Gson;
import com.google.gson.JsonArray;
import com.google.gson.JsonObject;
import com.lacina.cubeeclient.R;
import com.lacina.cubeeclient.activities.LoginAndBLETabsActivity;
import com.lacina.cubeeclient.activities.MainActivity;
import com.lacina.cubeeclient.activities.NewRegisterEventActivity;
import com.lacina.cubeeclient.activities.NewRegisterRuleActivity;
import com.lacina.cubeeclient.activities.SelectAreasActivity;
import com.lacina.cubeeclient.adapters.CubeeGridAdapter;
import com.lacina.cubeeclient.controllers.CubeeController;
import com.lacina.cubeeclient.model.Cubee;
import com.lacina.cubeeclient.model.User;
import com.lacina.cubeeclient.serverConnection.AppConfig;
import com.lacina.cubeeclient.serverConnection.RequestQueueSingleton;
import com.lacina.cubeeclient.serverConnection.callbacks.OnGetJsonArrayCallback;
import com.lacina.cubeeclient.serverConnection.callbacks.OnPostJsonObjectCallback;
import com.lacina.cubeeclient.serverConnection.callbacks.old.OnGetUserInfoCallback;
import com.lacina.cubeeclient.serverConnection.requests.GetJsonArrayRequest;
import com.lacina.cubeeclient.serverConnection.requests.PostJsonObjectRequest;
import com.lacina.cubeeclient.serverConnection.requests.old.GetUserInfoRequest;
import com.lacina.cubeeclient.utils.ActivityUtils;
import com.lacina.cubeeclient.utils.VolleyErrorUtils;
import com.lacina.cubeeclient.utils.ConnectionUtils;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;

import butterknife.BindView;
import butterknife.ButterKnife;

/**
 * Fragment for fill the central of the main activity at the beggining.
 * Shows all Cubees in a list fragmentView and control the interaction this them.
 */

@SuppressWarnings({"ALL", "JavaDoc"})
public class CubeeGridFragment extends Fragment {
    /**
     * Tag used to auxiliate the application log
     */
    private static final String TAG = "HomeFrag";

    /**
     * Signal button view
     */
    @SuppressWarnings("unused")
    @BindView(R.id.img_btn_signal)
    public ImageButton ibtmSignal;

    /**
     * Event button view
     */
    @SuppressWarnings("unused")
    @BindView(R.id.img_btn_event)
    public ImageButton ibtmEvent;

    /**
     * Rule button view
     */
    @SuppressWarnings("unused")
    @BindView(R.id.img_btn_rule)
    public ImageButton ibtmRule;

    /**
     * Db9 button view
     */
    @SuppressWarnings({"WeakerAccess", "unused"})
    @BindView(R.id.img_btn_db9)
    public ImageButton ibtmDb9;

    @SuppressWarnings("unused")
    @BindView(R.id.btm_register_event_or_rules)
    public Button btnRegisterCubee;

    /**
     * Cancel Register Event button view
     */
    @SuppressWarnings("unused")
    @BindView(R.id.btm_cancel_register_event_or_rules)
    public Button cancelRegisterCubee;

    /**
     * alert text view
     * Show a alert if there is not any cubee in this sector or group
     */
    @SuppressWarnings("unused")
    @BindView(R.id.tv_alert_text)
    public TextView tv_alert_text;

    /**
     * Right buttons view
     * Add rule, event and send signal
     */
    @SuppressWarnings("unused")
    @BindView(R.id.ln_rigth_command_bar)
    public LinearLayout ln_rigth_command_bar;

    /**
     * buttons to confirm or cancel a new event or rule
     */
    @SuppressWarnings("unused")
    @BindView(R.id.ln_buttons_fragment_home)
    public LinearLayout lnButonsFragmentHome;

    /**
     * List fragmentView to show all the user CUBEEs
     */
    @SuppressWarnings("unused")
    @BindView(R.id.gridviewcubees)
    public GridView gvCubeesList;

    /**
     * Field to represents this activity
     */
    private Activity activity;

    /**
     * Id the current sector or group
     */
    private String cubeeSectorId;

    /**
     * List of selected cubees
     */
    private ArrayList<Cubee> listCubeeSelected;

    /**
     * Callback for get cubee request callback
     * If sucess add all cubees to the list
     */
    private boolean firstTimeRequest = true;

    /**
     * Token who represents a firebaseUser used to validation
     */
    private String userToken;

    /**
     * Field to represents this applicationContext
     */
    private Context context;

    /**
     * Handler used to loop a runnable, so its possible to receive a new signal information
     */
    private final Handler handler = new Handler();

    /**
     * Runnable to set a behaviour to do in loop
     */
    private Runnable runnable;

    /**
     * Cubee Controller for cubee related operations:
     */
    private CubeeController cubeeController;

    /**
     * Firebase user profile information object.
     * Contain helper methods to o change or retrieve profile information,
     * as well as to manage that user's authentication state.
     */
    private FirebaseUser firebaseUser;

    /**
     * Adapter of the cubee grid
     */
    private CubeeGridAdapter adapterhomeCubeeGrid;

    /**
     * Selected command in right bar
     */
    private int selectedComand;

    //Construtors and singleton method----------------------------------------
    @SuppressWarnings("unused")
    public CubeeGridFragment() {
        // Required empty public constructor
        Log.d("Home", "new frag");
    }


    //Activity methods --------------------------------------------------------

    public static CubeeGridFragment newInstance(String sectorId) {
        Bundle args = new Bundle();
        args.putString("sectorIdToSet", sectorId);
        CubeeGridFragment fragment = new CubeeGridFragment();
        fragment.setArguments(args);

        Log.d(TAG, "Sector id = " + sectorId);
        return fragment;
    }

    /**
     * Set the fields
     *
     * @param savedInstanceState
     */
    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        firebaseUser = FirebaseAuth.getInstance().getCurrentUser();
        activity = getActivity();
        this.context = activity.getApplicationContext();
        this.cubeeController = CubeeController.getInstance();
        runnable = new Runnable() {
            @Override
            public void run() {
                Log.d(TAG, "Run");
                if (selectedComand != 2 && selectedComand != 1) {
                    setCubeeListView();
                }

            }
        };
    }

    /**
     * Bind the fragmentView
     *
     * @param inflater
     * @param container
     * @param savedInstanceState
     * @return
     */
    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        /*

     */
        View fragmentView = inflater.inflate(R.layout.fragment_home, container, false);

        ButterKnife.bind(this, fragmentView);

        cubeeSectorId = getArguments().getString("sectorIdToSet");

        initRegisterEventsOrRulesButtons();
        initRightCommandBarButtons(fragmentView);
        return fragmentView;
    }

    @Override
    public void onResume() {
        super.onResume();
        if (listCubeeSelected != null) {
            listCubeeSelected.clear();
        }

        cubeeSectorId = getArguments().getString("sectorIdToSet");
        if (cubeeSectorId != null) {
            ActivityUtils.showProgressDialog(activity, "carregando.");
            setCubeeListView();
        }
        CubeeController.getInstance().setCubeeAtual(null);
    }

    @Override
    public void onPause() {
        super.onPause();
        ActivityUtils.cancelProgressDialog();

        handler.removeCallbacks(runnable);
    }

    // Buttons ----------------------------------------------------------------------

    private void initRegisterEventsOrRulesButtons() {

        btnRegisterCubee.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (!listCubeeSelected.isEmpty()) {
                    if (selectedComand == 1) {
                        if(listCubeeSelected.size() > 1){
                            Intent intent = new Intent(activity, NewRegisterRuleActivity.class);
                            intent.putExtra("ListOfCubees", listCubeeSelected);
                            startActivity(intent);
                        }else if(listCubeeSelected.size() == 1){
                            ActivityUtils.showToast(activity, "Por favor, selecione pelo menos dois CUBEEs.");
                        }
                    }
                    if (selectedComand == 2) {
                        Intent intent = new Intent(activity, NewRegisterEventActivity.class);
                        intent.putExtra("ListOfCubees", listCubeeSelected);
                        startActivity(intent);
                    }
                    resetRightCommandBar();
                } else {
                    ActivityUtils.showToast(activity, "Por favor, selecione pelo menos um CUBEE.");
                }

            }
        });


        cancelRegisterCubee.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                lnButonsFragmentHome.setVisibility(View.GONE);
                resetRightCommandBar();
                setCubeeListView();
            }
        });

    }

    private void initRightCommandBarButtons(View view) {


        ibtmRule.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (selectedComand != 1) {
                    ActivityUtils.showToastLongNoSuperiorDaTela(activity, "Selecione CUBEEs para registrar uma regra.");
                    //noinspection Convert2Diamond
                    listCubeeSelected = new ArrayList<Cubee>();
                    resetRightCommandBar();
                    selectedComand = 1;
                    ibtmRule.setBackgroundResource(R.mipmap.btn_rule_selected);
                    getLnButonsFragmentHome().setVisibility(View.VISIBLE);
                } else {
                    setCubeeListView();
                    resetRightCommandBar();
                    getLnButonsFragmentHome().setVisibility(View.GONE);
                }
            }
        });


        ibtmEvent.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {

                if (selectedComand != 2) {
                    ActivityUtils.showToastLongNoSuperiorDaTela(activity, "Selecione CUBEEs para registrar um Evento.");
                    //noinspection Convert2Diamond
                    listCubeeSelected = new ArrayList<Cubee>();
                    resetRightCommandBar();
                    selectedComand = 2;
                    ibtmEvent.setBackgroundResource(R.mipmap.btn_event_selected);
                    getLnButonsFragmentHome().setVisibility(View.VISIBLE);
                } else {
                    setCubeeListView();
                    resetRightCommandBar();
                    getLnButonsFragmentHome().setVisibility(View.GONE);
                }
            }
        });


        ibtmSignal.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                //setCubeeListView();
                if (selectedComand != 3) {
                    ActivityUtils.showToastLongNoSuperiorDaTela(activity, "Selecione um CUBEE para enviar um sinal.");
                    resetRightCommandBar();
                    ibtmSignal.setBackgroundResource(R.mipmap.btn_signal_selected);
                    selectedComand = 3;
                } else {
                    resetRightCommandBar();
                }
            }
        });

        ibtmDb9.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (selectedComand != 4) {
                    ActivityUtils.showToastLongNoSuperiorDaTela(activity, "Selecione um CUBEE para enviar uma regra DB9");
                    //noinspection Convert2Diamond
                    listCubeeSelected = new ArrayList<Cubee>();
                    resetRightCommandBar();
                    selectedComand = 4;
                    ibtmDb9.setBackgroundColor(Color.GRAY);
                } else {
                    setCubeeListView();
                    resetRightCommandBar();
                    getLnButonsFragmentHome().setVisibility(View.GONE);
                }
            }
        });


    }

    @SuppressWarnings("SameParameterValue")
    private void sendRequestCommandCubee(final Integer command, Cubee cubee) {
        Map<String, String> params = new HashMap<>();
        params.put("idCubee", cubee.get_id());
        params.put("idToken", userToken);
        params.put("command", Integer.toString(command));
        PostJsonObjectRequest postCommandCubee = new PostJsonObjectRequest(onPostCubeeCommandCallback(), AppConfig.getInstance().setCommandCubee());
        RequestQueueSingleton.getInstance(activity).addToRequestQueue(postCommandCubee.getRequest(params));
    }

    private OnPostJsonObjectCallback onPostCubeeCommandCallback() {
        return new OnPostJsonObjectCallback() {
            @Override
            public void onPostJsonObjectCallbackSuccess(JsonObject jsonObject) {

            }

            @Override
            public void onPostJsonObjectCallbackError(VolleyError volleyError) {
                String myMessage = "Erro ao enviar commando ao CUBEE. ";
                ActivityUtils.showToast(activity, VolleyErrorUtils.decideRightMessage(volleyError, myMessage, activity, true));
            }


        };
    }

    //Set GriView ------------------------------------------------------------------------
    private void resetRightCommandBar() {
        ibtmRule.setBackgroundResource(R.mipmap.btn_rule_unselected);
        ibtmEvent.setBackgroundResource(R.mipmap.btn_event_unselected);
        ibtmSignal.setBackgroundResource(R.mipmap.btn_signal_unselected);
        ibtmDb9.setBackgroundColor(Color.TRANSPARENT);
        selectedComand = 0;
        adapterhomeCubeeGrid = new CubeeGridAdapter(activity, CubeeController.getInstance().getCubeeList());
        gvCubeesList.setAdapter(adapterhomeCubeeGrid);

        getLnButonsFragmentHome().setVisibility(View.GONE);
        setCubeeListView();


    }

    /**
     * Setup list with all cubees from the service.
     * CALLBACK: {@link #getCubeesListCallback()}
     */
    private void setCubeeListView() {
        firebaseUser = FirebaseAuth.getInstance().getCurrentUser();
        if (firebaseUser != null && getActivity() != null) {
            firebaseUser.getIdToken(true)
                    .addOnCompleteListener(new OnCompleteListener<GetTokenResult>() {
                        public void onComplete(@NonNull Task<GetTokenResult> task) {
                            if (task.isSuccessful()) {
                                userToken = task.getResult().getToken();
                                checkUser(userToken);
                                Map<String, String> headers = new HashMap<>();
                                headers.put("idToken", userToken);
                                if (cubeeSectorId.equals("all_cubees")) {
                                    GetJsonArrayRequest cubeeList = new GetJsonArrayRequest(getCubeesListCallback(), AppConfig.getInstance().getCubees());
                                    RequestQueueSingleton.getInstance(activity).addToRequestQueue(cubeeList.getRequest(headers));
                                } else {
                                    if (!cubeeSectorId.equals("no_sector")) {
                                        headers.put("idSector", cubeeSectorId);
                                    }
                                    GetJsonArrayRequest noSectorCubeeList = new GetJsonArrayRequest(getCubeesListCallback(), AppConfig.getInstance().getCubeesBySector());
                                    RequestQueueSingleton.getInstance(activity).addToRequestQueue(noSectorCubeeList.getRequest(headers));
                                }


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

    private OnGetJsonArrayCallback getCubeesListCallback() {
        return new OnGetJsonArrayCallback() {
            @Override
            public void onGetJsonArrayCallbackSuccess(JsonArray jsonArray) {
                Gson gson = new Gson();
                ArrayList cubeeListResponse = new ArrayList();
                for (int i = 0; i < jsonArray.size(); i++) {
                    //noinspection unchecked
                    cubeeListResponse.add(gson.fromJson(jsonArray.get(i), Cubee.class));
                }

                ActivityUtils.cancelProgressDialog();


                if (cubeeListResponse.isEmpty()) {
                    tv_alert_text.setVisibility(View.VISIBLE);
                    ln_rigth_command_bar.setVisibility(View.GONE);
                } else {
                    ln_rigth_command_bar.setVisibility(View.VISIBLE);
                    tv_alert_text.setVisibility(View.GONE);
                }


                if (selectedComand != 2 && selectedComand != 1) {
                    if (cubeeListResponse.isEmpty()) {
                        handler.removeCallbacks(runnable);
                        if (firstTimeRequest) {
                            ActivityUtils.showToast(activity, "Voce não tem cubees neste Grupo ou Setor");
                            firstTimeRequest = false;
                        }

                    } else {
                        //noinspection unchecked
                        CubeeController.getInstance().setCubeeList(cubeeListResponse);
                        //noinspection unchecked
                        cubeeController.setCubeeList(cubeeListResponse);
                        adapterhomeCubeeGrid = new CubeeGridAdapter(activity, CubeeController.getInstance().getCubeeList());
                        gvCubeesList.setAdapter(adapterhomeCubeeGrid);
                        gvCubeesList.setOnItemClickListener(onItemClickListener());
                    }
                }

                handler.postDelayed(runnable, 6000);
            }

            @Override
            public void onGetJsonArrayCallbackError(VolleyError volleyError) {
                handler.removeCallbacks(runnable);
                ActivityUtils.cancelProgressDialog();
            }

        };
    }

    //Grid View click listeners --------------------------------------------------------------

    /**
     * Click listener for each item at Cubee List
     */
    private AdapterView.OnItemClickListener onItemClickListener() {
        return new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                if (selectedComand == 1) {
                    Cubee cubeeSelected = (Cubee) parent.getItemAtPosition(position);
                    if (listCubeeSelected.contains(cubeeSelected)) {
                        listCubeeSelected.remove(cubeeSelected);
                    } else {
                        listCubeeSelected.add(cubeeSelected);
                    }

                    adapterhomeCubeeGrid.setListSelectedCubees(listCubeeSelected);
                    adapterhomeCubeeGrid.notifyDataSetChanged();


                } else if (selectedComand == 2) {

                    Cubee cubeeSelected = (Cubee) parent.getItemAtPosition(position);
                    if (listCubeeSelected.contains(cubeeSelected)) {
                        listCubeeSelected.remove(cubeeSelected);
                    } else {
                        listCubeeSelected.add(cubeeSelected);
                    }

                    adapterhomeCubeeGrid.setListSelectedCubees(listCubeeSelected);
                    adapterhomeCubeeGrid.notifyDataSetChanged();


                } else if (selectedComand == 3) {
                    sendRequestCommandCubee(3, (Cubee) adapterhomeCubeeGrid.getItem(position));
                    ActivityUtils.showToast(context, "Sinal enviado");
                } else if (selectedComand == 4){
                    Cubee cubeeSelected = (Cubee) parent.getItemAtPosition(position);

                    Intent intent = new Intent(activity, SelectAreasActivity.class);
                    intent.putExtra("cubeeSelected", cubeeSelected);
                    startActivity(intent);

                    adapterhomeCubeeGrid.notifyDataSetChanged();
//                    resetRightCommandBar();
                } else {
                    Cubee cubeeSelected = (Cubee) parent.getItemAtPosition(position);
                    CubeeController.getInstance().setCubeeAtual(cubeeSelected);
                    Fragment cubeeInformationFragment = (Fragment) new CubeeInformationFragment();
                    changeCenterFragment(cubeeInformationFragment);
                }


            }
        };
    }

    private void changeCenterFragment(Fragment fragment) {
        AppCompatActivity appCompatActivity = (AppCompatActivity) activity;
        FragmentTransaction transaction = appCompatActivity.getSupportFragmentManager().beginTransaction();
        if(fragment instanceof CubeeInformationFragment){
            transaction.replace(R.id.fl_fragment_center, fragment, "CUBEE_INFORMATION_FRAGMENT");
        }else{
            transaction.replace(R.id.fl_fragment_center, fragment);
        }
        try {
            transaction.commit();
        } catch (IllegalStateException ignored) {
            // TODO
        }
    }


    // Check User Login Methods -----------------------------------------------------
    private void checkUser(String token) {
        Map<String, String> headers = new HashMap<>();
        headers.put("idToken", token);
        GetUserInfoRequest getUserInfo = new GetUserInfoRequest(onGetUserInfoCallback(), AppConfig.getInstance().getUser());
        RequestQueueSingleton.getInstance(activity).addToRequestQueue(getUserInfo.getRequest(headers));

    }

    private OnGetUserInfoCallback onGetUserInfoCallback() {
        return new OnGetUserInfoCallback() {
            @Override
            public void onGetUserInfoCallbackSuccess(User user) {

            }

            @Override
            public void onGetUserInfoCallbackError(String message) {
                ActivityUtils.showToast(activity, "Erro de autenticação");
                ConnectionUtils.logout(activity);
            }
        };
    }

    /**
     * Logout form the aplication and go back to {@link MainActivity}
     */
    private void logout() {
        LoginManager loginManager = LoginManager.getInstance();

        if (loginManager != null) {
            LoginManager.getInstance().logOut();
        }

        FirebaseAuth.getInstance().signOut();

        Intent intent = new Intent(activity, LoginAndBLETabsActivity.class);
        activity.startActivity(intent);
        activity.finish();
    }

    //Getters and setters ----------------------------------------------
    private LinearLayout getLnButonsFragmentHome() {
        return lnButonsFragmentHome;
    }



}

