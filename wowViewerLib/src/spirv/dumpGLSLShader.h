//
// Created by deamon on 04.12.19.
//

#ifndef AWEBWOWVIEWERCPP_DUMPGLSLSHADER_H
#define AWEBWOWVIEWERCPP_DUMPGLSLSHADER_H

#include <iostream>
#include "webGLSLCompiler.h"
#include "fileHelpers.h"

void dumpGLSLText(std::vector<std::string> &shaderFilePaths, int glslVersion) {

    // Read SPIR-V from disk or similar.
    spirv_cross::WebGLSLCompiler::Options options;

    for (auto &filePath : shaderFilePaths) {
        std::vector<uint32_t> spirv_binary = readFile(filePath);

        std::string fileName = basename(filePath);
        auto tokens = split(fileName, '.');

        spirv_cross::WebGLSLCompiler glsl(std::move(spirv_binary));

        // The SPIR-V is now parsed, and we can perform reflection on it.
        spirv_cross::ShaderResources resources = glsl.get_shader_resources();

//         Get all sampled images in the shader.
        for (auto &resource : resources.uniform_buffers)
        {
            unsigned set = glsl.get_decoration(resource.id, spv::DecorationDescriptorSet);
            unsigned binding = glsl.get_decoration(resource.id, spv::DecorationBinding);
//            printf("Image %s at set = %u, binding = %u\n", resource.name.c_str(), set, binding);

            // Modify the decoration to prepare it for GLSL.
            glsl.unset_decoration(resource.id, spv::DecorationDescriptorSet);

            // Some arbitrary remapping if we want.

            if (glslVersion >= 300) {
                glsl.unset_decoration(resource.id, spv::DecorationBinding);
                options.enable_420pack_extension = false;
            } else {
                glsl.unset_decoration(resource.id, spv::DecorationBinding);
                options.enable_420pack_extension = false;
            }

        }

        // Set some options.

        options.version = glslVersion;
        options.es = false;
        glsl.set_common_options(options);


        // Compile to GLSL, ready to give to GL driver.
        std::string source = glsl.compile();
        std::cout << source << std::endl;

    }

}
#endif //AWEBWOWVIEWERCPP_DUMPGLSLSHADER_H
