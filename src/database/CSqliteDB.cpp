//
// Created by deamon on 24.12.19.
//

#include <sqlite3.h>
#include "CSqliteDB.h"
#include <cmath>
#include <algorithm>

CSqliteDB::CSqliteDB(std::string dbFileName) :
    m_sqliteDatabase(dbFileName, SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE),
    getWmoAreaAreaName(m_sqliteDatabase,
        "select wat.AreaName_lang as wmoAreaName, at.AreaName_lang from WMOAreaTable wat "
        "left join AreaTable at on at.id = wat.AreaTableID "
        "where wat.WMOID = ? and wat.NameSetID = ? and (wat.WMOGroupID = -1 or wat.WMOGroupID = ?) ORDER BY wat.WMOGroupID DESC"),
    getAreaNameStatement(m_sqliteDatabase,
        "select at.AreaName_lang from AreaTable at where at.ID = ?"),

    getLightStatement(m_sqliteDatabase,
        "select l.GameCoords_0, l.GameCoords_1, l.GameCoords_2, l.GameFalloffStart, l.GameFalloffEnd, l.LightParamsID_0, IFNULL(ls.SkyboxFileDataID, 0) from Light l "
        "    left join LightParams lp on lp.ID = l.LightParamsID_0 "
        "    left join LightSkybox ls on ls.ID = lp.LightSkyboxID "
        " where  "
        "   ((l.ContinentID = ?) and (( "
        "    abs(l.GameCoords_0 - ?) < l.GameFalloffEnd and "
        "    abs(l.GameCoords_1 - ?) < l.GameFalloffEnd and "
        "    abs(l.GameCoords_2 - ?) < l.GameFalloffEnd "
        "  ) or (l.GameCoords_0 = 0 and l.GameCoords_1 = 0 and l.GameCoords_2 = 0))) "
        "    or (l.GameCoords_0 = 0 and l.GameCoords_1 = 0 and l.GameCoords_2 = 0 and l.ContinentID = 0)  "
        "ORDER BY l.ID desc"),
    getLightData(m_sqliteDatabase,
        "select ld.AmbientColor, ld.DirectColor, ld.RiverCloseColor, ld.Time from LightData ld "

        " where ld.LightParamID = ? ORDER BY Time ASC"
        ),
    getLiquidObjectInfo(m_sqliteDatabase,
        "select ltxt.FileDataID, lm.LVF, ltxt.OrderIndex, lt.MinimapStaticCol from LiquidObject lo "
        "left join LiquidTypeXTexture ltxt on ltxt.LiquidTypeID = lo.LiquidTypeID "
        "left join LiquidType lt on lt.ID = lo.LiquidTypeID "
        "left join LiquidMaterial lm on lt.MaterialID = lm.ID "
        "where lo.ID = ? "
    ),
    getLiquidTypeInfo(m_sqliteDatabase,
        "select ltxt.FileDataID from LiquidTypeXTexture ltxt where "
        "ltxt.LiquidTypeID = ? order by ltxt.OrderIndex"
    )



{
    char *sErrMsg = "";
    sqlite3_exec(m_sqliteDatabase.getHandle(), "PRAGMA synchronous = OFF", NULL, NULL, &sErrMsg);

}

void CSqliteDB::getMapArray(std::vector<MapRecord> &mapRecords) {
    SQLite::Statement getMapList(m_sqliteDatabase, "select m.ID, m.Directory, m.MapName_lang, m.WdtFileDataID, m.MapType from Map m where m.WdtFileDataID > 0");

    while (getMapList.executeStep())
    {
        MapRecord mapRecord;

        // Demonstrate how to get some typed column value
        mapRecord.ID = getMapList.getColumn(0);
        mapRecord.MapDirectory = std::string((const char*) getMapList.getColumn(1));
        mapRecord.MapName = std::string((const char*) getMapList.getColumn(2));
        mapRecord.WdtFileID = getMapList.getColumn(3);
        mapRecord.MapType = getMapList.getColumn(4);

        mapRecords.push_back(mapRecord);
    }

}

