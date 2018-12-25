package com.lacina.cubeeclient.activities.User.Register;


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
public class CreateUserExistent {

    @Rule
    public ActivityTestRule<SplashScreenActivity> mActivityTestRule = new ActivityTestRule<>(SplashScreenActivity.class);

    @Test
    public void createUserExistent() {
        // Added a sleep statement to match the app's execution delay.
        // The recommended way to handle such scenarios is to use Espresso idling resources:
        // https://google.github.io/android-testing-support-library/docs/espresso/idling-resource/index.html
        try {
            Thread.sleep(6000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        ViewInteraction appCompatButton = onView(
                allOf(withId(R.id.create_account), withText("Criar conta"),
                        withParent(allOf(withId(R.id.email_login_form),
                                withParent(withId(R.id.refresh)))),
                        isDisplayed()));
        appCompatButton.perform(click());

        ViewInteraction appCompatAutoCompleteTextView = onView(
                withId(R.id.name_register));
        appCompatAutoCompleteTextView.perform(scrollTo(), replaceText("Cubee1"), closeSoftKeyboard());

        ViewInteraction appCompatEditText = onView(
                withId(R.id.number_register));
        appCompatEditText.perform(scrollTo(), replaceText("99999999"), closeSoftKeyboard());

        ViewInteraction appCompatAutoCompleteTextView4 = onView(
                withId(R.id.actv_email_register));
        appCompatAutoCompleteTextView4.perform(scrollTo(), replaceText("cubee1@cubee.com"), closeSoftKeyboard());

        ViewInteraction appCompatEditText2 = onView(
                withId(R.id.ed_password_register));
        appCompatEditText2.perform(scrollTo(), replaceText("cubee"), closeSoftKeyboard());

        ViewInteraction appCompatEditText3 = onView(
                withId(R.id.confirm_password_register));
        appCompatEditText3.perform(scrollTo(), replaceText("cubee"), closeSoftKeyboard());

        ViewInteraction appCompatButton2 = onView(
                allOf(withId(R.id.btn_register_user), withText("Register"),
                        withParent(allOf(withId(R.id.email_login_form),
                                withParent(withId(R.id.sing_up_form))))));
        appCompatButton2.perform(scrollTo(), click());

    }

}
