package com.lacina.cubeeclient.fragments;


import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentTransaction;
import android.text.TextUtils;
import android.util.Log;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AutoCompleteTextView;
import android.widget.Button;

import com.android.volley.VolleyError;
import com.google.android.gms.tasks.OnCompleteListener;
import com.google.android.gms.tasks.Task;
import com.google.firebase.auth.FirebaseAuth;
import com.google.firebase.auth.FirebaseUser;
import com.google.firebase.auth.GetTokenResult;
import com.google.gson.JsonObject;
import com.lacina.cubeeclient.R;
import com.lacina.cubeeclient.serverConnection.AppConfig;
import com.lacina.cubeeclient.serverConnection.RequestQueueSingleton;
import com.lacina.cubeeclient.serverConnection.callbacks.OnPostJsonObjectCallback;
import com.lacina.cubeeclient.serverConnection.requests.PostJsonObjectRequest;
import com.lacina.cubeeclient.utils.ActivityUtils;
import com.lacina.cubeeclient.utils.VolleyErrorUtils;

import java.util.HashMap;
import java.util.Map;

import butterknife.BindView;
import butterknife.ButterKnife;

/**
 * Fragment for register cubee, get id, name and send request to server.
 **/
@SuppressWarnings({"ALL", "JavaDoc"})
public class RegisterCubeeFragment extends Fragment {

    /**
     * Edit text view to input the CUBEE name
     */
    @SuppressWarnings("unused")
    @BindView(R.id.edt_cubee_name)
    public AutoCompleteTextView edtNameCubee;

    /**
     * Edit text view to input the CUBEE id
     */
    @SuppressWarnings("unused")
    @BindView(R.id.id_cubee)
    public AutoCompleteTextView edtIdCubee;

    /**
     * Button view to create a CUBEE
     */
    @SuppressWarnings("unused")
    @BindView(R.id.btn_create_cubee)
    public Button btnCreateCubee;

    /**
     * Firebase user profile information object.
     * Contain helper methods to o change or retrieve profile information,
     * as well as to manage that user's authentication state.
     */
    private FirebaseUser firebaseUser;

    @SuppressWarnings("unused")
    private Button btnFindCubeeBtl;

    /**
     * Field to reference this activity
     */
    private Activity activity;

    /**
     * Object to gerenciate the fragments
     */
    private FragmentManager fragmentManager;

    /**
     * Token who represents a firebaseUser used to validation
     */
    private String userToken;

    /**
     * Tag to auxiliate the application log
     */
    private final String TAG = "RegisterCubeeFrag";

    @SuppressWarnings("unused")
    public RegisterCubeeFragment() {
        // Required empty public constructor
    }

    @SuppressWarnings("unused")
    public static RegisterCubeeFragment newInstance() {
        return new RegisterCubeeFragment();
    }

    /**
     * Bind the view and set the fields
     *
     * @param inflater
     * @param container
     * @param savedInstanceState
     * @return View
     */
    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        // Inflate the layout for this fragment
        View fragmentView = inflater.inflate(R.layout.fragment_register_cubee, container, false);
        ButterKnife.bind(this, fragmentView);
        activity = getActivity();
        this.fragmentManager = getFragmentManager();

        firebaseUser = FirebaseAuth.getInstance().getCurrentUser();
        activity = getActivity();
        firebaseUser.getIdToken(true)
                .addOnCompleteListener(new OnCompleteListener<GetTokenResult>() {
                    public void onComplete(@NonNull Task<GetTokenResult> task) {
                        if (task.isSuccessful()) {
                            userToken = task.getResult().getToken();
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


        fragmentView.setFocusableInTouchMode(true);
        fragmentView.requestFocus();
        fragmentView.setOnKeyListener(new View.OnKeyListener() {
            @Override
            public boolean onKey(View v, int keyCode, KeyEvent event) {
                if (keyCode == KeyEvent.KEYCODE_BACK) {
                    Log.i(TAG, "onKey Back listener is working!!!");
                    FragmentTransaction ft = getFragmentManager().beginTransaction();
                    ft.replace(R.id.fl_fragment_center, CubeeGridFragment.newInstance(null));
                    ft.commit();
                    return true;
                } else {
                    return false;
                }
            }
        });

        btnCreateCubee.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                attemptRegisterCubee();
            }
        });


        return fragmentView;
    }

    /**
     * Validate form and try to send Register Cubee Request
     **/
    private void attemptRegisterCubee() {
        // Reset errors.
        edtNameCubee.setError(null);
        edtIdCubee.setError(null);

        // Store values at the time of the login attempt.
        String name = edtNameCubee.getText().toString().trim();
        String id = edtIdCubee.getText().toString().trim();

        boolean cancel = false;
        View focusView = null;


        // Check for a valid email address.
        if (TextUtils.isEmpty(name)) {
            edtNameCubee.setError(getString(R.string.error_field_required));
            focusView = edtNameCubee;
            cancel = true;
        }

        // Check for a valid name.
        if (TextUtils.isEmpty(id)) {
            edtIdCubee.setError(getString(R.string.error_field_required));
            focusView = edtIdCubee;
            cancel = true;
        }

        if (cancel) {
            focusView.requestFocus();
        } else {
            sendRegisterCubee(id, name);
        }
    }

    /**
     * Send register request to server.
     **/
    private void sendRegisterCubee(String id, String name) {
        Map<String, String> params = new HashMap<>();
        params.put("cubeeId", id);
        params.put("cubeeName", name);
        params.put("token", userToken);
        PostJsonObjectRequest postCubeeRegister = new PostJsonObjectRequest(onPostCubeeRegisterCallback(), AppConfig.getInstance().registerCubee());
        RequestQueueSingleton.getInstance(activity).addToRequestQueue(postCubeeRegister.getRequest(params));
    }

    /**
     * Callbacks to the request.
     * If Sucess: |Goes back to home
     * If Error: Show message to user.
     **/
    private OnPostJsonObjectCallback onPostCubeeRegisterCallback() {
        return new OnPostJsonObjectCallback() {
            @Override
            public void onPostJsonObjectCallbackSuccess(JsonObject jsonObject) {
                ActivityUtils.cancelProgressDialog();
                FragmentTransaction fragmentTransaction = fragmentManager.beginTransaction();
                fragmentTransaction.replace(R.id.fl_fragment_center, CubeeGridFragment.newInstance(null));
//              fragmentTransaction.addToBackStack(null);
                fragmentTransaction.commit();
            }

            @Override
            public void onPostJsonObjectCallbackError(VolleyError volleyError) {
                ActivityUtils.cancelProgressDialog();
                String myMessage = "Erro ao cadastrar Cubee. ";
                ActivityUtils.showToast(activity, VolleyErrorUtils.decideRightMessage(volleyError, myMessage, activity, false));

            }

        };
    }
}
