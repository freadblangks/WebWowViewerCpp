//
// Created by Deamon on 5/20/2023.
//

#include "LiquidInstance.h"
#include "LiquidDataGetters.h"
#include "../../algorithms/mathHelper.h"

LiquidInstance::LiquidInstance(const HApiContainer &api,
                               const HMapSceneBufferCreate &sceneRenderer,
                               const SMLiquidInstance &liquidInstance,
                               std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> &waterPlacementChunk,
                               const mathfu::vec3 &liquidBasePos,
                               const PointerChecker<char> &mH2OBlob, CAaBox &waterAaBB) :
                               m_api(api), m_sceneRenderer(sceneRenderer), m_waterPlacementChunk(waterPlacementChunk),
                               m_waterBBox(waterAaBB){

    //Creating liquid instance from ADT data
    liquid_object = liquidInstance.liquid_object_or_lvf >= 42 ? liquidInstance.liquid_object_or_lvf : 0;
    liquidType = liquidInstance.liquid_type;

    //1. Get Data from DB
    int basetextureFDID;
    mathfu::vec3 color;
    int liquidFlags;
    int liquidVertexFormat;
    bool generateTexCoordsFromPos;
    getInfoFromDatabase(liquidInstance, basetextureFDID, color, liquidFlags,
                        liquidVertexFormat,
                        generateTexCoordsFromPos);

    //2. Parse Vertex Data from ADT
    std::vector<LiquidVertexFormat> vertexBuffer;
    std::vector<uint16_t> indexBuffer;

    createAdtVertexData(liquidInstance, liquidBasePos, mH2OBlob, m_waterBBox, liquidVertexFormat,
                        generateTexCoordsFromPos, vertexBuffer,
                        indexBuffer);

    HGVertexBufferBindings vertexWaterBufferBindings = createLiquidVao(sceneRenderer, vertexBuffer, indexBuffer);


    //Create material(s)
    PipelineTemplate pipelineTemplate;
    pipelineTemplate.element = DrawElementMode::TRIANGLES;
    pipelineTemplate.depthWrite = false;
    pipelineTemplate.depthCulling = true;
    pipelineTemplate.backFaceCulling = false;
    pipelineTemplate.blendMode = EGxBlendEnum::GxBlend_Alpha;

    WaterMaterialTemplate waterMaterialTemplate;
    waterMaterialTemplate.color = color;
    waterMaterialTemplate.liquidFlags = liquidFlags;

    assert(liquidFlags >= 0 && liquidFlags <= 2063);

    if (basetextureFDID != 0) {
        auto htext = m_api->cacheStorage->getTextureCache()->getFileId(basetextureFDID);
        waterMaterialTemplate.texture = m_api->hDevice->createBlpTexture(htext, true, true);
    } else {
        waterMaterialTemplate.texture = m_api->hDevice->getBlackTexturePixel();
    }
    auto waterMaterial = sceneRenderer->createWaterMaterial(m_waterPlacementChunk, pipelineTemplate, waterMaterialTemplate);

    m_liquidMaterials.push_back(waterMaterial);

    //Create mesh(es)
    for (int i = 0; i < m_liquidMaterials.size(); i++) {
        gMeshTemplate meshTemplate(vertexWaterBufferBindings);
        meshTemplate.meshType = MeshType::eWmoMesh;

        meshTemplate.start = 0;
        meshTemplate.end = indexBuffer.size();

        auto mesh = sceneRenderer->createSortableMesh(meshTemplate, waterMaterial, 99);
        m_liquidMeshes.push_back(mesh);
    }

    assert(waterMaterial->liquidFlags >= 0 && waterMaterial->liquidFlags <= 2063);
}