std::string CSqliteDB::getAreaName(int areaId) {
    getAreaNameStatement.reset();

    getAreaNameStatement.bind(1, areaId);
    while (getAreaNameStatement.executeStep()) {
        std::string areaName = getAreaNameStatement.getColumn(0);

        return areaName;
    }


    return std::string();
}

std::string CSqliteDB::getWmoAreaName(int wmoId, int nameId, int groupId) {
    getWmoAreaAreaName.reset();

    getWmoAreaAreaName.bind(1, wmoId);
    getWmoAreaAreaName.bind(2, nameId);
    getWmoAreaAreaName.bind(3, groupId);

    while (getWmoAreaAreaName.executeStep()) {
        std::string wmoAreaName = getWmoAreaAreaName.getColumn(0);
        std::string areaName = getWmoAreaAreaName.getColumn(1);

        if (wmoAreaName == "") {
            return areaName;
        } else {
            return wmoAreaName;
        }
    }

    return "";
}

template <int T>
float getFloatFromInt(int value) {
    if (T == 0) {
        return (value & 0xFF) / 255.0f;
    }
    if (T == 1) {
        return ((value >> 8) & 0xFF) / 255.0f;
    }
    if (T == 2) {
        return ((value >> 16) & 0xFF) / 255.0f;
    }
}


