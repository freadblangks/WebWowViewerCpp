#ifdef COMPILING_VS
/* vertex shader code */
layout(location = 0) in vec3 aHeight;
layout(location = 1) in vec4 aColor;
layout(location = 2) in vec4 aVertexLighting;
layout(location = 3) in vec3 aNormal;
layout(location = 4) in float aIndex;

layout(std140) uniform sceneWideBlockVSPS {
    mat4 uLookAtMat;
    mat4 uPMatrix;
};
layout(std140) uniform meshWideBlockVS {
    vec4 uPos;
};

mat3 blizzTranspose(mat4 value) {
    return mat3(
        value[0].xyz,
        value[1].xyz,
        value[2].xyz
    );
}

out vec2 vChunkCoords;
out vec3 vPosition;
out vec4 vColor;
out vec3 vNormal;
out vec3 vVertexLighting;

const float UNITSIZE_X =  (1600.0 / 3.0) / 16.0 / 8.0;
const float UNITSIZE_Y =  (1600.0 / 3.0) / 16.0 / 8.0;

void main() {

/*
     Y
  X  0    1    2    3    4    5    6    7    8
        9   10   11   12   13   14   15   16
*/
    float iX = mod(aIndex, 17.0);
    float iY = floor(aIndex/17.0);

    if (iX > 8.01) {
        iY = iY + 0.5;
        iX = iX - 8.5;
    }

//    vec4 worldPoint = vec4(
//        uPos.x - iY * UNITSIZE_Y,
//        uPos.y - iX * UNITSIZE_X,
//        uPos.z + aHeight,
//        1);

    vec4 worldPoint = vec4(aHeight, 1.0);

    vChunkCoords = vec2(iX, iY);

    vPosition = (uLookAtMat * worldPoint).xyz;
    vColor = aColor;
    vVertexLighting = aVertexLighting.rgb;
    vNormal = (uLookAtMat * vec4(aNormal, 0)).xyz;

    gl_Position = uPMatrix * uLookAtMat * worldPoint;
}
#endif //COMPILING_VS

#ifdef COMPILING_FS
precision highp float;

in vec2 vChunkCoords;
in vec3 vPosition;
in vec4 vColor;
in vec3 vNormal;
in vec3 vVertexLighting;

uniform sampler2D uAlphaTexture;
uniform sampler2D uLayer0;
uniform sampler2D uLayer1;
uniform sampler2D uLayer2;
uniform sampler2D uLayer3;
uniform sampler2D uLayerHeight0;
uniform sampler2D uLayerHeight1;
uniform sampler2D uLayerHeight2;
uniform sampler2D uLayerHeight3;

layout(std140) uniform modelWideBlockPS {
    vec4 uViewUp;
    vec4 uSunDir_FogStart;
    vec4 uSunColor_uFogEnd;
    vec4 uAmbientLight;
    vec4 FogColor;
};

layout(std140) uniform meshWideBlockPS {
    vec4 uHeightScale;
    vec4 uHeightOffset;
};

out vec4 outColor;

vec3 makeDiffTerm(vec3 matDiffuse) {
  vec3 currColor;
    float mult = 1.0;
    vec3 lDiffuse = vec3(0.0, 0.0, 0.0);
    if (true) {
        vec3 normalizedN = normalize(vNormal);
        float nDotL = dot(normalizedN, -(uSunDir_FogStart.xyz));
        float nDotUp = dot(normalizedN, uViewUp.xyz);

        vec4 AmbientLight = uAmbientLight;

        vec3 adjAmbient = (AmbientLight.rgb );
        vec3 adjHorizAmbient = (AmbientLight.rgb );
        vec3 adjGroundAmbient = (AmbientLight.rgb );

        if ((nDotUp >= 0.0))
        {
            currColor = mix(adjHorizAmbient, adjAmbient, vec3(nDotUp));
        }
        else
        {
            currColor= mix(adjHorizAmbient, adjGroundAmbient, vec3(-(nDotUp)));
        }

        vec3 skyColor = (currColor * 1.10000002);
        vec3 groundColor = (currColor* 0.699999988);

        lDiffuse = (uSunColor_uFogEnd.xyz * clamp(nDotL, 0.0, 1.0));
        currColor = mix(groundColor, skyColor, vec3((0.5 + (0.5 * nDotL))));
    } else {
        currColor = vec3 (1.0, 1.0, 1.0) ;
        mult = 1.0;
    }
    vec3 localDiffuse = vVertexLighting;

    vec3 gammaDiffTerm = matDiffuse * (currColor + lDiffuse);
    vec3 linearDiffTerm = (matDiffuse * matDiffuse) * localDiffuse;
    return sqrt(gammaDiffTerm*gammaDiffTerm + linearDiffTerm) ;
}


