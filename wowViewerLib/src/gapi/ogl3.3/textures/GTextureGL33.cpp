//
// Created by Deamon on 7/23/2018.
//
#include <iostream>
#include "../GDeviceGL33.h"
#include "GTextureGL33.h"
#include "../../../engine/opengl/header.h"
#include "../../interface/IDevice.h"

GTextureGL33::GTextureGL33(IDevice &device) : m_device(dynamic_cast<GDeviceGL33 &>(device)) {
    createBuffer();
}

GTextureGL33::~GTextureGL33() {
    destroyBuffer();
}

void GTextureGL33::createBuffer() {
    glGenTextures(1, &textureIdentifier);
}

void GTextureGL33::destroyBuffer() {
    const GLuint indent = textureIdentifier;
    m_device.addDeallocationRecord([indent]() -> void {
        glDeleteTextures(1, &indent);
    });
}

void GTextureGL33::bind() {
    glBindTexture(GL_TEXTURE_2D, textureIdentifier);

}

void GTextureGL33::unbind() {
    glBindTexture(GL_TEXTURE_2D, 0);
}

bool GTextureGL33::getIsLoaded() {
    return true;
}

static int pureTexturesUploaded = 0;
void GTextureGL33::loadData(int width, int height, void *data) {
//    std::cout << "pureTexturesUploaded = " << pureTexturesUploaded++ << std::endl;

    m_device.bindTexture(this, 0);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenerateMipmap(GL_TEXTURE_2D);

    m_device.bindTexture(nullptr, 0);
}


