package com.lacina.cubeeclient.fragments;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.v4.app.Fragment;
import android.text.TextUtils;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.ListView;

import com.android.volley.VolleyError;
import com.facebook.login.LoginManager;
import com.google.android.gms.tasks.OnCompleteListener;
import com.google.android.gms.tasks.Task;
import com.google.firebase.auth.FirebaseAuth;
import com.google.firebase.auth.FirebaseUser;
import com.google.firebase.auth.GetTokenResult;
import com.google.gson.Gson;
import com.google.gson.JsonObject;
import com.lacina.cubeeclient.R;
import com.lacina.cubeeclient.activities.LoginAndBLETabsActivity;
import com.lacina.cubeeclient.adapters.SelectCubeeListAdapter;
import com.lacina.cubeeclient.interfaces.OnSectorAdded;
import com.lacina.cubeeclient.model.Cubee;
import com.lacina.cubeeclient.model.User;
import com.lacina.cubeeclient.serverConnection.AppConfig;
import com.lacina.cubeeclient.serverConnection.RequestQueueSingleton;
import com.lacina.cubeeclient.serverConnection.callbacks.OnPostJsonObjectCallback;
import com.lacina.cubeeclient.serverConnection.callbacks.old.OnGetCubeesListBySectorCallback;
import com.lacina.cubeeclient.serverConnection.callbacks.old.OnGetUserInfoCallback;
import com.lacina.cubeeclient.serverConnection.requests.PostJsonObjectRequest;
import com.lacina.cubeeclient.serverConnection.requests.old.GetCubeesListBySectorRequest;
import com.lacina.cubeeclient.serverConnection.requests.old.GetUserInfoRequest;
import com.lacina.cubeeclient.utils.ActivityUtils;
import com.lacina.cubeeclient.utils.ConnectionUtils;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import butterknife.BindView;
import butterknife.ButterKnife;


@SuppressWarnings("ALL")
public class RegisterSectorFragment extends Fragment {

    private static final String TAG = "RegisterSectorFrag";

    @SuppressWarnings("unused")
    @BindView(R.id.ed_name_sector)
    public EditText nameSector;

    @SuppressWarnings("unused")
    @BindView(R.id.btn_ok_sector)
    public Button btnOK;

    @SuppressWarnings("unused")
    @BindView(R.id.lv_cubees_sector)
    public ListView lvCubeesToAdd;

    /**
     * Field to reference this activity
     */
    private Activity activity;

    /**
     * Firebase user profile information object.
     * Contain helper methods to o change or retrieve profile information,
     * as well as to manage that user's authentication state.
     */
    private FirebaseUser firebaseUser;

    private String userToken;

    private SelectCubeeListAdapter addCubeeToSectorRegistAdapter;

    @SuppressWarnings("CanBeFinal")
    private List<Cubee> cubeesToAddToSectorAtRegister = new ArrayList<Cubee>();

    private Context context;


    @SuppressWarnings("unused")
    public RegisterSectorFragment() {
        // Required empty public constructor
    }