void
LiquidInstance::getInfoFromDatabase(const SMLiquidInstance &liquidInstance, int &basetextureFDID, mathfu::vec3 &color,
                                    int &liquidFlags, int &liquidVertexFormat, bool &generateTexCoordsFromPos) {
    basetextureFDID= 0;
    color= mathfu::vec3(0, 0, 0);
    liquidFlags = 0;
    liquidVertexFormat = 0;
    generateTexCoordsFromPos= false;
    mathfu::vec3 minimapStaticCol = {0, 0, 0};
    if (m_api->databaseHandler != nullptr) {
        LiquidTypeAndMat liqMatAndType;
        std::vector<LiquidTextureData> liquidTextureData;

        if (liquidInstance.liquid_object_or_lvf > 41) {

            m_api->databaseHandler->getLiquidObjectData(liquidInstance.liquid_object_or_lvf, liquidInstance.liquid_type, liqMatAndType, liquidTextureData);
            //From Liquid::GetVertexFormat
            if (liquidInstance.liquid_type == 2) {
                liquidVertexFormat = 2;
            }
        } else {
            liquidVertexFormat = liquidInstance.liquid_object_or_lvf;
            m_api->databaseHandler->getLiquidTypeData(liquidInstance.liquid_type, liqMatAndType, liquidTextureData);

            assert(liquidInstance.liquid_object_or_lvf == liqMatAndType.matLVF);
        }
        basetextureFDID = liquidTextureData.size() > 0 ? liquidTextureData[0].fileDataId : 0;
        if (liqMatAndType.color1[0] > 0 || liqMatAndType.color1[1] > 0 || liqMatAndType.color1[2] > 0) {
            color = mathfu::vec3(liqMatAndType.color1[2], liqMatAndType.color1[1], liqMatAndType.color1[0]);
        }
        minimapStaticCol = mathfu::vec3(liqMatAndType.minimapStaticCol[2], liqMatAndType.minimapStaticCol[1], liqMatAndType.minimapStaticCol[0]);

        liquidFlags = liqMatAndType.flags;
//        assert(liquidVertexFormat == liqMatAndType.matLVF);
//        liquidVertexFormat = liqMatAndType.matLVF;
        generateTexCoordsFromPos = getLiquidSettings(liquidInstance.liquid_object_or_lvf,
                                                     liquidInstance.liquid_type,
                                                     liqMatAndType.materialID,
                                                     liqMatAndType.flowSpeed <=0).generateTexCoordsFromPos;
    }

    assert(liquidFlags >= 0 && liquidFlags <= 2063);
}

HGVertexBufferBindings LiquidInstance::createLiquidVao(const HMapSceneBufferCreate &sceneRenderer,
                                                       std::vector<LiquidVertexFormat> &vertexBuffer,
                                                       std::vector<uint16_t> &indexBuffer) const {
    auto waterIBO = sceneRenderer->createWaterIndexBuffer(indexBuffer.size() * sizeof(uint16_t));
    waterIBO->uploadData(indexBuffer.data(), indexBuffer.size() * sizeof(uint16_t));

    auto waterVBO = sceneRenderer->createWaterVertexBuffer(vertexBuffer.size() * sizeof(LiquidVertexFormat));
    waterVBO->uploadData(
        vertexBuffer.data(),
        vertexBuffer.size() * sizeof(LiquidVertexFormat)
    );


    auto vertexWaterBufferBindings = sceneRenderer->createWaterVAO(waterVBO, waterIBO);
    return vertexWaterBufferBindings;
}

