#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform PushConstants {
    vec2 windowExtent;
    vec2 viewportScale;
} pushConstants;

layout(set = 0, binding = 0) uniform sampler biLinearSampler;
layout(set = 0, binding = 1) uniform texture2D textures[16];

layout(location = 0) in flat vec4 inClippingRectangle;
layout(location = 1) in vec3 inTextureCoord;
layout(location = 2) in flat vec4 inColor;
layout(location = 3) in flat float inDistanceMultiplier;
layout(location = 4) in flat float inShadowSize;

layout(location = 0) out vec4 outColor;

bool isClipped()
{
    return (
        gl_FragCoord.x < inClippingRectangle.x ||
        gl_FragCoord.x > inClippingRectangle.z ||
        gl_FragCoord.y < inClippingRectangle.y ||
        gl_FragCoord.y > inClippingRectangle.w
    );
}

void main()
{
    if (isClipped()) {
        discard;
        //outColor = vec4(1.0, 1.0, 0.0, 1.0); return;
    }

    // Distance in screen coordinates from closest edge.
    float distance = texture(sampler2D(textures[int(inTextureCoord.z)], biLinearSampler), inTextureCoord.xy).r * inDistanceMultiplier;

    float glyph = clamp(distance + 0.5, 0.0, 1.0);
    
    if (inShadowSize > 0.0) {
        if (glyph >= 1.0) {
            outColor = inColor;

        } else {
            float shadow = clamp(distance * inShadowSize + 0.5, 0.0, 1.0);

            if (shadow > 0.0) {
                vec4 shadowColor = vec4(0.0, 0.0, 0.0, shadow);
                vec4 glyphColor = inColor * glyph;
                outColor = glyphColor + shadowColor * (1.0 - glyph);

            } else {
                discard;
            }
        }

    } else if (glyph > 0.0) {
        outColor = inColor * glyph;

    } else {
        discard;
    }
}
