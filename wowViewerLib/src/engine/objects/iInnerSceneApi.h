//
// Created by deamon on 08.09.17.
//

#ifndef WEBWOWVIEWERCPP_IINNERSCENEAPI_H
#define WEBWOWVIEWERCPP_IINNERSCENEAPI_H

#include "mathfu/glsl_mappings.h"
#include "../persistance/header/M2FileHeader.h"
#include "wowFrameData.h"

class iInnerSceneApi {
public:
    virtual ~iInnerSceneApi() = default;
    virtual void setReplaceTextureArray(std::vector<int> &replaceTextureArray) = 0;
    virtual void setAnimationId(int animationId) = 0;

    virtual void checkCulling(WoWFrameData *frameData) = 0;
    virtual void collectMeshes(WoWFrameData *frameData) = 0;
    virtual void draw(WoWFrameData *frameData) = 0;

    virtual void doPostLoad(WoWFrameData *frameData) = 0;
    virtual void update(WoWFrameData *frameData) = 0;
    virtual void updateBuffers(WoWFrameData *frameData) = 0;
    virtual mathfu::vec4 getAmbientColor() = 0;
    virtual void setAmbientColorOverride(mathfu::vec4 &ambientColor, bool override) = 0;
    virtual bool getCameraSettings(M2CameraResult &cameraResult) = 0;
};
#endif //WEBWOWVIEWERCPP_IINNERSCENEAPI_H
