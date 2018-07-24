//
// Created by deamon on 05.06.18.
//

#ifndef WEBWOWVIEWERCPP_GMESH_H
#define WEBWOWVIEWERCPP_GMESH_H

#include "GVertexBufferBindings.h"
#include "GBlpTexture.h"

class gMeshTemplate {
public:
    gMeshTemplate(HGVertexBufferBindings bindings, HGShaderPermutation shader) : bindings(bindings), shader(shader) {}
    HGVertexBufferBindings bindings;
    HGShaderPermutation shader;
    bool depthWrite;
    bool depthCulling;
    bool backFaceCulling;
    EGxBlendEnum blendMode;

    int start;
    int end;
    int element;
    int textureCount;
    HGTexture texture[4] = {nullptr,nullptr,nullptr,nullptr};
    HGUniformBuffer vertexBuffers[3] = {nullptr,nullptr,nullptr};
    HGUniformBuffer fragmentBuffers[3] = {nullptr,nullptr,nullptr};
};

enum class MeshType {
    eGeneralMesh = 0,
    eM2Mesh = 1,
};

class GMesh {
    friend class GDevice;

protected:
    explicit GMesh(GDevice &device,
                   const gMeshTemplate &meshTemplate
    );

public:
    ~GMesh();
    HGUniformBuffer getVertexUniformBuffer(int slot) {
        return m_vertexUniformBuffer[slot];
    }
    HGUniformBuffer getFragmentUniformBuffer(int slot) {
        return m_fragmentUniformBuffer[slot];
    }
    EGxBlendEnum getGxBlendMode() { return m_blendMode; }
    bool getIsTransparent() { return m_isTransparent; }
    MeshType getMeshType() {
        return m_meshType;
    }

    void setEnd(int end) {m_end = end; };
protected:
    MeshType m_meshType;
private:
    HGVertexBufferBindings m_bindings;
    HGShaderPermutation m_shader;

    HGUniformBuffer m_vertexUniformBuffer[3] = {nullptr, nullptr, nullptr};
    HGUniformBuffer m_fragmentUniformBuffer[3] = {nullptr, nullptr, nullptr};
    HGTexture m_texture[4];

    int8_t m_depthWrite;
    int8_t m_depthCulling;
    int8_t m_backFaceCulling;
    EGxBlendEnum m_blendMode;
    bool m_isTransparent;

    int m_start;
    int m_end;
    int m_element;

    int m_textureCount;

private:
    GDevice &m_device;
};


#endif //WEBWOWVIEWERCPP_GMESH_H