void CSqliteDB::getEnvInfo(int mapId, float x, float y, float z, int ptime, LightResult &lightResult) {
    getLightStatement.reset();

    getLightStatement.bind(1, mapId);
    getLightStatement.bind(2, x);
    getLightStatement.bind(3, y);
    getLightStatement.bind(4, z);

struct InnerLightResult {
    float pos[3];
    float fallbackStart;
    float fallbackEnd;
    float blendAlpha = 0;
    int paramId;
    int skyBoxFileId;
};

    std::vector<InnerLightResult> innerResults;

    float totalBlend = 0;
    while (getLightStatement.executeStep()) {
        InnerLightResult ilr;
        ilr.pos[0] = getLightStatement.getColumn(0).getDouble();
        ilr.pos[1] = getLightStatement.getColumn(1).getDouble();
        ilr.pos[2] = getLightStatement.getColumn(2).getDouble();
        ilr.fallbackStart = getLightStatement.getColumn(3).getDouble();
        ilr.fallbackEnd = getLightStatement.getColumn(4).getDouble();
        ilr.paramId = getLightStatement.getColumn(5).getDouble();
        ilr.skyBoxFileId = getLightStatement.getColumn(6).getInt();

        bool defaultRec = false;
        if (ilr.pos[0] == 0 && ilr.pos[1] == 0 && ilr.pos[2] == 0 ) {
            if (totalBlend > 1) continue;
            defaultRec = true;
        }

        if (!defaultRec) {
            float distanceToLight =
                std::sqrt(
                    (ilr.pos[0] - x) * (ilr.pos[0] - x) +
                    (ilr.pos[1] - y) * (ilr.pos[1] - y) +
                    (ilr.pos[2] - z) * (ilr.pos[2] - z));

            ilr.blendAlpha = 1.0f - (distanceToLight - ilr.fallbackStart) / (ilr.fallbackEnd - ilr.fallbackStart);
            if (ilr.blendAlpha <= 0) continue;

            totalBlend += ilr.blendAlpha;
        } else {
            ilr.blendAlpha = 1.0f - totalBlend;
        }



        innerResults.push_back(ilr);
    }

    //From lowest to highest
    std::sort(innerResults.begin(), innerResults.end(), [](const InnerLightResult &a, const InnerLightResult &b) {
        return a.blendAlpha - b.blendAlpha;
    });

    struct InnerLightDataRes {
        int ambientLight;
        int directLight;
        int closeRiverColor;
        int time;
    };

    lightResult.ambientColor[0] = 0;
    lightResult.ambientColor[1] = 0;
    lightResult.ambientColor[2] = 0;

    lightResult.directColor[0] = 0;
    lightResult.directColor[1] = 0;
    lightResult.directColor[2] = 0;

    lightResult.closeRiverColor[0] = 0;
    lightResult.closeRiverColor[1] = 0;
    lightResult.closeRiverColor[2] = 0;

    lightResult.skyBoxFdid = 0;


    float totalSummator = 0.0f;
    for (int i = 0; i < innerResults.size() && totalSummator < 1.0f; i++) {
        auto &innerResult = innerResults[i];

        getLightData.reset();

        getLightData.bind(1, innerResult.paramId);

        InnerLightDataRes lastLdRes = {0, 0, -1};
        bool assigned = false;
        float innerAlpha = innerResult.blendAlpha < 1.0 ? innerResult.blendAlpha : 1.0;
        if (totalSummator + innerAlpha > 1.0f) {
            innerAlpha = 1.0f - totalSummator;
        }

        if (lightResult.skyBoxFdid == 0) {
            lightResult.skyBoxFdid = innerResult.skyBoxFileId;
        }

        while (getLightData.executeStep()) {
            InnerLightDataRes currLdRes;
            currLdRes.ambientLight = getLightData.getColumn(0);
            currLdRes.directLight = getLightData.getColumn(1);
            currLdRes.closeRiverColor = getLightData.getColumn(2);
            currLdRes.time = getLightData.getColumn(3);
            if (currLdRes.time > ptime) {
                assigned = true;

                if (lastLdRes.time == -1) {
                    //Set as is
                    lightResult.ambientColor[0] += getFloatFromInt<0>(currLdRes.ambientLight) * innerAlpha;
                    lightResult.ambientColor[1] += getFloatFromInt<1>(currLdRes.ambientLight) * innerAlpha;
                    lightResult.ambientColor[2] += getFloatFromInt<2>(currLdRes.ambientLight) * innerAlpha;

                    lightResult.directColor[0] += getFloatFromInt<0>(currLdRes.directLight) * innerAlpha;
                    lightResult.directColor[1] += getFloatFromInt<1>(currLdRes.directLight) * innerAlpha;
                    lightResult.directColor[2] += getFloatFromInt<2>(currLdRes.directLight) * innerAlpha;

                    lightResult.closeRiverColor[0] += getFloatFromInt<0>(currLdRes.closeRiverColor) * innerAlpha;
                    lightResult.closeRiverColor[1] += getFloatFromInt<1>(currLdRes.closeRiverColor) * innerAlpha;
                    lightResult.closeRiverColor[2] += getFloatFromInt<2>(currLdRes.closeRiverColor) * innerAlpha;
                } else {
                    //Blend using time as alpha
                    float timeAlphaBlend = 1.0f - (((float)currLdRes.time - (float)ptime) / ((float)currLdRes.time - (float)lastLdRes.time));

                    lightResult.ambientColor[0] += (getFloatFromInt<0>(currLdRes.ambientLight) * timeAlphaBlend + getFloatFromInt<0>(lastLdRes.ambientLight)*(1.0 - timeAlphaBlend)) * innerAlpha;
                    lightResult.ambientColor[1] += (getFloatFromInt<1>(currLdRes.ambientLight) * timeAlphaBlend + getFloatFromInt<1>(lastLdRes.ambientLight)*(1.0 - timeAlphaBlend)) * innerAlpha;
                    lightResult.ambientColor[2] += (getFloatFromInt<2>(currLdRes.ambientLight) * timeAlphaBlend + getFloatFromInt<2>(lastLdRes.ambientLight)*(1.0 - timeAlphaBlend)) * innerAlpha;

                    lightResult.directColor[0] += (getFloatFromInt<0>(currLdRes.directLight) * timeAlphaBlend + getFloatFromInt<0>(lastLdRes.directLight)*(1.0 - timeAlphaBlend)) * innerAlpha;
                    lightResult.directColor[1] += (getFloatFromInt<1>(currLdRes.directLight) * timeAlphaBlend + getFloatFromInt<1>(lastLdRes.directLight)*(1.0 - timeAlphaBlend)) * innerAlpha;
                    lightResult.directColor[2] += (getFloatFromInt<2>(currLdRes.directLight) * timeAlphaBlend + getFloatFromInt<2>(lastLdRes.directLight)*(1.0 - timeAlphaBlend)) * innerAlpha;

                    lightResult.closeRiverColor[0] += (getFloatFromInt<0>(currLdRes.closeRiverColor) * timeAlphaBlend + getFloatFromInt<0>(lastLdRes.closeRiverColor)*(1.0 - timeAlphaBlend)) * innerAlpha;
                    lightResult.closeRiverColor[1] += (getFloatFromInt<1>(currLdRes.closeRiverColor) * timeAlphaBlend + getFloatFromInt<1>(lastLdRes.closeRiverColor)*(1.0 - timeAlphaBlend)) * innerAlpha;
                    lightResult.closeRiverColor[2] += (getFloatFromInt<2>(currLdRes.closeRiverColor) * timeAlphaBlend + getFloatFromInt<2>(lastLdRes.closeRiverColor)*(1.0 - timeAlphaBlend)) * innerAlpha;
                }
                break;
            }
            lastLdRes = currLdRes;
        }

        if (!assigned) {
            lightResult.ambientColor[0] += getFloatFromInt<0>(lastLdRes.ambientLight) * innerAlpha;
            lightResult.ambientColor[1] += getFloatFromInt<1>(lastLdRes.ambientLight) * innerAlpha;
            lightResult.ambientColor[2] += getFloatFromInt<2>(lastLdRes.ambientLight) * innerAlpha;

            lightResult.directColor[0] += getFloatFromInt<0>(lastLdRes.directLight) * innerAlpha;
            lightResult.directColor[1] += getFloatFromInt<1>(lastLdRes.directLight) * innerAlpha;
            lightResult.directColor[2] += getFloatFromInt<2>(lastLdRes.directLight) * innerAlpha;

            lightResult.closeRiverColor[0] += getFloatFromInt<0>(lastLdRes.closeRiverColor) * innerAlpha;
            lightResult.closeRiverColor[1] += getFloatFromInt<1>(lastLdRes.closeRiverColor) * innerAlpha;
            lightResult.closeRiverColor[2] += getFloatFromInt<2>(lastLdRes.closeRiverColor) * innerAlpha;
        }

        totalSummator+= innerResult.blendAlpha;
    }

}

void CSqliteDB::getLiquidObjectData(int liquidObjectId, std::vector<LiquidMat> &loData) {
    getLiquidObjectInfo.reset();

    getLiquidObjectInfo.bind(1, liquidObjectId);

    while (getLiquidObjectInfo.executeStep()) {
        LiquidMat lm = {};
        lm.FileDataId = getLiquidObjectInfo.getColumn(0).getInt();
        lm.LVF = getLiquidObjectInfo.getColumn(1).getInt();
        lm.OrderIndex = getLiquidObjectInfo.getColumn(2).getInt();
        int color1 = getLiquidObjectInfo.getColumn(3).getInt();
        lm.color1[0] = getFloatFromInt<0>(color1);
        lm.color1[1] = getFloatFromInt<1>(color1);
        lm.color1[2] = getFloatFromInt<2>(color1);

        loData.push_back(lm);
    }
}

void CSqliteDB::getLiquidTypeData(int liquidTypeId, std::vector<int > &fileDataIds) {
    getLiquidTypeInfo.reset();

    getLiquidTypeInfo.bind(1, liquidTypeId);
    while (getLiquidTypeInfo.executeStep()) {
        fileDataIds.push_back(getLiquidTypeInfo.getColumn(0).getInt());
    }

}