void LiquidInstance::createAdtVertexData(const SMLiquidInstance &liquidInstance, const mathfu::vec3 &liquidBasePos,
                                         const PointerChecker<char> &mH2OBlob, CAaBox &waterAaBB,
                                         int liquidVertexFormat, bool generateTexCoordsFromPos,
                                         std::vector<LiquidVertexFormat> &vertexBuffer,
                                         std::vector<uint16_t> &indexBuffer) const {
    if (!liquidInstance.offset_vertex_data && liquidInstance.liquid_type != 2) {
        liquidVertexFormat = 2;
    }

    float *vertexDataPtr = nullptr;
    if (liquidInstance.offset_vertex_data != 0) {
        vertexDataPtr = ((float *) (&mH2OBlob[liquidInstance.offset_vertex_data]));
    }

    //Set iteration restrictions for triangles
    int y_begin = 0;
    int x_begin = 0;
    int x_end = 8;
    int y_end = 8;
    int totalCount = 9 * 9;
    if (liquidInstance.liquid_object_or_lvf <= 41) {
        x_begin = liquidInstance.x_offset;
        y_begin = liquidInstance.y_offset;
        x_end = liquidInstance.x_offset + liquidInstance.width;
        y_end = liquidInstance.y_offset + liquidInstance.height;

        totalCount = (liquidInstance.width + 1) * (liquidInstance.height + 1) ;
    }

    float minX = 999999;
    float maxX = -999999;
    float minY = 999999;
    float maxY = -999999;
    float minZ = 999999;
    float maxZ = -999999;

    minX = std::min(minX, waterAaBB.min.x);
    maxX = std::max(maxX, waterAaBB.max.x);
    minY = std::min(minY, waterAaBB.min.y);
    maxY = std::max(maxY, waterAaBB.max.y);
    minZ = std::min(minZ, waterAaBB.min.z);
    maxZ = std::max(maxZ, waterAaBB.max.z);

    //Parse the blob
    for (int y = 0; y < liquidInstance.height + 1; y++) {
        for (int x = 0; x < liquidInstance.width + 1; x++) {
            mathfu::vec3 pos =
                liquidBasePos -
                mathfu::vec3(
                    MathHelper::UNITSIZE*(y+liquidInstance.y_offset),
                    MathHelper::UNITSIZE*(x+liquidInstance.x_offset),
                    -liquidInstance.min_height_level
                );

            mathfu::vec2 uv = mathfu::vec2(0,0);
            if (vertexDataPtr!= nullptr) {
                pos.z = getLiquidVertexHeight(liquidVertexFormat, vertexDataPtr, totalCount, y * (liquidInstance.width + 1) + x);
            }

            if (generateTexCoordsFromPos) {
                uv = mathfu::vec2(pos.x * 0.6f, pos.y * 0.6f);
            } else {
                uv = getLiquidVertexCoords(liquidVertexFormat, vertexDataPtr, totalCount, y * (liquidInstance.width + 1) + x);
            }

            minX = std::min(minX, pos.x);  maxX = std::max(maxX, pos.x);
            minY = std::min(minY, pos.y);  maxY = std::max(maxY, pos.y);
            minZ = std::min(minZ, pos.z);  maxZ = std::max(maxZ, pos.z);

            LiquidVertexFormat vertex;
            vertex.pos_transp = mathfu::vec4(pos, 1.0);
            vertex.uv = uv;

            vertexBuffer.push_back(vertex);
        }
    }
    waterAaBB = CAaBox(
        C3Vector(mathfu::vec3(minX, minY, minZ)),
        C3Vector(mathfu::vec3(maxX, maxY, maxZ))
    );

    uint8_t *existsTable = getLiquidExistsTable(mH2OBlob, liquidInstance);

    for (int y = y_begin; y < y_end; y++) {
        for (int x = x_begin; x < x_end; x++) {

            int maskIndex = (y - y_begin) * (x_end - x_begin) + (x - x_begin);
            bool exists = (existsTable[maskIndex >> 3] >> ((maskIndex & 7))) & 1;

            if (!exists) continue;

            const int16_t vertIndexes[4] = {
                (int16_t) (y * (liquidInstance.width + 1 ) + x),
                (int16_t) (y * (liquidInstance.width + 1) + x + 1),
                (int16_t) ((y + 1) * (liquidInstance.width + 1) + x),
                (int16_t) ((y + 1) * (liquidInstance.width + 1) + x + 1),
            };

            indexBuffer.push_back (vertIndexes[0]);
            indexBuffer.push_back (vertIndexes[1]);
            indexBuffer.push_back (vertIndexes[2]);

            indexBuffer.push_back (vertIndexes[1]);
            indexBuffer.push_back (vertIndexes[3]);
            indexBuffer.push_back (vertIndexes[2]);
        }
    }
}

void LiquidInstance::updateLiquidMaterials(const HFrameDependantData &frameDependantData, IMapApi *mapApi) {
    for (auto &waterMaterial : m_liquidMaterials) {
        auto &waterChunk = waterMaterial->m_materialPS->getObject();
        if ((waterMaterial->liquidFlags & 1024) > 0) {// Ocean
            waterChunk.color = frameDependantData->closeOceanColor;
        } else if (waterMaterial->liquidFlags == 15) { //River/Lake
            //Query river color
            mathfu::vec3 closeRiverColor = {0, 0, 0};
            if (m_api->getConfig()->useCloseRiverColorForDB) {

                mathfu::vec3 waterPos = (mathfu::vec3(m_waterBBox.max) + mathfu::vec3(m_waterBBox.min)) / 2.0f;
                bool waterColorFound = true;
                if (m_api->getConfig()->colorOverrideHolder != nullptr) {
                    waterColorFound = false;

                    for (auto &riverOverride : *m_api->getConfig()->colorOverrideHolder) {
                        if (riverOverride.liquidObjectId == liquid_object || riverOverride.liquidType == liquidType) {
                            closeRiverColor = riverOverride.color.xyz();
                            waterColorFound = true;
                            break;
                        }
                    }
                }
                if (!waterColorFound) {
                    std::vector<LightResult> lightResults = {};
                    mapApi->getLightResultsFromDB(waterPos, m_api->getConfig(), lightResults, nullptr);
                    for (auto &_light : lightResults) {
                        closeRiverColor += mathfu::vec3(_light.closeRiverColor) * _light.blendCoef;
                    }
                    closeRiverColor = mathfu::vec3(closeRiverColor[2], closeRiverColor[1], closeRiverColor[0]);
                }
            }

            waterChunk.color = frameDependantData->closeRiverColor;
        } else {
            waterChunk.color = mathfu::vec4(waterMaterial->color, 0.7);
        }

        auto time = mapApi->getCurrentSceneTime();

        waterMaterial->m_materialPS->save();
    }
}

void LiquidInstance::collectMeshes(std::vector<HGSortableMesh> &transparentMeshes) {
    //TODO: Get time and right mesh instance for animation
    transparentMeshes.push_back(m_liquidMeshes[0]);
}
