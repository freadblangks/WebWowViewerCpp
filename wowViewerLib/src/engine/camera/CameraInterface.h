//
// Created by deamon on 15.06.17.
//

#ifndef WOWVIEWERLIB_CAMERAINTERFACE_H
#define WOWVIEWERLIB_CAMERAINTERFACE_H

class ICamera;
#include <mathfu/vector.h>
#include "../../include/controllable.h"
#include "../DrawStage.h"
#include "../../include/iostuff.h"


class ICamera : public IControllable {
public:

    virtual HCameraMatrices getCameraMatrices() = 0;

    virtual void tick(animTime_t timeDelta) = 0;
};

#endif //WOWVIEWERLIB_CAMERAINTERFACE_H
