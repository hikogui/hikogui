#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : require

layout(push_constant) uniform push_constants {
    vec2 windowExtent;
    vec2 viewportScale;
} pushConstants;

layout(set = 0, binding = 0) uniform sampler bilinearSampler;
layout(set = 0, binding = 1) uniform texture2D textures[64];

layout(location = 0) in flat vec4 inClippingRectangle;
layout(location = 1) in vec3 inAtlasPosition;

layout(origin_upper_left) in vec4 gl_FragCoord;
layout(location = 0) out vec4 outColor;

bool isClipped()
{
    return greaterThanEqual(gl_FragCoord.xyxy, inClippingRectangle) != bvec4(true, true, false, false);
}

void main() {
    if (isClipped()) {
        discard;
        //outColor = vec4(1.0, 1.0, 0.0, 1.0); return;
    }

    int atlasTextureIndex = int(inAtlasPosition.z);
    vec2 textureCoord = inAtlasPosition.xy;

    // Vulkan blending operation expects pre-multiplied alpha and the texture map is in pre-multiplied alpha. 
    outColor = texture(sampler2D(textures[atlasTextureIndex], bilinearSampler), textureCoord);
}
