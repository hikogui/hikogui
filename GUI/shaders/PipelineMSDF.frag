#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform PushConstants {
    vec2 windowExtent;
    vec2 viewportScale;
} pushConstants;

layout(set = 0, binding = 0) uniform sampler samp;
layout(set = 0, binding = 1) uniform texture2D textures[16];

layout(location = 0) in vec2 inClippingRectangleMinimum;
layout(location = 1) in vec2 inClippingRectangleMaximum;
layout(location = 2) in vec3 inAtlasPosition;
layout(location = 3) in vec4 inColor;

layout(origin_upper_left) in vec4 gl_FragCoord;
layout(location = 0) out vec4 outColor;

float median(float a, float b, float c) {
    return max(min(a, b), min(max(a, b), c));
}

float distanceRange = 4.0;
float distanceMultiplier = distanceRange * 2.0;

void main() {
    if (clamp(gl_FragCoord.xy, inClippingRectangleMinimum, inClippingRectangleMaximum) != gl_FragCoord.xy) {
        //discard;
        outColor = vec4(1.0, 1.0, 0.0, 1.0);

    } else {

        int atlasTextureIndex = int(inAtlasPosition.z);
        vec2 textureCoord = inAtlasPosition.xy;

        vec4 distances = texture(sampler2D(textures[atlasTextureIndex], samp), textureCoord);
        float distance = (median(distances.x, distances.y, distances.z) - 0.5) * distanceMultiplier;
        float w = clamp(distance / fwidth(distance) + 0.5, 0.0, 1.0);
        //float w = distance > 0.0 ? 1.0 : 0.0;

        //if (w < 0.2) {
        //    outColor = vec4(1.0, 0.0, 1.0, 1.0);
        //} else {
            outColor = inColor * vec4(w, w, w, w);
        //}
        //outColor = vec4(0.0, 0.0, 0.0, w);
    }
}
