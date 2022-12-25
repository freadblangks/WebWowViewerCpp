//
// Created by Deamon on 11.12.22.
//

#ifndef AWEBWOWVIEWERCPP_MAPSCENEPLAN_H
#define AWEBWOWVIEWERCPP_MAPSCENEPLAN_H

#include <vector>
#include "FrameDependentData.h"
#include "../../engine/objects/ViewsObjects.h"
#include "../../engine/objects/wmo/wmoObject.h"

struct MapRenderPlan {
    int adtAreadId = -1;
    int areaId = -1;
    int parentAreaId = -1;

    HCameraMatrices renderingMatrices;

    //Result of culling test
    std::vector<WmoGroupResult> m_currentInteriorGroups = {};
    bool currentWmoGroupIsExtLit = false;
    bool currentWmoGroupShowExtSkybox = false;
    std::shared_ptr<WmoObject> m_currentWMO = nullptr;
    int m_currentWmoGroup = -1;

    FrameViewsHolder viewsHolder;

    HFrameDependantData frameDependentData = std::make_shared<FrameDependantData>();

    //Objects for update and rendering
    std::vector<std::shared_ptr<ADTObjRenderRes>> adtArray = {};
    M2ObjectListContainer m2Array;
    WMOListContainer wmoArray;
    WMOGroupListContainer wmoGroupArray;

    //Settings for the frame
};
typedef std::shared_ptr<MapRenderPlan> HMapRenderPlan;
#endif //AWEBWOWVIEWERCPP_MAPSCENEPLAN_H
