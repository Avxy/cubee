package com.lacina.cubeeclient.activities.User.Login;


import android.support.test.espresso.ViewInteraction;
import android.support.test.rule.ActivityTestRule;
import android.support.test.runner.AndroidJUnit4;
import android.test.suitebuilder.annotation.LargeTest;

import com.lacina.cubeeclient.R;
import com.lacina.cubeeclient.activities.SplashScreenActivity;

import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;

import static android.support.test.espresso.Espresso.onView;
import static android.support.test.espresso.action.ViewActions.click;
import static android.support.test.espresso.action.ViewActions.closeSoftKeyboard;
import static android.support.test.espresso.action.ViewActions.replaceText;
import static android.support.test.espresso.action.ViewActions.scrollTo;
import static android.support.test.espresso.matcher.ViewMatchers.isDisplayed;
import static android.support.test.espresso.matcher.ViewMatchers.withId;
import static android.support.test.espresso.matcher.ViewMatchers.withParent;
import static android.support.test.espresso.matcher.ViewMatchers.withText;
import static org.hamcrest.Matchers.allOf;

@LargeTest
@RunWith(AndroidJUnit4.class)
public class LoginPasswordEmptyTest {

    @Rule
    public ActivityTestRule<SplashScreenActivity> mActivityTestRule = new ActivityTestRule<>(SplashScreenActivity.class);

    @Test
    public void loginPasswordEmpty() {
        // Added a sleep statement to match the app's execution delay.
        // The recommended way to handle such scenarios is to use Espresso idling resources:
        // https://google.github.io/android-testing-support-library/docs/espresso/idling-resource/index.html
        try {
            Thread.sleep(6000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        ViewInteraction appCompatButton = onView(
                allOf(withId(R.id.login_button), withText("Login"),
                        withParent(allOf(withId(R.id.email_login_form),
                                withParent(withId(R.id.refresh)))),
                        isDisplayed()));
        appCompatButton.perform(click());

        // Added a sleep statement to match the app's execution delay.
        // The recommended way to handle such scenarios is to use Espresso idling resources:
        // https://google.github.io/android-testing-support-library/docs/espresso/idling-resource/index.html
        try {
            Thread.sleep(861);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        ViewInteraction appCompatAutoCompleteTextView = onView(
                withId(R.id.actv_email_register));
        appCompatAutoCompleteTextView.perform(scrollTo(), replaceText("cubee1@cubee.com"), closeSoftKeyboard());

        // Added a sleep statement to match the app's execution delay.
        // The recommended way to handle such scenarios is to use Espresso idling resources:
        // https://google.github.io/android-testing-support-library/docs/espresso/idling-resource/index.html
        try {
            Thread.sleep(3000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        ViewInteraction appCompatButton2 = onView(
                allOf(withId(R.id.login_button_email_password), withText("Login"),
                        withParent(allOf(withId(R.id.email_login_form),
                                withParent(withId(R.id.sing_up_form))))));
        appCompatButton2.perform(scrollTo(), click());

        try {
            Thread.sleep(861);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

    }

}
