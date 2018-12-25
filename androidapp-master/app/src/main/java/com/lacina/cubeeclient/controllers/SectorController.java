package com.lacina.cubeeclient.controllers;

import com.lacina.cubeeclient.model.Sector;

import java.util.List;


/**
 * Sector controller singleton to control Sectors related operations.
 **/
@SuppressWarnings("ALL")
public class SectorController {

    private static SectorController INSTANCE;

    @SuppressWarnings("unused")
    private Sector sectorAtual;

    private List<Sector> sectorList;

    private SectorController() {
    }

    public static SectorController getInstance() {
        if (INSTANCE == null) {
            INSTANCE = new SectorController();

        }

        return INSTANCE;
    }

    public List<Sector> getSectorList() {
        return sectorList;
    }

    public void setSectorList(List<Sector> sectorList) {
        this.sectorList = sectorList;
    }


}
