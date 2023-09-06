//
// Created by Deamon on 11/26/2022.
//

#include "MapSceneRenderer.h"
#include "../../engine/objects/scenes/map.h"
#include "../../gapi/interface/sortLambda.h"
#include "../frame/FrameProfile.h"

std::shared_ptr<MapRenderPlan>
MapSceneRenderer::processCulling(const std::shared_ptr<FrameInputParams<MapSceneParams>> &frameInputParams) {
//    auto &scenes = frameInputParams->frameParameters->scene;

//    for (auto &scene : scenes) {
        auto mapScene = std::dynamic_pointer_cast<Map>(frameInputParams->frameParameters->scene);
        auto mapPlan = std::make_shared<HMapRenderPlan::element_type>();
        mapScene->makeFramePlan(*frameInputParams, mapPlan);
//    }

    return mapPlan;
}

void MapSceneRenderer::collectMeshes(const std::shared_ptr<MapRenderPlan> &renderPlan,
                                     const std::shared_ptr<std::vector<HGMesh>> &hopaqueMeshes,
                                     const std::shared_ptr<std::vector<HGSortableMesh>> &htransparentMeshes,
                                     const std::shared_ptr<std::vector<HGMesh>> &hSkyOpaqueMeshes,
                                     const std::shared_ptr<std::vector<HGSortableMesh>> &hSkyTransparentMeshes) {
    ZoneScoped;
    mapProduceUpdateCounter.beginMeasurement();

    auto &opaqueMeshes = *hopaqueMeshes;
    auto &transparentMeshes = *htransparentMeshes;

    auto &skyOpaqueMeshes = *hSkyOpaqueMeshes;
    auto &skyTransparentMeshes = *hSkyTransparentMeshes;

    opaqueMeshes.reserve(30000);
    transparentMeshes.reserve(30000);

    skyOpaqueMeshes.reserve(1000);
    skyTransparentMeshes.reserve(1000);

    const auto& cullStage = renderPlan;
    auto fdd = cullStage->frameDependentData;

    int m_viewRenderOrder = 0;

    // Put everything into one array and sort
    interiorViewCollectMeshCounter.beginMeasurement();
    bool renderPortals = m_config->renderPortals;
    bool renderADT = m_config->renderAdt;
    bool renderWMO = m_config->renderWMO;

    for (auto &view : cullStage->viewsHolder.getInteriorViews()) {
        view->collectMeshes(renderADT, true, renderWMO, opaqueMeshes, transparentMeshes);
    }
    interiorViewCollectMeshCounter.endMeasurement();

    exteriorViewCollectMeshCounter.beginMeasurement();
    {
        auto exteriorView = cullStage->viewsHolder.getExterior();
        if (exteriorView != nullptr) {
            exteriorView->collectMeshes(renderADT, true, renderWMO, opaqueMeshes, transparentMeshes);
        }
    }
    exteriorViewCollectMeshCounter.endMeasurement();

    m2CollectMeshCounter.beginMeasurement();
    if (m_config->renderM2) {
        for (auto &m2Object : cullStage->m2Array.getDrawn()) {
            if (m2Object == nullptr) continue;
            m2Object->collectMeshes(opaqueMeshes, transparentMeshes, m_viewRenderOrder);
            m2Object->drawParticles(opaqueMeshes, transparentMeshes, m_viewRenderOrder);
        }

        auto skyBoxView = cullStage->viewsHolder.getSkybox();
        if (skyBoxView) {
            for (auto &m2Object : skyBoxView->m2List.getDrawn()) {
                if (m2Object == nullptr) continue;
                m2Object->collectMeshes(skyOpaqueMeshes, skyTransparentMeshes, m_viewRenderOrder);
                m2Object->drawParticles(skyOpaqueMeshes, skyTransparentMeshes, m_viewRenderOrder);
            }
        }
    }
    m2CollectMeshCounter.endMeasurement();

    //No need to sort array which has only one element
    sortMeshCounter.beginMeasurement();
    if (transparentMeshes.size() > 1) {
        std::sort(transparentMeshes.begin(), transparentMeshes.end(), SortMeshes);
    }
    if (skyTransparentMeshes.size() > 1) {
        std::sort(skyTransparentMeshes.begin(), skyTransparentMeshes.end(), SortMeshes);
    }
    sortMeshCounter.endMeasurement();

    mapProduceUpdateCounter.endMeasurement();


    m_config->mapProduceUpdateTime = mapProduceUpdateCounter.getTimePerFrame();
    m_config->interiorViewCollectMeshTime = interiorViewCollectMeshCounter.getTimePerFrame();
    m_config->exteriorViewCollectMeshTime = exteriorViewCollectMeshCounter.getTimePerFrame();
    m_config->m2CollectMeshTime = m2CollectMeshCounter.getTimePerFrame();
    m_config->sortMeshTime = sortMeshCounter.getTimePerFrame();
    m_config->collectBuffersTime = collectBuffersCounter.getTimePerFrame();
    m_config->sortBuffersTime = sortBuffersCounter.getTimePerFrame();
}