void main() {
    vec2 vTexCoord = vChunkCoords;
    const float threshold = 1.5;

    vec2 alphaCoord = vec2(vChunkCoords.x/8.0, vChunkCoords.y/8.0 );
    vec3 alphaBlend = texture( uAlphaTexture, alphaCoord).gba;

    vec2 tcLayer0 = vTexCoord;
    vec2 tcLayer1 = vTexCoord;
    vec2 tcLayer2 = vTexCoord;
    vec2 tcLayer3 = vTexCoord;

    float minusAlphaBlendSum = (1.0 - clamp(dot(alphaBlend, vec3(1.0)), 0.0, 1.0));
    vec4 weightsVector = vec4(minusAlphaBlendSum, alphaBlend);
    float weightedTexture_x = (minusAlphaBlendSum * ((texture(uLayerHeight0, tcLayer0).w * uHeightScale[0]) + uHeightOffset[0]));
    float weightedTexture_y = (weightsVector.y * ((texture(uLayerHeight1, tcLayer1).w * uHeightScale[1]) + uHeightOffset[1]));
    float weightedTexture_z = (weightsVector.z * ((texture(uLayerHeight2, tcLayer2).w * uHeightScale[2]) + uHeightOffset[2]));
    float weightedTexture_w = (weightsVector.w * ((texture(uLayerHeight3, tcLayer3).w * uHeightScale[3]) + uHeightOffset[3]));
    vec4 weights = vec4(weightedTexture_x, weightedTexture_y, weightedTexture_z, weightedTexture_w);
    vec4 weights_temp = (weights * (vec4(1.0) - clamp((vec4(max(max(weightedTexture_x, weightedTexture_y), max(weightedTexture_z, weightedTexture_w))) - weights), 0.0, 1.0)));
    vec4 weightsNormalized = (weights_temp / vec4(dot(vec4(1.0), weights_temp)));

    
    vec4 weightedLayer_0 = (texture(uLayer0, tcLayer0) * weightsNormalized.x);
    vec3 matDiffuse_0 = weightedLayer_0.xyz;
    float specBlend_0 = weightedLayer_0.w;

    vec4 weightedLayer_1 = (texture(uLayer1, tcLayer1) * weightsNormalized.y);
    vec3 matDiffuse_1 = (matDiffuse_0 + weightedLayer_1.xyz);
    float specBlend_1 = (specBlend_0 + weightedLayer_1.w);

    vec4 weightedLayer_2 = (texture(uLayer2, tcLayer1) * weightsNormalized.z);
    vec3 matDiffuse_2 = (matDiffuse_1 + weightedLayer_2.xyz);
    float specBlend_2 = (specBlend_1 + weightedLayer_2.w);

    vec4 weightedLayer_3 = (texture(uLayer3, tcLayer1) * weightsNormalized.w);
    vec3 matDiffuse_3 = (matDiffuse_2 + weightedLayer_3.xyz);
    float specBlend_3 = (specBlend_2 + weightedLayer_3.w);

    vec4 final = vec4(matDiffuse_3, specBlend_3);

    vec3 matDiffuse = final.rgb *  vColor.rgb;
    vec4 finalColor = vec4(makeDiffTerm(matDiffuse), 1.0);

    //Spec part
    float specBlend = final.a;
    vec3 halfVec = -(normalize((uSunDir_FogStart.xyz + normalize(vPosition))));
    vec3 lSpecular = ((uSunColor_uFogEnd.xyz * pow(max(0.0, dot(halfVec, vNormal)), 20.0)));
    vec3 specTerm = (vec3(specBlend) * lSpecular);
    finalColor.rgb += specTerm;

    // --- Fog start ---
    /*
    vec3 fogColor = uFogColor;
    float fog_start = uFogStart;
    float fog_end = uFogEnd;
    float fog_rate = 1.5;
    float fog_bias = 0.01;

    //vec4 fogHeightPlane = pc_fog.heightPlane;
    //float heightRate = pc_fog.color_and_heightRate.w;

    float distanceToCamera = length(vPosition.xyz);
    float z_depth = (distanceToCamera - fog_bias);
    float expFog = 1.0 / (exp((max(0.0, (z_depth - fog_start)) * fog_rate)));
    //float height = (dot(fogHeightPlane.xyz, vPosition.xyz) + fogHeightPlane.w);
    //float heightFog = clamp((height * heightRate), 0, 1);
    float heightFog = 1.0;
    expFog = (expFog + heightFog);
    float endFadeFog = clamp(((fog_end - distanceToCamera) / (0.699999988 * fog_end)), 0.0, 1.0);

    finalColor.rgb = mix(fogColor.rgb, finalColor.rgb, vec3(min(expFog, endFadeFog)));
    */
    // --- Fog end ---

    finalColor.a = 1.0;
    outColor = finalColor;
}

#endif //COMPILING_FS