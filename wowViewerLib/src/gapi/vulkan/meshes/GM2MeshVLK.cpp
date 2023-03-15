//
// Created by Deamon on 7/22/2018.
//

#include "GM2MeshVLK.h"
#include "../GPipelineVLK.h"


GM2MeshVLK::GM2MeshVLK(const gMeshTemplate &meshTemplate,
                       const HMaterialVLK &material) : GMeshVLK(meshTemplate, material){
}

void GM2MeshVLK::setLayer(int layer) {
    m_layer = layer;
}

void GM2MeshVLK::setPriorityPlane(int priorityPlane) {
    m_priorityPlane = priorityPlane;
}

void GM2MeshVLK::setSortDistance(float distance) {
    m_sortDistance = distance;
}
float GM2MeshVLK::getSortDistance() {
    return m_sortDistance;
}