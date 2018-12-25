package com.lacina.cubeeclient.activities;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.support.v7.app.AppCompatActivity;

import com.lacina.cubeeclient.R;

/**
 * Activity that appears at launching.
 * Used to do some verifications before start the application logic.
 **/
@SuppressWarnings({"ALL", "JavaDoc"})
public class SplashScreenActivity extends AppCompatActivity {

    /**
     * Field to reference this activity.
     */
    @SuppressWarnings("unused")
    private Activity activity;

    /**
     * Field to reference this aplicationContext.
     */
    @SuppressWarnings("unused")
    private Context context;

    /**
     * Bind view, set fields, and set the time of the SplashScreen
     * @param savedInstanceState
     */
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_splash_screen);
        activity = this;
        context = this.getApplicationContext();

        Handler delay = new Handler();
        delay.postDelayed(new Runnable() {
            @Override
            public void run() {
                startLoginAndBLETabsActivity();
            }
        }, 1000);

    }

    /**
     * Go to the first activity with user interaction.
     * {@link com.lacina.cubeeclient.activities.LoginAndBLETabsActivity}
     **/
    private void startLoginAndBLETabsActivity() {
        Intent intentInitialTabsActivity = new Intent(getApplicationContext(), LoginAndBLETabsActivity.class);
        startActivity(intentInitialTabsActivity);
        finish();
    }

}