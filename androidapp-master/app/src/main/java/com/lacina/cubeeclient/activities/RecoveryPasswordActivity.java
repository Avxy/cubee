package com.lacina.cubeeclient.activities;

import android.app.Activity;
import android.content.Context;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.v7.app.AppCompatActivity;
import android.text.TextUtils;
import android.util.Log;
import android.view.View;
import android.widget.AutoCompleteTextView;
import android.widget.Button;

import com.google.android.gms.tasks.OnCompleteListener;
import com.google.android.gms.tasks.Task;
import com.google.firebase.auth.FirebaseAuth;
import com.lacina.cubeeclient.R;
import com.lacina.cubeeclient.utils.ActivityUtils;

import butterknife.BindView;
import butterknife.ButterKnife;
import butterknife.OnClick;

/**
 * Activity for send Recovery password request to server
 **/
@SuppressWarnings({"ALL", "JavaDoc"})
public class RecoveryPasswordActivity extends AppCompatActivity {

    /**
     * Tag used to auxiliate the aplication log
     */
    private static final String TAG = "Recovery";

    /**
     * Edit text view to input the user email
     */
    @SuppressWarnings("unused")
    @BindView(R.id.actvEmailRecovery)
    public AutoCompleteTextView mEmailView;

    /**
     * Button view to click for recovery the password
     */
    @SuppressWarnings("unused")
    @BindView(R.id.btn_recovery)
    public Button btnRecoveryPass;

    /**
     * Field to reference this aplicationContext
     */
    @SuppressWarnings("unused")
    private Context context;

    /**
     * Field to reference this activity
     */
    private Activity activity;

    /**
     * The entry point of the Firebase Authentication SDK.
     **/
    private FirebaseAuth firebaseAuth;

    /**
     * Bind view and set the fields
     * @param savedInstanceState
     */
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_recovery_password);
        context = getApplicationContext();
        activity = this;
        ButterKnife.bind(this);
        firebaseAuth = FirebaseAuth.getInstance();
    }

    /**
     * ON CLICK - Click to recover the user password
     */
    @SuppressWarnings("unused")
    @OnClick(R.id.btn_recovery)
    public void clickRecoveryPasswordButton() {
        attemptRecoveryEmail();
    }

    /**
     * Validate form and try to send request
     **/
    private void attemptRecoveryEmail() {
        // Reset errors.
        mEmailView.setError(null);

        // Store values at the time of the login attempt.
        String email = mEmailView.getText().toString().trim();

        boolean cancel = false;
        View focusView = null;


        // Check for a valid email address.
        if (TextUtils.isEmpty(email)) {
            mEmailView.setError(getString(R.string.error_field_required));
            focusView = mEmailView;
            cancel = true;
        } else if (!isEmailValid(email)) {
            mEmailView.setError(getString(R.string.error_invalid_email));
            focusView = mEmailView;
            cancel = true;
        }


        if (cancel) {
            focusView.requestFocus();
        } else {
            recoveryEmail(email);
        }
    }

    /**
     * Send request to Firebase
     * <p>
     * If request isSuccessful: Return to login.
     * If request not successful shoow error
     *
     * @param email User email
     **/
    private void recoveryEmail(String email) {
        firebaseAuth.sendPasswordResetEmail(email).addOnCompleteListener(new OnCompleteListener<Void>() {
            @Override
            public void onComplete(@NonNull Task<Void> task) {
                if (task.isSuccessful()) {
                    finish();
                } else {
                    // If request fails, display a message to the user.
                    ActivityUtils.showToast(activity, "Erro ao recurerar senha.");
                    Log.w(TAG, "createUserWithEmail:failure", task.getException());
                }
            }
        });
    }

    /**
     * Validates user email
     * @param email email to be validated
     * @return
     */
    private boolean isEmailValid(String email) {
        //TODO: Replace this with your own logic
        return email.contains("@");
    }


}
