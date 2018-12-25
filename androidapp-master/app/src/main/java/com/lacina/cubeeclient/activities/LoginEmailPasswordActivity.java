package com.lacina.cubeeclient.activities;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.v7.app.AppCompatActivity;
import android.text.TextUtils;
import android.util.Log;
import android.view.View;
import android.widget.AutoCompleteTextView;
import android.widget.EditText;

import com.google.android.gms.tasks.OnCompleteListener;
import com.google.android.gms.tasks.Task;
import com.google.firebase.auth.AuthResult;
import com.google.firebase.auth.FirebaseAuth;
import com.google.firebase.auth.FirebaseUser;
import com.lacina.cubeeclient.R;
import com.lacina.cubeeclient.serverConnection.callbacks.old.OnGetCheckConnectionServerCallback;
import com.lacina.cubeeclient.utils.ActivityUtils;
import com.lacina.cubeeclient.utils.ConnectionUtils;

import butterknife.BindView;
import butterknife.ButterKnife;
import butterknife.OnClick;

/**
 * Activity for login with email and password
 * <p>
 * Show when user click on login at Login fragment
 **/
@SuppressWarnings("ALL")
public class LoginEmailPasswordActivity extends AppCompatActivity {

    /**
     * Email input edit text view
     */
    @SuppressWarnings("unused")
    @BindView(R.id.actv_email_register)
    public AutoCompleteTextView edtEmail;

    /**
     * Password inout edit text view
     */
    @SuppressWarnings("unused")
    @BindView(R.id.ed_password_register)
    public EditText edtPasswordView;

    /**
     * SingIn button to attempt login
     */
    @SuppressWarnings("unused")
    @BindView(R.id.login_button_email_password)
    public View mSingUpButton;

    /**
     * Button to go to recovery password activity
     * {@link com.lacina.cubeeclient.activities.RecoveryPasswordActivity}
     */
    @SuppressWarnings("unused")
    @BindView(R.id.btn_recovery)
    public View mRecoveryButton;

    private final String TAG = "LoginEmailPassA";

    /**
     * The entry point of the Firebase Authentication SDK.
     */
    private FirebaseAuth firebaseAuth;

    /**
     * Field to recovery application context
     */
    private Context context;

    /**
     * Field to recovery this activity.
     */
    private Activity activity;

    /**
     * Bind view and set context, activity and firebaseAuth fields.
     *
     * @param savedInstanceState default param, a bundle of saved information.
     */
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_login_email_password);
        context = getApplicationContext();
        activity = this;
        ButterKnife.bind(this);
        firebaseAuth = FirebaseAuth.getInstance();


        //TODO retirar essa linha só para testes
        edtEmail.setText("cubee1@cubee.com");
        edtPasswordView.setText("cubee1");


    }

    /**
     * ON CLICK - Goes to recovery activity.
     * {@link com.lacina.cubeeclient.activities.RecoveryPasswordActivity}
     */
    @SuppressWarnings("unused")
    @OnClick(R.id.btn_recovery)
    public void clickRecoveryBtn() {
        Intent intent = new Intent(getApplicationContext(), RecoveryPasswordActivity.class);
        startActivity(intent);
    }

    /**
     * ON CLICK - Attempt login
     */
    @SuppressWarnings("unused")
    @OnClick(R.id.login_button_email_password)
    public void clickLoginBtn() {
        attemptLogIn();
    }

    /**
     * Validate Login form and try to send request to server.
     **/
    private void attemptLogIn() {
        edtEmail.setError(null);
        edtPasswordView.setError(null);


        String email = edtEmail.getText().toString().trim();
        String password = edtPasswordView.getText().toString().trim();


        boolean cancel = false;
        View focusView = null;

        //Check for valid password
        if (!TextUtils.isEmpty(password) && !isPasswordValid(password)) {
            edtPasswordView.setError(getString(R.string.error_invalid_password));
            focusView = edtPasswordView;
            cancel = true;
        }

        // Check for a valid email address.
        if (TextUtils.isEmpty(email)) {
            edtEmail.setError(getString(R.string.error_field_required));
            focusView = edtEmail;
            cancel = true;
        } else if (!isEmailValid(email)) {
            edtEmail.setError(getString(R.string.error_invalid_email));
            focusView = edtEmail;
            cancel = true;
        }

        if (TextUtils.isEmpty(password)) {
            edtPasswordView.setError(getString(R.string.error_field_required));
            focusView = edtPasswordView;
            cancel = true;
        }


        if (cancel) {
            focusView.requestFocus();
            ActivityUtils.showToast(context, getString(R.string.errors_to_fix));
        } else {
            ActivityUtils.showProgressDialog(activity, getString(R.string.attempting_login));
            login(email, password);
        }

    }

    /**
     * Validate email logic
     */
    private boolean isEmailValid(String email) {
        return (email.contains("@") && email.contains("."));
    }

    /**
     * Validate password logic
     */
    private boolean isPasswordValid(String password) {
        return password.length() > 5;
    }

    /**
     * Try to send login request
     **/
    private void login(String email, String password) {
        Log.d(TAG, "signIn:" + email);
        firebaseAuth.signInWithEmailAndPassword(email, password)
                .addOnCompleteListener(this, new OnCompleteListener<AuthResult>() {
                            @Override
                            public void onComplete(@NonNull Task<AuthResult> task) {
                                if (task.isSuccessful()) {
                                    // Sign in success, update UI with the signed-in user's information
                                    Log.d(TAG, "signInWithEmail:success");
                                    FirebaseUser user = firebaseAuth.getCurrentUser();
                                    ActivityUtils.cancelProgressDialog();
                                    updateUI(user);
                                } else {
                                    // If sign in fails, display a message to the user.
                                    Log.w(TAG, "signInWithEmail:failure", task.getException());
                                    ActivityUtils.showToast(activity, getString(R.string.email_password_error));
                                    ActivityUtils.cancelProgressDialog();
                                }
                            }
                        }
                );
    }

    /**
     * If user logged and server on start Main Activity
     **/
    private void updateUI(FirebaseUser user) {
        if (user != null) {
            ConnectionUtils.checkServerConnection(activity, new OnGetCheckConnectionServerCallback() {
                @Override
                public void onGetCheckConnectionServerResponse(boolean response) {
                    if (response) {
                        Intent intent = new Intent(getApplicationContext(), MainActivity.class);
                        startActivity(intent);
                        finish();
                    }


                }
            });

        }

    }
}
