#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform sampler samp;
layout(set = 0, binding = 1) uniform texture2D textures[16];

layout(location = 0) in vec3 inTextureCoord;
layout(location = 1) in vec4 inColor;
layout(location = 2) in float inDistanceMultiplier;

layout(location = 0) out vec4 outColor;

void main() {
    float distance = texture(sampler2D(textures[int(inTextureCoord.z)], samp), inTextureCoord.xy).r;

    float w = clamp(distance * inDistanceMultiplier + 0.5, 0.0, 1.0);
    outColor = inColor * vec4(w, w, w, w);    
}