    public static RegisterSectorFragment newInstance() {
        RegisterSectorFragment fragment = new RegisterSectorFragment();
        Bundle args = new Bundle();
        fragment.setArguments(args);
        return fragment;
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        firebaseUser = FirebaseAuth.getInstance().getCurrentUser();

    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        View fragmentView = inflater.inflate(R.layout.fragment_register_sector, container, false);
        activity = getActivity();
        ButterKnife.bind(this, fragmentView);
        context = getActivity();
        btnOK.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                attemptRegisterSector();
            }
        });

        // Inflate the layout for this fragment
        return fragmentView;
    }

    private void attemptRegisterSector() {
        // Reset errors.
        nameSector.setError(null);

        // Store values at the time of the login attempt.
        String name = nameSector.getText().toString().trim();

        boolean cancel = false;
        View focusView = null;


        // Check for a valid email address.
        if (TextUtils.isEmpty(name)) {
            nameSector.setError(getString(R.string.error_field_required));
            focusView = nameSector;
            cancel = true;
        }


        if (cancel) {
            focusView.requestFocus();
        } else {
            sendRegisterSector(name);
        }
    }

    private void sendRegisterSector(String sectorName) {
        @SuppressWarnings("Convert2Diamond") List<String> listIdCubeeToRegisterInASector = new ArrayList<String>();
        for (Cubee cubee : cubeesToAddToSectorAtRegister) {
            listIdCubeeToRegisterInASector.add(cubee.get_id());
        }

        final String jsonArrayCubeesToRegisterInASector = new Gson().toJson(listIdCubeeToRegisterInASector);
        Map<String, String> params = new HashMap<>();
        params.put("userToken", userToken);
        params.put("sectorName", sectorName);
        if (listIdCubeeToRegisterInASector.size() > 0) {
            params.put("listIdCubees", jsonArrayCubeesToRegisterInASector);
        }

        PostJsonObjectRequest postSectorsRequest = new PostJsonObjectRequest(onPostSectorRegisterRequest(), AppConfig.getInstance().postSector());
        RequestQueueSingleton.getInstance(context).addToRequestQueue(postSectorsRequest.getRequest(params));
        ActivityUtils.showProgressDialog(activity, "Cadastrando Setor " + sectorName);
    }

    private OnPostJsonObjectCallback onPostSectorRegisterRequest() {
        return new OnPostJsonObjectCallback() {
            @Override
            public void onPostJsonObjectCallbackSuccess(JsonObject jsonObject) {
                try {
                    ((OnSectorAdded) activity).onSectorAdded();
                } catch (ClassCastException cce) {
                    cce.printStackTrace();
                }
                ActivityUtils.cancelProgressDialog();
                ActivityUtils.showToast(context, "Setor cadastrado com sucesso");

            }

            @Override
            public void onPostJsonObjectCallbackError(VolleyError volleyError) {
                ActivityUtils.cancelProgressDialog();
                ActivityUtils.showToast(activity, "Erro ao cadastrar setor" +
                        "");
            }

        };
    }


    @Override
    public void onResume() {
        super.onResume();
        setListView();
    }

    private void setListView() {
        if (firebaseUser != null) {
            firebaseUser.getIdToken(true)
                    .addOnCompleteListener(new OnCompleteListener<GetTokenResult>() {
                        public void onComplete(@NonNull Task<GetTokenResult> task) {
                            if (task.isSuccessful()) {
                                userToken = task.getResult().getToken();
                                checkUser(userToken);
                                Map<String, String> headers = new HashMap<>();
                                headers.put("idToken", userToken);
                                GetCubeesListBySectorRequest getUserInfo = new GetCubeesListBySectorRequest(getCubeesListCallback(), AppConfig.getInstance().getCubeesBySector());
                                RequestQueueSingleton.getInstance(activity).addToRequestQueue(getUserInfo.getRequest(headers));

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

    private void checkUser(String userToken) {
        Map<String, String> headers = new HashMap<>();
        headers.put("idToken", userToken);
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

    private OnGetCubeesListBySectorCallback getCubeesListCallback() {
        return new OnGetCubeesListBySectorCallback() {
            @Override
            public void onGetCubeesListCallbackSucess(ArrayList<Cubee> response) {
                ActivityUtils.cancelProgressDialog();
                if (response.isEmpty()) {
                    ActivityUtils.showToast(activity, "Não existem cubees para serem adicionados ao setor");


                } else {
                    addCubeeToSectorRegistAdapter = new SelectCubeeListAdapter(activity, response);
                    lvCubeesToAdd.setAdapter(addCubeeToSectorRegistAdapter);
                    lvCubeesToAdd.setOnItemClickListener(onItemClickListener());
                }


            }

            @Override
            public void onGetCubeesListCallbackError(String error) {
                ActivityUtils.cancelProgressDialog();
            }
        };
    }

    private AdapterView.OnItemClickListener onItemClickListener() {
        return new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                Cubee cubeeSelected = (Cubee) parent.getItemAtPosition(position);
                ImageView imgCubee = (ImageView) view.findViewById(R.id.cubee_img);

                if (!cubeesToAddToSectorAtRegister.contains(cubeeSelected)) {
                    imgCubee.setBackgroundResource(R.mipmap.img_cubee_signal_off);
                    cubeesToAddToSectorAtRegister.add(cubeeSelected);

                } else {
                    imgCubee.setBackgroundResource(R.mipmap.cubee_simple);
                    cubeesToAddToSectorAtRegister.remove(cubeeSelected);
                }

                addCubeeToSectorRegistAdapter.setItemSelected(cubeesToAddToSectorAtRegister);

                Log.d(TAG, "" + cubeeSelected);


            }
        };
    }
}



