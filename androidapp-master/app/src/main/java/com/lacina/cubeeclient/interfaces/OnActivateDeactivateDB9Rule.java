package com.lacina.cubeeclient.interfaces;

import com.lacina.cubeeclient.model.Cubee;

/**
 * Interface used to communicate adapter with fragment to send a command to CUBEE via WiFI
 * Fragment: {@link com.lacina.cubeeclient.fragments.Db9ListCubeesFragment}
 * Adapter: {@link com.lacina.cubeeclient.adapters.CubeeDb9ListAdapter}
 */
public interface OnActivateDeactivateDB9Rule {
    void activateDB9Rule(Cubee cubee);

    void deactivateDB9Rule(Cubee cubee);
}
