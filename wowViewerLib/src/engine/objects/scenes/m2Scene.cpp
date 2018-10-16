//
// Created by deamon on 08.09.17.
//

#include "m2Scene.h"
#include "../../algorithms/mathHelper.h"
#include "../../../gapi/interface/meshes/IM2Mesh.h"
#include "../../../gapi/interface/IDevice.h"

void M2Scene::checkCulling(WoWFrameData *frameData) {
    mathfu::vec4 cameraPos = mathfu::vec4(frameData->m_cameraVec3, 1.0);
    mathfu::mat4 &frustumMat = frameData->m_perspectiveMatrixForCulling;
    mathfu::mat4 &lookAtMat4 = frameData->m_lookAtMat4;


    mathfu::mat4 projectionModelMat = frustumMat*lookAtMat4;

    std::vector<mathfu::vec4> frustumPlanes = MathHelper::getFrustumClipsFromMatrix(projectionModelMat);
//    MathHelper::fixNearPlane(frustumPlanes, cameraPos);

    std::vector<mathfu::vec3> frustumPoints = MathHelper::calculateFrustumPointsFromMat(projectionModelMat);

    m_drawModel = m_m2Object->checkFrustumCulling(cameraPos, frustumPlanes, frustumPoints);
}

void M2Scene::draw(WoWFrameData *frameData) {
    if (!m_drawModel) return;

    m_api->getDevice()->drawMeshes(frameData->renderedThisFrame);
}

void M2Scene::doPostLoad(WoWFrameData *frameData) {
    if (m_m2Object->doPostLoad()) {
        auto max = m_m2Object->getAABB().max;
        auto min = m_m2Object->getAABB().min;
        m_api->setCameraPosition(4.0f*min.x, (min.y+max.y)/2.0f, 0);
        m_api->setCameraOffset(
            min.x + ((max.x-min.x)/2.0f),
            min.y + ((max.y-min.y)/2.0f),
            min.z + ((max.z-min.z)/2.0f));
    }
}

void M2Scene::update(WoWFrameData *frameData) {
    m_m2Object->update(frameData->deltaTime, frameData->m_cameraVec3, frameData->m_lookAtMat4);
}

mathfu::vec4 M2Scene::getAmbientColor() {
    if (doOverride) {
        return m_ambientColorOverride;
    } else {
        return m_m2Object->getAmbientLight();
    }
}

bool M2Scene::getCameraSettings(M2CameraResult &result) {
    if (m_cameraView > -1 && m_m2Object->getGetIsLoaded()) {
        result = m_m2Object->updateCamera(0, m_cameraView);
        return true;
    }
    return false;
}

void M2Scene::setAmbientColorOverride(mathfu::vec4 &ambientColor, bool override) {
    doOverride = override;
    m_ambientColorOverride = ambientColor;

    m_m2Object->setAmbientColorOverride(ambientColor, override);
}

void M2Scene::collectMeshes(WoWFrameData * frameData) {
    frameData->renderedThisFrame = std::vector<HGMesh>();

    m_m2Object->collectMeshes(frameData->renderedThisFrame, 0);
    m_m2Object->drawParticles(frameData->renderedThisFrame, 0);

    std::sort(frameData->renderedThisFrame.begin(),
              frameData->renderedThisFrame.end(),
              IDevice::sortMeshes
    );

}
