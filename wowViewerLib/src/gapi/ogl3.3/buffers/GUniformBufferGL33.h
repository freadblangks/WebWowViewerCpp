//
// Created by Deamon on 6/30/2018.
//

#ifndef AWEBWOWVIEWERCPP_GUNIFORMBUFFER_H
#define AWEBWOWVIEWERCPP_GUNIFORMBUFFER_H


#include <cstdio>
#include <cassert>
#include "../GDeviceGL33.h"
#include "../../interface/buffers/IUniformBuffer.h"

class GUniformBufferGL33 : public IUniformBuffer {
public:
    friend class GDeviceGL33;

    explicit GUniformBufferGL33(IDevice &device, size_t size);
    ~GUniformBufferGL33() override;

    void setIdentifierBuffer(void * ptr, uint32_t offset) {
        uint8_t frameIndex = (m_device.getFrameNumber() + 1) & 1;
        pIdentifierBuffer[frameIndex] = ptr;
        m_offset[frameIndex] = offset;
    }
    void * getIdentifierBuffer() {
        if (m_buffCreated) {
            return pIdentifierBuffer[0];
        }

        return pIdentifierBuffer[m_device.getFrameNumber() & 1];
    }

    void *getPointerForModification() override;
    void *getPointerForUpload() override;

    void save(bool initialSave = false) override;
    void createBuffer() override;
private:

    void destroyBuffer();
    void bind(int bindingPoint); //Should be called only by GDevice
    void unbind();

private:
    void uploadData(void * data, int length);

private:
    IDevice &m_device;

private:
    size_t m_size;
    size_t m_offset[2] = {0, 0};
    void * pIdentifierBuffer[2];

    void * pFrameOneContent;
    void * pFrameTwoContent;
    bool m_buffCreated = false;
    bool m_dataUploaded = false;

    int m_creationIndex = 0;

    bool m_needsUpdate = false;
};


#endif //AWEBWOWVIEWERCPP_GUNIFORMBUFFER_H
