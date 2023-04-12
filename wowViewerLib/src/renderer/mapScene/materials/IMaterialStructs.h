//
// Created by Deamon on 3/5/2023.
//

#ifndef AWEBWOWVIEWERCPP_IMATERIALSTRUCTS_H
#define AWEBWOWVIEWERCPP_IMATERIALSTRUCTS_H

#include "../../../gapi/interface/materials/IMaterial.h"
#include "../../../gapi/UniformBufferStructures.h"
//----------------------------
// Material Templates
//----------------------------

struct M2MaterialTemplate {
    int vertexShader = 0;
    int pixelShader = 0;
    std::array<HGTexture, 4> textures = {nullptr, nullptr, nullptr, nullptr};
};

struct M2ParticleMaterialTemplate {
    std::array<HGTexture, 3> textures = {nullptr, nullptr, nullptr};
};

struct WMOMaterialTemplate {
    std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> m_modelWide;

    std::array<HGTexture, 9> textures = {nullptr, nullptr, nullptr,
                                         nullptr, nullptr, nullptr,
                                         nullptr, nullptr, nullptr};
};

struct ADTMaterialTemplate {
    std::array<HGTexture, 9> textures = {nullptr, nullptr, nullptr,
                                         nullptr, nullptr, nullptr,
                                         nullptr, nullptr, nullptr};
};

struct WaterMaterialTemplate {
    HGTexture texture = nullptr;
    mathfu::vec3 color;
    int liquidFlags;
};

class IM2ModelData {
public:
    std::shared_ptr<IBufferChunk<M2::PlacementMatrix>> m_placementMatrix = nullptr;
    std::shared_ptr<IBufferChunk<M2::Bones>> m_bonesData = nullptr;
    std::shared_ptr<IBufferChunk<M2::M2Colors>> m_colors = nullptr;
    std::shared_ptr<IBufferChunk<M2::TextureWeights>> m_textureWeights = nullptr;
    std::shared_ptr<IBufferChunk<M2::TextureMatrices>> m_textureMatrices = nullptr;
    std::shared_ptr<IBufferChunk<M2::modelWideBlockPS>> m_modelFragmentData = nullptr;
};

class IM2Material : public IMaterial {
public:
    int vertexShader;
    int pixelShader;
    EGxBlendEnum blendMode;
    std::shared_ptr<IBufferChunk<M2::meshWideBlockVSPS>> m_vertexFragmentData = nullptr;
    };
class IM2ParticleMaterial : public IMaterial {
public:
    std::shared_ptr<IBufferChunk<Particle::meshParticleWideBlockPS>> m_fragmentData = nullptr;
};

class ISkyMeshMaterial : public IMaterial {
public:
    std::shared_ptr<IBufferChunk<DnSky::meshWideBlockVS>> m_skyColors = nullptr;
};

class IWMOMaterial : public IMaterial {
public:
    std::shared_ptr<IBufferChunk<WMO::meshWideBlockVS>> m_materialVS= nullptr;
    std::shared_ptr<IBufferChunk<WMO::meshWideBlockPS>> m_materialPS = nullptr;
};

class IADTMaterial : public IMaterial {
public:
    std::shared_ptr<IBufferChunk<ADT::meshWideBlockVSPS>> m_materialVSPS = nullptr;
    std::shared_ptr<IBufferChunk<ADT::meshWideBlockPS>> m_materialPS = nullptr;
};

class IWaterMaterial : public IMaterial {
public:
    mathfu::vec3 color;
    int liquidFlags;
    int materialId;
    float scrollSpeedX;
    float scrollSpeedY;
    std::shared_ptr<IBufferChunk<Water::meshWideBlockPS>> m_materialPS = nullptr;
};

#endif //AWEBWOWVIEWERCPP_IMATERIALSTRUCTS_H
