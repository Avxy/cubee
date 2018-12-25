package com.lacina.cubeeclient.activities.User.EditUser;


import android.support.test.espresso.ViewInteraction;
import android.support.test.rule.ActivityTestRule;
import android.support.test.runner.AndroidJUnit4;
import android.test.suitebuilder.annotation.LargeTest;

import com.lacina.cubeeclient.R;
import com.lacina.cubeeclient.activities.SplashScreenActivity;

import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;

import static android.support.test.InstrumentationRegistry.getInstrumentation;
import static android.support.test.espresso.Espresso.onView;
import static android.support.test.espresso.Espresso.openActionBarOverflowOrOptionsMenu;
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
public class EditUserEmptyNameTest {

    @Rule
    public ActivityTestRule<SplashScreenActivity> mActivityTestRule = new ActivityTestRule<>(SplashScreenActivity.class);

    @Test
    public void editUserEmptyName() {
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
            Thread.sleep(473);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        ViewInteraction appCompatAutoCompleteTextView = onView(
                withId(R.id.actv_email_register));
        appCompatAutoCompleteTextView.perform(scrollTo(), replaceText("cubee1@cubee.com"), closeSoftKeyboard());

        ViewInteraction appCompatEditText = onView(
                withId(R.id.ed_password_register));
        appCompatEditText.perform(scrollTo(), replaceText("cubee1"), closeSoftKeyboard());

        ViewInteraction appCompatButton2 = onView(
                allOf(withId(R.id.login_button_email_password), withText("Login"),
                        withParent(allOf(withId(R.id.email_login_form),
                                withParent(withId(R.id.sing_up_form))))));
        appCompatButton2.perform(scrollTo(), click());

        // Added a sleep statement to match the app's execution delay.
        // The recommended way to handle such scenarios is to use Espresso idling resources:
        // https://google.github.io/android-testing-support-library/docs/espresso/idling-resource/index.html
        try {
            Thread.sleep(3590);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        openActionBarOverflowOrOptionsMenu(getInstrumentation().getTargetContext());

        ViewInteraction appCompatTextView = onView(
                allOf(withId(R.id.title), withText("Visualizar usuário"), isDisplayed()));
        appCompatTextView.perform(click());

        ViewInteraction appCompatButton3 = onView(
                allOf(withId(R.id.edit_button), withText("Editar")));
        appCompatButton3.perform(scrollTo(), click());

        try {
            Thread.sleep(3590);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        ViewInteraction appCompatAutoCompleteTextView2 = onView(
                allOf(withId(R.id.name_register), withText("Cubee1")));
        appCompatAutoCompleteTextView2.perform(scrollTo());

        try {
            Thread.sleep(3590);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        appCompatAutoCompleteTextView2.perform(replaceText(""));

        ViewInteraction appCompatButton4 = onView(
                allOf(withId(R.id.save_button), withText("Salvar")));
        appCompatButton4.perform(scrollTo(), click());

        try {
            Thread.sleep(3590);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

    }

}
