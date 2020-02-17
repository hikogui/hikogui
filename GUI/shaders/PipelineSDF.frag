#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform PushConstants {
    vec2 windowExtent;
    vec2 viewportScale;
} pushConstants;

layout(set = 0, binding = 0) uniform sampler samp;
layout(set = 0, binding = 1) uniform texture2D textures[16];

layout(location = 0) in vec4 inClippingRectangle;
layout(location = 1) in vec3 inTextureCoord;
layout(location = 2) in vec4 inColor;
layout(location = 3) in float inDistanceMultiplier;

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


    float distance = texture(sampler2D(textures[int(inTextureCoord.z)], samp), inTextureCoord.xy).r;

    float w = clamp(distance * inDistanceMultiplier + 0.5, 0.0, 1.0);
    outColor = inColor * vec4(w, w, w, w);
    
    if (w == 0.0) {
        // XXX draw shadow and set deeper depth to properly overlap shadows of
        // multiple glyphs.
        discard;
        outColor = vec4(1.0, 1.0, 1.0, 0.1);
    }
}
