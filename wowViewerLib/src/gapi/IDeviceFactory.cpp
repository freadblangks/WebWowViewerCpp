//
// Created by Deamon on 9/4/2018.
//

#include <GL/glew.h>
#include "IDeviceFactory.h"

#include "ogl2.0/GDeviceGL20.h"
#include "ogl3.3/GDeviceGL33.h"
#ifdef LINK_OGL4
#include "ogl4.x/GDeviceGL4x.h"
#endif
#ifndef SKIP_VULKAN
#include "vulkan/GDeviceVulkan.h"
#endif

void initOGLPointers(){
//#ifdef _WIN32
    glewExperimental = true; // Needed in core profile
    auto result = glewInit();

    if (result != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
    }
//#endif
}

IDevice *IDeviceFactory::createDevice(std::string gapiName, void * data) {
#ifdef LINK_OGL2
    if (gapiName == "ogl2") {
        initOGLPointers();
        return new GDeviceGL20();
    } else
#endif

#ifdef LINK_OGL3
    if (gapiName == "ogl3") {
        initOGLPointers();
        return new GDeviceGL33();
    } else
#endif
#ifdef LINK_OGL4
    if (gapiName == "ogl4") {
        initOGLPointers();
//        return new GDeviceGL4x();
    } else
#endif


#ifndef SKIP_VULKAN
    if (gapiName == "vulkan") {
        return new GDeviceVLK((vkCallInitCallback *) data);
    } else
#endif
    {}

    return nullptr;
}

