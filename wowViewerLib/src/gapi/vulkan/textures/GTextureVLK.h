//
// Created by Deamon on 7/23/2018.
//

#ifndef AWEBWOWVIEWERCPP_GTEXTUREVLK_H
#define AWEBWOWVIEWERCPP_GTEXTUREVLK_H

class GFrameBufferVLK;
class GCommandBuffer;

#include "../IDeviceVulkan.h"
#include "../../interface/textures/ITexture.h"
#include "../bindable/DSBindable.h"


class GTextureVLK : public ITexture, public std::enable_shared_from_this<GTextureVLK>, public IDSBindable {
    friend class GFrameBufferVLK;
public:
    //Used for rendering to texture in framebuffer
    explicit GTextureVLK(IDeviceVulkan &device,
                         int width, int height,
                         bool xWrapTex, bool yWrapTex,
                         bool isDepthTexture,
                         const VkFormat textureFormatGPU,
                         VkSampleCountFlagBits numSamples,
                         int vulkanMipMapCount,
                         VkImageUsageFlags imageUsageFlags);

    //Dumb param is introduced only so that compiler would not invoke constructor with two bools, instead with one
    explicit GTextureVLK(IDeviceVulkan &device,
                         const VkImage &image,
                         const VkImageView &imageView,
                         bool dumbParam);

    explicit GTextureVLK(IDeviceVulkan &device, bool xWrapTex, bool yWrapTex, const std::function<void(const std::weak_ptr<GTextureVLK>&)> &onUpdateCallback);

    void createTexture(const HMipmapsVector &mipmaps, const VkFormat &textureFormatGPU, std::vector<uint8_t> unitedBuffer);
public:
    ~GTextureVLK() override;

    void readData(std::vector<uint8_t> &buff) override {};
    void loadData(int width, int height, void *data, ITextureFormat textureFormat) override;
    bool getIsLoaded() override;
    void createTexture(TextureFormat textureFormat, const HMipmapsVector &mipmaps) override {
        throw "Not Implemented in this class";
    }

    auto * getAndPlanDestroy() {
        if (!stagingBufferCreated || m_tempUpdateData == nullptr) {
            return (decltype(m_tempUpdateData)) nullptr;
        }
        auto *l_device = &m_device;
        auto *l_tempUpdateData = m_tempUpdateData;

        auto &l_stagingBuffer = m_tempUpdateData->stagingBuffer;
        auto &l_stagingBufferAlloc = m_tempUpdateData->stagingBufferAlloc;

        m_device.addDeallocationRecord([l_tempUpdateData, l_device, l_stagingBuffer, l_stagingBufferAlloc]() {
            vmaDestroyBuffer(l_device->getVMAAllocator(), l_stagingBuffer, l_stagingBufferAlloc);

            delete l_tempUpdateData;
        });

        m_tempUpdateData = nullptr;
        m_uploaded = true;
        m_loaded = true;

        return l_tempUpdateData;
    }
    TextureStatus postLoad() override;

    struct Texture {
        VkSampler sampler = VK_NULL_HANDLE;
        VkImage image;
        VkImageView view;
    } texture;
private:
    void createBuffer();
    void destroyBuffer();
    virtual void bind(); //Should be called only by GDevice
    void unbind();


    struct updateData {
        VkBuffer stagingBuffer;
        VmaAllocation stagingBufferAlloc = VK_NULL_HANDLE;
        VmaAllocationInfo stagingBufferAllocInfo = {};

        std::vector<VkBufferImageCopy> bufferCopyRegions = {};
    } * m_tempUpdateData = nullptr;


    VmaAllocation imageAllocation = VK_NULL_HANDLE;
    VmaAllocationInfo imageAllocationInfo = {};

    std::function<void(const std::weak_ptr<GTextureVLK>&)> m_onDataUpdate = nullptr;

protected:
    IDeviceVulkan &m_device;

    bool stagingBufferCreated = false;

    bool m_uploaded = false;
    bool m_loaded = false;
    bool m_wrapX = true;
    bool m_wrapY = true;

    int m_width = 0;
    int m_height = 0;


    void createVulkanImageObject(
        bool isDepthTexture,
        const VkFormat textureFormatGPU,
        VkSampleCountFlagBits numSamples,
        int vulkanMipMapCount,
        VkImageUsageFlags imageUsageFlags
    );
};

typedef std::shared_ptr<GTextureVLK> HGTextureVLK;


#endif //AWEBWOWVIEWERCPP_GTEXTUREVLK_H
