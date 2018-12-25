package com.lacina.cubeeclient.fragments;

import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.view.KeyEvent;
import android.view.View;

/**
 * Base fragment used when the fragment needs to override on back pressed
 */
public abstract class BackableFragment extends Fragment implements View.OnKeyListener {

    @Override
    public void onViewCreated(View view, Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);
        view.setFocusableInTouchMode(true);
        view.requestFocus();
        view.setOnKeyListener(this);
    }

    @Override
    public boolean onKey(View v, int keyCode, KeyEvent event) {
        if (event.getAction() == KeyEvent.ACTION_UP) {
            if (keyCode == KeyEvent.KEYCODE_BACK) {
                onBackButtonPressed();
                return true;
            }
        }

        return false;
    }

    protected abstract void onBackButtonPressed();
}