package com.lacina.cubeeclient.fragments;


import android.app.Activity;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
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
import android.widget.EditText;
import android.widget.ImageView;

import com.android.volley.VolleyError;
import com.google.android.gms.tasks.OnCompleteListener;
import com.google.android.gms.tasks.Task;
import com.google.firebase.auth.FirebaseAuth;
import com.google.firebase.auth.FirebaseUser;
import com.google.firebase.auth.GetTokenResult;
import com.google.gson.JsonObject;
import com.lacina.cubeeclient.R;
import com.lacina.cubeeclient.activities.MainActivity;
import com.lacina.cubeeclient.interfaces.ChangeCenterFragmentInterface;
import com.lacina.cubeeclient.model.User;
import com.lacina.cubeeclient.serverConnection.AppConfig;
import com.lacina.cubeeclient.serverConnection.RequestQueueSingleton;
import com.lacina.cubeeclient.serverConnection.callbacks.OnPostJsonObjectCallback;
import com.lacina.cubeeclient.serverConnection.callbacks.old.OnGetUserInfoCallback;
import com.lacina.cubeeclient.serverConnection.requests.PostJsonObjectRequest;
import com.lacina.cubeeclient.serverConnection.requests.old.GetUserInfoRequest;
import com.lacina.cubeeclient.utils.ActivityUtils;
import com.lacina.cubeeclient.utils.VolleyErrorUtils;

import java.util.HashMap;
import java.util.Map;

import butterknife.BindView;
import butterknife.ButterKnife;

/**
 * Fragment to edit user profile. Validate a form and send it to server.
 **/
@SuppressWarnings("ALL")
public class EditProfileFragment extends BackableFragment {

    /**
     * Buttons to edit user's profile informations.
     */

    @SuppressWarnings("unused")
    @BindView(R.id.name_register)
    public AutoCompleteTextView edNameView;

    @SuppressWarnings("unused")
    @BindView(R.id.actv_email_register)
    public AutoCompleteTextView edEmailView;

    @SuppressWarnings("unused")
    @BindView(R.id.ed_password_register)
    public EditText edPasswordView;

    @SuppressWarnings("unused")
    @BindView(R.id.number_register)
    public EditText edNumberView;

    @SuppressWarnings("unused")
    @BindView(R.id.cubee_icon)
    public ImageView imgUser;

    @SuppressWarnings("unused")
    @BindView(R.id.confirm_password_register)
    public EditText edConfirmPasswordView;

    @SuppressWarnings("unused")
    @BindView(R.id.save_button)
    public Button btmSave;

    private FirebaseUser mUser;

    /**
     * Field to reference this activity
     */
    private Activity activity;

    private View fragmentView;

    private FragmentManager fragmentManager;

    /**
     * idToken used in request to autenticate with backend
     */
    private String idToken;

    private final String TAG = "EditProfileFrag";

    //Fragment Methods ----------------------------------------------------

    @SuppressWarnings("unused")
    public EditProfileFragment() {
        // Required empty public constructor
    }

    public static EditProfileFragment newInstance() {
        return new EditProfileFragment();
    }

    /**
     * Method called at launching.
     * <p>
     * Recovery user id and user profile info at the very beginning.
     **/
    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mUser = FirebaseAuth.getInstance().getCurrentUser();
        activity = getActivity();
        mUser.getIdToken(true)
                .addOnCompleteListener(new OnCompleteListener<GetTokenResult>() {
                    public void onComplete(@NonNull Task<GetTokenResult> task) {
                        if (task.isSuccessful()) {
                            idToken = task.getResult().getToken();
                            Map<String, String> headers = new HashMap<>();
                            headers.put("idToken", idToken);
                            GetUserInfoRequest getUserInfo = new GetUserInfoRequest(getUserInfoCallback(), AppConfig.getInstance().getUser());
                            RequestQueueSingleton.getInstance(activity).addToRequestQueue(getUserInfo.getRequest(headers));
                        } else {
                            //Log de erro
                            //noinspection ThrowableResultOfMethodCallIgnored,ThrowableResultOfMethodCallIgnored
                            Log.e(TAG, "Error Get token" + (task.getException() == null ? "null exception" : task.getException().getMessage()));
                            FirebaseAuth.getInstance().signOut();
                            Intent intent = new Intent(activity, LoginTabFragment.class);
                            startActivity(intent);
                            activity.finish();
                        }
                    }
                });

