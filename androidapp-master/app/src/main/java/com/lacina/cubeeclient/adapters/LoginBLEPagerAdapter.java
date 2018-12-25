package com.lacina.cubeeclient.adapters;

import android.content.Context;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentPagerAdapter;
import android.view.ViewGroup;

import com.lacina.cubeeclient.activities.LoginAndBLETabsActivity;
import com.lacina.cubeeclient.fragments.LoginTabFragment;
import com.lacina.cubeeclient.fragments.ScanCubeesTabFragment;

import java.util.HashMap;
import java.util.Map;

/**
 * Adapter for login and ble tabs.
 * USED IN: {@link LoginAndBLETabsActivity}
 **/
@SuppressWarnings("ALL")
public class LoginBLEPagerAdapter extends FragmentPagerAdapter {
    private final int LOGIN = 0;

    private final int BLE = 1;

    private final int numOfTabs;

    private final Map<Integer, String> fragmentTags;

    private final FragmentManager fragmentManager;


    /**
     * Reference to caller context
     */
    private final Context context;

    public LoginBLEPagerAdapter(FragmentManager fm, int numOfTabs, Context context) {
        super(fm);
        this.fragmentTags = new HashMap<>();
        fragmentManager = fm;
        this.numOfTabs = numOfTabs;
        this.context = context;
    }


    @Override
    public Fragment getItem(int position) {
        switch (position) {
            case LOGIN:
                return Fragment.instantiate(context, LoginTabFragment.class.getName());

            case BLE:
                return Fragment.instantiate(context, ScanCubeesTabFragment.class.getName());

            default:
                return null;
        }
    }

    @Override
    public int getCount() {
        return numOfTabs;
    }

    @Override
    public Object instantiateItem(ViewGroup container, int position) {
        Object obj = super.instantiateItem(container, position);

        if (obj instanceof Fragment) {
            Fragment f = (Fragment) obj;
            String tag = f.getTag();
            fragmentTags.put(position, tag);
        }
        return obj;
    }

    @SuppressWarnings("unused")
    public Fragment getFragment(int position) {
        String tag = this.fragmentTags.get(position);
        return fragmentManager.findFragmentByTag(tag);
    }

}
