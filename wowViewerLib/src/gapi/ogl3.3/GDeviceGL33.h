//
// Created by deamon on 05.06.18.
//

#ifndef WEBWOWVIEWERCPP_GDEVICE33_H
#define WEBWOWVIEWERCPP_GDEVICE33_H

#include <memory>

#ifndef __EMSCRIPTEN__
#define SINGLE_BUFFER_UPLOAD
#endif

class GVertexBufferGL33;
class GVertexBufferBindingsGL33;
class GIndexBufferGL33;
class GUniformBufferGL33;
class GBlpTextureGL33;
class GTextureGL33;
class GShaderPermutationGL33;
class GMeshGL33;
class GM2MeshGL33;
class GOcclusionQueryGL33;
class GParticleMeshGL33;


class gMeshTemplate;

typedef std::shared_ptr<GUniformBufferGL33> HGL33UniformBuffer;
typedef std::shared_ptr<GMeshGL33> HGL33Mesh;

#include <unordered_set>
#include <list>
#include <array>
#include "GVertexBufferBindingsGL33.h"
#include "buffers/GIndexBufferGL33.h"
#include "buffers/GVertexBufferGL33.h"
#include "textures/GTextureGL33.h"
#include "textures/GBlpTextureGL33.h"
#include "buffers/GUniformBufferGL33.h"
#include "GShaderPermutationGL33.h"
#include "meshes/GMeshGL33.h"
#include "../interface/IDevice.h"



class GDeviceGL33 : public IDevice {
public:
    GDeviceGL33();
    ~GDeviceGL33() override {};

    void reset() override;

    unsigned int getUpdateFrameNumber() override;
    unsigned int getCullingFrameNumber() override;
    unsigned int getDrawFrameNumber() override;


    void increaseFrameNumber() override;
    bool getIsAsynBuffUploadSupported() override {
        return false;
    }

    float getAnisLevel() override;

    void bindProgram(IShaderPermutation *program) override;

    void bindIndexBuffer(IIndexBuffer *buffer) override;
    void bindVertexBuffer(IVertexBuffer *buffer) override;
    void bindUniformBuffer(IUniformBuffer *buffer, int slot, int offset, int length) override;

    void bindVertexBufferBindings(IVertexBufferBindings *buffer) override;

    void bindTexture(ITexture *texture, int slot) override;

    void updateBuffers(std::vector<HGMesh> &meshes) override;
    void prepearMemoryForBuffers(std::vector<HGMesh> &meshes) override {};
    void uploadTextureForMeshes(std::vector<HGMesh> &meshes) override;
    void drawMeshes(std::vector<HGMesh> &meshes) override;
    //    void drawM2Meshes(std::vector<HGM2Mesh> &meshes);
public:
    std::shared_ptr<IShaderPermutation> getShader(std::string shaderName, void *permutationDescriptor) override;

    HGUniformBuffer createUniformBuffer(size_t size) override;
    HGVertexBuffer createVertexBuffer() override;
    HGVertexBufferDynamic createVertexBufferDynamic(size_t size) override;
    HGIndexBuffer createIndexBuffer() override;
    HGVertexBufferBindings createVertexBufferBindings() override;
    HGUniformBufferChunk createUniformBufferChunk(size_t size) override;

    HGTexture createBlpTexture(HBlpTexture &texture, bool xWrapTex, bool yWrapTex) override;
    HGTexture createTexture() override;
    HGTexture getWhiteTexturePixel() override { return m_whitePixelTexture; };
    HGTexture getBlackTexturePixel() override { return m_blackPixelTexture; };
    HGMesh createMesh(gMeshTemplate &meshTemplate) override;
    HGM2Mesh createM2Mesh(gMeshTemplate &meshTemplate) override;
    HGParticleMesh createParticleMesh(gMeshTemplate &meshTemplate) override;
    HGPUFence createFence() override;

    HGOcclusionQuery createQuery(HGMesh boundingBoxMesh) override;

    HGVertexBufferBindings getBBVertexBinding() override;
    HGVertexBufferBindings getBBLinearBinding() override;

    std::string loadShader(std::string fileName, IShaderType shaderType) override;

    virtual void clearScreen() override;
    void setClearScreenColor(float r, float g, float b) override ;
    void beginFrame() override ;
    void commitFrame() override ;
    void setViewPortDimensions(float x, float y, float width, float height) override;

    void shrinkData() override;

    struct DeallocationRecord {
        unsigned int frameNumberToDoAt;
        std::function<void()> callback;
    };