        this.fragmentManager = getFragmentManager();


    }


    /**
     * Callback for the get info callback
     **/
    private OnGetUserInfoCallback getUserInfoCallback() {
        return new OnGetUserInfoCallback() {
            @Override
            public void onGetUserInfoCallbackSuccess(User user) {
                if (user != null) {
                    edNameView.setText(user.getName());
                    edEmailView.setText(user.getEmail());
                    edNumberView.setText(user.getTelephone());
                }
            }

            @Override
            public void onGetUserInfoCallbackError(String message) {

            }
        };
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        // Inflate the layout for this fragment
        fragmentView = inflater.inflate(R.layout.fragment_edit_profile, container, false);
        activity = getActivity();
        ButterKnife.bind(this, fragmentView);

        createClickLinesters();

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


        return fragmentView;

    }

    @Override
    public void onBackButtonPressed() {
        if (activity instanceof ChangeCenterFragmentInterface) {
            ((ChangeCenterFragmentInterface) activity).changeCenterFragmentGridFragmentToSelected();
        }
    }

    private void createClickLinesters() {
        btmSave.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                attemptEdit();
            }
        });
    }

    //Requests------------------------------------------------------------------

    /**
     * Validate form and try to send request to server
     **/
    private void attemptEdit() {

        // Reset errors.
        edEmailView.setError(null);
        edPasswordView.setError(null);
        edNameView.setError(null);
        edNameView.setError(null);
        edConfirmPasswordView.setHint(null);

        // Store values at the time of the login attempt.
        String email = edEmailView.getText().toString().trim();
        String password = edPasswordView.getText().toString().trim();
        String name = edNameView.getText().toString().trim();
        String number = edNumberView.getText().toString().trim();
        String confirmPassword = edConfirmPasswordView.getText().toString().trim();

        boolean cancel = false;
        View focusView = null;

        // Check for a valid name. Empty
        if (TextUtils.isEmpty(name)) {
            edNameView.setError(getString(R.string.error_invalid_name));
            focusView = edPasswordView;
            cancel = true;
        }
//
//        Check for a valid number
//        if (TextUtils.isEmpty(number)) {
//            edNumberView.setError(getString(R.string.number_invalid));
//            focusView = edNumberView;
//            cancel = true;
//        }

        // Check for a valid email address.
        if (TextUtils.isEmpty(email) && !isEmailValid(email)) {
            edEmailView.setError(getString(R.string.error_invalid_email));
            focusView = edEmailView;
            cancel = true;
        }

//        Check for a valid password, if the user entered one.
        if (!TextUtils.isEmpty(password) && !isPasswordValid(password)) {
            edPasswordView.setError(getString(R.string.error_invalid_password));
            focusView = edPasswordView;
            cancel = true;
        }

        // Check valid email
        if (!password.equals(confirmPassword)) {
            edPasswordView.setError(getString(R.string.error_password_match));
            focusView = edPasswordView;
            cancel = true;
        }

        if (cancel) {
            focusView.requestFocus();
        } else {
            sendEditUser(email, name, password, number);
            ActivityUtils.showProgressDialog(activity, getString(R.string.editing));
        }

    }

    /**
     * Send request to server
     */
    private void sendEditUser(String email, String name, String password, String number) {
        Map<String, String> params = new HashMap<>();
        params.put("idToken", idToken);
        params.put("name", name);
        params.put("email", email);
        params.put("telephone", number);
        params.put("password", password);

        PostJsonObjectRequest postUserEdit = new PostJsonObjectRequest(postUserEditCallbacks(), AppConfig.getInstance().editUser());
        RequestQueueSingleton.getInstance(activity).addToRequestQueue(postUserEdit.getRequest(params));
    }


    /**
     * Register callback for the User Edit Request
     */
    private OnPostJsonObjectCallback postUserEditCallbacks() {
        return new OnPostJsonObjectCallback() {
            @Override
            public void onPostJsonObjectCallbackSuccess(JsonObject jsonObject) {
                ActivityUtils.cancelProgressDialog();
                ActivityUtils.alertDialogSimpleCallback(activity, "Usuário editado com sucesso", "Se você alterou a sua senha você será deslogado.", new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        changeCenterFragment(MainActivity.currentFragment);
                    }
                });
            }

            @Override
            public void onPostJsonObjectCallbackError(VolleyError volleyError) {
                String myMessage = "Erro ao editar cubees. ";
                ActivityUtils.showToast(activity, VolleyErrorUtils.decideRightMessage(volleyError, myMessage, activity, false));
                ActivityUtils.cancelProgressDialog();
            }

        };
    }

    private void changeCenterFragment(Fragment fragment) {
        FragmentTransaction fragmentTransaction = fragmentManager.beginTransaction();
        fragmentTransaction.replace(R.id.fl_fragment_center, fragment);
        //fragmentTransaction.addToBackStack(null);
        fragmentTransaction.commit();

    }

    // Validators ----------------------------------------------------------------
    /**
     * Validates the email
     */
    private boolean isEmailValid(String email) {
        //TODO: Replace this with your own logic
        return email.contains("@");
    }

    /**
     * Validates the password
     */
    private boolean isPasswordValid(String password) {
        //TODO: Replace this with your own logic..
        return password.length() > 5;
    }

}