void MapSceneRenderer::updateSceneWideChunk(const std::shared_ptr<IBufferChunkVersioned<sceneWideBlockVSPS>> &sceneWideChunk,
                                            const HCameraMatrices &renderingMatrices,
                                            const HFrameDependantData &fdd,
                                            bool isVulkan,
                                            animTime_t sceneTime
                                            ) {
    ZoneScoped;

    static const mathfu::mat4 vulkanMatrixFix = mathfu::mat4(1, 0, 0, 0,
                                                              0, -1, 0, 0,
                                                              0, 0, 1.0/2.0, 1/2.0,
                                                              0, 0, 0, 1).Transpose();

    const static mathfu::vec4 zUp = {0,0,1.0,0};


    auto &blockPSVS = sceneWideChunk->getObject(0);
    blockPSVS.uLookAtMat = renderingMatrices->lookAtMat;
    if (isVulkan) {
        blockPSVS.uPMatrix = vulkanMatrixFix*renderingMatrices->perspectiveMat;
    } else {
        blockPSVS.uPMatrix = renderingMatrices->perspectiveMat;
    }
    blockPSVS.uInteriorSunDir = renderingMatrices->interiorDirectLightDir;
    blockPSVS.uViewUpSceneTime = mathfu::vec4(renderingMatrices->viewUp.xyz(), sceneTime);

    blockPSVS.extLight.uExteriorAmbientColor = fdd->exteriorAmbientColor;
    blockPSVS.extLight.uExteriorHorizontAmbientColor = fdd->exteriorHorizontAmbientColor;
    blockPSVS.extLight.uExteriorGroundAmbientColor = fdd->exteriorGroundAmbientColor;
    blockPSVS.extLight.uExteriorDirectColor = fdd->exteriorDirectColor;
    blockPSVS.extLight.uExteriorDirectColorDir = mathfu::vec4(fdd->exteriorDirectColorDir, 1.0);
    blockPSVS.extLight.uAdtSpecMult_FogCount = mathfu::vec4(m_config->adtSpecMult, fdd->fogResults.size(), 0, 1.0);

    for (int i = 0; i < std::min<int>(fdd->fogResults.size(), FOG_MAX_SHADER_COUNT); i++)
    {
        auto &fogResult = fdd->fogResults[i];

        mathfu::vec4 heightPlane = mathfu::vec4(
            zUp.xyz(),
            -(mathfu::vec3::DotProduct(zUp.xyz(), (zUp * fogResult.FogHeight).xyz()))
        );
        heightPlane = renderingMatrices->invTranspViewMat * heightPlane;

        float fogEnd = std::min<float>(std::max<float>(m_config->farPlane, 277.5), m_config->farPlane);
        if (m_config->disableFog || !fdd->FogDataFound) {
            fogEnd = 100000000.0f;
            fogResult.FogScaler = 0;
            fogResult.FogDensity = 0;
            fogResult.FogHeightDensity = 0;
        }

        const float densityMultFix =  0.00050000002 * std::pow(10, m_config->fogDensityIncreaser);
        float fogScaler = fogResult.FogScaler;
        if (fogScaler <= 0.00000001f) fogScaler = 0.5f;
        float fogStart = std::min<float>(m_config->farPlane, 3000) * fogScaler;

        blockPSVS.fogData[i].densityParams = mathfu::vec4(
            fogStart,
            fogEnd,
            fogResult.FogDensity * densityMultFix,
            0);
        blockPSVS.fogData[i].classicFogParams = mathfu::vec4(0, 0, 0, 0);
        blockPSVS.fogData[i].heightPlane = heightPlane;
        blockPSVS.fogData[i].color_and_heightRate = mathfu::vec4(fogResult.FogColor, fogResult.FogHeightScaler
//            * 0.01
        );
        blockPSVS.fogData[i].heightDensity_and_endColor = mathfu::vec4(
            fogResult.FogHeightDensity * densityMultFix,
            fogResult.EndFogColor.x,
            fogResult.EndFogColor.y,
            fogResult.EndFogColor.z
        );

        fogResult.SunFogAngle = 0.0f;
        blockPSVS.fogData[i].sunAngle_and_sunColor = mathfu::vec4(
            fogResult.SunFogAngle,
            fogResult.SunFogColor.x,
            fogResult.SunFogColor.y,
            fogResult.SunFogColor.z
        );
        blockPSVS.fogData[i].heightColor_and_endFogDistance = mathfu::vec4(
            fogResult.FogHeightColor,
            (fogResult.EndFogColorDistance > 0) ?
                fogResult.EndFogColorDistance :
                1000.0f
        );
        blockPSVS.fogData[i].sunPercentage = mathfu::vec4(
            fogResult.SunFogAngle * fogResult.SunFogStrength,
            0, 1.0, 1.0);
        blockPSVS.fogData[i].sunDirection_and_fogZScalar = mathfu::vec4(
            fdd->exteriorDirectColorDir, //TODO: for fog this is calculated from SUN position
            fogResult.FogZScalar
        );
        blockPSVS.fogData[i].heightFogCoeff = fogResult.FogHeightCoefficients;
        blockPSVS.fogData[i].mainFogCoeff = fogResult.MainFogCoefficients;
        blockPSVS.fogData[i].heightDensityFogCoeff = fogResult.HeightDensityFogCoefficients;

        bool mainFogOk = (fogResult.MainFogStartDist + 0.001 <= fogResult.MainFogEndDist);
        blockPSVS.fogData[i].mainFogEndDist_mainFogStartDist_legacyFogScalar_blendAlpha = mathfu::vec4(
            mainFogOk ? fogResult.MainFogEndDist : fogResult.MainFogStartDist + 0.001,
            fogResult.MainFogStartDist >= 0.0 ? fogResult.MainFogStartDist : 0.0f,
            fogResult.LegacyFogScalar,
            fogResult.FogBlendAlpha
        );
        blockPSVS.fogData[i].heightFogEndColor_fogStartOffset = mathfu::vec4(
            fogResult.HeightEndFogColor,
            fogResult.FogStartOffset
        );
    }
    sceneWideChunk->saveVersion(0);
}