    void addDeallocationRecord(std::function<void()> callback) {
        DeallocationRecord dr;
        dr.frameNumberToDoAt = m_frameNumber+4;
        dr.callback = callback;
        listOfDeallocators.push_back(dr);
    };
private:
    void drawMesh(HGMesh hmesh);
    bool isDepthPreFill = false;
protected:
    struct BlpCacheRecord {
        BlpTexture* texture;
        bool wrapX;
        bool wrapY;

        bool operator==(const BlpCacheRecord &other) const {
          return
              (texture == other.texture) &&
              (wrapX == other.wrapX) &&
              (wrapY == other.wrapY);

        };
    };
    struct BlpCacheRecordHasher {
        std::size_t operator()(const BlpCacheRecord& k) const {
            using std::hash;
            return hash<void*>{}(k.texture) ^ (hash<bool>{}(k.wrapX) << 8) ^ (hash<bool>{}(k.wrapY) << 16);
        };
    };
    std::unordered_map<BlpCacheRecord, std::weak_ptr<GTextureGL33>, BlpCacheRecordHasher> loadedTextureCache;

    unsigned int m_frameNumber = 0;

    uint8_t m_lastColorMask = 0xFF;
    int8_t m_lastDepthWrite = -1;
    int8_t m_lastDepthCulling = -1;
    int8_t m_backFaceCulling = -1;
    int8_t m_triCCW = -1;
    int maxUniformBufferSize = -1;
    int uniformBufferOffsetAlign = -1;
    float m_anisotropicLevel = 0.0;
    bool m_isInSkyBoxDepthMode = false;
    EGxBlendEnum m_lastBlendMode = EGxBlendEnum::GxBlend_UNDEFINED;
    GIndexBufferGL33 *m_lastBindIndexBuffer = nullptr;
	IVertexBuffer* m_lastBindVertexBuffer = nullptr;
    GVertexBufferBindingsGL33 *m_vertexBufferBindings = nullptr;
    GShaderPermutationGL33 * m_shaderPermutation = nullptr;

    HGVertexBufferBindings m_vertexBBBindings;
    HGVertexBufferBindings m_lineBBBindings;
    HGVertexBufferBindings m_defaultVao;

    GTextureGL33 *m_lastTexture[10] = {
        nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,
        nullptr};

    struct GUBOMappingRec { GUniformBufferGL33 * buffer = nullptr; uint32_t offset = 0;};
    std::array<GUBOMappingRec, 5> m_UniformBuffer;

    HGTexture m_blackPixelTexture;
    HGTexture m_whitePixelTexture;

    float clearColor[3] = {0,0,0};

public:
    struct M2ShaderCacheRecordHasher {
        std::size_t operator()(const M2ShaderCacheRecord& k) const {
            using std::hash;
            return hash<int>{}(k.vertexShader) ^ (hash<int>{}(k.pixelShader)) ^
                   (hash<bool>{}(k.unlit)) ^
                   (hash<bool>{}(k.alphaTestOn) << 8) ^
                   (hash<bool>{}(k.unFogged) << 16) ^
                   (hash<bool>{}(k.unShadowed) << 24);
        };
    };
    std::unordered_map<M2ShaderCacheRecord, std::weak_ptr<IShaderPermutation>, M2ShaderCacheRecordHasher> m2ShaderCache;

    struct WMOShaderCacheRecordHasher {
        std::size_t operator()(const WMOShaderCacheRecord& k) const {
            using std::hash;
            return hash<int>{}(k.vertexShader) ^ (hash<int>{}(k.pixelShader)) ^
                   (hash<bool>{}(k.unlit)) ^
                   (hash<bool>{}(k.alphaTestOn) << 8) ^
                   (hash<bool>{}(k.unFogged) << 16) ^
                   (hash<bool>{}(k.unShadowed) << 24);
        };
    };
    std::unordered_map<WMOShaderCacheRecord, std::weak_ptr<IShaderPermutation>, WMOShaderCacheRecordHasher> wmoShaderCache;
protected:
    //Caches
    std::unordered_map<size_t, HGShaderPermutation> m_shaderPermutCache;
    std::list<std::weak_ptr<GUniformBufferGL33>> m_unfiormBufferCache;
#ifdef SINGLE_BUFFER_UPLOAD
    struct FrameUniformBuffers {
        HGUniformBuffer m_uniformBufferForUpload;
    };
#else
    struct FrameUniformBuffers {
        std::vector<HGUniformBuffer> m_uniformBuffersForUpload;
    };
#endif


    std::array<FrameUniformBuffers, 4> m_UBOFrames = {};

    std::vector<char> aggregationBufferForUpload;

    std::unordered_map<ShaderContentCacheRecord, std::string, ShaderContentCacheRecordHasher> shaderCache;

    int uniformBuffersCreated = 0;
    std::list<DeallocationRecord> listOfDeallocators;
};




#endif //WEBWOWVIEWERCPP_GDEVICE_H
